"""
14_void_is_destructive_interference.py — 0 != "nothing happened"

WHY THIS MATTERS:
In boolean detection, no click is ambiguous: was the field weak (low energy)
or destructively interfering (high energy cancellation)?  These are physically
opposite situations.  The full psi distinguishes them immediately — the
individual source amplitudes |psi_A|^2 + |psi_B|^2 reveal energy is present
even where the total |psi|^2 ~ 0.

This script decomposes every "dark" pixel into genuine void (both sources
weak) vs destructive interference (high energy cancelling).  The fraction
of dark pixels that are actually high-energy cancellation is the information
the boolean detector misattributes to "nothing."
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import *


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("14  VOID IS DESTRUCTIVE INTERFERENCE — 0 != 'nothing happened'")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  In boolean detection, no click is ambiguous: was the field weak")
    print("  (low energy) or destructively interfering (high energy cancellation)?")
    print("  These are physically opposite.  The full psi distinguishes them")
    print("  immediately.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']
    K_B = d['K_B']
    separation = d['separation']

    # 1. Compute full dual-source field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=separation,
        phase_offset=d['phase_offset'], t=d['t'],
    )
    intensity_total = born_rule(psi_re, psi_im)

    # 2. Compute individual source fields
    print("  Computing individual source fields ...")
    cx, cy = W / 2.0, H / 2.0
    src_a_x = cx - separation / 2.0
    src_b_x = cx + separation / 2.0

    re_a, im_a = compute_single_source_field(
        W=W, H=H, lam=d['lam'],
        source_x=src_a_x, source_y=cy, phase=0.0, t=d['t'],
    )
    re_b, im_b = compute_single_source_field(
        W=W, H=H, lam=d['lam'],
        source_x=src_b_x, source_y=cy, phase=d['phase_offset'], t=d['t'],
    )

    intensity_a = born_rule(re_a, im_a)
    intensity_b = born_rule(re_b, im_b)
    intensity_individual_sum = intensity_a + intensity_b

    # 3. Identify "dark" pixels: total intensity below threshold
    #    Use 0.1 * median as threshold for "dark"
    median_intensity = np.median(intensity_total[intensity_total > 0])
    dark_threshold = 0.1 * median_intensity
    dark_mask = intensity_total < dark_threshold

    n_dark = int(dark_mask.sum())
    n_total = W * H
    print(f"  Dark pixels (|psi|^2 < {dark_threshold:.6f}): {n_dark:,} / {n_total:,} "
          f"({100*n_dark/n_total:.1f}%)")

    # 4. Classify dark pixels
    #    Destructive interference: individual energies present but cancelling
    #    Genuine void: individual energies also small
    energy_threshold = median_intensity  # if individual sum >= median, energy is present

    dark_individual_energy = intensity_individual_sum[dark_mask]
    is_cancellation = dark_individual_energy >= energy_threshold
    is_genuine_void = ~is_cancellation

    n_cancel = int(is_cancellation.sum())
    n_genuine = int(is_genuine_void.sum())
    frac_cancel = n_cancel / max(n_dark, 1)
    frac_genuine = n_genuine / max(n_dark, 1)

    # Average energy in each category
    avg_energy_cancel = float(dark_individual_energy[is_cancellation].mean()) if n_cancel > 0 else 0.0
    avg_energy_genuine = float(dark_individual_energy[is_genuine_void].mean()) if n_genuine > 0 else 0.0

    # ------------------------------------------------------------------
    # Print results
    print()
    print(f"  Classification of {n_dark:,} dark pixels:")
    print(f"    Destructive interference: {n_cancel:,} ({100*frac_cancel:.1f}%)")
    print(f"      avg(|psi_A|^2 + |psi_B|^2) = {avg_energy_cancel:.6f}")
    print(f"    Genuine void (weak field):  {n_genuine:,} ({100*frac_genuine:.1f}%)")
    print(f"      avg(|psi_A|^2 + |psi_B|^2) = {avg_energy_genuine:.6f}")
    print()
    print(f"  >>> {100*frac_cancel:.1f}% of what the detector calls 'nothing' is actually")
    print(f"      high-energy destructive interference. <<<")
    print()

    # ------------------------------------------------------------------
    # Figure: 4 panels
    plt = setup_style()

    fig, axes = plt.subplots(2, 2, figsize=(11, 10))
    fig.suptitle("Void Is Destructive Interference: 0 != 'Nothing Happened'",
                 color=ACCENT_COLOR, fontsize=13, fontweight='bold', y=0.98)

    # (a) |psi|^2 showing dark regions
    ax = axes[0, 0]
    disp_total = intensity_total / max(intensity_total.max(), 1e-12)
    im_a_plot = ax.imshow(disp_total, origin='lower', cmap='inferno', aspect='equal')
    ax.set_title('(a) |psi|^2 — total intensity', color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb_a = fig.colorbar(im_a_plot, ax=ax, fraction=0.046, pad=0.04)
    cb_a.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (b) Dark regions colored by type
    #     Red = destructive interference, Gray = genuine void, Black = not dark
    ax = axes[0, 1]
    dark_rgb = np.zeros((H, W, 3), dtype=np.float64)
    # Build classification map over dark pixels
    dark_class = np.zeros((H, W), dtype=np.int8)  # 0 = not dark
    dark_class[dark_mask] = 1  # default: genuine void
    # Overwrite cancellation pixels
    dark_yx = np.argwhere(dark_mask)
    cancel_yx = dark_yx[is_cancellation]
    if len(cancel_yx) > 0:
        dark_class[cancel_yx[:, 0], cancel_yx[:, 1]] = 2

    # Color: genuine void = gray, cancellation = red
    dark_rgb[dark_class == 1, :] = [0.4, 0.4, 0.4]  # gray
    dark_rgb[dark_class == 2, 0] = 1.0               # red
    dark_rgb[dark_class == 2, 1] = 0.2
    dark_rgb[dark_class == 2, 2] = 0.2
    ax.imshow(dark_rgb, origin='lower', aspect='equal')
    ax.set_title('(b) Dark pixel classification\n(red = cancellation, gray = genuine void)',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])

    # (c) |psi_A|^2 + |psi_B|^2 — individual energy sum
    ax = axes[1, 0]
    disp_indiv = intensity_individual_sum / max(intensity_individual_sum.max(), 1e-12)
    im_c_plot = ax.imshow(disp_indiv, origin='lower', cmap='inferno', aspect='equal')
    ax.set_title('(c) |psi_A|^2 + |psi_B|^2 — energy present\neven where |psi|^2 ~ 0',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])
    cb_c = fig.colorbar(im_c_plot, ax=ax, fraction=0.046, pad=0.04)
    cb_c.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)

    # (d) Bar chart / classification fractions
    ax = axes[1, 1]
    labels = ['Destructive\ninterference', 'Genuine\nvoid']
    fractions = [frac_cancel, frac_genuine]
    colors = ['#ff4444', '#888888']
    bars = ax.bar(labels, fractions, color=colors, edgecolor='none', width=0.5)
    ax.set_ylabel('Fraction of dark pixels', color=TEXT_COLOR, fontsize=9)
    ax.set_title('(d) What "no detection" really means',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_ylim(0, 1.05)
    ax.grid(True, axis='y', alpha=0.3)

    # Annotate bars with percentages
    for bar, frac in zip(bars, fractions):
        ax.text(bar.get_x() + bar.get_width() / 2,
                bar.get_height() + 0.02,
                f'{100*frac:.1f}%', ha='center', va='bottom',
                color=TEXT_COLOR, fontsize=10, fontweight='bold')

    # Key finding annotation
    ax.annotate(
        f'{100*frac_cancel:.0f}% of "nothing" is high-energy cancellation',
        xy=(0.5, 0.92), xycoords='axes fraction',
        ha='center', va='top', fontsize=9, color='#ffcc66',
        bbox=dict(boxstyle='round,pad=0.3', facecolor='#1a1a2e',
                  edgecolor=GRID_COLOR),
    )

    # Metrics text
    metrics_text = (
        f"Dark pixels: {n_dark:,}/{n_total:,}  |  "
        f"Cancellation: {n_cancel:,} ({100*frac_cancel:.1f}%)  |  "
        f"Genuine void: {n_genuine:,} ({100*frac_genuine:.1f}%)  |  "
        f"threshold: 0.1 * median(|psi|^2)"
    )
    fig.text(0.5, 0.01, metrics_text, ha='center', va='bottom',
             color=SUBTLE_COLOR, fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                       edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0.04, 1, 0.95])
    outpath = OUTPUT_DIR / "14_void_is_destructive_interference.png"
    fig.savefig(outpath, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '14_void_is_destructive_interference.py',
        'description': 'Classification of dark pixels: destructive interference vs genuine void',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'n_dark_pixels': n_dark,
            'n_total_pixels': n_total,
            'dark_fraction': float(n_dark / n_total),
            'n_destructive_interference': n_cancel,
            'n_genuine_void': n_genuine,
            'frac_destructive_interference': float(frac_cancel),
            'frac_genuine_void': float(frac_genuine),
            'avg_energy_cancellation': avg_energy_cancel,
            'avg_energy_genuine_void': avg_energy_genuine,
            'dark_threshold': float(dark_threshold),
            'energy_threshold_median': float(median_intensity),
        },
    }
    save_json('14_void_is_destructive_interference', summary)

    print()
    print(f"  RESULT: {100*frac_cancel:.1f}% of what the detector calls 'nothing' is actually")
    print(f"  high-energy destructive interference.  The boolean detector cannot")
    print(f"  distinguish cancellation from absence.")
    print()


if __name__ == '__main__':
    main()
