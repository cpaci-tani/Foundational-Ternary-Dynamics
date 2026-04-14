"""
Zero Mode Count vs Automorphism Group Order — Route D Check

Tests whether the coincidence |zero modes of BCC| = |Aut(E)| = 4 extends
to other lattice types, or whether it breaks (indicating coincidence).

Known facts:
- BCC (body-centered cubic): Watson integral involves Gamma(1/4)
  -> CM curve E: y^2 = x^3 - x with j = 1728
  -> |Aut(E)| = 4 (the units of Z[i]: {1, -1, i, -i})
  -> BCC Laplacian has 4 zero modes on any even-L torus

- FCC (face-centered cubic): Watson integral involves Gamma(1/3)
  -> CM curve E: y^2 = x^3 - 1 with j = 0
  -> |Aut(E)| = 6 (the units of Z[omega]: {1, omega, omega^2, -1, -omega, -omega^2})

- SC (simple cubic): Watson integral involves different Gamma combination
  -> Not directly associated with a single CM curve
  -> SC Laplacian has 1 zero mode (translation only)

If |zero modes| = |Aut(E)| for all three:
  BCC: 4 = 4 MATCH
  FCC: should be 6 (if connection holds) — but we computed 2 earlier!
  SC: should be 1 (if trivial curve) — matches!

This would BREAK the pattern at FCC (2 != 6), refuting a direct connection.

Status: [EXPLORATORY]
"""

import numpy as np
from scipy.special import gamma
import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

# ============================================================================
# Sublattice eigenvalue functions
# ============================================================================

def sigma_SC(kx, ky, kz):
    return 1.0 - (np.cos(kx) + np.cos(ky) + np.cos(kz)) / 3.0

def sigma_FCC(kx, ky, kz):
    return 1.0 - (np.cos(kx)*np.cos(ky) + np.cos(kx)*np.cos(kz) + np.cos(ky)*np.cos(kz)) / 3.0

def sigma_BCC(kx, ky, kz):
    return 1.0 - np.cos(kx)*np.cos(ky)*np.cos(kz)

# ============================================================================
# Count zero modes on L×L×L torus
# ============================================================================

def count_zero_modes(sigma_func, L, tol=1e-10):
    """Count k-points where sigma(k) < tol on an L×L×L torus."""
    count = 0
    zero_points = []
    for nx in range(L):
        for ny in range(L):
            for nz in range(L):
                kx = 2 * np.pi * nx / L
                ky = 2 * np.pi * ny / L
                kz = 2 * np.pi * nz / L
                s = sigma_func(kx, ky, kz)
                if abs(s) < tol:
                    count += 1
                    # Normalize to [0, 2pi) for display
                    zero_points.append((
                        round(kx / np.pi, 4),
                        round(ky / np.pi, 4),
                        round(kz / np.pi, 4)
                    ))
    return count, zero_points

# ============================================================================
# Main
# ============================================================================

print("=" * 78)
print("  ZERO MODE COUNT vs AUTOMORPHISM GROUP ORDER")
print("  Route D check: does |zero modes| = |Aut(E)| hold across lattice types?")
print("=" * 78)
print()

# Known automorphism group orders
# BCC -> CM curve j=1728, E: y^2=x^3-x, Aut(E) = Z[i]* = {1,-1,i,-i}, |Aut| = 4
# FCC -> CM curve j=0, E: y^2=x^3-1, Aut(E) = Z[w]* = {±1,±w,±w^2}, |Aut| = 6
# SC -> No single CM curve; Watson integral is more complex. |Aut| = ?

print("  Known automorphism groups:")
print("    BCC -> j=1728, E: y^2=x^3-x, |Aut(E)| = 4")
print("    FCC -> j=0,    E: y^2=x^3-1, |Aut(E)| = 6")
print("    SC  -> no single CM curve, |Aut| unclear")
print()

# Compute zero modes for each sublattice across multiple L values
print("  Zero mode counts on L×L×L torus:")
print()
print(f"  {'L':>4}  {'SC':>6}  {'FCC':>6}  {'BCC':>6}")
print("  " + "-" * 28)

for L in [2, 4, 6, 8, 10, 12, 16, 20]:
    n_sc, pts_sc = count_zero_modes(sigma_SC, L)
    n_fcc, pts_fcc = count_zero_modes(sigma_FCC, L)
    n_bcc, pts_bcc = count_zero_modes(sigma_BCC, L)
    print(f"  {L:4d}  {n_sc:6d}  {n_fcc:6d}  {n_bcc:6d}")

print()

# Detailed zero mode locations for L=4 (representative)
L = 4
print(f"  Zero mode locations (k/pi) on L={L} torus:")
print()

for name, func in [("SC", sigma_SC), ("FCC", sigma_FCC), ("BCC", sigma_BCC)]:
    n, pts = count_zero_modes(func, L)
    print(f"  {name} ({n} zero modes):")
    for p in pts:
        print(f"    k/pi = ({p[0]}, {p[1]}, {p[2]})")
    print()

# ============================================================================
# Analysis
# ============================================================================

print("=" * 78)
print("  ANALYSIS")
print("=" * 78)
print()

# Check the pattern
n_sc, _ = count_zero_modes(sigma_SC, 8)
n_fcc, _ = count_zero_modes(sigma_FCC, 8)
n_bcc, _ = count_zero_modes(sigma_BCC, 8)

print(f"  Zero mode counts (L=8):  SC={n_sc}, FCC={n_fcc}, BCC={n_bcc}")
print(f"  |Aut(E)| values:         BCC=4,  FCC=6,  SC=?")
print()

if n_bcc == 4:
    print("  BCC: 4 zero modes = |Aut(E_j1728)| = 4   MATCH MATCH")
else:
    print(f"  BCC: {n_bcc} zero modes != |Aut(E_j1728)| = 4   MISS MISMATCH")

if n_fcc == 6:
    print("  FCC: 6 zero modes = |Aut(E_j0)| = 6       MATCH MATCH")
elif n_fcc == 2:
    print("  FCC: 2 zero modes != |Aut(E_j0)| = 6       MISS MISMATCH")
    print("       (FCC has 2, not 6 — the pattern BREAKS at FCC)")
else:
    print(f"  FCC: {n_fcc} zero modes vs |Aut(E_j0)| = 6  — {('MATCH' if n_fcc == 6 else 'MISMATCH')}")

print()
print("  VERDICT:")
if n_fcc != 6:
    print("  The pattern |zero modes| = |Aut(E)| does NOT hold across all lattice types.")
    print("  BCC's 4 zero modes matching |Aut(E_j1728)| = 4 appears to be a")
    print("  COINCIDENCE or reflects a BCC-specific structure, not a general principle.")
    print()
    print("  However, a WEAKER pattern may hold:")
    print(f"    SC:  {n_sc} zero mode  = 1 (generic elliptic curve |Aut| = 2, halved?)")
    print(f"    FCC: {n_fcc} zero modes = 2 (= |Aut(E_j0)|/3 = 6/3?)")
    print(f"    BCC: {n_bcc} zero modes = 4 (= |Aut(E_j1728)|)")
    print()
    print("  Or the connection is:")
    print(f"    SC:  {n_sc} = 2^0")
    print(f"    FCC: {n_fcc} = 2^1")
    print(f"    BCC: {n_bcc} = 2^2")
    print("  which is: zero modes = 2^(number of axes paired in the eigenvalue product)")
    print("    SC:  0 paired axes -> 2^0 = 1")
    print("    FCC: 1 pair (two axes coupled per term) -> ... actually 3 pairs")
    print("    BCC: all 3 axes coupled -> 2^2 = 4")
else:
    print("  The pattern |zero modes| = |Aut(E)| appears to hold across lattice types!")
    print("  This suggests a deep connection between spectral graph theory and")
    print("  elliptic curve arithmetic that deserves further investigation.")
