"""
Figure 1.6: Vector Addition / Interference
==========================================
Demonstrates constructive and destructive interference through
vector addition of flux fields.

Shows:
- Left panel: Constructive interference (aligned vectors)
- Right panel: Destructive interference (opposing vectors)
- Intensity formula: I = |J_A + J_B|²
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyArrowPatch
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def draw_vector(ax, start, vec, color, label=None, width=0.03):
    """Draw a vector arrow from start point."""
    ax.annotate('', xy=(start[0] + vec[0], start[1] + vec[1]),
                xytext=start,
                arrowprops=dict(arrowstyle='->', color=color, lw=3,
                               mutation_scale=20))
    if label:
        mid = (start[0] + vec[0]/2, start[1] + vec[1]/2 + 0.15)
        ax.text(mid[0], mid[1], label, fontsize=12, fontweight='bold',
                color=color, ha='center', va='bottom')


def generate_interference():
    """
    Generate the vector interference visualization.

    Two panels showing constructive and destructive interference
    with vector diagrams and resulting intensity calculations.
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Common parameters
    origin = (0.5, 0.3)
    vec_length = 0.8

    # =========================================================================
    # Panel 1: Constructive Interference
    # =========================================================================
    ax1.set_xlim(0, 3)
    ax1.set_ylim(0, 2)
    ax1.set_aspect('equal')
    ax1.axis('off')

    # Vector J_A (pointing right)
    J_A = (vec_length, 0)
    draw_vector(ax1, origin, J_A, COLORS['matter'], r'$\mathbf{J}_A$')

    # Vector J_B (also pointing right, head-to-tail)
    J_B_start = (origin[0] + J_A[0], origin[1])
    J_B = (vec_length, 0)
    draw_vector(ax1, J_B_start, J_B, COLORS['antimatter'], r'$\mathbf{J}_B$')

    # Combined vector (from origin to final point)
    combined_end = (origin[0] + J_A[0] + J_B[0], origin[1])
    ax1.annotate('', xy=combined_end, xytext=(origin[0], origin[1] - 0.15),
                arrowprops=dict(arrowstyle='->', color=COLORS['accent1'], lw=4,
                               mutation_scale=25))
    ax1.text((origin[0] + combined_end[0])/2, origin[1] - 0.35,
             r'$\mathbf{J}_A + \mathbf{J}_B$', fontsize=12, fontweight='bold',
             color=COLORS['accent1'], ha='center')

    # Intensity calculation
    ax1.text(1.5, 1.5, r'$I = |\mathbf{J}_A + \mathbf{J}_B|^2$',
             fontsize=14, ha='center', fontweight='bold')
    ax1.text(1.5, 1.2, r'$= |J_A|^2 + |J_B|^2 + 2J_A \cdot J_B$',
             fontsize=12, ha='center')
    ax1.text(1.5, 0.9, r'$= (|J_A| + |J_B|)^2$',
             fontsize=12, ha='center', color=COLORS['accent1'])
    ax1.text(1.5, 0.6, r'(when $\mathbf{J}_A \parallel \mathbf{J}_B$)',
             fontsize=10, ha='center', style='italic')

    ax1.set_title('Constructive Interference', fontsize=14, fontweight='bold',
                  color=COLORS['accent1'], pad=10)

    # Add "BRIGHT" label
    ax1.text(2.7, 0.3, 'BRIGHT', fontsize=16, fontweight='bold',
             color=COLORS['accent1'], ha='center', va='center',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='#d4ffd4', edgecolor=COLORS['accent1']))

    # =========================================================================
    # Panel 2: Destructive Interference
    # =========================================================================
    ax2.set_xlim(0, 3)
    ax2.set_ylim(0, 2)
    ax2.set_aspect('equal')
    ax2.axis('off')

    # Vector J_A (pointing right)
    origin2 = (0.8, 0.5)
    draw_vector(ax2, origin2, J_A, COLORS['matter'], r'$\mathbf{J}_A$')

    # Vector J_B (pointing left, opposite direction)
    J_B_start2 = (origin2[0] + J_A[0], origin2[1])
    J_B_opposite = (-vec_length, 0)
    draw_vector(ax2, J_B_start2, J_B_opposite, COLORS['antimatter'], r'$\mathbf{J}_B$')

    # Combined vector (effectively zero)
    ax2.plot([origin2[0], origin2[0]], [origin2[1] - 0.15, origin2[1] - 0.15],
             'o', color=COLORS['void'], markersize=15, zorder=5)
    ax2.text(origin2[0], origin2[1] - 0.35,
             r'$\mathbf{J}_A + \mathbf{J}_B \approx 0$', fontsize=12, fontweight='bold',
             color=COLORS['void'], ha='center')

    # Intensity calculation
    ax2.text(1.5, 1.5, r'$I = |\mathbf{J}_A + \mathbf{J}_B|^2$',
             fontsize=14, ha='center', fontweight='bold')
    ax2.text(1.5, 1.2, r'$= |J_A|^2 + |J_B|^2 - 2|J_A||J_B|$',
             fontsize=12, ha='center')
    ax2.text(1.5, 0.9, r'$= (|J_A| - |J_B|)^2 \approx 0$',
             fontsize=12, ha='center', color=COLORS['void'])
    ax2.text(1.5, 0.6, r'(when $\mathbf{J}_A$ opposite to $\mathbf{J}_B$)',
             fontsize=10, ha='center', style='italic')

    ax2.set_title('Destructive Interference', fontsize=14, fontweight='bold',
                  color=COLORS['void'], pad=10)

    # Add "DARK" label
    ax2.text(2.7, 0.5, 'DARK', fontsize=16, fontweight='bold',
             color=COLORS['void'], ha='center', va='center',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='#e0e0e0', edgecolor=COLORS['void']))

    # =========================================================================
    # Add overall title and explanation
    # =========================================================================
    fig.suptitle('Vector Addition Determines Intensity', fontsize=16, fontweight='bold', y=0.98)

    # Explanation box
    explanation = (
        "In TRD, flux fields are vectors.\n"
        "Intensity (observability) depends on |J|².\n"
        "Wave interference is simply vector addition."
    )
    fig.text(0.5, 0.02, explanation, ha='center', va='bottom', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white', edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.08, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_interference()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_1_6_interference.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
