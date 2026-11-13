#ifndef CondFormats_MTDObjects_BTLTDCCalibrationHost_h
#define CondFormats_MTDObjects_BTLTDCCalibrationHost_h

#include "CondFormats/MTDObjects/interface/BTLTDCCalibrationSoA.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"

namespace btl {
  using BTLTDCCalibrationHost = PortableHostCollection<BTLTDCCalibrationSoA>;
}  // namespace btl

#endif
