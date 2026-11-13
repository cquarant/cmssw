#!/bin/bash
set -euo pipefail
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT
for mode in False True; do
  mkdir "$test_dir/$mode"
  cd "$test_dir/$mode"
  cmsRun "$SCRAM_TEST_PATH/testBTLCalibrationDB_cfg.py" fromSoA="$mode"
  cmsRun "$SCRAM_TEST_PATH/testBTLCalibrationDB_cfg.py" read=True
  python3 - <<'CHECK'
import sqlite3
with sqlite3.connect("btl.db") as db:
    assert db.execute('select SINCE from IOV where TAG_NAME="tdc" order by SINCE').fetchall() == [(1,), (3,)]
    assert db.execute('select SINCE from IOV where TAG_NAME="qdc" order by SINCE').fetchall() == [(1,), (2,), (3,)]
CHECK
done
