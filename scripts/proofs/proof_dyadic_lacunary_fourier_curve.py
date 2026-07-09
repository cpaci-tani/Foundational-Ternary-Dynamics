"""
proof_dyadic_lacunary_fourier_curve.py
======================================

Exact and high-precision checks for the four-mode dyadic lacunary Fourier
seed curve documented in:

    docs/theory/09_mathematical/general_math/
        EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md

The curve is

    x(t) = cos t + (1/2)cos 2t + (1/2)cos 4t + (3/8)cos 8t
    y(t) = 2sin t - sin 2t + sin 4t - (3/4)sin 8t

What this verifies
------------------
1. The hidden Fibonacci velocity spine:

       2^k a_k = (1, 1, 2, 3) = F_{k+1},  k = 0..3.

   This is an exact low-complexity description of the seed coefficients,
   not a search result and not an FTD derivation.

2. The uniform 3:1 chirality ledger:

       c_+ = (a_k + b_k)/2,  c_- = (a_k - b_k)/2,
       b_k = 2(-1)^k a_k.

   Each mode has dominant/subdominant rotating amplitudes in magnitude ratio
   3:1, with dominant orientation alternating by k.

3. Chebyshev reduction, implicit degree, regularity resultant, area, centroid,
   and turning number.

4. The trigonal node relay:

       roots r of P(r)=0 on the x-axis generate the off-axis nodes by
       u_\u00b1 = cos(theta \u00b1 2*pi/3), with r = cos(theta).

   This is verified algebraically using the symmetric variables
   s = u + v and p = uv.

5. The Fibonacci-continuation warning:

       a_k = F_{k+1}/2^k  implies  a_k ~ (phi/2)^k,

   so the infinite continuation is continuous but derivative-rough and
   signed-area-unstable. This supports only the finite-readout interpretation.

6. The geometric-tail Weierstrass thresholds:

       derivative term ratio:       2*lambda
       signed-area term ratio:      2*lambda^2
       m-th energy term ratio:      4^m*lambda^2
       rough-regime Holder index:   H = -log(lambda)/log(2)

   The image-dimension upper bound from H-Holder regularity is
   min(2, 1/H), hitting the plane cap at lambda = 1/sqrt(2), the same
   threshold at which signed area stops converging absolutely.

What this is NOT
----------------
- NOT a numerical near-miss search.
- NOT an alpha route.
- NOT a derivation of N_c, generations, or Moore structure.
- NOT a new LEDGER claim.
"""

from __future__ import annotations

from fractions import Fraction
import math

import numpy as np
import sympy as sp


A = [Fraction(1, 1), Fraction(1, 2), Fraction(1, 2), Fraction(3, 8)]
B = [2 * ((-1) ** k) * a for k, a in enumerate(A)]


def fib(n: int) -> int:
    if n <= 0:
        raise ValueError("Fibonacci index must be positive")
    f0, f1 = 0, 1
    for _ in range(n):
        f0, f1 = f1, f0 + f1
    return f0


def check_fibonacci_velocity_spine() -> None:
    q = [(1 << k) * a for k, a in enumerate(A)]
    expected = [Fraction(fib(k + 1), 1) for k in range(len(A))]
    assert q == expected, f"velocity spine mismatch: got {q}, expected {expected}"


def check_chirality_ledger() -> None:
    signed_area_over_pi = Fraction(0, 1)
    for k, (a, b) in enumerate(zip(A, B)):
        n = 1 << k
        c_plus = (a + b) / 2
        c_minus = (a - b) / 2

        # Each mode has a 3:1 dominant/subdominant magnitude ratio.
        mags = sorted([abs(c_plus), abs(c_minus)])
        assert mags[1] == 3 * mags[0], (
            f"mode {n}: chirality ratio is not 3:1; "
            f"c_plus={c_plus}, c_minus={c_minus}"
        )

        # Dominant orientation alternates: even k forward, odd k backward.
        if k % 2 == 0:
            assert abs(c_plus) > abs(c_minus), f"mode {n}: expected forward dominance"
        else:
            assert abs(c_minus) > abs(c_plus), f"mode {n}: expected backward dominance"

        # The complex-coefficient area formula equals the sine/cosine formula.
        area_from_complex = n * (c_plus * c_plus - c_minus * c_minus)
        area_from_fourier = n * a * b
        assert area_from_complex == area_from_fourier, (
            f"mode {n}: area formulas disagree: "
            f"{area_from_complex} vs {area_from_fourier}"
        )
        signed_area_over_pi += area_from_fourier

    assert signed_area_over_pi == Fraction(3, 4), (
        f"area/pi mismatch: got {signed_area_over_pi}, expected 3/4"
    )


def check_chebyshev_and_resultants() -> None:
    u, x, y = sp.symbols("u x y")

    X = 48 * u**8 - 96 * u**6 + 64 * u**4 - 15 * u**2 + u + sp.Rational(3, 8)
    P = -96 * u**7 + 144 * u**5 - 52 * u**3 + 2

    # Reconstruct X from the cosine Chebyshev ladder.
    X_from_cheb = (
        u
        + sp.Rational(1, 2) * (2 * u**2 - 1)
        + sp.Rational(1, 2) * (8 * u**4 - 8 * u**2 + 1)
        + sp.Rational(3, 8) * (128 * u**8 - 256 * u**6 + 160 * u**4 - 32 * u**2 + 1)
    )
    assert sp.expand(X_from_cheb - X) == 0, "Chebyshev X(u) reduction failed"

    # y/sin(t) = 2 - 2U_1 + 2U_3 - (3/2)U_7.
    U1 = 2 * u
    U3 = 8 * u**3 - 4 * u
    U7 = 128 * u**7 - 192 * u**5 + 80 * u**3 - 8 * u
    P_from_cheb = 2 - U1 + U3 - sp.Rational(3, 4) * U7
    assert sp.expand(P_from_cheb - P) == 0, "Chebyshev P(u) reduction failed"

    # Implicit image degree.
    F = sp.resultant(x - X, y**2 - (1 - u**2) * P**2, u)
    poly = sp.Poly(F, x, y)
    assert poly.total_degree() == 16, f"implicit degree {poly.total_degree()} != 16"

    # Regularity resultant.
    R = 384 * u**7 - 576 * u**5 + 256 * u**3 - 30 * u + 1
    S = -768 * u**8 + 1536 * u**6 - 928 * u**4 + 156 * u**2 + 2 * u
    regularity_resultant = sp.resultant(R, S, u)
    assert regularity_resultant == 60514543933804184076288, (
        f"regularity resultant changed: {regularity_resultant}"
    )


def check_area_centroid_exact() -> None:
    t = sp.symbols("t", real=True)
    x = sp.cos(t) + sp.Rational(1, 2) * sp.cos(2 * t) + sp.Rational(1, 2) * sp.cos(4 * t) + sp.Rational(3, 8) * sp.cos(8 * t)
    y = 2 * sp.sin(t) - sp.sin(2 * t) + sp.sin(4 * t) - sp.Rational(3, 4) * sp.sin(8 * t)
    xp = sp.diff(x, t)
    yp = sp.diff(y, t)

    area = sp.simplify(sp.Rational(1, 2) * sp.integrate(x * yp - y * xp, (t, 0, 2 * sp.pi)))
    assert area == sp.Rational(3, 4) * sp.pi, f"area mismatch: {area}"

    x_cent_num = sp.simplify(sp.Rational(1, 2) * sp.integrate(x**2 * yp, (t, 0, 2 * sp.pi)))
    y_cent_num = sp.simplify(-sp.Rational(1, 2) * sp.integrate(y**2 * xp, (t, 0, 2 * sp.pi)))
    assert x_cent_num == 0 and y_cent_num == 0, (
        f"centroid numerators are not zero: {x_cent_num}, {y_cent_num}"
    )


def check_turning_number_numeric() -> None:
    # Polynomial from w^8 z'(t), up to non-zero scalar:
    # -1/2(3w^16 - 6w^12 + w^10 - 3w^9 - w^7 + 3w^6 - 2w^4 + 9)
    coeffs = [
        -1.5, 0, 0, 0, 3.0, 0, -0.5, 1.5,
        0, 0.5, -1.5, 0, 1.0, 0, 0, 0, -4.5,
    ]
    roots = np.roots(coeffs)
    moduli = np.array([abs(r) for r in roots])
    inside = int(np.sum(moduli < 1.0))
    margin = min(1.0 - np.max(moduli[moduli < 1.0]), np.min(moduli[moduli > 1.0]) - 1.0)

    assert inside == 6, f"expected 6 zeros inside unit disk, got {inside}"
    assert margin > 1e-3, f"unit-circle separation too small for stable count: {margin}"
    turn = inside - 8
    assert turn == -2, f"turning number mismatch: {turn}"


def check_trigonal_node_relay() -> None:
    u, v, s, p = sp.symbols("u v s p")

    def X(z):
        return 48 * z**8 - 96 * z**6 + 64 * z**4 - 15 * z**2 + z + sp.Rational(3, 8)

    def P(z):
        return -96 * z**7 + 144 * z**5 - 52 * z**3 + 2

    def Q(z):
        return (1 - z**2) * P(z) ** 2

    # Axis branch-collapse points are exactly P(u)=0, y=0. For this seed
    # there are three such real roots in (0,1).
    axis_poly = sp.Poly(P(u), u)
    assert axis_poly.count_roots(0, 1) == 3, "expected three positive axis roots"
    assert axis_poly.count_roots(-1, 1) == 3, "expected exactly three axis roots in (-1,1)"

    # Off-axis overlaps have distinct u,v with X(u)=X(v), Q(u)=Q(v).
    # Divide out the diagonal u=v and rewrite in s=u+v, p=uv.
    A_uv = sp.Poly(X(u) - X(v), u).div(sp.Poly(u - v, u))[0].as_expr()
    B_uv = sp.Poly(Q(u) - Q(v), u).div(sp.Poly(u - v, u))[0].as_expr()

    G = sp.groebner([s - u - v, p - u * v], u, v, s, p, order="lex")

    def symmetric_reduction(expr):
        return sp.factor(G.reduce(sp.expand(expr))[1])

    A_sp = symmetric_reduction(A_uv)
    B_sp = symmetric_reduction(B_uv)

    # The 120-degree phase relay is p = s^2 - 3/4. Under this substitution,
    # the first overlap equation becomes H(s)=P(-s)/2, while the second is
    # automatically divisible by the same H(s).
    H = 48 * s**7 - 72 * s**5 + 26 * s**3 + 1
    p_relay = s**2 - sp.Rational(3, 4)
    A_relay = sp.factor(A_sp.subs(p, p_relay))
    B_relay = sp.factor(B_sp.subs(p, p_relay))

    assert A_relay == H, f"relay A equation mismatch: {A_relay}"
    quotient, remainder = sp.div(B_relay, H)
    assert remainder == 0, f"relay B equation not divisible by H: remainder {remainder}"

    # H(s)=P(-s)/2. Thus every axis root r of P(r)=0 gives s=-r and
    # p=r^2-3/4 for an off-axis pair.
    assert sp.expand(H - P(-s) / 2) == 0, "H(s) != P(-s)/2"

    # Trigonometric interpretation: if r=cos(theta), then
    # cos(theta+2pi/3)+cos(theta-2pi/3)=-r and product=r^2-3/4.
    theta = sp.symbols("theta")
    r = sp.cos(theta)
    u_plus = sp.cos(theta + 2 * sp.pi / 3)
    u_minus = sp.cos(theta - 2 * sp.pi / 3)
    assert sp.trigsimp(u_plus + u_minus + r) == 0
    assert sp.trigsimp(u_plus * u_minus - (r**2 - sp.Rational(3, 4))) == 0


def check_fibonacci_continuation_warning() -> None:
    phi = (1 + sp.sqrt(5)) / 2
    lambda_eff = phi / 2

    # Continuity threshold lambda < 1 survives.
    assert bool(lambda_eff < 1), "Fibonacci continuation should be position-continuous"

    # Derivative threshold lambda < 1/2 fails.
    assert bool(lambda_eff > sp.Rational(1, 2)), "Fibonacci continuation should fail derivative control"

    # Signed-area absolute convergence threshold lambda < 1/sqrt(2) fails.
    assert bool(lambda_eff > 1 / sp.sqrt(2)), "Fibonacci continuation should fail area control"

    # The modal area terms F_{k+1}^2 / 2^k do not tend to zero because
    # the asymptotic ratio is phi^2 / 2 > 1.
    assert bool(phi**2 / 2 > 1), "Fibonacci area terms should grow in magnitude"


def check_weierstrass_thresholds() -> None:
    lam = sp.symbols("lam", positive=True)
    m = sp.symbols("m", integer=True, positive=True)

    derivative_ratio = 2 * lam
    area_ratio = 2 * lam**2
    energy_ratio_m = 2 ** (2 * m) * lam**2

    assert sp.solve_univariate_inequality(derivative_ratio < 1, lam) == (lam < sp.Rational(1, 2))
    assert sp.solve_univariate_inequality(area_ratio < 1, lam) == (lam < 1 / sp.sqrt(2))
    assert sp.simplify(energy_ratio_m.subs(lam, 2 ** (-m)) - 1) == 0

    H_at_area_threshold = sp.simplify(-sp.log(1 / sp.sqrt(2)) / sp.log(2))
    assert H_at_area_threshold == sp.Rational(1, 2), (
        f"area threshold should correspond to Holder H=1/2, got {H_at_area_threshold}"
    )

    # The rough-regime Holder index is defined by lambda = 2^{-H}.
    H = -sp.log(lam) / sp.log(2)
    assert sp.simplify(2 ** (-H) - lam) == 0

    # Image dimension upper bound from an H-Holder parameterized curve is
    # min(2, 1/H). The cap begins exactly at H=1/2, i.e. lambda=1/sqrt(2).
    assert sp.simplify((1 / H_at_area_threshold) - 2) == 0


def main() -> None:
    checks = [
        ("Fibonacci velocity spine", check_fibonacci_velocity_spine),
        ("3:1 chirality ledger and signed area", check_chirality_ledger),
        ("Chebyshev reduction, degree, regularity resultant", check_chebyshev_and_resultants),
        ("Exact area and centroid", check_area_centroid_exact),
        ("Turning number", check_turning_number_numeric),
        ("Trigonal node relay", check_trigonal_node_relay),
        ("Fibonacci continuation warning", check_fibonacci_continuation_warning),
        ("Weierstrass thresholds", check_weierstrass_thresholds),
    ]

    print("Dyadic lacunary Fourier curve C3 probe")
    print("=" * 52)
    for name, fn in checks:
        fn()
        print(f"PASS - {name}")
    print("=" * 52)
    print("OK - exact curve facts verified. No FTD physics claim promoted.")


if __name__ == "__main__":
    main()
