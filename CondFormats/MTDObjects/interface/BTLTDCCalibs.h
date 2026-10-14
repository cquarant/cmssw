#ifndef CondFormats_MTDObjects_BTLTDCCalibs_h
#define CondFormats_MTDObjects_BTLTDCCalibs_h

#include "CondFormats/MTDObjects/interface/MTDCondObjectContainer.h"
#include "CondFormats/Serialization/interface/Serializable.h"
#include "boost/serialization/array.hpp"

#include <array>
#include <cstddef>

static constexpr std::size_t kNTACs = 8;

// One TDC Calib for a specific electronic channel and TAC.
// Replace dummyValue with the actual TDC Calib parameters.
struct BTLTDCChannelCalib {

    std::array<float, kNTACs> t0{};
    std::array<float, kNTACs> a0{};
    std::array<float, kNTACs> a1{};
    std::array<float, kNTACs> a2{};

    COND_SERIALIZABLE;
};

// For each BTL crystal, we store two electronic readout channels
// (minus and plus) and, for each channel, eight TAC Calibs.
struct BTLTDCCrystalCalib {
    static constexpr std::size_t kNReadoutSides = 2;
    static constexpr std::size_t kMinus = 0;
    static constexpr std::size_t kPlus = 1;

    std::array<BTLTDCChannelCalib, kNReadoutSides> channels{};

    BTLTDCChannelCalib& at(std::size_t side, std::size_t tac) {
        return channels.at(side);
    }

    const BTLTDCChannelCalib& at(std::size_t side, std::size_t tac) const {
        return channels.at(side);
    }

    COND_SERIALIZABLE;
};

// Top-level condition payload. The key is a BTLDetId::rawId(); the value
// contains all 2 x 8 readout-channel/TAC Calibs for the crystal.
typedef MTDCondObjectContainer<BTLTDCCrystalCalib> BTLTDCCalibsMap;
typedef BTLTDCCalibsMap::const_iterator BTLTDCCalibsMapIterator;
typedef BTLTDCCalibsMap BTLTDCCalibs;

#endif  // CondFormats_MTDObjects_BTLTDCCalibs_h
