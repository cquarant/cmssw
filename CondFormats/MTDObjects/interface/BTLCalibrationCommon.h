#ifndef CondFormats_MTDObjects_BTLCalibrationCommon_h
#define CondFormats_MTDObjects_BTLCalibrationCommon_h

#include <Eigen/Core>

namespace btl {
  inline constexpr int kCalibrationTACs = 8;
  using TACVector = Eigen::Matrix<float, kCalibrationTACs, 1>;
}  // namespace btl

#endif
