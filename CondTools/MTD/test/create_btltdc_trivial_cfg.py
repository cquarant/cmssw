import FWCore.ParameterSet.Config as cms

process = cms.Process("BTLTDC")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = cms.untracked.string("INFO")
process.MessageLogger.cerr.INFO.limit = cms.untracked.int32(-1)
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))

# No collision events needed: the payload is constant across all runs.
process.source = cms.Source(
    "EmptyIOVSource",
    firstValue=cms.uint64(1),
    lastValue=cms.uint64(1),
    timetype=cms.string("runnumber"),
    interval=cms.uint64(1),
)

# Provide the dummy payload through EventSetup.
process.btlTdcTrivial = cms.ESSource(
    "MTDTrivialConditionsRetriever",
)

process.btlTdcTrivial.btltdc_t0 = cms.untracked.double(-0.048343264)
process.btlTdcTrivial.btltdc_a0 = cms.untracked.double(57.244545)
process.btlTdcTrivial.btltdc_a1 = cms.untracked.double(511.27832)
process.btlTdcTrivial.btltdc_a2 = cms.untracked.double(-7.8838577)

# Write the produced EventSetup payload to a SQLite conditions database.
process.load("CondCore.CondDB.CondDB_cfi")
process.CondDB.connect = cms.string("sqlite_file:BTLTDCCalibs_trivial.db")
process.PoolDBOutputService = cms.Service(
    "PoolDBOutputService",
    process.CondDB,
    timetype=cms.untracked.string("runnumber"),
    toPut=cms.VPSet(
        cms.PSet(
            record=cms.string("BTLTDCCalibsRcd"),
            tag=cms.string("BTLTDCCalibs_trivial_v1"),
        )
    ),
)

process.write = cms.EDAnalyzer("BTLTDCCalibsWriter")
process.p = cms.Path(process.write)
