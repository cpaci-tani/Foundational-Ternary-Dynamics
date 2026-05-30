"""
explore_born_equilibrium_preservation.py -- pre-registered v2 test.

Hash-locked under git tag: preregister-born-equilibrium-preservation-v1
Pre-registration manifest:
  docs/theory/06_reference frame context/PREREG_BORN_EQUILIBRIUM_PRESERVATION_v1.md

Tests whether the FTD substrate (6-neighbour Python lattice) PRESERVES a
Born-distributed initial ensemble under deterministic evolution.

Construction (FROZEN in manifest):
  - 3D cubic lattice L=24, periodic BCs, 6-neighbour face Laplacian
  - Three target |psi(v)|^2 profiles: Gaussian, uniform-envelope, two-bump
  - Initial conditions: J_{x,y,z}(v) ~ Normal(0, sigma^2(v)) with
    sigma^2(v) proportional to |psi(v)|^2
  - 100 trials per profile, 80 ticks each
  - Manifestation: ReLU threshold K_B=0.5; evaporation at K_B_evap=0.25

Primary measurement: long-run manifestation rate per voxel vs |psi(v)|^2.
Secondary measurement: first-event spatial distribution vs |psi(v)|^2.

Outcome -> tag map FROZEN in manifest section 4.2.
"""

from __future__ import annotations

import csv
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple

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
    alpha: float = 4.0 / 3.0           # variance normalization
    n_trials: int = 100
    ticks_per_trial: int = 80
    burnin_ticks: int = 20              # for long-run rate, skip first 20
    seed_master: int = 42


P = Params()


# ---- Substrate primitives (mirroring v1) ----

def laplacian6(F: np.ndarray) -> np.ndarray:
    return (np.roll(F, 1, 0) + np.roll(F, -1, 0)
          + np.roll(F, 1, 1) + np.roll(F, -1, 1)
          + np.roll(F, 1, 2) + np.roll(F, -1, 2) - 6 * F)


# ---- Target |psi(v)|^2 profiles (FROZEN) ----

def coords_grid(L: int) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    coords = np.arange(L)
    cx = cy = cz = L // 2
    dx = np.minimum(np.abs(coords - cx), L - np.abs(coords - cx))
    dy = np.minimum(np.abs(coords - cy), L - np.abs(coords - cy))
    dz = np.minimum(np.abs(coords - cz), L - np.abs(coords - cz))
    Dx, Dy, Dz = np.meshgrid(dx, dy, dz, indexing="ij")
    return Dx.astype(float), Dy.astype(float), Dz.astype(float)


def psi_sq_gaussian(p: Params) -> np.ndarray:
    """Profile 1: single Gaussian at lattice centre, sigma_psi = 3.0 = L/8."""
    Dx, Dy, Dz = coords_grid(p.L)
    r2 = Dx*Dx + Dy*Dy + Dz*Dz
    sigma_psi = 3.0
    psi_sq = np.exp(-r2 / (2 * sigma_psi * sigma_psi))
    return psi_sq


def psi_sq_uniform_envelope(p: Params) -> np.ndarray:
    """Profile 2: smooth-edge uniform region, R=6.0, w=1.5."""
    Dx, Dy, Dz = coords_grid(p.L)
    r = np.sqrt(Dx*Dx + Dy*Dy + Dz*Dz)
    R = 6.0
    w = 1.5
    psi_sq = 0.5 * (np.tanh((R - r) / w) + 1.0)
    return psi_sq


def psi_sq_two_bump(p: Params) -> np.ndarray:
    """Profile 3: two Gaussian bumps at cx ± 5, sigma_psi = 2.5."""
    L = p.L
    coords = np.arange(L)
    cx = cy = cz = L // 2
    # use SIGNED periodic differences for centre offset
    def dist2(ax, ay, az):
        Cx = np.minimum(np.abs(coords - ax), L - np.abs(coords - ax))
        Cy = np.minimum(np.abs(coords - ay), L - np.abs(coords - ay))
        Cz = np.minimum(np.abs(coords - az), L - np.abs(coords - az))
        Dx_, Dy_, Dz_ = np.meshgrid(Cx, Cy, Cz, indexing="ij")
        return Dx_*Dx_ + Dy_*Dy_ + Dz_*Dz_
    sigma_psi = 2.5
    bump_a = np.exp(-dist2(cx - 5, cy, cz) / (2 * sigma_psi * sigma_psi))
    bump_b = np.exp(-dist2(cx + 5, cy, cz) / (2 * sigma_psi * sigma_psi))
    return bump_a + bump_b


PROFILES = {
    "P1_gaussian":          psi_sq_gaussian,
    "P2_uniform_envelope":  psi_sq_uniform_envelope,
    "P3_two_bump":          psi_sq_two_bump,
}


# ---- Initial-condition sampling ----

def normalize_variance_field(psi_sq: np.ndarray, p: Params) -> np.ndarray:
    """sigma^2(v) chosen so max(sigma^2) = alpha (matches v1 energy scale)."""
    max_psi_sq = psi_sq.max()
    if max_psi_sq <= 0:
        raise RuntimeError("psi_sq target has non-positive maximum")
    return (p.alpha / max_psi_sq) * psi_sq  # sigma^2(v)


def initial_flux(p: Params, sigma_sq_field: np.ndarray,
                 trial_idx: int) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    sigma_field = np.sqrt(sigma_sq_field)
    rngs = [np.random.default_rng(seed=p.seed_master + trial_idx * 3 + axis)
            for axis in range(3)]
    Jx = sigma_field * rngs[0].normal(0.0, 1.0, sigma_field.shape)
    Jy = sigma_field * rngs[1].normal(0.0, 1.0, sigma_field.shape)
    Jz = sigma_field * rngs[2].normal(0.0, 1.0, sigma_field.shape)
    return Jx, Jy, Jz


# ---- Single-trial run ----

@dataclass
class TrialResult:
    long_count: np.ndarray   # int (L,L,L): events in [burnin, ticks_per_trial)
    first_event_site: Optional[Tuple[int, int, int]]


def run_trial(p: Params, sigma_sq_field: np.ndarray, trial_idx: int) -> TrialResult:
    L = p.L
    Jx, Jy, Jz = initial_flux(p, sigma_sq_field, trial_idx)
    Jx_prev, Jy_prev, Jz_prev = Jx.copy(), Jy.copy(), Jz.copy()
    s = np.zeros((L, L, L), dtype=np.int8)
    long_count = np.zeros((L, L, L), dtype=np.int64)
    first_event_site: Optional[Tuple[int, int, int]] = None
    damp = 1.0 - p.gamma

    for t in range(p.ticks_per_trial):
        Jx_new = damp * (2 * Jx - Jx_prev + p.c2 * laplacian6(Jx))
        Jy_new = damp * (2 * Jy - Jy_prev + p.c2 * laplacian6(Jy))
        Jz_new = damp * (2 * Jz - Jz_prev + p.c2 * laplacian6(Jz))
        Jx_prev, Jy_prev, Jz_prev = Jx, Jy, Jz
        Jx, Jy, Jz = Jx_new, Jy_new, Jz_new

        mag2 = Jx*Jx + Jy*Jy + Jz*Jz
        mag = np.sqrt(mag2)
        can_manifest = (s == 0) & (mag > p.K_B)
        sign_choice = np.sign(Jx + Jy + Jz)
        sign_choice[sign_choice == 0] = 1

        # First-event: site that manifested earliest in this trial
        if first_event_site is None and can_manifest.any():
            # Pick one deterministically: argmax of mag2 among manifesting sites
            flat_idx = (np.where(can_manifest, mag2, -1.0)).argmax()
            ix, iy, iz = np.unravel_index(flat_idx, can_manifest.shape)
            if can_manifest[ix, iy, iz]:
                first_event_site = (int(ix), int(iy), int(iz))

        if t >= p.burnin_ticks:
            long_count += can_manifest.astype(np.int64)

        s = np.where(can_manifest, sign_choice.astype(np.int8), s)
        can_evap = (np.abs(s) > 0) & (mag < p.K_B_evap)
        s = np.where(can_evap, np.int8(0), s)

    return TrialResult(long_count=long_count, first_event_site=first_event_site)


def run_ensemble_for_profile(p: Params, profile_name: str,
                              psi_sq_target: np.ndarray) -> Dict:
    sigma_sq_field = normalize_variance_field(psi_sq_target, p)
    L = p.L
    total_long = np.zeros((L, L, L), dtype=np.int64)
    first_hist = np.zeros((L, L, L), dtype=np.int64)
    print(f"\n  -- Profile {profile_name} --")
    print(f"     max sigma^2 = {sigma_sq_field.max():.4f} (matches v1 energy scale {p.alpha:.4f})")
    print(f"     psi^2 range: [{psi_sq_target.min():.4f}, {psi_sq_target.max():.4f}]")
    for trial in range(p.n_trials):
        res = run_trial(p, sigma_sq_field, trial)
        total_long += res.long_count
        if res.first_event_site is not None:
            first_hist[res.first_event_site] += 1
        if (trial + 1) % 25 == 0:
            print(f"     trial {trial+1}/{p.n_trials}; long events {total_long.sum()}")
    n_long_samples = p.n_trials * (p.ticks_per_trial - p.burnin_ticks)
    freq_long = total_long / n_long_samples
    hist_first = first_hist / p.n_trials  # fraction of trials whose first event was at v
    return {
        "psi_sq_target": psi_sq_target,
        "sigma_sq_field": sigma_sq_field,
        "long_count": total_long,
        "freq_long": freq_long,
        "first_hist": first_hist,
        "hist_first": hist_first,
        "n_long_samples": n_long_samples,
    }


# ---- Analysis ----

def select_mask(p: Params, psi_sq_target: np.ndarray) -> np.ndarray:
    L = p.L
    mask = np.ones((L, L, L), dtype=bool)
    mask[:2, :, :] = False
    mask[-2:, :, :] = False
    mask[:, :2, :] = False
    mask[:, -2:, :] = False
    mask[:, :, :2] = False
    mask[:, :, -2:] = False
    mask &= (psi_sq_target > 0.01)
    return mask


def bin_and_fit(psi_sq_flat: np.ndarray, freq_flat: np.ndarray,
                n_bins: int = 14, seed_offset: int = 1) -> Optional[dict]:
    if len(psi_sq_flat) < n_bins * 5:
        return None
    edges = np.percentile(psi_sq_flat, np.linspace(0, 100, n_bins + 1))
    bins = []
    for i in range(n_bins):
        lo, hi = edges[i], edges[i + 1]
        if i == n_bins - 1:
            inb = (psi_sq_flat >= lo) & (psi_sq_flat <= hi)
        else:
            inb = (psi_sq_flat >= lo) & (psi_sq_flat < hi)
        n_sites = int(inb.sum())
        if n_sites < 5:
            continue
        bins.append({
            "lo": float(lo), "hi": float(hi),
            "mean_psi_sq": float(psi_sq_flat[inb].mean()),
            "mean_freq": float(freq_flat[inb].mean()),
            "n_sites": n_sites,
        })
    if len(bins) < 4:
        return {"bins": bins, "fit": None}
    psi_arr = np.array([b["mean_psi_sq"] for b in bins])
    freq_arr = np.array([b["mean_freq"] for b in bins])
    valid = (freq_arr > 0) & (psi_arr > 0)
    if valid.sum() < 3:
        return {"bins": bins, "fit": None}
    x = np.log(psi_arr[valid])
    y = np.log(freq_arr[valid])
    slope, intercept = np.polyfit(x, y, 1)
    y_pred = intercept + slope * x
    ss_res = np.sum((y - y_pred) ** 2)
    ss_tot = np.sum((y - y.mean()) ** 2)
    r2 = 1.0 - ss_res / ss_tot if ss_tot > 0 else 0.0
    # bootstrap CI
    rng = np.random.default_rng(seed=P.seed_master + seed_offset)
    n_boot = 1000
    samples = []
    for _ in range(n_boot):
        idx = rng.integers(0, valid.sum(), size=valid.sum())
        if len(set(x[idx].tolist())) < 2:
            continue
        sl, _ = np.polyfit(x[idx], y[idx], 1)
        samples.append(sl)
    samples = np.array(samples)
    if len(samples) > 0:
        n_lo, n_hi = np.percentile(samples, [2.5, 97.5])
    else:
        n_lo, n_hi = float("nan"), float("nan")
    return {
        "bins": bins,
        "fit": {"n": float(slope), "log_A": float(intercept),
                "R2": float(r2), "CI": (float(n_lo), float(n_hi))},
    }


def classify_profile(fit_result: Optional[dict]) -> str:
    """Manifest section 4.1 per-profile classification."""
    if fit_result is None or fit_result.get("fit") is None:
        return "D"
    f = fit_result["fit"]
    n, r2 = f["n"], f["R2"]
    if 0.85 <= n <= 1.15 and r2 > 0.95:
        return "A"
    if abs(n) < 0.15 and r2 < 0.50:
        return "B"
    if abs(n) > 0.30 and (n < 0.85 or n > 1.15):
        return "C"
    return "D"


def aggregate_verdict(classes: Dict[str, str]) -> Tuple[str, str]:
    """Manifest section 4.2 aggregate verdict."""
    vals = list(classes.values())
    cA = vals.count("A")
    cB = vals.count("B")
    cC = vals.count("C")
    if cA == 3:
        return "A_strong", (
            "All 3 profiles preserve Born scaling. "
            "[NUMERICAL FACT - Born preservation] + [OBSERVATION supporting DGZ-equilibrium reading]; "
            "strong T1c foothold; substrate IS Born-equilibrium-compatible."
        )
    if cA == 2:
        return "A_partial", (
            "2 of 3 profiles preserve Born scaling. "
            "[NUMERICAL FACT - partial preservation]; preservation depends on profile shape."
        )
    if cB >= 2:
        return "B_equip", (
            "2 or more profiles drift to equipartition (Born profile washed out). "
            "[CLOSED NEGATIVE for DGZ-equilibrium in 6-neighbour substrate]; "
            "substrate equilibrium is uniform / equipartition, not Born. "
            "T1c not closable via DGZ route in this regime."
        )
    if cC >= 2:
        return "C_drift", (
            "2 or more profiles drift to non-trivial non-Born equilibrium. "
            "[NUMERICAL FACT - drift to non-Born equilibrium]; "
            "substrate has non-Born stationary distribution."
        )
    return "D_mixed", (
        "Mixed / inconclusive across profiles. "
        "[NUMERICAL FACT - mixed/inconclusive]; no aggregate Born claim, no clean negative."
    )


# ---- Main ----

def main():
    p = Params()
    print("=" * 78)
    print("BORN-EQUILIBRIUM PRESERVATION TEST (DGZ analog in FTD substrate)")
    print("Hash-locked: preregister-born-equilibrium-preservation-v1")
    print("=" * 78)
    print(f"Params: L={p.L}, K_B={p.K_B}, alpha={p.alpha:.4f}, gamma={p.gamma}, "
          f"trials={p.n_trials}, ticks_per_trial={p.ticks_per_trial}, "
          f"burnin={p.burnin_ticks}, seed={p.seed_master}")

    csv_rows: List[List] = []
    per_profile_summary: Dict[str, dict] = {}

    for prof_name, prof_fn in PROFILES.items():
        psi_sq = prof_fn(p)
        ens = run_ensemble_for_profile(p, prof_name, psi_sq)
        mask = select_mask(p, psi_sq)
        print(f"     masked voxels: {int(mask.sum())} / {p.L**3}")

        psi_sq_flat = psi_sq[mask]
        freq_long_flat = ens["freq_long"][mask]
        hist_first_flat = ens["hist_first"][mask]

        primary = bin_and_fit(psi_sq_flat, freq_long_flat, n_bins=14, seed_offset=1)
        secondary = bin_and_fit(psi_sq_flat, hist_first_flat, n_bins=14, seed_offset=2)

        cls = classify_profile(primary)
        per_profile_summary[prof_name] = {
            "primary": primary,
            "secondary": secondary,
            "class": cls,
        }

        # CSV rows
        L = p.L
        for ix in range(L):
            for iy in range(L):
                for iz in range(L):
                    csv_rows.append([
                        prof_name, ix, iy, iz,
                        f"{psi_sq[ix, iy, iz]:.6f}",
                        f"{ens['sigma_sq_field'][ix, iy, iz]:.6f}",
                        int(ens["long_count"][ix, iy, iz]),
                        f"{ens['freq_long'][ix, iy, iz]:.6f}",
                        int(ens["first_hist"][ix, iy, iz]),
                        f"{ens['hist_first'][ix, iy, iz]:.6f}",
                        int(mask[ix, iy, iz]),
                    ])

        print(f"     primary fit (freq_long vs psi^2): "
              f"n = {primary['fit']['n']:.4f}, R^2 = {primary['fit']['R2']:.4f}"
              if primary and primary.get("fit") else
              f"     primary fit: failed")
        if secondary and secondary.get("fit"):
            print(f"     secondary fit (hist_first vs psi^2): "
                  f"n = {secondary['fit']['n']:.4f}, R^2 = {secondary['fit']['R2']:.4f}")
        print(f"     per-profile class: {cls}")

    classes = {name: per_profile_summary[name]["class"] for name in per_profile_summary}
    outcome, msg = aggregate_verdict(classes)
    print("\n" + "=" * 78)
    print(f"AGGREGATE OUTCOME: {outcome}")
    print(msg)
    print(f"Per-profile classes: {classes}")
    print("=" * 78)

    csv_path = RESULTS_DIR / "born_equilibrium_preservation_2026-05-23.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["profile", "x", "y", "z", "psi_sq_target", "sigma_sq",
                    "long_count", "freq_long", "first_event_count",
                    "hist_first", "in_mask"])
        w.writerows(csv_rows)
    print(f"\nCSV written: {csv_path}")

    md_path = RESULTS_DIR / "born_equilibrium_preservation_2026-05-23.md"
    with md_path.open("w", encoding="utf-8") as f:
        f.write("# Born-Equilibrium Preservation Test -- Results\n\n")
        f.write("**Date:** 2026-05-23\n")
        f.write("**Pre-registration:** "
                "docs/theory/06_reference frame context/PREREG_BORN_EQUILIBRIUM_PRESERVATION_v1.md\n")
        f.write("**Git tag:** preregister-born-equilibrium-preservation-v1\n\n")
        f.write("---\n\n## Aggregate outcome\n\n")
        f.write(f"**Outcome `{outcome}`.** {msg}\n\n")
        f.write(f"Per-profile classes:\n")
        for name, cls in classes.items():
            f.write(f"- {name}: **{cls}**\n")
        f.write("\n---\n\n## Per-profile fits\n\n")
        for name, summary in per_profile_summary.items():
            f.write(f"### {name}\n\n")
            pf = summary["primary"]
            sf = summary["secondary"]
            if pf and pf.get("fit"):
                fit = pf["fit"]
                f.write(f"**Primary (long-run rate vs |psi|^2):** "
                        f"n = **{fit['n']:.4f}**, 95% CI = [{fit['CI'][0]:.4f}, {fit['CI'][1]:.4f}], "
                        f"R^2 = **{fit['R2']:.4f}**.  \n")
            else:
                f.write("**Primary:** fit failed (insufficient data).  \n")
            if sf and sf.get("fit"):
                fit = sf["fit"]
                f.write(f"**Secondary (first-event hist vs |psi|^2):** "
                        f"n = {fit['n']:.4f}, R^2 = {fit['R2']:.4f}.  \n")
            f.write(f"**Class:** {summary['class']}\n\n")
            if pf and pf.get("bins"):
                f.write("| bin range (psi^2) | mean psi^2 | mean freq_long | n_sites |\n")
                f.write("|---|---|---|---|\n")
                for b in pf["bins"]:
                    f.write(f"| {b['lo']:.4f} - {b['hi']:.4f} | "
                            f"{b['mean_psi_sq']:.4f} | {b['mean_freq']:.6f} | {b['n_sites']} |\n")
                f.write("\n")
        f.write("\n---\n\n## Full data\n\n")
        f.write(f"See `born_equilibrium_preservation_2026-05-23.csv` "
                f"({len(csv_rows)} rows, 3 profiles x {p.L**3} voxels).\n")
    print(f"MD written:  {md_path}")


if __name__ == "__main__":
    main()
