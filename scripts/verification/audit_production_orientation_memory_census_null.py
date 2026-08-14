#!/usr/bin/env python3
"""Reproducible post-hoc null/persistence diagnosis of the FTD-0908 corpus.

This audit was written after the corpus existed. It cannot change the frozen
FTD-0908 Outcome A and is not a preregistered discriminator.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
import statistics
from collections import Counter, defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0908"
PAIR_FILE = RESULTS / "ftd_0908_pair_observations_v1.csv"
TICK_FILE = RESULTS / "ftd_0908_tick_census_v1.csv"
SUMMARY_FILE = RESULTS / "ftd_0908_summary_v1.json"
LOCKS = {
    PAIR_FILE: "9AC0FFED497615362D26C4D2BE1E295CEE7BEDFB445AB92B5F53B5F9225FCB07",
    TICK_FILE: "16AF93BF2AE469F219BEEDBF6FBF001D27FD097C33C9EC91995A4FFF1CB0A3B7",
    SUMMARY_FILE: "F6BDEB2C033C0351B97E1ECDA56EB998D8563649E4C0DD00A770A52F9676F775",
}
PERSISTENCE_TICKS = 8


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def vector(row: dict[str, str], names: tuple[str, str, str]) -> tuple[float, float, float]:
    return tuple(float(row[name]) for name in names)  # type: ignore[return-value]


def dot(left: tuple[float, float, float], right: tuple[float, float, float]) -> float:
    return sum(a * b for a, b in zip(left, right))


for path, expected in LOCKS.items():
    actual = sha256(path) if path.is_file() else "MISSING"
    print(f"{'PASS' if actual == expected else 'FAIL'}  {path.name}  {actual}")
    if actual != expected:
        raise SystemExit(1)

pairs = read_csv(PAIR_FILE)
ticks = read_csv(TICK_FILE)
summary = json.loads(SUMMARY_FILE.read_text(encoding="utf-8"))

by_tick: dict[tuple[tuple[int, str, int], int], list[dict[str, str]]] = defaultdict(list)
actual_histories: dict[
    tuple[tuple[int, str, int], tuple[int, int]], list[tuple[int, int]]
] = defaultdict(list)
observation_chirality: Counter[int] = Counter()
for row in pairs:
    arm = (int(row["volume"]), row["family"], int(row["seed"]))
    tick = int(row["tick"])
    key = (int(row["positive_id"]), int(row["negative_id"]))
    chi = int(row["chi"])
    by_tick[(arm, tick)].append(row)
    actual_histories[(arm, key)].append((tick, chi))
    observation_chirality[chi] += 1

# Reconstruct the already frozen rotated-negative null, but now retain its
# pseudo-pair IDs and chirality so persistence can be diagnosed. This is the
# post-hoc step and therefore supplies no FTD-0908 acceptance gate.
null_histories: dict[
    tuple[tuple[int, str, int], tuple[int, int]], list[tuple[int, int]]
] = defaultdict(list)
for (arm, tick), rows in by_tick.items():
    ordered = sorted(
        rows,
        key=lambda row: (int(row["negative_id"]), int(row["positive_id"])),
    )
    for rank, positive in enumerate(ordered):
        negative = ordered[(rank + 1) % len(ordered)]
        separation = vector(positive, ("dx", "dy", "dz"))
        norm = math.sqrt(dot(separation, separation))
        axis = tuple(value / norm for value in separation)
        q_plus = dot(axis, vector(positive, ("jpx", "jpy", "jpz")))
        p_plus = dot(axis, vector(positive, ("wpx", "wpy", "wpz")))
        q_minus = dot(axis, vector(negative, ("jmx", "jmy", "jmz")))
        p_minus = dot(axis, vector(negative, ("wmx", "wmy", "wmz")))
        ell = q_plus * p_minus - q_minus * p_plus
        null_key = (
            int(positive["positive_id"]), int(negative["negative_id"])
        )
        null_histories[(arm, null_key)].append((tick, 1 if ell > 0.0 else -1))


def history_metrics(histories: dict) -> tuple[dict, list[dict[str, int]]]:
    arm_runs: dict[tuple[int, str, int], list[int]] = defaultdict(list)
    identity_metrics: list[dict[str, int]] = []
    for (arm, _key), observations in histories.items():
        observations.sort()
        previous_tick = -2
        previous_chi = 0
        current = 0
        maximum = 0
        switches = 0
        gaps = 0
        for tick, chi in observations:
            if previous_tick != -2:
                switches += tick == previous_tick + 1 and chi != previous_chi
                gaps += tick != previous_tick + 1
            current = (
                current + 1
                if tick == previous_tick + 1 and chi == previous_chi
                else 1
            )
            maximum = max(maximum, current)
            previous_tick = tick
            previous_chi = chi
        arm_runs[arm].append(maximum)
        identity_metrics.append({
            "observations": len(observations),
            "maximum_run": maximum,
            "switches": switches,
            "gaps": gaps,
            "first_tick": observations[0][0],
            "last_tick": observations[-1][0],
        })
    return arm_runs, identity_metrics


actual_runs, actual_identity = history_metrics(actual_histories)
null_runs, _null_identity = history_metrics(null_histories)

groups: dict[tuple[int, str], dict[str, object]] = defaultdict(lambda: {
    "actual_pass": 0,
    "null_pass": 0,
    "actual_persistent": 0,
    "null_persistent": 0,
    "actual_max": [],
    "null_max": [],
})
for arm_summary in summary["arms"]:
    arm = (
        int(arm_summary["volume"]),
        str(arm_summary["family"]),
        int(arm_summary["seed"]),
    )
    actual_values = actual_runs.get(arm, [])
    null_values = null_runs.get(arm, [])
    actual_maximum = max(actual_values, default=0)
    null_maximum = max(null_values, default=0)
    group = groups[arm[:2]]
    group["actual_pass"] = int(group["actual_pass"]) + int(
        actual_maximum >= PERSISTENCE_TICKS
    )
    group["null_pass"] = int(group["null_pass"]) + int(
        null_maximum >= PERSISTENCE_TICKS
    )
    group["actual_persistent"] = int(group["actual_persistent"]) + sum(
        value >= PERSISTENCE_TICKS for value in actual_values
    )
    group["null_persistent"] = int(group["null_persistent"]) + sum(
        value >= PERSISTENCE_TICKS for value in null_values
    )
    group["actual_max"].append(actual_maximum)  # type: ignore[union-attr]
    group["null_max"].append(null_maximum)  # type: ignore[union-attr]

print("\nPOST_HOC_NULL_AUDIT=TRUE")
print("THIS_IS_NOT_FTD_0908_ADJUDICATION=TRUE")
print("volume family actual_pass null_pass actual_persistent null_persistent actual_max null_max")
for (volume, family), values in sorted(groups.items()):
    print(
        volume,
        family,
        values["actual_pass"],
        values["null_pass"],
        values["actual_persistent"],
        values["null_persistent"],
        values["actual_max"],
        values["null_max"],
    )

persistent_runs = [
    item["maximum_run"] for item in actual_identity
    if item["maximum_run"] >= PERSISTENCE_TICKS
]
print(f"PAIR_IDENTITIES={len(actual_identity)}")
print(f"PERSISTENT_ACTUAL_IDENTITIES={len(persistent_runs)}")
print(
    "PERSISTENT_RUN_MIN_MEDIAN_MAX="
    f"{min(persistent_runs)},{statistics.median(persistent_runs):g},"
    f"{max(persistent_runs)}"
)
print(f"ACTUAL_CONSECUTIVE_SIGN_SWITCHES={sum(item['switches'] for item in actual_identity)}")
print(f"ACTUAL_IDENTITIES_WITH_SIGN_SWITCH={sum(item['switches'] > 0 for item in actual_identity)}")
print(f"ACTUAL_IDENTITY_GAPS={sum(item['gaps'] for item in actual_identity)}")
print(f"CHI_PLUS_OBSERVATIONS={observation_chirality[1]}")
print(f"CHI_MINUS_OBSERVATIONS={observation_chirality[-1]}")
print(
    "ROTATED_NULL_PERSISTENT_IDENTITIES="
    f"{sum(sum(value >= PERSISTENCE_TICKS for value in runs) for runs in null_runs.values())}"
)
print(
    "ACTUAL_AND_NULL_ARM_PASS_PATTERNS_EQUAL="
    f"{all(values['actual_pass'] == values['null_pass'] for values in groups.values())}"
)
print("PAIR_SPECIFIC_MEMORY_ESTABLISHED=FALSE")
print("CENTRAL_MEMORY_LAW_TESTED=FALSE")
