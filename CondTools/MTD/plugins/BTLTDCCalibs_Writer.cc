#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibsRcd.h"
#include "CondFormats/MTDObjects/interface/BTLTDCCalibs.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include <stdexcept>

// Separate from the ESProducer: copy its payload into SQLite, exactly as
// the ECAL Python example delegates writing to EcalDBCopy.
class BTLTDCCalibsWriter : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
  explicit BTLTDCCalibsWriter(const edm::ParameterSet&)
      : token_(esConsumes<BTLTDCCalibs, BTLTDCCalibsRcd, edm::Transition::BeginRun>()) {}

  void beginRun(const edm::Run&, const edm::EventSetup& setup) override {
    const auto& payload = setup.getData(token_);
    edm::Service<cond::service::PoolDBOutputService> db;
    if (!db.isAvailable()) {
      throw std::runtime_error("BTLTDCCalibsWriter requires PoolDBOutputService");
    }
    db->writeOneIOV(payload, db->beginOfTime(), "BTLTDCCalibsRcd");
    edm::LogInfo("BTLTDCCalibsWriter") << "Stored " << payload.btlItems().size() << " crystal entries";
  }
  void endRun(const edm::Run&, const edm::EventSetup&) override {}
  void analyze(const edm::Event&, const edm::EventSetup&) override {}

private:
  edm::ESGetToken<BTLTDCCalibs, BTLTDCCalibsRcd> token_;
};

DEFINE_FWK_MODULE(BTLTDCCalibsWriter);
