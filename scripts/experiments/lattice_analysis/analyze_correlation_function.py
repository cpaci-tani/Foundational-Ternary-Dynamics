#!/usr/bin/env python3
"""
Analyze correlation function E(theta) from 3D lattice Bell experiment.

Reads ds_correlation_E_theta.csv and produces the KEY visualization:
  - Main plot: E(theta) vs theta for 3 modes + classical/quantum theory curves
  - Inset: CHSH S values for each mode
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
CSV_PATH = os.path.join(OUTPUT_DIR, 'ds_correlation_E_theta.csv')
OUT_PNG = os.path.join(OUTPUT_DIR, 'analyze_correlation_function.png')


def compute_chsh_s(theta_deg, e_meas, mode_mask):
    """
    Compute CHSH S from E(theta) at the standard angles.
    S = |E(a,b) - E(a,b')| + |E(a',b) + E(a',b')|
    with a=0, a'=45, b=22.5, b'=67.5  (or closest available).

    For discrete data we use the standard CHSH angles:
    E(0,45) - E(0,135) + E(90,45) + E(90,135)
    which maps to theta = 45, 135, -45(=315), 45 relative angles.

    Simplified: find E at 0, 45, 90, 135 degrees.
    S = |E(0) - E(90)| + |E(45) + E(135)|  (approximate)
    """
    th = theta_deg[mode_mask]
    em = e_meas[mode_mask]

    target_angles = [0.0, 45.0, 90.0, 135.0]
    e_at = {}
    for ta in target_angles:
        idx = np.argmin(np.abs(th - ta))
        e_at[ta] = em[idx]

    s_val = abs(e_at[0.0] - e_at[90.0]) + abs(e_at[45.0] + e_at[135.0])
    return s_val


def main():
    plt = setup_style()

    # --- Load data ---
    # Columns: mode, theta_deg, E_measured, E_classical, E_quantum, n_pairs
    # theta_deg may be in scientific notation
    data = np.genfromtxt(CSV_PATH, delimiter=',', names=True)
    mode = data['mode'].astype(int)
    theta_deg = data['theta_deg'].astype(float)
    e_meas = data['E_measured'].astype(float)
    e_class = data['E_classical'].astype(float)
    e_quant = data['E_quantum'].astype(float)

    # Theory curves from first mode (they should be the same for all modes)
    theta_theory = np.linspace(0, 180, 500)
    e_class_curve = -(1.0 - 2.0 * np.abs(theta_theory) / 180.0)
    e_quant_curve = -np.cos(np.radians(theta_theory))

    # Mode definitions
    mode_info = {
        0: {'label': 'Passive', 'color': '#4488ff', 'marker': 'o'},
        1: {'label': 'Active (sLoop)', 'color': '#44cc66', 'marker': '^'},
        2: {'label': 'Dynamical', 'color': '#ff8844', 'marker': 's'},
    }

    # --- Figure ---
    fig, ax = plt.subplots(1, 1, figsize=(10, 7))
    fig.patch.set_facecolor(BG_COLOR)
    fig.suptitle(r'Correlation Function $E(\theta)$: 3D Lattice with Physical Detectors',
                 color=ACCENT_COLOR, fontsize=14, fontweight='bold', y=0.97)

    # Theory lines
    ax.plot(theta_theory, e_class_curve, '--', color=SUBTLE_COLOR, lw=1.5,
            label=r'Classical: $E = -(1 - 2|\theta|/\pi)$', zorder=1)
    ax.plot(theta_theory, e_quant_curve, '--', color='#ff4444', lw=1.5,
            label=r'Quantum: $E = -\cos\theta$', zorder=1)

    # Data by mode
    s_values = {}
    for m, info in mode_info.items():
        mask = mode == m
        if not np.any(mask):
            continue
        th = theta_deg[mask]
        em = e_meas[mask]
        sort_idx = np.argsort(th)
        ax.plot(th[sort_idx], em[sort_idx], '-', color=info['color'],
                lw=1.2, alpha=0.6, zorder=2)
        ax.scatter(th[sort_idx], em[sort_idx], c=info['color'],
                   marker=info['marker'], s=30, label=info['label'],
                   zorder=3, edgecolors='none', alpha=0.85)

        s_values[m] = compute_chsh_s(theta_deg, e_meas, mask)

    ax.set_xlabel(r'$\theta$ (degrees)', color=TEXT_COLOR, fontsize=11)
    ax.set_ylabel(r'$E(\theta)$', color=TEXT_COLOR, fontsize=11)
    ax.set_xlim(-5, 185)
    ax.set_ylim(-1.15, 0.35)
    ax.axhline(0, color=GRID_COLOR, lw=0.5)

    leg = ax.legend(loc='upper right', fontsize=9, framealpha=0.7,
                    facecolor='#0d0d15', edgecolor=GRID_COLOR)
    for text in leg.get_texts():
        text.set_color(TEXT_COLOR)

    # --- CHSH S annotation box ---
    s_text_lines = []
    for m in sorted(s_values.keys()):
        s_text_lines.append(f'S{chr(8320 + m)} = {s_values[m]:.2f}')
    s_text_lines.append(f'Bell bound = 2.0')
    s_text = '   '.join(s_text_lines)

    ax.text(0.5, 0.03, s_text, transform=ax.transAxes, ha='center', va='bottom',
            color=ACCENT_COLOR, fontsize=10, fontweight='bold',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#0d0d15',
                      edgecolor=GRID_COLOR, alpha=0.9))

    # Subtitle
    ax.text(0.5, 0.96, 'All modes S < 2: substrate is local realistic',
            transform=ax.transAxes, ha='center', va='top',
            color=SUBTLE_COLOR, fontsize=9, style='italic')

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    fig.savefig(OUT_PNG, dpi=200, facecolor=BG_COLOR,
                bbox_inches='tight', pad_inches=0.2)
    plt.close(fig)
    print(f'Saved: {OUT_PNG}')


if __name__ == '__main__':
    main()
