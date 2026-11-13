#include "CalibMTD/MTDTrivialCondChannels/interface/BTLCalibrationConversions.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationSoARcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationSoARcd.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/ModuleFactory.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include <memory>

class BTLTDCCalibrationSoAESProducer : public edm::ESProducer {
public:
  explicit BTLTDCCalibrationSoAESProducer(const edm::ParameterSet&) { token_ = setWhatProduced(this).consumes(); }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    descriptions.addWithDefaultLabel(desc);
  }

  std::unique_ptr<btl::BTLTDCCalibrationHost> produce(const BTLTDCCalibrationSoARcd& record) {
    return std::make_unique<btl::BTLTDCCalibrationHost>(btl::makeSoA(record.get(token_)));
  }

private:
  edm::ESGetToken<BTLTDCCalibration, BTLTDCCalibrationRcd> token_;
};

DEFINE_FWK_EVENTSETUP_MODULE(BTLTDCCalibrationSoAESProducer);

class BTLQDCCalibrationSoAESProducer : public edm::ESProducer {
public:
  explicit BTLQDCCalibrationSoAESProducer(const edm::ParameterSet&) { token_ = setWhatProduced(this).consumes(); }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    descriptions.addWithDefaultLabel(desc);
  }

  std::unique_ptr<btl::BTLQDCCalibrationHost> produce(const BTLQDCCalibrationSoARcd& record) {
    return std::make_unique<btl::BTLQDCCalibrationHost>(btl::makeSoA(record.get(token_)));
  }

private:
  edm::ESGetToken<BTLQDCCalibration, BTLQDCCalibrationRcd> token_;
};

DEFINE_FWK_EVENTSETUP_MODULE(BTLQDCCalibrationSoAESProducer);
