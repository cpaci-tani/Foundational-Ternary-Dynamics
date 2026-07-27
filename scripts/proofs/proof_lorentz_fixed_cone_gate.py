#!/usr/bin/env python3
"""Exact fixed-cone gate for P4-local Lorentz improvement (FTD-0409).

No numerical or physical-constant search is performed.  The target is frozen
to the live engine cone c^2=1/3.  The script proves that scalar period-two and
period-three nearest-Moore kick sequences cannot combine that cone with exact
q^4 cancellation and full production-band stability.  It also closes the
minimal positive-Hermitian one-auxiliary oscillator route.

For period four it constructs an exact stable trace polynomial with the right
infrared germ, then proves that this particular c3=0 endpoint-saturating target
cannot be factored into four real scalar kicks.  General period-four transfer
maps and multi-state paraunitary maps remain open.
"""

from __future__ import annotations

from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def bernstein_coefficients_on_subboxes(poly: sp.Expr, x: sp.Symbol,
                                        subdivisions: int) -> list[sp.Expr]:
    """Degree-preserving exact Bernstein coefficients on equal subintervals."""
    degree = sp.Poly(poly, x).degree()
    t = sp.symbols("t", real=True)
    coefficients: list[sp.Expr] = []
    for index in range(subdivisions):
        lower = sp.Rational(index, subdivisions)
        upper = sp.Rational(index + 1, subdivisions)
        mapped = sp.Poly(sp.expand(poly.subs(x, lower + (upper - lower) * t)), t)
        power = [mapped.coeff_monomial(t**j) for j in range(degree + 1)]
        for i in range(degree + 1):
            coefficients.append(sp.simplify(sum(
                power[j] * sp.binomial(i, j) / sp.binomial(degree, j)
                for j in range(i + 1)
            )))
    return coefficients


def main() -> None:
    checks = 0
    M = sp.symbols("M", real=True, nonnegative=True)
    mmax = sp.Rational(16, 3)
    v2 = sp.Rational(1, 3)

    # ------------------------------------------------------------------
    # Period two: exact fixed-cone failure.
    # ------------------------------------------------------------------
    k0, k1 = sp.symbols("k0 k1", real=True)
    transfer = lambda k: sp.Matrix([[2 - k * M, -1], [1, 0]])
    p2 = transfer(k1) * transfer(k0)
    A = (k0 + k1) / 2
    B = k0 * k1 / 4
    X = sp.expand(A * M - B * M**2)
    require(sp.factor(p2.det()) == 1
            and sp.simplify(p2.trace() / 2 - (1 - 2 * X)) == 0,
            "P2-1 exact period-two pole is sin^2(theta)=AM-BM^2")
    checks += 1

    b_cancel = sp.simplify(v2 * (4 * v2 - 1) / 12)
    require(b_cancel == sp.Rational(1, 108),
            "P2-2 fixed c^2=1/3 cancellation requires B=1/108")
    checks += 1
    x_fixed = sp.expand(v2 * M - b_cancel * M**2)
    require(x_fixed.subs(M, mmax) == sp.Rational(368, 243) > 1,
            "P2-3 fixed-cone period two is unstable at M18,max")
    checks += 1

    # ------------------------------------------------------------------
    # Period three: a no-go independent of kick reality/sign.
    # ------------------------------------------------------------------
    ka, kb, kc = sp.symbols("ka kb kc", real=True)
    p3 = transfer(kc) * transfer(kb) * transfer(ka)
    s1 = ka + kb + kc
    s2 = ka * kb + ka * kc + kb * kc
    s3 = ka * kb * kc
    expected_trace3 = 1 - sp.Rational(3, 2) * s1 * M + s2 * M**2 - s3 * M**3 / 2
    require(sp.simplify(p3.trace() / 2 - expected_trace3) == 0,
            "P3-1 exact period-three Floquet discriminant is cubic in M")
    checks += 1

    sumsq = ka**2 + kb**2 + kc**2
    beta3 = sp.simplify((3 * sumsq - 2 * s2) / 36)
    beta3_fixed_sum = sp.simplify(beta3.subs(kc, 1 - ka - kb))
    sumsq_fixed_sum = sp.expand(sumsq.subs(kc, 1 - ka - kb))
    require(sp.simplify(beta3_fixed_sum - (4 * sumsq_fixed_sum - 1) / 36) == 0,
            "P3-2 at mean c^2=1/3 the M^2 phase coefficient is (4 sum(k_i^2)-1)/36")
    checks += 1
    require(sp.solve(sp.Eq((4 * sp.Symbol("Q") - 1) / 36,
                              v2 / 12), sp.Symbol("Q")) == [sp.Rational(1, 2)],
            "P3-3 q^4 cancellation forces sum(k_i^2)=1/2")
    checks += 1
    require(sp.simplify((1 - sp.Rational(1, 2)) / 2) == sp.Rational(1, 4),
            "P3-4 fixed sum and square norm force pair sum s2=1/4")
    checks += 1

    product = sp.symbols("p", real=True)
    c3_fixed = 1 - sp.Rational(3, 2) * M + sp.Rational(1, 4) * M**2 - product * M**3 / 2
    require(c3_fixed.subs(M, 3) == -sp.Rational(5, 4) - sp.Rational(27, 2) * product,
            "P3-5 stability at M=3 requires p<=-1/54")
    checks += 1
    require(c3_fixed.subs(M, mmax) == sp.Rational(1, 9) - sp.Rational(2048, 27) * product,
            "P3-6 stability at M=16/3 requires p>=-3/256")
    checks += 1
    require(-sp.Rational(3, 256) > -sp.Rational(1, 54),
            "P3-7 the two necessary stability intervals are disjoint")
    checks += 1

    # ------------------------------------------------------------------
    # Minimal positive-Hermitian one-auxiliary oscillator no-go.
    # ------------------------------------------------------------------
    a, g, mu, c, ell2 = sp.symbols("a g mu c ell2", real=True)
    lam = a * M + ell2 * M**2
    stiffness_det = sp.expand((a * M - lam) * (mu + c * M - lam) - g**2 * M**2)
    require(sp.expand(stiffness_det).coeff(M, 2) == -ell2 * mu - g**2,
            "AUX-1 acoustic eigenvalue curvature is ell2=-g^2/mu")
    checks += 1
    required_ell2 = sp.simplify(v2 * (1 - v2) / 12)
    require(required_ell2 == sp.Rational(1, 54) > 0,
            "AUX-2 fixed-cone centered time requires positive stiffness curvature +1/54")
    checks += 1
    g_pos, mu_pos = sp.symbols("g_pos mu_pos", positive=True)
    require(-g_pos**2 / mu_pos < 0,
            "AUX-3 a real coupling to a positive-gap Hermitian auxiliary has the wrong curvature sign")
    checks += 1

    # ------------------------------------------------------------------
    # Scalar positive-link moment obstruction at the fixed cone.
    # ------------------------------------------------------------------
    require(sp.simplify(v2 - v2**2) == sp.Rational(2, 9),
            "LINK-1 Moore displacements obey d_i^4=d_i^2 but c^4!=c^2 at c^2=1/3")
    checks += 1
    link_moment = sp.symbols("link_moment", real=True)
    require(sp.solve([sp.Eq(link_moment, v2),
                      sp.Eq(link_moment, v2**2)], [link_moment]) == [],
        "LINK-2 a positive scalar link average cannot match both second and fourth rotational moments",
    )
    checks += 1

    # ------------------------------------------------------------------
    # Period-four boundary: stable trace exists, this natural factorization fails.
    # ------------------------------------------------------------------
    # If theta^2=(1/3)M+(1/36)M^2+..., then
    # cos(4theta)=1-(8/3)M+(26/27)M^2+... .  Set c3=0 and choose c4 so the
    # production endpoint saturates C4(16/3)=-1.
    c4 = -sp.Rational(1843, 98304)
    trace4_target = (
        1 - sp.Rational(8, 3) * M + sp.Rational(26, 27) * M**2 + c4 * M**4
    )
    require(trace4_target.subs(M, mmax) == -1,
            "P4-1 exact degree-four target saturates C4(Mmax)=-1")
    checks += 1

    theta_a, theta_b = sp.symbols("theta_a theta_b")
    cosine_germ = sp.expand(
        1 - 8 * (theta_a * M + theta_b * M**2)
        + sp.Rational(32, 3) * (theta_a * M + theta_b * M**2) ** 2
    )
    require(cosine_germ.coeff(M, 1).subs(theta_a, v2) == -sp.Rational(8, 3)
            and cosine_germ.coeff(M, 2).subs({theta_a: v2, theta_b: v2 / 12})
            == sp.Rational(26, 27),
            "P4-2 target has the fixed c^2=1/3 dimension-six-free infrared germ")
    checks += 1

    x = sp.symbols("x", real=True)
    trace4_x = sp.factor(trace4_target.subs(M, sp.Rational(16, 3) * x))
    positive_minus = 1843 * x**3 - 3328 * x + 1728
    positive_plus = 1843 * x**3 + 1843 * x**2 - 1485 * x + 243
    require(sp.factor(1 - trace4_x) == 2 * x * positive_minus / 243,
            "P4-3 upper stability gap factors as x times a cubic")
    checks += 1
    require(sp.simplify(1 + trace4_x
                        - 2 * (1 - x) * positive_plus / 243) == 0,
            "P4-4 lower stability gap factors as (1-x) times a cubic")
    checks += 1
    minus_bernstein = bernstein_coefficients_on_subboxes(positive_minus, x, 8)
    plus_bernstein = bernstein_coefficients_on_subboxes(positive_plus, x, 8)
    require(min(minus_bernstein) == sp.Rational(167, 384) > 0,
            "P4-5 exact Bernstein certificate proves the upper gap nonnegative")
    checks += 1
    require(min(plus_bernstein) == sp.Rational(2555, 384) > 0,
            "P4-6 exact Bernstein certificate proves the lower gap nonnegative")
    checks += 1

    # For four real scalar kicks, group opposite sites in the temporal cell:
    # a=k1+k3, b=k2+k4, p=k1*k3, q=k2*k4.  The c3=0 target requires
    # aq+bp=0, while pq=2*c4=-K.  If p<0<q, reality of the positive-product
    # pair gives q<=b^2/4.  Setting r=a/b=(-p)/q>0 then bounds
    # K=r*q^2 <= 16r/[81(1+r)^4] <= 1/48.  The target has K>1/48.
    K = -2 * c4
    r = sp.symbols("r", positive=True)
    envelope = 16 * r / (81 * (1 + r) ** 4)
    require(K == sp.Rational(1843, 49152) > sp.Rational(1, 48),
            "P4-7 target scalar-kick product magnitude exceeds 1/48")
    checks += 1
    require(sp.simplify(sp.diff(envelope, r)
                        + 16 * (3 * r - 1) / (81 * (r + 1) ** 5)) == 0,
            "P4-8 real-pair envelope is maximized at r=1/3")
    checks += 1
    require(envelope.subs(r, sp.Rational(1, 3)) == sp.Rational(1, 48),
            "P4-9 no four-real-kick factorization reaches the target product")
    checks += 1

    # Source/status contract: FTD-0408 remains default off and its selected
    # speed is not silently changed by this mathematical gate.
    header = read("engine/include/ftd/lorentz_period2.h")
    toggles = read("engine/include/ftd/term_toggles.h")
    require("LORENTZ_PERIOD2_EFFECTIVE_C2 = 1.0 / 13.0" in header
            and "bool lorentz_period2_floquet = false" in toggles,
            "SRC-1 FTD-0409 leaves the FTD-0408 prototype and production defaults unchanged")
    checks += 1

    print()
    print(f"RESULT  {checks}/{checks} exact/source-contract checks passed")
    print("FIXED TARGET c^2=1/3, theta^2=S2/3+O(q^6), P4, full-band stability")
    print("PERIOD 2    CLOSED: X(16/3)=368/243>1")
    print("PERIOD 3    CLOSED: p<=-1/54 and p>=-3/256 are incompatible")
    print("ONE AUX     CLOSED for positive-gap Hermitian stiffness mixing")
    print("PERIOD 4    stable trace target exists; selected c3=0 witness has no real-kick factorization")
    print("OPEN        general period-4 scalar cell or multi-state paraunitary transfer")
    print("VERDICT     MINIMAL FIXED-CONE CLASSES CLOSED; COMMON CONE NOT RECOVERED")


if __name__ == "__main__":
    main()
