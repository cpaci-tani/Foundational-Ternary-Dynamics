"""
Figure 3.19: Crystal Defects
============================
Shows point defects, line defects, and grain boundaries
in crystalline structures from TRD perspective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_crystal_defects():
    """Generate the crystal defects visualization."""
    fig, axes = plt.subplots(1, 3, figsize=(15, 5), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Point defects
    ax = axes[0]
    ax.set_xlim(-0.5, 6.5)
    ax.set_ylim(-0.5, 6.5)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Point Defects', fontsize=12, fontweight='bold')

    # Regular lattice with defects
    for i in range(7):
        for j in range(7):
            if i == 2 and j == 3:
                # Vacancy
                ax.add_patch(Circle((i, j), 0.3, facecolor='white',
                                   edgecolor='gray', linestyle='--', linewidth=2))
            elif i == 4 and j == 4:
                # Interstitial (between lattice sites)
                ax.add_patch(Circle((i, j), 0.25, facecolor=COLORS['matter'],
                                   edgecolor='black', linewidth=1))
                ax.add_patch(Circle((i+0.4, j+0.4), 0.2, facecolor=COLORS['accent1'],
                                   edgecolor='black', linewidth=1))
            elif i == 5 and j == 2:
                # Substitutional impurity
                ax.add_patch(Circle((i, j), 0.28, facecolor=COLORS['antimatter'],
                                   edgecolor='black', linewidth=1))
            else:
                ax.add_patch(Circle((i, j), 0.25, facecolor=COLORS['matter'],
                                   edgecolor='black', linewidth=1))

    ax.annotate('Vacancy', xy=(2, 3), xytext=(0, 5),
               arrowprops=dict(arrowstyle='->', color='gray'),
               fontsize=9, ha='center')
    ax.annotate('Interstitial', xy=(4.4, 4.4), xytext=(5.5, 6),
               arrowprops=dict(arrowstyle='->', color='gray'),
               fontsize=9, ha='center')
    ax.annotate('Substitution', xy=(5, 2), xytext=(6.5, 0.5),
               arrowprops=dict(arrowstyle='->', color='gray'),
               fontsize=9, ha='center')

    # Panel 2: Edge dislocation
    ax = axes[1]
    ax.set_xlim(-0.5, 8.5)
    ax.set_ylim(-0.5, 6.5)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Edge Dislocation', fontsize=12, fontweight='bold')

    # Draw distorted lattice
    for j in range(7):
        for i in range(9):
            # Extra half-plane in upper portion
            if j >= 3 and i == 4:
                continue  # Gap for dislocation

            # Distortion near dislocation
            x_offset = 0
            y_offset = 0
            if j == 3 and i >= 4:
                x_offset = -0.15
            elif j == 2 and i >= 4:
                x_offset = -0.1
            elif j >= 4 and i == 3:
                x_offset = 0.1
            elif j >= 4 and i == 4:
                x_offset = 0.1

            ax.add_patch(Circle((i + x_offset, j + y_offset), 0.2,
                               facecolor=COLORS['matter'], edgecolor='black',
                               linewidth=0.5))

    # Draw the extra half-plane
    ax.plot([4, 4], [4, 6], 'r-', linewidth=3, label='Extra half-plane')
    ax.annotate('', xy=(4, 3.3), xytext=(4, 4),
               arrowprops=dict(arrowstyle='->', color='red', lw=2))
    ax.text(4, 6.3, 'Extra\nhalf-plane', fontsize=9, ha='center', color='red')

    # Burgers vector
    ax.annotate('', xy=(5, 3), xytext=(3, 3),
               arrowprops=dict(arrowstyle='->', color='blue', lw=2))
    ax.text(4, 2.5, 'Burgers vector b', fontsize=9, ha='center', color='blue')

    # Panel 3: Grain boundary
    ax = axes[2]
    ax.set_xlim(-0.5, 8.5)
    ax.set_ylim(-0.5, 6.5)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Grain Boundary', fontsize=12, fontweight='bold')

    # Left grain (angle 0)
    for i in range(4):
        for j in range(7):
            ax.add_patch(Circle((i, j), 0.2, facecolor=COLORS['matter'],
                               edgecolor='black', linewidth=0.5))

    # Right grain (rotated ~15 degrees)
    angle = np.radians(15)
    for i in range(4):
        for j in range(7):
            x = 4.5 + i * np.cos(angle) - j * 0.1 * np.sin(angle)
            y = j * np.cos(angle) + i * 0.1 * np.sin(angle)
            if 0 <= y <= 6.5 and x <= 8.5:
                ax.add_patch(Circle((x, y), 0.2, facecolor=COLORS['antimatter'],
                                   edgecolor='black', linewidth=0.5, alpha=0.8))

    # Grain boundary line
    ax.axvline(x=4, color='green', linewidth=2, linestyle='--')
    ax.text(4, -0.3, 'Grain\nboundary', fontsize=9, ha='center', color='green')

    # Add legend for grains
    ax.text(1.5, 6.3, 'Grain 1', fontsize=9, ha='center', color=COLORS['matter'])
    ax.text(6.5, 6.3, 'Grain 2', fontsize=9, ha='center', color=COLORS['antimatter'])

    fig.suptitle('Crystal Defects: TRD Perspective', fontsize=14, fontweight='bold', y=0.98)

    fig.text(0.5, 0.02, 'TRD: Defects are stable flux perturbations that modify local '
            'energy landscapes and enable material plasticity.',
            fontsize=10, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='gray'))

    plt.tight_layout(rect=[0, 0.08, 1, 0.92])
    return fig


if __name__ == '__main__':
    fig = generate_crystal_defects()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_19_crystal_defects.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
