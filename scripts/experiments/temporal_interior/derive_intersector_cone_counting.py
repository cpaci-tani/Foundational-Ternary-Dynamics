"""derive_intersector_cone_counting.py — how expensive is a common cone?

THE PROBLEM (FTD-0412).  One sector can always be given unit speed by
choosing the time unit; that removes one normalisation and no more.  If two
propagating sectors have slopes c_A and c_B, the ratio c_A/c_B survives
every common unit change and is physical preferred-frame data.  The engine
currently carries c^2 = 1/3 (production flux), 1/7 (BCC-time prototype) and
raw 1 (standalone Wilson matter): an ORDER-UNITY mismatch, and the largest
quantitative problem in the framework.

FTD-0412 showed that a scalar Wilson parameter r cannot match the
quartic-free flux pole, because matching at q^4 needs
    (r^2/4) S_2^2 - (1/3) Q_4 = 0
in every direction, and S_2^2, Q_4 are INDEPENDENT cubic invariants while r
is one knob.  FTD-0413 exits by adding a face-diagonal kinetic weight --
one more knob -- and buys q^4, leaving q^6 open.

THIS SCRIPT ASKS THE STRUCTURAL QUESTION BEHIND THAT PATTERN:

  (1) EXACT no-go against the PRODUCTION flux.  FTD-0412 worked at q^4
      against the BCC-time pole.  Here: can scalar-r Wilson be made
      proportional to the production M18 symbol EXACTLY, at all orders?
  (2) INVARIANT COUNTING.  How many O_h invariants appear at each order,
      hence how many conditions must be met to match two sectors there?
  (3) THE COST CURVE.  Conditions accumulate as sum(count(n)).  Does the
      matching cost close at some order, or does every new order demand a
      new tuned coefficient -- an unbounded price?

The Wilson spectrum is E^2 = |K(q)|^2 + M(q)^2, a SUM OF SQUARES of
trigonometric polynomials, which is a real structural restriction; the flux
symbol is a general non-negative trigonometric polynomial.  (3) reports
where that bites.
"""
from __future__ import annotations

import itertools

import sympy as sp

q1, q2, q3, r, cs, kap = sp.symbols('q1 q2 q3 r c_s kappa', real=True)
Q = (q1, q2, q3)


# ---------------------------------------------------------------------
def flux_symbol():
    """Production M18 spatial symbol, -L_18(q) >= 0, vanishing at q=0."""
    c = [sp.cos(x) for x in Q]
    L = (sp.Rational(2, 3) * sum(c)
         + sp.Rational(2, 3) * (c[0]*c[1] + c[1]*c[2] + c[2]*c[0]) - 4)
    return sp.expand_trig(sp.simplify(-L))


def wilson_symbol():
    """Free massless Wilson: E^2 = c_s^2 [ sum sin^2 q_i + r^2 (sum(1-cos))^2 ]."""
    s = sum(sp.sin(x) ** 2 for x in Q)
    w = sum(1 - sp.cos(x) for x in Q)
    return cs ** 2 * (s + r ** 2 * w ** 2)


# ---------------------------------------------------------------------
print("=" * 72)
print("(1) EXACT MATCHING: scalar-r Wilson vs the PRODUCTION M18 flux")
print("=" * 72)
F = flux_symbol()
W = wilson_symbol()

# rewrite both as polynomials in u_i = cos q_i (sin^2 = 1 - u^2)
u1, u2, u3 = sp.symbols('u1 u2 u3', real=True)
U = (u1, u2, u3)
sub = {sp.cos(Q[i]): U[i] for i in range(3)}
Fu = sp.expand(F.rewrite(sp.cos).subs(sub))
Wu = sp.expand(sp.expand_trig(W).subs({sp.sin(Q[i]) ** 2: 1 - U[i] ** 2
                                       for i in range(3)}).subs(sub))
print(f"  flux   -L18 = {sp.factor(Fu)}")
print(f"  Wilson E^2  = {sp.expand(Wu)}")

# demand Wilson = kappa * flux identically in (u1,u2,u3)
diff = sp.expand(Wu - kap * Fu)
poly = sp.Poly(diff, u1, u2, u3)
eqs = [sp.Eq(c, 0) for c in poly.coeffs()]
print(f"\n  identical matching gives {len(eqs)} coefficient equations in "
      f"(c_s, r, kappa)")
sol = sp.solve(eqs, [cs ** 2, r ** 2, kap], dict=True)
print(f"  solutions with c_s^2 > 0: {sol if sol else 'NONE'}")

# the sharp contradiction is between the u1*u2 and u1 coefficients, both of
# which fix kappa in terms of c_s^2 r^2 -- with opposite signs
c12 = sp.simplify(poly.coeff_monomial(u1 * u2))
c1 = sp.simplify(poly.coeff_monomial(u1))
k12 = sp.solve(c12, kap)[0]
k1 = sp.solve(c1, kap)[0]
print(f"\n  coeff of u1*u2 : {c12} = 0  ->  kappa = {k12}")
print(f"  coeff of u1    : {c1} = 0  ->  kappa = {k1}")
print(f"  both  =>  {sp.simplify(k12 - k1)} = 0  =>  c_s^2 r^2 = 0")
print("  so no real (c_s, r) matches the production flux at ANY order: NO-GO")
assert sp.simplify(k12 - k1) != 0, "expected a contradiction"


# ---------------------------------------------------------------------
print()
print("=" * 72)
print("(2) HOW MANY CONDITIONS PER ORDER: O_h invariants")
print("=" * 72)
print("  Invariants are polynomials in p1 = sum q_i^2, p2 = sum_{i<j} q_i^2 q_j^2,")
print("  p3 = q1^2 q2^2 q3^2 -- so the count at order q^{2n} is the number of")
print("  ways to write n from parts {1,2,3}.  Isotropy needs ALL BUT p1^n killed.")
print()
print(f"  {'order':>7} {'invariants':>12} {'conditions':>12} "
      f"{'cumulative':>12}")
tot = 0
counts = {}
for n in range(1, 9):
    k = sum(1 for a in range(n + 1) for b in range((n - a) // 2 + 1)
            for cc in range((n - a - 2 * b) // 3 + 1)
            if a + 2 * b + 3 * cc == n)
    counts[n] = k
    cond = k - 1
    tot += cond
    print(f"  q^{2*n:<5} {k:>12} {cond:>12} {tot:>12}")

print("""
  Under naive order-by-order finite-shell tuning, matching two sectors
  through order q^{2N} imposes sum_{n<=N}(count(n)-1) conditions, and that
  sum GROWS WITHOUT BOUND.  This counting is not a no-go against a finite
  all-orders identity, which can satisfy infinitely many coefficients at
  once (as the exact SOS identity below does).""")


# ---------------------------------------------------------------------
print()
print("=" * 72)
print("(3) DOES THE FTD-0413 KNOB SURVIVE TO q^6?")
print("=" * 72)
print("  FTD-0413 buys q^4 by adding a face-diagonal kinetic weight -- one")
print("  extra knob for the one q^4 condition.  Enrich BOTH structures:")
print("     K_i = a1 sin q_i + a2 sin q_i (cos q_j + cos q_k)")
print("     M   = b1 sum(1 - cos q_i) + b2 sum_{i<j} (1 - cos q_i cos q_j)")
print("  E^2 = sum_i K_i^2 + M^2, matched to kappa * (-L18).\n")

a1, a2, b1, b2 = sp.symbols('a1 a2 b1 b2', real=True)
t = sp.symbols('t', positive=True)


def series_invariants(expr, order=8):
    """Expand in q about 0 and return {(deg, invariant): coefficient}."""
    e = expr.subs({Q[i]: t * Q[i] for i in range(3)})
    s = sp.series(e, t, 0, order).removeO()
    out = {}
    for n in range(1, order // 2 + 1):
        c = sp.expand(sp.simplify(s.coeff(t, 2 * n)))
        out[n] = sp.Poly(c, *Q) if c != 0 else None
    return out


K = [a1 * sp.sin(Q[i]) + a2 * sp.sin(Q[i]) * (sp.cos(Q[(i+1) % 3])
                                              + sp.cos(Q[(i+2) % 3]))
     for i in range(3)]
M = (b1 * sum(1 - sp.cos(x) for x in Q)
     + b2 * sum(1 - sp.cos(Q[i]) * sp.cos(Q[j])
                for i, j in ((0, 1), (1, 2), (2, 0))))
E2 = sum(k ** 2 for k in K) + M ** 2

resid = sp.expand(E2 - kap * flux_symbol())
inv = series_invariants(resid, order=8)

print(f"  {'order':>7} {'#conditions':>13}   residual coefficients must vanish")
eqs = []
for n in (1, 2, 3):
    p = inv[n]
    if p is None:
        continue
    cs_ = [sp.simplify(c) for c in p.coeffs()]
    uniq = sorted({sp.srepr(sp.simplify(c)) for c in cs_})
    eqs += cs_
    print(f"  q^{2*n:<5} {len(uniq):>13}")

uniq_eqs = list({sp.srepr(sp.expand(e)): sp.expand(e) for e in eqs}.values())
print(f"\n  distinct equations through q^6: {len(uniq_eqs)}")
print(f"  free parameters: a1, a2, b1, b2, kappa = 5")

# E^2 is QUADRATIC in (a,b) with no a-b cross terms, so the system is LINEAR
# in the six monomials below.  Solve there, then impose the rank-1 and
# positivity conditions that a real (a1,a2,b1,b2) must satisfy.
A11, A12, A22, B11, B12, B22 = sp.symbols(
    'A11 A12 A22 B11 B12 B22', real=True)
mono = {a1**2: A11, a1*a2: A12, a2**2: A22,
        b1**2: B11, b1*b2: B12, b2**2: B22}
lin = []
for e in uniq_eqs:
    p = sp.Poly(e, a1, a2, b1, b2)
    acc = 0
    for m, c in zip(p.monoms(), p.coeffs()):
        term = a1**m[0] * a2**m[1] * b1**m[2] * b2**m[3]
        acc += c * (mono.get(term, term))
    lin.append(sp.expand(acc))
unk = [A11, A12, A22, B11, B12, B22, kap]
sol = sp.solve(lin, unk, dict=True)
print(f"  linear system in the 6 monomials + kappa: "
      f"{len(lin)} equations, {len(unk)} unknowns")
print(f"  solutions: {len(sol)}")
ok = False
for s in sol:
    k = sp.simplify(s.get(kap, kap))
    if k == 0:
        continue
    r1 = sp.simplify(s.get(A11, A11) * s.get(A22, A22) - s.get(A12, A12)**2)
    r2 = sp.simplify(s.get(B11, B11) * s.get(B22, B22) - s.get(B12, B12)**2)
    print(f"    kappa = {k}")
    print(f"    rank-1 residual (a): {r1}      (must be 0)")
    print(f"    rank-1 residual (b): {r2}      (must be 0)")
    print(f"    positivity A11={sp.simplify(s.get(A11, A11))}, "
          f"B11={sp.simplify(s.get(B11, B11))}")
    if r1 == 0 and r2 == 0:
        ok = True
print(f"""
  matched through q^6 by a real operator: {ok}
  => the enriched operator buys q^2 and q^4 (FTD-0413) and the knob is
     spent.  In this naive order-by-order finite-shell ansatz, each new
     order contributes count(n)-1 conditions while each new shell adds one
     coefficient.  That is a Symanzik-style improvement programme, not a
     proof that no finite all-orders structural identity can close.""")


# =====================================================================
print()
print("=" * 72)
print("(4) RESOLVED: the flux symbol IS a sum of squares")
print("=" * 72)
sos = (sp.Rational(4, 3) * sum(sp.sin(x / 2) ** 2 for x in Q)
       + sp.Rational(2, 3) * sum(sp.sin((Q[i] - Q[j]) / 2) ** 2
                                 + sp.sin((Q[i] + Q[j]) / 2) ** 2
                                 for i, j in ((0, 1), (1, 2), (2, 0))))
ident = sp.simplify(sp.expand_trig(sp.expand(flux_symbol() - sos)))
print("  -L18 = (4/3) sum_i sin^2(q_i/2)")
print("       + (2/3) sum_{i<j} [ sin^2((q_i-q_j)/2) + sin^2((q_i+q_j)/2) ]")
print(f"  [verify] identity holds symbolically: {ident == 0}")
assert ident == 0, "SOS decomposition of the flux symbol failed"

import numpy as np
rng = np.random.default_rng(3)
f = sp.lambdify(Q, flux_symbol() - sos, "numpy")
worst = max(abs(float(f(*rng.uniform(-np.pi, np.pi, 3)))) for _ in range(2000))
print(f"  [verify] numeric, 2000 random q: max |residual| = {worst:.3e}")
assert worst < 1e-12, "SOS decomposition fails numerically"
print(f"  [verify] squares: 3 face + 6 edge = 9")
print(f"  [verify] anticommuting structures in dim 2^k = 2k+1 -> "
      f"9 needs dim {2**4}")
print("""
  THIS IS AN ECONOMICAL HALF-ANGLE BASIS, NOT A NECESSITY THEOREM.
  The phase in exp(iq)-1 = 2i exp(iq/2) sin(q/2) means a half-angle
  magnitude does not by itself place endpoints at half sites.  Every graph
  hop also has the exact integer-frequency identity
      2w(1-cos(q.d)) = w sin^2(q.d) + w(1-cos(q.d))^2.
  For M18 the 3 face pairs and 6 edge pairs therefore give an exact
  18-square integer-hop construction.  Half offsets lower the known square
  count; they are not forced by existence.

  UPPER BOUND ONLY: 9 is this decomposition's count, not proven minimal;
  and doubling, chirality and induced-interaction locality are untested.""")
