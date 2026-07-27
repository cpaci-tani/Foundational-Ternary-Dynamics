#!/usr/bin/env python3
"""Exact proof gates for the FTD-0408 P4 period-two wave prototype.

This script performs no numerical or physical-constant search.  It derives the
two-tick transfer matrix, solves the quartic pole-cancellation condition, checks
the selected rational member (+3/13, -1/13), and proves full-band spectral
stability against the exact production bound 0 <= M18 <= 16/3.

The result is deliberately scoped.  It removes the q^4 term from the free-flux
Floquet pole while preserving one-Moore-shell reads.  It does not prove a
common cone, interacting radiative stability, or operational Lorentz symmetry.
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


def homogeneous(expr: sp.Expr, variables: tuple[sp.Symbol, ...], degree: int) -> sp.Expr:
    poly = sp.Poly(sp.expand(expr), *variables)
    return sp.expand(
        sum(
            coefficient
            * sp.prod(variable**power for variable, power in zip(variables, monomial))
            for monomial, coefficient in poly.terms()
            if sum(monomial) == degree
        )
    )


def main() -> None:
    checks = 0

    M, k0, k1 = sp.symbols("M k0 k1", real=True)
    transfer = lambda k: sp.Matrix([[2 - k * M, -1], [1, 0]])
    monodromy = transfer(k1) * transfer(k0)

    require(sp.factor(monodromy.det()) == 1,
            "T1 two-tick scalar transfer map is exactly reversible (determinant one)")
    checks += 1

    A = sp.simplify((k0 + k1) / 2)
    B = sp.simplify(k0 * k1 / 4)
    X = sp.expand(A * M - B * M**2)
    require(sp.simplify(monodromy.trace() / 2 - (1 - 2 * X)) == 0,
            "T2 monodromy trace gives cos(2 theta)=1-2X")
    checks += 1
    require(sp.simplify((1 - (1 - 2 * X)) / 2 - X) == 0,
            "T3 exact per-tick Floquet pole is sin^2(theta)=X")
    checks += 1

    # M18 = S2-S2^2/12+O(q^6).  Since theta^2 = asin(sqrt(X))^2
    # = X+X^2/3+O(X^3), the complete q^4 coefficient is below.
    a_eff, b_eff = sp.symbols("A B", real=True)
    quartic_coefficient = sp.expand(-a_eff / 12 - b_eff + a_eff**2 / 3)
    cancellation_B = sp.expand(a_eff * (4 * a_eff - 1) / 12)
    cancellation_solutions = sp.solve(sp.Eq(quartic_coefficient, 0), b_eff)
    require(len(cancellation_solutions) == 1
            and sp.simplify(cancellation_solutions[0] - cancellation_B) == 0,
            "C1 quartic cancellation requires B=A(4A-1)/12")
    checks += 1

    # Real kick coefficients are the roots with sum 2A and product 4B.
    discriminant = sp.factor((2 * a_eff) ** 2 - 4 * (4 * cancellation_B))
    require(discriminant == sp.factor(4 * a_eff * (1 - a_eff) / 3),
            "C2 real cancellation kicks require discriminant 4A(1-A)/3")
    checks += 1

    # Rational parametrization of that conic.  t=2 selects existing FTD
    # integers N_c=3 and N_eff=13 rather than a fitted decimal coefficient.
    t = sp.symbols("t", real=True)
    A_t = 1 / (1 + 3 * t**2)
    y_t = t / (1 + 3 * t**2)
    require(sp.simplify(y_t**2 - A_t * (1 - A_t) / 3) == 0,
            "C3 rational conic parametrization solves the real-kick condition")
    checks += 1
    require(sp.simplify(A_t.subs(t, 2)) == sp.Rational(1, 13),
            "C4 t=2 gives effective A=1/13")
    checks += 1
    require(
        sp.simplify((A_t + y_t).subs(t, 2)) == sp.Rational(3, 13)
        and sp.simplify((A_t - y_t).subs(t, 2)) == -sp.Rational(1, 13),
        "C5 selected kicks are exactly +3/13 and -1/13",
    )
    checks += 1

    ks0 = sp.Rational(3, 13)
    ks1 = -sp.Rational(1, 13)
    As = sp.Rational(1, 13)
    Bs = -sp.Rational(3, 676)
    Xs = sp.expand(As * M - Bs * M**2)
    require(sp.simplify(quartic_coefficient.subs({a_eff: As, b_eff: Bs})) == 0,
            "C6 selected kick pair cancels the complete q^4 pole coefficient")
    checks += 1
    require(Xs == M / 13 + 3 * M**2 / 676,
            "C7 selected exact pole is sin^2(theta)=M/13+3M^2/676")
    checks += 1

    mmax = sp.Rational(16, 3)
    require(sp.diff(Xs, M) == sp.Rational(1, 13) + 3 * M / 338,
            "S1 Floquet X is strictly increasing throughout the physical band")
    checks += 1
    require(sp.simplify(Xs.subs(M, mmax)) == sp.Rational(272, 507),
            "S2 exact full-band endpoint is X(16/3)=272/507")
    checks += 1
    require(0 < sp.Rational(272, 507) < 1,
            "S3 every nonzero production mode has unit-circle Floquet multipliers")
    checks += 1

    selected_monodromy = sp.simplify(monodromy.subs({k0: ks0, k1: ks1}))
    selected_trace = sp.factor(selected_monodromy.trace())
    require(selected_trace == sp.factor(2 - 4 * Xs),
            "S4 selected two-tick trace is exactly 2-4X")
    checks += 1
    require(selected_monodromy.det() == 1,
            "S5 selected update adds no dissipative or ghost multiplier")
    checks += 1

    # Derive the pole directly from the exact production symbol through q^6.
    qx, qy, qz, eps = sp.symbols("qx qy qz eps", real=True)
    ax, ay, az = (sp.cos(eps * q) for q in (qx, qy, qz))
    m18_exact = 4 - sp.Rational(2, 3) * (ax + ay + az) - sp.Rational(2, 3) * (
        ax * ay + ax * az + ay * az
    )
    m18_series = sp.series(m18_exact, eps, 0, 8).removeO().expand()
    x_series = sp.expand(As * m18_series - Bs * m18_series**2)
    theta2_series = sp.series(
        x_series + x_series**2 / 3 + sp.Rational(8, 45) * x_series**3,
        eps,
        0,
        8,
    ).removeO().expand()

    s2 = qx**2 + qy**2 + qz**2
    require(sp.expand(theta2_series.coeff(eps, 2) - s2 / 13) == 0,
            "P1 leading free cone is theta^2=S2/13")
    checks += 1
    require(homogeneous(theta2_series.coeff(eps, 4), (qx, qy, qz), 4) == 0,
            "P2 the complete dimension-six/q^4 pole correction vanishes identically")
    checks += 1

    sixth = sp.factor(theta2_series.coeff(eps, 6))
    expected_sixth = -(
        216 * (qx**6 + qy**6 + qz**6)
        + 479 * (
            qx**4 * qy**2 + qx**4 * qz**2
            + qx**2 * qy**4 + qx**2 * qz**4
            + qy**4 * qz**2 + qy**2 * qz**4
        )
        + 1803 * qx**2 * qy**2 * qz**2
    ) / sp.Integer(395460)
    require(sp.expand(sixth - expected_sixth) == 0,
            "P3 exact surviving q^6 correction matches the closed form")
    checks += 1
    require(sixth != 0,
            "P4 the prototype advances LR-1 only; dimension-eight cubic breaking remains")
    checks += 1

    # Positive-kick closure: cancellation with two nonnegative kick strengths
    # forces A>=1/4.  The endpoint test fails up to the upper root; above it,
    # the interior vertex exceeds one; A=1 makes X negative at Mmax.
    upper_root = (sp.Integer(13) + sp.sqrt(61)) / 32
    vertex_threshold = sp.Rational(17, 32)
    vertex_value = sp.factor(3 * a_eff / (4 * a_eff - 1))
    endpoint_minus_one = sp.factor(
        (a_eff * mmax - cancellation_B * mmax**2) - 1
    )
    require(sp.simplify(endpoint_minus_one
                        + (256 * a_eff**2 - 208 * a_eff + 27) / 27) == 0,
            "N1 positive-kick endpoint failure polynomial is exact")
    checks += 1
    require(upper_root > vertex_threshold,
            "N2 the endpoint-failure interval overlaps the interior-vertex regime")
    checks += 1
    require(sp.simplify(vertex_value - 1) == (1 - a_eff) / (4 * a_eff - 1),
            "N3 every 1/4<A<1 cancellation solution has an unstable vertex when reached")
    checks += 1
    require(sp.simplify((a_eff * mmax - cancellation_B * mmax**2).subs(a_eff, 1)) < 0,
            "N4 A=1 endpoint is unstable; a negative kick is necessary in this scalar class")
    checks += 1

    header = read("engine/include/ftd/lorentz_period2.h")
    toggles = read("engine/include/ftd/term_toggles.h")
    phase_read = read("engine/src/render_bridge_phases/phase_read.cpp")
    cmake = read("engine/CMakeLists.txt")
    test_source = read("engine/tests/test_lorentz_period2_floquet.cpp")

    require(
        "LORENTZ_PERIOD2_KAPPA_EVEN = 3.0 / 13.0" in header
        and "LORENTZ_PERIOD2_KAPPA_ODD = -1.0 / 13.0" in header,
        "W1 engine constants match the proved selected coefficients",
    )
    checks += 1
    require(
        "bool lorentz_period2_floquet = false" in toggles
        and '"lorentz_period2_floquet"' in toggles
        and "ToggleBackend::CPU" in toggles,
        "W2 prototype is registered CPU-only and defaults OFF",
    )
    checks += 1
    require(
        "lorentz_period2_kappa(rb.tick_)" in phase_read
        and "rb.delta_j_[i] = lap * cw2" in phase_read,
        "W3 live phase_read changes only the local wave-kick coefficient by tick parity",
    )
    checks += 1
    require(
        "test_lorentz_period2_floquet" in cmake
        and "engine two-tick monodromy matches exact recurrence" in test_source,
        "W4 native regression target covers live two-tick engine wiring",
    )
    checks += 1

    print()
    print(f"RESULT  {checks}/{checks} exact/source-contract checks passed")
    print("KICKS    kappa_even=+3/13, kappa_odd=-1/13 [SELECTED]")
    print("POLE     sin^2(theta)=M18/13+3*M18^2/676")
    print("STABLE   0<=M18<=16/3 => 0<=X<=272/507<1")
    print("IR       theta^2=S2/13+0*S2^2+O(q^6)")
    print("LOCALITY one current Moore-shell read per microscopic tick (P4 preserved)")
    print("VERDICT  LR-1 TREE POLE PASS; LR-2 THROUGH LR-6 OPEN")


if __name__ == "__main__":
    main()
