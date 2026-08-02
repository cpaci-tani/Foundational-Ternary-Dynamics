"""Independent serialized-record certificate for FTD-0748.

This proof reads the frozen FTD-0745 prefix baseline and the twelve FTD-0748
records.  It does not call the C++ verdict function or rerun dynamics.  A
successful process exit certifies record integrity and faithful reconstruction
of the registered mixed/negative outcome; it does not turn failed physics
gates into passing checks.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CANONICAL_CURRENT_HORIZON_CUDA_v1.md"
PREEXEC = ROOT / "docs/theory/07_assessment/AUDIT_CANONICAL_CURRENT_HORIZON_CUDA_PREEXEC_v1.md"
RUNNER = ROOT / "engine/tests/campaign_canonical_current_horizon_cuda.cpp"
AGGREGATE_HEADER = ROOT / "engine/include/ftd/eft/quadratic_coat_face_current.h"
AGGREGATE_SOURCE = ROOT / "engine/src/eft/quadratic_coat_face_current.cpp"
PARENT_RUNNER = ROOT / "engine/tests/campaign_causal_horizon_environmental_persistence_cuda.cpp"
BASELINE = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.csv"
RESULTS = ROOT / "engine/results/ftd_0748"
STEM = "ftd_0748_canonical_current_horizon_cuda_v1"

HASHES = {
    PROTOCOL: "D01039341BCA3098C9F837549A26199CCE5BB6660C84A7C86C5037D17A2B0C46",
    PREEXEC: "F5F4D8CBC37FC60A67FFA7741579F67C14F79CB7D8C9071E6C26F7E7E0DBBC47",
    RUNNER: "70948B76A359DC01A92DC2BD46289DDA1D318009B51DB63889D95413DDC2EED8",
    AGGREGATE_HEADER: "77E67E4EBC4B27F7A70B8289195EA2D3A398A862C04DA29041C4ED33B8DA7409",
    AGGREGATE_SOURCE: "DD39B5776D74F9D942F0F5BA7518ED2D4B97E42927E361D314E5C7D6F1D0F1D0",
    PARENT_RUNNER: "85E4FBE7D0A3A21EB760C3D9F173CAA9BE7F9699596A93609FABD50683462F14",
    BASELINE: "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C",
    RESULTS / f"{STEM}_face.csv": "78B4BD60D5F910C28A2FF42DE4D102D882BE436C50FAF7FF6F5F3510967B2F66",
    RESULTS / f"{STEM}_face.json": "BBA7088709F4182D5D54654F875E460C7DCCB0758F8885B76CE3200B11192024",
    RESULTS / f"{STEM}_face_support.csv": "65F8F3C9F6CBBAF4DF1ED43FDB8C56FAC8E16B0289579BB9167295DC20C5F0D9",
    RESULTS / f"{STEM}_face_support.json": "2D724DB2A5554C8437FBD94030360C6DBFD41C8F15CDC7CB8C1D13ED28BFD450",
    RESULTS / f"{STEM}_edge.csv": "A7EF36D0BCC16CE08966EC8C17FE605492E8E835984851B1F580B0FB4D1AB728",
    RESULTS / f"{STEM}_edge.json": "7289D8EE93E048EEE7DBEFCE47933C50E2249CFA6064AE384AC8751C8FC1B2BE",
    RESULTS / f"{STEM}_edge_support.csv": "DC564895697CA87383A65631A99A9AE87EBE4F9C4E702B046A6BA065BB4EF33C",
    RESULTS / f"{STEM}_edge_support.json": "4151EC4FCCFCD99E40E9703D9E00F66455829697B09BBE69BD47089D7BB1349D",
    RESULTS / f"{STEM}_body.csv": "CE5C0F0AAA9E2E6F7568EB3CA03A5C6EB5932C25F18C384EF20F89F47A077ADD",
    RESULTS / f"{STEM}_body.json": "36E8C6C4D75E4A682A231DA0C9EA22D1CEDDE9550D74FEE17807E1EF8FFC93AF",
    RESULTS / f"{STEM}_body_support.csv": "2BDF0989E0C955D913CFD6B8586E91C02C577F4026BD4E5532560CBB0E9110B7",
    RESULTS / f"{STEM}_body_support.json": "4E0A9126C60A38C89E391B87CD0E5614A94A62BC09D7B7415B7B7922326515DB",
}

RAYS = {"face": "0_0_1", "edge": "0_1_-1", "body": "1_1_1"}
EXPECTED_VERDICT = {
    "face": "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE",
    "edge": "CANONICAL_HORIZON_PREFIX_DRIFT",
    "body": "CANONICAL_HORIZON_PREFIX_DRIFT",
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


def close(left: float, right: float, tolerance: float = 1e-15) -> bool:
    return math.isclose(left, right, rel_tol=1e-13, abs_tol=tolerance)


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
    for prefix in (
        "inside", "outside", "transport_into", "source_exchange",
        "cumulative_outward",
    )
)
discrete_columns = (
    "valid", "common", "regional_valid", "source_radius", "graph_inside",
)

independent: dict[str, dict[str, object]] = {}
for slug, direction in RAYS.items():
    csv_path = RESULTS / f"{STEM}_{slug}.csv"
    json_path = RESULTS / f"{STEM}_{slug}.json"
    support_csv_path = RESULTS / f"{STEM}_{slug}_support.csv"
    support_json_path = RESULTS / f"{STEM}_{slug}_support.json"
    with csv_path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    with support_csv_path.open(newline="", encoding="utf-8") as handle:
        support_rows = list(csv.DictReader(handle))
    summary = json.loads(json_path.read_text(encoding="utf-8"))
    support_summary = json.loads(support_json_path.read_text(encoding="utf-8"))
    label = f"{slug}/{direction}"

    expected_ticks = list(range(HORIZON + 1))
    check(f"{label}: main record has exactly 313 ordered rows", (
        len(rows) == HORIZON + 1
        and [integer(row, "tick") for row in rows] == expected_ticks
    ))
    check(f"{label}: support record has exactly 313 ordered rows", (
        len(support_rows) == HORIZON + 1
        and [integer(row, "tick") for row in support_rows] == expected_ticks
    ))
    check(f"{label}: main metadata", (
        summary["ftd_id"] == "FTD-0748"
        and summary["protocol_sha256"] == HASHES[PROTOCOL]
        and summary["baseline_csv_sha256"] == HASHES[BASELINE]
        and summary["backend"] == "wsl2_cuda_canonical_net_current"
        and summary["arm"] == slug
        and summary["direction"] == direction
        and summary["polarity"] == "plus_minus"
        and summary["volume"] == 321
        and summary["horizon"] == HORIZON
        and summary["contact_tick"] == 313
    ))
    check(f"{label}: support metadata", (
        support_summary["ftd_id"] == "FTD-0748"
        and support_summary["protocol_sha256"] == HASHES[PROTOCOL]
        and support_summary["backend"] == "wsl2_cuda_canonical_net_current"
        and support_summary["arm"] == slug
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
        bool(summary["initialized"]) and bool(summary["preparation_pass"])
        and bool(summary["initial_pass"]) and bool(summary["forward_executed"])
        and all_forward and max_common <= 1e-10 and max_energy <= 1e-8
        and max_recoil <= 1e-9 and max_speed <= 1e-12
        and max_regional <= 1e-10 and max_outside_source <= 1e-10
        and max_source <= 3 and balance <= 1e-8 and HORIZON < 313
    )
    check(f"{label}: H0 execution", h0)

    max_moment = max(number(row, "moment_residual") for row in support_rows)
    max_discarded = max(number(row, "discarded_l1") for row in support_rows)
    max_support = max(integer(row, "net_support") for row in support_rows)
    support_alignment = all(
        integer(support, "net_support") == integer(main, "source_entries")
        and integer(support, "source_radius") == integer(main, "source_radius")
        for main, support in zip(rows, support_rows)
    )
    a0 = (
        all(flag(row, "valid") for row in support_rows)
        and max_moment <= 1e-12 and max_discarded <= 1e-10
        and max(integer(row, "source_radius") for row in support_rows) <= 3
        and support_alignment
    )
    check(f"{label}: A0 canonical aggregation", a0)
    check(f"{label}: support summary reconstructs", (
        bool(support_summary["aggregation_pass"]) == a0
        and support_summary["maximum_net_support"] == max_support
        and close(support_summary["maximum_discarded_l1"], max_discarded)
        and close(support_summary["maximum_moment_residual"], max_moment)
    ))

    mismatch_columns: set[str] = set()
    mismatch_ticks: set[int] = set()
    prefix_difference = 0.0
    prefix_location = (-1, "")
    for tick in range(PREFIX + 1):
        now = rows[tick]
        old = baseline[(direction, "plus_minus", tick)]
        for column in discrete_columns:
            if now[column] != old[column]:
                mismatch_columns.add(column)
                mismatch_ticks.add(tick)
        for column in scalar_columns:
            difference = abs(number(now, column) - number(old, column))
            if difference > prefix_difference:
                prefix_difference = difference
                prefix_location = (tick, column)
    h1_discrete = not mismatch_columns
    h1_scalar = prefix_difference <= 1e-10
    h1 = h1_discrete and h1_scalar
    check(f"{label}: H1 corrected discrete prefix", h1_discrete)
    print(
        f"GATE  {label}: H1 scalar prefix "
        f"{'PASS' if h1_scalar else 'FAIL'} ({prefix_difference:.12e}, "
        f"tick={prefix_location[0]}, column={prefix_location[1]})"
    )
    check(f"{label}: H1 scalar status matches frozen mixed record", (
        h1_scalar == (slug == "face")
    ))

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
    h4 = (
        outside48[0] <= 1e-12 and max(outside48) > 1e-8
        and 0 <= first48 <= 300 and max_outside_source <= 1e-10
    )
    check(f"{label}: H4 radius-48 arrival at {first48}", h4)

    outward = [-number(rows[tick], "transport_into_48")
               for tick in range(first48, HORIZON + 1)]
    post = outside48[301:313]
    h5 = (
        h4 and min(outward) >= -1e-10 and outside48[-1] > 1e-9
        and all(value > 1e-9 for value in post)
    )
    check(f"{label}: H5 post-arrival persistence", h5)

    if not h0:
        verdict = "CANONICAL_HORIZON_EXECUTION_INVALID"
    elif not a0:
        verdict = "CANONICAL_HORIZON_CURRENT_AGGREGATION_INVALID"
    elif not h1:
        verdict = "CANONICAL_HORIZON_PREFIX_DRIFT"
    elif not h2:
        verdict = "CANONICAL_HORIZON_CORE_NOT_PERSISTENT"
    elif not h3:
        verdict = "CANONICAL_HORIZON_NEAR_FIELD_NOT_STABLE"
    elif not h4:
        verdict = "CANONICAL_HORIZON_R48_ARRIVAL_FAIL"
    elif not h5:
        verdict = "CANONICAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT"
    else:
        verdict = "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE"
    check(f"{label}: ordered verdict reconstructs", (
        verdict == EXPECTED_VERDICT[slug] == summary["verdict"]
    ))
    check(f"{label}: serialized gate summary reconstructs", (
        bool(summary["exact_pass"]) == h0
        and bool(summary["support_pass"]) == a0
        and bool(summary["prefix_discrete_pass"]) == h1_discrete
        and bool(summary["prefix_pass"]) == h1
        and bool(summary["core_pass"]) == h2
        and bool(summary["near_field_pass"]) == h3
        and bool(summary["arrival_pass"]) == h4
        and bool(summary["post_arrival_pass"]) == h5
        and close(summary["prefix_scalar_difference"], prefix_difference)
    ))

    independent[slug] = {
        "verdict": verdict,
        "prefix_difference": prefix_difference,
        "prefix_location": prefix_location,
        "discrete_mismatch_columns": sorted(mismatch_columns),
        "discrete_mismatch_ticks": len(mismatch_ticks),
        "maximum_net_support": max_support,
        "maximum_moment_residual": max_moment,
        "maximum_discarded_l1": max_discarded,
        "arrival_tick": first48,
        "onset_tick": onset,
        "late_min": late_min,
        "late_max": late_max,
        "final_outside_48": outside48[-1],
        "max_common": max_common,
        "max_energy": max_energy,
        "max_recoil": max_recoil,
    }

all_constructive = all(
    result["verdict"]
    == "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE"
    for result in independent.values()
)
print(
    "GATE  three-arm constructive conjunction "
    f"{'PASS' if all_constructive else 'FAIL'}"
)
check("registered three-arm conjunction reconstructs closed negative",
      not all_constructive)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0748 independent certificate: {passed}/{len(checks)} checks passed")
for slug, result in independent.items():
    print(slug, json.dumps(result, sort_keys=True))
raise SystemExit(0 if passed == len(checks) else 1)
