#!/usr/bin/env python3
"""Independent record certificate for FTD-0610."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_SINGLE_CORE_NEUTRALIZER_CONTROL_v1.md"
RESULT = ROOT / "engine/results/ftd_0610/ftd_0610_single_core_neutralizer_v1.json"
TICKS = ROOT / "engine/results/ftd_0610/ftd_0610_single_core_neutralizer_ticks_v1.csv"
EXPECTED = "DB4363D2A132BB84BFF10218FCE8B4B20BC4C677F6FE813815F368E38A4EED85"


def protocol_hash() -> str:
    raw = PROTOCOL.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def finite(value: object) -> bool:
    return isinstance(value, (int, float)) and math.isfinite(float(value))


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with TICKS.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    checks: dict[str, bool] = {}
    checks["protocol_hash"] = protocol_hash() == EXPECTED
    checks["record_protocol"] = record["protocol_sha256"] == EXPECTED
    checks["identity"] = record["ftd_id"] == "FTD-0610"
    checks["production_unchanged"] = record["production_changed"] is False
    checks["option_default_false"] = (
        record["shared_anchor_option_default"] is False
    )
    checks["static_reproduction"] = (
        record["search_complete"]
        and record["static_seed_pass"]
        and record["admissible_starts"] == 24
        and record["terminated_starts"] >= 18
        and record["clustered_starts"] >= 2
    )
    controls = record["controls"]
    checks["two_controls"] = [item["name"] for item in controls] == [
        "uniform",
        "frozen_partner",
    ]
    checks["fixtures"] = all(
        item["fixture_valid"]
        and abs(item["moving_charge"] - 1.0) <= 1e-12
        and abs(item["stationary_charge"] + 1.0) <= 1e-12
        and abs(item["total_charge"]) <= 1e-12
        and item["poisson_residual"] <= 1e-11
        and item["gauss_residual"] <= 1e-11
        and item["curl_residual"] <= 1e-11
        and item["energy_crosscheck"] <= 1e-11
        for item in controls
    )
    checks["covariance"] = all(
        item["covariance_pass"]
        and item["covariance_state_residual"] <= 1e-12
        and item["covariance_diagnostic_residual"] <= 1e-12
        for item in controls
    )
    expected_ticks = (16 + 128 + 64) * 2 * 2
    checks["tick_count"] = len(rows) == expected_ticks == 832
    checks["all_tick_records_valid"] = all(row["valid"] == "1" for row in rows)
    checks["tick_gates"] = all(
        finite(float(row["common_gate"]))
        and float(row["common_gate"]) <= 1e-12
        for row in rows
    )
    checks["tick_energy"] = all(
        finite(float(row["energy_drift"]))
        and float(row["energy_drift"]) <= 1e-10
        for row in rows
    )
    checks["tick_fibre"] = all(
        int(row["anchor_multiplicity"]) <= 2 for row in rows
    )
    checks["complete_arms"] = all(
        arm["execution_complete"]
        and arm["forward_ticks"] == arm["ticks_requested"]
        and arm["reverse_ticks"] == arm["ticks_requested"]
        and arm["worst_common_gate"] <= 1e-12
        and arm["maximum_energy_drift"] <= 1e-10
        and arm["reverse_recovery"] <= 1e-9
        and arm["minimum_pair_distance"] >= 0.5
        and arm["maximum_pair_distance"] <= 2.0
        for item in controls
        for arm in item["arms"]
    )
    uniform = controls[0]["arms"]
    frozen = controls[1]["arms"]
    checks["uniform_rest_not_static"] = (
        not uniform[0]["physical_pass"]
        and abs(uniform[0]["longitudinal_displacement"]) > 1e-10
        and uniform[0]["center_momentum_change"] > 1e-10
    )
    checks["slow_uniform_fails"] = (
        not uniform[1]["physical_pass"]
        and uniform[1]["longitudinal_displacement"] < 1.5
    )
    checks["fast_uniform_passes"] = uniform[2]["physical_pass"]
    checks["frozen_reverses_slow_motion"] = (
        not frozen[1]["physical_pass"]
        and frozen[1]["longitudinal_displacement"] < 0.0
    )
    checks["fast_frozen_passes"] = frozen[2]["physical_pass"]
    checks["verdict"] = (
        record["verdict"] == "SINGLE_CORE_STATIC_REFERENCE_NOT_ISOLATED"
    )
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0610 independent certificate: {len(checks) - len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=SINGLE_CORE_STATIC_REFERENCE_NOT_ISOLATED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
