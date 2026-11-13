import FWCore.ParameterSet.Config as cms

btlTrivialCalibrationSource = cms.ESSource(
    "BTLTrivialCalibrationSource",
    channelIds=cms.vuint32(),  # Empty means the full hardware range.
    t0=cms.double(-0.048343264),
    a0=cms.double(57.244545),
    a1=cms.double(511.27832),
    a2=cms.double(-7.8838577),
    p0=cms.double(49.542229),
    p1=cms.double(-0.323424),
    p2=cms.double(0.062578),
    p3=cms.double(-0.002484),
    p4=cms.double(0.0),
    p5=cms.double(0.0),
    p6=cms.double(0.0),
    p7=cms.double(0.0),
    p8=cms.double(0.0),
    p9=cms.double(0.0),
)
