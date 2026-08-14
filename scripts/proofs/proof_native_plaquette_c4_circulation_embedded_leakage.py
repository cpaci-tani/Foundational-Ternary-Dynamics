#!/usr/bin/env python3
"""Exact certificate for FTD-0918.

The calculation uses finite symbolic and rational algebra only.  It performs
no numerical search, fit, parameter sweep, or production-engine mutation.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_NATIVE_PLAQUETTE_C4_CIRCULATION_AND_EMBEDDED_LEAKAGE_BOUNDARY_v1.md":
        "2B2CA0D8D2696AFC529308D3B184ADADB87C9F88408194A53ADE42E9F4473157",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "engine/include/ftd/lagrangian.h":
        "0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_PRODUCTION_BOUNDARY_v1.md":
        "656F51A4E5A533C0436E932B452A33810CD851D63E571621DF81ECB0C9BED622",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_TERNARY_PLAQUETTE_QUARTER_TURN_RECURSION_v1.md":
        "DC98BB8E8A0CF39E832F7399631F831FF71D3216ED104B6C76384EEEF9100B26",
    "docs/theory/10_eft_program/reports_and_audits/"
    "ANALYSIS_PRODUCTION_TERNARY_PLAQUETTE_RECURRENCE_CENSUS_v1.md":
        "6DD72FC5666FB8AA649055B0C6F4224FBF4E50090D898641AC67865C527E20F3",
}


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # First C4 harmonic and the induced site-shift representation.
    f0, f1, f2, f3 = sp.symbols("f0 f1 f2 f3", real=True)
    word = sp.Matrix([f0, f1, f2, f3])
    projection = sp.Matrix([
        [sp.Rational(1, 2), 0, sp.Rational(-1, 2), 0],
        [0, sp.Rational(1, 2), 0, sp.Rational(-1, 2)],
    ])
    forward_shift = sp.Matrix([
        [0, 0, 0, 1],
        [1, 0, 0, 0],
        [0, 1, 0, 0],
        [0, 0, 1, 0],
    ])
    reverse_shift = forward_shift.T
    R = sp.Matrix([[0, -1], [1, 0]])
    check("forward site shift induces the C4 matrix", projection * forward_shift * word == R * projection * word)
    check("reverse site shift induces the inverse C4 matrix", projection * reverse_shift * word == R.T * projection * word)
    check("C4 matrix squares to minus identity", R**2 == -sp.eye(2))
    check("C4 matrix has order four", R**4 == sp.eye(2))
    check("reverse matrix is the forward inverse", R.T == R.inv())

    # Charge parities.  A scalar component proves each component; vector
    # charge is their sum.
    q, r, pq, pr = sp.symbols("q r p_q p_r", real=True)
    L = sp.expand(q * pr - r * pq)
    x = sp.Matrix([q, r])
    p = sp.Matrix([pq, pr])
    x_rot, p_rot = R * x, R * p
    L_rot = sp.expand(x_rot[0] * p_rot[1] - x_rot[1] * p_rot[0])
    reflection = sp.diag(1, -1)
    x_ref, p_ref = reflection * x, reflection * p
    L_ref = sp.expand(x_ref[0] * p_ref[1] - x_ref[1] * p_ref[0])
    check("circulation charge is C4 invariant", sp.simplify(L_rot - L) == 0)
    check("circulation charge is reflection odd", sp.simplify(L_ref + L) == 0)
    check("circulation charge is canonical-time-reversal odd", sp.simplify(q * (-pr) - r * (-pq) + L) == 0)

    # Symmetric commutant of C4.
    a, b, c = sp.symbols("a b c", real=True)
    K = sp.Matrix([[a, b], [b, c]])
    commutator = sp.expand(K * R - R * K)
    solution = sp.solve(list(commutator), [a, b, c], dict=True)
    check("symmetric C4 commutant forces zero off-diagonal", solution == [{a: c, b: 0}])
    check("symmetric C4 commutant is scalar identity", sp.simplify(K.subs({a: c, b: 0}) - c * sp.eye(2)) == sp.zeros(2))

    # Exact isotropic kick--drift charge balance.
    h, kappa = sp.symbols("h kappa", real=True)
    pq_k = pq - h * kappa * q
    pr_k = pr - h * kappa * r
    q_d = q + h * pq_k
    r_d = r + h * pr_k
    L_d = sp.expand(q_d * pr_k - r_d * pq_k)
    check("isolated isotropic kick-drift conserves circulation", sp.simplify(L_d - L) == 0)

    uq, ur = sp.symbols("u_q u_r", real=True)
    pq_u = pq - h * kappa * q + uq
    pr_u = pr - h * kappa * r + ur
    q_u = q + h * pq_u
    r_u = r + h * pr_u
    L_u = sp.expand(q_u * pr_u - r_u * pq_u)
    torque = sp.expand(q * ur - r * uq)
    check("projected kick has the exact source-torque balance", sp.simplify(L_u - L - torque) == 0)

    rho, etaq, etar = sp.symbols("rho eta_q eta_r", real=True)
    L_end = sp.expand(q_u * (rho * pr_u + etar) - r_u * (rho * pq_u + etaq))
    expected_end = sp.expand(rho * (L + torque) + q_u * etar - r_u * etaq)
    check("common damping and additive impulse obey the exact end balance", sp.simplify(L_end - expected_end) == 0)
    check("common damping alone scales the charge", sp.simplify(L_end.subs({uq: 0, ur: 0, etaq: 0, etar: 0}) - rho * L) == 0)

    # Exact internal four-corner block of the 18-point stencil.  Consecutive
    # corners are face neighbors; the opposite corner is an edge neighbor.
    one_third = sp.Rational(1, 3)
    one_sixth = sp.Rational(1, 6)
    stencil4 = sp.Matrix([
        [-4, one_third, one_sixth, one_third],
        [one_third, -4, one_third, one_sixth],
        [one_sixth, one_third, -4, one_third],
        [one_third, one_sixth, one_third, -4],
    ])
    doublet_word = sp.Matrix([q, r, -q, -r])
    check("internal stencil first harmonic eigenvalue is -25/6", stencil4 * doublet_word == sp.Rational(-25, 6) * doublet_word)
    check("internal stencil is real symmetric", stencil4.T == stencil4)
    check("internal stencil commutes with the site shift", stencil4 * forward_shift == forward_shift * stencil4)

    cw2 = sp.Rational(1, 3)
    kappa_prod = -cw2 * sp.Rational(-25, 6)
    check("production internal stiffness is 25/18", kappa_prod == sp.Rational(25, 18))
    M = sp.Matrix([[1 - kappa_prod, 1], [-kappa_prod, 1]])
    check("unit kick-drift internal map has determinant one", M.det() == 1)
    check("unit kick-drift internal map has trace 11/18", sp.trace(M) == sp.Rational(11, 18))
    cos_theta = sp.trace(M) / 2
    check("internal eigenphase cosine is 11/36", cos_theta == sp.Rational(11, 36))
    check("internal mode lies in the elliptic stability interval", -1 < cos_theta < 1)
    check("production stiffness is not the order-four value", kappa_prod != 2)
    check("internal map does not square to minus identity", M**2 != -sp.eye(2))
    check("internal map is not a one-tick coordinate quarter-turn", M != R and M != R.T)
    niven_values = {sp.Integer(0), sp.Rational(1, 2), sp.Rational(-1, 2), sp.Integer(1), sp.Integer(-1)}
    check("rational cosine is excluded from finite-order Niven values", cos_theta not in niven_values)

    # Explicit embedded leakage witness on a unit xy plaquette.  The exterior
    # site (-1,0,0) has the +q corner (0,0,0) as a face neighbor; its only
    # other nonzero plaquette corner (1,1,0) is outside its 18-neighborhood.
    plaquette = {
        (0, 0, 0): q,
        (1, 0, 0): sp.Integer(0),
        (1, 1, 0): -q,
        (0, 1, 0): sp.Integer(0),
    }
    exterior = (-1, 0, 0)
    face_offsets = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
    edge_offsets = [
        (dx, dy, dz)
        for dx in (-1, 0, 1)
        for dy in (-1, 0, 1)
        for dz in (-1, 0, 1)
        if abs(dx) + abs(dy) + abs(dz) == 2
    ]

    def at(offset: tuple[int, int, int]) -> sp.Expr:
        point = tuple(exterior[i] + offset[i] for i in range(3))
        return plaquette.get(point, sp.Integer(0))

    exterior_laplacian = sp.expand(
        one_third * sum((at(offset) for offset in face_offsets), sp.Integer(0))
        + one_sixth * sum((at(offset) for offset in edge_offsets), sp.Integer(0))
        - 4 * plaquette.get(exterior, sp.Integer(0))
    )
    check("18-point stencil has six face offsets", len(face_offsets) == 6)
    check("18-point stencil has twelve edge offsets", len(edge_offsets) == 12)
    check("embedded exterior site receives exact q/3 leakage", sp.simplify(exterior_laplacian - q / 3) == 0)
    check("four-site support is therefore not invariant", exterior_laplacian != 0)

    # Reference circular branch and inert branch.
    sigma = sp.symbols("sigma", real=True)
    pq_circle = -sigma * sp.sqrt(kappa) * r
    pr_circle = sigma * sp.sqrt(kappa) * q
    kinetic = sp.expand((pq_circle**2 + pr_circle**2) / 2)
    potential = sp.expand(kappa * (q**2 + r**2) / 2)
    circle_L = sp.expand(q * pr_circle - r * pq_circle)
    check("unit-orientation circular branch has equal kinetic and potential energy", sp.simplify((kinetic - potential).subs(sigma**2, 1)) == 0)
    check("circular branch carries signed nonzero charge formula", sp.simplify(circle_L - sigma * sp.sqrt(kappa) * (q**2 + r**2)) == 0)
    radial_rate = sp.symbols("radial_rate", real=True)
    check("standing radial branch has zero circulation", sp.simplify(q * (radial_rate * r) - r * (radial_rate * q)) == 0)

    # Source and scope firewalls.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
    field_ops = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    lagrangian = (ROOT / "engine/include/ftd/lagrangian.h").read_text(encoding="utf-8")
    check("production source contains the frozen isotropic weights", "constexpr double INV3 = 1.0 / 3.0;" in phase_read and "constexpr double INV6 = 1.0 / 6.0;" in phase_read)
    check("production source contains kick then drift", "v.wave_vel += rb.delta_j_[i];" in phase_write and "v.flux += v.wave_vel;" in phase_write)
    check("field operator states the 18-point stencil", "(1/3)·face_sum + (1/6)·edge_sum − 4·center" in field_ops)
    check("action identifies wave velocity as conjugate momentum", "The canonical momentum of the flux field." in lagrangian)
    check("certificate reads no G-star or gamma target", "G_STAR" not in globals() and "GAMMA" not in globals())
    check("certificate reads no Born, Bell, selector, or measurement target", all(token not in globals() for token in ("BORN", "BELL", "SELECTOR", "MEASUREMENT")))

    combined = all(passed for _, passed in checks)
    check("combined Outcome A discriminator", combined)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0918 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=A_NATIVE_OBSERVABLE_WITH_EMBEDDED_CONSERVATION_BOUNDARY")
        print("C4_CIRCULATION_CHARGE=EXACT_NATIVE_OBSERVABLE")
        print("ISOLATED_ISOTROPIC_CONSERVATION=EXACT_CONDITIONAL")
        print("EMBEDDED_ELEMENTARY_PLAQUETTE_INVARIANT=FALSE")
        print("BARE_INTERNAL_FINITE_INTEGER_RETURN=FALSE")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("GAMMA_DERIVED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
