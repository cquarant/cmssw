#include "CalibMTD/MTDTrivialCondChannels/interface/BTLCalibrationConversions.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationSoARcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationSoARcd.h"
#include "DataFormats/Portable/interface/PortableCollection.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/ESProducer.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/ModuleFactory.h"
#include <memory>

namespace ALPAKA_ACCELERATOR_NAMESPACE {

  class BTLTDCCalibrationSoAESProducer : public ESProducer {
  public:
    explicit BTLTDCCalibrationSoAESProducer(const edm::ParameterSet& config) : ESProducer(config) {
      token_ = setWhatProduced(this).consumes();
    }

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

  class BTLQDCCalibrationSoAESProducer : public ESProducer {
  public:
    explicit BTLQDCCalibrationSoAESProducer(const edm::ParameterSet& config) : ESProducer(config) {
      token_ = setWhatProduced(this).consumes();
    }

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

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

DEFINE_FWK_EVENTSETUP_ALPAKA_MODULE(BTLTDCCalibrationSoAESProducer);
DEFINE_FWK_EVENTSETUP_ALPAKA_MODULE(BTLQDCCalibrationSoAESProducer);
