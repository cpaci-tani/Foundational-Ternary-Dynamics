"""
Figure 3.20: Phonon Propagation
===============================
Shows lattice vibrations and phonon modes as flux
wave propagation through discrete lattice.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_phonon_propagation():
    """Generate the phonon propagation visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Longitudinal (acoustic) mode
    ax = axes[0, 0]
    ax.set_xlim(-1, 15)
    ax.set_ylim(-2, 2)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Longitudinal (Acoustic) Mode', fontsize=11, fontweight='bold')

    # Draw atoms with longitudinal displacement
    n_atoms = 15
    k = 2 * np.pi / 5  # wavevector
    amplitude = 0.4

    for i in range(n_atoms):
        # Displacement along propagation direction
        displacement = amplitude * np.sin(k * i)
        x = i + displacement
        ax.add_patch(Circle((x, 0), 0.3, facecolor=COLORS['matter'],
                           edgecolor='black', linewidth=1))
        # Equilibrium position marker
        ax.plot([i, i], [-0.1, 0.1], 'k--', alpha=0.3)

    ax.annotate('', xy=(14, 1.5), xytext=(0, 1.5),
               arrowprops=dict(arrowstyle='->', color='blue', lw=2))
    ax.text(7, 1.8, 'Wave propagation', fontsize=9, ha='center', color='blue')

    # Panel 2: Transverse (optical) mode
    ax = axes[0, 1]
    ax.set_xlim(-1, 15)
    ax.set_ylim(-2, 2)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Transverse (Optical) Mode', fontsize=11, fontweight='bold')

    for i in range(n_atoms):
        # Displacement perpendicular to propagation
        displacement = amplitude * np.sin(k * i)
        y = displacement
        ax.add_patch(Circle((i, y), 0.3, facecolor=COLORS['antimatter'],
                           edgecolor='black', linewidth=1))
        # Equilibrium position marker
        ax.plot([i-0.1, i+0.1], [0, 0], 'k--', alpha=0.3)

    ax.annotate('', xy=(14, 1.5), xytext=(0, 1.5),
               arrowprops=dict(arrowstyle='->', color='blue', lw=2))
    ax.text(7, 1.8, 'Wave propagation', fontsize=9, ha='center', color='blue')

    # Panel 3: Dispersion relation
    ax = axes[1, 0]

    k_vals = np.linspace(0, np.pi, 100)

    # Acoustic branch: omega ~ |k| for small k
    omega_acoustic = 2 * np.sin(k_vals / 2)

    # Optical branch: omega starts at finite value
    omega_optical = np.sqrt(3 + np.cos(k_vals))

    ax.plot(k_vals, omega_acoustic, color=COLORS['matter'], linewidth=2,
            label='Acoustic branch')
    ax.plot(k_vals, omega_optical, color=COLORS['antimatter'], linewidth=2,
            label='Optical branch')

    ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
    ax.axvline(x=0, color='gray', linestyle='-', linewidth=0.5)

    ax.set_xlabel('Wavevector k', fontsize=10)
    ax.set_ylabel('Frequency omega', fontsize=10)
    ax.set_title('Phonon Dispersion Relation', fontsize=11, fontweight='bold')
    ax.set_xlim(0, np.pi)
    ax.set_ylim(0, 2.5)
    ax.set_xticks([0, np.pi/2, np.pi])
    ax.set_xticklabels(['0', 'pi/2', 'pi'])
    ax.legend(loc='upper right', fontsize=9)

    # Remove top and right spines
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    # Panel 4: TRD interpretation
    ax = axes[1, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('TRD Phonon Model', fontsize=11, fontweight='bold')

    # Draw lattice with flux connections
    for i in range(5):
        for j in range(5):
            x = 2 + i * 1.5
            y = 2 + j * 1.5

            # Atom
            ax.add_patch(Circle((x, y), 0.25, facecolor=COLORS['matter'],
                               edgecolor='black', linewidth=1))

            # Flux connections (springs)
            if i < 4:
                ax.plot([x + 0.25, x + 1.25], [y, y], 'b-', linewidth=1, alpha=0.5)
            if j < 4:
                ax.plot([x, x], [y + 0.25, y + 1.25], 'b-', linewidth=1, alpha=0.5)

    # Add flux wave overlay
    for i in range(5):
        x = 2 + i * 1.5
        y = 2 + 2 * 1.5  # Middle row
        displacement = 0.3 * np.sin(2 * np.pi * i / 4)
        ax.annotate('', xy=(x + displacement, y + 0.4), xytext=(x, y + 0.35),
                   arrowprops=dict(arrowstyle='->', color='red', lw=1.5))

    trd_text = (
        "In TRD:\n"
        "- Phonons = collective flux oscillations\n"
        "- Acoustic: in-phase flux waves\n"
        "- Optical: out-of-phase flux waves\n"
        "- Discreteness naturally quantizes modes"
    )
    ax.text(5, 0.5, trd_text, fontsize=9, ha='center',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    fig.suptitle('Phonon Propagation: Lattice Vibrations as Flux Waves',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_phonon_propagation()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_20_phonon_propagation.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
