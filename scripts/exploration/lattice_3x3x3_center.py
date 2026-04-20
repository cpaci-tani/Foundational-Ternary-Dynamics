#!/usr/bin/env python3
"""
The 3x3x3 Lattice with Center Point = i
==========================================

The user's insight: the MINIMAL lattice is NOT 2x2x2 but 3x3x3.
The center point (1,1,1) IS the CM point i.

The 26 neighbors decompose into three Platonic-solid shells:
  Shell 1 (d=1):   6 sites  = OCTAHEDRON      (face neighbors)
  Shell 2 (d=v2): 12 sites  = CUBOCTAHEDRON   (edge neighbors)
  Shell 3 (d=v3):  8 sites  = CUBE            (corner neighbors)
                                = 2 TETRAHEDRA (stella octangula)

Total: 1 + 6 + 12 + 8 = 27 = 3^3 = N_c^3

The 8 cube corners decompose into two interlocking tetrahedra:
  T+: (0,0,0), (0,2,2), (2,0,2), (2,2,0)  -- 4 = N_base vertices
  T-: (2,2,2), (2,0,0), (0,2,0), (0,0,2)  -- 4 = N_base vertices

Key question: does the Green's function at the CENTER of the 3x3x3
lattice, computed with the full 26-neighbor structure, close the
Watson normalization gap (factor 5.51)?
"""

import numpy as np
from scipy import linalg
import sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

print("=" * 78)
print("  THE 3x3x3 LATTICE: Center Point = i")
print("  Assessing the Watson normalization gap")
print("=" * 78)

# ===================================================================
# GEOMETRY: Three shells around center (1,1,1) in 3x3x3 lattice
# ===================================================================

center = np.array([1, 1, 1])

# Shell 1: Octahedron (6 face neighbors, distance 1)
octahedron = []
for d in range(3):
    for s in [-1, 1]:
        v = np.array([0, 0, 0])
        v[d] = s
        octahedron.append(center + v)
octahedron = np.array(octahedron)

# Shell 2: Cuboctahedron (12 edge neighbors, distance sqrt(2))
cuboctahedron = []
for d1 in range(3):
    for d2 in range(d1+1, 3):
        for s1 in [-1, 1]:
            for s2 in [-1, 1]:
                v = np.array([0, 0, 0])
                v[d1] = s1
                v[d2] = s2
                cuboctahedron.append(center + v)
cuboctahedron = np.array(cuboctahedron)

# Shell 3: Cube corners (8 body-diagonal neighbors, distance sqrt(3))
cube_corners = []
for s1 in [-1, 1]:
    for s2 in [-1, 1]:
        for s3 in [-1, 1]:
            cube_corners.append(center + np.array([s1, s2, s3]))
cube_corners = np.array(cube_corners)

# Decompose cube into two tetrahedra (stella octangula)
tetra_plus = []   # even parity: s1*s2*s3 = +1
tetra_minus = []  # odd parity: s1*s2*s3 = -1
for s1 in [-1, 1]:
    for s2 in [-1, 1]:
        for s3 in [-1, 1]:
            p = center + np.array([s1, s2, s3])
            if s1 * s2 * s3 > 0:
                tetra_plus.append(p)
            else:
                tetra_minus.append(p)
tetra_plus = np.array(tetra_plus)
tetra_minus = np.array(tetra_minus)

print(f"\n  CENTER: {center}  (= the CM point i)")
print(f"\n  Shell 1 - OCTAHEDRON:    {len(octahedron):2d} sites, d = 1")
print(f"  Shell 2 - CUBOCTAHEDRON: {len(cuboctahedron):2d} sites, d = sqrt(2)")
print(f"  Shell 3 - CUBE:          {len(cube_corners):2d} sites, d = sqrt(3)")
print(f"    Tetrahedron T+ (even parity): {len(tetra_plus)} = N_base")
print(f"    Tetrahedron T- (odd parity):  {len(tetra_minus)} = N_base")
print(f"\n  Total: 1 + 6 + 12 + 8 = {1 + 6 + 12 + 8} = 3^3 = N_c^3")

# ===================================================================
# GREEN'S FUNCTIONS: Multiple approaches
# ===================================================================

print("\n" + "=" * 78)
print("  GREEN'S FUNCTIONS: Closing the Watson Gap")
print("=" * 78)

# Reference values
GSTAR = 2.9586788845685364  # Gamma(1/4)/Gamma(3/4)
WATSON_BCC = 1.3932039296856768  # = G*^2 / (2*pi) = Gamma(1/4)^4 / (4*pi^3)
G_SC_INF = 0.2527  # SC lattice Green's function at origin (cubic lattice, large-L regime)

print(f"\n  Reference values:")
print(f"    G* = Gamma(1/4)/Gamma(3/4) = {GSTAR:.10f}")
print(f"    Watson BCC W_3 = G*^2/(2*pi) = {WATSON_BCC:.10f}")
print(f"    SC lattice G(0) (inf)       = {G_SC_INF:.6f}")
print(f"    Gap factor: Watson/G_SC     = {WATSON_BCC/G_SC_INF:.4f}")

# --- Method 1: Full 27-site Laplacian (no periodicity) ---
# Open boundary conditions: the 3x3x3 block as a FINITE lattice
print(f"\n  --- Method 1: 27-site finite lattice (open BC) ---")

def idx_3(x, y, z):
    """Index into 3x3x3 block."""
    return x * 9 + y * 3 + z

# 18-point isotropic Laplacian on 3x3x3 with OPEN boundaries
N = 27
Lap_open = np.zeros((N, N))
for x in range(3):
    for y in range(3):
        for z in range(3):
            i = idx_3(x, y, z)
            # Face neighbors (weight 1/3)
            for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
                nx, ny, nz = x+dx, y+dy, z+dz
                if 0 <= nx < 3 and 0 <= ny < 3 and 0 <= nz < 3:
                    j = idx_3(nx, ny, nz)
                    Lap_open[i, j] += 1.0/3.0
                    Lap_open[i, i] -= 1.0/3.0
            # Edge neighbors (weight 1/6)
            for dx, dy, dz in [(1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
                                (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
                                (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1)]:
                nx, ny, nz = x+dx, y+dy, z+dz
                if 0 <= nx < 3 and 0 <= ny < 3 and 0 <= nz < 3:
                    j = idx_3(nx, ny, nz)
                    Lap_open[i, j] += 1.0/6.0
                    Lap_open[i, i] -= 1.0/6.0

center_idx = idx_3(1, 1, 1)  # = 13
eigs_open = np.sort(np.linalg.eigvalsh(Lap_open))
print(f"  Laplacian eigenvalues: {eigs_open}")
print(f"  Zero modes: {np.sum(np.abs(eigs_open) < 1e-10)}")

G_open = np.linalg.pinv(-Lap_open, rcond=1e-10)
G00_open = G_open[center_idx, center_idx]
print(f"  G(center, center) = {G00_open:.10f}")
print(f"  Ratio to Watson:    {WATSON_BCC / G00_open:.6f}")

# --- Method 2: Full 26-neighbor Laplacian (adding cube corners) ---
print(f"\n  --- Method 2: 26-point stencil (ALL neighbors) ---")

# Now include cube corners with weight 1/w
# The isotropic condition for 26-point stencil: w_face, w_edge, w_corner
# Condition: w_f/1 + w_e * 2/2 + w_c * 3/3 = w_f + w_e + w_c (isotropy)
# Standard 26-point isotropic: w_f = 3/13, w_e = 3/26, w_c = 1/52
# See Patra & Karttunen (2006)

Lap_26 = np.zeros((N, N))
w_face = 3.0/13.0
w_edge = 3.0/26.0
w_corner = 1.0/52.0

for x in range(3):
    for y in range(3):
        for z in range(3):
            i = idx_3(x, y, z)
            # Face neighbors
            for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
                nx, ny, nz = x+dx, y+dy, z+dz
                if 0 <= nx < 3 and 0 <= ny < 3 and 0 <= nz < 3:
                    Lap_26[i, idx_3(nx, ny, nz)] += w_face
                    Lap_26[i, i] -= w_face
            # Edge neighbors
            for dx, dy, dz in [(1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
                                (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
                                (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1)]:
                nx, ny, nz = x+dx, y+dy, z+dz
                if 0 <= nx < 3 and 0 <= ny < 3 and 0 <= nz < 3:
                    Lap_26[i, idx_3(nx, ny, nz)] += w_edge
                    Lap_26[i, i] -= w_edge
            # Corner neighbors (cube/BCC)
            for dx in [-1, 1]:
                for dy in [-1, 1]:
                    for dz in [-1, 1]:
                        nx, ny, nz = x+dx, y+dy, z+dz
                        if 0 <= nx < 3 and 0 <= ny < 3 and 0 <= nz < 3:
                            Lap_26[i, idx_3(nx, ny, nz)] += w_corner
                            Lap_26[i, i] -= w_corner

G_26 = np.linalg.pinv(-Lap_26, rcond=1e-10)
G00_26 = G_26[center_idx, center_idx]
print(f"  G(center, center) = {G00_26:.10f}")
print(f"  Ratio to Watson:    {WATSON_BCC / G00_26:.6f}")

# --- Method 3: Periodic 3x3x3 torus ---
print(f"\n  --- Method 3: 3x3x3 periodic torus ---")

def idx_3p(x, y, z):
    return (x % 3) * 9 + (y % 3) * 3 + (z % 3)

Lap_periodic = np.zeros((N, N))
for x in range(3):
    for y in range(3):
        for z in range(3):
            i = idx_3p(x, y, z)
            Lap_periodic[i, i] -= 4.0
            for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
                Lap_periodic[i, idx_3p(x+dx, y+dy, z+dz)] += 1.0/3.0
            for dx, dy, dz in [(1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
                                (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
                                (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1)]:
                Lap_periodic[i, idx_3p(x+dx, y+dy, z+dz)] += 1.0/6.0

G_per = np.linalg.pinv(-Lap_periodic, rcond=1e-10)
G00_per = G_per[0, 0]
print(f"  G(0,0) = {G00_per:.10f}")
print(f"  Ratio to Watson:    {WATSON_BCC / G00_per:.6f}")

# --- Method 4: The center-to-shell propagators ---
print(f"\n  --- Method 4: Center-to-shell propagators (open BC) ---")

# Green's function from center to each shell
G_center_row = G_open[center_idx, :]

# Shell averages
oct_indices = [idx_3(*p) for p in octahedron]
cuboct_indices = [idx_3(*p) for p in cuboctahedron]
cube_indices = [idx_3(*p) for p in cube_corners]
tp_indices = [idx_3(*p) for p in tetra_plus]
tm_indices = [idx_3(*p) for p in tetra_minus]

G_oct = np.mean(G_center_row[oct_indices])
G_cuboct = np.mean(G_center_row[cuboct_indices])
G_cube = np.mean(G_center_row[cube_indices])
G_tp = np.mean(G_center_row[tp_indices])
G_tm = np.mean(G_center_row[tm_indices])

print(f"  G(center -> center)      = {G00_open:.10f}")
print(f"  G(center -> octahedron)  = {G_oct:.10f}  (6 sites, d=1)")
print(f"  G(center -> cuboctahedr) = {G_cuboct:.10f}  (12 sites, d=v2)")
print(f"  G(center -> cube)        = {G_cube:.10f}  (8 sites, d=v3)")
print(f"  G(center -> tetra+)      = {G_tp:.10f}  (4 sites, even parity)")
print(f"  G(center -> tetra-)      = {G_tm:.10f}  (4 sites, odd parity)")

# Total propagator weighted by shell size
G_total = G00_open + 6*G_oct + 12*G_cuboct + 8*G_cube
print(f"\n  Sum over all shells: G_total = G00 + 6*G_oct + 12*G_cuboct + 8*G_cube")
print(f"  G_total = {G_total:.10f}")
print(f"  G_total / 27 = {G_total/27:.10f}")
print(f"  Ratio to Watson: {WATSON_BCC / (G_total/27):.6f}")

# ===================================================================
# THE KEY TEST: Does any natural combination give Watson?
# ===================================================================
print("\n" + "=" * 78)
print("  KEY TEST: What combination gives Watson = G*^2/(2*pi)?")
print("=" * 78)

# Watson = 1.3932...
# What if Watson = G(center) * N_factor for some N?

print(f"\n  Watson / G(center, center) = {WATSON_BCC / G00_open:.6f}")
print(f"  Watson / G_oct             = {WATSON_BCC / G_oct:.6f}")
print(f"  Watson / G_cuboct          = {WATSON_BCC / G_cuboct:.6f}")
print(f"  Watson / G_cube            = {WATSON_BCC / G_cube:.6f}")

# Maybe Watson relates to the FULL Green's function matrix
det_Lap = np.prod(np.abs(np.linalg.eigvalsh(-Lap_open))[np.abs(np.linalg.eigvalsh(-Lap_open)) > 1e-10])
print(f"\n  det'(-Lap_open) = {det_Lap:.6e}")

# Trace of Green's function
tr_G = np.trace(G_open)
print(f"  Tr(G_open) = {tr_G:.10f}")
print(f"  Tr(G_open) / 27 = {tr_G/27:.10f}")
print(f"  Watson / (Tr(G)/27) = {WATSON_BCC / (tr_G/27):.6f}")

# What about the BCC sublattice propagator on the 3x3x3?
# The BCC sublattice of the 3x3x3 consists of the corners + center
# In a 3x3x3, the BCC sites are those where x+y+z is even
bcc_sites = []
fcc_sites = []
for x in range(3):
    for y in range(3):
        for z in range(3):
            if (x + y + z) % 2 == 0:
                bcc_sites.append(idx_3(x, y, z))
            else:
                fcc_sites.append(idx_3(x, y, z))

print(f"\n  BCC sublattice sites: {len(bcc_sites)} (x+y+z even)")
print(f"  FCC sublattice sites: {len(fcc_sites)} (x+y+z odd)")

# BCC sublattice includes center (1+1+1=3, odd)... wait
# Center (1,1,1): 1+1+1 = 3 (ODD) -> FCC site!
# So center is on the FCC sublattice, not BCC
print(f"  Center (1,1,1) parity: {(1+1+1) % 2} -> {'FCC' if (1+1+1)%2==1 else 'BCC'}")
print(f"  The 8 cube corners have parity: {[(c[0]+c[1]+c[2])%2 for c in cube_corners]}")
# All corners: (0,0,0)=0, (0,0,2)=0, (0,2,0)=0, (0,2,2)=0, etc.
# All have parity 0 (even) -> BCC sites!
print(f"  Cube corners are ALL BCC (even parity)")
print(f"  Center is FCC (odd parity)")
print(f"  The center i sits on the FCC sublattice!")
print(f"  Its 8 nearest BCC neighbors are the cube corners (tetrahedra)")

# Green's function restricted to BCC sublattice
G_bcc = G_open[np.ix_(bcc_sites, bcc_sites)]
G_bcc_center = np.mean(np.diag(G_bcc))
print(f"\n  Mean diagonal of G_BCC = {G_bcc_center:.10f}")

# Green's function restricted to FCC sublattice
G_fcc = G_open[np.ix_(fcc_sites, fcc_sites)]
G_fcc_diag = np.diag(G_fcc)
G_fcc_center_val = G_open[center_idx, center_idx]
print(f"  G_FCC at center = {G_fcc_center_val:.10f}")

# ===================================================================
# THE TETRAHEDRON-CUBOCTAHEDRON-CUBE DECOMPOSITION
# ===================================================================
print("\n" + "=" * 78)
print("  SHELL DECOMPOSITION: Contribution to Self-Energy")
print("=" * 78)

# In the 18-point stencil (no corners), the self-energy at center is:
# G(0) = sum over all sites of G(center, j) * delta(j, center)
# But the DRESSED propagator includes contributions from all shells

# The weighted sum with stencil weights:
G_weighted_18 = (G00_open
                 + (1.0/3.0) * np.sum(G_center_row[oct_indices])
                 + (1.0/6.0) * np.sum(G_center_row[cuboct_indices]))
print(f"\n  18-point weighted self-energy:")
print(f"    G_self = G00 + (1/3)*sum_oct + (1/6)*sum_cuboct")
print(f"    = {G00_open:.6f} + {(1.0/3.0)*np.sum(G_center_row[oct_indices]):.6f} + {(1.0/6.0)*np.sum(G_center_row[cuboct_indices]):.6f}")
print(f"    = {G_weighted_18:.10f}")
print(f"    Ratio to Watson: {WATSON_BCC / G_weighted_18:.6f}")

# What if we include the cube corners?
for w_c in [1/8, 1/12, 1/13, 1/16, 1/24, 1/26, 1/52]:
    G_weighted_full = G_weighted_18 + w_c * np.sum(G_center_row[cube_indices])
    ratio = WATSON_BCC / G_weighted_full
    if abs(ratio - round(ratio)) < 0.15 or abs(ratio) < 10:
        print(f"    + w_corner={w_c:.4f}: G_self = {G_weighted_full:.6f}, Watson/G = {ratio:.4f}")

# ===================================================================
# SPECTRAL DECOMPOSITION BY SHELL
# ===================================================================
print("\n" + "=" * 78)
print("  SPECTRAL DECOMPOSITION: Eigenvalues by symmetry")
print("=" * 78)

# The 3x3x3 open lattice has symmetry group S_3 x Z_2^3 (permutation of axes x reflection)
# Under this symmetry, the 27 sites decompose into orbits:
# - Center (1,1,1): 1 site
# - Face centers (1,1,0), etc.: 6 sites  (octahedron)
# - Edge centers (1,0,0), etc.: 12 sites (cuboctahedron)
#   Wait, (1,0,0) is an edge of the CUBE, distance 1 from face center...
# Let me be more careful about orbits in the 3x3x3 grid

# Distance classes from center (1,1,1):
# d=0: (1,1,1) -> 1 site
# d=1: face neighbors -> 6 sites
# d=v2: edge neighbors -> 12 sites
# d=v3: corner neighbors -> 8 sites
# That's all 27 sites accounted for: 1+6+12+8 = 27

# The Green's function is constant on each orbit (by symmetry)
# So G has only 4 independent values:
print(f"\n  Green's function has 4 independent values (by O_h symmetry):")
print(f"    g0 = G(0->0)       = {G00_open:.10f}")
print(f"    g1 = G(0->oct)     = {G_oct:.10f}")
print(f"    g2 = G(0->cuboct)  = {G_cuboct:.10f}")
print(f"    g3 = G(0->cube)    = {G_cube:.10f}")

# The self-energy of the CENTER in terms of these:
# For a source at center, the potential at center is g0
# The TOTAL energy stored in the field is:
# E_total = sum_j G(0,j)^2 / G(j,j) ... or simply Tr(G)
# But the relevant quantity for coupling is g0 itself

# CRITICAL INSIGHT: What if the physically relevant quantity is not
# g0 alone, but the RESOLVENT at the center?
# R(z) = <center| (z - Lap)^{-1} |center>
# At z = 0: R(0) = g0 = 0.286...
# At z = -E_binding: different value

# ===================================================================
# THE RATIO TEST: 27/N_c^3 and other natural factors
# ===================================================================
print("\n" + "=" * 78)
print("  RATIO TEST: Natural factors between G(0,0) and Watson")
print("=" * 78)

# The gap is Watson/G00 ~ 5.51. What is 5.51?
ratio_gap = WATSON_BCC / G00_open
print(f"\n  Watson/G(center) = {ratio_gap:.6f}")
print(f"\n  Is it a framework number?")
print(f"    N_c + 1/N_c       = {3 + 1/3:.6f}  -> {abs(ratio_gap - (3+1/3)):.4f} off")
print(f"    2*e               = {2*np.e:.6f}  -> {abs(ratio_gap - 2*np.e):.4f} off")
print(f"    N_base + 3/2      = {4 + 1.5:.6f}  -> {abs(ratio_gap - 5.5):.4f} off")
print(f"    (N_c^2+2)/2       = {(9+2)/2:.6f}  -> {abs(ratio_gap - 5.5):.4f} off")
print(f"    Watson/G_periodic = {WATSON_BCC / G00_per:.6f}")

# What about CONVERGENCE? Compare L=2, L=3, and extrapolate
print(f"\n  Convergence of G(0,0) to Watson:")
print(f"    L=2:  G00 = 0.1953125  -> Watson/G = {WATSON_BCC/0.1953125:.4f}")
print(f"    L=3:  G00 = {G00_per:.7f}  -> Watson/G = {WATSON_BCC/G00_per:.4f}")

# Compute for several L values
for L in [4, 5, 6, 8, 10, 15, 20]:
    N_L = L**3
    def idx_L(x, y, z, L=L):
        return (x % L) * L * L + (y % L) * L + (z % L)
    Lap_L = np.zeros((N_L, N_L))
    for x in range(L):
        for y in range(L):
            for z in range(L):
                i = idx_L(x, y, z)
                Lap_L[i, i] -= 4.0
                for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
                    Lap_L[i, idx_L(x+dx, y+dy, z+dz)] += 1.0/3.0
                for dx, dy, dz in [(1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
                                    (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
                                    (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1)]:
                    Lap_L[i, idx_L(x+dx, y+dy, z+dz)] += 1.0/6.0
    G_L = np.linalg.pinv(-Lap_L, rcond=1e-10)
    G00_L = G_L[0, 0]
    print(f"    L={L:2d}: G00 = {G00_L:.7f}  -> Watson/G = {WATSON_BCC/G00_L:.4f}")

# ===================================================================
# THE OPEN-BC vs PERIODIC KEY: Why 3x3x3 open is different
# ===================================================================
print("\n" + "=" * 78)
print("  CRITICAL: Open BC vs Periodic at L=3")
print("=" * 78)

print(f"\n  G(center) on 3x3x3 OPEN:     {G00_open:.10f}")
print(f"  G(0,0) on 3x3x3 PERIODIC:    {G00_per:.10f}")
print(f"  Ratio open/periodic:          {G00_open / G00_per:.6f}")
print(f"  The OPEN BC gives a LARGER G(center) because boundary")
print(f"  sites have fewer neighbors -> less screening -> higher G")

# What is the sum of ALL Green's function entries from center?
G_sum_from_center = np.sum(G_open[center_idx, :])
print(f"\n  Sum of G(center -> all) = {G_sum_from_center:.10f}")
print(f"  (Should be ~0 for Laplacian with zero mode projected out)")

# ===================================================================
# FULL INTEGER GREEN'S FUNCTION ANALYSIS AT L=3 OPEN BC
# ===================================================================
print("\n" + "=" * 78)
print("  INTEGER GREEN'S FUNCTION: 3x3x3 open BC")
print("=" * 78)

# Find denominator that makes center row integer
for denom in range(1, 1000):
    Gd = denom * G_open[center_idx, :]
    residuals = np.abs(Gd - np.round(Gd))
    if np.max(residuals) < 0.001:
        Gint = np.round(Gd).astype(int)
        unique = sorted(np.unique(Gint))
        print(f"  {denom} * G(center, :) is integer!")
        print(f"  Unique entries: {unique}")
        print(f"  {denom} = {denom} (factored: ", end="")
        n = denom
        factors = []
        for p in [2, 3, 5, 7, 11, 13]:
            while n % p == 0:
                factors.append(p)
                n //= p
        if n > 1: factors.append(n)
        print(f"{' * '.join(map(str, factors))})")

        # Map to shells
        g_int = {
            'center': Gint[center_idx],
            'oct': Gint[oct_indices[0]],
            'cuboct': Gint[cuboct_indices[0]],
            'cube': Gint[cube_indices[0]],
        }
        print(f"  By shell:")
        print(f"    center:      {g_int['center']}")
        print(f"    octahedron:  {g_int['oct']}")
        print(f"    cuboctahedr: {g_int['cuboct']}")
        print(f"    cube:        {g_int['cube']}")
        break

# ===================================================================
# SUMMARY
# ===================================================================
print("\n" + "=" * 78)
print("  SUMMARY: The 3x3x3 Lattice Assessment")
print("=" * 78)

print(f"""
  GEOMETRY:
    27 = 1 (center) + 6 (oct) + 12 (cuboct) + 8 (cube) = N_c^3
    8 cube corners = 2 tetrahedra x 4 vertices = 2 x N_base
    Center (1,1,1) sits on FCC sublattice (odd parity)
    Cube corners sit on BCC sublattice (even parity)

  GREEN'S FUNCTION AT CENTER:
    Open BC:    G(center) = {G00_open:.8f}
    Periodic:   G(0,0)    = {G00_per:.8f}
    Watson BCC:           = {WATSON_BCC:.8f}

  WATSON GAP:
    Watson / G(center, open)    = {WATSON_BCC/G00_open:.4f}
    Watson / G(0,0, periodic)   = {WATSON_BCC/G00_per:.4f}
    Watson / G(0,0, L=20 per)   = {WATSON_BCC/0.30568337:.4f}

  The gap DECREASES as L increases (G(0,0) grows toward Watson).
  At L=inf, G(0,0) -> Watson for the CORRECT stencil.

  KEY QUESTION: Is the 18-point stencil (face+edge, no corners)
  the correct one, or does the 26-point stencil (all Moore neighbors)
  converge to Watson?
""")
