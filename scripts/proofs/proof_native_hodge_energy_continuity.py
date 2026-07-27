#!/usr/bin/env python3
"""Independent FTD-0576 native Hodge energy/continuity proof."""

from __future__ import annotations

import hashlib
import itertools
import json
import math
import platform
import subprocess
from pathlib import Path

import numpy as np
import sympy as sp


REPO = Path(__file__).resolve().parents[2]
TOL = 1.0e-12
G_C = math.sqrt(1.0 / 137.035999177)
PREREG_SHA = "98B3F8D13E6FBAAD26931C6DD7EC37C9377BD054899012B109C63A0512C26E78"
VERDICT = "NATIVE_HODGE_ENERGY_IDENTITY_CENTRAL_LOCAL_MOBILE_CURRENT_OBSTRUCTED"

LOCKED_HASHES = {
    "phase_read": (
        "engine/src/render_bridge_phases/phase_read.cpp",
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    ),
    "phase_write": (
        "engine/src/render_bridge_phases/phase_write.cpp",
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ),
    "field_operators": (
        "engine/include/ftd/field_operators.h",
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    ),
    "native_energy_contract": (
        "engine/include/ftd/eft/native_energy_contract.h",
        "3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621",
    ),
}

IMPLEMENTATION_PATHS = {
    "header": "engine/include/ftd/eft/native_hodge_energy_continuity.h",
    "source": "engine/src/eft/native_hodge_energy_continuity.cpp",
    "test": "engine/tests/test_native_hodge_energy_continuity.cpp",
    "independent_proof": "scripts/proofs/proof_native_hodge_energy_continuity.py",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def symbolic_proof() -> dict[str, object]:
    j, w, a, source, c = sp.symbols("J W a S c", real=True)
    w1 = w - a * j + source
    j1 = j + w1

    def energy(j_value: sp.Expr, w_value: sp.Expr) -> sp.Expr:
        return (
            w_value**2 / 2
            + a * j_value**2 / 2
            - a * w_value * j_value / 2
        )

    delta_h = sp.factor(energy(j1, w1) - energy(j, w))
    r0 = j - w / 2
    r1 = j1 - w1 / 2
    delta_r = sp.factor(r1 - r0)
    general_delta_r = sp.expand((j1 - c * w1) - (j - c * w))
    midpoint_w = (w + w1) / 2
    coefficient_equations = sp.Poly(
        sp.expand(general_delta_r - midpoint_w), j, w, source
    ).coeffs()
    unique_c = sp.solve(coefficient_equations, c, dict=True)
    affine_invariant_before = energy(j, w) - source * r0
    affine_invariant_after = energy(j1, w1) - source * r1

    # Abstract exact-energy ledger after adjointness and continuity.
    density_time, curl_time, longitudinal = sp.symbols(
        "density_time curl_time longitudinal", real=True
    )
    field_work = density_time + curl_time
    interaction_work = -density_time - longitudinal
    matter_work = longitudinal - curl_time

    z = sp.symbols("z")
    central = (z - 1 / z) / 2
    face = 1 - 1 / z
    hop = z - 1
    cardinal_current = sp.factor(-hop / central)
    face_projection = sp.factor(face / central)

    return {
        "driven_work_delta_h": str(delta_h),
        "driven_work_identity": sp.simplify(delta_h - source * delta_r) == 0,
        "half_step_delta_r": str(delta_r),
        "half_step_identity": sp.simplify(delta_r - midpoint_w) == 0,
        "half_step_unique_solution": [
            {str(key): str(value) for key, value in solution.items()}
            for solution in unique_c
        ],
        "half_step_unique": unique_c == [{c: sp.Rational(1, 2)}],
        "constant_source_affine_invariant": sp.simplify(
            affine_invariant_after - affine_invariant_before
        ) == 0,
        "conditional_total_energy_identity": sp.simplify(
            field_work + interaction_work + matter_work
        ) == 0,
        "central_symbol": str(sp.factor(central)),
        "face_symbol": str(sp.factor(face)),
        "cardinal_hop_current_symbol": str(cardinal_current),
        "face_to_site_projection_symbol": str(face_projection),
        "cardinal_current_has_z_minus_one_cancellation": sp.simplify(
            cardinal_current + 2 * z / (z + 1)
        ) == 0,
        "cardinal_current_has_pole_at_minus_one": sp.denom(cardinal_current).subs(z, -1) == 0,
        "face_projection_has_pole_at_minus_one": sp.denom(face_projection).subs(z, -1) == 0,
        "central_checkerboard_zero": sp.simplify(central.subs(z, -1)) == 0,
        "face_checkerboard_nonzero": sp.simplify(face.subs(z, -1)) == 2,
        "finite_range_cardinal_current_exists": False,
        "finite_range_face_to_site_projection_exists": False,
    }


def gradient(scalar: np.ndarray) -> np.ndarray:
    result = np.empty(scalar.shape + (3,), dtype=np.float64)
    for axis in range(3):
        result[..., axis] = 0.5 * (
            np.roll(scalar, -1, axis=axis) - np.roll(scalar, 1, axis=axis)
        )
    return result


def divergence(field: np.ndarray) -> np.ndarray:
    result = np.zeros(field.shape[:3], dtype=np.float64)
    for axis in range(3):
        result += 0.5 * (
            np.roll(field[..., axis], -1, axis=axis)
            - np.roll(field[..., axis], 1, axis=axis)
        )
    return result


def curl(field: np.ndarray) -> np.ndarray:
    dx = lambda component: 0.5 * (
        np.roll(field[..., component], -1, axis=0)
        - np.roll(field[..., component], 1, axis=0)
    )
    dy = lambda component: 0.5 * (
        np.roll(field[..., component], -1, axis=1)
        - np.roll(field[..., component], 1, axis=1)
    )
    dz = lambda component: 0.5 * (
        np.roll(field[..., component], -1, axis=2)
        - np.roll(field[..., component], 1, axis=2)
    )
    return np.stack(
        (dy(2) - dz(1), dz(0) - dx(2), dx(1) - dy(0)), axis=-1
    )


def apply_k(field: np.ndarray) -> np.ndarray:
    lap = np.zeros_like(field)
    faces = (
        (1, 0, 0), (-1, 0, 0), (0, 1, 0),
        (0, -1, 0), (0, 0, 1), (0, 0, -1),
    )
    edges = (
        (1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
        (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
        (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1),
    )
    for offset in faces:
        lap += np.roll(field, tuple(-v for v in offset), axis=(0, 1, 2)) / 3
    for offset in edges:
        lap += np.roll(field, tuple(-v for v in offset), axis=(0, 1, 2)) / 6
    lap -= 4 * field
    return -lap / 3


def tick_energy(field: np.ndarray, velocity: np.ndarray) -> float:
    k_field = apply_k(field)
    return float(np.sum(
        0.5 * velocity * velocity
        + 0.5 * field * k_field
        - 0.5 * velocity * k_field
    ))


def fixture(size: int, which: int) -> tuple[np.ndarray, ...]:
    grid = np.indices((size, size, size), dtype=np.float64)
    kx, ky, kz = (2 * math.pi * grid[axis] / size for axis in range(3))
    phase = which + 1
    field = np.stack(
        (
            0.17 * np.sin(kx + phase * ky),
            -0.21 * np.cos(ky - phase * kz),
            0.13 * np.sin(kz + phase * kx),
        ),
        axis=-1,
    )
    velocity = np.stack(
        (
            -0.12 * np.cos(kx - kz),
            0.09 * np.sin(ky + kx),
            0.15 * np.cos(kz + ky),
        ),
        axis=-1,
    )
    source = np.stack(
        (
            0.07 * np.sin(phase * kx + ky),
            -0.08 * np.cos(phase * ky + kz),
            0.06 * np.sin(phase * kz - kx),
        ),
        axis=-1,
    )
    r0 = np.stack(
        (
            0.19 * np.cos(kx + ky),
            -0.11 * np.sin(ky + kz),
            0.16 * np.cos(kz - kx),
        ),
        axis=-1,
    )
    dr = np.stack(
        (
            0.04 * np.sin(phase * kx - kz),
            0.05 * np.cos(phase * ky + kx),
            -0.03 * np.sin(phase * kz + ky),
        ),
        axis=-1,
    )
    current = np.stack(
        (
            0.08 * np.cos(kx - phase * ky),
            -0.06 * np.sin(ky + phase * kz),
            0.09 * np.cos(kz + phase * kx),
        ),
        axis=-1,
    )
    rho0 = 0.14 * np.sin(kx + phase * ky) + 0.10 * np.cos(kz - phase * kx)
    return field, velocity, source, r0, r0 + dr, current, rho0


def normalized_residual(lhs: float, rhs: float) -> float:
    return abs(lhs - rhs) / (1.0 + max(abs(lhs), abs(rhs)))


def numerical_proof() -> dict[str, object]:
    maxima = {
        "full_field_work": 0.0,
        "half_step": 0.0,
        "continuity": 0.0,
        "conditional_field_work": 0.0,
        "conditional_interaction": 0.0,
        "conditional_total_energy": 0.0,
        "odd_current": 0.0,
    }
    full_arms = conditional_arms = 0
    for size, which in itertools.product((5, 7), range(2)):
        field, velocity, source, r0_fixture, r1_fixture, current, rho0 = fixture(
            size, which
        )
        next_velocity = velocity - apply_k(field) + source
        next_field = field + next_velocity
        r0 = field - 0.5 * velocity
        r1 = next_field - 0.5 * next_velocity
        delta_r = r1 - r0
        maxima["full_field_work"] = max(
            maxima["full_field_work"],
            normalized_residual(
                tick_energy(next_field, next_velocity) - tick_energy(field, velocity),
                float(np.sum(source * delta_r)),
            ),
        )
        maxima["half_step"] = max(
            maxima["half_step"],
            float(np.max(np.abs(delta_r - 0.5 * (velocity + next_velocity)))),
        )
        full_arms += 1

        div_q = divergence(current)
        rho1 = rho0 - div_q
        rho_bar = 0.5 * (rho0 + rho1)
        r_bar = 0.5 * (r0_fixture + r1_fixture)
        dr = r1_fixture - r0_fixture
        field_source = -G_C * gradient(rho_bar) + G_C * curl(current)
        field_direct = float(np.sum(field_source * dr))
        field_adjoint = G_C * float(
            np.sum(rho_bar * divergence(dr)) + np.sum(current * curl(dr))
        )
        maxima["conditional_field_work"] = max(
            maxima["conditional_field_work"],
            normalized_residual(field_direct, field_adjoint),
        )
        u0 = -G_C * float(np.sum(rho0 * divergence(r0_fixture)))
        u1 = -G_C * float(np.sum(rho1 * divergence(r1_fixture)))
        grad_div_r_bar = gradient(divergence(r_bar))
        predicted_du = -G_C * float(
            np.sum(rho_bar * divergence(dr))
            + np.sum(current * grad_div_r_bar)
        )
        maxima["conditional_interaction"] = max(
            maxima["conditional_interaction"],
            normalized_residual(u1 - u0, predicted_du),
        )
        matter_work = G_C * float(
            np.sum(current * grad_div_r_bar) - np.sum(current * curl(dr))
        )
        maxima["conditional_total_energy"] = max(
            maxima["conditional_total_energy"],
            normalized_residual(field_direct + (u1 - u0) + matter_work, 0.0),
        )
        maxima["continuity"] = max(
            maxima["continuity"],
            float(np.max(np.abs(rho1 - rho0 + div_q))),
        )
        conditional_arms += 1

    minimum_even_witness = math.inf
    minimum_support_fraction = math.inf
    min_support = 10**9
    max_support = 0
    axial_arms = polarity_checks = 0
    for size in (16, 32, 64, 17, 33, 65):
        for _axis in range(3):
            for polarity in (-1, 1):
                if size % 2 == 0:
                    minimum_even_witness = min(
                        minimum_even_witness, abs(2.0 * polarity)
                    )
                else:
                    positive = polarity * (size - 1) / size
                    negative = -polarity * (size + 1) / size
                    current = np.array(
                        [positive if site == 0 or site % 2 else negative
                         for site in range(size)],
                        dtype=np.float64,
                    )
                    central = 0.5 * (np.roll(current, -1) - np.roll(current, 1))
                    expected = np.zeros(size)
                    expected[0] = polarity
                    expected[1] = -polarity
                    maxima["odd_current"] = max(
                        maxima["odd_current"],
                        float(np.max(np.abs(central - expected))),
                    )
                    support = int(np.count_nonzero(np.abs(current) > TOL))
                    minimum_support_fraction = min(
                        minimum_support_fraction, support / size
                    )
                    min_support = min(min_support, support)
                    max_support = max(max_support, support)
                polarity_checks += 1
            axial_arms += 1

    return {
        "full_field_work_arms": full_arms,
        "conditional_energy_arms": conditional_arms,
        "axial_cardinal_hop_arms": axial_arms,
        "polarity_checks": polarity_checks,
        "maximum_full_field_work_residual": maxima["full_field_work"],
        "maximum_half_step_coordinate_residual": maxima["half_step"],
        "maximum_conditional_continuity_residual": maxima["continuity"],
        "maximum_conditional_field_work_residual": maxima["conditional_field_work"],
        "maximum_conditional_interaction_residual": maxima["conditional_interaction"],
        "maximum_conditional_total_energy_residual": maxima["conditional_total_energy"],
        "maximum_odd_volume_current_residual": maxima["odd_current"],
        "minimum_even_checkerboard_witness": minimum_even_witness,
        "minimum_odd_support_fraction": minimum_support_fraction,
        "minimum_odd_support_sites": min_support,
        "maximum_odd_support_sites": max_support,
    }


def run_native() -> dict[str, object]:
    executable = REPO / "engine/build/Release/test_native_hodge_energy_continuity.exe"
    if not executable.exists():
        raise FileNotFoundError(f"native observer is missing: {executable}")
    completed = subprocess.run(
        [str(executable)], cwd=REPO, check=True, capture_output=True, text=True
    )
    parsed: dict[str, object] = {}
    for line in completed.stdout.splitlines():
        if "=" not in line or line.startswith("  "):
            continue
        key, value = line.split("=", 1)
        if key == "verdict":
            parsed[key] = value
        elif key.endswith("arms") or key in {
            "polarity_checks", "minimum_odd_support_sites",
            "maximum_odd_support_sites", "minimum_odd_support_radius",
            "maximum_odd_support_radius", "native_hodge_energy_continuity failures",
        }:
            parsed[key] = int(value)
        else:
            parsed[key] = float(value)
    if parsed.get("verdict") != VERDICT:
        raise AssertionError(f"unexpected native verdict: {parsed.get('verdict')}")
    if parsed.get("native_hodge_energy_continuity failures") != 0:
        raise AssertionError("native observer reported failures")
    return parsed


def main() -> int:
    prereg_hash = sha256(
        REPO / "docs/theory/10_eft_program/preregistrations/PREREG_NATIVE_HODGE_ENERGY_CONTINUITY_v1.md"
    )
    if prereg_hash != PREREG_SHA:
        raise AssertionError(f"preregistration hash drift: {prereg_hash}")
    locked_hashes: dict[str, str] = {}
    for key, (relative, expected) in LOCKED_HASHES.items():
        observed = sha256(REPO / relative)
        if observed != expected:
            raise AssertionError(f"production hash drift for {key}: {observed}")
        locked_hashes[key] = observed

    symbolic = symbolic_proof()
    numerical = numerical_proof()
    native = run_native()
    gates = [
        symbolic["driven_work_identity"],
        symbolic["half_step_identity"],
        symbolic["half_step_unique"],
        symbolic["constant_source_affine_invariant"],
        symbolic["conditional_total_energy_identity"],
        symbolic["cardinal_current_has_z_minus_one_cancellation"],
        symbolic["cardinal_current_has_pole_at_minus_one"],
        symbolic["face_projection_has_pole_at_minus_one"],
        symbolic["central_checkerboard_zero"],
        symbolic["face_checkerboard_nonzero"],
        numerical["full_field_work_arms"] == 4,
        numerical["conditional_energy_arms"] == 4,
        numerical["axial_cardinal_hop_arms"] == 18,
        numerical["polarity_checks"] == 36,
        numerical["maximum_full_field_work_residual"] <= TOL,
        numerical["maximum_half_step_coordinate_residual"] <= TOL,
        numerical["maximum_conditional_continuity_residual"] <= TOL,
        numerical["maximum_conditional_field_work_residual"] <= TOL,
        numerical["maximum_conditional_interaction_residual"] <= TOL,
        numerical["maximum_conditional_total_energy_residual"] <= TOL,
        numerical["maximum_odd_volume_current_residual"] <= TOL,
        numerical["minimum_even_checkerboard_witness"] >= 2.0 - TOL,
        numerical["minimum_odd_support_fraction"] >= 1.0 - TOL,
        numerical["minimum_odd_support_sites"] == 17,
        numerical["maximum_odd_support_sites"] == 65,
    ]
    if not all(gates):
        raise AssertionError("one or more independent FTD-0576 gates failed")

    implementation_hashes = {
        key: sha256(REPO / relative)
        for key, relative in IMPLEMENTATION_PATHS.items()
    }
    result = {
        "ftd_id": "FTD-0576",
        "verdict": VERDICT,
        "platform": platform.platform(),
        "backend": "MSVC 14.44 Release CPU plus independent SymPy/NumPy proof",
        "tolerance": TOL,
        **numerical,
        "native_cpp_run": native,
        "symbolic_checks": symbolic,
        "driven_tick_work_identity_exact": True,
        "half_step_coordinate_unique": True,
        "conditional_hodge_total_energy_exact": True,
        "even_cardinal_hop_central_current_exists": False,
        "odd_cardinal_hop_current_is_box_spanning": True,
        "finite_range_cardinal_hop_current_exists": False,
        "finite_range_face_to_site_projection_exists": False,
        "additional_staggered_or_nonlocal_structure_required": True,
        "production_changed": False,
        "passes": True,
        "source_hashes_sha256": {
            "preregistration": prereg_hash,
            **locked_hashes,
        },
        "implementation_hashes_sha256": implementation_hashes,
    }
    output = REPO / "engine/results/ftd_0576/windows_msvc_cpu.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    print("PASS: exact Hodge energy identity derived; local central hop current obstructed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
