"""
Figure 2.14: Molecular Orbitals
===============================
Visualizes molecular orbital formation from atomic orbital overlap,
showing bonding and antibonding combinations in TRD.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Ellipse, FancyArrowPatch
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_molecular_orbitals():
    """
    Generate the molecular orbitals visualization.

    Shows:
    1. Top: Atomic orbital overlap (constructive vs destructive)
    2. Middle: Energy level diagram
    3. Bottom: 3D-ish orbital shapes
    """
    fig = plt.figure(figsize=(14, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    gs = fig.add_gridspec(3, 2, height_ratios=[1, 1, 1.2], hspace=0.3, wspace=0.2)

    ax_bonding = fig.add_subplot(gs[0, 0])
    ax_antibonding = fig.add_subplot(gs[0, 1])
    ax_energy = fig.add_subplot(gs[1, :])
    ax_shapes = fig.add_subplot(gs[2, :])

    # =========================================================================
    # Top Left: Bonding Orbital (Constructive Interference)
    # =========================================================================
    ax_bonding.set_xlim(-5, 5)
    ax_bonding.set_ylim(-3, 3)
    ax_bonding.set_aspect('equal')
    ax_bonding.axis('off')

    ax_bonding.set_title('Bonding Orbital (Constructive)',
                        fontsize=12, fontweight='bold')

    # Two atomic orbitals overlapping constructively
    x = np.linspace(-5, 5, 200)

    # Left atomic orbital
    psi_L = np.exp(-((x + 1.5)**2))
    # Right atomic orbital (same phase)
    psi_R = np.exp(-((x - 1.5)**2))
    # Molecular orbital
    psi_bonding = psi_L + psi_R

    ax_bonding.fill_between(x, 0, psi_L, alpha=0.3, color=COLORS['antimatter'],
                           label='Atom A')
    ax_bonding.fill_between(x, 0, psi_R, alpha=0.3, color=COLORS['antimatter'],
                           label='Atom B')
    ax_bonding.plot(x, psi_bonding, 'k-', linewidth=2.5, label='Bonding MO')

    # Mark nuclei
    ax_bonding.plot(-1.5, 0, 'o', markersize=10, color=COLORS['matter'])
    ax_bonding.plot(1.5, 0, 'o', markersize=10, color=COLORS['matter'])

    ax_bonding.text(0, 1.8, 'Enhanced electron\ndensity between nuclei',
                    fontsize=9, ha='center', color=COLORS['accent1'])

    ax_bonding.legend(loc='upper right', fontsize=8)

    # Plus signs to show same phase
    ax_bonding.text(-3, 1.2, '+', fontsize=16, ha='center', color=COLORS['antimatter'])
    ax_bonding.text(3, 1.2, '+', fontsize=16, ha='center', color=COLORS['antimatter'])

    # =========================================================================
    # Top Right: Antibonding Orbital (Destructive Interference)
    # =========================================================================
    ax_antibonding.set_xlim(-5, 5)
    ax_antibonding.set_ylim(-3, 3)
    ax_antibonding.set_aspect('equal')
    ax_antibonding.axis('off')

    ax_antibonding.set_title('Antibonding Orbital (Destructive)',
                            fontsize=12, fontweight='bold')

    # Opposite phase
    psi_L_anti = np.exp(-((x + 1.5)**2))
    psi_R_anti = -np.exp(-((x - 1.5)**2))
    psi_antibonding = psi_L_anti + psi_R_anti

    ax_antibonding.fill_between(x, 0, psi_L_anti, alpha=0.3, color=COLORS['antimatter'])
    ax_antibonding.fill_between(x, 0, psi_R_anti, alpha=0.3, color=COLORS['matter'])
    ax_antibonding.plot(x, psi_antibonding, 'k-', linewidth=2.5, label='Antibonding MO')

    # Mark nuclei
    ax_antibonding.plot(-1.5, 0, 'o', markersize=10, color=COLORS['matter'])
    ax_antibonding.plot(1.5, 0, 'o', markersize=10, color=COLORS['matter'])

    # Node at center
    ax_antibonding.axvline(x=0, color='red', linewidth=2, linestyle='--')
    ax_antibonding.text(0.3, 2, 'Node\n(zero density)',
                        fontsize=9, color='red')

    # Plus and minus to show opposite phase
    ax_antibonding.text(-3, 1.2, '+', fontsize=16, ha='center', color=COLORS['antimatter'])
    ax_antibonding.text(3, 1.2, '-', fontsize=16, ha='center', color=COLORS['matter'])

    ax_antibonding.legend(loc='upper right', fontsize=8)

    # =========================================================================
    # Middle: Energy Level Diagram
    # =========================================================================
    ax_energy.set_xlim(0, 10)
    ax_energy.set_ylim(-1, 5)
    ax_energy.axis('off')

    ax_energy.set_title('Molecular Orbital Energy Diagram',
                        fontsize=13, fontweight='bold')

    # Atomic levels (left)
    ax_energy.hlines(2, 0.5, 2, colors='black', linewidth=2)
    ax_energy.text(1.25, 1.5, 'Atom A\n1s', fontsize=10, ha='center')
    ax_energy.plot(1.1, 2, 'o', markersize=8, color=COLORS['antimatter'])
    ax_energy.annotate('', xy=(1.1, 2.2), xytext=(1.1, 2.05),
                      arrowprops=dict(arrowstyle='->', color='black', lw=1.5))

    # Atomic levels (right)
    ax_energy.hlines(2, 8, 9.5, colors='black', linewidth=2)
    ax_energy.text(8.75, 1.5, 'Atom B\n1s', fontsize=10, ha='center')
    ax_energy.plot(8.9, 2, 'o', markersize=8, color=COLORS['antimatter'])
    ax_energy.annotate('', xy=(8.9, 2.2), xytext=(8.9, 2.05),
                      arrowprops=dict(arrowstyle='->', color='black', lw=1.5))

    # Molecular levels (center)
    # Bonding (lower energy)
    ax_energy.hlines(1, 4, 6, colors=COLORS['accent1'], linewidth=3)
    ax_energy.text(5, 0.4, r'$\sigma$ (bonding)', fontsize=11, ha='center',
                   color=COLORS['accent1'])
    # Two electrons in bonding
    ax_energy.plot(4.7, 1, 'o', markersize=8, color=COLORS['antimatter'])
    ax_energy.plot(5.3, 1, 'o', markersize=8, color=COLORS['antimatter'])
    ax_energy.annotate('', xy=(4.7, 1.2), xytext=(4.7, 1.05),
                      arrowprops=dict(arrowstyle='->', color='black', lw=1.5))
    ax_energy.annotate('', xy=(5.3, 0.8), xytext=(5.3, 0.95),
                      arrowprops=dict(arrowstyle='->', color='black', lw=1.5))

    # Antibonding (higher energy)
    ax_energy.hlines(3.5, 4, 6, colors=COLORS['matter'], linewidth=3)
    ax_energy.text(5, 4, r'$\sigma^*$ (antibonding)', fontsize=11, ha='center',
                   color=COLORS['matter'])

    # Correlation lines
    ax_energy.plot([2, 4], [2, 1], 'k--', linewidth=1, alpha=0.5)
    ax_energy.plot([2, 4], [2, 3.5], 'k--', linewidth=1, alpha=0.5)
    ax_energy.plot([8, 6], [2, 1], 'k--', linewidth=1, alpha=0.5)
    ax_energy.plot([8, 6], [2, 3.5], 'k--', linewidth=1, alpha=0.5)

    # Energy arrow
    ax_energy.annotate('', xy=(3.5, 4), xytext=(3.5, 0.5),
                      arrowprops=dict(arrowstyle='->', color='gray', lw=2))
    ax_energy.text(3.3, 2.25, 'E', fontsize=12, rotation=90, va='center')

    # Bond order label
    ax_energy.text(5, -0.5, 'Bond Order = (2 bonding - 0 antibonding) / 2 = 1',
                   fontsize=10, ha='center',
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Bottom: 3D Orbital Shapes
    # =========================================================================
    ax_shapes.set_xlim(-8, 8)
    ax_shapes.set_ylim(-2, 3)
    ax_shapes.set_aspect('equal')
    ax_shapes.axis('off')

    ax_shapes.set_title('Molecular Orbital Shapes (Isosurfaces)',
                        fontsize=13, fontweight='bold')

    # Sigma bonding orbital (elongated peanut)
    sigma = Ellipse((-4, 0.5), 4, 1.5, facecolor=COLORS['antimatter'],
                    edgecolor='black', linewidth=2, alpha=0.6)
    ax_shapes.add_patch(sigma)
    ax_shapes.text(-4, -1.2, r'$\sigma$ bonding', fontsize=11, ha='center')

    # Mark nuclei
    ax_shapes.plot(-5, 0.5, 'o', markersize=8, color=COLORS['matter'])
    ax_shapes.plot(-3, 0.5, 'o', markersize=8, color=COLORS['matter'])

    # Sigma antibonding (two lobes with node)
    lobe_L = Ellipse((2, 0.5), 1.8, 1.2, facecolor=COLORS['antimatter'],
                     edgecolor='black', linewidth=2, alpha=0.6)
    lobe_R = Ellipse((6, 0.5), 1.8, 1.2, facecolor=COLORS['matter'],
                     edgecolor='black', linewidth=2, alpha=0.6)
    ax_shapes.add_patch(lobe_L)
    ax_shapes.add_patch(lobe_R)

    # Node plane
    ax_shapes.axvline(x=4, ymin=0.2, ymax=0.7, color='red', linewidth=2,
                      linestyle='--')

    ax_shapes.text(4, -1.2, r'$\sigma^*$ antibonding', fontsize=11, ha='center')
    ax_shapes.text(4, 2, 'nodal\nplane', fontsize=9, ha='center', color='red')

    # Mark nuclei
    ax_shapes.plot(3, 0.5, 'o', markersize=8, color=COLORS['matter'])
    ax_shapes.plot(5, 0.5, 'o', markersize=8, color=COLORS['matter'])

    # Phase labels
    ax_shapes.text(2, 0.5, '+', fontsize=14, ha='center', va='center',
                   color='white', fontweight='bold')
    ax_shapes.text(6, 0.5, '-', fontsize=14, ha='center', va='center',
                   color='white', fontweight='bold')

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Molecular Orbitals: Flux Interference Creates Bonding/Antibonding',
                fontsize=16, fontweight='bold', y=0.99)

    explanation = (
        "In TRD, molecular orbitals form from atomic flux interference patterns.\n"
        "Constructive interference (bonding) lowers energy; destructive (antibonding) raises it."
    )
    fig.text(0.5, 0.01, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.04, 1, 0.97])

    return fig


if __name__ == '__main__':
    fig = generate_molecular_orbitals()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_2_14_molecular_orbitals.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
