#ifndef CalibMTD_MTDTrivialCondChannels_MTDTrivialConditionsRetriever_h
#define CalibMTD_MTDTrivialCondChannels_MTDTrivialConditionsRetriever_h

#include "CondFormats/DataRecord/interface/BTLTDCCalibsRcd.h"
#include "CondFormats/MTDObjects/interface/BTLTDCCalibs.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/EventSetupRecordInfiniteIntervalFinder.h"

#include <memory>

namespace edm { class ParameterSet; }

// Trivial EventSetup producer: its BTL TDC payload is constant for all IOVs.
class MTDTrivialConditionsRetriever : public edm::ESProducer,
                                     public edm::EventSetupRecordInfiniteIntervalFinder {
public:
  explicit MTDTrivialConditionsRetriever(const edm::ParameterSet&);
  ~MTDTrivialConditionsRetriever() override = default;

  std::unique_ptr<BTLTDCCalibs> produceBTLTDCCalibs(const BTLTDCCalibsRcd&);
private:

  // tdc calibration (fitted function parameters)
  double btltdc_t0_;
  double btltdc_a0_;
  double btltdc_a1_;
  double btltdc_a2_;
  
  const BTLTDCCrystalCalib calibration_;
};

#endif  // CalibMTD_MTDTrivialCondChannels_MTDTrivialConditionsRetriever_h
