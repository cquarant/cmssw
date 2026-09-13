#include "CondFormats/DataRecord/interface/BTLTDCCalibrationRcd.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsId.h"
#include "CondFormats/MTDObjects/interface/BTLTDCCalibration.h"
#include "EventFilter/MTDRawToDigi/plugins/BTLTDCCalibrationESProducer.h"

#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ModuleFactory.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include <array>
#include <cstdint>
#include <memory>

// ============================================================================
// Test EDAnalyzer
//
// Reads BTLTDCCalibration from EventSetup and checks a few ElectronicsIds.
// ============================================================================

class BTLTDCCalibrationTest : public edm::one::EDAnalyzer<> {
public:
  explicit BTLTDCCalibrationTest(const edm::ParameterSet&)
      : calibrationToken_(esConsumes<BTLTDCCalibration, BTLTDCCalibrationRcd>()) {}

  void analyze(const edm::Event&, const edm::EventSetup& iSetup) override {
    const auto& calibration = iSetup.getData(calibrationToken_);

    edm::LogPrint("BTLTDCCalibrationTest")
        << "Payload sizes:"
        << "\n  t0 : " << calibration.t0.size()
        << "\n  a0 : " << calibration.a0.size()
        << "\n  a1 : " << calibration.a1.size()
        << "\n  a2 : " << calibration.a2.size();

    // Three deliberately different channels
    const std::array<BTLElectronicsId, 3> channels = {
        BTLElectronicsId(0, 0, 0, 0),
        BTLElectronicsId(0, 0, 1, 15),
        BTLElectronicsId(11, 35, 23, 31)};

    for (const auto& electronicsId : channels) {
      const uint32_t index = electronicsId.denseIndex();

      edm::LogPrint("BTLTDCCalibrationTest")
          << "ElectronicsId"
          << " FED=" << electronicsId.fedId()
          << " HS=" << electronicsId.hsLinkId()
          << " eLink=" << electronicsId.eLinkId()
          << " channel=" << electronicsId.channelId()
          << "\n  denseIndex = " << index
          << "\n  t0 = " << calibration.t0.at(index)
          << "\n  a0 = " << calibration.a0.at(index)
          << "\n  a1 = " << calibration.a1.at(index)
          << "\n  a2 = " << calibration.a2.at(index);
    }
  }

private:
  edm::ESGetToken<BTLTDCCalibration, BTLTDCCalibrationRcd> calibrationToken_;
};

DEFINE_FWK_MODULE(BTLTDCCalibrationTest);