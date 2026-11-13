#!/bin/bash
set -euo pipefail
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT
for mode in False True; do
  mkdir "$test_dir/$mode"
  cd "$test_dir/$mode"
  cmsRun "$SCRAM_TEST_PATH/testBTLCalibrationDB_cfg.py" fromSoA="$mode"
  cmsRun "$SCRAM_TEST_PATH/testBTLCalibrationDB_cfg.py" read=True
  cmsRun "$SCRAM_TEST_PATH/readBTLCalibration_cfg.py" inputDB=btl.db firstRun=2 rawId=0 tac=7 tdcTag=tdc qdcTag=qdc > dump.log
  cmsRun "$SCRAM_TEST_PATH/readBTLCalibration_cfg.py" inputDB=btl.db firstRun=2 denseIndex=0 tac=7 tdcTag=tdc qdcTag=qdc > dense-zero.log
  cmsRun "$SCRAM_TEST_PATH/readBTLCalibration_cfg.py" inputDB=btl.db firstRun=2 denseIndex=331775 tac=7 tdcTag=tdc qdcTag=qdc > dense-last.log
  python3 - <<'CHECK'
import sqlite3
from pathlib import Path
with sqlite3.connect("btl.db") as db:
    assert db.execute('select SINCE from IOV where TAG_NAME="tdc" order by SINCE').fetchall() == [(1,), (3,)]
    assert db.execute('select SINCE from IOV where TAG_NAME="qdc" order by SINCE').fetchall() == [(1,), (2,), (3,)]
# At run 2 the TDC payload is from run 1, while QDC is from run 2.
for kind, run, names in [
    ("TDC", 1, [f"{name}_{disc}" for disc in ("T1", "T2") for name in ("t0", "a0", "a1", "a2")]),
    ("QDC", 2, [f"p{i}" for i in range(10)]),
]:
    lines = [line for line in Path("dump.log").read_text().splitlines() if line.startswith(kind + " ")]
    assert len(lines) == 1, lines
    fields = dict(field.split("=", 1) for field in lines[0].split()[1:])
    assert fields["rawId"] == "0" and fields["tac"] == "7" and fields["valid"] == "1", fields
    for coefficient, name in enumerate(names):
        assert float(fields[name]) == 10000 * run + 100 + 8 * coefficient + 7, (name, fields)
    dense_zero = [line for line in Path("dense-zero.log").read_text().splitlines() if line.startswith(kind + " ")]
    assert dense_zero == lines, (dense_zero, lines)
    last = [line for line in Path("dense-last.log").read_text().splitlines() if line.startswith(kind + " ")]
    assert len(last) == 1, last
    fields = dict(field.split("=", 1) for field in last[0].split()[1:])
    assert int(fields["rawId"]) == (143 << 16) | (35 << 10) | (23 << 5) | 31, fields
    assert fields["denseIndex"] == "331775" and fields["tac"] == "7" and fields["valid"] == "1", fields
    for coefficient, name in enumerate(names):
        assert float(fields[name]) == 10000 * run + 8 * coefficient + 7, (name, fields)
CHECK
done
python3 - <<'CHECK'
import os
import subprocess
for selection, message in [
    (["denseIndex=331776"], "denseIndex must be"),
    (["denseIndex=-2"], "denseIndex must be"),
    (["denseIndex=0", "rawId=0"], "Specify either rawId or denseIndex"),
]:
    result = subprocess.run(
        ["cmsRun", os.path.join(os.environ["SCRAM_TEST_PATH"], "readBTLCalibration_cfg.py"),
         "inputDB=btl.db", "tdcTag=tdc", "qdcTag=qdc", *selection],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
    )
    assert result.returncode != 0 and message in result.stdout, result.stdout
CHECK
