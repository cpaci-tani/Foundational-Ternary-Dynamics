"""Independent serialized-record certificate for FTD-0747.

This proof reads the frozen FTD-0745 baseline plus all FTD-0747 CSV/JSON
records.  It does not call the C++ verdict function or rerun dynamics.  It also
isolates the registered WSL CRLF header-loader defect from the actual prefix
comparison.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CUDA_v2.md"
RUNNER = ROOT / "engine/tests/campaign_causal_horizon_environmental_persistence_cuda.cpp"
EXECUTABLE = ROOT / "engine/build_wsl/campaign_causal_horizon_environmental_persistence_cuda"
BASELINE = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.csv"
RESULTS = ROOT / "engine/results/ftd_0747"
STEM = "ftd_0747_causal_horizon_environmental_persistence_cuda_v2"

HASHES = {
    PROTOCOL: "1FB4A49897D8FEC333C686A54D44A90EA6E51D799EDBD9168F8D313287F4FD5F",
    RUNNER: "85E4FBE7D0A3A21EB760C3D9F173CAA9BE7F9699596A93609FABD50683462F14",
    EXECUTABLE: "907B873ABF89F352FD340BBD874AC0CB94282F0BDEABCE81798F85B026E9A01B",
    BASELINE: "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C",
    RESULTS / f"{STEM}_face.csv": "F4C2F19794D884E3371E651C7DD7FC616D6CDF513B23804CD3224D2DFA3F3BC1",
    RESULTS / f"{STEM}_face.json": "646145CFDCF1FAD622C7149AEE21EE060ECD2752D788E5D69835DC6AC50DA1F4",
    RESULTS / f"{STEM}_edge.csv": "F73592E08B1FB3FC7F000813D02F9AF8A89350CEF00ADFCDF5B2A3A1F9ED6876",
    RESULTS / f"{STEM}_edge.json": "18FBF0E00EDDF13F34EE0EB37046C0F0FE71299AB0E7F8F4BAE9BBCF1E7D7092",
    RESULTS / f"{STEM}_body.csv": "87B76B3BE4E44DAD538CFB6988B003C5266B930EE335679046F713D9E31B48AE",
    RESULTS / f"{STEM}_body.json": "A97262B5CDB70FB77B192CA74E4A41A73287F121CE0BB37A42488DC178B7F821",
}

RAYS = {"face": "0_0_1", "edge": "0_1_-1", "body": "1_1_1"}
EXPECTED_VERDICT = {
    "face": "CAUSAL_HORIZON_PREFIX_DRIFT",
    "edge": "CAUSAL_HORIZON_PREFIX_DRIFT",
    "body": "CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE",
}
EXPECTED_DISCRETE_MISMATCH = {
    "face": ({"source_entries"}, 158),
    "edge": ({"source_entries"}, 73),
    "body": (set(), 0),
}
RADII = (8, 12, 16, 24, 32, 48)
HORIZON = 312
PREFIX = 184
checks: list[tuple[str, bool]] = []


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


def number(row: dict[str, str], column: str) -> float:
    return float(row[column])


def integer(row: dict[str, str], column: str) -> int:
    return int(row[column])


def flag(row: dict[str, str], column: str) -> bool:
    return bool(integer(row, column))


for path, expected in HASHES.items():
    check(f"frozen SHA-256 {path.name}", path.is_file() and sha256(path) == expected)

with BASELINE.open(newline="", encoding="utf-8") as handle:
    baseline_rows = list(csv.DictReader(handle))

baseline = {
    (row["direction"], row["polarity"], integer(row, "tick")): row
    for row in baseline_rows
    if row["family"] == "unbound" and integer(row, "tick") <= PREFIX
}

scalar_columns = (
    "max_residual", "total_energy_residual", "recoil_defect", "speed_excess",
    "regional_residual", "outside_source_residual", "separation",
    "pair_energy", "field_energy",
) + tuple(
    f"{prefix}_{radius}"
    for radius in RADII
    for prefix in ("inside", "outside", "transport_into",
                   "source_exchange", "cumulative_outward")
)
discrete_columns = (
    "valid", "common", "regional_valid", "source_radius",
    "source_entries", "graph_inside",
)

independent: dict[str, dict[str, object]] = {}
for slug, direction in RAYS.items():
    csv_path = RESULTS / f"{STEM}_{slug}.csv"
    json_path = RESULTS / f"{STEM}_{slug}.json"
    with csv_path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    summary = json.loads(json_path.read_text(encoding="utf-8"))
    label = f"{slug}/{direction}"
    check(f"{label}: exactly 313 rows", len(rows) == HORIZON + 1)
    check(f"{label}: ticks 0..312", [integer(row, "tick") for row in rows]
          == list(range(HORIZON + 1)))
    check(f"{label}: metadata", (
        summary["ftd_id"] == "FTD-0747"
        and summary["protocol_sha256"] == HASHES[PROTOCOL]
        and summary["baseline_csv_sha256"] == HASHES[BASELINE]
        and summary["backend"] == "wsl2_cuda_matched_face_edge"
        and summary["direction"] == direction
        and summary["volume"] == 321
        and summary["horizon"] == HORIZON
        and summary["contact_tick"] == 313
    ))

    all_forward = all(
        flag(row, "valid") and flag(row, "common")
        and flag(row, "regional_valid") for row in rows
    )
    max_common = max(number(row, "max_residual") for row in rows)
    max_energy = max(number(row, "total_energy_residual") for row in rows)
    max_recoil = max(number(row, "recoil_defect") for row in rows)
    max_speed = max(number(row, "speed_excess") for row in rows)
    max_regional = max(number(row, "regional_residual") for row in rows)
    max_outside_source = max(number(row, "outside_source_residual") for row in rows)
    max_source = max(integer(row, "source_radius") for row in rows)
    pair = [number(row, "pair_energy") for row in rows]
    field = [number(row, "field_energy") for row in rows]
    balance = abs(pair[-1] - pair[0] + field[-1] - field[0])
    h0 = (
        all_forward and max_common <= 1e-10 and max_energy <= 1e-8
        and max_recoil <= 1e-9 and max_speed <= 1e-12
        and max_regional <= 1e-10 and max_outside_source <= 1e-10
        and max_source <= 3 and balance <= 1e-8 and HORIZON < 313
    )
    check(f"{label}: H0 exact execution", h0)

    mismatch_columns: set[str] = set()
    mismatch_ticks: set[int] = set()
    prefix_difference = 0.0
    for tick in range(PREFIX + 1):
        now = rows[tick]
        old = baseline[(direction, "plus_minus", tick)]
        for column in discrete_columns:
            if now[column] != old[column]:
                mismatch_columns.add(column)
                mismatch_ticks.add(tick)
        for column in scalar_columns:
            prefix_difference = max(prefix_difference,
                abs(number(now, column) - number(old, column)))
    prefix_discrete = not mismatch_columns
    prefix_scalar = prefix_difference <= 1e-10
    h1 = prefix_discrete and prefix_scalar
    print(f"GATE  {label}: H1 discrete prefix "
          f"{'PASS' if prefix_discrete else 'FAIL'}")
    expected_columns, expected_tick_count = EXPECTED_DISCRETE_MISMATCH[slug]
    check(f"{label}: H1 discrete status matches frozen record",
          prefix_discrete == (slug == "body"))
    check(f"{label}: H1 mismatch signature",
          mismatch_columns == expected_columns
          and len(mismatch_ticks) == expected_tick_count)
    check(f"{label}: H1 scalar prefix <=1e-10 ({prefix_difference:.3e})",
          prefix_scalar)

    graph = [flag(row, "graph_inside") for row in rows]
    onset = next((tick for tick in range(HORIZON + 1) if all(
        graph[later] and pair[later] < -1e-6
        for later in range(tick, HORIZON + 1)
    )), -1)
    h2 = onset >= 0 and HORIZON - onset + 1 >= 160
    check(f"{label}: H2 persistent core", h2)

    late_inside = [number(rows[tick], "inside_8") for tick in range(281, 313)]
    late_min, late_max = min(late_inside), max(late_inside)
    h3 = late_min >= 5e-4 and late_max <= 4.0 * late_min
    check(f"{label}: H3 stable near field", h3)

    outside48 = [number(row, "outside_48") for row in rows]
    first48 = next((tick for tick, value in enumerate(outside48)
                    if value > 1e-8), -1)
    h4 = (outside48[0] <= 1e-12 and max(outside48) > 1e-8
          and 0 <= first48 <= 300 and max_outside_source <= 1e-10)
    check(f"{label}: H4 radius-48 arrival at {first48}", h4)

    outward = [-number(rows[tick], "transport_into_48")
               for tick in range(first48, HORIZON + 1)]
    post = outside48[301:313]
    h5 = (h4 and min(outward) >= -1e-10 and outside48[-1] > 1e-9
          and all(value > 1e-9 for value in post))
    check(f"{label}: H5 post-arrival persistence", h5)

    if not h0:
        verdict = "CAUSAL_HORIZON_EXECUTION_INVALID"
    elif not h1:
        verdict = "CAUSAL_HORIZON_PREFIX_DRIFT"
    elif not h2:
        verdict = "CAUSAL_HORIZON_CORE_NOT_PERSISTENT"
    elif not h3:
        verdict = "CAUSAL_HORIZON_NEAR_FIELD_NOT_STABLE"
    elif not h4:
        verdict = "CAUSAL_HORIZON_R48_ARRIVAL_FAIL"
    elif not h5:
        verdict = "CAUSAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT"
    else:
        verdict = "CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE"
    check(f"{label}: independent verdict is {EXPECTED_VERDICT[slug]}",
          verdict == EXPECTED_VERDICT[slug])
    check(f"{label}: serialized loader defect isolated", (
        summary["verdict"] == "CAUSAL_HORIZON_PREFIX_DRIFT"
        and summary["prefix_scalar_difference"] is None
        and not bool(summary["prefix_discrete_pass"])
        and bool(summary["core_pass"])
        and bool(summary["near_field_pass"])
        and bool(summary["arrival_pass"])
        and bool(summary["post_arrival_pass"])
    ))
    independent[slug] = {
        "verdict": verdict, "prefix_difference": prefix_difference,
        "arrival_tick": first48, "onset_tick": onset,
        "late_min": late_min, "late_max": late_max,
        "final_outside_48": outside48[-1],
        "max_common": max_common, "max_energy": max_energy,
        "max_recoil": max_recoil,
        "discrete_mismatch_columns": sorted(mismatch_columns),
        "discrete_mismatch_ticks": len(mismatch_ticks),
    }

raw_header = BASELINE.read_bytes().split(b"\n", 1)[0]
check("CRLF baseline condition exists", raw_header.endswith(b"\r"))
check("last header is cumulative_outward_48 after CR normalization",
      raw_header.rstrip(b"\r").endswith(b"cumulative_outward_48"))
all_constructive = all(
    result["verdict"] == "CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE"
    for result in independent.values()
)
print(f"GATE  three-arm constructive conjunction "
      f"{'PASS' if all_constructive else 'FAIL'}")
check("registered three-arm conjunction reconstructs closed negative",
      not all_constructive)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0747 independent certificate: {passed}/{len(checks)} checks passed")
for slug, result in independent.items():
    print(slug, json.dumps(result, sort_keys=True))
raise SystemExit(0 if passed == len(checks) else 1)
