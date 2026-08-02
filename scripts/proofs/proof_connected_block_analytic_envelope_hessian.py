"""Independent FTD-0637 analytic-envelope Hessian certificate."""
from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0637"
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CONNECTED_BLOCK_ANALYTIC_ENVELOPE_HESSIAN_v1.md"
SHA = "1BF4C901DA20669248C2790F49F9EF9D488C8D2C67C6EA6B4E89ADC9493178F1"


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    summary = json.loads((RESULT / "ftd_0637_connected_block_analytic_envelope_hessian_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0637_connected_block_analytic_envelope_hessian_arms_v1.csv").open()))
    gradients = list(csv.DictReader((RESULT / "ftd_0637_connected_block_analytic_envelope_gradient_v1.csv").open()))
    matrix_rows = list(csv.DictReader((RESULT / "ftd_0637_connected_block_analytic_envelope_hessian_v1.csv").open()))
    eigen_rows = list(csv.DictReader((RESULT / "ftd_0637_connected_block_analytic_envelope_eigenvalues_v1.csv").open()))
    assert summary["protocol_sha256"] == SHA
    assert summary["verdict"] == "CONNECTED_BLOCK_ANALYTIC_NONSTATIONARY"
    assert len(arms) == 2 and len(gradients) == 96 and len(matrix_rows) == 2 * 48 * 48 and len(eigen_rows) == 96
    for arm in arms:
        assert arm["valid"] == "1" and arm["stationary"] == "0" and arm["positive"] == "1"
        assert float(arm["gradient_inf"]) > 1e-8
        assert float(arm["gradient_comparison"]) <= 5e-8
        assert float(arm["hessian_comparison"]) <= 5e-4
        assert float(arm["poisson_residual"]) <= 1e-13
        assert float(arm["antisymmetry"]) <= 1e-12
        assert float(arm["translation_identity"]) <= 1e-12
        label = arm["label"]
        matrix = np.zeros((48, 48))
        for row in matrix_rows:
            if row["label"] == label:
                matrix[int(row["row"]), int(row["col"])] = float(row["value"])
        recorded = np.array([float(r["eigenvalue"]) for r in eigen_rows if r["label"] == label])
        recomputed = np.linalg.eigvalsh(matrix)
        assert np.max(np.abs(recorded - recomputed)) <= 1e-9
        assert recomputed[0] > 1e-5
        values = [abs(float(r["value"])) for r in gradients if r["label"] == label]
        assert abs(max(values) - float(arm["gradient_inf"])) <= 1e-15
    assert float(summary["covariance_residual"]) <= 1e-6
    print("FTD-0637 certificate: analytic Hessian positive; locked stationarity gate fails")


if __name__ == "__main__":
    main()
