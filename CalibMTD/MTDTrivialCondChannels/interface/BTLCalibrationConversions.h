#ifndef CalibMTD_MTDTrivialCondChannels_BTLCalibrationConversions_h
#define CalibMTD_MTDTrivialCondChannels_BTLCalibrationConversions_h

#include "CondFormats/MTDObjects/interface/BTLTDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLTDCCalibrationHost.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibrationHost.h"

namespace btl {
  BTLTDCCalibrationHost makeSoA(const BTLTDCCalibration&);
  BTLQDCCalibrationHost makeSoA(const BTLQDCCalibration&);
  // Compact output: rows with no valid TACs are omitted.
  BTLTDCCalibration makePayload(const BTLTDCCalibrationHost&);
  BTLQDCCalibration makePayload(const BTLQDCCalibrationHost&);
}  // namespace btl

#endif
