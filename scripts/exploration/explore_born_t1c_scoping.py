"""
explore_born_t1c_scoping.py -- scoping run for T1c (Born rule emergence).

Target: LEDGER FTD-0187 / target T1c -- the [OPEN] step "probability
(manifestation frequency) = normalized energy density."

Scope of this script: SCOPING ONLY. Find a parameter regime in a
simplified Python lattice where:
  (a) manifestation events occur frequently enough for statistics, AND
  (b) the spatial distribution of |J| has enough dynamic range to fit
      a power law freq ~ |J|^n.

Caveats (must be reported with any result):
  - 6-neighbour face Laplacian, NOT FTD-canonical 26-neighbour Moore.
  - K_B and g_c are tuned for statistics, not at canonical FTD values.
  - This is a simplified Python lattice, NOT the C++ engine.

Any "Born-supporting" or "Born-refuting" claim from this script is
[NUMERICAL FACT] of this simplified dynamics ONLY. A canonical T1c
test requires an engine experiment with 26-neighbour Moore stencil,
canonical K_B = 0.511, and Moore-mediated wave dynamics. That is
the planned next step IF scoping produces a meaningful n.

No pre-registration. The scoping run is methodological -- it asks
"does this experimental design produce data?", not "what is n?".
A subsequent hash-locked production run will ask "what is n?".
"""

from __future__ import annotations

import csv
from dataclasses import dataclass
from pathlib import Path

import numpy as np

REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "scripts" / "exploration" / "results"
RESULTS_DIR.mkdir(parents=True, exist_ok=True)


@dataclass
class Params:
    L: int = 32                 # lattice side length
    K_B: float = 0.30           # manifestation threshold (LOWER than canonical 0.511 for stats)
    K_B_evap: float = 0.15      # evaporation threshold
    c2: float = 1.0 / 3.0       # c^2 = 1/D (D=3 spatial)
    g_c: float = 0.40           # source coupling (HIGHER than 0.085 for stats)
    gamma: float = 0.005        # damping near manifested sites
    n_trials: int = 200          # number of ensemble samples
    ticks_per_trial: int = 40    # ticks of evolution per trial
    n_burnin_ticks: int = 200    # ticks to build flux field before counting
    seed: int = 42


class Lattice:
    """Minimal Python lattice for T1c scoping. 6-neighbour Laplacian, periodic BCs."""

    def __init__(self, p: Params):
        L = p.L
        self.p = p
        self.s = np.zeros((L, L, L), dtype=int)
        self.Jx = np.zeros((L, L, L))
        self.Jy = np.zeros((L, L, L))
        self.Jz = np.zeros((L, L, L))
        self.Jx_prev = np.zeros((L, L, L))
        self.Jy_prev = np.zeros((L, L, L))
        self.Jz_prev = np.zeros((L, L, L))

    def _lap(self, F):
        return (np.roll(F, 1, 0) + np.roll(F, -1, 0)
              + np.roll(F, 1, 1) + np.roll(F, -1, 1)
              + np.roll(F, 1, 2) + np.roll(F, -1, 2) - 6 * F)

    def _grad_s(self):
        gx = (np.roll(self.s, -1, 0) - np.roll(self.s, 1, 0)).astype(float) / 2
        gy = (np.roll(self.s, -1, 1) - np.roll(self.s, 1, 1)).astype(float) / 2
        gz = (np.roll(self.s, -1, 2) - np.roll(self.s, 1, 2)).astype(float) / 2
        return gx, gy, gz

    def flux_mag(self):
        return np.sqrt(self.Jx**2 + self.Jy**2 + self.Jz**2)

    def tick(self):
        p = self.p
        gx, gy, gz = self._grad_s()
        Jx_new = 2*self.Jx - self.Jx_prev + p.c2 * self._lap(self.Jx) + p.g_c * gx
        Jy_new = 2*self.Jy - self.Jy_prev + p.c2 * self._lap(self.Jy) + p.g_c * gy
        Jz_new = 2*self.Jz - self.Jz_prev + p.c2 * self._lap(self.Jz) + p.g_c * gz

        # mild damping everywhere (not just near particles -- prevents runaway)
        damp = 1.0 - p.gamma
        Jx_new *= damp
        Jy_new *= damp
        Jz_new *= damp

        self.Jx_prev = self.Jx.copy()
        self.Jy_prev = self.Jy.copy()
        self.Jz_prev = self.Jz.copy()
        self.Jx, self.Jy, self.Jz = Jx_new, Jy_new, Jz_new

        # Manifestation: void -> +-1 where |J| > K_B
        mag = self.flux_mag()
        can_manifest = (self.s == 0) & (mag > p.K_B)
        sign = np.sign(self.Jx + self.Jy + self.Jz)
        sign[sign == 0] = 1
        self.s[can_manifest] = sign[can_manifest].astype(int)

        # Evaporation: +-1 -> 0 where |J| < K_B_evap
        can_evap = (np.abs(self.s) > 0) & (mag < p.K_B_evap)
        self.s[can_evap] = 0


def run_experiment(p: Params, verbose: bool = True):
    rng = np.random.default_rng(p.seed)
    L = p.L
    lat = Lattice(p)

    # Set a strong oscillating dipole source: two opposite manifested particles
    # at (L/2 +- 4, L/2, L/2), kept locked. The dipole generates a flux field
    # with substantial spatial variation across the lattice -- this gives us
    # dynamic range in |J| for the binning step.
    cx, cy, cz = L // 2, L // 2, L // 2
    src_a = (cx - 4, cy, cz)
    src_b = (cx + 4, cy, cz)

    # Burn-in: build the flux field
    if verbose:
        print(f"  Burn-in: {p.n_burnin_ticks} ticks to build flux field...")
    for t in range(p.n_burnin_ticks):
        lat.s[src_a] = 1
        lat.s[src_b] = -1
        lat.tick()

    # Snapshot the flux magnitude AFTER burn-in -- this is our "background" |J|
    J_snapshot = lat.flux_mag().copy()
    if verbose:
        print(f"  |J| snapshot: min={J_snapshot.min():.4f} max={J_snapshot.max():.4f} "
              f"mean={J_snapshot.mean():.4f}")

    # Ensemble: run n_trials, each trial reset states except sources, count
    # manifestations per voxel.
    manifest_count = np.zeros((L, L, L), dtype=int)
    if verbose:
        print(f"  Running {p.n_trials} trials x {p.ticks_per_trial} ticks each...")

    for trial in range(p.n_trials):
        # Reset states (keep sources locked, clear everything else)
        lat.s[:] = 0
        lat.s[src_a] = 1
        lat.s[src_b] = -1

        # Perturb J slightly with deterministic per-trial noise so trials are
        # not bit-identical. This gives the ensemble its variation.
        noise_scale = 0.01
        lat.Jx += rng.normal(0, noise_scale, lat.Jx.shape)
        lat.Jy += rng.normal(0, noise_scale, lat.Jy.shape)
        lat.Jz += rng.normal(0, noise_scale, lat.Jz.shape)

        # Run the trial
        any_manifested = np.zeros((L, L, L), dtype=bool)
        for t in range(p.ticks_per_trial):
            lat.s[src_a] = 1
            lat.s[src_b] = -1
            lat.tick()
            # Mark each voxel as having manifested if it ever became non-zero
            any_manifested |= (np.abs(lat.s) > 0)

        # Exclude the source sites from the count
        any_manifested[src_a] = False
        any_manifested[src_b] = False
        manifest_count += any_manifested.astype(int)

    return J_snapshot, manifest_count, src_a, src_b


def analyse(J: np.ndarray, count: np.ndarray, p: Params, src_a, src_b, verbose: bool = True):
    L = p.L
    # Mask: exclude sources and their immediate neighbourhood
    mask = np.ones_like(J, dtype=bool)
    for src in (src_a, src_b):
        for dx in range(-2, 3):
            for dy in range(-2, 3):
                for dz in range(-2, 3):
                    ix = (src[0] + dx) % L
                    iy = (src[1] + dy) % L
                    iz = (src[2] + dz) % L
                    mask[ix, iy, iz] = False

    # Only sites with non-trivial |J| -- below numerical noise filter
    mask &= (J > 0.01)

    J_sel = J[mask]
    count_sel = count[mask]
    freq_sel = count_sel.astype(float) / p.n_trials  # fraction of trials manifesting

    if verbose:
        print(f"\n  Sites included in analysis: {mask.sum()} (out of {L**3 - 2 * 5**3})")
        print(f"  Manifestation frequency: min={freq_sel.min():.4f} max={freq_sel.max():.4f} "
              f"mean={freq_sel.mean():.4f}")
        print(f"  |J| in analysis: min={J_sel.min():.4f} max={J_sel.max():.4f}")

    # Bin by |J| (using equal-count percentile bins for stable statistics)
    n_bins = 14
    bin_edges = np.percentile(J_sel, np.linspace(0, 100, n_bins + 1))

    bin_data = []  # (mean_J, mean_freq, n_sites)
    for i in range(n_bins):
        in_bin = (J_sel >= bin_edges[i]) & (J_sel < bin_edges[i + 1])
        if i == n_bins - 1:
            in_bin = (J_sel >= bin_edges[i]) & (J_sel <= bin_edges[i + 1])
        n_sites = in_bin.sum()
        if n_sites < 5:
            continue
        mean_J = J_sel[in_bin].mean()
        mean_freq = freq_sel[in_bin].mean()
        bin_data.append((mean_J, mean_freq, n_sites))

    # Print bin table
    if verbose:
        print(f"\n  {'|J| range':>20} | {'mean |J|':>10} | {'mean |J|^2':>10} | "
              f"{'mean freq':>10} | {'n sites':>8}")
        print("  " + "-" * 78)
        for i, (mJ, mF, nS) in enumerate(bin_data):
            lo = bin_edges[i]
            hi = bin_edges[i + 1]
            print(f"  {f'{lo:.3f}-{hi:.3f}':>20} | {mJ:>10.4f} | {mJ**2:>10.4f} | "
                  f"{mF:>10.4f} | {nS:>8d}")

    # Fit freq = A * |J|^n on bins with non-zero freq
    if len(bin_data) >= 4:
        arr = np.array(bin_data)
        bJ = arr[:, 0]
        bF = arr[:, 1]
        valid = (bF > 0)
        if valid.sum() >= 3:
            log_J = np.log(bJ[valid])
            log_F = np.log(bF[valid])
            n_fit, log_A = np.polyfit(log_J, log_F, 1)
            # bootstrap CI on n
            n_boot = 1000
            rng = np.random.default_rng(p.seed + 1)
            n_samples = []
            for _ in range(n_boot):
                idx = rng.integers(0, valid.sum(), size=valid.sum())
                lj = log_J[idx]
                lf = log_F[idx]
                if len(set(lj.tolist())) < 2:
                    continue
                slope, _ = np.polyfit(lj, lf, 1)
                n_samples.append(slope)
            n_samples = np.array(n_samples)
            n_low, n_high = np.percentile(n_samples, [2.5, 97.5])
            if verbose:
                print(f"\n  POWER-LAW FIT (freq = A * |J|^n):")
                print(f"    n     = {n_fit:.4f}")
                print(f"    95% CI = [{n_low:.4f}, {n_high:.4f}]")
                print(f"    A     = {np.exp(log_A):.4e}")
                print(f"    Born predicts n = 2 (T1c)")
                print(f"    Classical (linear)  n = 1")
                if 1.8 <= n_low and n_high <= 2.2:
                    print(f"    -> Consistent with Born (n=2 inside 95% CI)")
                elif 0.8 <= n_low and n_high <= 1.2:
                    print(f"    -> Consistent with classical linear (n=1 inside 95% CI)")
                else:
                    print(f"    -> Neither Born nor classical-linear fits within CI")
            return n_fit, (n_low, n_high), bin_data, bJ, bF
    return None, None, bin_data, None, None


def main():
    p = Params()
    print("=" * 76)
    print("BORN-RULE T1c SCOPING RUN")
    print("Target: LEDGER FTD-0187 / T1c -- 'freq = normalised energy density'")
    print("Caveats: 6-neighbour Python lattice (not 26-neighbour engine), tuned K_B/g_c")
    print("=" * 76 + "\n")
    print(f"Params: L={p.L} K_B={p.K_B} g_c={p.g_c} gamma={p.gamma} "
          f"trials={p.n_trials} ticks_per_trial={p.ticks_per_trial} seed={p.seed}")

    J, count, src_a, src_b = run_experiment(p, verbose=True)
    n_fit, ci, bin_data, bJ, bF = analyse(J, count, p, src_a, src_b, verbose=True)

    # Write CSV
    out_csv = RESULTS_DIR / "born_t1c_scoping_2026-05-23.csv"
    with out_csv.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["bin_idx", "mean_J", "mean_J_squared", "mean_freq", "n_sites"])
        for i, (mJ, mF, nS) in enumerate(bin_data):
            w.writerow([i, f"{mJ:.6f}", f"{mJ**2:.6f}", f"{mF:.6f}", nS])
    print(f"\nCSV written: {out_csv}")
    if n_fit is not None:
        print(f"FINAL: n = {n_fit:.4f}, 95% CI = [{ci[0]:.4f}, {ci[1]:.4f}]")


if __name__ == "__main__":
    main()
