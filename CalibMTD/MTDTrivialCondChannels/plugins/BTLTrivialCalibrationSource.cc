// Uniform, configurable channel/TAC calibrations; no CondObjectContainer dependency.
#include "CondFormats/MTDObjects/interface/BTLTDCCalibrationHost.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibrationHost.h"
#include "CondFormats/MTDObjects/interface/BTLReadoutMap.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationRcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationRcd.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/EventSetupRecordInfiniteIntervalFinder.h"
#include "FWCore/Framework/interface/SourceFactory.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "HeterogeneousCore/AlpakaInterface/interface/host.h"
#include <array>
#include <cmath>
#include <memory>

class BTLTrivialCalibrationSource : public edm::ESProducer, public edm::EventSetupRecordInfiniteIntervalFinder {
public:
  explicit BTLTrivialCalibrationSource(const edm::ParameterSet& config) {
    tdcMap_ = setWhatProduced(this, &BTLTrivialCalibrationSource::produceTDC).consumes();
    qdcMap_ = setWhatProduced(this, &BTLTrivialCalibrationSource::produceQDC).consumes();
    findingRecord<BTLTDCCalibrationRcd>();
    findingRecord<BTLQDCCalibrationRcd>();
    coefficients_[0] = config.getParameter<double>("t0");
    coefficients_[1] = config.getParameter<double>("a0");
    coefficients_[2] = config.getParameter<double>("a1");
    coefficients_[3] = config.getParameter<double>("a2");
    coefficients_[4] = config.getParameter<double>("p0");
    coefficients_[5] = config.getParameter<double>("p1");
    coefficients_[6] = config.getParameter<double>("p2");
    coefficients_[7] = config.getParameter<double>("p3");
    coefficients_[8] = config.getParameter<double>("p4");
    coefficients_[9] = config.getParameter<double>("p5");
    coefficients_[10] = config.getParameter<double>("p6");
    coefficients_[11] = config.getParameter<double>("p7");
    coefficients_[12] = config.getParameter<double>("p8");
    coefficients_[13] = config.getParameter<double>("p9");
    for (auto coefficient : coefficients_) {
      if (!std::isfinite(coefficient))
        throw cms::Exception("Configuration") << "BTL calibration coefficients must be finite floats";
    }
  }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<double>("t0", -0.048343264);
    desc.add<double>("a0", 57.244545);
    desc.add<double>("a1", 511.27832);
    desc.add<double>("a2", -7.8838577);
    desc.add<double>("p0", 49.542229);
    desc.add<double>("p1", -0.323424);
    desc.add<double>("p2", 0.062578);
    desc.add<double>("p3", -0.002484);
    desc.add<double>("p4", 0.0);
    desc.add<double>("p5", 0.0);
    desc.add<double>("p6", 0.0);
    desc.add<double>("p7", 0.0);
    desc.add<double>("p8", 0.0);
    desc.add<double>("p9", 0.0);
    descriptions.add("btlTrivialCalibrationSource", desc);
  }

  std::unique_ptr<btl::BTLTDCCalibrationHost> produceTDC(const BTLTDCCalibrationRcd& record) {
    const auto& ids = record.get(tdcMap_).channelRawIds();
    auto result = std::make_unique<btl::BTLTDCCalibrationHost>(cms::alpakatools::host(), ids.size());
    result->zeroInitialise();
    auto view = result->view();
    for (uint32_t i = 0; i < ids.size(); ++i) {
      view[i].rawId() = ids[i];
      view[i].valid() = 0xff;
      view[i].t0().setConstant(coefficients_[0]);
      view[i].a0().setConstant(coefficients_[1]);
      view[i].a1().setConstant(coefficients_[2]);
      view[i].a2().setConstant(coefficients_[3]);
    }
    return result;
  }

  std::unique_ptr<btl::BTLQDCCalibrationHost> produceQDC(const BTLQDCCalibrationRcd& record) {
    const auto& ids = record.get(qdcMap_).channelRawIds();
    auto result = std::make_unique<btl::BTLQDCCalibrationHost>(cms::alpakatools::host(), ids.size());
    result->zeroInitialise();
    auto view = result->view();
    for (uint32_t i = 0; i < ids.size(); ++i) {
      view[i].rawId() = ids[i];
      view[i].valid() = 0xff;
      view[i].p0().setConstant(coefficients_[4]);
      view[i].p1().setConstant(coefficients_[5]);
      view[i].p2().setConstant(coefficients_[6]);
      view[i].p3().setConstant(coefficients_[7]);
      view[i].p4().setConstant(coefficients_[8]);
      view[i].p5().setConstant(coefficients_[9]);
      view[i].p6().setConstant(coefficients_[10]);
      view[i].p7().setConstant(coefficients_[11]);
      view[i].p8().setConstant(coefficients_[12]);
      view[i].p9().setConstant(coefficients_[13]);
    }
    return result;
  }

private:
  edm::ESGetToken<BTLReadoutMap, BTLReadoutMapRcd> tdcMap_, qdcMap_;
  std::array<float, 14> coefficients_;
};

DEFINE_FWK_EVENTSETUP_SOURCE(BTLTrivialCalibrationSource);
