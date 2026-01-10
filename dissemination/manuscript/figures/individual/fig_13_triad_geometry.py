"""
Figure 1.13: Triad Geometry
===========================
Shows the geometric configuration of triads - stable three-quark structures.

Two panels:
- Left: Proton (uud) - two up quarks, one down quark, net charge +1
- Right: Neutron (udd) - one up quark, two down quarks, net charge 0

Quarks arranged in equilateral triangles with pairwise distance √2.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, RegularPolygon
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def draw_quark(ax, pos, quark_type, charge):
    """Draw a quark as a colored circle with labels."""
    # Colors for quarks
    if quark_type == 'u':
        color = COLORS['matter']  # Red for up
        name = 'up'
    else:
        color = COLORS['antimatter']  # Blue for down
        name = 'down'

    # Draw quark circle
    circle = Circle(pos, 0.15, facecolor=color, edgecolor='black',
                   linewidth=2, zorder=5)
    ax.add_patch(circle)

    # Quark symbol
    ax.text(pos[0], pos[1], quark_type, fontsize=20, ha='center', va='center',
            fontweight='bold', color='white', zorder=6)

    # Charge label (outside circle)
    charge_offset = 0.25
    ax.text(pos[0], pos[1] + charge_offset, charge, fontsize=11, ha='center',
            fontweight='bold', color=color)


def draw_triad(ax, quarks, title, net_charge, net_charge_color):
    """
    Draw a triad configuration.

    Parameters:
    - quarks: list of (quark_type, charge) for each vertex
    - title: e.g., "Proton (uud)"
    - net_charge: e.g., "+1"
    """
    # Equilateral triangle positions
    angles = [np.pi/2, np.pi/2 + 2*np.pi/3, np.pi/2 + 4*np.pi/3]
    radius = 0.5
    positions = [(radius * np.cos(a), radius * np.sin(a)) for a in angles]

    # Draw binding lines (flux connections)
    for i in range(3):
        j = (i + 1) % 3
        ax.plot([positions[i][0], positions[j][0]],
               [positions[i][1], positions[j][1]],
               color=COLORS['accent1'], linewidth=3, linestyle='-',
               alpha=0.6, zorder=2)
        # Distance label
        mid_x = (positions[i][0] + positions[j][0]) / 2
        mid_y = (positions[i][1] + positions[j][1]) / 2

    # Draw quarks
    for pos, (quark_type, charge) in zip(positions, quarks):
        draw_quark(ax, pos, quark_type, charge)

    # Title
    ax.text(0, 0.95, title, fontsize=16, ha='center', fontweight='bold')

    # Distance annotation
    ax.text(0, -0.1, r'$d = \sqrt{2}$ voxels', fontsize=10, ha='center',
            style='italic', color='gray')

    # Net charge
    ax.text(0, -0.85, f'Net charge: {net_charge}', fontsize=14, ha='center',
            fontweight='bold', color=net_charge_color,
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                     edgecolor=net_charge_color, linewidth=2))

    # Charge calculation
    charge_calc = " + ".join([f"({c})" for _, c in quarks]) + f" = {net_charge}"
    ax.text(0, -1.05, charge_calc, fontsize=10, ha='center', color='gray')


def generate_triad_geometry():
    """
    Generate the proton and neutron triad geometry diagrams.

    Two panels showing equilateral triangle arrangements.
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 7), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Configure axes
    for ax in [ax1, ax2]:
        ax.set_xlim(-1.2, 1.2)
        ax.set_ylim(-1.3, 1.2)
        ax.set_aspect('equal')
        ax.axis('off')

    # =========================================================================
    # Proton (uud)
    # =========================================================================
    proton_quarks = [
        ('u', '+2/3'),  # Top
        ('u', '+2/3'),  # Bottom-left
        ('d', '-1/3'),  # Bottom-right
    ]
    draw_triad(ax1, proton_quarks, 'Proton (uud)', '+1', COLORS['matter'])

    # Add stability indicator
    ax1.text(0.8, 0.8, 'STABLE', fontsize=12, ha='center', fontweight='bold',
             color=COLORS['accent1'],
             bbox=dict(boxstyle='round,pad=0.2', facecolor='#d4ffd4'))

    # =========================================================================
    # Neutron (udd)
    # =========================================================================
    neutron_quarks = [
        ('u', '+2/3'),  # Top
        ('d', '-1/3'),  # Bottom-left
        ('d', '-1/3'),  # Bottom-right
    ]
    draw_triad(ax2, neutron_quarks, 'Neutron (udd)', '0', COLORS['void'])

    # Add stability indicator (neutrons decay outside nuclei)
    ax2.text(0.8, 0.8, 'METASTABLE', fontsize=10, ha='center', fontweight='bold',
             color=COLORS['accent2'],
             bbox=dict(boxstyle='round,pad=0.2', facecolor='#ffd4ff'))
    ax2.text(0.8, 0.6, r'$\tau \approx 15$ min', fontsize=9, ha='center',
             color='gray', style='italic')

    # =========================================================================
    # Overall title and legend
    # =========================================================================
    fig.suptitle('Triad Geometry: Quark Configurations', fontsize=18,
                fontweight='bold', y=0.98)

    # Legend
    legend_elements = [
        mpatches.Patch(facecolor=COLORS['matter'], edgecolor='black',
                       label='Up quark (u): +2/3'),
        mpatches.Patch(facecolor=COLORS['antimatter'], edgecolor='black',
                       label='Down quark (d): -1/3'),
        mpatches.Patch(facecolor=COLORS['accent1'], alpha=0.6,
                       label='Flux binding'),
    ]
    fig.legend(handles=legend_elements, loc='lower center', ncol=3,
               fontsize=10, bbox_to_anchor=(0.5, 0.02))

    # Explanation
    explanation = (
        "Triads are stable configurations of three quarks bound by flux exchange.\n"
        "The equilateral geometry minimizes energy while satisfying color neutrality."
    )
    fig.text(0.5, 0.08, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.12, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_triad_geometry()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_1_13_triad_geometry.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
