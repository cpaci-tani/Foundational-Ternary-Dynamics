"""Independent FTD-0660 action-transfer ledger certificate."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_INTERNAL_MODE_ACTION_TRANSFER_v1.md"
RESULT = ROOT / "engine/results/ftd_0660/ftd_0660_internal_mode_action_transfer_v1.json"
ARMS = ROOT / "engine/results/ftd_0660/ftd_0660_internal_mode_action_transfer_arms_v1.csv"
TICKS = ROOT / "engine/results/ftd_0660/ftd_0660_internal_mode_action_transfer_ticks_v1.csv"
PROTOCOL_SHA = "7731CFC6D1C4C41FF9BD3118D2B78568E99E1D8D91126C92E744AAB91F2D9C9B"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def f(value: str) -> float:
    return float(value)


def truth(value: str) -> bool:
    return value == "1"


assert sha256(PROTOCOL) == PROTOCOL_SHA
summary = json.loads(RESULT.read_text())
assert summary["protocol_sha256"] == PROTOCOL_SHA
assert summary["production_changed"] is False

with ARMS.open(newline="") as stream:
    arms = list(csv.DictReader(stream))
with TICKS.open(newline="") as stream:
    ticks = list(csv.DictReader(stream))
by_label: dict[str, list[dict[str, str]]] = defaultdict(list)
for row in ticks:
    by_label[row["label"]].append(row)

assert len(arms) == 18
assert len(by_label) == 18
assert all(len(rows) == 129 for rows in by_label.values())
assert all(truth(row["initialization"]) and truth(row["forward"])
           and truth(row["reverse"]) and truth(row["bounded"])
           and truth(row["redress"]) for row in arms)

nonzero = [row for row in arms if not truth(row["zero"])]
zeros = [row for row in arms if truth(row["zero"])]
assert len(nonzero) == 16 and len(zeros) == 2

worst_energy = 0.0
worst_decomposition = 0.0
minimum_doublet = math.inf
minimum_dynamic = math.inf
minimum_far = math.inf
maximum_far = 0.0
for arm in arms:
    rows = by_label[arm["label"]]
    for row in rows:
        kinetic = f(row["kinetic_excitation"])
        binding = f(row["binding_excitation"])
        field = f(row["field_excitation"])
        dressing = f(row["dressing_excitation"])
        residual = f(row["residual_field_energy"])
        interference = f(row["field_interference"])
        total = f(row["total_excitation"])
        assert abs((kinetic + binding + field) - total) <= 5e-15
        assert abs((dressing + residual + interference) - field) <= 1e-12
        worst_energy = max(worst_energy, f(row["total_energy_drift"]))
        worst_decomposition = max(
            worst_decomposition, f(row["field_decomposition_residual"]))
    if not truth(arm["zero"]):
        minimum_doublet = min(minimum_doublet, f(arm["min_doublet_ratio"]))
        minimum_dynamic = min(minimum_dynamic, f(arm["max_dynamic_ratio"]))
        minimum_far = min(minimum_far, f(arm["max_far_fraction"]))
        maximum_far = max(maximum_far, f(arm["max_far_fraction"]))
        assert f(arm["min_doublet_ratio"]) <= 0.60
        assert f(arm["max_dynamic_ratio"]) >= 0.05
        assert f(arm["max_far_fraction"]) >= 0.10
        assert int(arm["near_onset"]) <= int(arm["middle_onset"]) <= int(arm["far_onset"])

assert math.isclose(worst_energy, summary["worst_energy_drift"], rel_tol=1e-12)
assert math.isclose(worst_decomposition, summary["worst_decomposition_residual"], rel_tol=1e-12)
assert math.isclose(minimum_doublet, summary["minimum_doublet_ratio"], rel_tol=1e-12)
assert math.isclose(minimum_dynamic, summary["minimum_dynamic_ratio"], rel_tol=1e-12)
assert math.isclose(minimum_far, summary["minimum_far_fraction"], rel_tol=1e-12)
assert math.isclose(maximum_far, summary["maximum_far_fraction"], rel_tol=1e-12)

zero_residual = 0.0
for arm in zeros:
    for row in by_label[arm["label"]]:
        zero_residual = max(
            zero_residual,
            abs(f(row["kinetic_excitation"])),
            abs(f(row["binding_excitation"])),
            abs(f(row["field_excitation"])),
            abs(f(row["dressing_excitation"])),
            abs(f(row["residual_field_energy"])),
            abs(f(row["field_interference"])),
            f(row["residual_norm"]),
        )
assert math.isclose(zero_residual, summary["zero_residual"], rel_tol=1e-12)
assert zero_residual > 1e-20

assert summary["coverage_pass"] == 1
assert summary["execution_pass"] == 1
assert summary["bounded_pass"] == 1
assert summary["transfer_pass"] == 1
assert summary["amplitude_pass"] == 1
assert summary["sign_pass"] == 1
assert summary["dynamic_morphology_pass"] == 1
assert summary["local_morphology_pass"] == 0
assert summary["covariance_pass"] == 0
assert summary["zero_control_pass"] == 0
assert summary["verdict"] == "INTERNAL_MODE_ACTION_TRANSFER_MIXED"

print("FTD-0660 internal-mode action-transfer certificate: PASS (MIXED)")
