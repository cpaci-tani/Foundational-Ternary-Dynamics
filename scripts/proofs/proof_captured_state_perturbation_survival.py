"""Independent run-record certificate for FTD-0732."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CAPTURED_STATE_PERTURBATION_SURVIVAL_v1.md"
TEST = ROOT / "engine/tests/test_captured_state_perturbation_survival.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0732/ftd_0732_captured_state_perturbation_survival_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0732/ftd_0732_captured_state_perturbation_survival_v1.csv"

PREREG_SHA256 = "1A93899A9960D099AC0F64E039E06A527260211393FF80F6CF833333801B0903"
TEST_SHA256 = "4D706C2A6D4F56623D5B2577C67CA1C3D8A98B9B73DAA7C500D3DCDCCED95A4E"
JSON_SHA256 = "508EAB61F17068591A48B3D61BBBA9F598FCBB330D20F301E5D9BF6E83062B09"
CSV_SHA256 = "15926F9E64B8DE3A633CCE4794B07DAF40E6293D29D97DE63C89493980C2E2AD"
VERDICT = "CAPTURE_PERTURBATION_TRANSACTION_UNRESOLVED"
CUTOFF2 = 1.5


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def close(actual: float, expected: float, tolerance: float = 1e-13) -> bool:
    return abs(actual - expected) <= tolerance * max(1.0, abs(expected))


def values(row: dict[str, str], name: str) -> list[float]:
    return [float(value) for value in row[name].split(";")]


def ticks(row: dict[str, str]) -> list[int]:
    return [] if not row["transition_ticks"] else [
        int(value) for value in row["transition_ticks"].split(";")
    ]


def arm_key(row: dict[str, str]) -> tuple[str, str, str]:
    return row["direction"], row["polarity"], row["variant"]


def inside(separation: float) -> bool:
    return separation * separation < CUTOFF2


def recompute_transitions(separation: list[float]) -> list[int]:
    membership = [inside(value) for value in separation]
    return [
        tick for tick in range(1, len(membership))
        if membership[tick] != membership[tick - 1]
    ]


def main() -> None:
    checks: list[str] = []
    check(sha256(PREREG) == PREREG_SHA256, "protocol hash locked", checks)
    check(sha256(TEST) == TEST_SHA256, "runner hash locked", checks)
    check(sha256(JSON_PATH) == JSON_SHA256, "JSON hash locked", checks)
    check(sha256(CSV_PATH) == CSV_SHA256, "CSV hash locked", checks)
    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    check(summary["identifier"] == "FTD-0732", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256,
          "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked unresolved verdict", checks)
    check(summary["arm_count"] == len(rows) == 84, "84 locked histories", checks)
    stage_a = [row for row in rows if row["volume"] == "33"]
    stage_b = [row for row in rows if row["volume"] == "65"]
    check(len(stage_a) == 66 and len(stage_b) == 18,
          "66/18 stage partition", checks)
    check(all(row["parent_valid"] == "1" for row in rows),
          "all parent centers valid", checks)
    check(all(row["parent_reproduction_pass"] == "1" for row in rows),
          "all volume-specific parent anchors reproduce", checks)

    invalid = [row for row in rows if row["initialized"] == "0"]
    valid = [row for row in rows if row["initialized"] == "1"]
    check(len(invalid) == 6 and len(valid) == 78,
          "six invalid compression probes and 78 admissible arms", checks)
    check(all(row["volume"] == "33" and row["variant"] == "separation_minus"
              for row in invalid), "only inward-compression probes invalid", checks)
    for row in invalid:
        label = f"invalid {row['direction']} {row['polarity']}"
        separation = values(row, "separation_history")
        internal = values(row, "internal_history")
        check(len(separation) == len(internal) == 1,
              f"initial invalid state persisted {label}", checks)
        check(inside(separation[0]), f"compression remains graph-inside {label}", checks)
        check(internal[0] > 0.0, f"compression pair energy positive {label}", checks)
        check(float(row["initial_gauss_residual"]) <= 1e-12,
              f"compression Gauss constraint passes {label}", checks)
        check(float(row["initial_momentum_preservation"]) <= 1e-15,
              f"compression momentum constraint passes {label}", checks)
        check(float(row["initial_maximum_speed"]) <= 1 / math.sqrt(3) + 1e-12,
              f"compression causal constraint passes {label}", checks)

    for index, row in enumerate(valid):
        label = f"valid {index} L={row['volume']} {arm_key(row)}"
        separation = values(row, "separation_history")
        internal = values(row, "internal_history")
        field = values(row, "field_history")
        check(len(separation) == len(internal) == len(field) == 257,
              f"257 persisted states {label}", checks)
        observed_ticks = recompute_transitions(separation)
        check(observed_ticks == ticks(row) == [],
              f"no graph transition recomputed {label}", checks)
        check(all(inside(value) for value in separation),
              f"all states graph-inside {label}", checks)
        check(all(value < -1e-6 for value in internal),
              f"all states negative-energy {label}", checks)
        check(all(value >= -1e-12 for value in field),
              f"all field energies nonnegative {label}", checks)
        balance = abs((field[-1] - field[0]) + (internal[-1] - internal[0]))
        check(close(balance, float(row["pair_field_balance"])),
              f"energy balance recomputed {label}", checks)
        check(balance <= 1e-8, f"energy balance gate {label}", checks)
        check(row["executed"] == row["identity_pass"] == row["recoil_pass"]
              == row["inverse_pass"] == row["positive_field_energy"]
              == row["survives"] == "1", f"all emitted gates pass {label}", checks)
        check(row["final_class"] == "survives",
              f"survival class emitted {label}", checks)
        check(float(row["initial_gauss_residual"]) <= 1e-12,
              f"initial Gauss gate {label}", checks)
        check(float(row["initial_momentum_preservation"]) <= 1e-15,
              f"initial momentum gate {label}", checks)
        check(float(row["initial_maximum_speed"]) <= 1 / math.sqrt(3) + 1e-12,
              f"initial causal gate {label}", checks)
        check(float(row["max_common_residual"]) <= 1e-10,
              f"common-action gate {label}", checks)
        check(float(row["max_recoil_defect"]) <= 1e-9,
              f"recoil gate {label}", checks)
        check(float(row["inverse_recovery"]) <= 1e-8,
              f"inverse gate {label}", checks)
        energy_margin = min(-value / 0.01 for value in internal)
        graph_margin = min(math.sqrt(CUTOFF2) - value for value in separation)
        check(close(energy_margin, float(row["minimum_energy_margin"])),
              f"energy margin recomputed {label}", checks)
        check(close(graph_margin, float(row["minimum_graph_margin"])),
              f"graph margin recomputed {label}", checks)

    check(sum(row["survives"] == "1" for row in stage_a) == 60,
          "60/66 Stage-A survivors", checks)
    check(sum(row["survives"] == "1" for row in stage_b) == 18,
          "18/18 Stage-B survivors", checks)
    centers = [row for row in rows if row["variant"] == "center"]
    check(len(centers) == 12 and all(row["survives"] == "1" for row in centers),
          "12/12 centers survive through parent tick 384", checks)

    selectors = summary["selectors"]
    check(len(selectors) == 6, "six held-out selector groups", checks)
    by_a = {arm_key(row): row for row in stage_a}
    for selector in selectors:
        direction = selector["direction"]
        polarity = selector["polarity"]
        candidates = [
            row for row in stage_a
            if row["direction"] == direction and row["polarity"] == polarity
            and row["variant"] != "center" and row["initialized"] == "1"
        ]
        energy_sorted = sorted(candidates,
            key=lambda row: (float(row["minimum_energy_margin"]), row["variant"]))
        energy_variant = energy_sorted[0]["variant"]
        graph_sorted = sorted(candidates,
            key=lambda row: (float(row["minimum_graph_margin"]), row["variant"]))
        graph_variant = next(
            row["variant"] for row in graph_sorted
            if row["variant"] != energy_variant
        )
        check(selector["energy_variant"] == energy_variant == "radial_impulse_plus",
              f"energy selector independently reproduced {direction} {polarity}", checks)
        check(selector["graph_variant"] == graph_variant == "dynamic_field_minus",
              f"graph selector independently reproduced {direction} {polarity}", checks)
        for variant in ("center", energy_variant, graph_variant):
            large = next(row for row in stage_b
                if row["direction"] == direction and row["polarity"] == polarity
                and row["variant"] == variant)
            small = by_a[(direction, polarity, variant)]
            check(large["survives"] == small["survives"] == "1",
                  f"held-out volume class match {direction} {polarity} {variant}", checks)
            check(ticks(large) == ticks(small) == [],
                  f"held-out transition match {direction} {polarity} {variant}", checks)

    for direction in {row["direction"] for row in stage_a}:
        for variant in {row["variant"] for row in stage_a}:
            plus = by_a[(direction, "plus_minus", variant)]
            minus = by_a[(direction, "minus_plus", variant)]
            check(plus["initialized"] == minus["initialized"]
                  and plus["survives"] == minus["survives"],
                  f"polarity class match {direction} {variant}", checks)

    check(summary["stage_a_survives"] == 60, "summary Stage A", checks)
    check(summary["stage_b_survives"] == 18, "summary Stage B", checks)
    check(summary["center_survives"] == 12, "summary centers", checks)
    check(summary["polarity_mismatches"] == 0, "summary polarity match", checks)
    check(summary["volume_mismatches"] == 0, "summary volume match", checks)
    check(close(summary["maximum_common"], max(
        float(row["max_common_residual"]) for row in valid)),
        "summary common maximum", checks)
    check(close(summary["maximum_recoil"], max(
        float(row["max_recoil_defect"]) for row in valid)),
        "summary recoil maximum", checks)
    check(close(summary["maximum_inverse"], max(
        float(row["inverse_recovery"]) for row in valid)),
        "summary inverse maximum", checks)
    check(close(summary["maximum_balance"], max(
        float(row["pair_field_balance"]) for row in valid)),
        "summary balance maximum", checks)

    print(f"FTD-0732 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print("invalid: 6/6 inward-compression probes start positive-energy")
    print("subordinate observation: admissible continuations 78/78 survive; centers 12/12")


if __name__ == "__main__":
    main()
