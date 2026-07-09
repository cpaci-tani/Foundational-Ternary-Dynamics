#!/usr/bin/env python3
"""
proof_alpha_readout_fixed_point.py

P1 of the ARC-D2 self-consistency alpha-readout (targets MC-T4.3).

Analytic verification that the self-consistency map

    f(x) = 16 G*^2 (1 - G*/x) = 16 G*^2 - 16 G*^3 / x

  * has the two master-quadratic roots x_+ , x_-  as its fixed points,
  * ATTRACTS to x_+   (|f'(x_+)| < 1),
  * REPELS  from x_-  (|f'(x_-)| > 1),
  * lands on x_+ from essentially any seed (a Mobius map with distinct
    fixed points: x_+ is the global attractor, x_- the isolated repeller),

so the transcendental branch surd delta = sqrt(G*(4G*-1)) -- which
FTD-0244 proves NO native operator can realize -- is selected here by
DYNAMICAL STABILITY alone, with no algebraic branch choice.

Stage A additionally shows G* is computed NATIVELY from the finite BCC
lattice Green's function (no alpha input):  G_L(0) -> W_3 = G*^2/(2 pi).

Key structural fact (dissolves the FTD-0242 "assembly not forced" step to
a single posit):  Det = 16 G*^3 = (16 G*^2) * G* = Tr * G*.  The only
content beyond the forced trace is "the feedback coefficient is G*".

EPISTEMIC STATUS.  This script proves P1 ONLY -- the fixed-point and
stability facts, which are elementary.  It does NOT prove x_+ = 1/alpha.
Under the ARC-D2 pre-registration a PASS on P1 + the (open) engine test
P2 (feedback coefficient measured = G*, not tuned) would make
x_+ = 1/alpha a [conditional theorem given Axiom 6 (self-consistency
readout)].  Unconditionally x_+ = 1/alpha stays [STRONGLY MOTIVATED
CONJECTURE].  No epistemic tag is promoted by this script.
"""

import math
import sys

CHECKS = []


def check(name, cond, detail=""):
    CHECKS.append(bool(cond))
    flag = "PASS" if cond else "FAIL"
    print(f"  [{flag}] {name}" + (f"   {detail}" if detail else ""))


# ---------------------------------------------------------------------------
# Analytic G* -- the reflection-formula ratio.  No alpha anywhere.
# ---------------------------------------------------------------------------
G = math.gamma(0.25) / math.gamma(0.75)                # G* = Gamma(1/4)/Gamma(3/4)
TWO_PI = 2.0 * math.pi
W3_analytic = math.gamma(0.25) ** 4 / (4.0 * math.pi ** 3)   # Watson BCC integral

print("== Constants (computed from Gamma; no alpha) ==")
print(f"  G* = Gamma(1/4)/Gamma(3/4) = {G:.12f}")
print(f"  W_3 (Watson, analytic)     = {W3_analytic:.12f}")
print(f"  G*^2 / (2 pi)              = {G * G / TWO_PI:.12f}")
check("Watson identity  W_3 = G*^2/(2 pi)",
      abs(W3_analytic - G * G / TWO_PI) < 1e-12,
      f"|diff|={abs(W3_analytic - G * G / TWO_PI):.2e}")

# ---------------------------------------------------------------------------
# Master-quadratic coefficients and roots (all from G*, no alpha).
#   x^2 - 16 G*^2 x + 16 G*^3 = 0
# ---------------------------------------------------------------------------
Tr = 16.0 * G ** 2          # forced trace 16 G*^2  (16 = |O_h|/D = 48/3)
Det = 16.0 * G ** 3         # determinant  16 G*^3
disc = Tr * Tr - 4.0 * Det
xplus = 0.5 * (Tr + math.sqrt(disc))
xminus = 0.5 * (Tr - math.sqrt(disc))

print("\n== Master quadratic  x^2 - 16 G*^2 x + 16 G*^3 = 0 ==")
print(f"  Tr  = 16 G*^2 = {Tr:.9f}")
print(f"  Det = 16 G*^3 = {Det:.9f}")
print(f"  x_+ = {xplus:.9f}")
print(f"  x_- = {xminus:.9f}")
check("structural reduction  Det = Tr * G*  (one posit, not two)",
      abs(Det - Tr * G) < 1e-9, f"|diff|={abs(Det - Tr * G):.2e}")

# ---------------------------------------------------------------------------
# Stage A: G* computed natively from the finite BCC lattice Green's function.
#   G_L(0) = (1/L^3) sum_{k, 1-sigma>0} 1/(1 - cos kx cos ky cos kz) -> W_3
# ---------------------------------------------------------------------------
def bcc_green_origin(L):
    cos_tab = [math.cos(TWO_PI * j / L) for j in range(L)]
    total = 0.0
    skipped = 0
    for a in range(L):
        ca = cos_tab[a]
        for b in range(L):
            cab = ca * cos_tab[b]
            for c in range(L):
                d = 1.0 - cab * cos_tab[c]
                if d < 1e-12:               # singular modes (sigma = 1): excluded
                    skipped += 1
                    continue
                total += 1.0 / d
    return total / (L * L * L), skipped


print("\n== Stage A: native G* from the BCC lattice Green's function (no alpha) ==")
print("   finite-L values carry a leading O(1/L) surface correction; a 1/L")
print("   Richardson extrapolation removes it (G_L = W_inf - c/L).")
GLvals = {}
for L in (16, 24, 32, 48):
    GL, sk = bcc_green_origin(L)
    GLvals[L] = GL
    print(f"  L={L:3d}:  G_L(0) = {GL:.6f}   (skipped {sk} singular modes)")
L1, L2 = 32, 48
W_inf = (GLvals[L2] * L2 - GLvals[L1] * L1) / (L2 - L1)   # 1/L extrapolation
Gstar_lat = math.sqrt(TWO_PI * W_inf)
print(f"  1/L-extrapolation (L={L1},{L2}):  W_inf = {W_inf:.6f}  vs  "
      f"W_3 = {W3_analytic:.6f}")
print(f"  => native G* = sqrt(2pi W_inf) = {Gstar_lat:.6f}  vs  "
      f"Gamma-ratio G* = {G:.6f}")
check("lattice G_L(0) -> W_3 (1/L-extrapolated) within 0.1%",
      abs(W_inf - W3_analytic) / W3_analytic < 1e-3,
      f"rel={abs(W_inf - W3_analytic) / W3_analytic:.4%}")
check("native lattice G* matches Gamma-ratio G* within 0.1%",
      abs(Gstar_lat - G) / G < 1e-3, f"rel={abs(Gstar_lat - G) / G:.4%}")

# ---------------------------------------------------------------------------
# The self-consistency map and its fixed points.
# ---------------------------------------------------------------------------
def f(x):
    return Tr - Det / x                     # 16 G*^2 (1 - G*/x)


def fprime(x):
    return Det / (x * x)                     # f'(x) = 16 G*^3 / x^2


check("f(x_+) = x_+   (x_+ is a fixed point)", abs(f(xplus) - xplus) < 1e-8)
check("f(x_-) = x_-   (x_- is a fixed point)", abs(f(xminus) - xminus) < 1e-8)

# ---------------------------------------------------------------------------
# P1a: stability -- x_+ attracts, x_- repels.
# ---------------------------------------------------------------------------
print("\n== Stability of the fixed points ==")
print(f"  f'(x_+) = {fprime(xplus):.6f}    (|.| < 1  =>  attracting)")
print(f"  f'(x_-) = {fprime(xminus):.6f}   (|.| > 1  =>  repelling)")
check("x_+ attracting   |f'(x_+)| < 1", abs(fprime(xplus)) < 1.0,
      f"f'={fprime(xplus):.4f}")
check("x_- repelling    |f'(x_-)| > 1", abs(fprime(xminus)) > 1.0,
      f"f'={fprime(xminus):.4f}")

# ---------------------------------------------------------------------------
# P1b: dynamical branch selection -- iterate to convergence from many seeds.
# ---------------------------------------------------------------------------
def iterate(x0, tol=1e-13, nmax=1000):
    x = x0
    for n in range(1, nmax + 1):
        if x == 0.0:                        # pole guard (measure-zero pre-image)
            return float("nan"), n
        xn = f(x)
        if abs(xn - x) < tol:
            return xn, n
        x = xn
    return x, nmax


print("\n== Dynamical selection: iterate x_{n+1} = f(x_n) from many seeds ==")
seeds = [Tr, 4.0, 10.0, 100.0, 1000.0, -50.0, 2.0]   # natural seed = bare trace Tr
all_to_plus = True
for s in seeds:
    xstar, n = iterate(s)
    hit = abs(xstar - xplus) < 1e-6
    all_to_plus = all_to_plus and hit
    print(f"  seed = {s:10.4f}  ->  {xstar:.9f}  in {n:3d} steps   "
          f"({'x_+' if hit else '??'})")
check("all seeds converge to x_+  (never x_-)", all_to_plus)

xstar_rep, nrep = iterate(xminus + 1e-6)
check("x_- is repelling: seed (x_- + 1e-6) escapes to x_+",
      abs(xstar_rep - xplus) < 1e-6, f"-> {xstar_rep:.6f} in {nrep} steps")

# ---------------------------------------------------------------------------
# The engine hook: budget equation  x/K + G*/x = 1  at the fixed point.
#   (algebraically identical to the master quadratic with K = Tr = 16 G*^2)
# ---------------------------------------------------------------------------
budget = xplus / Tr + G / xplus
print("\n== Budget-equation form  x/K + G*/x = 1  (K = 16 G*^2) ==")
print(f"  x_+/Tr + G*/x_+ = {budget:.12f}")
check("budget equation holds at x_+  (x/K + G*/x = 1)",
      abs(budget - 1.0) < 1e-9, f"|diff|={abs(budget - 1.0):.2e}")

# ---------------------------------------------------------------------------
# NON-CIRCULARITY + physics comparison.  alpha is used NOWHERE above; it
# enters ONLY on the next two lines, as a final comparison.
# ---------------------------------------------------------------------------
ALPHA_INV_CODATA = 137.035999084            # CODATA 2022 (comparison only)
ppm = abs(xplus - ALPHA_INV_CODATA) / ALPHA_INV_CODATA * 1e6
print("\n== Physics comparison (alpha appears ONLY here) ==")
print(f"  x_+ (computed, no alpha) = {xplus:.9f}")
print(f"  1/alpha (CODATA 2022)    = {ALPHA_INV_CODATA:.9f}")
print(f"  tree-level match         = {ppm:.2f} ppm")
check("tree-level x_+ vs 1/alpha  < 2 ppm", ppm < 2.0, f"{ppm:.2f} ppm")

# ---------------------------------------------------------------------------
npass = sum(CHECKS)
ntot = len(CHECKS)
print(f"\n==== {npass}/{ntot} checks passed ====")
print("EPISTEMIC: P1 (fixed-point existence + stability + dynamical selection)")
print("verified analytically. This does NOT prove x_+ = 1/alpha; that stays")
print("[SMC] / [conditional on Axiom 6]. P2 (engine feedback coefficient")
print("measured = G*, not tuned) is the open, falsifiable test.")
sys.exit(0 if npass == ntot else 1)
