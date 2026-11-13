# BTL calibration SoAs

`btl::BTLTDCCalibrationSoA` and `btl::BTLQDCCalibrationSoA` are separate
channel-level conditions layouts. Each has a `PortableHostCollection` alias
and an Alpaka `PortableCollection` alias, with EventSetup type registrations.
They follow the layout/host/device split used by HGCal and ECAL. They do not
merge the TDC and QDC IOVs or replace the existing map-based payloads.

## Channel and TAC addressing

One row corresponds to **one electronics channel**, including a single readout
side of a crystal. Every calibration coefficient is an Eigen vector of eight
floats, addressed by the TAC ID. `SOA_EIGEN_COLUMN` stores a component across
channels contiguously, as in `HGCalCalibParamSoA`.

The row index is `BTLReadoutMap::denseChannelIndex(electronicsId)`. After filling
a readout map, call `initialize()` before publishing it through EventSetup.
`channelRawIds()` contains the sorted electronics IDs; its size is the number
of rows to allocate in **both** collections. Populate each row's `rawId` with
the corresponding entry. A missing channel throws instead of returning row 0.
Adding channels or clearing the map invalidates the index until initialization.

This is the rank of each raw ID among the mapped channels, not the packed raw
ID itself. It is independent of insertion order and contains no holes, including
for partial detector maps. The current branch uses HS links 4..75 and a
seven-bit HS field, whereas the reference branch uses HS links 0..35, different
FED partitioning and a six-bit field. Ranking the actual map preserves existing
serialized electronics IDs. It deliberately does not import the incompatible
arithmetic formula from that branch.

The lookup is a host operation (`lower_bound`, O(log N)). Resolve it before a
kernel and pass the dense index to the device, or provide a device readout-map
lookup in the raw-to-digi stage. A kernel uses `view[channelIndex]` directly;
it must not call `BTLReadoutMap`. Indices are only meaningful for the matching
readout-map IOV. The new calibration records depend on `BTLReadoutMapRcd` so
that a map change invalidates derived calibration products. An ESProducer must
also propagate the calibration source IOVs into its output record.

`valid` is a per-TAC bit mask, independent for TDC and QDC: bit `t` is set only
when TAC `t` has a calibration. Allocation does not initialize the collection;
producers should call `zeroInitialise()` and then fill coefficients, raw IDs,
and valid bits explicitly. Zero coefficients are not usable dummy calibrations.

```cpp
map.initialize();  // while building the map, before publishing it
btl::BTLTDCCalibrationHost tdc(cms::alpakatools::host(), map.channelRawIds().size());
tdc.zeroInitialise();
for (uint32_t i = 0; i < map.channelRawIds().size(); ++i) {
  tdc.view()[i].rawId() = map.channelRawIds()[i];
}
// For a measured coefficient set, with 0 <= tac < 8:
const auto i = map.denseChannelIndex(electronicsId);
tdc.view()[i].t0()(tac) = measuredT0;
tdc.view()[i].a0()(tac) = measuredA0;
tdc.view()[i].a1()(tac) = measuredA1;
tdc.view()[i].a2()(tac) = measuredA2;
tdc.view()[i].valid() |= uint8_t(1u << tac);
```

## Parameter definitions and units

TDC fields are `t0`, `a0`, `a1`, `a2`, matching the electronics simulation:

```
q = coarse - time / clockPeriod - t0
fineADC = a0 + a1*q + a2*q*q
```

`t0` and `q` are in clock periods. Coefficients are in ADC counts per
corresponding power of `q`. Reconstruction inverts the quadratic on its physical
branch, then computes `time = (coarse - t0 - q) * clockPeriod`. The output is in
seconds if the clock period is in seconds (the reference uses 6.25 ns).
The layout stores the response parameters; it does not implement an inverse,
clock rollover handling, time walk, or an absolute time offset. It currently
provides one response per TAC and channel, shared by T1 and T2 as in the dummy
model. Independent responses for the two discriminators would require an
additional dimension or separate coefficients once that calibration is defined.

QDC fields are `p0` through `p9`, including the currently zero higher orders:

```
ti = EOIcoarse - time / clockPeriod
pedestalADC = p0 + p1*ti + ... + p9*ti^9
correctedADC = Qfine - pedestalADC
```

`ti` is in clock periods and the pedestal is in ADC counts. These coefficients
remove an integration-time-dependent pedestal; they do not establish the
ADC-to-energy scale. QDC is stored per TAC and channel as for the existing TDC
calibration. Measurements common to TACs can be replicated by a producer.
No hard-coded dummy coefficients are built into either layout.

## EventSetup, persistence, and consumers

The records are `BTLTDCCalibrationRcd` and `BTLQDCCalibrationRcd`. Host/device
product types can be used by ESProducers, with the usual Alpaka host-to-device
copy support. Type registration does not itself produce a collection or copy
it to the GPU.

These are in-memory conditions products, like the referenced HGCal layout.
`PortableHostCollection` is not automatically a Boost-serializable CondDB
payload. No DB dictionary or `COND_SERIALIZABLE` annotation is claimed for it.
A DB-backed integration needs either dedicated serializable source payloads
and an ESProducer that fills these SoAs, or a separately validated serialization
adapter. The existing persistent payloads and their readers remain available.
`BTLTrivialCalibrationSource` in `CalibMTD/MTDTrivialCondChannels` produces both
host collections directly, without a CondObjectContainer or detector-geometry
loop. Each coefficient is configurable and repeated across all mapped channels
and eight TACs; every TAC is marked valid. The default coefficients match the
referenced electronics simulation. TDC and QDC have separate records and
consume the same readout map. No DB writer or simulation consumer is supplied.

## Uniform calibration source

In a job with the MTD geometry and topology loaded:

```python
process.btlReadoutMap = cms.ESProducer("BTLReadoutMapESProducer")
process.load("CalibMTD.MTDTrivialCondChannels.btlTrivialCalibrationSource_cfi")
process.btlTrivialCalibrationSource.a1 = 511.27832
process.btlTrivialCalibrationSource.p0 = 49.542229
```

All parameters are tracked doubles (`t0`, `a0`..`a2`, `p0`..`p9`), stored as
floats. Nonfinite values, including overflow on conversion to float, are
rejected. `BTLReadoutMapESProducer` initializes the dense index before publishing
the map. DB-loaded maps must likewise have their transient index initialized.

Run a geometry-independent framework test with a sparse electronics map:

```sh
cmsRun CalibMTD/MTDTrivialCondChannels/test/testBTLCalibrationSoA_cfg.py
```

For the full geometry, use
`CalibMTD/MTDTrivialCondChannels/test/testBTLCalibrationSoAGeometry_cfg.py`.
These examples only request EventSetup products; they do not write SQLite files.

## Validation

`testBTLCalibrationSoA` checks ordering, dense indices for sparse channel sets,
insertion-order independence, missing channels, cache invalidation, rejected
duplicate insertions, and independent SoA storage across channels and TACs.

Verified in a temporary `CMSSW_20_1_0_pre5` area because the original nightly
release is no longer available on CVMFS: package builds for `MTDObjects` and
`DataRecord` (including CUDA and ROCm type registrations), dictionary version
and transient checks, and execution of `testBTLCalibrationSoA` all passed.
GPU kernels were not executed.

`testBTLTrivialCalibrationSoA` requests both products through EventSetup over two
events and checks all 14 configured coefficients, raw IDs and validity bits
for every sparse-map channel and TAC. The source/plugin builds and sparse-map
framework test passed in `CMSSW_20_1_0_pre5` (`el9_amd64_gcc14`); `cmsRun` had to
run outside the filesystem sandbox because it crashed during startup inside it.
The full-geometry example also passed, requesting both host products using
the real `BTLReadoutMapESProducer` (Run4D127 geometry in this release).

## References

- [HGCal calibration layout](https://github.com/cquarant/cmssw/blob/cq_btlCondContainer_v1-2/CondFormats/HGCalObjects/interface/HGCalCalibParamSoA.h)
- [BTL digi layout](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/DataFormats/FTLDigiSoA/interface/BTLDigiSoA.h)
- [Electronics coefficients](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/SimFastTiming/FastTimingCommon/interface/BTLElectronicsSim.h)
- [Forward conversion formulas](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/SimFastTiming/FastTimingCommon/src/BTLElectronicsSim.cc)
- [Reference dense index](https://github.com/martinamalberti/cmssw/blob/mm_BTLReadoutMap_20261029/CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h)
