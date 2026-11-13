#ifndef CondFormats_DataRecord_BTLQDCCalibrationRcd_h
#define CondFormats_DataRecord_BTLQDCCalibrationRcd_h

#include "CondFormats/DataRecord/interface/BTLReadoutMapRcd.h"
#include "FWCore/Framework/interface/DependentRecordImplementation.h"
#include "FWCore/Utilities/interface/mplVector.h"

// Row ordering depends on the readout-map IOV.
class BTLQDCCalibrationRcd
    : public edm::eventsetup::DependentRecordImplementation<BTLQDCCalibrationRcd, edm::mpl::Vector<BTLReadoutMapRcd>> {
};

#endif
