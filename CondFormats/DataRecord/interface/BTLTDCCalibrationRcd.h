#ifndef CondFormats_DataRecord_BTLTDCCalibrationRcd_h
#define CondFormats_DataRecord_BTLTDCCalibrationRcd_h

#include "CondFormats/DataRecord/interface/BTLReadoutMapRcd.h"
#include "FWCore/Framework/interface/DependentRecordImplementation.h"
#include "FWCore/Utilities/interface/mplVector.h"

// Row ordering depends on the readout-map IOV.
class BTLTDCCalibrationRcd
    : public edm::eventsetup::DependentRecordImplementation<BTLTDCCalibrationRcd, edm::mpl::Vector<BTLReadoutMapRcd>> {
};

#endif
