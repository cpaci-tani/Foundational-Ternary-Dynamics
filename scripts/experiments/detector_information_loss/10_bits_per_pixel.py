"""
10_bits_per_pixel.py — Total information at each degradation stage

WHY THIS MATTERS:
The spine of the suite. Quantifies total information at each stage of the
measurement pipeline:  psi -> |psi|^2 -> N clicks -> 1 click.

At each transition, information is irreversibly destroyed. The full complex
field psi carries ~2x the Shannon entropy of the real-valued |psi|^2.
Sampling N clicks from |psi|^2 compresses the continuous distribution into
a sparse histogram. A single click is log2(W*H) bits total — roughly
0.00007 bits per pixel for a 512x512 grid.
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    compute_dual_source_field, born_rule, sample_detector_clicks,
    phase_field, amplitude_field, shannon_entropy,
    setup_style, make_figure, save_json, detector_dots_image,
    phase_to_rgb, DEFAULTS, OUTPUT_DIR,
)


def main():
    # ------------------------------------------------------------------
    print("=" * 64)
    print("10  BITS PER PIXEL — Total information at each degradation stage")
    print("=" * 64)
    print()
    print("WHY THIS MATTERS:")
    print("  Quantifies total information at each stage of measurement:")
    print("  psi -> |psi|^2 -> N clicks -> 1 click.")
    print("  Each transition irreversibly destroys information.")
    print()

    d = DEFAULTS
    W, H = d['W'], d['H']
    N_pixels = W * H

    # 1. Compute full field
    print("  Computing dual-source field ...")
    psi_re, psi_im = compute_dual_source_field(
        W=W, H=H, lam=d['lam'], separation=d['separation'],
        phase_offset=d['phase_offset'], t=d['t'],
    )

    # 2. Derived fields
    born = born_rule(psi_re, psi_im)
    theta = phase_field(psi_re, psi_im)
    amp = amplitude_field(psi_re, psi_im)
    clicks = sample_detector_clicks(born, d['N_clicks'])

    # ------------------------------------------------------------------
    # Stage 1: Full psi — 2 channels (Re and Im)
    # ------------------------------------------------------------------
    H_re = shannon_entropy(psi_re, bins=256)
    H_im = shannon_entropy(psi_im, bins=256)
    bits_psi = H_re + H_im

    # ------------------------------------------------------------------
    # Stage 2: |psi|^2 — 1 channel
    # ------------------------------------------------------------------
    bits_born = shannon_entropy(born, bins=256)

    # ------------------------------------------------------------------
    # Stage 3: N clicks — bits/pixel of the click histogram image
    # ------------------------------------------------------------------
    dots_img = detector_dots_image(clicks, W, H)
    # Use the same value-histogram entropy as stages 1-2: how many bits
    # per pixel to encode the count at each lattice site?
    bits_N_clicks = shannon_entropy(dots_img, bins=256)

    # ------------------------------------------------------------------
    # Stage 4: 1 click — bits/pixel of a single-click image
    # ------------------------------------------------------------------
    # A single click produces an image with (N_pixels-1) zeros and one 1.
    # Value-histogram entropy: H = -[(N-1)/N log2((N-1)/N) + 1/N log2(1/N)]
    p1 = 1.0 / N_pixels
    p0 = 1.0 - p1
    bits_1_click = -(p0 * np.log2(p0) + p1 * np.log2(p1))

    # ------------------------------------------------------------------
    # Percentage lost at each transition (all in bits/pixel)
    # ------------------------------------------------------------------
    loss_born = bits_psi - bits_born
    loss_N_clicks = bits_born - bits_N_clicks
    loss_1_click = bits_N_clicks - bits_1_click

    pct_born = 100.0 * loss_born / bits_psi if bits_psi > 0 else 0.0
    pct_N_clicks = 100.0 * (bits_psi - bits_N_clicks) / bits_psi if bits_psi > 0 else 0.0
    pct_1_click = 100.0 * (bits_psi - bits_1_click) / bits_psi if bits_psi > 0 else 0.0

    # ------------------------------------------------------------------
    # Print results (all bits/pixel)
    # ------------------------------------------------------------------
    print(f"  Stage 1 — Full psi (Re + Im):")
    print(f"    H(Re) = {H_re:.4f} bits/px,  H(Im) = {H_im:.4f} bits/px")
    print(f"    Total bits/pixel = {bits_psi:.4f}")
    print()
    print(f"  Stage 2 — |psi|^2:")
    print(f"    H(|psi|^2) = {bits_born:.4f} bits/pixel")
    print(f"    Lost: {loss_born:.4f} bits/px  ({pct_born:.1f}% of original)")
    print()
    print(f"  Stage 3 — {d['N_clicks']:,} clicks:")
    print(f"    H(click image) = {bits_N_clicks:.4f} bits/pixel")
    print(f"    Lost from psi: {pct_N_clicks:.1f}% of original")
    print()
    print(f"  Stage 4 — 1 click:")
    print(f"    H(single-click image) = {bits_1_click:.6f} bits/pixel")
    print(f"    Lost from psi: {pct_1_click:.1f}% of original")
    print()

    # ------------------------------------------------------------------
    # Figure: custom gridspec layout
    # ------------------------------------------------------------------
    plt = setup_style()
    import matplotlib.gridspec as gridspec

    fig = plt.figure(figsize=(16, 10))
    fig.patch.set_facecolor('#0a0a0f')
    gs = gridspec.GridSpec(2, 3, figure=fig, hspace=0.35, wspace=0.3,
                           left=0.06, right=0.96, top=0.92, bottom=0.08)

    fig.suptitle("Bits Per Pixel: Information at Each Degradation Stage",
                 color='#a0b8d8', fontsize=13, fontweight='bold')

    # ---- Row 1: Three field images ----
    # (a) Full psi as phase-color image
    phase_rgb = phase_to_rgb(theta, amplitude=amp)
    ax_a = fig.add_subplot(gs[0, 0])
    ax_a.imshow(phase_rgb, origin='lower', aspect='equal')
    ax_a.set_title(f'(a) Full psi  [{bits_psi:.2f} bits/px]',
                   color='#c8d8e8', fontsize=9)
    ax_a.set_xticks([])
    ax_a.set_yticks([])

    # (b) |psi|^2 grayscale
    born_disp = born / max(born.max(), 1e-12)
    ax_b = fig.add_subplot(gs[0, 1])
    ax_b.imshow(born_disp, origin='lower', cmap='gray', aspect='equal')
    ax_b.set_title(f'(b) |psi|^2  [{bits_born:.2f} bits/px]',
                   color='#c8d8e8', fontsize=9)
    ax_b.set_xticks([])
    ax_b.set_yticks([])

    # (c) Detector dots
    ax_c = fig.add_subplot(gs[0, 2])
    ax_c.imshow(dots_img, origin='lower', cmap='hot', aspect='equal')
    ax_c.set_title(f'(c) {d["N_clicks"]:,} clicks  [{bits_N_clicks:.2f} bits/px]',
                   color='#c8d8e8', fontsize=9)
    ax_c.set_xticks([])
    ax_c.set_yticks([])

    # ---- Row 2 left: Stacked bar chart ----
    ax_bar = fig.add_subplot(gs[1, 0:2])
    ax_bar.set_facecolor('#0a0a0f')

    stages = ['Full psi\n(Re+Im)', '|psi|^2', f'{d["N_clicks"]:,}\nclicks', '1 click']
    values = [bits_psi, bits_born, bits_N_clicks, bits_1_click]
    colors = ['#6699cc', '#44aa88', '#cc8844', '#cc4444']

    bars = ax_bar.bar(stages, values, color=colors, edgecolor='#1a1a2e',
                      linewidth=0.8, width=0.6)

    # Annotate each bar
    for bar, val in zip(bars, values):
        ypos = bar.get_height()
        ax_bar.text(bar.get_x() + bar.get_width() / 2, ypos + max(values) * 0.02,
                    f'{val:.2f}', ha='center', va='bottom',
                    color='#c8d8e8', fontsize=9, fontweight='bold')

    ax_bar.set_ylabel('Information (bits/pixel)', color='#c8d8e8', fontsize=10)
    ax_bar.set_title('(d) Information content at each stage',
                     color='#c8d8e8', fontsize=10)
    ax_bar.spines['top'].set_visible(False)
    ax_bar.spines['right'].set_visible(False)
    ax_bar.spines['left'].set_color('#667788')
    ax_bar.spines['bottom'].set_color('#667788')
    ax_bar.tick_params(axis='x', colors='#c8d8e8', labelsize=9)

    # ---- Row 2 right: Waterfall diagram of losses ----
    ax_wf = fig.add_subplot(gs[1, 2])
    ax_wf.set_facecolor('#0a0a0f')

    # Waterfall: start at bits_psi, then show each loss (all bits/pixel)
    transitions = [
        ('psi -> |psi|^2', loss_born),
        ('|psi|^2 -> N clicks', loss_N_clicks),
        ('N clicks -> 1 click', loss_1_click),
    ]

    cumulative = bits_psi
    y_labels = []
    y_positions = []
    bar_widths = []
    bar_starts = []
    bar_colors = []

    # Starting bar
    y_labels.append(f'Full psi\n{bits_psi:.2f} b/px')
    y_positions.append(0)
    bar_starts.append(0)
    bar_widths.append(cumulative)
    bar_colors.append('#6699cc')

    for i, (label, loss_val) in enumerate(transitions):
        cumulative_new = cumulative - loss_val
        y_labels.append(f'{label}\n-{loss_val:.2f} b/px')
        y_positions.append(i + 1)
        bar_starts.append(cumulative_new)
        bar_widths.append(loss_val)
        bar_colors.append('#cc4444')
        cumulative = cumulative_new

    # Remaining bar
    y_labels.append(f'1 click\n{bits_1_click:.4f} b/px')
    y_positions.append(len(transitions) + 1)
    bar_starts.append(0)
    bar_widths.append(bits_1_click)
    bar_colors.append('#44aa88')

    ax_wf.barh(y_positions, bar_widths, left=bar_starts, color=bar_colors,
               edgecolor='#1a1a2e', height=0.6)
    ax_wf.set_yticks(y_positions)
    ax_wf.set_yticklabels(y_labels, fontsize=7, color='#c8d8e8')
    ax_wf.set_xlabel('Bits/pixel', color='#c8d8e8', fontsize=9)
    ax_wf.set_title('(e) Waterfall: information losses',
                     color='#c8d8e8', fontsize=10)
    ax_wf.invert_yaxis()
    ax_wf.spines['top'].set_visible(False)
    ax_wf.spines['right'].set_visible(False)
    ax_wf.spines['left'].set_color('#667788')
    ax_wf.spines['bottom'].set_color('#667788')

    outpath = OUTPUT_DIR / "10_bits_per_pixel.png"
    fig.savefig(outpath, dpi=200, facecolor='#0a0a0f',
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f"  Saved: {outpath}")

    # ------------------------------------------------------------------
    # Save JSON
    # ------------------------------------------------------------------
    summary = {
        'script': '10_bits_per_pixel.py',
        'description': 'Total information at each degradation stage: psi -> born -> N clicks -> 1 click',
        'parameters': {k: float(v) if isinstance(v, (int, float)) else v
                       for k, v in d.items()},
        'results': {
            'stage_1_psi': {
                'H_re_bits': float(H_re),
                'H_im_bits': float(H_im),
                'total_bits_per_pixel': float(bits_psi),
            },
            'stage_2_born': {
                'H_born_bits_per_pixel': float(bits_born),
                'bits_lost_from_psi': float(loss_born),
                'pct_lost_from_psi': float(pct_born),
            },
            'stage_3_N_clicks': {
                'N_clicks': int(d['N_clicks']),
                'bits_per_pixel': float(bits_N_clicks),
                'pct_lost_from_psi': float(pct_N_clicks),
            },
            'stage_4_one_click': {
                'bits_per_pixel': float(bits_1_click),
                'pct_lost_from_psi': float(pct_1_click),
            },
            'grid': {'W': W, 'H': H, 'N_pixels': N_pixels},
        },
    }
    save_json('10_bits_per_pixel', summary)

    print()
    print(f"  RESULT: Full psi carries {bits_psi:.2f} bits/pixel.")
    print(f"  |psi|^2 retains {bits_born:.2f} bits/pixel ({100 - pct_born:.1f}% of original).")
    print(f"  {d['N_clicks']:,} clicks carry {bits_N_clicks:.4f} bits/pixel.")
    print(f"  1 click carries {bits_1_click:.6f} bits/pixel.")
    print()


if __name__ == '__main__':
    main()
