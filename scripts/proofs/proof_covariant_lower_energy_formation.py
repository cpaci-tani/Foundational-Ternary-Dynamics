"""Independent run-record certificate for FTD-0726."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_COVARIANT_LOWER_ENERGY_FORMATION_v1.md"
TEST = ROOT / "engine/tests/test_covariant_lower_energy_formation.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0726/ftd_0726_covariant_lower_energy_formation_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0726/ftd_0726_covariant_lower_energy_formation_v1.csv"

PREREG_SHA256 = "8C484A05DC94F4099687757660F6D0873E614A7D55FAE40637539BECEFF4A335"
TEST_SHA256 = "1C13A6DDF707C46C0262B0CBED84F5C961BE89651D356A35240B2C5D5EA499FC"
JSON_SHA256 = "FE73C4FBCBB3D1FB796D0BB2A758FF8EC3A867915A1711E91713C7FC407D697D"
CSV_SHA256 = "8A428E7F8E248A64E3287278E5E0BDE75EB82ED409A22D1D54DFEDFE2F993146"
VERDICT = "COVARIANT_ENERGETIC_TRAPPING_WITHOUT_DETACHED_FIELD"
MOMENTA = (0.0060, 0.0075, 0.0085, 0.0095, 0.0120)
EXPECTED_NEGATIVE = (52, 52, 52, 52, 0)
E_REST = 0.511 / 3.0
C_SPEED_SQUARED = 1.0 / 3.0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def close(actual: float, expected: float, tolerance: float = 1e-13) -> bool:
    return abs(actual - expected) <= tolerance * max(1.0, abs(expected))


def pair_kinetic(momentum: float) -> float:
    return 2.0 * (
        math.sqrt(E_REST * E_REST + C_SPEED_SQUARED * momentum * momentum)
        - E_REST
    )


def main() -> None:
    checks: list[str] = []
    check(sha256(PREREG) == PREREG_SHA256, "protocol hash locked", checks)
    check(sha256(TEST) == TEST_SHA256, "runner hash locked", checks)
    check(sha256(JSON_PATH) == JSON_SHA256, "JSON hash locked", checks)
    check(sha256(CSV_PATH) == CSV_SHA256, "CSV hash locked", checks)

    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    check(summary["identifier"] == "FTD-0726", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256, "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked verdict", checks)
    check(summary["arm_count"] == 312, "312 summary arms", checks)
    check(len(rows) == 312, "312 CSV arms", checks)
    check(summary["executed_arms"] == 312, "all arms executed", checks)
    check(summary["identity_pass_arms"] == 312, "all rowwise identity arms pass", checks)
    check(summary["inverse_pass_arms"] == 312, "all inverse arms pass", checks)
    check(summary["recoil_pass_arms"] == 312, "all recoil arms pass", checks)

    directions = {
        "0_0_1", "0_1_-1", "0_1_0", "0_1_1", "1_-1_-1",
        "1_-1_0", "1_-1_1", "1_0_-1", "1_0_0", "1_0_1",
        "1_1_-1", "1_1_0", "1_1_1",
    }
    check({row["direction"] for row in rows} == directions, "13 Moore rays", checks)
    check({row["family"] for row in rows} == {"unbound", "bound"}, "two families", checks)
    check({row["polarity"] for row in rows} == {"plus_minus", "minus_plus"}, "polarity mirrors", checks)
    check({row["translation"] for row in rows} == {"origin", "shifted"}, "translation copies", checks)
    check(all(row["initialized"] == "1" for row in rows), "all initial dressings valid", checks)
    check(all(row["executed"] == "1" for row in rows), "all histories complete", checks)
    check(all(row["identity_pass"] == "1" for row in rows), "rowwise common-action identities", checks)
    check(all(row["inverse_pass"] == "1" for row in rows), "rowwise state inverses", checks)
    check(all(row["recoil_pass"] == "1" for row in rows), "rowwise recoil symmetry", checks)

    unbound = [row for row in rows if row["family"] == "unbound"]
    bound = [row for row in rows if row["family"] == "bound"]
    check(len(unbound) == 260 and len(bound) == 52, "260 unbound and 52 bound arms", checks)
    check({float(row["momentum"]) for row in unbound} == set(MOMENTA), "five locked momenta", checks)
    check(all(row["bound_control_pass"] == "1" for row in bound), "all bound controls pass", checks)
    check(summary["bound_control_pass_arms"] == 52, "summary bound controls", checks)
    check(all(int(row["graph_transitions"]) == 0 for row in bound), "bound graph retained", checks)
    check(all(int(row["active_ticks"]) == 48 for row in bound), "bound active 48 ticks", checks)

    max_common = max(float(row["max_common_residual"]) for row in rows)
    max_recoil = max(float(row["max_recoil_defect"]) for row in rows)
    max_inverse = max(float(row["inverse_recovery"]) for row in rows)
    max_balance = max(float(row["pair_field_balance"]) for row in rows)
    check(max_common <= 1e-10, "rowwise common residual gate", checks)
    check(max_recoil <= 1e-9, "rowwise recoil gate", checks)
    check(max_inverse <= 1e-8, "rowwise inverse gate", checks)
    check(max_balance <= 1e-8, "rowwise energy-transfer gate", checks)
    check(close(max_common, summary["maximum_common_residual"]), "summary common maximum", checks)
    check(close(max_recoil, summary["maximum_recoil_defect"]), "summary recoil maximum", checks)
    check(close(max_inverse, summary["maximum_inverse_recovery"]), "summary inverse maximum", checks)
    check(close(max_balance, summary["maximum_pair_field_balance"]), "summary balance maximum", checks)

    check(summary["maximum_scalar_history_spread"] <= 1e-9, "locked scalar covariance gate passes", checks)
    check(close(summary["maximum_scalar_history_spread"], 8.9040064210621495e-10), "scalar spread value locked", checks)

    grouped: list[list[dict[str, str]]] = []
    for momentum, expected_negative in zip(MOMENTA, EXPECTED_NEGATIVE):
        group = [row for row in unbound if float(row["momentum"]) == momentum]
        grouped.append(group)
        check(len(group) == 52, f"52 arms p={momentum:.4f}", checks)
        initial_values = {float(row["initial_pair_internal"]) for row in group}
        check(len(initial_values) == 1, f"common initial energy p={momentum:.4f}", checks)
        check(close(next(iter(initial_values)), pair_kinetic(momentum)), f"dispersion energy p={momentum:.4f}", checks)
        negative = sum(row["negative_sector"] == "1" for row in group)
        captured = sum(row["captured"] == "1" for row in group)
        check(negative == expected_negative, f"raw negative count p={momentum:.4f}", checks)
        check(captured == 0, f"raw capture count p={momentum:.4f}", checks)
        check(all(float(row["energy_export"]) > 0.0 for row in group), f"positive field export p={momentum:.4f}", checks)
        check(all(float(row["dynamic_field_norm"]) > 1e-8 for row in group), f"dynamic field p={momentum:.4f}", checks)
        check(all(float(row["magnetic_energy"]) > 1e-10 for row in group), f"magnetic field p={momentum:.4f}", checks)
        if expected_negative:
            check(all(int(row["graph_transitions"]) == 1 for row in group), f"raw trapping topology p={momentum:.4f}", checks)
            check(all(float(row["final_pair_internal"]) < -1e-6 for row in group), f"raw negative final energy p={momentum:.4f}", checks)
            check(all(int(row["dynamic_median_radius2"]) == 3 for row in group), f"raw radius-three morphology p={momentum:.4f}", checks)
        else:
            check(all(int(row["graph_transitions"]) == 2 for row in group), f"raw escape topology p={momentum:.4f}", checks)
            check(all(float(row["final_pair_internal"]) > 1e-6 for row in group), f"raw positive final energy p={momentum:.4f}", checks)
            check(all(int(row["dynamic_median_radius2"]) == 5 for row in group), f"raw radius-five morphology p={momentum:.4f}", checks)

    check(summary["negative_sector_unbound_arms"] == 208, "summary raw negative count 208", checks)
    check(summary["captured_unbound_arms"] == 0, "summary capture count zero", checks)
    check(summary["monotone_negative_fraction"] is True, "raw negative fractions monotone", checks)
    check(summary["monotone_capture_fraction"] is True, "raw capture fractions monotone", checks)

    summaries = summary["momentum_summaries"]
    check(len(summaries) == 5, "five JSON momentum summaries", checks)
    for momentum, expected_negative, group, item in zip(MOMENTA, EXPECTED_NEGATIVE, grouped, summaries):
        check(close(item["momentum"], momentum), f"summary momentum p={momentum:.4f}", checks)
        check(item["arms"] == 52, f"summary arms p={momentum:.4f}", checks)
        check(item["negative_sector"] == expected_negative, f"summary negative p={momentum:.4f}", checks)
        check(item["captured"] == 0, f"summary capture p={momentum:.4f}", checks)
        exports = [float(row["energy_export"]) for row in group]
        finals = [float(row["final_pair_internal"]) for row in group]
        check(close(item["minimum_energy_export"], min(exports)), f"summary export min p={momentum:.4f}", checks)
        check(close(item["maximum_energy_export"], max(exports)), f"summary export max p={momentum:.4f}", checks)
        check(close(item["minimum_final_pair_internal"], min(finals)), f"summary final min p={momentum:.4f}", checks)
        check(close(item["maximum_final_pair_internal"], max(finals)), f"summary final max p={momentum:.4f}", checks)

    print(f"FTD-0726 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print(f"scalar_history_spread={summary['maximum_scalar_history_spread']:.17g}")
    print("negative_counts=52/52/52/52/0; qualified_capture=0/260")
    for momentum, group in zip(MOMENTA, grouped):
        exports = [float(row["energy_export"]) for row in group]
        finals = [float(row["final_pair_internal"]) for row in group]
        print(
            f"p={momentum:.4f} export=[{min(exports):.17g},{max(exports):.17g}] "
            f"final=[{min(finals):.17g},{max(finals):.17g}]"
        )


if __name__ == "__main__":
    main()
