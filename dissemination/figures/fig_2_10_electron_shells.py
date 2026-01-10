"""
Figure 2.10: Electron Shell Structure
=====================================
Visualizes how electron shells emerge in TRD from flux equilibria
around a positive nucleus.

Shells form at discrete radii where the attractive nuclear flux
balances the repulsive electron-electron flux.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_electron_shells():
    """
    Generate the electron shell structure visualization.

    Shows:
    1. Left: 2D cross-section with shells
    2. Right: Radial flux profile
    """
    fig, (ax_shells, ax_flux) = plt.subplots(1, 2, figsize=(14, 7), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # =========================================================================
    # Left: Shell Structure (2D cross-section)
    # =========================================================================
    ax_shells.set_xlim(-8, 8)
    ax_shells.set_ylim(-8, 8)
    ax_shells.set_aspect('equal')

    # Shell radii (proportional to n^2)
    shell_radii = [1.5, 3.5, 6.0]  # n=1, 2, 3
    shell_colors = ['#FF6B6B', '#4ECDC4', '#95E1D3']
    shell_capacities = [2, 8, 18]  # 2n^2

    # Draw shells as translucent rings
    for i, (r, color, cap) in enumerate(zip(shell_radii, shell_colors, shell_capacities)):
        # Shell zone
        ring = Wedge((0, 0), r + 0.3, 0, 360, width=0.6,
                     facecolor=color, alpha=0.2, edgecolor=color, linewidth=2)
        ax_shells.add_patch(ring)

        # Shell label
        ax_shells.text(r + 0.5, r + 0.5, f'n={i+1}\n(max {cap})',
                      fontsize=9, ha='left')

    # Draw nucleus
    nucleus = Circle((0, 0), 0.6, facecolor=COLORS['matter'],
                     edgecolor='black', linewidth=2)
    ax_shells.add_patch(nucleus)
    ax_shells.text(0, 0, '+Z', fontsize=12, ha='center', va='center',
                   color='white', fontweight='bold')

    # Draw some electrons in shells
    # n=1: 2 electrons
    for angle in [0, np.pi]:
        x = shell_radii[0] * np.cos(angle)
        y = shell_radii[0] * np.sin(angle)
        e = Circle((x, y), 0.25, facecolor=COLORS['antimatter'],
                   edgecolor='black', linewidth=1.5)
        ax_shells.add_patch(e)

    # n=2: 6 electrons shown
    for angle in np.linspace(0, 2*np.pi, 6, endpoint=False):
        x = shell_radii[1] * np.cos(angle)
        y = shell_radii[1] * np.sin(angle)
        e = Circle((x, y), 0.22, facecolor=COLORS['antimatter'],
                   edgecolor='black', linewidth=1.5)
        ax_shells.add_patch(e)

    # n=3: 4 electrons shown
    for angle in np.linspace(0, 2*np.pi, 4, endpoint=False):
        x = shell_radii[2] * np.cos(angle + np.pi/4)
        y = shell_radii[2] * np.sin(angle + np.pi/4)
        e = Circle((x, y), 0.2, facecolor=COLORS['antimatter'],
                   edgecolor='black', linewidth=1.5, alpha=0.7)
        ax_shells.add_patch(e)

    ax_shells.set_title('Electron Shell Structure\n(2D Cross-Section)',
                        fontsize=14, fontweight='bold')
    ax_shells.set_xlabel('x (Bohr radii units)')
    ax_shells.set_ylabel('y (Bohr radii units)')

    # Legend
    legend_elements = [
        mpatches.Patch(facecolor=COLORS['matter'], edgecolor='black',
                      label='Nucleus (+Z)'),
        mpatches.Patch(facecolor=COLORS['antimatter'], edgecolor='black',
                      label='Electrons (-1)'),
    ]
    ax_shells.legend(handles=legend_elements, loc='upper right', fontsize=9)

    ax_shells.grid(True, alpha=0.2)

    # =========================================================================
    # Right: Radial Flux Profile
    # =========================================================================
    r = np.linspace(0.1, 8, 200)

    # Nuclear attractive flux (falls off as 1/r^2)
    Z = 12  # Example: Magnesium
    nuclear_flux = -Z / r**2

    # Electron repulsive flux (shells create peaks)
    electron_flux = np.zeros_like(r)
    for shell_r, n_electrons in [(1.5, 2), (3.5, 8)]:
        # Each shell creates a screening effect
        electron_flux += n_electrons * np.exp(-2 * np.abs(r - shell_r)) / (r + 0.1)

    # Total effective flux
    total_flux = nuclear_flux + 2 * electron_flux

    ax_flux.plot(r, nuclear_flux, color=COLORS['matter'], linewidth=2,
                 label='Nuclear (attractive)', linestyle='--')
    ax_flux.plot(r, 2 * electron_flux, color=COLORS['antimatter'], linewidth=2,
                 label='Electron (repulsive)', linestyle='--')
    ax_flux.plot(r, total_flux, color='black', linewidth=2.5,
                 label='Net flux')

    ax_flux.axhline(y=0, color='gray', linewidth=1)

    # Mark shell equilibrium points
    for i, shell_r in enumerate(shell_radii[:2]):
        ax_flux.axvline(x=shell_r, color=shell_colors[i], linewidth=1.5,
                       linestyle=':', alpha=0.7)
        ax_flux.text(shell_r, 3, f'n={i+1}', fontsize=10, ha='center',
                    color=shell_colors[i])

    ax_flux.set_xlabel('Radius (Bohr radii)', fontsize=11)
    ax_flux.set_ylabel('Flux Density', fontsize=11)
    ax_flux.set_title('Radial Flux Profile\n(Equilibrium at Shell Radii)',
                      fontsize=14, fontweight='bold')
    ax_flux.legend(loc='lower right', fontsize=9)
    ax_flux.set_xlim(0, 8)
    ax_flux.set_ylim(-5, 5)
    ax_flux.grid(True, alpha=0.3)

    # Equilibrium explanation
    ax_flux.text(6, 4, 'Shells form where\nnet flux = 0',
                fontsize=10, ha='center',
                bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Electron Shells in TRD: Flux Equilibria Define Orbital Radii',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "Shell radii emerge where attractive nuclear flux balances electron-electron repulsion.\n"
        "The n^2 scaling arises from the 3D geometry of flux field equilibrium."
    )
    fig.text(0.5, 0.02, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.06, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_electron_shells()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_2_10_electron_shells.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
