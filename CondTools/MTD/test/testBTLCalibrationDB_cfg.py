import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing("analysis")
options.register("read", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool, "Read DB")
options.register("fromSoA", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool, "Write converted SoAs")
options.parseArguments()
process = cms.Process("TESTBTLDB")
process.source = cms.Source("EmptySource", numberEventsInRun=cms.untracked.uint32(2))
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(6))
process.load("CondCore.CondDB.CondDB_cfi")
process.CondDB.connect = "sqlite_file:btl.db"
records = cms.VPSet(
    cms.PSet(record=cms.string("BTLTDCCalibrationRcd"), tag=cms.string("tdc")),
    cms.PSet(record=cms.string("BTLQDCCalibrationRcd"), tag=cms.string("qdc")),
)
process.load("CalibMTD.MTDTrivialCondChannels.btlCalibrationSoA_cff")
process.check = cms.EDAnalyzer("BTLCalibrationIOVCheck")
if options.read:
    process.db = cms.ESSource("PoolDBESSource", process.CondDB, toGet=records)
    process.p = cms.Path(process.check)
else:
    process.calibration = cms.ESSource("BTLCalibrationIOVTestSource")
    process.PoolDBOutputService = cms.Service(
        "PoolDBOutputService", process.CondDB,
        timetype=cms.untracked.string("runnumber"), toPut=records,
    )
    process.write = cms.EDAnalyzer("BTLCalibrationWriter", fromSoA=cms.bool(options.fromSoA))
    process.p = cms.Path(process.check + process.write)
