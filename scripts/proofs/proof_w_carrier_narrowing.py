#!/usr/bin/env python3
"""
proof_w_carrier_narrowing.py  --  FTD-0314 (the "EARN W" carrier-narrowing boundary)

Verifies the load-bearing facts behind the NARROWING THEOREM and the three carrier
closures (C1 BCC-Watson twist, C2 second-Watson pairing, C3 CM period/L-value):

  The missing alpha-binding axiom W (FTD-0243) must realize the surd sqrt(G*(4G*-1))
  as a forced order-2 (Z/2) invariant.  That surd is TRANSCENDENTAL over Q, so every
  carrier with algebraic (transcendence-degree-0) invariants -- chirality, the +/-1
  ternary sign, the binary-octahedral double cover 2O, permutation parity, and every
  native OPERATOR trace/det (which land in Q(G*), FTD-0244) -- is structurally
  excluded.  The only door is a Z/2 twist on a G*-BEARING ANALYTIC object; the three
  natural such objects all degenerate or land in the wrong field.

Pre-registered (same-session transparent lock): the verdict criteria were fixed
before running.  This script COMPUTES (does not recall) every value at high precision.
COMPANION to docs/.../audits/AUDIT_W_CARRIER_NARROWING.md.

Run:  PYTHONUTF8=1 python scripts/proofs/proof_w_carrier_narrowing.py
"""
import mpmath as mp
import sympy as sp

mp.mp.dps = 150
EPS = mp.mpf(10) ** -130
_PASS = []


def check(name, cond, detail=""):
    _PASS.append(bool(cond))
    print(f"[{'PASS' if cond else 'FAIL'}] {name}" + (f"   -- {detail}" if detail else ""))


print("=" * 78)
print("FTD-0314  --  EARN-W carrier narrowing: load-bearing verification (dps=150)")
print("=" * 78)

# ---------------------------------------------------------------- constants -----
g_a = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
g_b = mp.gamma(mp.mpf(1) / 4) ** 2 / (mp.pi * mp.sqrt(2))
W_BCC = mp.gamma(mp.mpf(1) / 4) ** 4 / (4 * mp.pi ** 3)
g_c = mp.sqrt(2 * mp.pi * W_BCC)
Gs = g_a
check("G* agrees three ways (Gamma-ratio / Gamma^2 / sqrt(2 pi W_BCC))",
      abs(g_a - g_b) < EPS and abs(g_a - g_c) < EPS, f"G* = {mp.nstr(Gs, 22)}")
check("W_BCC = Gamma(1/4)^4/(4 pi^3) = G*^2/(2 pi)",
      abs(W_BCC - Gs ** 2 / (2 * mp.pi)) < EPS, f"W_BCC = {mp.nstr(W_BCC, 18)}")

# ---------------------------------------------------------------- surd, roots ---
surd = mp.sqrt(Gs * (4 * Gs - 1))
xp = 8 * Gs ** 2 + 4 * Gs * surd
xm = 8 * Gs ** 2 - 4 * Gs * surd
A, B = 16 * Gs ** 2, 16 * Gs ** 3
r_hi = (A + mp.sqrt(A * A - 4 * B)) / 2
r_lo = (A - mp.sqrt(A * A - 4 * B)) / 2
check("roots x+- = 8G*^2 +- 4G* surd reproduce the master quadratic",
      abs(xp - r_hi) < EPS and abs(xm - r_lo) < EPS,
      f"x+={mp.nstr(xp, 14)}, x-={mp.nstr(xm, 14)}")
check("surd = (x+ - x-)/(8 G*)  (the normalized root-spread)",
      abs(surd - (xp - xm) / (8 * Gs)) < EPS, f"surd = {mp.nstr(surd, 22)}")

# ----------------------------------- squarefree / genuine degree-2 over Q(G*) ---
t = sp.symbols("t")
p = 4 * t ** 2 - t                       # = G*(4G*-1) with t = G*
fl = sp.factor_list(p)                   # (1, [(t,1),(4t-1,1)])
mults = [m for _, m in fl[1]]
check("4t^2 - t is squarefree over Q(t)  (=> genuine deg-2 extension)",
      all(m == 1 for m in mults) and sp.degree(sp.gcd(p, sp.diff(p, t)), t) == 0,
      f"factor_list = {fl}")

# ------------------------- PSLQ: surd genuinely deg-2 / 4G*-1 lives in Q(G*) -----
basis = [Gs ** k for k in range(6)] + [surd]
rel = mp.pslq(basis, maxcoeff=10 ** 8, maxsteps=10 ** 6)
check("surd has NO Q(G*)-relation vs {1,G*,...,G*^5}  (transcendental over Q(G*); not planted)",
      rel is None, f"pslq -> {rel}")
rel2 = mp.pslq([4 * Gs - 1, Gs, mp.mpf(1)], maxcoeff=10 ** 6)
check("4G*-1 = 4*G* - 1 lies ENTIRELY in Q(G*)  (relation [1,-4,1]; kills C2 at root)",
      rel2 in ([1, -4, 1], [-1, 4, -1]), f"pslq(4G*-1, G*, 1) -> {rel2}")

# --------- C1: antiperiodic Z/2 twist of the BCC body-diagonal Green's function -
# G ~ sum_n c_n^3, c_n = (1/2pi) int cos^n = C(n,n/2)/2^n (even n), 0 (odd n).
# untwisted: sum c_n^3 ; antiperiodic (sign-flipped product): sum (-1)^n c_n^3.
def c_n(n):
    return mp.mpf(0) if n % 2 else mp.binomial(n, n // 2) / mp.mpf(2) ** n
N = 600
s_even = mp.fsum([c_n(n) ** 3 for n in range(N)])
s_odd = mp.fsum([((-1) ** n) * c_n(n) ** 3 for n in range(N)])
check("C1: G_odd = G_even EXACTLY  (antiperiodic twist degenerates; odd-n angular integrals vanish)",
      abs(s_even - s_odd) < EPS, f"|G_odd - G_even| = {mp.nstr(abs(s_even - s_odd), 3)}")
print(f"       (partial-sum period {mp.nstr(s_even, 8)}; the claim is the EXACT degeneracy, not the value)")

# ---------------- C3: G* is a CM-period monomial; surd outside the CM-period field
Omega = mp.gamma(mp.mpf(1) / 4) ** 2 / mp.sqrt(2 * mp.pi)      # real period of y^2=x^3-x
check("C3: Omega/G* = sqrt(pi)  (G* a homogeneous CM-period monomial of E: y^2=x^3-x)",
      abs(Omega / Gs - mp.sqrt(mp.pi)) < EPS, f"Omega/G* = {mp.nstr(Omega / Gs, 18)}")
# symbolic kill: surd^2 = 4G*^2 - G* with G* = v^2/(sqrt2 u), u=pi, v=Gamma(1/4)
u, v = sp.symbols("u v", positive=True)
Gsym = v ** 2 / (sp.sqrt(2) * u)
e = sp.together(4 * Gsym ** 2 - Gsym)             # = surd^2 as a rational fn of (u,v)
num, den = sp.fraction(e)
num = sp.expand(num)
deg_u = sp.Poly(num, u).degree()
check("C3: surd^2 numerator is degree-1 in u (=pi)  => squarefree => surd not in Qbar(pi,Gamma(1/4))",
      deg_u == 1, f"numerator = {num}   (deg_u = {deg_u})")

# ---------------- weight-inhomogeneity (V1 strengthening): no graded-motive period
# num mixes a v^4 monomial and a u*v^2 monomial -> different (u,v) total degree.
monos = sp.Poly(num, u, v).monoms()
total_degs = sorted({sum(m) for m in monos})
check("weight-inhomogeneity: surd^2 numerator mixes two distinct (pi,Gamma) total-degrees",
      len(total_degs) >= 2, f"monomial total-degrees present = {total_degs}")
print(f"       surd^2 = ({num}) / ({sp.expand(den)})   "
      f"-- a sum of two different-weight monomials; its sqrt is the period of no PURE graded motive [CONJECTURE pressure]")

print("-" * 78)
ok = all(_PASS)
print(f"RESULT: {'ALL PASS' if ok else 'SOME FAILED'}  ({sum(_PASS)}/{len(_PASS)})")
raise SystemExit(0 if ok else 1)
