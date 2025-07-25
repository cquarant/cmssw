import FWCore.ParameterSet.Config as cms

from RecoLocalFastTime.FTLRecProducers.MTDTimeCalibESProducer_cfi import *

from SimFastTiming.FastTimingCommon.mtdDigitizer_cfi import mtdDigitizer
MTDTimeCalibESProducer.BTLLightCollSlope = mtdDigitizer.barrelDigitizer.DeviceSimulation.LightCollectionSlope

# the following numbers are obtained on single pions 0.7-10 GeV noPU
# to have backpropagated time average at 0
MTDTimeCalibESProducer.BTLTimeOffset = cms.double(0.0115)
MTDTimeCalibESProducer.ETLTimeOffset = cms.double(0.0066)
