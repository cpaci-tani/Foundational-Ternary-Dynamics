"""
proof_dyadic_curve_singularity_budget.py
========================================

Projective singularity-budget probe for the dyadic lacunary Fourier curve C3.

Companion document:

    docs/theory/09_mathematical/general_math/
        EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md

This script verifies the algebraic budget behind the geometric-savant reading:

    a degree-16 rational plane curve has arithmetic genus 105.

For this curve the singularity defect is accounted for by:

    75 finite double-pair units + 15 at each of the two infinity cusps = 105.

What this proves/checks
----------------------
1. The homogeneous phase parametrization has degree 16 and no base point.
2. The implicit affine image has total degree 16; therefore the map is
   generically one-to-one onto its image and the normalization has genus 0.
3. The affine self-pair resultant, after removing the diagonal w=z, has
   nonzero factors of degrees 14, 28, and 108 in w. These represent
   150 ordered finite preimages, hence 75 unordered finite double-pair units.
4. The two points at infinity have local coordinates eta~t^4 and
   rho=xi-(3/16)eta^2~t^11, giving a first Puiseux lower-bound
   delta >= (4-1)(11-1)/2 = 15 at each infinity point.
5. The lower-bound budget already equals the genus defect 105, so there is
   no room for hidden extra defect: the finite double-pair units are ordinary
   in the genus-budget sense, and each infinity branch contributes exactly 15.

What this is NOT
----------------
- NOT a physics claim.
- NOT an alpha, color, or generation derivation.
- NOT a numerical near-miss search.
"""

from __future__ import annotations

import sympy as sp


w, z, q = sp.symbols("w z q")
x, y, u = sp.symbols("x y u")


def nx(var):
    """Numerator for x(w) with common denominator 16*w^8."""
    return (
        3 * (var**16 + 1)
        + 4 * (var**12 + var**4)
        + 4 * (var**10 + var**6)
        + 8 * (var**9 + var**7)
    )


def nv(var):
    """Numerator for v(w)=i*y(w) with common denominator 16*w^8."""
    return (
        -6 * (var**16 - 1)
        + 8 * (var**12 - var**4)
        - 8 * (var**10 - var**6)
        + 16 * (var**9 - var**7)
    )


def X_of_u(var):
    return 48 * var**8 - 96 * var**6 + 64 * var**4 - 15 * var**2 + var + sp.Rational(3, 8)


def P_of_u(var):
    return -96 * var**7 + 144 * var**5 - 52 * var**3 + 2


def order_at_zero(expr, var):
    poly = sp.Poly(sp.expand(expr), var)
    return min(deg[0] for deg, coeff in poly.terms() if coeff != 0)


def check_projective_parametrization() -> None:
    S, T = sp.symbols("S T")
    Xh = (
        3 * (S**16 + T**16)
        + 4 * (S**12 * T**4 + S**4 * T**12)
        + 4 * (S**10 * T**6 + S**6 * T**10)
        + 8 * (S**9 * T**7 + S**7 * T**9)
    )
    Vh = (
        -6 * (S**16 - T**16)
        + 8 * (S**12 * T**4 - S**4 * T**12)
        - 8 * (S**10 * T**6 - S**6 * T**10)
        + 16 * (S**9 * T**7 - S**7 * T**9)
    )
    Zh = 16 * S**8 * T**8

    assert sp.Poly(Xh, S, T).total_degree() == 16
    assert sp.Poly(Vh, S, T).total_degree() == 16
    assert sp.Poly(Zh, S, T).total_degree() == 16

    # Base-point check: no common zero on P^1. If S=0, Xh=3, Vh=6.
    # If T=0, Xh=3, Vh=-6. If S,T both nonzero and Zh!=0, no base point.
    assert Xh.subs({S: 0, T: 1}) == 3 and Vh.subs({S: 0, T: 1}) == 6
    assert Xh.subs({S: 1, T: 0}) == 3 and Vh.subs({S: 1, T: 0}) == -6


def check_implicit_degree_and_genus() -> None:
    X = X_of_u(u)
    P = P_of_u(u)
    # Use v=i*y, so y_original^2 = Q(u) becomes v^2 + Q(u)=0.
    Q = (1 - u**2) * P**2
    Fv = sp.resultant(x - X, y**2 + Q, u)
    total_degree = sp.Poly(Fv, x, y).total_degree()
    assert total_degree == 16, f"implicit degree mismatch: {total_degree}"

    arithmetic_genus = (16 - 1) * (16 - 2) // 2
    assert arithmetic_genus == 105


def check_affine_self_pair_budget() -> None:
    # Same affine point iff nx(w)/(16w^8)=nx(z)/(16z^8) and
    # nv(w)/(16w^8)=nv(z)/(16z^8). Remove the diagonal w=z.
    E1 = sp.expand(nx(w) * z**8 - nx(z) * w**8)
    E2 = sp.expand(nv(w) * z**8 - nv(z) * w**8)
    Q1 = sp.div(E1, w - z)[0]
    Q2 = sp.div(E2, w - z)[0]

    resultant = sp.factor(sp.resultant(Q1, Q2, z))
    factors = sp.factor_list(resultant)[1]

    # Ignore the w^150 factor: it is the projective-infinity contribution
    # caused by clearing denominators in the affine chart.
    nonzero_degrees = []
    for factor, multiplicity in factors:
        poly = sp.Poly(factor, w)
        if poly.degree() == 1 and factor == w:
            assert multiplicity == 150
            continue
        assert multiplicity == 1, f"unexpected repeated finite factor {factor}"
        assert sp.gcd(poly, sp.Poly(sp.diff(factor, w), w)).degree() == 0
        nonzero_degrees.append(poly.degree())

    assert sorted(nonzero_degrees) == [14, 28, 108], nonzero_degrees
    ordered_preimages = sum(nonzero_degrees)
    unordered_double_pair_units = ordered_preimages // 2
    assert ordered_preimages == 150
    assert unordered_double_pair_units == 75


def check_infinity_cusps() -> None:
    # Point at w=0 is [3:6:0]. Work in chart X!=0:
    # eta = V/X - 2, xi = Z/X.
    X0 = nx(w)
    V0 = nv(w)
    Z0 = 16 * w**8
    eta0 = sp.series(V0 / X0 - 2, w, 0, 35).removeO()
    xi0 = sp.series(Z0 / X0, w, 0, 35).removeO()
    rho0 = sp.series(xi0 - sp.Rational(3, 16) * eta0**2, w, 0, 35).removeO()

    assert order_at_zero(eta0, w) == 4
    assert order_at_zero(xi0, w) == 8
    assert order_at_zero(rho0, w) == 11

    # Point at w=infinity. Use q=1/w and multiply the homogeneous coordinates
    # by q^16. The point is [3:-6:0], so eta=V/X+2.
    Xinf = sp.expand(q**16 * nx(1 / q))
    Vinf = sp.expand(q**16 * nv(1 / q))
    Zinf = sp.expand(q**16 * 16 * (1 / q) ** 8)
    etainf = sp.series(Vinf / Xinf + 2, q, 0, 35).removeO()
    xiinf = sp.series(Zinf / Xinf, q, 0, 35).removeO()
    rhoinf = sp.series(xiinf - sp.Rational(3, 16) * etainf**2, q, 0, 35).removeO()

    assert order_at_zero(etainf, q) == 4
    assert order_at_zero(xiinf, q) == 8
    assert order_at_zero(rhoinf, q) == 11

    infinity_delta_lower_bound = ((4 - 1) * (11 - 1)) // 2
    assert infinity_delta_lower_bound == 15


def check_global_budget_identity() -> None:
    arithmetic_genus = (16 - 1) * (16 - 2) // 2
    finite_pair_units = 75
    infinity_units = 2 * 15
    assert finite_pair_units + infinity_units == arithmetic_genus


def main() -> None:
    checks = [
        ("projective degree-16 parametrization has no base point", check_projective_parametrization),
        ("implicit degree and arithmetic genus", check_implicit_degree_and_genus),
        ("affine self-pair budget: 75 finite units", check_affine_self_pair_budget),
        ("two infinity cusps: orders (4,11), delta lower bound 15 each", check_infinity_cusps),
        ("global singularity budget identity", check_global_budget_identity),
    ]

    print("Dyadic curve projective singularity budget")
    print("=" * 60)
    for name, fn in checks:
        fn()
        print(f"PASS - {name}")
    print("=" * 60)
    print("OK - 75 finite units + 15 + 15 at infinity = genus defect 105.")
    print("No FTD physics claim promoted.")


if __name__ == "__main__":
    main()
