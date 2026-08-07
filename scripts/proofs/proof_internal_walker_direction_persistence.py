#!/usr/bin/env python3
"""Independent record certificate for FTD-0616."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_INTERNAL_WALKER_DIRECTION_PERSISTENCE_v1.md"
PARENT = ROOT / "engine/results/ftd_0615/ftd_0615_zero_momentum_internal_modes_v1.json"
RESULT = ROOT / "engine/results/ftd_0616/ftd_0616_internal_walker_direction_persistence_v1.json"
ARMS = ROOT / "engine/results/ftd_0616/ftd_0616_internal_walker_arms_v1.csv"
TICKS = ROOT / "engine/results/ftd_0616/ftd_0616_internal_walker_ticks_v1.csv"
EXPECTED_PROTOCOL = "E55D5CFA92EB719569B2A8F6D4F19EDB9C90DE49BA4C2B1721AC06F0B0AA730B"
EXPECTED_PARENT = "8B7DD5809DE70B5EEA3C398C4A58AE2B0F64EFD6FD0BF653FBA4F4F0569ABA2C"


def protocol_hash() -> str:
    raw = PROTOCOL.read_bytes()
    marker_offset = raw.index(b"`protocol_sha256=")
    digest = hashlib.sha256()
    digest.update(raw[0:marker_offset])
    return digest.hexdigest().upper()


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def key(item: dict) -> tuple[int, int, int]:
    return int(item["mode"]), int(item["sign"]), int(item["rotation"])


def vector(row: dict[str, str], names: tuple[str, str, str]) -> tuple[float, float, float]:
    return tuple(float(row[name]) for name in names)  # type: ignore[return-value]


def sub(lhs: tuple[float, ...], rhs: tuple[float, ...]) -> tuple[float, ...]:
    return tuple(a - b for a, b in zip(lhs, rhs))


def add(lhs: tuple[float, ...], rhs: tuple[float, ...]) -> tuple[float, ...]:
    return tuple(a + b for a, b in zip(lhs, rhs))


def scale(value: tuple[float, ...], factor: float) -> tuple[float, ...]:
    return tuple(factor * entry for entry in value)


def norm(value: tuple[float, ...]) -> float:
    return math.sqrt(sum(entry * entry for entry in value))


def cosine(lhs: tuple[float, ...], rhs: tuple[float, ...]) -> float:
    return sum(a * b for a, b in zip(lhs, rhs)) / (norm(lhs) * norm(rhs))


def cycle(value: tuple[float, float, float], turns: int) -> tuple[float, float, float]:
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
    expected = {(mode, sign, rotation) for mode in (0, 1)
                for sign in (-1, 1) for rotation in (0, 1, 2)}
    ticks: dict[tuple[int, int, int], list[dict[str, str]]] = {}
    for row in tick_rows:
        ticks.setdefault(key(row), []).append(row)
    for values in ticks.values():
        values.sort(key=lambda row: int(row["tick"]))

    reconstructed_windows: dict[tuple[int, int, int], list[tuple[float, float, float]]] = {}
    maximum_metric_residual = 0.0
    for arm_key in expected:
        rows = ticks[arm_key]
        centers = [vector(row, ("cx", "cy", "cz")) for row in rows]
        windows = [sub(centers[(j + 1) * 128], centers[j * 128]) for j in range(4)]
        reconstructed_windows[arm_key] = windows  # type: ignore[assignment]
        lengths = [norm(window) for window in windows]
        speeds = [length / 128.0 for length in lengths]
        mean = sum(speeds) / 4.0
        cv = math.sqrt(sum((speed - mean) ** 2 for speed in speeds) / 4.0) / mean
        minimum_cosine = min(cosine(windows[j], windows[j + 1]) for j in range(3))
        arm = json_arms[arm_key]
        final_displacement = vector(rows[-1], ("dx", "dy", "dz"))
        maximum_metric_residual = max(
            maximum_metric_residual,
            norm(sub(final_displacement, tuple(arm["displacement"]))),
            abs(min(lengths) - arm["minimum_window_displacement"]),
            abs(minimum_cosine - arm["minimum_successive_cosine"]),
            abs(cv - arm["window_speed_cv"]),
        )

    maximum_center_covariance = 0.0
    for mode in (0, 1):
        for sign in (-1, 1):
            base = ticks[(mode, sign, 0)]
            for rotation in (1, 2):
                rotated = ticks[(mode, sign, rotation)]
                for lhs, rhs in zip(base, rotated):
                    maximum_center_covariance = max(
                        maximum_center_covariance,
                        norm(sub(cycle(vector(lhs, ("cx", "cy", "cz")), rotation),
                                 vector(rhs, ("cx", "cy", "cz")))),
                    )

    antipodes = []
    mismatches = []
    maximum_sign_even_z = 0.0
    maximum_sign_odd_in_plane = 0.0
    minimum_sign_odd_z = math.inf
    minimum_sign_even_in_plane = math.inf
    maximum_phase_parity_residual = 0.0
    maximum_mode_phase_residual = 0.0
    for mode in (0, 1):
        negative = ticks[(mode, -1, 0)]
        positive = ticks[(mode, 1, 0)]
        dn = vector(negative[-1], ("dx", "dy", "dz"))
        dp = vector(positive[-1], ("dx", "dy", "dz"))
        antipodes.append(cosine(dp, dn))
        mismatches.append(abs(norm(dp) / norm(dn) - 1.0))
        even = scale(add(dp, dn), 0.5)
        odd = scale(sub(dp, dn), 0.5)
        maximum_sign_even_z = max(maximum_sign_even_z, abs(even[2]))
        maximum_sign_odd_in_plane = max(
            maximum_sign_odd_in_plane, math.hypot(odd[0], odd[1]))
        minimum_sign_odd_z = min(minimum_sign_odd_z, abs(odd[2]))
        minimum_sign_even_in_plane = min(
            minimum_sign_even_in_plane, math.hypot(even[0], even[1]))
        for nrow, prow in zip(negative, positive):
            maximum_phase_parity_residual = max(
                maximum_phase_parity_residual,
                abs(float(nrow["internal_q"]) + float(prow["internal_q"])),
                abs(float(nrow["internal_p"]) + float(prow["internal_p"])),
            )
    for sign in (-1, 1):
        mode0 = ticks[(0, sign, 0)]
        mode1 = ticks[(1, sign, 0)]
        for lhs, rhs in zip(mode0, mode1):
            maximum_mode_phase_residual = max(
                maximum_mode_phase_residual,
                abs(float(lhs["internal_q"]) - float(rhs["internal_q"])),
                abs(float(lhs["internal_p"]) - float(rhs["internal_p"])),
            )

    all_window_lengths = [norm(window) for windows in reconstructed_windows.values()
                          for window in windows]
    all_window_cosines = [cosine(windows[j], windows[j + 1])
                          for windows in reconstructed_windows.values() for j in range(3)]
    checks = {
        "protocol_hash": protocol_hash() == EXPECTED_PROTOCOL,
        "record_protocol": record["protocol_sha256"] == EXPECTED_PROTOCOL,
        "parent_cryptographic_hash": file_hash(PARENT) == EXPECTED_PARENT,
        "record_parent_hash": record["parent_result_sha256"] == EXPECTED_PARENT,
        "identity": record["ftd_id"] == "FTD-0616",
        "production_unchanged": record["production_changed"] is False,
        "parent_rest_basis": record["parent_record_fingerprint_pass"]
        and record["rest_fingerprint_pass"] and record["rest_gate_pass"]
        and record["basis_coverage"],
        "arm_cardinality": len(json_arms) == len(csv_arms) == len(ticks) == 12
        and set(json_arms) == set(csv_arms) == set(ticks) == expected,
        "tick_cardinality": len(tick_rows) == 12 * 513
        and all(len(rows) == 513 and int(rows[0]["tick"]) == 0
                and int(rows[-1]["tick"]) == 512 for rows in ticks.values()),
        "all_algebraic_gates": record["arm_coverage"] and all(
            arm["complete"] and arm["base_pass"] and arm["intact"]
            and arm["forward_ticks"] == arm["reverse_ticks"] == 512
            and arm["worst_common_gate"] <= 1e-12
            and arm["maximum_energy_drift"] <= 1e-10
            and arm["reverse_recovery"] <= 1e-8
            and arm["maximum_anchor_multiplicity"] <= 2
            and arm["minimum_pair_distance"] >= 0.5
            and arm["maximum_pair_distance"] <= 2.0
            for arm in json_arms.values()),
        "metric_reconstruction": maximum_metric_residual <= 1e-14,
        "whole_history_covariance": record["covariance_pass"]
        and maximum_center_covariance <= 1e-8
        and record["maximum_covariance_residual"] <= 1e-8
        and all(arm["covariance_pass"] for arm in json_arms.values()),
        "direction_gate_fails": not record["direction_control_pass"]
        and max(antipodes) > -0.99
        and abs(max(antipodes) - record["maximum_sign_antipode_cosine"]) <= 1e-14
        and abs(max(mismatches) - record["maximum_sign_magnitude_mismatch"]) <= 1e-14,
        "sign_parity_decomposition": maximum_sign_even_z <= 2e-9
        and maximum_sign_odd_in_plane <= 1e-8
        and minimum_sign_odd_z > 2.87
        and minimum_sign_even_in_plane > 1.64,
        "internal_phase_sign_parity": maximum_phase_parity_residual <= 1e-8,
        "internal_phase_mode_degeneracy": maximum_mode_phase_residual <= 1e-8,
        "translation_continues_each_window": min(all_window_lengths) > 0.916,
        "straight_persistence_fails": not record["persistence_pass"]
        and all(not arm["persistent"] for arm in json_arms.values())
        and min(all_window_cosines) < 0.37,
        "pseudomomentum_not_closed": max(
            arm["maximum_pseudomomentum_defect"] for arm in json_arms.values()) > 9e-4,
        "locked_verdict": record["verdict"] == "INTERNAL_WALKER_TRANSIENT_OR_UNCONTROLLED",
    }
    failed = [name for name, passed in checks.items() if not passed]
    print(f"FTD-0616 independent certificate: {len(checks)-len(failed)}/{len(checks)} checks pass")
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"maximum_center_covariance={maximum_center_covariance:.17g}")
    print(f"minimum_window_displacement={min(all_window_lengths):.17g}")
    print(f"minimum_successive_cosine={min(all_window_cosines):.17g}")
    print(f"sign_even_in_plane_min={minimum_sign_even_in_plane:.17g}")
    print(f"sign_odd_z_min={minimum_sign_odd_z:.17g}")
    if failed:
        print("failed=" + ",".join(failed))
        return 1
    print("verdict=INTERNAL_WALKER_TRANSIENT_OR_UNCONTROLLED")
    print("measured_pattern=continuous curved transport with sign-even in-plane and sign-odd axial components")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
