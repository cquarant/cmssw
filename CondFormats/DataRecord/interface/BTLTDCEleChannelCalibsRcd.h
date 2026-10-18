#ifndef CondFormats_DataRecord_BTLTDCEleChannelCalibsRcd_h
#define CondFormats_DataRecord_BTLTDCEleChannelCalibsRcd_h

#include "CondFormats/DataRecord/interface/BTLReadoutMapRcd.h"
#include "FWCore/Framework/interface/DependentRecordImplementation.h"
#include "FWCore/Utilities/interface/mplVector.h"
#include "Geometry/Records/interface/MTDGeometryRecord.h"
#include "Geometry/Records/interface/MTDTopologyRcd.h"

class BTLTDCEleChannelCalibsRcd
    : public edm::eventsetup::DependentRecordImplementation<
          BTLTDCEleChannelCalibsRcd,
          edm::mpl::Vector<MTDDigiGeometryRecord, MTDTopologyRcd, BTLReadoutMapRcd> > {};
#endif
