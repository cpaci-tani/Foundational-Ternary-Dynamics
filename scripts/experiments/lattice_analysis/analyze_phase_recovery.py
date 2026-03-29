#!/usr/bin/env python3
"""
Analyze phase recovery on the 3D lattice detection plane.

Reads ds_phase_plane.csv (64x64 detection plane) and produces a 2x2 figure:
  (a) Phase field colored by phase_to_rgb
  (b) |J|^2 intensity heatmap
  (c) Ternary state field
  (d) Phase histogram with entropy annotation
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
CSV_PATH = os.path.join(OUTPUT_DIR, 'ds_phase_plane.csv')
OUT_PNG = os.path.join(OUTPUT_DIR, 'analyze_phase_recovery.png')

N = 64  # expected grid size


def main():
    plt = setup_style()
    from matplotlib.colors import ListedColormap

    # --- Load data ---
    data = np.genfromtxt(CSV_PATH, delimiter=',', names=True)
    phase = data['phase'].reshape(N, N)
    mag = data['mag'].reshape(N, N)
    state = data['state'].reshape(N, N)

    # --- Figure ---
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    fig.patch.set_facecolor(BG_COLOR)
    fig.suptitle('3D Lattice Phase Recovery: Detection Plane at x=48',
                 color=ACCENT_COLOR, fontsize=14, fontweight='bold', y=0.97)

    # (a) Phase field via phase_to_rgb
    ax = axes[0, 0]
    rgb = phase_to_rgb(phase, amplitude=mag)
    ax.imshow(rgb, origin='lower', aspect='equal')
    ax.set_title('(a) Phase field (hue = phase, luminance = |J|)',
                 color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])

    # (b) |J|^2 intensity
    ax = axes[0, 1]
    intensity = mag ** 2
    im = ax.imshow(intensity, origin='lower', cmap='inferno', aspect='equal')
    cb = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cb.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)
    ax.set_title(r'(b) $|J|^2$ intensity', color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])

    # (c) State field (+1 red, 0 black, -1 blue)
    ax = axes[1, 0]
    cmap_state = ListedColormap(['#3377ff', '#000000', '#ff3333'])
    im_s = ax.imshow(state, origin='lower', cmap=cmap_state, vmin=-1, vmax=1,
                     aspect='equal')
    cb_s = fig.colorbar(im_s, ax=ax, fraction=0.046, pad=0.04, ticks=[-1, 0, 1])
    cb_s.ax.set_yticklabels(['-1', '0', '+1'])
    cb_s.ax.yaxis.set_tick_params(color=SUBTLE_COLOR)
    ax.set_title('(c) State field s(y,z)', color=TEXT_COLOR, fontsize=9)
    ax.set_xticks([])
    ax.set_yticks([])

    # (d) Phase histogram with entropy
    ax = axes[1, 1]
    flat_phase = phase.ravel()
    counts, bin_edges = np.histogram(flat_phase, bins=64)
    probs = counts / counts.sum()
    probs_nz = probs[probs > 0]
    entropy = -np.sum(probs_nz * np.log2(probs_nz))

    bin_centers = 0.5 * (bin_edges[:-1] + bin_edges[1:])
    ax.bar(bin_centers, counts, width=(bin_edges[1] - bin_edges[0]) * 0.9,
           color=ACCENT_COLOR, edgecolor='none', alpha=0.85)
    ax.set_xlabel('Phase (radians)', color=TEXT_COLOR)
    ax.set_ylabel('Count', color=TEXT_COLOR)
    ax.set_title('(d) Phase histogram', color=TEXT_COLOR, fontsize=9)
    ax.text(0.95, 0.90, f'H = {entropy:.2f} bits',
            transform=ax.transAxes, ha='right', va='top',
            color=ACCENT_COLOR, fontsize=11, fontweight='bold',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#0d0d15',
                      edgecolor=GRID_COLOR))

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    fig.savefig(OUT_PNG, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f'Saved: {OUT_PNG}')


if __name__ == '__main__':
    main()
