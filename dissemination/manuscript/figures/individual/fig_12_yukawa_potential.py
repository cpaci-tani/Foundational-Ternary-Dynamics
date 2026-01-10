"""
Figure 1.12: Yukawa Potential
=============================
Visualizes the strong force using the Yukawa potential form:
F(r) = g² × exp(-m×r) × (1 + m×r) / r²

Shows:
- Repulsive core at very small distances
- Attractive region at intermediate distances
- Exponential falloff at large distances
- Comparison with Coulomb potential
"""

import numpy as np
import matplotlib.pyplot as plt
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, FORCE_COLORS, apply_trd_style


def generate_yukawa_potential():
    """
    Generate the Yukawa potential visualization.

    Shows the strong force potential with:
    - Main potential curve
    - Repulsive core modification
    - Attractive region shading
    - Falloff comparison
    """
    fig, ax = plt.subplots(figsize=(10, 7), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Parameters
    g2 = 1.0          # Coupling constant squared
    m_pion = 1.0      # Pion mass (sets range scale, ~140 MeV -> 1.4 fm)
    r_core = 0.4      # Repulsive core radius

    # Distance range (in units of 1/m_pion ≈ 1.4 fm)
    r = np.linspace(0.2, 5, 500)

    # Yukawa potential: V(r) = -g² × exp(-m×r) / r
    V_yukawa = -g2 * np.exp(-m_pion * r) / r

    # Yukawa force (derivative): F(r) ∝ g² × exp(-m×r) × (1 + m×r) / r²
    F_yukawa = g2 * np.exp(-m_pion * r) * (1 + m_pion * r) / r**2

    # Add repulsive core at very small r
    V_with_core = np.where(r < r_core,
                           10 * (r_core / r - 1)**2,  # Repulsive
                           V_yukawa)

    # Coulomb for comparison (1/r)
    V_coulomb = -g2 / r

    # Plot potentials
    ax.plot(r, V_with_core, color=FORCE_COLORS['strong'], linewidth=3,
            label='Strong (Yukawa + core)')
    ax.plot(r, V_coulomb, color=FORCE_COLORS['electromagnetic'], linewidth=2,
            linestyle='--', alpha=0.7, label='EM (Coulomb 1/r)')

    # Reference lines
    ax.axhline(y=0, color=COLORS['grid'], linestyle='-', alpha=0.5, linewidth=1)

    # Shade attractive region
    attractive_mask = V_with_core < 0
    ax.fill_between(r, 0, V_with_core, where=attractive_mask,
                    alpha=0.2, color=FORCE_COLORS['strong'],
                    label='Attractive region')

    # Shade repulsive core
    repulsive_mask = V_with_core > 0
    ax.fill_between(r, 0, V_with_core, where=repulsive_mask,
                    alpha=0.3, color=COLORS['matter'],
                    label='Repulsive core')

    # Mark key distances
    ax.axvline(x=r_core, color=COLORS['highlight'], linestyle=':', linewidth=2, alpha=0.8)
    ax.annotate(f'Core radius\n$r_c \\approx {r_core}$ fm',
                xy=(r_core, 2), xytext=(r_core + 0.5, 4),
                fontsize=10, ha='left',
                arrowprops=dict(arrowstyle='->', color=COLORS['highlight'], lw=1.5))

    # Mark confinement scale
    ax.axvline(x=1.0, color=COLORS['accent2'], linestyle=':', linewidth=2, alpha=0.8)
    ax.annotate('Range\n$\\sim 1/m_\\pi$',
                xy=(1.0, -0.5), xytext=(1.5, -1.5),
                fontsize=10, ha='left',
                arrowprops=dict(arrowstyle='->', color=COLORS['accent2'], lw=1.5))

    # Configure axes
    ax.set_xlim(0, 5)
    ax.set_ylim(-3, 8)

    apply_trd_style(ax,
                    title='Strong Force: Yukawa Potential',
                    xlabel='Distance $r$ (fm)',
                    ylabel='Potential $V(r)$ (arbitrary units)')

    # Formula box
    formula_text = (
        r'Yukawa: $V(r) = -g^2 \frac{e^{-mr}}{r}$' + '\n\n'
        r'Range: $\lambda = 1/m_\pi \approx 1.4$ fm' + '\n\n'
        r'TRD Form: $F(r) = g^2 \frac{e^{-mr}(1+mr)}{r^2}$'
    )
    ax.text(0.98, 0.98, formula_text, transform=ax.transAxes,
            fontsize=10, verticalalignment='top', horizontalalignment='right',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white', alpha=0.95, edgecolor='gray'))

    # Legend
    ax.legend(loc='upper left', fontsize=10, framealpha=0.95)

    # Add physics explanation
    physics_text = (
        "At short range: Repulsive core prevents overlap\n"
        "At medium range: Attractive binding\n"
        "Beyond ~2 fm: Force vanishes exponentially"
    )
    ax.text(0.02, 0.02, physics_text, transform=ax.transAxes,
            fontsize=9, verticalalignment='bottom',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#ffffd0', alpha=0.9, edgecolor='gray'))

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_yukawa_potential()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_1_12_yukawa_potential.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
