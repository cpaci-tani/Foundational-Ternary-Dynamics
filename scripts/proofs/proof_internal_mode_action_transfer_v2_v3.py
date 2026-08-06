"""Independent FTD-0661/0662 correction-chain certificate."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
P2 = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_INTERNAL_MODE_ACTION_TRANSFER_v2.md"
P3 = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_INTERNAL_MODE_ACTION_TRANSFER_v3.md"
J1 = ROOT / "engine/results/ftd_0660/ftd_0660_internal_mode_action_transfer_v1.json"
J2 = ROOT / "engine/results/ftd_0661/ftd_0661_internal_mode_action_transfer_v2.json"
J3 = ROOT / "engine/results/ftd_0662/ftd_0662_internal_mode_action_transfer_v3.json"
A3 = ROOT / "engine/results/ftd_0662/ftd_0662_internal_mode_action_transfer_arms_v3.csv"
T3 = ROOT / "engine/results/ftd_0662/ftd_0662_internal_mode_action_transfer_ticks_v3.csv"
P2_SHA = "8496808C086B0DA6811A1908EEAE72DBBD9F70BFE84329671E6F75404E4F4814"
P3_SHA = "F517C08CB66B6AE2388CBE3C04E1EE5429C4B596723C41732A443649C542136D"
J1_SHA = "08CA4F43FD8E35C5ED596D7379A8EF0EFA931D32BF7A4A7F2E7FB8F3F4CA2D15"
J2_SHA = "45E0AFAB3E986C72A06252087DDB06F662754964013109E92A205AD37C22C421"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def f(value: str) -> float:
    return float(value)


def truth(value: str) -> bool:
    return value == "1"


assert sha256(P2) == P2_SHA
assert sha256(P3) == P3_SHA
assert sha256(J1) == J1_SHA
assert sha256(J2) == J2_SHA
j2 = json.loads(J2.read_text())
j3 = json.loads(J3.read_text())
assert j2["protocol_sha256"] == P2_SHA and j2["parent_result_sha256"] == J1_SHA
assert j3["protocol_sha256"] == P3_SHA and j3["parent_result_sha256"] == J2_SHA
assert j2["verdict"] == "INTERNAL_MODE_ACTION_TRANSFER_MIXED"
assert j2["zero_control_pass"] == 1 and j2["covariance_pass"] == 0
assert math.isclose(j2["tight_frame_covariance_residual"], 0.083493720519154158)

with A3.open(newline="") as stream:
    arms = list(csv.DictReader(stream))
with T3.open(newline="") as stream:
    ticks = list(csv.DictReader(stream))
by_label: dict[str, dict[int, dict[str, str]]] = defaultdict(dict)
for row in ticks:
    by_label[row["label"]][int(row["tick"])] = row

assert len(arms) == 34 and len(by_label) == 34
assert all(len(rows) == 129 for rows in by_label.values())
assert all(truth(row["initialization"]) and truth(row["forward"])
           and truth(row["reverse"]) and truth(row["bounded"])
           and truth(row["redress"]) for row in arms)
nonzero = [row for row in arms if not truth(row["zero"])]
zeros = [row for row in arms if truth(row["zero"])]
assert len(nonzero) == 32 and len(zeros) == 2

minimum_doublet = math.inf
minimum_dynamic = math.inf
minimum_far = math.inf
maximum_far = 0.0
zero_residual = 0.0
for arm in arms:
    rows = by_label[arm["label"]]
    for tick in rows.values():
        kinetic = f(tick["kinetic_excitation"])
        binding = f(tick["binding_excitation"])
        field = f(tick["field_excitation"])
        dressing = f(tick["dressing_excitation"])
        residual = f(tick["residual_field_energy"])
        interference = f(tick["field_interference"])
        total = f(tick["total_excitation"])
        assert abs(kinetic + binding + field - total) <= 5e-15
        assert abs(dressing + residual + interference - field) <= 1e-12
        if truth(arm["zero"]):
            zero_residual = max(zero_residual, abs(kinetic), abs(binding),
                                abs(field), abs(dressing), abs(residual),
                                abs(interference), f(tick["residual_norm"]))
    if not truth(arm["zero"]):
        minimum_doublet = min(minimum_doublet, f(arm["min_doublet_ratio"]))
        minimum_dynamic = min(minimum_dynamic, f(arm["max_dynamic_ratio"]))
        minimum_far = min(minimum_far, f(arm["max_far_fraction"]))
        maximum_far = max(maximum_far, f(arm["max_far_fraction"]))
        assert f(arm["min_doublet_ratio"]) <= 0.60
        assert f(arm["max_dynamic_ratio"]) >= 0.05
        assert f(arm["max_far_fraction"]) >= 0.10
        assert int(arm["near_onset"]) <= int(arm["middle_onset"]) <= int(arm["far_onset"])

assert zero_residual <= 1e-14
assert math.isclose(zero_residual, j3["zero_residual"], rel_tol=1e-12)
assert math.isclose(minimum_doublet, j3["minimum_doublet_ratio"], rel_tol=1e-12)
assert math.isclose(minimum_dynamic, j3["minimum_dynamic_ratio"], rel_tol=1e-12)
assert math.isclose(minimum_far, j3["minimum_far_fraction"], rel_tol=1e-12)
assert math.isclose(maximum_far, j3["maximum_far_fraction"], rel_tol=1e-12)

lookup = {(int(row["orientation"]), int(row["polarization"]),
           int(row["amplitude"]), int(row["quadrature"])): row
          for row in nonzero}
metrics = (
    "doublet_energy", "kinetic_excitation", "binding_excitation",
    "field_excitation", "dressing_excitation", "residual_field_energy",
    "field_interference", "residual_norm", "near_norm", "middle_norm",
    "far_norm",
)


def normalized_tight_residual(amplitude: int, quadrature: int) -> float:
    difference = norm_x = norm_y = 0.0
    for tick in range(129):
        sums: list[dict[str, float]] = [{m: 0.0 for m in metrics} for _ in range(2)]
        for orientation in (0, 1):
            for polarization in range(4):
                arm = lookup[(orientation, polarization, amplitude, quadrature)]
                rows = by_label[arm["label"]]
                scale = f(rows[0]["doublet_energy"])
                for metric in metrics:
                    sums[orientation][metric] += f(rows[tick][metric]) / scale
        for metric in metrics:
            x, y = sums[0][metric], sums[1][metric]
            difference += (x-y)**2
            norm_x += x*x
            norm_y += y*y
    return math.sqrt(difference) / max(1e-300, math.sqrt(norm_x), math.sqrt(norm_y))


covariance = max(normalized_tight_residual(a, q) for a in (1, 2) for q in (1, 3))
assert math.isclose(covariance, j3["normalized_tight_frame_covariance_residual"], rel_tol=2e-12)
assert covariance <= 0.05

assert all(j3[key] == 1 for key in (
    "coverage_pass", "execution_pass", "bounded_pass", "transfer_pass",
    "amplitude_pass", "sign_pass", "covariance_pass", "zero_control_pass",
    "dynamic_morphology_pass",
))
assert j3["local_morphology_pass"] == 0
assert j3["verdict"] == "INTERNAL_MODE_DYNAMIC_FIELD_TRANSFER_CONSTRUCTIVE"
assert j3["production_changed"] is False

print("FTD-0661/0662 action-transfer correction certificate: PASS (v2 MIXED; v3 CONSTRUCTIVE)")
