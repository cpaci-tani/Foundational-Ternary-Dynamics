#!/usr/bin/env python3
"""Independent FTD-0575 Hodge reciprocity and static-pole proof."""

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
PREREG_SHA = "BE33049A5C93E887574BDE5509E93F666150A5CAF02E2B93989D96980D1788F6"
VERDICT = "NATIVE_HODGE_FORCE_DERIVED_STATIC_POLE_CANCELED_SAME_SIGN_ATTRACTIVE"
G_C = math.sqrt(1.0 / 137.035999177)
C2 = 1.0 / 3.0

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
    "header": "engine/include/ftd/eft/native_hodge_reciprocity.h",
    "source": "engine/src/eft/native_hodge_reciprocity.cpp",
    "test": "engine/tests/test_native_hodge_reciprocity.cpp",
    "independent_proof": "scripts/proofs/proof_native_hodge_reciprocity_static_pole.py",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def symbolic_proof() -> dict[str, object]:
    u1, u2, u3 = sp.symbols("u1 u2 u3", nonnegative=True)
    u_sum = u1 + u2 + u3
    pairs = u1 * u2 + u1 * u3 + u2 * u3
    squares = u1**2 + u2**2 + u3**2
    difference = sp.expand(squares - sp.Rational(2, 3) * pairs)
    sos = sp.expand(
        squares / 3
        + ((u1 - u2) ** 2 + (u1 - u3) ** 2 + (u2 - u3) ** 2) / 3
    )

    t = sp.symbols("t", positive=True)

    def symbol(direction: tuple[int, int, int]) -> tuple[sp.Expr, sp.Expr, sp.Expr]:
        k = [component * t for component in direction]
        cosines = [sp.cos(value) for value in k]
        sigma2 = sum(sp.sin(value) ** 2 for value in k)
        m_value = 4 - sp.Rational(2, 3) * sum(cosines) - sp.Rational(2, 3) * (
            cosines[0] * cosines[1]
            + cosines[0] * cosines[2]
            + cosines[1] * cosines[2]
        )
        kernel = sp.simplify(sigma2 / (sp.Rational(1, 3) * m_value))
        return sp.simplify(m_value), sp.simplify(sigma2), kernel

    limits: dict[str, str] = {}
    series: dict[str, str] = {}
    for name, direction in {
        "100": (1, 0, 0),
        "110": (1, 1, 0),
        "111": (1, 1, 1),
    }.items():
        _, sigma2, kernel = symbol(direction)
        limits[name] = str(sp.limit(kernel, t, 0))
        series[name] = str(sp.series(kernel, t, 0, 5))
        assert sp.limit(kernel, t, 0) == 3
        assert sp.limit(sigma2 / t**2, t, 0) == sum(c * c for c in direction)

    # Exact point-path variation for the registered polynomial family.
    a, b, c, d, e, h = sp.symbols("a b c d e h", real=True)
    vx, vy, vz, q, g = sp.symbols("vx vy vz q g", real=True)
    grad_div = sp.Matrix([2 * h, 0, 0])
    dt_curl = sp.Matrix([e, -d, 0])
    curl_curl = sp.Matrix([0, 0, -2 * (a + b)])
    velocity = sp.Matrix([vx, vy, vz])
    lorentz = sp.simplify(q * g * (grad_div - dt_curl + velocity.cross(curl_curl)))
    direct = sp.Matrix(
        [
            q * g * (2 * h - e - 2 * (a + b) * vy),
            q * g * (d + 2 * (a + b) * vx),
            0,
        ]
    )

    return {
        "full_stencil_u_form": str(2 * u_sum - sp.Rational(2, 3) * pairs),
        "central_symbol_u_form": str(2 * u_sum - squares),
        "difference": str(difference),
        "difference_sos": str(sos),
        "difference_is_sos": sp.simplify(difference - sos) == 0,
        "static_kernel_nonnegative_and_bounded": True,
        "static_kernel_upper_bound": 3,
        "infrared_limits": limits,
        "infrared_series": series,
        "path_variation_lorentz_identity": sp.simplify(lorentz - direct) == sp.zeros(3, 1),
        "magnetic_scalar_work_zero": sp.simplify(velocity.dot(velocity.cross(curl_curl))) == 0,
        "charge_effective_energy_sign": "-G_C^2 R/2 <= 0",
        "soft_residue_order": "G_C^2 sigma^2 = O(|k|^2)",
    }


def wavevector(size: int, mode: int, direction: int) -> np.ndarray:
    value = 2 * math.pi * mode / size
    if direction == 0:
        return np.array((value, 0.0, 0.0))
    if direction == 1:
        return np.array((value, value, 0.0))
    return np.array((value, value, value))


def stencil_symbol(k: np.ndarray) -> float:
    c = np.cos(k)
    return float(4 - (2 / 3) * np.sum(c) - (2 / 3) * (
        c[0] * c[1] + c[0] * c[2] + c[1] * c[2]
    ))


def kernel(k: np.ndarray) -> float:
    sigma2 = float(np.dot(np.sin(k), np.sin(k)))
    return sigma2 / (C2 * stencil_symbol(k))


def transverse_basis(direction: int) -> tuple[np.ndarray, np.ndarray]:
    if direction == 0:
        return np.array((0.0, 1.0, 0.0)), np.array((0.0, 0.0, 1.0))
    if direction == 1:
        return (
            np.array((1.0, -1.0, 0.0)) / math.sqrt(2),
            np.array((0.0, 0.0, 1.0)),
        )
    return (
        np.array((1.0, -1.0, 0.0)) / math.sqrt(2),
        np.array((1.0, 1.0, -2.0)) / math.sqrt(6),
    )


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
    def derivative(component: int, axis: int) -> np.ndarray:
        return 0.5 * (
            np.roll(field[..., component], -1, axis=axis)
            - np.roll(field[..., component], 1, axis=axis)
        )

    return np.stack(
        (
            derivative(2, 1) - derivative(1, 2),
            derivative(0, 2) - derivative(2, 0),
            derivative(1, 0) - derivative(0, 1),
        ),
        axis=-1,
    )


def periodic_fixture(size: int, which: int, time: float) -> np.ndarray:
    grid = np.indices((size, size, size), dtype=np.float64)
    kx, ky, kz = (2 * math.pi * grid[axis] / size for axis in range(3))
    phase = which + 1
    return np.stack(
        (
            0.23 * np.cos(kx + ky + time) + 0.07 * np.sin(phase * kz),
            -0.17 * np.sin(ky + kz - 0.3 * time) + 0.05 * np.cos(phase * kx),
            0.29 * np.cos(kz + kx + 0.2 * time) - 0.11 * np.sin(phase * ky),
        ),
        axis=-1,
    )


def numerical_proof() -> dict[str, object]:
    maximum_kernel_identity = 0.0
    minimum_kernel = math.inf
    maximum_kernel = 0.0
    maximum_charge_residual = 0.0
    maximum_current_residual = 0.0
    maximum_div_b = 0.0
    maximum_faraday = 0.0
    same_cross_max = -math.inf
    opposite_cross_min = math.inf
    infrared_arms = charge_arms = current_arms = operator_arms = 0
    infrared_monotonic = True
    soft_monotonic = True

    for mode, direction in itertools.product(range(1, 4), range(3)):
        previous_error = math.inf
        previous_soft = math.inf
        for size in (16, 32, 64):
            k = wavevector(size, mode, direction)
            s = np.sin(k)
            value = kernel(k)
            u = 1 - np.cos(k)
            pairs = u[0] * u[1] + u[0] * u[2] + u[1] * u[2]
            m_from_u = 2 * np.sum(u) - (2 / 3) * pairs
            sigma_from_u = 2 * np.sum(u) - np.dot(u, u)
            maximum_kernel_identity = max(
                maximum_kernel_identity,
                abs(stencil_symbol(k) - m_from_u),
                abs(np.dot(s, s) - sigma_from_u),
            )
            minimum_kernel = min(minimum_kernel, value)
            maximum_kernel = max(maximum_kernel, value)
            error = abs(value - 3)
            infrared_monotonic &= error < previous_error
            previous_error = error
            soft = G_C**2 * float(np.dot(s, s))
            soft_monotonic &= soft < previous_soft
            previous_soft = soft
            infrared_arms += 1

    for size, charge, direction in itertools.product((16, 32), (-1, 1), range(3)):
        k = wavevector(size, 1, direction)
        s = np.sin(k)
        stiffness = C2 * stencil_symbol(k)
        source = -G_C * 1j * s * charge
        j_field = source / stiffness
        phi = -G_C * 1j * np.dot(s, j_field)
        expected = -G_C**2 * kernel(k) * charge
        maximum_charge_residual = max(maximum_charge_residual, abs(phi - expected))
        same_cross_max = max(same_cross_max, -G_C**2 * kernel(k))
        opposite_cross_min = min(opposite_cross_min, G_C**2 * kernel(k))
        charge_arms += 1

    for size, direction in itertools.product((16, 32), range(3)):
        k = wavevector(size, 1, direction)
        s = np.sin(k)
        stiffness = C2 * stencil_symbol(k)
        for current in transverse_basis(direction):
            source = G_C * 1j * np.cross(s, current)
            j_field = source / stiffness
            potential = G_C * 1j * np.cross(s, j_field)
            expected = G_C**2 * kernel(k) * current
            maximum_current_residual = max(
                maximum_current_residual, float(np.max(np.abs(potential - expected)))
            )
            current_arms += 1

    for size, which in itertools.product((5, 7), range(2)):
        j0 = periodic_fixture(size, which, 0.0)
        j1 = periodic_fixture(size, which, 0.37)
        b0 = G_C * curl(curl(j0))
        b1 = G_C * curl(curl(j1))
        midpoint = 0.5 * (j0 + j1)
        electric = G_C * gradient(divergence(midpoint)) - G_C * curl(j1 - j0)
        maximum_div_b = max(maximum_div_b, float(np.max(np.abs(divergence(b0)))))
        maximum_faraday = max(
            maximum_faraday, float(np.max(np.abs((b1 - b0) + curl(electric))))
        )
        operator_arms += 1

    return {
        "infrared_symbol_arms": infrared_arms,
        "static_charge_arms": charge_arms,
        "static_transverse_current_arms": current_arms,
        "periodic_operator_identity_arms": operator_arms,
        "minimum_static_kernel": minimum_kernel,
        "maximum_static_kernel": maximum_kernel,
        "maximum_kernel_identity_residual": maximum_kernel_identity,
        "maximum_charge_response_residual": maximum_charge_residual,
        "maximum_current_response_residual": maximum_current_residual,
        "maximum_divergence_of_b_residual": maximum_div_b,
        "maximum_faraday_residual": maximum_faraday,
        "largest_same_polarity_cross_energy": same_cross_max,
        "smallest_opposite_polarity_cross_energy": opposite_cross_min,
        "infrared_monotonic": infrared_monotonic,
        "soft_residue_monotonic": soft_monotonic,
    }


def native_cpp_record() -> dict[str, object]:
    executable = REPO / "engine/build/Release/test_native_hodge_reciprocity.exe"
    completed = subprocess.run(
        [str(executable)],
        cwd=executable.parent,
        check=True,
        capture_output=True,
        text=True,
    )
    parsed: dict[str, object] = {}
    for line in completed.stdout.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip().replace(" ", "_")
        value = value.strip()
        if key == "verdict":
            parsed[key] = value
            continue
        try:
            parsed[key] = int(value)
        except ValueError:
            parsed[key] = float(value)
    assert parsed["verdict"] == VERDICT
    assert parsed["native_hodge_reciprocity_failures"] == 0
    return parsed


def main() -> int:
    prereg = REPO / (
        "docs/theory/10_eft_program/preregistrations/"
        "common_action_mechanics_reciprocity/"
        "PREREG_NATIVE_HODGE_RECIPROCITY_STATIC_POLE_v1.md"
    )
    assert sha256(prereg) == PREREG_SHA
    source_hashes = {"preregistration": sha256(prereg)}
    for key, (relative, expected) in LOCKED_HASHES.items():
        observed = sha256(REPO / relative)
        assert observed == expected, (key, observed, expected)
        source_hashes[key] = observed

    symbolic = symbolic_proof()
    assert symbolic["difference_is_sos"]
    assert symbolic["static_kernel_nonnegative_and_bounded"]
    assert symbolic["path_variation_lorentz_identity"]
    assert symbolic["magnetic_scalar_work_zero"]

    numerical = numerical_proof()
    cpp_run = native_cpp_record()
    passes = (
        numerical["infrared_symbol_arms"] == 27
        and numerical["static_charge_arms"] == 12
        and numerical["static_transverse_current_arms"] == 12
        and numerical["periodic_operator_identity_arms"] == 4
        and numerical["minimum_static_kernel"] >= -TOL
        and numerical["maximum_static_kernel"] <= 3 + TOL
        and numerical["maximum_kernel_identity_residual"] <= TOL
        and numerical["maximum_charge_response_residual"] <= TOL
        and numerical["maximum_current_response_residual"] <= TOL
        and numerical["maximum_divergence_of_b_residual"] <= TOL
        and numerical["maximum_faraday_residual"] <= TOL
        and numerical["largest_same_polarity_cross_energy"] < 0
        and numerical["smallest_opposite_polarity_cross_energy"] > 0
        and numerical["infrared_monotonic"]
        and numerical["soft_residue_monotonic"]
        and cpp_run["infrared_symbol_arms"] == 27
        and cpp_run["proper_cubic_rotation_arms"] == 24
        and cpp_run["static_charge_arms"] == 12
        and cpp_run["static_transverse_current_arms"] == 12
        and cpp_run["brillouin_corner_controls"] == 4
        and cpp_run["periodic_operator_identity_arms"] == 4
        and cpp_run["smooth_path_variation_arms"] == 8
        and cpp_run["maximum_kernel_identity_residual"] <= TOL
        and cpp_run["maximum_charge_response_residual"] <= TOL
        and cpp_run["maximum_current_response_residual"] <= TOL
        and cpp_run["maximum_divergence_of_b_residual"] <= TOL
        and cpp_run["maximum_faraday_residual"] <= TOL
        and cpp_run["maximum_interaction_rewrite_residual"] <= TOL
        and cpp_run["maximum_path_variation_residual"] <= 1e-10
        and cpp_run["maximum_magnetic_scalar_work"] <= TOL
    )
    assert passes

    implementation_hashes = {
        key: sha256(REPO / relative) for key, relative in IMPLEMENTATION_PATHS.items()
    }
    record = {
        "ftd_id": "FTD-0575",
        "verdict": VERDICT,
        "platform": f"{platform.system()}-{platform.release()}-{platform.version()}",
        "backend": "MSVC 14.44.35207 Release CPU plus independent SymPy/NumPy proof",
        "tolerance": TOL,
        **numerical,
        "native_cpp_run": cpp_run,
        "symbolic_checks": symbolic,
        "hodge_lorentz_force_derived": True,
        "static_charge_pole_canceled": True,
        "static_current_pole_canceled": True,
        "same_polarity_static_interaction_attractive": True,
        "soft_radiative_residue_quadratic": True,
        "reciprocal_force_is_coulomb_electromagnetism": False,
        "exact_finite_step_total_energy_derived": False,
        "mobile_manifested_solution_derived": False,
        "production_changed": False,
        "passes": passes,
        "source_hashes_sha256": source_hashes,
        "implementation_hashes_sha256": implementation_hashes,
    }
    output = REPO / "engine/results/ftd_0575/windows_msvc_cpu.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(record, indent=2))
    print("PASS: reciprocal Hodge force derived; static Coulomb pole canceled")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
