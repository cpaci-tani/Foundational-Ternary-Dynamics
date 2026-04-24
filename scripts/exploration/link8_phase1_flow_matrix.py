"""
Link 8 Phase 1 — Analytical linearized flow matrix for a two-coupling (SC+FCC, BCC)
Gaussian lattice theory under 2x2x2 block-averaging.

Purpose
-------
The engine's 18-point coupling stencil is structurally BCC-orthogonal
(see AUDIT_LINK8_CLOSURE.md Option beta). If we EXTEND the engine to
carry an independent BCC coupling g_BCC alongside the existing
g_SCFCC, the two couplings flow together under a block-spin RG step.
That flow is 2-dimensional; its 2x2 flow matrix M has a characteristic
polynomial.

Candidate 1's implicit hypothesis was that the master quadratic
x^2 - 16 G*^2 x + 16 G*^3 = 0 is the characteristic polynomial of
an RG step. In the 2-coupling picture that becomes a concrete prediction:
    trace(M) = 16 G*^2 = 140.0601
    det(M)   = 16 G*^3 = 414.3924

This script computes M numerically (linearized about the Gaussian fixed
point with g_SCFCC = 1, g_BCC = 0) via the standard Wilsonian formula

    sigma_eff(K) = 1 / sum_{m in {0,1}^3} |F(K+pi m)|^2 / sigma(K+pi m)

projected onto the {sigma_SCFCC, sigma_BCC} basis in the coarse BZ.

Output: M, trace(M), det(M), eigenvalues, and deviations from the target
(trace, det) = (140.0601, 414.3924).

Interpretation
--------------
If trace(M) and det(M) match targets within 10%: Phase 1 PASSES; proceed
to Phase 2 (engine BCC-coupling extension + Candidate 1 rerun).
If trace(M) and det(M) are O(1) or otherwise far from targets: Phase 1
closes negative analytically; no engine code needed.

Numerical method
----------------
Grid: fine BZ (-pi, pi)^3 with N = 64 per axis (enough for 4-5 sig fig
on the integrals). Coarse BZ is (-pi/2, pi/2)^3. Aliasing sum is evaluated
by broadcasting the fine grid and reshaping into (N_c, N_c, N_c, 8) for
the eight aliased modes m in {0,1}^3.

Author: Phase 1 analytical gate, 2026-04-20.
"""

from __future__ import annotations

import math
import numpy as np
import mpmath


# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------
G_STAR = float(mpmath.gamma(mpmath.mpf('1')/4) / mpmath.gamma(mpmath.mpf('3')/4))
TRACE_TARGET =  16 * G_STAR**2              # 140.0601354...
DET_TARGET   =  16 * G_STAR**3              # 414.3924377...
ROOT_HI      = 0.5 * (TRACE_TARGET + math.sqrt(TRACE_TARGET**2 - 4*DET_TARGET))  # 137.036
ROOT_LO      = 0.5 * (TRACE_TARGET - math.sqrt(TRACE_TARGET**2 - 4*DET_TARGET))  # 3.024


# ---------------------------------------------------------------------------
# Grid
# ---------------------------------------------------------------------------
N_FINE = 64                 # grid points per axis on fine BZ
N_COARSE = N_FINE // 2      # = 32
assert N_FINE % 2 == 0

k_fine_1d = np.linspace(-math.pi, math.pi, N_FINE, endpoint=False)
k_coarse_1d = np.linspace(-math.pi/2, math.pi/2, N_COARSE, endpoint=False)


def sigma_SCFCC(kx, ky, kz):
    cx, cy, cz = np.cos(kx), np.cos(ky), np.cos(kz)
    return 1.0 - (cx + cy + cz)/6.0 - (cx*cy + cx*cz + cy*cz)/6.0


def sigma_BCC(kx, ky, kz):
    cx, cy, cz = np.cos(kx), np.cos(ky), np.cos(kz)
    return 1.0 - cx * cy * cz


def Fsq(kx, ky, kz):
    """|F(k)|^2 for 2x2x2 arithmetic-mean block filter."""
    return (np.cos(kx/2)**2 * np.cos(ky/2)**2 * np.cos(kz/2)**2)


# ---------------------------------------------------------------------------
# Build fine-grid quantities
# ---------------------------------------------------------------------------
KX_F, KY_F, KZ_F = np.meshgrid(k_fine_1d, k_fine_1d, k_fine_1d, indexing='ij')
sig_SF_fine = sigma_SCFCC(KX_F, KY_F, KZ_F)
sig_BC_fine = sigma_BCC(KX_F, KY_F, KZ_F)
Fsq_fine = Fsq(KX_F, KY_F, KZ_F)

# Guard against the exact-zero at k=0 and other zero modes.
# sig_SCFCC(k)=0 at k=0. sig_BCC(k)=0 at k=0 AND at (pi,pi,0) and permutations.
# Handle by a "skip at zero modes" pattern: the integrand contributions at
# these measure-zero points are dropped, which is the correct regularization.
# Replace sig_SF with a large finite value so 1/sig_SF = 0 at those points;
# keep sig_BC as its actual value (appears in numerator only).
eps_reg = 1e10
zero_mask_SF = np.abs(sig_SF_fine) < 1e-14
sig_SF_safe = np.where(zero_mask_SF, eps_reg, sig_SF_fine)
sig_BC_safe = sig_BC_fine  # appears only in numerators; no singularity issue


# ---------------------------------------------------------------------------
# Aliasing sum: reshape fine grid into coarse-grid + alias axis
# ---------------------------------------------------------------------------
# k_fine index layout: N_fine points, ordered linearly in k.
# For each coarse index i_c in [0, N_COARSE), the 2 fine indices that alias
# to the same coarse K are at i_c and i_c + N_COARSE (shifted by pi).
#
# Reshape axis: (N_fine,) -> (N_coarse, 2) in the order (low-k, pi-shifted).
# Use np.fft.fftshift logic: but with our endpoint=False linspace from -pi
# to pi, the k values are
#    k_fine_1d[i] = -pi + 2*pi*i/N_fine
#    for i in [0..N_coarse):  k = -pi + 2*pi*i/N_fine ∈ [-pi, 0)
#    for i in [N_coarse..N_fine): k ∈ [0, pi)
# And the coarse k range is [-pi/2, pi/2).
# So the coarse index i_c in [0..N_coarse):
#    k_coarse = -pi/2 + 2*pi*i_c/N_fine
# Corresponding fine-k indices (where fine-k = coarse-k + pi*m, m ∈ {0, 1}):
#    For m=0: fine-k = coarse-k ∈ [-pi/2, pi/2).
#             fine_idx = (coarse-k + pi) * N_fine / (2 pi)
#                      = (coarse-k - (-pi)) * N_fine / (2 pi)
#                      = i_c + N_COARSE/2   [since coarse-k starts at -pi/2
#                                             which is fine idx N_COARSE/2
#                                             counting from 0 at fine k=-pi]
#    For m=1: fine-k = coarse-k + pi (wrap to (-pi, pi] range)
#             If coarse-k < 0: fine-k = coarse-k + pi ∈ [pi/2, pi)
#                fine_idx = (coarse-k + 2*pi) * N_fine / (2 pi)  ...
#
# This is getting fragile; let me use explicit index arithmetic.
#
# Simpler: construct the alias index map directly.
# ---------------------------------------------------------------------------

def coarse_to_fine_idx(i_c):
    """Given a coarse-grid index i_c in [0, N_COARSE), return the list of
    2 fine-grid indices whose k values are aliased (k and k + pi).
    """
    # coarse k value: k_coarse = -pi/2 + 2*pi*i_c/N_fine
    # fine index for k_coarse: (k_coarse + pi) * N_fine / (2*pi) = (N_fine/4 + i_c)
    i_f_m0 = N_FINE // 4 + i_c
    # fine index for k_coarse + pi: shift by N_fine/2
    i_f_m1 = (i_f_m0 + N_FINE // 2) % N_FINE
    return i_f_m0, i_f_m1


# Build alias tensors: shape (N_COARSE, N_COARSE, N_COARSE, 2, 2, 2)
# where the last three axes index m_x, m_y, m_z each in {0, 1}.
alias_ix = np.zeros((N_COARSE, 2), dtype=int)
for i_c in range(N_COARSE):
    alias_ix[i_c] = coarse_to_fine_idx(i_c)

# Indexing pattern: for coarse indices (i, j, k) and alias indices (mx, my, mz):
# fine_ix[i, j, k, mx] = alias_ix[i, mx], similarly for y, z.
# Use np.ix_ / broadcasting.

def gather_alias(fine_array):
    """Gather fine_array (N_FINE, N_FINE, N_FINE) into coarse-and-alias
    shape (N_COARSE, N_COARSE, N_COARSE, 8) via the 8 aliased modes."""
    ix = alias_ix[:, :, None, None, None, None]    # (N_c, 2, 1,1,1,1)  -> i -> mx
    iy = alias_ix[None, None, :, :, None, None]    # (1,1,N_c,2,1,1)
    iz = alias_ix[None, None, None, None, :, :]    # (1,1,1,1,N_c,2)
    # Broadcast to shape (N_c, 2, N_c, 2, N_c, 2)
    ix_b, iy_b, iz_b = np.broadcast_arrays(ix, iy, iz)
    gathered = fine_array[ix_b, iy_b, iz_b]       # (N_c,2, N_c,2, N_c,2)
    # Reorder/reshape to (N_c, N_c, N_c, 8) with last axis = mx*4+my*2+mz
    gathered = np.transpose(gathered, (0, 2, 4, 1, 3, 5))  # (N_c,N_c,N_c, 2,2,2)
    gathered = gathered.reshape(N_COARSE, N_COARSE, N_COARSE, 8)
    return gathered


# Build alias-sums of various fine-grid functions
sig_SF_alias = gather_alias(sig_SF_safe)     # (N_c, N_c, N_c, 8)
sig_BC_alias = gather_alias(sig_BC_safe)
Fsq_alias    = gather_alias(Fsq_fine)


# ---------------------------------------------------------------------------
# Free-theory coarse propagator G_0 and interaction kernel H_BCC
# ---------------------------------------------------------------------------
# H_0(K_c) = Σ_m |F(k_m)|² / sigma_SCFCC(k_m)
# H_BCC(K_c) = Σ_m |F(k_m)|² · sigma_BCC(k_m) / sigma_SCFCC(k_m)^2

H_0   = np.sum(Fsq_alias / sig_SF_alias, axis=-1)                      # (N_c, N_c, N_c)
H_BCC = np.sum(Fsq_alias * sig_BC_alias / sig_SF_alias**2, axis=-1)    # (N_c, N_c, N_c)

# Free-theory coarse inverse propagator sigma_eff_0 = 1/H_0
sig_eff_0 = 1.0 / H_0    # (N_c, N_c, N_c)


# Coarse-lattice basis operators (evaluated on coarse BZ)
KX_C, KY_C, KZ_C = np.meshgrid(k_coarse_1d, k_coarse_1d, k_coarse_1d, indexing='ij')
sig_SF_c = sigma_SCFCC(KX_C, KY_C, KZ_C)
sig_BC_c = sigma_BCC(KX_C, KY_C, KZ_C)


# ---------------------------------------------------------------------------
# Inner product on coarse BZ
# ---------------------------------------------------------------------------
# <f, g>_c = (1 / |coarse BZ|) * int_coarse f g
#         = mean over coarse grid points (since grid uniformly samples)
# We use the MEAN normalization (not integral with 1/(2pi)^3), because any
# overall normalization cancels in the matrix equation.

def inner(f, g):
    return np.mean(f * g)


A_SF_SF = inner(sig_SF_c, sig_SF_c)   # <sigma_SCFCC, sigma_SCFCC>_c
A_SF_BC = inner(sig_SF_c, sig_BC_c)   # <sigma_SCFCC, sigma_BCC>_c
A_BC_BC = inner(sig_BC_c, sig_BC_c)

# Right-hand-side inner products:
# sigma_eff(K) ≈ sigma_eff_0(K) * [dg_SCFCC + dg_BCC * H_BCC / H_0]
# Match to dg'_SCFCC * sigma_SCFCC(K) + dg'_BCC * sigma_BCC(K)
# Project both sides onto sigma_SCFCC(K) and sigma_BCC(K):
#
# dg'_SCFCC * A_SF_SF + dg'_BCC * A_SF_BC = <sigma_eff_0 * [dg_SCFCC + dg_BCC H_BCC/H_0], sigma_SCFCC>_c
#                                         = dg_SCFCC * <sigma_eff_0, sigma_SCFCC>_c
#                                         + dg_BCC * <sigma_eff_0 * H_BCC/H_0, sigma_SCFCC>_c
# dg'_SCFCC * A_SF_BC + dg'_BCC * A_BC_BC = dg_SCFCC * <sigma_eff_0, sigma_BCC>_c
#                                         + dg_BCC * <sigma_eff_0 * H_BCC/H_0, sigma_BCC>_c

# RHS inner products
P_SF    = inner(sig_eff_0,                 sig_SF_c)   # dg_SCFCC coefficient -> sigma_SCFCC channel
P_BC    = inner(sig_eff_0,                 sig_BC_c)   # dg_SCFCC coefficient -> sigma_BCC channel
Q_SF    = inner(sig_eff_0 * H_BCC / H_0,   sig_SF_c)   # dg_BCC coefficient   -> sigma_SCFCC channel
Q_BC    = inner(sig_eff_0 * H_BCC / H_0,   sig_BC_c)   # dg_BCC coefficient   -> sigma_BCC channel

# 2x2 linear system:
#    G * (dg'_SCFCC, dg'_BCC)^T = (P_SF dg_SCFCC + Q_SF dg_BCC, P_BC dg_SCFCC + Q_BC dg_BCC)^T
# where G = [[A_SF_SF, A_SF_BC], [A_SF_BC, A_BC_BC]] (Gram matrix)

G_mat = np.array([[A_SF_SF, A_SF_BC],
                  [A_SF_BC, A_BC_BC]])
RHS_from_SF = np.array([P_SF, P_BC])
RHS_from_BC = np.array([Q_SF, Q_BC])

# Flow matrix M: (dg'_SCFCC, dg'_BCC)^T = M @ (dg_SCFCC, dg_BCC)^T
# M = G^{-1} @ [RHS_from_SF  RHS_from_BC]
RHS_matrix = np.column_stack([RHS_from_SF, RHS_from_BC])   # 2x2
M = np.linalg.solve(G_mat, RHS_matrix)

tr_M  = np.trace(M)
det_M = np.linalg.det(M)
evals = np.linalg.eigvals(M)


# ---------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------
print("="*72)
print("  Link 8 Phase 1 — Linearized 2-coupling flow matrix M")
print("="*72)
print(f"  G*           = {G_STAR:.10f}")
print(f"  Trace target = 16 G*^2 = {TRACE_TARGET:.6f}")
print(f"  Det   target = 16 G*^3 = {DET_TARGET:.6f}")
print(f"  Root targets = ({ROOT_HI:.6f}, {ROOT_LO:.6f}) = (1/alpha, N_c)")
print()
print(f"  Grid: N_fine = {N_FINE}, N_coarse = {N_COARSE}")
print()
print(f"  Gram matrix G (basis {{sigma_SCFCC, sigma_BCC}} on coarse BZ):")
print(f"    A_SF_SF = <sigma_SF, sigma_SF> = {A_SF_SF:.6f}")
print(f"    A_SF_BC = <sigma_SF, sigma_BC> = {A_SF_BC:.6f}")
print(f"    A_BC_BC = <sigma_BC, sigma_BC> = {A_BC_BC:.6f}")
print()
print(f"  Projection inner products (sigma_eff_0 = 1/H_0):")
print(f"    <sigma_eff_0, sigma_SF>              = {P_SF:.6f}")
print(f"    <sigma_eff_0, sigma_BC>              = {P_BC:.6f}")
print(f"    <sigma_eff_0 * H_BCC/H_0, sigma_SF>  = {Q_SF:.6f}")
print(f"    <sigma_eff_0 * H_BCC/H_0, sigma_BC>  = {Q_BC:.6f}")
print()
print(f"  Flow matrix M (columns = (g_SCFCC, g_BCC); rows = (g'_SCFCC, g'_BCC)):")
print(f"    M = [[{M[0,0]:+.6f}, {M[0,1]:+.6f}],")
print(f"         [{M[1,0]:+.6f}, {M[1,1]:+.6f}]]")
print()
print(f"  trace(M) = {tr_M:+.6f}   (target {TRACE_TARGET:.6f}, ratio = {tr_M/TRACE_TARGET:.4f})")
print(f"  det(M)   = {det_M:+.6f}   (target {DET_TARGET:.6f}, ratio = {det_M/DET_TARGET:.4f})")
print(f"  eigenvalues = {evals}")
print()

# Verdict
trace_dev = abs(tr_M - TRACE_TARGET) / TRACE_TARGET
det_dev   = abs(det_M - DET_TARGET) / abs(DET_TARGET)
print("="*72)
print(f"  trace deviation from target = {100*trace_dev:.2f}%")
print(f"  det   deviation from target = {100*det_dev:.2f}%")
print()
if trace_dev < 0.10 and det_dev < 0.10:
    print("  VERDICT: trace and det match targets within 10%.")
    print("  Phase 1 PASSES. Proceed to Phase 2 (engine BCC-coupling toggle + Candidate 1 rerun).")
elif max(abs(tr_M), abs(det_M)) < 10.0:
    print("  VERDICT: trace and det are O(1) -- orders of magnitude below targets.")
    print("  Phase 1 CLOSES NEGATIVE analytically. No engine code warranted.")
    print("  The linearized flow matrix eigenvalues are scaling dimensions,")
    print("  not physical couplings; they cannot match 1/alpha and N_c by")
    print("  dimensional analysis.")
else:
    print("  VERDICT: ambiguous — matrix elements are of intermediate size.")
    print("  Inspect matrix structure before deciding on Phase 2.")
print("="*72)
