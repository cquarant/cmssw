import FWCore.ParameterSet.Config as cms

btlCalibrationWriter = cms.EDAnalyzer("BTLCalibrationWriter", fromSoA=cms.bool(False))
