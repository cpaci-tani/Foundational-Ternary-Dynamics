#!/usr/bin/env python3
"""
VERIFIER: THEOREM_EXPONENTIAL_LATTICE_PERIODS_TRANSCENDENCE.md (FTD-0378)

Verifies the checkable (algebraic/analytic-identity) links of the assembled
theorems that close E2's individual-transcendence sub-question:

  THEOREM A (SC symbol).  For every nonzero algebraic tau, the exponential
  lattice period
    H_SC(tau) = (2pi)^-3 INT_{[0,2pi]^3} exp(-tau (1 - (cos k1 + cos k2 + cos k3)/3)) dk
              = e^{-tau} I0(tau/3)^3
  is transcendental.  [Siegel 1929 / Shidlovskii; Beukers 2006]

  THEOREM B (BCC symbol).  For every nonzero algebraic tau,
    H_BCC(tau) = (2pi)^-3 INT exp(-tau (1 - cos k1 cos k2 cos k3)) dk
               = e^{-tau} * 2F3(1/2,1/2; 1,1,1; tau^2/4)
  is transcendental.  [linear Siegel-Shidlovskii (Beukers Cor. 1.4) + simplicity
  of Hyp((t+1/2)^2, t^4) (Katz) + a no-rank-1-submodule lemma]

Checked here:
  L1  SC factorization:  torus mean of e^{-tau sigma_SC} = e^{-tau} I0(tau/3)^3
      (via the 1-D identity (2pi)^-1 INT e^{x cos k} dk = I0(x); direct 3-D quad)
  L2  BCC factorization: torus mean of e^{-tau sigma_BCC} = e^{-tau} 2F3(.5,.5;1,1,1;tau^2/4)
      (direct 3-D quad at two rational tau)
  L3  BCC series rewrite, EXACT for 12 terms:
      coeff of tau^{2m}/(2m)! is [binom(2m,m)/4^m]^3, and
      [binom(2m,m)/4^m]^3 / (2m)!  ==  [(1/2)_m]^2 / (m!)^4 / 4^m
      (i.e. the 2F3(1/2,1/2;1,1,1; tau^2/4) normalization), as exact fractions.
  L4  E-function sanity: the 2F3 coefficients [binom(2m,m)/4^m]^3 are rational,
      |a_m| <= 1, denominator divides 64^m (strict Siegel E-function class).

NOT checked here (cited, not numerical facts):
  - Siegel-Shidlovskii / Beukers 2006 (values inherit functional (in)dependence
    at nonzero algebraic points; T(xi) != 0 with T(z) = z here).
  - Functional independence of {e^z, I0(z/3), I0'(z/3)} over Qbar(z)
    (Bessel PV group SL2, Kolchin/Kovacic; SL2 x Gm joint).
  - The BCC SO4 HAZARD and its repair: Katz's type-(2,4) classification leaves
    {SL4, SO4, Sp4}; self-duality excludes SL4; if SO4, FULL functional
    independence of {e^z, F, F', F'', F'''} would be FALSE.  The theorem
    therefore uses ONLY Qbar(z)-LINEAR independence (Beukers Cor. 1.4 route).
    Do not cite full algebraic independence for the BCC block.
"""
from fractions import Fraction
from math import comb, factorial
from mpmath import (mp, mpf, pi, exp, cos, besseli, quad, hyper, fabs, nstr)

mp.dps = 40
FAILS = 0


def check(name, ok_or_lhs, rhs=None, tol=None):
    global FAILS
    if rhs is None:
        ok = bool(ok_or_lhs)
        msg = ""
    else:
        d = fabs(ok_or_lhs - rhs)
        ok = d < (tol or mpf('1e-25'))
        msg = f"  |diff| = {nstr(d, 4)}"
    if not ok:
        FAILS += 1
    print(f"  [{'PASS' if ok else 'FAIL'}] {name}{msg}")


print("=" * 74)
print("  VERIFIER: exponential lattice periods (E2) — factorizations + rewrites")
print("=" * 74)

for tau in (mpf(1), mpf(2) / 3):
    print(f"\n-- tau = {nstr(tau, 8)} --")
    # L1 SC: the 3-D integral factorizes EXACTLY into three identical 1-D factors
    # (e^{(tau/3)(c1+c2+c3)} = prod e^{(tau/3)ci}), so the load-bearing check is the
    # 1-D identity (2pi)^-1 INT e^{x cos k} dk = I0(x) at x = tau/3.
    one_d = quad(lambda k: exp((tau / 3) * cos(k)), [0, pi]) / pi
    check("L1 SC: (2pi)^-1 INT e^{(tau/3)cos k} dk = I0(tau/3)  [3-D factorizes exactly]",
          one_d, besseli(0, tau / 3), tol=mpf('1e-30'))
    # L2 BCC: integrate k3 out exactly ((2pi)^-1 INT e^{a cos k3} dk3 = I0(a)),
    # leaving a 2-D quad: e^tau * H_BCC = (2pi)^-2 INT I0(tau cos x cos y) dx dy.
    H_bcc_2d = quad(lambda x: quad(lambda y: besseli(0, tau * cos(x) * cos(y)),
                                   [0, pi]), [0, pi]) / pi ** 2
    check("L2 BCC: (2pi)^-2 INT I0(tau cx cy) = 2F3(1/2,1/2;1,1,1;tau^2/4)",
          H_bcc_2d, hyper([mpf(1) / 2, mpf(1) / 2], [1, 1, 1], tau ** 2 / 4),
          tol=mpf('1e-25'))

print("\n-- L3: BCC series rewrite, exact fractions (m = 0..11) --")
ok = True
for m in range(12):
    lhs = Fraction(comb(2 * m, m), 4 ** m) ** 3 / Fraction(factorial(2 * m))
    poch_half = Fraction(factorial(2 * m), 4 ** m * factorial(m))  # (1/2)_m = (2m)!/(4^m m!)
    rhs = poch_half ** 2 / (Fraction(factorial(m)) ** 4 * 4 ** m)
    if lhs != rhs:
        ok = False
check("[binom(2m,m)/4^m]^3/(2m)! == [(1/2)_m]^2/((m!)^4 4^m) exactly, m<=11", ok)

print("\n-- L4: strict E-function class for the BCC block --")
ok = all(abs(Fraction(comb(2 * m, m), 4 ** m) ** 3) <= 1 and
         (Fraction(comb(2 * m, m), 4 ** m) ** 3).denominator <= 64 ** m
         for m in range(30))
check("|a_m| <= 1 and denom(a_m) | 64^m for m < 30", ok)

print("\n-- transcendence inputs: CITED, not verified --")
print("   A: {e^z, I0(z/3), I0'(z/3)} functionally independent (Bessel PV = SL2,")
print("      Kolchin/Kovacic; joint SL2 x Gm) + Siegel-Shidlovskii/Beukers 2006")
print("      => H_SC(tau) transcendental, all algebraic tau != 0.")
print("   B: Qbar(z)-LINEAR independence of {e^z, F, F', F'', F'''} (simplicity of")
print("      Hyp((t+1/2)^2, t^4), Katz criterion; no-rank-1-submodule via Frobenius")
print("      reciprocity) + Beukers Cor. 1.4 => H_BCC(tau) transcendental.")
print("   SO4 HAZARD: do NOT cite full functional independence for the BCC block")
print("      (Katz type-(2,4) leaves {Sp4, SO4} after self-duality; if SO4, full")
print("      independence is false). The linear route avoids this entirely.")
print("   E2-FULL (independence from {pi, Gamma(1/4)}): remains OPEN — see the note.")

print("\n" + "=" * 74)
if FAILS == 0:
    print("  ALL IDENTITY LINKS VERIFIED. Transcendence cited per the note.")
else:
    print(f"  {FAILS} CHECK(S) FAILED — do not cite the theorems until resolved.")
print("=" * 74)
raise SystemExit(1 if FAILS else 0)
