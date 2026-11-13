import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing("analysis")
options.register("inputDB", "btlCalibration.db", VarParsing.multiplicity.singleton, VarParsing.varType.string, "SQLite input")
options.register("firstRun", 1, VarParsing.multiplicity.singleton, VarParsing.varType.int, "Run to read")
options.register("rawId", -2, VarParsing.multiplicity.singleton, VarParsing.varType.int, "Electronics raw ID (-1: all channels; default: channel 0)")
options.register("denseIndex", -1, VarParsing.multiplicity.singleton, VarParsing.varType.int, "Dense channel index 0..331775 (alternative to rawId)")
options.register("tac", 0, VarParsing.multiplicity.singleton, VarParsing.varType.int, "TAC 0..7 (-1: all TACs)")
options.register("tdcTag", "BTLTDCCalibration_v1", VarParsing.multiplicity.singleton, VarParsing.varType.string, "TDC tag")
options.register("qdcTag", "BTLQDCCalibration_v1", VarParsing.multiplicity.singleton, VarParsing.varType.string, "QDC tag")
options.parseArguments()

process = cms.Process("READBTLCALIBRATION")
process.source = cms.Source("EmptySource", firstRun=cms.untracked.uint32(options.firstRun))
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))
process.load("CondCore.CondDB.CondDB_cfi")
process.CondDB.connect = "sqlite_file:" + options.inputDB
process.calibrationDB = cms.ESSource(
    "PoolDBESSource", process.CondDB,
    toGet=cms.VPSet(
        cms.PSet(record=cms.string("BTLTDCCalibrationRcd"), tag=cms.string(options.tdcTag)),
        cms.PSet(record=cms.string("BTLQDCCalibrationRcd"), tag=cms.string(options.qdcTag)),
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
process.dump = cms.EDAnalyzer(
    "BTLCalibrationReader",
    rawId=cms.int32(options.rawId), denseIndex=cms.int32(options.denseIndex), tac=cms.int32(options.tac),
)
process.p = cms.Path(process.dump + process.get)
