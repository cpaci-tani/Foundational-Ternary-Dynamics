#!/usr/bin/env python3
"""
Analyze 3D vortex filament topology on the lattice.

Reads ds_vortex_lines.csv (can be very large, ~82k rows) and produces a 2x2 figure:
  (a) 3D scatter of vortex positions colored by filament_id (first 5000 points)
  (b) Histogram of filament sizes
  (c) |J| at vortex cores histogram
  (d) Summary text panel
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
CSV_PATH = os.path.join(OUTPUT_DIR, 'ds_vortex_lines.csv')
OUT_PNG = os.path.join(OUTPUT_DIR, 'analyze_vortex_lines.png')

MAX_SCATTER = 5000  # limit scatter points to avoid overplotting


def main():
    plt = setup_style()
    from mpl_toolkits.mplot3d import Axes3D  # noqa: F401

    # --- Load data ---
    data = np.genfromtxt(CSV_PATH, delimiter=',', names=True)
    x = data['x']
    y = data['y']
    z = data['z']
    filament_id = data['filament_id'].astype(int)
    j_mag = data['J_mag']

    n_total = len(x)
    unique_filaments = np.unique(filament_id)
    n_filaments = len(unique_filaments)

    # Filament sizes
    fil_sizes = np.array([np.sum(filament_id == fid) for fid in unique_filaments])
    longest = np.max(fil_sizes) if len(fil_sizes) > 0 else 0
    mean_jmag = np.mean(j_mag)

    # --- Figure ---
    fig = plt.figure(figsize=(12, 10))
    fig.patch.set_facecolor(BG_COLOR)
    fig.suptitle('3D Vortex Filaments: Topology Invisible to Intensity Detectors',
                 color=ACCENT_COLOR, fontsize=13, fontweight='bold', y=0.97)

    # (a) 3D scatter (first MAX_SCATTER points)
    ax3d = fig.add_subplot(2, 2, 1, projection='3d')
    ax3d.set_facecolor(BG_COLOR)
    idx_sub = np.arange(min(MAX_SCATTER, n_total))
    # Color by filament_id with a discrete colormap
    cmap = plt.cm.get_cmap('tab20', min(n_filaments, 20))
    colors = cmap(filament_id[idx_sub] % 20)
    ax3d.scatter(x[idx_sub], y[idx_sub], z[idx_sub],
                 c=colors, s=2, alpha=0.6, depthshade=True)
    ax3d.set_xlabel('x', color=SUBTLE_COLOR, fontsize=7)
    ax3d.set_ylabel('y', color=SUBTLE_COLOR, fontsize=7)
    ax3d.set_zlabel('z', color=SUBTLE_COLOR, fontsize=7)
    ax3d.set_title(f'(a) Vortex positions (first {len(idx_sub):,})',
                   color=TEXT_COLOR, fontsize=9)
    ax3d.tick_params(colors=SUBTLE_COLOR, labelsize=6)
    ax3d.xaxis.pane.fill = False
    ax3d.yaxis.pane.fill = False
    ax3d.zaxis.pane.fill = False
    ax3d.xaxis.pane.set_edgecolor(GRID_COLOR)
    ax3d.yaxis.pane.set_edgecolor(GRID_COLOR)
    ax3d.zaxis.pane.set_edgecolor(GRID_COLOR)

    # (b) Histogram of filament sizes
    ax_hist = fig.add_subplot(2, 2, 2)
    if len(fil_sizes) > 0:
        ax_hist.hist(fil_sizes, bins=min(50, n_filaments),
                     color=ACCENT_COLOR, edgecolor='none', alpha=0.85)
    ax_hist.set_xlabel('Vortices per filament', color=TEXT_COLOR)
    ax_hist.set_ylabel('Count', color=TEXT_COLOR)
    ax_hist.set_title('(b) Filament size distribution', color=TEXT_COLOR, fontsize=9)

    # (c) |J| at vortex cores
    ax_jmag = fig.add_subplot(2, 2, 3)
    ax_jmag.hist(j_mag, bins=80, color='#ff6644', edgecolor='none', alpha=0.85)
    ax_jmag.axvline(mean_jmag, color=ACCENT_COLOR, ls='--', lw=1.5,
                    label=f'mean = {mean_jmag:.4f}')
    ax_jmag.set_xlabel('|J| at vortex core', color=TEXT_COLOR)
    ax_jmag.set_ylabel('Count', color=TEXT_COLOR)
    ax_jmag.set_title('(c) |J| at vortex cores (dark spots)',
                       color=TEXT_COLOR, fontsize=9)
    leg = ax_jmag.legend(fontsize=8, framealpha=0.5)
    for text in leg.get_texts():
        text.set_color(TEXT_COLOR)

    # (d) Summary text panel
    ax_text = fig.add_subplot(2, 2, 4)
    ax_text.set_xlim(0, 1)
    ax_text.set_ylim(0, 1)
    ax_text.axis('off')

    summary_lines = [
        f'Total vortex sites:   {n_total:,}',
        f'Number of filaments:  {n_filaments:,}',
        f'Longest filament:     {longest:,} voxels',
        f'Mean |J| at cores:    {mean_jmag:.6f}',
        '',
        'Vortex cores sit at |J| minima.',
        'Intensity detectors cannot see them.',
        'Phase information is required.',
    ]
    summary_text = '\n'.join(summary_lines)
    ax_text.text(0.1, 0.85, summary_text, transform=ax_text.transAxes,
                 va='top', ha='left', color=TEXT_COLOR, fontsize=11,
                 family='monospace', linespacing=1.6,
                 bbox=dict(boxstyle='round,pad=0.6', facecolor='#0d0d15',
                           edgecolor=GRID_COLOR))
    ax_text.set_title('(d) Summary statistics', color=TEXT_COLOR, fontsize=9)

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    fig.savefig(OUT_PNG, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f'Saved: {OUT_PNG}')


if __name__ == '__main__':
    main()
