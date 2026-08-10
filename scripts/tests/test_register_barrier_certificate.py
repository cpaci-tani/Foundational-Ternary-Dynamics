"""Regression gate for the outward-rounded register-barrier certificate."""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path


SCRIPT = (
    Path(__file__).resolve().parents[1]
    / "experiments"
    / "temporal_interior"
    / "derive_register_barrier_lower_bound.py"
)


def load_certificate_module():
    spec = importlib.util.spec_from_file_location(
        "derive_register_barrier_lower_bound", SCRIPT)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_register_barrier_outward_interval_certificate():
    module = load_certificate_module()
    certificate = module.build_certificate(verbose=False)

    assert certificate["status"] == "proved_with_stated_arithmetic_model"
    assert certificate["arithmetic"]["checks_passed"] is True
    assert certificate["coverage"]["total_boxes"] == 11_250_000
    assert certificate["coverage"]["sampled_points_used"] == 0
    assert certificate["analytic_attainment"]["sampled_points_used"] == 0
    assert certificate["result"]["uncertified_boxes"] == 0
    assert certificate["result"]["boxes_by_analytic_N_le_2"] == 11_250_000
    assert certificate["result"]["boxes_by_outward_interval"] == 0
    assert certificate["result"]["worst_outward_energy_lower"] is None
    assert certificate["result"]["margin_above_minus_2_lower"] is None
    assert certificate["analytic_bond_law"]["radial_curvature_at_r_0"] == (
        "d^2 V(r^2)/dr^2 at r=1 is 96 eps")

    # The public certificate must remain machine serializable.
    encoded = json.dumps(certificate, allow_nan=False, sort_keys=True)
    decoded = json.loads(encoded)
    assert decoded["schema"] == "ftd.register_barrier.interval-certificate.v1"
