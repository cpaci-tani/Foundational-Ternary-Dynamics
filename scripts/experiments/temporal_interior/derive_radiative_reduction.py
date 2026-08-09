"""derive_radiative_reduction.py — what the radiative-stability problem
actually requires, and what survives for a single-substrate theory.

Three exact results:

  (A) STENCIL UNIQUENESS.  The production weights (1/3, 1/6) are not
      tuned: they are the unique O_h-symmetric 18-point weights with the
      correct continuum limit and isotropic O(k^4) term.  Isotropy is
      therefore structural.

  (B) DIMENSION-4 LORENTZ VIOLATION VANISHES BY SYMMETRY.  Cubic symmetry
      forces the quadratic form in k to be a multiple of delta_ij, so the
      dangerous dim-4 coefficient is exactly zero -- not small, zero.

  (C) THE OBSERVABLE CONTENT IS RELATIONAL.  For ONE propagating sector a
      dim-4 LV coefficient is absorbed by a units redefinition and is
      unobservable; only DIFFERENCES between sectors, and the
      energy/direction dependence within a sector, are physical.  We
      verify the absorption explicitly and show what survives with two
      sectors.

This does not solve the naturalness problem for multi-species effective
field theories.  It locates which part of that problem a single-substrate
theory actually inherits.
"""
from __future__ import annotations

import sympy as sp
import itertools

k1, k2, k3, t = sp.symbols('k1 k2 k3 t', positive=True)
wf, we = sp.symbols('w_f w_e', real=True)
K2 = k1**2 + k2**2 + k3**2

print("=" * 70)
print("(A) STENCIL UNIQUENESS")
print("=" * 70)

c1, c2, c3 = sp.cos(k1), sp.cos(k2), sp.cos(k3)
# general O_h-symmetric 18-point stencil: one weight per shell
L = (wf * 2*(c1 + c2 + c3)
     + we * 4*(c1*c2 + c2*c3 + c3*c1)
     - (6*wf + 12*we))
Lt = L.subs({k1: t*k1, k2: t*k2, k3: t*k3})
ser = sp.expand(sp.series(Lt, t, 0, 7).removeO())

q2 = sp.expand(ser.coeff(t, 2))
q4 = sp.expand(ser.coeff(t, 4))
print(f"  k^2 coefficient: {sp.factor(q2)}")
print(f"  k^4 coefficient: {sp.factor(q4)}")

# condition 1: correct continuum limit, k^2 coefficient = -K2
cond_limit = sp.simplify(q2 + K2)
c_lim = sp.Poly(cond_limit, k1, k2, k3).coeffs()
eq1 = sp.simplify(c_lim[0])
print(f"\n  continuum-limit condition: {sp.Eq(eq1, 0)}")

# condition 2: k^4 term isotropic, i.e. proportional to (K2)^2
lam = sp.symbols('lambda')
resid = sp.expand(q4 - lam * K2**2)
polys = sp.Poly(resid, k1, k2, k3)
eqs = [sp.simplify(c) for c in polys.coeffs()]
sol = sp.solve(eqs + [eq1], [wf, we, lam], dict=True)
print(f"  isotropy + limit  =>  {sol}")
assert len(sol) == 1, "expected a unique solution"
s0 = sol[0]
print(f"\n  UNIQUE: w_face = {s0[wf]}, w_edge = {s0[we]}, "
      f"k^4 coefficient = {s0[lam]}*(k^2)^2")
print("  These are exactly the production weights.  The isotropy of the")
print("  leading correction is forced by symmetry + normalisation, not tuned.")

print()
print("=" * 70)
print("(B) THE DIMENSION-4 COEFFICIENT VANISHES BY CUBIC SYMMETRY")
print("=" * 70)
# A general quadratic form c_ij k_i k_j invariant under the cubic group
# must be a multiple of delta_ij.  Verify by averaging an arbitrary
# symmetric matrix over the 48 elements of O_h.
C = sp.Matrix(3, 3, lambda i, j: sp.Symbol(f'c{min(i,j)}{max(i,j)}'))
perms = list(itertools.permutations(range(3)))
signs = list(itertools.product([1, -1], repeat=3))
acc = sp.zeros(3, 3)
n = 0
for p in perms:
    for sgn in signs:
        R = sp.zeros(3, 3)
        for i in range(3):
            R[i, p[i]] = sgn[i]
        acc += R.T * C * R
        n += 1
avg = sp.simplify(acc / n)
print(f"  |O_h| = {n}")
print("  cubic average of a general symmetric c_ij:")
sp.pprint(avg)
offdiag = [sp.simplify(avg[i, j]) for i in range(3) for j in range(3) if i != j]
diag = [sp.simplify(avg[i, i]) for i in range(3)]
print(f"\n  off-diagonal entries all zero: {all(e == 0 for e in offdiag)}")
print(f"  diagonal entries all equal:    "
      f"{sp.simplify(diag[0]-diag[1]) == 0 and sp.simplify(diag[1]-diag[2]) == 0}")
print("  => any cubic-invariant quadratic form is isotropic: c_ij = c*delta_ij.")
print("  The anisotropic dim-4 operator is FORBIDDEN, with coefficient")
print("  identically zero rather than merely small.")

print()
print("=" * 70)
print("(C) FOR ONE SECTOR, DIM-4 LV IS A UNITS REDEFINITION")
print("=" * 70)
eps, x, tau, c0 = sp.symbols('epsilon x tau c_0', positive=True)
omega, kk = sp.symbols('omega k', positive=True)

# one sector with a dim-4 LV shift: omega^2 = c0^2 (1+eps) k^2
disp1 = sp.Eq(omega**2, c0**2 * (1 + eps) * kk**2)
print(f"  sector 1 dispersion: {disp1}")
# rescale time t -> t/sqrt(1+eps): the shift is absorbed
print("  rescale  t -> t/sqrt(1+eps)  (equivalently, define the unit of")
print("  speed by this sector).  Then:")
absorbed = sp.simplify((c0**2 * (1 + eps) * kk**2) / (1 + eps))
print(f"    omega'^2 = {absorbed}  -- the LV shift is gone.")
print("  A single sector therefore has NO observable dim-4 LV: its speed")
print("  IS the definition of the speed.  There is nothing to compare to.")

print()
eps1, eps2 = sp.symbols('epsilon_1 epsilon_2', real=True)
print("  With TWO sectors, epsilon_1 and epsilon_2:")
print("    after the same rescaling, sector 1 -> 1, sector 2 -> "
      f"{sp.simplify((1+eps2)/(1+eps1))}")
delta = sp.simplify(sp.series((1 + eps2)/(1 + eps1) - 1, eps1, 0, 2).removeO())
print(f"    difference to first order: {sp.simplify(delta)}")
print("  Only the DIFFERENCE is physical.  This is why every experimental")
print("  bound on dim-4 Lorentz violation is a bound on a relative")
print("  quantity: photon vs matter, or one species against another.")

print()
print("=" * 70)
print("CONSEQUENCE")
print("=" * 70)
print("""  The radiative-stability problem is a statement about DIFFERENTIAL
  renormalisation: loops in sector B shift B's cone relative to A's.  Its
  observable content therefore requires at least two independently
  renormalised propagating sectors.

  A theory whose excitations are all configurations of ONE substrate
  field does not automatically inherit that problem -- it inherits
  instead the question of whether its emergent sectors share a cone.
  That is a different, and directly checkable, question.

  What remains observable for a single substrate, and is computed
  exactly elsewhere in this programme:
     energy dependence  : omega^2 = C^2 k^2 (1 - (ka)^2/12 + ...)
     direction dependence: |dv/v| = (ka)^4 / 3240
  Both isotropic-suppressed and both far below every current bound.""")
