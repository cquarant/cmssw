#include "CalibMTD/MTDTrivialCondChannels/test/BTLCalibrationTestData.h"
#include "CondFormats/MTDObjects/interface/BTLTDCCalibrationHost.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibrationHost.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationSoARcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationSoARcd.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/EventSetupRecordIntervalFinder.h"
#include "FWCore/Framework/interface/SourceFactory.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include <memory>

class BTLCalibrationIOVTestSource : public edm::ESProducer, public edm::EventSetupRecordIntervalFinder {
public:
  explicit BTLCalibrationIOVTestSource(const edm::ParameterSet&) {
    setWhatProduced(this, &BTLCalibrationIOVTestSource::tdc);
    setWhatProduced(this, &BTLCalibrationIOVTestSource::qdc);
    findingRecord<BTLTDCCalibrationRcd>();
    findingRecord<BTLQDCCalibrationRcd>();
  }
  std::unique_ptr<BTLTDCCalibration> tdc(const BTLTDCCalibrationRcd& r) {
    return std::make_unique<BTLTDCCalibration>(btltest::makeTDC(r.validityInterval().first().eventID().run()));
  }
  std::unique_ptr<BTLQDCCalibration> qdc(const BTLQDCCalibrationRcd& r) {
    return std::make_unique<BTLQDCCalibration>(btltest::makeQDC(r.validityInterval().first().eventID().run()));
  }
  void setIntervalFor(const edm::eventsetup::EventSetupRecordKey& key,
                      const edm::IOVSyncValue& sync,
                      edm::ValidityInterval& interval) override {
    unsigned first = sync.eventID().run();
    unsigned last = first;
    if (key == edm::eventsetup::EventSetupRecordKey::makeKey<BTLTDCCalibrationRcd>()) {
      first = (first - 1) / 2 * 2 + 1;
      last = first + 1;
    }
    interval =
        edm::ValidityInterval(edm::IOVSyncValue(edm::EventID(first, 0, 0)),
                              edm::IOVSyncValue(edm::EventID(
                                  last, edm::EventID::maxLuminosityBlockNumber(), edm::EventID::maxEventNumber())));
  }
};

class BTLCalibrationIOVCheck : public edm::one::EDAnalyzer<> {
public:
  explicit BTLCalibrationIOVCheck(const edm::ParameterSet&) : tdc_(esConsumes()), qdc_(esConsumes()) {}
  void analyze(const edm::Event& event, const edm::EventSetup& setup) override {
    const auto tdc = setup.getData(tdc_).const_view();
    const auto tdcExpected = btltest::makeTDC((event.id().run() - 1) / 2 * 2 + 1);
    btltest::check(tdc.metadata().size() == 331776);
    btltest::check(tdc[1].valid() == 0);
    for (unsigned channel = 0; channel < tdcExpected.size(); ++channel) {
      const auto index = BTLElectronicsId(tdcExpected.rawIds[channel]).denseChannelIndex();
      btltest::check(tdc[index].rawId() == tdcExpected.rawIds[channel]);
      btltest::check(tdc[index].valid() == tdcExpected.valid[channel]);
      for (unsigned tac = 0; tac < 8; ++tac) {
        btltest::check(tdc[index].t0_T1()(tac) == tdcExpected.t0_T1[channel * 8 + tac]);
        btltest::check(tdc[index].a0_T1()(tac) == tdcExpected.a0_T1[channel * 8 + tac]);
        btltest::check(tdc[index].a1_T1()(tac) == tdcExpected.a1_T1[channel * 8 + tac]);
        btltest::check(tdc[index].a2_T1()(tac) == tdcExpected.a2_T1[channel * 8 + tac]);
        btltest::check(tdc[index].t0_T2()(tac) == tdcExpected.t0_T2[channel * 8 + tac]);
        btltest::check(tdc[index].a0_T2()(tac) == tdcExpected.a0_T2[channel * 8 + tac]);
        btltest::check(tdc[index].a1_T2()(tac) == tdcExpected.a1_T2[channel * 8 + tac]);
        btltest::check(tdc[index].a2_T2()(tac) == tdcExpected.a2_T2[channel * 8 + tac]);
      }
    }
    const auto qdc = setup.getData(qdc_).const_view();
    const auto qdcExpected = btltest::makeQDC(event.id().run());
    btltest::check(qdc.metadata().size() == 331776);
    btltest::check(qdc[1].valid() == 0);
    for (unsigned channel = 0; channel < qdcExpected.size(); ++channel) {
      const auto index = BTLElectronicsId(qdcExpected.rawIds[channel]).denseChannelIndex();
      btltest::check(qdc[index].rawId() == qdcExpected.rawIds[channel]);
      btltest::check(qdc[index].valid() == qdcExpected.valid[channel]);
      for (unsigned tac = 0; tac < 8; ++tac) {
        btltest::check(qdc[index].p0()(tac) == qdcExpected.p0[channel * 8 + tac]);
        btltest::check(qdc[index].p1()(tac) == qdcExpected.p1[channel * 8 + tac]);
        btltest::check(qdc[index].p2()(tac) == qdcExpected.p2[channel * 8 + tac]);
        btltest::check(qdc[index].p3()(tac) == qdcExpected.p3[channel * 8 + tac]);
        btltest::check(qdc[index].p4()(tac) == qdcExpected.p4[channel * 8 + tac]);
        btltest::check(qdc[index].p5()(tac) == qdcExpected.p5[channel * 8 + tac]);
        btltest::check(qdc[index].p6()(tac) == qdcExpected.p6[channel * 8 + tac]);
        btltest::check(qdc[index].p7()(tac) == qdcExpected.p7[channel * 8 + tac]);
        btltest::check(qdc[index].p8()(tac) == qdcExpected.p8[channel * 8 + tac]);
        btltest::check(qdc[index].p9()(tac) == qdcExpected.p9[channel * 8 + tac]);
      }
    }
  }

private:
  edm::ESGetToken<btl::BTLTDCCalibrationHost, BTLTDCCalibrationSoARcd> tdc_;
  edm::ESGetToken<btl::BTLQDCCalibrationHost, BTLQDCCalibrationSoARcd> qdc_;
};
DEFINE_FWK_EVENTSETUP_SOURCE(BTLCalibrationIOVTestSource);
DEFINE_FWK_MODULE(BTLCalibrationIOVCheck);
