"""FTD-0266: Mechanism beta, sustained-kinetics variant
======================================================
Pre-registration (in-session; thresholds stated BEFORE compute):

Physical model
--------------
Pre-genesis dynamics is exactly linear (symplectic-Euler, 18-pt stencil, alpha=1/18).
Instead of asking 'did |J(delta,t)| ever exceed threshold?' (envelope — FTD-0265),
we ask 'what is the time-integrated Boltzmann probability that genesis fired?'

Boltzmann kinetics (from voxel.h, engine canonical):
  excess(delta, t, A) = max(0,  A * |J_unit(delta,t)| - 1 )          [normalized: K_GENESIS cancels]
  p(delta, t, A)      = 1 - exp( -N_c * excess(delta,t,A) )           [N_c = K_GENESIS/K_MANIFEST = 3]
  P_genesis(delta, A) = 1 - prod_{t=0}^{T}  (1 - p(delta,t,A))
  E[N(A)]             = sum_{delta, r<=RMAX} P_genesis(delta,A)

Variants:
  (a) no per-tick Gauss projection (same as FTD-0265 variant a)
  (b) per-tick FFT divergence projection (same as FTD-0265 variant b)

FROZEN THRESHOLDS (do not change after this docstring):

  T1 (count at A=10): E[N(10)] IN [2.0, 8.0]
       rationale: measured 4.0; the dwell-time model should suppress the
       33-voxel envelope prediction; acceptable if it lands within 2x of measured.

  T2 (shape): log10-RMS( log10(E[N(A)]/N_meas(A)) ) <= 0.20
       across the frozen FTD-0263 staircase (same criterion as FTD-0265).

  T3 (onset): second-voxel onset A_join(rank 2) IN [7.0, 11.0]
       rationale: measured ~8.75; the dwell-time model should move the envelope's
       4.62 upward into the measured range (or close to it).

  Verdict:
    DWELL-SUPPORTED: T1 AND T2 AND T3 in BOTH variants, OR T1 AND T2 in at least one
    DWELL-PARTIAL:   T1 OR T2 in at least one variant, but not SUPPORTED
    DWELL-FAIL:      neither T1 nor T2 in any variant

  Priors (stated before compute):
    DWELL-SUPPORTED: 35%
    DWELL-PARTIAL:   40%
    DWELL-FAIL:      25%
  Reasoning: pencil calc shows peak Boltzmann rate at A=10 rank-8 voxels is ~0.89;
  even short transient crossings (1-2 ticks) produce P_fire ~ 0.89; so dwell-time
  alone may not suppress to measured levels — survival physics may still be needed.
  But the actual time-series shape of the pulse is unknown; if the crossing is very
  narrow (< 1 tick dwell), the correction is large.

Frozen FTD-0263 staircase (18 points, from run_of_record):
  A:     9.0  9.5 10.0 10.5 11.0 11.5 12.0 12.5 13.0 13.5 14.0 14.5 15.0 15.5 16.0 17.0 18.0
  N_bar: 2.00 3.00 4.00 5.20 6.00 7.80 8.40 10.40 11.80 13.40 16.40 18.00 19.80 20.80 21.60 24.20 24.60

Parameters: L=64, T_MAX=110, RMAX=12, ALPHA=1/18, N_c=3
"""

import numpy as np
import math

# ── constants ──────────────────────────────────────────────────────────────────
L      = 64
T_MAX  = 110
RMAX   = 12
ALPHA  = 1.0 / 18.0
N_c    = 3.0    # K_GENESIS / K_MANIFEST = 1.533 / 0.511

# Frozen FTD-0263 staircase
A_GRID  = [9.0, 9.5, 10.0, 10.5, 11.0, 11.5, 12.0, 12.5, 13.0, 13.5, 14.0, 14.5, 15.0, 15.5, 16.0, 17.0, 18.0]
N_MEAS  = [2.00, 3.00, 4.00, 5.20, 6.00, 7.80, 8.40, 10.40, 11.80, 13.40, 16.40, 18.00, 19.80, 20.80, 21.60, 24.20, 24.60]

# ── dispersion self-check ──────────────────────────────────────────────────────
def _dispersion_check():
    """
    18-pt stencil eigenvalue for k=(k,0,0):
      Faces (weight 2): 2*cos(k)+4  (2 x-faces + 2 y-faces + 2 z-faces contribute 2cos k, 2, 2)
      Edges (weight 1): 4*cos(k)+4  (4 xy-edges + 4 xz-edges give cos k each; 4 yz-edges give 1 each)
      Centre: -24
      → raw eigenvalue = 12cos(k) + 12 - 24 = 12*(cos k - 1)
      → μ = ALPHA * 12*(cos k - 1) = (2/3)*(cos k - 1)  (negative definite, stable)

    Symplectic-Euler characteristic: λ² - (2+μ)λ + 1 = 0
      Oscillation: ω = arccos(1 + μ/2)
    """
    n = 4
    k = 2*math.pi*n / L
    mu = ALPHA * 12 * (math.cos(k) - 1)          # = (2/3)*(cos k - 1) < 0
    om = math.acos(max(-1.0, 1.0 + mu/2))          # symplectic-Euler exact dispersion
    c  = 1.0/math.sqrt(3)
    om_cont = 2.0*c*math.sin(k/2)                  # continuum half-step form
    print(f"dispersion self-check (axis n={n}): omega={om:.4f} vs 2c*sin(k/2)={om_cont:.4f}  (alpha=1/18 pin)")

_dispersion_check()

# ── stencil operators ──────────────────────────────────────────────────────────
def laplacian18(f):
    """18-pt stencil Laplacian, alpha=1/18 (face weight 2, edge weight 1)."""
    out = np.zeros_like(f)
    for c in range(3):
        # Face neighbors (weight 2)
        for ax in range(3):
            out[..., c] += 2*(np.roll(f[..., c], -1, ax) + np.roll(f[..., c], 1, ax))
        # Edge neighbors (weight 1): all 12 edges
        for ax1, ax2 in [(0,1),(0,2),(1,2)]:
            for s1, s2 in [(-1,-1),(-1,1),(1,-1),(1,1)]:
                out[..., c] += np.roll(np.roll(f[..., c], s1, ax1), s2, ax2)
        # Subtract center (18 neighbors sum = 2*6 + 1*12 = 24)
        out[..., c] -= 24 * f[..., c]
    return ALPHA * out

def project_divfree(J):
    """FFT-based divergence projection using central-difference symbol."""
    Jf = np.fft.fftn(J, axes=(0,1,2))
    kx = np.fft.fftfreq(L, d=1.0/(2*np.pi))
    ky, kz = np.fft.fftfreq(L, d=1.0/(2*np.pi)), np.fft.fftfreq(L, d=1.0/(2*np.pi))
    KX, KY, KZ = np.meshgrid(kx, ky, kz, indexing='ij')
    # Central-difference symbol
    Sx = np.sin(KX/2 * 2*np.pi / (2*np.pi)) * L / np.pi if L > 1 else KX
    # Proper: sin(k_j) for central-difference
    Sx = np.sin(KX)
    Sy = np.sin(KY)
    Sz = np.sin(KZ)
    denom = Sx**2 + Sy**2 + Sz**2
    denom[0,0,0] = 1.0  # avoid division by zero at DC
    div = Sx*Jf[...,0] + Sy*Jf[...,1] + Sz*Jf[...,2]
    Jf[...,0] -= div * Sx / denom
    Jf[...,1] -= div * Sy / denom
    Jf[...,2] -= div * Sz / denom
    return np.real(np.fft.ifftn(Jf, axes=(0,1,2)))

# ── run with trajectory storage ────────────────────────────────────────────────
def run_kinetics(project=False):
    """
    Return:
      P_genesis: shape (L,L,L) — cumulative Boltzmann probability of having fired by T_MAX
                 for unit injection (A=1); actual P for amplitude A computed from A-scaled trajectory
      J_traj: dict {delta_index: list of |J(delta,t)| values} for r<=RMAX
              (stored as 1D list over t for each voxel)
    """
    half = L // 2

    # Pre-compute voxels within r <= RMAX
    xs = np.arange(L)
    X, Y, Z = np.meshgrid(xs, xs, xs, indexing='ij')
    Xc = np.where(X <= half, X, X - L)
    Yc = np.where(Y <= half, Y, Y - L)
    Zc = np.where(Z <= half, Z, Z - L)
    r2 = Xc**2 + Yc**2 + Zc**2
    mask = r2 <= RMAX**2  # shape (L,L,L)

    # Build sorted list of analysis voxels by r2 (for ranked spectrum)
    voxels = np.argwhere(mask)  # (N_vox, 3)
    vox_r2 = r2[mask]           # (N_vox,)
    sort_idx = np.argsort(-vox_r2)  # descending r2 for rank by envelopes later
    voxels = voxels[sort_idx]
    vox_r2 = vox_r2[sort_idx]
    N_vox = len(voxels)

    # Initialize fields
    J = np.zeros((L, L, L, 3))
    V = np.zeros((L, L, L, 3))

    # Unit injection at array ORIGIN (0,0,0) — periodic BCs make this equivalent to
    # injecting at any site; centering analysis sphere on (0,0,0) is then correct.
    # (Injecting at half,half,half with Xc=where(X<=half,X,X-L) would center the
    #  analysis sphere on the WRONG site and give all-zero trajectories.)
    J[0, 0, 0, 0] = 1.0
    if project:
        J = project_divfree(J)

    # Store per-voxel |J| trajectories (shape: N_vox x T_MAX)
    # Use float32 to save memory
    traj = np.zeros((N_vox, T_MAX), dtype=np.float32)

    for t in range(T_MAX):
        # Record |J| at each analysis voxel
        Jmag = np.sqrt(J[...,0]**2 + J[...,1]**2 + J[...,2]**2)
        for vi, (ix, iy, iz) in enumerate(voxels):
            traj[vi, t] = Jmag[ix, iy, iz]

        # Symplectic Euler step
        V = V + laplacian18(J)
        J = J + V
        if project:
            J = project_divfree(J)

    return traj, voxels, vox_r2

def compute_P_genesis(traj, A_vals):
    """
    For each voxel and each A, compute P_genesis = 1 - prod_t (1 - p(t)).
    p(t) = 1 - exp(-N_c * max(0, A * |J_unit(t)| - 1))

    Returns:
      EN: list of E[N(A)] for each A in A_vals (summed over all voxels in mask)
      P_fire_by_rank: dict of per-A P_genesis sorted by descending envelope (for ranked spectrum)
    """
    N_vox, T = traj.shape
    # Envelope for ranking
    envelope = traj.max(axis=1)  # (N_vox,)
    rank_idx = np.argsort(-envelope)  # descending envelope

    EN = []
    for A in A_vals:
        # excess(v, t) = max(0, A * traj[v,t] - 1)
        excess = np.maximum(0.0, A * traj - 1.0)  # (N_vox, T)
        # p(v,t) = 1 - exp(-N_c * excess)
        p = 1.0 - np.exp(-N_c * excess)  # (N_vox, T)
        # log(1-p(v,t)) = -N_c * excess(v,t) for the product
        log_no_fire = -N_c * excess  # = log(1-p) since exp(-N_c*excess) = 1-p
        # P_genesis(v) = 1 - exp(sum_t log(1-p(v,t))) = 1 - exp(-N_c * sum_t excess(v,t))
        P_genesis = 1.0 - np.exp(np.sum(log_no_fire, axis=1))  # (N_vox,)
        EN.append(float(P_genesis.sum()))

    # For the ranked spectrum: compute the cumulative P_genesis for representative A
    # Show for A=10.0 and A=9.0 (the tricky ones)
    return EN, rank_idx, envelope

def broken_fit(A_arr, N_arr):
    """Simple broken power law fit: N = a * A^p1 for A < A_knee, a * A_knee^(p1-p2) * A^p2 for A >= A_knee."""
    best = (1e9, None)
    log_A = np.log(A_arr)
    log_N = np.log(np.maximum(N_arr, 0.01))
    n = len(A_arr)
    for ki in range(1, n-1):
        # Fit two segments
        lA1, lN1 = log_A[:ki+1], log_N[:ki+1]
        lA2, lN2 = log_A[ki:], log_N[ki:]
        if len(lA1) < 2 or len(lA2) < 2:
            continue
        p1 = np.polyfit(lA1, lN1, 1)[0]
        p2 = np.polyfit(lA2, lN2, 1)[0]
        res1 = lN1 - np.polyval(np.polyfit(lA1, lN1, 1), lA1)
        res2 = lN2 - np.polyval(np.polyfit(lA2, lN2, 1), lA2)
        rss = np.sum(res1**2) + np.sum(res2**2)
        if rss < best[0]:
            best = (rss, (A_arr[ki], N_arr[ki], p1, p2))
    if best[1] is None:
        return None
    return best[1]

def evaluate(label, EN_pred, rank_idx, envelope, A_vals, N_meas_vals):
    """Evaluate T1/T2/T3 against frozen thresholds, print results."""
    print(f"\n===== variant {label} =====")

    # Show ranked spectrum for A=10 (predicted count breakdown)
    A10_idx = A_vals.index(10.0)
    # Compute P_genesis for A=10 at each rank
    # (already have EN[A10_idx] but show top ranks)
    print(f"E[N] at A=10: {EN_pred[A10_idx]:.2f}  (measured 4.00)")
    print(f"E[N] at A=9:  {EN_pred[A_vals.index(9.0)]:.2f}  (measured 2.00)")

    # Table
    print(f"\n{'A':>6} | {'N_meas':>8} | {'N_pred':>8}")
    for A, Nm, Np in zip(A_vals, N_meas_vals, EN_pred):
        print(f"  {A:5.1f} | {Nm:8.2f} | {Np:8.2f}")

    # T1: E[N(10)] in [2.0, 8.0]
    EN10 = EN_pred[A10_idx]
    t1 = 2.0 <= EN10 <= 8.0

    # T2: shape log10-RMS
    log_ratios = [math.log10(max(Np, 0.01) / max(Nm, 0.01))
                  for Np, Nm in zip(EN_pred, N_meas_vals)]
    rms = math.sqrt(sum(r**2 for r in log_ratios) / len(log_ratios))
    t2 = rms <= 0.20

    # T3: second-voxel onset — A at which E[N] first >= 2
    A_onset2 = None
    for A, Np in zip(A_vals, EN_pred):
        if Np >= 2.0:
            A_onset2 = A
            break
    t3 = A_onset2 is not None and 7.0 <= A_onset2 <= 11.0

    # Broken-power fit
    Afit = np.array(A_vals)
    Nfit = np.array(EN_pred)
    fit = broken_fit(Afit, Nfit)
    if fit:
        knee_A, knee_N, p_lo, p_hi = fit
        print(f"predicted-curve elbow: knee_A={knee_A:.1f} knee_N={knee_N:.1f} (p_lo={p_lo:.2f}, p_hi={p_hi:.2f})")
    print(f"T3 second-voxel onset: A={A_onset2}  (target [7.0, 11.0])")
    print(f"T1 E[N(10)] in [2.0,8.0]: {'PASS' if t1 else 'FAIL'} ({EN10:.2f})")
    print(f"T2 shape RMS={rms:.3f} (<= 0.2): {'PASS' if t2 else 'FAIL'}")
    print(f"T3 onset in [7.0,11.0]: {'PASS' if t3 else 'FAIL'} (A_onset2={A_onset2})")

    return t1, t2, t3

# ── main ───────────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    results = {}

    for proj, label in [(False, "a:no-projection"), (True, "b:fft-projection")]:
        print(f"\nRunning variant {label}...")
        traj, voxels, vox_r2 = run_kinetics(project=proj)
        EN, rank_idx, envelope = compute_P_genesis(traj, A_GRID)
        t1, t2, t3 = evaluate(label, EN, rank_idx, envelope, A_GRID, N_MEAS)
        results[label] = (t1, t2, t3)

    print("\n" + "=" * 48)
    print("VERDICT")
    print("=" * 48)

    # DWELL-SUPPORTED: T1 AND T2 in at least one variant (T3 optional but reported)
    # DWELL-PARTIAL: T1 OR T2 in at least one variant
    # DWELL-FAIL: neither
    supported = any(t1 and t2 for t1,t2,t3 in results.values())
    partial   = not supported and any(t1 or t2 for t1,t2,t3 in results.values())

    if supported:
        verdict = "DWELL-SUPPORTED"
    elif partial:
        verdict = "DWELL-PARTIAL"
    else:
        verdict = "DWELL-FAIL"

    print(f"\n{verdict}")
    for label, (t1,t2,t3) in results.items():
        print(f"  variant {label}: T1={'PASS' if t1 else 'FAIL'} T2={'PASS' if t2 else 'FAIL'} T3={'PASS' if t3 else 'FAIL'}")
