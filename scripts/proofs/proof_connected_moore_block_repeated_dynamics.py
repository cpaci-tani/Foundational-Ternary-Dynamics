"""Independent run-record certificate for FTD-0623."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CONNECTED_MOORE_BLOCK_REPEATED_DYNAMICS_v1.md"
PARENT = ROOT / "engine/results/ftd_0622/ftd_0622_connected_moore_block_action_v1.json"
RESULT = ROOT / "engine/results/ftd_0623/ftd_0623_connected_moore_block_repeated_v1.json"
ARMS = ROOT / "engine/results/ftd_0623/ftd_0623_connected_moore_block_repeated_arms_v1.csv"
TICKS = ROOT / "engine/results/ftd_0623/ftd_0623_connected_moore_block_repeated_ticks_v1.csv"

PROTOCOL_SHA = "7AA42C401938C48F134A1BF95C70FD8C6026B24B0FE2979173BBEF598800A3F7"
PARENT_SHA = "6ED5287FB9AD84BACED79885E24E2352FE05CA82FA77636DD968297D6DF73396"
RESULT_SHA = "4E86C850BB1354EC1A9C738FF1C50B94D558528966FED2F0EE40B26B67D69926"
ARMS_SHA = "1C102BF26A5A2313814BDAD688CC45362E440F725213EB214670DBD89F740E08"
TICKS_SHA = "C810B1B8020DAB4DD3705383F2425A2401B833CBAC82F0ED7E1B33101B51D80D"
VERDICT = "CONNECTED_INTEGER_OBJECT_REPEATED_MOBILITY_CONSTRUCTIVE"
C_SPEED = 1.0 / math.sqrt(3.0)
E_REST = 0.511 * C_SPEED * C_SPEED
LAUNCH_P = 0.12


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def vec(row: dict[str, str], prefix: str) -> tuple[float, float, float]:
    names = (f"{prefix}_x", f"{prefix}_y", f"{prefix}_z")
    return tuple(float(row[name]) for name in names)


def norm(v: tuple[float, float, float]) -> float:
    return math.sqrt(sum(x * x for x in v))


def add(a: tuple[float, float, float], b: tuple[float, float, float]):
    return tuple(x + y for x, y in zip(a, b))


def sub(a: tuple[float, float, float], b: tuple[float, float, float]):
    return tuple(x - y for x, y in zip(a, b))


def cycle(v: tuple[float, float, float]):
    return v[2], v[0], v[1]


def maximum(v: tuple[float, float, float]) -> float:
    return max(abs(x) for x in v)


def relative(a: float, b: float) -> float:
    return abs(a - b) / max(1e-300, abs(a), abs(b))


def close(a: float, b: float, tolerance: float = 2e-12) -> bool:
    return abs(a - b) <= tolerance * max(1.0, abs(a), abs(b))


summary = json.loads(RESULT.read_text(encoding="utf-8"))
with ARMS.open(newline="", encoding="utf-8") as stream:
    arms = list(csv.DictReader(stream))
with TICKS.open(newline="", encoding="utf-8") as stream:
    ticks = list(csv.DictReader(stream))
by_label = {row["label"]: row for row in arms}
histories = {
    label: sorted((row for row in ticks if row["label"] == label),
                  key=lambda row: int(row["tick"]))
    for label in by_label
}

checks: dict[str, bool] = {}
checks["protocol hash"] = sha(PREREG) == PROTOCOL_SHA
checks["parent hash"] = sha(PARENT) == PARENT_SHA
checks["result hash"] = sha(RESULT) == RESULT_SHA
checks["arms hash"] = sha(ARMS) == ARMS_SHA
checks["ticks hash"] = sha(TICKS) == TICKS_SHA
checks["summary identity"] = summary["ftd_id"] == "FTD-0623"
checks["summary protocol"] = summary["protocol_sha256"] == PROTOCOL_SHA
checks["summary parent"] = summary["parent_result_sha256"] == PARENT_SHA
checks["production frozen"] = summary["production_changed"] is False

labels = {
    "rest", "parallel_positive", "parallel_negative",
    "transverse_positive", "cyclic_parallel",
}
checks["arm coverage"] = len(arms) == 5 and set(by_label) == labels
checks["tick coverage"] = len(ticks) == 80 and all(
    [int(row["tick"]) for row in histories[label]] == list(range(1, 17))
    for label in labels
)
checks["row identities"] = all(row["ftd_id"] == "FTD-0623" for row in arms + ticks)
checks["action flags"] = all(
    all(row[key] == "1" for key in ("init", "forward", "reverse", "coherence"))
    for row in arms
)
checks["per-tick exactness"] = all(float(row["common_residual"]) <= 1e-10 for row in ticks)
checks["energy sector identity"] = all(
    close(float(row["kinetic"]) + float(row["binding"]) + float(row["field"]),
          float(row["total"]))
    for row in ticks
)
checks["trajectory energy drift"] = all(float(row["energy_drift"]) <= 1e-9 for row in ticks)
checks["coherence gates"] = all(
    float(row["shape_error"]) <= 0.25 and float(row["edge_strain"]) <= 0.25
    for row in ticks
)

for label, arm in by_label.items():
    history = histories[label]
    checks[f"{label} hop reconstruction"] = sum(int(row["site_hops"]) for row in history) == int(arm["total_hops"])
    checks[f"{label} shape maximum"] = close(max(float(row["shape_error"]) for row in history), float(arm["maximum_shape_error"]))
    checks[f"{label} strain maximum"] = close(max(float(row["edge_strain"]) for row in history), float(arm["maximum_edge_strain"]))
    forward_maximum = max(float(row["common_residual"]) for row in history)
    full_maximum = float(arm["maximum_common_residual"])
    checks[f"{label} residual envelope"] = (
        forward_maximum <= full_maximum + 1e-15 and full_maximum <= 1e-10
    )
    checks[f"{label} drift maximum"] = close(max(float(row["energy_drift"]) for row in history), float(arm["maximum_energy_drift"]))
    checks[f"{label} final momentum"] = maximum(sub(vec(history[-1], "momentum"), vec(arm, "final_momentum"))) <= 2e-12
    checks[f"{label} cumulative D"] = close(float(history[-1]["cumulative_D"]), float(arm["cumulative_D"]))
    cumulative = (0.0, 0.0, 0.0)
    ratios: list[float] = []
    for row in history:
        cumulative = add(cumulative, vec(row, "spline"))
        if norm(cumulative) > 1e-14:
            ratios.append(float(row["cumulative_D"]) / norm(cumulative))
    checks[f"{label} cumulative-vector normalization"] = not ratios or max(ratios) - min(ratios) <= 2e-10 * max(ratios)

initial_center = (7.5, 7.5, 7.5)
for label, arm in by_label.items():
    displacement = sub(vec(histories[label][-1], "center"), initial_center)
    checks[f"{label} displacement reconstruction"] = maximum(sub(displacement, vec(arm, "center"))) <= 2e-12

free_energy = math.sqrt(E_REST * E_REST + C_SPEED * C_SPEED * LAUNCH_P * LAUNCH_P)
free_speed = C_SPEED * C_SPEED * LAUNCH_P / free_energy
checks["free-dispersion reconstruction"] = all(
    close(float(by_label[label]["free_speed"]), free_speed)
    and close(float(by_label[label]["free_displacement"]), 16.0 * free_speed)
    for label in labels - {"rest"}
)
rest = by_label["rest"]
checks["rest stability"] = (
    norm(vec(rest, "center")) <= 1e-8
    and norm(vec(rest, "final_momentum")) <= 1e-8
    and int(rest["total_hops"]) == 0
    and rest["rest_pass"] == "1"
)
checks["boost transport gates"] = all(
    row["transport_pass"] == "1"
    and float(row["projected_displacement"]) >= 0.75
    and norm(vec(row, "center")) <= 1.5 * float(row["free_displacement"])
    and float(row["transverse_displacement"]) <= 0.10
    and int(row["total_hops"]) >= 16
    for label, row in by_label.items() if label != "rest"
)
checks["inverse recovery"] = all(float(row["recovery"]) <= 1e-8 for row in arms)

pos, neg = by_label["parallel_positive"], by_label["parallel_negative"]
mirror_residuals = [maximum(add(vec(pos, "center"), vec(neg, "center")))]
for a, b in zip(histories["parallel_positive"], histories["parallel_negative"]):
    mirror_residuals += [abs(float(a["field"]) - float(b["field"])),
                         abs(float(a["shape_error"]) - float(b["shape_error"]))]
mirror = max(mirror_residuals)
checks["sign mirror"] = mirror <= 1e-8
checks["summary mirror reconstruction"] = close(mirror, float(summary["worst_sign_mirror_residual"]))

rot = by_label["cyclic_parallel"]
covariance_residuals = [
    maximum(sub(vec(rot, "center"), cycle(vec(pos, "center")))),
    maximum(sub(vec(rot, "final_momentum"), cycle(vec(pos, "final_momentum")))),
    relative(float(rot["cumulative_D"]), float(pos["cumulative_D"])),
    abs(int(rot["total_hops"]) - int(pos["total_hops"])),
]
for a, b in zip(histories["parallel_positive"], histories["cyclic_parallel"]):
    covariance_residuals += [
        maximum(sub(vec(b, "center"), cycle(vec(a, "center")))),
        maximum(sub(vec(b, "momentum"), cycle(vec(a, "momentum")))),
        relative(float(b["field"]), float(a["field"])),
        relative(float(b["shape_error"]), float(a["shape_error"])),
        relative(float(b["edge_strain"]), float(a["edge_strain"])),
        abs(int(b["site_hops"]) - int(a["site_hops"])),
    ]
covariance = max(covariance_residuals)
checks["cubic covariance"] = covariance <= 1e-8
checks["summary covariance reconstruction"] = close(covariance, float(summary["worst_covariance_residual"]), 1e-10)
checks["summary common reconstruction"] = close(max(float(row["maximum_common_residual"]) for row in arms), float(summary["worst_common_residual"]))
checks["summary drift reconstruction"] = close(max(float(row["maximum_energy_drift"]) for row in arms), float(summary["worst_energy_drift"]))
checks["summary recovery reconstruction"] = close(max(float(row["recovery"]) for row in arms), float(summary["worst_recovery"]))
checks["summary flags"] = all(summary[key] == 1 for key in (
    "parent_pass", "coverage_pass", "action_pass", "rest_pass",
    "mobility_pass", "sign_mirror_pass", "covariance_pass"))
checks["registered verdict"] = summary["verdict"] == VERDICT

failed = [name for name, passed in checks.items() if not passed]
print(f"FTD-0623 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
for name, passed in checks.items():
    print(f"{'PASS' if passed else 'FAIL'}  {name}")
if failed:
    raise SystemExit("failed checks: " + ", ".join(failed))
