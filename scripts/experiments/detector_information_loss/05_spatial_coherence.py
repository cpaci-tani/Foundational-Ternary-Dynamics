"""
05_spatial_coherence.py — The mutual coherence function

WHY THIS MATTERS:
Gamma(r1, r2) = psi*(r1) . psi(r2) encodes how the field at one point
predicts the field at another, including phase across arbitrary distances.
The detector's intensity autocorrelation g^2 loses all phase-sensitive
correlations, collapsing the oscillating coherence profile to a smooth
envelope.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, sample_detector_clicks,
    amplitude_field, shannon_entropy, mutual_information,
    setup_style, make_figure, save_json, detector_dots_image,
    DEFAULTS,
)


def compute_coherence_profile(psi_re, psi_im, max_dx, midline_band=20):
    """Compute |Gamma(dx, 0)| along the midline.

    Gamma(dx) = <psi*(x,y) . psi(x+dx,y)> averaged over x,y in the
    midline band.  Returns (displacements, |Gamma|, Re(Gamma), Im(Gamma)).
    """
    H, W = psi_re.shape
    cy = H // 2
    y_lo = max(cy - midline_band, 0)
    y_hi = min(cy + midline_band, H)

    displacements = np.arange(0, max_dx + 1)
    gamma_abs = np.zeros(len(displacements))
    gamma_re = np.zeros(len(displacements))
    gamma_im = np.zeros(len(displacements))

    strip_re = psi_re[y_lo:y_hi, :]
    strip_im = psi_im[y_lo:y_hi, :]

    for i, dx in enumerate(displacements):
        if dx >= W:
            break
        # psi*(x,y) . psi(x+dx, y)
        conj_re = strip_re[:, :W - dx]
        conj_im = -strip_im[:, :W - dx]
        fwd_re = strip_re[:, dx:]
        fwd_im = strip_im[:, dx:]

        prod_re = conj_re * fwd_re - conj_im * fwd_im
        prod_im = conj_re * fwd_im + conj_im * fwd_re

        mean_re = prod_re.mean()
        mean_im = prod_im.mean()

        gamma_re[i] = mean_re
        gamma_im[i] = mean_im
        gamma_abs[i] = np.sqrt(mean_re ** 2 + mean_im ** 2)

    return displacements, gamma_abs, gamma_re, gamma_im


def compute_g2_profile(intensity, max_dx, midline_band=20):
    """Compute g^2(dx, 0) = <I(x)*I(x+dx)> / <I>^2 along the midline."""
    H, W = intensity.shape
    cy = H // 2
    y_lo = max(cy - midline_band, 0)
    y_hi = min(cy + midline_band, H)

    strip = intensity[y_lo:y_hi, :]
    mean_I = strip.mean()
    if mean_I <= 0:
        return np.arange(0, max_dx + 1), np.ones(max_dx + 1)

    displacements = np.arange(0, max_dx + 1)
    g2 = np.zeros(len(displacements))

    for i, dx in enumerate(displacements):
        if dx >= W:
            break
        product = strip[:, :W - dx] * strip[:, dx:]
        g2[i] = product.mean() / (mean_I ** 2)

    return displacements, g2


def compute_detector_pair_correlation(clicks, W, H, max_dx, midline_band=20):
    """Compute pair correlation from detector clicks near the midline.

    Bins click pairs by horizontal separation.
    """
    cy = H // 2
    # Select clicks near midline
    mask = np.abs(clicks[:, 1] - cy) < midline_band
    mid_clicks = clicks[mask]

    if len(mid_clicks) < 10:
        return np.arange(0, max_dx + 1), np.zeros(max_dx + 1)

    displacements = np.arange(0, max_dx + 1)
    pair_counts = np.zeros(len(displacements))

    xs = mid_clicks[:, 0].astype(int)

    # Build a histogram of x positions
    x_hist = np.zeros(W)
    np.add.at(x_hist, np.clip(xs, 0, W - 1), 1.0)

    # Autocorrelation of the x histogram
    for i, dx in enumerate(displacements):
        if dx >= W:
            break
        pair_counts[i] = np.sum(x_hist[:W - dx] * x_hist[dx:])

    # Normalize
    total = pair_counts[0] if pair_counts[0] > 0 else 1.0
    pair_counts /= total

    return displacements, pair_counts


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("05  SPATIAL COHERENCE — The mutual coherence function")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  Gamma(r1,r2) = psi*(r1).psi(r2) encodes how the field at one")
    print("  point predicts the field at another, including phase across")
    print("  arbitrary distances.  The detector's intensity autocorrelation")
    print("  loses all phase-sensitive correlations.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']
    max_dx = 200

    # 1. Compute full field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    intensity = born_rule(psi_re, psi_im)
    clicks = sample_detector_clicks(intensity, d['N_clicks'])

    # 2. Coherence profiles
    print("  Computing coherence Gamma(dx,0) ...")
    dx_arr, gamma_abs, gamma_re, gamma_im = compute_coherence_profile(
        psi_re, psi_im, max_dx)

    print("  Computing g^2(dx,0) ...")
    _, g2 = compute_g2_profile(intensity, max_dx)

    print("  Computing detector pair correlation ...")
    _, det_corr = compute_detector_pair_correlation(clicks, W, H, max_dx)

    # 3. Mutual information at several separations
    print("  Computing mutual information at various separations ...")
    separations_for_mi = [1, 4, 8, 16, 32, 64, 128]
    mi_full = []
    mi_born = []

    midline_band = 20
    cy = H // 2
    y_lo = max(cy - midline_band, 0)
    y_hi = min(cy + midline_band, H)

    for sep in separations_for_mi:
        if sep >= W:
            mi_full.append(0.0)
            mi_born.append(0.0)
            continue

        # Full field: use Re(psi) as a proxy for the field value at two points
        strip_left = psi_re[y_lo:y_hi, :W - sep]
        strip_right = psi_re[y_lo:y_hi, sep:]
        mi_f = mutual_information(strip_left, strip_right, bins=64)
        mi_full.append(mi_f)

        # |psi|^2 field
        int_left = intensity[y_lo:y_hi, :W - sep]
        int_right = intensity[y_lo:y_hi, sep:]
        mi_b = mutual_information(int_left, int_right, bins=64)
        mi_born.append(mi_b)

    # ------------------------------------------------------------------
    # Normalize coherence for display
    gamma_norm = gamma_abs / max(gamma_abs[0], 1e-12)

    # ------------------------------------------------------------------
    # Print results
    print()
    print(f"  Gamma(0) = {gamma_abs[0]:.6f}  (self-correlation)")
    print(f"  Gamma has {np.sum(np.diff(np.sign(gamma_re)) != 0)} sign changes (oscillatory)")
    print(f"  g^2(0)   = {g2[0]:.4f}")
    print(f"  g^2 has {np.sum(np.diff(np.sign(g2 - 1.0)) != 0)} crossings of g^2=1")
    print()
    print("  Mutual information (bits) at separation d:")
    print(f"  {'d':>6s}  {'I(full)':>10s}  {'I(|psi|^2)':>12s}  {'ratio':>8s}")
    for sep, mf, mb in zip(separations_for_mi, mi_full, mi_born):
        ratio = mf / max(mb, 1e-12) if mb > 0.001 else float('inf')
        print(f"  {sep:>6d}  {mf:>10.4f}  {mb:>12.4f}  {ratio:>8.2f}x")
    print()

    # ------------------------------------------------------------------
    # Figure: 4 panels
    plt = setup_style()

    fig, axes = plt.subplots(1, 4, figsize=(20, 4.5))
    fig.suptitle("Spatial Coherence: What the Detector Cannot See",
                 color='#a0b8d8', fontsize=12, fontweight='bold', y=0.98)

    # (a) |Gamma(dx,0)| coherence profile
    ax = axes[0]
    ax.plot(dx_arr, gamma_norm, color='#66ccff', linewidth=1.0, label='|Gamma|')
    ax.fill_between(dx_arr, 0, gamma_norm, alpha=0.15, color='#66ccff')
    ax.set_xlabel('Displacement dx (pixels)', fontsize=8, color='#c8d8e8')
    ax.set_ylabel('|Gamma(dx,0)| / |Gamma(0)|', fontsize=8, color='#c8d8e8')
    ax.set_title('(a) Coherence |Gamma(dx,0)|', color='#c8d8e8', fontsize=9)
    ax.set_xlim(0, max_dx)
    ax.set_ylim(0, None)
    ax.grid(True, alpha=0.15, color='#1a1a2e')
    # Show Re(Gamma) to emphasize oscillation
    gamma_re_norm = gamma_re / max(abs(gamma_re).max(), 1e-12)
    ax.plot(dx_arr, gamma_re_norm, color='#ff8866', linewidth=0.6,
            alpha=0.7, label='Re(Gamma)')
    ax.legend(fontsize=7, loc='upper right',
              facecolor='#0d0d15', edgecolor='#667788')

    # (b) g^2 profile (envelope only)
    ax = axes[1]
    ax.plot(dx_arr, g2, color='#ffaa44', linewidth=1.0)
    ax.axhline(1.0, color='#667788', linewidth=0.5, linestyle='--', alpha=0.6)
    ax.fill_between(dx_arr, 1.0, g2, alpha=0.15, color='#ffaa44')
    ax.set_xlabel('Displacement dx (pixels)', fontsize=8, color='#c8d8e8')
    ax.set_ylabel('g^2(dx,0)', fontsize=8, color='#c8d8e8')
    ax.set_title('(b) Intensity autocorrelation g^2', color='#c8d8e8', fontsize=9)
    ax.set_xlim(0, max_dx)
    ax.grid(True, alpha=0.15, color='#1a1a2e')

    # (c) Detector pair correlation (noisy)
    ax = axes[2]
    ax.plot(dx_arr, det_corr, color='#88dd88', linewidth=0.8, alpha=0.8)
    ax.fill_between(dx_arr, 0, det_corr, alpha=0.1, color='#88dd88')
    ax.set_xlabel('Displacement dx (pixels)', fontsize=8, color='#c8d8e8')
    ax.set_ylabel('Pair correlation (normalized)', fontsize=8, color='#c8d8e8')
    ax.set_title(f'(c) Detector pair corr. (N={d["N_clicks"]:,})',
                 color='#c8d8e8', fontsize=9)
    ax.set_xlim(0, max_dx)
    ax.grid(True, alpha=0.15, color='#1a1a2e')

    # (d) MI vs separation: full field vs |psi|^2
    ax = axes[3]
    ax.plot(separations_for_mi, mi_full, 'o-', color='#66ccff',
            linewidth=1.2, markersize=4, label='Full field Re(psi)')
    ax.plot(separations_for_mi, mi_born, 's-', color='#ffaa44',
            linewidth=1.2, markersize=4, label='|psi|^2')
    ax.set_xlabel('Separation d (pixels)', fontsize=8, color='#c8d8e8')
    ax.set_ylabel('Mutual information (bits)', fontsize=8, color='#c8d8e8')
    ax.set_title('(d) MI vs separation', color='#c8d8e8', fontsize=9)
    ax.set_xscale('log')
    ax.legend(fontsize=7, loc='upper right',
              facecolor='#0d0d15', edgecolor='#667788')
    ax.grid(True, alpha=0.15, color='#1a1a2e')

    # Metrics text
    avg_mi_ratio = np.mean([
        mf / max(mb, 1e-12) for mf, mb in zip(mi_full, mi_born) if mb > 0.001
    ]) if any(mb > 0.001 for mb in mi_born) else float('inf')
    metrics = (
        f"Gamma oscillations: {np.sum(np.diff(np.sign(gamma_re)) != 0)} sign changes  |  "
        f"g^2 is smooth envelope only  |  "
        f"MI(full)/MI(born) avg ratio: {avg_mi_ratio:.1f}x"
    )
    fig.text(0.5, 0.01, metrics, ha='center', va='bottom',
             color='#667788', fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                       edgecolor='#1a1a2e'))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])
    from experiments.detector_information_loss.field_engine import OUTPUT_DIR
    outpath = OUTPUT_DIR / "05_spatial_coherence.png"
    fig.savefig(outpath, dpi=200, facecolor='#0a0a0f',
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '05_spatial_coherence.py',
        'description': 'Mutual coherence Gamma vs intensity autocorrelation g^2',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'gamma_0': float(gamma_abs[0]),
            'gamma_sign_changes': int(np.sum(np.diff(np.sign(gamma_re)) != 0)),
            'g2_0': float(g2[0]),
            'max_displacement_px': int(max_dx),
            'mutual_information': {
                'separations': separations_for_mi,
                'MI_full_field_bits': [float(x) for x in mi_full],
                'MI_born_field_bits': [float(x) for x in mi_born],
            },
            'avg_MI_ratio_full_over_born': float(avg_mi_ratio),
        },
    }
    save_json('05_spatial_coherence', summary)

    print()
    print(f"  RESULT: Gamma(dx) oscillates with {np.sum(np.diff(np.sign(gamma_re)) != 0)} "
          f"sign changes,")
    print(f"  encoding rich phase-sensitive structure.")
    print(f"  g^2(dx) sees only the smooth envelope — all oscillatory")
    print(f"  phase correlations are destroyed.")
    print(f"  Full field carries {avg_mi_ratio:.1f}x more mutual information")
    print(f"  between separated points than |psi|^2.")
    print()


if __name__ == '__main__':
    main()
