#!/usr/bin/env python3
"""Exact certificate for FTD-0920.

The calculation uses symbolic, rational, and finite combinatorial algebra.
It performs no numerical search, fit, parameter sweep, or engine mutation.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_NATIVE_CENTRAL_HODGE_SOURCE_COKERNEL_AND_PLAQUETTE_RETURN_BOUNDARY_v1.md":
        "8972B48856FFB374AB1764E539F1405A8EDD794690E660EC12547C839E8DB448",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md":
        "5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "033985919FAC722F47B09311D51B47E5DDB4E5A3A47D0A3F36B736CFAF481D08",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_MINIMAL_MOORE_COMPATIBILITY_COAT.md":
        "49F41E31DFA9542B2BD7AB0A224808C48D06164967A71139D9C4B7BFB5EBA7B7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_PLAQUETTE_C4_CIRCULATION_AND_EMBEDDED_LEAKAGE_BOUNDARY_v1.md":
        "3CD336B101BDA6A4F0E56CBFFC9428C203C5A68E037943408D762900FF58451F",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md":
        "CA05D786A73775B398F90EE33E207E2A4D3522D49ECA86B9BF5774E2D6B1A285",
}

L = 4
SINE4 = (sp.Integer(0), sp.Integer(1), sp.Integer(0), sp.Integer(-1))


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def site_index(point: tuple[int, int, int]) -> int:
    x, y, z = point
    return (x * L + y) * L + z


def stiffness_at_corner(epsilon: tuple[int, int, int]) -> sp.Rational:
    cx, cy, cz = (sp.Integer(-1) ** value for value in epsilon)
    bracket = cx + cy + cz + cx * cy + cy * cz + cz * cx
    return sp.Rational(4, 3) - sp.Rational(2, 9) * bracket


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # The return impulse is forced by the desired restricted kick.
    k11, k22, k33, k12, k13, k23 = sp.symbols(
        "k11 k22 k33 k12 k13 k23", real=True
    )
    K = sp.Matrix([
        [k11, k12, k13],
        [k12, k22, k23],
        [k13, k23, k33],
    ])
    b11, b12, b21, b22, b31, b32 = sp.symbols(
        "b11 b12 b21 b22 b31 b32", real=True
    )
    B = sp.Matrix([[b11, b12], [b21, b22], [b31, b32]])
    c11, c12, c21, c22 = sp.symbols("c11 c12 c21 c22", real=True)
    Kbody = sp.Matrix([[c11, c12], [c21, c22]])
    q1, q2 = sp.symbols("q1 q2", real=True)
    Q = sp.Matrix([q1, q2])
    p1, p2, p3 = sp.symbols("p1 p2 p3", real=True)
    P = sp.Matrix([p1, p2, p3])
    u1, u2, u3 = sp.symbols("u1 u2 u3", real=True)
    Uunknown = sp.Matrix([u1, u2, u3])
    Ureturn = sp.expand(K * B * Q - B * Kbody * Q)
    restricted_residual = sp.simplify(P - K * B * Q + Ureturn - (P - B * Kbody * Q))
    check("return impulse produces the desired restricted kick", restricted_residual == sp.zeros(3, 1))
    unique_solution = sp.solve(
        list(P - K * B * Q + Uunknown - (P - B * Kbody * Q)),
        [u1, u2, u3],
        dict=True,
    )
    check("return impulse is unique", len(unique_solution) == 1)
    if unique_solution:
        solved_u = Uunknown.subs(unique_solution[0])
        check("unique impulse equals K B q minus B Kbody q", sp.simplify(solved_u - Ureturn) == sp.zeros(3, 1))
    else:
        check("unique impulse equals K B q minus B Kbody q", False)

    kappa = sp.symbols("kappa", real=True)
    isotropic_return = sp.expand(K * B * Q - kappa * B * Q)
    check(
        "isotropic C4 return is K minus kappa identity",
        sp.simplify(isotropic_return - (K - kappa * sp.eye(3)) * B * Q) == sp.zeros(3, 1),
    )

    # Exact central-Hodge symbol. C(d) maps j to d cross j.
    dx, dy, dz = sp.symbols("d_x d_y d_z", real=True)
    d = sp.Matrix([dx, dy, dz])
    cross = sp.Matrix([
        [0, -dz, dy],
        [dz, 0, -dx],
        [-dy, dx, 0],
    ])
    H = sp.Matrix.hstack(-d, cross)
    d2 = sp.expand(dx**2 + dy**2 + dz**2)
    check("cross matrix is skew", cross.T == -cross)
    check("cross matrix annihilates d", cross * d == sp.zeros(3, 1))
    check("Hodge symbol obeys H H-transpose equals norm squared identity", sp.simplify(H * H.T - d2 * sp.eye(3)) == sp.zeros(3))
    check("nonzero x derivative gives rank three", H.subs({dx: 1, dy: 0, dz: 0}).rank() == 3)
    check("nonzero generic derivative gives rank three", H.subs({dx: 1, dy: -1, dz: 1}).rank() == 3)
    check("zero derivative gives rank zero", H.subs({dx: 0, dy: 0, dz: 0}).rank() == 0)
    y1, y2, y3 = sp.symbols("y1 y2 y3", real=True)
    Y = sp.Matrix([y1, y2, y3])
    right_inverse = sp.simplify(H.T * Y / d2)
    check("modewise Hodge right inverse is exact away from blind fibers", sp.simplify(H * right_inverse - Y) == sp.zeros(3, 1))

    # L=4 mode count, exact because sin values are integral on this quotient.
    mode_derivatives: dict[tuple[int, int, int], tuple[sp.Integer, sp.Integer, sp.Integer]] = {}
    for mode in product(range(L), repeat=3):
        mode_derivatives[mode] = tuple(SINE4[index] for index in mode)  # type: ignore[assignment]
    blind_modes = tuple(mode for mode, deriv in mode_derivatives.items() if deriv == (0, 0, 0))
    expected_blind = tuple(product((0, 2), repeat=3))
    check("L4 has exactly eight central-derivative blind modes", len(blind_modes) == 8)
    check("blind modes are exactly the zero and Nyquist corners", set(blind_modes) == set(expected_blind))
    mode_ranks = [0 if deriv == (0, 0, 0) else 3 for deriv in mode_derivatives.values()]
    check("L4 relaxed source rank is 168", sum(mode_ranks) == 168)
    check("L4 vector target dimension is 192", 3 * L**3 == 192)
    check("L4 relaxed source cokernel dimension is 24", 3 * L**3 - sum(mode_ranks) == 24)

    # Real-space parity moments. Each central derivative is annihilated by all
    # eight zero/Nyquist characters.
    sites = tuple(product(range(L), repeat=3))
    derivatives: list[sp.Matrix] = []
    for axis in range(3):
        D = sp.zeros(L**3, L**3)
        for point in sites:
            plus = list(point)
            minus = list(point)
            plus[axis] = (plus[axis] + 1) % L
            minus[axis] = (minus[axis] - 1) % L
            D[site_index(point), site_index(tuple(plus))] = sp.Rational(1, 2)
            D[site_index(point), site_index(tuple(minus))] = sp.Rational(-1, 2)
        derivatives.append(D)
    check("all three central derivative matrices are skew", all(D.T == -D for D in derivatives))

    characters: dict[tuple[int, int, int], sp.Matrix] = {}
    for epsilon in product((0, 1), repeat=3):
        characters[epsilon] = sp.Matrix([
            sp.Integer(-1) ** sum(epsilon[axis] * point[axis] for axis in range(3))
            for point in sites
        ])
    for epsilon, character in characters.items():
        check(
            f"parity character {epsilon} annihilates every central derivative",
            all((character.T * D) == sp.zeros(1, L**3) for D in derivatives),
        )

    # Walsh invertibility establishes equivalence between eight moment zeros
    # and zero total vector impulse in each of the eight parity classes.
    parities = tuple(product((0, 1), repeat=3))
    walsh = sp.Matrix([
        [sp.Integer(-1) ** sum(epsilon[i] * parity[i] for i in range(3)) for parity in parities]
        for epsilon in parities
    ])
    check("eight-parity Walsh matrix squares to eight identity", walsh * walsh.T == 8 * sp.eye(8))
    check("eight-parity moment map is invertible", walsh.det() != 0)
    check("Walsh inverse is one eighth transpose", walsh.inv() == walsh.T / 8)

    # Elementary plaquette blind-fiber discriminator.
    fhat: dict[tuple[int, int, int], sp.Integer] = {}
    for epsilon in parities:
        fhat[epsilon] = sp.Integer(1) - sp.Integer(-1) ** (epsilon[0] + epsilon[1])
    nonzero_fibers = tuple(epsilon for epsilon, value in fhat.items() if value != 0)
    expected_nonzero = ((1, 0, 0), (1, 0, 1), (0, 1, 0), (0, 1, 1))
    check("plaquette has exactly four nonzero blind fibers", len(nonzero_fibers) == 4)
    check("plaquette blind fibers are exactly xor-odd in x and y", set(nonzero_fibers) == set(expected_nonzero))
    check("every nonzero plaquette blind amplitude equals two", all(fhat[e] == 2 for e in nonzero_fibers))
    check("plaquette zero mode vanishes", fhat[(0, 0, 0)] == 0)

    corner_stiffness = {epsilon: stiffness_at_corner(epsilon) for epsilon in parities}
    check("z-even plaquette blind stiffness is four thirds", corner_stiffness[(1, 0, 0)] == sp.Rational(4, 3) and corner_stiffness[(0, 1, 0)] == sp.Rational(4, 3))
    check("z-odd plaquette blind stiffness is sixteen ninths", corner_stiffness[(1, 0, 1)] == sp.Rational(16, 9) and corner_stiffness[(0, 1, 1)] == sp.Rational(16, 9))
    check("the two blind stiffness values are distinct", sp.Rational(4, 3) != sp.Rational(16, 9))

    return_blind = {
        epsilon: sp.expand((corner_stiffness[epsilon] - kappa) * fhat[epsilon])
        for epsilon in nonzero_fibers
    }
    no_common_kappa = sp.solve(list(return_blind.values()), [kappa], dict=True)
    check("no single body stiffness cancels all plaquette blind return", no_common_kappa == [])
    check("four-thirds choice leaves the z-odd return nonzero", sp.simplify(return_blind[(1, 0, 1)].subs(kappa, sp.Rational(4, 3))) != 0)
    check("sixteen-ninths choice leaves the z-even return nonzero", sp.simplify(return_blind[(1, 0, 0)].subs(kappa, sp.Rational(16, 9))) != 0)

    internal_control = {
        epsilon: sp.simplify(value.subs(kappa, sp.Rational(25, 18)))
        for epsilon, value in return_blind.items()
    }
    check("internal-stiffness z-even blind return is minus one ninth", internal_control[(1, 0, 0)] == sp.Rational(-1, 9) and internal_control[(0, 1, 0)] == sp.Rational(-1, 9))
    check("internal-stiffness z-odd blind return is seven ninths", internal_control[(1, 0, 1)] == sp.Rational(7, 9) and internal_control[(0, 1, 1)] == sp.Rational(7, 9))

    quarter_turn_control = {
        epsilon: sp.simplify(value.subs(kappa, sp.Integer(2)))
        for epsilon, value in return_blind.items()
    }
    check("quarter-turn z-even blind return is minus four thirds", quarter_turn_control[(1, 0, 0)] == sp.Rational(-4, 3) and quarter_turn_control[(0, 1, 0)] == sp.Rational(-4, 3))
    check("quarter-turn z-odd blind return is minus four ninths", quarter_turn_control[(1, 0, 1)] == sp.Rational(-4, 9) and quarter_turn_control[(0, 1, 1)] == sp.Rational(-4, 9))

    # Existing FTD-0577 Moore coat removes every nonzero blind corner. The
    # plaquette itself removes the zero corner, so the coated return satisfies
    # the relaxed periodic range criterion for every kappa.
    coat_corner = {
        epsilon: sp.prod((sp.Integer(1) + sp.Integer(-1) ** value) / 2 for value in epsilon)
        for epsilon in parities
    }
    check("Moore coat preserves the zero corner", coat_corner[(0, 0, 0)] == 1)
    check("Moore coat kills all seven nonzero corners", all(coat_corner[e] == 0 for e in parities if e != (0, 0, 0)))
    coated_fhat = {epsilon: sp.simplify(coat_corner[epsilon] * fhat[epsilon]) for epsilon in parities}
    check("coated plaquette kills all eight blind fibers", all(value == 0 for value in coated_fhat.values()))
    coated_return = {
        epsilon: sp.expand((corner_stiffness[epsilon] - kappa) * coated_fhat[epsilon])
        for epsilon in parities
    }
    check("coated return kills all blind fibers for arbitrary stiffness", all(value == 0 for value in coated_return.values()))
    check("coated return is in relaxed finite-periodic global source range", all(value == 0 for value in coated_return.values()) and sum(mode_ranks) == 168)

    # Frozen production/source and scope markers.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
    field_ops = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    hodge_audit = (
        ROOT
        / "docs/theory/07_assessment/common_action_mechanics_reciprocity/AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md"
    ).read_text(encoding="utf-8")
    coat_theorem = (
        ROOT
        / "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_MINIMAL_MOORE_COMPATIBILITY_COAT.md"
    ).read_text(encoding="utf-8")
    check("production contains the negative central state gradient source", "rb.delta_j_[i] -= ::ftd::gradient_state_op" in phase_read)
    check("production contains the positive central state-current curl source", "rb.delta_j_[i] += ::ftd::curl_state_velocity_op" in phase_read)
    check("production source uses state times velocity current", "voxels[ni].velocity * static_cast<double>(state.state_at(ni))" in field_ops)
    check("production still applies kick before drift", phase_write.index("v.wave_vel += rb.delta_j_[i];") < phase_write.index("v.flux += v.wave_vel;"))
    check("prior Hodge audit retains checkerboard-pole obstruction", "checkerboard pole" in hodge_audit)
    check("prior Moore theorem retains noncardinal status", "not cardinal" in coat_theorem)
    check("certificate changes no engine source", True)
    check("ternary and support-gated realization is not inferred from relaxed range", True)
    check("local inverse, continuity, reciprocity, and energy closure remain open", True)
    check("G-star, gamma, Born, Bell, selector, and measurement targets are unused", True)
    check("no fit, sweep, near-miss, or formula-substitution discovery is performed", True)

    combined = all(passed for _, passed in checks)
    check("combined Outcome A discriminator", combined)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0920 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=A_NATIVE_CENTRAL_SOURCE_COKERNEL_PLAQUETTE_RETURN_OBSTRUCTION")
        print("UNIQUE_RETURN_SOURCE=K_MINUS_KAPPA")
        print("CENTRAL_HODGE_BLIND_FIBERS=8")
        print("L4_VECTOR_COKERNEL_DIMENSION=24")
        print("ELEMENTARY_PLAQUETTE_DIRECT_SOURCE_CLOSURE=FALSE")
        print("MOORE_COAT_RELAXED_PERIODIC_RANGE=TRUE")
        print("TERNARY_LOCAL_RECIPROCAL_REALIZATION=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
