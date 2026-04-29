#!/usr/bin/env python3
"""
Cluster-size temporal-variance analysis (regime 4) for FTD-0110 Bridge-II.

Tests the user's "frequency IS time" reframe:
  - Spatial regimes 1-3 set Var(N) at a fixed-time snapshot (boundary geometry).
  - Regime 4 (temporal) sets the frequency-spectrum / autocorrelation of N(t).
  - For an ergodic system at equilibrium, time-averaged Var(N(t)) per seed
    should approximately equal ensemble-averaged Var(N) across seeds.
  - Departure from ergodicity reveals where temporal entropy lives.

Reads cluster_history_seed*.csv files and computes per-seed:
  - mean N over snapshots (post-burn-in)
  - std N over snapshots (within-seed temporal fluctuation)
  - autocorrelation function C(τ)
  - Power spectrum (FFT of N(t))

Compares to ensemble (across-seed) variance at fixed time.
"""

from __future__ import annotations

import csv
import math
import statistics
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent.parent
RESULTS = REPO / "engine" / "results"

# Datasets to analyze: (label, dir)
DATASETS = [
    ("L=32 post-fix (FTD-0107)", RESULTS / "emergent_spectrum_postfix_2026-04-27" / "L32_ic1" / "ic1_inject"),
    ("L=64 G1 (FTD-0107)",       RESULTS / "emergent_spectrum_2026-04-27_L64" / "ic1_inject"),
    ("L=128 G2 (FTD-0107)",      RESULTS / "emergent_spectrum_2026-04-28_L128" / "ic1_inject"),
]


def read_history(csv_path: Path) -> list[tuple[int, int]]:
    """Return list of (tick, voxel_count) tuples for the largest cluster."""
    out: list[tuple[int, int]] = []
    if not csv_path.exists():
        return out
    with open(csv_path, newline="") as f:
        reader = csv.reader(f)
        next(reader, None)  # header
        for row in reader:
            if len(row) < 3 or not row[0].strip().lstrip("-").isdigit():
                continue
            try:
                tick = int(row[0])
                vox  = int(row[2])
            except ValueError:
                continue
            out.append((tick, vox))
    return out


def autocorr(x: list[float], max_lag: int) -> list[float]:
    """Sample autocorrelation function, lag 0 to max_lag (inclusive)."""
    n = len(x)
    if n < 2:
        return []
    mu = sum(x) / n
    var = sum((xi - mu) ** 2 for xi in x) / n
    if var == 0:
        return [1.0] + [float("nan")] * max_lag
    out = []
    for lag in range(max_lag + 1):
        if lag >= n:
            out.append(float("nan"))
            continue
        c = sum((x[i] - mu) * (x[i + lag] - mu) for i in range(n - lag)) / (n - lag)
        out.append(c / var)
    return out


def main() -> int:
    print("=" * 78)
    print("FTD-0110 Bridge-II Regime-4 (temporal) variance analysis (2026-04-29)")
    print("=" * 78)
    print()
    print("Reframe (user, 2026-04-28 evening): frequency IS time; amplitude is")
    print("event complexity. Spatial regimes 1-3 set Var(N) at fixed snapshot;")
    print("regime 4 is the temporal-axis entropy: how N(t) fluctuates across")
    print("ticks for a fixed seed.")
    print()
    print("Test: for each (lattice L, seed), compute mean N(t) and std N(t) over")
    print("the post-burn-in snapshot stream. Compare:")
    print("  - within-seed temporal std (regime 4)")
    print("  - across-seed ensemble std at terminal time (regimes 1-3)")
    print()

    for label, ddir in DATASETS:
        seeds = sorted(ddir.glob("cluster_history_seed*.csv"))
        if not seeds:
            continue
        print(f"--- {label} ---")
        print(f"    {ddir.name}")
        # Per-seed analysis
        all_terminal_N = []
        all_temporal_means = []
        all_temporal_stds = []
        all_temporal_minmax = []
        all_traces = []
        for f in seeds:
            history = read_history(f)
            if not history:
                continue
            ticks  = [h[0] for h in history]
            voxels = [h[1] for h in history]
            # Skip if a runaway seed (vacuum-collapse outlier) — voxel counts > 1000
            if max(voxels) > 1000:
                print(f"  {f.name[:35]:<35}  RUNAWAY (max={max(voxels)}, skipped)")
                continue
            # Skip burn-in: first half of snapshots are "transient"; we average
            # over the second half (more representative of steady state)
            n_trim = len(voxels) // 4
            steady = voxels[n_trim:]
            if len(steady) < 2:
                continue
            mu_t = sum(steady) / len(steady)
            sd_t = math.sqrt(sum((v - mu_t) ** 2 for v in steady) / len(steady))
            terminal = voxels[-1]
            all_terminal_N.append(terminal)
            all_temporal_means.append(mu_t)
            all_temporal_stds.append(sd_t)
            all_temporal_minmax.append((min(voxels), max(voxels)))
            all_traces.append((f.name[:35], voxels))
            print(f"  {f.name[:35]:<35}  ticks={ticks[0]}..{ticks[-1]}  "
                  f"snapshots={len(voxels)}  N(t) range={min(voxels)}..{max(voxels)}  "
                  f"⟨N(t)⟩={mu_t:.2f}  σ_t={sd_t:.3f}  terminal={terminal}")

        if len(all_terminal_N) < 2:
            print()
            continue

        # Ensemble (cross-seed) std at terminal time
        ens_mean = statistics.mean(all_terminal_N)
        ens_std  = statistics.stdev(all_terminal_N) if len(all_terminal_N) > 1 else 0
        # Mean of within-seed temporal stds
        mean_temporal_std = statistics.mean(all_temporal_stds) if all_temporal_stds else 0
        # Pooled temporal std (concatenate all seed traces)
        pooled = []
        for _, voxels in all_traces:
            n_trim = len(voxels) // 4
            pooled.extend(voxels[n_trim:])
        pooled_mean = sum(pooled) / len(pooled) if pooled else 0
        pooled_std  = math.sqrt(sum((v - pooled_mean) ** 2 for v in pooled) / len(pooled)) if pooled else 0
        # Ergodic ratio
        ergodic_ratio = mean_temporal_std / ens_std if ens_std > 0 else float("nan")
        print()
        print(f"  Ensemble (across-seed) terminal std:    {ens_std:.3f}")
        print(f"  Mean within-seed temporal std (steady): {mean_temporal_std:.3f}")
        print(f"  Pooled temporal std (all seeds, steady): {pooled_std:.3f}")
        print(f"  Ergodic ratio (temporal/ensemble):      {ergodic_ratio:.3f}")

        # Autocorrelation of pooled trace
        if pooled and len(pooled) > 5:
            ac = autocorr(pooled, min(15, len(pooled) // 2))
            print(f"  Autocorrelation (lag 0..{len(ac)-1}):")
            print(f"    " + "  ".join(f"{c:.2f}" for c in ac))
        print()

    print("=" * 78)
    print("INTERPRETATION")
    print("=" * 78)
    print()
    print("Ergodic ratio = (within-seed temporal std) / (across-seed ensemble std)")
    print()
    print("  ratio ≈ 1: ergodic. Temporal sampling = ensemble sampling.")
    print("              Regime 4 entropy contribution similar to spatial.")
    print("  ratio < 1: cluster is 'frozen' in time (each seed converges to a")
    print("              specific N, doesn't fluctuate); ensemble variance comes")
    print("              from initial-condition divergence (regimes 1-3 dominate).")
    print("  ratio > 1: cluster fluctuates in time more than across seeds;")
    print("              regime-4 entropy dominates spatial entropy.")
    print()
    print("Autocorrelation reveals the timescale of temporal entropy generation:")
    print("  - Fast decay (ac drops to 0 within a few lags): high-frequency churn")
    print("  - Slow decay (ac stays > 0.5 for many lags): slow drift, low-freq dynamics")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
