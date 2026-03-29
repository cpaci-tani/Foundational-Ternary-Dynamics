#!/usr/bin/env python3
"""
Analyze void classification on the 3D lattice detection plane.

Reads ds_void_classification.csv (64x64 plane) and produces a 2x2 figure:
  (a) Total |J|^2 intensity heatmap
  (b) Classification map (destructive / genuine void / not dark)
  (c) Individual energy |J_A|^2 + |J_B|^2
  (d) Bar chart: destructive vs genuine void fractions
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
CSV_PATH = os.path.join(OUTPUT_DIR, 'ds_void_classification.csv')
OUT_PNG = os.path.join(OUTPUT_DIR, 'analyze_void_classification.png')

N = 64  # expected grid size


def main():
    plt = setup_style()

    # --- Load data ---
    data = np.genfromtxt(CSV_PATH, delimiter=',', names=True)
    j_total_mag2 = data['J_total_mag2'].reshape(N, N)
    j_a_mag2 = data['J_A_mag2'].reshape(N, N)
    j_b_mag2 = data['J_B_mag2'].reshape(N, N)
    classification = data['classification'].astype(int).reshape(N, N)

    # Classification codes: 1=destructive, 0=genuine void, -1=not dark
    n_destructive = np.sum(classification == 1)
    n_genuine = np.sum(classification == 0)
    n_not_dark = np.sum(classification == -1)
    n_dark = n_destructive + n_genuine  # dark = destructive + genuine void
    pct_destructive = 100.0 * n_destructive / max(n_dark, 1)
    pct_genuine = 100.0 * n_genuine / max(n_dark, 1)

    # Individual energy (sum of individual source intensities)
    j_individual = j_a_mag2 + j_b_mag2

    # --- Figure ---
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    fig.patch.set_facecolor(BG_COLOR)
    fig.suptitle('Void Is Destructive Interference: 3D Lattice Confirms 2D Finding',
                 color=ACCENT_COLOR, fontsize=13, fontweight='bold', y=0.97)

    # (a) Total |J|^2 intensity
    ax = axes[0, 0]
    im = ax.imshow(j_total_mag2, origin='lower', cmap='inferno', aspect='equal')
    cb = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cb.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)
    ax.set_title(r'(a) Total $|J|^2$ intensity', color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])

    # (b) Classification map: destructive=red, genuine void=gray, not dark=black
    ax = axes[0, 1]
    rgb_map = np.zeros((N, N, 3))
    # destructive (1) = red
    mask_d = classification == 1
    rgb_map[mask_d] = [0.9, 0.2, 0.2]
    # genuine void (0) = gray
    mask_g = classification == 0
    rgb_map[mask_g] = [0.5, 0.5, 0.5]
    # not dark (-1) = black (already zeros)
    ax.imshow(rgb_map, origin='lower', aspect='equal')
    ax.set_title('(b) Classification: red=destructive, gray=genuine void',
                 color=TEXT_COLOR, fontsize=8)
    ax.set_xticks([])
    ax.set_yticks([])

    # (c) Individual energy |J_A|^2 + |J_B|^2
    ax = axes[1, 0]
    im2 = ax.imshow(j_individual, origin='lower', cmap='inferno', aspect='equal')
    cb2 = fig.colorbar(im2, ax=ax, fraction=0.046, pad=0.04)
    cb2.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)
    ax.set_title(r'(c) Individual $|J_A|^2 + |J_B|^2$ (energy present in dark zones)',
                 color=TEXT_COLOR, fontsize=8)
    ax.set_xticks([])
    ax.set_yticks([])

    # (d) Bar chart: destructive vs genuine void
    ax = axes[1, 1]
    labels = ['Destructive\nInterference', 'Genuine\nVoid']
    values = [pct_destructive, pct_genuine]
    colors_bar = ['#ff4444', '#888888']
    bars = ax.bar(labels, values, color=colors_bar, width=0.5, edgecolor='none')

    for bar, val in zip(bars, values):
        ax.text(bar.get_x() + bar.get_width() / 2.0, bar.get_height() + 1.5,
                f'{val:.1f}%', ha='center', va='bottom',
                color=TEXT_COLOR, fontsize=13, fontweight='bold')

    ax.set_ylabel('% of dark voxels', color=TEXT_COLOR)
    ax.set_ylim(0, 110)
    ax.set_title('(d) What is "nothing"?', color=TEXT_COLOR, fontsize=9)

    # Big annotation
    ax.text(0.5, 0.75, f'{pct_destructive:.1f}% of "nothing"\nis cancellation',
            transform=ax.transAxes, ha='center', va='top',
            color='#ff6666', fontsize=12, fontweight='bold',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                      edgecolor=GRID_COLOR, alpha=0.9))

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    fig.savefig(OUT_PNG, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f'Saved: {OUT_PNG}')


if __name__ == '__main__':
    main()
