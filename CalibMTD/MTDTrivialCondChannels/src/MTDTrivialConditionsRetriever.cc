#include "CalibMTD/MTDTrivialCondChannels/interface/MTDTrivialConditionsRetriever.h"

#include "DataFormats/ForwardDetId/interface/BTLDetId.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"

#include <cstddef>
#include <string>
#include <memory>

BTLTDCCrystalCalib makeTDCCrystalCalibration(double t0, double a0, double a1, double a2) {
  BTLTDCCrystalCalib crystal{};
  for (auto& side : crystal.channels) {
    side.t0.fill(t0);
    side.a0.fill(a0);
    side.a1.fill(a1);
    side.a2.fill(a2);
  }
  return crystal;
}

MTDTrivialConditionsRetriever::MTDTrivialConditionsRetriever(const edm::ParameterSet& ps) {
  auto cc = setWhatProduced(this, &MTDTrivialConditionsRetriever::produceBTLTDCCalibs);

  geomToken_ = cc.consumes();
  topoToken_ = cc.consumes();

  auto channelCC = setWhatProduced(this, &MTDTrivialConditionsRetriever::produceBTLTDCEleChannelCalibs);
  channelGeomToken_ = channelCC.consumes();
  channelTopoToken_ = channelCC.consumes();
  readoutMapToken_ = channelCC.consumes();

  btltdc_t0_ = ps.getUntrackedParameter<double>("btltdc_t0", 200.);
  btltdc_a0_ = ps.getUntrackedParameter<double>("btltdc_a0", 200.);
  btltdc_a1_ = ps.getUntrackedParameter<double>("btltdc_a1", 200.);
  btltdc_a2_ = ps.getUntrackedParameter<double>("btltdc_a2", 200.);
  
  findingRecord<BTLTDCCalibsRcd>();
  findingRecord<BTLTDCEleChannelCalibsRcd>();
}
  
  std::unique_ptr<BTLTDCCalibs> MTDTrivialConditionsRetriever::produceBTLTDCCalibs(const BTLTDCCalibsRcd& tdcRecord) {

  const auto& geom = tdcRecord.get(geomToken_);
  const auto& topo = tdcRecord.get(topoToken_);
  auto btlCrysLayout = MTDTopologyMode::crysLayoutFromTopoMode(topo.getMTDTopologyMode());

  auto tdc_singleXtal = makeTDCCrystalCalibration(btltdc_t0_,btltdc_a0_,btltdc_a1_,btltdc_a2_);

  auto tdc_calibs = std::make_unique<BTLTDCCalibs>();

  // -- Readout map defined only for CrysLayout::v4 - If not v4, returns empty map
  if (static_cast<int>(btlCrysLayout) < static_cast<int>(BTLDetId::CrysLayout::v4)) {
    throw cms::Exception("MyFatalError")
      << "Trivial conditions retriever not available for BTL crystal layout " << static_cast<int>(btlCrysLayout)
      << ", use layout 7 (v4) or later!" << std::endl;
  }

  // -- Loop over geometry
  for (const auto& det : geom.detUnits()) {
    BTLDetId modId = BTLDetId(det->geographicalId().rawId());  // this is the sensor module ID
    // -- Select BTL only

    if (modId.mtdSubDetector() != MTDDetId::BTL)
      continue;

    // -- Loop over crystals in a sensor module
    for (uint iCrystal = 0; iCrystal < BTLDetId::kCrystalsPerModuleV2; iCrystal++) {
      BTLDetId btlId(modId.mtdSide(), modId.mtdRR(), modId.runit(), modId.dmodule(), modId.smodule(), iCrystal);
      tdc_calibs->setValue(btlId, tdc_singleXtal);
    }
  }

  edm::LogInfo("MTDTrivialConditionsRetriever")
      << "Created trivial BTL TDC conditions for " << tdc_calibs->btlItems().size()
      << " BTL crystal IDs, two readout sides and eight TACs per side";
  return tdc_calibs;
}

std::unique_ptr<BTLTDCEleChannelCalibs> MTDTrivialConditionsRetriever::produceBTLTDCEleChannelCalibs(
    const BTLTDCEleChannelCalibsRcd& tdcRecord) {
  const auto& geom = tdcRecord.get(channelGeomToken_);
  const auto& topo = tdcRecord.get(channelTopoToken_);
  const auto btlCrysLayout = MTDTopologyMode::crysLayoutFromTopoMode(topo.getMTDTopologyMode());

  if (static_cast<int>(btlCrysLayout) < static_cast<int>(BTLDetId::CrysLayout::v4)) {
    throw cms::Exception("MyFatalError")
        << "Trivial conditions retriever not available for BTL crystal layout " << static_cast<int>(btlCrysLayout)
        << ", use layout 7 (v4) or later!" << std::endl;
  }

  const auto& readoutMap = tdcRecord.get(readoutMapToken_);
  BTLTDCEleChannelCalib channelCalib{};
  channelCalib.t0.fill(btltdc_t0_);
  channelCalib.a0.fill(btltdc_a0_);
  channelCalib.a1.fill(btltdc_a1_);
  channelCalib.a2.fill(btltdc_a2_);

  auto tdcCalibs = std::make_unique<BTLTDCEleChannelCalibs>();
  for (const auto& det : geom.detUnits()) {
    const BTLDetId modId(det->geographicalId().rawId());
    if (modId.mtdSubDetector() != MTDDetId::BTL)
      continue;

    for (unsigned int iCrystal = 0; iCrystal < BTLDetId::kCrystalsPerModuleV2; ++iCrystal) {
      const BTLDetId btlId(modId.mtdSide(), modId.mtdRR(), modId.runit(), modId.dmodule(), modId.smodule(), iCrystal);
      const auto electronicsIds = readoutMap.getElectronicsId(btlId);
      tdcCalibs->setValue(electronicsIds.minus.rawId(), channelCalib);
      tdcCalibs->setValue(electronicsIds.plus.rawId(), channelCalib);
    }
  }

  edm::LogInfo("MTDTrivialConditionsRetriever")
      << "Created trivial BTL TDC conditions for " << tdcCalibs->size()
      << " electronic channel IDs, eight TACs per channel";
  return tdcCalibs;
}
