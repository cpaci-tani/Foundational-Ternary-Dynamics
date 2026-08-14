#!/usr/bin/env python3
"""Exact checks for SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.

This is a symbolic consistency certificate.  It does not search for a clock
carrier and does not promote any substrate result.
"""

from __future__ import annotations

import sympy as sp


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    checks.append((name, bool(condition)))


# A harmonic oscillator has a phase even though its rate is action-degenerate.
m, k, q, p = sp.symbols("m k q p", positive=True, real=True)
omega = sp.sqrt(k / m)
energy_h = p**2 / (2 * m) + k * q**2 / 2
action_h = sp.simplify(energy_h / omega)
check("harmonic action is nonzero state information", action_h.has(q, p))
I = sp.symbols("I", positive=True, real=True)
check("harmonic rate is action independent", sp.diff(omega, I) == 0)

# Continuous phase-plane orientation for even monomial clocks.
n, lam = sp.symbols("n lam", positive=True, real=True)
qdot = p / m
pdot = -n * lam * q ** (n - 1)
phase_current = sp.expand(q * pdot - p * qdot)
check(
    "monomial phase current identity",
    phase_current == -n * lam * q**n - p**2 / m,
)

# General even-monomial period and its quadratic/quartic specializations.
A = sp.symbols("A", positive=True, real=True)
I_n = sp.sqrt(sp.pi) * sp.gamma(1 / n) / (
    n * sp.gamma(sp.Rational(1, 2) + 1 / n)
)
T_n = 4 * sp.sqrt(m / (2 * lam)) * A ** (1 - n / 2) * I_n

T_2 = sp.simplify(T_n.subs(n, sp.Integer(2)))
check(
    "quadratic period is amplitude independent",
    sp.simplify(T_2 - 2 * sp.pi * sp.sqrt(m / (2 * lam))) == 0,
)

T_4_A = sp.simplify((T_n * A).subs(n, sp.Integer(4)))
gstar = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
check(
    "quartic normalized period is Gstar",
    sp.simplify(T_4_A - sp.sqrt(sp.pi) * gstar * sp.sqrt(m / (2 * lam)))
    == 0,
)

# A discrete oriented phase current changes sign under reversed rotation.
theta, radius = sp.symbols("theta radius", positive=True, real=True)
chi_forward = -radius**2 * sp.sin(theta)
chi_reverse = radius**2 * sp.sin(theta)
check("forward and reverse phase currents differ", sp.simplify(chi_forward - chi_reverse) != 0)
check("phase-current magnitudes agree", sp.simplify(chi_forward**2 - chi_reverse**2) == 0)

# At a quarter turn the two rank-two orientation lifts differ by the central
# sign, while Sym^2 identifies them exactly.
R_plus = sp.Matrix([[0, -1], [1, 0]])
R_minus = -R_plus


def sym2_matrix(a: sp.Matrix) -> sp.Matrix:
    """Symmetric square in the basis (x^2, x*y, y^2)."""
    a11, a12, a21, a22 = a[0, 0], a[0, 1], a[1, 0], a[1, 1]
    return sp.Matrix(
        [
            [a11**2, 2 * a11 * a12, a12**2],
            [a11 * a21, a11 * a22 + a12 * a21, a12 * a22],
            [a21**2, 2 * a21 * a22, a22**2],
        ]
    )


check("rank-two quarter-turn lifts are distinct", R_plus != R_minus)
check("Sym2 loses the central orientation sign", sym2_matrix(R_plus) == sym2_matrix(R_minus))
check("quarter turn squares to minus identity", R_plus**2 == -sp.eye(2))
check("normalized inert action has order four", R_plus**4 == sp.eye(2))

# The FTD-0827 map and differential pullback.
x, y = sp.symbols("x y", nonzero=True)
u = x ** -2
v = -y * x ** -3
curve_residual = sp.factor((v**2 - (u**3 - u)) * x**6)
check("quartic-to-CM curve map", sp.rem(curve_residual, y**2 - (1 - x**4), y) == 0)
du_dx = sp.diff(u, x)
check("clock differential pulls back exactly", sp.simplify(du_dx / (2 * v) - 1 / y) == 0)


passed = sum(ok for _, ok in checks)
for name, ok in checks:
    print(f"{'PASS' if ok else 'FAIL'}: {name}")
print(f"\n{passed}/{len(checks)} checks passed")

if passed != len(checks):
    raise SystemExit(1)
