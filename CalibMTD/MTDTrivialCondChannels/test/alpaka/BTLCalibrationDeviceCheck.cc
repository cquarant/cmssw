#include "CondFormats/DataRecord/interface/BTLQDCCalibrationSoARcd.h"
#include "CondFormats/DataRecord/interface/BTLTDCCalibrationSoARcd.h"
#include "CondFormats/MTDObjects/interface/alpaka/BTLQDCCalibrationDevice.h"
#include "CondFormats/MTDObjects/interface/alpaka/BTLTDCCalibrationDevice.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/ESGetToken.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/MakerMacros.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/global/EDProducer.h"
#include <cstring>
#include <type_traits>

namespace ALPAKA_ACCELERATOR_NAMESPACE {
  class BTLCalibrationDeviceCheck : public global::EDProducer<> {
  public:
    explicit BTLCalibrationDeviceCheck(const edm::ParameterSet& config)
        : EDProducer<>(config),
          tdcHost_(esConsumes()),
          qdcHost_(esConsumes()),
          tdcDevice_(esConsumes()),
          qdcDevice_(esConsumes()) {}

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
      edm::ParameterSetDescription desc;
      descriptions.addWithDefaultLabel(desc);
    }

    void produce(edm::StreamID, device::Event& event, const device::EventSetup& setup) const override {
      check(event.queue(), setup.getData(tdcHost_), setup.getData(tdcDevice_), "TDC");
      check(event.queue(), setup.getData(qdcHost_), setup.getData(qdcDevice_), "QDC");
    }

  private:
    template <typename HostCollection, typename DeviceCollection>
    static void check(Queue& queue, const HostCollection& host, const DeviceCollection& device, const char* name) {
      if constexpr (std::is_same_v<HostCollection, DeviceCollection>) {
        if (&host != &device)
          throw cms::Exception("TestFailure") << "The CPU backend did not reuse the BTL " << name << " host SoA";
      } else {
        const auto copied = cms::alpakatools::CopyToHost<DeviceCollection>::copyAsync(queue, device);
        alpaka::wait(queue);
        const auto expected = host.buffer();
        const auto actual = copied.buffer();
        const auto size = alpaka::getExtentProduct(expected);
        if (alpaka::getExtentProduct(actual) != size || std::memcmp(expected.data(), actual.data(), size) != 0)
          throw cms::Exception("TestFailure") << "The BTL " << name << " device SoA differs from the host SoA";
      }
    }

    const edm::ESGetToken<btl::BTLTDCCalibrationHost, BTLTDCCalibrationSoARcd> tdcHost_;
    const edm::ESGetToken<btl::BTLQDCCalibrationHost, BTLQDCCalibrationSoARcd> qdcHost_;
    const device::ESGetToken<btl::BTLTDCCalibrationDevice, BTLTDCCalibrationSoARcd> tdcDevice_;
    const device::ESGetToken<btl::BTLQDCCalibrationDevice, BTLQDCCalibrationSoARcd> qdcDevice_;
  };
}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

DEFINE_FWK_ALPAKA_MODULE(BTLCalibrationDeviceCheck);
