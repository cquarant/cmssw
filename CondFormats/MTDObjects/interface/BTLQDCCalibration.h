#ifndef CondFormats_MTDObjects_BTLQDCCalibration_h
#define CondFormats_MTDObjects_BTLQDCCalibration_h

#include "CondFormats/Serialization/interface/Serializable.h"
#include <cstddef>
#include <cstdint>
#include <vector>

// Persistent payload, independent of the portable host/device memory layout.
// rawIds[i] identifies channel i; valid[i] is its per-TAC validity mask.
// Each coefficient has rawIds.size()*8 entries, at i*8 + tac.
// Entries may be sparse and in arbitrary order. Call validate() before use/write.
struct BTLQDCCalibration {
  std::vector<uint32_t> rawIds;
  std::vector<uint8_t> valid;
  std::vector<float> p0;
  std::vector<float> p1;
  std::vector<float> p2;
  std::vector<float> p3;
  std::vector<float> p4;
  std::vector<float> p5;
  std::vector<float> p6;
  std::vector<float> p7;
  std::vector<float> p8;
  std::vector<float> p9;

  std::size_t size() const { return rawIds.size(); }
  void validate() const;

  COND_SERIALIZABLE;
};

#endif
