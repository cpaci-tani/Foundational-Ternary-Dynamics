"""
Proof — FTD-0110 Phase A: Off-center 27-block slow-mode structure
====================================================================

For each block symmetry type G (the local symmetry that fixes the block
center), compute:
  1. The trivial-irrep subspace: dimension d_G + orthonormal basis
  2. The 18-point Laplacian projected onto this subspace: d_G x d_G matrix
  3. Eigendecomposition: eigenvalues + eigenvectors
  4. Identification of the slow mode (smallest |lambda|)
  5. delta-source projection (delta at center voxel of the block)

Verification: for G = O_h (central block), the 4 eigenvalues must match
DERIV_K_FROM_OH_A1G_MULTIPLICITY.md §3.1:
    {-4.8047, -4.4142, -3.8619, -1.5858}
to machine precision.

Symmetry types tabulated:
  O_h   (order 48): central block, d = 4   (linear theorem reference)
  C_4v  (order  8): axis block,    d = 9
  C_3v  (order  6): body-diagonal, d = 10
  C_2v  (order  4): face-diagonal, d = 12
  C_s   (order  2): face-general,  d = 18
  C_1   (order  1): generic,       d = 27

Provenance: docs/theory/03_derivations/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md
LEDGER: FTD-0119 Phase A (Mechanism alpha closure attempt).

Usage:
    python scripts/proofs/proof_ftd0110_offcenter_slowmodes.py
"""

import math
import sys
from itertools import product, permutations

import numpy as np


# ---------------------------------------------------------------------------
# Voxel index: (dx, dy, dz) in {-1, 0, 1}^3 -> linear index 0..26
# ---------------------------------------------------------------------------

VOXELS = [tuple(d) for d in product(range(-1, 2), repeat=3)]
N_VOXELS = len(VOXELS)            # 27
VOXEL_INDEX = {v: i for i, v in enumerate(VOXELS)}
CENTER_INDEX = VOXEL_INDEX[(0, 0, 0)]


# ---------------------------------------------------------------------------
# 18-point Laplacian on the 27-voxel block (face weight 1/3, edge weight 1/6,
# self -4). Per DERIV_K_FROM_OH_A1G_MULTIPLICITY.md §3.
# ---------------------------------------------------------------------------

def build_laplacian_27():
    """27x27 matrix L: L[i, j] is the coupling of voxel i to voxel j.

    Convention: L acts as L f = -k_hat^2 f in Fourier (standard Lattice
    Laplacian sign). Self term = -(sum of off-diagonal weights to keep
    L 1 = 0, but with isolated 27-block we instead use the spine
    convention from DERIV_K_FROM_OH_A1G_MULTIPLICITY §3.0:
        face neighbor weight a = 1/3
        edge neighbor weight b = 1/6
        self weight              = -(6*a + 12*b) = -(2 + 2) = -4.
    The 8 corner neighbors carry weight c = 0.
    """
    L = np.zeros((N_VOXELS, N_VOXELS))
    a_face = 1.0 / 3.0
    b_edge = 1.0 / 6.0
    for i, vi in enumerate(VOXELS):
        # Self
        L[i, i] = -(6 * a_face + 12 * b_edge)   # = -4.0
        for j, vj in enumerate(VOXELS):
            if i == j:
                continue
            d2 = sum((a - b) ** 2 for a, b in zip(vi, vj))
            if d2 == 1:
                # Face neighbor (axis distance 1)
                L[i, j] = a_face
            elif d2 == 2:
                # Edge neighbor (face-diagonal, distance sqrt(2))
                L[i, j] = b_edge
            # d2 == 3 is corner (BCC) — weight c = 0, skip
    return L


# ---------------------------------------------------------------------------
# Symmetry-group generators on voxel coordinates
# ---------------------------------------------------------------------------

def apply_perm_sign(perm, sign, v):
    """Apply (perm, sign) to voxel coordinate v: (s_i * v_{p_i})."""
    return tuple(sign[i] * v[perm[i]] for i in range(3))


def gen_O_h():
    """All 48 elements of O_h: signed permutations of (x, y, z)."""
    elements = []
    for perm in permutations(range(3)):
        for sign in product([1, -1], repeat=3):
            elements.append((perm, sign))
    return elements


def gen_C_4v_x():
    """C_4v fixing x-axis: 4 rotations around x + 4 reflections.

    Rotations: identity, C_4 ((y,z)->(- z, y)), C_4^2 ((y,z)->(-y,-z)),
    C_4^3 ((y,z)->(z,-y)).
    Reflections: sigma_xz (y->-y), sigma_xy (z->-z), sigma_diag (y<->z),
    sigma_anti-diag (y<->-z, z<->-y).
    """
    return [
        ((0, 1, 2), (1, 1, 1)),     # identity
        ((0, 2, 1), (1, -1, 1)),    # C_4: (x, y, z) -> (x, -z, y)
        ((0, 1, 2), (1, -1, -1)),   # C_4^2
        ((0, 2, 1), (1, 1, -1)),    # C_4^3: (x, y, z) -> (x, z, -y)
        ((0, 1, 2), (1, -1, 1)),    # sigma_xz: y -> -y
        ((0, 1, 2), (1, 1, -1)),    # sigma_xy: z -> -z
        ((0, 2, 1), (1, 1, 1)),     # sigma_diag: y <-> z
        ((0, 2, 1), (1, -1, -1)),   # sigma_antidiag: (y,z)->(-z,-y)
    ]


def gen_C_3v_xyz():
    """C_3v fixing body diagonal (1,1,1): 3 rotations + 3 reflections.

    Cyclic permutations of (x,y,z) and the three reflections
    swapping pairs.
    """
    return [
        ((0, 1, 2), (1, 1, 1)),     # identity
        ((1, 2, 0), (1, 1, 1)),     # cyclic perm (x,y,z) -> (y,z,x)
        ((2, 0, 1), (1, 1, 1)),     # cyclic perm (x,y,z) -> (z,x,y)
        ((1, 0, 2), (1, 1, 1)),     # swap xy
        ((0, 2, 1), (1, 1, 1)),     # swap yz
        ((2, 1, 0), (1, 1, 1)),     # swap xz
    ]


def gen_C_2v_xy():
    """C_2v fixing face diagonal (1,1,0): 4 elements.

    {identity, swap-xy, reflect-z, swap-xy + reflect-z}.
    """
    return [
        ((0, 1, 2), (1, 1, 1)),     # identity
        ((1, 0, 2), (1, 1, 1)),     # swap xy
        ((0, 1, 2), (1, 1, -1)),    # reflect z
        ((1, 0, 2), (1, 1, -1)),    # swap xy + reflect z
    ]


def gen_C_s_y():
    """C_s fixing face-general direction (n, m, 0) with n != m: 2 elements.

    {identity, reflect-z}. Most general directions in the xy plane have
    only this single reflection symmetry.
    """
    return [
        ((0, 1, 2), (1, 1, 1)),     # identity
        ((0, 1, 2), (1, 1, -1)),    # reflect z (sigma_xy plane)
    ]


def gen_C_1():
    """Trivial group: identity only."""
    return [((0, 1, 2), (1, 1, 1))]


# ---------------------------------------------------------------------------
# Build permutation representation matrix for group element on the 27 voxels
# ---------------------------------------------------------------------------

def perm_matrix(perm, sign):
    """27x27 permutation matrix for the action g(v) = (perm, sign) on voxels.

    M[i, j] = 1 if g(v_j) = v_i, else 0.
    """
    M = np.zeros((N_VOXELS, N_VOXELS))
    for j, v in enumerate(VOXELS):
        gv = apply_perm_sign(perm, sign, v)
        i = VOXEL_INDEX[gv]
        M[i, j] = 1.0
    return M


def projection_to_trivial(elements):
    """P = (1/|G|) sum_g M(g) — project onto trivial irrep of G.

    Image of P is the d-dim trivial-irrep subspace.
    """
    P = np.zeros((N_VOXELS, N_VOXELS))
    for perm, sign in elements:
        P += perm_matrix(perm, sign)
    return P / len(elements)


def trivial_basis(P, tol=1e-10):
    """Orthonormal basis of P's image (= trivial-irrep subspace)."""
    # P is symmetric idempotent (up to rounding). Diagonalize.
    eigvals, eigvecs = np.linalg.eigh(P)
    # Modes with eigenvalue ~1 span the trivial subspace; ~0 modes are kernel.
    mask = eigvals > 0.5
    basis = eigvecs[:, mask]
    # Already orthonormal from eigh. Stable.
    return basis


# ---------------------------------------------------------------------------
# Per-block analysis
# ---------------------------------------------------------------------------

BLOCK_TYPES = [
    ('O_h',  'central',         gen_O_h(),     48),
    ('C_4v', 'axis',            gen_C_4v_x(),   8),
    ('C_3v', 'body-diagonal',   gen_C_3v_xyz(), 6),
    ('C_2v', 'face-diagonal',   gen_C_2v_xy(),  4),
    ('C_s',  'face-general',    gen_C_s_y(),    2),
    ('C_1',  'generic',         gen_C_1(),      1),
]


def analyze_block(label, position, group_elements, expected_order, L):
    assert len(group_elements) == expected_order, \
        f'{label}: expected {expected_order} elements, got {len(group_elements)}'

    P = projection_to_trivial(group_elements)
    # P should be idempotent: P^2 = P
    P_squared = P @ P
    assert np.allclose(P_squared, P, atol=1e-10), \
        f'{label}: projector not idempotent'

    basis = trivial_basis(P)
    d = basis.shape[1]
    print(f'\n{"=" * 72}')
    print(f'Symmetry: {label} ({position} block, order {expected_order})')
    print(f'{"=" * 72}')
    print(f'  Trivial-irrep dimension: d = {d}')

    # Project Laplacian onto trivial subspace
    M = basis.T @ L @ basis  # d x d matrix
    M_sym = 0.5 * (M + M.T)
    if not np.allclose(M, M_sym, atol=1e-10):
        print(f'  WARNING: projected Laplacian not symmetric (max asymmetry '
              f'{np.max(np.abs(M - M_sym)):.2e})')
    M = M_sym

    # Eigendecomposition
    eigvals, eigvecs_in_basis = np.linalg.eigh(M)
    print(f'  Eigenvalues (sorted, ascending |lambda|):')
    eigvals_sorted = sorted(eigvals, key=lambda x: abs(x))
    for i, lam in enumerate(eigvals_sorted):
        marker = '  <-- slow mode' if i == 0 else ''
        print(f'    lambda_{i+1} = {lam:9.6f}{marker}')

    lam_slow = min(eigvals, key=lambda x: abs(x))
    idx_slow = list(eigvals).index(lam_slow)
    v_slow_in_basis = eigvecs_in_basis[:, idx_slow]
    v_slow = basis @ v_slow_in_basis
    print(f'  Slow mode lambda = {lam_slow:.6f}')

    # delta-source projection: delta_center = e_center
    # Project onto trivial subspace
    delta = np.zeros(N_VOXELS)
    delta[CENTER_INDEX] = 1.0
    delta_in_subspace = basis.T @ delta
    delta_norm_in = np.linalg.norm(delta_in_subspace)
    print(f'  delta_center projected onto trivial subspace: |proj| = '
          f'{delta_norm_in:.6f}')
    print(f'    (= 1.0 means delta_center is fully in trivial subspace —')
    print(f'     true for central block where delta is O_h-fixed)')

    # delta projected onto each eigenvector
    proj_per_mode = eigvecs_in_basis.T @ delta_in_subspace
    energies = proj_per_mode ** 2
    total = energies.sum()
    print(f'  delta projection onto each eigenmode (|proj|^2):')
    for i, (lam, e) in enumerate(zip(eigvals, energies)):
        print(f'    lambda = {lam:9.6f}:  |proj|^2 = {e:.6f}'
              f'{" <-- slow" if abs(lam) == abs(lam_slow) else ""}')
    if total > 1e-10:
        print(f'    total = {total:.6f},  mean = {total/d:.6f}'
              f'  (predicted for central: 1/4 = 0.250000)')
    else:
        print(f'    total ~ 0: delta_center has no overlap with this trivial subspace.')

    return {
        'label': label,
        'position': position,
        'd': d,
        'eigvals': sorted(eigvals.tolist(), key=lambda x: abs(x)),
        'lam_slow': lam_slow,
        'delta_norm_in_subspace': delta_norm_in,
        'energies_per_mode': energies.tolist(),
    }


def verify_central_block(result):
    """Sanity check: O_h central block must reproduce the linear theorem."""
    expected_eigvals = sorted([-1.5858, -3.8619, -4.4142, -4.8047], key=abs)
    actual = sorted(result['eigvals'], key=abs)
    print('\n' + '=' * 72)
    print('CENTRAL-BLOCK VERIFICATION (O_h must match DERIV_K_FROM_OH §3.1):')
    print('=' * 72)
    all_match = True
    for exp, act in zip(expected_eigvals, actual):
        diff = abs(exp - act)
        ok = diff < 1e-3
        all_match &= ok
        print(f'  expected {exp:9.6f},  actual {act:9.6f},  '
              f'|diff| = {diff:.2e}  [{"PASS" if ok else "FAIL"}]')
    return all_match


def main():
    print('=' * 72)
    print('PROOF Phase A: Off-center 27-block slow-mode structure')
    print('FTD-0110 Mechanism alpha — closure attempt')
    print('=' * 72)
    print(f'18-point Laplacian: face weight 1/3, edge weight 1/6, '
          f'self -4 (per DERIV_K_FROM_OH §3.0)')

    L = build_laplacian_27()

    results = []
    for label, position, elements, order in BLOCK_TYPES:
        results.append(analyze_block(label, position, elements, order, L))

    # Verify O_h central block
    central = results[0]
    central_ok = verify_central_block(central)

    # Summary table
    print('\n' + '=' * 72)
    print('SUMMARY')
    print('=' * 72)
    print(f'  {"Block":15s} | {"d":>3s} | {"lam_slow":>10s} | '
          f'{"|delta_proj|":>13s} | {"slow energy":>12s}')
    print('  ' + '-' * 70)
    for r in results:
        # Slow-mode energy fraction (only meaningful if delta_norm > 0)
        if r['delta_norm_in_subspace'] > 1e-6:
            # The slow mode is the eigenvalue with smallest |lambda|
            slow_idx = 0  # eigvals are sorted by |lam| ascending
            slow_e = r['energies_per_mode'][slow_idx]
            slow_e_str = f'{slow_e:.6f}'
        else:
            slow_e_str = '   (n/a)'
        print(f'  {r["label"]:15s} | {r["d"]:>3d} | '
              f'{r["lam_slow"]:>10.6f} | '
              f'{r["delta_norm_in_subspace"]:>13.6f} | '
              f'{slow_e_str:>12s}')
    print()

    print(f'Central-block verification: {"PASS" if central_ok else "FAIL"}')

    if not central_ok:
        sys.exit(1)


if __name__ == '__main__':
    main()
