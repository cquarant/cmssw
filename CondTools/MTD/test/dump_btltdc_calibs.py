#!/usr/bin/env python3
"""Print BTL TDC conditions from SQLite; run after cmsenv."""

import argparse
import os
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("db", type=Path, help="SQLite conditions file")
    parser.add_argument("--tag", default="BTLTDCCalibs_trivial_v1")
    parser.add_argument("--run", type=int, default=1, help="Run selecting the IOV (default: 1)")
    parser.add_argument("--max-crystals", type=int, help="Print only the first N crystals")
    args = parser.parse_args()
    if not args.db.is_file():
        parser.error(f"Database not found: {args.db}")
    if args.run < 1 or args.run >= 2**32:
        parser.error("--run must be between 1 and 4294967295")
    if args.max_crystals is not None and args.max_crystals < 1:
        parser.error("--max-crystals must be positive")
    if not os.environ.get("CMSSW_BASE"):
        parser.error("Set up the CMSSW environment with cmsenv first")

    import ROOT

    ROOT.gROOT.SetBatch(True)
    for library in ("libCondCoreCondDB", "libCondFormatsMTDObjects"):
        if ROOT.gSystem.Load(library) < 0:
            raise RuntimeError(f"Cannot load {library}; build the CMSSW packages with scram b")

    # CMSSW stores Boost-serialized objects, so SQLite alone cannot decode them.
    if not ROOT.gInterpreter.Declare(r'''
        #include "CondCore/CondDB/interface/ConnectionPool.h"
        #include "CondFormats/MTDObjects/interface/BTLTDCCalibs.h"
        #include "FWCore/PluginManager/interface/PluginManager.h"
        #include "FWCore/PluginManager/interface/standard.h"

        std::unique_ptr<BTLTDCCalibs> readBTLTDCForTable(
            const std::string& connection, const std::string& tag, unsigned int run) {
          edmplugin::PluginManager::configure(edmplugin::standard::config());
          cond::persistency::ConnectionPool pool;
          pool.configure();
          auto session = pool.createSession(connection, false);
          session.transaction().start(true);
          const auto interval = session.readIov(tag).getInterval(run);
          auto payload = session.fetchPayload<BTLTDCCalibs>(interval.payloadId);
          session.transaction().commit();
          return payload;
        }

        std::vector<unsigned int> btlTDCIdsForTable(const BTLTDCCalibs& payload, unsigned int limit) {
          std::vector<unsigned int> ids;
          for (const auto& entry : payload.btlItems()) {
            if (limit && ids.size() >= limit) break;
            ids.push_back(entry.first);
          }
          return ids;
        }

        std::vector<float> btlTDCValuesForTable(
            const BTLTDCCalibs& payload, unsigned int id, unsigned int side, unsigned int tac) {
          const auto& channel = payload.btlItems().at(id).channels.at(side);
          return {channel.t0.at(tac), channel.a0.at(tac), channel.a1.at(tac), channel.a2.at(tac)};
        }
    '''):
        raise RuntimeError("Cannot compile the CMSSW payload reader")

    payload = ROOT.readBTLTDCForTable("sqlite_file:" + str(args.db.resolve()), args.tag, args.run)
    print(f"Tag: {args.tag} | run: {args.run} | crystals: {payload.sizeBTL()}")
    print(f"{'rawId':>10} {'side':>5} {'TAC':>3} {'t0':>13} {'a0':>13} {'a1':>13} {'a2':>13}")
    for raw_id in ROOT.btlTDCIdsForTable(payload, args.max_crystals or 0):
        for side, name in enumerate(("minus", "plus")):
            for tac in range(8):
                values = ROOT.btlTDCValuesForTable(payload, raw_id, side, tac)
                print(f"{raw_id:10d} {name:>5} {tac:3d} " + " ".join(f"{value:13.9g}" for value in values))


if __name__ == "__main__":
    main()
