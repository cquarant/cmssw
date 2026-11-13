# BTL calibration SoAs

`btl::BTLTDCCalibrationSoA` and `btl::BTLQDCCalibrationSoA` are separate
channel-level conditions layouts. Each has a `PortableHostCollection` alias
and an Alpaka `PortableCollection` alias, with EventSetup type registrations.
They follow the layout/host/device split used by HGCal and ECAL. TDC and QDC
have independent persistent payloads and IOVs. The former
map-based calibration payloads and their container workflow have been removed.

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
For sparse calibration payloads, rows for missing channels remain zero with
`valid == 0`.
A valid electronics ID with raw ID zero is distinguished by its validity mask.

Validate host IDs with `isValid()` before indexing: the arithmetic function
assumes a valid FED/HS-link assignment. Host code can use
`BTLElectronicsId::denseChannelIndex()` directly. The plain constexpr helper
`BTLElectronicsIndexing::denseChannelIndex(fed, hs, el, channel)` implements the
same arithmetic for code that already has the four fields. Kernels can consume
the resulting index and access `view[index]` without a map search.

`BTLReadoutMap::initialize()` rebuilds the transient reverse map; the arithmetic
index itself needs no initialization. Payloads identify channels by electronics
ID, so converting a calibration to its dense SoA does not require the readout
map or geometry. A consumer needing crystal IDs still uses `BTLReadoutMapRcd`.
The derived TDC and QDC records depend on their respective persistent calibration
records, so source IOV changes invalidate the corresponding SoA independently.

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

## Persistent payloads, EventSetup and consumers

The persistent types are `BTLTDCCalibration` and `BTLQDCCalibration`, in separate
headers and records `BTLTDCCalibrationRcd` and `BTLQDCCalibrationRcd`. Each is an
ordinary serializable C++ payload with:

- `rawIds`: electronics IDs, one per stored channel, in arbitrary order;
- `valid`: one eight-bit TAC validity mask per stored channel;
- coefficient vectors `t0`, `a0`..`a2`, or `p0`..`p9`.

Each coefficient vector has exactly `rawIds.size()*8` floats. The value for
stored channel `i`, TAC `t`, is at `i*8+t`. Sparse payloads are supported; a raw
ID of zero is valid. `validate()` rejects malformed vector sizes, duplicate or
invalid IDs (including reserved high bits), and nonfinite coefficients. Missing
TACs should carry finite placeholder coefficients and have their valid bits
cleared. Both types have `COND_SERIALIZABLE`, dictionaries, generated Boost
serialization, EventSetup type registrations and CondDB plugin registrations.

`BTLTDCCalibrationSoAESProducer` and `BTLQDCCalibrationSoAESProducer` consume these
payloads and produce `btl::BTLTDCCalibrationHost` and
`btl::BTLQDCCalibrationHost`. Their records are **`BTLTDCCalibrationSoARcd`** and
**`BTLQDCCalibrationSoARcd`**, each dependent on its own persistent record.
Consumers of the host/device SoAs must use these derived records. Conversion
validates the input, zero-initializes the full hardware range, and copies each
stored channel to `BTLElectronicsId(rawId).denseChannelIndex()`.

Load the conversion producers with:

```python
process.load("CalibMTD.MTDTrivialCondChannels.btlCalibrationSoA_cff")
```

EventSetup caches the derived products for their IOVs; the conversion is not
performed per event. The portable host/device aliases and type registrations
remain available for Alpaka consumers. These conversion producers publish host
products; device consumers still need the usual Alpaka copy integration.
`PortableHostCollection` is not the CondDB payload: the persistent vectors and
portable memory layout are deliberately separate.

## Dummy input and measured calibration results

`BTLTrivialCalibrationSource` now produces the **persistent payloads**, not SoAs.
The configurable doubles `t0`, `a0`..`a2`, `p0`..`p9` are converted to finite
floats and replicated over all eight TACs. All selected TACs are marked valid.
An empty `channelIds` list (the default) selects all 331776 hardware channels
without geometry or a readout map. A nonempty `cms.vuint32` list selects a sparse
set of electronics raw IDs. It is independent of the mapped detector subset.

```python
process.load("CalibMTD.MTDTrivialCondChannels.btlTrivialCalibrationSource_cfi")
process.btlTrivialCalibrationSource.channelIds = cms.vuint32(0, 20)
process.btlTrivialCalibrationSource.a1 = 511.27832
process.load("CalibMTD.MTDTrivialCondChannels.btlCalibrationSoA_cff")
```

Measured calibrations can fill the public vectors in either persistent payload,
call `validate()`, and publish it in the corresponding persistent record. No
`MTDContainer` or `CondObjectContainer` is needed. Alternatively, an algorithm
can publish the host SoAs in the derived records and use the writer's SoA input
mode. Do not enable another producer of the same product in the same record.
Device results must first be copied to host with the normal queue synchronization.

The reusable conversion functions live in
`CalibMTD/MTDTrivialCondChannels/interface/BTLCalibrationConversions.h`:

```cpp
auto host = btl::makeSoA(tdcPayload);  // overloads for TDC and QDC
auto persistent = btl::makePayload(host);
```

The reverse conversion checks the collection size and that each valid row's raw
ID agrees with its dense index. It stores rows with nonzero validity masks in
dense-index order, retaining all eight TAC coefficient slots for those rows.
Rows without valid TACs are omitted. Thus the round trip preserves usable
calibrations and validity, but may reorder channels and discard all-invalid rows.

## Writing and reading CondDB

`BTLCalibrationWriter` is a separate analyzer using `PoolDBOutputService`.
With `fromSoA=False`, it validates and writes persistent inputs from the dummy
source or any measured-payload producer. With `fromSoA=True`, it constructs the
two persistent payloads from host calibration SoAs using `makePayload()`.
It writes each product on first use and when its input record changes, with
independent TDC/QDC IOVs. The output time type must be `runnumber`; the IOV starts
at the current event's run. Run the writer from the first run of each intended
IOV. Multiple different calibration IOVs within one run are not supported.

Write both dummy payloads to a new local SQLite database, then read them back
through `PoolDBESSource` and build the derived SoAs:

```sh
cmsRun CondTools/MTD/test/writeBTLCalibration_cfg.py outputDB=btlCalibration.db firstRun=1
cmsRun CondTools/MTD/test/readBTLCalibration_cfg.py inputDB=btlCalibration.db firstRun=1
```

To exercise the reverse conversion when writing:

```sh
cmsRun CondTools/MTD/test/writeBTLCalibration_cfg.py outputDB=btlFromSoA.db fromSoA=True
```

The examples use separate tags `BTLTDCCalibration_v1` and `BTLQDCCalibration_v1`.
The old `BTLTDCCalibs` / `BTLTDCEleChannelCalibs` payload types, records, writers,
dump utility and trivial retriever have been removed. Old tags are not readable
through the new payload types: migration requires reading them with the previous
code and explicitly converting their data, including any electronics encoding
changes. Their serialized representation is not compatible with these vectors.

## Validation

Run the following commands from the `src` directory of a compatible CMSSW
release containing these changes. Initialize the runtime and build first:

```sh
source /cvmfs/cms.cern.ch/cmsset_default.sh
eval "$(scram runtime -sh)"
scram b -j 8
```

Run the four registered tests (hardware indexing/SoA storage, payload/SoA
conversions, trivial-source EventSetup integration, and SQLite persistence):

```sh
scram b runtests_TESTS="testBTLCalibrationSoA testBTLCalibrationConversions testBTLTrivialCalibrationSoA testBTLCalibrationDB"
```

Alternatively, run them individually. The shell wrappers need `SCRAM_TEST_PATH`
to locate their configuration files. The database wrapper creates and cleans
its own temporary directory and tests both persistent-input and SoA-input writing:

```sh
testBTLCalibrationSoA
testBTLCalibrationConversions
cmsRun CalibMTD/MTDTrivialCondChannels/test/testBTLCalibrationSoA_cfg.py
SCRAM_TEST_PATH="$CMSSW_BASE/src/CondTools/MTD/test" bash CondTools/MTD/test/testBTLCalibrationDB.sh
```

The full-geometry check is additional to the registered tests:

```sh
cmsRun CalibMTD/MTDTrivialCondChannels/test/testBTLCalibrationSoAGeometry_cfg.py
```

The standalone SQLite writer/readback commands are listed in the preceding
section; use a new output database filename for each independent writing test.

`testBTLCalibrationSoA` exhaustively checks all 331776 valid electronics IDs,
round-trip field decoding, consecutive dense indices and increasing raw IDs,
plus invalid FED/HS-link assignments and range boundaries. It also checks
sparse-map addressing at the first and last rows, independent storage across
channels/TACs, zero validity in holes, reverse lookups and atomic duplicate
rejection.

`testBTLTrivialCalibrationSoA` requests both derived products through persistent
dummy inputs over two events with a sparse map and checks all 14 configurable coefficients, raw IDs and validity
bits at their hardware indices. The full-geometry example additionally checks
complete coverage and every coefficient for all 331776 channels using the real
`BTLReadoutMapESProducer`.

The persistence integration test `testBTLCalibrationDB` writes and reads a local
SQLite database in both writer modes. It checks every coefficient of sparse,
nonuniform test payloads after reconstruction as SoAs, including partial TAC
masks, raw ID zero and the last channel. Across three runs (two events per run),
it checks separate TDC boundaries at runs 1/3 and QDC boundaries at runs 1/2/3,
including the exact stored IOV counts. `testBTLCalibrationConversions` checks
round trips, empty payloads, reordering and malformed-input rejection.

Verified on 2026-11-13 in a temporary `CMSSW_20_1_0_pre5` area
(`el9_amd64_gcc14`): package build, generated serialization, dictionary checks,
and all four registered tests passed. The full Run4D127 geometry check also
passed, verifying all coefficients on all 331776 channels. The standalone dummy
writer/readback examples successfully stored both tags and requested both
derived SoAs from SQLite. The original nightly is no longer
available on CVMFS. Framework tests ran outside the filesystem sandbox because
`cmsRun` crashes during startup inside it. GPU kernels were not executed.

## References

- [Original vector-based TDC payload](https://github.com/cquarant/cmssw/blob/cq_btlCondFormat/CondFormats/MTDObjects/interface/BTLTDCCalibration.h)

- [HGCal calibration layout](https://github.com/cquarant/cmssw/blob/cq_btlCondContainer_v1-2/CondFormats/HGCalObjects/interface/HGCalCalibParamSoA.h)
- [BTL digi layout](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/DataFormats/FTLDigiSoA/interface/BTLDigiSoA.h)
- [Electronics coefficients](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/SimFastTiming/FastTimingCommon/interface/BTLElectronicsSim.h)
- [Forward conversion formulas](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/SimFastTiming/FastTimingCommon/src/BTLElectronicsSim.cc)
- [Reference dense index](https://github.com/martinamalberti/cmssw/blob/mm_BTLReadoutMap_20261029/CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h)
