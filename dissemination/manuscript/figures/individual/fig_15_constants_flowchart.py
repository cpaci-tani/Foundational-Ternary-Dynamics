"""
Figure 1.15: Constants Derivation Flowchart
===========================================
Hierarchical flowchart showing how all physical constants
derive from four framework integers.

Hierarchy:
Level 0: Framework Integers {b₃=7, N_c=3, n_eff=13, N_base=4}
Level 1: Lemniscatic constant G* = 2.9587
Level 2: Master Quadratic → α = 1/137.036, N_c = 3.024
Level 3: Derived constants (KB, φ, gravity, sin²θ_W, α_s)
Level 4: Particle masses
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Rectangle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style
from utils.physics_constants import (
    B3, N_C, N_EFF, N_BASE, G_STAR, ALPHA, ALPHA_INV,
    KB, PHI, GRAVITY_BIAS, SIN2_THETA_W, ALPHA_S
)


def draw_box(ax, x, y, width, height, text, color, fontsize=10, multiline=False):
    """Draw a rounded box with text."""
    box = FancyBboxPatch(
        (x - width/2, y - height/2), width, height,
        boxstyle="round,pad=0.02,rounding_size=0.02",
        facecolor=color,
        edgecolor='black',
        linewidth=2,
        zorder=5
    )
    ax.add_patch(box)

    if multiline:
        lines = text.split('\n')
        for i, line in enumerate(lines):
            offset = (len(lines) - 1) / 2 - i
            ax.text(x, y + offset * 0.06, line, fontsize=fontsize,
                    ha='center', va='center', fontweight='bold', zorder=6)
    else:
        ax.text(x, y, text, fontsize=fontsize, ha='center', va='center',
                fontweight='bold', zorder=6)

    return (x, y - height/2)  # Return bottom center for arrow connection


def draw_arrow(ax, start, end, color='gray'):
    """Draw an arrow from start to end."""
    ax.annotate('', xy=end, xytext=start,
               arrowprops=dict(arrowstyle='->', color=color, lw=2,
                              connectionstyle='arc3,rad=0'))


def generate_constants_flowchart():
    """
    Generate the constants derivation flowchart.

    Shows the hierarchical derivation from 4 integers
    to all Standard Model constants.
    """
    fig, ax = plt.subplots(figsize=(14, 16), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.0)
    ax.set_aspect('equal')
    ax.axis('off')

    # Color scheme for levels
    level_colors = {
        0: '#FFE66D',  # Yellow - inputs
        1: '#4ECDC4',  # Teal - lemniscatic constant
        2: '#FF6B6B',  # Coral - primary derived
        3: '#95E1D3',  # Mint - secondary derived
        4: '#DCD6F7',  # Lavender - particle masses
    }

    # =========================================================================
    # Level 0: Framework Integers
    # =========================================================================
    y0 = 0.85
    integers = [
        (f'$b_3 = {B3}$', -0.9),
        (f'$N_c = {N_C}$', -0.3),
        (f'$n_{{eff}} = {N_EFF}$', 0.3),
        (f'$N_{{base}} = {N_BASE}$', 0.9),
    ]

    # Title for level 0
    ax.text(0, y0 + 0.12, 'Framework Integers (The Only Inputs)', fontsize=12,
            ha='center', fontweight='bold', color='gray')

    level0_bottoms = []
    for text, x in integers:
        draw_box(ax, x, y0, 0.35, 0.12, text, level_colors[0], fontsize=11)
        level0_bottoms.append((x, y0 - 0.06))

    # =========================================================================
    # Level 1: Lemniscatic Constant
    # =========================================================================
    y1 = 0.55
    draw_box(ax, 0, y1, 0.8, 0.15,
             f'Lemniscate-Alpha Curve\n$G^* = {G_STAR:.4f}$',
             level_colors[1], fontsize=11, multiline=True)

    # Arrows from level 0 to level 1
    for x, _ in level0_bottoms:
        draw_arrow(ax, (x, y0 - 0.06), (0, y1 + 0.08))

    # =========================================================================
    # Level 2: Master Quadratic Outputs
    # =========================================================================
    y2 = 0.25
    ax.text(0, y2 + 0.12, 'Master Quadratic: $x^2 - 16G^{*2}x + 16G^{*3} = 0$',
            fontsize=10, ha='center', style='italic', color='gray')

    draw_box(ax, -0.5, y2, 0.6, 0.12,
             f'$\\alpha = 1/{ALPHA_INV:.3f}$',
             level_colors[2], fontsize=11)

    draw_box(ax, 0.5, y2, 0.6, 0.12,
             f'$N_c = 3.024 \\to 3$',
             level_colors[2], fontsize=11)

    # Arrows from level 1 to level 2
    draw_arrow(ax, (0, y1 - 0.08), (-0.5, y2 + 0.06))
    draw_arrow(ax, (0, y1 - 0.08), (0.5, y2 + 0.06))

    # =========================================================================
    # Level 3: Derived Constants
    # =========================================================================
    y3 = -0.1
    ax.text(0, y3 + 0.18, 'Derived Constants', fontsize=11,
            ha='center', fontweight='bold', color='gray')

    constants = [
        (f'$K_B = {KB:.3f}$\n(electron mass)', -1.1),
        (f'$\\phi = {PHI:.3f}$\n(golden ratio)', -0.55),
        (f'$G = {GRAVITY_BIAS:.2f}$\n(gravity)', 0),
        (f'$\\sin^2\\theta_W = 3/13$\n$= {SIN2_THETA_W:.4f}$', 0.55),
        (f'$\\alpha_s = 7/59$\n$= {ALPHA_S:.4f}$', 1.1),
    ]

    for text, x in constants:
        draw_box(ax, x, y3, 0.4, 0.18, text, level_colors[3], fontsize=9, multiline=True)

    # Arrows from level 2 to level 3
    for text, x in constants:
        if x < 0:
            draw_arrow(ax, (-0.5, y2 - 0.06), (x, y3 + 0.09))
        else:
            draw_arrow(ax, (0.5, y2 - 0.06), (x, y3 + 0.09))

    # =========================================================================
    # Level 4: Particle Masses
    # =========================================================================
    y4 = -0.5
    ax.text(0, y4 + 0.15, 'All Standard Model Particle Masses', fontsize=11,
            ha='center', fontweight='bold', color='gray')

    particles = [
        'Leptons\n(e, μ, τ)',
        'Quarks\n(u,d,s,c,b,t)',
        'Bosons\n(W, Z, H)',
        'Baryons\n(p, n)',
    ]
    x_positions = [-0.9, -0.3, 0.3, 0.9]

    for text, x in zip(particles, x_positions):
        draw_box(ax, x, y4, 0.4, 0.15, text, level_colors[4], fontsize=9, multiline=True)

    # Arrows from level 3 to level 4
    for x in x_positions:
        nearest_source = min([-1.1, -0.55, 0, 0.55, 1.1], key=lambda s: abs(s - x))
        draw_arrow(ax, (nearest_source, y3 - 0.09), (x, y4 + 0.08))

    # =========================================================================
    # Summary box
    # =========================================================================
    y5 = -0.85
    summary_box = FancyBboxPatch(
        (-1.2, y5 - 0.12), 2.4, 0.24,
        boxstyle="round,pad=0.03",
        facecolor='white',
        edgecolor=COLORS['highlight'],
        linewidth=3,
        zorder=5
    )
    ax.add_patch(summary_box)

    ax.text(0, y5, 'All 15 SM particle masses derived with < 0.5% average error',
            fontsize=12, ha='center', va='center', fontweight='bold',
            color=COLORS['highlight'], zorder=6)

    # Arrows to summary
    for x in x_positions:
        draw_arrow(ax, (x, y4 - 0.08), (x * 0.5, y5 + 0.12))

    # =========================================================================
    # Legend
    # =========================================================================
    legend_elements = [
        mpatches.Patch(facecolor=level_colors[0], edgecolor='black', label='Input integers'),
        mpatches.Patch(facecolor=level_colors[1], edgecolor='black', label='Lemniscatic constant'),
        mpatches.Patch(facecolor=level_colors[2], edgecolor='black', label='Primary derived'),
        mpatches.Patch(facecolor=level_colors[3], edgecolor='black', label='Secondary derived'),
        mpatches.Patch(facecolor=level_colors[4], edgecolor='black', label='Particle masses'),
    ]
    ax.legend(handles=legend_elements, loc='lower left',
              fontsize=9, bbox_to_anchor=(-0.1, -1.1), ncol=3)

    # Title
    fig.suptitle('Constants Derivation Flowchart',
                fontsize=18, fontweight='bold', y=0.98)

    # Key insight
    ax.text(0, -1.3, 'From 4 integers to all of physics',
            fontsize=14, ha='center', style='italic', color='gray')

    plt.tight_layout(rect=[0, 0, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_constants_flowchart()
    output_path = Path(__file__).parent.parent / 'ch14' / 'fig_1_15_constants_flowchart.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
