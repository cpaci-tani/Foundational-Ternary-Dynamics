"""
15_parameter_sensitivity.py — The detector's sluggish response to change

WHY THIS MATTERS:
3% separation change.  Full psi changes instantly.  Detector takes thousands
of clicks to notice.  The ratio is "the cost of going boolean."

We find N* = the minimum number of detector clicks needed to detect a 3%
parameter change at 3-sigma confidence via chi-squared test.  The full field
detects this change with zero additional samples.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import *


def bin_clicks_2d(clicks, W, H, n_bins=32):
    """Bin click positions into a 2D histogram."""
    if len(clicks) == 0:
        return np.zeros((n_bins, n_bins))
    hist, _, _ = np.histogram2d(
        clicks[:, 1], clicks[:, 0],
        bins=n_bins, range=[[0, H], [0, W]],
    )
    return hist


def chi_squared_stat(hist1, hist2):
    """Chi-squared statistic between two histograms.

    Uses the standard formula with pooled expectation: treat (hist1 + hist2)/2
    as expected after normalizing both to the same total count.
    """
    t1 = hist1.sum()
    t2 = hist2.sum()
    if t1 == 0 or t2 == 0:
        return 0.0

    h1 = hist1.ravel()
    h2 = hist2.ravel() * (t1 / t2)  # scale hist2 to match hist1 total
    expected = (h1 + h2) / 2.0

    mask = expected > 0
    if mask.sum() == 0:
        return 0.0
    chi2 = np.sum((h1[mask] - expected[mask]) ** 2 / expected[mask])
    return float(chi2)


def chi2_critical_3sigma(dof):
    """Compute chi-squared critical value at p=0.003 (3-sigma).

    Tries scipy first; falls back to normal approximation for large dof.
    """
    try:
        from scipy.stats import chi2
        return float(chi2.ppf(0.997, dof))
    except ImportError:
        # Normal approximation: chi2 ~ Normal(dof, sqrt(2*dof)) for large dof
        return dof + 3.0 * np.sqrt(2.0 * dof)


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("15  PARAMETER SENSITIVITY — The detector's sluggish response")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  3% separation change.  Full psi changes instantly.  Detector takes")
    print("  thousands of clicks to notice.  The ratio is 'the cost of going")
    print("  boolean.'")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    sep_1 = d['separation']       # d  = 80.0
    sep_2 = sep_1 * 1.03          # d' = 82.4  (3% shift)

    # 1. Compute fields at both separations
    print(f"  Computing field at d = {sep_1:.1f} ...")
    psi_re1, psi_im1 = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=sep_1,
        phase_offset=d['phase_offset'], t=d['t'],
    )

    print(f"  Computing field at d' = {sep_2:.1f} (3% shift) ...")
    psi_re2, psi_im2 = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=sep_2,
        phase_offset=d['phase_offset'], t=d['t'],
    )

    # 2. Field difference: L2 norm of complex difference
    diff_re = psi_re1 - psi_re2
    diff_im = psi_im1 - psi_im2
    diff_mag = np.sqrt(diff_re ** 2 + diff_im ** 2)
    field_L2 = float(np.sqrt(np.sum(diff_re ** 2 + diff_im ** 2)))
    field_L2_per_pixel = float(np.mean(diff_mag))

    print(f"  Field L2 distance ||psi(d) - psi(d')||  = {field_L2:.4f}")
    print(f"  Mean per-pixel |delta psi|              = {field_L2_per_pixel:.6f}")
    print()

    # 3. Born-rule distributions
    born1 = born_rule(psi_re1, psi_im1)
    born2 = born_rule(psi_re2, psi_im2)

    # 4. Chi-squared sweep over N_clicks
    N_values = [100, 500, 1000, 5000, 10_000, 25_000]
    n_bins = 16
    dof = n_bins * n_bins - 1

    # 3-sigma threshold at p=0.003
    chi2_3sigma = chi2_critical_3sigma(dof)

    print(f"  Chi-squared test: {n_bins}x{n_bins} bins, dof={dof}")
    print(f"  3-sigma threshold: chi2 > {chi2_3sigma:.1f}")
    print()

    rng = np.random.default_rng(42)
    chi2_values = []
    n_star = None

    # Run multiple trials per N and take the median for robustness
    n_trials = 7
    print(f"  {'N_clicks':>10s}  {'chi2 (median)':>14s}  {'> 3sigma?':>10s}")
    print(f"  {'-'*10}  {'-'*14}  {'-'*10}")

    for N in N_values:
        trial_chi2 = []
        for trial in range(n_trials):
            clicks1 = sample_detector_clicks(born1, N, rng=rng)
            clicks2 = sample_detector_clicks(born2, N, rng=rng)
            h1 = bin_clicks_2d(clicks1, W, H, n_bins)
            h2 = bin_clicks_2d(clicks2, W, H, n_bins)
            trial_chi2.append(chi_squared_stat(h1, h2))
        median_chi2 = float(np.median(trial_chi2))
        chi2_values.append(median_chi2)
        detected = median_chi2 > chi2_3sigma
        marker = "YES" if detected else "no"
        print(f"  {N:>10,d}  {median_chi2:>14.1f}  {marker:>10s}")
        if detected and n_star is None:
            n_star = N

    print()
    if n_star is not None:
        print(f"  >>> The full field detects a 3% change instantly.")
        print(f"  >>> The boolean detector needs N* = {n_star:,} clicks. <<<")
    else:
        print(f"  >>> The boolean detector could not detect the 3% change even at")
        print(f"      N = {N_values[-1]:,} clicks.  N* > {N_values[-1]:,}. <<<")
        n_star = -1  # sentinel

    print()

    # ------------------------------------------------------------------
    # Figure: 4 panels
    plt = setup_style()

    fig, axes = plt.subplots(2, 2, figsize=(11, 10))
    fig.suptitle("Parameter Sensitivity: The Cost of Going Boolean",
                 color=ACCENT_COLOR, fontsize=13, fontweight='bold', y=0.98)

    # (a) |psi(d) - psi(d')| difference map — use actual range for contrast
    ax = axes[0, 0]
    im_a = ax.imshow(diff_mag, origin='lower', cmap='magma', aspect='equal',
                     vmin=0, vmax=diff_mag.max())
    ax.set_title("(a) |psi(d) - psi(d')| — field detects change instantly",
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb_a = fig.colorbar(im_a, ax=ax, fraction=0.046, pad=0.04)
    cb_a.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (b) Click histogram difference at small N
    N_demo_lo = 1000
    rng_demo = np.random.default_rng(123)
    clicks_lo_1 = sample_detector_clicks(born1, N_demo_lo, rng=rng_demo)
    clicks_lo_2 = sample_detector_clicks(born2, N_demo_lo, rng=rng_demo)
    h_lo_1 = bin_clicks_2d(clicks_lo_1, W, H, n_bins)
    h_lo_2 = bin_clicks_2d(clicks_lo_2, W, H, n_bins)
    diff_lo = h_lo_1 - h_lo_2

    ax = axes[0, 1]
    vmax_lo = max(np.abs(diff_lo).max(), 1)
    im_b = ax.imshow(diff_lo, origin='lower', cmap='RdBu_r', aspect='equal',
                     vmin=-vmax_lo, vmax=vmax_lo)
    ax.set_title(f'(b) Click histogram diff, N={N_demo_lo:,} (noisy)',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb_b = fig.colorbar(im_b, ax=ax, fraction=0.046, pad=0.04)
    cb_b.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (c) Click histogram difference at large N
    N_demo_hi = 50_000
    rng_demo2 = np.random.default_rng(456)
    clicks_hi_1 = sample_detector_clicks(born1, N_demo_hi, rng=rng_demo2)
    clicks_hi_2 = sample_detector_clicks(born2, N_demo_hi, rng=rng_demo2)
    h_hi_1 = bin_clicks_2d(clicks_hi_1, W, H, n_bins)
    h_hi_2 = bin_clicks_2d(clicks_hi_2, W, H, n_bins)
    diff_hi = h_hi_1 - h_hi_2

    ax = axes[1, 0]
    vmax_hi = max(np.abs(diff_hi).max(), 1)
    im_c = ax.imshow(diff_hi, origin='lower', cmap='RdBu_r', aspect='equal',
                     vmin=-vmax_hi, vmax=vmax_hi)
    ax.set_title(f'(c) Click histogram diff, N={N_demo_hi:,} (clearer)',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb_c = fig.colorbar(im_c, ax=ax, fraction=0.046, pad=0.04)
    cb_c.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (d) Chi-squared vs N curve with 3-sigma line
    ax = axes[1, 1]
    ax.semilogx(N_values, chi2_values, 'o-', color='#66bbff', linewidth=1.5,
                markersize=6, label='chi2 (median)')
    ax.axhline(chi2_3sigma, color='#ff6666', linestyle='--', linewidth=1.2,
               label=f'3-sigma = {chi2_3sigma:.0f}')
    ax.set_xlabel('N (detector clicks)', color=TEXT_COLOR, fontsize=9)
    ax.set_ylabel('Chi-squared statistic', color=TEXT_COLOR, fontsize=9)
    ax.set_title('(d) Detection threshold vs sample size',
                 color=TEXT_COLOR, fontsize=9)
    ax.legend(fontsize=8, facecolor=BG_COLOR, edgecolor=GRID_COLOR,
              labelcolor=TEXT_COLOR)
    ax.grid(True, alpha=0.3)

    # Annotate N*
    if n_star > 0:
        ax.annotate(
            f'N* = {n_star:,}',
            xy=(n_star, chi2_3sigma), xytext=(n_star * 2, chi2_3sigma * 1.5),
            arrowprops=dict(arrowstyle='->', color='#ffcc66', lw=1.5),
            fontsize=10, color='#ffcc66',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#1a1a2e',
                      edgecolor=GRID_COLOR),
        )
    else:
        ax.annotate(
            f'N* > {N_values[-1]:,}',
            xy=(0.5, 0.85), xycoords='axes fraction',
            ha='center', va='top', fontsize=10, color='#ffcc66',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#1a1a2e',
                      edgecolor=GRID_COLOR),
        )

    # Metrics text
    n_star_str = f"{n_star:,}" if n_star > 0 else f">{N_values[-1]:,}"
    metrics_text = (
        f"separation shift: {sep_1:.1f} -> {sep_2:.1f} (3%)  |  "
        f"field L2 = {field_L2:.2f}  |  "
        f"N* = {n_star_str} clicks  |  "
        f"chi2 threshold (3sigma) = {chi2_3sigma:.0f}"
    )
    fig.text(0.5, 0.01, metrics_text, ha='center', va='bottom',
             color=SUBTLE_COLOR, fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                       edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0.04, 1, 0.95])
    outpath = OUTPUT_DIR / "15_parameter_sensitivity.png"
    fig.savefig(outpath, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '15_parameter_sensitivity.py',
        'description': 'Detector sluggishness: N* clicks needed to detect 3% parameter shift',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'separation_original': float(sep_1),
            'separation_shifted': float(sep_2),
            'shift_percent': 3.0,
            'field_L2_distance': field_L2,
            'field_L2_per_pixel': field_L2_per_pixel,
            'N_star': int(n_star) if n_star > 0 else None,
            'chi2_3sigma_threshold': float(chi2_3sigma),
            'n_bins': n_bins,
            'dof': dof,
            'N_values': N_values,
            'chi2_values': chi2_values,
            'n_trials_per_N': n_trials,
        },
    }
    save_json('15_parameter_sensitivity', summary)

    print()
    print(f"  RESULT: The full field detects a 3% change instantly (L2 = {field_L2:.2f}).")
    if n_star > 0:
        print(f"  The boolean detector needs N* = {n_star:,} clicks.")
    else:
        print(f"  The boolean detector could not detect the change even at N = {N_values[-1]:,}.")
    print()


if __name__ == '__main__':
    main()
