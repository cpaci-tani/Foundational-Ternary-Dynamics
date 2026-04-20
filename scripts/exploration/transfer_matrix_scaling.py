#!/usr/bin/env python3
"""
Transfer Matrix Scaling: L=2 and L=3 Torus
===========================================
Test whether lambda_1/lambda_2 converges toward 47 = D_constraint
as the lattice size increases.

L=2: 2x2x2 torus, 81x81 transfer matrix (exact)
L=3: 3x3x3 torus, 19683x19683 transfer matrix (vectorized)

The Five Minds proposed: if the spectral gap converges to ln(47),
the framework integer D=47 is DERIVED from lattice geometry, not assumed.
"""

import numpy as np
from itertools import product
import time

print("=" * 78)
print("  TRANSFER MATRIX SCALING: L=2 and L=3 Torus")
print("  Target: does lambda_1/lambda_2 -> 47 for arbitrarily large L?")
print("=" * 78)

def build_18pt_laplacian(L):
    """Build the 18-point isotropic Laplacian on LxLxL torus."""
    N = L**3

    def idx(x, y, z):
        return (x % L) * L * L + (y % L) * L + (z % L)

    Lap = np.zeros((N, N))
    for i in range(N):
        x, y, z = i // (L*L), (i // L) % L, i % L
        Lap[i, i] += -4.0
        # 6 face neighbors (weight 1/3)
        for dx, dy, dz in [(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]:
            Lap[i, idx(x+dx, y+dy, z+dz)] += 1.0/3.0
        # 12 edge neighbors (weight 1/6)
        for dx, dy, dz in [(1,1,0),(1,-1,0),(-1,1,0),(-1,-1,0),
                            (1,0,1),(1,0,-1),(-1,0,1),(-1,0,-1),
                            (0,1,1),(0,1,-1),(0,-1,1),(0,-1,-1)]:
            Lap[i, idx(x+dx, y+dy, z+dz)] += 1.0/6.0

    return Lap

def transfer_matrix_L2():
    """Exact transfer matrix for L=2 (81x81)."""
    L = 2
    N = L**3  # 8
    n_slice = L * L  # 4 sites per slice

    Lap = build_18pt_laplacian(L)
    G = np.linalg.pinv(-Lap, rcond=1e-10)

    G_aa = G[:n_slice, :n_slice]
    G_ab = G[:n_slice, n_slice:]

    slice_cfgs = list(product([-1, 0, 1], repeat=n_slice))
    N_cfg = len(slice_cfgs)  # 81

    # Vectorized: build config matrix
    S = np.array(slice_cfgs, dtype=np.float64)  # (81, 4)

    # Quadratic forms
    q_aa = np.einsum('ai,ij,aj->a', S, G_aa, S)  # (81,)
    coupling = S @ G_ab @ S.T  # (81, 81)

    # T[a,b] = exp(0.5 * (2*coupling[a,b] + 0.5*(q_aa[a] + q_aa[b])))
    #        = exp(coupling[a,b] + 0.25*(q_aa[a] + q_aa[b]))
    T = np.exp(coupling + 0.25 * (q_aa[:, None] + q_aa[None, :]))

    eigs = np.sort(np.real(np.linalg.eigvals(T)))[::-1]
    return eigs, G

def transfer_matrix_L3():
    """Transfer matrix for L=3 (19683x19683) — vectorized."""
    L = 3
    N = L**3  # 27
    n_slice = L * L  # 9 sites per z-slice

    print(f"\n  Building {L}x{L}x{L} Laplacian ({N} sites)...")
    t0 = time.time()
    Lap = build_18pt_laplacian(L)
    G = np.linalg.pinv(-Lap, rcond=1e-10)
    print(f"  Green's function computed in {time.time()-t0:.1f}s")

    # Extract blocks for z=0 slice (sites 0..8) and z=1 slice (sites 9..17)
    # On 3x3x3 torus with 3 z-layers, the transfer matrix connects adjacent z-slices
    G_aa = G[:n_slice, :n_slice]           # intra z=0
    G_ab = G[:n_slice, n_slice:2*n_slice]  # z=0 to z=1

    N_cfg = 3**n_slice  # 19683
    print(f"  Enumerating {N_cfg} slice configurations...")
    t0 = time.time()

    # Build config matrix efficiently
    # Each config is a 9-element vector of {-1,0,1}
    S = np.array(list(product([-1, 0, 1], repeat=n_slice)), dtype=np.float32)
    print(f"  Config matrix: {S.shape}, built in {time.time()-t0:.1f}s")

    # Precompute quadratic forms
    print(f"  Computing quadratic forms...")
    t0 = time.time()
    q_aa = np.einsum('ai,ij,aj->a', S.astype(np.float64), G_aa, S.astype(np.float64))
    print(f"  q_aa computed in {time.time()-t0:.1f}s")

    # The coupling matrix S @ G_ab @ S.T is 19683x19683
    # Memory: 19683^2 * 8 bytes = ~2.9 GB for float64, ~1.5 GB for float32
    print(f"  Computing coupling matrix ({N_cfg}x{N_cfg})...")
    print(f"  Estimated memory: {N_cfg**2 * 4 / 1e9:.1f} GB (float32)")
    t0 = time.time()

    # Compute in float32 to save memory, then convert for eigenvalues
    SG = (S.astype(np.float64) @ G_ab).astype(np.float32)  # (19683, 9)
    coupling = SG @ S.T  # (19683, 19683) in float32
    print(f"  Coupling matrix computed in {time.time()-t0:.1f}s")

    # Build transfer matrix
    print(f"  Building transfer matrix...")
    t0 = time.time()
    q_outer = 0.25 * (q_aa[:, None].astype(np.float32) + q_aa[None, :].astype(np.float32))
    T = np.exp(coupling + q_outer)
    del coupling, q_outer, SG  # free memory
    print(f"  Transfer matrix built in {time.time()-t0:.1f}s")

    # Get top eigenvalues using scipy for efficiency
    print(f"  Computing top eigenvalues...")
    t0 = time.time()
    try:
        from scipy.sparse.linalg import eigsh
        # T is symmetric positive, use eigsh for top k
        T_f64 = T.astype(np.float64)
        del T
        eigs_top = eigsh(T_f64, k=min(50, N_cfg-2), which='LM', return_eigenvectors=False)
        eigs_top = np.sort(eigs_top)[::-1]
        print(f"  Top {len(eigs_top)} eigenvalues computed in {time.time()-t0:.1f}s")
        del T_f64
        return eigs_top, G
    except MemoryError:
        print(f"  MemoryError with full matrix! Trying power iteration...")
        # Fallback: power iteration for top eigenvalue
        v = np.random.randn(N_cfg).astype(np.float32)
        v /= np.linalg.norm(v)
        for _ in range(100):
            w = T @ v
            lam = np.linalg.norm(w)
            v = w / lam
        eigs_top = np.array([float(lam)])
        del T
        return eigs_top, G

# =========================================================================
# L = 2 ANALYSIS
# =========================================================================
print("\n" + "=" * 78)
print("  L = 2  (2x2x2 torus, 81x81 transfer matrix)")
print("=" * 78)

eigs_2, G_2 = transfer_matrix_L2()

print(f"\n  Top 10 eigenvalues:")
for i in range(min(10, len(eigs_2))):
    print(f"    lambda_{i+1:2d} = {eigs_2[i]:14.6f}")

ratio_2 = eigs_2[0] / eigs_2[1] if eigs_2[1] > 1e-10 else float('inf')
gap_2 = np.log(ratio_2) if ratio_2 > 0 else float('inf')
print(f"\n  lambda_1/lambda_2 = {ratio_2:.6f}")
print(f"  Spectral gap = ln(lambda_1/lambda_2) = {gap_2:.6f}")
print(f"  ln(47) = {np.log(47):.6f}")
print(f"  Gap / ln(47) = {gap_2 / np.log(47):.6f}")

# Check integer Green's function
print(f"\n  Integer Green's function analysis (128*G):")
G128 = np.round(128 * G_2).astype(int)
unique_vals = np.unique(G128)
print(f"  Unique entries of 128*G: {sorted(unique_vals)}")

# =========================================================================
# L = 3 ANALYSIS
# =========================================================================
print("\n" + "=" * 78)
print("  L = 3  (3x3x3 torus, 19683x19683 transfer matrix)")
print("=" * 78)

try:
    eigs_3, G_3 = transfer_matrix_L3()

    print(f"\n  Top 10 eigenvalues:")
    for i in range(min(10, len(eigs_3))):
        print(f"    lambda_{i+1:2d} = {eigs_3[i]:14.6f}")

    if len(eigs_3) >= 2:
        ratio_3 = eigs_3[0] / eigs_3[1] if eigs_3[1] > 1e-10 else float('inf')
        gap_3 = np.log(ratio_3) if ratio_3 > 0 else float('inf')
        print(f"\n  lambda_1/lambda_2 = {ratio_3:.6f}")
        print(f"  Spectral gap = ln(lambda_1/lambda_2) = {gap_3:.6f}")
        print(f"  ln(47) = {np.log(47):.6f}")
        print(f"  Gap / ln(47) = {gap_3 / np.log(47):.6f}")

        # Integer Green's function for L=3
        print(f"\n  Integer Green's function analysis:")
        # Find the denominator that makes G integer-like
        for denom in [27, 54, 81, 108, 162, 243, 486, 729]:
            Gd = denom * G_3
            residuals = np.abs(Gd - np.round(Gd))
            max_res = np.max(residuals)
            if max_res < 0.05:
                print(f"  {denom}*G is approximately integer (max residual = {max_res:.4f})")
                Gint = np.round(Gd).astype(int)
                unique = np.unique(Gint)
                print(f"  Unique entries: {sorted(unique)[:20]}{'...' if len(unique) > 20 else ''}")
                break

        # =========================================================================
        # EXTRAPOLATION
        # =========================================================================
        print("\n" + "=" * 78)
        print("  EXTRAPOLATION: L=2, L=3 -> L=infinity")
        print("=" * 78)

        print(f"\n  L=2: lambda_1/lambda_2 = {ratio_2:.6f}, gap = {gap_2:.6f}")
        print(f"  L=3: lambda_1/lambda_2 = {ratio_3:.6f}, gap = {gap_3:.6f}")

        # Linear extrapolation in 1/L
        # gap(L) = gap_inf + c/L
        # gap(2) = gap_inf + c/2
        # gap(3) = gap_inf + c/3
        # => gap_inf = (3*gap(3) - 2*gap(2))
        # => c = 6*(gap(2) - gap(3))
        gap_inf = 3*gap_3 - 2*gap_2
        c_coeff = 6*(gap_2 - gap_3)

        print(f"\n  Linear extrapolation (gap = gap_inf + c/L):")
        print(f"    gap_inf = {gap_inf:.6f}")
        print(f"    ln(47) = {np.log(47):.6f}")
        print(f"    gap_inf / ln(47) = {gap_inf / np.log(47):.6f}")

        # Quadratic extrapolation in 1/L^2
        # gap(L) = gap_inf + c/L^2
        gap_inf_q = (9*gap_3 - 4*gap_2) / 5
        print(f"\n  Quadratic extrapolation (gap = gap_inf + c/L^2):")
        print(f"    gap_inf = {gap_inf_q:.6f}")
        print(f"    ln(47) = {np.log(47):.6f}")
        print(f"    gap_inf / ln(47) = {gap_inf_q / np.log(47):.6f}")

        # Ratio extrapolation
        ratio_inf = np.exp(gap_inf)
        ratio_inf_q = np.exp(gap_inf_q)
        print(f"\n  Extrapolated ratio lambda_1/lambda_2:")
        print(f"    Linear:    {ratio_inf:.4f}  (target: 47)")
        print(f"    Quadratic: {ratio_inf_q:.4f}  (target: 47)")

        # Additional eigenvalue ratios at L=3
        print(f"\n  Eigenvalue ratios at L=3:")
        for i in range(1, min(20, len(eigs_3))):
            if eigs_3[i] > 1e-10:
                r = eigs_3[0] / eigs_3[i]
                flag = ""
                if abs(r - 3) < 0.3: flag = " *** ~ N_c = 3 ***"
                elif abs(r - 4) < 0.3: flag = " *** ~ N_base = 4 ***"
                elif abs(r - 7) < 0.5: flag = " *** ~ b_3 = 7 ***"
                elif abs(r - 13) < 1: flag = " *** ~ N_eff = 13 ***"
                elif abs(r - 16) < 1: flag = " *** ~ |Aut|^2 = 16 ***"
                elif abs(r - 27) < 2: flag = " *** ~ N_c^3 = 27 ***"
                elif abs(r - 42) < 3: flag = " *** ~ 42 ***"
                elif abs(r - 47) < 3: flag = " *** ~ D = 47 ***"
                elif abs(r - np.sqrt(13)) < 0.2: flag = " *** ~ sqrt(N_eff) ***"
                print(f"    lambda_1/lambda_{i+1:2d} = {r:12.4f}{flag}")

    # KEY TEST: framework polynomial mod 27
    print(f"\n  Framework polynomial P(x) = (x-3)(x-4)(x-7)(x-13) at L=3:")
    print(f"  P(x) mod 27 = (x-1)(x-N_c)(x^2 + N_base*x + N_eff)")

    # Check degeneracy structure
    if len(eigs_3) >= 10:
        print(f"\n  Degeneracy structure (top 20, rounded to 0.01):")
        rounded = np.round(eigs_3[:min(50, len(eigs_3))], 2)
        unique_eigs, counts = np.unique(rounded, return_counts=True)
        for e, c in sorted(zip(unique_eigs, counts), key=lambda x: -x[0])[:20]:
            print(f"    eigenvalue {e:14.4f}: degeneracy {c}")

except MemoryError:
    print("\n  *** MemoryError: L=3 transfer matrix too large for available RAM ***")
    print("  Falling back to deeper L=2 analysis...")

    # Additional L=2 analysis
    print(f"\n  Extended L=2 eigenvalue ratios:")
    for i in range(len(eigs_2)):
        for j in range(i+1, len(eigs_2)):
            if eigs_2[j] > 0.01:
                r = eigs_2[i] / eigs_2[j]
                if abs(r - 47) < 5:
                    print(f"    lambda_{i+1}/lambda_{j+1} = {r:.4f} ~ 47?")

except Exception as e:
    print(f"\n  *** Error: {e} ***")
    import traceback
    traceback.print_exc()

print("\n" + "=" * 78)
print("  DONE")
print("=" * 78)
