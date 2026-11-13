#ifndef CondFormats_MTDObjects_BTLTDCCalibrationSoA_h
#define CondFormats_MTDObjects_BTLTDCCalibrationSoA_h

#include <cstdint>

#include "CondFormats/MTDObjects/interface/BTLCalibrationCommon.h"
#include "DataFormats/SoATemplate/interface/SoACommon.h"
#include "DataFormats/SoATemplate/interface/SoALayout.h"

namespace btl {
  // One row per electronics channel; independent T1 and T2 coefficients per TAC.
  // For each discriminator, using its own t0/a0/a1/a2 coefficients:
  // Model: fineADC = a0 + a1*q + a2*q*q,
  // q = coarse - time/clockPeriod - t0. Thus t0 and q are in clock units,
  // and a0/a1/a2 are in ADC counts per corresponding power of q.
  // Reconstructed time = (coarse - t0 - q) * clockPeriod; the consumer
  // chooses the physical inverse branch and the output time unit.
  // These coefficients describe the fine-time response, not time walk.
  // Row index is BTLElectronicsId::denseChannelIndex(), over the full BTL range.
  // valid is an eight-bit mask: bit t marks both T1 and T2 calibrated for TAC t.
  GENERATE_SOA_LAYOUT(BTLTDCCalibrationSoALayout,
                      SOA_COLUMN(uint32_t, rawId),
                      SOA_EIGEN_COLUMN(TACVector, t0_T1),
                      SOA_EIGEN_COLUMN(TACVector, a0_T1),
                      SOA_EIGEN_COLUMN(TACVector, a1_T1),
                      SOA_EIGEN_COLUMN(TACVector, a2_T1),
                      SOA_EIGEN_COLUMN(TACVector, t0_T2),
                      SOA_EIGEN_COLUMN(TACVector, a0_T2),
                      SOA_EIGEN_COLUMN(TACVector, a1_T2),
                      SOA_EIGEN_COLUMN(TACVector, a2_T2),
                      SOA_COLUMN(uint8_t, valid))

  using BTLTDCCalibrationSoA = BTLTDCCalibrationSoALayout<>;
}  // namespace btl

#endif
