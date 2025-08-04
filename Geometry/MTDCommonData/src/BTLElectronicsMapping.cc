#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/MTDCommonData/interface/BTLElectronicsMapping.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <Geometry/MTDCommonData/interface/MTDTopologyMode.h>

#include <ostream>
#include <algorithm>
#include <vector>

// BTLElectronicsMapping constructor
BTLElectronicsMapping::BTLElectronicsMapping(const BTLDetId::CrysLayout lay) {
  if (static_cast<int>(lay) < 7) {
    throw cms::Exception("BTLElectronicsMapping")
        << "MTD Topology mode with layout " << static_cast<int>(lay) << " is not supported\n"
        << "use layout : 7 (v4) or later!" << std::endl;
  }
}

////////////////////////////////////////////////////////////
//
//     Methods to Get SiPM Channel from crystal ID
//
////////////////////////////////////////////////////////////

int BTLElectronicsMapping::SiPMCh(bool smodCopy, uint32_t crystal, uint32_t SiPMSide) {
  
  if (0 > int(crystal) || crystal > BTLDetId::kCrystalsPerModuleV2) {
    edm::LogWarning("MTDGeom") << "BTLNumberingScheme::BTLElectronicsMapping(): "
                               << "****************** Bad crystal number = " << int(crystal);
    return 0;
  }

  if (0 > int(smodCopy) || smodCopy > BTLDetId::kSModulesPerDM) {
    edm::LogWarning("MTDGeom") << "BTLNumberingScheme::getUnitID(): "
                               << "****************** Bad sensor module copy (could be 0 or 1)= " << int(smodCopy);
    return 0;
  }

  if (smodCopy == 0)
    return BTLElectronicsMapping::SiPMChannelMapFW[crystal + SiPMSide * BTLDetId::kCrystalsPerModuleV2];
  else
    return BTLElectronicsMapping::SiPMChannelMapBW[crystal + SiPMSide * BTLDetId::kCrystalsPerModuleV2];
}


int BTLElectronicsMapping::SiPMCh(BTLDetId det, uint32_t SiPMSide) {
  bool smodCopy = (bool)det.smodule();
  uint32_t crystal = det.crystal();
  return BTLElectronicsMapping::SiPMCh(smodCopy, crystal, SiPMSide);
}


int BTLElectronicsMapping::SiPMCh(BTLDetId det, uint32_t crystal, uint32_t SiPMSide) {
  bool smodCopy = (bool)det.smodule();
  return BTLElectronicsMapping::SiPMCh(smodCopy, crystal, SiPMSide);
}

int BTLElectronicsMapping::SiPMCh(uint32_t rawId, uint32_t SiPMSide) {
  BTLDetId theId(rawId);
  return BTLElectronicsMapping::SiPMCh(theId, SiPMSide);
}

int BTLElectronicsMapping::SiPMCh(uint32_t rawId, uint32_t crystal, uint32_t SiPMSide) {
  BTLDetId theId(rawId);
  return BTLElectronicsMapping::SiPMCh(theId, crystal, SiPMSide);
}


BTLElectronicsMapping::SiPMChPair BTLElectronicsMapping::GetSiPMChPair(bool smodCopy, uint32_t crystal) {
  BTLElectronicsMapping::SiPMChPair SiPMChs;
  SiPMChs.Minus = BTLElectronicsMapping::SiPMCh(smodCopy, crystal, 0);
  SiPMChs.Plus = BTLElectronicsMapping::SiPMCh(smodCopy, crystal, 1);
  return SiPMChs;
}


BTLElectronicsMapping::SiPMChPair BTLElectronicsMapping::GetSiPMChPair(BTLDetId det) {
  BTLElectronicsMapping::SiPMChPair SiPMChs;
  SiPMChs.Minus = BTLElectronicsMapping::SiPMCh(det, 0);
  SiPMChs.Plus = BTLElectronicsMapping::SiPMCh(det, 1);
  return SiPMChs;
}


BTLElectronicsMapping::SiPMChPair BTLElectronicsMapping::GetSiPMChPair(BTLDetId det, uint32_t crystal) {
  BTLElectronicsMapping::SiPMChPair SiPMChs;
  SiPMChs.Minus = BTLElectronicsMapping::SiPMCh(det, crystal, 0);
  SiPMChs.Plus = BTLElectronicsMapping::SiPMCh(det, crystal, 1);
  return SiPMChs;
}


BTLElectronicsMapping::SiPMChPair BTLElectronicsMapping::GetSiPMChPair(uint32_t rawID) {
  BTLElectronicsMapping::SiPMChPair SiPMChs;
  SiPMChs.Minus = BTLElectronicsMapping::SiPMCh(rawID, (uint32_t)0);
  SiPMChs.Plus = BTLElectronicsMapping::SiPMCh(rawID, (uint32_t)1);
  return SiPMChs;
}

BTLElectronicsMapping::SiPMChPair BTLElectronicsMapping::GetSiPMChPair(uint32_t rawID, uint32_t crystal) {
  BTLElectronicsMapping::SiPMChPair SiPMChs;
  SiPMChs.Minus = BTLElectronicsMapping::SiPMCh(rawID, crystal, 0);
  SiPMChs.Plus = BTLElectronicsMapping::SiPMCh(rawID, crystal, 1);
  return SiPMChs;
}

////////////////////////////////////////////////////////////
//
//     Methods to Get TOFHIR Channel from crystal ID
//
////////////////////////////////////////////////////////////

int BTLElectronicsMapping::TOFHIRCh(bool smodCopy, uint32_t crystal, uint32_t SiPMSide) {
  int SiPMCh_ = BTLElectronicsMapping::SiPMCh(smodCopy, crystal, SiPMSide);
  return BTLElectronicsMapping::THChannelMap[SiPMCh_];
}

int BTLElectronicsMapping::TOFHIRCh(BTLDetId det, uint32_t SiPMSide) {
  bool smodCopy = (bool)det.smodule();
  uint32_t crystal = det.crystal();
  return BTLElectronicsMapping::TOFHIRCh(smodCopy, crystal, SiPMSide);
}

int BTLElectronicsMapping::TOFHIRCh(BTLDetId det, uint32_t crystal, uint32_t SiPMSide) {
  bool smodCopy = (bool)det.smodule();
  return BTLElectronicsMapping::TOFHIRCh(smodCopy, crystal, SiPMSide);
}

int BTLElectronicsMapping::TOFHIRCh(uint32_t rawId, uint32_t SiPMSide) {
  BTLDetId theId(rawId);
  return BTLElectronicsMapping::TOFHIRCh(theId, SiPMSide);
}

int BTLElectronicsMapping::TOFHIRCh(uint32_t rawId, uint32_t crystal, uint32_t SiPMSide) {
  BTLDetId theId(rawId);
  return BTLElectronicsMapping::TOFHIRCh(theId, crystal, SiPMSide);
}

BTLElectronicsMapping::TOFHIRChPair BTLElectronicsMapping::GetTOFHIRChPair(bool smodCopy, uint32_t crystal) {
  BTLElectronicsMapping::TOFHIRChPair TOFHIRChs;
  TOFHIRChs.Minus = BTLElectronicsMapping::TOFHIRCh(smodCopy, crystal, 0);
  TOFHIRChs.Plus = BTLElectronicsMapping::TOFHIRCh(smodCopy, crystal, 1);
  return TOFHIRChs;
}

BTLElectronicsMapping::TOFHIRChPair BTLElectronicsMapping::GetTOFHIRChPair(BTLDetId det) {
  BTLElectronicsMapping::TOFHIRChPair TOFHIRChs;
  TOFHIRChs.Minus = BTLElectronicsMapping::TOFHIRCh(det, 0);
  TOFHIRChs.Plus = BTLElectronicsMapping::TOFHIRCh(det, 1);
  return TOFHIRChs;
}

BTLElectronicsMapping::TOFHIRChPair BTLElectronicsMapping::GetTOFHIRChPair(BTLDetId det, uint32_t crystal) {
  BTLElectronicsMapping::TOFHIRChPair TOFHIRChs;
  TOFHIRChs.Minus = BTLElectronicsMapping::TOFHIRCh(det, crystal, 0);
  TOFHIRChs.Plus = BTLElectronicsMapping::TOFHIRCh(det, crystal, 1);
  return TOFHIRChs;
}

BTLElectronicsMapping::TOFHIRChPair BTLElectronicsMapping::GetTOFHIRChPair(uint32_t rawID) {
  BTLElectronicsMapping::TOFHIRChPair TOFHIRChs;
  TOFHIRChs.Minus = BTLElectronicsMapping::TOFHIRCh(rawID, 0);
  TOFHIRChs.Plus = BTLElectronicsMapping::TOFHIRCh(rawID, 1);
  return TOFHIRChs;
}

BTLElectronicsMapping::TOFHIRChPair BTLElectronicsMapping::GetTOFHIRChPair(uint32_t rawID, uint32_t crystal) {
  BTLElectronicsMapping::TOFHIRChPair TOFHIRChs;
  TOFHIRChs.Minus = BTLElectronicsMapping::TOFHIRCh(rawID, crystal, 0);
  TOFHIRChs.Plus = BTLElectronicsMapping::TOFHIRCh(rawID, crystal, 1);
  return TOFHIRChs;
}

// Get crystal ID from TOFHIR Channel

int BTLElectronicsMapping::THChToXtal(bool smodCopy, uint32_t THCh) {
  if (0 > int(smodCopy) || BTLDetId::kSModulesPerDM < smodCopy) {
    edm::LogWarning("MTDGeom") << "BTLNumberingScheme::getUnitID(): "
                               << "****************** Bad sensor module copy (could be 0 or 1)= " << int(smodCopy);
    return 0;
  }

  auto THChPos =
      std::find(BTLElectronicsMapping::THChannelMap.begin(), BTLElectronicsMapping::THChannelMap.end(), THCh);
  int targetSiPMCh = std::distance(BTLElectronicsMapping::THChannelMap.begin(), THChPos);

  std::array<uint32_t, BTLDetId::kCrystalsPerModuleV2 * 2> SiPMChMap;
  if (smodCopy == 0)
    SiPMChMap = BTLElectronicsMapping::SiPMChannelMapFW;
  else
    SiPMChMap = BTLElectronicsMapping::SiPMChannelMapBW;

  auto targetpos = std::find(SiPMChMap.begin(), SiPMChMap.end(), targetSiPMCh);
  return std::distance(SiPMChMap.begin(), targetpos) % BTLDetId::kCrystalsPerModuleV2 + 1;
}

BTLDetId BTLElectronicsMapping::THChToBTLDetId(
    uint32_t zside, uint32_t rod, uint32_t runit, uint32_t dmodule, uint32_t smodule, uint32_t THCh) {
  if (0 > int(THCh) || 31 < THCh) {
    edm::LogWarning("MTDGeom") << "BTLNumberingScheme::getUnitID(): "
                               << "****************** Bad TOFHIR channel = " << int(THCh);
    return 0;
  }

  if (0 > int(smodule) || BTLDetId::kSModulesPerDM < smodule) {
    edm::LogWarning("MTDGeom") << "BTLNumberingScheme::getUnitID(): "
                               << "****************** Bad sensor module copy = " << int(smodule);
    return 0;
  }

  if (0 > int(dmodule) || 12 < dmodule) {
    edm::LogWarning("MTDGeom") << "BTLNumberingScheme::getUnitID(): "
                               << "****************** Bad module copy = " << int(dmodule);
    return 0;
  }

  if (1 > rod || 36 < rod) {
    edm::LogWarning("MTDGeom") << "BTLNumberingScheme::getUnitID(): "
                               << "****************** Bad rod copy = " << rod;
    return 0;
  }

  if (1 < zside) {
    edm::LogWarning("MTDGeom") << "BTLNumberingScheme::getUnitID(): "
                               << "****************** Bad side = " << zside;
    return 0;
  }

  int crystal = BTLElectronicsMapping::THChToXtal(smodule, THCh);

  return BTLDetId(zside, rod, runit, dmodule, smodule, crystal);
}

// Get TOFHIR asic number
// if dmodule is odd number (DM range [1-12])
//    SM1 --> TOFHIR A0 (simply 0)
//    SM2 --> TOFHIR A1 (simply 1)
// else if dmodule is even number the order is inverted
//    SM1 --> TOFHIR A1 (simply 1)
//    SM2 --> TOFHIR A0 (simply 0)
int BTLElectronicsMapping::TOFHIRASIC(uint32_t dmodule, uint32_t smodule) {
  if (dmodule % BTLDetId::kSModulesInDM == 0)
    return smodule;
  else
    return BTLDetId::kSModulesInDM - smodule - 1;
}

int BTLElectronicsMapping::TOFHIRASIC(BTLDetId det) {
  uint32_t dmodule = det.dmodule();
  uint32_t smodule = det.smodule();
  return BTLElectronicsMapping::TOFHIRASIC(dmodule, smodule);
}

int BTLElectronicsMapping::TOFHIRASIC(uint32_t rawID) {
  BTLDetId theId(rawID);
  return BTLElectronicsMapping::TOFHIRASIC(theId);
}

/** Returns FE board number */
int BTLElectronicsMapping::FEBoardFromDM(uint32_t dmodule) { return dmodule; }

int BTLElectronicsMapping::FEBoard(BTLDetId det) {
  uint32_t dmodule = det.dmodule();
  return BTLElectronicsMapping::FEBoardFromDM(dmodule);
}

int BTLElectronicsMapping::FEBoard(uint32_t rawID) {
  BTLDetId theId(rawID);
  return BTLElectronicsMapping::FEBoard(theId);
}

/** Returns CC board number */
int BTLElectronicsMapping::CCBoardFromRU(uint32_t runit) { return runit; }

int BTLElectronicsMapping::CCBoard(BTLDetId det) {
  uint32_t runit = det.runit();
  return BTLElectronicsMapping::CCBoardFromRU(runit);
}

int BTLElectronicsMapping::CCBoard(uint32_t rawID) {
  BTLDetId theId(rawID);
  return BTLElectronicsMapping::CCBoard(theId);
}
