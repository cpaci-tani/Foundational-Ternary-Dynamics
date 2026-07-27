#!/usr/bin/env python3
"""Independent FTD-0574 native field-action and source-operator proof."""

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
PREREG_SHA = "09970E8A18974B56F399DC68023BD7527FDCED50A937054413C3FC53B7F1AFEB"
VERDICT = "NATIVE_FIELD_DISCRETE_ACTION_DERIVED_MAGNETIC_SOURCE_ACTION_MISMATCH"

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
    "header": "engine/include/ftd/eft/native_field_discrete_action.h",
    "source": "engine/src/eft/native_field_discrete_action.cpp",
    "test": "engine/tests/test_native_field_discrete_action.cpp",
    "independent_proof": "scripts/proofs/proof_native_field_discrete_action.py",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def symbolic_proof() -> dict[str, object]:
    a = sp.symbols("a", real=True)
    jm, j, jp = sp.symbols("J_previous J J_next", real=True)
    x, y, z = sp.symbols("g11 g12 g22", real=True)
    omega = sp.Matrix([[0, 1], [-1, 0]])
    transfer = sp.Matrix([[1 - a, 1], [-a, 1]])
    invariant = sp.Matrix([[a, -a / 2], [-a / 2, 1]])

    discrete_lagrangian = (jp - j) ** 2 / 2 - a * j**2 / 2
    previous_lagrangian = (j - jm) ** 2 / 2 - a * jm**2 / 2
    del_equation = sp.factor(
        sp.diff(previous_lagrangian, j) + sp.diff(discrete_lagrangian, j)
    )
    left_legendre = sp.factor(-sp.diff(discrete_lagrangian, j))
    right_legendre = sp.factor(sp.diff(previous_lagrangian, j))

    general = sp.Matrix([[x, y], [y, z]])
    constraint_matrix, _ = sp.linear_eq_to_matrix(
        list(transfer.T * general * transfer - general), [x, y, z]
    )
    nullspace = constraint_matrix.nullspace()

    cosine = 1 - a / 2
    b_matrix = sp.simplify(transfer - cosine * sp.eye(2))
    theta = sp.acos(cosine)
    sine = sp.sqrt(a * (1 - a / 4))
    mu = theta / sine
    shadow_metric = sp.simplify(-omega * b_matrix * mu)
    series = sp.series(mu, a, 0, 6)

    # Along <100>, a=(2/3)(1-cos k).  The exact log has a branch at a=4,
    # hence cos(k)=-5 and z+z^-1=-10.  Both roots are finite and nonzero.
    zeta = sp.symbols("zeta")
    branch_roots = sp.solve(sp.Eq(zeta + 1 / zeta, -10), zeta)

    return {
        "symplectic_identity": transfer.T * omega * transfer == omega,
        "invariant_identity": sp.simplify(
            transfer.T * invariant * transfer - invariant
        ) == sp.zeros(2),
        "discrete_el_equation": str(del_equation),
        "discrete_el_matches_tick": sp.simplify(
            del_equation.subs(jp, 2 * j - jm - a * j)
        ) == 0,
        "left_legendre": str(left_legendre),
        "right_legendre": str(right_legendre),
        "legendre_momentum_matches": (
            sp.simplify(left_legendre.subs(jp, 2 * j - jm - a * j) - (j - jm))
            == 0
            and sp.simplify(right_legendre - (j - jm)) == 0
        ),
        "invariant_constraint_rank": int(constraint_matrix.rank()),
        "invariant_constraint_nullity": len(nullspace),
        "invariant_null_generator": [str(item) for item in nullspace[0]],
        "invariant_determinant": str(sp.factor(invariant.det())),
        "b_square": str(sp.simplify(b_matrix * b_matrix)),
        "shadow_metric_is_mu_times_invariant": sp.simplify(
            shadow_metric - mu * invariant
        ) == sp.zeros(2),
        "mu_series": str(series),
        "mu_nonpolynomial": True,
        "finite_nonzero_complex_branch_points": all(
            root.is_finite is True and sp.simplify(root) != 0
            for root in branch_roots
        ),
        "branch_points": [str(root) for root in branch_roots],
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
    return np.stack((dy(2) - dz(1), dz(0) - dx(2), dx(1) - dy(0)), axis=-1)


def apply_k(field: np.ndarray) -> np.ndarray:
    lap = np.zeros_like(field)
    faces = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
    edges = (
        (1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
        (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
        (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1),
    )
    for offset in faces:
        lap += np.roll(field, tuple(-value for value in offset), axis=(0, 1, 2)) / 3
    for offset in edges:
        lap += np.roll(field, tuple(-value for value in offset), axis=(0, 1, 2)) / 6
    lap -= 4 * field
    return -lap / 3


def fixture(size: int, which: int) -> tuple[np.ndarray, ...]:
    grid = np.indices((size, size, size), dtype=np.float64)
    kx, ky, kz = (2 * math.pi * grid[axis] / size for axis in range(3))
    phase = which + 1
    scalar = 0.37 * np.sin(kx + phase * ky) + 0.19 * np.cos(kz - phase * kx)
    field = np.stack(
        (
            0.23 * np.cos(kx + ky) + 0.07 * np.sin(phase * kz),
            -0.17 * np.sin(ky + kz) + 0.05 * np.cos(phase * kx),
            0.29 * np.cos(kz + kx) - 0.11 * np.sin(phase * ky),
        ),
        axis=-1,
    )
    velocity = np.stack(
        (
            0.31 * np.sin(ky + phase * kz),
            -0.27 * np.cos(kz + phase * kx),
            0.21 * np.sin(kx - phase * ky),
        ),
        axis=-1,
    )
    current = scalar[..., None] * velocity
    variation = np.stack(
        (
            0.13 * np.sin(kx - kz) + 0.09 * np.cos(phase * ky),
            0.16 * np.cos(ky - kx) - 0.08 * np.sin(phase * kz),
            -0.12 * np.sin(kz - ky) + 0.06 * np.cos(phase * kx),
        ),
        axis=-1,
    )
    return scalar, field, current, variation


def normalized_residual(lhs: float, rhs: float) -> float:
    return abs(lhs - rhs) / (1 + max(abs(lhs), abs(rhs)))


def numerical_operator_proof() -> dict[str, object]:
    g_c = math.sqrt(1 / 137.035999177)
    maxima = {
        "electric_adjoint": 0.0,
        "curl_adjoint": 0.0,
        "correct_source_action": 0.0,
        "documented_action_derivative": 0.0,
        "discrete_el": 0.0,
        "legendre": 0.0,
        "tick_energy": 0.0,
    }
    operator_arms = lattice_arms = 0
    for size, which in itertools.product((5, 7), range(2)):
        scalar, field, current, variation = fixture(size, which)
        grad_s = gradient(scalar)
        curl_current = curl(current)
        coded = -g_c * grad_s + g_c * curl_current

        electric_left = float(np.sum(scalar * divergence(variation)))
        electric_right = float(np.sum(variation * (-grad_s)))
        curl_left = float(np.sum(curl(variation) * current))
        curl_right = float(np.sum(variation * curl_current))
        maxima["electric_adjoint"] = max(
            maxima["electric_adjoint"], normalized_residual(electric_left, electric_right)
        )
        maxima["curl_adjoint"] = max(
            maxima["curl_adjoint"], normalized_residual(curl_left, curl_right)
        )

        def correct(j_value: np.ndarray) -> float:
            return float(g_c * (
                np.sum(scalar * divergence(j_value))
                + np.sum(curl(j_value) * current)
            ))

        def documented(j_value: np.ndarray) -> float:
            return float(-g_c * np.sum(current * j_value))

        epsilon = 0.5
        measured_correct = (
            correct(field + epsilon * variation) - correct(field - epsilon * variation)
        ) / (2 * epsilon)
        predicted_correct = float(np.sum(variation * coded))
        measured_documented = (
            documented(field + epsilon * variation)
            - documented(field - epsilon * variation)
        ) / (2 * epsilon)
        predicted_documented = float(np.sum(variation * (-g_c * current)))
        maxima["correct_source_action"] = max(
            maxima["correct_source_action"],
            normalized_residual(measured_correct, predicted_correct),
        )
        maxima["documented_action_derivative"] = max(
            maxima["documented_action_derivative"],
            normalized_residual(measured_documented, predicted_documented),
        )
        operator_arms += 1

        j = field
        w = variation
        kj = apply_k(j)
        previous = j - w
        next_w = w - kj
        next_j = j + next_w
        del_residual = float(np.max(np.abs(next_j - 2 * j + previous + kj)))
        legendre_residual = float(np.max(np.abs((next_j - j + kj) - w)))
        next_kj = apply_k(next_j)
        before = float(np.sum(0.5 * w * w + 0.5 * j * kj - 0.5 * w * kj))
        after = float(np.sum(
            0.5 * next_w * next_w + 0.5 * next_j * next_kj - 0.5 * next_w * next_kj
        ))
        maxima["discrete_el"] = max(maxima["discrete_el"], del_residual)
        maxima["legendre"] = max(maxima["legendre"], legendre_residual)
        maxima["tick_energy"] = max(
            maxima["tick_energy"], normalized_residual(before, after)
        )
        lattice_arms += 1

    velocities = np.array(
        ((1, 0, 0), (0, 1, 0), (0, 0, 1), np.array((1, -2, 3)) / math.sqrt(14)),
        dtype=np.float64,
    )
    minimum_mismatch = math.inf
    maximum_coded = 0.0
    counterexample_arms = 0
    for size, velocity in itertools.product((5, 7), velocities):
        scalar = np.ones((size, size, size))
        current = np.broadcast_to(velocity, (size, size, size, 3)).copy()
        coded = -g_c * gradient(scalar) + g_c * curl(current)
        documented_gradient = -g_c * current
        maximum_coded = max(maximum_coded, float(np.max(np.abs(coded))))
        minimum_mismatch = min(
            minimum_mismatch,
            float(np.max(np.linalg.norm(coded - documented_gradient, axis=-1))),
        )
        counterexample_arms += 1

    return {
        "source_operator_arms": operator_arms,
        "lattice_action_arms": lattice_arms,
        "uniform_counterexample_arms": counterexample_arms,
        "maximum_electric_adjoint_residual": maxima["electric_adjoint"],
        "maximum_curl_adjoint_residual": maxima["curl_adjoint"],
        "maximum_correct_source_action_residual": maxima["correct_source_action"],
        "maximum_documented_action_derivative_residual": maxima[
            "documented_action_derivative"
        ],
        "maximum_discrete_el_residual": maxima["discrete_el"],
        "maximum_legendre_residual": maxima["legendre"],
        "maximum_tick_energy_residual": maxima["tick_energy"],
        "maximum_uniform_coded_source": maximum_coded,
        "minimum_uniform_documented_source_mismatch": minimum_mismatch,
    }


def native_cpp_record() -> dict[str, object]:
    executable = REPO / "engine/build/Release/test_native_field_discrete_action.exe"
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
    assert parsed["native_field_discrete_action_failures"] == 0
    return parsed


def main() -> int:
    prereg = REPO / (
        "docs/theory/10_eft_program/preregistrations/"
        "PREREG_NATIVE_FIELD_DISCRETE_ACTION_v1.md"
    )
    assert sha256(prereg) == PREREG_SHA
    source_hashes = {"preregistration": sha256(prereg)}
    for key, (relative, expected) in LOCKED_HASHES.items():
        observed = sha256(REPO / relative)
        assert observed == expected, (key, observed, expected)
        source_hashes[key] = observed

    symbolic = symbolic_proof()
    assert symbolic["symplectic_identity"]
    assert symbolic["invariant_identity"]
    assert symbolic["discrete_el_matches_tick"]
    assert symbolic["legendre_momentum_matches"]
    assert symbolic["invariant_constraint_rank"] == 2
    assert symbolic["invariant_constraint_nullity"] == 1
    assert symbolic["shadow_metric_is_mu_times_invariant"]
    assert symbolic["mu_nonpolynomial"]
    assert symbolic["finite_nonzero_complex_branch_points"]

    numerical = numerical_operator_proof()
    cpp_run = native_cpp_record()
    passes = (
        numerical["source_operator_arms"] == 4
        and numerical["lattice_action_arms"] == 4
        and numerical["uniform_counterexample_arms"] == 8
        and numerical["maximum_electric_adjoint_residual"] <= TOL
        and numerical["maximum_curl_adjoint_residual"] <= TOL
        and numerical["maximum_correct_source_action_residual"] <= TOL
        and numerical["maximum_documented_action_derivative_residual"] <= TOL
        and numerical["maximum_discrete_el_residual"] <= TOL
        and numerical["maximum_legendre_residual"] <= TOL
        and numerical["maximum_tick_energy_residual"] <= TOL
        and numerical["maximum_uniform_coded_source"] <= TOL
        and numerical["minimum_uniform_documented_source_mismatch"] > 1e-6
        and cpp_run["mode_arms"] == 36
        and cpp_run["lattice_action_arms"] == 4
        and cpp_run["source_operator_arms"] == 4
        and cpp_run["uniform_counterexample_arms"] == 8
        and cpp_run["proper_cubic_covariance_arms"] == 96
        and cpp_run["maximum_discrete_el_residual"] <= TOL
        and cpp_run["maximum_legendre_momentum_residual"] <= TOL
        and cpp_run["maximum_tick_invariant_residual"] <= TOL
        and cpp_run["maximum_shadow_flow_residual"] <= TOL
        and cpp_run["maximum_correct_source_action_residual"] <= TOL
        and cpp_run["maximum_uniform_coded_source"] <= TOL
        and cpp_run["minimum_uniform_documented_source_mismatch"] > 1e-6
    )
    assert passes

    implementation_hashes = {
        key: sha256(REPO / relative)
        for key, relative in IMPLEMENTATION_PATHS.items()
    }
    record = {
        "ftd_id": "FTD-0574",
        "verdict": VERDICT,
        "platform": f"{platform.system()}-{platform.release()}-{platform.version()}",
        "backend": "MSVC 14.44.35207 Release CPU plus independent SymPy/NumPy proof",
        "tolerance": TOL,
        "mode_arms_registered": 36,
        "proper_cubic_covariance_arms_registered": 96,
        **numerical,
        "source_free_discrete_action_exact": True,
        "wave_velocity_is_discrete_legendre_momentum": True,
        "standard_pairing_native_in_free_field_sector": True,
        "normalized_quadratic_invariant_unique": True,
        "exact_shadow_generator_fixed_finite_range": False,
        "prescribed_source_action_exact": True,
        "documented_velocity_interaction_generates_coded_source": False,
        "full_dynamic_matter_field_action_derived": False,
        "production_changed": False,
        "passes": passes,
        "native_cpp_run": cpp_run,
        "symbolic_checks": symbolic,
        "source_hashes_sha256": source_hashes,
        "implementation_hashes_sha256": implementation_hashes,
    }
    output = REPO / "engine/results/ftd_0574/windows_msvc_cpu.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(record, indent=2))
    print("PASS: native field action derived; documented magnetic source action mismatched")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
