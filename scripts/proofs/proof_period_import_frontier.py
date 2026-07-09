#!/usr/bin/env python3
"""
VERIFIER: MATH_PERIOD_IMPORT_FRONTIER.md  (FTD-0375)

Checks the ALGEBRAIC identities of the period-conjecture import-frontier framing.
Transcendence facts (A4: GPC-for-E_lemn via Chudnovsky; A5: delta transcendental)
are CITED to Chudnovsky 1976, NOT verified numerically -- transcendence is not a
numerical property, and NO near-miss / coincidence search is performed here.

Identities checked (60 digits):
  A1  G* = Gamma(1/4)/Gamma(3/4) = Gamma(1/4)^2/(pi*sqrt2) = 2*varpi/sqrt(pi)
  A2  varpi (Gamma formula) = 2*Integral_0^1 dt/sqrt(1-t^4)      [varpi is a period]
  A3  G*^2 = 4*varpi^2/pi                                        [G*^2 in P[1/pi]]
  A5  delta^2 = 4*G*^2 - G*     (delta = sqrt(G*(4G*-1)))        [delta^2 in Q(G*)]
      x+ = 8*G*^2 + 4*G*delta   (delta is the discriminant surd of the master quad)
      master-quadratic residual  x+^2 - 16 G*^2 x+ + 16 G*^3 = 0
"""
from mpmath import mp, mpf, pi, gamma, sqrt, quad, fabs
mp.dps = 60

FAILS = 0


def check(name, lhs, rhs, tol=mpf('1e-50')):
    global FAILS
    d = fabs(lhs - rhs)
    ok = d < tol
    if not ok:
        FAILS += 1
    print(f"  [{'PASS' if ok else 'FAIL'}] {name}: |diff| = {mp.nstr(d, 3)}")


print("=" * 72)
print("  VERIFIER: MATH_PERIOD_IMPORT_FRONTIER (FTD-0375)")
print("=" * 72)

G14 = gamma(mpf(1) / 4)
G34 = gamma(mpf(3) / 4)
varpi = G14**2 / (2 * sqrt(2 * pi))
G = G14 / G34

print("\n-- A1: G* elementary tower [THEOREM, elementary] --")
check("G* = Gamma(1/4)^2/(pi*sqrt2)", G, G14**2 / (pi * sqrt(2)))
check("G* = 2*varpi/sqrt(pi)", G, 2 * varpi / sqrt(pi))
print(f"    G*    = {mp.nstr(G, 40)}")
print(f"    varpi = {mp.nstr(varpi, 40)}   (G* != varpi, FTD-0117)")
assert fabs(G - varpi) > mpf('0.3'), "G* must differ from varpi"

print("\n-- A2: varpi is a period (Gamma formula == lemniscate integral) --")
lemn = 2 * quad(lambda t: 1 / sqrt(1 - t**4), [0, 1])
check("varpi = 2*Int_0^1 dt/sqrt(1-t^4)", varpi, lemn, tol=mpf('1e-20'))

print("\n-- A3: G*^2 = 4 varpi^2 / pi  (in P[1/pi]) --")
check("G*^2 = 4 varpi^2/pi", G**2, 4 * varpi**2 / pi)

print("\n-- A5: delta and the discriminant surd --")
delta = sqrt(G * (4 * G - 1))
check("delta^2 = 4 G*^2 - G*", delta**2, 4 * G**2 - G)
xplus = 8 * G**2 + 4 * G * delta
xplus_quadformula = (16 * G**2 + sqrt((16 * G**2)**2 - 4 * 16 * G**3)) / 2
check("x+ = 8 G*^2 + 4 G* delta (= larger root)", xplus, xplus_quadformula)
resid = xplus**2 - 16 * G**2 * xplus + 16 * G**3
check("master-quadratic residual = 0", resid, mpf(0), tol=mpf('1e-45'))
print(f"    x+    = {mp.nstr(xplus, 40)}")
print(f"    1/x+  = {mp.nstr(1 / xplus, 40)}   (compare 1/alpha)")

print("\n-- Transcendence: CITED to Chudnovsky 1976, NOT verified --")
print("  A4  trdeg_Q(period field of h^1(E_lemn)) = dim G_mot = 2")
print("      upper bound automatic (period torsor); lower bound = Chudnovsky 1976")
print("      (alg. independence of Gamma(1/4), pi). The CM instance of GPC.")
print("  A5  delta transcendental over Q  <=  Chudnovsky 1976 (via G* transcendental).")
print("      delta NOT-in N: delivered by FTD-0369/0370, conditional on")
print("      Chudnovsky (E0, proven) PLUS open E1/E2 -- NOT Chudnovsky alone.")
print("  These are not numerical facts and are deliberately not asserted here.")

print("\n" + "=" * 72)
if FAILS == 0:
    print("  ALL ALGEBRAIC IDENTITIES PASS. Transcendence cited to Chudnovsky 1976.")
else:
    print(f"  {FAILS} IDENTITY CHECK(S) FAILED.")
print("=" * 72)
raise SystemExit(1 if FAILS else 0)
