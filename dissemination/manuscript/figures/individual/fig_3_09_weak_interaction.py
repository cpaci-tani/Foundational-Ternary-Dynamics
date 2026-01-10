"""
Figure 3.9: Weak Interaction Mechanism
======================================
Shows how the weak force causes flavor changes (transmutation)
when field stress exceeds a threshold.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_weak_interaction():
    """Generate the weak interaction mechanism visualization."""
    fig, axes = plt.subplots(1, 2, figsize=(14, 6), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Left: Beta decay process
    ax = axes[0]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 8)
    ax.axis('off')
    ax.set_title('Beta Decay: Neutron -> Proton', fontsize=12, fontweight='bold')

    # Before (neutron = udd)
    ax.text(2, 7, 'Before:', fontsize=11, fontweight='bold')
    ax.text(2, 6.3, 'Neutron (udd)', fontsize=10)

    # Draw three quarks
    ax.add_patch(Circle((1, 5), 0.3, facecolor='#FF6B6B', edgecolor='black'))
    ax.text(1, 5, 'u', fontsize=10, ha='center', va='center', color='white')
    ax.add_patch(Circle((2, 5), 0.3, facecolor='#4ECDC4', edgecolor='black'))
    ax.text(2, 5, 'd', fontsize=10, ha='center', va='center', color='white')
    ax.add_patch(Circle((3, 5), 0.3, facecolor='#4ECDC4', edgecolor='black'))
    ax.text(3, 5, 'd', fontsize=10, ha='center', va='center', color='white')

    # W boson emission
    ax.annotate('', xy=(5, 3.5), xytext=(3.3, 4.7),
               arrowprops=dict(arrowstyle='->', color='purple', lw=2,
                              connectionstyle='arc3,rad=-0.3'))
    ax.text(4.5, 4.2, 'W-', fontsize=11, color='purple', fontweight='bold')

    # After (proton = uud)
    ax.text(7, 7, 'After:', fontsize=11, fontweight='bold')
    ax.text(7, 6.3, 'Proton (uud)', fontsize=10)

    ax.add_patch(Circle((6, 5), 0.3, facecolor='#FF6B6B', edgecolor='black'))
    ax.text(6, 5, 'u', fontsize=10, ha='center', va='center', color='white')
    ax.add_patch(Circle((7, 5), 0.3, facecolor='#FF6B6B', edgecolor='black'))
    ax.text(7, 5, 'u', fontsize=10, ha='center', va='center', color='white')
    ax.add_patch(Circle((8, 5), 0.3, facecolor='#4ECDC4', edgecolor='black'))
    ax.text(8, 5, 'd', fontsize=10, ha='center', va='center', color='white')

    # W- decay products
    ax.text(5, 2.8, 'W- decay:', fontsize=10)
    ax.add_patch(Circle((4.5, 2), 0.25, facecolor=COLORS['antimatter'], edgecolor='black'))
    ax.text(4.5, 2, 'e-', fontsize=9, ha='center', va='center', color='white')
    ax.add_patch(Circle((5.5, 2), 0.25, facecolor='#888888', edgecolor='black'))
    ax.text(5.5, 2, 'v', fontsize=9, ha='center', va='center')
    ax.text(5, 1.3, 'electron + antineutrino', fontsize=9, ha='center')

    # Right: TRD mechanism
    ax = axes[1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 8)
    ax.axis('off')
    ax.set_title('TRD: Stress-Induced Transmutation', fontsize=12, fontweight='bold')

    # Stress threshold diagram
    ax.text(5, 7, 'Field Stress Calculation:', fontsize=11, fontweight='bold',
            ha='center')

    stress_eq = r'stress = |$\nabla \cdot J$| + |$\nabla \times J$| + |$\nabla \rho$|'
    ax.text(5, 6.2, stress_eq, fontsize=11, ha='center')

    # Threshold bar
    ax.add_patch(Rectangle((2, 4.5), 6, 0.8, facecolor='#e0e0e0', edgecolor='black'))
    ax.add_patch(Rectangle((2, 4.5), 4, 0.8, facecolor='#FF6B6B', edgecolor='none'))
    ax.plot([6, 6], [4.3, 5.5], 'k--', linewidth=2)
    ax.text(6, 5.7, 'Threshold', fontsize=9, ha='center')
    ax.text(4, 4.9, 'Current Stress', fontsize=9, ha='center', color='white')

    # Explanation
    ax.text(5, 3.5, 'When stress > threshold:', fontsize=10, ha='center',
            fontweight='bold')
    ax.text(5, 2.8, 'Polarity can flip (+1 <-> -1)', fontsize=10, ha='center')
    ax.text(5, 2.1, 'Flavor change occurs (d -> u)', fontsize=10, ha='center')

    ax.text(5, 1, 'sin^2(theta_W) = 3/13 = 0.2308\n(Weinberg angle from curve geometry)',
            fontsize=9, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#fff8e8'))

    fig.suptitle('Weak Force: Transmutation Under High Field Stress',
                fontsize=14, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0, 1, 0.93])
    return fig


if __name__ == '__main__':
    fig = generate_weak_interaction()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_3_9_weak_interaction.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
