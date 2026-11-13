#include "CondFormats/MTDObjects/interface/BTLReadoutMap.h"
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
    map->add(BTLDetId(2), {BTLElectronicsId(1, 75, 23, 31), BTLElectronicsId(1, 75, 23, 0)});
    map->add(BTLDetId(1), {BTLElectronicsId(0, 4, 0, 20), BTLElectronicsId(0, 4, 0, 3)});
    map->initialize();
    return map;
  }
};

class BTLCalibrationSoACheck : public edm::one::EDAnalyzer<> {
public:
  explicit BTLCalibrationSoACheck(const edm::ParameterSet&)
      : tdc_(esConsumes()), qdc_(esConsumes()), map_(esConsumes()) {}
  void analyze(const edm::Event&, const edm::EventSetup& setup) override {
    const auto tdc = setup.getData(tdc_).const_view();
    const auto qdc = setup.getData(qdc_).const_view();
    const auto& ids = setup.getData(map_).channelRawIds();
    check(static_cast<std::size_t>(tdc.metadata().size()) == ids.size() &&
          static_cast<std::size_t>(qdc.metadata().size()) == ids.size());
    for (uint32_t i = 0; i < ids.size(); ++i) {
      check(tdc[i].rawId() == ids[i] && qdc[i].rawId() == ids[i]);
      check(tdc[i].valid() == 0xff && qdc[i].valid() == 0xff);
      for (int tac = 0; tac < btl::kCalibrationTACs; ++tac) {
        check(tdc[i].t0()(tac) == 1.f);
        check(tdc[i].a0()(tac) == 2.f);
        check(tdc[i].a1()(tac) == 3.f);
        check(tdc[i].a2()(tac) == 4.f);
        check(qdc[i].p0()(tac) == 5.f);
        check(qdc[i].p1()(tac) == 6.f);
        check(qdc[i].p2()(tac) == 7.f);
        check(qdc[i].p3()(tac) == 8.f);
        check(qdc[i].p4()(tac) == 9.f);
        check(qdc[i].p5()(tac) == 10.f);
        check(qdc[i].p6()(tac) == 11.f);
        check(qdc[i].p7()(tac) == 12.f);
        check(qdc[i].p8()(tac) == 13.f);
        check(qdc[i].p9()(tac) == 14.f);
      }
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
};
DEFINE_FWK_EVENTSETUP_MODULE(BTLCalibrationTestMap);
DEFINE_FWK_MODULE(BTLCalibrationSoACheck);
