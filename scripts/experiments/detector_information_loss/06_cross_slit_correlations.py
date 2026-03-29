"""
06_cross_slit_correlations.py — Correlations between the two slit contributions

WHY THIS MATTERS:
psi = psi_A + psi_B.  The full field lets you decompose this and compute
the cross-term 2*Re(psi_A* . psi_B) that IS the interference.  The
detector gives |psi_A + psi_B|^2 with no way to separate the cross-term
from the self-terms.  The cross-term has both positive AND negative values
(constructive/destructive) while |psi|^2 mixes them irreversibly.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, compute_single_source_field,
    born_rule, setup_style, make_figure, save_json,
    DEFAULTS,
)


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("06  CROSS-SLIT CORRELATIONS — Decomposing interference")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  psi = psi_A + psi_B.  The cross-term 2*Re(psi_A* . psi_B)")
    print("  IS the interference.  It has positive AND negative values")
    print("  (constructive/destructive).  |psi|^2 mixes them irreversibly.")
    print("  The detector can never separate what came from each slit.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']
    lam = d['lam']
    sep = d['separation']
    phase_offset = d['phase_offset']
    t = d['t']

    cx, cy = W / 2.0, H / 2.0

    # 1. Compute individual source fields
    print("  Computing source A field ...")
    reA, imA = compute_single_source_field(
        W=W, H=H, lam=lam,
        source_x=cx - sep / 2.0, source_y=cy,
        phase=0.0, t=t,
    )

    print("  Computing source B field ...")
    reB, imB = compute_single_source_field(
        W=W, H=H, lam=lam,
        source_x=cx + sep / 2.0, source_y=cy,
        phase=phase_offset, t=t,
    )

    # 2. Verify sum matches dual-source computation
    print("  Verifying psi_A + psi_B = psi_total ...")
    psi_re_sum = reA + reB
    psi_im_sum = imA + imB

    psi_re_dual, psi_im_dual = compute_dual_source_field(
        W=W, H=H, lam=lam, separation=sep,
        phase_offset=phase_offset, t=t,
    )
    sum_error = np.max(np.abs(psi_re_sum - psi_re_dual)) + \
                np.max(np.abs(psi_im_sum - psi_im_dual))
    print(f"  Decomposition check: max|psi_sum - psi_dual| = {sum_error:.2e}")

    # 3. Compute terms
    # Self-terms: |psi_A|^2 + |psi_B|^2
    self_A = reA ** 2 + imA ** 2
    self_B = reB ** 2 + imB ** 2
    self_terms = self_A + self_B

    # Cross-term: 2*Re(psi_A* . psi_B) = 2*(reA*reB + imA*imB)
    cross_term = 2.0 * (reA * reB + imA * imB)

    # Total |psi|^2
    total = born_rule(psi_re_sum, psi_im_sum)

    # Verify: total = self_terms + cross_term
    decomp_error = np.max(np.abs(total - (self_terms + cross_term)))
    print(f"  |psi|^2 = self + cross check: max error = {decomp_error:.2e}")

    # 4. Energy fractions
    # Use sum of absolute values for meaningful fractions
    total_energy = np.sum(np.abs(total))
    self_energy = np.sum(self_terms)
    cross_energy_pos = np.sum(cross_term[cross_term > 0])
    cross_energy_neg = np.sum(np.abs(cross_term[cross_term < 0]))
    cross_energy_net = np.sum(cross_term)

    frac_self = self_energy / max(total_energy, 1e-12)
    frac_cross_pos = cross_energy_pos / max(total_energy, 1e-12)
    frac_cross_neg = cross_energy_neg / max(total_energy, 1e-12)
    frac_cross_net = cross_energy_net / max(total_energy, 1e-12)

    # Cross-term statistics
    n_positive = np.sum(cross_term > 0)
    n_negative = np.sum(cross_term < 0)
    n_total = cross_term.size
    frac_constructive = n_positive / n_total
    frac_destructive = n_negative / n_total

    # ------------------------------------------------------------------
    # Print results
    print()
    print(f"  Self-terms |psi_A|^2 + |psi_B|^2:")
    print(f"    Total energy fraction: {frac_self:.4f}")
    print(f"  Cross-term 2*Re(psi_A* . psi_B):")
    print(f"    Constructive (>0) energy: {frac_cross_pos:.4f}")
    print(f"    Destructive  (<0) energy: {frac_cross_neg:.4f}")
    print(f"    Net cross-term fraction:  {frac_cross_net:+.4f}")
    print(f"    Pixels constructive: {frac_constructive:.1%}")
    print(f"    Pixels destructive:  {frac_destructive:.1%}")
    print(f"  Cross-term range: [{cross_term.min():.6f}, {cross_term.max():.6f}]")
    print(f"  |psi|^2 range:    [{total.min():.6f}, {total.max():.6f}]")
    print()

    # ------------------------------------------------------------------
    # Figure: 4 panels
    plt = setup_style()

    fig, axes = plt.subplots(1, 4, figsize=(20, 4.8))
    fig.suptitle("Cross-Slit Correlations: The Irreversible Mixing",
                 color='#a0b8d8', fontsize=12, fontweight='bold', y=0.98)

    # (a) Cross-term heatmap (RdBu_r, centered at 0)
    ax = axes[0]
    vmax_cross = max(abs(cross_term.min()), abs(cross_term.max()))
    im0 = ax.imshow(cross_term, origin='lower', cmap='RdBu_r',
                     vmin=-vmax_cross, vmax=vmax_cross, aspect='equal')
    ax.set_title('(a) Cross-term 2Re(psi_A* . psi_B)', color='#c8d8e8', fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb0 = fig.colorbar(im0, ax=ax, fraction=0.046, pad=0.04)
    cb0.ax.yaxis.set_tick_params(color='#667788')

    # (b) Self-term sum (warm colormap)
    ax = axes[1]
    im1 = ax.imshow(self_terms, origin='lower', cmap='inferno', aspect='equal')
    ax.set_title('(b) Self-terms |psi_A|^2 + |psi_B|^2', color='#c8d8e8', fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb1 = fig.colorbar(im1, ax=ax, fraction=0.046, pad=0.04)
    cb1.ax.yaxis.set_tick_params(color='#667788')

    # (c) Total |psi|^2 (inseparable sum)
    ax = axes[2]
    im2 = ax.imshow(total, origin='lower', cmap='inferno', aspect='equal')
    ax.set_title('(c) Total |psi|^2 (inseparable)', color='#c8d8e8', fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb2 = fig.colorbar(im2, ax=ax, fraction=0.046, pad=0.04)
    cb2.ax.yaxis.set_tick_params(color='#667788')

    # (d) Midline plot: all three terms stacked
    ax = axes[3]
    mid_y = H // 2
    x_px = np.arange(W)

    ax.fill_between(x_px, 0, self_terms[mid_y, :],
                     alpha=0.3, color='#ffaa44', label='Self-terms')
    ax.plot(x_px, self_terms[mid_y, :], color='#ffaa44', linewidth=0.8)

    ax.fill_between(x_px, 0, cross_term[mid_y, :],
                     where=cross_term[mid_y, :] > 0,
                     alpha=0.3, color='#4488ff', label='Cross (+)')
    ax.fill_between(x_px, 0, cross_term[mid_y, :],
                     where=cross_term[mid_y, :] < 0,
                     alpha=0.3, color='#ff4444', label='Cross (-)')
    ax.plot(x_px, cross_term[mid_y, :], color='#66ccff', linewidth=0.6)

    ax.plot(x_px, total[mid_y, :], color='#ffffff', linewidth=1.0,
            alpha=0.9, label='Total |psi|^2')

    ax.axhline(0, color='#667788', linewidth=0.4, linestyle='--')
    ax.set_xlabel('x (pixels)', fontsize=8, color='#c8d8e8')
    ax.set_ylabel('Intensity', fontsize=8, color='#c8d8e8')
    ax.set_title('(d) Midline: self + cross = total', color='#c8d8e8', fontsize=9)
    ax.legend(fontsize=6, loc='upper right',
              facecolor='#0d0d15', edgecolor='#667788')
    ax.grid(True, alpha=0.15, color='#1a1a2e')

    # Metrics text
    metrics = (
        f"Self fraction: {frac_self:.3f}  |  "
        f"Cross +: {frac_cross_pos:.3f}  Cross -: {frac_cross_neg:.3f}  "
        f"Net: {frac_cross_net:+.3f}  |  "
        f"Constructive: {frac_constructive:.1%}  Destructive: {frac_destructive:.1%}"
    )
    fig.text(0.5, 0.01, metrics, ha='center', va='bottom',
             color='#667788', fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                       edgecolor='#1a1a2e'))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])
    from experiments.detector_information_loss.field_engine import OUTPUT_DIR
    outpath = OUTPUT_DIR / "06_cross_slit_correlations.png"
    fig.savefig(outpath, dpi=200, facecolor='#0a0a0f',
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '06_cross_slit_correlations.py',
        'description': 'Decomposition of |psi|^2 into self-terms and cross-term',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'decomposition_error': float(decomp_error),
            'energy_fractions': {
                'self_terms': float(frac_self),
                'cross_positive': float(frac_cross_pos),
                'cross_negative': float(frac_cross_neg),
                'cross_net': float(frac_cross_net),
            },
            'pixel_fractions': {
                'constructive': float(frac_constructive),
                'destructive': float(frac_destructive),
            },
            'cross_term_range': {
                'min': float(cross_term.min()),
                'max': float(cross_term.max()),
            },
            'total_range': {
                'min': float(total.min()),
                'max': float(total.max()),
            },
        },
    }
    save_json('06_cross_slit_correlations', summary)

    print()
    print(f"  RESULT: The cross-term 2*Re(psi_A* . psi_B) carries the")
    print(f"  interference pattern with {frac_constructive:.1%} constructive")
    print(f"  and {frac_destructive:.1%} destructive pixels.")
    print(f"  |psi|^2 mixes self-terms and cross-term irreversibly.")
    print(f"  The detector sees only the total — it can never know")
    print(f"  what fraction came from each slit.")
    print()


if __name__ == '__main__':
    main()
