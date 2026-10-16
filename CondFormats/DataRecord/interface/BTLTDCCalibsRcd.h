#ifndef CondFormats_DataRecord_BTLTDCCalibsRcd_h
#define CondFormats_DataRecord_BTLTDCCalibsRcd_h

#include "FWCore/Framework/interface/EventSetupRecordImplementation.h"
#include "Geometry/Records/interface/MTDGeometryRecord.h"
#include "Geometry/Records/interface/MTDTopologyRcd.h"

class BTLTDCCalibsRcd
    : public edm::eventsetup::DependentRecordImplementation<BTLTDCCalibsRcd, edm::mpl::Vector<MTDDigiGeometryRecord, MTDTopologyRcd> > {};
#endif
