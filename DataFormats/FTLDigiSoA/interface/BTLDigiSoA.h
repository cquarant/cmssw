#ifndef DataFormats_FTLDigiSoA_interface_BTLDigiSoA_h
#define DataFormats_FTLDigiSoA_interface_BTLDigiSoA_h

#include "DataFormats/SoATemplate/interface/SoALayout.h"

namespace btldigi {
    GENERATE_SOA_LAYOUT(BTLDigiSoALayout,
                        SOA_COLUMN(uint32_t, rawId), // Raw ID of the module/TOFHIR
                        SOA_COLUMN(uint16_t, flag),  // reserved+status+channel id
                        SOA_COLUMN(uint32_t, BCcount), 
                        SOA_COLUMN(uint32_t, data_0), // essentially a T1 and T2 coarse
                        SOA_COLUMN(uint64_t, data_1)  // rest of TOFHIR Data word
    )

    using BTLDigiSoA = BTLDigiSoALayout<>;
    using BTLDigiSoAView = BTLDigiSoA::View;
    using BTLDigiSoAConstView = BTLDigiSoA::ConstView;
} // namespace btldigi
#endif  // DataFormats_FTLDigi_interface_BTLDigiSoA_h