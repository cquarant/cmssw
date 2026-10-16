#ifndef CondFormats_MTDObjects_BTLTDChannelCCalibs_h
#define CondFormats_MTDObjects_BTLTDChannelCCalibs_h

#include "CondFormats/MTDObjects/interface/BTLChannelCondObjectContainer.h"
#include "CondFormats/Serialization/interface/Serializable.h"
#include "boost/serialization/array.hpp"

#include <array>
#include <cstddef>

// TDC calibration parameters for all TACs of one electronic channel.
struct BTLTDChannelCCalib {
  static constexpr std::size_t kNTACs = 8;

  std::array<float, kNTACs> t0{};
  std::array<float, kNTACs> a0{};
  std::array<float, kNTACs> a1{};
  std::array<float, kNTACs> a2{};

  COND_SERIALIZABLE;
};

// The key is a BTLElectronicsId::rawId(). The plus and minus readout
// channels of a crystal have separate entries, each with eight TACs.
typedef BTLChannelCondObjectContainer<BTLTDChannelCCalib> BTLTDChannelCCalibsMap;
typedef BTLTDChannelCCalibsMap::const_iterator BTLTDChannelCCalibsMapIterator;
typedef BTLTDChannelCCalibsMap BTLTDChannelCCalibs;

#endif  // CondFormats_MTDObjects_BTLTDChannelCCalibs_h
