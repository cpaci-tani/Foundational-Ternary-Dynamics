"""Independent certificate for FTD-0706 complete relative-orbit test."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0706"
SUMMARY = RESULT / "ftd_0706_complete_moving_dressing_relative_orbit_v1.json"
METRICS = RESULT / "ftd_0706_complete_moving_dressing_relative_orbit_metrics_v1.csv"
RUNNER = ROOT / "engine/tests/test_complete_moving_dressing_relative_orbit.cpp"
PREREG = (ROOT / "docs/theory/10_eft_program/preregistrations/"
          "PREREG_COMPLETE_MOVING_DRESSING_RELATIVE_ORBIT_v1.md")

PROTOCOL_SHA = "D07F8CE10B43D209A3C2EAA6AA9A316B12192CE2CF072612935E0F8451FE8FA7"
HASHES = {
    SUMMARY: "3F7E0A2A7F551EEF5C1391CAB09800698ECD4582EA7A401865AC7362FAE094F6",
    METRICS: "EB29D5465C7165B8349868C1783272305C6CB54529C56008E05C93BE24DD5039",
    RUNNER: "E719B13CF1D5851156BF6E8D2056BC9FD737116175E3C43F1B2295E3D624F709",
    PREREG: PROTOCOL_SHA,
}


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for path, expected in HASHES.items():
    assert sha(path) == expected, path

summary = json.loads(SUMMARY.read_text())
assert summary["protocol_sha256"] == PROTOCOL_SHA
assert summary["verdict"] == "MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID"
assert summary["production_changed"] is False
assert summary["volume"] == 33 and summary["ticks"] == 2
assert summary["target_translation_x"] == 1
assert summary["parent_pass"] == 1
assert summary["normalization_pass"] == 1
assert summary["execution_pass"] == 0
assert summary["inverse_pass"] == 1
assert summary["rest_pass"] == 0
assert summary["covariance_pass"] == 1

with METRICS.open(newline="") as stream:
    rows = list(csv.DictReader(stream))
assert len(rows) == 1
row = rows[0]
assert row["ftd_id"] == "FTD-0706"
assert row["verdict"] == summary["verdict"]
assert int(row["total_hops"]) == summary["total_hops"] == 12
for key in ("position_residual", "momentum_residual", "electric_residual",
            "magnetic_residual", "complete_residual", "inverse_residual",
            "rest_residual", "covariance_residual"):
    assert math.isclose(float(row[key]), float(summary[key]),
                        rel_tol=0.0, abs_tol=0.0)
assert math.isclose(float(row["max_energy_drift"]),
                    float(summary["maximum_energy_drift"]),
                    rel_tol=0.0, abs_tol=0.0)
assert math.isclose(float(row["max_common_residual"]),
                    float(summary["maximum_common_residual"]),
                    rel_tol=0.0, abs_tol=0.0)

# The common action, inversion, and translation-covariance controls pass.
assert summary["maximum_energy_drift"] <= 1e-10
assert summary["maximum_common_residual"] <= 1e-10
assert summary["inverse_residual"] <= 1e-9
assert summary["covariance_residual"] <= 1e-9

# The locked rest fixed-point control fails, forcing the invalid verdict.
assert summary["rest_residual"] > 1e-9

# Diagnostic only: the constituent core is close to T_1 while the complete
# face/edge dressing is not.  These inequalities do not override the invalid
# verdict because the rest preparation failed its preregistered control.
assert summary["position_residual"] <= 0.05
assert summary["momentum_residual"] <= 0.05
assert summary["electric_residual"] > 1e-6
assert summary["magnetic_residual"] > 1e-6
assert math.isclose(summary["complete_residual"],
                    summary["electric_residual"],
                    rel_tol=0.0, abs_tol=0.0)

print("FTD-0706 complete moving-dressing relative-orbit certificate: PASS")
print(f"verdict={summary['verdict']}")
print(f"rest={summary['rest_residual']:.6e} "
      f"position={summary['position_residual']:.6e} "
      f"momentum={summary['momentum_residual']:.6e}")
print(f"electric={summary['electric_residual']:.6e} "
      f"magnetic={summary['magnetic_residual']:.6e}")
