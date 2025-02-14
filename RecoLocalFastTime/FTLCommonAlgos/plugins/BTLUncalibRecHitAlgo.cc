#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "RecoLocalFastTime/FTLCommonAlgos/interface/MTDUncalibratedRecHitAlgoBase.h"
#include "RecoLocalFastTime/FTLClusterizer/interface/BTLRecHitsErrorEstimatorIM.h"

#include "CommonTools/Utils/interface/FormulaEvaluator.h"

class BTLUncalibRecHitAlgo : public BTLUncalibratedRecHitAlgoBase {
public:
  /// Constructor
  BTLUncalibRecHitAlgo(const edm::ParameterSet& conf, edm::ConsumesCollector& sumes)
      : MTDUncalibratedRecHitAlgoBase<BTLDataFrame>(conf, sumes),
        invLightSpeedLYSO_(conf.getParameter<double>("invLightSpeedLYSO")),
        c_LYSO_(1. / invLightSpeedLYSO_),
        npeToADC_(conf.getParameter<std::vector<double>>("npeToADC")),
        npePerMeV_(conf.getParameter<double>("npePerMeV")),
        invADCPerMeV_(1. / (npeToADC_[1] * npePerMeV_)),
        tdc_to_ns_(conf.getParameter<double>("tdcLSB_ns")),
        timeError_(conf.getParameter<std::string>("timeResolutionInNs")),
        timeCorr_p0_(conf.getParameter<double>("timeCorr_p0")),
        timeCorr_p1_(conf.getParameter<double>("timeCorr_p1")),
        timeCorr_p2_(conf.getParameter<double>("timeCorr_p2")) {}

  /// Destructor
  ~BTLUncalibRecHitAlgo() override {}

  /// get event and eventsetup information
  void getEvent(const edm::Event&) final {}
  void getEventSetup(const edm::EventSetup&) final {}

  /// make the rec hit
  FTLUncalibratedRecHit makeRecHit(const BTLDataFrame& dataFrame) const final;

private:
  float timewalkcorr(float& amplitude) const;

  const double invLightSpeedLYSO_;
  const double c_LYSO_;
  const std::vector<double> npeToADC_;
  const double npePerMeV_;
  const double invADCPerMeV_;
  const double tdc_to_ns_;
  const reco::FormulaEvaluator timeError_;
  const double timeCorr_p0_;
  const double timeCorr_p1_;
  const double timeCorr_p2_;
};

FTLUncalibratedRecHit BTLUncalibRecHitAlgo::makeRecHit(const BTLDataFrame& dataFrame) const {
  // The reconstructed amplitudes and times of the right and left hits are saved in a std::pair
  std::pair<float, float> amplitude(0., 0.);
  std::pair<float, float> time(0., 0.);

  unsigned char flag = 0;

  const auto& sampleRight = dataFrame.sample(0);
  const auto& sampleLeft = dataFrame.sample(1);

  double nHits = 0.;

  LogDebug("BTLUncalibRecHit") << "Original input time t1, t2 " << float(sampleRight.toa()) * tdc_to_ns_ << ", "
                               << float(sampleLeft.toa()) * tdc_to_ns_ << std::endl;
  if (sampleRight.data() > 0) {
    // Correct the time of the right SiPM for the time-walk
    amplitude.first = float(sampleRight.data());
    time.first = float(sampleRight.toa()) - timewalkcorr(amplitude.first);
    flag |= 0x1;

    // Convert ADC counts to MeV
    amplitude.first = (float(sampleRight.data()) - npeToADC_[0]) * invADCPerMeV_;
    time.first *= tdc_to_ns_;

    nHits += 1.;
  }

  // --- If available, reconstruct the amplitude and time of the second SiPM
  if (sampleLeft.data() > 0) {
    // Correct the time of the left SiPM for the time-walk
    amplitude.second = float(sampleLeft.data());
    time.second = float(sampleLeft.toa()) - timewalkcorr(amplitude.second);
    flag |= 0x1;

    // Convert ADC counts to MeV
    amplitude.second = (float(sampleLeft.data()) - npeToADC_[0]) * invADCPerMeV_;
    time.second *= tdc_to_ns_;

    nHits += 1.;

  }

  // --- Calculate the error on the hit time using the provided parameterization

  const std::array<double, 1> amplitudeV = {{(amplitude.first + amplitude.second) / nHits}};
  const std::array<double, 1> emptyV = {{0.}};

  double timeError = (nHits > 0. ? timeError_.evaluate(amplitudeV, emptyV) : -1.);

  // Calculate the position
  // Distance from center of bar to hit

  float position = 0.5f * (c_LYSO_ * (time.second - time.first));
  float positionError = BTLRecHitsErrorEstimatorIM::positionError();

  LogDebug("BTLUncalibRecHit") << "DetId: " << dataFrame.id().rawId() << " x position = " << position << " +/- "
                               << positionError;
  LogDebug("BTLUncalibRecHit") << "ADC+: set the charge to: (" << amplitude.first << ", " << amplitude.second << ")  ("
                               << sampleRight.data() << ", " << sampleLeft.data() << ") " << invADCPerMeV_ << ' '
                               << std::endl;
  LogDebug("BTLUncalibRecHit") << "TDC+: set the time to: (" << time.first << ", " << time.second << ")  ("
                               << sampleRight.toa() << ", " << sampleLeft.toa() << ") " << tdc_to_ns_ << ' '
                               << std::endl;

  return FTLUncalibratedRecHit(
      dataFrame.id(), dataFrame.row(), dataFrame.column(), amplitude, time, timeError, position, positionError, flag);
}

float BTLUncalibRecHitAlgo::timewalkcorr(float& amp) const {
  return timeCorr_p0_ * pow(amp, timeCorr_p1_) + timeCorr_p2_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_EDM_PLUGIN(BTLUncalibratedRecHitAlgoFactory, BTLUncalibRecHitAlgo, "BTLUncalibRecHitAlgo");
