#!/usr/bin/env python3
"""Independent record certificate for FTD-0611."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_UNIFORM_NEUTRALIZED_SINGLE_CORE_STATIC_v1.md"
RESULT = ROOT / "engine/results/ftd_0611/ftd_0611_uniform_single_core_static_v1.json"
STARTS = ROOT / "engine/results/ftd_0611/ftd_0611_uniform_single_core_starts_v1.csv"
EXPECTED = "45FC3250CE24A236EBC231DAD9AA171CADFD754FA8289601892B73C107279B69"


def protocol_hash() -> str:
    raw = PROTOCOL.read_bytes()
    return hashlib.sha256(raw[: raw.index(b"`protocol_sha256=")]).hexdigest().upper()


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with STARTS.open(newline="", encoding="utf-8") as stream:
        starts = list(csv.DictReader(stream))
    checks: dict[str, bool] = {}
    checks["protocol_hash"] = protocol_hash() == EXPECTED
    checks["record_protocol"] = record["protocol_sha256"] == EXPECTED
    checks["identity"] = record["ftd_id"] == "FTD-0611"
    checks["production_unchanged"] = record["production_changed"] is False
    checks["green"] = record["green_valid"]
    checks["coverage_flags"] = all(record[name] for name in (
        "search_coverage", "differential_coverage", "direct_coverage",
        "covariance_coverage", "transaction_coverage",
    ))
    checks["start_count"] = len(starts) == 16
    checks["start_coverage"] = all(
        row["admissible"] == "1"
        and row["terminated"] == "1"
        and int(row["evaluations"]) <= 2500
        and float(row["diameter"]) <= 1e-7
        and float(row["energy_spread"]) <= 1e-14
        for row in starts
    )
    checks["summary_starts"] = (
        record["admissible_starts"] == 16
        and record["terminated_starts"] == 16
        and record["clustered_starts"] >= 2
    )
    eigenvalues = record["eigenvalues"]
    checks["finite_eigenvalues"] = len(eigenvalues) == 9 and all(
        math.isfinite(value) for value in eigenvalues
    )
    checks["positive_hessian"] = (
        record["positive_modes"] == 9
        and record["minimum_eigenvalue"] > 1e-6
        and min(eigenvalues) == record["minimum_eigenvalue"]
    )
    checks["signed_perturbations"] = record["increasing_perturbations"] == 18
    checks["field"] = (
        abs(record["total_charge"]) <= 1e-11
        and record["field_gate"] <= 1e-11
    )
    checks["covariance"] = (
        record["covariance_energy_residual"] <= 1e-12
        and record["covariance_state_residual"] <= 1e-12
    )
    rest = record["rest"]
    checks["rest_execution"] = (
        rest["execution_complete"]
        and rest["forward_ticks"] == 16
        and rest["reverse_ticks"] == 16
        and rest["worst_common_gate"] <= 1e-12
        and rest["maximum_energy_drift"] <= 1e-10
        and rest["maximum_anchor_multiplicity"] <= 2
        and rest["minimum_pair_distance"] >= 0.5
        and rest["maximum_pair_distance"] <= 2.0
        and rest["reverse_recovery"] <= 1e-9
    )
    checks["gradient_gate_fails"] = record["gradient_inf"] > 1e-8
    checks["rest_drift_gate_fails"] = (
        abs(rest["longitudinal_displacement"]) > 1e-8
        or rest["transverse_drift"] > 1e-8
        or rest["center_momentum_change"] > 1e-8
    )
    checks["closed_verdict"] = (
        record["verdict"]
        == "UNIFORM_NEUTRALIZED_COMPACT_STATIC_CLOSED_NEGATIVE"
    )
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0611 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=UNIFORM_NEUTRALIZED_COMPACT_STATIC_CLOSED_NEGATIVE")
    print("scope=locked precision conjunction; positive nine-mode basin remains")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
