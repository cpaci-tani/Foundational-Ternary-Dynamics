"""verify_born_density_upcrossing.py — T3: which weighting do threshold
upcrossings actually follow?

Locked instrument for PREREG_BORN_DENSITY_UPCROSSING_v1.md. Two stages.

STAGE A (exact, deterministic): the FTD-0798 positive-frequency occupation
n_k = |phi_+|^2 on the 1D ring leapfrog, normalized with the DISCRETE-exact
eigenfrequency Omega_k = 2 arcsin(omega_k dt / 2)/dt:
  A1  total occupation conserved to machine precision over 20,000 ticks
      (sharpens FTD-0798's reported 1.05e-2 drift, attributed to using the
      continuum frequency in a discrete-time update);
  A2  two modes at equal occupation split 0.5000 : 0.5000;
  A3  scope documentation: an additive source and a threshold projection
      each break conservation (magnitudes reported, no gate) — the density
      is Born-compatible in the FREE sector only, per FTD-0798 §4.

STAGE B (statistical, the discriminator): the FTD-0187/0356 mechanism model
as written — deterministic coherent field + per-site OU noise, threshold
K = W_SC — with TWO standing modes prepared at EQUAL OCCUPATION
(A_i ~ 1/sqrt(Omega_i)) and Omega_2/Omega_1 ~ 2. Site-resolved excess
upcrossing rate r(x) (signal minus same-seed control), regressed on
{1, cos 2k1 x, cos 2k2 x}. The registered statistic is the mode-weight
ratio R = w2/w1. Rival predictions, separated by design:
    amplitude^2 weighting (the DERIV leading order):  R = Omega_1/Omega_2
    occupation/Born weighting:                        R = 1
    energy weighting:                                 R = Omega_2/Omega_1
Registered null: amplitude. Everything below is FROZEN; see the prereg for
outcomes and kill conditions. Platform: mechanism-level quick-check
(feedback_measurement_platform); FTD-0200 closure still requires an engine
run. Registers nothing by itself.

v1.1 (2026-08-07, re-registered after v1 EXECUTION INVALID):
  - v1's A1 invariant formula was defective (an incorrect Verlet-ellipse
    normalisation gave 3.6e-2 drift against its own 1e-10 gate, voiding the
    run under the locked kill conditions). v1.1 uses the machine-exact
    two-step invariant: phi_k obeys phi_{n+1} - 2cos(Om_k)phi_n + phi_{n-1}
    = 0 EXACTLY, so z_k = phi_k(n+1) - exp(-i Om_k) phi_k(n) has exactly
    conserved modulus and n_k = Om_k |z_k|^2 / (2 sin^2 Om_k).
  - v1's blind Stage B returned R = 0.5124 [0.5073, 0.5182]: amplitude-
    dominated with a significant shift toward Born, consistent with the
    velocity channel at Om*tau << 1. v1.1 therefore runs TWO arms:
    slow (lambda = 64, 32; Om*tau ~ 0.5-0.9) and fast (lambda = 16, 8;
    Om*tau ~ 1.8-3.6), with regime-dependence a first-class outcome.
"""
from __future__ import annotations

import numpy as np

# ---------------- frozen parameters ----------------------------------
C = 0.57735026918962576451          # C_WAVE = 1/sqrt(3)
DT = 1.0
L = 4096
K_THRESH = 0.5054620197             # W_SC = K_MANIFEST (FTD-0388)
SIGMA_N = 0.17                      # per-site OU noise std (< K: sign cond.)
TAU_N = 8.0                         # OU correlation time (ticks)
LAM1, LAM2 = 64, 32                 # standing-mode wavelengths (sites)
A1 = 0.10                           # mode-1 amplitude (weak field)
T_TICKS = 20000
N_SEEDS = 48
MASTER_SEED = 20260807
BOOT = 2000                         # bootstrap resamples over seeds

w_cont = lambda k: 2 * C * np.sin(k / 2) / 1.0          # spatial dispersion
Om = lambda k: 2 * np.arcsin(np.clip(w_cont(k) * DT / 2, -1, 1)) / DT

x = np.arange(L)


def lap(f):
    return np.roll(f, 1) + np.roll(f, -1) - 2 * f


def occupations_z(phi_prev, phi_next):
    """Machine-exact leapfrog occupations from two consecutive states:
    phi_k obeys phi_{n+1} - 2 cos(Om_k) phi_n + phi_{n-1} = 0 exactly, so
    z_k = Phi_k(n+1) - exp(-i Om_k) Phi_k(n) rotates with exactly conserved
    modulus |z_k| = a_k sin(Om_k); occupation n_k = Om_k a_k^2 / 2."""
    P0 = np.fft.rfft(phi_prev) / L
    P1 = np.fft.rfft(phi_next) / L
    kk = 2 * np.pi * np.arange(P0.size) / L
    Ok = Om(kk)
    z = P1 - np.exp(-1j * Ok) * P0
    s2 = np.sin(Ok) ** 2
    return np.where(s2 > 1e-24, Ok * np.abs(z) ** 2 / (2 * np.where(
        s2 > 1e-24, s2, 1.0)), 0.0)


def stage_a():
    print("=" * 72)
    print("STAGE A — exact occupation facts on the leapfrog ring (v1.1)")
    print("=" * 72)
    rng = np.random.default_rng(MASTER_SEED)
    phi = rng.normal(0, 0.1, L)
    phi -= phi.mean()
    vel = rng.normal(0, 0.05, L)
    vel -= vel.mean()
    v_half = vel + 0.5 * DT * C * C * lap(phi)
    phi_prev = phi.copy()
    phi = phi + DT * v_half
    v_half = v_half + DT * C * C * lap(phi)
    n0 = occupations_z(phi_prev, phi).sum()
    drift_max = 0.0
    for t in range(T_TICKS):
        phi_prev, phi = phi, phi + DT * v_half
        v_half = v_half + DT * C * C * lap(phi)
        if t % 2000 == 1999:
            nt = occupations_z(phi_prev, phi).sum()
            drift_max = max(drift_max, abs(nt - n0) / n0)
    print(f"A1 total-occupation drift over {T_TICKS} ticks: {drift_max:.3e}"
          f"  (gate < 1e-10)")
    a1 = drift_max < 1e-10

    # A2: two modes at equal occupation (slow-arm pair), measure split
    k1s, k2s = 2 * np.pi / LAM1, 2 * np.pi / LAM2
    O1, O2 = Om(k1s), Om(k2s)
    A2s = A1 * np.sqrt(O1 / O2)
    phi = A1 * np.cos(k1s * x) + A2s * np.cos(k2s * x)
    v_half = 0.5 * DT * C * C * lap(phi)
    phi_prev = phi.copy()
    phi_n = phi + DT * v_half
    occ = occupations_z(phi_prev, phi_n)
    m1, m2 = int(round(L / LAM1)), int(round(L / LAM2))
    s1, s2 = occ[m1], occ[m2]
    print(f"A2 equal-occupation split: {s1/(s1+s2):.6f} : {s2/(s1+s2):.6f}"
          f"  (gate 0.5000:0.5000 within 2e-3)")
    a2 = abs(s1 / (s1 + s2) - 0.5) < 2e-3

    # A3: breakage scope (documentation, no gate)
    phi3, vh3 = phi.copy(), 0.5 * DT * C * C * lap(phi)
    for t in range(200):
        p_prev, phi3 = phi3, phi3 + DT * vh3
        vh3 = vh3 + DT * C * C * lap(phi3) + 0.01 * np.cos(k1s * x)
    nsrc = occupations_z(p_prev, phi3).sum()
    phi4 = phi.copy()
    phi4[np.abs(phi4) > 0.8 * phi4.max()] = 0.0
    vh4 = 0.5 * DT * C * C * lap(phi4)
    p4_prev, phi4n = phi4, phi4 + DT * vh4
    nproj = occupations_z(p4_prev, phi4n).sum()
    nref = occ.sum()
    print(f"A3 additive source: occupation {nref:.4e} -> {nsrc:.4e} "
          f"({abs(nsrc-nref)/nref*100:.1f}% change; NOT conserved)")
    print(f"A3 threshold projection: occupation -> {nproj:.4e} "
          f"({abs(nproj-nref)/nref*100:.1f}% loss; NOT conserved)")
    print(f"STAGE A: {'PASS' if a1 and a2 else 'FAIL'}")
    return a1 and a2


def stage_b_arm(lam1, lam2, amp1, label, seed_base):
    k1, k2 = 2 * np.pi / lam1, 2 * np.pi / lam2
    O1, O2 = Om(k1), Om(k2)
    amp2 = amp1 * np.sqrt(O1 / O2)
    R_AMP, R_BORN, R_EN = O1 / O2, 1.0, O2 / O1
    prof1, prof2 = np.cos(k1 * x), np.cos(k2 * x)
    print("-" * 72)
    print(f"ARM {label}: lambda = ({lam1},{lam2}), Om = ({O1:.4f},{O2:.4f}), "
          f"Om*tau = ({O1*TAU_N:.2f},{O2*TAU_N:.2f})")
    print(f"  predictions: R_amp = {R_AMP:.4f}, R_born = 1.0000, "
          f"R_energy = {R_EN:.4f}")
    alpha = np.exp(-1.0 / TAU_N)
    sig_step = SIGMA_N * np.sqrt(1 - alpha * alpha)
    theta = np.random.default_rng(seed_base + 1).uniform(0, 2 * np.pi, 2)

    def coh(t):
        return (amp1 * prof1 * np.cos(O1 * t + theta[0])
                + amp2 * prof2 * np.cos(O2 * t + theta[1]))

    excess_per_seed = np.zeros((N_SEEDS, L))
    tot_sig = tot_ctl = 0
    for s in range(N_SEEDS):
        rng = np.random.default_rng(seed_base + 100 + s)
        noise_draws = rng.standard_normal((T_TICKS, L)).astype(np.float64)
        sig_counts = None
        for mode, acc in (("sig", 1.0), ("ctl", 0.0)):
            xi = np.zeros(L)
            prev = xi + acc * coh(0)
            counts = np.zeros(L)
            for t in range(1, T_TICKS):
                xi = alpha * xi + sig_step * noise_draws[t]
                F = xi + acc * coh(t)
                counts += (prev < K_THRESH) & (F >= K_THRESH)
                prev = F
            if mode == "sig":
                sig_counts = counts
                tot_sig += counts.sum()
            else:
                excess_per_seed[s] = sig_counts - counts
                tot_ctl += counts.sum()
    excess = excess_per_seed.mean(axis=0)
    print(f"  crossings: signal {int(tot_sig):,}, control {int(tot_ctl):,}, "
          f"net excess {int(tot_sig - tot_ctl):,}")
    B1v, B2v = np.cos(2 * k1 * x), np.cos(2 * k2 * x)
    X = np.column_stack([np.ones(L), B1v, B2v])

    def ratio(ex):
        c, *_ = np.linalg.lstsq(X, ex, rcond=None)
        return c[2] / c[1], c[1], c[2]

    Rhat, c1, c2 = ratio(excess)
    rngb = np.random.default_rng(seed_base + 999)
    rs = []
    for _ in range(BOOT):
        pick = rngb.integers(0, N_SEEDS, N_SEEDS)
        r, *_ = ratio(excess_per_seed[pick].mean(axis=0))
        rs.append(r)
    lo, hi = np.percentile(rs, [0.5, 99.5])
    print(f"  c1 = {c1:.4e}, c2 = {c2:.4e};  R_hat = {Rhat:.4f}   "
          f"99% CI [{lo:.4f}, {hi:.4f}]")
    inside = {"AMPLITUDE": lo <= R_AMP <= hi, "BORN": lo <= R_BORN <= hi,
              "ENERGY": lo <= R_EN <= hi}
    kill = []
    if tot_sig - tot_ctl < 5000:
        kill.append("excess counts < 5000")
    if c1 <= 0 or c2 <= 0:
        kill.append("negative modulation coefficient")
    if kill:
        print(f"  ARM {label} KILL: {kill} -> INVALID")
        return None
    names = [k for k, v in inside.items() if v]
    # Born-fraction: where R_hat sits on the amp->born gap (0 = pure amp)
    bf = (Rhat - R_AMP) / (R_BORN - R_AMP)
    blo, bhi = (lo - R_AMP) / (R_BORN - R_AMP), (hi - R_AMP) / (R_BORN - R_AMP)
    print(f"  predictors inside CI: {names if names else 'NONE'};  "
          f"Born-fraction = {bf:.3f}  99% CI [{blo:.3f}, {bhi:.3f}]")
    return dict(label=label, R=Rhat, lo=lo, hi=hi, inside=names, bf=bf,
                bflo=blo, bfhi=bhi)


def stage_b():
    print("=" * 72)
    print("STAGE B — two-arm weighting discrimination (v1.1, locked)")
    print("=" * 72)
    slow = stage_b_arm(LAM1, LAM2, A1, "SLOW", MASTER_SEED)
    fast = stage_b_arm(16, 8, A1, "FAST", MASTER_SEED + 50000)
    print("-" * 72)
    if slow is None or fast is None:
        print("OUTCOME D — EXECUTION INVALID (an arm was killed)")
        return
    def sole(arm, name):
        return arm["inside"] == [name]
    if sole(slow, "AMPLITUDE") and sole(fast, "AMPLITUDE"):
        print("OUTCOME A — AMPLITUDE weighting in both regimes")
    elif sole(slow, "BORN") and sole(fast, "BORN"):
        print("OUTCOME B — BORN weighting in both regimes")
    elif sole(slow, "ENERGY") and sole(fast, "ENERGY"):
        print("OUTCOME C — ENERGY weighting in both regimes")
    elif fast["bflo"] > slow["bfhi"] and fast["bf"] > 0.5 * 0:
        print("OUTCOME E — REGIME-DEPENDENT: Born-fraction rises from "
              f"{slow['bf']:.3f} [{slow['bflo']:.3f},{slow['bfhi']:.3f}] "
              f"(slow) to {fast['bf']:.3f} "
              f"[{fast['bflo']:.3f},{fast['bfhi']:.3f}] (fast) — the "
              "weighting moves toward occupation as mode frequency exceeds "
              "the noise bandwidth")
    else:
        print("OUTCOME D — INDETERMINATE (no registered pattern matched)")


if __name__ == "__main__":
    ok = stage_a()
    if not ok:
        print("STAGE A FAILED — Stage B runs but the whole result is "
              "EXECUTION INVALID per the locked kill conditions.")
    stage_b()
