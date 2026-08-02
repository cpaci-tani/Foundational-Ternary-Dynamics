"""Independent run-record certificate for FTD-0622.

This script does not call the C++ implementation and does not trust its
summary booleans.  It reconstructs the registered arm matrix, norm identities,
cubic covariance, exactness gates, and the two width trends from the CSV.
"""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CONNECTED_MOORE_BLOCK_COMMON_ACTION_v1.md"
PARENT = ROOT / "engine/results/ftd_0621/ftd_0621_ternary_block_bipole_peierls_v1.json"
RESULT = ROOT / "engine/results/ftd_0622/ftd_0622_connected_moore_block_action_v1.json"
ARMS = ROOT / "engine/results/ftd_0622/ftd_0622_connected_moore_block_action_v1.csv"

PROTOCOL_SHA256 = "7E09ADBC2A16513DD3495BB117015F574E150F7B8BA5632C03BC96783AFE00AF"
PARENT_SHA256 = "D6ED6A0BF3C9B351ED59E4B16C0FD82430A4713B4ED06B0092F9BDCBB4026383"
RESULT_SHA256 = "6ED5287FB9AD84BACED79885E24E2352FE05CA82FA77636DD968297D6DF73396"
ARMS_SHA256 = "81E989AE992CF0D00A7FCB54118883DD0779E1A0FC03AF6D839F665CF230E49A"
VERDICT = "CONNECTED_MOORE_BLOCK_ACTION_CONSTRUCTIVE_IR_TREND_POSITIVE"
C_SPEED = 1.0 / math.sqrt(3.0)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def vector(row: dict[str, str], prefix: str) -> tuple[float, float, float]:
    return tuple(float(row[f"{prefix}_{axis}"]) for axis in "xyz")


def norm(value: tuple[float, float, float]) -> float:
    return math.sqrt(sum(component * component for component in value))


def cycle(value: tuple[float, float, float]) -> tuple[float, float, float]:
    return value[2], value[0], value[1]


def relative(lhs: float, rhs: float) -> float:
    return abs(lhs - rhs) / max(1e-300, abs(lhs), abs(rhs))


def close(lhs: float, rhs: float, tolerance: float = 2e-12) -> bool:
    return abs(lhs - rhs) <= tolerance * max(1.0, abs(lhs), abs(rhs))


summary = json.loads(RESULT.read_text(encoding="utf-8"))
with ARMS.open(newline="", encoding="utf-8") as stream:
    rows = list(csv.DictReader(stream))

checks: dict[str, bool] = {}
checks["protocol file hash"] = sha256(PREREG) == PROTOCOL_SHA256
checks["parent record hash"] = sha256(PARENT) == PARENT_SHA256
checks["result record hash"] = sha256(RESULT) == RESULT_SHA256
checks["arm record hash"] = sha256(ARMS) == ARMS_SHA256
checks["summary identity"] = summary["ftd_id"] == "FTD-0622"
checks["summary protocol"] = summary["protocol_sha256"] == PROTOCOL_SHA256
checks["summary parent"] = summary["parent_result_sha256"] == PARENT_SHA256
checks["production frozen"] = summary["production_changed"] is False

expected = {
    (1, 0, 0, 0.0), (1, 0, 0, 0.25), (1, 0, 1, 0.25),
    (2, 0, 0, 0.0), (2, 0, 0, 0.25), (2, 0, 1, 0.25),
    (3, 0, 0, 0.0), (3, 0, 0, 0.25), (3, 0, 1, 0.25),
    (2, 1, 1, 0.25), (2, 1, 2, 0.25),
    (2, 2, 2, 0.25), (2, 2, 0, 0.25),
}
observed = {
    (int(row["width"]), int(row["orientation"]),
     int(row["phase_axis"]), float(row["phase"]))
    for row in rows
}
checks["registered arm coverage"] = len(rows) == 13 and observed == expected
checks["unique arm labels"] = len({row["label"] for row in rows}) == 13
checks["row identities"] = all(row["ftd_id"] == "FTD-0622" for row in rows)

expected_edges = {1: 1, 2: 72, 3: 365}
checks["exact constituent counts"] = all(
    int(row["constituents"]) == 2 * int(row["width"]) ** 3 for row in rows
)
checks["exact Moore edge counts"] = all(
    int(row["edges"]) == expected_edges[int(row["width"])] for row in rows
)
checks["all algebra and inverse flags"] = all(
    all(row[key] == "1" for key in ("init", "forward", "reverse", "count", "no_hop", "rest"))
    for row in rows
)
checks["iteration lock"] = all(
    int(row["forward_iterations"]) <= 48 and int(row["reverse_iterations"]) <= 48
    for row in rows
)
checks["aggregate exactness"] = all(float(row["maximum_common_residual"]) <= 1e-10 for row in rows)
checks["state-only inversion"] = all(float(row["recovery"]) <= 1e-8 for row in rows)

checks["local vector norms"] = all(
    close(norm(vector(row, "local")), float(row["local_defect"])) for row in rows
)
checks["spline vector norms"] = all(
    close(norm(vector(row, "spline")), float(row["spline_defect"])) for row in rows
)
checks["centre vector norms"] = all(
    close(norm(vector(row, "center")), float(row["center_displacement"])) for row in rows
)
checks["normalization identity"] = all(
    close(
        float(row["normalized_spline_defect"]),
        C_SPEED * float(row["spline_defect"]) / float(row["field_energy"]),
    )
    for row in rows
)

rest_rows = [row for row in rows if float(row["phase"]) == 0.0]
checks["integer-phase centre rest"] = all(
    norm(vector(row, "matter")) <= 1e-8
    and float(row["center_displacement"]) <= 1e-8
    for row in rest_rows
)


def find(width: int, orientation: int, phase_axis: int) -> dict[str, str]:
    matches = [
        row for row in rows
        if int(row["width"]) == width
        and int(row["orientation"]) == orientation
        and int(row["phase_axis"]) == phase_axis
        and float(row["phase"]) == 0.25
    ]
    assert len(matches) == 1
    return matches[0]


checks["Peierls trend parallel"] = all(
    float(find(width, 0, 0)["peierls_index"])
    > float(find(width + 1, 0, 0)["peierls_index"])
    for width in (1, 2)
)
checks["Peierls trend transverse"] = all(
    float(find(width, 0, 1)["peierls_index"])
    > float(find(width + 1, 0, 1)["peierls_index"])
    for width in (1, 2)
)
checks["reaction-defect trend parallel"] = all(
    float(find(width, 0, 0)["normalized_spline_defect"])
    > float(find(width + 1, 0, 0)["normalized_spline_defect"])
    for width in (1, 2)
)
checks["reaction-defect trend transverse"] = all(
    float(find(width, 0, 1)["normalized_spline_defect"])
    > float(find(width + 1, 0, 1)["normalized_spline_defect"])
    for width in (1, 2)
)

covariance_residuals: list[float] = []
for phase_class, base_axis in (("parallel", 0), ("transverse", 1)):
    base = find(2, 0, base_axis)
    for turns in (1, 2):
        rotated_axis = turns if phase_class == "parallel" else (turns + 1) % 3
        rotated = find(2, turns, rotated_axis)
        for prefix in ("matter", "local", "spline", "center"):
            expected_vector = vector(base, prefix)
            for _ in range(turns):
                expected_vector = cycle(expected_vector)
            covariance_residuals.extend(
                abs(lhs - rhs)
                for lhs, rhs in zip(vector(rotated, prefix), expected_vector)
            )
        for key in ("field_energy", "normalized_spline_defect", "maximum_edge_strain"):
            covariance_residuals.append(relative(float(rotated[key]), float(base[key])))

reconstructed_covariance = max(covariance_residuals)
checks["cubic covariance"] = reconstructed_covariance <= 1e-8
checks["summary covariance reconstruction"] = close(
    reconstructed_covariance, float(summary["worst_covariance_residual"]), 1e-10
)
checks["summary residual reconstruction"] = close(
    max(float(row["maximum_common_residual"]) for row in rows),
    float(summary["worst_common_residual"]),
)
checks["summary recovery reconstruction"] = close(
    max(float(row["recovery"]) for row in rows),
    float(summary["worst_recovery"]),
)
checks["summary gate flags"] = all(
    summary[key] == 1
    for key in (
        "parent_pass", "coverage_pass", "small_width_pass", "all_width_pass",
        "covariance_pass", "rest_pass", "peierls_trend_pass", "defect_trend_pass",
    )
)
checks["registered verdict"] = summary["verdict"] == VERDICT

failed = [name for name, passed in checks.items() if not passed]
print(f"FTD-0622 independent certificate: {len(checks) - len(failed)}/{len(checks)} checks pass")
for name, passed in checks.items():
    print(f"{'PASS' if passed else 'FAIL'}  {name}")
if failed:
    raise SystemExit("failed checks: " + ", ".join(failed))
