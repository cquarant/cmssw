#!/usr/bin/env python3
"""Print crystal and electronic-channel BTL TDC conditions from SQLite; run after cmsenv."""

import argparse
import os
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("db", type=Path, help="SQLite conditions file")
    parser.add_argument("--tag", default="BTLTDCCalibs_trivial_v1", help="Crystal payload tag")
    parser.add_argument("--channel-tag", default="BTLTDCEleChannelCalibs_trivial_v1")
    parser.add_argument("--calibration", choices=("both", "crystals", "channels"), default="both",
                        help="Payloads to print (default: both); crystal IDs select channels via the readout mapping")
    parser.add_argument("--run", type=int, default=1, help="Run selecting the IOV (default: 1)")
    parser.add_argument("--max-crystals", type=int, help="Print the first N crystals in each payload, including both sides and all TACs")
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
    for library in ("libCondCoreCondDB", "libCondFormatsMTDObjects", "libEventFilterMTDRawToDigi"):
        if ROOT.gSystem.Load(library) < 0:
            raise RuntimeError(f"Cannot load {library}; build the CMSSW packages with scram b")

    # CMSSW stores Boost-serialized objects, so SQLite alone cannot decode them.
    if not ROOT.gInterpreter.Declare(r'''
        #include "CondCore/CondDB/interface/ConnectionPool.h"
        #include "CondFormats/MTDObjects/interface/BTLTDCCalibs.h"
        #include "CondFormats/MTDObjects/interface/BTLTDCEleChannelCalibs.h"
        #include "EventFilter/MTDRawToDigi/interface/BTLElectronicsMapping.h"
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

        std::unique_ptr<BTLTDCEleChannelCalibs> readBTLTDCChannelsForTable(
            const std::string& connection, const std::string& tag, unsigned int run) {
          cond::persistency::ConnectionPool pool;
          pool.configure();
          auto session = pool.createSession(connection, false);
          session.transaction().start(true);
          const auto interval = session.readIov(tag).getInterval(run);
          auto payload = session.fetchPayload<BTLTDCEleChannelCalibs>(interval.payloadId);
          session.transaction().commit();
          return payload;
        }

        // Match the producer: lpGBT0, with separate TOFHIR channels for both sides.
        unsigned int btlTDCElectronicsIdForTable(unsigned int rawId, unsigned int side) {
          const BTLDetId det(rawId);
          const BTLElectronicsMapping mapping;
          return BTLElectronicsId(mapping.slink(det), mapping.hslink(det), mapping.elink(det),
                                  mapping.TOFHIRCh(det, side)).rawId();
        }

        std::vector<float> btlTDCChannelValuesForTable(
            const BTLTDCEleChannelCalibs& payload, unsigned int id, unsigned int tac) {
          const auto& channel = payload.btlItems().at(id);
          return {channel.t0.at(tac), channel.a0.at(tac), channel.a1.at(tac), channel.a2.at(tac)};
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

    connection = "sqlite_file:" + str(args.db.resolve())
    payload = ROOT.readBTLTDCForTable(connection, args.tag, args.run)
    ids = list(ROOT.btlTDCIdsForTable(payload, args.max_crystals or 0))

    if args.calibration in ("both", "crystals"):
        print(f"Crystal tag: {args.tag} | run: {args.run} | crystals: {payload.sizeBTL()} | selected: {len(ids)}")
        print(f"{'rawId':>10} {'side':>5} {'TAC':>3} {'t0':>13} {'a0':>13} {'a1':>13} {'a2':>13}")
        for raw_id in ids:
            for side, name in enumerate(("minus", "plus")):
                for tac in range(8):
                    values = ROOT.btlTDCValuesForTable(payload, raw_id, side, tac)
                    print(f"{raw_id:10d} {name:>5} {tac:3d} " + " ".join(f"{value:13.9g}" for value in values))

    if args.calibration in ("both", "channels"):
        channels = ROOT.readBTLTDCChannelsForTable(connection, args.channel_tag, args.run)
        # Validate the selection before printing any channel rows.
        selected = []
        for raw_id in ids:
            for side, name in enumerate(("minus", "plus")):
                electronics_id = ROOT.btlTDCElectronicsIdForTable(raw_id, side)
                if channels.btlItems().count(electronics_id) == 0:
                    raise RuntimeError(
                        f"Channel {electronics_id} for crystal {raw_id} ({name}) is absent from "
                        f"{args.channel_tag}; check that payloads and the current readout mapping match")
                selected.append((raw_id, name, electronics_id))
        print(f"\nChannel tag: {args.channel_tag} | run: {args.run} | channels: {channels.size()} "
              f"| selected crystals: {len(ids)} | selected channels: {len(selected)}")
        print(f"{'crystalId':>10} {'side':>5} {'elecRawId':>10} {'FED':>4} {'HS':>3} {'EL':>3} {'CH':>3} "
              f"{'TAC':>3} {'t0':>13} {'a0':>13} {'a1':>13} {'a2':>13}")
        for raw_id, name, electronics_id in selected:
            elec = ROOT.BTLElectronicsId(electronics_id)
            for tac in range(8):
                values = ROOT.btlTDCChannelValuesForTable(channels, electronics_id, tac)
                print(f"{raw_id:10d} {name:>5} {electronics_id:10d} {elec.fedId():4d} "
                      f"{elec.hsLinkId():3d} {elec.eLinkId():3d} {elec.channelId():3d} {tac:3d} "
                      + " ".join(f"{value:13.9g}" for value in values))


if __name__ == "__main__":
    main()
