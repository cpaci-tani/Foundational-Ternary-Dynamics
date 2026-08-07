"""Independent FTD-0659 certificate from locked protocol and raw records."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_NATIVE_EXCITED_MATTER_CLOCK_v1.md"
PARENT = ROOT / "engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_v1.json"
MODES = ROOT / "engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_modes_v1.csv"
RESULT = ROOT / "engine/results/ftd_0659/ftd_0659_native_excited_matter_clock_v1.json"
ARMS = ROOT / "engine/results/ftd_0659/ftd_0659_native_excited_matter_clock_arms_v1.csv"
TICKS = ROOT / "engine/results/ftd_0659/ftd_0659_native_excited_matter_clock_ticks_v1.csv"

PROTOCOL_SHA = "FF9566F6D6B7BCAEB7970359043C62F643A6A8315AF43C01EE0C5CFD21ECC342"
PARENT_SHA = "AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A"
TARGETS = (2e-6, 4e-6, 8e-6)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def truth(value: str) -> bool:
    return value == "1"


def f(value: str) -> float:
    return float(value)


def relative(a: float, b: float) -> float:
    return abs(a - b) / max(1e-300, abs(a), abs(b))


assert sha256(PROTOCOL) == PROTOCOL_SHA
assert sha256(PARENT) == PARENT_SHA

summary = json.loads(RESULT.read_text())
parent = json.loads(PARENT.read_text())
assert parent["verdict"] == "CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_CONSTRUCTIVE"
assert summary["protocol_sha256"] == PROTOCOL_SHA
assert summary["parent_result_sha256"] == PARENT_SHA
assert summary["production_changed"] is False

with MODES.open(newline="") as stream:
    mode_rows = list(csv.DictReader(stream))
mode = {(int(row["orientation"]), int(row["mode"])): row for row in mode_rows}
for orientation in (0, 1):
    assert int(mode[(orientation, 6)]["group"]) == int(mode[(orientation, 7)]["group"])
    assert int(mode[(orientation, 5)]["group"]) != int(mode[(orientation, 6)]["group"])
    eigen6 = f(mode[(orientation, 6)]["hessian_eigen"])
    eigen7 = f(mode[(orientation, 7)]["hessian_eigen"])
    assert relative(eigen6, eigen7) <= 1e-9
assert f(mode[(0, 6)]["hessian_eigen"]) / f(mode[(0, 5)]["hessian_eigen"]) > 100

with ARMS.open(newline="") as stream:
    arms = list(csv.DictReader(stream))
with TICKS.open(newline="") as stream:
    ticks = list(csv.DictReader(stream))

assert len(arms) == 74
by_label: dict[str, list[dict[str, str]]] = defaultdict(list)
for row in ticks:
    by_label[row["label"]].append(row)
assert set(by_label) == {row["label"] for row in arms}
assert all(len(rows) == 257 for rows in by_label.values())

nonzero = [row for row in arms if not truth(row["zero"])]
zeros = [row for row in arms if truth(row["zero"])]
assert len(nonzero) == 72 and len(zeros) == 2
assert all(truth(row["initialization"]) and truth(row["forward"])
           and truth(row["reverse"]) and truth(row["bounded"])
           for row in arms)

clock_pass = True
max_action_drift = 0.0
minimum_support = math.inf
worst_phase_error = 0.0
worst_phase_rms = 0.0
for arm in nonzero:
    rows = by_label[arm["label"]]
    orientation = int(arm["orientation"])
    omega = 0.5 * (f(mode[(orientation, 6)]["omega"])
                   + f(mode[(orientation, 7)]["omega"]))
    expected = 2.0 * f(mode[(orientation, 6)]["phase"])
    actions: list[float] = []
    supports: list[float] = []
    steps: list[float] = []
    for row in rows:
        q0, q1, p0, p1 = map(f, (row["q0"], row["q1"], row["p0"], row["p1"]))
        q2, p2, qp = q0*q0 + q1*q1, p0*p0 + p1*p1, q0*p0 + q1*p1
        action = (p2 + omega*omega*q2) / (2.0*omega)
        z_abs = math.hypot(omega*omega*q2 - p2, -2.0*omega*qp)
        support = z_abs / (2.0*omega*action)
        assert math.isclose(action, f(row["action"]), rel_tol=2e-12, abs_tol=1e-30)
        assert math.isclose(z_abs, f(row["z_abs"]), rel_tol=2e-12, abs_tol=1e-30)
        assert math.isclose(support, f(row["support"]), rel_tol=2e-12, abs_tol=1e-12)
        actions.append(action)
        supports.append(support)
        if row["phase_step"].lower() != "nan":
            steps.append(f(row["phase_step"]))
    drift = max(abs(value-actions[0])/actions[0] for value in actions)
    mean = sum(steps)/len(steps)
    rms = math.sqrt(sum((value-mean)**2 for value in steps)/len(steps))
    phase_error = abs(mean-expected)/expected
    assert math.isclose(drift, f(arm["max_action_drift"]), rel_tol=2e-10)
    assert math.isclose(min(supports), f(arm["min_support"]), rel_tol=2e-12)
    assert math.isclose(mean, f(arm["mean_phase_step"]), rel_tol=2e-12)
    assert math.isclose(rms, f(arm["phase_step_rms"]), rel_tol=2e-10)
    assert math.isclose(phase_error, f(arm["phase_error"]), rel_tol=2e-10)
    max_action_drift = max(max_action_drift, drift)
    minimum_support = min(minimum_support, min(supports))
    worst_phase_error = max(worst_phase_error, phase_error)
    worst_phase_rms = max(worst_phase_rms, rms)
    clock_pass &= (drift <= 0.02 and min(supports) >= 0.90
                   and phase_error <= 0.02 and rms <= 0.05
                   and f(arm["leakage"]) <= 0.10)

assert math.isclose(max_action_drift, summary["worst_action_drift"], rel_tol=2e-10)
assert math.isclose(minimum_support, summary["minimum_support"], rel_tol=2e-12)
assert math.isclose(worst_phase_error, summary["worst_phase_error"], rel_tol=2e-10)
assert math.isclose(worst_phase_rms, summary["worst_phase_rms"], rel_tol=2e-10)
assert clock_pass is False

lookup = {(int(row["orientation"]), int(row["polarization"]),
           int(row["amplitude"]), int(row["quadrature"])): row
          for row in nonzero}

amplitude_pass = True
for orientation in (0, 1):
    for polarization in range(3):
        for quadrature in range(4):
            base = lookup[(orientation, polarization, 0, quadrature)]
            for amplitude in (1, 2):
                arm = lookup[(orientation, polarization, amplitude, quadrature)]
                phase_residual = relative(f(base["mean_phase_step"]),
                                          f(arm["mean_phase_step"]))
                target_ratio = (TARGETS[amplitude]/TARGETS[0])**2
                action_residual = abs(f(arm["initial_action"])
                                      / f(base["initial_action"])
                                      / target_ratio - 1.0)
                amplitude_pass &= phase_residual <= 0.005 and action_residual <= 0.02
assert amplitude_pass is True

def history_rms(a: dict[str, str], b: dict[str, str]) -> float:
    aa, bb = by_label[a["label"]], by_label[b["label"]]
    a0, b0 = f(aa[0]["unwrapped_phase"]), f(bb[0]["unwrapped_phase"])
    residuals = [(f(x["unwrapped_phase"])-a0) - (f(y["unwrapped_phase"])-b0)
                 for x, y in zip(aa, bb)]
    return math.sqrt(sum(value*value for value in residuals)/len(residuals))

quadrature_pass = True
for orientation in (0, 1):
    for polarization in range(3):
        for amplitude in range(3):
            base = lookup[(orientation, polarization, amplitude, 0)]
            for quadrature in (1, 2, 3):
                quadrature_pass &= history_rms(
                    base, lookup[(orientation, polarization, amplitude, quadrature)]) <= 0.05
assert quadrature_pass is True

polarization_pass = True
covariance_pass = True
for orientation in (0, 1):
    for amplitude in range(3):
        for quadrature in range(4):
            base = lookup[(orientation, 0, amplitude, quadrature)]
            for polarization in (1, 2):
                polarization_pass &= relative(
                    f(base["mean_phase_step"]),
                    f(lookup[(orientation, polarization, amplitude, quadrature)]["mean_phase_step"])) <= 0.005
for polarization in range(3):
    for amplitude in range(3):
        for quadrature in range(4):
            covariance_pass &= relative(
                f(lookup[(0, polarization, amplitude, quadrature)]["mean_phase_step"]),
                f(lookup[(1, polarization, amplitude, quadrature)]["mean_phase_step"])) <= 0.005
assert polarization_pass and covariance_pass

for arm in zeros:
    assert not truth(arm["phase_defined"])
    for row in by_label[arm["label"]]:
        assert f(row["action"]) <= 1e-20 and f(row["z_abs"]) <= 1e-20
        assert row["raw_phase"].lower() == "nan"

expected_verdict = (
    "NATIVE_EXCITED_MATTER_CLOCK_CONSTRUCTIVE" if clock_pass
    else "NATIVE_EXCITED_MATTER_CLOCK_MIXED"
)
assert expected_verdict == "NATIVE_EXCITED_MATTER_CLOCK_MIXED"
assert summary["verdict"] == expected_verdict
assert summary["clock_pass"] == 0
assert all(summary[key] == 1 for key in (
    "eigenspace_pass", "coverage_pass", "execution_pass", "bounded_pass",
    "amplitude_pass", "quadrature_pass", "polarization_pass",
    "covariance_pass", "zero_control_pass"))

print("FTD-0659 native excited matter clock certificate: PASS (MIXED)")
