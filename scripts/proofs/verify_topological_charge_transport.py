#!/usr/bin/env python3
"""Recompute the frozen FTD-0398 verdict from its canonical CSV."""

from __future__ import annotations

import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path

SEEDS = ("A_baseline", "C_hot", "E_cold")
FIELDS = (
    "seed", "tick", "radius", "Q", "min_j", "valid", "e_half",
    "manifest_x", "manifest_y", "manifest_z",
)


def load(path: Path):
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if tuple(reader.fieldnames or ()) != FIELDS:
            raise ValueError(f"schema mismatch: {reader.fieldnames}")
        rows = list(reader)
    if len(rows) != 3 * 9 * 6:
        raise ValueError(f"expected 162 rows, got {len(rows)}")
    parsed = {}
    for row in rows:
        key = (row["seed"], int(row["tick"]), int(row["radius"]))
        if key in parsed:
            raise ValueError(f"duplicate row {key}")
        if key[0] not in SEEDS or key[1] not in range(9) or key[2] not in range(1, 7):
            raise ValueError(f"out-of-protocol row {key}")
        q = float(row["Q"])
        min_j = float(row["min_j"])
        valid = int(row["valid"])
        energy = float(row["e_half"])
        coords = tuple(int(row[name]) for name in ("manifest_x", "manifest_y", "manifest_z"))
        if not all(math.isfinite(x) for x in (q, min_j, energy)):
            raise ValueError(f"non-finite value at {key}")
        if valid not in (0, 1) or bool(valid) != (min_j > 1e-12):
            raise ValueError(f"validity/floor mismatch at {key}")
        parsed[key] = (q, min_j, bool(valid), energy, coords)
    if len(parsed) != 162:
        raise ValueError("incomplete protocol grid")
    return parsed


def smallest_charged(data, seed, tick):
    for radius in range(1, 7):
        q, _, valid, _, _ = data[(seed, tick, radius)]
        if valid and abs(q) >= 0.5:
            return radius
    return None


def colocated(data):
    # One fixed R<=2 must carry unit charge at freeze and all four later ticks.
    return all(any(
        all(data[(seed, tick, radius)][2] and
            abs(data[(seed, tick, radius)][0]) >= 0.95
            for tick in range(2, 7))
        for radius in (1, 2)) for seed in SEEDS)


def transported(data):
    for seed in SEEDS:
        sequence = [smallest_charged(data, seed, tick) for tick in range(9)]
        first_index = next((i for i, radius in enumerate(sequence) if radius is not None), None)
        if first_index is None:
            return False
        start = sequence[first_index]
        move_index = next((i for i in range(first_index + 1, 9)
                           if sequence[i] is not None and sequence[i] >= start + 2), None)
        if move_index is None:
            return False
        for tick in range(move_index, 9):
            for radius in (1, 2):
                q, _, valid, _, _ = data[(seed, tick, radius)]
                if valid and abs(q) >= 0.5:
                    return False
    return True


def destroyed(data):
    # A previously charged shell must cross the |J|=0 definition boundary;
    # every later defined enclosing shell must remain in the trivial band.
    for seed in SEEDS:
        witness = False
        for radius in range(1, 7):
            charged_ticks = [tick for tick in range(9)
                             if data[(seed, tick, radius)][2] and
                             abs(data[(seed, tick, radius)][0]) >= 0.5]
            if not charged_ticks:
                continue
            first = min(charged_ticks)
            crossing = next((tick for tick in range(first + 1, 9)
                             if not data[(seed, tick, radius)][2]), None)
            if crossing is None:
                continue
            later = [data[(seed, tick, enclosing)]
                     for tick in range(crossing + 1, 9)
                     for enclosing in range(radius, 7)
                     if data[(seed, tick, enclosing)][2]]
            if later and all(abs(row[0]) <= 0.05 for row in later):
                witness = True
                break
        if not witness:
            return False
    return True


def correctness(data):
    expected_energy = {
        "A_baseline": 1.368676308503,
        "C_hot": 5.828246462835,
        "E_cold": 0.540720277788,
    }
    for seed in SEEDS:
        coords = data[(seed, 2, 1)][4]
        if coords == (-1, -1, -1) or any(c - 6 < 0 or c + 6 >= 17 for c in coords):
            return False
        q, _, valid, energy, _ = data[(seed, 2, 1)]
        if not valid or abs(q) > 5e-9 or abs(energy - expected_energy[seed]) >= 1e-9:
            return False
    return True


def verdict(data):
    if not correctness(data):
        return "INVALID"
    if colocated(data):
        return "COLOCALIZED"
    if transported(data):
        return "TRANSPORTED"
    if destroyed(data):
        return "ZERO-CROSSING/DESTROYED"
    return "UNDERDETERMINED"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("csv", type=Path)
    parser.add_argument("--expect", choices=(
        "COLOCALIZED", "TRANSPORTED", "ZERO-CROSSING/DESTROYED",
        "UNDERDETERMINED", "INVALID"))
    args = parser.parse_args()
    try:
        data = load(args.csv)
        result = verdict(data)
    except (OSError, ValueError) as error:
        print(f"VERDICT=INVALID\nERROR={error}")
        return 2
    print(f"ROWS={len(data)}")
    print("GRID=3 seeds x 9 ticks x 6 radii")
    for seed in SEEDS:
        sequence = [smallest_charged(data, seed, tick) for tick in range(9)]
        print(f"{seed}_smallest_charged={sequence}")
    print(f"VERDICT={result}")
    if args.expect and result != args.expect:
        print(f"EXPECTED={args.expect}")
        return 1
    return 0 if result != "INVALID" else 2


if __name__ == "__main__":
    raise SystemExit(main())
