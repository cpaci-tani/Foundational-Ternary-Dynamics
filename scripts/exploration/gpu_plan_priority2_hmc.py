"""
GPU Priority 2: non-perturbative HMC on BCC lattice (per
C:\\Users\\cpaci\\Downloads\\priority2_hmc_setup.md).

Field: eta(x) = phi(x) - x_+   (shifted; avoids unbounded-below phi^3 danger)
Action:
    S[eta] = a^3 sum_x [ 0.5 eta (-Delta_BCC eta)  +  0.5 m^2 eta^2  +  eta^3 / 3 ]
BCC Laplacian in position space:
    (-Delta_BCC eta)(x) = (1/a^2) [8 eta(x) - sum_{8 NN} eta(x + (+/-a/2)^3)]
Observable:
    <eta> over lattice sites, averaged over thermal MC configs.
One-loop prediction:
    <eta>_1-loop = -T_latt_BCC / m^2 = -0.02292246 / 134.0122 = -1.7105e-4

Success if |<eta>_MC + 1.7105e-4| < 3 * sigma_MC with sigma_MC ~ 1e-5.

GPU via cupy. FP64. HMC with leapfrog, eps=0.02, 50 steps per trajectory.

Parameter defaults match the spec's "recommended production run" for N=128.
"""
from __future__ import annotations

import argparse
import math
import time

import cupy as cp


# ---- constants (from spec) -------------------------------------------------
A_LAT       = 2.0 / 3.0
M_SQ        = 134.012207541816      # high precision; residual sensitive
X_PLUS_TREE = 137.036171458
ALPHA_INV   = 137.035999177

# T_latt result from Priority 1 at N=4096 (continuum-extrapolated target).
T_LATT_BCC  = 0.022922460         # ~ best estimate
ONE_LOOP    = -T_LATT_BCC / M_SQ  # -1.7105e-4


def neg_lap_bcc(eta, a):
    """(-Delta_BCC eta)(x) = (1/a^2)[8 eta(x) - sum_{8 NN} eta(x + (sx,sy,sz)·a/2)].

    On a Z^3 lattice with the BCC 8-diagonal hopping structure indexed by
    (sx,sy,sz) in {-1,+1}^3 (offsets of one unit in index space, which
    corresponds to hopping h=a/2 in physical space, per spec §1)."""
    s = cp.zeros_like(eta)
    for sx in (+1, -1):
        for sy in (+1, -1):
            for sz in (+1, -1):
                s = s + cp.roll(cp.roll(cp.roll(eta, -sx, axis=0),
                                        -sy, axis=1),
                                -sz, axis=2)
    return (8.0 * eta - s) / (a * a)


def action(eta, a, m_sq):
    """S[eta] per the spec."""
    lap = neg_lap_bcc(eta, a)
    s = 0.5 * eta * lap + 0.5 * m_sq * eta * eta + (eta * eta * eta) / 3.0
    return (a ** 3) * cp.sum(s)


def force(eta, a, m_sq):
    """dS/d eta (per spec): a^3 * [ (-Delta_BCC eta) + m^2 eta + eta^2 ]."""
    return (a ** 3) * (neg_lap_bcc(eta, a) + m_sq * eta + eta * eta)


def hmc_trajectory(eta, a, m_sq, eps, n_steps):
    """One HMC trajectory. Returns (eta_new, dH, accepted bool)."""
    p = cp.random.standard_normal(eta.shape, dtype=cp.float64)
    H_i = 0.5 * cp.sum(p * p) + action(eta, a, m_sq)

    # leapfrog: half kick, N drifts with full kicks in between, half kick
    eta_new = eta.copy()
    p_new   = p.copy()

    f = force(eta_new, a, m_sq)
    p_new = p_new - 0.5 * eps * f
    for i in range(n_steps):
        eta_new = eta_new + eps * p_new
        f = force(eta_new, a, m_sq)
        if i < n_steps - 1:
            p_new = p_new - eps * f
    p_new = p_new - 0.5 * eps * f

    H_f = 0.5 * cp.sum(p_new * p_new) + action(eta_new, a, m_sq)
    dH  = float(H_f - H_i)

    u = float(cp.random.random())
    accept = (u < math.exp(-min(dH, 700.0)))   # clamp to avoid overflow
    return (eta_new if accept else eta), dH, accept


def main(args):
    cp.cuda.Device(0).use()
    cp.random.seed(args.seed)

    print("=" * 74)
    print(" GPU Priority 2: HMC on BCC lattice — non-perturbative <eta>")
    print("=" * 74)
    print(f"   N          = {args.N}")
    print(f"   a_BCC      = {A_LAT}")
    print(f"   m^2        = {M_SQ}")
    print(f"   eps        = {args.eps}")
    print(f"   n_steps/t  = {args.n_steps}")
    print(f"   thermal    = {args.thermal} trajectories")
    print(f"   measure    = {args.measure} trajectories")
    print(f"   seed       = {args.seed}")
    print(f"   one-loop   = <eta>_1-loop = -T_latt_BCC/m^2 = {ONE_LOOP:.6e}")
    print()

    # Allocate
    eta = cp.zeros((args.N, args.N, args.N), dtype=cp.float64)

    # Warm-up + thermalization
    print(" Thermalization phase:")
    t0 = time.perf_counter()
    acc_count = 0
    dH_sum = 0.0
    for t in range(args.thermal):
        eta, dH, acc = hmc_trajectory(eta, A_LAT, M_SQ, args.eps, args.n_steps)
        acc_count += int(acc)
        dH_sum += abs(dH)
        if (t + 1) % max(1, args.thermal // 10) == 0:
            ar = 100.0 * acc_count / (t + 1)
            mean_eta = float(cp.mean(eta))
            max_eta  = float(cp.max(cp.abs(eta)))
            print(f"   t={t+1:>5}/{args.thermal}  acc={ar:5.1f}%  "
                  f"<eta>={mean_eta:+.4e}  max|eta|={max_eta:.3f}  "
                  f"<|dH|>={dH_sum/(t+1):.3e}")
    dt_therm = time.perf_counter() - t0
    print(f" Thermalization done in {dt_therm:.1f} s  "
          f"({1000*dt_therm/args.thermal:.1f} ms/traj)")
    print()

    # Production measurement
    print(" Measurement phase:")
    eta_samples = []
    eta_sq_samples = []
    eta_max_samples = []
    dH_samples = []
    t0 = time.perf_counter()
    acc_count = 0
    for t in range(args.measure):
        eta, dH, acc = hmc_trajectory(eta, A_LAT, M_SQ, args.eps, args.n_steps)
        acc_count += int(acc)
        eta_samples.append(float(cp.mean(eta)))
        eta_sq_samples.append(float(cp.mean(eta * eta)))
        eta_max_samples.append(float(cp.max(cp.abs(eta))))
        dH_samples.append(dH)
        if (t + 1) % max(1, args.measure // 10) == 0:
            ar = 100.0 * acc_count / (t + 1)
            running_mean = sum(eta_samples) / (t + 1)
            print(f"   t={t+1:>6}/{args.measure}  acc={ar:5.1f}%  "
                  f"<eta>_run={running_mean:+.5e}  "
                  f"max|eta|={eta_max_samples[-1]:.3f}")
    dt_meas = time.perf_counter() - t0
    print(f" Measurement done in {dt_meas:.1f} s")
    print()

    # Statistics
    import numpy as np
    samples = np.array(eta_samples)
    n = len(samples)
    mean = samples.mean()
    std  = samples.std(ddof=1)
    sem  = std / math.sqrt(n)

    # Integrated autocorrelation time (simple windowed estimator)
    c0 = float(((samples - mean) ** 2).mean())
    tau = 0.5
    for lag in range(1, min(200, n - 1)):
        cl = float(((samples[:-lag] - mean) * (samples[lag:] - mean)).mean())
        rho = cl / c0 if c0 > 0 else 0.0
        if rho < 0.05:
            break
        tau += rho
    sem_corrected = sem * math.sqrt(max(2 * tau, 1.0))

    print("=" * 74)
    print(" Results")
    print("=" * 74)
    print(f"   <eta>_MC            = {mean:+.6e}")
    print(f"   std (per sample)    = {std:.3e}")
    print(f"   sem (iid assumption)= {sem:.3e}")
    print(f"   tau_int (~rho>0.05) = {tau:.2f}")
    print(f"   sem (tau-corrected) = {sem_corrected:.3e}")
    print()
    print(f"   one-loop prediction = {ONE_LOOP:+.6e}")
    print(f"   MC - one-loop       = {mean - ONE_LOOP:+.3e}")
    deviation_sigma = abs(mean - ONE_LOOP) / sem_corrected if sem_corrected > 0 else float('inf')
    print(f"   |MC - one-loop| / sigma = {deviation_sigma:.2f}")
    print()

    # Convert to ppb on x+
    ppb = mean / X_PLUS_TREE * 1e9
    ppb_sem = sem_corrected / X_PLUS_TREE * 1e9
    print(f"   <eta>_MC mapped to ppb on x_+:  {ppb:+.2f}  +/- {ppb_sem:.2f}")
    print(f"   one-loop mapped to ppb:         {ONE_LOOP/X_PLUS_TREE*1e9:+.2f}")
    print()
    # Implied residual from 1/alpha
    x_plus_corrected = X_PLUS_TREE + mean
    residual_ppb = (x_plus_corrected - ALPHA_INV) / ALPHA_INV * 1e9
    one_loop_residual_ppb = (X_PLUS_TREE + ONE_LOOP - ALPHA_INV) / ALPHA_INV * 1e9
    print(f"   Residual from 1/alpha_CODATA:")
    print(f"      MC:       {residual_ppb:+.2f} ppb   "
          f"(one-loop predicts {one_loop_residual_ppb:+.2f} ppb)")
    print()
    print(f"   acceptance rate  = {100.0 * acc_count / args.measure:.1f}%")
    print(f"   <|dH|>           = {np.mean(np.abs(dH_samples)):.3e}")
    print(f"   max|eta| ever    = {max(eta_max_samples):.3f}")
    print()

    # Verdict
    if deviation_sigma < 3.0 and ppb_sem < 50.0:
        print("   VERDICT: MC <eta> agrees with one-loop prediction within 3 sigma.")
        print("            Non-perturbative confirms perturbative.")
    elif deviation_sigma < 3.0:
        print("   VERDICT: MC agrees with one-loop within 3 sigma, but precision")
        print("            is > 50 ppb. Longer run recommended for full confirmation.")
    else:
        print(f"   VERDICT: MC disagrees with one-loop by {deviation_sigma:.1f} sigma.")
        print("            Significant non-perturbative effect or systematic.")

    # Also dump raw samples to CSV for offline analysis
    import csv
    from pathlib import Path
    out = Path(__file__).parent / f"hmc_N{args.N}_eps{args.eps}_samples.csv"
    with out.open("w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["traj", "mean_eta", "mean_eta_sq", "max_abs_eta", "dH"])
        for i in range(n):
            w.writerow([i, eta_samples[i], eta_sq_samples[i],
                        eta_max_samples[i], dH_samples[i]])
    print(f"   raw samples -> {out}")


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--N",        type=int,   default=128)
    p.add_argument("--eps",      type=float, default=0.02)
    p.add_argument("--n_steps",  type=int,   default=50)
    p.add_argument("--thermal",  type=int,   default=1000)
    p.add_argument("--measure",  type=int,   default=10000)
    p.add_argument("--seed",     type=int,   default=42)
    main(p.parse_args())
