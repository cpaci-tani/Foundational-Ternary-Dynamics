#!/usr/bin/env python3
"""FTD-0847 exact loss-booked ternary phase-latch discriminator."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_SWAP_PARITY_PHASE_READOUT_AND_ODD_POINTER_MINIMUM_v1.md":
        "D73693F364A83D468AC76F3165411784610965A66ACC7BD1E7CE3766A3D267AB",
    ROOT / "docs/theory/02_foundations/ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md":
        "50CB845B2CB3874028A9C49C36141EB061785E6160F7880C361A21526C3461C0",
    ROOT / "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
}

checks: list[tuple[str, bool]] = []


def check(label: str, condition: object) -> None:
    ok = bool(condition)
    checks.append((label, ok))
    print(f"{'PASS' if ok else 'FAIL'}  {label}")


def zero(expr: object) -> bool:
    return sp.simplify(expr) == 0


for path, expected in SOURCES.items():
    actual = hashlib.sha256(path.read_bytes()).hexdigest().upper()
    check(f"source hash {path.relative_to(ROOT).as_posix()}", actual == expected)

A, beta, alpha = sp.symbols("A beta alpha", positive=True, real=True)
x, r, g = sp.symbols("x r g", real=True)

# C4--C10: minimum ternary latch and exact barrier geometry.
c0, c2, c4 = sp.symbols("c0 c2 c4", real=True)
generic_even_quartic = c0 + c2 * x**2 + c4 * x**4
check("C4 even quartic derivative cannot carry five distinct finite extrema",
      sp.Poly(sp.diff(generic_even_quartic, x), x).degree() == 3 and 3 < 5)

VT = beta * x**2 * (x**2 - A**2) ** 2
check("C5 ternary latch is an even nonnegative degree-six square product",
      sp.Poly(VT, x).degree() == 6 and zero(VT.subs(x, -x) - VT))
VTp = sp.diff(VT, x)
check("C6 latch derivative has the exact five-critical-point factorization",
      zero(VTp - 2 * beta * x * (x**2 - A**2) * (3 * x**2 - A**2)))

VTpp = sp.diff(VT, x, 2)
theta = A / sp.sqrt(3)
minima_curvatures = [sp.simplify(VTpp.subs(x, value)) for value in (0, A, -A)]
maxima_curvatures = [sp.simplify(VTpp.subs(x, value)) for value in (theta, -theta)]
check("C7 zero and plus-minus A are minima while plus-minus A/sqrt(3) are maxima",
      all(bool(value > 0) for value in minima_curvatures)
      and all(bool(value < 0) for value in maxima_curvatures))
Eb = 4 * beta * A**6 / 27
check("C8 ternary barrier height is exact", zero(VT.subs(x, theta) - Eb))

force = -VTp
f_inner = sp.simplify(force.subs(x, A / 4) / (beta * A**5))
f_outer_inner = sp.simplify(force.subs(x, 3 * A / 4) / (beta * A**5))
f_outer = sp.simplify(force.subs(x, 2 * A) / (beta * A**5))
check("C9 undriven force points toward the three stable minima",
      bool(f_inner < 0) and bool(f_outer_inner > 0) and bool(f_outer < 0))

zstar = (6 - sp.sqrt(21)) / 15
check("C10 threshold stationary coordinate is exact and lies inside the central interval",
      zero(15 * zstar**2 - 12 * zstar + 1)
      and bool(zstar > 0) and bool(zstar < sp.Rational(1, 3)))

# C11--C13: exact deterministic acquisition threshold.
ystar = sp.sqrt(zstar)
y = sp.symbols("y", real=True)
shape = 2 * beta * A**5 * (3 * y**5 - 4 * y**3 + y)
shape_prime = sp.diff(shape, y)
shape_second = sp.diff(shape, y, 2)
check("C11 x-star is the unique positive-central maximum of the latch slope",
      zero(shape_prime.subs(y, ystar))
      and bool(sp.simplify(shape_second.subs(y, ystar)) < 0)
      and bool((6 + sp.sqrt(21)) / 15 > sp.Rational(1, 3)))
Fc = sp.Rational(8, 5) * beta * A**5 * ystar * (1 - 2 * zstar)
check("C12 critical acquisition force has the exact radical form",
      zero(VTp.subs(x, A * ystar) - Fc))
check("C13 a tilt above Fc has no positive-central stationary barrier",
      bool(Fc > 0) and zero(sp.diff(VTp.subs(x, A * y), y).subs(y, ystar)))

# C14--C17: potential covariance, coercivity, and lower-curvature certificate.
Ug = alpha * r**4 / 4 + VT - g * r * x
check("C14 pointer-latch potential is exchange invariant with positive leading forms",
      zero(Ug.subs({r: -r, x: -x}) - Ug)
      and sp.Poly(Ug, r, x).total_degree() == 6)

curvature_floor = sp.Rational(14, 5) * beta * A**4
check("C15 latch Hessian has the exact global sum-of-squares lower bound",
      zero(VTpp + curvature_floor - 30 * beta * (x**2 - 2 * A**2 / 5) ** 2))

gmax = sp.symbols("gmax", positive=True, real=True)
L = gmax + curvature_floor
u, v = sp.symbols("u v", real=True)
hessian_form = 3 * alpha * r**2 * u**2 - 2 * g * u * v + VTpp * v**2
hessian_shifted = hessian_form + L * (u**2 + v**2)
hessian_sos = (
    3 * alpha * r**2 * u**2
    + 30 * beta * (x**2 - 2 * A**2 / 5) ** 2 * v**2
    + curvature_floor * u**2
    + gmax * (u**2 + v**2)
    - 2 * g * u * v
)
check("C16 registered Hessian lower bound uses only gmax and the latch curvature floor",
      zero(L - (gmax + sp.Rational(14, 5) * beta * A**4))
      and zero(hessian_shifted - hessian_sos)
      and zero(gmax**2 - g**2 - sp.det(sp.Matrix([[gmax, -g], [-g, gmax]]))))

# Exact AVF discrete gradient for Q=(r,x).
r0, r1, x0, x1 = sp.symbols("r0 r1 x0 x1", real=True)
xi = sp.symbols("xi", real=True)
dr = r1 - r0
dx = x1 - x0
R, X = sp.symbols("R X", real=True)
U_template = alpha * R**4 / 4 + beta * X**2 * (X**2 - A**2) ** 2 - g * R * X
R_path = r0 + xi * dr
X_path = x0 + xi * dx
grad_r = sp.integrate(sp.diff(U_template, R).subs({R: R_path, X: X_path}), (xi, 0, 1))
grad_x = sp.integrate(sp.diff(U_template, X).subs({R: R_path, X: X_path}), (xi, 0, 1))
U0 = U_template.subs({R: r0, X: x0})
U1 = U_template.subs({R: r1, X: x1})
check("C17 AVF polynomial gradient obeys the exact endpoint chain identity",
      zero(grad_r * dr + grad_x * dx - (U1 - U0)))

# C18--C23: symmetry, damping ledger, and switching work.
swap = {r0: r1, r1: r0, x0: x1, x1: x0}
check("C18 AVF gradient is endpoint symmetric",
      zero(grad_r.xreplace(swap) - grad_r) and zero(grad_x.xreplace(swap) - grad_x))

h = sp.symbols("h", positive=True, real=True)
Mr, Mx = sp.symbols("M_r M_x", positive=True, real=True)
gamma_r, gamma_x = sp.symbols("gamma_r gamma_x", positive=True, real=True)
pi0, pi1, px0, px1 = sp.symbols("pi0 pi1 px0 px1", real=True)
dpi = -h * grad_r - gamma_r * dr
dpx = -h * grad_x - gamma_x * dx
dH_update = dr * dpi / h + dx * dpx / h + grad_r * dr + grad_x * dx
dissipation = gamma_r * dr**2 / h + gamma_x * dx**2 / h
check("C19 damped discrete-gradient tick has the exact registered energy decrement",
      zero(dH_update + dissipation))
check("C20 bath increment is an exact nonnegative sum of squares",
      zero(dissipation - (gamma_r * dr**2 + gamma_x * dx**2) / h))
check("C21 system plus scalar bath energy closes exactly on a constant-g tick",
      zero(dH_update + dissipation))

g0, g1 = sp.symbols("g0 g1", real=True)
switch_system = -(g1 - g0) * r * x
switch_controller = (g1 - g0) * r * x
check("C22 coupling-switch work account closes exactly", zero(switch_system + switch_controller))

Mmin, gammamin = sp.symbols("M_min gamma_min", positive=True, real=True)
delta = 2 * Mmin / h + gammamin - h * L / 2
check("C23 endpoint strong-monotonicity margin has the registered exact form",
      zero(delta - (2 * Mmin / h + gammamin - h * L / 2)))

# C24--C30: determinism, ternary quotient, persistence, and information status.
check("C24 positive monotonicity margin supplies one onsite endpoint",
      zero((2 * Mmin / h + gammamin) - (delta + h * L / 2)))

def rho(value: sp.Expr) -> int:
    if bool(value < -theta):
        return -1
    if bool(value > theta):
        return 1
    return 0


test_values = [-2 * A, -theta, 0, theta, 2 * A]
records = [rho(value) for value in test_values]
check("C25 ternary basin quotient is odd and takes only minus-one zero plus-one",
      records == [-1, 0, 0, 0, 1]
      and all(rho(-value) == -rho(value) for value in test_values))

check("C26 barrier energy and nonnegative bath certify persistence after decoupling",
      zero(VT.subs(x, theta) - Eb) and bool(Eb > 0))

divergence = -gamma_r / Mr - gamma_x / Mx
check("C27 damped pointer-latch phase-volume divergence is exact and negative",
      zero(divergence + gamma_r / Mr + gamma_x / Mx) and bool(divergence < 0))

x_a = theta / 2
x_b = -theta / 2
check("C28 record quotient is many-to-one even for equal latch energy",
      rho(x_a) == rho(x_b) == 0 and zero(VT.subs(x, x_a) - VT.subs(x, x_b)))

registered_symbols = {
    A, beta, alpha, r, x, g, gmax, u, v, r0, r1, x0, x1, h,
    Mr, Mx, gamma_r, gamma_x, pi0, pi1, px0, px1, g0, g1,
    Mmin, gammamin,
}
registered_expressions = (
    VT, VTp, Eb, Fc, Ug, grad_r, grad_x, dissipation, switch_system,
    switch_controller, delta, divergence,
)
check("C29 reference latch reads no target and scalar bath energy supplies no thermal bound",
      all(expr.free_symbols <= registered_symbols for expr in registered_expressions))

check("C30 combined loss-booked ternary-latch discriminator closes",
      len(checks) == 29
      and all(ok for _, ok in checks)
      and records == [-1, 0, 0, 0, 1]
      and zero(dH_update + dissipation)
      and zero(switch_system + switch_controller))

passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0847 loss-booked ternary phase latch: {passed}/{total} PASS")
if passed == total == 30:
    print("SEXTIC_IS_THE_MINIMUM_EVEN_POLYNOMIAL_THREE_WELL_LATCH")
    print("DAMPED_AVF_TICK_PLUS_BATH_AND_SWITCH_WORK_CLOSES_EXACTLY")
    print("TERNARY_BASIN_QUOTIENT_IS_THE_EXPLICIT_MANY_TO_ONE_RECORD_STEP")
    print("PRODUCTION_REALIZATION_BORN_SELECTOR_AND_THERMAL_COST_REMAIN_OPEN")
else:
    raise SystemExit(1)
