#ifndef CondFormats_MTDObjects_BTLQDCCalibrationSoA_h
#define CondFormats_MTDObjects_BTLQDCCalibrationSoA_h

#include <cstdint>

#include "CondFormats/MTDObjects/interface/BTLCalibrationCommon.h"
#include "DataFormats/SoATemplate/interface/SoACommon.h"
#include "DataFormats/SoATemplate/interface/SoALayout.h"

namespace btl {
  // One row per electronics channel; each coefficient has one entry per TAC.
  // Pedestal in ADC counts: p0 + p1*ti + ... + p9*ti^9,
  // where ti = EOIcoarse - time/clockPeriod is in clock units.
  // Subtract this pedestal from Qfine before converting charge to energy.
  // Gain / ADC-to-energy conversion is a separate calibration.
  // Row i corresponds to BTLReadoutMap::channelRawIds()[i].
  // valid is an eight-bit mask: bit t marks the calibration of TAC t.
  GENERATE_SOA_LAYOUT(BTLQDCCalibrationSoALayout,
                      SOA_COLUMN(uint32_t, rawId),
                      SOA_EIGEN_COLUMN(TACVector, p0),
                      SOA_EIGEN_COLUMN(TACVector, p1),
                      SOA_EIGEN_COLUMN(TACVector, p2),
                      SOA_EIGEN_COLUMN(TACVector, p3),
                      SOA_EIGEN_COLUMN(TACVector, p4),
                      SOA_EIGEN_COLUMN(TACVector, p5),
                      SOA_EIGEN_COLUMN(TACVector, p6),
                      SOA_EIGEN_COLUMN(TACVector, p7),
                      SOA_EIGEN_COLUMN(TACVector, p8),
                      SOA_EIGEN_COLUMN(TACVector, p9),
                      SOA_COLUMN(uint8_t, valid))

  using BTLQDCCalibrationSoA = BTLQDCCalibrationSoALayout<>;
}  // namespace btl

#endif
