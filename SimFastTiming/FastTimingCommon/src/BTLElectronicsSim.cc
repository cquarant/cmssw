#include "SimFastTiming/FastTimingCommon/interface/BTLElectronicsSim.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/ForwardDetId/interface/BTLDetId.h"

#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandGaussQ.h"

#include "Math/ChebyshevPol.h"

using namespace mtd;

BTLElectronicsSim::BTLElectronicsSim(const edm::ParameterSet& pset, edm::ConsumesCollector iC)
    : debug_(pset.getUntrackedParameter<bool>("debug", false)),
      bxTime_(pset.getParameter<double>("bxTime")),
      testBeamMIPTimeRes_(pset.getParameter<double>("TestBeamMIPTimeRes")),
      scintillatorRiseTime_(pset.getParameter<double>("ScintillatorRiseTime")),
      scintillatorDecayTime_(pset.getParameter<double>("ScintillatorDecayTime")),
      channelTimeOffset_(pset.getParameter<double>("ChannelTimeOffset")),
      smearChannelTimeOffset_(pset.getParameter<double>("SmearChannelTimeOffset")),
      energyThreshold_(pset.getParameter<double>("EnergyThreshold")),
      timeThreshold1_(pset.getParameter<double>("TimeThreshold1")),
      timeThreshold2_(pset.getParameter<double>("TimeThreshold2")),
      referencePulseNpe_(pset.getParameter<double>("ReferencePulseNpe")),
      sigmaDigitization_(pset.getParameter<double>("SigmaDigitization")),
      sigmaClockGlobal_(pset.getParameter<double>("SigmaClockGlobal")),
      sigmaClockRU_(pset.getParameter<double>("SigmaClockRU")),
      paramDCR_(pset.getParameter<std::vector<double>>("DCRParam")),
      darkCountRate_(pset.getParameter<double>("DarkCountRate")),
      paramSR_(pset.getParameter<std::vector<double>>("SlewRateParam")),
      sigmaElectronicNoise_(pset.getParameter<double>("SigmaElectronicNoise")),
      electronicGain_(pset.getParameter<double>("ElectronicGain")),
      smearTimeForOOTtails_(pset.getParameter<bool>("SmearTimeForOOTtails")),
      npe_to_pC_(pset.getParameter<double>("Npe_to_pC")),
      npe_to_V_(pset.getParameter<double>("Npe_to_V")),
      sigmaRelTOFHIRenergy_(pset.getParameter<std::vector<double>>("SigmaRelTOFHIRenergy")),
      adcNbits_(pset.getParameter<uint32_t>("adcNbits")),
      tdcNbits_(pset.getParameter<uint32_t>("tdcNbits")),
      adcSaturation_MIP_(pset.getParameter<double>("adcSaturation_MIP")),
      adcBitSaturation_(std::pow(2, adcNbits_) - 1),
      adcLSB_MIP_(adcSaturation_MIP_ / adcBitSaturation_),
      adcThreshold_MIP_(pset.getParameter<double>("adcThreshold_MIP")),
      toaLSB_ns_(pset.getParameter<double>("toaLSB_ns")),
      tdcBitSaturation_(std::pow(2, tdcNbits_) - 1),
      corrCoeff_(pset.getParameter<double>("CorrelationCoefficient")),
      cosPhi_(0.5 * (sqrt(1. + corrCoeff_) + sqrt(1. - corrCoeff_))),
      sinPhi_(0.5 * corrCoeff_ / cosPhi_),
      scintillatorDecayTime2_(scintillatorDecayTime_ * scintillatorDecayTime_),
      scintillatorDecayTimeInv_(1. / scintillatorDecayTime_),
      sigmaConst2_(sigmaDigitization_ * sigmaDigitization_ + sigmaClockGlobal_ * sigmaClockGlobal_) {
#ifdef EDM_ML_DEBUG
  float lightOutput = 4.4f * pset.getParameter<double>("LightOutput");  // average npe for 4.4 MeV
  float s1 = sigma_stochastic(lightOutput);
  float s2 = sigma_DCR(lightOutput);
  float s3 = sigma_electronics(lightOutput);
  float s4 = sigmaDigitization_;
  float s4 = SigmaClock_;
  LogDebug("BTLElectronicsSim") << " BTL resolution model, for an average light output of " << std::fixed
                                << std::setw(14) << lightOutput << " :"
                                << "\n sigma stochastic   = " << std::setw(14) << sigma_stochastic(lightOutput)
                                << "\n sigma DCR          = " << std::setw(14) << sigma_DCR(lightOutput)
                                << "\n sigma electronics  = " << std::setw(14) << sigma_electronics(lightOutput)
                                << "\n sigma digitization = " << std::setw(14) << sigmaDigitization_
                                << "\n sigma clock        = " << std::setw(14)
                                << sqrt(sigmaClockGlobal_ * sigmaClockGlobal_ + sigmaClockRU_ * sigmaClockRU_)
                                << "\n ---------------------"
                                << "\n sigma total        = " << std::setw(14)
                                << std::sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4 + s5 * s5);
#endif
}

void BTLElectronicsSim::run(const mtd::MTDSimHitDataAccumulator& input,
                            BTLDigiCollection& output,
                            CLHEP::HepRandomEngine* hre) const {
  // --- Generate a different clock jitter for each readout unit
  std::vector<float> v_smearingClockRU;
  for (unsigned int iRU = 0; iRU < 2 * BTLDetId::HALF_ROD * BTLDetId::kCrystalTypes * BTLDetId::kRUPerTypeV2; ++iRU)
    v_smearingClockRU.push_back(CLHEP::RandGaussQ::shoot(hre, 0., sigmaClockRU_));

  MTDSimHitData chargeColl, toa1, toa2;
  for (MTDSimHitDataAccumulator::const_iterator it = input.begin(); it != input.end(); it++) {
    // --- Digitize only the in-time bucket
    const unsigned int iBX = mtd_digitizer::kInTimeBX;

    chargeColl.fill(0.f);
    toa1.fill(0.f);
    toa2.fill(0.f);
    for (size_t iside = 0; iside < 2; iside++) {
      // --- Get the number of photo-electrons
      float npe = (it->second).hit_info[2 * iside][iBX];

      // --- Skip the hits that are below the energy threshold
      if (npe < energyThreshold_)
        continue;

      // --- Get the time of arrival and add a channel time offset
      float finalToA1 = (it->second).hit_info[1 + 2 * iside][iBX] + channelTimeOffset_;

      if (smearChannelTimeOffset_ > 0.) {
        float timeSmearing = CLHEP::RandGaussQ::shoot(hre, 0., smearChannelTimeOffset_);
        finalToA1 += timeSmearing;
      }

      // --- Calculate and add the time walk: the time of arrival is read in correspondence
      //                                      with two thresholds on the signal pulse
      std::array<float, 3> times =
          btlPulseShape_.timeAtThr(npe / referencePulseNpe_, timeThreshold1_ * npe_to_V_, timeThreshold2_ * npe_to_V_);

      // --- If the pulse amplitude is smaller than timeThreshold2, the trigger does not fire
      if (times[1] == 0.)
        continue;

      float finalToA2 = finalToA1 + times[1];
      finalToA1 += times[0];

      // --- Estimate the time uncertainty due to photons from earlier OOT hits in the current BTL cell
      if (smearTimeForOOTtails_) {
        float rate_oot = 0.;
        // Loop on earlier OOT hits
        for (int ibx = 0; ibx < mtd_digitizer::kInTimeBX; ++ibx) {
          if ((it->second).hit_info[2 * iside][ibx] > 0.) {
            float hit_time = (it->second).hit_info[1 + 2 * iside][ibx] + bxTime_ * (ibx - mtd_digitizer::kInTimeBX);
            float npe_oot = CLHEP::RandPoissonQ::shoot(hre, (it->second).hit_info[2 * iside][ibx]);
            rate_oot += npe_oot * exp(hit_time * scintillatorDecayTimeInv_) * scintillatorDecayTimeInv_;
          }
        }  // ibx loop

        if (rate_oot > 0.) {
          float sigma_oot = sqrt(rate_oot * scintillatorRiseTime_) * scintillatorDecayTime_ / npe;
          float smearing_oot = CLHEP::RandGaussQ::shoot(hre, 0., sigma_oot);
          finalToA1 += smearing_oot;
          finalToA2 += smearing_oot;
        }
      }  // if smearTimeForOOTtails_

      // --- Stochastich term, uncertainty due to the fluctuations of the n-th photon arrival time
      if (testBeamMIPTimeRes_ > 0.) {
        // The time resolution is parametrized from the testbeam results.
        // The same parameterization is used for both thresholds.

        float sigma = sqrt2_ * sigma_stochastic(npe);  // the uncertainty is provided for the combination of two SiPMs
        float smearing_stat_thr1 = CLHEP::RandGaussQ::shoot(hre, 0., sigma);
        float smearing_stat_thr2 = CLHEP::RandGaussQ::shoot(hre, 0., sigma);

        finalToA1 += smearing_stat_thr1;
        finalToA2 += smearing_stat_thr2;
      }

      // --- Add in quadrature the uncertainties due to the SiPM DCR and the electronic noise
      float sigmaDCR = sigma_DCR(npe);
      float sigmaElec = sigma_electronics(npe);
      float sigma2_tot_thr1 = sigmaDCR * sigmaDCR + sigmaElec * sigmaElec;

      // --- Add in quadrature the uncertainties independent of npe: digitization and global clock distribution
      sigma2_tot_thr1 += sigmaConst2_;
      float sigma2_tot_thr2 = sigma2_tot_thr1;

      // --- Add the contribution due to the clock distribution within the readout units
      //     and smear the arrival times using the correlated uncertainties

      // Define a global readout-unit ID
      BTLDetId cellId((it->first).detid_);
      int iRU = 12 * (cellId.mtdRR() - 1) + 6 * cellId.mtdSide() + cellId.globalRunit() - 1;

      float smearing_thr1_uncorr = CLHEP::RandGaussQ::shoot(hre, 0., sqrt(sigma2_tot_thr1)) + v_smearingClockRU[iRU];
      float smearing_thr2_uncorr = CLHEP::RandGaussQ::shoot(hre, 0., sqrt(sigma2_tot_thr2)) + v_smearingClockRU[iRU];

      finalToA1 += cosPhi_ * smearing_thr1_uncorr + sinPhi_ * smearing_thr2_uncorr;
      finalToA2 += sinPhi_ * smearing_thr1_uncorr + cosPhi_ * smearing_thr2_uncorr;

      // --- Smear the energy according to TOFHIR energy branch measured resolution
      float tofhir_ampnoise_relsigma = ROOT::Math::Chebyshev4(npe,
                                                              sigmaRelTOFHIRenergy_[0],
                                                              sigmaRelTOFHIRenergy_[1],
                                                              sigmaRelTOFHIRenergy_[2],
                                                              sigmaRelTOFHIRenergy_[3],
                                                              sigmaRelTOFHIRenergy_[4]);

      float smearing_tofhir = CLHEP::RandGaussQ::shoot(hre, 0., tofhir_ampnoise_relsigma);

      // The amplitude resolution already includes the photostatistics fluctuation,
      // use the original average deposit.
      chargeColl[iside] = (it->second).hit_info[2 * iside][iBX] * npe_to_pC_ *
                          (1. + smearing_tofhir);  // the p.e. number is here converted to pC

      toa1[iside] = finalToA1;
      toa2[iside] = finalToA2;

    }  // iside loop

    // --- Run the shaper to create a new data frame
    BTLDataFrame rawDataFrame(it->first.detid_);
    runTrivialShaper(rawDataFrame, chargeColl, toa1, toa2, it->first.row_, it->first.column_);
    updateOutput(output, rawDataFrame);

  }  // MTDSimHitDataAccumulator loop
}

void BTLElectronicsSim::runTrivialShaper(BTLDataFrame& dataFrame,
                                         const mtd::MTDSimHitData& chargeColl,
                                         const mtd::MTDSimHitData& toa1,
                                         const mtd::MTDSimHitData& toa2,
                                         const uint8_t row,
                                         const uint8_t col) const {
  bool debug = debug_;
#ifdef EDM_ML_DEBUG
  for (int it = 0; it < (int)(chargeColl.size()); it++)
    debug |= (chargeColl[it] > adcThreshold_MIP_);
#endif

  if (debug)
    edm::LogVerbatim("BTLElectronicsSim") << "[runTrivialShaper]" << std::endl;

  //set new ADCs
  for (int it = 0; it < (int)(chargeColl.size()); it++) {
    BTLSample newSample;
    newSample.set(false, false, 0, 0, 0, row, col);

    //brute force saturation, maybe could to better with an exponential like saturation
    const uint32_t adc = std::min((uint32_t)std::floor(chargeColl[it] / adcLSB_MIP_), adcBitSaturation_);
    const uint32_t tdc_time1 = std::min((uint32_t)std::floor(toa1[it] / toaLSB_ns_), tdcBitSaturation_);
    const uint32_t tdc_time2 = std::min((uint32_t)std::floor(toa2[it] / toaLSB_ns_), tdcBitSaturation_);

    newSample.set(
        chargeColl[it] > adcThreshold_MIP_, tdc_time1 == tdcBitSaturation_, tdc_time2, tdc_time1, adc, row, col);
    dataFrame.setSample(it, newSample);

    if (debug)
      edm::LogVerbatim("BTLElectronicsSim") << adc << " (" << chargeColl[it] << "/" << adcLSB_MIP_ << ") ";
  }

  if (debug) {
    std::ostringstream msg;
    dataFrame.print(msg);
    edm::LogVerbatim("BTLElectronicsSim") << msg.str() << std::endl;
  }
}

void BTLElectronicsSim::updateOutput(BTLDigiCollection& coll, const BTLDataFrame& rawDataFrame) const {
  BTLDataFrame dataFrame(rawDataFrame.id());
  dataFrame.resize(dfSIZE);
  bool putInEvent(false);
  for (int it = 0; it < dfSIZE; ++it) {
    dataFrame.setSample(it, rawDataFrame[it]);
    if (it == 0)
      putInEvent = rawDataFrame[it].threshold();
  }

  if (putInEvent) {
    coll.push_back(dataFrame);
  }
}

float BTLElectronicsSim::sigma_stochastic(const float& npe) const {
  return testBeamMIPTimeRes_ * std::sqrt(scintillatorDecayTime_ / npe);
}

float BTLElectronicsSim::sigma_DCR(const float& npe) const {
  // Trick to safely switch off the electronics contribution for resolution studies:
  if (darkCountRate_ == 0.) {
    return 0.;
  }

  return paramDCR_[0] * std::pow((darkCountRate_ / paramDCR_[1]), paramDCR_[2]) * scintillatorDecayTime_ / npe;
}

float BTLElectronicsSim::sigma_electronics(const float npe) const {
  // Trick to safely switch off the electronics contribution for resolution studies:
  if (electronicGain_ == 0.) {
    return 0.;
  }

  float gainXnpe = electronicGain_ * npe;
  float res = sigmaElectronicNoise_ / sqrt2_;

  if (gainXnpe < paramSR_[0]) {
    res /= (paramSR_[2] * gainXnpe + paramSR_[1]);
  } else {
    res /= (paramSR_[3] * std::log(gainXnpe) + paramSR_[2] * paramSR_[0] - paramSR_[3] * std::log(paramSR_[0]));
  }

  return std::sqrt(res * res);
}
