#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/ModuleFactory.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CondFormats/MTDObjects/interface/BTLTDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsId.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationRcd.h"

#include <memory>

class BTLTDCCalibrationESProducer : public edm::ESProducer {
public:
  BTLTDCCalibrationESProducer(const edm::ParameterSet& iConfig)
      : t0_(iConfig.getParameter<double>("t0")),
        a0_(iConfig.getParameter<double>("a0")),
        a1_(iConfig.getParameter<double>("a1")),
        a2_(iConfig.getParameter<double>("a2")) {
    setWhatProduced(this);
  }

  std::unique_ptr<BTLTDCCalibration> produce(const BTLTDCCalibrationRcd&) {
    auto calib = std::make_unique<BTLTDCCalibration>();

    calib->t0.assign(BTLElectronicsId::kSize, t0_);
    calib->a0.assign(BTLElectronicsId::kSize, a0_);
    calib->a1.assign(BTLElectronicsId::kSize, a1_);
    calib->a2.assign(BTLElectronicsId::kSize, a2_);

    return calib;
  }

private:
  float t0_;
  float a0_;
  float a1_;
  float a2_;
};

DEFINE_FWK_EVENTSETUP_MODULE(BTLTDCCalibrationESProducer);