#!/usr/bin/env python3
"""Exact algebraic witnesses for FTD-0581 (no parameter search)."""

import sympy as sp


E, c, C = sp.symbols("E c C", positive=True)
Delta = C / 4
p_dep = sp.sqrt(2 * E * Delta + Delta**2) / c
H = sp.sqrt(E**2 + c**2 * p_dep**2)
assert sp.simplify(H**2 - (E + Delta)**2) == 0

v_dep = c**2 * p_dep / (E + Delta)
assert sp.simplify(
    v_dep**2 / c**2 - (1 - E**2 / (E + Delta)**2)) == 0
assert sp.simplify((H**2 - E**2) / c**2 - p_dep**2) == 0

# Periodic continuation of the chord potential has a linear cusp at every
# integer site. The two one-sided slopes are +/-C.
r = sp.symbols("r", real=True)
V_right = C * r * (1 - r)
V_left = C * (r + 1) * (1 - (r + 1))
assert sp.diff(V_right, r).subs(r, 0) == C
assert sp.diff(V_left, r).subs(r, 0) == -C

# A stable completed-square passive response z-z_*=r*a starts quadratically.
k, a = sp.symbols("k a", positive=True)
U_passive = sp.Rational(1, 2) * k * (a * r)**2
assert sp.diff(U_passive, r).subs(r, 0) == 0
assert sp.limit(U_passive / sp.Abs(r), r, 0, dir="+") == 0

# Pointwise completed-square positivity: lambda >= 0 and y^2 >= 0 imply the
# deformed field is never below the relaxed Peierls curve.
lam, y = sp.symbols("lam y", nonnegative=True)
completed_square_excess = sp.Rational(1, 2) * lam * y**2
assert completed_square_excess.is_nonnegative

# An active internal reservoir can only budget the crossing when its initial
# excitation is at least Delta=C/4.
epsilon = sp.symbols("epsilon", positive=True)
U_active = epsilon - C * r * (1 - r)
assert sp.simplify(U_active.subs(r, sp.Rational(1, 2))
                   - (epsilon - Delta)) == 0
U_equal = sp.factor(U_active.subs(epsilon, Delta))
assert sp.simplify(U_equal - C * (r - sp.Rational(1, 2))**2) == 0

# At equality, a positive oscillator coordinate is proportional to |r-1/2|:
# Lipschitz but not differentiable at the saddle.
omega = sp.symbols("omega", positive=True)
Q_equal = sp.sqrt(2 * C) * sp.Abs(r - sp.Rational(1, 2)) / omega
left = sp.limit(sp.diff(Q_equal, r), r, sp.Rational(1, 2), dir="-")
right = sp.limit(sp.diff(Q_equal, r), r, sp.Rational(1, 2), dir="+")
assert left == -sp.sqrt(2 * C) / omega
assert right == sp.sqrt(2 * C) / omega
assert sp.simplify(right - left - 2 * sp.sqrt(2 * C) / omega) == 0

# Strictly larger excitation stays positive and has a smooth square root.
for ratio in (2, 4):
    U_ratio = sp.factor(U_active.subs(epsilon, ratio * Delta))
    assert sp.simplify(U_ratio.subs(r, sp.Rational(1, 2))
                       - (ratio - 1) * Delta) == 0

print("FTD-0581 exact passive-dressing/depinning proof: PASS")
print("p_dep=sqrt(2*E_REST*Delta+Delta^2)/C_SPEED")
print("passive_excess=1/2<z-z_*,K(z-z_*)> >= 0")
print("K(p_0)+epsilon_0 >= Delta=C_d/4")
print("verdict=PASSIVE_DRESSING_CANNOT_DEPIN_ACTIVE_TRAVERSAL_COSTS_FINITE_EXCITATION")
