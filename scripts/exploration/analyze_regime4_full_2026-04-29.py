#!/usr/bin/env python3
"""
Full regime-4 (temporal/frequency) variance decomposition for FTD-0110.

Analyzes per-tick cluster_history files from:
  - Phase 1 (existing campaign at canonical A=10, stride-1):
      regime4_phase1/L32_stride1, L64_stride1, L32_30seeds, L32_ic2_stride1
  - Phase 2 (custom amplitude time-series binary, stride-1):
      regime4_phase2/A10_L32, A20_L32, A30_L32, A50_L32

For each dataset, computes:
  - Per-seed temporal mean/std of N(t)
  - Ensemble (across-seed) terminal mean/std
  - Total pooled std (combines temporal + ensemble)
  - Variance decomposition:
        Var_total = Var_within(seed) + Var_between(seeds)
                  ≡ regime-4   ≡ regimes-1-2-3
  - Autocorrelation of pooled trace
  - Per-tick boundary event rate (proxy: |N(t+1) - N(t)|)
"""

from __future__ import annotations

import csv
import math
import statistics
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent.parent
RESULTS = REPO / "engine" / "results"


def read_voxel_count_series(csv_path: Path) -> list[int]:
    series: list[int] = []
    if not csv_path.exists():
        return series
    with open(csv_path, newline="") as f:
        reader = csv.reader(f)
        next(reader, None)  # header
        for row in reader:
            if len(row) < 3:
                continue
            try:
                series.append(int(row[2]))
            except (ValueError, IndexError):
                continue
    return series


def autocorrelation(x: list[float], max_lag: int) -> list[float]:
    n = len(x)
    if n < 2:
        return []
    mu = sum(x) / n
    var = sum((xi - mu) ** 2 for xi in x) / n
    if var == 0:
        return [1.0] + [0.0] * max_lag
    out = []
    for lag in range(max_lag + 1):
        if lag >= n:
            out.append(float("nan"))
            continue
        c = sum((x[i] - mu) * (x[i + lag] - mu) for i in range(n - lag)) / (n - lag)
        out.append(c / var)
    return out


def event_rate(series: list[int]) -> float:
    """Mean per-tick |ΔN| across the series."""
    if len(series) < 2:
        return 0.0
    return sum(abs(series[i + 1] - series[i]) for i in range(len(series) - 1)) / (len(series) - 1)


def analyze_dir(label: str, ddir: Path, regime_label: str = ""):
    if not ddir.exists():
        return None
    csvs = sorted(ddir.glob("cluster_history_seed*.csv"))
    if not csvs:
        # check for nested ic1_inject/ic2_thermal subdirs
        for sub in ["ic1_inject", "ic2_thermal", "ic3_collision"]:
            sub_dir = ddir / sub
            if sub_dir.exists():
                csvs = sorted(sub_dir.glob("cluster_history_seed*.csv"))
                if csvs:
                    break
    if not csvs:
        return None

    print(f"--- {label}  (regime: {regime_label}) ---")
    print(f"    {ddir.name}")

    # Detect lattice size from path (L32, L64, L80 substrings); default L=32
    L = 32
    for cand in [128, 80, 64, 32, 16]:
        if f"L{cand}" in str(ddir) or f"L{cand}_" in str(ddir):
            L = cand
            break
    # Runaway threshold: cluster spanning > 25% of lattice volume = lattice-fill
    runaway_thresh = (L ** 3) // 4

    per_seed_traces = []
    for f in csvs:
        series = read_voxel_count_series(f)
        if len(series) < 5:
            continue
        if max(series) > runaway_thresh:
            print(f"  {f.name[:40]}: RUNAWAY skip (max={max(series)}, thresh={runaway_thresh}, L={L})")
            continue
        per_seed_traces.append(series)

    if len(per_seed_traces) < 2:
        print(f"  ... insufficient bound-state seeds")
        return None

    # Drop burn-in (first 20% of each trace)
    steady_traces = [s[len(s) // 5 :] for s in per_seed_traces]

    # Per-seed temporal stats
    per_seed_means = [statistics.mean(s) for s in steady_traces]
    per_seed_stds = [statistics.pstdev(s) if len(s) > 1 else 0 for s in steady_traces]
    per_seed_event_rates = [event_rate(s) for s in steady_traces]

    # Ensemble (terminal value across seeds)
    terminal_values = [s[-1] for s in steady_traces]
    ens_mean = statistics.mean(terminal_values)
    ens_std = statistics.stdev(terminal_values) if len(terminal_values) > 1 else 0
    ens_pstd = statistics.pstdev(terminal_values)

    # Variance decomposition (Anova-style):
    #   Var_total = Var_within + Var_between
    grand_mean = statistics.mean(per_seed_means)
    var_between = statistics.pstdev(per_seed_means) ** 2 * len(per_seed_means) / max(1, len(per_seed_means) - 0)
    # use group-means scatter (pop) → between-group variance
    var_between = sum((m - grand_mean) ** 2 for m in per_seed_means) / len(per_seed_means)
    # within-group variance: average of per-seed variances
    var_within = sum(s ** 2 for s in per_seed_stds) / len(per_seed_stds)
    var_total = var_between + var_within

    print(f"  Seeds (bound-state): {len(steady_traces)}")
    print(f"  Snapshots per seed: {len(steady_traces[0])}  (post-burn-in)")
    print(f"  Per-seed:")
    for i, s in enumerate(steady_traces):
        print(f"    seed {i}: mean={per_seed_means[i]:6.2f}  σ_t={per_seed_stds[i]:5.2f}  "
              f"range={min(s)}..{max(s)}  Δrate={per_seed_event_rates[i]:5.3f}")
    print(f"  Cross-seed terminal mean: {ens_mean:.2f}")
    print(f"  Cross-seed terminal std (sample, n-1):  {ens_std:.3f}")
    print(f"  Variance decomposition (Anova-style):")
    print(f"    Var_within  (regime-4, temporal)     = {var_within:6.3f}  σ_within  = {math.sqrt(var_within):.3f}")
    print(f"    Var_between (regimes-1-3, spatial IC) = {var_between:6.3f}  σ_between = {math.sqrt(var_between):.3f}")
    print(f"    Var_total                              = {var_total:6.3f}  σ_total   = {math.sqrt(var_total):.3f}")
    if var_total > 1e-9:
        f_temporal = var_within / var_total
        f_spatial = var_between / var_total
        print(f"    Fraction in regime-4 (temporal): {f_temporal*100:5.1f}%")
        print(f"    Fraction in regimes-1-3 (spatial-IC): {f_spatial*100:5.1f}%")

    # Pooled trace autocorrelation (within-seed only)
    all_within = []
    for s in steady_traces:
        m = statistics.mean(s)
        # demean each trace independently to isolate within-seed autocorrelation
        all_within.extend([v - m for v in s])
    if len(all_within) > 5:
        ac = autocorrelation(all_within, min(20, len(all_within) // 4))
        print(f"  Within-seed AC (lags 0..{len(ac)-1}):")
        # print as compact line
        ac_str = "    " + " ".join(f"{c:5.2f}" for c in ac[:21])
        print(ac_str)

    # Mean per-tick event rate (boundary churn)
    mean_event_rate = statistics.mean(per_seed_event_rates) if per_seed_event_rates else 0
    print(f"  Mean per-tick |ΔN| (boundary event rate): {mean_event_rate:.4f}")
    print()
    return {
        "label": label,
        "regime": regime_label,
        "ens_mean": ens_mean,
        "ens_std": ens_std,
        "var_within": var_within,
        "var_between": var_between,
        "var_total": var_total,
        "mean_event_rate": mean_event_rate,
        "n_seeds": len(steady_traces),
        "n_snapshots": len(steady_traces[0]),
    }


def main():
    print("=" * 80)
    print("FTD-0110 Bridge-II Regime-4 Full Analysis (2026-04-29)")
    print("=" * 80)
    print()
    print("Anova-style decomposition of cluster-size variance into")
    print("  Var_total = Var_within (regime-4 temporal) + Var_between (regimes 1-3 spatial-IC)")
    print()
    print("Each (L, A) → a regime classification per the four-regime structure:")
    print("  Regime 1 (lattice-pinned, A ≤ √27, cluster fits one 27-block)")
    print("  Regime 2 (free-boundary, 30 ≤ N ≤ 1000)")
    print("  Regime 3 (boundary-thickening, N > 1000)")
    print("  Regime 4 (temporal/frequency) -- this is the within-seed variance axis")
    print()

    # --- Phase 1 datasets ---
    p1 = RESULTS / "regime4_phase1"
    print("================ PHASE 1: stride-1 + ensemble-precision ================\n")
    results = []
    for label, sub, regime in [
        ("L=32 ic1 stride=1 (5 seeds)",     "L32_stride1",        "1 (lattice-pinned canonical)"),
        ("L=64 ic1 stride=1 (3 seeds)",     "L64_stride1",        "1 (lattice-pinned canonical)"),
        ("L=32 ic1 stride=50 (30 seeds)",   "L32_30seeds",        "1 (canonical ensemble precision)"),
        ("L=32 ic2 stride=1 (5 seeds)",     "L32_ic2_stride1",    "active-thermal (different physics)"),
    ]:
        r = analyze_dir(label, p1 / sub, regime)
        if r:
            results.append(r)

    # --- Phase 2 datasets ---
    p2 = RESULTS / "regime4_phase2"
    print("================ PHASE 2: amplitude-time-series ========================\n")
    for label, sub, regime in [
        ("A=10 L=32 stride=1 (10 seeds)",   "A10_L32",  "1 (lattice-pinned)"),
        ("A=20 L=32 stride=1 (10 seeds)",   "A20_L32",  "transition"),
        ("A=30 L=32 stride=1 (10 seeds)",   "A30_L32",  "2 (free-boundary)"),
        ("A=50 L=32 stride=1 (10 seeds)",   "A50_L32",  "2 (deep free-boundary)"),
    ]:
        r = analyze_dir(label, p2 / sub, regime)
        if r:
            results.append(r)

    # --- Phase 3 datasets: T sweep at A=50 ---
    p3 = RESULTS / "regime4_phase3"
    print("================ PHASE 3: temperature sweep at A=50 ====================\n")
    for label, sub, regime in [
        ("A=50 T=0.005 L=32 (10 seeds)", "T0.005",  "T-sweep canonical"),
        ("A=50 T=0.010 L=32 (10 seeds)", "T0.010",  "T-sweep 2x"),
        ("A=50 T=0.020 L=32 (10 seeds)", "T0.020",  "T-sweep 4x"),
        ("A=50 T=0.040 L=32 (10 seeds)", "T0.040",  "T-sweep 8x"),
    ]:
        r = analyze_dir(label, p3 / sub, regime)
        if r:
            results.append(r)

    # --- Phase 4 datasets: multi-amplitude T sweep ---
    p4 = RESULTS / "regime4_phase4"
    print("================ PHASE 4: multi-amplitude T sweep ======================\n")
    for label, sub, regime in [
        ("A=20 T=0.010 L=32 (10 seeds)", "A20_T0.010", "T-sweep at A=20"),
        ("A=20 T=0.020 L=32 (10 seeds)", "A20_T0.020", "T-sweep at A=20"),
        ("A=20 T=0.040 L=32 (10 seeds)", "A20_T0.040", "T-sweep at A=20"),
        ("A=30 T=0.010 L=32 (10 seeds)", "A30_T0.010", "T-sweep at A=30"),
        ("A=30 T=0.020 L=32 (10 seeds)", "A30_T0.020", "T-sweep at A=30"),
        ("A=30 T=0.040 L=32 (10 seeds)", "A30_T0.040", "T-sweep at A=30"),
        ("A=80 T=0.005 L=64 (5 seeds)",  "A80_T0.005_L64", "T-sweep at A=80"),
        ("A=80 T=0.010 L=64 (5 seeds)",  "A80_T0.010_L64", "T-sweep at A=80"),
        ("A=80 T=0.020 L=64 (5 seeds)",  "A80_T0.020_L64", "T-sweep at A=80"),
        ("A=80 T=0.040 L=64 (5 seeds)",  "A80_T0.040_L64", "T-sweep at A=80"),
    ]:
        r = analyze_dir(label, p4 / sub, regime)
        if r:
            results.append(r)

    # --- Phase 5 datasets: regime-3 thickening at L=80 ---
    p5 = RESULTS / "regime4_phase5"
    print("================ PHASE 5: regime-3 thickening (large N at L=80) ========\n")
    for label, sub, regime in [
        ("A=117.93 T=0.005 L=80 (5 seeds)", "tau_T0.005", "regime-3 thickening"),
        ("A=117.93 T=0.020 L=80 (5 seeds)", "tau_T0.020", "regime-3 thickening + active"),
    ]:
        r = analyze_dir(label, p5 / sub, regime)
        if r:
            results.append(r)

    # Summary table
    print("=" * 80)
    print("SUMMARY: Variance decomposition by regime")
    print("=" * 80)
    print(f"{'Dataset':<40} {'⟨N⟩':>7} {'σ_total':>8} {'σ_within':>9} "
          f"{'σ_between':>10} {'%temporal':>10} {'⟨|ΔN|⟩':>8}")
    for r in results:
        ft = r["var_within"] / r["var_total"] * 100 if r["var_total"] > 1e-9 else 0
        print(f"{r['label']:<40} {r['ens_mean']:>7.2f} "
              f"{math.sqrt(r['var_total']):>8.3f} "
              f"{math.sqrt(r['var_within']):>9.3f} "
              f"{math.sqrt(r['var_between']):>10.3f} "
              f"{ft:>9.1f}% "
              f"{r['mean_event_rate']:>8.4f}")

    print()
    print("INTERPRETATION:")
    print("  - %temporal close to 0%: regime-4 is degenerate (frozen state)")
    print("  - %temporal between 30-70%: ergodic regime (temporal ≈ spatial)")
    print("  - %temporal close to 100%: regime-4 dominates (boundary churn)")
    print()
    print("  - ⟨|ΔN|⟩ measures per-tick boundary event rate.")
    print("    Zero = no genesis/evaporation events at boundary in steady state")
    print("    Nonzero = active boundary; events count toward regime-4 entropy")


if __name__ == "__main__":
    main()
