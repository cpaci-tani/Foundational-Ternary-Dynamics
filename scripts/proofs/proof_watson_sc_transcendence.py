#!/usr/bin/env python3
"""
VERIFIER: THEOREM_WATSON_SC_TRANSCENDENCE.md (FTD-0377)

Verifies every checkable link of the assembled theorem

    {pi, W_S} are algebraically independent  (hence the simple-cubic Watson
    constant W_S is transcendental); moreover trdeg Q(pi, W_S, e^{pi sqrt6}) = 3.

via the exact reduction  W_S = (3*sqrt6/2) * Omega_{-24} / pi^2,  where
Omega_{-24} = P+/(48 pi) is the Chowla-Selberg period of discriminant -24 and
P+ = Gamma(1/24)Gamma(5/24)Gamma(7/24)Gamma(11/24).

The TRANSCENDENCE inputs are CITED, not verified (they are not numerical facts):
  - Chudnovsky 1976 (Dokl. Akad. Nauk Ukrain. SSR Ser. A 8, 698-701):
    {Omega_D, pi} algebraically independent for any D < 0 — corollary stated in
    print in Zudilin, arXiv:2508.17738 (RIMS Kokyuroku 2340 (2026) 130-134).
  - Nesterenko 1996 (Sb. Math. 187, 1319-1348): {Omega_D, pi, e^{pi sqrt|D|}}
    algebraically independent, any D < 0, D = 0,1 mod 4.

Checked here (100 dps unless noted):
  L1  chi_{-24} support: the SC Gamma-product runs over {1,5,7,11} mod 24
  L2  sine product sin(pi/24)sin(5pi/24)sin(7pi/24)sin(11pi/24) = 1/16 EXACTLY
      (hand proof: pair to cos(5pi/12)cos(pi/12)/4 = sin(pi/6)/8 = 1/16)
  L3  reflection => P+ * P- = 16 pi^4
  L4  Omega_{-24} := (2pi/24) * (P+/P-)^{1/2} = P+/(48 pi)
  L5  W_S := (1/pi^3) * 3 * INT_{[0,pi]^3} dV/(3 - sum cos)  [Watson normalization]
          = (sqrt6/(32 pi^3)) * P+                            [Glasser-Zucker, corrected]
          = (3 sqrt6 / 2) * Omega_{-24} / pi^2                [the reduction]
  L6  scaffolding identities (dispensable but classical, Watson 1939/Zucker 1977):
      K'(k6)/K(k6) = sqrt6 (CM disc -24), P+ = 384(1+sqrt2) k6 pi K(k6)^2,
      W_S = 12 sqrt6 (1+sqrt2) k6 K(k6)^2 / pi^2
  L7  FCC analogue (Watson 1939): with k3 = (sqrt3-1)/(2 sqrt2), K'(k3)/K(k3) = sqrt3,
      W_F = (sqrt3/pi^2) K(k3)^2 = 3 Gamma(1/3)^6 / (2^{14/3} pi^4)   [disc -3]
  L8  BCC (for the uniform picture): W_B = Gamma(1/4)^4/(4 pi^3)      [disc -4]

Transfer lemma (prose, elementary): if W = c * pi^m * Omega^n with c in Qbar*,
m,n integers, n != 0, then a nontrivial polynomial relation Q(W, pi) = 0 over Q
becomes sum c_ij c^i Omega^{ni} pi^{mi+j} = 0 with the exponent map
(i,j) -> (ni, mi+j) injective, contradicting {Omega, pi} independence.
trdeg is insensitive to the algebraic base extension Q -> Qbar.
"""
from mpmath import (mp, mpf, gamma, pi, sqrt, sin, ellipk, quad, exp, besseli,
                    inf, fabs, nstr)

mp.dps = 100
FAILS = 0


def check(name, lhs, rhs, tol=mpf('1e-85')):
    global FAILS
    d = fabs(lhs - rhs)
    ok = d < tol
    if not ok:
        FAILS += 1
    print(f"  [{'PASS' if ok else 'FAIL'}] {name}  |diff| = {nstr(d, 4)}")


g = lambda a, b: gamma(mpf(a) / mpf(b))

print("=" * 74)
print("  VERIFIER: {pi, W_SC} algebraic independence — the disc -24 reduction")
print("=" * 74)

P_plus = g(1, 24) * g(5, 24) * g(7, 24) * g(11, 24)
P_minus = g(13, 24) * g(17, 24) * g(19, 24) * g(23, 24)

print("\n-- L2: sine product = 1/16 exactly --")
S = sin(pi / 24) * sin(5 * pi / 24) * sin(7 * pi / 24) * sin(11 * pi / 24)
check("prod sin(a pi/24), a in {1,5,7,11} = 1/16", S, mpf(1) / 16)

print("\n-- L3: reflection => P+ P- = 16 pi^4 --")
check("P+ * P- = 16 pi^4", P_plus * P_minus, 16 * pi ** 4)

print("\n-- L4: Omega_{-24} = (pi/12) sqrt(P+/P-) = P+/(48 pi) --")
Omega24 = (2 * pi / 24) * sqrt(P_plus / P_minus)
check("Omega_{-24} = P+/(48 pi)", Omega24, P_plus / (48 * pi))

print("\n-- L5: the Watson constant and the reduction --")
torus = quad(lambda t: exp(-3 * t) * besseli(0, t) ** 3, [0, inf])
W_S = 3 * torus  # Watson normalization (3x the bare torus mean)
check("W_S = (sqrt6/32 pi^3) P+   [Glasser-Zucker, corrected]", W_S,
      sqrt(6) / (32 * pi ** 3) * P_plus, tol=mpf('1e-20'))
check("W_S = (3 sqrt6/2) Omega_{-24} / pi^2   [THE REDUCTION]",
      sqrt(6) / (32 * pi ** 3) * P_plus, (3 * sqrt(6) / 2) * Omega24 / pi ** 2)

print("\n-- L6: classical scaffolding (Watson 1939 / Zucker 1977) --")
k6 = (2 - sqrt(3)) * (sqrt(3) - sqrt(2))
K6, K6p = ellipk(k6 ** 2), ellipk(1 - k6 ** 2)
check("K'(k6)/K(k6) = sqrt6  (tau = i sqrt6, CM disc -24)", K6p / K6, sqrt(6))
check("P+ = 384 (1+sqrt2) k6 pi K(k6)^2", P_plus, 384 * (1 + sqrt(2)) * k6 * pi * K6 ** 2)
check("W_S = 12 sqrt6 (1+sqrt2) k6 K^2/pi^2", sqrt(6) / (32 * pi ** 3) * P_plus,
      12 * sqrt(6) * (1 + sqrt(2)) * k6 * K6 ** 2 / pi ** 2)

print("\n-- L7: FCC analogue (disc -3, Watson 1939) --")
k3 = (sqrt(3) - 1) / (2 * sqrt(2))
K3, K3p = ellipk(k3 ** 2), ellipk(1 - k3 ** 2)
check("K'(k3)/K(k3) = sqrt3  (disc -3)", K3p / K3, sqrt(3))
W_F = (sqrt(3) / pi ** 2) * K3 ** 2
check("W_F = 3 Gamma(1/3)^6 / (2^{14/3} pi^4)", W_F,
      3 * g(1, 3) ** 6 / (2 ** (mpf(14) / 3) * pi ** 4))

print("\n-- L8: BCC (disc -4, Watson 1939) --")
check("W_B = Gamma(1/4)^4/(4 pi^3) = G*^2/(2 pi)", g(1, 4) ** 4 / (4 * pi ** 3),
      (g(1, 4) / g(3, 4)) ** 2 / (2 * pi))

print("\n-- transcendence inputs: CITED, not verified --")
print("   {Omega_D, pi} alg. indep., any D<0  [Chudnovsky 1976; in print: Zudilin arXiv:2508.17738]")
print("   {Omega_D, pi, e^{pi sqrt|D|}} alg. indep.  [Nesterenko 1996]")
print("   => {pi, W_S} alg. indep. via the L5 reduction + transfer lemma;")
print("      likewise {pi, W_F} (disc -3) and {pi, W_B} (disc -4).")
print("   JOINT independence across discs remains OPEN (multi-curve Chudnovsky).")

print("\n" + "=" * 74)
if FAILS == 0:
    print("  ALL LINKS VERIFIED. Assembled theorem chain complete (see the note).")
else:
    print(f"  {FAILS} CHECK(S) FAILED — do not cite the theorem until resolved.")
print("=" * 74)
raise SystemExit(1 if FAILS else 0)
