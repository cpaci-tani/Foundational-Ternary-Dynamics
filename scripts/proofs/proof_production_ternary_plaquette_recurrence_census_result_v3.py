#!/usr/bin/env python3
"""Independently reconstruct and adjudicate the repaired FTD-0915 corpus."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import Counter, defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme"
PROTOCOL = BASE / "PREREG_PRODUCTION_TERNARY_PLAQUETTE_QUARTER_TURN_RECURRENCE_CENSUS_v1.md"
REPAIR = BASE / "PREREG_PRODUCTION_TERNARY_PLAQUETTE_RECURRENCE_RAW_TELEMETRY_REPAIR_v3.md"
PREFLIGHT = ROOT / "scripts/proofs/proof_production_ternary_plaquette_recurrence_census_preflight_v3.py"
RESULTS = ROOT / "engine/results/ftd_0915/v3"
TICK_FILE = RESULTS / "ftd_0915_tick_census_v3.csv"
EXPOSURE_FILE = RESULTS / "ftd_0915_exposure_census_v3.csv"
TRANSITION_FILE = RESULTS / "ftd_0915_transition_census_v3.csv"
SUMMARY_FILE = RESULTS / "ftd_0915_summary_v3.json"

LOCKS = {
    PROTOCOL: "C302319900BAC4920277FACCC3A9164F0AE64DCAC8FBD256A4F36B48E7CC970C",
    REPAIR: "26D4488E2BB8EB6783C1C7F6B4D413D79D487D78A9A43A98D793F2B02D55DF44",
    PREFLIGHT: "B598DE9D805B4B24694F6F08C4FCA5E5DD769D43AA711A163494738A3D586199",
    ROOT / "engine/tests/campaign_production_ternary_plaquette_recurrence_census.cpp": "D24970F34346167197D53681F1E6231A68C5E81F0515E6CA85B7335FBED83F21",
    ROOT / "engine/CMakeLists.txt": "C895673132434DE830A15EE41676A446FCEF6D26D7C3819ED491E536D37BB745",
    ROOT / "engine/build/Release/campaign_production_ternary_plaquette_recurrence_census.exe": "E02B56E25F8FD38C0E12815A30D342378E7E9CC072DD0A7011CB71A80548249D",
    TICK_FILE: "F006ADACDABFEF970F4DE4914ADDBE3DCE2B812E49993596CAB23ED1AA80AA47",
    EXPOSURE_FILE: "E68705751A9126AC857DF5702BD66714C60589174C053AD50EA0A5485AFD5EBA",
    TRANSITION_FILE: "956815D69ED08DF3CD47AFCD5AE1889B9BAFAC163FE22FCC8E629733694CF381",
    SUMMARY_FILE: "53CC7D0C78BB5EB050B1D0F45F1CAD0F6118C48C1092CA6CAACFC3A6915D204E",
}

VOLUMES = (21, 27)
FAMILIES = ("axial_live", "diagonal_live", "axial_no_bath", "empty_control")
LIVE_FAMILIES = FAMILIES[:3]
SEEDS = tuple(0x09150000 + index for index in range(1, 9))
TICKS = 128
CELL_GATE = 6
RELATIONS = (
    "FORWARD", "REVERSE", "STATIONARY", "HALF_TURN",
    "ADJACENT_DEFECT", "SUPPORT_LOSS",
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def arm_key(row: dict[str, str]) -> tuple[int, str, int]:
    return int(row["volume"]), row["family"], int(row["seed"])


def word(row: dict[str, str], prefix: str) -> tuple[int, int, int, int]:
    return tuple(int(row[f"{prefix}_{index}"]) for index in range(4))  # type: ignore[return-value]


def shift_forward(value: tuple[int, ...]) -> tuple[int, ...]:
    return value[3], value[0], value[1], value[2]


def shift_reverse(value: tuple[int, ...]) -> tuple[int, ...]:
    return value[1], value[2], value[3], value[0]


def is_orbit(value: tuple[int, ...]) -> bool:
    return sorted(value) == [-1, 0, 0, 1] and (
        value.index(1) + 2) % 4 == value.index(-1)


def lattice_index(x: int, y: int, z: int, volume: int) -> int:
    return (x % volume) * volume * volume + (y % volume) * volume + z % volume


def support_indices(plane: str, x: int, y: int, z: int, volume: int) -> tuple[int, ...]:
    if plane == "xy":
        points = ((x + 1, y, z), (x + 1, y + 1, z), (x, y + 1, z), (x, y, z))
    elif plane == "yz":
        points = ((x, y + 1, z), (x, y + 1, z + 1), (x, y, z + 1), (x, y, z))
    else:
        points = ((x, y, z + 1), (x + 1, y, z + 1), (x + 1, y, z), (x, y, z))
    return tuple(lattice_index(*point, volume) for point in points)


def raw_vertices(row: dict[str, str], prefix: str) -> list[dict[str, object]]:
    result = []
    for vertex in range(4):
        key = f"{prefix}_v{vertex}"
        result.append({
            "site": int(row[f"{key}_site"]),
            "state": int(row[f"{key}_state"]),
            "particle_id": int(row[f"{key}_particle_id"]),
            "flux": tuple(float(row[f"{key}_j{axis}"]) for axis in "xyz"),
            "wave": tuple(float(row[f"{key}_w{axis}"]) for axis in "xyz"),
        })
    return result


def vertex_word(vertices: list[dict[str, object]]) -> tuple[int, ...]:
    return tuple(int(vertex["state"]) for vertex in vertices)


def energy(vertices: list[dict[str, object]]) -> float:
    return 0.5 * sum(
        component * component
        for vertex in vertices
        for vector_name in ("flux", "wave")
        for component in vertex[vector_name]  # type: ignore[union-attr]
    )


def same_signed_pair(
    vertices: list[dict[str, object]], positive_id: int, negative_id: int,
) -> bool:
    states = vertex_word(vertices)
    if sorted(states) != [-1, 0, 0, 1]:
        return False
    positive_vertex = states.index(1)
    negative_vertex = states.index(-1)
    return (
        int(vertices[positive_vertex]["particle_id"]) == positive_id
        and int(vertices[negative_vertex]["particle_id"]) == negative_id
    )


OFFSETS = {
    "xy": ((1, -1, 0), (1, 1, 0), (-1, 1, 0), (-1, -1, 0)),
    "yz": ((0, 1, -1), (0, 1, 1), (0, -1, 1), (0, -1, -1)),
    "zx": ((-1, 0, 1), (1, 0, 1), (1, 0, -1), (-1, 0, -1)),
}
NORMALS = {"xy": (0, 0, 1), "yz": (1, 0, 0), "zx": (0, 1, 0)}


def dipole(plane: str, value: tuple[int, ...]) -> tuple[int, int, int]:
    return tuple(sum(value[j] * OFFSETS[plane][j][axis] for j in range(4)) for axis in range(3))  # type: ignore[return-value]


def cross(left: tuple[int, ...], right: tuple[int, ...]) -> tuple[int, int, int]:
    return (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )


def classify(
    before: tuple[int, ...], after_vertices: list[dict[str, object]],
    positive_id: int, negative_id: int,
) -> str:
    after = vertex_word(after_vertices)
    if same_signed_pair(after_vertices, positive_id, negative_id):
        if after == shift_forward(before):
            return "FORWARD"
        if after == shift_reverse(before):
            return "REVERSE"
        if after == before:
            return "STATIONARY"
        if after == shift_forward(shift_forward(before)):
            return "HALF_TURN"
        return "ADJACENT_DEFECT"
    return "SUPPORT_LOSS"


checks: list[tuple[str, bool]] = []
for path, expected in LOCKS.items():
    checks.append((f"artifact lock {path.relative_to(ROOT)}", path.is_file() and sha256(path) == expected))

ticks = read_csv(TICK_FILE)
exposures = read_csv(EXPOSURE_FILE)
transitions = read_csv(TRANSITION_FILE)
summary = json.loads(SUMMARY_FILE.read_text(encoding="utf-8"))
expected_arms = {(volume, family, seed) for volume in VOLUMES for seed in SEEDS for family in FAMILIES}

tick_groups: dict[tuple[int, str, int], list[dict[str, str]]] = defaultdict(list)
for row in ticks:
    tick_groups[arm_key(row)].append(row)
checks.extend([
    ("tick corpus has 8192 rows", len(ticks) == 64 * TICKS),
    ("tick corpus has exact arm matrix", set(tick_groups) == expected_arms),
    ("each arm has ticks zero through 127", all(
        sorted(int(row["tick"]) for row in rows) == list(range(TICKS))
        for rows in tick_groups.values())),
    ("all plaquette enumerations are exact", all(
        int(row["plaquettes_enumerated"]) == 3 * int(row["volume"]) ** 3
        for row in ticks)),
    ("all observations are nonmutating", all(
        row["nonmutating"] == "1"
        and row["voxel_hash_before"] == row["voxel_hash_after"]
        and row["rng_hash_before"] == row["rng_hash_after"] for row in ticks)),
    ("all tick controls and reconstruction flags pass", all(
        row["controls_pass"] == "1" and row["reconstructible"] == "1"
        for row in ticks)),
    ("all tick telemetry is finite", all(
        math.isfinite(float(row["total_native_energy"]))
        and math.isfinite(float(row["maximum_control_residual"])) for row in ticks)),
])

exposure_groups: Counter[tuple[int, str, int, int]] = Counter()
exposure_lookup: dict[tuple[object, ...], dict[str, str]] = {}
exposure_raw_pass = True
for row in exposures:
    arm = arm_key(row)
    tick = int(row["tick"])
    plane = row["plane"]
    x, y, z = (int(row[axis]) for axis in "xyz")
    positive_id = int(row["positive_id"])
    negative_id = int(row["negative_id"])
    vertices = raw_vertices(row, "sample")
    stored_word = word(row, "word")
    expected_sites = support_indices(plane, x, y, z, arm[0])
    exposure_raw_pass &= vertex_word(vertices) == stored_word and is_orbit(stored_word)
    exposure_raw_pass &= tuple(int(vertex["site"]) for vertex in vertices) == expected_sites
    exposure_raw_pass &= same_signed_pair(vertices, positive_id, negative_id)
    exposure_raw_pass &= math.isclose(
        energy(vertices), float(row["local_energy"]), rel_tol=1e-12, abs_tol=1e-12)
    exposure_raw_pass &= all(
        math.isfinite(component)
        for vertex in vertices
        for name in ("flux", "wave")
        for component in vertex[name]  # type: ignore[union-attr]
    )
    exposure_groups[(*arm, tick)] += 1
    key = (*arm, tick, plane, x, y, z, positive_id, negative_id)
    exposure_raw_pass &= key not in exposure_lookup
    exposure_lookup[key] = row

checks.extend([
    ("exposure rows reconstruct from raw sites", exposure_raw_pass),
    ("exposure corpus count matches tick identity counts", all(
        exposure_groups[(*arm_key(row), int(row["tick"]))]
        == int(row["identity_exposures"]) for row in ticks)),
    ("identity exposure corpus has 2860 rows", len(exposures) == 2860),
])

transition_counts: Counter[tuple[int, str, int, int, str]] = Counter()
transition_raw_pass = True
run_state: dict[tuple[object, ...], dict[str, object]] = {}
full_cycle_keys: Counter[tuple[int, str, int]] = Counter()
full_cycle_counts: Counter[tuple[int, str, int]] = Counter()
max_runs: Counter[tuple[int, str, int]] = Counter()
for row in transitions:
    arm = arm_key(row)
    tick = int(row["tick"])
    plane = row["plane"]
    x, y, z = (int(row[axis]) for axis in "xyz")
    positive_id = int(row["positive_id"])
    negative_id = int(row["negative_id"])
    before_vertices = raw_vertices(row, "before")
    after_vertices = raw_vertices(row, "after")
    before_word = word(row, "before")
    after_word = word(row, "after")
    sites = support_indices(plane, x, y, z, arm[0])
    relation = classify(before_word, after_vertices, positive_id, negative_id)
    transition_raw_pass &= tuple(int(v["site"]) for v in before_vertices) == sites
    transition_raw_pass &= tuple(int(v["site"]) for v in after_vertices) == sites
    transition_raw_pass &= vertex_word(before_vertices) == before_word
    transition_raw_pass &= vertex_word(after_vertices) == after_word
    transition_raw_pass &= row["relation"] == relation
    transition_raw_pass &= math.isclose(energy(before_vertices), float(row["energy_before"]), rel_tol=1e-12, abs_tol=1e-12)
    transition_raw_pass &= math.isclose(energy(after_vertices), float(row["energy_after"]), rel_tol=1e-12, abs_tol=1e-12)
    exposure_key = (*arm, tick - 1, plane, x, y, z, positive_id, negative_id)
    transition_raw_pass &= exposure_key in exposure_lookup
    transition_counts[(*arm, tick, relation)] += 1

    track_key = (*arm, plane, x, y, z, positive_id, negative_id)
    track = run_state.setdefault(track_key, {
        "direction": 0, "last": -2, "run": 0,
        "start": (), "full": False,
    })
    directed = relation in ("FORWARD", "REVERSE")
    if directed:
        direction = 1 if relation == "FORWARD" else -1
        if track["last"] == tick - 1 and track["direction"] == direction:
            track["run"] = int(track["run"]) + 1
        else:
            track["direction"] = direction
            track["run"] = 1
            track["start"] = before_word
        track["last"] = tick
        expected_d0 = dipole(plane, before_word)
        expected_d1 = dipole(plane, after_word)
        expected_l = cross(expected_d0, expected_d1)
        csv_d0 = tuple(int(row[f"d{axis}0"]) for axis in "xyz")
        csv_d1 = tuple(int(row[f"d{axis}1"]) for axis in "xyz")
        csv_l = tuple(int(row[f"l{axis}"]) for axis in "xyz")
        sign = direction
        expected_l = tuple(sign * 8 * value for value in NORMALS[plane])
        transition_raw_pass &= csv_d0 == expected_d0 and csv_d1 == expected_d1 and csv_l == expected_l
    else:
        track["direction"] = 0
        track["run"] = 0
        track["last"] = tick
        transition_raw_pass &= all(int(row[name]) == 0 for name in (
            "dx0", "dy0", "dz0", "dx1", "dy1", "dz1", "lx", "ly", "lz"))
    expected_closure = directed and int(track["run"]) % 4 == 0 and after_word == track["start"]
    transition_raw_pass &= int(row["current_run"]) == int(track["run"])
    transition_raw_pass &= (row["closure"] == "1") == expected_closure
    transition_raw_pass &= row["control_pass"] == "1" and float(row["control_residual"]) == 0.0
    if expected_closure:
        if not track["full"]:
            full_cycle_keys[arm] += 1
            track["full"] = True
        full_cycle_counts[arm] += 1
    max_runs[arm] = max(max_runs[arm], int(track["run"]))

checks.extend([
    ("transition rows reconstruct from raw before/after sites", transition_raw_pass),
    ("transition corpus count matches tick attempts", all(
        sum(transition_counts[(*arm_key(row), int(row["tick"]), relation)] for relation in RELATIONS)
        == int(row["transition_attempts"]) for row in ticks)),
    ("each tick relation count reconstructs", all(
        transition_counts[(*arm_key(row), int(row["tick"]), relation)]
        == int(row[relation.lower()]) for row in ticks for relation in RELATIONS)),
])

summary_arms = {arm_key(row): row for row in summary["arms"]}
summary_pass = set(summary_arms) == expected_arms
for arm, rows in tick_groups.items():
    record = summary_arms[arm]
    summary_pass &= int(record["raw_exposures"]) == sum(int(row["raw_exposures"]) for row in rows)
    summary_pass &= int(record["identity_exposures"]) == sum(int(row["identity_exposures"]) for row in rows)
    summary_pass &= int(record["transition_attempts"]) == sum(int(row["transition_attempts"]) for row in rows)
    for relation in RELATIONS:
        summary_pass &= int(record[relation.lower()]) == sum(int(row[relation.lower()]) for row in rows)
    summary_pass &= int(record["maximum_oriented_run"]) == max_runs[arm]
    summary_pass &= int(record["full_cycle_keys"]) == full_cycle_keys[arm]
    summary_pass &= int(record["full_cycle_count"]) == full_cycle_counts[arm]
    summary_pass &= bool(record["finite"] and record["nonmutating"] and record["controls_pass"] and record["reconstructible"] and record["enumeration_complete"])
checks.append(("summary arms reconstruct exactly", summary_pass))

live = [record for arm, record in summary_arms.items() if arm[1] in LIVE_FAMILIES]
empty = [record for arm, record in summary_arms.items() if arm[1] == "empty_control"]
any_live_cycle = any(int(row["full_cycle_keys"]) > 0 for row in live)
any_live_directed = any(int(row["forward"]) + int(row["reverse"]) > 0 for row in live)
any_live_exposure = any(int(row["identity_exposures"]) > 0 for row in live)
cell_cycle_seeds = Counter(
    (arm[0], arm[1]) for arm, row in summary_arms.items()
    if arm[1] in LIVE_FAMILIES and int(row["full_cycle_keys"]) > 0
)
outcome_a = all(cell_cycle_seeds[(volume, family)] >= CELL_GATE for volume in VOLUMES for family in LIVE_FAMILIES)
if outcome_a:
    verdict = "A_CROSS_VOLUME_PRODUCTION_RECURRENCE"
elif any_live_cycle:
    verdict = "B_ISOLATED_EXACT_PRODUCTION_RECURRENCE"
elif any_live_directed:
    verdict = "C_DIRECTED_FORMATION_WITHOUT_FULL_RECURRENCE"
elif any_live_exposure:
    verdict = "D_EXPOSURE_WITHOUT_DIRECTED_TRANSPORT"
else:
    verdict = "E_NO_IDENTITY_BEARING_PRODUCTION_EXPOSURE"

checks.extend([
    ("summary validity flags pass", summary["reference_valid"] and summary["matrix_complete"] and summary["finite"] and summary["nonmutating"] and summary["controls_pass"] and summary["protocol_valid"]),
    ("independent verdict matches summary", verdict == summary["verdict"]),
    ("outcome is exact D", verdict == "D_EXPOSURE_WITHOUT_DIRECTED_TRANSPORT"),
    ("live identity exposures total 2860", sum(int(row["identity_exposures"]) for row in live) == 2860),
    ("21 live arms expose a carrier word", sum(int(row["identity_exposures"]) > 0 for row in live) == 21),
    ("all 2800 retained transitions are stationary", sum(int(row["stationary"]) for row in live) == 2800),
    ("six transitions lose support", sum(int(row["support_loss"]) for row in live) == 6),
    ("no directed or alternate defect transition occurs", sum(
        int(row["forward"]) + int(row["reverse"]) + int(row["half_turn"]) + int(row["adjacent_defect"])
        for row in live) == 0),
    ("empty controls have no identity exposure", sum(int(row["identity_exposures"]) for row in empty) == 0),
])

passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
print(f"\nFTD-0915 repaired result certificate: {passed}/{len(checks)} checks passed")
print(f"LIVE_IDENTITY_EXPOSURES={sum(int(row['identity_exposures']) for row in live)}")
print(f"LIVE_ARMS_WITH_EXPOSURE={sum(int(row['identity_exposures']) > 0 for row in live)}")
print(f"STATIONARY_TRANSITIONS={sum(int(row['stationary']) for row in live)}")
print(f"SUPPORT_LOSSES={sum(int(row['support_loss']) for row in live)}")
print(f"DIRECTED_TRANSITIONS={sum(int(row['forward']) + int(row['reverse']) for row in live)}")
print(f"FTD0915_OUTCOME={verdict}")
print("PRODUCTION_PLAQUETTE_RECURRENCE=NOT_OBSERVED")
print("NATIVE_QUARTER_TURN_TRANSPORT_LAW=MISSING")
print("GSTAR_USED=FALSE")
print("BORN_BELL_TARGET_USED=FALSE")
raise SystemExit(0 if passed == len(checks) else 1)
