import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing("analysis")
options.register("inputDB", "btlCalibration.db", VarParsing.multiplicity.singleton, VarParsing.varType.string, "SQLite input")
options.register("firstRun", 1, VarParsing.multiplicity.singleton, VarParsing.varType.int, "Run to read")
options.parseArguments()

process = cms.Process("READBTLCALIBRATION")
process.source = cms.Source("EmptySource", firstRun=cms.untracked.uint32(options.firstRun))
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))
process.load("CondCore.CondDB.CondDB_cfi")
process.CondDB.connect = "sqlite_file:" + options.inputDB
process.calibrationDB = cms.ESSource(
    "PoolDBESSource", process.CondDB,
    toGet=cms.VPSet(
        cms.PSet(record=cms.string("BTLTDCCalibrationRcd"), tag=cms.string("BTLTDCCalibration_v1")),
        cms.PSet(record=cms.string("BTLQDCCalibrationRcd"), tag=cms.string("BTLQDCCalibration_v1")),
    ),
)
process.load("CalibMTD.MTDTrivialCondChannels.btlCalibrationSoA_cff")
process.get = cms.EDAnalyzer(
    "EventSetupRecordDataGetter",
    toGet=cms.VPSet(
        cms.PSet(record=cms.string("BTLTDCCalibrationSoARcd"), data=cms.vstring("btl::BTLTDCCalibrationHost")),
        cms.PSet(record=cms.string("BTLQDCCalibrationSoARcd"), data=cms.vstring("btl::BTLQDCCalibrationHost")),
    ),
    verbose=cms.untracked.bool(True),
)
process.p = cms.Path(process.get)
