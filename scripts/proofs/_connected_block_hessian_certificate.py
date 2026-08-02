"""Shared independent reader for FTD-0634--0636 Hessian records."""
from __future__ import annotations

import csv
import json
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]


def certify(ftd_id: str, protocol: str, stem: str, verdict: str, reason: str) -> None:
    number = ftd_id.split("-")[1]
    result = ROOT / f"engine/results/ftd_{number}"
    summary = json.loads((result / f"{stem}_hessian_v1.json").read_text())
    arms = list(csv.DictReader((result / f"{stem}_hessian_arms_v1.csv").open()))
    matrix_rows = list(csv.DictReader((result / f"{stem}_hessian_v1.csv").open()))
    assert summary["protocol_sha256"] == protocol and summary["verdict"] == verdict
    assert len(arms) == 2 and len(matrix_rows) == 2 * 48 * 48
    matrices: dict[str, np.ndarray] = {}
    for row in matrix_rows:
        matrices.setdefault(row["label"], np.zeros((48, 48)))[int(row["row"]), int(row["col"])] = float(row["value"])
    for row in arms:
        hessian = matrices[row["label"]]
        assert np.max(np.abs(hessian - hessian.T)) <= 1e-12
        eigenvalues = np.linalg.eigvalsh(hessian)
        assert abs(eigenvalues[0] - float(row["min_eigen"])) <= 1e-7
        assert abs(eigenvalues[-1] - float(row["max_eigen"])) <= 1e-7
        assert eigenvalues[0] > 1e-5
        measured = [float(row[f"measured_{axis}"]) for axis in "xyz"]
        rayleigh = [float(row[f"rayleigh_{axis}"]) for axis in "xyz"]
        if reason == "knot_local_gradient":
            assert max(abs(a - b / 16) for a, b in zip(rayleigh, measured)) <= 1e-5
    if reason == "coarse_gradient":
        assert all(float(row["gradient_inf"]) > 1e-8 for row in arms)
    elif reason == "parent_translation":
        assert all(float(row["gradient_inf"]) <= 1e-8 and row["valid"] == "0" for row in arms)
        assert any(abs(float(row["measured_x"]) - float(row["target_x"])) > 1e-5 for row in arms)
    elif reason == "knot_local_gradient":
        assert all(float(row["gradient_inf"]) > 1e-8 and row["valid"] == "0" for row in arms)
    print(f"{ftd_id} certificate: both 48x48 spectra recomputed; invalid gate reproduced ({reason})")
