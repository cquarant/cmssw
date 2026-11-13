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

The DAQ encoding and mapping follow
[`mm_BTLReadoutMap_20261029`](https://github.com/martinamalberti/cmssw/tree/mm_BTLReadoutMap_20261029):
HS-links 0..35 (six-bit field), e-links 0..23, TOFHIR channels 0..31,
and 144 FEDs, with 12 FEDs per Serenity and three HS-links per FED.
The FED field starts at bit 16. The shared specifications now live in
`CondFormats/MTDObjects/interface/BTLElectronicsSpecs.h`; the former EventFilter
header forwards to this one.

The row index is **`BTLElectronicsId::denseChannelIndex()`**, using the reference
`BTLElectronicsIndexing` arithmetic:

```text
serenity = (fedId - kFirstFEDId) / nFEDsPerSerenity
index = ((serenity * 36 + hsLinkId) * 24 + eLinkId) * 32 + channelId
```

Both collections allocate `BTLElectronicsIndexing::kNumberOfChannels` = 331776
rows. The index is independent of map insertion order and map completeness;
it is not the rank of an ID in a partial map. The full detector covers
0..331775 exactly once. `BTLReadoutMap::checkDenseChannelIndex()` checks that
coverage and increasing raw IDs. `getListOfDetIds()` enumerates mapped crystals.
For partial maps, rows for missing channels remain zero with `valid == 0`.
A valid electronics ID with raw ID zero is distinguished by its validity mask.

Validate host IDs with `isValid()` before indexing: the arithmetic function
assumes a valid FED/HS-link assignment. Host code can use
`BTLElectronicsId::denseChannelIndex()` directly. The plain constexpr helper
`BTLElectronicsIndexing::denseChannelIndex(fed, hs, el, channel)` implements the
same arithmetic for code that already has the four fields. Kernels can consume
the resulting index and access `view[index]` without a map search.

`BTLReadoutMap::initialize()` rebuilds the transient reverse map; the arithmetic
index itself needs no initialization. The calibration records still depend on
`BTLReadoutMapRcd`: changing which channels are mapped invalidates the derived
collections. A future producer of measured calibrations must also propagate
its source IOVs.

The raw-ID encoding differs from the former seven-bit HS-link format. Existing
readout-map and electronics-keyed calibration payloads made with that old
encoding must be regenerated; they cannot be interpreted as the new format.

`valid` is a per-TAC bit mask, independent for TDC and QDC: bit `t` is set only
when TAC `t` has a calibration. Allocation does not initialize the collection;
producers call `zeroInitialise()` and then fill coefficients, raw IDs and
valid bits. Zero coefficients alone are not usable dummy calibrations.

```cpp
btl::BTLTDCCalibrationHost tdc(cms::alpakatools::host(),
                              BTLElectronicsIndexing::kNumberOfChannels);
tdc.zeroInitialise();
// For a validated, mapped electronicsId and 0 <= tac < 8:
const auto i = electronicsId.denseChannelIndex();
tdc.view()[i].rawId() = electronicsId.rawId();
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
and eight TACs; every mapped TAC is marked valid. Unmapped rows remain invalid. The default coefficients match the
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
rejected. `BTLReadoutMapESProducer` initializes the transient reverse map before
publishing it. DB-loaded maps likewise need `initialize()` for reverse lookups.

Run a geometry-independent framework test with a sparse electronics map:

```sh
cmsRun CalibMTD/MTDTrivialCondChannels/test/testBTLCalibrationSoA_cfg.py
```

For the full geometry, use
`CalibMTD/MTDTrivialCondChannels/test/testBTLCalibrationSoAGeometry_cfg.py`.
These examples only request EventSetup products; they do not write SQLite files.

## Validation

`testBTLCalibrationSoA` exhaustively checks all 331776 valid electronics IDs,
round-trip field decoding, consecutive dense indices and increasing raw IDs,
plus invalid FED/HS-link assignments and range boundaries. It also checks
sparse-map addressing at the first and last rows, independent storage across
channels/TACs, zero validity in holes, reverse lookups and atomic duplicate
rejection.

`testBTLTrivialCalibrationSoA` requests both products over two events with a
sparse map and checks all 14 configurable coefficients, raw IDs and validity
bits at their hardware indices. The full-geometry example additionally checks
complete coverage and every coefficient for all 331776 channels using the real
`BTLReadoutMapESProducer`.

Verified in a temporary `CMSSW_20_1_0_pre5` area (`el9_amd64_gcc14`) because
the original nightly is no longer available on CVMFS: the updated packages built
successfully (including CUDA/ROCm type registrations and dictionary checks),
the exhaustive indexing/storage test passed, and both sparse-map and
full-geometry EventSetup tests passed. The full-geometry test used Run4D127
and checked every channel at its arithmetic dense index. GPU kernels were not
executed. Framework tests ran outside the filesystem sandbox because `cmsRun`
crashes during startup inside it.

## References

- [HGCal calibration layout](https://github.com/cquarant/cmssw/blob/cq_btlCondContainer_v1-2/CondFormats/HGCalObjects/interface/HGCalCalibParamSoA.h)
- [BTL digi layout](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/DataFormats/FTLDigiSoA/interface/BTLDigiSoA.h)
- [Electronics coefficients](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/SimFastTiming/FastTimingCommon/interface/BTLElectronicsSim.h)
- [Forward conversion formulas](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/SimFastTiming/FastTimingCommon/src/BTLElectronicsSim.cc)
- [Reference dense index](https://github.com/martinamalberti/cmssw/blob/mm_BTLReadoutMap_20261029/CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h)
