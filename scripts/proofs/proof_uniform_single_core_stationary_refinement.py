#!/usr/bin/env python3
"""Independent record certificate for FTD-0612."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_UNIFORM_SINGLE_CORE_STATIONARY_REFINEMENT_v1.md"
RESULT = ROOT / "engine/results/ftd_0612/ftd_0612_uniform_single_core_refinement_v1.json"
ITERATIONS = ROOT / "engine/results/ftd_0612/ftd_0612_uniform_single_core_iterations_v1.csv"
EXPECTED = "B0C93907D5EEB6BE96ED9BA485E2BC452E6180FE619533052A2D870C73B52002"
LOCKED_ENERGY = 0.0015517955076684736


def protocol_hash() -> str:
    raw = PROTOCOL.read_bytes()
    return hashlib.sha256(raw[: raw.index(b"`protocol_sha256=")]).hexdigest().upper()


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with ITERATIONS.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    rest = record["rest"]
    checks = {
        "protocol_hash": protocol_hash() == EXPECTED,
        "record_protocol": record["protocol_sha256"] == EXPECTED,
        "identity": record["ftd_id"] == "FTD-0612",
        "production_unchanged": record["production_changed"] is False,
        "fingerprint": record["fingerprint_pass"]
        and abs(record["initial_energy"] - LOCKED_ENERGY) <= 1e-15,
        "coverage": all(record[name] for name in (
            "search_coverage", "refinement_coverage", "direct_coverage",
            "covariance_coverage", "transaction_coverage",
        )),
        "one_iteration": record["iterations"] == len(rows) == 1,
        "accepted_newton": float(rows[0]["damping"]) == 1.0
        and float(rows[0]["minimum_pivot"]) > 1e-8
        and float(rows[0]["energy_after"]) < float(rows[0]["energy_before"]),
        "gradient": record["gradient_inf"] <= 1e-10,
        "stability": record["positive_modes"] == 9
        and record["minimum_eigenvalue"] > 1e-6
        and record["increasing_perturbations"] == 18,
        "field": abs(record["total_charge"]) <= 1e-11
        and record["field_gate"] <= 1e-11,
        "covariance": record["covariance_energy_residual"] <= 1e-12
        and record["covariance_state_residual"] <= 1e-12,
        "rest_coverage": rest["execution_complete"]
        and rest["forward_ticks"] == 64 and rest["reverse_ticks"] == 64,
        "rest_static": abs(rest["longitudinal_displacement"]) <= 1e-9
        and rest["transverse_drift"] <= 1e-9
        and rest["center_momentum_change"] <= 1e-9,
        "rest_common_action": rest["worst_common_gate"] <= 1e-12
        and rest["maximum_energy_drift"] <= 1e-10
        and rest["maximum_anchor_multiplicity"] <= 2
        and rest["minimum_pair_distance"] >= 0.5
        and rest["maximum_pair_distance"] <= 2.0
        and rest["reverse_recovery"] <= 1e-9,
        "verdict": record["verdict"]
        == "REFINED_UNIFORM_SINGLE_CORE_STATIC_CONSTRUCTIVE",
    }
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0612 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=REFINED_UNIFORM_SINGLE_CORE_STATIC_CONSTRUCTIVE")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
