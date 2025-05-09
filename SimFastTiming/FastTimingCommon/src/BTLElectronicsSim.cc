#define EDM_ML_DEBUG

#include "SimFastTiming/FastTimingCommon/interface/BTLElectronicsSim.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/ForwardDetId/interface/BTLDetId.h"

#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandGaussQ.h"

using namespace mtd;

BTLElectronicsSim::BTLElectronicsSim(const edm::ParameterSet& pset, edm::ConsumesCollector iC)
    : bxTime_(pset.getParameter<double>("bxTime")),
      lcepositionSlope_(pset.getParameter<double>("LCEpositionSlope")),
      sigmaLCEpositionSlope_(pset.getParameter<double>("SigmaLCEpositionSlope")),
      pulseAmpThreshold_(pset.getParameter<double>("PulseAmpThreshold")),
      channelRearmMode_(pset.getParameter<uint32_t>("ChannelRearmMode")),
      channelRearmNClocks_(pset.getParameter<double>("ChannelRearmNClocks")),
      t1Delay_(pset.getParameter<double>("T1Delay")),
      sipmGain_(pset.getParameter<double>("SiPMGain")),
      paramPulseAmpA_(pset.getParameter<std::vector<double>>("PulseAmpAParam")),
      paramThr1Rise_(pset.getParameter<std::vector<double>>("TimeAtThr1RiseParam")),
      paramThr2Rise_(pset.getParameter<std::vector<double>>("TimeAtThr2RiseParam")),
      paramTimeOverThr1_(pset.getParameter<std::vector<double>>("TimeOverThr1Param")),
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
      tdcLSB_ns_(pset.getParameter<double>("tdcLSB_ns")),
      tdcBitSaturation_(std::pow(2, tdcNbits_) - 1),
      corrCoeff_(pset.getParameter<double>("CorrelationCoefficient")),
      cosPhi_(0.5 * (sqrt(1. + corrCoeff_) + sqrt(1. - corrCoeff_))),
      sinPhi_(0.5 * corrCoeff_ / cosPhi_),
      scintillatorDecayTimeInv_(1. / scintillatorDecayTime_),
      sigmaConst2_(sigmaTDC_ * sigmaTDC_ + sigmaClockGlobal_ * sigmaClockGlobal_),
#ifdef EDM_ML_DEBUG
      debug_(true) {
#else
      debug_(false) {
#endif
#ifdef EDM_ML_DEBUG
  float lightOutput = 4.4f * pset.getParameter<double>("LightOutput");  // average Npe for 4.4 MeV
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

  // --- Array to store a different local clock jitter for each readout unit
  smearingClockRU_ = new std::array<float, numberOfRUs_>();
  smearingClockRU_->fill(0.f);
}

BTLElectronicsSim::~BTLElectronicsSim() { delete smearingClockRU_; }

void BTLElectronicsSim::run(const mtd::MTDSimHitDataAccumulator& input,
                            BTLDigiCollection& output,
                            CLHEP::HepRandomEngine* hre) const {
  // --- Fill the readout-unit clock jitter array
  for (unsigned int iRU = 0; iRU < numberOfRUs_; ++iRU) {
    (*smearingClockRU_)[iRU] = CLHEP::RandGaussQ::shoot(hre, 0., sigmaClockRU_);
  }

  // --- Loop over the simhits (which have been propagated to the right and left sides of the crystal bar)
  for (MTDSimHitDataAccumulator::const_iterator it = input.begin(); it != input.end(); it++) {
    // --- Digitize only the in-time bucket
    const unsigned int iBX = mtd_digitizer::kInTimeBX;

    // --- Apply a common Npe Poisson fluctuation and independet Gaussian smearings
    //     for the LCE position slope to the right and left hits of the bar
    float npe[2] = {0.f, 0.f};
    // If both sides of the bar have an hit, the original simhit Npe and x can be determined:
    if ((it->second).hit_info[0][iBX] != 0. && (it->second).hit_info[2][iBX] != 0.) {
      float npe_origin = 0.5 * ((it->second).hit_info[0][iBX] + (it->second).hit_info[2][iBX]);
      float x_origin =
          0.5 * ((it->second).hit_info[0][iBX] - (it->second).hit_info[2][iBX]) / (npe_origin * lcepositionSlope_);

      float npe_fluctuated = CLHEP::RandPoissonQ::shoot(hre, npe_origin);

      float lceSlope_smearing = CLHEP::RandGaussQ::shoot(hre, 0., sigmaLCEpositionSlope_);
      npe[0] = npe_fluctuated * (1. + (sigmaLCEpositionSlope_ + lceSlope_smearing) * x_origin);

      lceSlope_smearing = CLHEP::RandGaussQ::shoot(hre, 0., sigmaLCEpositionSlope_);
      npe[1] = npe_fluctuated * (1. - (sigmaLCEpositionSlope_ + lceSlope_smearing) * x_origin);

    }
    // If there is a hit only on one side of the bar, the original simhit Npe and x can't be
    // determined and only a Poisson fluctuation to Npe_R or Npe_L is applied:
    else if ((it->second).hit_info[0][iBX] != 0. && (it->second).hit_info[2][iBX] == 0.) {
      npe[0] = CLHEP::RandPoissonQ::shoot(hre, (it->second).hit_info[0][iBX]);
    } else if ((it->second).hit_info[0][iBX] == 0. && (it->second).hit_info[2][iBX] != 0.) {
      npe[1] = CLHEP::RandPoissonQ::shoot(hre, (it->second).hit_info[2][iBX]);
    }
    // If there is no hit on either side of the bar, the hit is skipped:
    else {
      continue;
    }

    float charge_adc[2] = {0.f, 0.f};
    float toa1[2] = {0.f, 0.f};
    float toa2[2] = {0.f, 0.f};
    for (size_t iside = 0; iside < 2; iside++) {
      // --- Skip the empty buckets
      if (npe[iside] == 0.) {
        continue;
      }

      // ================================================================================
      //  TOFHiR's time branch
      // ================================================================================

      // --- Get the values of T1 and T2 at the two thresholds on the pulse rising
      //     edge from empirical parametrizations
      float time_at_T1 = time_at_Thr1Rise(npe[iside]);
      float time_at_T2 = time_at_Thr2Rise(npe[iside]);

      // --- Skip the hit if either T1 or T2 doesn't reach the threshold
      //     within the BX time
      if (time_at_T1 > bxTime_ || time_at_T2 > bxTime_) {
        continue;
      }

      // --- Skip the hit if its amplitude is below threshold
      if (pulse_amp_A(npe[iside]) < pulseAmpThreshold_) {
        continue;
      }

      float finalToA1 = (it->second).hit_info[1 + 2 * iside][iBX] + time_at_T1;
      float finalToA2 = (it->second).hit_info[1 + 2 * iside][iBX] + time_at_T2;

      // --- Loop over the earlier OOT hits in the current bar to determine the channel
      //     rearming time and estimate the photon flux arriving at the in-time BX
      float channelRearmingTime = -bxTime_ * (mtd_digitizer::kInTimeBX - 1);
      float rate_oot = 0.;
      for (int ibx = 0; ibx < mtd_digitizer::kInTimeBX; ++ibx) {
        // Skip the OOT empty buckets
        if ((it->second).hit_info[2 * iside][ibx] == 0.) {
          continue;
        }

        float hit_time_oot = (it->second).hit_info[1 + 2 * iside][ibx];
        float hit_npe_oot = CLHEP::RandPoissonQ::shoot(hre, (it->second).hit_info[2 * iside][ibx]);

        // Channel rearming time (the hit is skipped if either T1 or T2 doesn't reach the
        // threshold within the BX time or an earlier hit is holding the channel)
        float time_at_T1_oot = time_at_Thr1Rise(hit_npe_oot);
        float time_at_T2_oot = time_at_Thr2Rise(hit_npe_oot);

        if (channelRearmMode_ && time_at_T1_oot < bxTime_ && time_at_T2_oot < bxTime_ &&
            hit_time_oot + time_at_T1_oot > channelRearmingTime) {
          channelRearmingTime = rearming_time(hit_time_oot + time_at_T1_oot, hit_npe_oot);
        }

        // Rate of photons from earlier OOT hits in the current BTL cell
        if (smearTimeForOOTtails_) {
          rate_oot += hit_npe_oot * exp(hit_time_oot * scintillatorDecayTimeInv_) * scintillatorDecayTimeInv_;
        }

      }  // ibx loop

      // --- Skip the hit if the readout channel is not rearmed
      if (channelRearmMode_ && finalToA1 < channelRearmingTime) {
        continue;
      }

      // --- Uncertainty due to photons from earlier OOT hits in the current BTL cell
      if (smearTimeForOOTtails_ && rate_oot > 0.) {
        float sigma_oot = sqrt(rate_oot * scintillatorRiseTime_) * scintillatorDecayTime_ / npe[iside];
        float smearing_oot = CLHEP::RandGaussQ::shoot(hre, 0., sigma_oot);
        finalToA1 += smearing_oot;
        finalToA2 += smearing_oot;
      }

      // --- Stochastich term
      float sigmaStoc = sigma_stochastic(npe[iside]);
      finalToA1 += CLHEP::RandGaussQ::shoot(hre, 0., sigmaStoc);
      finalToA2 += CLHEP::RandGaussQ::shoot(hre, 0., sigmaStoc);

      // --- Add in quadrature the uncertainties due to the SiPM DCR and the electronic noise
      float sigmaDCR = sigma_DCR(npe[iside]);
      float sigmaElec = sigma_electronics(npe[iside]);
      float sigma2_tot_thr1 = sigmaDCR * sigmaDCR + sigmaElec * sigmaElec;

      // --- Add in quadrature the uncertainties independent of Npe: digitization and global clock distribution
      sigma2_tot_thr1 += sigmaConst2_;

      float sigma2_tot_thr2 = sigma2_tot_thr1;

      // --- Add the contribution due to the clock distribution within the readout units
      //     and smear the T1 and T2 arrival times assuming correlated uncertainties

      // Define a global readout-unit ID
      BTLDetId cellId((it->first).detid_);
      const int iRU = ((it->first).detid_ & BTLDetId::kBTLNewFormat
                           ? 12 * cellId.mtdRR() + 6 * cellId.mtdSide() + cellId.runit()
                           : 12 * (cellId.mtdRR() - 1) + 6 * cellId.mtdSide() + cellId.runit() - 1);

      float smearing_thr1_uncorr = CLHEP::RandGaussQ::shoot(hre, 0., sqrt(sigma2_tot_thr1)) + (*smearingClockRU_)[iRU];
      float smearing_thr2_uncorr = CLHEP::RandGaussQ::shoot(hre, 0., sqrt(sigma2_tot_thr2)) + (*smearingClockRU_)[iRU];

      finalToA1 += cosPhi_ * smearing_thr1_uncorr + sinPhi_ * smearing_thr2_uncorr;
      finalToA2 += sinPhi_ * smearing_thr1_uncorr + cosPhi_ * smearing_thr2_uncorr;

      toa1[iside] = finalToA1;
      toa2[iside] = finalToA2;

      // ================================================================================
      //  TOFHiR's energy branch
      // ================================================================================

      // --- Get the pulse amplitude in ADC counts
      float amp = pulse_amp(npe[iside]);

      // --- Get the average uncertainty on the pulse amplitude (here the unsmeared
      //     value of Npe is used, because the parameterization of the relative
      //     amplitude resolution already includes the photostatistics fluctuation)
      float sigma_amp = amp * pulse_ampRes((it->second).hit_info[2 * iside][iBX]);

      charge_adc[iside] = CLHEP::RandGaussQ::shoot(hre, amp, sigma_amp);

    }  // iside loop

    // --- Run the shaper to create a new data frame
    BTLDataFrame rawDataFrame(it->first.detid_);
    runTrivialShaper(rawDataFrame, charge_adc, toa1, toa2, it->first.row_, it->first.column_);
    updateOutput(output, rawDataFrame);

  }  // MTDSimHitDataAccumulator loop

}

void BTLElectronicsSim::runTrivialShaper(BTLDataFrame& dataFrame,
                                         const float (&charge_adc)[2],
                                         const float (&toa1)[2],
                                         const float (&toa2)[2],
                                         const uint8_t row,
                                         const uint8_t col) const {
  bool debug = debug_;
#ifdef EDM_ML_DEBUG
  for (int iside = 0; iside < dfSIZE; iside++) {
    debug |= (charge_adc[iside] > adcThreshold_MIP_);
  }
#endif

  if (debug) {
    LogTrace("BTLElectronicsSim") << "[runTrivialShaper] DetId " << dataFrame.id().rawId() << std::endl;
  }

  // --- Digitize the hit charge and times
  for (int iside = 0; iside < dfSIZE; iside++) {
    BTLSample newSample;
    newSample.set(false, false, 0, 0, 0, row, col);

    //brute force saturation, maybe could to better with an exponential like saturation
    const uint32_t adc = std::min((uint32_t)std::floor(charge_adc[iside]), adcBitSaturation_);
    const uint32_t tdc_time1 = std::min((uint32_t)std::floor(toa1[iside] / tdcLSB_ns_), tdcBitSaturation_);
    const uint32_t tdc_time2 = std::min((uint32_t)std::floor(toa2[iside] / tdcLSB_ns_), tdcBitSaturation_);

    newSample.set(
        charge_adc[iside] > adcThreshold_MIP_, tdc_time1 == tdcBitSaturation_, tdc_time2, tdc_time1, adc, row, col);
    dataFrame.setSample(iside, newSample);

    if (debug) {
      LogTrace("BTLElectronicsSim") << "Side " << iside << ": ADC = " << adc << " (" << charge_adc[iside] << "), "
                                    << "TDC1 = " << tdc_time1 << " (" << toa1[iside] << "), "
                                    << "TDC2 = " << tdc_time2 << " (" << toa2[iside] << ")" << std::endl;
    }
  }  // iside loop

  if (debug) {
    std::ostringstream msg;
    dataFrame.print(msg);
    LogTrace("BTLElectronicsSim") << msg.str() << std::endl;
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

float BTLElectronicsSim::rearming_time(const float& hit_time, const float& hit_npe) const {
  // mode 1: the channel is rearmed after the falling edge of the trigger_B signal
  // mode 2: the channel is rearmed after n cycles of the TOFHiR clock
  float deadTime = (channelRearmMode_ == 1 ? t1Delay_ + time_over_Thr1(hit_npe) : channelRearmNClocks_ * tofhirClock_);

  // Sync the rearming time with the next rising edge of the TOFHiR clock
  return (std::floor((hit_time + deadTime) / tofhirClock_) + 1.) * tofhirClock_;
}

float BTLElectronicsSim::pulse_amp_A(const float& npe) const {
  float gainXnpe = sipmGain_ * npe;
  return paramPulseAmpA_[0] + paramPulseAmpA_[1] * gainXnpe;
}

float BTLElectronicsSim::time_at_Thr1Rise(const float& npe) const {
  return paramThr1Rise_[0] * std::pow(sipmGain_ * npe, paramThr1Rise_[1]);
}

float BTLElectronicsSim::time_at_Thr2Rise(const float& npe) const {
  return paramThr2Rise_[0] * std::pow(sipmGain_ * npe, paramThr2Rise_[1]);
}

float BTLElectronicsSim::time_over_Thr1(const float& npe) const {
  float gainXnpe = sipmGain_ * npe;

  float time_over_thr1 =
      (gainXnpe <= paramTimeOverThr1_[0]
           ? paramTimeOverThr1_[4] * gainXnpe * gainXnpe * gainXnpe + paramTimeOverThr1_[3] * gainXnpe * gainXnpe +
                 paramTimeOverThr1_[2] * gainXnpe + paramTimeOverThr1_[1]
           : paramTimeOverThr1_[5] * gainXnpe + paramTimeOverThr1_[6]);

  return time_over_thr1;
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
