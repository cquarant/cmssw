import FWCore.ParameterSet.Config as cms
import Configuration.Geometry.defaultPhase2ConditionsEra_cff as _settings
import Geometry.MTDCommonData.defaultMTDConditionsEra_cff as _mtdgeo
_mtdgeo.check_mtdgeo()
_PH2_GLOBAL_TAG, _PH2_ERA = _settings.get_era_and_conditions(_mtdgeo.MTD_DEFAULT_VERSION)
from Configuration.ProcessModifiers.dd4hep_cff import dd4hep

_mtdgeo.check_mtdgeo()
_, _era = _settings.get_era_and_conditions(_mtdgeo.MTD_DEFAULT_VERSION)
process = cms.Process("TESTBTLTDC", _era, dd4hep)
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Geometry.MTDNumberingBuilder.mtdTopology_cfi")
process.load("Geometry.MTDCommonData.GeometryDD4hepExtendedRun4MTDDefaultReco_cff")

process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))
process.btlReadoutMap = cms.ESProducer("BTLReadoutMapESProducer")
process.btlTdcTrivial = cms.ESSource(
    "MTDTrivialConditionsRetriever",
    btltdc_t0=cms.untracked.double(-0.048343264),
    btltdc_a0=cms.untracked.double(57.244545),
    btltdc_a1=cms.untracked.double(511.27832),
    btltdc_a2=cms.untracked.double(-7.8838577),
)
process.check = cms.EDAnalyzer(
    "EventSetupRecordDataGetter",
    toGet=cms.VPSet(
        cms.PSet(record=cms.string("BTLTDCCalibsRcd"), data=cms.vstring("BTLTDCCalibs")),
        cms.PSet(record=cms.string("BTLTDChannelCCalibsRcd"), data=cms.vstring("BTLTDChannelCCalibs")),
    ),
    verbose=cms.untracked.bool(True),
)
process.p = cms.Path(process.check)
