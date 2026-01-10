"""
Figure 3.15: Metallic Bonding
=============================
Visualizes metallic bonding as delocalized electron sea
shared among positive ion cores.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_metallic_bonding():
    """Generate the metallic bonding visualization."""
    fig, ax = plt.subplots(figsize=(12, 8), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])
    ax.set_xlim(-1, 11)
    ax.set_ylim(-1, 8)
    ax.set_aspect('equal')
    ax.axis('off')

    # Draw electron sea background
    sea = Rectangle((-0.5, -0.5), 11, 8, facecolor=COLORS['antimatter'],
                   edgecolor='black', linewidth=2, alpha=0.2)
    ax.add_patch(sea)

    # Draw positive ion cores in lattice
    np.random.seed(42)
    for i in range(5):
        for j in range(4):
            x = 1 + i * 2
            y = 1 + j * 2

            # Ion core (positive)
            ax.add_patch(Circle((x, y), 0.5, facecolor=COLORS['matter'],
                               edgecolor='black', linewidth=2))
            ax.text(x, y, '+', fontsize=14, ha='center', va='center',
                    color='white', fontweight='bold')

    # Draw some free electrons
    for _ in range(25):
        x = np.random.uniform(0, 10)
        y = np.random.uniform(0, 7)

        # Avoid overlap with ion cores
        valid = True
        for i in range(5):
            for j in range(4):
                ix, iy = 1 + i*2, 1 + j*2
                if np.sqrt((x-ix)**2 + (y-iy)**2) < 0.8:
                    valid = False
                    break

        if valid:
            ax.add_patch(Circle((x, y), 0.15, facecolor=COLORS['antimatter'],
                               edgecolor='black', linewidth=0.5, alpha=0.8))

    # Labels
    ax.text(5, 7.5, 'Metallic Bond: Electron Sea Model', fontsize=14,
            fontweight='bold', ha='center')

    ax.text(5, -0.7, 'Delocalized electrons (blue) shared among positive ion cores (red)',
            fontsize=10, ha='center')

    # Properties box
    props = (
        "Properties from delocalized electrons:\n"
        "- Electrical conductivity\n"
        "- Thermal conductivity\n"
        "- Malleability\n"
        "- Metallic luster"
    )
    ax.text(11.5, 4, props, fontsize=9, va='center',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     edgecolor='gray'))

    return fig


if __name__ == '__main__':
    fig = generate_metallic_bonding()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_15_metallic_bonding.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
