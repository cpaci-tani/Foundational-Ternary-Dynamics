#!/usr/bin/env python3
"""Independent record certificate for FTD-0617."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_INTERNAL_GAIT_ANGULAR_RESPONSE_v1.md"
PARENT = ROOT / "engine/results/ftd_0616/ftd_0616_internal_walker_direction_persistence_v1.json"
RESULT = ROOT / "engine/results/ftd_0617/ftd_0617_internal_gait_angular_response_v1.json"
ARMS = ROOT / "engine/results/ftd_0617/ftd_0617_internal_gait_arms_v1.csv"
TICKS = ROOT / "engine/results/ftd_0617/ftd_0617_internal_gait_ticks_v1.csv"
EXPECTED_PROTOCOL = "3BBD327679EB34D2F4196D897EEEF3040E6A90899C489589612D707B833E1065"
EXPECTED_PARENT = "9EB7E10D912FE290795BB78E150744EC508C360F50E3BC209AF20091156A6B40"


def prefix_hash() -> str:
    raw = PROTOCOL.read_bytes()
    marker = bytes([96]) + b"protocol_sha256="
    return hashlib.sha256(raw[:raw.index(marker)]).hexdigest().upper()


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def key(item: dict) -> tuple[int, int]:
    return int(item["angle"]), int(item["rotation"])


def vec(values) -> tuple[float, float, float]:
    return tuple(float(value) for value in values)  # type: ignore[return-value]


def row_vec(row: dict[str, str], names: tuple[str, str, str]) -> tuple[float, float, float]:
    return tuple(float(row[name]) for name in names)  # type: ignore[return-value]


def add(lhs, rhs):
    return tuple(a + b for a, b in zip(lhs, rhs))


def sub(lhs, rhs):
    return tuple(a - b for a, b in zip(lhs, rhs))


def scale(value, factor: float):
    return tuple(factor * entry for entry in value)


def norm(value) -> float:
    return math.sqrt(sum(entry * entry for entry in value))


def cycle(value, turns: int):
    result = value
    for _ in range(turns):
        result = (result[1], result[2], result[0])
    return result


def main() -> int:
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with ARMS.open(newline="", encoding="utf-8") as stream:
        arm_rows = list(csv.DictReader(stream))
    with TICKS.open(newline="", encoding="utf-8") as stream:
        tick_rows = list(csv.DictReader(stream))
    json_arms = {key(arm): arm for arm in record["arms"]}
    csv_arms = {key(arm): arm for arm in arm_rows}
    ticks: dict[tuple[int, int], list[dict[str, str]]] = {}
    for row in tick_rows:
        ticks.setdefault(key(row), []).append(row)
    for rows in ticks.values():
        rows.sort(key=lambda row: int(row["tick"]))
    expected = {(angle, 0) for angle in range(8)} | {
        (angle, rotation) for angle in (0, 2) for rotation in (1, 2)
    }

    displacement = [vec(json_arms[(angle, 0)]["displacement"]) for angle in range(8)]
    cosine_coefficients = []
    sine_coefficients = []
    for harmonic in range(5):
        cosine = [0.0, 0.0, 0.0]
        sine = [0.0, 0.0, 0.0]
        for angle, value in enumerate(displacement):
            theta = 2.0 * math.pi * angle / 8.0
            for component in range(3):
                cosine[component] += value[component] * math.cos(harmonic * theta) / 8.0
                sine[component] += value[component] * math.sin(harmonic * theta) / 8.0
        cosine_coefficients.append(tuple(cosine))
        sine_coefficients.append(tuple(sine))

    maximum_dft_residual = 0.0
    for angle, value in enumerate(displacement):
        theta = 2.0 * math.pi * angle / 8.0
        reconstructed = add(cosine_coefficients[0],
                            scale(cosine_coefficients[4], math.cos(4.0 * theta)))
        for harmonic in range(1, 4):
            reconstructed = add(reconstructed, scale(add(
                scale(cosine_coefficients[harmonic], math.cos(harmonic * theta)),
                scale(sine_coefficients[harmonic], math.sin(harmonic * theta))), 2.0))
        maximum_dft_residual = max(maximum_dft_residual, norm(sub(reconstructed, value)))

    coefficient_residual = 0.0
    for harmonic, stored in enumerate(record["fourier"]):
        coefficient_residual = max(
            coefficient_residual,
            norm(sub(cosine_coefficients[harmonic], vec(stored["cosine"]))),
            norm(sub(sine_coefficients[harmonic], vec(stored["sine"]))),
        )

    even = []
    odd = []
    for angle in range(4):
        even.append(scale(add(displacement[angle], displacement[angle + 4]), 0.5))
        odd.append(scale(sub(displacement[angle], displacement[angle + 4]), 0.5))
    even_rms = math.sqrt(sum(norm(value) ** 2 for value in even) / 4.0)
    odd_rms = math.sqrt(sum(norm(value) ** 2 for value in odd) / 4.0)
    maximum_even_axial = max(abs(value[2]) for value in even)
    maximum_odd_planar = max(math.hypot(value[0], value[1]) for value in odd)

    maximum_center_covariance = 0.0
    for angle in (0, 2):
        base = ticks[(angle, 0)]
        for rotation in (1, 2):
            rotated = ticks[(angle, rotation)]
            for lhs, rhs in zip(base, rotated):
                maximum_center_covariance = max(maximum_center_covariance,
                    norm(sub(cycle(row_vec(lhs, ("cx", "cy", "cz")), rotation),
                             row_vec(rhs, ("cx", "cy", "cz")))))

    main_norms = [norm(value) for value in displacement]
    axis_norms = [main_norms[angle] for angle in (0, 2, 4, 6)]
    diagonal_norms = [main_norms[angle] for angle in (1, 3, 5, 7)]
    harmonic_norms = [math.sqrt(norm(cosine_coefficients[h]) ** 2
                                + norm(sine_coefficients[h]) ** 2)
                      for h in range(5)]
    checks = {
        "protocol_hash": prefix_hash() == EXPECTED_PROTOCOL,
        "record_protocol": record["protocol_sha256"] == EXPECTED_PROTOCOL,
        "parent_cryptographic_hash": file_hash(PARENT) == EXPECTED_PARENT,
        "record_parent_hash": record["parent_result_sha256"] == EXPECTED_PARENT,
        "identity": record["ftd_id"] == "FTD-0617",
        "production_unchanged": record["production_changed"] is False,
        "parent_rest_basis": record["parent_record_fingerprint_pass"]
        and record["rest_fingerprint_pass"] and record["rest_gate_pass"]
        and record["basis_coverage"],
        "arm_cardinality": len(json_arms) == len(csv_arms) == len(ticks) == 12
        and set(json_arms) == set(csv_arms) == set(ticks) == expected,
        "tick_cardinality": len(tick_rows) == 12 * 257
        and all(len(rows) == 257 and int(rows[0]["tick"]) == 0
                and int(rows[-1]["tick"]) == 256 for rows in ticks.values()),
        "all_algebraic_gates": record["arm_coverage"] and all(
            arm["complete"] and arm["base_pass"] and arm["intact"]
            and arm["forward_ticks"] == arm["reverse_ticks"] == 256
            and arm["worst_common_gate"] <= 1e-12
            and arm["maximum_energy_drift"] <= 1e-10
            and arm["reverse_recovery"] <= 1e-8
            and arm["maximum_anchor_multiplicity"] <= 2
            for arm in json_arms.values()),
        "dft_reconstruction": record["dft_pass"]
        and maximum_dft_residual <= 1e-12
        and abs(maximum_dft_residual - record["maximum_dft_residual"]) <= 1e-14,
        "fourier_coefficients": coefficient_residual <= 1e-14,
        "mixed_parity_rms": even_rms > 0.25 and odd_rms > 0.25
        and abs(even_rms - record["even_rms"]) <= 1e-14
        and abs(odd_rms - record["odd_rms"]) <= 1e-14,
        "parity_geometry": maximum_even_axial <= 2e-9
        and maximum_odd_planar <= 1e-8,
        "whole_history_covariance": record["covariance_pass"]
        and maximum_center_covariance <= 1e-8
        and record["maximum_covariance_residual"] <= 1e-8
        and all(arm["covariance_pass"] for arm in json_arms.values()),
        "axis_diagonal_selection": min(axis_norms) > 2.45
        and max(diagonal_norms) < 0.272
        and min(diagonal_norms) < 0.101,
        "nonlinear_odd_harmonic": harmonic_norms[3] > harmonic_norms[1]
        and harmonic_norms[3] > 0.76,
        "pseudomomentum_not_closed": max(
            arm["maximum_pseudomomentum_defect"] for arm in json_arms.values()) > 1.2e-3,
        "locked_verdict": record["verdict"] == "MIXED_PARITY_INTERNAL_GAIT_RESPONSE_RESOLVED",
    }
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0617 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"even_rms={even_rms:.17g} odd_rms={odd_rms:.17g}")
    print(f"axis_norm_range={min(axis_norms):.17g},{max(axis_norms):.17g}")
    print(f"diagonal_norm_range={min(diagonal_norms):.17g},{max(diagonal_norms):.17g}")
    print(f"harmonic_1_norm={harmonic_norms[1]:.17g} harmonic_3_norm={harmonic_norms[3]:.17g}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=MIXED_PARITY_INTERNAL_GAIT_RESPONSE_RESOLVED")
    print("measured_pattern=planar-even/axial-odd response with strong discrete third harmonic and axis/diagonal selection")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
