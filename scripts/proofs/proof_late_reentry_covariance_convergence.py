"""Independent run-record certificate for FTD-0729."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_LATE_REENTRY_COVARIANCE_CONVERGENCE_v1.md"
TEST = ROOT / "engine/tests/test_late_reentry_covariance_convergence.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0729/ftd_0729_late_reentry_covariance_convergence_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0729/ftd_0729_late_reentry_covariance_convergence_v1.csv"

PREREG_SHA256 = "96751A97197E6F52625FFECD53CF7B66752960290968530196E9A8F9A52AD384"
TEST_SHA256 = "983228218500B11A6CDE27386260C3AD662A814830C1858A9A0C09B5CC7B6A16"
JSON_SHA256 = "C9EF34EBEC55484B8658A2010A68F71B9B5828F7ACD62F64EC3289C24C1AD6E9"
CSV_SHA256 = "204724D260D33641DA6B90F2D96250650BBAF5D35169D5EC4467EE4092BE2F3D"
VERDICT = "LATE_REENTRY_ROOT_CONDITIONING_CONFIRMED"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def close(actual: float, expected: float, tolerance: float = 1e-13) -> bool:
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

    check(summary["identifier"] == "FTD-0729", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256, "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked verdict", checks)
    check(summary["tick_record_count"] == 582, "582 tick records", checks)
    check(len(rows) == 582, "582 CSV rows", checks)
    check({row["condition"] for row in rows} == {"parent", "tight", "ultra"}, "three conditions", checks)
    check({row["polarity"] for row in rows} == {"plus_minus", "minus_plus"}, "two polarity pairs", checks)

    conditions = summary["conditions"]
    check([item["label"] for item in conditions] == ["parent", "tight", "ultra"], "condition order", checks)
    for item in conditions:
        label = item["label"]
        group = [row for row in rows if row["condition"] == label]
        check(len(group) == 194, f"194 records {label}", checks)
        check(item["pairs"] == 2 and item["histories"] == 4, f"pair/history matrix {label}", checks)
        check(item["executed_histories"] == 4, f"executed histories {label}", checks)
        check(item["gate_pass_histories"] == 4, f"gate-pass histories {label}", checks)
        check(item["class_agreement_pairs"] == 2, f"class agreement {label}", checks)
        check(item["graph_transitions"] == 12, f"three transitions per history {label}", checks)
        check(item["final_negative_histories"] == 0, f"final sign class {label}", checks)
        check(item["maximum_common"] <= 1e-10, f"common residual {label}", checks)
        check(item["maximum_recoil"] <= 1e-9, f"recoil residual {label}", checks)
        csv_scalar = max(float(row["scalar_difference"]) for row in group)
        csv_electric = max(float(row["electric_difference"]) for row in group)
        csv_magnetic = max(float(row["magnetic_difference"]) for row in group)
        csv_matter = max(float(row["matter_difference"]) for row in group)
        csv_complete = max(float(row["complete_difference"]) for row in group)
        check(close(item["maximum_scalar"], csv_scalar), f"scalar summary {label}", checks)
        check(close(item["maximum_electric"], csv_electric), f"electric summary {label}", checks)
        check(close(item["maximum_magnetic"], csv_magnetic), f"magnetic summary {label}", checks)
        check(close(item["maximum_matter"], csv_matter), f"matter summary {label}", checks)
        check(close(item["maximum_complete"], csv_complete), f"complete summary {label}", checks)

    parent, tight, ultra = conditions
    check(close(tight["plus_minus_scalar"], 5.6798055148021831e-10), "FTD-0728 worst reproduced", checks)
    check(ultra["maximum_scalar"] <= 1e-9, "ultra scalar below gate", checks)
    check(ultra["maximum_complete"] <= 1e-9, "ultra complete below gate", checks)
    scalar_ratio = ultra["maximum_scalar"] / tight["maximum_scalar"]
    complete_ratio = ultra["maximum_complete"] / tight["maximum_complete"]
    check(close(summary["ultra_to_tight_scalar_ratio"], scalar_ratio), "scalar ratio recomputed", checks)
    check(close(summary["ultra_to_tight_complete_ratio"], complete_ratio), "complete ratio recomputed", checks)
    check(scalar_ratio <= 0.2, "scalar fivefold gate", checks)
    check(complete_ratio <= 0.2, "complete fivefold gate", checks)
    check(close(ultra["maximum_scalar"], 6.2501115394297813e-12), "ultra scalar locked", checks)
    check(close(ultra["maximum_complete"], 2.9611868512802175e-12), "ultra complete locked", checks)
    check(ultra["worst_scalar_tick"] == 92, "ultra scalar tick", checks)
    check(ultra["worst_scalar_component"] == "separation", "ultra scalar component", checks)
    check(ultra["worst_complete_tick"] == 92, "ultra complete tick", checks)
    check(ultra["worst_complete_component"] == "matter", "ultra complete component", checks)
    check(ultra["maximum_electric"] < tight["maximum_electric"], "electric converges", checks)
    check(ultra["maximum_magnetic"] < tight["maximum_magnetic"], "magnetic converges", checks)
    check(ultra["maximum_matter"] < tight["maximum_matter"], "matter converges", checks)

    print(f"FTD-0729 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print(f"scalar_ratio={scalar_ratio:.17g}; complete_ratio={complete_ratio:.17g}")
    print(f"ultra_scalar={ultra['maximum_scalar']:.17g}; ultra_complete={ultra['maximum_complete']:.17g}")


if __name__ == "__main__":
    main()
