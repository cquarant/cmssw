import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing("analysis")
options.register("outputDB", "btlCalibration.db", VarParsing.multiplicity.singleton, VarParsing.varType.string, "SQLite output")
options.register("fromSoA", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool, "Convert SoA host results")
options.register("firstRun", 1, VarParsing.multiplicity.singleton, VarParsing.varType.int, "First run of validity")
options.parseArguments()

process = cms.Process("WRITEBTLCALIBRATION")
process.source = cms.Source("EmptySource", firstRun=cms.untracked.uint32(options.firstRun))
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))
process.load("CalibMTD.MTDTrivialCondChannels.btlTrivialCalibrationSource_cfi")
process.load("CalibMTD.MTDTrivialCondChannels.btlCalibrationSoA_cff")
process.load("CondTools.MTD.btlCalibrationWriter_cfi")
process.btlCalibrationWriter.fromSoA = options.fromSoA
process.load("CondCore.CondDB.CondDB_cfi")
process.CondDB.connect = "sqlite_file:" + options.outputDB
process.PoolDBOutputService = cms.Service(
    "PoolDBOutputService", process.CondDB,
    timetype=cms.untracked.string("runnumber"),
    toPut=cms.VPSet(
        cms.PSet(record=cms.string("BTLTDCCalibrationRcd"), tag=cms.string("BTLTDCCalibration_v1")),
        cms.PSet(record=cms.string("BTLQDCCalibrationRcd"), tag=cms.string("BTLQDCCalibration_v1")),
    ),
)
process.p = cms.Path(process.btlCalibrationWriter)
