"""
explore_threshold_crossing_born.py -- pre-registered T1c threshold-crossing test.

Hash-locked under git tag: preregister-threshold-crossing-born-v1
Pre-registration manifest:
  docs/theory/06_consciousness/PREREG_THRESHOLD_CROSSING_BORN_v1.md

Tests the corpus assertion in SPEC_SIX_ALGORITHMS.md:65 and
AUDIT_EPISTEMIC_AUDIT.md:393 that "threshold crossing statistics produce
the Born rule" -- specifically, that manifestation event frequency
scales as |J|^2 (Born) in an FTD substrate.

Three competing fits are evaluated:
  H_power: freq = A * |J|^n  (Born: n=2; classical linear: n=1)
  H_Rice:  freq = B * exp(-k * (K_B - mu)^2 / sigma^2)

Outcome -> tag mapping is FROZEN in manifest section 4.3.
The construction, sweep grid, parameters, and decision rule are
all locked. This script is the mechanical realization.

Carrier: 3D cubic lattice L=24, periodic BCs, 6-neighbour face
Laplacian, mild damping gamma=0.001.
Initial: smooth Gaussian-envelope sinusoidal flux + per-trial noise.
Manifestation: deterministic ReLU-threshold rule s = sign(J) if
|J| > K_B and s = 0; evaporate if |J| < K_B_evap.
"""

from __future__ import annotations

import csv
from dataclasses import dataclass
from pathlib import Path
from typing import Optional, Tuple

import numpy as np

REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "scripts" / "exploration" / "results"
RESULTS_DIR.mkdir(parents=True, exist_ok=True)


# ---- FROZEN parameters (manifest section 2.4) ----

@dataclass(frozen=True)
class Params:
    L: int = 24
    c2: float = 1.0 / 3.0
    gamma: float = 0.001
    K_B: float = 0.5
    K_B_evap: float = 0.25
    A: float = 2.0
    sigma_env: float = 4.0       # L/6 = 4.0 exactly
    n_x: int = 2
    n_y: int = 3
    n_z: int = 1
    eps_scale: float = 0.10      # 0.05 * A
    n_trials: int = 100
    ticks_per_trial: int = 80
    seed_master: int = 42


P = Params()


# ---- Substrate primitives ----

def laplacian6(F: np.ndarray) -> np.ndarray:
    """6-neighbour face Laplacian with periodic BCs."""
    return (np.roll(F, 1, 0) + np.roll(F, -1, 0)
          + np.roll(F, 1, 1) + np.roll(F, -1, 1)
          + np.roll(F, 1, 2) + np.roll(F, -1, 2) - 6 * F)


def gaussian_envelope(L: int, sigma_env: float) -> np.ndarray:
    """Gaussian envelope centred at lattice mid-point, periodic-aware distance."""
    cx = cy = cz = L // 2
    coords = np.arange(L)
    dx = np.minimum(np.abs(coords - cx), L - np.abs(coords - cx))
    dy = np.minimum(np.abs(coords - cy), L - np.abs(coords - cy))
    dz = np.minimum(np.abs(coords - cz), L - np.abs(coords - cz))
    Dx, Dy, Dz = np.meshgrid(dx, dy, dz, indexing="ij")
    r2 = Dx**2 + Dy**2 + Dz**2
    g = np.exp(-r2 / (2 * sigma_env**2))
    return g / g.max()


def sinusoid_axis(L: int, axis: int, n_wave: int) -> np.ndarray:
    """sin(2 pi n_wave * v_axis / L), broadcast to L^3."""
    coords = np.arange(L)
    s1d = np.sin(2 * np.pi * n_wave * coords / L)
    if axis == 0:
        return s1d[:, None, None] * np.ones((L, L, L))
    if axis == 1:
        return s1d[None, :, None] * np.ones((L, L, L))
    return s1d[None, None, :] * np.ones((L, L, L))


def initial_flux(p: Params, trial_idx: int) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """Smooth Gaussian-envelope sinusoidal flux + per-trial random perturbation."""
    L = p.L
    g = gaussian_envelope(L, p.sigma_env)
    Jx = p.A * g * sinusoid_axis(L, 0, p.n_x)
    Jy = p.A * g * sinusoid_axis(L, 1, p.n_y)
    Jz = p.A * g * sinusoid_axis(L, 2, p.n_z)
    rng = np.random.default_rng(seed=p.seed_master + trial_idx)
    Jx += rng.normal(0, p.eps_scale, (L, L, L))
    Jy += rng.normal(0, p.eps_scale, (L, L, L))
    Jz += rng.normal(0, p.eps_scale, (L, L, L))
    return Jx, Jy, Jz


# ---- Simulation core ----

@dataclass
class TrialResult:
    count: np.ndarray   # int (L,L,L): count of 0 -> ±1 transitions over trial
    sum_J2: np.ndarray  # float (L,L,L): running sum of |J(t)|^2 over ticks
    sum_J2_sq: np.ndarray  # for variance: sum of (|J(t)|^2)^2
    n_samples: int


def run_trial(p: Params, trial_idx: int) -> TrialResult:
    L = p.L
    Jx, Jy, Jz = initial_flux(p, trial_idx)
    Jx_prev = Jx.copy()
    Jy_prev = Jy.copy()
    Jz_prev = Jz.copy()
    s = np.zeros((L, L, L), dtype=np.int8)
    count = np.zeros((L, L, L), dtype=np.int64)
    sum_J2 = np.zeros((L, L, L), dtype=np.float64)
    sum_J2_sq = np.zeros((L, L, L), dtype=np.float64)

    damp = 1.0 - p.gamma

    for t in range(p.ticks_per_trial):
        # Wave equation step
        Jx_new = damp * (2 * Jx - Jx_prev + p.c2 * laplacian6(Jx))
        Jy_new = damp * (2 * Jy - Jy_prev + p.c2 * laplacian6(Jy))
        Jz_new = damp * (2 * Jz - Jz_prev + p.c2 * laplacian6(Jz))
        Jx_prev, Jy_prev, Jz_prev = Jx, Jy, Jz
        Jx, Jy, Jz = Jx_new, Jy_new, Jz_new

        # Manifestation: 0 -> +-1 where |J| > K_B
        mag2 = Jx * Jx + Jy * Jy + Jz * Jz
        sum_J2 += mag2
        sum_J2_sq += mag2 * mag2
        mag = np.sqrt(mag2)
        can_manifest = (s == 0) & (mag > p.K_B)
        sign_choice = np.sign(Jx + Jy + Jz)
        sign_choice[sign_choice == 0] = 1
        # Count transitions BEFORE applying
        count += can_manifest.astype(np.int64)
        s = np.where(can_manifest, sign_choice.astype(np.int8), s)

        # Evaporation
        can_evap = (np.abs(s) > 0) & (mag < p.K_B_evap)
        s = np.where(can_evap, np.int8(0), s)

    return TrialResult(count=count, sum_J2=sum_J2, sum_J2_sq=sum_J2_sq, n_samples=p.ticks_per_trial)


def run_ensemble(p: Params) -> Tuple[np.ndarray, np.ndarray, np.ndarray, int]:
    """Aggregate count, mu_sq, sigma_sq across trials."""
    L = p.L
    total_count = np.zeros((L, L, L), dtype=np.int64)
    total_sum_J2 = np.zeros((L, L, L), dtype=np.float64)
    total_sum_J2_sq = np.zeros((L, L, L), dtype=np.float64)
    total_samples = 0
    for trial in range(p.n_trials):
        res = run_trial(p, trial)
        total_count += res.count
        total_sum_J2 += res.sum_J2
        total_sum_J2_sq += res.sum_J2_sq
        total_samples += res.n_samples
        if (trial + 1) % 25 == 0:
            print(f"    trial {trial + 1}/{p.n_trials} complete; "
                  f"cumulative events {total_count.sum()}")
    mu_sq = total_sum_J2 / total_samples
    var_J2 = total_sum_J2_sq / total_samples - mu_sq * mu_sq
    sigma_sq = np.maximum(var_J2, 0.0)  # numerical floor
    return total_count, mu_sq, sigma_sq, total_samples


# ---- Analysis ----

def select_voxels(p: Params, mu_sq: np.ndarray) -> np.ndarray:
    """Mask: exclude central 3x3x3 cube, periodic rim of 2 cells, and out-of-range mu_sq."""
    L = p.L
    mask = np.ones((L, L, L), dtype=bool)
    cx = cy = cz = L // 2
    # Central 3x3x3 cube
    for dx in range(-1, 2):
        for dy in range(-1, 2):
            for dz in range(-1, 2):
                mask[(cx + dx) % L, (cy + dy) % L, (cz + dz) % L] = False
    # Periodic rim of 2 cells (top + bottom on each axis)
    mask[:2, :, :] = False
    mask[-2:, :, :] = False
    mask[:, :2, :] = False
    mask[:, -2:, :] = False
    mask[:, :, :2] = False
    mask[:, :, -2:] = False
    # mu_sq in [0.05, 4*A^2]
    mask &= (mu_sq > 0.05) & (mu_sq < 4.0 * p.A * p.A)
    return mask


def bin_and_average(mu_sq_flat, sigma_sq_flat, freq_flat, n_bins=14):
    """Equal-count bins by mu_sq."""
    edges = np.percentile(mu_sq_flat, np.linspace(0, 100, n_bins + 1))
    bins = []
    for i in range(n_bins):
        lo, hi = edges[i], edges[i + 1]
        if i == n_bins - 1:
            inb = (mu_sq_flat >= lo) & (mu_sq_flat <= hi)
        else:
            inb = (mu_sq_flat >= lo) & (mu_sq_flat < hi)
        n_sites = int(inb.sum())
        if n_sites < 5:
            continue
        bins.append({
            "lo": lo, "hi": hi,
            "mean_mu_sq": float(mu_sq_flat[inb].mean()),
            "mean_sigma_sq": float(sigma_sq_flat[inb].mean()),
            "mean_freq": float(freq_flat[inb].mean()),
            "n_sites": n_sites,
        })
    return bins


def fit_power(bins, seed_offset: int = 1) -> Optional[dict]:
    """H_power: freq = A * mu_sq^(n/2). Returns n, log_A, R^2, 95% CI on n."""
    mu_sq_arr = np.array([b["mean_mu_sq"] for b in bins])
    freq_arr = np.array([b["mean_freq"] for b in bins])
    valid = (freq_arr > 0) & (mu_sq_arr > 0)
    if valid.sum() < 3:
        return None
    x = np.log(mu_sq_arr[valid])
    y = np.log(freq_arr[valid])
    slope, log_A = np.polyfit(x, y, 1)
    # n = 2 * slope (since slope corresponds to n/2 when fitting log freq vs log mu_sq)
    n_est = 2.0 * slope
    y_pred = log_A + slope * x
    ss_res = np.sum((y - y_pred) ** 2)
    ss_tot = np.sum((y - y.mean()) ** 2)
    r2 = 1.0 - ss_res / ss_tot if ss_tot > 0 else 0.0
    # Bootstrap CI on n
    rng = np.random.default_rng(seed=P.seed_master + seed_offset)
    n_boot = 1000
    samples = []
    for _ in range(n_boot):
        idx = rng.integers(0, valid.sum(), size=valid.sum())
        xb = x[idx]
        yb = y[idx]
        if len(set(xb.tolist())) < 2:
            continue
        sl, _ = np.polyfit(xb, yb, 1)
        samples.append(2.0 * sl)
    samples = np.array(samples)
    if len(samples) > 0:
        n_lo, n_hi = np.percentile(samples, [2.5, 97.5])
    else:
        n_lo, n_hi = float("nan"), float("nan")
    return {"n": n_est, "log_A": log_A, "R2": r2, "CI": (n_lo, n_hi)}


def fit_rice(bins, p: Params) -> Optional[dict]:
    """H_Rice: log freq = log B - k * (K_B - mu)^2 / sigma_sq."""
    mu_sq_arr = np.array([b["mean_mu_sq"] for b in bins])
    sigma_sq_arr = np.array([b["mean_sigma_sq"] for b in bins])
    freq_arr = np.array([b["mean_freq"] for b in bins])
    valid = (freq_arr > 0) & (sigma_sq_arr > 1e-6)
    if valid.sum() < 3:
        return None
    mu_arr = np.sqrt(mu_sq_arr[valid])
    rice_x = -((p.K_B - mu_arr) ** 2) / sigma_sq_arr[valid]
    y = np.log(freq_arr[valid])
    slope, intercept = np.polyfit(rice_x, y, 1)
    y_pred = intercept + slope * rice_x
    ss_res = np.sum((y - y_pred) ** 2)
    ss_tot = np.sum((y - y.mean()) ** 2)
    r2 = 1.0 - ss_res / ss_tot if ss_tot > 0 else 0.0
    return {"k": slope, "log_B": intercept, "R2": r2}


def decide(power: Optional[dict], rice: Optional[dict]) -> Tuple[str, str]:
    """Pre-registered decision rule from manifest section 4.3."""
    if power is None:
        return "D", ("H_power could not be fit (insufficient data). No clean scaling. "
                     "[NUMERICAL FACT - no clean scaling] Inconclusive.")
    n = power["n"]
    n_lo, n_hi = power["CI"]
    pR2 = power["R2"]
    rR2 = rice["R2"] if rice is not None else -1.0
    if (1.8 <= n <= 2.2) and pR2 > 0.95 and rR2 < pR2:
        return "A", (
            f"BORN SCALING. n = {n:.3f} in [1.8, 2.2], 95% CI = [{n_lo:.3f}, {n_hi:.3f}], "
            f"H_power R^2 = {pR2:.4f}, H_Rice R^2 = {rR2:.4f}. "
            f"[NUMERICAL FACT - Born scaling] + [OBSERVATION supporting corpus assertion]; "
            f"engine-canonical confirmation still required."
        )
    if (0.8 <= n <= 1.2) and pR2 > 0.95:
        return "B", (
            f"LINEAR SCALING. n = {n:.3f} in [0.8, 1.2], 95% CI = [{n_lo:.3f}, {n_hi:.3f}], "
            f"H_power R^2 = {pR2:.4f}. [NUMERICAL FACT - linear] + [CLOSED NEGATIVE for Born]; "
            f"SPEC_SIX_ALGORITHMS.md:65 + AUDIT_EPISTEMIC_AUDIT.md:393 need retag."
        )
    if rice is not None and rR2 > pR2 + 0.05 and rR2 > 0.90:
        return "C", (
            f"RICE / UPCROSSING SCALING. H_Rice R^2 = {rR2:.4f}, H_power R^2 = {pR2:.4f}. "
            f"[NUMERICAL FACT - Gaussian-process upcrossing] + [CLOSED NEGATIVE for Born]; "
            f"SPEC_SIX_ALGORITHMS.md:65 + AUDIT_EPISTEMIC_AUDIT.md:393 need retag."
        )
    return "D", (
        f"NO CLEAN SCALING. n = {n:.3f} (CI [{n_lo:.3f}, {n_hi:.3f}]), "
        f"H_power R^2 = {pR2:.4f}, H_Rice R^2 = {rR2:.4f}. "
        f"Neither Born/linear/Rice fits the pre-registered thresholds. "
        f"[NUMERICAL FACT - no clean scaling] Inconclusive in this regime."
    )


# ---- Main ----

def main():
    p = Params()
    print("=" * 78)
    print("THRESHOLD-CROSSING -> BORN RULE TEST")
    print("Target: corpus assertions SPEC_SIX_ALGORITHMS.md:65, AUDIT_EPISTEMIC_AUDIT.md:393")
    print("Hash-locked: preregister-threshold-crossing-born-v1")
    print("=" * 78 + "\n")
    print(f"Params: L={p.L}, K_B={p.K_B}, A={p.A}, gamma={p.gamma}, "
          f"trials={p.n_trials}, ticks_per_trial={p.ticks_per_trial}, seed={p.seed_master}")
    print(f"Initial flux: Gaussian envelope sigma_env={p.sigma_env} * sin (2pi (n_x={p.n_x}, "
          f"n_y={p.n_y}, n_z={p.n_z}) * v / L) + N(0, {p.eps_scale})")
    print()
    print("  Running ensemble...")
    count, mu_sq, sigma_sq, total_samples = run_ensemble(p)
    freq = count.astype(np.float64) / total_samples

    print(f"\n  Voxels with at least one manifestation event: "
          f"{int((count > 0).sum())} / {p.L**3}")
    print(f"  Total events: {int(count.sum())}")
    print(f"  mu_sq range: [{mu_sq.min():.4f}, {mu_sq.max():.4f}], "
          f"mean = {mu_sq.mean():.4f}")
    print(f"  freq range: [{freq.min():.6f}, {freq.max():.6f}], mean = {freq.mean():.6f}")

    mask = select_voxels(p, mu_sq)
    print(f"\n  Voxels in analysis mask: {int(mask.sum())} / {p.L**3}")
    if mask.sum() == 0:
        print("  ERROR: no voxels pass the analysis mask. Outcome: D.")
        return

    mu_sq_flat = mu_sq[mask]
    sigma_sq_flat = sigma_sq[mask]
    freq_flat = freq[mask]

    bins = bin_and_average(mu_sq_flat, sigma_sq_flat, freq_flat, n_bins=14)
    print(f"\n  Non-empty bins: {len(bins)}")
    print(f"  {'bin range':>20} | {'mean mu^2':>10} | {'mean sigma^2':>12} | "
          f"{'mean freq':>10} | {'n':>5}")
    print("  " + "-" * 78)
    for b in bins:
        rng_str = f"{b['lo']:.3f}-{b['hi']:.3f}"
        print(f"  {rng_str:>20} | {b['mean_mu_sq']:>10.4f} | "
              f"{b['mean_sigma_sq']:>12.4f} | {b['mean_freq']:>10.6f} | {b['n_sites']:>5d}")

    power = fit_power(bins, seed_offset=1)
    rice = fit_rice(bins, p)
    if power is not None:
        n_lo, n_hi = power["CI"]
        print(f"\n  H_power fit: freq ~ |J|^n, n = {power['n']:.4f}, "
              f"95% CI = [{n_lo:.4f}, {n_hi:.4f}], R^2 = {power['R2']:.4f}")
    if rice is not None:
        print(f"  H_Rice fit:  log freq = log B - k * (K_B - mu)^2 / sigma^2, "
              f"k = {rice['k']:.4f}, R^2 = {rice['R2']:.4f}")

    outcome, msg = decide(power, rice)
    print(f"\n  OUTCOME: {outcome}")
    print(f"  {msg}")

    # ---- Write CSV ----
    csv_path = RESULTS_DIR / "threshold_crossing_born_2026-05-23.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["x", "y", "z", "count", "mu_sq", "sigma_sq", "freq", "in_mask"])
        L = p.L
        for ix in range(L):
            for iy in range(L):
                for iz in range(L):
                    w.writerow([ix, iy, iz, int(count[ix, iy, iz]),
                                f"{mu_sq[ix, iy, iz]:.6f}",
                                f"{sigma_sq[ix, iy, iz]:.6f}",
                                f"{freq[ix, iy, iz]:.6f}",
                                int(mask[ix, iy, iz])])
    print(f"\nCSV written: {csv_path}")

    # ---- Write MD ----
    md_path = RESULTS_DIR / "threshold_crossing_born_2026-05-23.md"
    with md_path.open("w", encoding="utf-8") as f:
        f.write("# Threshold-Crossing -> Born Rule Test -- Results\n\n")
        f.write(f"**Date:** 2026-05-23\n")
        f.write(f"**Pre-registration:** docs/theory/06_consciousness/PREREG_THRESHOLD_CROSSING_BORN_v1.md\n")
        f.write(f"**Git tag:** preregister-threshold-crossing-born-v1\n\n")
        f.write("---\n\n## Outcome\n\n")
        f.write(f"**Outcome {outcome}.** {msg}\n\n")
        f.write("---\n\n## Summary statistics\n\n")
        f.write(f"- L = {p.L}, K_B = {p.K_B}, A = {p.A}, trials = {p.n_trials}, "
                f"ticks_per_trial = {p.ticks_per_trial}, total samples = {total_samples}\n")
        f.write(f"- Voxels with at least one manifestation event: "
                f"**{int((count > 0).sum())} / {p.L**3}**\n")
        f.write(f"- Total events: **{int(count.sum())}**\n")
        f.write(f"- mu_sq range: [{mu_sq.min():.4f}, {mu_sq.max():.4f}], "
                f"mean = {mu_sq.mean():.4f}\n")
        f.write(f"- freq range: [{freq.min():.6f}, {freq.max():.6f}], "
                f"mean = {freq.mean():.6f}\n")
        f.write(f"- Voxels in analysis mask: **{int(mask.sum())} / {p.L**3}**\n")
        f.write(f"- Non-empty bins: **{len(bins)}**\n\n")
        f.write("---\n\n## Bin table\n\n")
        f.write("| bin range (mu^2) | mean mu^2 | mean sigma^2 | mean freq | n sites |\n")
        f.write("|---|---|---|---|---|\n")
        for b in bins:
            f.write(f"| {b['lo']:.3f} - {b['hi']:.3f} | {b['mean_mu_sq']:.4f} | "
                    f"{b['mean_sigma_sq']:.4f} | {b['mean_freq']:.6f} | {b['n_sites']} |\n")
        f.write("\n---\n\n## Fits\n\n")
        if power is not None:
            n_lo, n_hi = power["CI"]
            f.write(f"**H_power: `freq ~ |J|^n`**  \n")
            f.write(f"- n = **{power['n']:.4f}**\n")
            f.write(f"- 95% CI: [{n_lo:.4f}, {n_hi:.4f}]\n")
            f.write(f"- R^2 = {power['R2']:.4f}\n")
            f.write(f"- Born predicts n = 2; classical linear predicts n = 1.\n\n")
        if rice is not None:
            f.write(f"**H_Rice: `log freq = log B - k * (K_B - mu)^2 / sigma^2`**  \n")
            f.write(f"- k = {rice['k']:.4f}\n")
            f.write(f"- log B = {rice['log_B']:.4f}\n")
            f.write(f"- R^2 = {rice['R2']:.4f}\n\n")
        f.write("---\n\n## Full data\n\n")
        f.write(f"See `threshold_crossing_born_2026-05-23.csv` ({p.L**3} rows).\n")
    print(f"MD written:  {md_path}")


if __name__ == "__main__":
    main()
