#ifndef __SimFastTiming_FastTimingCommon_BTLElectronicsSimSoA_h__
#define __SimFastTiming_FastTimingCommon_BTLElectronicsSimSoA_h__

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SimDataFormats/TrackingHit/interface/PSimHit.h"

#include "DataFormats/FTLDigi/interface/FTLDigiCollections.h"

#include "Geometry/MTDCommonData/interface/BTLElectronicsMapping.h"
#include "DataFormats/FTLDigiSoA/interface/BTLDigiHostCollection.h"
#include "SimFastTiming/FastTimingCommon/interface/MTDDigitizerTypes.h"

#include "SimFastTiming/FastTimingCommon/interface/BTLPulseShape.h"

using namespace btldigi;

namespace mtd = mtd_digitizer;

namespace CLHEP {
  class HepRandomEngine;
}

class BTLElectronicsSimSoA {
public:
  BTLElectronicsSimSoA(const edm::ParameterSet& pset, edm::ConsumesCollector iC);

  void getEvent(const edm::Event& evt) {}

  void getEventSetup(const edm::EventSetup& evt) {}

  void run(const mtd::MTDSimHitDataAccumulator& input, BTLDigiHostCollection& output, CLHEP::HepRandomEngine* hre) const;

  void runTrivialShaper(BTLDataFrame& dataFrame,
                        const mtd::MTDSimHitData& chargeColl,
                        const mtd::MTDSimHitData& toa1,
                        const mtd::MTDSimHitData& toa2,
                        const uint8_t row,
                        const uint8_t col) const;

  // void updateOutput(BTLDigiCollection& coll, const BTLDataFrame& rawDataFrame) const;

  static constexpr int dfSIZE = 2;
  // static constexpr float T_clk = 6250; // ps, TOFHIR clock period
  static constexpr float T_clk = 6.250; // ns, TOFHIR clock period

  static constexpr uint16_t T1coarseMask = 0x7FFF; // 15 bits for T1 coarse time
  static constexpr uint16_t T2coarseMask = 0x2FF; // 10 bits for T2 coarse time
  static constexpr uint16_t TfineShift = 5;   // 10 bits for fine time
  
private:
  float sigma2_pe(const float& Q, const float& R) const;

  float sigma_stochastic(const float& npe) const;

  float sigma2_DCR(const float& npe) const;

  float sigma2_electronics(const float npe) const;

  uint16_t timetoTcoarse(float time, const uint16_t mask) const;
  uint16_t timetoTfine(float time) const;
  uint16_t chargetoQfine(float charge, float time1, float time2) const;

  const bool debug_;

  const float bxTime_;
  const float testBeamMIPTimeRes_;
  const float ScintillatorRiseTime_;
  const float ScintillatorDecayTime_;
  const float ChannelTimeOffset_;
  const float smearChannelTimeOffset_;

  const float EnergyThreshold_;
  const float TimeThreshold1_;
  const float TimeThreshold2_;
  const float ReferencePulseNpe_;

  const float SinglePhotonTimeResolution_;
  const float DarkCountRate_;
  const float SigmaElectronicNoise_;
  const float SigmaClock_;
  const bool smearTimeForOOTtails_;
  const float Npe_to_pC_;
  const float Npe_to_V_;
  const std::vector<double> sigmaRelTOFHIRenergy_;

  // adc/tdc bitwidths
  const uint32_t adcNbits_, tdcNbits_;

  // synthesized adc/tdc information
  const float adcSaturation_MIP_;
  const uint32_t adcBitSaturation_;
  const float adcLSB_MIP_;
  const float adcThreshold_MIP_;
  const float toaLSB_ns_;
  const uint32_t tdcBitSaturation_;

  const float CorrCoeff_;
  const float cosPhi_;
  const float sinPhi_;

  const float ScintillatorDecayTime2_;
  const float ScintillatorDecayTimeInv_;
  const float SPTR2_;
  const float DCRxRiseTime_;
  const float SigmaElectronicNoise2_;
  const float SigmaClock2_;

  const BTLPulseShape btlPulseShape_;

  // tdc calibration parameters 
  // (to be modified: these parameters are evaluated by channel and stored in parquet files)
  static constexpr float a0_ = 57.244545; 
  static constexpr float a1_ = 511.27832; 
  static constexpr float a2_ = -7.8838577;
  static constexpr float t0_ = -0.048343264; 

  // qdc calibration parameters 
  // (to be modified: these parameters are evaluated by channel and stored in parquet files)
  static constexpr float p0_ = 49.542229; 
  static constexpr float p1_ = -0.323424;
  static constexpr float p2_ = 0.062578;
  static constexpr float p3_ = -0.002484;
  static constexpr float p4_ = 0.0;
  static constexpr float p5_ = 0.0;
  static constexpr float p6_ = 0.0;
  static constexpr float p7_ = 0.0;
  static constexpr float p8_ = 0.0;
  static constexpr float p9_ = 0.0;

};

#endif
