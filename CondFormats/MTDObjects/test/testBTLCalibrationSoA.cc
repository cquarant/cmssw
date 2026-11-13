#include "CondFormats/MTDObjects/interface/BTLReadoutMap.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h"
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
  // Exhaustively verify that the reference DAQ assignment covers 0..331775
  // exactly once, in raw-ID order, including FED and Serenity boundaries.
  uint32_t next = 0, previousRawId = 0;
  for (int serenity = 0; serenity < 12; ++serenity) {
    for (int hs = 0; hs < 36; ++hs) {
      const int fed = serenity * 12 + hs / 3;
      for (int el = 0; el < 24; ++el) {
        for (int ch = 0; ch < 32; ++ch) {
          const BTLElectronicsId id(fed, hs, el, ch);
          require(id.isValid());
          require(id.fedId() == fed && id.hsLinkId() == hs && id.eLinkId() == el && id.channelId() == ch);
          require(id.denseChannelIndex() == next);
          require(next == 0 || previousRawId < id.rawId());
          previousRawId = id.rawId();
          ++next;
        }
      }
    }
  }
  require(next == BTLElectronicsIndexing::kNumberOfChannels);
  require(!BTLElectronicsId(0, 3, 0, 0).isValid());  // HS-link belongs to FED 1
  require(!BTLElectronicsId(12, 35, 0, 0).isValid());
  require(!BTLElectronicsId(144, 0, 0, 0).isValid());
  require(!BTLElectronicsId(0, 36, 0, 0).isValid());
  require(!BTLElectronicsId(0, 0, 24, 0).isValid());

  // A sparse map must not renumber channels when entries are inserted or removed.
  BTLReadoutMap map;
  const BTLElectronicsId a(0, 0, 0, 0), b(0, 0, 0, 20);
  const BTLElectronicsId c(143, 35, 23, 0), d(143, 35, 23, 31);
  map.add(BTLDetId(2), {d, c});
  map.add(BTLDetId(1), {b, a});
  map.initialize();
  const std::vector<uint32_t> expected{a.rawId(), b.rawId(), c.rawId(), d.rawId()};
  require(a.denseChannelIndex() == 0 && b.denseChannelIndex() == 20);
  require(c.denseChannelIndex() == 331744 && d.denseChannelIndex() == 331775);
  for (auto rawId : expected) {
    const BTLElectronicsId id(rawId);
    const auto detId = map.getDetId(id);
    const auto pair = map.getElectronicsId(detId);
    require(pair.minus == id || pair.plus == id);
  }
  requireException([&] { map.getDetId(BTLElectronicsId(0, 0, 0, 1)); });

  btl::BTLTDCCalibrationHost tdc(cms::alpakatools::host(), BTLElectronicsIndexing::kNumberOfChannels);
  btl::BTLQDCCalibrationHost qdc(cms::alpakatools::host(), BTLElectronicsIndexing::kNumberOfChannels);
  tdc.zeroInitialise();
  qdc.zeroInitialise();
  for (auto rawId : expected) {
    const auto i = BTLElectronicsId(rawId).denseChannelIndex();
    tdc.view()[i].rawId() = rawId;
    qdc.view()[i].rawId() = rawId;
    // Fill each TAC and channel with distinct values, including p9.
    for (int tac = 0; tac < btl::kCalibrationTACs; ++tac) {
      tdc.view()[i].a1_T1()(tac) = i + 0.125f * tac + 1;
      tdc.view()[i].a1_T2()(tac) = i + 0.125f * tac + 3;
      qdc.view()[i].p9()(tac) = i + 0.125f * tac + 2;
    }
    tdc.view()[i].valid() = 0xff;
    qdc.view()[i].valid() = 0x01;
  }
  const auto& tdcView = tdc.const_view();
  const auto& qdcView = qdc.const_view();
  for (auto rawId : expected) {
    const auto i = BTLElectronicsId(rawId).denseChannelIndex();
    require(tdcView[i].rawId() == rawId && qdcView[i].rawId() == rawId);
    require(tdcView[i].valid() == 0xff && qdcView[i].valid() == 0x01);
    for (int tac = 0; tac < btl::kCalibrationTACs; ++tac) {
      require(tdcView[i].a1_T1()(tac) == i + 0.125f * tac + 1);
      require(tdcView[i].a1_T2()(tac) == i + 0.125f * tac + 3);
      require(qdcView[i].p9()(tac) == i + 0.125f * tac + 2);
      require(tdcView[i].t0_T1()(tac) == 0.f && tdcView[i].t0_T2()(tac) == 0.f && qdcView[i].p0()(tac) == 0.f);
    }
  }

  require(tdcView[1].valid() == 0 && qdcView[1].valid() == 0);
  map.add(BTLDetId(3), {BTLElectronicsId(12, 0, 0, 0), BTLElectronicsId(12, 0, 0, 1)});
  map.initialize();
  require(d.denseChannelIndex() == 331775);
  require(map.getListOfDetIds().size() == 3);
  map.clear();
  map.initialize();
  require(map.getListOfDetIds().empty());

  BTLReadoutMap duplicate;
  duplicate.add(BTLDetId(1), {a, b});
  duplicate.initialize();
  requireException([&] { duplicate.add(BTLDetId(2), {a, c}); });
  requireException([&] { duplicate.add(BTLDetId(2), {c, a}); });
  requireException([&] { duplicate.add(BTLDetId(2), {c, c}); });
  require(duplicate.size() == 1);
  require(duplicate.getListOfDetIds().size() == 1);
  duplicate.initialize();
  require(duplicate.getListOfDetIds().size() == 1);
}
