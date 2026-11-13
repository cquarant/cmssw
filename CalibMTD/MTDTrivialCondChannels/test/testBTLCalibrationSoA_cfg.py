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
process.load("CalibMTD.MTDTrivialCondChannels.btlCalibrationSoA_cff")
process.btlTrivialCalibrationSource.channelIds = cms.vuint32(0, 20, (143 << 16) | (35 << 10) | (23 << 5), (143 << 16) | (35 << 10) | (23 << 5) | 31)
process.btlTrivialCalibrationSource.t0_T1 = 1.0
process.btlTrivialCalibrationSource.a0_T1 = 2.0
process.btlTrivialCalibrationSource.a1_T1 = 3.0
process.btlTrivialCalibrationSource.a2_T1 = 4.0
process.btlTrivialCalibrationSource.t0_T2 = 5.0
process.btlTrivialCalibrationSource.a0_T2 = 6.0
process.btlTrivialCalibrationSource.a1_T2 = 7.0
process.btlTrivialCalibrationSource.a2_T2 = 8.0
process.btlTrivialCalibrationSource.p0 = 9.0
process.btlTrivialCalibrationSource.p1 = 10.0
process.btlTrivialCalibrationSource.p2 = 11.0
process.btlTrivialCalibrationSource.p3 = 12.0
process.btlTrivialCalibrationSource.p4 = 13.0
process.btlTrivialCalibrationSource.p5 = 14.0
process.btlTrivialCalibrationSource.p6 = 15.0
process.btlTrivialCalibrationSource.p7 = 16.0
process.btlTrivialCalibrationSource.p8 = 17.0
process.btlTrivialCalibrationSource.p9 = 18.0
process.check = cms.EDAnalyzer("BTLCalibrationSoACheck", expected=cms.vdouble(*range(1, 19)), complete=cms.bool(False))
process.p = cms.Path(process.check)
