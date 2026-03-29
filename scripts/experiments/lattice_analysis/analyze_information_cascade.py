#!/usr/bin/env python3
"""
Analyze the information cascade through the detection pipeline.

Reads ds_information_cascade.csv and produces a 1x2 figure:
  (a) Bar chart of entropy at each stage
  (b) Waterfall chart showing losses between stages
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from experiments.detector_information_loss.field_engine import (
    setup_style, BG_COLOR, TEXT_COLOR, SUBTLE_COLOR, ACCENT_COLOR, GRID_COLOR,
    phase_to_rgb,
)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = os.path.join(SCRIPT_DIR, 'output')
CSV_PATH = os.path.join(OUTPUT_DIR, 'ds_information_cascade.csv')
OUT_PNG = os.path.join(OUTPUT_DIR, 'analyze_information_cascade.png')


def main():
    plt = setup_style()

    # --- Load data ---
    # Columns: stage, label, entropy_bits_per_voxel, pct_of_original
    # Read as structured array; label may contain spaces so use csv
    import csv
    stages = []
    labels = []
    entropies = []
    pcts = []
    with open(CSV_PATH, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            stages.append(int(row['stage']))
            labels.append(row['label'].strip())
            entropies.append(float(row['entropy_bits_per_voxel']))
            pcts.append(float(row['pct_of_original']))

    n_stages = len(stages)
    entropies = np.array(entropies)
    pcts = np.array(pcts)

    # --- Figure ---
    fig, (ax_bar, ax_water) = plt.subplots(1, 2, figsize=(14, 6))
    fig.patch.set_facecolor(BG_COLOR)
    fig.suptitle(f'Information Cascade: 3D Lattice ({entropies[0]:.2f} '
                 f'\u2192 {entropies[-1]:.2f} bits/voxel)',
                 color=ACCENT_COLOR, fontsize=14, fontweight='bold', y=0.97)

    # (a) Bar chart of entropy at each stage
    x_pos = np.arange(n_stages)
    bar_colors = [ACCENT_COLOR if i == 0 else '#ff6644' for i in range(n_stages)]
    bars = ax_bar.bar(x_pos, entropies, color=bar_colors, width=0.6,
                      edgecolor='none', alpha=0.9)

    for i, (bar, ent, pct) in enumerate(zip(bars, entropies, pcts)):
        ax_bar.text(bar.get_x() + bar.get_width() / 2.0, bar.get_height() + 0.3,
                    f'{ent:.2f}\n({pct:.0f}%)',
                    ha='center', va='bottom', color=TEXT_COLOR,
                    fontsize=9, fontweight='bold')

    ax_bar.set_xticks(x_pos)
    ax_bar.set_xticklabels(labels, fontsize=8, rotation=15, ha='right')
    ax_bar.set_ylabel('Entropy (bits/voxel)', color=TEXT_COLOR)
    ax_bar.set_title('(a) Entropy at each detection stage',
                     color=TEXT_COLOR, fontsize=9)
    ax_bar.set_ylim(0, entropies.max() * 1.35)

    # (b) Waterfall chart showing losses between stages
    # Each bar starts where the previous ended; losses shown as red drops
    bottoms = np.zeros(n_stages)
    heights = np.zeros(n_stages)

    # First bar: full entropy
    bottoms[0] = 0
    heights[0] = entropies[0]

    for i in range(1, n_stages):
        loss = entropies[i - 1] - entropies[i]
        bottoms[i] = entropies[i]
        heights[i] = loss  # the loss segment

    # Draw remaining entropy as blue bars
    ax_water.bar(x_pos, entropies, color=ACCENT_COLOR, width=0.6,
                 edgecolor='none', alpha=0.5, label='Retained')

    # Draw loss segments on top
    for i in range(1, n_stages):
        loss = entropies[i - 1] - entropies[i]
        ax_water.bar(x_pos[i], loss, bottom=entropies[i], color='#ff4444',
                     width=0.6, edgecolor='none', alpha=0.8,
                     label='Lost' if i == 1 else None)
        ax_water.text(x_pos[i], entropies[i] + loss / 2.0,
                      f'-{loss:.2f}', ha='center', va='center',
                      color='#ffaaaa', fontsize=9, fontweight='bold')

    # Connector lines
    for i in range(n_stages - 1):
        ax_water.plot([x_pos[i] + 0.3, x_pos[i + 1] - 0.3],
                      [entropies[i], entropies[i]], '--',
                      color=SUBTLE_COLOR, lw=0.8, alpha=0.6)

    ax_water.set_xticks(x_pos)
    ax_water.set_xticklabels(labels, fontsize=8, rotation=15, ha='right')
    ax_water.set_ylabel('Entropy (bits/voxel)', color=TEXT_COLOR)
    ax_water.set_title('(b) Information lost at each stage',
                       color=TEXT_COLOR, fontsize=9)
    ax_water.set_ylim(0, entropies.max() * 1.2)

    leg = ax_water.legend(fontsize=8, loc='upper right', framealpha=0.7,
                          facecolor='#0d0d15', edgecolor=GRID_COLOR)
    for text in leg.get_texts():
        text.set_color(TEXT_COLOR)

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    fig.savefig(OUT_PNG, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f'Saved: {OUT_PNG}')


if __name__ == '__main__':
    main()
