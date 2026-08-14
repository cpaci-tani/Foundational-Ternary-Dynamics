#!/usr/bin/env python3
"""Independent raw-corpus reconstruction/adjudication for locked FTD-0911."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0911"
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_HELD_OUT_PAIR_SPECIFIC_PHASE_WEDGE_AND_CENTRALITY_v1.md"
)
RUNNER = ROOT / "engine/tests/campaign_held_out_pair_specific_phase_wedge_centrality.cpp"
PROTOCOL_SHA256 = "D0C7976FE334EA5D814D40DADEDBEF9CB8419B0A518AFE0492C2F3A183FF88FE"
RUNNER_SHA256 = "092954834F568DF2CCCB0F4908CE3E6E0212C45CAE2CFAEF568518C27ED7CE5D"
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

VOLUMES = (19, 23)
SEEDS = tuple(range(0x09110001, 0x09110009))
FAMILIES = ("axial_live", "diagonal_live", "axial_no_bath", "empty_control")
LIVE_FAMILIES = ("axial_live", "diagonal_live", "axial_no_bath")
TICKS = 128
MINIMUM_PAIR_RUN = 8
MINIMUM_COMMON_SUPPORT = 32
CELL_SEED_GATE = 6
CENTRAL_QUALIFIED_SEED_GATE = 12
TOLERANCE = 1e-11
CONTROL_FACTOR = 256.0
CHRONOLOGY_LAGS = (1, 2, 4, 8)

FILES = {
    "ticks": RESULTS / "ftd_0911_tick_census_v1.csv",
    "pairs": RESULTS / "ftd_0911_pair_observations_v1.csv",
    "derangements": RESULTS / "ftd_0911_derangements_v1.csv",
    "chronology": RESULTS / "ftd_0911_chronology_controls_v1.csv",
    "transitions": RESULTS / "ftd_0911_central_transitions_v1.csv",
    "summary": RESULTS / "ftd_0911_summary_v1.json",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def integer(row: dict[str, str], name: str) -> int:
    return int(row[name])


def real(row: dict[str, str], name: str) -> float:
    value = float(row[name])
    if not math.isfinite(value):
        raise ValueError(f"non-finite {name}")
    return value


def boolean(row: dict[str, str], name: str) -> bool:
    if row[name] not in {"0", "1"}:
        raise ValueError(f"non-Boolean {name}")
    return row[name] == "1"


def close(left: float, right: float, *scales: float) -> bool:
    scale = max(1.0, abs(left), abs(right), *(abs(value) for value in scales))
    return math.isfinite(left) and math.isfinite(right) and (
        abs(left - right) <= CONTROL_FACTOR * TOLERANCE * scale
    )


def vector(row: dict[str, str], names: tuple[str, str, str]) -> tuple[float, float, float]:
    return tuple(real(row, name) for name in names)  # type: ignore[return-value]


def dot(left: tuple[float, ...], right: tuple[float, ...]) -> float:
    return sum(a * b for a, b in zip(left, right))


def wedge2(left: tuple[float, float], right: tuple[float, float]) -> float:
    return left[0] * right[1] - left[1] * right[0]


def pseudo_chirality(positive: dict[str, str], negative: dict[str, str]) -> tuple[bool, int]:
    separation = vector(positive, ("dx", "dy", "dz"))
    norm = math.sqrt(dot(separation, separation))
    if not math.isfinite(norm) or norm <= TOLERANCE:
        return False, 0
    axis = tuple(value / norm for value in separation)
    q_plus = dot(axis, vector(positive, ("jpx", "jpy", "jpz")))
    p_plus = dot(axis, vector(positive, ("wpx", "wpy", "wpz")))
    q_minus = dot(axis, vector(negative, ("jmx", "jmy", "jmz")))
    p_minus = dot(axis, vector(negative, ("wmx", "wmy", "wmz")))
    ell = q_plus * p_minus - q_minus * p_plus
    scale = max(1.0, abs(q_plus * p_minus), abs(q_minus * p_plus))
    valid = math.isfinite(ell) and abs(ell) > TOLERANCE * scale
    return valid, (1 if ell > 0.0 else -1) if valid else 0


def longest_presence_run(history: dict[int, dict[str, str]]) -> int:
    maximum = 0
    current = 0
    previous = -2
    for tick in sorted(history):
        current = current + 1 if tick == previous + 1 else 1
        maximum = max(maximum, current)
        previous = tick
    return maximum


def longest_common_interval(histories: list[dict[int, dict[str, str]]]) -> tuple[int, int]:
    if not histories:
        return -1, -2
    common = set(histories[0])
    for history in histories[1:]:
        common &= set(history)
    best_start, best_end = -1, -2
    start, previous = -1, -2
    for tick in sorted(common):
        if tick != previous + 1:
            start = tick
        if tick - start > best_end - best_start:
            best_start, best_end = start, tick
        previous = tick
    return best_start, best_end


checks: list[tuple[str, bool]] = []
details: list[str] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, bool(condition)))
    if not condition and detail:
        details.append(f"{name}: {detail}")


for path, expected in SOURCE_LOCKS.items():
    check(f"source lock {path.relative_to(ROOT)}", path.is_file() and sha256(path) == expected)
check("all six corpus files exist", all(path.is_file() for path in FILES.values()))
if not all(path.is_file() for path in FILES.values()):
    for name, ok in checks:
        print(f"{'PASS' if ok else 'FAIL'}  {name}")
    print("PAIR_CENTRALITY_VERDICT=NOT_AVAILABLE")
    raise SystemExit(1)

try:
    tick_rows = read_csv(FILES["ticks"])
    pair_rows = read_csv(FILES["pairs"])
    derangement_rows = read_csv(FILES["derangements"])
    chronology_rows = read_csv(FILES["chronology"])
    transition_rows = read_csv(FILES["transitions"])
    summary: dict[str, Any] = json.loads(FILES["summary"].read_text(encoding="utf-8"))
except (OSError, ValueError, csv.Error, json.JSONDecodeError) as error:
    check("corpus parses", False, str(error))
    tick_rows, pair_rows = [], []
    derangement_rows, chronology_rows, transition_rows = [], [], []
    summary = {}
else:
    check("corpus parses", True)

expected_arms = {
    (volume, family, seed)
    for volume in VOLUMES for seed in SEEDS for family in FAMILIES
}
tick_by_arm: dict[tuple[int, str, int], dict[int, dict[str, str]]] = defaultdict(dict)
tick_ok = True
for row in tick_rows:
    try:
        arm = (integer(row, "volume"), row["family"], integer(row, "seed"))
        tick = integer(row, "tick")
        if tick in tick_by_arm[arm]:
            tick_ok = False
        tick_by_arm[arm][tick] = row
        for name in ("positive_count", "negative_count", "genesis_events", "evaporation_events", "valid_pairs"):
            if integer(row, name) < 0:
                tick_ok = False
        for name in ("native_wave_energy", "worst_control_residual"):
            if real(row, name) < 0.0:
                tick_ok = False
        if not all(boolean(row, name) for name in ("nonmutating", "controls_pass", "reconstructible")):
            tick_ok = False
        if row["voxel_hash_before"] != row["voxel_hash_after"]:
            tick_ok = False
        if row["rng_hash_before"] != row["rng_hash_after"]:
            tick_ok = False
    except (KeyError, ValueError) as error:
        tick_ok = False
        details.append(f"tick: {error}")
matrix_ok = (
    set(tick_by_arm) == expected_arms
    and len(tick_rows) == 64 * TICKS
    and all(set(rows) == set(range(TICKS)) for rows in tick_by_arm.values())
)
check("tick matrix is exactly 64 arms by 128 ticks", matrix_ok)
check("tick telemetry is finite, read-only, and control-clean", tick_ok)

histories: dict[
    tuple[int, str, int], dict[tuple[int, int], dict[int, dict[str, str]]]
] = defaultdict(lambda: defaultdict(dict))
pair_by_arm_tick: dict[tuple[tuple[int, str, int], int], list[dict[str, str]]] = defaultdict(list)
pair_ok = True
for row in pair_rows:
    try:
        arm = (integer(row, "volume"), row["family"], integer(row, "seed"))
        tick = integer(row, "tick")
        key = (integer(row, "positive_id"), integer(row, "negative_id"))
        if arm not in expected_arms or tick not in range(TICKS) or min(key) < 0:
            pair_ok = False
        if tick in histories[arm][key]:
            pair_ok = False
        separation = vector(row, ("dx", "dy", "dz"))
        if not close(max(abs(value) for value in separation), 1.0):
            pair_ok = False
        norm = math.sqrt(dot(separation, separation))
        axis = tuple(value / norm for value in separation)
        q_plus = dot(axis, vector(row, ("jpx", "jpy", "jpz")))
        q_minus = dot(axis, vector(row, ("jmx", "jmy", "jmz")))
        p_plus = dot(axis, vector(row, ("wpx", "wpy", "wpz")))
        p_minus = dot(axis, vector(row, ("wmx", "wmy", "wmz")))
        ell = q_plus * p_minus - q_minus * p_plus
        wedge_scale = max(1.0, abs(q_plus * p_minus), abs(q_minus * p_plus))
        expected_chi = 1 if ell > 0.0 else -1
        if not all((
            close(real(row, "q_plus"), q_plus), close(real(row, "q_minus"), q_minus),
            close(real(row, "p_plus"), p_plus), close(real(row, "p_minus"), p_minus),
            close(real(row, "ell"), ell), abs(ell) > TOLERANCE * wedge_scale,
            integer(row, "chi") == expected_chi,
        )):
            pair_ok = False
        histories[arm][key][tick] = row
        pair_by_arm_tick[(arm, tick)].append(row)
    except (KeyError, ValueError, ZeroDivisionError) as error:
        pair_ok = False
        details.append(f"pair: {error}")
check("raw pair records reconstruct from endpoint fields", pair_ok)

aggregate_ok = True
for arm in expected_arms:
    for tick in range(TICKS):
        try:
            if integer(tick_by_arm[arm][tick], "valid_pairs") != len(pair_by_arm_tick.get((arm, tick), [])):
                aggregate_ok = False
        except (KeyError, ValueError):
            aggregate_ok = False
check("tick pair counts reconstruct", aggregate_ok)

reported_derangements = {}
derangement_parse_ok = True
for row in derangement_rows:
    try:
        arm = (integer(row, "volume"), row["family"], integer(row, "seed"))
        shift = integer(row, "shift")
        key = (arm, shift)
        if key in reported_derangements:
            derangement_parse_ok = False
        reported_derangements[key] = row
    except (KeyError, ValueError):
        derangement_parse_ok = False

reported_chronology = {}
chronology_parse_ok = True
for row in chronology_rows:
    try:
        arm = (integer(row, "volume"), row["family"], integer(row, "seed"))
        lag = integer(row, "lag")
        key = (arm, lag)
        if key in reported_chronology:
            chronology_parse_ok = False
        reported_chronology[key] = row
    except (KeyError, ValueError):
        chronology_parse_ok = False

pair_metrics: dict[tuple[int, str, int], dict[str, Any]] = {}
pair_discriminator_ok = derangement_parse_ok
chronology_ok = chronology_parse_ok
for arm in expected_arms:
    retained = [
        (key, history) for key, history in sorted(histories.get(arm, {}).items())
        if longest_presence_run(history) >= MINIMUM_PAIR_RUN
    ]
    start, end = longest_common_interval([history for _key, history in retained])
    length = end - start + 1 if end >= start else 0
    qualified = len(retained) >= 2 and length >= MINIMUM_COMMON_SUPPORT
    actual_same = actual_flip = 0
    maximum_null = minimum_null = -1
    all_valid = True
    null_shift_count = 0
    if qualified:
        for _key, history in retained:
            for tick in range(start, end):
                same = integer(history[tick], "chi") == integer(history[tick + 1], "chi")
                actual_same += same
                actual_flip += not same
        for shift in range(1, len(retained)):
            same = flips = 0
            valid = True
            for index, (_key, positive_history) in enumerate(retained):
                negative_history = retained[(index + shift) % len(retained)][1]
                previous_chi = 0
                for tick in range(start, end + 1):
                    sample_valid, chi = pseudo_chirality(positive_history[tick], negative_history[tick])
                    valid = valid and sample_valid
                    if not valid:
                        break
                    if tick > start:
                        same += chi == previous_chi
                        flips += chi != previous_chi
                    previous_chi = chi
                if not valid:
                    break
            all_valid = all_valid and valid
            maximum_null = max(maximum_null, same) if valid else maximum_null
            minimum_null = same if valid and minimum_null < 0 else (min(minimum_null, same) if valid else minimum_null)
            null_shift_count += 1
            report = reported_derangements.get((arm, shift))
            if report is None or not all((
                integer(report, "retained_pairs") == len(retained),
                integer(report, "common_start") == start,
                integer(report, "common_end") == end,
                boolean(report, "valid") == valid,
                integer(report, "same") == same,
                integer(report, "flips") == flips,
            )):
                pair_discriminator_ok = False
        for lag in CHRONOLOGY_LAGS:
            same = flips = observations = 0
            valid = True
            for _key, history in retained:
                previous_chi = 0
                have_previous = False
                for tick in range(start, end - lag + 1):
                    sample_valid, chi = pseudo_chirality(history[tick], history[tick + lag])
                    valid = valid and sample_valid
                    if not valid:
                        break
                    if have_previous:
                        same += chi == previous_chi
                        flips += chi != previous_chi
                    previous_chi = chi
                    have_previous = True
                    observations += 1
                if not valid:
                    break
            report = reported_chronology.get((arm, lag))
            if report is None or not all((
                boolean(report, "valid") == valid,
                integer(report, "observations") == observations,
                integer(report, "same") == same,
                integer(report, "flips") == flips,
            )):
                chronology_ok = False
    else:
        if any(key[0] == arm for key in reported_derangements):
            pair_discriminator_ok = False
        if any(key[0] == arm for key in reported_chronology):
            chronology_ok = False
    pair_pass = qualified and all_valid and maximum_null >= 0 and actual_same > maximum_null
    pair_metrics[arm] = {
        "qualified": qualified, "pass": pair_pass,
        "retained_pairs": len(retained), "common_start": start,
        "common_end": end, "common_length": length,
        "actual_same": actual_same, "actual_flip": actual_flip,
        "maximum_null_same": maximum_null, "minimum_null_same": minimum_null,
        "null_shift_count": null_shift_count,
        "all_pseudo_wedges_valid": all_valid,
    }
expected_derangement_rows = sum(
    metrics["null_shift_count"] for metrics in pair_metrics.values()
)
check("all fixed cyclic derangements independently reconstruct", pair_discriminator_ok and len(derangement_rows) == expected_derangement_rows)
expected_chronology_rows = sum(
    len(CHRONOLOGY_LAGS) for metrics in pair_metrics.values() if metrics["qualified"]
)
check("chronology controls independently reconstruct", chronology_ok and len(chronology_rows) == expected_chronology_rows)

reported_transitions = {}
transition_parse_ok = True
for row in transition_rows:
    try:
        arm = (integer(row, "volume"), row["family"], integer(row, "seed"))
        key = (integer(row, "positive_id"), integer(row, "negative_id"))
        transition_key = (arm, key, integer(row, "tick_before"), integer(row, "tick_after"))
        if transition_key in reported_transitions:
            transition_parse_ok = False
        reported_transitions[transition_key] = row
    except (KeyError, ValueError):
        transition_parse_ok = False

central_metrics: dict[tuple[int, str, int], dict[str, Any]] = {}
central_ok = transition_parse_ok
expected_transition_count = 0
for arm in expected_arms:
    metrics = {
        "transitions": 0, "identity_failures": 0, "central_failures": 0,
        "identity_pass": True, "central_pass": True,
        "maximum_identity_residual": 0.0, "maximum_abs_delta_wedge": 0.0,
        "maximum_abs_torque_p": 0.0, "maximum_abs_torque_q": 0.0,
    }
    for key, history in histories.get(arm, {}).items():
        ordered_ticks = sorted(history)
        for before_tick, after_tick in zip(ordered_ticks, ordered_ticks[1:]):
            if after_tick != before_tick + 1:
                continue
            before, after = history[before_tick], history[after_tick]
            q0 = (real(before, "q_plus"), real(before, "q_minus"))
            p0 = (real(before, "p_plus"), real(before, "p_minus"))
            q1 = (real(after, "q_plus"), real(after, "q_minus"))
            p1 = (real(after, "p_plus"), real(after, "p_minus"))
            qbar = (0.5 * (q0[0] + q1[0]), 0.5 * (q0[1] + q1[1]))
            pbar = (0.5 * (p0[0] + p1[0]), 0.5 * (p0[1] + p1[1]))
            dq = (q1[0] - q0[0], q1[1] - q0[1])
            dp = (p1[0] - p0[0], p1[1] - p0[1])
            delta_ell = real(after, "ell") - real(before, "ell")
            torque_p = wedge2(qbar, dp)
            torque_q = wedge2(dq, pbar)
            residual = delta_ell - torque_p - torque_q
            scale = max(
                1.0, abs(real(after, "ell")), abs(real(before, "ell")),
                abs(qbar[0] * dp[1]), abs(qbar[1] * dp[0]),
                abs(dq[0] * pbar[1]), abs(dq[1] * pbar[0]),
            )
            accepted = CONTROL_FACTOR * TOLERANCE * scale
            identity_pass = abs(residual) <= accepted
            exact_pass = identity_pass and all(
                abs(value) <= accepted for value in (delta_ell, torque_p, torque_q)
            )
            report = reported_transitions.get((arm, key, before_tick, after_tick))
            if report is None or not all((
                close(real(report, "ell_before"), real(before, "ell")),
                close(real(report, "ell_after"), real(after, "ell")),
                close(real(report, "delta_ell"), delta_ell),
                close(real(report, "torque_p"), torque_p),
                close(real(report, "torque_q"), torque_q),
                close(real(report, "identity_residual"), residual),
                close(real(report, "accepted"), accepted),
                boolean(report, "identity_pass") == identity_pass,
                boolean(report, "central_pass") == exact_pass,
            )):
                central_ok = False
            metrics["transitions"] += 1
            metrics["identity_failures"] += not identity_pass
            metrics["central_failures"] += not exact_pass
            metrics["identity_pass"] = metrics["identity_pass"] and identity_pass
            metrics["central_pass"] = metrics["central_pass"] and exact_pass
            metrics["maximum_identity_residual"] = max(metrics["maximum_identity_residual"], abs(residual))
            metrics["maximum_abs_delta_wedge"] = max(metrics["maximum_abs_delta_wedge"], abs(delta_ell))
            metrics["maximum_abs_torque_p"] = max(metrics["maximum_abs_torque_p"], abs(torque_p))
            metrics["maximum_abs_torque_q"] = max(metrics["maximum_abs_torque_q"], abs(torque_q))
    if metrics["transitions"] == 0:
        metrics["central_pass"] = False
    central_metrics[arm] = metrics
    expected_transition_count += metrics["transitions"]
check("parameter-free midpoint transition ledger independently reconstructs", central_ok and len(transition_rows) == expected_transition_count)

summary_arms_ok = True
summary_arm_map = {}
try:
    for item in summary["arms"]:
        arm = (int(item["volume"]), str(item["family"]), int(item["seed"]))
        if arm in summary_arm_map:
            summary_arms_ok = False
        summary_arm_map[arm] = item
    if set(summary_arm_map) != expected_arms:
        summary_arms_ok = False
    for arm in expected_arms:
        item = summary_arm_map[arm]
        pair = pair_metrics[arm]
        central = central_metrics[arm]
        comparisons = (
            bool(item["pair_qualified"]) == pair["qualified"],
            bool(item["pair_pass"]) == pair["pass"],
            int(item["retained_pairs"]) == pair["retained_pairs"],
            int(item["common_start"]) == pair["common_start"],
            int(item["common_end"]) == pair["common_end"],
            int(item["common_length"]) == pair["common_length"],
            int(item["actual_same"]) == pair["actual_same"],
            int(item["actual_flip"]) == pair["actual_flip"],
            int(item["maximum_null_same"]) == pair["maximum_null_same"],
            int(item["minimum_null_same"]) == pair["minimum_null_same"],
            int(item["null_shift_count"]) == pair["null_shift_count"],
            bool(item["all_pseudo_wedges_valid"]) == pair["all_pseudo_wedges_valid"],
            int(item["central_transitions"]) == central["transitions"],
            int(item["central_failures"]) == central["central_failures"],
            int(item["identity_failures"]) == central["identity_failures"],
            close(float(item["maximum_identity_residual"]), central["maximum_identity_residual"]),
            close(float(item["maximum_abs_delta_wedge"]), central["maximum_abs_delta_wedge"]),
            close(float(item["maximum_abs_torque_p"]), central["maximum_abs_torque_p"]),
            close(float(item["maximum_abs_torque_q"]), central["maximum_abs_torque_q"]),
            int(item["valid_pair_observations"]) == sum(len(rows) for (candidate_arm, _tick), rows in pair_by_arm_tick.items() if candidate_arm == arm),
            int(item["unique_pairs"]) == len(histories.get(arm, {})),
            all(bool(item[name]) for name in ("finite", "nonmutating", "controls_pass", "reconstructible")),
        )
        if not all(comparisons):
            summary_arms_ok = False
except (KeyError, TypeError, ValueError) as error:
    summary_arms_ok = False
    details.append(f"summary arms: {error}")
check("summary arm metrics independently reconstruct", summary_arms_ok)

qualified_counts: dict[tuple[int, str], int] = defaultdict(int)
pass_counts: dict[tuple[int, str], int] = defaultdict(int)
central_qualified_seeds = 0
central_all_pass = True
for arm in expected_arms:
    volume, family, _seed = arm
    if family in LIVE_FAMILIES and pair_metrics[arm]["qualified"]:
        qualified_counts[(volume, family)] += 1
        pass_counts[(volume, family)] += pair_metrics[arm]["pass"]
    if family == "axial_no_bath" and central_metrics[arm]["transitions"] > 0:
        central_qualified_seeds += 1
        central_all_pass = central_all_pass and central_metrics[arm]["central_pass"]
pair_qualified = all(
    qualified_counts[(volume, family)] >= CELL_SEED_GATE
    for volume in VOLUMES for family in LIVE_FAMILIES
)
pair_pass = all(
    pass_counts[(volume, family)] >= CELL_SEED_GATE
    for volume in VOLUMES for family in LIVE_FAMILIES
)
central_qualified = central_qualified_seeds >= CENTRAL_QUALIFIED_SEED_GATE
central_pass = central_qualified and central_all_pass
identity_all_pass = all(
    metrics["identity_failures"] == 0 for metrics in central_metrics.values()
)
protocol_valid = all((
    matrix_ok, tick_ok, pair_ok, aggregate_ok, pair_discriminator_ok,
    chronology_ok, central_ok, identity_all_pass, summary_arms_ok,
))
if not protocol_valid:
    verdict = "PROTOCOL_INVALID_NO_PAIR_OR_CENTRALITY_VERDICT"
elif not pair_qualified or not central_qualified:
    verdict = "OUTCOME_U_UNQUALIFIED"
elif pair_pass and central_pass:
    verdict = "OUTCOME_A_PAIR_SPECIFIC_AND_EXACT_CENTRAL"
elif pair_pass:
    verdict = "OUTCOME_B_PAIR_SPECIFIC_NOT_EXACT_CENTRAL"
elif central_pass:
    verdict = "OUTCOME_C_NOT_PAIR_SPECIFIC_BUT_EXACT_CENTRAL"
else:
    verdict = "OUTCOME_D_NOT_PAIR_SPECIFIC_NOT_EXACT_CENTRAL"

summary_verdict_ok = False
try:
    summary_verdict_ok = all((
        summary["identifier"] == "FTD-0911",
        summary["protocol_sha256"] == PROTOCOL_SHA256,
        int(summary["arm_count"]) == 64,
        int(summary["ticks_per_arm"]) == TICKS,
        bool(summary["matrix_complete"]) == matrix_ok,
        bool(summary["finite"]) == tick_ok,
        bool(summary["nonmutating"]) == tick_ok,
        bool(summary["controls_pass"]) == (tick_ok and identity_all_pass),
        bool(summary["protocol_valid"]) == protocol_valid,
        bool(summary["pair_qualified"]) == pair_qualified,
        bool(summary["pair_pass"]) == pair_pass,
        int(summary["central_qualified_seeds"]) == central_qualified_seeds,
        bool(summary["central_qualified"]) == central_qualified,
        bool(summary["central_pass"]) == central_pass,
        summary["verdict"] == verdict,
    ))
except (KeyError, TypeError, ValueError) as error:
    details.append(f"summary verdict: {error}")
check("summary verdict matches independent adjudication", summary_verdict_ok)

passed = sum(ok for _name, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
if details:
    print("\nFailure details:")
    for detail in details[:20]:
        print(f"  {detail}")
print(f"\nFTD-0911 result adjudication: {passed}/{len(checks)} checks passed")
print(f"PAIR_SPECIFICITY={'PASS' if pair_pass else 'FAIL'}")
print(f"EXACT_CENTRALITY={'PASS' if central_pass else 'FAIL'}")
print(f"PAIR_CENTRALITY_VERDICT={verdict}")
print("PERTURBATION_APPLIED=FALSE")
print("MAINTENANCE_ERASURE_WORK_CLOSED=FALSE")
raise SystemExit(0 if passed == len(checks) and protocol_valid else 1)
