"""Independent run-record certificate for FTD-0722."""

from __future__ import annotations

import csv
import hashlib
import json
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_FIELD_ASSISTED_DERIVED_PAIR_CAPTURE_v1.md"
TEST = ROOT / "engine/tests/test_field_assisted_derived_pair_capture.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0722/ftd_0722_field_assisted_derived_pair_capture_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0722/ftd_0722_field_assisted_derived_pair_capture_v1.csv"

PREREG_SHA256 = "19594ECA39EC9489A3D07BC1AC04021BC1D4FC3597B0E8AFEE55312A51E09C68"
TEST_SHA256 = "C694C32D5428F0A09B6F12A58FD91EDD7940A27ED53F6A3BDD35036BDCB58537"
JSON_SHA256 = "1AAE192D20C5B745D079307B7A3C64B394C9C15ED5E168FF3B1DD2DBFC85E582"
CSV_SHA256 = "546A36472E79698D4554AB942EBD8EE13820AFE616E4799D00E9E9AE1DA1B9C5"
VERDICT = "FIELD_ASSISTED_CAPTURE_NOT_OBSERVED_LOCKED_V1"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def potential(d: Fraction) -> Fraction:
    if d >= Fraction(3, 2):
        return Fraction(0)
    return -16 * Fraction(1, 100) * (d - Fraction(3, 2)) ** 2 * (
        d - Fraction(3, 4)
    )


def close(actual: float, expected: float, tolerance: float = 1e-15) -> bool:
    return abs(actual - expected) <= tolerance * max(1.0, abs(expected))


def main() -> None:
    checks: list[str] = []
    check(sha256(PREREG) == PREREG_SHA256, "protocol hash locked", checks)
    check(sha256(TEST) == TEST_SHA256, "runner hash locked", checks)
    check(sha256(JSON_PATH) == JSON_SHA256, "JSON hash locked", checks)
    check(sha256(CSV_PATH) == CSV_SHA256, "CSV hash locked", checks)

    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    check(summary["identifier"] == "FTD-0722", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256, "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked verdict", checks)
    check(summary["arm_count"] == 104, "104 summary arms", checks)
    check(len(rows) == 104, "104 CSV arms", checks)
    check(summary["executed_arms"] == 104, "all arms executed", checks)
    check(summary["identity_pass_arms"] == 104, "all identity arms pass", checks)
    check(summary["inverse_pass_arms"] == 104, "all inverse arms pass", checks)
    check(summary["recoil_pass_arms"] == 104, "all recoil arms pass", checks)

    directions = {
        "0_0_1", "0_1_-1", "0_1_0", "0_1_1", "1_-1_-1",
        "1_-1_0", "1_-1_1", "1_0_-1", "1_0_0", "1_0_1",
        "1_1_-1", "1_1_0", "1_1_1",
    }
    check({row["direction"] for row in rows} == directions, "13 Moore rays", checks)
    check({row["family"] for row in rows} == {"scattering", "bound"}, "two families", checks)
    check({row["polarity"] for row in rows} == {"plus_minus", "minus_plus"}, "polarity mirrors", checks)
    check({row["translation"] for row in rows} == {"origin", "shifted"}, "translation copies", checks)
    check(all(row["initialized"] == "1" for row in rows), "all initial dressings valid", checks)
    check(all(row["executed"] == "1" for row in rows), "all histories complete", checks)
    check(all(row["identity_pass"] == "1" for row in rows), "rowwise identities", checks)
    check(all(row["inverse_pass"] == "1" for row in rows), "rowwise inverses", checks)
    check(all(row["recoil_pass"] == "1" for row in rows), "rowwise recoil symmetry", checks)

    scattering = [row for row in rows if row["family"] == "scattering"]
    bound = [row for row in rows if row["family"] == "bound"]
    check(len(scattering) == 52 and len(bound) == 52, "52 arms per family", checks)
    check(all(int(row["graph_transitions"]) == 2 for row in scattering), "every encounter enters and exits", checks)
    check(all(int(row["active_ticks"]) == 11 for row in scattering), "11 interaction ticks per encounter", checks)
    check(all(float(row["initial_pair_internal"]) > 1e-6 for row in scattering), "unbound starts positive", checks)
    check(all(float(row["final_pair_internal"]) > 1e-6 for row in scattering), "unbound ends positive", checks)
    check(all(row["negative_sector"] == "0" for row in scattering), "no unbound negative-sector arm", checks)
    check(all(row["captured"] == "0" for row in scattering), "no captured arm", checks)
    check(summary["captured_unbound_arms"] == 0, "summary capture count zero", checks)
    check(summary["negative_sector_unbound_arms"] == 0, "summary negative count zero", checks)

    check(all(int(row["graph_transitions"]) == 0 for row in bound), "bound graph retained", checks)
    check(all(int(row["active_ticks"]) == 24 for row in bound), "bound active all ticks", checks)
    check(all(float(row["initial_pair_internal"]) < -1e-6 for row in bound), "bound starts negative", checks)
    check(all(float(row["final_pair_internal"]) < -1e-6 for row in bound), "bound ends negative", checks)
    check(all(row["bound_control_pass"] == "1" for row in bound), "all bound controls pass", checks)
    check(summary["bound_control_pass_arms"] == 52, "summary bound count", checks)

    check(all(float(row["final_pair_internal"]) < float(row["initial_pair_internal"]) for row in scattering), "field removes pair energy", checks)
    check(all(float(row["final_field_energy"]) > float(row["initial_field_energy"]) for row in scattering), "field receives encounter energy", checks)
    check(all(float(row["dynamic_field_norm"]) > 1e-8 for row in scattering), "dynamic field nonzero", checks)
    check(all(float(row["magnetic_energy"]) > 1e-10 for row in scattering), "magnetic field nonzero", checks)
    check(all(float(row["pair_field_balance"]) <= 1e-8 for row in rows), "pair-field balance", checks)

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

    initial = float(scattering[0]["initial_pair_internal"])
    final_min = min(float(row["final_pair_internal"]) for row in scattering)
    final_max = max(float(row["final_pair_internal"]) for row in scattering)
    transfer_min = min(
        float(row["initial_pair_internal"]) - float(row["final_pair_internal"])
        for row in scattering
    )
    transfer_max = max(
        float(row["initial_pair_internal"]) - float(row["final_pair_internal"])
        for row in scattering
    )
    check(initial > final_max > final_min > 0.0, "energy loss insufficient for capture", checks)
    check(transfer_min > 0.0 and transfer_max < initial, "strict partial energy export", checks)

    check(potential(Fraction(0)) == Fraction(27, 100), "repulsive core exact", checks)
    check(potential(Fraction(1)) == Fraction(-1, 100), "well minimum exact", checks)
    check(potential(Fraction(3, 2)) == 0, "compact threshold exact", checks)
    check(Fraction(0) > potential(Fraction(1)), "bound sector below continuum", checks)

    print(f"FTD-0722 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print(f"unbound_initial={initial:.17g}")
    print(f"unbound_final_range=[{final_min:.17g},{final_max:.17g}]")
    print(f"exported_energy_range=[{transfer_min:.17g},{transfer_max:.17g}]")


if __name__ == "__main__":
    main()
