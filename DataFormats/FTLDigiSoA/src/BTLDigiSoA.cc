#include <ostream>

#include <fmt/format.h>

#include "DataFormats/FTLDigiSoA/interface/BTLDigiSoA.h"

namespace btldigi {

  // std::ostream& operator<<(std::ostream& out, BTLDigiSoA::View::const_element const& digi);
  std::ostream& operator<<(std::ostream& out, BTLDigiSoA::View::const_element const& digi) {
    out << ", rawId : " << digi.rawId()
        // << ", BC0count: " << digi.BC0count()
        // << ", status: " << digi.status()
        // << ", BCcount: " << digi.BCcount()
        << ", chIDR: " << (int)digi.chIDR()
        << ", T1coarseR: " << digi.T1coarseR()
        << ", T1fineR: " << digi.T1fineR()
        << ", T2coarseR: " << digi.T2coarseR()
        << ", T2fineR: " << digi.T2fineR()
        << ", ChargeR: " << digi.ChargeR()
        << ", chIDL: " << (int)digi.chIDL()
        << ", T1coarseL: " << digi.T1coarseL()
        << ", T1fineL: " << digi.T1fineL()
        << ", T2coarseL: " << digi.T2coarseL()
        << ", T2fineL: " << digi.T2fineL()
        << ", ChargeL: " << digi.ChargeL();
        // << ", IdleTimeR: " << digi.IdleTimeR()
        // << ", PrevTrigFR: " << (int)digi.PrevTrigFR()
        // << ", TACIDR: " << (int)digi.TACIDR()
        // << ", IdleTimeL: " << digi.IdleTimeL()
        // << ", PrevTrigFL: " << (int)digi.PrevTrigFL()
        // << ", TACIDL: " << (int)digi.TACIDL();
    return out;
  }

  // std::ostream& operator<<(std::ostream& out, BTLDigiSoALayout<128ul, false>::ViewTemplateFreeParams<128ul, false, true, true>::element const& digi) {
  //       out << ", rawId : " << digi.rawId()
  //           << ", chIDR: " << (int)digi.chIDR()
  //           << ", T1coarseR: " << digi.T1coarseR()
  //           << ", T1fineR: " << digi.T1fineR()
  //           << ", T2coarseR: " << digi.T2coarseR()
  //           << ", T2fineR: " << digi.T2fineR()
  //           << ", ChargeR: " << digi.ChargeR()
  //           << ", chIDL: " << (int)digi.chIDL()
  //           << ", T1coarseL: " << digi.T1coarseL()
  //           << ", T1fineL: " << digi.T1fineL()
  //           << ", T2coarseL: " << digi.T2coarseL()
  //           << ", T2fineL: " << digi.T2fineL()
  //           << ", ChargeL: " << digi.ChargeL();
  //       return out;
  //   }
}  // namespace btldigi