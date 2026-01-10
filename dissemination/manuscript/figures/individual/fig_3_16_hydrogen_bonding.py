"""
Figure 3.16: Hydrogen Bonding
=============================
Visualizes hydrogen bonds in water molecules,
showing the flux field interactions.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_hydrogen_bonding():
    """Generate the hydrogen bonding visualization."""
    fig, ax = plt.subplots(figsize=(12, 8), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])
    ax.set_xlim(-1, 13)
    ax.set_ylim(-1, 9)
    ax.set_aspect('equal')
    ax.axis('off')

    def draw_water(ax, x, y, angle=0):
        """Draw a water molecule at position (x,y) with rotation angle."""
        # Oxygen (center)
        ax.add_patch(Circle((x, y), 0.4, facecolor='red', edgecolor='black',
                           linewidth=2))
        ax.text(x, y, 'O', fontsize=10, ha='center', va='center', color='white',
                fontweight='bold')

        # Hydrogens (at 104.5 degree angle)
        h_angle1 = np.radians(angle + 52.25)
        h_angle2 = np.radians(angle - 52.25)
        h_dist = 0.8

        h1_x = x + h_dist * np.cos(h_angle1)
        h1_y = y + h_dist * np.sin(h_angle1)
        h2_x = x + h_dist * np.cos(h_angle2)
        h2_y = y + h_dist * np.sin(h_angle2)

        ax.add_patch(Circle((h1_x, h1_y), 0.25, facecolor='white',
                           edgecolor='black', linewidth=1))
        ax.text(h1_x, h1_y, 'H', fontsize=8, ha='center', va='center')

        ax.add_patch(Circle((h2_x, h2_y), 0.25, facecolor='white',
                           edgecolor='black', linewidth=1))
        ax.text(h2_x, h2_y, 'H', fontsize=8, ha='center', va='center')

        # Covalent bonds
        ax.plot([x, h1_x], [y, h1_y], 'k-', linewidth=2)
        ax.plot([x, h2_x], [y, h2_y], 'k-', linewidth=2)

        return (h1_x, h1_y), (h2_x, h2_y)

    # Draw several water molecules
    molecules = [
        (3, 5, 0),
        (7, 5, 180),
        (3, 2, 90),
        (7, 2, -90),
        (10, 4, 45),
    ]

    h_positions = []
    o_positions = []

    for x, y, angle in molecules:
        h1, h2 = draw_water(ax, x, y, angle)
        h_positions.extend([h1, h2])
        o_positions.append((x, y))

    # Draw hydrogen bonds (dashed lines between H and O of different molecules)
    # H-bond 1: between molecule 0 and 1
    ax.plot([3.6, 6.2], [5, 5], 'b--', linewidth=2, alpha=0.7)
    ax.text(5, 5.3, 'H-bond', fontsize=9, ha='center', color='blue')

    # H-bond 2: between molecule 0 and 2
    ax.plot([3, 3], [4.2, 2.8], 'b--', linewidth=2, alpha=0.7)

    # H-bond 3: between molecule 1 and 3
    ax.plot([7, 7], [4.2, 2.8], 'b--', linewidth=2, alpha=0.7)

    # Title
    ax.text(6, 8.5, 'Hydrogen Bonding in Water', fontsize=14, fontweight='bold',
            ha='center')

    # Explanation
    ax.text(6, 7.5, 'Dashed lines: H-bonds (~20 kJ/mol, weaker than covalent ~400 kJ/mol)',
            fontsize=10, ha='center')

    # TRD interpretation box
    trd_text = (
        "TRD Interpretation:\n\n"
        "H-bond = flux field attraction\n"
        "between partial charges:\n"
        "- O has excess electron density\n"
        "- H has electron deficit\n"
        "Flux gradient creates attraction"
    )
    ax.text(11, 2, trd_text, fontsize=9, ha='left',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     edgecolor='blue', linewidth=2))

    return fig


if __name__ == '__main__':
    fig = generate_hydrogen_bonding()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_16_hydrogen_bonding.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
