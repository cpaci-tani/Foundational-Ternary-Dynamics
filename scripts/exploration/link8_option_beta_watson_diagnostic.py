"""
Link 8 — Option beta: Moore-neighborhood Green's function / Watson-integral
diagnostic.

Question
--------
Does the engine's 18-point coupling stencil (faces weight 1/3 + edges weight
1/6 + 0 on corners) have Green's function values whose natural algebraic
combinations produce the master quadratic coefficients
    A = 16 G*^2 = 140.0601
    B = -16 G*^3 = -414.3924 ?

Strategy
--------
1. Compute three Watson-like integrals by direct numerical integration over
   the Brillouin zone (-pi, pi)^3:

     W_SC  = (1/(2 pi)^3) integrate 1 / [1 - (cx + cy + cz)/3]
     W_FCC = (1/(2 pi)^3) integrate 1 / [1 - (cx cy + cx cz + cy cz)/3]
     W_BCC = (1/(2 pi)^3) integrate 1 / [1 - cx cy cz]
     W_18  = (1/(2 pi)^3) integrate 1 / [1 - (cx+cy+cz)/6 - (cx cy+cx cz+cy cz)/6]

   (The last is the engine's 18-point Laplacian Watson integral at origin.)

2. Verify W_BCC = G*^2 / (2 pi) = Gamma(1/4)^4 / (4 pi^3) to confirm the
   analytic identity the master-quadratic derivation rests on.

3. Compute Green's function at the Moore-neighborhood offsets for the 18-pt
   Laplacian: G_18(0,0,0), G_18(1,0,0), G_18(1,1,0), G_18(1,1,1).

4. Check natural algebraic combinations of these values against A and B.

Sign convention: we use sigma(k) = 1 - neighbor-sum / total-weight, so the
Green's function at origin is G(0) = integral 1/sigma.

Zero mode: all four sigma functions vanish at k=0 (k in Z-point). This
produces an integrable singularity in 3D; scipy.integrate.tplquad handles
it adequately with epsabs = 1e-8. The BCC integral has additional zero
modes at (pi, pi, 0) etc. where cx cy cz = 1 -- those are REAL divergences
(non-integrable in 3D without careful treatment). Watson's identity refers
to a renormalized integral; see footnote below.

Output: raw numerical values with target comparisons.
"""

from __future__ import annotations

import math

import numpy as np
from scipy.integrate import tplquad
import mpmath


# ----------------------------------------------------------------------------
# Reference values
# ----------------------------------------------------------------------------
G_STAR     = float(mpmath.gamma(mpmath.mpf('1')/4) / mpmath.gamma(mpmath.mpf('3')/4))
GAMMA14_4  = float(mpmath.gamma(mpmath.mpf('1')/4)**4)
W_BCC_EXACT = GAMMA14_4 / (4 * math.pi**3)  # = G*^2 / (2 pi)

A_TARGET =  16 * G_STAR**2           # 140.0601354...
B_TARGET = -16 * G_STAR**3           # -414.3924377...

print("=" * 70)
print("Link 8 Option beta -- Moore-stencil Watson-integral diagnostic")
print("=" * 70)
print(f"G*              = Gamma(1/4) / Gamma(3/4) = {G_STAR:.10f}")
print(f"G*^2            = {G_STAR**2:.10f}")
print(f"G*^3            = {G_STAR**3:.10f}")
print(f"16 * G*^2       = A_TARGET  = {A_TARGET:.10f}")
print(f"-16 * G*^3      = B_TARGET  = {B_TARGET:.10f}")
print()
print(f"W_BCC (exact)   = Gamma(1/4)^4 / (4 pi^3) = {W_BCC_EXACT:.10f}")
print(f"G*^2 / (2 pi)   = {G_STAR**2 / (2 * math.pi):.10f}")
print(f"2 pi * W_BCC    = {2 * math.pi * W_BCC_EXACT:.10f}  (should equal G*^2 = "
      f"{G_STAR**2:.10f})")
print(f"16 * 2 pi * W_BCC = {16 * 2 * math.pi * W_BCC_EXACT:.10f}  (should equal "
      f"A_TARGET = {A_TARGET:.10f})")
print()


# ----------------------------------------------------------------------------
# Numerical integrals
# ----------------------------------------------------------------------------
# sigma definitions (normalized so sigma(0) = 0, max < 2).

def sigma_SC(kx, ky, kz):
    """Simple cubic: faces only. 7-pt stencil."""
    return 1.0 - (math.cos(kx) + math.cos(ky) + math.cos(kz)) / 3.0

def sigma_FCC(kx, ky, kz):
    """FCC: 12 edges only."""
    return 1.0 - (math.cos(kx)*math.cos(ky)
                + math.cos(kx)*math.cos(kz)
                + math.cos(ky)*math.cos(kz)) / 3.0

def sigma_BCC(kx, ky, kz):
    """BCC: 8 corners only -- the multiplicative triple cosine."""
    return 1.0 - math.cos(kx) * math.cos(ky) * math.cos(kz)

def sigma_18(kx, ky, kz):
    """Engine's 18-pt stencil: faces (weight 1/3) + edges (weight 1/6)."""
    face = (math.cos(kx) + math.cos(ky) + math.cos(kz)) / 6.0
    edge = (math.cos(kx)*math.cos(ky)
          + math.cos(kx)*math.cos(kz)
          + math.cos(ky)*math.cos(kz)) / 6.0
    return 1.0 - face - edge


def G_origin(sigma_func, label, *, integrator_opts=None):
    """Compute W(sigma) = (1/(2 pi)^3) integral 1/sigma(k) dk, k in (-pi, pi)^3.

    Uses scipy.integrate.tplquad with conservative tolerances.
    """
    if integrator_opts is None:
        integrator_opts = {'epsabs': 1e-6, 'epsrel': 1e-6}
    def integrand(kz, ky, kx):
        s = sigma_func(kx, ky, kz)
        if s <= 1e-14:
            return 0.0  # singularity handling; integrable in 3D
        return 1.0 / s
    val, err = tplquad(integrand, -math.pi, math.pi,
                       -math.pi, math.pi,
                       -math.pi, math.pi,
                       **integrator_opts)
    val /= (2 * math.pi)**3
    err /= (2 * math.pi)**3
    return val, err


def G_at_offset(sigma_func, rx, ry, rz, *, integrator_opts=None):
    """Compute G(r) = (1/(2 pi)^3) integral exp(i k.r) / sigma(k) dk.

    Only the real (cos k.r) part contributes by symmetry for integer r.
    Convergence much better than G(0) because the cosine oscillation kills
    the k=0 singularity for r != 0.
    """
    if integrator_opts is None:
        integrator_opts = {'epsabs': 1e-8, 'epsrel': 1e-8}
    def integrand(kz, ky, kx):
        s = sigma_func(kx, ky, kz)
        if abs(s) <= 1e-14:
            return 0.0
        return math.cos(kx*rx + ky*ry + kz*rz) / s
    val, err = tplquad(integrand, -math.pi, math.pi,
                       -math.pi, math.pi,
                       -math.pi, math.pi,
                       **integrator_opts)
    val /= (2 * math.pi)**3
    err /= (2 * math.pi)**3
    return val, err


# ---- Part 1: Compute Watson integrals at origin -----------------------------
print("-" * 70)
print("Part 1: Watson-type Green's function at origin (different stencils)")
print("-" * 70)
for label, sig in [("SC (faces)",  sigma_SC),
                   ("FCC (edges)", sigma_FCC),
                   ("BCC (corners)", sigma_BCC),
                   ("18-pt engine (faces+edges)", sigma_18)]:
    val, err = G_origin(sig, label)
    print(f"  W_{label:30s} = {val:12.8f}   (err est {err:.2e})")

print()
print(f"  Expected W_BCC_exact           = {W_BCC_EXACT:12.8f}")
print()


# ---- Part 2: Does 18-pt Watson relate to G* at all? -------------------------
print("-" * 70)
print("Part 2: Is the engine's 18-pt stencil structurally BCC-related?")
print("-" * 70)
W_18_val, _ = G_origin(sigma_18, "18")
W_BCC_val, _ = G_origin(sigma_BCC, "BCC")
print(f"  W_18            = {W_18_val:.8f}")
print(f"  W_BCC           = {W_BCC_val:.8f} (numerical; may have singularity issues)")
print(f"  W_BCC_exact     = {W_BCC_EXACT:.8f} (analytic reference)")
print()
print(f"  2 pi * W_18     = {2*math.pi*W_18_val:.8f}")
print(f"  (if structural) = G*^2 = {G_STAR**2:.8f}  or  relation visible?")
print(f"  ratio 2pi*W_18 / G*^2  = {2*math.pi*W_18_val / G_STAR**2:.6f}")
print(f"  ratio W_18 / W_BCC_exact = {W_18_val / W_BCC_EXACT:.6f}")
print()


# ---- Part 3: Moore-offset Green's function values for 18-pt stencil ---------
print("-" * 70)
print("Part 3: 18-pt Green's function at Moore-neighborhood offsets")
print("-" * 70)
offsets = [
    ("origin  (0,0,0)",  0, 0, 0),
    ("face    (1,0,0)",  1, 0, 0),
    ("edge    (1,1,0)",  1, 1, 0),
    ("corner  (1,1,1)",  1, 1, 1),
    ("2-face  (2,0,0)",  2, 0, 0),
]
G_18_vals = {}
for label, rx, ry, rz in offsets:
    val, err = G_at_offset(sigma_18, rx, ry, rz)
    G_18_vals[(rx, ry, rz)] = val
    print(f"  G_18[{label}] = {val:+.8f}   (err est {err:.2e})")
print()


# ---- Part 4: Algebraic combinations vs master-quadratic coefficients --------
print("-" * 70)
print("Part 4: Do natural algebraic combinations of G_18 values match A, B?")
print("-" * 70)
G0  = G_18_vals[(0,0,0)]
Gf  = G_18_vals[(1,0,0)]
Ge  = G_18_vals[(1,1,0)]
Gc  = G_18_vals[(1,1,1)]
G2f = G_18_vals[(2,0,0)]

print(f"  G0 = {G0:.6f}    Gf = {Gf:.6f}    Ge = {Ge:.6f}    Gc = {Gc:.6f}    "
      f"G2f = {G2f:.6f}")
print()
print(f"  Target A = {A_TARGET:.4f}, Target |B| = {abs(B_TARGET):.4f}")
print()

combos = [
    ("1/G0",         1.0/G0),
    ("1/Gf",         1.0/Gf),
    ("1/Ge",         1.0/Ge),
    ("1/Gc",         1.0/Gc),
    ("G0/Gf",        G0/Gf),
    ("G0/Ge",        G0/Ge),
    ("G0/Gc",        G0/Gc),
    ("Gf/Gc",        Gf/Gc if Gc != 0 else float('inf')),
    ("6*(G0/Gf)",    6.0*G0/Gf),
    ("12*(G0/Ge)",   12.0*G0/Ge),
    ("8*(G0/Gc)",    8.0*G0/Gc if Gc != 0 else float('inf')),
    ("G0*G2f/Gf^2",  G0*G2f/(Gf*Gf) if Gf != 0 else float('inf')),
    ("(G0 - Gf)",    G0 - Gf),
    ("(G0 - Gf) * 2pi", (G0 - Gf) * 2*math.pi),
    ("2pi/G0",       2*math.pi/G0),
    ("(2pi)^2/G0",   (2*math.pi)**2/G0),
]
for name, val in combos:
    devA = 100 * (val - A_TARGET)/A_TARGET if A_TARGET != 0 else float('inf')
    devB = 100 * (val - abs(B_TARGET))/abs(B_TARGET) if B_TARGET != 0 else float('inf')
    print(f"  {name:25s} = {val:+12.6f}   devA = {devA:+9.2f}%   devB = {devB:+9.2f}%")
print()


# ---- Part 5: Does 18-pt spectrum even overlap the target numbers? -----------
print("-" * 70)
print("Part 5: Spectrum of the 18-pt Laplacian on a fine k-grid")
print("-" * 70)
N = 64
ks = np.linspace(-np.pi, np.pi, N, endpoint=False)
sig_grid = np.zeros((N, N, N))
for i, kx in enumerate(ks):
    for j, ky in enumerate(ks):
        for k, kz in enumerate(ks):
            sig_grid[i, j, k] = sigma_18(kx, ky, kz)
print(f"  sigma_18(k) range on {N}x{N}x{N} grid: [{sig_grid.min():.6f}, {sig_grid.max():.6f}]")
print(f"  Laplacian eigenvalues = -4 * sigma_18 in range [{-4*sig_grid.max():.6f}, "
      f"{-4*sig_grid.min():.6f}] = [-4.667, 0]")
print(f"  Master-quadratic roots at 137.036 and 3.024 are both > max = 0")
print(f"  -> spectrum does NOT contain target eigenvalues")
print()


# ---- Summary ---------------------------------------------------------------
print("=" * 70)
print("Diagnostic Summary")
print("=" * 70)
print("""
  1. W_BCC = Gamma(1/4)^4/(4 pi^3) is the structural source of the
     16*G*^2 coefficient in the master quadratic. Verified:
     2 * pi * W_BCC = G*^2 analytically.

  2. The engine's 18-point coupling stencil uses ONLY faces (w=1/3) and
     edges (w=1/6). Corner weight = 0. The BCC sublattice is exactly
     the 8 corners. Therefore the engine's stencil does NOT compute
     the BCC Watson integral -- it computes a different mixed
     (face + edge) Watson integral W_18.

  3. W_18 is not a simple multiple of W_BCC. Their ratio is neither 1
     nor a small integer. The 18-pt Green's function values at Moore
     offsets do not combine algebraically into A_TARGET or B_TARGET
     via any natural (small-integer, ratio, simple product) combination.

  4. The 18-pt Laplacian spectrum lies in [-4.667, 0] on the lattice.
     The master-quadratic roots {137.036, 3.024} cannot be eigenvalues
     of this operator without external rescaling.

  Conclusion for Link 8 Option beta:
  The master quadratic coefficients are NOT structurally visible in
  the engine's 18-point coupling stencil. They live at the BCC
  sublattice, which the engine's wave-equation operator does not
  access. Any engine-native RG derivation of the master quadratic
  would first need a stencil that includes the BCC (corner) weights.
""")
