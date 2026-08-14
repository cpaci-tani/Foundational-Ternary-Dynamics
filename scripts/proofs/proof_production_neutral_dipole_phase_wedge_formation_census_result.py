#!/usr/bin/env python3
"""Independently reconstruct and adjudicate the locked FTD-0908 census."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0908"
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_PRODUCTION_NEUTRAL_DIPOLE_PHASE_WEDGE_FORMATION_CENSUS_v1.md"
)
RUNNER = ROOT / "engine/tests/campaign_production_orientation_memory_census.cpp"

PROTOCOL_SHA256 = "53348A90021C609E3EBA5DC7D565F6EA78832498C206D0D4B3F1964CCC7C4993"
RUNNER_SHA256 = "4FBA0AF9F02440CCA7B166BFFD1A5C2875B18D86B4E402E004F23C4412CB9F34"
SOURCE_LOCKS = {
    PROTOCOL: PROTOCOL_SHA256,
    RUNNER: RUNNER_SHA256,
    ROOT / "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/include/ftd/render_bridge.h":
        "560CB59E2FCD6E174640CA6BF048FD16AEC36AD2B13EE97FA31E301CF373D91C",
    ROOT / "engine/src/render_bridge.cpp":
        "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/include/ftd/constants.h":
        "5C9E4EA46DE1D5E0BF4479AA9E115520E70B729E7E81335FCEF08CE99704BAB0",
    ROOT / "engine/include/ftd/eft/native_ternary_dipole_phase_wedge_memory.h":
        "BADAE9D26E5FED6FCD4317A7534648256AFF051E2CAADB7E6BEEA00603AEDF46",
    ROOT / "engine/src/eft/native_ternary_dipole_phase_wedge_memory.cpp":
        "AA021926D1DE32AE9D04FB72682379DBB7F6CD3A1BB150AADBA6A957DFBF20B5",
}

TICKS = 96
PERSISTENCE_TICKS = 8
TOLERANCE = 1e-11
CONTROL_FACTOR = 256.0
VOLUMES = (17, 25)
SEEDS = (0x09080001, 0x09080002, 0x09080003, 0x09080004)
FAMILIES = ("axial_live", "diagonal_live", "axial_no_bath", "empty_control")
LIVE_FAMILIES = ("axial_live", "diagonal_live", "axial_no_bath")

TICK_FILE = RESULTS / "ftd_0908_tick_census_v1.csv"
PAIR_FILE = RESULTS / "ftd_0908_pair_observations_v1.csv"
SUMMARY_FILE = RESULTS / "ftd_0908_summary_v1.json"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(left: float, right: float, *scales: float) -> bool:
    scale = max(1.0, abs(left), abs(right), *(abs(value) for value in scales))
    return math.isfinite(left) and math.isfinite(right) and (
        abs(left - right) <= CONTROL_FACTOR * TOLERANCE * scale
    )


def as_int(row: dict[str, str], name: str) -> int:
    return int(row[name])


def as_float(row: dict[str, str], name: str) -> float:
    value = float(row[name])
    if not math.isfinite(value):
        raise ValueError(f"non-finite {name}")
    return value


def as_bool(row: dict[str, str], name: str) -> bool:
    if row[name] not in {"0", "1"}:
        raise ValueError(f"non-Boolean {name}")
    return row[name] == "1"


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def dot(left: tuple[float, float, float], right: tuple[float, float, float]) -> float:
    return sum(a * b for a, b in zip(left, right))


checks: list[tuple[str, bool]] = []
details: list[str] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, bool(condition)))
    if not condition and detail:
        details.append(f"{name}: {detail}")


for path, expected in SOURCE_LOCKS.items():
    check(
        f"source lock {path.relative_to(ROOT)}",
        path.is_file() and sha256(path) == expected,
    )

check("all three frozen result files exist", all(
    path.is_file() for path in (TICK_FILE, PAIR_FILE, SUMMARY_FILE)
))
if not all(path.is_file() for path in (TICK_FILE, PAIR_FILE, SUMMARY_FILE)):
    for name, ok in checks:
        print(f"{'PASS' if ok else 'FAIL'}  {name}")
    print("\nFTD-0908 result adjudication: campaign corpus is absent or incomplete")
    print("PRODUCTION_FORMATION_VERDICT=NOT_AVAILABLE")
    raise SystemExit(1)

try:
    tick_rows = read_csv(TICK_FILE)
    pair_rows = read_csv(PAIR_FILE)
    summary: dict[str, Any] = json.loads(SUMMARY_FILE.read_text(encoding="utf-8"))
except (OSError, ValueError, csv.Error, json.JSONDecodeError) as error:
    check("result corpus parses", False, str(error))
    tick_rows = []
    pair_rows = []
    summary = {}
else:
    check("result corpus parses", True)

expected_arms = {
    (volume, family, seed)
    for volume in VOLUMES
    for seed in SEEDS
    for family in FAMILIES
}

tick_by_arm: dict[tuple[int, str, int], dict[int, dict[str, str]]] = defaultdict(dict)
tick_parse_ok = True
for row in tick_rows:
    try:
        arm = (as_int(row, "volume"), row["family"], as_int(row, "seed"))
        tick = as_int(row, "tick")
        if tick in tick_by_arm[arm]:
            tick_parse_ok = False
        tick_by_arm[arm][tick] = row
        for name in (
            "positive_count", "negative_count", "genesis_events",
            "evaporation_events", "valid_pairs", "chi_positive",
            "chi_negative", "longest_current_run", "randomized_valid_pairs",
        ):
            if as_int(row, name) < 0:
                tick_parse_ok = False
        for name in (
            "max_abs_ell", "rms_ell", "native_wave_energy",
            "randomized_max_abs_ell", "worst_control_residual",
        ):
            if as_float(row, name) < 0.0:
                tick_parse_ok = False
        if not all(as_bool(row, name) for name in (
            "nonmutating", "controls_pass", "reconstructible"
        )):
            tick_parse_ok = False
        if row["voxel_hash_before"] != row["voxel_hash_after"]:
            tick_parse_ok = False
        if row["rng_hash_before"] != row["rng_hash_after"]:
            tick_parse_ok = False
    except (KeyError, ValueError) as error:
        tick_parse_ok = False
        details.append(f"tick row: {error}")

matrix_ok = (
    set(tick_by_arm) == expected_arms
    and len(tick_rows) == len(expected_arms) * TICKS
    and all(set(rows) == set(range(TICKS)) for rows in tick_by_arm.values())
)
check("tick matrix is exactly 32 arms by 96 ticks", matrix_ok)
check("tick telemetry is finite, nonnegative, read-only, and control-clean", tick_parse_ok)

pair_by_arm_tick: dict[
    tuple[tuple[int, str, int], int], list[dict[str, str]]
] = defaultdict(list)
histories: dict[
    tuple[int, str, int], dict[tuple[int, int], list[tuple[int, int]]]
] = defaultdict(lambda: defaultdict(list))
pair_keys_by_arm: dict[tuple[int, str, int], set[tuple[int, int]]] = defaultdict(set)
pair_reconstruction_ok = True
pair_control_ok = True
seen_pair_tick: set[tuple[int, str, int, int, int, int]] = set()

for row in pair_rows:
    try:
        volume = as_int(row, "volume")
        family = row["family"]
        seed = as_int(row, "seed")
        tick = as_int(row, "tick")
        arm = (volume, family, seed)
        positive_id = as_int(row, "positive_id")
        negative_id = as_int(row, "negative_id")
        positive_site = as_int(row, "positive_site")
        negative_site = as_int(row, "negative_site")
        chi = as_int(row, "chi")
        separation = tuple(as_float(row, name) for name in ("dx", "dy", "dz"))
        positive_flux = tuple(as_float(row, name) for name in ("jpx", "jpy", "jpz"))
        negative_flux = tuple(as_float(row, name) for name in ("jmx", "jmy", "jmz"))
        positive_wave = tuple(as_float(row, name) for name in ("wpx", "wpy", "wpz"))
        negative_wave = tuple(as_float(row, name) for name in ("wmx", "wmy", "wmz"))
        q_plus = as_float(row, "q_plus")
        q_minus = as_float(row, "q_minus")
        p_plus = as_float(row, "p_plus")
        p_minus = as_float(row, "p_minus")
        ell = as_float(row, "ell")

        duplicate_key = (volume, family, seed, tick, positive_id, negative_id)
        if duplicate_key in seen_pair_tick:
            pair_reconstruction_ok = False
        seen_pair_tick.add(duplicate_key)

        norm = math.sqrt(dot(separation, separation))
        if arm not in expected_arms or tick not in range(TICKS):
            pair_reconstruction_ok = False
        if positive_id < 0 or negative_id < 0:
            pair_reconstruction_ok = False
        if not (0 <= positive_site < volume ** 3 and 0 <= negative_site < volume ** 3):
            pair_reconstruction_ok = False
        if not close(max(abs(value) for value in separation), 1.0):
            pair_reconstruction_ok = False
        if norm <= TOLERANCE:
            pair_reconstruction_ok = False
            continue
        axis = tuple(value / norm for value in separation)
        q_plus_rebuilt = dot(axis, positive_flux)
        q_minus_rebuilt = dot(axis, negative_flux)
        p_plus_rebuilt = dot(axis, positive_wave)
        p_minus_rebuilt = dot(axis, negative_wave)
        ell_rebuilt = q_plus_rebuilt * p_minus_rebuilt - q_minus_rebuilt * p_plus_rebuilt
        wedge_scale = max(
            1.0,
            abs(q_plus_rebuilt * p_minus_rebuilt),
            abs(q_minus_rebuilt * p_plus_rebuilt),
        )
        expected_chi = 1 if ell_rebuilt > 0.0 else -1
        valid_nonzero = abs(ell_rebuilt) > TOLERANCE * wedge_scale
        if not all((
            close(q_plus, q_plus_rebuilt), close(q_minus, q_minus_rebuilt),
            close(p_plus, p_plus_rebuilt), close(p_minus, p_minus_rebuilt),
            close(ell, ell_rebuilt), valid_nonzero, chi == expected_chi,
        )):
            pair_reconstruction_ok = False

        gram_pp = q_plus * q_plus + p_plus * p_plus
        gram_mm = q_minus * q_minus + p_minus * p_minus
        gram_pm = q_plus * q_minus + p_plus * p_minus
        gram_det = gram_pp * gram_mm - gram_pm * gram_pm
        if not close(gram_det, ell * ell):
            pair_control_ok = False
        # Signed-cubic covariance and inversion invariance reduce to dot-product
        # preservation under the frozen orthogonal maps. Canonical W -> -W
        # reverses the independently reconstructed wedge and chirality.
        signed_cubic = lambda value: (-value[1], value[2], -value[0])
        negate = lambda value: (-value[0], -value[1], -value[2])
        transformed_axis = signed_cubic(axis)
        cubic_ell = dot(transformed_axis, signed_cubic(positive_flux)) * dot(
            transformed_axis, signed_cubic(negative_wave)
        ) - dot(transformed_axis, signed_cubic(negative_flux)) * dot(
            transformed_axis, signed_cubic(positive_wave)
        )
        inverted_axis = negate(axis)
        inversion_ell = dot(inverted_axis, negate(positive_flux)) * dot(
            inverted_axis, negate(negative_wave)
        ) - dot(inverted_axis, negate(negative_flux)) * dot(
            inverted_axis, negate(positive_wave)
        )
        reversed_ell = q_plus * (-p_minus) - q_minus * (-p_plus)
        if not all((
            close(cubic_ell, ell), close(inversion_ell, ell),
            close(reversed_ell, -ell),
        )):
            pair_control_ok = False

        pair_by_arm_tick[(arm, tick)].append(row)
        key = (positive_id, negative_id)
        histories[arm][key].append((tick, chi))
        pair_keys_by_arm[arm].add(key)
    except (KeyError, ValueError, ZeroDivisionError) as error:
        pair_reconstruction_ok = False
        details.append(f"pair row: {error}")

check("each pair record reconstructs from endpoint fields and IDs", pair_reconstruction_ok)
check("independent algebraic controls pass", pair_control_ok)

tick_pair_counts_ok = True
randomized_ok = True
for arm in expected_arms:
    for tick in range(TICKS):
        tick_row = tick_by_arm.get(arm, {}).get(tick)
        if tick_row is None:
            tick_pair_counts_ok = False
            continue
        rows = pair_by_arm_tick.get((arm, tick), [])
        try:
            if as_int(tick_row, "valid_pairs") != len(rows):
                tick_pair_counts_ok = False
            chi_positive = sum(as_int(row, "chi") > 0 for row in rows)
            chi_negative = sum(as_int(row, "chi") < 0 for row in rows)
            if as_int(tick_row, "chi_positive") != chi_positive:
                tick_pair_counts_ok = False
            if as_int(tick_row, "chi_negative") != chi_negative:
                tick_pair_counts_ok = False
            maximum_abs = max((abs(as_float(row, "ell")) for row in rows), default=0.0)
            rms = math.sqrt(sum(as_float(row, "ell") ** 2 for row in rows) / len(rows)) if rows else 0.0
            if not close(as_float(tick_row, "max_abs_ell"), maximum_abs):
                tick_pair_counts_ok = False
            if not close(as_float(tick_row, "rms_ell"), rms):
                tick_pair_counts_ok = False

            ordered = sorted(rows, key=lambda row: (
                as_int(row, "negative_id"), as_int(row, "positive_id")
            ))
            random_count = 0
            random_max = 0.0
            for rank, positive_row in enumerate(ordered):
                negative_row = ordered[(rank + 1) % len(ordered)]
                separation = tuple(as_float(positive_row, name) for name in ("dx", "dy", "dz"))
                norm = math.sqrt(dot(separation, separation))
                axis = tuple(value / norm for value in separation)
                q_plus = dot(axis, tuple(as_float(positive_row, name) for name in ("jpx", "jpy", "jpz")))
                p_plus = dot(axis, tuple(as_float(positive_row, name) for name in ("wpx", "wpy", "wpz")))
                q_minus = dot(axis, tuple(as_float(negative_row, name) for name in ("jmx", "jmy", "jmz")))
                p_minus = dot(axis, tuple(as_float(negative_row, name) for name in ("wmx", "wmy", "wmz")))
                ell = q_plus * p_minus - q_minus * p_plus
                wedge_scale = max(1.0, abs(q_plus * p_minus), abs(q_minus * p_plus))
                if abs(ell) > TOLERANCE * wedge_scale:
                    random_count += 1
                    random_max = max(random_max, abs(ell))
            if as_int(tick_row, "randomized_valid_pairs") != random_count:
                randomized_ok = False
            if not close(as_float(tick_row, "randomized_max_abs_ell"), random_max):
                randomized_ok = False
        except (KeyError, ValueError, ZeroDivisionError) as error:
            tick_pair_counts_ok = False
            randomized_ok = False
            details.append(f"arm/tick {arm}/{tick}: {error}")

check("tick aggregates reconstruct from pair records", tick_pair_counts_ok)
check("deterministic rotated-negative null reconstructs", randomized_ok)

arm_metrics: dict[tuple[int, str, int], dict[str, int]] = {}
for arm in expected_arms:
    maximum_run = 0
    persistent_pairs = 0
    for observations in histories.get(arm, {}).values():
        observations.sort()
        current = 0
        previous_tick = -2
        previous_chi = 0
        pair_maximum = 0
        for tick, chi in observations:
            current = current + 1 if (
                tick == previous_tick + 1 and chi == previous_chi
            ) else 1
            previous_tick = tick
            previous_chi = chi
            pair_maximum = max(pair_maximum, current)
        maximum_run = max(maximum_run, pair_maximum)
        persistent_pairs += pair_maximum >= PERSISTENCE_TICKS
    arm_metrics[arm] = {
        "maximum_sign_stable_run": maximum_run,
        "persistent_pair_count": persistent_pairs,
        "valid_pair_observations": sum(
            len(pair_by_arm_tick.get((arm, tick), [])) for tick in range(TICKS)
        ),
        "unique_pairs": len(pair_keys_by_arm.get(arm, set())),
        "ticks_with_pairs": sum(
            bool(pair_by_arm_tick.get((arm, tick))) for tick in range(TICKS)
        ),
    }

summary_arms_ok = True
summary_arm_map: dict[tuple[int, str, int], dict[str, Any]] = {}
try:
    for item in summary["arms"]:
        key = (int(item["volume"]), str(item["family"]), int(item["seed"]))
        if key in summary_arm_map:
            summary_arms_ok = False
        summary_arm_map[key] = item
    if set(summary_arm_map) != expected_arms:
        summary_arms_ok = False
    for arm, expected in arm_metrics.items():
        item = summary_arm_map[arm]
        for name, value in expected.items():
            if int(item[name]) != value:
                summary_arms_ok = False
        if not all(bool(item[name]) for name in (
            "finite", "nonmutating", "controls_pass", "reconstructible"
        )):
            summary_arms_ok = False
except (KeyError, TypeError, ValueError) as error:
    summary_arms_ok = False
    details.append(f"summary arms: {error}")
check("summary arm metrics independently reconstruct", summary_arms_ok)

protocol_valid = all((
    matrix_ok,
    tick_parse_ok,
    pair_reconstruction_ok,
    pair_control_ok,
    tick_pair_counts_ok,
    randomized_ok,
    summary_arms_ok,
))
seed_pass_counts: dict[tuple[int, str], int] = defaultdict(int)
any_live_pair = False
for arm, metrics in arm_metrics.items():
    volume, family, _seed = arm
    if family in LIVE_FAMILIES:
        any_live_pair = any_live_pair or metrics["valid_pair_observations"] > 0
        if metrics["maximum_sign_stable_run"] >= PERSISTENCE_TICKS:
            seed_pass_counts[(volume, family)] += 1

outcome_a = protocol_valid and all(
    seed_pass_counts[(volume, family)] >= 3
    for volume in VOLUMES
    for family in LIVE_FAMILIES
)
if not protocol_valid:
    verdict = "PROTOCOL_INVALID_NO_FORMATION_VERDICT"
elif outcome_a:
    verdict = "CROSS_VOLUME_PERSISTENT_ORIENTATION_MEMORY_CANDIDATES"
elif any_live_pair:
    verdict = "FORMATION_WITHOUT_CROSS_VOLUME_PERSISTENCE"
else:
    verdict = "NO_OBSERVED_LOCAL_ORIENTATION_MEMORY_FORMATION"

summary_verdict_ok = False
try:
    summary_verdict_ok = all((
        summary["identifier"] == "FTD-0908",
        summary["protocol_sha256"] == PROTOCOL_SHA256,
        int(summary["arm_count"]) == 32,
        int(summary["ticks_per_arm"]) == TICKS,
        int(summary["persistence_threshold"]) == PERSISTENCE_TICKS,
        bool(summary["matrix_complete"]) == matrix_ok,
        bool(summary["finite"]) == tick_parse_ok,
        bool(summary["nonmutating"]) == tick_parse_ok,
        bool(summary["controls_pass"]) == (
            tick_parse_ok and pair_reconstruction_ok and pair_control_ok
        ),
        bool(summary["protocol_valid"]) == protocol_valid,
        bool(summary["any_live_pair"]) == any_live_pair,
        summary["verdict"] == verdict,
    ))
except (KeyError, TypeError, ValueError) as error:
    details.append(f"summary verdict: {error}")
check("summary verdict matches independent adjudication", summary_verdict_ok)

passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
if details:
    print("\nFailure details:")
    for detail in details[:20]:
        print(f"  {detail}")
print(f"\nFTD-0908 result adjudication: {passed}/{len(checks)} checks passed")
print(f"PRODUCTION_FORMATION_VERDICT={verdict}")
print("CENTRAL_MEMORY_LAW_TESTED=FALSE")
print("MAINTENANCE_ERASURE_WORK_CLOSED=FALSE")
print("PRODUCTION_TICK_MODIFIED=FALSE")
raise SystemExit(0 if passed == len(checks) and protocol_valid else 1)
