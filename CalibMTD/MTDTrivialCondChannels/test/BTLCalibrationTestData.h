#ifndef CalibMTD_MTDTrivialCondChannels_test_BTLCalibrationTestData_h
#define CalibMTD_MTDTrivialCondChannels_test_BTLCalibrationTestData_h
#include "CondFormats/MTDObjects/interface/BTLTDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsId.h"
#include <stdexcept>

namespace btltest {
  inline void check(bool value) {
    if (!value)
      throw std::runtime_error("Unexpected BTL calibration payload/SoA content");
  }
  inline float value(unsigned run, unsigned channel, unsigned coefficient, unsigned tac) {
    return 10000.f * run + 100.f * channel + 8.f * coefficient + tac;
  }
  inline BTLTDCCalibration makeTDC(unsigned run) {
    BTLTDCCalibration result;
    // Deliberately unsorted, including raw ID zero and the last dense index.
    result.rawIds = {BTLElectronicsId(143, 35, 23, 31).rawId(), 0, 20};
    result.valid = {0x81, 0xff, 0x04};
    for (unsigned channel = 0; channel < result.size(); ++channel)
      for (unsigned tac = 0; tac < 8; ++tac) {
        result.t0_T1.push_back(value(run, channel, 0, tac));
        result.a0_T1.push_back(value(run, channel, 1, tac));
        result.a1_T1.push_back(value(run, channel, 2, tac));
        result.a2_T1.push_back(value(run, channel, 3, tac));
        result.t0_T2.push_back(value(run, channel, 4, tac));
        result.a0_T2.push_back(value(run, channel, 5, tac));
        result.a1_T2.push_back(value(run, channel, 6, tac));
        result.a2_T2.push_back(value(run, channel, 7, tac));
      }
    return result;
  }
  inline BTLQDCCalibration makeQDC(unsigned run) {
    BTLQDCCalibration result;
    // Deliberately unsorted, including raw ID zero and the last dense index.
    result.rawIds = {BTLElectronicsId(143, 35, 23, 31).rawId(), 0, 20};
    result.valid = {0x81, 0xff, 0x04};
    for (unsigned channel = 0; channel < result.size(); ++channel)
      for (unsigned tac = 0; tac < 8; ++tac) {
        result.p0.push_back(value(run, channel, 0, tac));
        result.p1.push_back(value(run, channel, 1, tac));
        result.p2.push_back(value(run, channel, 2, tac));
        result.p3.push_back(value(run, channel, 3, tac));
        result.p4.push_back(value(run, channel, 4, tac));
        result.p5.push_back(value(run, channel, 5, tac));
        result.p6.push_back(value(run, channel, 6, tac));
        result.p7.push_back(value(run, channel, 7, tac));
        result.p8.push_back(value(run, channel, 8, tac));
        result.p9.push_back(value(run, channel, 9, tac));
      }
    return result;
  }
}  // namespace btltest
#endif
