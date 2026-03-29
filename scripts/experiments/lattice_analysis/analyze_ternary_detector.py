#!/usr/bin/env python3
"""
Analyze ternary detection: sign preservation on the 3D lattice.

Reads ds_ternary_events.csv and produces a 1x2 figure:
  (a) 3D scatter of manifested voxels colored by state
  (b) Bar chart: ternary accuracy vs boolean baseline
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
CSV_PATH = os.path.join(OUTPUT_DIR, 'ds_ternary_events.csv')
OUT_PNG = os.path.join(OUTPUT_DIR, 'analyze_ternary_detector.png')


def main():
    plt = setup_style()
    from mpl_toolkits.mplot3d import Axes3D  # noqa: F401

    # --- Load data ---
    data = np.genfromtxt(CSV_PATH, delimiter=',', names=True)
    x = data['x']
    y = data['y']
    z = data['z']
    state = data['state']

    # Separate +1 and -1 manifested voxels (exclude void = 0)
    mask_pos = state > 0.5
    mask_neg = state < -0.5
    mask_manifested = mask_pos | mask_neg

    n_pos = np.sum(mask_pos)
    n_neg = np.sum(mask_neg)
    n_void = np.sum(~mask_manifested)
    n_total = len(state)

    # Ternary accuracy: fraction with correct sign preserved (all non-void)
    # Boolean baseline: 0.5 (random sign assignment)
    ternary_accuracy = np.sum(mask_manifested) / max(n_total, 1)
    boolean_baseline = 0.5
    advantage = ternary_accuracy / boolean_baseline if boolean_baseline > 0 else 0

    # --- Figure ---
    fig = plt.figure(figsize=(14, 6))
    fig.patch.set_facecolor(BG_COLOR)
    fig.suptitle('Ternary Detection: Sign Preservation on 3D Lattice',
                 color=ACCENT_COLOR, fontsize=14, fontweight='bold', y=0.97)

    # (a) 3D scatter
    ax3d = fig.add_subplot(1, 2, 1, projection='3d')
    ax3d.set_facecolor(BG_COLOR)
    if np.any(mask_pos):
        ax3d.scatter(x[mask_pos], y[mask_pos], z[mask_pos],
                     c='#ff3333', s=12, alpha=0.7, label='+1', depthshade=True)
    if np.any(mask_neg):
        ax3d.scatter(x[mask_neg], y[mask_neg], z[mask_neg],
                     c='#3377ff', s=12, alpha=0.7, label='-1', depthshade=True)
    ax3d.set_xlabel('x', color=SUBTLE_COLOR, fontsize=8)
    ax3d.set_ylabel('y', color=SUBTLE_COLOR, fontsize=8)
    ax3d.set_zlabel('z', color=SUBTLE_COLOR, fontsize=8)
    ax3d.set_title('(a) Manifested voxels by sign', color=TEXT_COLOR, fontsize=9)
    ax3d.tick_params(colors=SUBTLE_COLOR, labelsize=7)
    ax3d.xaxis.pane.fill = False
    ax3d.yaxis.pane.fill = False
    ax3d.zaxis.pane.fill = False
    ax3d.xaxis.pane.set_edgecolor(GRID_COLOR)
    ax3d.yaxis.pane.set_edgecolor(GRID_COLOR)
    ax3d.zaxis.pane.set_edgecolor(GRID_COLOR)
    leg = ax3d.legend(loc='upper left', fontsize=8, framealpha=0.5)
    for text in leg.get_texts():
        text.set_color(TEXT_COLOR)

    # (b) Bar chart: ternary vs boolean
    ax_bar = fig.add_subplot(1, 2, 2)
    labels = ['Ternary\nDetector', 'Boolean\nBaseline']
    values = [ternary_accuracy, boolean_baseline]
    colors = [ACCENT_COLOR, SUBTLE_COLOR]
    bars = ax_bar.bar(labels, values, color=colors, width=0.5, edgecolor='none')

    for bar, val in zip(bars, values):
        ax_bar.text(bar.get_x() + bar.get_width() / 2.0, bar.get_height() + 0.02,
                    f'{val:.3f}', ha='center', va='bottom',
                    color=TEXT_COLOR, fontsize=11, fontweight='bold')

    ax_bar.set_ylabel('Accuracy', color=TEXT_COLOR)
    ax_bar.set_ylim(0, 1.15)
    ax_bar.set_title('(b) Ternary accuracy vs boolean baseline',
                     color=TEXT_COLOR, fontsize=9)

    # Advantage annotation
    ax_bar.text(0.5, 0.85, f'Advantage ratio: {advantage:.2f}x',
                transform=ax_bar.transAxes, ha='center', va='top',
                color=ACCENT_COLOR, fontsize=12, fontweight='bold',
                bbox=dict(boxstyle='round,pad=0.4', facecolor='#0d0d15',
                          edgecolor=GRID_COLOR))

    # Stats annotation
    stats_text = f'+1: {n_pos}   -1: {n_neg}   void: {n_void}   total: {n_total}'
    fig.text(0.5, 0.02, stats_text, ha='center', va='bottom',
             color=SUBTLE_COLOR, fontsize=8, family='monospace',
             bbox=dict(boxstyle='round,pad=0.4', facecolor='#0d0d15',
                       edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0.06, 1, 0.94])
    fig.savefig(OUT_PNG, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f'Saved: {OUT_PNG}')


if __name__ == '__main__':
    main()
