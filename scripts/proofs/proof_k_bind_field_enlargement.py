"""
proof_k_bind_field_enlargement.py — checkable algebra for the FTD-0351 repair of
FTD-0244 Lemma 1 (FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md).

Repair being verified: the K-BIND invariant field is Q(G*, pi), NOT Q(G*) as the
document previously claimed (generator 3 = G_BCC(0)*I with G_BCC(0) = G*^2/(2 pi)
has trace G*^2/pi, which involves pi). The theorem's conclusion survives the
enlargement because delta = sqrt(G*(4G*-1)) remains outside Q(G*, pi):

  Q(G*, pi) ~ Q(t, u)  (two-variable rational function field, CONDITIONAL on
  Chudnovsky 1976: {pi, Gamma(1/4)} algebraically independent over Q, and
  G* = Gamma(1/4)^2/(pi sqrt(2)), so a G*-pi relation would violate it);
  delta^2 = t(4t-1) is squarefree with odd valuation at the prime (t), hence a
  non-square in Q(u)(t), hence y^2 - t(4t-1) is irreducible and [F(delta):F] = 2.

This script machine-verifies every step that IS checkable:
  1. G_BCC(0) = G*^2/(2 pi) = Gamma(1/4)^4/(4 pi^3)   (Watson closed form, 50 digits)
  2. generator-3 invariants: Tr = G*^2/pi, Det = G*^4/(4 pi^2)  (pi-involving)
  3. t(4t-1) squarefree  (factor_list, gcd with derivative)
  4. odd valuation at (t): v_t(t(4t-1)) = 1  =>  non-square in Q(u)(t)
  5. y^2 - t(4t-1) admits no factorization over Q[y, t, u]
  6. (16G*^3)^2 = (16G*^2)^3 / 16  (the two Vieta targets are algebraically
     DEPENDENT as numbers — symbolic + 50 digits; the correct claim is
     functional independence of (Tr, Det) on M_2, item 7)
  7. companion matrix [[0, -16g^3], [1, 16g^2]] has char poly x^2 - 16g^2 x + 16g^3
     (i.e. P(x) is realizable OVER THE BASE FIELD — no obstruction to assembly)
  8. x_pm = 8G*^2 +/- 4G* delta matches the quadratic formula at 50 digits;
     delta -> -delta swaps x_+ <-> x_-, while Tr and Det are delta-free (blind)
  9. (evidence only, not proof) PSLQ finds no small integer relation among
     monomials in {G*, pi} up to total degree 3 at dps=60

What is NOT machine-checkable here and stays CONDITIONAL: Chudnovsky 1976 itself,
and the generator-representativeness of the axiom set S (FTD-0347, FLAGGED —
orthogonal to this repair).
"""

import sys
from mpmath import mp, mpf, gamma, sqrt, pi as mppi, pslq
import sympy as sp

mp.dps = 60
G = gamma(mpf(1)/4) / gamma(mpf(3)/4)
PI = +mppi
TOL = mpf(10) ** -48

checks = []
def check(name, ok, detail=""):
    checks.append((name, bool(ok)))
    print(f"  [{'PASS' if ok else 'FAIL'}] {name}" + (f" — {detail}" if detail else ""))

print("=" * 92)
print("FTD-0351 repair verification: K-BIND invariant-field enlargement Q(G*) -> Q(G*, pi)")
print(f"G* = {mp.nstr(G, 30)}")
print("=" * 92)

# 1. Watson closed form for the generator-3 scalar
print("\n1. G_BCC(0) = G*^2/(2 pi) = Gamma(1/4)^4/(4 pi^3)")
lhs = G**2 / (2*PI)
rhs = gamma(mpf(1)/4)**4 / (4*PI**3)
check("G*^2/(2pi) == Gamma(1/4)^4/(4pi^3) at 50 digits", abs(lhs - rhs) < TOL,
      f"|diff| = {mp.nstr(abs(lhs-rhs), 3)}")

# 2. generator-3 invariants involve pi
print("\n2. Generator 3 invariants (2x2 scalar operator G_BCC(0)*I)")
tr3 = 2 * lhs           # = G*^2/pi
det3 = lhs**2           # = G*^4/(4 pi^2)
check("Tr(G_BCC(0) I) = G*^2/pi", abs(tr3 - G**2/PI) < TOL, f"value {mp.nstr(tr3, 20)}")
check("Det(G_BCC(0) I) = G*^4/(4 pi^2)", abs(det3 - G**4/(4*PI**2)) < TOL,
      f"value {mp.nstr(det3, 20)}")
print("     (membership of G*^2/pi in Q(G*) would force pi in Q(G*),")
print("      contradicting Theorem 9 / FTD-0112 / OT-2.3 — hence the enlargement)")

# 3. squarefreeness of t(4t-1)
print("\n3. delta^2 corresponds to c(t) = t(4t-1) in Q(t,u); squarefreeness")
t, u, y, g, x = sp.symbols('t u y g x')
c = t*(4*t - 1)
fl = sp.factor_list(c)
sf = all(e == 1 for (_, e) in fl[1]) and sp.gcd(c, sp.diff(c, t)) == 1
check("factor_list(t(4t-1)) = [(t,1), (4t-1,1)], gcd(c, c') = 1", sf, f"{fl}")

# 4. odd valuation at (t)
print("\n4. Valuation at the prime (t) of Q(u)[t]")
v = 0
cc = sp.Poly(c, t)
while cc.eval(0) == 0:
    cc = sp.Poly(sp.cancel(cc.as_expr()/t), t)
    v += 1
check("v_(t)(t(4t-1)) = 1 (odd) => non-square in Q(u)(t)", v == 1, f"v = {v}")

# 5. no factorization of y^2 - c over Q[y,t,u]
print("\n5. Irreducibility witness over Q[y, t, u]")
fl2 = sp.factor_list(y**2 - c, y, t, u, domain='QQ')
irr = len(fl2[1]) == 1 and fl2[1][0][1] == 1
check("factor_list(y^2 - t(4t-1)) has a single factor of multiplicity 1", irr, f"{fl2}")

# 6. algebraic dependence of the Vieta targets as numbers
print("\n6. (16G*^3)^2 = (16G*^2)^3/16 — targets algebraically DEPENDENT as numbers")
sym_dep = sp.simplify((16*g**3)**2 - (16*g**2)**3/16)
check("symbolic: (16g^3)^2 - (16g^2)^3/16 == 0", sym_dep == 0, f"residual {sym_dep}")
check("numeric at 50 digits", abs((16*G**3)**2 - (16*G**2)**3/16) < mpf(10)**-30,
      f"|diff| = {mp.nstr(abs((16*G**3)**2 - (16*G**2)**3/16), 3)}")

# 7. companion matrix realizes P(x) over the base field
print("\n7. Companion matrix realizes P(x) OVER THE BASE FIELD (no assembly obstruction)")
T_W = sp.Matrix([[0, -16*g**3], [1, 16*g**2]])
cp = T_W.charpoly(x).as_expr()
target = x**2 - 16*g**2*x + 16*g**3
check("charpoly([[0,-16g^3],[1,16g^2]]) == x^2 - 16g^2 x + 16g^3",
      sp.expand(cp - target) == 0, f"charpoly = {sp.expand(cp)}")

# 8. roots, and Galois blindness of the invariants
print("\n8. Roots and the delta -> -delta swap")
delta = sqrt(G*(4*G - 1))
xp = 8*G**2 + 4*G*delta
xm = 8*G**2 - 4*G*delta
Dq = (16*G**2)**2 - 4*16*G**3
xp_q = (16*G**2 + sqrt(Dq))/2
xm_q = (16*G**2 - sqrt(Dq))/2
check("x_+ = 8G*^2 + 4G* delta matches quadratic formula (50 digits)",
      abs(xp - xp_q) < TOL, f"x_+ = {mp.nstr(xp, 25)}")
check("x_- = 8G*^2 - 4G* delta matches quadratic formula (50 digits)",
      abs(xm - xm_q) < TOL, f"x_- = {mp.nstr(xm, 25)}")
# sigma: delta -> -delta swaps the roots; Tr and Det are delta-free
xp_sigma = 8*G**2 + 4*G*(-delta)
check("sigma(x_+) = x_-  (delta -> -delta swaps roots)", abs(xp_sigma - xm) < TOL)
check("Tr = x_+ + x_- and Det = x_+ x_- are delta-free (Galois-blind)",
      abs((xp + xm) - 16*G**2) < TOL and abs(xp*xm - 16*G**3) < mpf(10)**-40)

# 9. evidence-only PSLQ: no small relation among G*-pi monomials (deg <= 3)
#    NOTE precision discipline: with 10 basis terms, dps=60 invites spurious hits
#    (a first run at dps=60/maxcoeff=1e10 returned a candidate whose residual is
#    -9.1e-45 != 0 at dps=300 — an artifact, not a relation). We therefore run at
#    dps=300 with maxcoeff=1e8, and re-verify any candidate at dps=600 before
#    counting it as a genuine relation.
print("\n9. PSLQ evidence (NOT a proof; the independence is conditional on Chudnovsky 1976)")
mp.dps = 300
G_hi = gamma(mpf(1)/4) / gamma(mpf(3)/4)
PI_hi = +mppi
basis = [mpf(1), G_hi, PI_hi, G_hi**2, G_hi*PI_hi, PI_hi**2,
         G_hi**3, G_hi**2*PI_hi, G_hi*PI_hi**2, PI_hi**3]
rel = pslq(basis, maxcoeff=10**8, maxsteps=10**6)
genuine = False
if rel is not None:
    mp.dps = 600
    G_vh = gamma(mpf(1)/4) / gamma(mpf(3)/4)
    PI_vh = +mppi
    basis_vh = [mpf(1), G_vh, PI_vh, G_vh**2, G_vh*PI_vh, PI_vh**2,
                G_vh**3, G_vh**2*PI_vh, G_vh*PI_vh**2, PI_vh**3]
    resid = sum(cf*b for cf, b in zip(rel, basis_vh))
    genuine = abs(resid) < mpf(10)**-550
    mp.dps = 300
check("no genuine integer relation among {1, G*, pi} monomials up to degree 3 "
      "(dps=300, maxcoeff=1e8, dps=600 residual guard)",
      rel is None or not genuine, f"pslq -> {rel}")

print()
print("=" * 92)
n_pass = sum(1 for _, ok in checks if ok)
print(f"RESULT: {n_pass}/{len(checks)} checks passed")
if n_pass == len(checks):
    print("All checkable algebra of the FTD-0351 field-enlargement repair VERIFIED.")
    print("Conditional (not machine-checkable): Chudnovsky 1976; generator")
    print("representativeness of S remains FLAGGED (FTD-0347) — orthogonal to this repair.")
else:
    print("AT LEAST ONE CHECK FAILED — do not cite the repair as verified.")
    sys.exit(1)
