# BTL TDC and QDC calibrations

BTL stores two calibrations for each electronics channel. TDC describes the
timing response of the T1 and T2 discriminators. QDC describes the pedestal used
to correct the charge measurement.

CMSSW stores each calibration in a serializable payload for the conditions
database (CondDB). When a module requests the calibration in EventSetup, an
ESProducer can convert that payload to a structure of arrays (SoA) on the CPU
(host). Alpaka makes the SoA available on the GPU (device) when a consumer
requests it. TDC and QDC have independent intervals of validity (IOVs), so
each calibration can change at a different run.

To try the full path from writing a payload to reading it back, start with
[the SQLite example](#inspecting-the-stored-coefficients-yourself).

## Classes, records and modules

| Role | TDC | QDC |
| --- | --- | --- |
| Persistent CondDB payload (`COND_SERIALIZABLE`) | [`BTLTDCCalibration`](../interface/BTLTDCCalibration.h) | [`BTLQDCCalibration`](../interface/BTLQDCCalibration.h) |
| Persistent EventSetup record | `BTLTDCCalibrationRcd` | `BTLQDCCalibrationRcd` |
| SoA layout | [`btl::BTLTDCCalibrationSoA`](../interface/BTLTDCCalibrationSoA.h) | [`btl::BTLQDCCalibrationSoA`](../interface/BTLQDCCalibrationSoA.h) |
| Host collection (`PortableHostCollection`) | [`btl::BTLTDCCalibrationHost`](../interface/BTLTDCCalibrationHost.h) | [`btl::BTLQDCCalibrationHost`](../interface/BTLQDCCalibrationHost.h) |
| Alpaka device collection (`PortableCollection`) | [`btl::BTLTDCCalibrationDevice`](../interface/alpaka/BTLTDCCalibrationDevice.h) | [`btl::BTLQDCCalibrationDevice`](../interface/alpaka/BTLQDCCalibrationDevice.h) |
| EventSetup record for the host and device SoAs | `BTLTDCCalibrationSoARcd` | `BTLQDCCalibrationSoARcd` |

- **Fill the persistent payloads with fixed values.** `BTLTrivialCalibrationSource`
  is an EventSetup source that fills both payloads with configurable defaults.
  [Source implementation](../../../CalibMTD/MTDTrivialCondChannels/plugins/BTLTrivialCalibrationSource.cc)
- **Read or write the persistent payloads.** `PoolDBESSource` reads the tags
  into their persistent records. `BTLCalibrationWriter` writes both payloads
  through `PoolDBOutputService`. `BTLCalibrationReader` prints their stored
  coefficients for inspection.
  [Writer](../../../CondTools/MTD/plugins/BTLCalibrationWriter.cc) ·
  [Reader](../../../CondTools/MTD/plugins/BTLCalibrationReader.cc)
- **Build the host SoAs.** `BTLTDCCalibrationSoAESProducer` and
  `BTLQDCCalibrationSoAESProducer` read their respective persistent records
  and publish the converted collections in the SoA records. They use
  `btl::makeSoA()` for the conversion.
  [ESProducer implementation](../../../CalibMTD/MTDTrivialCondChannels/plugins/alpaka/BTLCalibrationSoAESProducer.cc) ·
  [Conversion functions](../../../CalibMTD/MTDTrivialCondChannels/interface/BTLCalibrationConversions.h)
- **Copy from host to device.** The same two Alpaka ESProducers register the
  standard asynchronous copy provided by `CopyToDevice<PortableHostCollection<...>>`.
  EventSetup creates and caches the device collection for each device that
  the backend serves when the product is requested. The serial backend uses
  the host collection directly.
  [Host and device setup](#using-the-soas-in-eventsetup)

## Channel and TAC addressing

One SoA row represents one electronics channel, which reads one side of a
crystal. Each channel has eight TAC slots, numbered 0 through 7. Each
coefficient is an Eigen vector with one value per TAC. `SOA_EIGEN_COLUMN`
stores values for the same TAC contiguously across channels, as in
`HGCalCalibParamSoA`.

The DAQ encoding uses HS-links 0..35 (a six-bit field), e-links 0..23 and
TOFHIR channels 0..31. There are 144 FEDs, with 12 FEDs per Serenity and
three HS-links per FED. The FED field starts at bit 16. The shared specifications are in
`CondFormats/MTDObjects/interface/BTLElectronicsSpecs.h`.
[`mm_BTLReadoutMap_20261029`](https://github.com/martinamalberti/cmssw/tree/mm_BTLReadoutMap_20261029)

`BTLElectronicsId::denseChannelIndex()` converts a channel's encoded
electronics ID (`rawId`) to its SoA row. The calculation in
`BTLElectronicsIndexing` uses the hardware address.

```text
serenity = (fedId - kFirstFEDId) / nFEDsPerSerenity
index = ((serenity * 36 + hsLinkId) * 24 + eLinkId) * 32 + channelId
```

Both collections allocate `BTLElectronicsIndexing::kNumberOfChannels` rows
(331,776). Each channel keeps the same row even if a payload stores only a
subset of channels. `BTLReadoutMap::checkDenseChannelIndex()` checks coverage
of rows 0 through 331775 and increasing raw IDs. `getListOfDetIds()` enumerates
the mapped crystals.

Check `isValid()` before indexing because the calculation assumes a valid
FED/HS-link assignment. Host code can use
`BTLElectronicsId::denseChannelIndex()` directly. The plain constexpr helper
`BTLElectronicsIndexing::denseChannelIndex(fed, hs, el, channel)` implements the
same arithmetic for code that already has the four fields. Kernels can consume
the resulting index and access `view[index]` without a map search.

`BTLReadoutMap::initialize()` rebuilds the transient reverse map. The
conversion to SoA uses electronics IDs alone. Use `BTLReadoutMapRcd` when a
consumer also needs to look up crystal IDs.

The raw-ID encoding differs from the former seven-bit HS-link format. Existing
readout-map and electronics-keyed calibration payloads made with that old
encoding must be regenerated.

`valid` is an eight-bit mask for each channel. Bit `t` tells you whether TAC
`t` has a calibration. In TDC, that bit applies to both T1 and T2. Producers
call `zeroInitialise()` after allocation, then fill the raw IDs, coefficients
and valid bits. Rows for channels absent from the payload keep a zero mask.
Use the mask to check validity, including for the channel whose raw ID is zero.

```cpp
btl::BTLTDCCalibrationHost tdc(cms::alpakatools::host(),
                              BTLElectronicsIndexing::kNumberOfChannels);
tdc.zeroInitialise();
// With a valid electronicsId and 0 <= tac < 8
const auto i = electronicsId.denseChannelIndex();
tdc.view()[i].rawId() = electronicsId.rawId();
tdc.view()[i].t0_T1()(tac) = measuredT0_T1;
tdc.view()[i].a0_T1()(tac) = measuredA0_T1;
tdc.view()[i].a1_T1()(tac) = measuredA1_T1;
tdc.view()[i].a2_T1()(tac) = measuredA2_T1;
tdc.view()[i].t0_T2()(tac) = measuredT0_T2;
tdc.view()[i].a0_T2()(tac) = measuredA0_T2;
tdc.view()[i].a1_T2()(tac) = measuredA1_T2;
tdc.view()[i].a2_T2()(tac) = measuredA2_T2;
tdc.view()[i].valid() |= uint8_t(1u << tac);
```

## Parameter definitions and units

### TDC timing response

TDC stores `t0`, `a0`, `a1` and `a2` separately for T1 and T2, with one
value per channel and TAC. The field names use the `_T1` and `_T2` suffixes,
for example `a1_T1` and `a1_T2`. The electronics simulation uses the
coefficients of the corresponding discriminator in this response model.

```
q = coarse - time / clockPeriod - t0
fineADC = a0 + a1*q + a2*q*q
```

`t0` and `q` are in clock periods. Coefficients are in ADC counts per
corresponding power of `q`. Reconstruction inverts the quadratic on its physical
branch, then computes `time = (coarse - t0 - q) * clockPeriod`. The output is in
seconds if the clock period is in seconds (the reference uses 6.25 ns).
The payload contains the response parameters. The reconstruction code is
responsible for the inverse and for any clock rollover, time-walk or absolute
time-offset corrections.

### QDC pedestal

QDC stores `p0` through `p9` for each channel and TAC. These coefficients
describe how the pedestal varies with integration time. The defaults set
`p4` through `p9` to zero.

```
ti = EOIcoarse - time / clockPeriod
pedestalADC = p0 + p1*ti + ... + p9*ti^9
correctedADC = Qfine - pedestalADC
```

`ti` is in clock periods and the pedestal is in ADC counts.
`correctedADC` remains in ADC counts. A producer can replicate a pedestal
measurement across TACs when that measurement applies to all of them.

## Persistent payload contents

Each CondDB payload stores the following vectors.

- `rawIds` holds the electronics IDs, one per stored channel, in any order.
- `valid` holds one eight-bit TAC validity mask per stored channel.
- TDC stores `t0_T1`, `a0_T1`..`a2_T1` and `t0_T2`, `a0_T2`..`a2_T2`.
  QDC stores `p0`..`p9`.

Each coefficient vector has exactly `rawIds.size()*8` floats. The value for
stored channel `i`, TAC `t`, is at `i*8+t`. A payload may contain any subset of
the hardware channels. Use finite placeholder coefficients and clear the
validity bits for missing TACs in a stored channel. `validate()` rejects
malformed vector sizes, duplicate or invalid IDs (including reserved high
bits), and nonfinite coefficients. Both types have dictionaries, generated
Boost serialization and registrations for EventSetup and CondDB.

## Using the SoAs in EventSetup

Load the conversion producers in your CMSSW configuration. The process also
needs an input for the persistent records, such as `PoolDBESSource` or the
fixed-value source described below.

```python
process.load("CalibMTD.MTDTrivialCondChannels.btlCalibrationSoA_cff")
```

The conversion validates the payload, zero-initializes the full SoA and
places each stored channel at `BTLElectronicsId(rawId).denseChannelIndex()`.
EventSetup rebuilds each SoA when its corresponding persistent record changes.

CPU consumers can request the host types using `edm::ESGetToken` and the SoA
records in the table above. For GPU jobs, load
`Configuration.StandardSequences.Accelerators_cff` to enable GPU discovery.
An Alpaka consumer requests the device types through `device::EventSetup`
using the same SoA records.

```cpp
// Members declared in the Alpaka backend namespace and initialized with esConsumes().
device::ESGetToken<btl::BTLTDCCalibrationDevice, BTLTDCCalibrationSoARcd> tdcToken_;
device::ESGetToken<btl::BTLQDCCalibrationDevice, BTLQDCCalibrationSoARcd> qdcToken_;
// With setup of type device::EventSetup
const auto& tdc = setup.getData(tdcToken_);
const auto& qdc = setup.getData(qdcToken_);
```

## Fixed values and measured calibration results

`BTLTrivialCalibrationSource` is useful for testing with known inputs. It
converts the 18 configurable coefficients to floats and repeats them across
all eight TACs on each selected channel. All those TACs are marked valid.
T1 and T2 share the same defaults and can be configured independently.
An empty `channelIds` list (the default) selects all 331,776 hardware channels
without loading geometry or a readout map. To limit the payload to specific
channels, pass their electronics raw IDs in `cms.vuint32`.

```python
process.load("CalibMTD.MTDTrivialCondChannels.btlTrivialCalibrationSource_cfi")
process.btlTrivialCalibrationSource.channelIds = cms.vuint32(0, 20)
process.btlTrivialCalibrationSource.a1_T1 = 511.27832
process.btlTrivialCalibrationSource.a1_T2 = 512.0
process.load("CalibMTD.MTDTrivialCondChannels.btlCalibrationSoA_cff")
```

For measured calibrations, fill the public vectors, call `validate()` and
publish the payload in its persistent record. A calibration algorithm that
already produces SoAs can publish the host collections in the SoA records and
use the writer's SoA input mode. Configure one producer for each product in
its record. For results computed on a device, copy them to host and wait for
that copy before writing.

The reusable conversion functions are declared in
`CalibMTD/MTDTrivialCondChannels/interface/BTLCalibrationConversions.h`.

```cpp
auto host = btl::makeSoA(tdcPayload);  // overloads for TDC and QDC
auto persistent = btl::makePayload(host);
```

The reverse conversion checks the collection size and that each valid row's raw
ID agrees with its dense index. It stores rows with nonzero validity masks in
dense-index order, retaining all eight TAC coefficient slots for those rows.
The round trip preserves all usable calibrations and their validity masks.
It may reorder channels and omits rows with every TAC invalid.

## Writing and reading CondDB

`BTLCalibrationWriter` has two input modes. With `fromSoA=False` (the default),
it validates and writes the persistent payloads. With `fromSoA=True`, it
converts the host SoAs with `btl::makePayload()` before writing. It writes each
payload when it first sees that record and again when the record changes.
Configure `PoolDBOutputService` with the `runnumber` time type and start the
job at the first run of each intended interval. Each IOV starts at the
current event's run, with at most one IOV per run for each tag.

The example configs use tags `BTLTDCCalibration_v1` and `BTLQDCCalibration_v1`.
The reader's `firstRun` argument is a run number. It selects the interval
containing that run separately for each tag. Use `tdcTag` and `qdcTag` to
inspect other tags.

Regenerate development databases written with the earlier single-response TDC
schema to get separate T1 and T2 coefficient vectors. For old
`BTLTDCCalibs` or `BTLTDCEleChannelCalibs` tags, read the payloads with the
previous code and explicitly convert the data, including any electronics
encoding changes.

## Tests and how to repeat them

Use the `src` directory of a CMSSW area containing this code. Set up the
runtime and build the release area first.

```sh
source /cvmfs/cms.cern.ch/cmsset_default.sh
eval "$(scram runtime -sh)"
scram b -j 8
```

The four registered tests can run together through SCRAM.

```sh
scram b runtests_testBTLCalibrationSoA runtests_testBTLCalibrationConversions \
  runtests_testBTLTrivialCalibrationSoA runtests_testBTLCalibrationDB
```

The same checks can be run individually with the commands below. The
automated checks report a failure through the exit status. On success, the
standalone binaries may print nothing, and the `cmsRun` tests normally show
event processing without a fatal exception.

1. **Electronics indexing and SoA storage** (`testBTLCalibrationSoA`).
   Checks all 331,776 valid hardware IDs, round-trip decoding, increasing raw IDs,
   consecutive dense indices and invalid address boundaries. It also checks
   sparse rows, independent channel and TAC values, reverse lookups and duplicate
   rejection.

   ```sh
   "$CMSSW_BASE/test/$SCRAM_ARCH/testBTLCalibrationSoA"
   ```

2. **Persistent payload ↔ host SoA** (`testBTLCalibrationConversions`).
   Checks round trips for TDC and QDC, including separate T1/T2 coefficients,
   sparse and empty payloads, row reordering and malformed inputs.

   ```sh
   "$CMSSW_BASE/test/$SCRAM_ARCH/testBTLCalibrationConversions"
   ```

3. **Fixed-value source and Alpaka EventSetup products**
   (`testBTLTrivialCalibrationSoA`). Checks raw IDs, validity masks and all
   18 configured coefficients across two events in a sparse map. T1 and T2 use
   distinct values. The Alpaka consumer obtains both device products through
   EventSetup. On CPU it checks that they refer to the host collections. On
   GPU it copies them back and compares the full buffers with the host ones.

   ```sh
   cmsRun CalibMTD/MTDTrivialCondChannels/test/testBTLCalibrationSoA_cfg.py
   ```

   On a GPU machine, check the startup log for the CUDA or ROCm Alpaka service
   and the detected GPU. To require a GPU, add
   `process.options.accelerators = ["gpu-nvidia"]` (CUDA) or
   `process.options.accelerators = ["gpu-amd"]` (ROCm) to a local copy of the
   config before running it. The job then requires the selected accelerator.

4. **SQLite persistence and independent IOVs** (`testBTLCalibrationDB`).
   Writes and reads temporary databases in both writer modes. It checks
   coefficients and partial TAC masks in the rebuilt SoAs, including raw ID
   zero and the last channel. It queries the stored IOV boundaries, which must
   be runs 1 and 3 for TDC and runs 1, 2 and 3 for QDC. It also checks the dump
   for run 2, custom tags and dense indices 0 and 331775, including error
   handling for invalid selections. The wrapper removes its temporary
   databases when it exits.

   ```sh
   SCRAM_TEST_PATH="$PWD/CondTools/MTD/test" bash CondTools/MTD/test/testBTLCalibrationDB.sh
   ```

5. **Full geometry coverage** (additional check). Uses the real
   `BTLReadoutMapESProducer` and checks the mapping, validity masks and all
   coefficients for all 331,776 channels.

   ```sh
   cmsRun CalibMTD/MTDTrivialCondChannels/test/testBTLCalibrationSoAGeometry_cfg.py
   ```

All five checks passed with `CMSSW_20_1_X_2026-11-12-2300`
(`el9_amd64_gcc14`) on the serial backend, including the full Run4D127 geometry.
The ESProducers and the device check compiled for serial, CUDA and ROCm.
The GPU copy check still needs a run on GPU hardware.

### Inspecting the stored coefficients yourself

From the same `src` directory, write a fresh SQLite database with the default
coefficients and read one channel and TAC back. Run the setup commands above
first if you are opening a new shell.

```sh
btl_check_dir=$(mktemp -d)
cmsRun CondTools/MTD/test/writeBTLCalibration_cfg.py \
  outputDB="$btl_check_dir/btlCalibration.db" firstRun=1
cmsRun CondTools/MTD/test/readBTLCalibration_cfg.py \
  inputDB="$btl_check_dir/btlCalibration.db" firstRun=1 denseIndex=3000 tac=3
```

The output should include the summary and both coefficient lines below. CMSSW
also logs a `got data of type` message for each host SoA. Those two messages
confirm that EventSetup built the SoAs from the SQLite payloads. The
coefficient lines show the persistent values read from the database.

```text
BTL calibration run=1 TDC_channels=331776 QDC_channels=331776
TDC rawId=69304 denseIndex=3000 tac=3 valid=1 t0_T1=-0.0483432636 a0_T1=57.244545 a1_T1=511.27832 a2_T1=-7.88385773 t0_T2=-0.0483432636 a0_T2=57.244545 a1_T2=511.27832 a2_T2=-7.88385773
QDC rawId=69304 denseIndex=3000 tac=3 valid=1 p0=49.5422287 p1=-0.323424011 p2=0.0625780001 p3=-0.00248400006 p4=0 p5=0 p6=0 p7=0 p8=0 p9=0
```

- Use `denseIndex` for a SoA row (0 through 331775), or `rawId` for the
  encoded electronics ID in decimal. Supply one selector at a time. In this
  example, `rawId=69304` selects the same channel as `denseIndex=3000`.
- With neither selector, the reader uses raw ID 0. The default TAC is 0.
  Use `tac=-1` to see all eight TACs for the selected channel. Use `rawId=-1`
  to print all stored channels. Add `> "$btl_check_dir/btlCalibration.txt"`
  to save the dump.
- A printed `valid=0` means the channel is present and that TAC is
  uncalibrated. A missing channel is reported as `not stored in this payload`.

To check the writer's reverse conversion, write a second database using host
SoAs as input and inspect the same channel. The two coefficient lines and the
channel counts should match the first run because both jobs use the same
defaults.

```sh
cmsRun CondTools/MTD/test/writeBTLCalibration_cfg.py \
  outputDB="$btl_check_dir/btlFromSoA.db" firstRun=1 fromSoA=True
cmsRun CondTools/MTD/test/readBTLCalibration_cfg.py \
  inputDB="$btl_check_dir/btlFromSoA.db" firstRun=1 denseIndex=3000 tac=3
```

## References

- [Original vector-based TDC payload](https://github.com/cquarant/cmssw/blob/cq_btlCondFormat/CondFormats/MTDObjects/interface/BTLTDCCalibration.h)

- [HGCal calibration layout](https://github.com/cquarant/cmssw/blob/cq_btlCondContainer_v1-2/CondFormats/HGCalObjects/interface/HGCalCalibParamSoA.h)
- [BTL digi layout](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/DataFormats/FTLDigiSoA/interface/BTLDigiSoA.h)
- [Electronics coefficients](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/SimFastTiming/FastTimingCommon/interface/BTLElectronicsSim.h)
- [Forward conversion formulas](https://github.com/24LopezR/cmssw/blob/MTD_201X_GPU_fix52456/SimFastTiming/FastTimingCommon/src/BTLElectronicsSim.cc)
- [Reference dense index](https://github.com/martinamalberti/cmssw/blob/mm_BTLReadoutMap_20261029/CondFormats/MTDObjects/interface/BTLElectronicsIndexing.h)
