"""
Avalanche Statistics at the FTD Phase Transition
=================================================

Measures whether the FTD vacuum phase transition (at beta = pi)
produces power-law avalanche statistics.

The hypothesis: If the system is genuinely critical at beta = pi,
manifestation events should cascade in clusters whose size distribution
follows P(s) ~ s^{-tau} with tau ~ 3/2 (mean-field branching process
exponent, matching Beggs & Plenz neural avalanche data).

Method:
  1. Enable ZPF at a controlled amplitude (overriding the FDR formula)
  2. Run warmup to reach quasi-equilibrium
  3. Track new manifestations ("births") per tick
  4. Cluster births spatially using scipy.ndimage.label
  5. Record each cluster's size as one "avalanche"
  6. Fit power law P(s) ~ s^{-tau} on log-log scale
  7. Compute spatial pair correlation function g(r)

Key gap noted: The coupling term g_c * s * div(J) from the action
principle (CLAUDE.md section 13.2) is NOT implemented as a force in
the simulation. This baseline uses the simulation as-is.
"""

import numpy as np
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from scipy.ndimage import label
from collections import defaultdict

from ternary_matrix.config import PhysicsConfig
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation


# 6-connectivity structure for scipy.ndimage.label
STRUCT_6 = np.array([
    [[0, 0, 0], [0, 1, 0], [0, 0, 0]],
    [[0, 1, 0], [1, 1, 1], [0, 1, 0]],
    [[0, 0, 0], [0, 1, 0], [0, 0, 0]],
])


def measure_avalanches(universe, prev_states, struct=STRUCT_6):
    """
    Identify new manifestation events ("births") and cluster them
    spatially. Each spatially connected cluster of births = one avalanche.

    Args:
        universe: Current universe (after tick)
        prev_states: States array from before the tick
        struct: Structuring element for connectivity

    Returns:
        list of avalanche sizes (integers), one per cluster
    """
    # Births: sites that were void (0) last tick and are now manifest (!=0)
    births = (prev_states == 0) & (universe.states != 0)

    if not np.any(births):
        return []

    birth_mask = births.astype(np.int32)
    labeled, num_clusters = label(birth_mask, structure=struct)

    if num_clusters == 0:
        return []

    # Count sites per cluster (skip background label 0)
    counts = np.bincount(labeled.ravel())
    sizes = counts[1:].tolist()
    return sizes


def compute_correlation_function(states, max_r=None):
    """
    Compute the radial pair correlation function g(r) for manifested sites.

    g(r) = <n(0) n(r)> / <n>^2

    where n(x) = 1 if site x is manifested, 0 otherwise.

    Uses FFT-based autocorrelation for efficiency.

    Returns:
        (r_values, g_r) arrays, or (None, None) if density is 0 or 1
    """
    mask = (states != 0).astype(np.float64)
    rho_bar = np.mean(mask)

    if rho_bar < 1e-6 or rho_bar > 1 - 1e-6:
        return None, None

    N = mask.shape[0]
    if max_r is None:
        max_r = N // 2

    # FFT autocorrelation
    fft_mask = np.fft.rfftn(mask)
    autocorr = np.fft.irfftn(fft_mask * np.conj(fft_mask), s=mask.shape)
    autocorr /= mask.size
    g = autocorr / (rho_bar ** 2)

    # Radial distance array (wrapped for periodic BC)
    coords = np.arange(N)
    coords = np.where(coords > N // 2, coords - N, coords)
    xx, yy, zz = np.meshgrid(coords, coords, coords, indexing='ij')
    r = np.sqrt(xx ** 2 + yy ** 2 + zz ** 2)

    # Bin by integer distance using bincount
    r_int = np.round(r).astype(int).ravel()
    g_flat = g.ravel()

    # Only keep r <= max_r
    valid = r_int <= max_r
    r_valid = r_int[valid]
    g_valid = g_flat[valid]

    g_sum = np.bincount(r_valid, weights=g_valid, minlength=max_r + 1)
    g_count = np.bincount(r_valid, minlength=max_r + 1)
    g_count = np.maximum(g_count, 1)  # avoid /0

    g_r = g_sum / g_count
    r_values = np.arange(max_r + 1)

    # r=0 is self-correlation, skip it
    return r_values[1:], g_r[1:]


def fit_power_law(sizes, s_min=1):
    """
    Fit a power law P(s) ~ s^{-tau} to an avalanche size distribution
    using log-binned data and least-squares regression.

    Returns:
        (tau, r_squared, n_decades) or (None, None, 0) if insufficient data
    """
    if len(sizes) < 10:
        return None, None, 0

    sizes = np.array(sizes)
    sizes = sizes[sizes >= s_min]

    if len(sizes) < 10:
        return None, None, 0

    # Log-bin the histogram
    s_max = sizes.max()
    if s_max <= s_min:
        return None, None, 0

    n_bins = max(10, int(np.log2(s_max / s_min) * 4))
    bin_edges = np.logspace(np.log10(s_min), np.log10(s_max + 1), n_bins + 1)
    hist, edges = np.histogram(sizes, bins=bin_edges)

    # Bin centers (geometric mean)
    centers = np.sqrt(edges[:-1] * edges[1:])

    # Normalize to probability density
    bin_widths = edges[1:] - edges[:-1]
    pdf = hist / (len(sizes) * bin_widths)

    # Filter to non-zero bins
    nz = pdf > 0
    if nz.sum() < 3:
        return None, None, 0

    log_s = np.log10(centers[nz])
    log_p = np.log10(pdf[nz])

    # Linear regression: log_p = -tau * log_s + const
    A = np.vstack([log_s, np.ones_like(log_s)]).T
    result = np.linalg.lstsq(A, log_p, rcond=None)
    slope, intercept = result[0]
    tau = -slope

    # R-squared
    residuals = log_p - (slope * log_s + intercept)
    ss_res = np.sum(residuals ** 2)
    ss_tot = np.sum((log_p - np.mean(log_p)) ** 2)
    r_squared = 1 - ss_res / ss_tot if ss_tot > 0 else 0

    n_decades = log_s[-1] - log_s[0]

    return tau, r_squared, n_decades


def fit_correlation_decay(r_values, g_r, r_min=2, r_max=None):
    """
    Fit g(r) to determine if it's power-law or exponential decay.

    Returns:
        dict with 'power_law_eta', 'exp_xi', and fit qualities
    """
    if r_values is None or len(r_values) < 5:
        return {'power_law_eta': None, 'exp_xi': None}

    if r_max is None:
        r_max = len(r_values)

    # Filter to fitting range, g(r) > 0, g(r) != 1 (avoid self-corr)
    mask = (r_values >= r_min) & (r_values <= r_max) & (g_r > 0) & (np.abs(g_r - 1) > 1e-6)
    r_fit = r_values[mask].astype(float)
    g_fit = g_r[mask]

    if len(r_fit) < 4:
        return {'power_law_eta': None, 'exp_xi': None}

    result = {'power_law_eta': None, 'power_law_R2': 0,
              'exp_xi': None, 'exp_R2': 0}

    # Fit power law: log(g-1) ~ -eta * log(r)  (g(r) = 1 + A*r^{-eta})
    delta_g = g_fit - 1.0
    pos = delta_g > 0
    if pos.sum() >= 3:
        log_r = np.log10(r_fit[pos])
        log_dg = np.log10(delta_g[pos])
        A = np.vstack([log_r, np.ones_like(log_r)]).T
        res = np.linalg.lstsq(A, log_dg, rcond=None)
        eta = -res[0][0]
        resid = log_dg - (res[0][0] * log_r + res[0][1])
        ss_res = np.sum(resid ** 2)
        ss_tot = np.sum((log_dg - np.mean(log_dg)) ** 2)
        r2 = 1 - ss_res / ss_tot if ss_tot > 0 else 0
        result['power_law_eta'] = eta
        result['power_law_R2'] = r2

    # Fit exponential: log(g-1) ~ -r/xi
    if pos.sum() >= 3:
        ln_dg = np.log(delta_g[pos])
        A = np.vstack([r_fit[pos], np.ones_like(r_fit[pos])]).T
        res = np.linalg.lstsq(A, ln_dg, rcond=None)
        xi = -1.0 / res[0][0] if res[0][0] < 0 else float('inf')
        resid = ln_dg - (res[0][0] * r_fit[pos] + res[0][1])
        ss_res = np.sum(resid ** 2)
        ss_tot = np.sum((ln_dg - np.mean(ln_dg)) ** 2)
        r2 = 1 - ss_res / ss_tot if ss_tot > 0 else 0
        result['exp_xi'] = xi
        result['exp_R2'] = r2

    return result


def run_avalanche_scan(grid_size=32, warmup_ticks=200, measure_ticks=2000,
                       sigma_values=None, report_interval=500):
    """
    Main experiment: scan ZPF amplitude across the phase transition
    and measure avalanche statistics + spatial correlations at each.
    """
    if sigma_values is None:
        sigma_values = [0.060, 0.065, 0.070, 0.073, 0.075, 0.078, 0.080, 0.085, 0.090]

    print("=" * 75)
    print("AVALANCHE DYNAMICS AT THE FTD PHASE TRANSITION")
    print("=" * 75)
    print()
    print(f"Grid:            {grid_size}^3 = {grid_size**3:,} sites")
    print(f"Warmup:          {warmup_ticks} ticks")
    print(f"Measurement:     {measure_ticks} ticks")
    print(f"Sigma values:    {sigma_values}")
    print()
    print("Hypothesis: If critical at beta=pi, expect tau ~ 1.5")
    print("            (mean-field branching process / neural avalanche)")
    print()

    # Modify the global singleton for ZPF control
    import ternary_matrix.config as cfg
    original_zpf = cfg.CONSTANTS.ENABLE_ZPF
    original_amp = cfg.CONSTANTS.ZPF_AMPLITUDE

    results = []

    try:
        cfg.CONSTANTS.ENABLE_ZPF = True

        for sigma in sigma_values:
            print("-" * 75)
            print(f"SIGMA = {sigma:.4f}")
            print("-" * 75)

            cfg.CONSTANTS.ZPF_AMPLITUDE = sigma

            # Create fresh universe
            universe = Universe(size=grid_size)
            n_sites = grid_size ** 3

            # Warmup phase
            print(f"  Warming up ({warmup_ticks} ticks)...", end="", flush=True)
            for t in range(warmup_ticks):
                master_equation.tick(universe)
            f_warmup = universe.get_manifested_count() / n_sites
            print(f" done. f_manifest = {f_warmup*100:.2f}%")

            # Measurement phase
            all_avalanche_sizes = []
            ticks_with_births = 0
            total_births = 0

            print(f"  Measuring ({measure_ticks} ticks)...", flush=True)

            for t in range(measure_ticks):
                prev_states = universe.states.copy()
                master_equation.tick(universe)

                sizes = measure_avalanches(universe, prev_states)
                if sizes:
                    all_avalanche_sizes.extend(sizes)
                    ticks_with_births += 1
                    total_births += sum(sizes)

                if (t + 1) % report_interval == 0:
                    f_now = universe.get_manifested_count() / n_sites
                    print(f"    tick {t+1:5d}: f={f_now*100:.2f}%, "
                          f"avalanches so far: {len(all_avalanche_sizes)}")

            # Compute final manifest fraction
            f_final = universe.get_manifested_count() / n_sites
            f_births = ticks_with_births / measure_ticks

            # Fit power law
            tau, r2, n_dec = fit_power_law(all_avalanche_sizes)

            # Avalanche statistics
            if all_avalanche_sizes:
                sizes_arr = np.array(all_avalanche_sizes)
                mean_size = np.mean(sizes_arr)
                max_size = np.max(sizes_arr)
                median_size = np.median(sizes_arr)
            else:
                mean_size = max_size = median_size = 0

            # Correlation function (average over last few snapshots)
            r_vals, g_r = compute_correlation_function(universe.states)
            corr_fit = fit_correlation_decay(r_vals, g_r) if r_vals is not None else {}

            entry = {
                'sigma': sigma,
                'f_manifest': f_final,
                'f_births': f_births,
                'n_avalanches': len(all_avalanche_sizes),
                'total_births': total_births,
                'mean_size': mean_size,
                'max_size': max_size,
                'median_size': median_size,
                'tau': tau,
                'tau_R2': r2,
                'tau_decades': n_dec,
                'corr_eta': corr_fit.get('power_law_eta'),
                'corr_eta_R2': corr_fit.get('power_law_R2', 0),
                'corr_xi': corr_fit.get('exp_xi'),
                'corr_xi_R2': corr_fit.get('exp_R2', 0),
                'r_values': r_vals,
                'g_r': g_r,
                'avalanche_sizes': all_avalanche_sizes,
            }
            results.append(entry)

            print(f"  Results: f={f_final*100:.1f}%, "
                  f"avalanches={len(all_avalanche_sizes)}, "
                  f"mean_s={mean_size:.2f}, max_s={max_size}")
            if tau is not None:
                print(f"  Power law: tau={tau:.3f}, R^2={r2:.3f}, "
                      f"decades={n_dec:.2f}")
            else:
                print(f"  Power law: insufficient data")
            if corr_fit.get('corr_xi') is not None:
                print(f"  Correlation: xi={corr_fit.get('exp_xi', 0):.2f} "
                      f"(R2={corr_fit.get('exp_R2', 0):.3f}), "
                      f"eta={corr_fit.get('power_law_eta', 0):.2f} "
                      f"(R2={corr_fit.get('power_law_R2', 0):.3f})")
            print()

    finally:
        cfg.CONSTANTS.ENABLE_ZPF = original_zpf
        cfg.CONSTANTS.ZPF_AMPLITUDE = original_amp

    # Summary table
    print()
    print("=" * 75)
    print("SUMMARY TABLE")
    print("=" * 75)
    print()
    print(f"{'Sigma':>7} | {'f_man%':>7} | {'f_birth':>7} | "
          f"{'N_aval':>7} | {'<s>':>7} | {'s_max':>7} | "
          f"{'tau':>6} | {'R^2':>5} | {'xi':>6} | Character")
    print("-" * 95)

    for r in results:
        # Classify character
        if r['f_manifest'] < 0.01:
            char = "Subcritical"
        elif r['f_manifest'] > 0.95:
            char = "Supercritical"
        elif r['tau'] is not None and r['tau_R2'] is not None and r['tau_R2'] > 0.8:
            if 1.2 < r['tau'] < 2.0:
                char = "CRITICAL?"
            else:
                char = "Power-law"
        else:
            char = "Transitional"

        tau_str = f"{r['tau']:.2f}" if r['tau'] is not None else "  N/A"
        r2_str = f"{r['tau_R2']:.2f}" if r['tau_R2'] is not None else " N/A"
        xi_str = f"{r['corr_xi']:.1f}" if r['corr_xi'] is not None and r['corr_xi'] < 1000 else "  N/A"

        print(f"{r['sigma']:7.4f} | {r['f_manifest']*100:6.2f}% | "
              f"{r['f_births']:7.3f} | {r['n_avalanches']:7d} | "
              f"{r['mean_size']:7.2f} | {r['max_size']:7d} | "
              f"{tau_str:>6} | {r2_str:>5} | {xi_str:>6} | {char}")

    print()

    # Interpretation
    print("=" * 75)
    print("INTERPRETATION")
    print("=" * 75)
    print()

    # Find critical point candidate (sigma with tau closest to 1.5 and good R2)
    critical_candidates = [r for r in results
                           if r['tau'] is not None
                           and r['tau_R2'] is not None
                           and r['tau_R2'] > 0.7]

    if critical_candidates:
        best = min(critical_candidates, key=lambda r: abs(r['tau'] - 1.5))
        print(f"Best critical candidate: sigma = {best['sigma']:.4f}")
        print(f"  tau = {best['tau']:.3f} (neural avalanche prediction: 1.500)")
        print(f"  R^2 = {best['tau_R2']:.3f}")
        print(f"  f_manifest = {best['f_manifest']*100:.1f}%")
        print()

        if abs(best['tau'] - 1.5) < 0.2 and best['tau_R2'] > 0.85:
            print("[EMERGENT] The phase transition exhibits power-law avalanche")
            print("  statistics with tau ~ 1.5, matching the mean-field branching")
            print("  process / neural criticality universality class.")
            print("  This is a QUANTITATIVE prediction, not a postulate.")
        elif abs(best['tau'] - 1.5) < 0.5:
            print("[EMERGENT] Power-law avalanches detected but tau deviates from 1.5.")
            print(f"  tau = {best['tau']:.3f} suggests a different universality class.")
        else:
            print("[EMERGENT] Power-law statistics found but with tau far from 1.5.")
            print("  The biological parallel may be analogical, not structural.")
    else:
        # Check for bimodal (mostly size 1 + some very large)
        all_sizes = []
        for r in results:
            all_sizes.extend(r['avalanche_sizes'])

        if all_sizes:
            sizes_arr = np.array(all_sizes)
            frac_single = np.mean(sizes_arr == 1)
            frac_large = np.mean(sizes_arr > 100)

            if frac_single > 0.8 and frac_large > 0.01:
                print("[EMERGENT] BIMODAL distribution detected.")
                print(f"  {frac_single*100:.0f}% single-site, "
                      f"{frac_large*100:.1f}% system-spanning.")
                print("  Strong first-order character: no intermediate scale.")
            else:
                print("[EMERGENT] No clear power law found.")
                print("  The transition appears to be purely first-order")
                print("  with no critical fluctuations.")
        else:
            print("No avalanches detected at any sigma. Check grid size and ticks.")

    print()
    print("NOTE: The coupling term g_c * s * div(J) from the action principle")
    print("(CLAUDE.md section 13.2) is NOT implemented as a force in this")
    print("simulation. Results reflect passive threshold crossing, not active")
    print("state-flux feedback. Future work should implement the coupling and")
    print("compare avalanche statistics with and without it.")

    return results


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="FTD Avalanche Statistics")
    parser.add_argument("--grid", type=int, default=32, help="Grid size (default 32)")
    parser.add_argument("--warmup", type=int, default=200, help="Warmup ticks (default 200)")
    parser.add_argument("--measure", type=int, default=2000, help="Measurement ticks (default 2000)")
    parser.add_argument("--quick", action="store_true", help="Quick run (smaller grid, fewer ticks)")
    args = parser.parse_args()

    if args.quick:
        results = run_avalanche_scan(
            grid_size=24, warmup_ticks=100, measure_ticks=500,
            sigma_values=[0.065, 0.070, 0.075, 0.080, 0.085],
            report_interval=250,
        )
    else:
        results = run_avalanche_scan(
            grid_size=args.grid, warmup_ticks=args.warmup,
            measure_ticks=args.measure,
            report_interval=500,
        )
