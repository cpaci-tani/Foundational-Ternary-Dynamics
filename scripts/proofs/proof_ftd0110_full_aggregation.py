"""
Proof — FTD-0110 Phase C: Cluster aggregation with proper Langevin equipartition
====================================================================================

Phase B falsified the naive "f_slow per block = 1/√d" hypothesis. This
script tests the NEXT candidate framework based on Phase B's actual
findings:

LANGEVIN EQUIPARTITION FRAMEWORK:
  Linear theorem at central block: under Langevin equipartition, the
  injected energy A² distributes equally over the 4 A_{1g} modes; slow
  mode gets A²/4 → cluster size = A²/4.

  Multi-block extension: at each voxel x, define the per-voxel
  manifestation efficiency as
       η(x) = (block_energy_at_x × 1/d_G(x)) / total_injected_energy
  where:
       block_energy_at_x = Σ_{δ∈{-1,0,1}^3} |G_L(x + δ)|²
       d_G(x)              = trivial-irrep dim of block's local symmetry

  Cluster size: N(A) = A² × Σ_voxels η(x) summed over cluster voxels.

  Per the linear theorem at central x=0: η(0) = (block energy at origin)
  × (1/4) / (total field energy). If most field energy concentrates at
  the central block, this gives η(0) close to 1/4 → recovers linear thm.

This script:
  1. Computes G_L on a finite lattice (L=32)
  2. For each candidate voxel x, computes block_energy(x), d_G(x), η(x)
  3. Aggregates over a cluster of radius R(A) for each empirical A
  4. Compares to engine k(A) data

Provenance: docs/theory/03_derivations/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md
LEDGER: FTD-0119 (Mechanism α 1/√d closed-negative; this is the next
        candidate framework following Phase B's distance-dependence finding).

Usage:
    python scripts/proofs/proof_ftd0110_full_aggregation.py
"""

import math
import sys
from itertools import product

import numpy as np


L_LATTICE = 32
C_LAT = 1.0 / math.sqrt(3.0)


# Symmetry classification (from previous scripts)
def classify_position(dx, dy, dz):
    if dx == 0 and dy == 0 and dz == 0:
        return 'O_h'
    n_zeros = sum(1 for v in (dx, dy, dz) if v == 0)
    nonzero = sorted(abs(v) for v in (dx, dy, dz) if v != 0)
    n_distinct = len(set(nonzero))

    if n_zeros == 2:
        return 'C_4v'
    elif n_zeros == 1 and n_distinct == 1:
        return 'C_2v'
    elif n_zeros == 0 and n_distinct == 1:
        return 'C_3v'
    elif n_zeros == 1 and n_distinct == 2:
        return 'C_s'
    else:
        return 'C_1'


D_PER_SYMMETRY = {
    'O_h':  4,
    'C_4v': 9,
    'C_3v': 10,
    'C_2v': 12,
    'C_s':  18,
    'C_1':  27,
}


def green_table(L_side, c=C_LAT, max_dist=14):
    """Vectorized lattice Poisson Green's function table."""
    n = np.arange(L_side)
    k1, k2, k3 = np.meshgrid(n, n, n, indexing='ij')
    k1f = 2 * np.pi * k1.flatten() / L_side
    k2f = 2 * np.pi * k2.flatten() / L_side
    k3f = 2 * np.pi * k3.flatten() / L_side
    kh2 = 4 * (np.sin(0.5 * k1f) ** 2 + np.sin(0.5 * k2f) ** 2 + np.sin(0.5 * k3f) ** 2)
    nonzero = kh2 > 1e-14
    kh2 = kh2[nonzero]
    k1f, k2f, k3f = k1f[nonzero], k2f[nonzero], k3f[nonzero]
    inv_kh2 = 1.0 / (c * c * kh2)
    table = {}
    for rx in range(-max_dist, max_dist + 1):
        for ry in range(-max_dist, max_dist + 1):
            for rz in range(-max_dist, max_dist + 1):
                phase = np.cos(k1f * rx + k2f * ry + k3f * rz)
                table[(rx, ry, rz)] = float(np.sum(phase * inv_kh2)) / (L_side ** 3)
    return table


def block_energy(x, gtable):
    """Sum of |G_L(x + δ)|² over the 27 voxels of the block at x."""
    cx, cy, cz = x
    total = 0.0
    for dx, dy, dz in product(range(-1, 2), repeat=3):
        rx, ry, rz = cx + dx, cy + dy, cz + dz
        g = gtable.get((rx, ry, rz), 0.0)
        total += g * g
    return total


def total_field_energy(gtable, max_dist=14):
    """Total field energy summed over the lattice (within table)."""
    return sum(g * g for g in gtable.values())


def cluster_voxels(R):
    """Voxels in a sphere of radius R around origin."""
    voxels = []
    Ri = math.ceil(R)
    for x, y, z in product(range(-Ri, Ri + 1), repeat=3):
        r = math.sqrt(x*x + y*y + z*z)
        if r <= R:
            voxels.append((x, y, z, r))
    return voxels


def predict_k(R, gtable, total_energy):
    """Predicted k(A) = sum over cluster voxels of η(x).

    η(x) = (block_energy(x) × 1/d_G(x)) / total_field_energy
    """
    voxels = cluster_voxels(R)
    total = 0.0
    for x, y, z, r in voxels:
        eta_x = block_energy((x, y, z), gtable) / D_PER_SYMMETRY[classify_position(x, y, z)]
        total += eta_x
    return total / total_energy


# Empirical engine data
EMPIRICAL = [
    (10.00,   0.252),
    (15.00,   0.224),
    (20.00,   0.234),
    (28.77,   0.253),
    (30.00,   0.262),
    (33.05,   0.245),
    (50.00,   0.222),
    (62.42,   0.224),
    (85.70,   0.212),
    (117.93,  0.206),
]


def main():
    print('=' * 78)
    print('PROOF Phase C: Cluster aggregation with Langevin equipartition framework')
    print('=' * 78)
    print(f'Lattice: L = {L_LATTICE}, c = 1/√3')
    print()
    print('Computing G_L table...')
    gtable = green_table(L_LATTICE, max_dist=14)
    G0 = gtable[(0, 0, 0)]
    print(f'  G_L(0)        = {G0:.6f}')

    # Total field energy via the table
    total_E = total_field_energy(gtable)
    print(f'  total_field_E = {total_E:.6f}  (sum |G_L(r)|² over |r|≤14)')
    print()

    # Sanity check at central block: η(0) should give roughly k_linear = 1/4
    eta_0 = block_energy((0, 0, 0), gtable) / D_PER_SYMMETRY['O_h'] / total_E
    print(f'  η(origin) = block_E(0) × (1/4) / total_E = {eta_0:.6f}')
    print(f'    [Linear theorem at central block: k_linear = 1/4 = 0.250]')
    print()

    # Comparison
    print(f'  {"A":>7s} | {"k_emp":>7s} | {"R(A)":>5s} | {"k_pred":>8s} | {"k_pred/k_emp":>13s}')
    print(f'  {"-"*7} | {"-"*7} | {"-"*5} | {"-"*8} | {"-"*13}')
    for A, k_emp in EMPIRICAL:
        N = k_emp * A * A
        R = (3 * N / (4 * math.pi)) ** (1/3)
        k_pred = predict_k(R, gtable, total_E)
        ratio = k_pred / k_emp if k_emp > 0 else 0
        print(f'  {A:>7.2f} | {k_emp:>7.3f} | {R:>5.2f} | {k_pred:>8.4f} | {ratio:>13.4f}')

    print()
    print('INTERPRETATION:')
    print('  k_pred = Σ over cluster voxels of (block_E(x) × 1/d_G(x)) / total_E')
    print()
    print('  This is the Langevin-equipartition extension of the linear theorem.')
    print('  At central block: 1/d_G = 1/4 → recovers k_linear = 1/4 directly.')
    print('  At off-center blocks: 1/d_G < 1/4 → cluster contribution ↓ as far')
    print('  blocks contribute, suggesting k(A) ↓ with A.')
    print()
    print('  If k_pred/k_emp ≈ 1 across A: framework supports cluster physics.')
    print('  If k_pred ≠ k_emp (off by O(1)): framework is missing something.')


if __name__ == '__main__':
    main()
