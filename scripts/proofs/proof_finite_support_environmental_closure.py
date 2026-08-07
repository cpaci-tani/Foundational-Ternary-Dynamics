"""Independent serialized-record certificate for FTD-0745.

The proof reconstructs the registered held-out gates and verdict from the
frozen FTD-0739 CSV plus the FTD-0745 CSV/JSON. It does not call the C++ verdict
logic, rerun dynamics, or search parameters.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_FINITE_SUPPORT_ENVIRONMENTAL_CLOSURE_v1.md"
RUNNER = ROOT / "engine/tests/test_finite_support_environmental_closure.cpp"
EXECUTABLE = ROOT / "engine/build/Release/test_finite_support_environmental_closure.exe"
BASELINE = ROOT / "engine/results/ftd_0739/ftd_0739_finite_support_outgoing_tail_formation_v1.csv"
CSV = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.csv"
JSON = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.json"

HASHES = {
    PROTOCOL: "D5FB9923FCBF69E2DFD75300FEE4C381AE28EAA10843BF0D52B2D60FCE456888",
    RUNNER: "7F2205D688A53EF802126FB529C560D1B743BCC896D8DBE769DC54BDDD28776E",
    EXECUTABLE: "B140CE3047A7EA263FF4F2829CD80FCD7C2122EA19EA12822900A2CFB31A6688",
    BASELINE: "E9B9B2FCE0FDA1350DBD6195AE039E99004141C86CB8A3F195ACE5CF24ADC622",
    CSV: "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C",
    JSON: "B6325EFBC06F486F6135C20E97F78B50752E637138C7B277AC513ED2E761DC2A",
}

HORIZON = 184
PREFIX_HORIZON = 136
RADII = (8, 12, 16, 24, 32, 48)
EXPECTED_KEYS = {
    ("bound", "0_0_1", "plus_minus"),
    ("unbound", "0_0_1", "plus_minus"),
    ("unbound", "0_1_-1", "plus_minus"),
    ("unbound", "1_1_1", "minus_plus"),
    ("unbound", "1_1_1", "plus_minus"),
}

checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def number(row: dict[str, str], column: str) -> float:
    return float(row[column])


def integer(row: dict[str, str], column: str) -> int:
    return int(row[column])


def flag(row: dict[str, str], column: str) -> bool:
    return bool(int(row[column]))


def close(left: float, right: float, *, atol: float = 1e-15) -> bool:
    return math.isclose(left, right, rel_tol=1e-12, abs_tol=atol)


for path, expected in HASHES.items():
    check(f"frozen SHA-256 {path.name}", path.is_file() and sha256(path) == expected)

with CSV.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))
with BASELINE.open(newline="", encoding="utf-8") as handle:
    baseline_rows = [row for row in csv.DictReader(handle) if row["phase"] == "forward"]
raw_summary = JSON.read_text(encoding="utf-8")
nonstandard_sentinel = '"late_inside_8_minimum": inf,'
check("raw summary has exactly one quarantined bound-only inf sentinel", (
    raw_summary.count(nonstandard_sentinel) == 1
    and raw_summary.count(": inf") == 1
))
summary = json.loads(raw_summary.replace(
    nonstandard_sentinel,
    '"late_inside_8_minimum": null,',
    1,
))

check("CSV data row count 925", len(rows) == 925)
check("frozen baseline forward row count 685", len(baseline_rows) == 685)
check("JSON protocol hash", summary["protocol_sha256"] == HASHES[PROTOCOL])
check("JSON baseline hash", summary["baseline_csv_sha256"] == HASHES[BASELINE])
check("JSON volume/horizon/contact", (
    summary["volume"], summary["horizon"], summary["contact_tick"]
) == (193, 184, 185))
check("JSON radius ladder", tuple(summary["radii"]) == RADII)
check("JSON thresholds", (
    summary["tail_threshold"], summary["tail_final_threshold"],
    summary["late_near_minimum"], summary["late_near_dynamic_range"]
) == (1e-8, 1e-9, 5e-4, 4.0))

groups: dict[tuple[str, str, str], list[dict[str, str]]] = defaultdict(list)
for row in rows:
    groups[(row["family"], row["direction"], row["polarity"])].append(row)
check("exact five-history key set", set(groups) == EXPECTED_KEYS)
json_arms = {
    (arm["family"], arm["direction"], arm["polarity"]): arm
    for arm in summary["arms"]
}
check("JSON/CSV arm keys agree", set(json_arms) == EXPECTED_KEYS)
check("quarantined sentinel belongs only to unused bound near-field slot", (
    json_arms[("bound", "0_0_1", "plus_minus")]["late_inside_8_minimum"] is None
    and all(
        arm["late_inside_8_minimum"] is not None
        for key, arm in json_arms.items()
        if key[0] == "unbound"
    )
))

arm_results: dict[tuple[str, str, str], dict[str, object]] = {}
for key in sorted(EXPECTED_KEYS):
    label = "/".join(key)
    arm = groups[key]
    check(f"{label}: ticks 0..184", [integer(row, "tick") for row in arm] == list(range(185)))
    check(f"{label}: forward phase only", all(row["phase"] == "forward" for row in arm))
    check(f"{label}: all forward states valid/common/regional", all(
        flag(row, "valid") and flag(row, "common") and flag(row, "regional_valid")
        for row in arm
    ))
    max_common = max(number(row, "max_residual") for row in arm)
    max_energy = max(number(row, "total_energy_residual") for row in arm)
    max_recoil = max(number(row, "recoil_defect") for row in arm)
    max_speed = max(number(row, "speed_excess") for row in arm)
    max_regional = max(number(row, "regional_residual") for row in arm)
    max_outside_source = max(number(row, "outside_source_residual") for row in arm)
    max_source = max(integer(row, "source_radius") for row in arm)
    check(f"{label}: common residual <=1e-10", max_common <= 1e-10)
    check(f"{label}: energy residual <=1e-8", max_energy <= 1e-8)
    check(f"{label}: recoil <=1e-9", max_recoil <= 1e-9)
    check(f"{label}: speed excess <=1e-12", max_speed <= 1e-12)
    check(f"{label}: regional residual <=1e-10", max_regional <= 1e-10)
    check(f"{label}: outside source residual <=1e-10", max_outside_source <= 1e-10)
    check(f"{label}: source radius <=3", max_source <= 3)

    graph = [flag(row, "graph_inside") for row in arm]
    pair = [number(row, "pair_energy") for row in arm]
    field = [number(row, "field_energy") for row in arm]
    transitions = [tick for tick in range(1, HORIZON + 1) if graph[tick] != graph[tick - 1]]
    onset = next((
        tick for tick in range(HORIZON + 1)
        if all(graph[later] and pair[later] < -1e-6 for later in range(tick, HORIZON + 1))
    ), -1)
    balance = abs(pair[-1] - pair[0] + field[-1] - field[0])
    check(f"{label}: pair-plus-field balance <=1e-8", balance <= 1e-8)
    check(f"{label}: balance agrees with JSON", close(balance, json_arms[key]["pair_field_balance"]))
    check(f"{label}: onset agrees with JSON", onset == json_arms[key]["energetic_onset_tick"])
    check(f"{label}: reverse execution serialized pass", bool(json_arms[key]["reverse_executed"]))
    check(f"{label}: exact combined forward/reverse gate", bool(json_arms[key]["exact_pass"]))
    check(f"{label}: inverse recovery <=1e-8", json_arms[key]["inverse_recovery"] <= 1e-8)

    core = False
    near = False
    arrival = False
    no_return = False
    bound = False
    first_ticks = [-1] * len(RADII)
    maximum_outside = [0.0] * len(RADII)
    final_outside = [0.0] * len(RADII)
    minimum_increment = [math.inf] * len(RADII)
    late_min = math.inf
    late_max = 0.0

    if key[0] == "bound":
        bound = not transitions and all(graph) and all(value < -1e-6 for value in pair)
        check(f"{label}: bound control independently passes", bound)
        check(f"{label}: bound JSON gate", bool(json_arms[key]["bound_control_pass"]) == bound)
    else:
        core = onset >= 0 and HORIZON - onset + 1 >= 64
        late = arm[HORIZON - 32 + 1:]
        late_inside = [number(row, "inside_8") for row in late]
        late_min, late_max = min(late_inside), max(late_inside)
        near = late_min >= 5e-4 and late_max <= 4.0 * late_min
        arrival = max_outside_source <= 1e-10
        no_return = True
        for index, radius in enumerate(RADII):
            outside = [number(row, f"outside_{radius}") for row in arm]
            first = next((tick for tick, value in enumerate(outside) if value > 1e-8), -1)
            first_ticks[index] = first
            maximum_outside[index] = max(outside)
            final_outside[index] = outside[-1]
            increments = [
                -number(arm[tick], f"transport_into_{radius}")
                for tick in range(first, HORIZON + 1)
            ] if first >= 0 else []
            minimum_increment[index] = min(increments, default=math.inf)
            if index >= 1:
                arrival = arrival and outside[0] <= 1e-12 and first >= 0
                arrival = arrival and max(outside) > 1e-8 and outside[-1] > 1e-9
                if index > 1:
                    arrival = arrival and first >= first_ticks[index - 1]
                no_return = no_return and minimum_increment[index] >= -1e-10
        check(f"{label}: 64-tick core tail", core)
        check(f"{label}: late near-field gate", near)
        check(f"{label}: ordered shell-arrival gate independently reproduced", (
            arrival == bool(json_arms[key]["arrival_pass"])
        ))
        check(f"{label}: no-return gate independently reproduced", (
            no_return == bool(json_arms[key]["no_return_pass"])
        ))
        check(f"{label}: shell summaries agree with JSON", (
            first_ticks == json_arms[key]["first_tail_ticks"]
            and all(close(a, b) for a, b in zip(maximum_outside, json_arms[key]["maximum_outside"], strict=True))
            and all(close(a, b) for a, b in zip(final_outside, json_arms[key]["final_outside"], strict=True))
            and all(
                (math.isinf(a) and b is None) or (b is not None and close(a, b))
                for a, b in zip(minimum_increment, json_arms[key]["minimum_outward_increment"], strict=True)
            )
        ))
        check(f"{label}: result flags agree with JSON", (
            bool(json_arms[key]["core_pass"]),
            bool(json_arms[key]["near_field_pass"]),
            bool(json_arms[key]["arrival_pass"]),
            bool(json_arms[key]["no_return_pass"]),
        ) == (core, near, arrival, no_return))

    arm_results[key] = {
        "transitions": transitions,
        "onset": onset,
        "core": core,
        "near": near,
        "arrival": arrival,
        "no_return": no_return,
        "bound": bound,
        "first_ticks": first_ticks,
    }

# Reconstruct the complete causal prefix comparison against FTD-0739.
baseline = {
    (row["family"], row["direction"], row["polarity"], integer(row, "tick")): row
    for row in baseline_rows
}
prefix_discrete = True
prefix_difference = 0.0
column_pairs = (
    ("max_residual", "max_residual"),
    ("total_energy_residual", "total_energy_residual"),
    ("recoil_defect", "recoil_defect"),
    ("speed_excess", "speed_excess"),
    ("regional_residual", "regional_residual"),
    ("separation", "separation"),
    ("pair_energy", "pair_energy"),
    ("field_energy", "field_energy"),
    ("inside_8", "inside_energy_8"),
    ("outside_8", "outside_energy_8"),
    ("transport_into_8", "boundary_transport_into_8"),
    ("source_exchange_8", "source_exchange_8"),
    ("inside_12", "inside_energy_12"),
    ("outside_12", "outside_energy_12"),
    ("transport_into_12", "boundary_transport_into_12"),
    ("source_exchange_12", "source_exchange_12"),
    ("cumulative_outward_12", "cumulative_outward_12"),
)
for key in EXPECTED_KEYS:
    for tick in range(PREFIX_HORIZON + 1):
        new = groups[key][tick]
        old = baseline[(key[0], key[1], key[2], tick)]
        prefix_discrete = prefix_discrete and all(
            new[new_name] == old[old_name]
            for new_name, old_name in (
                ("valid", "valid"), ("common", "common"),
                ("regional_valid", "regional_valid"),
                ("source_radius", "source_radius"),
                ("source_entries", "source_entries"),
                ("graph_inside", "graph_inside"),
            )
        )
        for new_name, old_name in column_pairs:
            prefix_difference = max(
                prefix_difference,
                abs(number(new, new_name) - number(old, old_name)),
            )
prefix_pass = prefix_discrete and prefix_difference <= 1e-10
check("causal prefix discrete fields exact", prefix_discrete)
check("causal prefix scalar difference <=1e-10", prefix_difference <= 1e-10)
check("causal prefix difference agrees with JSON", close(prefix_difference, summary["prefix_scalar_difference"]))
check("causal prefix discrete flag agrees with JSON", prefix_discrete == bool(summary["prefix_discrete_pass"]))

# Full persisted polarity comparison.
plus_key = ("unbound", "1_1_1", "plus_minus")
minus_key = ("unbound", "1_1_1", "minus_plus")
plus, minus = groups[plus_key], groups[minus_key]
discrete_columns = (
    "family", "direction", "phase", "tick", "valid", "common",
    "regional_valid", "source_radius", "source_entries", "graph_inside",
)
scalar_columns = (
    "max_residual", "total_energy_residual", "recoil_defect", "speed_excess",
    "regional_residual", "outside_source_residual", "separation", "pair_energy",
    "field_energy",
) + tuple(
    f"{prefix}_{radius}"
    for radius in RADII
    for prefix in ("inside", "outside", "transport_into", "source_exchange", "cumulative_outward")
)
polarity_discrete = all(
    all(left[column] == right[column] for column in discrete_columns)
    for left, right in zip(plus, minus, strict=True)
)
polarity_difference = max(
    abs(number(left, column) - number(right, column))
    for left, right in zip(plus, minus, strict=True)
    for column in scalar_columns
)
polarity_pass = (
    polarity_discrete
    and arm_results[plus_key]["transitions"] == arm_results[minus_key]["transitions"]
    and arm_results[plus_key]["first_ticks"] == arm_results[minus_key]["first_ticks"]
    and polarity_difference <= 1e-9
)
check("polarity discrete histories identical", polarity_discrete)
check("polarity scalar difference <=1e-9", polarity_difference <= 1e-9)
check("polarity difference agrees with JSON", close(polarity_difference, summary["polarity_scalar_difference"]))

infrastructure = (
    len(rows) == 925 and set(groups) == EXPECTED_KEYS
    and all(bool(json_arms[key]["preparation_pass"]) for key in EXPECTED_KEYS)
    and all(bool(json_arms[key]["forward_executed"]) for key in EXPECTED_KEYS)
    and all(bool(json_arms[key]["reverse_executed"]) for key in EXPECTED_KEYS)
    and all(bool(json_arms[key]["exact_pass"]) for key in EXPECTED_KEYS)
    and all(bool(json_arms[key]["inverse_pass"]) for key in EXPECTED_KEYS)
    and all(bool(json_arms[key]["support_pass"]) for key in EXPECTED_KEYS)
)
control = bool(arm_results[("bound", "0_0_1", "plus_minus")]["bound"])
cores = all(bool(result["core"]) for key, result in arm_results.items() if key[0] == "unbound")
near = all(bool(result["near"]) for key, result in arm_results.items() if key[0] == "unbound")
arrivals = all(bool(result["arrival"]) for key, result in arm_results.items() if key[0] == "unbound")
no_return = all(bool(result["no_return"]) for key, result in arm_results.items() if key[0] == "unbound")

if not infrastructure:
    verdict = "ENVIRONMENTAL_CLOSURE_EXECUTION_INVALID"
elif not prefix_pass:
    verdict = "ENVIRONMENTAL_CLOSURE_CAUSAL_PREFIX_DRIFT"
elif not control:
    verdict = "ENVIRONMENTAL_CLOSURE_BOUND_CONTROL_UNSTABLE"
elif not polarity_pass:
    verdict = "ENVIRONMENTAL_CLOSURE_POLARITY_SENSITIVE"
elif not cores:
    verdict = "ENVIRONMENTAL_CLOSURE_CORE_NOT_PERSISTENT"
elif not near:
    verdict = "ENVIRONMENTAL_CLOSURE_NEAR_FIELD_NOT_STABLE"
elif not arrivals:
    verdict = "ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL"
elif not no_return:
    verdict = "ENVIRONMENTAL_CLOSURE_OUTGOING_COMPONENT_RETURNS"
else:
    verdict = "FINITE_LADDER_ENVIRONMENTAL_CLOSURE_CONSTRUCTIVE"

check("independent verdict agrees with JSON", verdict == summary["verdict"])
passed = sum(condition for _, condition in checks)
print(f"\nFTD-0745 independent certificate: {passed}/{len(checks)} checks passed")
print(f"verdict={verdict}")
raise SystemExit(0 if passed == len(checks) else 1)
