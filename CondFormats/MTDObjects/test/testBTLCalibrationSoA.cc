#include "CondFormats/MTDObjects/interface/BTLReadoutMap.h"
#include "CondFormats/MTDObjects/interface/BTLTDCCalibrationHost.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibrationHost.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "HeterogeneousCore/AlpakaInterface/interface/host.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {
  void require(bool value) {
    if (!value) {
      throw std::runtime_error("BTL calibration SoA test failed");
    }
  }

  template <typename F>
  void requireException(F&& f) {
    bool thrown = false;
    try {
      f();
    } catch (cms::Exception const&) {
      thrown = true;
    }
    require(thrown);
  }
}  // namespace

int main() {
  // Deliberately sparse IDs and reverse insertion order: the index must
  // depend only on the channel set, not unordered_map iteration order.
  BTLReadoutMap map;
  const BTLElectronicsId a(0, 4, 0, 3), b(0, 4, 0, 20);
  const BTLElectronicsId c(1, 75, 23, 0), d(1, 75, 23, 31);
  map.add(BTLDetId(2), {d, c});
  map.add(BTLDetId(1), {b, a});
  requireException([&] { map.denseChannelIndex(a); });
  map.initialize();
  const std::vector<uint32_t> expected{a.rawId(), b.rawId(), c.rawId(), d.rawId()};
  require(map.channelRawIds() == expected);
  for (uint32_t i = 0; i < expected.size(); ++i) {
    require(map.denseChannelIndex(BTLElectronicsId(expected[i])) == i);
  }
  requireException([&] { map.denseChannelIndex(BTLElectronicsId(0)); });
  map.initialize();
  require(map.channelRawIds() == expected);

  BTLReadoutMap reordered;
  reordered.add(BTLDetId(1), {b, a});
  reordered.add(BTLDetId(2), {d, c});
  reordered.initialize();
  require(reordered.channelRawIds() == expected);

  btl::BTLTDCCalibrationHost tdc(cms::alpakatools::host(), expected.size());
  btl::BTLQDCCalibrationHost qdc(cms::alpakatools::host(), expected.size());
  tdc.zeroInitialise();
  qdc.zeroInitialise();
  for (uint32_t i = 0; i < expected.size(); ++i) {
    tdc.view()[i].rawId() = expected[i];
    qdc.view()[i].rawId() = expected[i];
    // Fill each TAC and channel with distinct values, including p9.
    for (int tac = 0; tac < btl::kCalibrationTACs; ++tac) {
      tdc.view()[i].a1()(tac) = 100 * i + tac + 1;
      qdc.view()[i].p9()(tac) = 100 * i + tac + 2;
    }
    tdc.view()[i].valid() = 0xff;
    qdc.view()[i].valid() = 0x01;
  }
  const auto& tdcView = tdc.const_view();
  const auto& qdcView = qdc.const_view();
  for (uint32_t i = 0; i < expected.size(); ++i) {
    require(tdcView[i].rawId() == expected[i] && qdcView[i].rawId() == expected[i]);
    require(tdcView[i].valid() == 0xff && qdcView[i].valid() == 0x01);
    for (int tac = 0; tac < btl::kCalibrationTACs; ++tac) {
      require(tdcView[i].a1()(tac) == 100 * i + tac + 1);
      require(qdcView[i].p9()(tac) == 100 * i + tac + 2);
      require(tdcView[i].t0()(tac) == 0.f && qdcView[i].p0()(tac) == 0.f);
    }
  }

  map.add(BTLDetId(3), {BTLElectronicsId(2, 4, 0, 0), BTLElectronicsId(2, 4, 0, 1)});
  requireException([&] { map.channelRawIds(); });
  map.initialize();
  require(map.channelRawIds().size() == 6);
  map.clear();
  requireException([&] { map.channelRawIds(); });
  map.initialize();
  require(map.channelRawIds().empty());

  BTLReadoutMap duplicate;
  duplicate.add(BTLDetId(1), {a, b});
  duplicate.initialize();
  requireException([&] { duplicate.add(BTLDetId(2), {a, c}); });
  requireException([&] { duplicate.add(BTLDetId(2), {c, a}); });
  requireException([&] { duplicate.add(BTLDetId(2), {c, c}); });
  require(duplicate.size() == 1);
  require(duplicate.channelRawIds().size() == 2);
  duplicate.initialize();
  require(duplicate.channelRawIds().size() == 2);
}
