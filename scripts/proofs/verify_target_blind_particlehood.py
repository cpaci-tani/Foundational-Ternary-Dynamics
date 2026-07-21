#!/usr/bin/env python3
"""Recompute FTD-0399 metrics and verdict from frozen raw profiles."""

from __future__ import annotations

import argparse
import csv
import math
from collections import defaultdict, deque
from pathlib import Path

PROTOCOLS = ("dissipative", "undamped")
SIZES = (33, 65)
SEEDS = ("A_baseline", "C_hot", "E_cold")
BAND = 0.01
FLOOR = 1e-15
SUMMARY_FIELDS = (
    "protocol", "L", "seed", "t_post", "N", "charge", "centroid",
    "local_energy", "raw_distance", "shape_distance", "energy_cv",
    "cross_L_distance",
)
DETAIL_FIELDS = (
    "protocol", "L", "seed", "t_post", "dx", "dy", "dz",
    "Jx", "Jy", "Jz", "Vx", "Vy", "Vz", "state", "color", "spin",
    "flavor", "cluster_count", "N", "charge", "centroid_x", "centroid_y",
    "centroid_z", "local_energy", "localized", "boundary_clear",
)


def horizon(L: int) -> int:
    return 12 if L == 33 else 24


def norm(values):
    return math.sqrt(sum(value * value for value in values))


def raw_distance(a, b):
    return math.sqrt(sum((x - y) ** 2 for x, y in zip(a, b))) / max(norm(a), norm(b), FLOOR)


def shape_distance(a, b):
    na, nb = max(norm(a), FLOOR), max(norm(b), FLOOR)
    return math.sqrt(sum((x / na - y / nb) ** 2 for x, y in zip(a, b)))


def close(a, b):
    if math.isnan(a) and math.isnan(b):
        return True
    return math.isclose(a, b, rel_tol=2e-13, abs_tol=2e-13)


def read_details(path: Path):
    groups = defaultdict(list)
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if tuple(reader.fieldnames or ()) != DETAIL_FIELDS:
            raise ValueError("detail schema mismatch")
        for row in reader:
            key = (row["protocol"], int(row["L"]), row["seed"], int(row["t_post"]))
            groups[key].append(row)

    expected_keys = {
        (protocol, L, seed, tick)
        for protocol in PROTOCOLS for L in SIZES for seed in SEEDS
        for tick in range(horizon(L) + 1)
    }
    if set(groups) != expected_keys:
        raise ValueError(f"detail key grid mismatch: got {len(groups)}, expected {len(expected_keys)}")

    snapshots = {}
    expected_offsets = {(x, y, z) for x in range(-4, 5)
                        for y in range(-4, 5) for z in range(-4, 5)}
    for key, rows in groups.items():
        if len(rows) != 729:
            raise ValueError(f"{key}: expected 729 detail rows, got {len(rows)}")
        rows.sort(key=lambda row: (int(row["dx"]), int(row["dy"]), int(row["dz"])))
        offsets = {(int(r["dx"]), int(r["dy"]), int(r["dz"])) for r in rows}
        if offsets != expected_offsets:
            raise ValueError(f"{key}: local offset cube mismatch")

        global_names = ("cluster_count", "N", "charge", "centroid_x", "centroid_y",
                        "centroid_z", "local_energy", "localized", "boundary_clear")
        for name in global_names:
            if len({row[name] for row in rows}) != 1:
                raise ValueError(f"{key}: inconsistent repeated {name}")
        cluster_count = int(rows[0]["cluster_count"])
        N = int(rows[0]["N"])
        charge = int(rows[0]["charge"])
        centroid = tuple(float(rows[0][name]) for name in
                         ("centroid_x", "centroid_y", "centroid_z"))
        reported_energy = float(rows[0]["local_energy"])
        localized = int(rows[0]["localized"])
        boundary_clear = int(rows[0]["boundary_clear"])
        if localized not in (0, 1) or boundary_clear not in (0, 1):
            raise ValueError(f"{key}: non-Boolean gate")

        profile = []
        energy = 0.0
        manifested = set()
        local_charge = 0
        centroid_sums = [0.0, 0.0, 0.0]
        for row in rows:
            values = [float(row[name]) for name in
                      ("Jx", "Jy", "Jz", "Vx", "Vy", "Vz",
                       "state", "color", "spin", "flavor")]
            if not all(math.isfinite(value) for value in values):
                raise ValueError(f"{key}: non-finite profile")
            profile.extend(values)
            energy += 0.5 * sum(value * value for value in values[:6])
            state = int(row["state"])
            if state:
                offset = (int(row["dx"]), int(row["dy"]), int(row["dz"]))
                manifested.add(offset)
                local_charge += state
                for axis in range(3):
                    centroid_sums[axis] += offset[axis]
        if not close(energy, reported_energy):
            raise ValueError(f"{key}: local energy does not recompute")
        if localized:
            if len(manifested) != N or local_charge != charge:
                raise ValueError(f"{key}: localized N/charge mismatch")
            if N:
                center = key[1] // 2
                expected_centroid = tuple(center + value / N for value in centroid_sums)
                if any(not close(a, b) for a, b in zip(centroid, expected_centroid)):
                    raise ValueError(f"{key}: centroid mismatch")
            # Recompute 26-connected cluster count inside the complete local support.
            remaining = set(manifested)
            clusters = 0
            while remaining:
                clusters += 1
                queue = deque([remaining.pop()])
                while queue:
                    point = queue.popleft()
                    for dx in (-1, 0, 1):
                        for dy in (-1, 0, 1):
                            for dz in (-1, 0, 1):
                                if dx == dy == dz == 0:
                                    continue
                                neighbor = (point[0] + dx, point[1] + dy, point[2] + dz)
                                if neighbor in remaining:
                                    remaining.remove(neighbor)
                                    queue.append(neighbor)
            if clusters != cluster_count:
                raise ValueError(f"{key}: cluster count does not recompute")
        snapshots[key] = {
            "profile": profile, "cluster_count": cluster_count, "N": N,
            "charge": charge, "centroid": centroid, "energy": energy,
            "localized": bool(localized), "boundary_clear": bool(boundary_clear),
        }
    return snapshots


def metrics(snapshots, key):
    protocol, L, seed, tick = key
    current = snapshots[key]
    raw = max(raw_distance(current["profile"], snapshots[(protocol, L, other, tick)]["profile"])
              for other in SEEDS if other != seed)
    shape = max(shape_distance(current["profile"], snapshots[(protocol, L, other, tick)]["profile"])
                for other in SEEDS if other != seed)
    energies = [snapshots[(protocol, L, other, tick)]["energy"] for other in SEEDS]
    mean = sum(energies) / 3.0
    cv = math.sqrt(sum((value - mean) ** 2 for value in energies) / 3.0) / max(abs(mean), FLOOR)
    cross = math.nan
    if tick <= 12:
        cross = raw_distance(snapshots[(protocol, 33, seed, tick)]["profile"],
                             snapshots[(protocol, 65, seed, tick)]["profile"])
    return raw, shape, cv, cross


def read_and_check_summary(path: Path, snapshots):
    rows = {}
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if tuple(reader.fieldnames or ()) != SUMMARY_FIELDS:
            raise ValueError("summary schema mismatch")
        for row in reader:
            key = (row["protocol"], int(row["L"]), row["seed"], int(row["t_post"]))
            if key in rows:
                raise ValueError(f"duplicate summary row {key}")
            rows[key] = row
    if set(rows) != set(snapshots):
        raise ValueError("summary/detail key grids differ")
    for key, snapshot in snapshots.items():
        row = rows[key]
        if int(row["N"]) != snapshot["N"] or int(row["charge"]) != snapshot["charge"]:
            raise ValueError(f"{key}: summary N/charge mismatch")
        centroid = tuple(float(value) for value in row["centroid"].split(";"))
        if len(centroid) != 3 or any(not close(a, b) for a, b in zip(centroid, snapshot["centroid"])):
            raise ValueError(f"{key}: summary centroid mismatch")
        expected = (snapshot["energy"],) + metrics(snapshots, key)
        actual = tuple(float(row[name]) for name in
                       ("local_energy", "raw_distance", "shape_distance", "energy_cv",
                        "cross_L_distance"))
        if any(not close(a, b) for a, b in zip(actual, expected)):
            raise ValueError(f"{key}: summary metric mismatch")


def check_stderr(path: Path):
    text = path.read_text(encoding="utf-8")
    history_lines = [line for line in text.splitlines() if line.startswith("GATE,") and ",manifest=" in line]
    nonvacuity_lines = [line for line in text.splitlines() if line.startswith("GATE,") and "freeze_nonvacuous=" in line]
    if len(history_lines) != 12 or len(nonvacuity_lines) != 4:
        raise ValueError("stderr gate-row count mismatch")
    if any("manifest=1" not in line or "freeze=2" not in line or "duplicate=1" not in line
           for line in history_lines):
        raise ValueError("manifestation/freeze/duplicate gate failed")
    if any("freeze_nonvacuous=1" not in line for line in nonvacuity_lines):
        raise ValueError("non-vacuity gate failed")


def verdict(snapshots):
    # Raw profile records carry the causal-window gate for every frame.
    if any(not snapshot["boundary_clear"] for snapshot in snapshots.values()):
        return "INVALID"
    stable = all(snapshot["cluster_count"] == 1 and snapshot["N"] >= 1 and snapshot["localized"]
                 for snapshot in snapshots.values())
    if not stable:
        return "NO-STABLE-EXCITATION"

    def converges(protocol):
        for L in SIZES:
            for tick in range(9, 13):
                for seed in SEEDS:
                    raw, shape, cv, cross = metrics(snapshots, (protocol, L, seed, tick))
                    if max(raw, shape, cv, cross) > BAND:
                        return False
        return True

    dissipative = converges("dissipative")
    undamped = converges("undamped")
    if dissipative and undamped:
        return "SPECIES-INVARIANT"
    if dissipative and not undamped:
        return "DISSIPATIVE-ATTRACTOR"
    return "HISTORY-FAMILY"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("details", type=Path)
    parser.add_argument("summary", type=Path)
    parser.add_argument("stderr", type=Path)
    parser.add_argument("--expect", choices=(
        "SPECIES-INVARIANT", "DISSIPATIVE-ATTRACTOR", "HISTORY-FAMILY",
        "NO-STABLE-EXCITATION", "INVALID"))
    args = parser.parse_args()
    try:
        snapshots = read_details(args.details)
        read_and_check_summary(args.summary, snapshots)
        check_stderr(args.stderr)
        result = verdict(snapshots)
    except (OSError, ValueError) as error:
        print(f"VERDICT=INVALID\nERROR={error}")
        return 2
    print(f"SNAPSHOTS={len(snapshots)}")
    print(f"DETAIL_ROWS={len(snapshots) * 729}")
    for protocol in PROTOCOLS:
        maxima = {name: 0.0 for name in ("raw", "shape", "cv", "cross")}
        for L in SIZES:
            for tick in range(9, 13):
                for seed in SEEDS:
                    values = metrics(snapshots, (protocol, L, seed, tick))
                    for name, value in zip(maxima, values):
                        maxima[name] = max(maxima[name], value)
        print(f"{protocol}_final4_max=" + ",".join(f"{k}:{v:.17g}" for k, v in maxima.items()))
    print(f"VERDICT={result}")
    if args.expect and result != args.expect:
        print(f"EXPECTED={args.expect}")
        return 1
    return 0 if result != "INVALID" else 2


if __name__ == "__main__":
    raise SystemExit(main())
