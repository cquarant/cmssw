#ifndef __SimFastTiming_FastTimingCommon_BTLElectronicsSim_h__
#define __SimFastTiming_FastTimingCommon_BTLElectronicsSim_h__

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SimDataFormats/TrackingHit/interface/PSimHit.h"

#include "DataFormats/FTLDigi/interface/FTLDigiCollections.h"
#include "SimFastTiming/FastTimingCommon/interface/MTDDigitizerTypes.h"

namespace mtd = mtd_digitizer;

namespace CLHEP {
  class HepRandomEngine;
}

class BTLElectronicsSim {
public:
  BTLElectronicsSim(const edm::ParameterSet& pset, edm::ConsumesCollector iC);

  void getEvent(const edm::Event& evt) {}

  void getEventSetup(const edm::EventSetup& evt) {}

  void run(const mtd::MTDSimHitDataAccumulator& input, BTLDigiCollection& output, CLHEP::HepRandomEngine* hre) const;

  void runTrivialShaper(BTLDataFrame& dataFrame,
                        const mtd::MTDSimHitData& chargeColl,
                        const mtd::MTDSimHitData& toa1,
                        const mtd::MTDSimHitData& toa2,
                        const uint8_t row,
                        const uint8_t col) const;

  void updateOutput(BTLDigiCollection& coll, const BTLDataFrame& rawDataFrame) const;

  static constexpr int dfSIZE = 2;

private:
  float time_at_Thr1Rise(const float& npe) const;

  float time_at_Thr2Rise(const float& npe) const;

  float sigma_stochastic(const float& npe) const;

  float sigma_DCR(const float& npe) const;

  float sigma_electronics(const float& npe) const;

  float pulse_amp(const float& npe) const;

  float pulse_ampRes(const float& npe) const;

  static constexpr float sqrt2_ = 1.41421356f;

  const bool debug_;

  const float bxTime_;
  const float energyThreshold_;
  const float channelTimeOffset_;
  const float smearChannelTimeOffset_;
  const float sipmGain_;
  const std::vector<double> paramThr1Rise_;
  const std::vector<double> paramThr2Rise_;
  const bool smearTimeForOOTtails_;
  const float scintillatorRiseTime_;
  const float scintillatorDecayTime_;
  const std::vector<double> stocasticParam_;
  const float darkCountRate_;
  const std::vector<double> paramDCR_;
  const float sigmaElectronicNoise_;
  const std::vector<double> paramSR_;
  const float sigmaTDC_;
  const float sigmaClockGlobal_;
  const float sigmaClockRU_;

  const std::vector<double> paramPulseAmp_;
  const std::vector<double> paramPulseAmpRes_;

  // adc/tdc bitwidths
  const uint32_t adcNbits_, tdcNbits_;

  // synthesized adc/tdc information
  //const float adcSaturation_MIP_;
  const uint32_t adcBitSaturation_;
  //const float adcLSB_MIP_;
  const float adcThreshold_MIP_;
  const float toaLSB_ns_;
  const uint32_t tdcBitSaturation_;

  const float corrCoeff_;
  const float cosPhi_;
  const float sinPhi_;

  const float scintillatorDecayTimeInv_;
  const float sigmaConst2_;
};

#endif
