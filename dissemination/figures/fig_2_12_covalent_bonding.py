"""
Figure 2.12: Covalent Bonding
=============================
Visualizes how covalent bonds form in TRD through flux field overlap.

When two atoms approach, their electron flux fields can merge into
a shared region, lowering total system energy.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Ellipse
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_covalent_bonding():
    """
    Generate the covalent bonding visualization.

    Shows:
    1. Top: Two separate atoms
    2. Middle: Approaching atoms with flux overlap
    3. Bottom: Bonded molecule (H2 analog)
    """
    fig, axes = plt.subplots(3, 1, figsize=(12, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    for ax in axes:
        ax.set_xlim(-8, 8)
        ax.set_ylim(-4, 4)
        ax.set_aspect('equal')
        ax.axis('off')

    ax_sep, ax_approach, ax_bonded = axes

    # =========================================================================
    # Top: Separated Atoms
    # =========================================================================
    ax_sep.set_title('Stage 1: Separated Atoms (No Interaction)',
                     fontsize=13, fontweight='bold', pad=10)

    # Left atom
    nucleus_L = Circle((-4, 0), 0.3, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=2)
    ax_sep.add_patch(nucleus_L)
    ax_sep.text(-4, 0, '+', fontsize=12, ha='center', va='center',
                color='white', fontweight='bold')

    # Electron cloud (flux field)
    for r in [0.8, 1.2, 1.6]:
        cloud = Circle((-4, 0), r, facecolor=COLORS['antimatter'],
                       edgecolor='none', alpha=0.15)
        ax_sep.add_patch(cloud)

    # Electron
    e_L = Circle((-4 + 1.0, 0.5), 0.15, facecolor=COLORS['antimatter'],
                 edgecolor='black', linewidth=1)
    ax_sep.add_patch(e_L)

    # Right atom
    nucleus_R = Circle((4, 0), 0.3, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=2)
    ax_sep.add_patch(nucleus_R)
    ax_sep.text(4, 0, '+', fontsize=12, ha='center', va='center',
                color='white', fontweight='bold')

    for r in [0.8, 1.2, 1.6]:
        cloud = Circle((4, 0), r, facecolor=COLORS['antimatter'],
                       edgecolor='none', alpha=0.15)
        ax_sep.add_patch(cloud)

    e_R = Circle((4 - 1.0, -0.5), 0.15, facecolor=COLORS['antimatter'],
                 edgecolor='black', linewidth=1)
    ax_sep.add_patch(e_R)

    ax_sep.text(0, -3, 'Distance too large for flux overlap', fontsize=10,
                ha='center', style='italic')

    # =========================================================================
    # Middle: Approaching (Flux Overlap)
    # =========================================================================
    ax_approach.set_title('Stage 2: Approaching - Flux Fields Begin to Overlap',
                         fontsize=13, fontweight='bold', pad=10)

    # Left atom (closer)
    nucleus_L2 = Circle((-2.5, 0), 0.3, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=2)
    ax_approach.add_patch(nucleus_L2)
    ax_approach.text(-2.5, 0, '+', fontsize=12, ha='center', va='center',
                     color='white', fontweight='bold')

    for r in [0.8, 1.2, 1.6, 2.0]:
        cloud = Circle((-2.5, 0), r, facecolor=COLORS['antimatter'],
                       edgecolor='none', alpha=0.1)
        ax_approach.add_patch(cloud)

    # Right atom (closer)
    nucleus_R2 = Circle((2.5, 0), 0.3, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=2)
    ax_approach.add_patch(nucleus_R2)
    ax_approach.text(2.5, 0, '+', fontsize=12, ha='center', va='center',
                     color='white', fontweight='bold')

    for r in [0.8, 1.2, 1.6, 2.0]:
        cloud = Circle((2.5, 0), r, facecolor=COLORS['antimatter'],
                       edgecolor='none', alpha=0.1)
        ax_approach.add_patch(cloud)

    # Overlap region (brighter)
    overlap = Ellipse((0, 0), 3.5, 2, facecolor=COLORS['highlight'],
                      edgecolor='none', alpha=0.3)
    ax_approach.add_patch(overlap)

    ax_approach.text(0, 2.5, 'Overlap Region:\nEnhanced flux density',
                    fontsize=10, ha='center',
                    bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    ax_approach.text(0, -3, 'Electrons attracted to both nuclei in overlap zone',
                    fontsize=10, ha='center', style='italic')

    # =========================================================================
    # Bottom: Bonded Molecule
    # =========================================================================
    ax_bonded.set_title('Stage 3: Covalent Bond Formed (H2 Molecule)',
                        fontsize=13, fontweight='bold', pad=10)

    # Two nuclei at equilibrium distance
    d = 1.5  # Bond length

    nucleus_L3 = Circle((-d/2, 0), 0.25, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=2)
    nucleus_R3 = Circle((d/2, 0), 0.25, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=2)
    ax_bonded.add_patch(nucleus_L3)
    ax_bonded.add_patch(nucleus_R3)

    ax_bonded.text(-d/2, 0, '+', fontsize=10, ha='center', va='center',
                   color='white', fontweight='bold')
    ax_bonded.text(d/2, 0, '+', fontsize=10, ha='center', va='center',
                   color='white', fontweight='bold')

    # Bonding electron cloud (shared)
    # Molecular orbital shape (elongated)
    sigma = Ellipse((0, 0), 4, 2, facecolor=COLORS['antimatter'],
                    edgecolor=COLORS['antimatter'], alpha=0.3, linewidth=2)
    ax_bonded.add_patch(sigma)

    # Two electrons in the bond
    e1 = Circle((-0.4, 0.3), 0.12, facecolor=COLORS['antimatter'],
                edgecolor='black', linewidth=1)
    e2 = Circle((0.4, -0.3), 0.12, facecolor=COLORS['antimatter'],
                edgecolor='black', linewidth=1)
    ax_bonded.add_patch(e1)
    ax_bonded.add_patch(e2)

    # Spin arrows
    ax_bonded.annotate('', xy=(-0.4, 0.5), xytext=(-0.4, 0.35),
                       arrowprops=dict(arrowstyle='->', color='black', lw=1.5))
    ax_bonded.annotate('', xy=(0.4, -0.5), xytext=(0.4, -0.35),
                       arrowprops=dict(arrowstyle='->', color='black', lw=1.5))

    # Energy well diagram (small inset)
    ax_inset = ax_bonded.inset_axes([0.7, 0.2, 0.25, 0.6])
    r_plot = np.linspace(0.5, 5, 100)
    # Lennard-Jones-like potential
    E = 4 * ((1/r_plot)**12 - (1/r_plot)**6)
    ax_inset.plot(r_plot, E, 'k-', linewidth=2)
    ax_inset.axhline(y=0, color='gray', linewidth=0.5)
    ax_inset.axvline(x=1.12, color=COLORS['accent1'], linestyle='--', linewidth=1.5)
    ax_inset.set_xlim(0.5, 4)
    ax_inset.set_ylim(-1.5, 2)
    ax_inset.set_xlabel('r', fontsize=8)
    ax_inset.set_ylabel('E', fontsize=8)
    ax_inset.set_title('Bond Energy', fontsize=9)
    ax_inset.text(1.5, -1, 'Equilibrium', fontsize=7, color=COLORS['accent1'])

    ax_bonded.text(-5, 0, 'Shared electron pair:\nLowers total energy',
                   fontsize=10, ha='center',
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    ax_bonded.text(0, -3, 'Bond forms at equilibrium distance where energy is minimized',
                   fontsize=10, ha='center', style='italic')

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Covalent Bonding: Flux Overlap Creates Shared Electron Region',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "In TRD, covalent bonds form when atomic flux fields overlap, creating a shared region.\n"
        "Electrons in the overlap are attracted to both nuclei, lowering total system energy."
    )
    fig.text(0.5, 0.01, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.96])

    return fig


if __name__ == '__main__':
    fig = generate_covalent_bonding()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_2_12_covalent_bonding.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
