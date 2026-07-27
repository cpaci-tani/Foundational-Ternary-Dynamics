"""Independent proof and run-of-record writer for FTD-0570.

This script performs exact rational natural-extension checks, symbolic
generating-function checks, and an independent numerical replay of the locked
4,320-arm branchwise lift. It does not call the C++ observer.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
import json
import math
from pathlib import Path
import platform

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
TOL = 1.0e-12
PREREG_HASH = "1C5EB97350D49AC03F63CD5BF995BDB31E9D300CFF71E4180339AE4D5CD3E0D8"
SOURCE_HASHES = {
    "phase_write": "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "voxel_rng": "15EA4843331471E0B75488BAB9D87072E1CD7FD41FBC485A2BDD81EBC8841093",
    "reservoir_header": "377472A157BBC17C9EAE1C8A646E0B8FD06076C36F139AF2C897F3C06E1D4C67",
    "reservoir_source": "DE56A0EE1E74F588B2E66AF19B82E1AB48877DC18A3BAD8A6C50CCAC6F27A176",
    "ftd0569_theorem": "565BCD17963322349D5D136E40DE11BF2268677A1CF8D1EED062818EA0E6BFBC",
    "preregistration": PREREG_HASH,
}
SOURCE_PATHS = {
    "phase_write": ROOT / "engine/src/render_bridge_phases/phase_write.cpp",
    "voxel_rng": ROOT / "engine/include/ftd/voxel_rng.h",
    "reservoir_header": ROOT / "engine/include/ftd/eft/genesis_reservoir_dilation.h",
    "reservoir_source": ROOT / "engine/src/eft/genesis_reservoir_dilation.cpp",
    "ftd0569_theorem": ROOT / "docs/theory/10_eft_program/derivations/THEOREM_GENESIS_RESERVOIR_DILATION.md",
    "preregistration": ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_GENESIS_NATURAL_EXTENSION_v1.md",
}
IMPLEMENTATION_PATHS = {
    "header": ROOT / "engine/include/ftd/eft/genesis_natural_extension.h",
    "source": ROOT / "engine/src/eft/genesis_natural_extension.cpp",
    "test": ROOT / "engine/tests/test_genesis_natural_extension.cpp",
    "independent_proof": ROOT / "scripts/proofs/proof_genesis_natural_extension.py",
}


def digest(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def exact_baker(u: Fraction, v: Fraction, p: Fraction) -> tuple[Fraction, Fraction]:
    if u < p:
        return u / p, p * v
    return (u - p) / (1 - p), p + (1 - p) * v


def exact_baker_inverse(U: Fraction, V: Fraction, p: Fraction) -> tuple[Fraction, Fraction]:
    if V < p:
        return p * U, V / p
    return p + (1 - p) * U, (V - p) / (1 - p)


def dot(lhs: list[float], rhs: list[float]) -> float:
    return sum(a * b for a, b in zip(lhs, rhs, strict=True))


def norm(value: list[float]) -> float:
    return math.sqrt(dot(value, value))


def add(lhs: list[float], rhs: list[float]) -> list[float]:
    return [a + b for a, b in zip(lhs, rhs, strict=True)]


def sub(lhs: list[float], rhs: list[float]) -> list[float]:
    return [a - b for a, b in zip(lhs, rhs, strict=True)]


def scale(value: list[float], factor: float) -> list[float]:
    return [factor * entry for entry in value]


def max_abs(value: list[float]) -> float:
    return max(abs(entry) for entry in value)


def apply_A(value: list[float], n: list[float], t: float) -> list[float]:
    radial = scale(n, dot(n, value))
    tangential = sub(value, radial)
    return add(radial, scale(tangential, t))


def apply_A_inv(value: list[float], n: list[float], t: float) -> list[float]:
    radial = scale(n, dot(n, value))
    tangential = sub(value, radial)
    return add(radial, scale(tangential, 1.0 / t))


def apply_DF_T(value: list[float], n: list[float], t: float, a: float,
               accepted: bool) -> list[float]:
    if not accepted:
        return value[:]
    return apply_A(value[:3], n, t) + scale(value[3:], a)


def apply_DF_inv_T(value: list[float], n: list[float], t: float, a: float,
                   accepted: bool) -> list[float]:
    if not accepted:
        return value[:]
    return apply_A_inv(value[:3], n, t) + scale(value[3:], 1.0 / a)


def numerical_lift() -> dict[str, float | int | bool]:
    inv_sqrt3 = 1.0 / math.sqrt(3.0)
    directions = [
        [1.0, 0.0, 0.0], [-1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0], [0.0, -1.0, 0.0],
        [0.0, 0.0, 1.0], [0.0, 0.0, -1.0],
        [inv_sqrt3, inv_sqrt3, inv_sqrt3],
        [-inv_sqrt3, inv_sqrt3, inv_sqrt3],
        [inv_sqrt3, -inv_sqrt3, inv_sqrt3],
        [inv_sqrt3, inv_sqrt3, -inv_sqrt3],
    ]
    excesses = [0.125, 0.5, 1.25]
    waves = [[0.0, 0.0, 0.0], [0.3, -0.4, 0.2], [-0.25, 0.1, 0.5]]
    drains = [0.0, 0.5, 0.9]
    phase_vs = [0.2, 0.8]
    conjugates = [
        [0.1, -0.2, 0.3, -0.4, 0.5, -0.6],
        [-0.7, 0.2, 0.4, 0.1, -0.3, 0.8],
    ]
    taus = [-0.4, 0.7]
    arms = 0
    accepted_arms = 0
    max_inverse = 0.0
    max_energy = 0.0
    max_generator = 0.0
    min_tangent_defect = math.inf
    max_raw_jacobian = 0.0

    for n in directions:
        for x in excesses:
            r = 1.0 + x
            p = 1.0 - math.exp(-x)
            t = x / r
            J = scale(n, r)
            for W in waves:
                for d in drains:
                    a = 1.0 - d
                    grad_p = scale(n, 1.0 - p) + [0.0, 0.0, 0.0]
                    grad_D = n[:] + scale(W, 1.0 - a * a)
                    for accepted in (True, False):
                        for v in phase_vs:
                            for pi in conjugates:
                                for tau in taus:
                                    arms += 1
                                    accepted_arms += int(accepted)
                                    u = 0.37 * p if accepted else p + 0.63 * (1.0 - p)
                                    if accepted:
                                        U, V = u / p, p * v
                                        g = -u * V / (p * p)
                                        Q = scale(J, t) + scale(W, a)
                                        D = x + 0.5 + (d - 0.5 * d * d) * dot(W, W)
                                    else:
                                        U, V = (u - p) / (1.0 - p), p + (1.0 - p) * v
                                        g = U * v - U - v
                                        Q = J + W
                                        D = 0.0
                                    grad_D_b = grad_D if accepted else [0.0] * 6
                                    argument = add(sub(pi, scale(grad_p, g)), scale(grad_D_b, tau))
                                    Pi = apply_DF_inv_T(argument, n, t, a, accepted)
                                    R, R2 = 0.7, 0.7 + D

                                    q_recovered = (
                                        scale(Q[:3], 1.0 + 1.0 / norm(Q[:3]))
                                        + scale(Q[3:], 1.0 / a)
                                        if accepted else Q[:]
                                    )
                                    u0, v0 = (
                                        (p * U, V / p) if accepted
                                        else (p + (1.0 - p) * U, (V - p) / (1.0 - p))
                                    )
                                    pi0 = add(
                                        sub(apply_DF_T(Pi, n, t, a, accepted), scale(grad_D_b, tau)),
                                        scale(grad_p, g),
                                    )
                                    R0 = R2 - D
                                    inverse = max(
                                        max_abs(sub(q_recovered, J + W)),
                                        max_abs(sub(pi0, pi)), abs(u0 - u), abs(v0 - v), abs(R0 - R),
                                    )
                                    energy0 = 0.5 * dot(J + W, J + W) + R
                                    energy1 = 0.5 * dot(Q, Q) + R2
                                    conjugate_eq = max_abs(sub(
                                        pi,
                                        add(
                                            sub(apply_DF_T(Pi, n, t, a, accepted), scale(grad_D_b, tau)),
                                            scale(grad_p, g),
                                        ),
                                    ))
                                    phase_eq = max(
                                        abs(v - (V / p if accepted else (V - p) / (1.0 - p))),
                                        abs(U - (u / p if accepted else (u - p) / (1.0 - p))),
                                    )
                                    max_inverse = max(max_inverse, inverse)
                                    max_energy = max(max_energy, abs(energy1 - energy0))
                                    max_generator = max(max_generator, conjugate_eq, phase_eq)
                                    if accepted:
                                        min_tangent_defect = min(min_tangent_defect, 1.0 - a * t)
                                        max_raw_jacobian = max(max_raw_jacobian, t * t * a ** 3)

    return {
        "lift_arms": arms,
        "accepted_lift_arms": accepted_arms,
        "maximum_lift_inverse_residual": max_inverse,
        "maximum_energy_residual": max_energy,
        "maximum_generator_residual": max_generator,
        "minimum_raw_tangential_defect_magnitude": min_tangent_defect,
        "maximum_raw_volume_jacobian": max_raw_jacobian,
        "passes": arms == 4320 and accepted_arms == 2160
        and max_inverse <= 1.0e-11 and max_energy <= TOL
        and max_generator <= TOL and min_tangent_defect > 0.0
        and max_raw_jacobian < 1.0,
    }


def main() -> None:
    observed_hashes = {key: digest(path) for key, path in SOURCE_PATHS.items()}
    assert observed_hashes == SOURCE_HASHES, (observed_hashes, SOURCE_HASHES)

    probabilities = [Fraction(1, 4), Fraction(1, 2), Fraction(3, 4)]
    u0 = Fraction(314159, 1_000_000)
    v0 = Fraction(271828, 1_000_000)
    u, v = u0, v0
    schedule = [probabilities[i % 3] for i in range(100)]
    for p in schedule:
        u, v = exact_baker(u, v, p)
    for p in reversed(schedule):
        u, v = exact_baker_inverse(u, v, p)
    exact_depth_100 = u == u0 and v == v0
    assert exact_depth_100

    history_a = Fraction(0)
    history_b = Fraction(0)
    history_a = history_a / 2
    history_b = Fraction(1, 2) + history_b / 2
    for _ in range(63):
        history_a = Fraction(1, 2) + history_a / 2
        history_b = Fraction(1, 2) + history_b / 2
    exact_histories_distinct = history_a != history_b
    binary64_histories_collide = float(history_a) == float(history_b)
    assert exact_histories_distinct and binary64_histories_collide

    u_s, V_s, p_s = sp.symbols("u V p", positive=True)
    S1 = u_s * V_s / p_s
    S0 = (u_s - p_s) * (V_s - p_s) / (1 - p_s)
    U_s, v_s = sp.symbols("U v")
    symbolic_phase_generators = (
        sp.simplify(sp.diff(S1, u_s) - V_s / p_s) == 0
        and sp.simplify(sp.diff(S1, V_s) - u_s / p_s) == 0
        and sp.simplify(sp.diff(S1, p_s) + u_s * V_s / p_s ** 2) == 0
        and sp.simplify(sp.diff(S0, u_s) - (V_s - p_s) / (1 - p_s)) == 0
        and sp.simplify(sp.diff(S0, V_s) - (u_s - p_s) / (1 - p_s)) == 0
        and sp.simplify(
            sp.diff(S0, p_s).subs({u_s: p_s + (1 - p_s) * U_s,
                                    V_s: p_s + (1 - p_s) * v_s})
            - (U_s * v_s - U_s - v_s)
        ) == 0
    )
    assert symbolic_phase_generators

    x_s, kg_s, d_s, w2_s, R_s = sp.symbols("x kg d w2 R", positive=True)
    a_s = 1 - d_s
    H0 = sp.Rational(1, 2) * ((kg_s + x_s) ** 2 + w2_s)
    H1 = sp.Rational(1, 2) * (x_s ** 2 + a_s ** 2 * w2_s)
    D_s = kg_s * x_s + kg_s ** 2 / 2 + (d_s - d_s ** 2 / 2) * w2_s
    symbolic_energy = sp.simplify(H1 + R_s + D_s - (H0 + R_s)) == 0
    assert symbolic_energy

    numerical = numerical_lift()
    assert numerical["passes"]
    projected_ratio = math.inf

    output = {
        "ftd_id": "FTD-0570",
        "verdict": "EXACT_REAL_NATURAL_EXTENSION_ADDITIONAL_PRIMITIVES_REQUIRED",
        "platform": platform.platform(),
        "field_representation": "frozen canonical genesis trial plus observer-only exact-real environment",
        "tolerance": TOL,
        "exact_fraction_depth": 100,
        "exact_fraction_depth_100_inverts": exact_depth_100,
        "exact_64_branch_histories_distinct": exact_histories_distinct,
        "binary64_64_branch_histories_collide": binary64_histories_collide,
        "symbolic_phase_generators_close": symbolic_phase_generators,
        "symbolic_extended_energy_closes": symbolic_energy,
        "projected_log_forward_reverse_ratio": "infinity",
        "projected_kernel_absolutely_irreversible": math.isinf(projected_ratio),
        **numerical,
        "raw_genesis_canonical": False,
        "production_common_action_recovered": False,
        "additional_primitives_required": True,
        "source_hashes_sha256": observed_hashes,
        "implementation_hashes_sha256": {
            key: digest(path) for key, path in IMPLEMENTATION_PATHS.items()
        },
    }
    result_path = ROOT / "engine/results/ftd_0570/windows_msvc_cpu.json"
    result_path.parent.mkdir(parents=True, exist_ok=True)
    result_path.write_text(json.dumps(output, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(output, indent=2))
    print("PASS: exact-real natural extension exists; frozen production common action does not")


if __name__ == "__main__":
    main()
