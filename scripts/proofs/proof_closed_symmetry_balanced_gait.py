#!/usr/bin/env python3
"""Independent record certificate for FTD-0618."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CLOSED_SYMMETRY_BALANCED_GAIT_v1.md"
PARENT = ROOT / "engine/results/ftd_0617/ftd_0617_internal_gait_angular_response_v1.json"
RESULT = ROOT / "engine/results/ftd_0618/ftd_0618_closed_symmetry_balanced_gait_v1.json"
ARMS = ROOT / "engine/results/ftd_0618/ftd_0618_closed_symmetry_balanced_gait_arms_v1.csv"
TICKS = ROOT / "engine/results/ftd_0618/ftd_0618_closed_symmetry_balanced_gait_ticks_v1.csv"
EXPECTED_PROTOCOL = "C8D6D2550A38BA01FAA52CDDB37A152AA0EB6D258BFBA8C1AA092B1973387A73"
EXPECTED_PARENT = "DABFBE348F9714E8B1F5EAF78D1EB06744A3BAE22D2BA4C9FBB2D2C5099995C0"


def prefix_hash() -> str:
    raw = PROTOCOL.read_bytes()
    marker = bytes([96]) + b"protocol_sha256="
    return hashlib.sha256(raw[:raw.index(marker)]).hexdigest().upper()


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def vec(values) -> tuple[float, float, float]:
    return tuple(float(value) for value in values)  # type: ignore[return-value]


def row_vec(row: dict[str, str], names: tuple[str, str, str]) -> tuple[float, float, float]:
    return tuple(float(row[name]) for name in names)  # type: ignore[return-value]


def norm(value) -> float:
    return math.sqrt(sum(entry * entry for entry in value))


def residual(lhs, rhs) -> float:
    return norm(tuple(a - b for a, b in zip(lhs, rhs)))


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with ARMS.open(newline="", encoding="utf-8") as stream:
        arm_rows = list(csv.DictReader(stream))
    with TICKS.open(newline="", encoding="utf-8") as stream:
        tick_rows = list(csv.DictReader(stream))
    json_arms = {int(arm["sign"]): arm for arm in record["arms"]}
    csv_arms = {int(arm["sign"]): arm for arm in arm_rows}
    ticks: dict[int, list[dict[str, str]]] = {}
    for row in tick_rows:
        ticks.setdefault(int(row["sign"]), []).append(row)
    for rows in ticks.values():
        rows.sort(key=lambda row: int(row["tick"]))

    final_residual = max(residual(
        row_vec(ticks[sign][-1], ("dx", "dy", "dz")),
        vec(json_arms[sign]["displacement"])) for sign in (0, 1, -1))
    maximum_transverse = max(
        math.hypot(float(row["dx"]), float(row["dy"]))
        for sign in (1, -1) for row in ticks[sign])
    maximum_symmetry = max(float(row["half_turn_residual"])
                           for row in tick_rows)
    maximum_cumulative = max(float(row["cumulative_pseudomomentum_drift"])
                             for row in tick_rows)
    plus_z = float(json_arms[1]["displacement"][2])
    minus_z = float(json_arms[-1]["displacement"][2])
    minimum_axial = min(abs(plus_z), abs(minus_z))
    sign_residual = abs(plus_z + minus_z)
    rest_norm = norm(vec(json_arms[0]["displacement"]))

    csv_json_residual = 0.0
    for sign in (0, 1, -1):
        csv_json_residual = max(csv_json_residual, residual(
            row_vec(csv_arms[sign], ("dx", "dy", "dz")),
            vec(json_arms[sign]["displacement"])))

    checks = {
        "protocol_hash": prefix_hash() == EXPECTED_PROTOCOL,
        "record_protocol": record["protocol_sha256"] == EXPECTED_PROTOCOL,
        "parent_cryptographic_hash": file_hash(PARENT) == EXPECTED_PARENT,
        "record_parent_hash": record["parent_result_sha256"] == EXPECTED_PARENT,
        "identity": record["ftd_id"] == "FTD-0618",
        "production_and_environment": record["production_changed"] is False
        and record["stationary_density_present"] is False,
        "exact_net_charge": record["net_charge"] == 0,
        "parent_and_rest": record["parent_fingerprint_pass"]
        and record["rest_fingerprint_pass"],
        "arm_cardinality": set(json_arms) == set(csv_arms) == set(ticks) == {0, 1, -1}
        and len(json_arms) == len(csv_arms) == len(ticks) == 3,
        "tick_cardinality": len(tick_rows) == 3 * 129
        and all(len(rows) == 129 and int(rows[0]["tick"]) == 0
                and int(rows[-1]["tick"]) == 128 for rows in ticks.values()),
        "record_reconstruction": final_residual <= 1e-14
        and csv_json_residual <= 1e-14,
        "all_algebraic_gates": record["arm_coverage"] and all(
            arm["initialized"] and arm["complete"] and arm["algebraic_pass"]
            and arm["forward_ticks"] == arm["reverse_ticks"] == 128
            and arm["worst_common_gate"] <= 1e-12
            and arm["maximum_energy_drift"] <= 1e-10
            and arm["reverse_recovery"] <= 1e-8
            and arm["maximum_anchor_multiplicity"] <= 2
            and arm["minimum_internal_distance"] >= 0.5
            and arm["maximum_internal_distance"] <= 2.0
            for arm in json_arms.values()),
        "zero_initial_momentum": all(
            arm["initial_core_momentum"] <= 1e-12
            and arm["initial_total_momentum"] <= 1e-12
            for arm in json_arms.values()),
        "rest_gate": record["rest_pass"] and rest_norm <= 1e-8,
        "transverse_balance": record["transverse_pass"]
        and maximum_transverse <= 1e-8
        and abs(maximum_transverse - record["maximum_transverse"]) <= 1e-14,
        "axial_transport": record["axial_pass"] and minimum_axial >= 0.5
        and abs(minimum_axial - record["minimum_active_axial"]) <= 1e-14,
        "sign_reversal": record["sign_pass"] and sign_residual <= 1e-8
        and abs(sign_residual - record["sign_axial_residual"]) <= 1e-14,
        "half_turn_symmetry": record["symmetry_pass"]
        and maximum_symmetry <= 1e-8
        and abs(maximum_symmetry - record["maximum_symmetry_residual"]) <= 1e-14,
        "momentum_gate_fails": record["momentum_pass"] is False
        and maximum_cumulative > 0.05
        and abs(maximum_cumulative
                - record["maximum_cumulative_pseudomomentum_drift"]) <= 1e-14,
        "locked_verdict": record["verdict"]
        == "SYMMETRY_BALANCED_GAIT_KINEMATIC_MOMENTUM_OPEN",
    }
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0618 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"rest_displacement={rest_norm:.17g}")
    print(f"maximum_transverse={maximum_transverse:.17g}")
    print(f"active_axial=+{plus_z:.17g},{minus_z:.17g}")
    print(f"sign_axial_residual={sign_residual:.17g}")
    print(f"maximum_half_turn_residual={maximum_symmetry:.17g}")
    print(f"maximum_cumulative_pseudomomentum_drift={maximum_cumulative:.17g}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=SYMMETRY_BALANCED_GAIT_KINEMATIC_MOMENTUM_OPEN")
    print("boundary=symmetry-balanced neutral transport is exact kinematically; declared total pseudomomentum is not conserved")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
