"""
Zero-Point Fluctuation Equilibrium Test
========================================

Tests the fluctuation-dissipation relation (FDR) prediction:

    sigma_zpf^2 / DAMPING = K_B^2 / (2*pi)

When zero-point fluctuations are added to the wave equation at the
amplitude fixed by self-duality, the vacuum should reach a specific
equilibrium characterized by:

    - Manifest fraction: exp(-pi) ~ 4.32%  (Rayleigh threshold)
    - RMS flux per component: sigma_eq = K_B / sqrt(2*pi) ~ 0.204
    - Matter/antimatter symmetry: N(+1) ~ N(-1)

This is a FALSIFIABLE test of the partition function interpretation.
If the simulation gives a different manifest fraction, the interpretation
is wrong (or the FDR mapping is wrong).
"""

import numpy as np
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from ternary_matrix.config import PhysicsConfig
from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import master_equation, waves


def run_zpf_test(grid_size=32, num_ticks=500, report_interval=50):
    """
    Run the FDR equilibrium test.

    Creates a vacuum universe with ZPF enabled, evolves it, and
    measures whether equilibrium statistics match predictions.
    """
    # --- Config with ZPF enabled ---
    config = PhysicsConfig(
        GRID_SIZE=grid_size,
        ENABLE_ZPF=True,
    )

    # Compute predicted values
    sigma_zpf = waves.zpf_sigma(damping=config.DAMPING, kb=config.KB)
    sigma_eq = config.KB / np.sqrt(2 * np.pi)  # equilibrium RMS per component
    f_rayleigh = np.exp(-np.pi)                 # predicted manifest fraction
    beta_eq = np.pi                             # self-dual inverse temperature

    print("=" * 65)
    print("ZERO-POINT FLUCTUATION EQUILIBRIUM TEST")
    print("=" * 65)
    print()
    print(f"Grid:           {grid_size}^3 = {grid_size**3:,} sites")
    print(f"Ticks:          {num_ticks}")
    print(f"K_B:            {config.KB}")
    print(f"DAMPING:        {config.DAMPING}")
    print(f"sigma_zpf:      {sigma_zpf:.6f}  (noise injected per tick)")
    print(f"sigma_eq:       {sigma_eq:.6f}  (predicted equilibrium RMS)")
    print(f"beta_eq:        pi = {beta_eq:.6f}")
    print(f"f_manifest:     exp(-pi) = {f_rayleigh:.6f} = {f_rayleigh*100:.2f}%")
    print()

    # --- Create universe ---
    # Enable ZPF on the global singleton (import-time binding means
    # waves.py already has a reference to the original CONSTANTS object).
    import ternary_matrix.config as cfg
    original_zpf = cfg.CONSTANTS.ENABLE_ZPF
    cfg.CONSTANTS.ENABLE_ZPF = True

    try:
        universe = Universe(size=grid_size)
        n_sites = grid_size ** 3

        # Start from vacuum (all flux = 0, all states = 0)
        # ZPF noise will build up flux from nothing

        print("-" * 65)
        print(f"{'Tick':>6} | {'Manifest%':>10} | {'RMS |J|':>10} | "
              f"{'N(+1)':>7} | {'N(-1)':>7} | {'beta_eff':>10}")
        print("-" * 65)

        # Track time series for convergence analysis
        history = []

        for t in range(num_ticks):
            master_equation.tick(universe)

            if (t + 1) % report_interval == 0 or t == 0:
                n_manifest = universe.get_manifested_count()
                n_pos = universe.get_positive_count()
                n_neg = universe.get_negative_count()
                f_obs = n_manifest / n_sites

                # RMS flux magnitude (per component)
                flux_sq = np.mean(universe.flux ** 2)  # mean over all sites & components
                rms_flux = np.sqrt(flux_sq)

                # Effective beta = K_B^2 / (2 * sigma^2)
                # sigma^2 = mean of one component's variance
                sigma_obs = np.sqrt(np.mean(universe.flux ** 2, axis=(0, 1, 2)))
                sigma_avg = np.mean(sigma_obs)
                beta_obs = config.KB ** 2 / (2 * sigma_avg ** 2) if sigma_avg > 0 else float('inf')

                history.append({
                    'tick': t + 1,
                    'f_manifest': f_obs,
                    'rms_flux': rms_flux,
                    'n_pos': n_pos,
                    'n_neg': n_neg,
                    'sigma_avg': sigma_avg,
                    'beta_obs': beta_obs,
                })

                print(f"{t+1:6d} | {f_obs*100:9.4f}% | {rms_flux:10.6f} | "
                      f"{n_pos:7d} | {n_neg:7d} | {beta_obs:10.4f}")

        # --- Analysis ---
        print()
        print("=" * 65)
        print("EQUILIBRIUM ANALYSIS")
        print("=" * 65)

        # Use last 40% of ticks for equilibrium averages
        eq_start = len(history) // 2
        eq_data = history[eq_start:]

        if len(eq_data) < 2:
            print("Not enough data points for equilibrium analysis.")
            print("Try increasing num_ticks or decreasing report_interval.")
            return

        f_eq = np.mean([d['f_manifest'] for d in eq_data])
        f_std = np.std([d['f_manifest'] for d in eq_data])
        rms_eq = np.mean([d['rms_flux'] for d in eq_data])
        beta_eq_obs = np.mean([d['beta_obs'] for d in eq_data])
        sigma_eq_obs = np.mean([d['sigma_avg'] for d in eq_data])

        print()
        print(f"  Quantity          | Predicted      | Observed       | Match")
        print(f"  " + "-" * 60)
        print(f"  Manifest frac     | {f_rayleigh*100:12.4f}%  | "
              f"{f_eq*100:12.4f}%  | "
              f"{'PASS' if abs(f_eq - f_rayleigh) < 3*f_std + 0.01 else 'FAIL'}")
        print(f"  sigma (per comp)  | {sigma_eq:12.6f}   | "
              f"{sigma_eq_obs:12.6f}   | "
              f"{abs(sigma_eq_obs - sigma_eq)/sigma_eq*100:.1f}% off")
        print(f"  beta_effective    | {np.pi:12.6f}   | "
              f"{beta_eq_obs:12.6f}   | "
              f"{abs(beta_eq_obs - np.pi)/np.pi*100:.1f}% off")

        # Matter-antimatter symmetry
        n_pos_avg = np.mean([d['n_pos'] for d in eq_data])
        n_neg_avg = np.mean([d['n_neg'] for d in eq_data])
        asymmetry = (n_pos_avg - n_neg_avg) / max(n_pos_avg + n_neg_avg, 1)

        print(f"  N(+1)/N(-1)       | {'1.000':>12}    | "
              f"{n_pos_avg/max(n_neg_avg,1):12.4f}   | "
              f"asym={asymmetry:.4f}")

        print()

        # Verdict
        f_err_pct = abs(f_eq - f_rayleigh) / f_rayleigh * 100
        if f_err_pct < 20:
            print("[PASS] Manifest fraction within 20% of Rayleigh prediction.")
        elif f_err_pct < 50:
            print("[MARGINAL] Manifest fraction within 50% -- qualitatively right.")
        else:
            print(f"[FAIL] Manifest fraction off by {f_err_pct:.0f}%.")
            print("  The FDR at self-duality does not predict this system's equilibrium.")

        beta_err = abs(beta_eq_obs - np.pi) / np.pi * 100
        if beta_err < 10:
            print(f"[PASS] Effective beta within 10% of pi ({beta_err:.1f}% off).")
        else:
            print(f"[INFO] Effective beta = {beta_eq_obs:.4f} ({beta_err:.1f}% off from pi).")
            print("  The vacuum is NOT at the self-dual point.")

        return {
            'f_manifest': f_eq,
            'f_predicted': f_rayleigh,
            'sigma_obs': sigma_eq_obs,
            'sigma_pred': sigma_eq,
            'beta_obs': beta_eq_obs,
            'history': history,
        }

    finally:
        # Restore original setting
        cfg.CONSTANTS.ENABLE_ZPF = original_zpf


if __name__ == "__main__":
    # Small grid for quick test
    run_zpf_test(grid_size=32, num_ticks=500, report_interval=25)
