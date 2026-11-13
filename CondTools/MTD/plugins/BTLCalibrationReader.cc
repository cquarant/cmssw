#include "CondFormats/DataRecord/interface/BTLTDCCalibrationRcd.h"
#include "CondFormats/DataRecord/interface/BTLQDCCalibrationRcd.h"
#include "CondFormats/MTDObjects/interface/BTLTDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLQDCCalibration.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsId.h"
#include "CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/Exception.h"

#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <utility>

// PoolDBESSource deserializes the payloads; this analyzer only inspects them.
class BTLCalibrationReader : public edm::one::EDAnalyzer<> {
public:
  explicit BTLCalibrationReader(const edm::ParameterSet& config)
      : tdc_(esConsumes()),
        qdc_(esConsumes()),
        rawId_(config.getParameter<int>("rawId")),
        tac_(config.getParameter<int>("tac")) {
    const auto denseIndex = config.getParameter<int>("denseIndex");
    if (denseIndex < -1 || denseIndex >= BTLElectronicsIndexing::kNumberOfChannels)
      throw cms::Exception("Configuration")
          << "denseIndex must be 0.." << BTLElectronicsIndexing::kNumberOfChannels - 1 << ", or -1 to select by rawId";
    if (denseIndex >= 0) {
      if (rawId_ != -2)
        throw cms::Exception("Configuration") << "Specify either rawId or denseIndex, not both";
      const auto channel = denseIndex % BTLElectronicsSpecs::nChannelsPerELink;
      const auto chip = denseIndex / BTLElectronicsSpecs::nChannelsPerELink;
      const auto eLink = chip % BTLElectronicsSpecs::nELinksPerHSLink;
      const auto link = chip / BTLElectronicsSpecs::nELinksPerHSLink;
      const auto hsLink = link % BTLElectronicsSpecs::nHSLinksPerSerenity;
      const auto serenity = link / BTLElectronicsSpecs::nHSLinksPerSerenity;
      const auto fed = BTLElectronicsSpecs::kFirstFEDId + serenity * BTLElectronicsSpecs::nFEDsPerSerenity +
                       hsLink / BTLElectronicsSpecs::nHSLinksPerFED;
      rawId_ = BTLElectronicsId(fed, hsLink, eLink, channel).rawId();
    } else if (rawId_ == -2) {
      rawId_ = 0;
    }
    if (rawId_ < -1 || (rawId_ >= 0 && ((rawId_ >> 29) != 0 || !BTLElectronicsId(rawId_).isValid())))
      throw cms::Exception("Configuration") << "rawId must be a valid electronics ID, or -1 for all channels";
    if (tac_ < -1 || tac_ > 7)
      throw cms::Exception("Configuration") << "tac must be 0..7, or -1 for all TACs";
  }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    // -2 means no explicit raw-ID selection; the default remains channel zero.
    desc.add<int>("rawId", -2);
    desc.add<int>("denseIndex", -1);
    desc.add<int>("tac", 0);
    descriptions.addWithDefaultLabel(desc);
  }

  void analyze(const edm::Event& event, const edm::EventSetup& setup) override {
    const auto& tdc = setup.getData(tdc_);
    const auto& qdc = setup.getData(qdc_);
    tdc.validate();
    qdc.validate();
    std::cout << "BTL calibration run=" << event.id().run() << " TDC_channels=" << tdc.size()
              << " QDC_channels=" << qdc.size() << '\n';
    dump("TDC",
         tdc,
         {{"t0_T1", &tdc.t0_T1},
          {"a0_T1", &tdc.a0_T1},
          {"a1_T1", &tdc.a1_T1},
          {"a2_T1", &tdc.a2_T1},
          {"t0_T2", &tdc.t0_T2},
          {"a0_T2", &tdc.a0_T2},
          {"a1_T2", &tdc.a1_T2},
          {"a2_T2", &tdc.a2_T2}});
    dump("QDC",
         qdc,
         {{"p0", &qdc.p0},
          {"p1", &qdc.p1},
          {"p2", &qdc.p2},
          {"p3", &qdc.p3},
          {"p4", &qdc.p4},
          {"p5", &qdc.p5},
          {"p6", &qdc.p6},
          {"p7", &qdc.p7},
          {"p8", &qdc.p8},
          {"p9", &qdc.p9}});
  }

private:
  template <typename Payload>
  void dump(const char* name,
            const Payload& payload,
            std::initializer_list<std::pair<const char*, const std::vector<float>*>> coefficients) const {
    bool found = false;
    for (std::size_t channel = 0; channel < payload.size(); ++channel) {
      if (rawId_ >= 0 && payload.rawIds[channel] != static_cast<uint32_t>(rawId_))
        continue;
      found = true;
      for (int tac = 0; tac < 8; ++tac) {
        if (tac_ >= 0 && tac != tac_)
          continue;
        std::cout << name << " rawId=" << payload.rawIds[channel]
                  << " denseIndex=" << BTLElectronicsId(payload.rawIds[channel]).denseChannelIndex() << " tac=" << tac
                  << " valid=" << ((payload.valid[channel] >> tac) & 1);
        for (const auto& [coefficient, values] : coefficients)
          std::cout << ' ' << coefficient << '=' << std::setprecision(std::numeric_limits<float>::max_digits10)
                    << (*values)[channel * 8 + tac];
        std::cout << '\n';
      }
    }
    if (!found && rawId_ >= 0)
      std::cout << name << " rawId=" << rawId_ << " not stored in this payload\n";
  }

  edm::ESGetToken<BTLTDCCalibration, BTLTDCCalibrationRcd> tdc_;
  edm::ESGetToken<BTLQDCCalibration, BTLQDCCalibrationRcd> qdc_;
  int rawId_, tac_;
};

DEFINE_FWK_MODULE(BTLCalibrationReader);
