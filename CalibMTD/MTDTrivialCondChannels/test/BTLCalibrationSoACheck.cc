#include "CondFormats/MTDObjects/interface/BTLReadoutMap.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include <vector>
#include "CondFormats/MTDObjects/interface/BTLTDCCalibrationHost.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibrationHost.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationRcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationRcd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ModuleFactory.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <memory>

class BTLCalibrationTestMap : public edm::ESProducer {
public:
  explicit BTLCalibrationTestMap(const edm::ParameterSet&) { setWhatProduced(this); }
  std::unique_ptr<BTLReadoutMap> produce(const BTLReadoutMapRcd&) {
    auto map = std::make_unique<BTLReadoutMap>();
    map->add(BTLDetId(2), {BTLElectronicsId(143, 35, 23, 31), BTLElectronicsId(143, 35, 23, 0)});
    map->add(BTLDetId(1), {BTLElectronicsId(0, 0, 0, 20), BTLElectronicsId(0, 0, 0, 0)});
    map->initialize();
    return map;
  }
};

class BTLCalibrationSoACheck : public edm::one::EDAnalyzer<> {
public:
  explicit BTLCalibrationSoACheck(const edm::ParameterSet& config)
      : tdc_(esConsumes()),
        qdc_(esConsumes()),
        map_(esConsumes()),
        expected_(config.getParameter<std::vector<double>>("expected")),
        complete_(config.getParameter<bool>("complete")) {
    check(expected_.size() == 14);
  }
  void analyze(const edm::Event&, const edm::EventSetup& setup) override {
    const auto tdc = setup.getData(tdc_).const_view();
    const auto qdc = setup.getData(qdc_).const_view();
    const auto& map = setup.getData(map_);
    check(tdc.metadata().size() == BTLElectronicsIndexing::kNumberOfChannels &&
          qdc.metadata().size() == BTLElectronicsIndexing::kNumberOfChannels);
    std::vector<bool> occupied(BTLElectronicsIndexing::kNumberOfChannels, false);
    if (complete_)
      check(map.checkDenseChannelIndex());
    for (const auto& detId : map.getListOfDetIds()) {
      const auto pair = map.getElectronicsId(detId);
      for (const auto& id : {pair.minus, pair.plus}) {
        check(id.isValid());
        const auto i = id.denseChannelIndex();
        check(!occupied[i]);
        occupied[i] = true;
        check(map.getDetId(id) == detId);
        check(tdc[i].rawId() == id.rawId() && qdc[i].rawId() == id.rawId());
        check(tdc[i].valid() == 0xff && qdc[i].valid() == 0xff);
        for (int tac = 0; tac < btl::kCalibrationTACs; ++tac) {
          check(tdc[i].t0()(tac) == static_cast<float>(expected_[0]));
          check(tdc[i].a0()(tac) == static_cast<float>(expected_[1]));
          check(tdc[i].a1()(tac) == static_cast<float>(expected_[2]));
          check(tdc[i].a2()(tac) == static_cast<float>(expected_[3]));
          check(qdc[i].p0()(tac) == static_cast<float>(expected_[4]));
          check(qdc[i].p1()(tac) == static_cast<float>(expected_[5]));
          check(qdc[i].p2()(tac) == static_cast<float>(expected_[6]));
          check(qdc[i].p3()(tac) == static_cast<float>(expected_[7]));
          check(qdc[i].p4()(tac) == static_cast<float>(expected_[8]));
          check(qdc[i].p5()(tac) == static_cast<float>(expected_[9]));
          check(qdc[i].p6()(tac) == static_cast<float>(expected_[10]));
          check(qdc[i].p7()(tac) == static_cast<float>(expected_[11]));
          check(qdc[i].p8()(tac) == static_cast<float>(expected_[12]));
          check(qdc[i].p9()(tac) == static_cast<float>(expected_[13]));
        }
      }
    }
    for (int i = 0; i < BTLElectronicsIndexing::kNumberOfChannels; ++i) {
      check(occupied[i] || (!complete_ && tdc[i].valid() == 0 && qdc[i].valid() == 0));
    }
  }

private:
  static void check(bool value) {
    if (!value)
      throw cms::Exception("TestFailure") << "Unexpected BTL calibration SoA content";
  }
  edm::ESGetToken<btl::BTLTDCCalibrationHost, BTLTDCCalibrationRcd> tdc_;
  edm::ESGetToken<btl::BTLQDCCalibrationHost, BTLQDCCalibrationRcd> qdc_;
  edm::ESGetToken<BTLReadoutMap, BTLReadoutMapRcd> map_;
  std::vector<double> expected_;
  bool complete_;
};
DEFINE_FWK_EVENTSETUP_MODULE(BTLCalibrationTestMap);
DEFINE_FWK_MODULE(BTLCalibrationSoACheck);
