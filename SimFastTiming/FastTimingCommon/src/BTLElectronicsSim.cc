#include "SimFastTiming/FastTimingCommon/interface/BTLElectronicsSim.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/ForwardDetId/interface/BTLDetId.h"

#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandGaussQ.h"

using namespace mtd;

BTLElectronicsSim::BTLElectronicsSim(const edm::ParameterSet& pset, edm::ConsumesCollector iC)
    : debug_(pset.getUntrackedParameter<bool>("debug", false)),
      bxTime_(pset.getParameter<double>("bxTime")),
      energyThreshold_(pset.getParameter<double>("EnergyThreshold")),
      channelTimeOffset_(pset.getParameter<double>("ChannelTimeOffset")),
      smearChannelTimeOffset_(pset.getParameter<double>("SmearChannelTimeOffset")),
      sipmGain_(pset.getParameter<double>("SiPMGain")),
      paramThr1Rise_(pset.getParameter<std::vector<double>>("TimeAtThr1RiseParam")),
      paramThr2Rise_(pset.getParameter<std::vector<double>>("TimeAtThr2RiseParam")),
      smearTimeForOOTtails_(pset.getParameter<bool>("SmearTimeForOOTtails")),
      scintillatorRiseTime_(pset.getParameter<double>("ScintillatorRiseTime")),
      scintillatorDecayTime_(pset.getParameter<double>("ScintillatorDecayTime")),
      stocasticParam_(pset.getParameter<std::vector<double>>("StocasticParam")),
      darkCountRate_(pset.getParameter<double>("DarkCountRate")),
      paramDCR_(pset.getParameter<std::vector<double>>("DCRParam")),
      sigmaElectronicNoise_(pset.getParameter<double>("SigmaElectronicNoise")),
      paramSR_(pset.getParameter<std::vector<double>>("SlewRateParam")),
      sigmaTDC_(pset.getParameter<double>("SigmaTDC")),
      sigmaClockGlobal_(pset.getParameter<double>("SigmaClockGlobal")),
      sigmaClockRU_(pset.getParameter<double>("SigmaClockRU")),
      paramPulseAmp_(pset.getParameter<std::vector<double>>("PulseAmpParam")),
      paramPulseAmpRes_(pset.getParameter<std::vector<double>>("PulseAmpResParam")),
      adcNbits_(pset.getParameter<uint32_t>("adcNbits")),
      tdcNbits_(pset.getParameter<uint32_t>("tdcNbits")),
      adcBitSaturation_(std::pow(2, adcNbits_) - 1),
      adcThreshold_MIP_(pset.getParameter<double>("adcThreshold_MIP")),
      toaLSB_ns_(pset.getParameter<double>("toaLSB_ns")),
      tdcBitSaturation_(std::pow(2, tdcNbits_) - 1),
      corrCoeff_(pset.getParameter<double>("CorrelationCoefficient")),
      cosPhi_(0.5 * (sqrt(1. + corrCoeff_) + sqrt(1. - corrCoeff_))),
      sinPhi_(0.5 * corrCoeff_ / cosPhi_),
      scintillatorDecayTimeInv_(1. / scintillatorDecayTime_),
      sigmaConst2_(sigmaTDC_ * sigmaTDC_ + sigmaClockGlobal_ * sigmaClockGlobal_) {
#ifdef EDM_ML_DEBUG
  float lightOutput = 4.4f * pset.getParameter<double>("LightOutput");  // average npe for 4.4 MeV
  float s1 = sigma_stochastic(lightOutput);
  float s2 = sigma_DCR(lightOutput);
  float s3 = sigma_electronics(lightOutput);
  float s4 = sigmaTDC_;
  float s5 = sqrt(sigmaClockGlobal_ * sigmaClockGlobal_ + sigmaClockRU_ * sigmaClockRU_);
  LogDebug("BTLElectronicsSim") << " BTL resolution model, for an average light output of " << std::fixed
                                << std::setw(14) << lightOutput << " :"
                                << "\n sigma stochastic   = " << std::setw(14) << s1
                                << "\n sigma DCR          = " << std::setw(14) << s2
                                << "\n sigma electronics  = " << std::setw(14) << s3
                                << "\n sigma digitization = " << std::setw(14) << s4
                                << "\n sigma clock        = " << std::setw(14) << s5 << "\n ---------------------"
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

  // --- Loop over the simhits (which have been propagated to the two sides of the crystal bar)
  MTDSimHitData chargeColl, toa1, toa2;
  for (MTDSimHitDataAccumulator::const_iterator it = input.begin(); it != input.end(); it++) {
    // --- Digitize only the in-time bucket
    const unsigned int iBX = mtd_digitizer::kInTimeBX;

    if ((it->second).hit_info[0][iBX] == 0) {
      continue;
    }

    // --- Calculate a Poissonian smearing to be applied to the Npe of both bar sides
    float f_npeSmearing =
        CLHEP::RandPoissonQ::shoot(hre, (it->second).hit_info[0][iBX]) / (it->second).hit_info[0][iBX];

    chargeColl.fill(0.f);
    toa1.fill(0.f);
    toa2.fill(0.f);
    for (size_t iside = 0; iside < 2; iside++) {
      // --- Get the number of photo-electrons and apply the Poissonian smearing
      float npe = (it->second).hit_info[2 * iside][iBX] * f_npeSmearing;

      // --- Skip the hits that are below the energy threshold
      if (npe < energyThreshold_)
        continue;

      // ================================================================================
      //  TOFHiR's time branch
      // ================================================================================

      // --- Get the hit time of arrival and add an offset to the T1 channel
      float finalToA1 = (it->second).hit_info[1 + 2 * iside][iBX] + channelTimeOffset_;

      if (smearChannelTimeOffset_ > 0.) {
        float timeSmearing = CLHEP::RandGaussQ::shoot(hre, 0., smearChannelTimeOffset_);
        finalToA1 += timeSmearing;
      }

      float finalToA2 = (it->second).hit_info[1 + 2 * iside][iBX];

      // --- Get the values of T1 and T2 at the two thresholds on the pulse rising edge
      finalToA1 += time_at_Thr1Rise(npe);
      finalToA2 += time_at_Thr2Rise(npe);

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

      // --- Stochastich term
      float sigmaStoc = sigma_stochastic(npe);
      finalToA1 += CLHEP::RandGaussQ::shoot(hre, 0., sigmaStoc);
      finalToA2 += CLHEP::RandGaussQ::shoot(hre, 0., sigmaStoc);

      // --- Add in quadrature the uncertainties due to the SiPM DCR and the electronic noise
      float sigmaDCR = sigma_DCR(npe);
      float sigmaElec = sigma_electronics(npe);
      float sigma2_tot_thr1 = sigmaDCR * sigmaDCR + sigmaElec * sigmaElec;

      // --- Add in quadrature the uncertainties independent of Npe: digitization and global clock distribution
      sigma2_tot_thr1 += sigmaConst2_;

      float sigma2_tot_thr2 = sigma2_tot_thr1;

      // --- Add the contribution due to the clock distribution within the readout units
      //     and smear the T1 and T2 arrival times assuming correlated uncertainties

      // Define a global readout-unit ID
      BTLDetId cellId((it->first).detid_);
      int iRU = 12 * (cellId.mtdRR() - 1) + 6 * cellId.mtdSide() + cellId.globalRunit() - 1;

      float smearing_thr1_uncorr = CLHEP::RandGaussQ::shoot(hre, 0., sqrt(sigma2_tot_thr1)) + v_smearingClockRU[iRU];
      float smearing_thr2_uncorr = CLHEP::RandGaussQ::shoot(hre, 0., sqrt(sigma2_tot_thr2)) + v_smearingClockRU[iRU];

      finalToA1 += cosPhi_ * smearing_thr1_uncorr + sinPhi_ * smearing_thr2_uncorr;
      finalToA2 += sinPhi_ * smearing_thr1_uncorr + cosPhi_ * smearing_thr2_uncorr;

      toa1[iside] = finalToA1;
      toa2[iside] = finalToA2;

      // ================================================================================
      //  TOFHiR's energy branch
      // ================================================================================

      // --- Get the pulse amplitude in ADC counts
      float amp = pulse_amp(npe);

      // --- Get the relative uncertainty on the pulse amplitude
      //     (the unsmeared Npe is used, because the parameterization of the amplitude
      //      resolution already includes the photostatistics fluctuation)
      float amp_res = pulse_ampRes((it->second).hit_info[2 * iside][iBX]);

      chargeColl[iside] = amp * (1. + amp_res);

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
    const uint32_t adc = std::min((uint32_t)std::floor(chargeColl[it]), adcBitSaturation_);
    const uint32_t tdc_time1 = std::min((uint32_t)std::floor(toa1[it] / toaLSB_ns_), tdcBitSaturation_);
    const uint32_t tdc_time2 = std::min((uint32_t)std::floor(toa2[it] / toaLSB_ns_), tdcBitSaturation_);

    newSample.set(
        chargeColl[it] > adcThreshold_MIP_, tdc_time1 == tdcBitSaturation_, tdc_time2, tdc_time1, adc, row, col);
    dataFrame.setSample(it, newSample);

    if (debug)
      edm::LogVerbatim("BTLElectronicsSim") << adc << " (" << chargeColl[it] << ") ";
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

float BTLElectronicsSim::time_at_Thr1Rise(const float& npe) const {
  return paramThr1Rise_[0] * std::pow(sipmGain_ * npe, paramThr1Rise_[1]);
}

float BTLElectronicsSim::time_at_Thr2Rise(const float& npe) const {
  return paramThr2Rise_[0] * std::pow(sipmGain_ * npe, paramThr2Rise_[1]);
}

float BTLElectronicsSim::sigma_stochastic(const float& npe) const {
  // Trick to safely switch off the stochastic contribution for resolution studies:
  if (stocasticParam_[0] == 0.) {
    return 0.;
  }

  return sqrt2_ * stocasticParam_[0] *
         std::pow(npe, -stocasticParam_[1]);  // The uncertainty is provided for the combination of two SiPMs
}

float BTLElectronicsSim::sigma_DCR(const float& npe) const {
  // Trick to safely switch off the electronics contribution for resolution studies:
  if (darkCountRate_ == 0.) {
    return 0.;
  }

  return sqrt2_ * paramDCR_[0] * std::pow(darkCountRate_, paramDCR_[1]) /
         npe;  // The uncertainty is provided for the combination of two SiPMs
}

float BTLElectronicsSim::sigma_electronics(const float& npe) const {
  // Trick to safely switch off the electronics contribution for resolution studies:
  if (sigmaElectronicNoise_ == 0.) {
    return 0.;
  }

  float gainXnpe = sipmGain_ * npe;
  float res = sigmaElectronicNoise_;

  if (gainXnpe <= paramSR_[0]) {
    res /= (paramSR_[2] * gainXnpe + paramSR_[1]);
  } else {
    res /= (paramSR_[3] * std::log(gainXnpe) + paramSR_[2] * paramSR_[0] - paramSR_[3] * std::log(paramSR_[0]) +
            paramSR_[1]);
  }

  return std::sqrt(res * res);
}

float BTLElectronicsSim::pulse_amp(const float& npe) const { return paramPulseAmp_[0] + paramPulseAmp_[1] * npe; }

float BTLElectronicsSim::pulse_ampRes(const float& npe) const {
  return paramPulseAmpRes_[0] * std::pow(npe, paramPulseAmpRes_[1]);
}
