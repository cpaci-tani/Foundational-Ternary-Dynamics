"""Independent record, Hessian, and fixed-point certificate for FTD-0628."""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0628"
PROTOCOL = "4B6CA4AD4ACF106124AAF9C791AF4F7B3374DC30DF3A5A9FDEDC784F66D640C6"
PARENT = "E3451D9230A87610B68F8DF27D67C5D536C5582B24818ACF6CB93FDB7E62AE93"

checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


def read_csv(name: str) -> list[dict[str, str]]:
    with (RESULTS / name).open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def jacobi_eigenvalues(matrix: list[list[float]]) -> list[float]:
    a = [row[:] for row in matrix]
    for _ in range(128):
        p, q = 0, 1
        largest = abs(a[p][q])
        for i in range(4):
            for j in range(i + 1, 4):
                if abs(a[i][j]) > largest:
                    largest, p, q = abs(a[i][j]), i, j
        if largest < 1e-13:
            break
        angle = 0.5 * math.atan2(2.0 * a[p][q], a[q][q] - a[p][p])
        c, s = math.cos(angle), math.sin(angle)
        for k in range(4):
            if k not in (p, q):
                akp, akq = a[k][p], a[k][q]
                a[k][p] = a[p][k] = c * akp - s * akq
                a[k][q] = a[q][k] = s * akp + c * akq
        app, aqq, apq = a[p][p], a[q][q], a[p][q]
        a[p][p] = c * c * app - 2.0 * c * s * apq + s * s * aqq
        a[q][q] = s * s * app + 2.0 * c * s * apq + c * c * aqq
        a[p][q] = a[q][p] = 0.0
    return sorted(a[i][i] for i in range(4))


with (RESULTS / "ftd_0628_connected_block_static_dressing_refinement_v1.json").open(
    encoding="utf-8"
) as handle:
    summary = json.load(handle)
arms = read_csv("ftd_0628_connected_block_static_dressing_arms_v1.csv")
optimization = read_csv("ftd_0628_connected_block_static_dressing_optimization_v1.csv")
ticks = read_csv("ftd_0628_connected_block_static_dressing_ticks_v1.csv")

arms_by_label = {row["label"]: row for row in arms}
optimization_by_label: dict[str, list[dict[str, str]]] = {}
ticks_by_label: dict[str, list[dict[str, str]]] = {}
for row in optimization:
    optimization_by_label.setdefault(row["label"], []).append(row)
for row in ticks:
    ticks_by_label.setdefault(row["label"], []).append(row)

check("record id", summary["ftd_id"] == "FTD-0628")
check("protocol lock", summary["protocol_sha256"] == PROTOCOL)
check("parent result", summary["parent_result_sha256"] == PARENT)
check("production unchanged", summary["production_changed"] is False)
check("coverage gate", summary["coverage_pass"] == 1)
check("initialization gate", summary["initialization_pass"] == 1)
check("ansatz stationarity gate", summary["ansatz_stationarity_pass"] == 1)
check("full-space stationarity gate", summary["full_space_stationarity_pass"] == 1)
check("repeated fixed-point gate", summary["repeated_fixed_point_pass"] == 1)
check("covariance gate", summary["covariance_pass"] == 1)
check("rotated state", float(summary["rotated_state_residual"]) <= 1e-9)
check("covariance residual", float(summary["covariance_residual"]) <= 1e-9)
check("common action", float(summary["worst_common_residual"]) <= 1e-10)
check("energy drift", float(summary["worst_energy_drift"]) <= 1e-12)
check("recovery", float(summary["worst_recovery"]) <= 1e-10)
check(
    "verdict",
    summary["verdict"] == "CONNECTED_BLOCK_STATIC_DRESSED_FIXED_POINT_CONSTRUCTIVE",
)

check("two cyclic arms", set(arms_by_label) == {"static_x", "static_y"})
check("orientations", arms_by_label["static_x"]["orientation"] == "0" and arms_by_label["static_y"]["orientation"] == "1")
check("three Newton records", all(len(optimization_by_label[label]) == 3 for label in arms_by_label))
check("64 ticks per arm", all(len(ticks_by_label[label]) == 64 for label in arms_by_label))

for label, arm in arms_by_label.items():
    records = optimization_by_label[label]
    arm_ticks = ticks_by_label[label]
    gradient = [float(arm[f"g{i}"]) for i in range(4)]
    hessian = [[float(arm[f"h{i}{j}"]) for j in range(4)] for i in range(4)]
    recorded_eigenvalues = sorted(float(arm[f"e{i}"]) for i in range(4))
    recomputed_eigenvalues = jacobi_eigenvalues(hessian)

    check(f"{label} all gates flagged", all(arm[key] == "1" for key in (
        "init", "optimization", "positive_hessian", "one_step",
        "full_stationarity", "forward", "reverse", "repeated"
    )))
    check(f"{label} rigid start", all(math.isclose(float(records[0][key]), value, abs_tol=1e-15) for key, value in zip(("a", "b", "t_outer", "t_inner"), (1.5, 0.5, 0.5, 0.5))))
    energies = [float(row["energy"]) for row in records]
    check(f"{label} monotone Newton energy", energies[0] > energies[1] > energies[2])
    check(f"{label} accepted scales", [float(row["accepted_scale"]) for row in records] == [1.0, 1.0, 0.0])
    check(f"{label} final record", all(math.isclose(float(records[-1][key]), float(arm[target]), rel_tol=0.0, abs_tol=2e-15) for key, target in (("a", "a"), ("b", "b"), ("t_outer", "t_outer"), ("t_inner", "t_inner"), ("energy", "refined_energy"))))
    check(f"{label} lower energy", float(arm["refined_energy"]) < float(arm["rigid_energy"]))
    check(f"{label} gradient recomputation", max(abs(value) for value in gradient) <= 1e-9 and math.isclose(max(abs(value) for value in gradient), float(records[-1]["gradient_inf"]), rel_tol=2e-6, abs_tol=2e-13))
    check(f"{label} Hessian symmetry", max(abs(hessian[i][j] - hessian[j][i]) for i in range(4) for j in range(4)) <= 1e-14)
    check(f"{label} Hessian eigenvalues", all(math.isclose(a, b, rel_tol=2e-12, abs_tol=2e-10) for a, b in zip(recorded_eigenvalues, recomputed_eigenvalues)))
    check(f"{label} positive Hessian", min(recomputed_eigenvalues) > 1e-6)
    check(f"{label} full impulse", float(arm["max_impulse"]) <= 1e-9)
    check(f"{label} first state", float(arm["first_displacement"]) <= 1e-9 and float(arm["first_momentum"]) <= 1e-9)
    check(f"{label} tick coverage", [int(row["tick"]) for row in arm_ticks] == list(range(1, 65)))
    check(f"{label} centre fixed", max(float(row["center_displacement"]) for row in arm_ticks) <= 1e-10)
    check(f"{label} internal fixed", max(float(row["state_distance"]) for row in arm_ticks) <= 1e-8)
    check(f"{label} tick energy", max(float(row["energy_drift"]) for row in arm_ticks) <= 1e-12)
    check(f"{label} tick common action", max(float(row["common"]) for row in arm_ticks) <= 1e-10)
    check(f"{label} fibre", max(int(row["multiplicity"]) for row in arm_ticks) <= 2 and min(float(row["separation"]) for row in arm_ticks) >= 0.9)
    check(f"{label} recovery", float(arm["recovery"]) <= 1e-10)
    check(f"{label} arm maxima", math.isclose(float(arm["max_state_distance"]), max(float(row["state_distance"]) for row in arm_ticks), rel_tol=0.0, abs_tol=1e-18) and math.isclose(float(arm["max_energy_drift"]), max(float(row["energy_drift"]) for row in arm_ticks), rel_tol=0.0, abs_tol=1e-18))

x, y = arms_by_label["static_x"], arms_by_label["static_y"]
check("cyclic refined coordinates", max(abs(float(x[key]) - float(y[key])) for key in ("a", "b", "t_outer", "t_inner")) <= 1e-9)
check("outer contraction", all(float(row["a"]) < 1.5 for row in arms))
check("inner contraction", all(float(row["b"]) < 0.5 for row in arms))
check("transverse expansion", all(float(row["t_outer"]) > 0.5 and float(row["t_inner"]) > 0.5 for row in arms))

failed = [name for name, passed in checks if not passed]
for name, passed in checks:
    print(f"{'PASS' if passed else 'FAIL'}: {name}")
print(f"\n{sum(passed for _, passed in checks)}/{len(checks)} checks pass")
if failed:
    raise SystemExit("failed checks: " + ", ".join(failed))
