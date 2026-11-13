#ifndef CondFormats_MTDObjects_BTLQDCCalibrationHost_h
#define CondFormats_MTDObjects_BTLQDCCalibrationHost_h

#include "CondFormats/MTDObjects/interface/BTLQDCCalibrationSoA.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"

namespace btl {
  using BTLQDCCalibrationHost = PortableHostCollection<BTLQDCCalibrationSoA>;
}  // namespace btl

#endif
