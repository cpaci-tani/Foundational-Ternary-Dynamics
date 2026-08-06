"""Independent FTD-0638 analytic static-refinement certificate."""
from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0638"
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CONNECTED_BLOCK_ANALYTIC_STATIC_REFINEMENT_v1.md"
SHA = "2E74799DB0372137071B5CF840D5C330AF4A3FEDE14EE9C4972B2C1796D056BA"


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    summary = json.loads((RESULT / "ftd_0638_connected_block_analytic_static_refinement_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0638_connected_block_analytic_static_refinement_arms_v1.csv").open()))
    states = list(csv.DictReader((RESULT / "ftd_0638_connected_block_analytic_static_refinement_states_v1.csv").open()))
    eigens = list(csv.DictReader((RESULT / "ftd_0638_connected_block_analytic_static_refinement_eigenvalues_v1.csv").open()))
    assert summary["protocol_sha256"] == SHA
    assert summary["verdict"] == "CONNECTED_BLOCK_ANALYTIC_STATIC_BASIN_CONSTRUCTIVE"
    assert len(arms) == 2 and len(states) == 32 and len(eigens) == 96
    for arm in arms:
        assert all(arm[key] == "1" for key in ("valid", "stationary", "positive", "sector_preserved", "steps"))
        assert float(arm["energy_change"]) < 0
        assert float(arm["final_gradient"]) <= 1e-12
        assert float(arm["gradient_comparison"]) <= 5e-8
        assert float(arm["min_eigen"]) > 1e-5
        assert float(arm["max_displacement"]) < 1e-8
        spectrum = np.array([float(row["eigenvalue"]) for row in eigens if row["orientation"] == arm["orientation"]])
        assert len(spectrum) == 48 and np.all(np.diff(spectrum) >= -1e-12)
        assert abs(spectrum[0] - float(arm["min_eigen"])) <= 1e-12
    assert float(summary["energy_covariance"]) <= 1e-9
    assert float(summary["spectrum_covariance"]) <= 1e-9
    assert float(summary["displacement_covariance"]) <= 1e-9
    for row in states:
        displacement = np.array([float(row[f"{axis}1"]) - float(row[f"{axis}0"]) for axis in "xyz"])
        arm = next(a for a in arms if a["orientation"] == row["orientation"])
        assert np.linalg.norm(displacement) <= float(arm["max_displacement"]) + 1e-15
    print("FTD-0638 certificate: one analytic Newton step closes the positive static basin")


if __name__ == "__main__":
    main()
