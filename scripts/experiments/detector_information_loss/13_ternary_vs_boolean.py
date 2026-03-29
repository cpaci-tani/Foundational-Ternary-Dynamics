"""
13_ternary_vs_boolean.py — What ternary states preserve that boolean destroys

WHY THIS MATTERS:
FTD's ternary detector has three outcomes (+1, -1, 0) not two (click/no-click).
The sign carries phase information: +1 and -1 mean flux exceeded K_B in opposite
orientations.  A boolean detector collapses both to "click", destroying the sign
degree of freedom entirely.

Shannon entropy of the ternary record can reach log2(3) ~ 1.585 bits per site
(3 symbols), while boolean is capped at 1 bit (2 symbols).  More critically,
the ternary record preserves mutual information with the underlying phase field
that the boolean record discards.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import *


def ternary_state(psi_re, K_B):
    """Compute ternary state map: +1 where Re(psi) > K_B, -1 where Re(psi) < -K_B, 0 otherwise."""
    s = np.zeros_like(psi_re, dtype=np.int8)
    s[psi_re > K_B] = +1
    s[psi_re < -K_B] = -1
    return s


def boolean_state(psi_re, psi_im, K_B):
    """Compute boolean state map: 1 where |psi|^2 > K_B^2, 0 otherwise."""
    intensity = psi_re ** 2 + psi_im ** 2
    b = np.zeros_like(psi_re, dtype=np.int8)
    b[intensity > K_B ** 2] = 1
    return b


def discrete_entropy(labels):
    """Shannon entropy of a discrete label array, in bits."""
    unique, counts = np.unique(labels, return_counts=True)
    p = counts / counts.sum()
    p = p[p > 0]
    return -np.sum(p * np.log2(p))


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("13  TERNARY VS BOOLEAN — What ternary states preserve")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  FTD's ternary detector has three outcomes (+1, -1, 0) not two")
    print("  (click/no-click).  The sign carries phase information: +1 and -1")
    print("  at the same position mean flux exceeded K_B in opposite")
    print("  orientations.  Boolean collapses both to 'click'.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']

    # 1. Compute full field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    # 2. Compute phase field (ground truth for MI calculation)
    theta = phase_field(psi_re, psi_im)
    amp = amplitude_field(psi_re, psi_im)

    # Auto-calibrate K_B to the median of non-zero amplitudes so that
    # roughly half the pixels are above threshold and half below.
    amp_raw = np.sqrt(psi_re**2 + psi_im**2)
    K_B = float(np.median(amp_raw[amp_raw > 0]))
    print(f"  Auto-calibrated K_B = {K_B:.6f} (median of |psi| > 0)")

    # 3. Compute ternary and boolean state maps
    print("  Computing ternary and boolean state maps ...")
    s = ternary_state(psi_re, K_B)
    b = boolean_state(psi_re, psi_im, K_B)

    # 4. Shannon entropies
    H_ternary = discrete_entropy(s)
    H_boolean = discrete_entropy(b)
    H_max_ternary = np.log2(3)
    H_max_boolean = 1.0

    # 5. Mutual information with phase field
    #    Convert discrete states to float for MI calculation via joint histogram
    print("  Computing mutual information with phase field ...")
    s_float = s.astype(np.float64)
    b_float = b.astype(np.float64)

    MI_ternary_phase = mutual_information(s_float, theta, bins=64)
    MI_boolean_phase = mutual_information(b_float, theta, bins=64)

    # Ratio of information preserved
    if MI_boolean_phase > 0:
        MI_ratio = MI_ternary_phase / MI_boolean_phase
    else:
        MI_ratio = float('inf')

    # Population counts
    n_plus = int(np.sum(s == +1))
    n_minus = int(np.sum(s == -1))
    n_void = int(np.sum(s == 0))
    n_detect = int(np.sum(b == 1))
    n_silent = int(np.sum(b == 0))
    total = W * H

    # ------------------------------------------------------------------
    # Print results
    print()
    print(f"  Ternary populations:  +1: {n_plus:,} ({100*n_plus/total:.1f}%)  "
          f"-1: {n_minus:,} ({100*n_minus/total:.1f}%)  "
          f"0: {n_void:,} ({100*n_void/total:.1f}%)")
    print(f"  Boolean populations:   1: {n_detect:,} ({100*n_detect/total:.1f}%)  "
          f"0: {n_silent:,} ({100*n_silent/total:.1f}%)")
    print()
    print(f"  Shannon entropy:")
    print(f"    Ternary H(s) = {H_ternary:.4f} bits  (max = log2(3) = {H_max_ternary:.4f})")
    print(f"    Boolean H(b) = {H_boolean:.4f} bits  (max = 1.000)")
    print()
    print(f"  Mutual information with phase field:")
    print(f"    I(ternary; phase) = {MI_ternary_phase:.6f} bits")
    print(f"    I(boolean; phase) = {MI_boolean_phase:.6f} bits")
    print(f"    Ratio I(s;theta)/I(b;theta) = {MI_ratio:.2f}x")
    print()

    # ------------------------------------------------------------------
    # Figure: 4 panels
    plt = setup_style()

    fig, axes = plt.subplots(2, 2, figsize=(11, 10))
    fig.suptitle("Ternary vs Boolean Detection: Information Preserved",
                 color=ACCENT_COLOR, fontsize=13, fontweight='bold', y=0.98)

    # (a) Ternary state map: +1 as red, -1 as blue, 0 as black
    ax = axes[0, 0]
    ternary_rgb = np.zeros((H, W, 3), dtype=np.float64)
    ternary_rgb[s == +1, 0] = 1.0   # red
    ternary_rgb[s == -1, 2] = 1.0   # blue
    ax.imshow(ternary_rgb, origin='lower', aspect='equal')
    ax.set_title('(a) Ternary state: +1 (red), -1 (blue), 0 (black)',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])

    # (b) Boolean state map: white where detected, black where not
    ax = axes[0, 1]
    bool_display = b.astype(np.float64)
    ax.imshow(bool_display, origin='lower', cmap='gray', aspect='equal',
              vmin=0, vmax=1)
    ax.set_title('(b) Boolean state: 1 (white), 0 (black)',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])

    # (c) Phase field for reference
    phase_rgb = phase_to_rgb(theta, amplitude=amp)
    ax_c = axes[1, 0]
    ax_c.imshow(phase_rgb, origin='lower', aspect='equal')
    ax_c.set_title('(c) Phase field theta(x,y) — ground truth',
                    color=TEXT_COLOR, fontsize=9)
    ax_c.set_xticks([])
    ax_c.set_yticks([])

    # (d) Bar chart: entropy and MI comparison
    ax = axes[1, 1]
    labels = ['H(ternary)', 'H(boolean)', 'I(s;phase)', 'I(b;phase)']
    values = [H_ternary, H_boolean, MI_ternary_phase, MI_boolean_phase]
    colors = ['#66bbff', '#ff6666', '#66bbff', '#ff6666']
    bars = ax.bar(labels, values, color=colors, edgecolor='none', width=0.6)
    ax.set_ylabel('bits', color=TEXT_COLOR, fontsize=9)
    ax.set_title('(d) Entropy and Mutual Information',
                 color=TEXT_COLOR, fontsize=9)
    ax.grid(True, axis='y', alpha=0.3)
    ax.tick_params(axis='x', labelsize=8)

    # Annotate bars
    for bar, val in zip(bars, values):
        ax.text(bar.get_x() + bar.get_width() / 2,
                bar.get_height() + max(values) * 0.02,
                f'{val:.4f}', ha='center', va='bottom',
                color=TEXT_COLOR, fontsize=8)

    # Annotate MI ratio
    ax.annotate(
        f'Ternary preserves {MI_ratio:.1f}x more phase info',
        xy=(0.5, 0.92), xycoords='axes fraction',
        ha='center', va='top', fontsize=9, color='#ffcc66',
        bbox=dict(boxstyle='round,pad=0.3', facecolor='#1a1a2e',
                  edgecolor=GRID_COLOR),
    )

    # Metrics text
    metrics_text = (
        f"H(ternary) = {H_ternary:.4f} bits  |  "
        f"H(boolean) = {H_boolean:.4f} bits  |  "
        f"I(s;phase) = {MI_ternary_phase:.6f}  |  "
        f"I(b;phase) = {MI_boolean_phase:.6f}  |  "
        f"ratio = {MI_ratio:.2f}x"
    )
    fig.text(0.5, 0.01, metrics_text, ha='center', va='bottom',
             color=SUBTLE_COLOR, fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                       edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0.04, 1, 0.95])
    outpath = OUTPUT_DIR / "13_ternary_vs_boolean.png"
    fig.savefig(outpath, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    summary = {
        'script': '13_ternary_vs_boolean.py',
        'description': 'Ternary (+1,-1,0) vs boolean (1,0) detection — information preserved',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'H_ternary_bits': float(H_ternary),
            'H_boolean_bits': float(H_boolean),
            'H_max_ternary': float(H_max_ternary),
            'H_max_boolean': float(H_max_boolean),
            'MI_ternary_phase': float(MI_ternary_phase),
            'MI_boolean_phase': float(MI_boolean_phase),
            'MI_ratio': float(MI_ratio),
            'n_plus': n_plus,
            'n_minus': n_minus,
            'n_void': n_void,
            'n_detect_boolean': n_detect,
            'n_silent_boolean': n_silent,
            'K_B': float(K_B),
        },
    }
    save_json('13_ternary_vs_boolean', summary)

    print()
    print(f"  RESULT: The ternary detector preserves {MI_ratio:.1f}x more mutual")
    print(f"  information with the phase field than the boolean detector.")
    print(f"  H(ternary) = {H_ternary:.4f} bits vs H(boolean) = {H_boolean:.4f} bits.")
    print()


if __name__ == '__main__':
    main()
