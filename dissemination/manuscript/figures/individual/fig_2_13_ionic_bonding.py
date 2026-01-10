"""
Figure 2.13: Ionic Bonding
==========================
Visualizes ionic bonding in TRD: electron transfer creates
oppositely charged ions that attract via Coulomb flux interaction.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, Rectangle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_ionic_bonding():
    """
    Generate the ionic bonding visualization.

    Shows:
    1. Top: Neutral atoms (Na and Cl analog)
    2. Middle: Electron transfer
    3. Bottom: Ion pair and lattice
    """
    fig, axes = plt.subplots(3, 1, figsize=(12, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    ax_neutral, ax_transfer, ax_ionic = axes

    # =========================================================================
    # Top: Neutral Atoms
    # =========================================================================
    ax_neutral.set_xlim(-8, 8)
    ax_neutral.set_ylim(-3, 3)
    ax_neutral.set_aspect('equal')
    ax_neutral.axis('off')

    ax_neutral.set_title('Stage 1: Neutral Atoms (Before Interaction)',
                        fontsize=13, fontweight='bold', pad=10)

    # Sodium-like atom (left) - 1 valence electron
    na_x = -4
    nucleus_na = Circle((na_x, 0), 0.35, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=2)
    ax_neutral.add_patch(nucleus_na)
    ax_neutral.text(na_x, 0, 'Na', fontsize=10, ha='center', va='center',
                    color='white', fontweight='bold')

    # Inner shells (full)
    for r in [0.8, 1.3]:
        shell = Circle((na_x, 0), r, facecolor='none',
                       edgecolor=COLORS['antimatter'], linewidth=1.5, linestyle='--')
        ax_neutral.add_patch(shell)

    # Outer shell with 1 electron
    outer = Circle((na_x, 0), 1.8, facecolor='none',
                   edgecolor=COLORS['antimatter'], linewidth=2)
    ax_neutral.add_patch(outer)
    valence_e = Circle((na_x + 1.8, 0), 0.15, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=1)
    ax_neutral.add_patch(valence_e)

    ax_neutral.text(na_x, -2.5, 'Na: 1 valence e-\n(wants to lose it)',
                    fontsize=9, ha='center')

    # Chlorine-like atom (right) - 7 valence electrons
    cl_x = 4
    nucleus_cl = Circle((cl_x, 0), 0.35, facecolor=COLORS['accent1'],
                        edgecolor='black', linewidth=2)
    ax_neutral.add_patch(nucleus_cl)
    ax_neutral.text(cl_x, 0, 'Cl', fontsize=10, ha='center', va='center',
                    color='white', fontweight='bold')

    # Inner shells
    for r in [0.8, 1.3]:
        shell = Circle((cl_x, 0), r, facecolor='none',
                       edgecolor=COLORS['antimatter'], linewidth=1.5, linestyle='--')
        ax_neutral.add_patch(shell)

    # Outer shell with 7 electrons (one spot empty)
    outer_cl = Circle((cl_x, 0), 1.8, facecolor='none',
                      edgecolor=COLORS['antimatter'], linewidth=2)
    ax_neutral.add_patch(outer_cl)

    for i, angle in enumerate(np.linspace(0, 2*np.pi, 8, endpoint=False)):
        if i < 7:  # 7 electrons
            x = cl_x + 1.8 * np.cos(angle)
            y = 1.8 * np.sin(angle)
            e = Circle((x, y), 0.12, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=1)
            ax_neutral.add_patch(e)

    ax_neutral.text(cl_x, -2.5, 'Cl: 7 valence e-\n(wants 1 more)',
                    fontsize=9, ha='center')

    # =========================================================================
    # Middle: Electron Transfer
    # =========================================================================
    ax_transfer.set_xlim(-8, 8)
    ax_transfer.set_ylim(-3, 3)
    ax_transfer.set_aspect('equal')
    ax_transfer.axis('off')

    ax_transfer.set_title('Stage 2: Electron Transfer (Ionization)',
                         fontsize=13, fontweight='bold', pad=10)

    # Na losing electron
    na_x2 = -3
    nucleus_na2 = Circle((na_x2, 0), 0.35, facecolor=COLORS['matter'],
                         edgecolor='black', linewidth=2)
    ax_transfer.add_patch(nucleus_na2)
    ax_transfer.text(na_x2, 0, 'Na+', fontsize=9, ha='center', va='center',
                     color='white', fontweight='bold')

    # Show empty outer shell
    outer_na2 = Circle((na_x2, 0), 1.5, facecolor='none',
                       edgecolor='gray', linewidth=1.5, linestyle=':')
    ax_transfer.add_patch(outer_na2)

    # Electron moving
    transfer_e = Circle((0, 0.5), 0.18, facecolor=COLORS['antimatter'],
                        edgecolor='black', linewidth=2)
    ax_transfer.add_patch(transfer_e)

    # Arrow showing transfer
    ax_transfer.annotate('', xy=(2, 0.3), xytext=(-1.5, 0.3),
                        arrowprops=dict(arrowstyle='->', color='black',
                                       lw=2, connectionstyle='arc3,rad=0.2'))

    ax_transfer.text(0, 1.5, 'Electron transfers', fontsize=10, ha='center',
                     fontweight='bold')

    # Cl gaining electron
    cl_x2 = 3
    nucleus_cl2 = Circle((cl_x2, 0), 0.35, facecolor=COLORS['accent1'],
                         edgecolor='black', linewidth=2)
    ax_transfer.add_patch(nucleus_cl2)
    ax_transfer.text(cl_x2, 0, 'Cl-', fontsize=9, ha='center', va='center',
                     color='white', fontweight='bold')

    # Full outer shell (8 electrons)
    outer_cl2 = Circle((cl_x2, 0), 1.8, facecolor=COLORS['antimatter'],
                       edgecolor=COLORS['antimatter'], linewidth=2, alpha=0.2)
    ax_transfer.add_patch(outer_cl2)

    ax_transfer.text(-3, -2.5, 'Na+ (positive ion)', fontsize=10, ha='center',
                     color=COLORS['matter'])
    ax_transfer.text(3, -2.5, 'Cl- (negative ion)', fontsize=10, ha='center',
                     color=COLORS['accent1'])

    # =========================================================================
    # Bottom: Ion Pair and Lattice
    # =========================================================================
    ax_ionic.set_xlim(-8, 8)
    ax_ionic.set_ylim(-3, 4)
    ax_ionic.set_aspect('equal')
    ax_ionic.axis('off')

    ax_ionic.set_title('Stage 3: Ionic Bond (Coulomb Attraction)',
                       fontsize=13, fontweight='bold', pad=10)

    # Ion pair with attraction
    ion_dist = 1.5

    # Na+ ion
    na_ion = Circle((-ion_dist/2, 2), 0.4, facecolor=COLORS['matter'],
                    edgecolor='black', linewidth=2)
    ax_ionic.add_patch(na_ion)
    ax_ionic.text(-ion_dist/2, 2, 'Na+', fontsize=10, ha='center', va='center',
                  color='white', fontweight='bold')

    # Cl- ion
    cl_ion = Circle((ion_dist/2, 2), 0.5, facecolor=COLORS['accent1'],
                    edgecolor='black', linewidth=2)
    ax_ionic.add_patch(cl_ion)
    ax_ionic.text(ion_dist/2, 2, 'Cl-', fontsize=10, ha='center', va='center',
                  color='white', fontweight='bold')

    # Attraction arrows
    ax_ionic.annotate('', xy=(0.1, 2), xytext=(-0.3, 2),
                     arrowprops=dict(arrowstyle='<->', color='purple', lw=2))

    ax_ionic.text(0, 2.8, 'Coulomb attraction\nF = kq1q2/r^2', fontsize=9,
                  ha='center')

    # Small lattice section
    lattice_y = -1
    spacing = 1.2

    for i in range(5):
        for j in range(2):
            x = -2.4 + i * spacing
            y = lattice_y + j * spacing

            if (i + j) % 2 == 0:
                # Na+
                ion = Circle((x, y), 0.3, facecolor=COLORS['matter'],
                            edgecolor='black', linewidth=1)
            else:
                # Cl-
                ion = Circle((x, y), 0.35, facecolor=COLORS['accent1'],
                            edgecolor='black', linewidth=1)
            ax_ionic.add_patch(ion)

    ax_ionic.text(0, -2.5, 'NaCl Crystal Lattice\n(Alternating ions)',
                  fontsize=10, ha='center')

    # Energy diagram
    ax_inset = ax_ionic.inset_axes([0.72, 0.1, 0.25, 0.5])
    r = np.linspace(0.3, 4, 100)
    E_coulomb = -1 / r  # Attractive
    E_repulsion = 0.1 / r**8  # Short-range repulsion
    E_total = E_coulomb + E_repulsion

    ax_inset.plot(r, E_coulomb, '--', color='purple', linewidth=1.5,
                  label='Coulomb')
    ax_inset.plot(r, E_total, 'k-', linewidth=2, label='Total')
    ax_inset.axhline(y=0, color='gray', linewidth=0.5)
    ax_inset.axvline(x=1.1, color=COLORS['highlight'], linestyle=':', linewidth=1.5)
    ax_inset.set_xlim(0.3, 3)
    ax_inset.set_ylim(-1.5, 0.5)
    ax_inset.set_xlabel('r', fontsize=8)
    ax_inset.set_ylabel('E', fontsize=8)
    ax_inset.set_title('Bond Energy', fontsize=9)
    ax_inset.legend(fontsize=6, loc='lower right')

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Ionic Bonding: Electron Transfer Creates Coulomb-Bound Ion Pairs',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "In ionic bonding, one atom's flux field releases an electron to another atom's field.\n"
        "The resulting opposite charges attract via Coulomb interaction, forming a stable bond."
    )
    fig.text(0.5, 0.01, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.96])

    return fig


if __name__ == '__main__':
    fig = generate_ionic_bonding()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_2_13_ionic_bonding.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
