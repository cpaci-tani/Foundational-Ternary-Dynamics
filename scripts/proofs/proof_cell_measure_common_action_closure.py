#!/usr/bin/env python3
"""Independent run-of-record certificate for FTD-0649."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "engine/results/ftd_0649"
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CELL_MEASURE_COMMON_ACTION_CLOSURE_v1.md"
RUNNER = ROOT / "engine/tests/test_cell_measure_common_action_closure.cpp"
JSON_PATH = BASE / "ftd_0649_cell_measure_common_action_closure_v1.json"
ARMS_PATH = BASE / "ftd_0649_cell_measure_common_action_closure_arms_v1.csv"

HASHES = {
    PREREG: "612172E79EF58526FC4EE02DE84EDEA0AC6EEF6EDF1F52160EF8F35363AA7C5A",
    RUNNER: "256DD29974620F0C1F0B52A09D362CB9F8B03DA1CC2E7A24965C5134E8DEC53D",
    JSON_PATH: "7549A16D786279EE9E2AAC21BEC3B6C1CAD6712DA36100068E0126D1A5FBD9D3",
    ARMS_PATH: "301C188C4A4F14D96A2E9E197BDF1FF0F5FF5649E908906FA32AD94E5D2B88A1",
}
VERDICT = "CELL_MEASURE_RECIPROCAL_COMMON_ACTION_CONSTRUCTIVE"
E_REST = 0.511 / 3.0
M_INERTIAL = 0.511


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def close(a: float, b: float, tolerance: float = 1e-12) -> bool:
    return abs(a-b) <= tolerance * max(1.0, abs(a), abs(b))


def truth(value: str) -> bool:
    return value.lower() == "true"


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS {label}")


def main() -> None:
    for path, expected in HASHES.items():
        require(digest(path) == expected, f"hash {path.name}")
    record = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with ARMS_PATH.open(newline="", encoding="utf-8") as handle:
        arms = list(csv.DictReader(handle))

    require(record["protocol_sha256"] == HASHES[PREREG], "embedded protocol hash")
    require(record["verdict"] == VERDICT, "locked verdict")
    require(record["production_changed"] is False, "production unchanged")
    require(len(arms) == 45 and record["arm_count"] == 45, "locked arm count")
    for gate in ("coverage_pass", "execution_pass", "exact_pass",
                 "mirror_pass", "cubic_pass", "zero_pass",
                 "default_regression_pass"):
        require(record[gate] is True, gate)

    expected = set()
    for width in (2, 3, 4):
        for orientation in range(3):
            for axis in range(3):
                expected.add((width, orientation, axis, 1, "primary"))
            expected.add((width, orientation, 0, 0, "zero"))
        for axis in range(3):
            expected.add((width, 0, axis, -1, "mirror"))
    observed = {(int(row["width"]), int(row["orientation"]),
                 int(row["axis"]), int(row["sign"]), row["kind"])
                for row in arms}
    require(observed == expected, "complete width/cubic/mirror matrix")

    exact_columns = (
        "root", "force", "continuity", "gauss_before", "gauss_after",
        "kinetic_gradient", "electric_adjoint", "magnetic_work",
        "binding_work", "binding_sum", "matter_work", "field_work",
        "total_energy",
    )
    for row in arms:
        width = int(row["width"])
        a = 2.0 / width
        require(truth(row["initialized"]), f"initialized {row['label']}")
        require(truth(row["forward"]) and truth(row["reverse"]),
                f"forward/reverse {row['label']}")
        require(truth(row["exact"]) and truth(row["zero"]),
                f"action/static gates {row['label']}")
        require(close(float(row["mass_scale"]), a**3), f"mass scale {row['label']}")
        require(close(float(row["polarity_scale"]), a**3), f"polarity scale {row['label']}")
        require(close(float(row["binding_scale"]), a**3), f"binding scale {row['label']}")
        require(close(float(row["field_scale"]), 1.0/a), f"field scale {row['label']}")
        require(close(float(row["rest_energy"]), 16.0*E_REST),
                f"fixed rest energy {row['label']}")
        require(close(float(row["inertial_mass"]), 16.0*M_INERTIAL),
                f"fixed inertial mass {row['label']}")
        require(close(float(row["integrated_positive"]), 8.0),
                f"fixed positive polarity {row['label']}")
        require(float(row["forward_solve_residual"]) <= 2e-11,
                f"forward root {row['label']}")
        require(float(row["reverse_solve_residual"]) <= 2e-11,
                f"reverse root {row['label']}")
        for column in exact_columns:
            require(float(row[column]) <= 1e-9, f"{column} {row['label']}")
        require(float(row["causal"]) <= 1e-12, f"causal {row['label']}")
        require(float(row["recovery"]) <= 1e-8, f"recovery {row['label']}")

    require(record["worst_action_residual"] <= 1e-9, "summary action residual")
    require(record["worst_recovery"] <= 1e-8, "summary inverse recovery")
    require(record["mirror_residual"] <= 1e-7, "summary polarity mirror")
    require(record["cubic_residual"] <= 1e-7, "summary cubic covariance")
    require(record["worst_zero_displacement"] <= 1e-6, "summary zero control")
    print("FTD-0649 certificate complete")


if __name__ == "__main__":
    main()
