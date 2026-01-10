"""
Figure 2.11: Hydrogen Energy Levels
===================================
Visualizes the discrete energy levels of hydrogen emerging from
TRD's flux standing wave patterns.

The 1/n^2 energy scaling emerges from 3D spherical standing waves.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, FancyArrowPatch
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_hydrogen_energy():
    """
    Generate the hydrogen energy levels visualization.

    Shows:
    1. Left: Energy level diagram
    2. Right: Radial wavefunctions (flux patterns)
    """
    fig, (ax_levels, ax_waves) = plt.subplots(1, 2, figsize=(14, 9), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # =========================================================================
    # Left: Energy Level Diagram
    # =========================================================================
    ax_levels.set_xlim(0, 10)
    ax_levels.set_ylim(-14, 1)

    # Energy levels: E_n = -13.6/n^2 eV
    levels = {
        1: -13.6,
        2: -3.4,
        3: -1.51,
        4: -0.85,
        5: -0.54,
    }

    # Draw energy levels
    for n, E in levels.items():
        # Level line
        ax_levels.hlines(E, 1, 9, colors='black', linewidth=2)
        # Label
        ax_levels.text(0.5, E, f'n={n}', fontsize=11, va='center', ha='right')
        ax_levels.text(9.5, E, f'{E:.2f} eV', fontsize=10, va='center', ha='left')

        # Sublevel degeneracy indication
        if n > 1:
            for l in range(n):
                x_pos = 2 + l * 1.5
                ax_levels.plot(x_pos, E, 'o', markersize=6, color=COLORS['highlight'])

    # Draw some transitions
    transitions = [
        (3, 2, 'H-alpha\n656 nm', COLORS['matter']),
        (4, 2, 'H-beta\n486 nm', COLORS['antimatter']),
        (2, 1, 'Lyman-alpha\n122 nm', COLORS['accent1']),
    ]

    for n_upper, n_lower, label, color in transitions:
        E_upper = levels[n_upper]
        E_lower = levels[n_lower]
        x_pos = 6 + (n_upper - 2) * 1.2

        # Transition arrow
        ax_levels.annotate('', xy=(x_pos, E_lower + 0.2),
                          xytext=(x_pos, E_upper - 0.2),
                          arrowprops=dict(arrowstyle='->', color=color, lw=2))

        # Wavy line (photon emission)
        y_mid = (E_upper + E_lower) / 2
        t = np.linspace(0, 1, 30)
        x_wave = x_pos + 0.3 + 0.15 * np.sin(10 * t)
        y_wave = E_upper + (E_lower - E_upper) * t
        ax_levels.plot(x_wave, y_wave, color=color, linewidth=1.5)

        ax_levels.text(x_pos + 0.8, y_mid, label, fontsize=8, color=color)

    # Ionization continuum
    ax_levels.axhline(y=0, color='gray', linewidth=2, linestyle='--')
    ax_levels.text(5, 0.3, 'Ionization (E = 0)', fontsize=10, ha='center')

    ax_levels.fill_between([0, 10], [0, 0], [1, 1], color='gray', alpha=0.2)
    ax_levels.text(5, 0.7, 'Continuum (free electron)', fontsize=9,
                   ha='center', style='italic')

    ax_levels.set_ylabel('Energy (eV)', fontsize=12)
    ax_levels.set_title('Hydrogen Energy Levels\n$E_n = -13.6/n^2$ eV',
                        fontsize=14, fontweight='bold')
    ax_levels.set_xticks([])

    # TRD formula
    ax_levels.text(5, -12.5, r'$E_n = -\frac{m_e \alpha^2 c^2}{2n^2}$',
                   fontsize=12, ha='center',
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Right: Radial Wavefunctions (Flux Patterns)
    # =========================================================================
    r = np.linspace(0, 25, 500)

    # Simplified radial wavefunctions (proportional to actual)
    def R_1s(r):
        a0 = 1  # Bohr radius
        return 2 * np.exp(-r / a0)

    def R_2s(r):
        a0 = 1
        return (1/np.sqrt(8)) * (2 - r/a0) * np.exp(-r / (2*a0))

    def R_2p(r):
        a0 = 1
        return (1/np.sqrt(24)) * (r/a0) * np.exp(-r / (2*a0))

    def R_3s(r):
        a0 = 1
        return (2/81) * (27 - 18*r/a0 + 2*(r/a0)**2) * np.exp(-r / (3*a0))

    wavefunctions = [
        (R_1s, '1s (n=1, l=0)', COLORS['matter']),
        (R_2s, '2s (n=2, l=0)', COLORS['antimatter']),
        (R_2p, '2p (n=2, l=1)', COLORS['highlight']),
        (R_3s, '3s (n=3, l=0)', COLORS['accent1']),
    ]

    for wf, label, color in wavefunctions:
        y = wf(r)
        # Plot |R|^2 * r^2 (radial probability density)
        prob = y**2 * r**2
        prob = prob / np.max(prob)  # Normalize
        ax_waves.plot(r, prob, color=color, linewidth=2, label=label)

    ax_waves.set_xlabel('Radius (Bohr radii)', fontsize=12)
    ax_waves.set_ylabel('Radial Probability Density', fontsize=12)
    ax_waves.set_title('Radial Probability Distributions\n(Flux Standing Wave Patterns)',
                       fontsize=14, fontweight='bold')
    ax_waves.legend(loc='upper right', fontsize=10)
    ax_waves.set_xlim(0, 25)
    ax_waves.set_ylim(0, 1.1)
    ax_waves.grid(True, alpha=0.3)

    # Annotate nodes
    ax_waves.annotate('2s has 1 node', xy=(2, 0.02), xytext=(5, 0.3),
                     fontsize=9, arrowprops=dict(arrowstyle='->', color='gray'))

    ax_waves.text(20, 0.9, 'TRD: These are\nflux density profiles',
                 fontsize=10, ha='center',
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Hydrogen Atom: Discrete Energy Levels from Flux Quantization',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "Energy levels are discrete because only certain flux standing wave patterns fit the boundary conditions.\n"
        "In TRD, the 1/n^2 scaling emerges from the geometry of 3D spherical flux equilibria."
    )
    fig.text(0.5, 0.02, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.06, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_hydrogen_energy()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_2_11_hydrogen_energy.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
