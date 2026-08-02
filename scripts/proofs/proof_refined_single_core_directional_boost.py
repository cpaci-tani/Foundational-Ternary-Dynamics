#!/usr/bin/env python3
"""Independent record certificate for FTD-0613."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_REFINED_SINGLE_CORE_DIRECTIONAL_BOOST_v1.md"
RESULT = ROOT / "engine/results/ftd_0613/ftd_0613_refined_directional_boost_v1.json"
ARMS = ROOT / "engine/results/ftd_0613/ftd_0613_refined_directional_boost_arms_v1.csv"
EXPECTED = "1A750AA6C557294B6E252A0E77F4B33AD5791A251655EB5787CA946E74A92C35"


def protocol_hash() -> str:
    raw = PROTOCOL.read_bytes()
    return hashlib.sha256(raw[: raw.index(b"`protocol_sha256=")]).hexdigest().upper()


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with ARMS.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    arms = record["arms"]
    checks = {
        "protocol_hash": protocol_hash() == EXPECTED,
        "record_protocol": record["protocol_sha256"] == EXPECTED,
        "identity": record["ftd_id"] == "FTD-0613",
        "production_unchanged": record["production_changed"] is False,
        "rest": record["rest_fingerprint_pass"] and record["rest_gate_pass"]
        and record["rest_recovery"] <= 1e-9,
        "coverage": record["arm_coverage"] and record["symmetry_coverage"],
        "arm_count": len(arms) == len(rows) == 18,
        "complete_base": all(
            arm["execution_complete"] and arm["base_pass"]
            and arm["forward_ticks"] == arm["ticks"]
            and arm["reverse_ticks"] == arm["ticks"]
            and arm["worst_common_gate"] <= 1e-12
            and arm["maximum_energy_drift"] <= 1e-10
            and arm["reverse_recovery"] <= 1e-9
            and arm["maximum_anchor_multiplicity"] <= 2
            and arm["minimum_pair_distance"] >= 0.5
            and arm["maximum_pair_distance"] <= 2.0
            for arm in arms
        ),
        "fast_all_mobile": all(
            arm["mobility_pass"] for arm in arms if arm["speed_index"] == 2
        ),
        "lower_all_pinned": all(
            not arm["mobility_pass"] for arm in arms if arm["speed_index"] < 2
        ),
        "sign_control": record["sign_residual"] <= 0.25,
        "axis_control_fails": (not record["symmetry_pass"])
        and record["axis_residual"] > 0.25,
        "closed_verdict": record["verdict"]
        == "REFINED_COMPACT_CORE_DIRECTIONAL_MOBILITY_CLOSED_NEGATIVE",
    }
    # Recompute the registered symmetry residuals from the arm table.
    sign_residual = 0.0
    axis_residual = 0.0
    for speed in range(3):
        block = [arm for arm in arms if arm["speed_index"] == speed]
        projected_means = []
        transverse_means = []
        for axis in range(3):
            positive, negative = block[2 * axis], block[2 * axis + 1]
            sign_residual = max(
                sign_residual,
                abs(positive["projected_displacement"] - negative["projected_displacement"]),
                abs(positive["transverse_drift"] - negative["transverse_drift"]),
            )
            projected_means.append(
                0.5 * (positive["projected_displacement"] + negative["projected_displacement"])
            )
            transverse_means.append(
                0.5 * (positive["transverse_drift"] + negative["transverse_drift"])
            )
        axis_residual = max(
            axis_residual,
            max(projected_means) - min(projected_means),
            max(transverse_means) - min(transverse_means),
        )
    checks["recomputed_sign"] = abs(sign_residual - record["sign_residual"]) <= 1e-14
    checks["recomputed_axis"] = abs(axis_residual - record["axis_residual"]) <= 1e-14
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0613 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=REFINED_COMPACT_CORE_DIRECTIONAL_MOBILITY_CLOSED_NEGATIVE")
    print("measured_pattern=all lower-speed arms pinned; all 1/32 arms mobile; fixed-body axis gate fails")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
