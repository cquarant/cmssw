#include "CalibMTD/MTDTrivialCondChannels/interface/BTLCalibrationConversions.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationSoARcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationSoARcd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"

// Payload mode accepts dummy or measured payloads from any EventSetup producer.
// SoA mode converts host results from calibration algorithms to persistent payloads.
// Input IOV changes are written independently for TDC and QDC, at the current run.
class BTLCalibrationWriter : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit BTLCalibrationWriter(const edm::ParameterSet& config) : fromSoA_(config.getParameter<bool>("fromSoA")) {
    usesResource(cond::service::PoolDBOutputService::kSharedResource);
    if (fromSoA_) {
      tdcSoA_ = esConsumes();
      qdcSoA_ = esConsumes();
    } else {
      tdc_ = esConsumes();
      qdc_ = esConsumes();
    }
  }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<bool>("fromSoA", false);
    descriptions.addWithDefaultLabel(desc);
  }

  void analyze(const edm::Event& event, const edm::EventSetup& setup) override {
    edm::Service<cond::service::PoolDBOutputService> db;
    if (!db.isAvailable())
      throw cms::Exception("Configuration") << "BTLCalibrationWriter requires PoolDBOutputService";
    const auto tdcCache = fromSoA_ ? setup.get<BTLTDCCalibrationSoARcd>().cacheIdentifier()
                                   : setup.get<BTLTDCCalibrationRcd>().cacheIdentifier();
    const auto qdcCache = fromSoA_ ? setup.get<BTLQDCCalibrationSoARcd>().cacheIdentifier()
                                   : setup.get<BTLQDCCalibrationRcd>().cacheIdentifier();
    const bool writeTDC = first_ || tdcCache != tdcCache_;
    const bool writeQDC = first_ || qdcCache != qdcCache_;
    const auto run = event.id().run();
    if ((!first_ && writeTDC && run <= tdcRun_) || (!first_ && writeQDC && run <= qdcRun_))
      throw cms::Exception("InvalidIOV") << "BTLCalibrationWriter requires increasing run-number IOVs";
    BTLTDCCalibration tdc;
    BTLQDCCalibration qdc;
    // Validate both inputs before writing either one.
    if (writeTDC) {
      tdc = fromSoA_ ? btl::makePayload(setup.getData(tdcSoA_)) : setup.getData(tdc_);
      tdc.validate();
    }
    if (writeQDC) {
      qdc = fromSoA_ ? btl::makePayload(setup.getData(qdcSoA_)) : setup.getData(qdc_);
      qdc.validate();
    }
    if (writeTDC) {
      db->writeOneIOV(tdc, run, "BTLTDCCalibrationRcd");
      tdcRun_ = run;
    }
    if (writeQDC) {
      db->writeOneIOV(qdc, run, "BTLQDCCalibrationRcd");
      qdcRun_ = run;
    }
    tdcCache_ = tdcCache;
    qdcCache_ = qdcCache;
    first_ = false;
  }

private:
  bool fromSoA_, first_ = true;
  unsigned int tdcRun_ = 0, qdcRun_ = 0;
  unsigned long long tdcCache_ = 0, qdcCache_ = 0;
  edm::ESGetToken<BTLTDCCalibration, BTLTDCCalibrationRcd> tdc_;
  edm::ESGetToken<BTLQDCCalibration, BTLQDCCalibrationRcd> qdc_;
  edm::ESGetToken<btl::BTLTDCCalibrationHost, BTLTDCCalibrationSoARcd> tdcSoA_;
  edm::ESGetToken<btl::BTLQDCCalibrationHost, BTLQDCCalibrationSoARcd> qdcSoA_;
};

DEFINE_FWK_MODULE(BTLCalibrationWriter);
