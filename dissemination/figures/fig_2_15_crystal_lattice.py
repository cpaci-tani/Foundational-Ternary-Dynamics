"""
Figure 2.15: Crystal Lattice Structure
======================================
Visualizes how periodic crystal structures emerge from
repeated flux equilibria in TRD.

Shows various lattice types and their flux field patterns.
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from matplotlib.patches import Circle, Rectangle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_crystal_lattice():
    """
    Generate the crystal lattice visualization.

    Shows:
    1. Left: 3D cubic lattice
    2. Right: 2D lattice with flux field overlay
    """
    fig = plt.figure(figsize=(14, 7), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # 3D subplot
    ax3d = fig.add_subplot(121, projection='3d')
    # 2D subplot
    ax2d = fig.add_subplot(122)

    # =========================================================================
    # Left: 3D Cubic Lattice
    # =========================================================================
    # Draw atoms at lattice points
    n = 3  # 3x3x3 lattice
    spacing = 1.0

    # Two types of atoms for NaCl structure
    for i in range(n):
        for j in range(n):
            for k in range(n):
                x, y, z = i * spacing, j * spacing, k * spacing

                # Alternating atom types
                if (i + j + k) % 2 == 0:
                    color = COLORS['matter']
                    size = 80
                else:
                    color = COLORS['accent1']
                    size = 100

                ax3d.scatter(x, y, z, c=color, s=size, edgecolors='black',
                            linewidth=0.5)

    # Draw bonds (nearest neighbors)
    for i in range(n):
        for j in range(n):
            for k in range(n):
                x, y, z = i * spacing, j * spacing, k * spacing

                # Connect to neighbors
                if i < n - 1:
                    ax3d.plot([x, x+spacing], [y, y], [z, z], 'k-',
                             linewidth=0.5, alpha=0.5)
                if j < n - 1:
                    ax3d.plot([x, x], [y, y+spacing], [z, z], 'k-',
                             linewidth=0.5, alpha=0.5)
                if k < n - 1:
                    ax3d.plot([x, x], [y, y], [z, z+spacing], 'k-',
                             linewidth=0.5, alpha=0.5)

    ax3d.set_xlabel('x')
    ax3d.set_ylabel('y')
    ax3d.set_zlabel('z')
    ax3d.set_title('3D Crystal Lattice\n(NaCl-type structure)',
                   fontsize=13, fontweight='bold')

    # Set viewing angle
    ax3d.view_init(elev=20, azim=45)
    ax3d.set_box_aspect([1, 1, 1])

    # =========================================================================
    # Right: 2D Lattice with Flux Field
    # =========================================================================
    ax2d.set_xlim(-0.5, 4.5)
    ax2d.set_ylim(-0.5, 4.5)
    ax2d.set_aspect('equal')

    # Create flux field visualization
    x_grid = np.linspace(-0.5, 4.5, 50)
    y_grid = np.linspace(-0.5, 4.5, 50)
    X, Y = np.meshgrid(x_grid, y_grid)

    # Calculate flux from all atoms
    flux = np.zeros_like(X)

    for i in range(5):
        for j in range(5):
            # Distance from each atom
            r = np.sqrt((X - i)**2 + (Y - j)**2)
            r = np.maximum(r, 0.1)  # Avoid division by zero

            # Alternating positive/negative contributions
            sign = 1 if (i + j) % 2 == 0 else -1
            flux += sign / r**2

    # Plot flux field as contours
    levels = np.linspace(-5, 5, 21)
    contour = ax2d.contourf(X, Y, flux, levels=levels, cmap='RdBu_r', alpha=0.4)
    ax2d.contour(X, Y, flux, levels=[0], colors='black', linewidths=1)

    # Draw atoms
    for i in range(5):
        for j in range(5):
            if (i + j) % 2 == 0:
                color = COLORS['matter']
                size = 0.15
            else:
                color = COLORS['accent1']
                size = 0.18

            atom = Circle((i, j), size, facecolor=color,
                         edgecolor='black', linewidth=1, zorder=5)
            ax2d.add_patch(atom)

    ax2d.set_xlabel('x (lattice units)')
    ax2d.set_ylabel('y (lattice units)')
    ax2d.set_title('2D Cross-Section with Flux Field\n(Red/Blue = +/- flux density)',
                   fontsize=13, fontweight='bold')

    # Add colorbar
    cbar = plt.colorbar(contour, ax=ax2d, shrink=0.8)
    cbar.set_label('Net Flux Density', fontsize=10)

    # Legend
    legend_elements = [
        mpatches.Patch(facecolor=COLORS['matter'], edgecolor='black',
                      label='Cation (e.g., Na+)'),
        mpatches.Patch(facecolor=COLORS['accent1'], edgecolor='black',
                      label='Anion (e.g., Cl-)'),
    ]
    ax2d.legend(handles=legend_elements, loc='upper right', fontsize=9)

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Crystal Lattice: Periodic Flux Equilibrium Creates Order',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "Crystal structures emerge when flux field equilibria repeat periodically.\n"
        "Each atom sits at a flux minimum, with neighbors arranged to minimize total energy."
    )
    fig.text(0.5, 0.02, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.06, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_crystal_lattice()
    output_path = Path(__file__).parent.parent / 'ch04' / 'fig_2_15_crystal_lattice.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
