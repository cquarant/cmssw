import FWCore.ParameterSet.Config as cms
import Configuration.Geometry.defaultPhase2ConditionsEra_cff as _settings
import Geometry.MTDCommonData.defaultMTDConditionsEra_cff as _mtdgeo
_mtdgeo.check_mtdgeo()
_PH2_GLOBAL_TAG, _PH2_ERA = _settings.get_era_and_conditions(_mtdgeo.MTD_DEFAULT_VERSION)
from Configuration.ProcessModifiers.dd4hep_cff import dd4hep


process = cms.Process("BTLTDC",_PH2_ERA, dd4hep)
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

process.load("Geometry.MTDNumberingBuilder.mtdTopology_cfi")
process.load('Geometry.MTDCommonData.GeometryDD4hepExtendedRun4MTDDefaultReco_cff')

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
