#include "CalibMTD/MTDTrivialCondChannels/interface/MTDTrivialConditionsRetriever.h"

#include "DataFormats/ForwardDetId/interface/BTLDetId.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/Utilities/interface/Exception.h"

#include <cmath>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <limits>

BTLTDCCrystalCalib makeTDCCrystalCalibration(double t0, double a0, double a1, double a2) {
  BTLTDCCrystalCalib crystal{};
  for (auto& side : crystal.channels) {
    side.t0.fill(t0);
    side.a0.fill(a0);
    side.a1.fill(a1);
    side.a2.fill(a2);
  }
  return crystal;
}

MTDTrivialConditionsRetriever::MTDTrivialConditionsRetriever(const edm::ParameterSet& ps) {
  btltdc_t0_ = ps.getUntrackedParameter<double>("btltdc_t0", 200.);
  btltdc_a0_ = ps.getUntrackedParameter<double>("btltdc_a0", 200.);
  btltdc_a1_ = ps.getUntrackedParameter<double>("btltdc_a1", 200.);
  btltdc_a2_ = ps.getUntrackedParameter<double>("btltdc_a2", 200.);


  
  setWhatProduced(this, &MTDTrivialConditionsRetriever::produceBTLTDCCalibs);
  findingRecord<BTLTDCCalibsRcd>();
}

// MTDTrivialConditionsRetriever::MTDTrivialConditionsRetriever(double t0, double a0, double a1, double a2) {
  //   setWhatProduced(this, &MTDTrivialConditionsRetriever::produceBTLTDCCalibs);
  //   findingRecord<BTLTDCCalibsRcd>();
  // }
  
  std::unique_ptr<BTLTDCCalibs> MTDTrivialConditionsRetriever::produceBTLTDCCalibs(const BTLTDCCalibsRcd&) {
      
  auto tdc_singleXtal = makeTDCCrystalCalibration(btltdc_t0_,btltdc_a0_,btltdc_a1_,btltdc_a2_);

  auto tdc_calibs = std::make_unique<BTLTDCCalibs>();

  // Generate geometrical crystal IDs in the BTL v4 numbering convention.
  // IMPORTANT: these loops describe the nominal v4 ID grid, NOT an enumeration
  // driven by the geometry record. Adapt if your geometry/ID scheme differs.
  // zside: 0(-z) / 1(+z), rod: 1..HALF_ROD;
  // readout units, detector modules, sensor modules and crystals: zero-based.
  for (unsigned zside = 0; zside < 2; ++zside) {
    for (unsigned rod = 1; rod <= BTLDetId::HALF_ROD; ++rod) {
      for (unsigned ru = 0; ru < BTLDetId::kRUPerRod; ++ru) {
        for (unsigned dm = 0; dm < BTLDetId::kDModulesPerRU; ++dm) {
          for (unsigned sm = 0; sm < BTLDetId::kSModulesPerDM; ++sm) {
            for (unsigned crystal = 0; crystal < BTLDetId::kCrystalsPerModuleV2; ++crystal) {
              const BTLDetId id(zside, rod, ru, dm, sm, crystal);
              // Use the typed-ID overload. The rawId classifier is still TODO.
              tdc_calibs->setValue(id, tdc_singleXtal);
            }
          }
        }
      }
    }
  }

  edm::LogInfo("MTDTrivialConditionsRetriever")
      << "Created trivial BTL TDC conditions for " << tdc_calibs->btlItems().size()
      << " BTL crystal IDs, two readout sides and eight TACs per side";
  return tdc_calibs;
}
