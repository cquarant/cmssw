#include "CondCore/ESSources/interface/registration_macros.h"
#include "CondFormats/MTDObjects/interface/BTLReadoutMap.h"
#include "CondFormats/MTDObjects/interface/BTLTDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibration.h"
#include "CondFormats/DataRecord/interface/BTLReadoutMapRcd.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationRcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationRcd.h"

namespace {
  struct initializeBtlReadoutMap {
    void operator()(BTLReadoutMap& m) { m.initialize(); }
  };
}  // namespace

REGISTER_PLUGIN_INIT(BTLReadoutMapRcd, BTLReadoutMap, initializeBtlReadoutMap);
REGISTER_PLUGIN(BTLTDCCalibrationRcd, BTLTDCCalibration);
REGISTER_PLUGIN(BTLQDCCalibrationRcd, BTLQDCCalibration);
