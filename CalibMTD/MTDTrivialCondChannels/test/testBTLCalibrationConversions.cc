#include "CalibMTD/MTDTrivialCondChannels/interface/BTLCalibrationConversions.h"
#include "CalibMTD/MTDTrivialCondChannels/test/BTLCalibrationTestData.h"
#include "HeterogeneousCore/AlpakaInterface/interface/host.h"
#include <limits>

namespace {
  template <typename F>
  void rejects(F&& f) {
    bool thrown = false;
    try {
      f();
    } catch (const std::invalid_argument&) {
      thrown = true;
    }
    btltest::check(thrown);
  }
}  // namespace

int main() {
  {
    auto payload = btltest::makeTDC(1);
    auto soa = btl::makeSoA(payload);
    const auto view = soa.const_view();
    btltest::check(view[0].rawId() == 0 && view[0].valid() == 0xff);
    btltest::check(view[1].valid() == 0 && view[331775].valid() == 0x81);
    auto copy = btl::makePayload(soa);
    copy.validate();
    btltest::check(copy.rawIds == std::vector<uint32_t>({0, 20, payload.rawIds[0]}));
    for (unsigned channel = 0; channel < payload.size(); ++channel) {
      const unsigned row = channel == 0 ? 2 : channel - 1;
      btltest::check(copy.valid[row] == payload.valid[channel]);
      for (unsigned tac = 0; tac < 8; ++tac) {
        btltest::check(copy.t0_T1[row * 8 + tac] == payload.t0_T1[channel * 8 + tac]);
        btltest::check(copy.a0_T1[row * 8 + tac] == payload.a0_T1[channel * 8 + tac]);
        btltest::check(copy.a1_T1[row * 8 + tac] == payload.a1_T1[channel * 8 + tac]);
        btltest::check(copy.a2_T1[row * 8 + tac] == payload.a2_T1[channel * 8 + tac]);
        btltest::check(copy.t0_T2[row * 8 + tac] == payload.t0_T2[channel * 8 + tac]);
        btltest::check(copy.a0_T2[row * 8 + tac] == payload.a0_T2[channel * 8 + tac]);
        btltest::check(copy.a1_T2[row * 8 + tac] == payload.a1_T2[channel * 8 + tac]);
        btltest::check(copy.a2_T2[row * 8 + tac] == payload.a2_T2[channel * 8 + tac]);
      }
    }
    auto bad = payload;
    bad.valid.pop_back();
    rejects([&] { btl::makeSoA(bad); });
    bad = payload;
    bad.rawIds[1] = bad.rawIds[0];
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.rawIds[1] = 0x80000000;
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.rawIds[1] = BTLElectronicsId(0, 3, 0, 0).rawId();
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.a2_T1.pop_back();
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.a2_T2.pop_back();
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.t0_T1[0] = std::numeric_limits<float>::infinity();
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.t0_T2[0] = std::numeric_limits<float>::infinity();
    rejects([&] { bad.validate(); });
    soa.view()[0].rawId() = 20;
    rejects([&] { btl::makePayload(soa); });
    auto partial = btltest::makeTDC(1);
    partial.valid[0] = 0;
    auto compact = btl::makePayload(btl::makeSoA(partial));
    btltest::check(compact.size() == 2 && compact.rawIds[0] == 0 && compact.rawIds[1] == 20);
    auto nonfinite = btl::makeSoA(payload);
    nonfinite.view()[0].t0_T1()(0) = std::numeric_limits<float>::quiet_NaN();
    rejects([&] { btl::makePayload(nonfinite); });
    nonfinite = btl::makeSoA(payload);
    nonfinite.view()[0].t0_T2()(0) = std::numeric_limits<float>::quiet_NaN();
    rejects([&] { btl::makePayload(nonfinite); });
    btl::BTLTDCCalibrationHost wrongSize(cms::alpakatools::host(), 1);
    rejects([&] { btl::makePayload(wrongSize); });
    auto empty = btl::makeSoA(BTLTDCCalibration{});
    btltest::check(btl::makePayload(empty).size() == 0);
  }
  {
    auto payload = btltest::makeQDC(1);
    auto soa = btl::makeSoA(payload);
    const auto view = soa.const_view();
    btltest::check(view[0].rawId() == 0 && view[0].valid() == 0xff);
    btltest::check(view[1].valid() == 0 && view[331775].valid() == 0x81);
    auto copy = btl::makePayload(soa);
    copy.validate();
    btltest::check(copy.rawIds == std::vector<uint32_t>({0, 20, payload.rawIds[0]}));
    for (unsigned channel = 0; channel < payload.size(); ++channel) {
      const unsigned row = channel == 0 ? 2 : channel - 1;
      btltest::check(copy.valid[row] == payload.valid[channel]);
      for (unsigned tac = 0; tac < 8; ++tac) {
        btltest::check(copy.p0[row * 8 + tac] == payload.p0[channel * 8 + tac]);
        btltest::check(copy.p1[row * 8 + tac] == payload.p1[channel * 8 + tac]);
        btltest::check(copy.p2[row * 8 + tac] == payload.p2[channel * 8 + tac]);
        btltest::check(copy.p3[row * 8 + tac] == payload.p3[channel * 8 + tac]);
        btltest::check(copy.p4[row * 8 + tac] == payload.p4[channel * 8 + tac]);
        btltest::check(copy.p5[row * 8 + tac] == payload.p5[channel * 8 + tac]);
        btltest::check(copy.p6[row * 8 + tac] == payload.p6[channel * 8 + tac]);
        btltest::check(copy.p7[row * 8 + tac] == payload.p7[channel * 8 + tac]);
        btltest::check(copy.p8[row * 8 + tac] == payload.p8[channel * 8 + tac]);
        btltest::check(copy.p9[row * 8 + tac] == payload.p9[channel * 8 + tac]);
      }
    }
    auto bad = payload;
    bad.valid.pop_back();
    rejects([&] { btl::makeSoA(bad); });
    bad = payload;
    bad.rawIds[1] = bad.rawIds[0];
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.rawIds[1] = 0x80000000;
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.rawIds[1] = BTLElectronicsId(0, 3, 0, 0).rawId();
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.p9.pop_back();
    rejects([&] { bad.validate(); });
    bad = payload;
    bad.p0[0] = std::numeric_limits<float>::infinity();
    rejects([&] { bad.validate(); });
    soa.view()[0].rawId() = 20;
    rejects([&] { btl::makePayload(soa); });
    auto partial = btltest::makeQDC(1);
    partial.valid[0] = 0;
    auto compact = btl::makePayload(btl::makeSoA(partial));
    btltest::check(compact.size() == 2 && compact.rawIds[0] == 0 && compact.rawIds[1] == 20);
    auto nonfinite = btl::makeSoA(payload);
    nonfinite.view()[0].p0()(0) = std::numeric_limits<float>::quiet_NaN();
    rejects([&] { btl::makePayload(nonfinite); });
    btl::BTLQDCCalibrationHost wrongSize(cms::alpakatools::host(), 1);
    rejects([&] { btl::makePayload(wrongSize); });
    auto empty = btl::makeSoA(BTLQDCCalibration{});
    btltest::check(btl::makePayload(empty).size() == 0);
  }
}
