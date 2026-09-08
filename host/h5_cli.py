#!/usr/bin/env python3
"""H5 - 433 MHz Replay host helper: SIMULATION-ONLY replay analysis.
Proof-for-study. No live RF is ever emitted by this script.
Educational/authorized own-lab use only (see README "IMPORTANT").
"""
import argparse
import os
import sys

MOD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, MOD)
from hw_common import DEMO_TAG, require_lab, read_target


def parse_packet(line):
    line = line.strip()
    if ":" not in line:
        return None
    label, _, hexpart = line.partition(":")
    try:
        data = bytes.fromhex(hexpart.strip())
    except ValueError:
        return None
    return {"label": label.strip(), "data": data,
            "hex": hexpart.strip().upper(), "bin": bin_fmt(data)}


def bin_fmt(data):
    return " ".join(format(b, "08b") for b in data)


def timing_mismatch(a, b):
    return a != b


def analyze(text):
    return [p for l in text.splitlines() if (p := parse_packet(l))]


def run_demo():
    print("=== H5 433 MHz replay analysis (SIMULATION ONLY) ===")
    for p in analyze(read_target(
            "fixtures/packets.log",
            "pkt0: AA 12 34 56\npkt1: AA 12 34 57\n")):
        print("  %-6s HEX=%s" % (p["label"], p["hex"]))
        print("          BIN=%s" % p["bin"])
    print("  replay[0] -> simulated output only; no RF generated")
    print(DEMO_TAG)
    return 0


def main(argv=None):
    p = argparse.ArgumentParser(
        description="H5 433 MHz Replay - simulation-only analysis")
    p.add_argument("--demo", action="store_true",
                   help="offline simulation demo (exit 0)")
    p.add_argument("--simulate-replay", metavar="PKT",
                   help="print what a replay WOULD send (no RF)")
    p.add_argument("--file", help="capture log path")
    args = p.parse_args(argv)
    if args.demo or (not args.file and not args.simulate_replay):
        return run_demo()
    text = open(args.file).read() if args.file else read_target("fixtures/packets.log")
    packets = analyze(text)
    if args.simulate_replay:
        match = [x for x in packets if x["label"] == args.simulate_replay]
        target = match[0] if match else (packets[0] if packets else None)
        if not target:
            print("No packets to simulate.")
            return 1
        print("  [SIM] Would transmit %d bytes: %s" % (len(target["data"]), target["hex"]))
        print("  No RF emitted by simulator.")
        return 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
