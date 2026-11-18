import FWCore.ParameterSet.Config as cms
from HeterogeneousCore.AlpakaCore.ProcessAcceleratorAlpaka_cfi import ProcessAcceleratorAlpaka

btlTDCCalibrationSoA = cms.ESProducer("BTLTDCCalibrationSoAESProducer@alpaka")
btlQDCCalibrationSoA = cms.ESProducer("BTLQDCCalibrationSoAESProducer@alpaka")
