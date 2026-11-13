#ifndef CondFormats_MTDObjects_BTLTDCCalibration_h
#define CondFormats_MTDObjects_BTLTDCCalibration_h

#include "CondFormats/Serialization/interface/Serializable.h"
#include <cstddef>
#include <cstdint>
#include <vector>

// Persistent payload, independent of the portable host/device memory layout.
// rawIds[i] identifies channel i; valid[i] is its per-TAC validity mask.
// Each coefficient has rawIds.size()*8 entries, at i*8 + tac.
// Entries may be sparse and in arbitrary order. Call validate() before use/write.
struct BTLTDCCalibration {
  std::vector<uint32_t> rawIds;
  std::vector<uint8_t> valid;
  std::vector<float> t0;
  std::vector<float> a0;
  std::vector<float> a1;
  std::vector<float> a2;

  std::size_t size() const { return rawIds.size(); }
  void validate() const;

  COND_SERIALIZABLE;
};

#endif
