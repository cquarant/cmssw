#ifndef CondFormats_MTDObjects_BTLTDCCalibration_h
#define CondFormats_MTDObjects_BTLTDCCalibration_h

#include "CondFormats/Serialization/interface/Serializable.h"

#include <vector>

////////////////////////////////////////
//       C. Quaranta - 13/09/2026
// Class storing the format for the
// BTL TDC calibration payload
////////////////////////////////////////

struct BTLTDCCalibration {
  std::vector<float> t0;
  std::vector<float> a0;
  std::vector<float> a1;
  std::vector<float> a2;

  COND_SERIALIZABLE;
};

#endif