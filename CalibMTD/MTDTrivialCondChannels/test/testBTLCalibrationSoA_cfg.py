import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")
process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(2))
# A sparse map tests channel ordering without requiring detector geometry.
# The map record depends on geometry/topology records; only their IOVs are
# needed by this synthetic map producer.
process.geometryIOV = cms.ESSource(
    "EmptyESSource", recordName=cms.string("MTDDigiGeometryRecord"),
    firstValid=cms.vuint32(1), iovIsRunNotTime=cms.bool(True),
)
process.topologyIOV = cms.ESSource(
    "EmptyESSource", recordName=cms.string("MTDTopologyRcd"),
    firstValid=cms.vuint32(1), iovIsRunNotTime=cms.bool(True),
)
process.map = cms.ESProducer("BTLCalibrationTestMap")
process.load("CalibMTD.MTDTrivialCondChannels.btlTrivialCalibrationSource_cfi")
process.btlTrivialCalibrationSource.t0 = 1.0
process.btlTrivialCalibrationSource.a0 = 2.0
process.btlTrivialCalibrationSource.a1 = 3.0
process.btlTrivialCalibrationSource.a2 = 4.0
process.btlTrivialCalibrationSource.p0 = 5.0
process.btlTrivialCalibrationSource.p1 = 6.0
process.btlTrivialCalibrationSource.p2 = 7.0
process.btlTrivialCalibrationSource.p3 = 8.0
process.btlTrivialCalibrationSource.p4 = 9.0
process.btlTrivialCalibrationSource.p5 = 10.0
process.btlTrivialCalibrationSource.p6 = 11.0
process.btlTrivialCalibrationSource.p7 = 12.0
process.btlTrivialCalibrationSource.p8 = 13.0
process.btlTrivialCalibrationSource.p9 = 14.0
process.check = cms.EDAnalyzer("BTLCalibrationSoACheck", expected=cms.vdouble(*range(1, 15)), complete=cms.bool(False))
process.p = cms.Path(process.check)
