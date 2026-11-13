#include "CalibMTD/MTDTrivialCondChannels/interface/BTLCalibrationConversions.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsId.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h"
#include "HeterogeneousCore/AlpakaInterface/interface/host.h"
#include <stdexcept>

namespace btl {
  BTLTDCCalibrationHost makeSoA(const BTLTDCCalibration& payload) {
    payload.validate();
    BTLTDCCalibrationHost result(cms::alpakatools::host(), BTLElectronicsIndexing::kNumberOfChannels);
    result.zeroInitialise();
    auto view = result.view();
    for (std::size_t channel = 0; channel < payload.size(); ++channel) {
      const auto i = BTLElectronicsId(payload.rawIds[channel]).denseChannelIndex();
      view[i].rawId() = payload.rawIds[channel];
      view[i].valid() = payload.valid[channel];
      for (int tac = 0; tac < kCalibrationTACs; ++tac) {
        view[i].t0_T1()(tac) = payload.t0_T1[channel * kCalibrationTACs + tac];
        view[i].a0_T1()(tac) = payload.a0_T1[channel * kCalibrationTACs + tac];
        view[i].a1_T1()(tac) = payload.a1_T1[channel * kCalibrationTACs + tac];
        view[i].a2_T1()(tac) = payload.a2_T1[channel * kCalibrationTACs + tac];
        view[i].t0_T2()(tac) = payload.t0_T2[channel * kCalibrationTACs + tac];
        view[i].a0_T2()(tac) = payload.a0_T2[channel * kCalibrationTACs + tac];
        view[i].a1_T2()(tac) = payload.a1_T2[channel * kCalibrationTACs + tac];
        view[i].a2_T2()(tac) = payload.a2_T2[channel * kCalibrationTACs + tac];
      }
    }
    return result;
  }

  BTLTDCCalibration makePayload(const BTLTDCCalibrationHost& collection) {
    const auto view = collection.const_view();
    if (view.metadata().size() != BTLElectronicsIndexing::kNumberOfChannels)
      throw std::invalid_argument("BTL calibration: unexpected SoA size");
    BTLTDCCalibration result;
    for (int i = 0; i < view.metadata().size(); ++i) {
      if (view[i].valid() == 0)
        continue;
      const BTLElectronicsId id(view[i].rawId());
      if ((id.rawId() >> 29) != 0 || !id.isValid() || id.denseChannelIndex() != static_cast<uint32_t>(i))
        throw std::invalid_argument("BTL calibration: SoA raw ID does not match its dense row index");
      result.rawIds.push_back(id.rawId());
      result.valid.push_back(view[i].valid());
      for (int tac = 0; tac < kCalibrationTACs; ++tac) {
        result.t0_T1.push_back(view[i].t0_T1()(tac));
        result.a0_T1.push_back(view[i].a0_T1()(tac));
        result.a1_T1.push_back(view[i].a1_T1()(tac));
        result.a2_T1.push_back(view[i].a2_T1()(tac));
        result.t0_T2.push_back(view[i].t0_T2()(tac));
        result.a0_T2.push_back(view[i].a0_T2()(tac));
        result.a1_T2.push_back(view[i].a1_T2()(tac));
        result.a2_T2.push_back(view[i].a2_T2()(tac));
      }
    }
    result.validate();
    return result;
  }

  BTLQDCCalibrationHost makeSoA(const BTLQDCCalibration& payload) {
    payload.validate();
    BTLQDCCalibrationHost result(cms::alpakatools::host(), BTLElectronicsIndexing::kNumberOfChannels);
    result.zeroInitialise();
    auto view = result.view();
    for (std::size_t channel = 0; channel < payload.size(); ++channel) {
      const auto i = BTLElectronicsId(payload.rawIds[channel]).denseChannelIndex();
      view[i].rawId() = payload.rawIds[channel];
      view[i].valid() = payload.valid[channel];
      for (int tac = 0; tac < kCalibrationTACs; ++tac) {
        view[i].p0()(tac) = payload.p0[channel * kCalibrationTACs + tac];
        view[i].p1()(tac) = payload.p1[channel * kCalibrationTACs + tac];
        view[i].p2()(tac) = payload.p2[channel * kCalibrationTACs + tac];
        view[i].p3()(tac) = payload.p3[channel * kCalibrationTACs + tac];
        view[i].p4()(tac) = payload.p4[channel * kCalibrationTACs + tac];
        view[i].p5()(tac) = payload.p5[channel * kCalibrationTACs + tac];
        view[i].p6()(tac) = payload.p6[channel * kCalibrationTACs + tac];
        view[i].p7()(tac) = payload.p7[channel * kCalibrationTACs + tac];
        view[i].p8()(tac) = payload.p8[channel * kCalibrationTACs + tac];
        view[i].p9()(tac) = payload.p9[channel * kCalibrationTACs + tac];
      }
    }
    return result;
  }

  BTLQDCCalibration makePayload(const BTLQDCCalibrationHost& collection) {
    const auto view = collection.const_view();
    if (view.metadata().size() != BTLElectronicsIndexing::kNumberOfChannels)
      throw std::invalid_argument("BTL calibration: unexpected SoA size");
    BTLQDCCalibration result;
    for (int i = 0; i < view.metadata().size(); ++i) {
      if (view[i].valid() == 0)
        continue;
      const BTLElectronicsId id(view[i].rawId());
      if ((id.rawId() >> 29) != 0 || !id.isValid() || id.denseChannelIndex() != static_cast<uint32_t>(i))
        throw std::invalid_argument("BTL calibration: SoA raw ID does not match its dense row index");
      result.rawIds.push_back(id.rawId());
      result.valid.push_back(view[i].valid());
      for (int tac = 0; tac < kCalibrationTACs; ++tac) {
        result.p0.push_back(view[i].p0()(tac));
        result.p1.push_back(view[i].p1()(tac));
        result.p2.push_back(view[i].p2()(tac));
        result.p3.push_back(view[i].p3()(tac));
        result.p4.push_back(view[i].p4()(tac));
        result.p5.push_back(view[i].p5()(tac));
        result.p6.push_back(view[i].p6()(tac));
        result.p7.push_back(view[i].p7()(tac));
        result.p8.push_back(view[i].p8()(tac));
        result.p9.push_back(view[i].p9()(tac));
      }
    }
    result.validate();
    return result;
  }

}  // namespace btl
