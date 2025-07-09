#ifndef DataFormats_FTLDigiSoA_interface_BTLDigiSoA_h
#define DataFormats_FTLDigiSoA_interface_BTLDigiSoA_h

#include "DataFormats/SoATemplate/interface/SoALayout.h"

namespace btldigi {
    GENERATE_SOA_LAYOUT(BTLDigiSoALayout,
                        SOA_COLUMN(uint32_t, rawId    ), // Raw ID of the module/TOFHIR
                        SOA_COLUMN(uint16_t, BC0count ),  // BC0 count (reserved)
                        SOA_COLUMN(bool    , status   ),  // status of the TOFHIR
                        SOA_COLUMN(uint8_t , chID     ), // TOFHIR channel ID
                        SOA_COLUMN(uint32_t, BCcount  ), 
                        SOA_COLUMN(uint16_t, T1coarse ),
                        SOA_COLUMN(uint16_t, T2coarse ),  
                        SOA_COLUMN(uint16_t, EOIcoarse),  
                        SOA_COLUMN(uint16_t, Charge   ),  
                        SOA_COLUMN(uint16_t, T1fine   ),  
                        SOA_COLUMN(uint16_t, T2fine   ),  
                        SOA_COLUMN(uint16_t, IdleTime ),  
                        SOA_COLUMN(uint8_t , PrevTrigF),
                        SOA_COLUMN(uint8_t , TACID    )
    )

    using BTLDigiSoA = BTLDigiSoALayout<>;
    using BTLDigiSoAView = BTLDigiSoA::View;
    using BTLDigiSoAConstView = BTLDigiSoA::ConstView;
} // namespace btldigi
#endif  // DataFormats_FTLDigi_interface_BTLDigiSoA_h