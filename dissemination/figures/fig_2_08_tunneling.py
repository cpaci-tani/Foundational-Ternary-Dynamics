"""
Figure 2.8: Tunneling Probability
=================================
Visualizes quantum tunneling in TRD: particle, barrier, and flux
extending beyond the classical boundary.

In TRD, tunneling occurs because the flux field extends beyond the
particle's integer position, allowing manifestation on the far side.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Circle, FancyArrowPatch
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_tunneling():
    """
    Generate the tunneling probability visualization.

    Shows:
    1. Top: Classical picture (particle bounces)
    2. Middle: TRD picture (flux penetrates)
    3. Bottom: Transmission probability vs barrier
    """
    fig, (ax_classical, ax_trd, ax_prob) = plt.subplots(
        3, 1, figsize=(12, 12), dpi=150, height_ratios=[1, 1.2, 1]
    )
    fig.patch.set_facecolor(COLORS['background'])

    # =========================================================================
    # Top: Classical Picture
    # =========================================================================
    ax_classical.set_xlim(0, 20)
    ax_classical.set_ylim(0, 5)
    ax_classical.set_aspect('equal')

    # Draw potential barrier
    barrier = Rectangle((8, 0), 4, 4, facecolor='#d0d0d0', edgecolor='black',
                        linewidth=2, hatch='///')
    ax_classical.add_patch(barrier)
    ax_classical.text(10, 2, 'Barrier\nV > E', ha='center', va='center',
                      fontsize=10, fontweight='bold')

    # Draw particle approaching
    particle = Circle((4, 2), 0.5, facecolor=COLORS['matter'],
                      edgecolor='black', linewidth=2)
    ax_classical.add_patch(particle)
    ax_classical.annotate('', xy=(6, 2), xytext=(4.7, 2),
                         arrowprops=dict(arrowstyle='->', color='black', lw=2))

    # Draw particle bouncing back
    particle_bounce = Circle((3, 3), 0.4, facecolor=COLORS['matter'],
                             edgecolor='black', linewidth=2, alpha=0.5)
    ax_classical.add_patch(particle_bounce)
    ax_classical.annotate('', xy=(1.5, 3.5), xytext=(2.8, 3.2),
                         arrowprops=dict(arrowstyle='->', color='gray', lw=1.5))

    # X mark for transmission
    ax_classical.text(14, 2, 'X', fontsize=30, color='red', ha='center', va='center')
    ax_classical.text(14, 0.5, 'No transmission', fontsize=10, ha='center',
                      color='red', style='italic')

    ax_classical.set_title('Classical Picture: Particle Reflects',
                          fontsize=13, fontweight='bold')
    ax_classical.axis('off')

    # =========================================================================
    # Middle: TRD Picture
    # =========================================================================
    ax_trd.set_xlim(0, 20)
    ax_trd.set_ylim(-1, 6)

    # Draw barrier
    barrier_trd = Rectangle((8, 0), 4, 4, facecolor='#d0d0d0', edgecolor='black',
                            linewidth=2, alpha=0.6)
    ax_trd.add_patch(barrier_trd)
    ax_trd.text(10, 4.5, 'Potential Barrier', ha='center', fontsize=10)

    # Draw lattice positions
    for x in range(1, 20):
        ax_trd.axvline(x, color='gray', linewidth=0.5, alpha=0.3)
        if x < 8 or x > 12:
            ax_trd.text(x, -0.5, str(x), ha='center', fontsize=8, color='gray')

    # Draw particle at integer position
    particle_trd = Circle((5, 2), 0.5, facecolor=COLORS['matter'],
                          edgecolor='black', linewidth=2)
    ax_trd.add_patch(particle_trd)
    ax_trd.text(5, 2, '+', fontsize=14, ha='center', va='center',
                color='white', fontweight='bold')
    ax_trd.text(5, -0.8, 'Integer\nPosition', ha='center', fontsize=8)

    # Draw flux field extending through barrier
    x_flux = np.linspace(2, 18, 200)
    # Flux decays exponentially inside barrier
    flux = np.zeros_like(x_flux)
    for i, x in enumerate(x_flux):
        if x < 8:  # Before barrier
            flux[i] = 2.5 * np.exp(-0.3 * (5 - x)**2)
        elif x <= 12:  # Inside barrier
            # Exponential decay
            flux[i] = 2.5 * np.exp(-0.3 * 9) * np.exp(-0.8 * (x - 8))
        else:  # After barrier
            # Small transmitted amplitude
            flux[i] = 2.5 * np.exp(-0.3 * 9) * np.exp(-0.8 * 4) * np.exp(-0.1 * (x - 12)**2)

    ax_trd.fill_between(x_flux, 0, flux, alpha=0.4, color=COLORS['highlight'])
    ax_trd.plot(x_flux, flux, color=COLORS['highlight'], linewidth=2,
                label='Flux density |J|')

    # Mark KB threshold
    ax_trd.axhline(y=0.3, color=COLORS['accent1'], linestyle='--', linewidth=1.5)
    ax_trd.text(1, 0.4, 'KB threshold', fontsize=9, color=COLORS['accent1'])

    # Show possible manifestation on far side
    # Small probability region
    ax_trd.annotate('Flux > KB here\n(small probability)',
                    xy=(14, 0.35), xytext=(15, 1.5),
                    fontsize=9, ha='center',
                    arrowprops=dict(arrowstyle='->', color='green', lw=1.5))

    # Ghost particle (possible manifestation)
    particle_ghost = Circle((14, 2), 0.4, facecolor=COLORS['matter'],
                            edgecolor='black', linewidth=1.5, alpha=0.3,
                            linestyle='--')
    ax_trd.add_patch(particle_ghost)
    ax_trd.text(14, 2.8, 'Possible\nGenesis', ha='center', fontsize=8,
                color='green', style='italic')

    ax_trd.set_title('TRD Picture: Flux Penetrates, Enables Remote Manifestation',
                     fontsize=13, fontweight='bold')
    ax_trd.set_xlabel('Position (lattice units)')
    ax_trd.set_ylabel('Flux Density')
    ax_trd.legend(loc='upper right')
    ax_trd.set_ylim(-1, 5)

    # =========================================================================
    # Bottom: Transmission probability
    # =========================================================================
    # T ~ exp(-2*kappa*L) where kappa ~ sqrt(V-E)
    barrier_widths = np.linspace(0.5, 10, 100)
    kappa = 0.5  # Decay constant

    T = np.exp(-2 * kappa * barrier_widths)

    ax_prob.semilogy(barrier_widths, T, color=COLORS['accent1'], linewidth=2.5)
    ax_prob.fill_between(barrier_widths, T, alpha=0.2, color=COLORS['accent1'])

    ax_prob.set_xlabel('Barrier Width (lattice units)', fontsize=11)
    ax_prob.set_ylabel('Transmission Probability T', fontsize=11)
    ax_prob.set_title('Tunneling Probability vs Barrier Width', fontsize=13,
                      fontweight='bold')

    # Mark some reference points
    ax_prob.axhline(y=0.01, color='gray', linestyle=':', alpha=0.5)
    ax_prob.text(9, 0.015, '1%', fontsize=9, color='gray')

    ax_prob.axhline(y=0.001, color='gray', linestyle=':', alpha=0.5)
    ax_prob.text(9, 0.0015, '0.1%', fontsize=9, color='gray')

    # Formula
    ax_prob.text(0.95, 0.95, r'$T \approx e^{-2\kappa L}$' + '\n' +
                 r'$\kappa \sim \sqrt{V-E}$',
                 transform=ax_prob.transAxes, fontsize=11, ha='right', va='top',
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    ax_prob.grid(True, alpha=0.3)
    ax_prob.set_xlim(0, 10)
    ax_prob.set_ylim(1e-5, 1)

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Tunneling in TRD: Flux Extends Beyond Particle Position',
                fontsize=16, fontweight='bold', y=0.99)

    explanation = (
        "In TRD, 'tunneling' occurs because the flux field (not the particle) extends continuously.\n"
        "When flux beyond a barrier exceeds KB, manifestation can occur there - the particle 'tunnels'."
    )
    fig.text(0.5, 0.01, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.96])

    return fig


if __name__ == '__main__':
    fig = generate_tunneling()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_2_8_tunneling.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
