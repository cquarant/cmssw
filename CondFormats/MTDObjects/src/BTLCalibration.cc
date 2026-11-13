#include "CondFormats/MTDObjects/interface/BTLTDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsId.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h"
#include <cmath>
#include <initializer_list>
#include <stdexcept>
#include <vector>

namespace {
  void validate(const std::vector<uint32_t>& ids,
                const std::vector<uint8_t>& valid,
                std::initializer_list<const std::vector<float>*> coefficients) {
    if (ids.size() > BTLElectronicsIndexing::kNumberOfChannels || valid.size() != ids.size())
      throw std::invalid_argument("BTL calibration: invalid channel/mask vector sizes");
    std::vector<bool> seen(BTLElectronicsIndexing::kNumberOfChannels, false);
    for (auto rawId : ids) {
      const BTLElectronicsId id(rawId);
      if ((rawId >> 29) != 0 || !id.isValid())
        throw std::invalid_argument("BTL calibration: invalid electronics ID");
      const auto index = id.denseChannelIndex();
      if (seen[index])
        throw std::invalid_argument("BTL calibration: duplicate electronics ID");
      seen[index] = true;
    }
    for (const auto* values : coefficients) {
      if (values->size() != ids.size() * 8)
        throw std::invalid_argument("BTL calibration: invalid coefficient vector size");
      for (auto value : *values)
        if (!std::isfinite(value))
          throw std::invalid_argument("BTL calibration: nonfinite coefficient");
    }
  }
}  // namespace

void BTLTDCCalibration::validate() const { ::validate(rawIds, valid, {&t0, &a0, &a1, &a2}); }
void BTLQDCCalibration::validate() const {
  ::validate(rawIds, valid, {&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9});
}
