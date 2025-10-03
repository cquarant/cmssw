#include <memory>
#include <iostream>
#include <vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/FTLDigi/interface/FTLDigiCollections.h"
#include "DataFormats/FTLDigiSoA/interface/BTLDigiHostCollection.h"

class MTDDigiDumpSoA : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit MTDDigiDumpSoA(const edm::ParameterSet&);
  ~MTDDigiDumpSoA() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------

  edm::EDGetTokenT<BTLDigiCollection> tok_BTL_digi;
  edm::EDGetTokenT<btldigi::BTLDigiHostCollection> tok_BTL_digi_SoA;
  edm::EDGetTokenT<ETLDigiCollection> tok_ETL_digi;
};

MTDDigiDumpSoA::MTDDigiDumpSoA(const edm::ParameterSet& iConfig)

{
  tok_BTL_digi = consumes<BTLDigiCollection>(edm::InputTag("mix", "FTLBarrel"));
  tok_BTL_digi_SoA = consumes<btldigi::BTLDigiHostCollection>(edm::InputTag("mix", "FTLBarrelSoA"));
  tok_ETL_digi = consumes<ETLDigiCollection>(edm::InputTag("mix", "FTLEndcap"));
}

MTDDigiDumpSoA::~MTDDigiDumpSoA() {}

//
// member functions
//

// ------------ method called for each event ------------
void MTDDigiDumpSoA::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace std;

  edm::Handle<BTLDigiCollection> h_BTL_digi;
  iEvent.getByToken(tok_BTL_digi, h_BTL_digi);

  edm::Handle<btldigi::BTLDigiHostCollection> h_BTL_digi_SoA;
  iEvent.getByToken(tok_BTL_digi_SoA, h_BTL_digi_SoA);

  edm::Handle<ETLDigiCollection> h_ETL_digi;
  iEvent.getByToken(tok_ETL_digi, h_ETL_digi);

  // --- BTL DIGIs:

  if (!h_BTL_digi->empty()) {
    std::cout << " ----------------------------------------" << std::endl;
    std::cout << " BTL DIGI collection: \n" << std::endl;

    for (const auto& dataFrame : *h_BTL_digi) {
      // --- detector element ID:
      std::cout << "\n BTL DIGI:  row = " << dataFrame.row() << " col = " << dataFrame.column()
                << " BTLDetId = " << dataFrame.id();

      // --- loop over the dataFrame samples
      for (int isample = 0; isample < dataFrame.size(); ++isample) {
        const auto& sample = dataFrame.sample(isample);

        std::cout << "       sample " << isample << ":";
        if (sample.data() == 0 && sample.toa() == 0) {
          std::cout << std::endl;
          continue;
        }
        std::cout << "  amplitude = " << sample.data() << "  time1 = " << sample.toa() << "  time2 = " << sample.toa2()
                  << " r/c = " << (uint32_t)sample.row() << " / " << (uint32_t)sample.column()
                  << " th = " << sample.threshold() << " mode = " << sample.mode() << std::endl;

      }  // isaple loop

    }  // digi loop

  }  // if ( h_BTL_digi->size() > 0 )

  if (h_BTL_digi_SoA->view().metadata().size() > 0) {
    std::cout << " ----------------------------------------" << std::endl;
    std::cout << " BTL DIGI SoA collection: " << h_BTL_digi_SoA->view().metadata().size() << "\n" << std::endl;

    for(int i=0; i<h_BTL_digi_SoA->view().metadata().size(); i++){

      std::cout << "SoA row" << i << ", rawId : " << h_BTL_digi_SoA->view()[i].rawId() << "\n"
        << ", BC0count: " << h_BTL_digi_SoA->view()[i].BC0count() << "\n"
        << ", status: " << h_BTL_digi_SoA->view()[i].status() << "\n"
        << ", BCcount: " << h_BTL_digi_SoA->view()[i].BCcount() << "\n"
        << ", chIDR: " << (int)h_BTL_digi_SoA->view()[i].chIDR() << "\n"
        << ", T1coarseR: " << h_BTL_digi_SoA->view()[i].T1coarseR() << "\n"
        << ", T1fineR: " << h_BTL_digi_SoA->view()[i].T1fineR() << "\n"
        << ", T2coarseR: " << h_BTL_digi_SoA->view()[i].T2coarseR() << "\n"
        << ", T2fineR: " << h_BTL_digi_SoA->view()[i].T2fineR() << "\n"
        << ", ChargeR: " << h_BTL_digi_SoA->view()[i].ChargeR() << "\n"
        << ", chIDL: " << (int)h_BTL_digi_SoA->view()[i].chIDL() << "\n"
        << ", T1coarseL: " << h_BTL_digi_SoA->view()[i].T1coarseL() << "\n"
        << ", T1fineL: " << h_BTL_digi_SoA->view()[i].T1fineL() << "\n"
        << ", T2coarseL: " << h_BTL_digi_SoA->view()[i].T2coarseL() << "\n"
        << ", T2fineL: " << h_BTL_digi_SoA->view()[i].T2fineL() << "\n"
        << ", ChargeL: " << h_BTL_digi_SoA->view()[i].ChargeL() << "\n"
        << ", IdleTimeR: " << h_BTL_digi_SoA->view()[i].IdleTimeR() << "\n"
        << ", PrevTrigFR: " << (int)h_BTL_digi_SoA->view()[i].PrevTrigFR() << "\n"
        << ", TACIDR: " << (int)h_BTL_digi_SoA->view()[i].TACIDR() << "\n"
        << ", IdleTimeL: " << h_BTL_digi_SoA->view()[i].IdleTimeL() << "\n"
        << ", PrevTrigFL: " << (int)h_BTL_digi_SoA->view()[i].PrevTrigFL() << "\n"
        << ", TACIDL: " << (int)h_BTL_digi_SoA->view()[i].TACIDL() << std::endl;

    }
    // for (const auto& dataFrame : *h_BTL_digi) {
    //   // --- detector element ID:
    //   std::cout << "\n BTL DIGI:  row = " << dataFrame.row() << " col = " << dataFrame.column()
    //             << " BTLDetId = " << dataFrame.id();

    //   // --- loop over the dataFrame samples
    //   for (int isample = 0; isample < dataFrame.size(); ++isample) {
    //     const auto& sample = dataFrame.sample(isample);

    //     std::cout << "       sample " << isample << ":";
    //     if (sample.data() == 0 && sample.toa() == 0) {
    //       std::cout << std::endl;
    //       continue;
    //     }
    //     std::cout << "  amplitude = " << sample.data() << "  time1 = " << sample.toa() << "  time2 = " << sample.toa2()
    //               << " r/c = " << (uint32_t)sample.row() << " / " << (uint32_t)sample.column()
    //               << " th = " << sample.threshold() << " mode = " << sample.mode() << std::endl;

    //   }  // isaple loop

    // }  // digi loop

  }  // if ( h_BTL_digi->size() > 0 )

  // --- ETL DIGIs:

  if (!h_ETL_digi->empty()) {
    std::cout << "\n ----------------------------------------" << std::endl;
    std::cout << " ETL DIGI collection: \n" << std::endl;

    for (const auto& dataFrame : *h_ETL_digi) {
      // --- detector element ID:
      std::cout << "\n ETL DIGI:  row = " << dataFrame.row() << " col = " << dataFrame.column()
                << " ETLDetId = " << dataFrame.id();

      // --- loop over the dataFrame samples
      for (int isample = 0; isample < dataFrame.size(); ++isample) {
        const auto& sample = dataFrame.sample(isample);

        std::cout << "       sample " << isample << ":";
        if (sample.data() == 0 && sample.toa() == 0) {
          std::cout << std::endl;
          continue;
        }
        std::cout << "  amplitude = " << sample.data() << "  time = " << sample.toa() << " r/c = " << sample.row()
                  << " / " << sample.column() << " th = " << sample.threshold() << " mode = " << sample.mode()
                  << std::endl;

      }  // isample loop

    }  // digi loop

  }  // if ( h_ETL_digi->size() > 0 )
}

// ------------ method called once each job just before starting event loop  ------------
void MTDDigiDumpSoA::beginJob() {}

// ------------ method called once each job just after ending the event loop  ------------
void MTDDigiDumpSoA::endJob() {}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void MTDDigiDumpSoA::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(MTDDigiDumpSoA);
