import FWCore.ParameterSet.Config as cms
import Configuration.Geometry.defaultPhase2ConditionsEra_cff as _settings
import Geometry.MTDCommonData.defaultMTDConditionsEra_cff as _mtdgeo
_mtdgeo.check_mtdgeo()
from Configuration.ProcessModifiers.dd4hep_cff import dd4hep

_, _era = _settings.get_era_and_conditions(_mtdgeo.MTD_DEFAULT_VERSION)
process = cms.Process("TESTBTLSOA", _era, dd4hep)
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Geometry.MTDNumberingBuilder.mtdTopology_cfi")
process.load("Geometry.MTDCommonData.GeometryDD4hepExtendedRun4MTDDefaultReco_cff")

process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))
process.btlReadoutMap = cms.ESProducer("BTLReadoutMapESProducer")
process.load("CalibMTD.MTDTrivialCondChannels.btlTrivialCalibrationSource_cfi")
# Check the full map and every coefficient at its hardware dense index.
_coefficients = ["t0", "a0", "a1", "a2"] + [f"p{i}" for i in range(10)]
process.check = cms.EDAnalyzer(
    "BTLCalibrationSoACheck",
    expected=cms.vdouble(*[getattr(process.btlTrivialCalibrationSource, name).value() for name in _coefficients]),
    complete=cms.bool(True),
)
process.p = cms.Path(process.check)
