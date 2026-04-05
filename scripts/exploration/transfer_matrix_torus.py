#!/usr/bin/env python3
"""
Transfer Matrix Computation on 2x2x2 Torus
Target: does the spectrum reproduce (x-1)(x-3)(x^2+4x+13) mod 27?
"""

import numpy as np
from itertools import product

print("=" * 78)
print("  TRANSFER MATRIX: Ternary partition function on 2x2x2 torus")
print("=" * 78)

L = 2
N_SITES = L**3

# Scalar 18-point isotropic Laplacian on 2x2x2
def site_idx(x, y, z):
    return (x % L) * L * L + (y % L) * L + (z % L)

Lap = np.zeros((N_SITES, N_SITES))
for i in range(N_SITES):
    x, y, z = i // (L*L), (i // L) % L, i % L
    Lap[i, i] += -4.0
    for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
        Lap[i, site_idx(x+dx, y+dy, z+dz)] += 1.0/3.0
    for dx, dy, dz in [(1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
                        (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
                        (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1)]:
        Lap[i, site_idx(x+dx, y+dy, z+dz)] += 1.0/6.0

G = np.linalg.pinv(-Lap, rcond=1e-10)

print(f"\nScalar Green function G(0,0) = {G[0,0]:.6f}")
print(f"Laplacian eigenvalues: {np.sort(np.linalg.eigvalsh(Lap))}")

# Enumerate all 3^8 = 6561 configurations
configs = list(product([-1, 0, 1], repeat=N_SITES))
print(f"\n{len(configs)} total configurations")

# Quadratic form q(s) = s^T G s
q_vals = []
for s_tuple in configs:
    s = np.array(s_tuple, dtype=float)
    q_vals.append(s @ G @ s)
q_vals = np.array(q_vals)
print(f"q range: [{q_vals.min():.6f}, {q_vals.max():.6f}]")
print(f"Unique q values: {len(np.unique(np.round(q_vals, 10)))}")

# Transfer matrix: slice into z=0 (sites 0-3) and z=1 (sites 4-7)
slice_cfgs = list(product([-1, 0, 1], repeat=4))
N_slice = len(slice_cfgs)
print(f"\nTransfer matrix: {N_slice}x{N_slice}")

G_aa = G[:4, :4]  # intra z=0
G_ab = G[:4, 4:]  # inter z=0 to z=1

# Build T[a,b] = exp(coupling between slice configs a and b)
T = np.zeros((N_slice, N_slice))
for a_idx, a_cfg in enumerate(slice_cfgs):
    sa = np.array(a_cfg, dtype=float)
    for b_idx, b_cfg in enumerate(slice_cfgs):
        sb = np.array(b_cfg, dtype=float)
        # Symmetric coupling
        e = sa @ G_ab @ sb + sb @ G_ab.T @ sa + 0.5*(sa @ G_aa @ sa + sb @ G_aa @ sb)
        T[a_idx, b_idx] = np.exp(0.5 * e)

eigs = np.sort(np.linalg.eigvals(T).real)[::-1]

print(f"\nTop 20 eigenvalues of T:")
for i in range(min(20, len(eigs))):
    print(f"  lambda_{i+1:2d} = {eigs[i]:12.6f}")

print(f"\nZ = Tr(T^2) = {np.sum(eigs**2):.4f}")
print(f"Z = Tr(T)^? ... Tr(T) = {np.sum(eigs):.4f}")

# Eigenvalue ratios from the top
print(f"\nEigenvalue RATIOS (lambda_1 / lambda_n):")
for i in range(1, min(15, len(eigs))):
    if eigs[i] > 0.01:
        r = eigs[0] / eigs[i]
        flag = ""
        if abs(r - 3) < 0.3: flag = " *** ~ N_c = 3 ***"
        elif abs(r - 4) < 0.3: flag = " *** ~ N_base = 4 ***"
        elif abs(r - 7) < 0.5: flag = " *** ~ b_3 = 7 ***"
        elif abs(r - 13) < 1: flag = " *** ~ N_eff = 13 ***"
        elif abs(r - 16) < 1: flag = " *** ~ |Aut|^2 = 16 ***"
        elif abs(r - 27) < 2: flag = " *** ~ N_c^3 = 27 ***"
        elif abs(r - 42) < 3: flag = " *** ~ 42 ***"
        elif abs(r - 47) < 3: flag = " *** ~ D = 47 ***"
        print(f"  lambda_1/lambda_{i+1:2d} = {r:10.4f}{flag}")

# Degeneracy structure
print(f"\nDegeneracy structure (eigenvalues rounded to 0.001):")
rounded = np.round(eigs, 3)
unique_eigs, counts = np.unique(rounded, return_counts=True)
for e, c in sorted(zip(unique_eigs, counts), key=lambda x: -x[0])[:15]:
    print(f"  eigenvalue {e:12.6f}: degeneracy {c}")

# 3-adic analysis: eigenvalues mod 3
print(f"\nNormalized eigenvalue spectrum (lambda / lambda_max):")
normed = eigs / eigs[0]
for i in range(min(15, len(normed))):
    n = normed[i]
    # Express as fraction with small denominator
    for d in range(1, 50):
        if abs(n * d - round(n * d)) < 0.02:
            num = round(n * d)
            print(f"  lambda_{i+1:2d}/lambda_1 = {n:.6f} ~ {num}/{d}")
            break
    else:
        print(f"  lambda_{i+1:2d}/lambda_1 = {n:.6f}")

# KEY TEST: the polynomial P(x) mod 27 has roots at 1, 3, -2+3i, -2-3i
# If the transfer matrix encodes this, eigenvalue ratios should include 1:3
# and eigenvalue norms should include sqrt(13) ~ 3.606
print(f"\nKEY TEST: looking for ratio 3 and norm sqrt(13) = {np.sqrt(13):.4f}")
for i in range(len(eigs)):
    for j in range(i+1, len(eigs)):
        if eigs[j] > 0.01:
            r = eigs[i] / eigs[j]
            if abs(r - 3.0) < 0.05:
                print(f"  FOUND: lambda_{i+1}/lambda_{j+1} = {r:.6f} ~ 3")
            if abs(r - np.sqrt(13)) < 0.1:
                print(f"  FOUND: lambda_{i+1}/lambda_{j+1} = {r:.6f} ~ sqrt(13)")
