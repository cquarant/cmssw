#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "CondFormats/DataRecord/interface/BTLTDCEleChannelCalibsRcd.h"
#include "CondFormats/MTDObjects/interface/BTLTDCEleChannelCalibs.h"
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
class BTLTDCEleChannelCalibsWriter : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
  explicit BTLTDCEleChannelCalibsWriter(const edm::ParameterSet&)
      : token_(esConsumes<BTLTDCEleChannelCalibs, BTLTDCEleChannelCalibsRcd, edm::Transition::BeginRun>()) {}

  void beginRun(const edm::Run&, const edm::EventSetup& setup) override {
    const auto& payload = setup.getData(token_);
    edm::Service<cond::service::PoolDBOutputService> db;
    if (!db.isAvailable()) {
      throw std::runtime_error("BTLTDCEleChannelCalibsWriter requires PoolDBOutputService");
    }
    db->writeOneIOV(payload, db->beginOfTime(), "BTLTDCEleChannelCalibsRcd");
    edm::LogInfo("BTLTDCEleChannelCalibsWriter") << "Stored " << payload.btlItems().size() << " electronic channel entries";
  }
  void endRun(const edm::Run&, const edm::EventSetup&) override {}
  void analyze(const edm::Event&, const edm::EventSetup&) override {}

private:
  edm::ESGetToken<BTLTDCEleChannelCalibs, BTLTDCEleChannelCalibsRcd> token_;
};

DEFINE_FWK_MODULE(BTLTDCEleChannelCalibsWriter);
