"""
Figure 3.2: Boundary Conditions
===============================
Visualizes different boundary condition types:
toroidal (periodic), absorbing, and reflective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, FancyArrowPatch, Circle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_boundary_conditions():
    """Generate the boundary conditions visualization."""
    fig, axes = plt.subplots(1, 3, figsize=(14, 5), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    titles = ['Toroidal (Periodic)', 'Absorbing', 'Reflective']

    for ax, title in zip(axes, titles):
        ax.set_xlim(-0.5, 5.5)
        ax.set_ylim(-0.5, 5.5)
        ax.set_aspect('equal')
        ax.axis('off')

        # Draw lattice boundary
        boundary = Rectangle((0, 0), 5, 5, facecolor='#f0f0f0',
                            edgecolor='black', linewidth=3)
        ax.add_patch(boundary)

        # Draw lattice grid
        for i in range(6):
            ax.axhline(y=i, xmin=0, xmax=1, color='gray', linewidth=0.5, alpha=0.5)
            ax.axvline(x=i, ymin=0, ymax=1, color='gray', linewidth=0.5, alpha=0.5)

        ax.set_title(title, fontsize=12, fontweight='bold', pad=10)

    # Toroidal (left)
    ax = axes[0]
    # Particle moving right, wraps to left
    ax.add_patch(Circle((4.5, 2.5), 0.2, facecolor=COLORS['matter'], edgecolor='black'))
    ax.annotate('', xy=(5.3, 2.5), xytext=(4.7, 2.5),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))
    # Ghost appearing on left
    ax.add_patch(Circle((0.5, 2.5), 0.2, facecolor=COLORS['matter'], edgecolor='black',
                        alpha=0.5, linestyle='--'))
    ax.annotate('', xy=(0.3, 2.5), xytext=(-0.3, 2.5),
               arrowprops=dict(arrowstyle='->', color='gray', lw=2, linestyle='--'))
    ax.text(2.5, -0.3, 'Exits right -> enters left', fontsize=9, ha='center')

    # Absorbing (middle)
    ax = axes[1]
    # Particle at boundary, disappearing
    ax.add_patch(Circle((4.5, 2.5), 0.2, facecolor=COLORS['matter'], edgecolor='black'))
    ax.annotate('', xy=(5.3, 2.5), xytext=(4.7, 2.5),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))
    # X mark showing absorption
    ax.text(5.5, 2.5, 'X', fontsize=20, color='red', ha='center', va='center')
    ax.text(2.5, -0.3, 'Exits boundary -> removed', fontsize=9, ha='center')

    # Reflective (right)
    ax = axes[2]
    # Particle at boundary, bouncing back
    ax.add_patch(Circle((4.3, 2.5), 0.2, facecolor=COLORS['matter'], edgecolor='black'))
    ax.annotate('', xy=(4.8, 2.5), xytext=(4.5, 2.5),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))
    ax.annotate('', xy=(4.0, 2.5), xytext=(4.3, 2.5),
               arrowprops=dict(arrowstyle='->', color='blue', lw=2))
    ax.plot([5, 5], [1.5, 3.5], color='blue', linewidth=3)
    ax.text(2.5, -0.3, 'Hits boundary -> reverses', fontsize=9, ha='center')

    fig.suptitle('Boundary Conditions for Finite Lattice', fontsize=14, fontweight='bold')

    plt.tight_layout()
    return fig


if __name__ == '__main__':
    fig = generate_boundary_conditions()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_3_2_boundary_conditions.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
