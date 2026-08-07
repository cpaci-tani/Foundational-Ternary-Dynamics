"""Independent run-record certificate for FTD-0725."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_LOWER_ENERGY_COVARIANCE_CONDITIONING_v1.md"
TEST = ROOT / "engine/tests/test_lower_energy_covariance_conditioning.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0725/ftd_0725_lower_energy_covariance_conditioning_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0725/ftd_0725_lower_energy_covariance_conditioning_v1.csv"

PREREG_SHA256 = "712F491F72E9F30239060406FAA85EBB0F3635DFD3A8BD2143CBF68249A7DCB9"
TEST_SHA256 = "4424F879FACDF56917F5E2FE4C11E41A71169E1337DAC457D72448E91CF4B54D"
JSON_SHA256 = "829A76E2187F389318D71C8D3035957FD29E106D61D1DFBF0220006463E9E89E"
CSV_SHA256 = "C82EFD332A1D6CD02FD339E9F63D6B6BB58AD31BE5549AA22F53A962D27E32BD"
VERDICT = "COVARIANCE_DEFECT_NUMERICAL_CONDITIONING_CONFIRMED"
PARENT_SCALAR = 1.0680766715509549e-8


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def close(actual: float, expected: float, tolerance: float = 1e-15) -> bool:
    return abs(actual - expected) <= tolerance * max(1.0, abs(expected))


def maximum_row(rows: list[dict[str, str]], field: str) -> dict[str, str]:
    return max(rows, key=lambda row: float(row[field]))


def main() -> None:
    checks: list[str] = []
    check(sha256(PREREG) == PREREG_SHA256, "protocol hash locked", checks)
    check(sha256(TEST) == TEST_SHA256, "runner hash locked", checks)
    check(sha256(JSON_PATH) == JSON_SHA256, "JSON hash locked", checks)
    check(sha256(CSV_PATH) == CSV_SHA256, "CSV hash locked", checks)

    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    check(summary["identifier"] == "FTD-0725", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256, "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked verdict", checks)
    check(summary["tick_record_count"] == 7644, "summary tick count", checks)
    check(len(rows) == 7644, "CSV tick count", checks)
    check({row["condition"] for row in rows} == {"baseline", "tight"}, "two conditions", checks)
    check({row["family"] for row in rows} == {"unbound", "bound"}, "two families", checks)
    check({int(row["tick"]) for row in rows} == set(range(49)), "ticks zero through 48", checks)

    conditions = {item["label"]: item for item in summary["conditions"]}
    check(set(conditions) == {"baseline", "tight"}, "JSON conditions", checks)
    expected_directions = {
        "0_0_1", "0_1_-1", "0_1_0", "0_1_1", "1_-1_-1",
        "1_-1_0", "1_-1_1", "1_0_-1", "1_0_0", "1_0_1",
        "1_1_-1", "1_1_0", "1_1_1",
    }
    check({row["direction"] for row in rows} == expected_directions, "13 Moore rays", checks)

    fields = {
        "scalar": "scalar_difference",
        "electric": "electric_difference",
        "magnetic": "magnetic_difference",
        "matter": "matter_difference",
        "complete": "complete_state_difference",
        "common": "root_common_residual",
        "recoil": "recoil_defect",
    }
    for label in ("baseline", "tight"):
        item = conditions[label]
        group = [row for row in rows if row["condition"] == label]
        check(len(group) == 3822, f"3822 tick records {label}", checks)
        check(item["pairs"] == 78, f"78 pairs {label}", checks)
        check(item["histories"] == 156, f"156 histories {label}", checks)
        check(item["executed_histories"] == 156, f"all histories execute {label}", checks)
        check(item["gate_pass_histories"] == 156, f"all rowwise gates pass {label}", checks)
        check(item["class_agreement_pairs"] == 78, f"all translation classes agree {label}", checks)
        check(item["negative_unbound_histories"] == 104, f"raw negative count {label}", checks)
        check(item["bound_control_pass_histories"] == 26, f"bound controls {label}", checks)
        for name, field in fields.items():
            row = maximum_row(group, field)
            key = "maximum_common_residual" if name == "common" else (
                "maximum_recoil_defect" if name == "recoil" else
                f"maximum_{name}_difference"
            )
            check(close(float(row[field]), item[key]), f"recompute {name} maximum {label}", checks)
        scalar = maximum_row(group, "scalar_difference")
        complete = maximum_row(group, "complete_state_difference")
        check(scalar["family"] == item["worst_scalar_family"], f"scalar family {label}", checks)
        check(close(float(scalar["momentum"]), item["worst_scalar_momentum"]), f"scalar momentum {label}", checks)
        check(scalar["direction"] == item["worst_scalar_direction"], f"scalar direction {label}", checks)
        check(int(scalar["tick"]) == item["worst_scalar_tick"], f"scalar tick {label}", checks)
        scalar_components = {
            "separation": float(scalar["separation_difference"]),
            "pair_internal": float(scalar["internal_difference"]),
            "field_energy": float(scalar["field_difference"]),
        }
        check(max(scalar_components, key=scalar_components.get) == item["worst_scalar_component"], f"scalar component {label}", checks)
        check(complete["family"] == item["worst_complete_family"], f"complete family {label}", checks)
        check(close(float(complete["momentum"]), item["worst_complete_momentum"]), f"complete momentum {label}", checks)
        check(complete["direction"] == item["worst_complete_direction"], f"complete direction {label}", checks)
        check(int(complete["tick"]) == item["worst_complete_tick"], f"complete tick {label}", checks)
        complete_components = {
            "electric": float(complete["electric_difference"]),
            "magnetic": float(complete["magnetic_difference"]),
            "matter": float(complete["matter_difference"]),
        }
        check(max(complete_components, key=complete_components.get) == item["worst_complete_component"], f"complete component {label}", checks)
        check(item["maximum_common_residual"] <= (2.1e-11 if label == "baseline" else 2.1e-12), f"root residual scale {label}", checks)
        check(item["maximum_recoil_defect"] <= 1e-9, f"recoil gate {label}", checks)

    baseline = conditions["baseline"]
    tight = conditions["tight"]
    check(close(baseline["maximum_scalar_difference"], PARENT_SCALAR), "parent scalar defect reproduced", checks)
    check(baseline["maximum_scalar_difference"] > 1e-9, "baseline scalar gate fails", checks)
    check(baseline["maximum_complete_difference"] > 1e-9, "baseline complete gate fails", checks)
    check(tight["maximum_scalar_difference"] <= 1e-9, "tight scalar gate passes", checks)
    check(tight["maximum_complete_difference"] <= 1e-9, "tight complete gate passes", checks)
    scalar_ratio = tight["maximum_scalar_difference"] / baseline["maximum_scalar_difference"]
    complete_ratio = tight["maximum_complete_difference"] / baseline["maximum_complete_difference"]
    check(close(scalar_ratio, summary["tight_to_baseline_scalar_ratio"]), "scalar ratio recomputed", checks)
    check(close(complete_ratio, summary["tight_to_baseline_complete_ratio"]), "complete ratio recomputed", checks)
    check(scalar_ratio <= 0.2, "scalar improves at least fivefold", checks)
    check(complete_ratio <= 0.2, "complete state improves at least fivefold", checks)
    check(tight["maximum_electric_difference"] <= 1e-9, "tight electric covariance", checks)
    check(tight["maximum_magnetic_difference"] <= 1e-9, "tight magnetic covariance", checks)
    check(tight["maximum_matter_difference"] <= 1e-9, "tight matter covariance", checks)

    for label in ("baseline", "tight"):
        final = [row for row in rows if row["condition"] == label
                 and row["family"] == "unbound" and row["tick"] == "48"]
        check(len(final) == 65, f"65 final unbound pairs {label}", checks)
        origin_negative = sum(float(row["origin_internal"]) < -1e-6
                              and row["origin_edge"] == "1" for row in final)
        shifted_negative = sum(float(row["shifted_internal"]) < -1e-6
                               and row["shifted_edge"] == "1" for row in final)
        check(origin_negative == 52, f"52 origin raw negatives {label}", checks)
        check(shifted_negative == 52, f"52 shifted raw negatives {label}", checks)
        check(all((float(row["origin_internal"]) < -1e-6)
                  == (float(row["shifted_internal"]) < -1e-6)
                  for row in final), f"final sign agreement {label}", checks)

    print(f"FTD-0725 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print(
        f"scalar={baseline['maximum_scalar_difference']:.17g} -> "
        f"{tight['maximum_scalar_difference']:.17g} "
        f"(ratio {scalar_ratio:.17g})"
    )
    print(
        f"complete={baseline['maximum_complete_difference']:.17g} -> "
        f"{tight['maximum_complete_difference']:.17g} "
        f"(ratio {complete_ratio:.17g})"
    )


if __name__ == "__main__":
    main()
