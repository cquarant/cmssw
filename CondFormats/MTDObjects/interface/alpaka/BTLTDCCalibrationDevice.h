#ifndef CondFormats_MTDObjects_alpaka_BTLTDCCalibrationDevice_h
#define CondFormats_MTDObjects_alpaka_BTLTDCCalibrationDevice_h

#include "CondFormats/MTDObjects/interface/BTLTDCCalibrationHost.h"
#include "DataFormats/Portable/interface/alpaka/PortableCollection.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {
  namespace btl {
    using ::btl::BTLTDCCalibrationHost;
    using BTLTDCCalibrationDevice = PortableCollection<::btl::BTLTDCCalibrationSoA>;
  }  // namespace btl
}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

#endif
