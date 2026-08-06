"""Independent eigensystem and trajectory certificate for FTD-0629."""

from __future__ import annotations

import csv
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine" / "results" / "ftd_0629"
PROTOCOL = "BF823BB629BFAB7FA385E39AB83E4BDCC2DCA3E857EE424FAF65C3280898CB4F"
PARENT = "7C4B9A8E71D1D10CE2D5409CB17F25F23695CD54A426812CDA918F2A7A45AA6A"
THETA0 = [1.4993153663084844, 0.4994670538459639,
          0.50006590532229034, 0.50018096647517352]
HESSIAN = [
    [63.984246918488694, -64.008998464613882, 64.005401398219135, 64.022814124818296],
    [-64.008998464613882, 191.66543224202746, -64.016058330196643, 63.852374345947993],
    [64.005401398219135, -64.016058330196643, 288.10845102769656, 96.075177717647179],
    [64.022814124818296, 63.852374345947993, 96.075177717647179, 480.33216079722968],
]
MASSES = [8 * 0.511, 8 * 0.511, 16 * 0.511, 16 * 0.511]

checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


def read_csv(name: str) -> list[dict[str, str]]:
    with (RESULTS / name).open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def diagonalize(matrix: list[list[float]]) -> tuple[list[float], list[list[float]]]:
    a = [row[:] for row in matrix]
    vectors = [[1.0 if i == j else 0.0 for j in range(4)] for i in range(4)]
    for _ in range(128):
        p, q = 0, 1
        largest = abs(a[p][q])
        for i in range(4):
            for j in range(i + 1, 4):
                if abs(a[i][j]) > largest:
                    largest, p, q = abs(a[i][j]), i, j
        if largest < 1e-13:
            break
        angle = 0.5 * math.atan2(2 * a[p][q], a[q][q] - a[p][p])
        c, s = math.cos(angle), math.sin(angle)
        for k in range(4):
            if k not in (p, q):
                kp, kq = a[k][p], a[k][q]
                a[k][p] = a[p][k] = c * kp - s * kq
                a[k][q] = a[q][k] = s * kp + c * kq
        pp, qq, pq = a[p][p], a[q][q], a[p][q]
        a[p][p] = c * c * pp - 2 * c * s * pq + s * s * qq
        a[q][q] = s * s * pp + 2 * c * s * pq + c * c * qq
        a[p][q] = a[q][p] = 0.0
        for k in range(4):
            kp, kq = vectors[k][p], vectors[k][q]
            vectors[k][p] = c * kp - s * kq
            vectors[k][q] = s * kp + c * kq
    order = sorted(range(4), key=lambda i: a[i][i])
    values = [a[i][i] for i in order]
    columns: list[list[float]] = []
    for column in order:
        vector = [vectors[i][column] / math.sqrt(MASSES[i]) for i in range(4)]
        pivot = max(range(4), key=lambda i: abs(vector[i]))
        if vector[pivot] < 0:
            vector = [-value for value in vector]
        columns.append(vector)
    return values, columns


def phase_estimate(rows: list[dict[str, str]], mode: int) -> float:
    values = [float(row[f"q{mode}"]) for row in rows]
    numerator = sum(values[n] * (values[n + 1] + values[n - 1]) for n in range(1, 63))
    denominator = 2 * sum(values[n] ** 2 for n in range(1, 63))
    return math.acos(max(-1.0, min(1.0, numerator / denominator)))


with (RESULTS / "ftd_0629_connected_block_linear_modes_v1.json").open(encoding="utf-8") as handle:
    summary = json.load(handle)
modes_record = read_csv("ftd_0629_connected_block_linear_modes_modes_v1.csv")
arms = read_csv("ftd_0629_connected_block_linear_modes_arms_v1.csv")
ticks = read_csv("ftd_0629_connected_block_linear_modes_ticks_v1.csv")
arms_by_label = {row["label"]: row for row in arms}
ticks_by_label: dict[str, list[dict[str, str]]] = {}
for row in ticks:
    ticks_by_label.setdefault(row["label"], []).append(row)

check("record id", summary["ftd_id"] == "FTD-0629")
check("protocol", summary["protocol_sha256"] == PROTOCOL)
check("parent", summary["parent_result_sha256"] == PARENT)
check("production unchanged", summary["production_changed"] is False)
for key in ("coverage_pass", "eigensystem_pass", "execution_pass", "bounded_pass",
            "frequency_pass", "purity_pass", "amplitude_pass", "sign_pass",
            "covariance_pass"):
    check(key, summary[key] == 1)
check("verdict", summary["verdict"] == "CONNECTED_BLOCK_ADIABATIC_LINEAR_MODES_CONSTRUCTIVE")
check("common action", float(summary["worst_common_residual"]) <= 1e-10)
check("energy", float(summary["worst_energy_drift"]) <= 1e-12)
check("recovery", float(summary["worst_recovery"]) <= 1e-10)

mass_normalized = [[HESSIAN[i][j] / math.sqrt(MASSES[i] * MASSES[j]) for j in range(4)] for i in range(4)]
eigenvalues, vectors = diagonalize(mass_normalized)
check("four modes", len(modes_record) == 4)
orthogonality = 0.0
for m in range(4):
    record = modes_record[m]
    recorded_vector = [float(record[f"v{i}"]) for i in range(4)]
    omega = math.sqrt(eigenvalues[m])
    phase = 2 * math.atan(omega / 2)
    check(f"mode {m} eigenvalue", math.isclose(float(record["lambda"]), eigenvalues[m], rel_tol=2e-13, abs_tol=2e-13))
    check(f"mode {m} omega", math.isclose(float(record["omega"]), omega, rel_tol=2e-13, abs_tol=2e-13))
    check(f"mode {m} phase", math.isclose(float(record["phase"]), phase, rel_tol=2e-13, abs_tol=2e-13))
    check(f"mode {m} vector", max(abs(a - b) for a, b in zip(recorded_vector, vectors[m])) <= 2e-13)
    check(f"mode {m} positive", eigenvalues[m] > 0)
    for n in range(4):
        inner = sum(vectors[m][i] * MASSES[i] * vectors[n][i] for i in range(4))
        orthogonality = max(orthogonality, abs(inner - (1.0 if m == n else 0.0)))
check("M orthogonality", orthogonality <= 1e-12 and float(summary["orthogonality_residual"]) <= 1e-12)

check("sixteen arms", len(arms) == 16 and len(ticks_by_label) == 16)
for arm in arms:
    label, mode = arm["label"], int(arm["mode"])
    rows = ticks_by_label[label]
    check(f"{label} flags", all(arm[key] == "1" for key in ("init", "forward", "reverse", "bounded")))
    check(f"{label} ticks", len(rows) == 64 and [int(row["tick"]) for row in rows] == list(range(1, 65)))
    projection_residual = 0.0
    for row in rows:
        theta = [float(row[key]) for key in ("a", "b", "t_outer", "t_inner")]
        projected = [sum(vectors[m][i] * MASSES[i] * (theta[i] - THETA0[i]) for i in range(4)) for m in range(4)]
        projection_residual = max(projection_residual, max(abs(projected[m] - float(row[f"q{m}"])) for m in range(4)))
    check(f"{label} projection", projection_residual <= 2e-14)
    phase = phase_estimate(rows, mode)
    check(f"{label} phase recomputation", math.isclose(float(arm["phase"]), phase, rel_tol=2e-13, abs_tol=2e-13))
    predicted = float(modes_record[mode]["phase"])
    check(f"{label} frequency", abs(phase - predicted) / predicted <= 0.02)
    target_rms = math.sqrt(sum(float(row[f"q{mode}"]) ** 2 for row in rows) / 64)
    leakage = max(math.sqrt(sum(float(row[f"q{other}"]) ** 2 for row in rows) / 64) / target_rms for other in range(4) if other != mode)
    check(f"{label} leakage", math.isclose(float(arm["leakage"]), leakage, rel_tol=2e-12, abs_tol=2e-13) and leakage <= 0.10)
    check(f"{label} dynamics gates", max(float(row["center"]) for row in rows) <= 1e-8 and max(float(row["drift"]) for row in rows) <= 1e-12 and max(float(row["common"]) for row in rows) <= 1e-10 and max(int(row["multiplicity"]) for row in rows) <= 2 and min(float(row["separation"]) for row in rows) >= 0.9 and float(arm["recovery"]) <= 1e-10)

amplitude_residual = sign_residual = covariance_residual = 0.0
for mode in range(4):
    p1 = arms_by_label[f"x_m{mode}_p1"]
    p2 = arms_by_label[f"x_m{mode}_p2"]
    n1 = arms_by_label[f"x_m{mode}_n1"]
    y1 = arms_by_label[f"y_m{mode}_p1"]
    phase_p1, phase_p2 = float(p1["phase"]), float(p2["phase"])
    ratio = float(p2["initial_excess"]) / float(p1["initial_excess"])
    amplitude_residual = max(amplitude_residual, abs(phase_p1 - phase_p2) / max(abs(phase_p1), abs(phase_p2)), abs(ratio - 4.0))
    check(f"mode {mode} amplitude", abs(phase_p1 - phase_p2) / max(abs(phase_p1), abs(phase_p2)) <= 0.005 and 3.90 <= ratio <= 4.10)
    plus, minus, cyclic = ticks_by_label[p1["label"]], ticks_by_label[n1["label"]], ticks_by_label[y1["label"]]
    sr = max(abs(float(a[f"q{mode}"]) + float(b[f"q{mode}"])) / 1e-4 for a, b in zip(plus, minus))
    cr = max(abs(float(a[f"q{mode}"]) - float(b[f"q{mode}"])) / 1e-4 for a, b in zip(plus, cyclic))
    sr = max(sr, abs(float(p1["phase"]) - float(n1["phase"])) / max(abs(float(p1["phase"])), abs(float(n1["phase"]))))
    cr = max(cr, abs(float(p1["phase"]) - float(y1["phase"])) / max(abs(float(p1["phase"])), abs(float(y1["phase"]))))
    sign_residual, covariance_residual = max(sign_residual, sr), max(covariance_residual, cr)
    check(f"mode {mode} sign", sr <= 0.05)
    check(f"mode {mode} covariance", cr <= 0.05)

check("amplitude summary", math.isclose(amplitude_residual, float(summary["amplitude_residual"]), rel_tol=2e-12, abs_tol=2e-13))
check("sign summary", math.isclose(sign_residual, float(summary["sign_residual"]), rel_tol=2e-12, abs_tol=2e-13))
check("covariance summary", math.isclose(covariance_residual, float(summary["covariance_residual"]), rel_tol=2e-12, abs_tol=2e-13))

failed = [name for name, passed in checks if not passed]
for name, passed in checks:
    print(f"{'PASS' if passed else 'FAIL'}: {name}")
print(f"\n{sum(passed for _, passed in checks)}/{len(checks)} checks pass")
if failed:
    raise SystemExit("failed checks: " + ", ".join(failed))
