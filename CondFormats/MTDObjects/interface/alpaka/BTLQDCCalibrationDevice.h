#ifndef CondFormats_MTDObjects_alpaka_BTLQDCCalibrationDevice_h
#define CondFormats_MTDObjects_alpaka_BTLQDCCalibrationDevice_h

#include "CondFormats/MTDObjects/interface/BTLQDCCalibrationHost.h"
#include "DataFormats/Portable/interface/alpaka/PortableCollection.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {
  namespace btl {
    using ::btl::BTLQDCCalibrationHost;
    using BTLQDCCalibrationDevice = PortableCollection<::btl::BTLQDCCalibrationSoA>;
  }  // namespace btl
}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

#endif
