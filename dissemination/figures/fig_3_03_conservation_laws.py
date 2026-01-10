"""
Figure 3.3: Conservation Laws
=============================
Diagram showing which quantities are conserved in TRD
and how conservation emerges from the update rules.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Circle, FancyArrowPatch
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_conservation_laws():
    """Generate the conservation laws visualization."""
    fig, ax = plt.subplots(figsize=(12, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 10)
    ax.axis('off')

    # Title
    ax.text(6, 9.5, 'Conservation Laws in TRD', fontsize=16, fontweight='bold',
            ha='center')

    # Define conserved quantities
    conserved = [
        ('Total Flux', 'Closed system maintains\nconstant total |J|', '#4ECDC4', True),
        ('Net Charge', 'Sum of all state values\nremains constant', '#FF6B6B', True),
        ('Momentum', 'Vector sum of all\nflux × velocity', '#FFE66D', True),
        ('Particle Number', 'Genesis/annihilation\nbalanced in pairs', '#95E1D3', True),
        ('Energy', 'Kinetic + potential\n(approximate)', '#DCD6F7', True),
    ]

    not_conserved = [
        ('Entropy', 'Always increases\n(second law)', '#AEB6BF'),
        ('Local Density', 'Can concentrate\nor disperse', '#AEB6BF'),
    ]

    # Draw conserved quantities
    ax.text(3, 8.5, 'CONSERVED', fontsize=12, fontweight='bold', ha='center',
            color='green')

    for i, (name, desc, color, _) in enumerate(conserved):
        y = 7.5 - i * 1.4
        box = FancyBboxPatch((0.5, y - 0.5), 5, 1,
                            boxstyle="round,pad=0.02",
                            facecolor=color, edgecolor='green', linewidth=2)
        ax.add_patch(box)
        ax.text(1, y, name, fontsize=11, fontweight='bold', va='center')
        ax.text(3, y, desc, fontsize=9, va='center')

    # Draw not conserved quantities
    ax.text(9, 8.5, 'NOT CONSERVED', fontsize=12, fontweight='bold', ha='center',
            color='gray')

    for i, (name, desc, color) in enumerate(not_conserved):
        y = 7.5 - i * 1.4
        box = FancyBboxPatch((6.5, y - 0.5), 5, 1,
                            boxstyle="round,pad=0.02",
                            facecolor=color, edgecolor='gray', linewidth=2)
        ax.add_patch(box)
        ax.text(7, y, name, fontsize=11, fontweight='bold', va='center')
        ax.text(9, y, desc, fontsize=9, va='center')

    # Example: charge conservation in annihilation
    ax.text(6, 1.8, 'Example: Charge Conservation in Annihilation', fontsize=11,
            fontweight='bold', ha='center')

    # Before
    ax.text(2.5, 1.2, 'Before:', fontsize=10, ha='center')
    ax.add_patch(Circle((1.5, 0.7), 0.25, facecolor=COLORS['matter'], edgecolor='black'))
    ax.text(1.5, 0.7, '+', fontsize=12, ha='center', va='center', color='white')
    ax.add_patch(Circle((3.5, 0.7), 0.25, facecolor=COLORS['antimatter'], edgecolor='black'))
    ax.text(3.5, 0.7, '-', fontsize=12, ha='center', va='center', color='white')
    ax.text(2.5, 0.2, 'Net: +1 + (-1) = 0', fontsize=9, ha='center')

    # Arrow
    ax.annotate('', xy=(5.5, 0.7), xytext=(4.5, 0.7),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))

    # After
    ax.text(7.5, 1.2, 'After:', fontsize=10, ha='center')
    ax.add_patch(Circle((6.5, 0.7), 0.25, facecolor=COLORS['void'], edgecolor='black'))
    ax.text(6.5, 0.7, '0', fontsize=10, ha='center', va='center')
    ax.add_patch(Circle((8.5, 0.7), 0.25, facecolor=COLORS['void'], edgecolor='black'))
    ax.text(8.5, 0.7, '0', fontsize=10, ha='center', va='center')
    ax.text(7.5, 0.2, 'Net: 0 + 0 = 0', fontsize=9, ha='center')

    ax.text(10.5, 0.7, 'Conserved!', fontsize=10, color='green', fontweight='bold',
            va='center')

    return fig


if __name__ == '__main__':
    fig = generate_conservation_laws()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_3_3_conservation_laws.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
