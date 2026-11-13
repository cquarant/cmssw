#ifndef CondFormats_DataRecord_BTLQDCCalibrationSoARcd_h
#define CondFormats_DataRecord_BTLQDCCalibrationSoARcd_h

#include "CondFormats/DataRecord/interface/BTLQDCCalibrationRcd.h"
#include "FWCore/Framework/interface/DependentRecordImplementation.h"
#include "FWCore/Utilities/interface/mplVector.h"

class BTLQDCCalibrationSoARcd
    : public edm::eventsetup::DependentRecordImplementation<BTLQDCCalibrationSoARcd,
                                                            edm::mpl::Vector<BTLQDCCalibrationRcd>> {};

#endif
