// Uniform persistent payload source. The same payloads can be written to CondDB.
#include "CondFormats/MTDObjects/interface/BTLTDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsId.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationRcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationRcd.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/EventSetupRecordInfiniteIntervalFinder.h"
#include "FWCore/Framework/interface/SourceFactory.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <array>
#include <cmath>
#include <memory>
#include <vector>

class BTLTrivialCalibrationSource : public edm::ESProducer, public edm::EventSetupRecordInfiniteIntervalFinder {
public:
  explicit BTLTrivialCalibrationSource(const edm::ParameterSet& config)
      : ids_(config.getParameter<std::vector<unsigned int>>("channelIds")) {
    setWhatProduced(this, &BTLTrivialCalibrationSource::produceTDC);
    setWhatProduced(this, &BTLTrivialCalibrationSource::produceQDC);
    findingRecord<BTLTDCCalibrationRcd>();
    findingRecord<BTLQDCCalibrationRcd>();
    // An empty channelIds selects the full hardware range, independent of geometry.
    if (ids_.empty()) {
      ids_.reserve(BTLElectronicsIndexing::kNumberOfChannels);
      for (int serenity = 0; serenity < 12; ++serenity)
        for (int hs = 0; hs < 36; ++hs)
          for (int el = 0; el < 24; ++el)
            for (int ch = 0; ch < 32; ++ch)
              ids_.push_back(BTLElectronicsId(serenity * 12 + hs / 3, hs, el, ch).rawId());
    }
    coefficients_[0] = config.getParameter<double>("t0_T1");
    coefficients_[1] = config.getParameter<double>("a0_T1");
    coefficients_[2] = config.getParameter<double>("a1_T1");
    coefficients_[3] = config.getParameter<double>("a2_T1");
    coefficients_[4] = config.getParameter<double>("t0_T2");
    coefficients_[5] = config.getParameter<double>("a0_T2");
    coefficients_[6] = config.getParameter<double>("a1_T2");
    coefficients_[7] = config.getParameter<double>("a2_T2");
    coefficients_[8] = config.getParameter<double>("p0");
    coefficients_[9] = config.getParameter<double>("p1");
    coefficients_[10] = config.getParameter<double>("p2");
    coefficients_[11] = config.getParameter<double>("p3");
    coefficients_[12] = config.getParameter<double>("p4");
    coefficients_[13] = config.getParameter<double>("p5");
    coefficients_[14] = config.getParameter<double>("p6");
    coefficients_[15] = config.getParameter<double>("p7");
    coefficients_[16] = config.getParameter<double>("p8");
    coefficients_[17] = config.getParameter<double>("p9");
    for (auto coefficient : coefficients_)
      if (!std::isfinite(coefficient))
        throw cms::Exception("Configuration") << "BTL calibration coefficients must be finite floats";
  }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<std::vector<unsigned int>>("channelIds", {});
    desc.add<double>("t0_T1", -0.048343264);
    desc.add<double>("a0_T1", 57.244545);
    desc.add<double>("a1_T1", 511.27832);
    desc.add<double>("a2_T1", -7.8838577);
    desc.add<double>("t0_T2", -0.048343264);
    desc.add<double>("a0_T2", 57.244545);
    desc.add<double>("a1_T2", 511.27832);
    desc.add<double>("a2_T2", -7.8838577);
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

  std::unique_ptr<BTLTDCCalibration> produceTDC(const BTLTDCCalibrationRcd&) {
    auto result = std::make_unique<BTLTDCCalibration>();
    result->rawIds = ids_;
    result->valid.assign(ids_.size(), 0xff);
    result->t0_T1.assign(ids_.size() * 8, coefficients_[0]);
    result->a0_T1.assign(ids_.size() * 8, coefficients_[1]);
    result->a1_T1.assign(ids_.size() * 8, coefficients_[2]);
    result->a2_T1.assign(ids_.size() * 8, coefficients_[3]);
    result->t0_T2.assign(ids_.size() * 8, coefficients_[4]);
    result->a0_T2.assign(ids_.size() * 8, coefficients_[5]);
    result->a1_T2.assign(ids_.size() * 8, coefficients_[6]);
    result->a2_T2.assign(ids_.size() * 8, coefficients_[7]);
    result->validate();
    return result;
  }

  std::unique_ptr<BTLQDCCalibration> produceQDC(const BTLQDCCalibrationRcd&) {
    auto result = std::make_unique<BTLQDCCalibration>();
    result->rawIds = ids_;
    result->valid.assign(ids_.size(), 0xff);
    result->p0.assign(ids_.size() * 8, coefficients_[8]);
    result->p1.assign(ids_.size() * 8, coefficients_[9]);
    result->p2.assign(ids_.size() * 8, coefficients_[10]);
    result->p3.assign(ids_.size() * 8, coefficients_[11]);
    result->p4.assign(ids_.size() * 8, coefficients_[12]);
    result->p5.assign(ids_.size() * 8, coefficients_[13]);
    result->p6.assign(ids_.size() * 8, coefficients_[14]);
    result->p7.assign(ids_.size() * 8, coefficients_[15]);
    result->p8.assign(ids_.size() * 8, coefficients_[16]);
    result->p9.assign(ids_.size() * 8, coefficients_[17]);
    result->validate();
    return result;
  }

private:
  std::vector<unsigned int> ids_;
  std::array<float, 18> coefficients_;
};

DEFINE_FWK_EVENTSETUP_SOURCE(BTLTrivialCalibrationSource);
