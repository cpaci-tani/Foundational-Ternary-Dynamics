"""Independent run-record certificate for FTD-0723."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_FIELD_ASSISTED_CAPTURE_WINDOW_v1.md"
TEST = ROOT / "engine/tests/test_field_assisted_capture_window.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0723/ftd_0723_field_assisted_capture_window_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0723/ftd_0723_field_assisted_capture_window_v1.csv"

PREREG_SHA256 = "EBAF990F2DF6121DDC4E0E7A79A492B2A30D6D59CD29DF3DE54CC2B266B84CC6"
TEST_SHA256 = "05AA224853D3CF4219002975102901C04E0C3E036EFCDA5BC80061E6DDA307E7"
JSON_SHA256 = "E785C1061CD715B64414DD4685F80DDF2BC4C9A047B1EA2FB124834F45D38895"
CSV_SHA256 = "6B8B3CE2EB93E6DC3AD7977B0CC388DB2218C026235EC3E2E681842E5C3F60F5"
VERDICT = "NO_CAPTURE_WINDOW_OBSERVED_LOCKED_V1"
MOMENTA = (0.0200, 0.0225, 0.0250, 0.0275, 0.0300)
E_REST = 0.511 / 3.0
C_SPEED_SQUARED = 1.0 / 3.0
PARENT_EXPORT_MINIMUM = 0.0012012704657176076
PARENT_EXPORT_MAXIMUM = 0.001374812629945682


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


def threshold_momentum(exported_energy: float) -> float:
    half_pair_energy = exported_energy / 2.0
    return math.sqrt(
        ((E_REST + half_pair_energy) ** 2 - E_REST**2)
        / C_SPEED_SQUARED
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

    check(summary["identifier"] == "FTD-0723", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256, "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked verdict", checks)
    check(summary["arm_count"] == 312, "312 summary arms", checks)
    check(len(rows) == 312, "312 CSV arms", checks)
    check(summary["executed_arms"] == 312, "all arms executed", checks)
    check(summary["identity_pass_arms"] == 312, "all identity arms pass", checks)
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
    check(all(row["identity_pass"] == "1" for row in rows), "rowwise identities", checks)
    check(all(row["inverse_pass"] == "1" for row in rows), "rowwise inverses", checks)
    check(all(row["recoil_pass"] == "1" for row in rows), "rowwise recoil symmetry", checks)

    unbound = [row for row in rows if row["family"] == "unbound"]
    bound = [row for row in rows if row["family"] == "bound"]
    check(len(unbound) == 260 and len(bound) == 52, "260 unbound and 52 bound arms", checks)
    check({float(row["momentum"]) for row in unbound} == set(MOMENTA), "five locked momenta", checks)
    check({float(row["momentum"]) for row in bound} == {0.015}, "locked bound momentum", checks)
    check(all(int(row["graph_transitions"]) == 2 for row in unbound), "every encounter enters and exits", checks)
    check(all(float(row["initial_pair_internal"]) > 1e-6 for row in unbound), "unbound starts positive", checks)
    check(all(float(row["final_pair_internal"]) > 1e-6 for row in unbound), "unbound ends positive", checks)
    check(all(row["negative_sector"] == "0" for row in unbound), "no negative-sector arm", checks)
    check(all(row["captured"] == "0" for row in unbound), "no captured arm", checks)
    check(summary["captured_unbound_arms"] == 0, "summary capture count zero", checks)
    check(summary["negative_sector_unbound_arms"] == 0, "summary negative count zero", checks)

    check(all(int(row["graph_transitions"]) == 0 for row in bound), "bound graph retained", checks)
    check(all(int(row["active_ticks"]) == 24 for row in bound), "bound active all ticks", checks)
    check(all(float(row["initial_pair_internal"]) < -1e-6 for row in bound), "bound starts negative", checks)
    check(all(float(row["final_pair_internal"]) < -1e-6 for row in bound), "bound ends negative", checks)
    check(all(row["bound_control_pass"] == "1" for row in bound), "all bound controls pass", checks)
    check(summary["bound_control_pass_arms"] == 52, "summary bound count", checks)

    max_common = max(float(row["max_common_residual"]) for row in rows)
    max_recoil = max(float(row["max_recoil_defect"]) for row in rows)
    max_inverse = max(float(row["inverse_recovery"]) for row in rows)
    max_balance = max(float(row["pair_field_balance"]) for row in rows)
    check(max_common <= 1e-10, "common residual gate", checks)
    check(max_recoil <= 1e-9, "recoil gate", checks)
    check(max_inverse <= 1e-8, "inverse gate", checks)
    check(max_balance <= 1e-8, "energy-transfer gate", checks)
    check(close(max_common, summary["maximum_common_residual"]), "summary common maximum", checks)
    check(close(max_recoil, summary["maximum_recoil_defect"]), "summary recoil maximum", checks)
    check(close(max_inverse, summary["maximum_inverse_recovery"]), "summary inverse maximum", checks)
    check(close(max_balance, summary["maximum_pair_field_balance"]), "summary balance maximum", checks)
    check(summary["maximum_scalar_history_spread"] <= 1e-9, "translation/polarity history gate", checks)

    lower_threshold = threshold_momentum(PARENT_EXPORT_MINIMUM)
    upper_threshold = threshold_momentum(PARENT_EXPORT_MAXIMUM)
    check(close(lower_threshold, 0.024797812323480179), "parent lower threshold reproduced", checks)
    check(close(upper_threshold, 0.026531996461401596), "parent upper threshold reproduced", checks)
    check(MOMENTA[1] < lower_threshold < MOMENTA[3], "held-out bracket ordering", checks)
    check(MOMENTA[1] < upper_threshold < MOMENTA[3], "held-out upper ordering", checks)

    grouped: list[list[dict[str, str]]] = []
    for momentum in MOMENTA:
        group = [row for row in unbound if float(row["momentum"]) == momentum]
        grouped.append(group)
        check(len(group) == 52, f"52 arms at p={momentum:.4f}", checks)
        initial_values = {float(row["initial_pair_internal"]) for row in group}
        check(len(initial_values) == 1, f"common initial energy p={momentum:.4f}", checks)
        initial = next(iter(initial_values))
        check(close(initial, pair_kinetic(momentum)), f"dispersion energy p={momentum:.4f}", checks)
        exports = [float(row["energy_export"]) for row in group]
        check(all(exported > 0.0 for exported in exports), f"positive field export p={momentum:.4f}", checks)
        check(max(exports) < initial, f"export insufficient p={momentum:.4f}", checks)
        check(all(float(row["pair_field_balance"]) <= 1e-8 for row in group), f"pair-field balance p={momentum:.4f}", checks)
        check(all(float(row["dynamic_field_norm"]) > 1e-8 for row in group), f"dynamic field p={momentum:.4f}", checks)
        check(all(float(row["magnetic_energy"]) > 1e-10 for row in group), f"magnetic field p={momentum:.4f}", checks)
        check(all(int(row["dynamic_median_radius2"]) == 2 for row in group), f"radius-two morphology p={momentum:.4f}", checks)

    minimum_exports = [min(float(row["energy_export"]) for row in group) for group in grouped]
    maximum_exports = [max(float(row["energy_export"]) for row in group) for group in grouped]
    check(all(a < b for a, b in zip(minimum_exports, minimum_exports[1:])), "minimum export rises with momentum", checks)
    check(all(a < b for a, b in zip(maximum_exports, maximum_exports[1:])), "maximum export rises with momentum", checks)
    check(maximum_exports[0] < PARENT_EXPORT_MINIMUM, "low-p export below parent range", checks)
    check(summary["monotone_capture_fraction"] is True, "zero capture fractions monotone", checks)

    summaries = summary["momentum_summaries"]
    check(len(summaries) == 5, "five JSON momentum summaries", checks)
    for momentum, group, item in zip(MOMENTA, grouped, summaries):
        check(close(item["momentum"], momentum), f"summary momentum p={momentum:.4f}", checks)
        check(item["arms"] == 52, f"summary arms p={momentum:.4f}", checks)
        check(item["captured"] == 0, f"summary captures p={momentum:.4f}", checks)
        check(item["negative_sector"] == 0, f"summary negative p={momentum:.4f}", checks)
        check(item["graph_transitions"] == 104, f"summary transitions p={momentum:.4f}", checks)
        exports = [float(row["energy_export"]) for row in group]
        finals = [float(row["final_pair_internal"]) for row in group]
        check(close(item["minimum_energy_export"], min(exports)), f"summary export minimum p={momentum:.4f}", checks)
        check(close(item["maximum_energy_export"], max(exports)), f"summary export maximum p={momentum:.4f}", checks)
        check(close(item["minimum_final_pair_internal"], min(finals)), f"summary final minimum p={momentum:.4f}", checks)
        check(close(item["maximum_final_pair_internal"], max(finals)), f"summary final maximum p={momentum:.4f}", checks)

    print(f"FTD-0723 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print(f"threshold_bracket=[{lower_threshold:.17g},{upper_threshold:.17g}]")
    for momentum, group in zip(MOMENTA, grouped):
        initial = float(group[0]["initial_pair_internal"])
        exports = [float(row["energy_export"]) for row in group]
        finals = [float(row["final_pair_internal"]) for row in group]
        print(
            f"p={momentum:.4f} initial={initial:.17g} "
            f"export=[{min(exports):.17g},{max(exports):.17g}] "
            f"final=[{min(finals):.17g},{max(finals):.17g}]"
        )


if __name__ == "__main__":
    main()
