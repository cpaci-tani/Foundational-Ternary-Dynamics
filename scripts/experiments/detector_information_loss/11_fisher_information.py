"""
11_fisher_information.py — Statistical efficiency for parameter estimation

WHY THIS MATTERS:
Fisher information quantifies the best possible precision for estimating
source parameters. The Cramer-Rao bound states var(d_hat) >= 1/F(d).

The full complex field gives orders of magnitude better estimates than
detector clicks. This script computes the "click cost of one field snapshot":
how many detector clicks N* are needed to match the Fisher information
available in a single continuous |psi|^2 measurement.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, fisher_information,
    amplitude_field, phase_field,
    setup_style, save_json, DEFAULTS, OUTPUT_DIR,
)


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("11  FISHER INFORMATION — Statistical efficiency for parameter estimation")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  Fisher information quantifies best possible precision for estimating")
    print("  source parameters. Full field gives orders of magnitude better")
    print("  estimates than detector clicks.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    # Parameter: slit separation d_sep
    d_sep = d['separation']  # 80
    delta = 0.5              # small perturbation

    # ------------------------------------------------------------------
    # 1. Compute fields at d and d + delta
    # ------------------------------------------------------------------
    print("  Computing field at d = {:.1f} ...".format(d_sep))
    psi_re_0, psi_im_0 = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d_sep,
        phase_offset=d['phase_offset'], t=d['t'],
    )
    born_0 = born_rule(psi_re_0, psi_im_0)

    print("  Computing field at d = {:.1f} ...".format(d_sep + delta))
    psi_re_1, psi_im_1 = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d_sep + delta,
        phase_offset=d['phase_offset'], t=d['t'],
    )
    born_1 = born_rule(psi_re_1, psi_im_1)

    # ------------------------------------------------------------------
    # 2. Numerical derivatives
    # ------------------------------------------------------------------
    # d|psi|^2 / d(sep)
    d_born = (born_1 - born_0) / delta

    # d(Re psi) / d(sep)  and  d(Im psi) / d(sep)
    d_re = (psi_re_1 - psi_re_0) / delta
    d_im = (psi_im_1 - psi_im_0) / delta

    # ------------------------------------------------------------------
    # 3. Fisher information for |psi|^2 field
    # ------------------------------------------------------------------
    # F_I = sum (dI/dd)^2 / I  where I > 0
    F_born = fisher_information(born_0, d_born)

    # ------------------------------------------------------------------
    # 4. Fisher information for complex field
    # ------------------------------------------------------------------
    # Use both Re and Im channels: F_psi = F(|Re|, dRe/dd) + F(|Im|, dIm/dd)
    # More precisely, for the full complex field the Fisher info is:
    #   F = 4 * sum [ (d|psi|/dd)^2 ]  (quantum Fisher information)
    # But classically, treating Re and Im as independent channels:
    eps = 1e-12
    re_sq = psi_re_0 ** 2
    im_sq = psi_im_0 ** 2

    # Fisher from Re channel: sum (d Re / dd)^2 / (Re^2 + eps)
    mask_re = re_sq > eps
    F_re = np.sum(d_re.ravel()[mask_re.ravel()] ** 2 / re_sq.ravel()[mask_re.ravel()])

    # Fisher from Im channel: sum (d Im / dd)^2 / (Im^2 + eps)
    mask_im = im_sq > eps
    F_im = np.sum(d_im.ravel()[mask_im.ravel()] ** 2 / im_sq.ravel()[mask_im.ravel()])

    F_psi = F_re + F_im

    # ------------------------------------------------------------------
    # 5. Fisher information for N clicks
    # ------------------------------------------------------------------
    # For N i.i.d. samples from normalized |psi|^2:
    # F_N = N * F_born / (sum |psi|^2)^2   (Fisher info of the multinomial)
    # Actually: F_N = N * sum [ (dp_i/dd)^2 / p_i ] where p_i = I_i / sum(I)
    total_I = born_0.sum()
    if total_I > 0:
        p = born_0 / total_I
        dp = d_born / total_I
        mask_p = p.ravel() > 1e-15
        F_per_click = np.sum(dp.ravel()[mask_p] ** 2 / p.ravel()[mask_p])
    else:
        F_per_click = 0.0

    N_clicks_default = d['N_clicks']
    F_N = N_clicks_default * F_per_click

    # ------------------------------------------------------------------
    # 6. Cramer-Rao bounds
    # ------------------------------------------------------------------
    CR_born = 1.0 / F_born if F_born > 0 else float('inf')
    CR_psi = 1.0 / F_psi if F_psi > 0 else float('inf')
    CR_N = 1.0 / F_N if F_N > 0 else float('inf')

    # ------------------------------------------------------------------
    # 7. N* = clicks needed so F_N >= F_born (the full |psi|^2 field)
    # ------------------------------------------------------------------
    if F_per_click > 0:
        N_star_born = F_born / F_per_click
    else:
        N_star_born = float('inf')

    # N** = clicks needed to match F_psi (complex field)
    if F_per_click > 0:
        N_star_psi = F_psi / F_per_click
    else:
        N_star_psi = float('inf')

    # ------------------------------------------------------------------
    # Print results
    # ------------------------------------------------------------------
    print(f"  Fisher information for |psi|^2 field:   F_born = {F_born:.4e}")
    print(f"  Fisher information for complex psi:     F_psi  = {F_psi:.4e}")
    print(f"  Fisher information per click:           F_1    = {F_per_click:.4e}")
    print(f"  Fisher information for {N_clicks_default:,} clicks: F_N    = {F_N:.4e}")
    print()
    print(f"  Cramer-Rao bounds (variance lower bound for d_hat):")
    print(f"    Full psi:       sigma^2 >= {CR_psi:.4e}")
    print(f"    |psi|^2 field:  sigma^2 >= {CR_born:.4e}")
    print(f"    {N_clicks_default:,} clicks:  sigma^2 >= {CR_N:.4e}")
    print()
    print(f"  Click cost of one |psi|^2 snapshot:     N* = {N_star_born:,.0f} clicks")
    print(f"  Click cost of one complex-psi snapshot:  N** = {N_star_psi:,.0f} clicks")
    print()

    # ------------------------------------------------------------------
    # Figure
    # ------------------------------------------------------------------
    plt = setup_style()
    import matplotlib.gridspec as gridspec

    fig = plt.figure(figsize=(16, 9))
    fig.patch.set_facecolor('#0a0a0f')
    gs = gridspec.GridSpec(2, 2, figure=fig, hspace=0.35, wspace=0.3,
                           left=0.08, right=0.95, top=0.92, bottom=0.08)

    fig.suptitle("Fisher Information: Statistical Efficiency of Field vs. Clicks",
                 color='#a0b8d8', fontsize=13, fontweight='bold')

    # (a) |d psi / d(sep)| sensitivity map (complex magnitude)
    sensitivity_psi = np.sqrt(d_re ** 2 + d_im ** 2)
    ax_a = fig.add_subplot(gs[0, 0])
    vmax_a = np.percentile(sensitivity_psi[sensitivity_psi > 0], 99)
    im_a = ax_a.imshow(sensitivity_psi, origin='lower', cmap='magma',
                       aspect='equal', vmax=vmax_a)
    ax_a.set_title('(a) |d psi / d(sep)|  — complex sensitivity',
                   color='#c8d8e8', fontsize=9)
    ax_a.set_xticks([])
    ax_a.set_yticks([])
    cb_a = fig.colorbar(im_a, ax=ax_a, fraction=0.046, pad=0.04)
    cb_a.ax.yaxis.set_tick_params(color='#667788')

    # (b) d|psi|^2 / d(sep) sensitivity map
    ax_b = fig.add_subplot(gs[0, 1])
    vmax_b = np.percentile(np.abs(d_born[d_born != 0]), 99)
    im_b = ax_b.imshow(d_born, origin='lower', cmap='RdBu_r',
                       aspect='equal', vmin=-vmax_b, vmax=vmax_b)
    ax_b.set_title(r'(b) d|$\psi$|$^2$ / d(sep)  — intensity sensitivity',
                   color='#c8d8e8', fontsize=9)
    ax_b.set_xticks([])
    ax_b.set_yticks([])
    cb_b = fig.colorbar(im_b, ax=ax_b, fraction=0.046, pad=0.04)
    cb_b.ax.yaxis.set_tick_params(color='#667788')

    # (c) Cramer-Rao bound vs N_clicks, with field bound as horizontal line
    ax_c = fig.add_subplot(gs[1, 0:2])
    ax_c.set_facecolor('#0a0a0f')

    N_range = np.logspace(1, 8, 200)
    CR_vs_N = np.where(N_range * F_per_click > 0,
                       1.0 / (N_range * F_per_click),
                       float('inf'))

    ax_c.loglog(N_range, CR_vs_N, color='#cc8844', linewidth=2.0,
                label=r'CR bound ($N$ clicks)')

    # Horizontal lines for field bounds
    ax_c.axhline(y=CR_born, color='#44aa88', linewidth=1.5, linestyle='--',
                 label=r'CR bound (full $|\psi|^2$ field)')
    ax_c.axhline(y=CR_psi, color='#6699cc', linewidth=1.5, linestyle=':',
                 label=r'CR bound (complex $\psi$ field)')

    # Mark N*
    if np.isfinite(N_star_born):
        ax_c.axvline(x=N_star_born, color='#44aa88', linewidth=0.8,
                     linestyle='-.', alpha=0.7)
        ax_c.annotate(f'N* = {N_star_born:,.0f}',
                      xy=(N_star_born, CR_born),
                      xytext=(N_star_born * 3, CR_born * 10),
                      arrowprops=dict(arrowstyle='->', color='#44aa88', lw=1.2),
                      color='#44aa88', fontsize=10, fontweight='bold',
                      bbox=dict(boxstyle='round,pad=0.3', facecolor='#0d0d15',
                               edgecolor='#44aa88', alpha=0.9))

    # Mark N**
    if np.isfinite(N_star_psi) and N_star_psi < 1e9:
        ax_c.axvline(x=N_star_psi, color='#6699cc', linewidth=0.8,
                     linestyle='-.', alpha=0.7)
        ax_c.annotate(f'N** = {N_star_psi:,.0f}',
                      xy=(N_star_psi, CR_psi),
                      xytext=(N_star_psi * 3, CR_psi * 10),
                      arrowprops=dict(arrowstyle='->', color='#6699cc', lw=1.2),
                      color='#6699cc', fontsize=10, fontweight='bold',
                      bbox=dict(boxstyle='round,pad=0.3', facecolor='#0d0d15',
                               edgecolor='#6699cc', alpha=0.9))

    ax_c.set_xlabel('Number of detector clicks  N', color='#c8d8e8', fontsize=10)
    ax_c.set_ylabel(r'Cramer-Rao bound  $\sigma^2 \geq 1/F$',
                    color='#c8d8e8', fontsize=10)
    ax_c.set_title('(c) Estimation precision: clicks vs. field measurements',
                   color='#c8d8e8', fontsize=10)
    ax_c.legend(loc='upper right', fontsize=8, facecolor='#0d0d15',
                edgecolor='#667788', labelcolor='#c8d8e8')
    ax_c.grid(True, alpha=0.2, color='#1a1a2e')
    ax_c.spines['top'].set_visible(False)
    ax_c.spines['right'].set_visible(False)
    ax_c.spines['left'].set_color('#667788')
    ax_c.spines['bottom'].set_color('#667788')

    # Bottom annotation
    metrics_text = (
        f"F(psi) = {F_psi:.2e}  |  F(|psi|^2) = {F_born:.2e}  |  "
        f"F/click = {F_per_click:.2e}  |  "
        f"N* = {N_star_born:,.0f}  |  N** = {N_star_psi:,.0f}"
    )
    fig.text(0.5, 0.01, metrics_text, ha='center', va='bottom',
             color='#667788', fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                      edgecolor='#1a1a2e'))

    outpath = OUTPUT_DIR / "11_fisher_information.png"
    fig.savefig(outpath, dpi=200, facecolor='#0a0a0f',
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    # ------------------------------------------------------------------
    summary = {
        'script': '11_fisher_information.py',
        'description': 'Fisher information and Cramer-Rao bounds for slit separation estimation',
        'parameters': {
            'separation': float(d_sep),
            'delta': float(delta),
            'W': W, 'H': H,
            'lam': float(d['lam']),
            'N_clicks': int(N_clicks_default),
        },
        'results': {
            'F_born': float(F_born),
            'F_psi': float(F_psi),
            'F_psi_re': float(F_re),
            'F_psi_im': float(F_im),
            'F_per_click': float(F_per_click),
            'F_N_clicks': float(F_N),
            'CR_born': float(CR_born),
            'CR_psi': float(CR_psi),
            'CR_N_clicks': float(CR_N),
            'N_star_born': float(N_star_born),
            'N_star_psi': float(N_star_psi),
            'ratio_F_psi_over_F_born': float(F_psi / F_born) if F_born > 0 else None,
        },
    }
    save_json('11_fisher_information', summary)

    print()
    print(f"  RESULT: The full |psi|^2 field is equivalent to {N_star_born:,.0f} detector clicks.")
    print(f"  The complex psi field is equivalent to {N_star_psi:,.0f} detector clicks.")
    print(f"  A boolean detector must click ~{N_star_born:,.0f}x to match one field snapshot.")
    print()


if __name__ == '__main__':
    main()
