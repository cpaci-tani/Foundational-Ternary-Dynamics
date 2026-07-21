"""
proof_dyadic_octave_bifurcation_atlas.py
==========================================

Exact bifurcation atlas for the one-parameter octave-8 slice of the dyadic
lacunary Fourier family:

    a = (1, 1/2, 1/2, q),   beta = 2.

The script proves algebraic statements about axis branch seeds, their
transversality, and regularity thresholds. It uses resultants, Groebner bases,
and Sturm root counts only. It does not run a numerical search and makes no
FTD physics claim.
"""

from __future__ import annotations

import sympy as sp


u, q, c = sp.symbols("u q c", real=True)


X = (
    u
    + sp.Rational(1, 2) * sp.chebyshevt(2, u)
    + sp.Rational(1, 2) * sp.chebyshevt(4, u)
    + q * sp.chebyshevt(8, u)
)
P = -256 * q * u**7 + 384 * q * u**5 + (8 - 160 * q) * u**3 + (16 * q - 6) * u + 2
X_prime = sp.diff(X, u)
Y = sp.factor(u * P - (1 - u**2) * sp.diff(P, u))

Q_AXIS = (
    4194304 * q**8
    + 1572864 * q**7
    - 11304960 * q**6
    - 3051520 * q**5
    + 2763648 * q**4
    + 1024848 * q**3
    + 72225 * q**2
    + 1188 * q
    - 108
)
T_TANGENCY = 21 * q**2 - 28 * q + 4
G_REGULARITY = 128 * q**3 + 16 * q**2 - 18 * q - 3
J_CUBIC_IMAGE = 1024 * c**3 - 64 * c**2 - 40 * c + 1

AXIS_ROOT_INTERVALS = [
    (sp.Rational(-299, 183), sp.Rational(-1013, 620)),
    (sp.Rational(-38, 89), sp.Rational(-415, 972)),
    (sp.Rational(-110, 359), sp.Rational(-91, 297)),
    (sp.Rational(4, 147), sp.Rational(15, 551)),
    (sp.Rational(239, 414), sp.Rational(295, 511)),
    (sp.Rational(2806, 1871), sp.Rational(2809, 1873)),
]
REGULARITY_ROOT_INTERVALS = [
    (sp.Rational(-43, 128), sp.Rational(-173, 515)),
    (sp.Rational(-199, 1112), sp.Rational(-17, 95)),
    (sp.Rational(154, 395), sp.Rational(131, 336)),
]


def count_axis_roots(value: sp.Rational) -> int:
    """Count roots of P_q in the physical cosine interval (-1, 1)."""
    return sp.Poly(P.subs(q, value), u).count_roots(-1, 1)


def check_chebyshev_reduction() -> None:
    """Reduce y_q(t)/sin(t) to the displayed odd branch polynomial P_q."""
    coeffs = [sp.Rational(1), sp.Rational(1, 2), sp.Rational(1, 2), q]
    from_modes = 2 * sum(((-1) ** k) * coeffs[k] * sp.chebyshevu(2**k - 1, u) for k in range(4))
    assert sp.expand(from_modes - P) == 0
    assert sp.expand(X) == (
        128 * q * u**8 - 256 * q * u**6 + 160 * q * u**4 - 32 * q * u**2 + q
        + 4 * u**4 - 3 * u**2 + u
    )
    assert sp.simplify(P.subs(u, -1) - 16 * q) == 0
    assert sp.simplify(P.subs(u, 1) + 4 * (4 * q - 1)) == 0


def check_axis_discriminant_and_sturm_atlas() -> None:
    """Certify the axis-seed discriminant and root-count chambers."""
    resultant = sp.factor(sp.resultant(P, sp.diff(P, u), u))
    scale = sp.factor(resultant / (q**5 * Q_AXIS))
    assert scale.is_Integer and scale != 0
    assert sp.gcd(Q_AXIS, sp.diff(Q_AXIS, q)) == 1
    assert sp.Poly(Q_AXIS, q).count_roots(-sp.oo, sp.oo) == 6
    for left, right in AXIS_ROOT_INTERVALS:
        assert sp.Poly(Q_AXIS, q).count_roots(left, right) == 1

    # One exact rational representative from each chamber cut out by the six
    # real roots of Q_AXIS together with the endpoint events q=0 and q=1/4.
    chambers = [
        (sp.Rational(-2), 7),
        (sp.Rational(-1), 5),
        (sp.Rational(-3, 8), 3),
        (sp.Rational(-1, 10), 1),
        (sp.Rational(1, 100), 2),
        (sp.Rational(1, 8), 2),
        (sp.Rational(3, 8), 3),
        (sp.Rational(1), 5),
        (sp.Rational(2), 7),
    ]
    for value, expected in chambers:
        assert count_axis_roots(value) == expected

    assert sp.expand(P.subs(q, 0) - 2 * (u + 1) * (2 * u - 1) ** 2) == 0
    assert sp.expand(P.subs(q, sp.Rational(1, 4)) + 2 * (u - 1) * (
        32 * u**6 + 32 * u**5 - 16 * u**4 - 16 * u**3 + 1
    )) == 0


def check_axis_transversality_thresholds() -> None:
    """Find exactly where an axis coincidence has a vertical common tangent."""
    resultant = sp.factor(sp.resultant(P, X_prime, u))
    scale = sp.factor(resultant / (q**5 * T_TANGENCY))
    assert scale.is_Integer and scale != 0
    assert sp.gcd(Q_AXIS, T_TANGENCY) == 1

    tau_minus = (14 - 4 * sp.sqrt(7)) / 21
    tau_plus = (14 + 4 * sp.sqrt(7)) / 21
    u_minus = (sp.sqrt(7) - 1) / 4
    u_plus = -(sp.sqrt(7) + 1) / 4
    for threshold, root in [(tau_minus, u_minus), (tau_plus, u_plus)]:
        assert sp.simplify(T_TANGENCY.subs(q, threshold)) == 0
        assert sp.simplify(P.subs({q: threshold, u: root})) == 0
        assert sp.simplify(X_prime.subs({q: threshold, u: root})) == 0


def check_full_regularity_thresholds() -> None:
    """Classify every parameter at which the plane parametrization loses speed."""
    resultant = sp.factor(sp.resultant(X_prime, Y, u))
    scale = sp.factor(resultant / (q**5 * (4 * q - 1) * G_REGULARITY**3))
    assert scale.is_Integer and scale != 0
    assert sp.gcd(G_REGULARITY, sp.diff(G_REGULARITY, q)) == 1
    assert sp.Poly(G_REGULARITY, q).count_roots(-sp.oo, sp.oo) == 3
    for left, right in REGULARITY_ROOT_INTERVALS:
        assert sp.Poly(G_REGULARITY, q).count_roots(left, right) == 1

    # At the two endpoint events, an interior derivative zero is explicit.
    endpoint_cases = [(sp.Rational(0), u - sp.Rational(1, 2)), (sp.Rational(1, 4), u + sp.Rational(1, 2))]
    for value, expected_gcd in endpoint_cases:
        common = sp.gcd(
            sp.Poly(X_prime.subs(q, value), u),
            sp.Poly(Y.subs(q, value), u),
        ).monic()
        assert sp.expand(common.as_expr() - expected_gcd) == 0

    # For each real root gamma of G_REGULARITY, the interior speed-zero
    # cosine values obey a cubic with exactly three roots in (-1, 1).
    c_of_q = -4 * q**2 + q / 2 + sp.Rational(7, 16)
    assert sp.expand(sp.resultant(G_REGULARITY, c_of_q - c, q) + 16 * J_CUBIC_IMAGE) == 0
    assert sp.Poly(J_CUBIC_IMAGE, c).count_roots(-sp.Rational(1, 4), sp.Rational(1, 4)) == 3
    assert sp.Poly(J_CUBIC_IMAGE, c).count_roots(-sp.oo, sp.oo) == 3

    for gamma in sp.real_roots(G_REGULARITY):
        common = sp.gcd(
            sp.Poly(X_prime.subs(q, gamma), u, extension=gamma),
            sp.Poly(Y.subs(q, gamma), u, extension=gamma),
        ).monic()
        expected = u**3 - sp.Rational(3, 4) * u + c_of_q.subs(q, gamma)
        assert sp.simplify(common.as_expr() - expected) == 0
        assert common.count_roots(-1, 1) == 3
        assert common.count_roots(-sp.oo, sp.oo) == 3

    # The seed lies in a regular chamber.
    assert sp.factor(G_REGULARITY.subs(q, sp.Rational(3, 8))) == -sp.Rational(3, 4)


def main() -> None:
    checks = [
        ("Chebyshev axis-branch reduction", check_chebyshev_reduction),
        ("axis discriminant and Sturm chamber atlas", check_axis_discriminant_and_sturm_atlas),
        ("axis transversality thresholds", check_axis_transversality_thresholds),
        ("full regularity thresholds", check_full_regularity_thresholds),
    ]

    print("Dyadic octave-8 bifurcation atlas")
    print("=" * 60)
    for name, fn in checks:
        fn()
        print(f"PASS - {name}")
    print("=" * 60)
    print("OK - exact octave-8 control thresholds classified for this one-parameter slice.")
    print("No numerical search and no FTD physics claim promoted.")


if __name__ == "__main__":
    main()
