import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")

# ----------------------------------------------------------------------
# Create one dummy event.
# We only need an event so that the EDAnalyzer gets called.
# ----------------------------------------------------------------------

process.source = cms.Source(
    "EmptySource",
    firstRun = cms.untracked.uint32(1)
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

# ----------------------------------------------------------------------
# Give BTLTDCCalibrationRcd a valid IOV.
#
# Our Record is an independent EventSetupRecordImplementation, therefore
# something has to define its interval of validity.
# ----------------------------------------------------------------------

process.btlTDCCalibrationRcd = cms.ESSource(
    "EmptyESSource",
    recordName = cms.string("BTLTDCCalibrationRcd"),
    firstValid = cms.vuint32(1),
    iovIsRunNotTime = cms.bool(True)
)

# ----------------------------------------------------------------------
# Dummy TDC calibration producer
# ----------------------------------------------------------------------

process.btlTDCCalibrationESProducer = cms.ESProducer(
    "BTLTDCCalibrationESProducer",

    t0 = cms.double(-0.048343264),
    a0 = cms.double(57.244545),
    a1 = cms.double(511.27832),
    a2 = cms.double(-7.8838577)
)

# ----------------------------------------------------------------------
# Analyzer that reads the calibration from EventSetup
# ----------------------------------------------------------------------

process.btlTDCCalibrationTest = cms.EDAnalyzer(
    "BTLTDCCalibrationTest"
)

process.p = cms.Path(
    process.btlTDCCalibrationTest
)