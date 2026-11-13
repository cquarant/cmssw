#ifndef CondFormats_DataRecord_BTLTDCCalibrationSoARcd_h
#define CondFormats_DataRecord_BTLTDCCalibrationSoARcd_h

#include "CondFormats/DataRecord/interface/BTLTDCCalibrationRcd.h"
#include "FWCore/Framework/interface/DependentRecordImplementation.h"
#include "FWCore/Utilities/interface/mplVector.h"

class BTLTDCCalibrationSoARcd
    : public edm::eventsetup::DependentRecordImplementation<BTLTDCCalibrationSoARcd,
                                                            edm::mpl::Vector<BTLTDCCalibrationRcd>> {};

#endif
