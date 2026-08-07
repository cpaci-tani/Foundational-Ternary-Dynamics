"""Independent serialized-record certificate for FTD-0739.

This script does not call the C++ verdict logic.  It reads the frozen CSV/JSON
record, independently reconstructs the registered gates, transition/onset
classes, first-passage identity, polarity comparison, summary maxima, and
verdict, and freezes all source/result hashes.  It performs no search.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_v1.md"
RUNNER = ROOT / "engine/tests/test_finite_support_outgoing_tail_formation.cpp"
EXECUTABLE = ROOT / "engine/build/Release/test_finite_support_outgoing_tail_formation.exe"
CSV = ROOT / "engine/results/ftd_0739/ftd_0739_finite_support_outgoing_tail_formation_v1.csv"
JSON = ROOT / "engine/results/ftd_0739/ftd_0739_finite_support_outgoing_tail_formation_v1.json"

HASHES = {
    PROTOCOL: "9AA9B806877F07F9567291E73B58E6157CFBDAE425DE843B85D3753CECA7868E",
    RUNNER: "F08AD44732B5A51AE3C5ACABD540033224DEC56BF77FEFF5589DE27C8CF62DCC",
    EXECUTABLE: "C0E158D171B0BDF822FF2EC173E3810ACFB3B6F47947873C350F7C6E27B263B5",
    CSV: "E9B9B2FCE0FDA1350DBD6195AE039E99004141C86CB8A3F195ACE5CF24ADC622",
    JSON: "237F6EA3343BF6DA7C2E0979C5B77C4DD848EAF22FB79DF187A7C34055A19D5C",
}

HORIZON = 136
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


def close(lhs: float, rhs: float, *, atol: float = 1e-15) -> bool:
    return math.isclose(lhs, rhs, rel_tol=1e-12, abs_tol=atol)


for path, expected in HASHES.items():
    check(f"frozen SHA-256 {path.name}", path.is_file() and sha256(path) == expected)

with CSV.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))
summary = json.loads(JSON.read_text(encoding="utf-8"))

check("CSV data row count 1365", len(rows) == 1365)
check("JSON step row count 1365", summary["step_row_count"] == 1365)
check("JSON protocol hash", summary["protocol_sha256"] == HASHES[PROTOCOL])
check("JSON volume/horizon/support/contact", (
    summary["volume"], summary["horizon"], summary["initial_support_radius"],
    summary["locked_contact_tick"]
) == (145, 136, 4, 137))
check("horizon precedes contact", summary["horizon"] < summary["locked_contact_tick"])

groups: dict[tuple[str, str, str], list[dict[str, str]]] = defaultdict(list)
for row in rows:
    groups[(row["family"], row["direction"], row["polarity"])].append(row)
check("exact five-history key set", set(groups) == EXPECTED_KEYS)
check("five JSON arm records", summary["history_count"] == 5 and len(summary["arms"]) == 5)
json_arms = {
    (arm["family"], arm["direction"], arm["polarity"]): arm
    for arm in summary["arms"]
}
check("JSON/CSV arm keys agree", set(json_arms) == EXPECTED_KEYS)

arm_results: dict[tuple[str, str, str], dict[str, object]] = {}
all_max_common = 0.0
all_max_energy = 0.0
all_max_recoil = 0.0
all_max_speed = 0.0
all_max_regional = 0.0

for key in sorted(EXPECTED_KEYS):
    label = "/".join(key)
    arm_rows = groups[key]
    forward = [row for row in arm_rows if row["phase"] == "forward"]
    reverse = [row for row in arm_rows if row["phase"] == "reverse"]
    check(f"{label}: 273 serialized states", len(arm_rows) == 273)
    check(f"{label}: forward ticks 0..136", [integer(r, "tick") for r in forward] == list(range(137)))
    check(f"{label}: reverse roots 1..136", [integer(r, "tick") for r in reverse] == list(range(1, 137)))
    check(f"{label}: all roots valid/common", all(flag(r, "valid") and flag(r, "common") for r in arm_rows))
    check(f"{label}: forward regional observers valid", all(flag(r, "regional_valid") for r in forward))

    max_common = max(number(r, "max_residual") for r in arm_rows)
    max_energy = max(number(r, "total_energy_residual") for r in arm_rows)
    max_recoil = max(number(r, "recoil_defect") for r in arm_rows)
    max_speed = max(number(r, "speed_excess") for r in arm_rows)
    max_regional = max(number(r, "regional_residual") for r in forward)
    max_source = max(integer(r, "source_radius") for r in arm_rows)
    check(f"{label}: common residual <=1e-10", max_common <= 1e-10)
    check(f"{label}: total energy residual <=1e-8", max_energy <= 1e-8)
    check(f"{label}: recoil defect <=1e-9", max_recoil <= 1e-9)
    check(f"{label}: causal speed excess <=1e-12", max_speed <= 1e-12)
    check(f"{label}: regional residual <=1e-10", max_regional <= 1e-10)
    check(f"{label}: current support radius <=3", max_source <= 3)
    all_max_common = max(all_max_common, max_common)
    all_max_energy = max(all_max_energy, max_energy)
    all_max_recoil = max(all_max_recoil, max_recoil)
    all_max_speed = max(all_max_speed, max_speed)
    all_max_regional = max(all_max_regional, max_regional)

    graph = [flag(r, "graph_inside") for r in forward]
    pair = [number(r, "pair_energy") for r in forward]
    field = [number(r, "field_energy") for r in forward]
    outside12 = [number(r, "outside_energy_12") for r in forward]
    outward12 = [number(r, "cumulative_outward_12") for r in forward]
    transitions = [tick for tick in range(1, HORIZON + 1) if graph[tick] != graph[tick - 1]]
    serialized_transitions = [int(value) for value in json_arms[key]["transition_ticks"].split(";") if value]
    check(f"{label}: graph transitions independently reconstructed", transitions == serialized_transitions)

    pair_field_balance = abs(pair[-1] - pair[0] + field[-1] - field[0])
    check(f"{label}: pair-plus-field endpoint balance <=1e-8", pair_field_balance <= 1e-8)
    check(f"{label}: endpoint balance agrees with JSON", close(pair_field_balance, json_arms[key]["pair_field_balance"]))
    check(f"{label}: inverse recovery <=1e-8", json_arms[key]["inverse_recovery"] <= 1e-8)

    onset = next((
        tick for tick in range(HORIZON + 1)
        if all(graph[later] and pair[later] < -1e-6 for later in range(tick, HORIZON + 1))
    ), -1)
    check(f"{label}: energetic onset agrees with JSON", onset == json_arms[key]["energetic_onset_tick"])

    result: dict[str, object] = {
        "transitions": transitions,
        "onset": onset,
        "pair_field_balance": pair_field_balance,
        "max_outside": max(outside12),
        "max_outward": max(outward12),
        "max_source": max_source,
        "first_passage_residual": 0.0,
        "core": False,
        "first_passage": False,
        "tail": False,
        "bound": False,
    }

    if key[0] == "bound":
        bound = all(
            flag(row, "graph_inside") and number(row, "pair_energy") < -1e-6
            for row in arm_rows
        ) and not transitions
        check(f"{label}: bound control remains inside/negative on every stored state", bound)
        check(f"{label}: bound-control JSON flag", bool(json_arms[key]["bound_control_pass"]) == bound)
        result["bound"] = bound
    else:
        initial = forward[0]
        initial_pass = (
            not flag(initial, "graph_inside")
            and number(initial, "pair_energy") > 1e-6
            and number(initial, "outside_energy_12") <= 1e-12
        )
        check(f"{label}: unbound compact initial state", initial_pass)
        final_entry = max((tick for tick in transitions if tick <= onset and graph[tick]), default=-1)
        check(f"{label}: final entry agrees with JSON", final_entry == json_arms[key]["final_entry_tick"])
        predicted_onset = -1
        max_first_passage_residual = math.inf
        if final_entry >= 0:
            predicted = [
                pair[final_entry] - (field[tick] - field[final_entry])
                for tick in range(final_entry, HORIZON + 1)
            ]
            predicted_onset = next((
                final_entry + offset for offset, value in enumerate(predicted)
                if value < -1e-6
            ), -1)
            max_first_passage_residual = max(
                abs(predicted[offset] - pair[final_entry + offset])
                for offset in range(len(predicted))
            )
        first_passage = (
            final_entry >= 0 and predicted_onset == onset
            and max_first_passage_residual <= 1e-8
        )
        core = initial_pass and 0 <= onset <= 120 and HORIZON - onset + 1 >= 16
        first_tail = next((tick for tick, value in enumerate(outside12) if value > 1e-6), -1)
        tail = (
            outside12[0] <= 1e-12
            and max(outside12) > 1e-6
            and max(outward12) > 1e-6
            and outside12[-1] > 1e-7
            and first_tail >= 0
            and integer(forward[first_tail], "source_radius") <= 3
            and max_source <= 3
        )
        check(f"{label}: durable negative-core gate", core)
        check(f"{label}: predicted onset equals observed onset", predicted_onset == onset)
        check(f"{label}: first-passage residual <=1e-8", max_first_passage_residual <= 1e-8)
        check(f"{label}: outgoing-tail gate", tail)
        check(f"{label}: first tail tick agrees with JSON", first_tail == json_arms[key]["first_tail_tick"])
        check(f"{label}: core/first-passage/tail JSON flags", (
            bool(json_arms[key]["core_pass"]),
            bool(json_arms[key]["first_passage_pass"]),
            bool(json_arms[key]["tail_pass"]),
        ) == (core, first_passage, tail))
        result.update({
            "core": core,
            "first_passage": first_passage,
            "tail": tail,
            "first_passage_residual": max_first_passage_residual,
        })

    arm_results[key] = result


plus_key = ("unbound", "1_1_1", "plus_minus")
minus_key = ("unbound", "1_1_1", "minus_plus")
plus_rows = groups[plus_key]
minus_rows = groups[minus_key]
scalar_columns = (
    "max_residual", "total_energy_residual", "recoil_defect", "speed_excess",
    "regional_residual", "separation", "pair_energy", "field_energy",
    "inside_energy_8", "outside_energy_8", "boundary_transport_into_8",
    "source_exchange_8", "inside_energy_12", "outside_energy_12",
    "boundary_transport_into_12", "source_exchange_12", "cumulative_outward_12",
)
discrete_columns = (
    "family", "direction", "phase", "tick", "valid", "common",
    "regional_valid", "source_radius", "source_entries", "graph_inside",
)
polarity_discrete = all(
    all(lhs[column] == rhs[column] for column in discrete_columns)
    for lhs, rhs in zip(plus_rows, minus_rows, strict=True)
)
polarity_difference = max(
    abs(number(lhs, column) - number(rhs, column))
    for lhs, rhs in zip(plus_rows, minus_rows, strict=True)
    for column in scalar_columns
)
polarity_pass = (
    polarity_discrete
    and arm_results[plus_key]["transitions"] == arm_results[minus_key]["transitions"]
    and arm_results[plus_key]["onset"] == arm_results[minus_key]["onset"]
    and polarity_difference <= 1e-9
)
check("body conjugates have identical discrete histories", polarity_discrete)
check("body conjugate scalar difference <=1e-9", polarity_difference <= 1e-9)
check("polarity scalar difference agrees with JSON", close(polarity_difference, summary["polarity_scalar_difference"]))

core_count = sum(bool(result["core"]) for key, result in arm_results.items() if key[0] == "unbound")
first_passage_count = sum(bool(result["first_passage"]) for key, result in arm_results.items() if key[0] == "unbound")
tail_count = sum(bool(result["tail"]) for key, result in arm_results.items() if key[0] == "unbound")
bound_count = sum(bool(result["bound"]) for key, result in arm_results.items() if key[0] == "bound")
check("independent core count 4/4", core_count == summary["unbound_core_passes"] == 4)
check("independent first-passage count 4/4", first_passage_count == summary["unbound_first_passage_passes"] == 4)
check("independent tail count 4/4", tail_count == summary["unbound_tail_passes"] == 4)
check("independent bound control count 1/1", bound_count == summary["bound_controls"] == 1)

summary_pairs = (
    (all_max_common, summary["maximum_common_residual"], "global common residual"),
    (all_max_energy, summary["maximum_total_energy_residual"], "global energy residual"),
    (all_max_recoil, summary["maximum_recoil_defect"], "global recoil defect"),
    (all_max_speed, summary["maximum_causal_speed_excess"], "global speed excess"),
    (all_max_regional, summary["maximum_regional_residual"], "global regional residual"),
    (max(json_arms[key]["inverse_recovery"] for key in EXPECTED_KEYS), summary["maximum_inverse_recovery"], "global inverse recovery"),
    (max(float(result["pair_field_balance"]) for result in arm_results.values()), summary["maximum_pair_field_balance"], "global pair-field balance"),
    (max(float(result["first_passage_residual"]) for key, result in arm_results.items() if key[0] == "unbound"), summary["maximum_first_passage_residual"], "global first-passage residual"),
    (max(float(result["max_outside"]) for result in arm_results.values()), summary["maximum_outside_energy_12"], "global outside energy"),
    (max(float(result["max_outward"]) for result in arm_results.values()), summary["maximum_cumulative_outward_12"], "global outward transport"),
)
for measured, serialized, label in summary_pairs:
    check(f"{label} independently agrees with JSON", close(measured, serialized))
check("global source radius independently agrees with JSON", max(int(result["max_source"]) for result in arm_results.values()) == summary["maximum_source_radius"] == 3)

infrastructure = (
    len(rows) == 1365 and set(groups) == EXPECTED_KEYS
    and all(flag(row, "valid") and flag(row, "common") for row in rows)
    and all(flag(row, "regional_valid") for row in rows if row["phase"] == "forward")
    and all_max_common <= 1e-10 and all_max_energy <= 1e-8
    and all_max_recoil <= 1e-9 and all_max_speed <= 1e-12
    and all_max_regional <= 1e-10
    and max(int(result["max_source"]) for result in arm_results.values()) <= 3
    and all(json_arms[key]["inverse_recovery"] <= 1e-8 for key in EXPECTED_KEYS)
    and all(float(result["pair_field_balance"]) <= 1e-8 for result in arm_results.values())
)
if not infrastructure:
    verdict = "FINITE_SUPPORT_FORMATION_EXECUTION_INVALID"
elif bound_count != 1:
    verdict = "FINITE_SUPPORT_BOUND_CONTROL_UNSTABLE"
elif not polarity_pass:
    verdict = "FINITE_SUPPORT_FORMATION_POLARITY_SENSITIVE"
elif core_count != 4:
    verdict = "FINITE_SUPPORT_NO_DURABLE_NEGATIVE_CORE_ALL_RAYS"
elif first_passage_count != 4:
    verdict = "FINITE_SUPPORT_CAPTURE_ENERGY_LEDGER_MISMATCH"
elif tail_count != 4:
    verdict = "FINITE_SUPPORT_CORE_WITHOUT_OUTGOING_TAIL"
else:
    verdict = "FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_CONSTRUCTIVE"
check("independent verdict is constructive", verdict == "FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_CONSTRUCTIVE")
check("independent verdict agrees with JSON", verdict == summary["verdict"])

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0739 independent certificate: {passed}/{len(checks)} checks passed")
print(f"verdict={verdict}")
print(f"cores={core_count}/4 first_passage={first_passage_count}/4 tails={tail_count}/4 bound={bound_count}/1")
raise SystemExit(0 if passed == len(checks) else 1)
