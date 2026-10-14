#include "CondCore/ESSources/interface/registration_macros.h"

#include "CondFormats/MTDObjects/interface/BTLReadoutMap.h"
#include "CondFormats/DataRecord/interface/BTLReadoutMapRcd.h"

#include "CondFormats/MTDObjects/interface/BTLTDCCalibs.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibsRcd.h"

namespace {
  struct initializeBtlReadoutMap {
    void operator()(BTLReadoutMap& m) { m.initialize(); }
  };
}  // namespace

REGISTER_PLUGIN_INIT(BTLReadoutMapRcd, BTLReadoutMap, initializeBtlReadoutMap);
REGISTER_PLUGIN(BTLTDCCalibsRcd, BTLTDCCalibs);