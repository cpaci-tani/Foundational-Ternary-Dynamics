"""
Figure 3.24: Electric Polarization
==================================
Shows dielectric polarization and ferroelectricity
from TRD charge displacement perspective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Rectangle, Ellipse, FancyArrowPatch
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_electric_polarization():
    """Generate the electric polarization visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Unpolarized vs Polarized dielectric
    ax = axes[0, 0]
    ax.set_xlim(-1, 14)
    ax.set_ylim(-1, 6)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Dielectric Polarization', fontsize=11, fontweight='bold')

    # Unpolarized (left)
    ax.add_patch(Rectangle((0, 0), 5, 5, facecolor='#E0E0E0',
                           edgecolor='black', linewidth=2))
    ax.text(2.5, 5.3, 'No Field', fontsize=10, ha='center')

    np.random.seed(42)
    for _ in range(12):
        x = np.random.uniform(0.5, 4.5)
        y = np.random.uniform(0.5, 4.5)
        angle = np.random.uniform(0, 360)

        # Random dipole orientation
        dx = 0.3 * np.cos(np.radians(angle))
        dy = 0.3 * np.sin(np.radians(angle))

        ax.add_patch(Circle((x - dx, y - dy), 0.15, facecolor=COLORS['antimatter'],
                           edgecolor='black', linewidth=0.5))
        ax.add_patch(Circle((x + dx, y + dy), 0.15, facecolor=COLORS['matter'],
                           edgecolor='black', linewidth=0.5))
        ax.plot([x - dx, x + dx], [y - dy, y + dy], 'k-', linewidth=1)

    # Polarized (right)
    ax.add_patch(Rectangle((8, 0), 5, 5, facecolor='#E0E0E0',
                           edgecolor='black', linewidth=2))
    ax.text(10.5, 5.3, 'Applied Field E', fontsize=10, ha='center')

    # Field arrow
    ax.annotate('', xy=(14, 2.5), xytext=(13, 2.5),
               arrowprops=dict(arrowstyle='->', color='blue', lw=2))
    ax.text(13.5, 3, 'E', fontsize=12, color='blue')

    for i in range(3):
        for j in range(4):
            x = 9 + i * 1.5
            y = 0.8 + j * 1.1

            # Aligned dipoles
            ax.add_patch(Circle((x - 0.25, y), 0.15, facecolor=COLORS['antimatter'],
                               edgecolor='black', linewidth=0.5))
            ax.add_patch(Circle((x + 0.25, y), 0.15, facecolor=COLORS['matter'],
                               edgecolor='black', linewidth=0.5))
            ax.plot([x - 0.25, x + 0.25], [y, y], 'k-', linewidth=1)

    ax.text(6.5, 2.5, '-->', fontsize=20, ha='center', va='center')

    # Panel 2: Electronic vs Ionic vs Orientational polarization
    ax = axes[0, 1]
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 9)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Types of Polarization', fontsize=11, fontweight='bold')

    # Electronic polarization
    ax.text(2, 8.5, 'Electronic', fontsize=10, fontweight='bold', ha='center')
    ax.add_patch(Circle((2, 7), 0.6, facecolor='none',
                       edgecolor=COLORS['antimatter'], linewidth=2, linestyle='--'))
    ax.add_patch(Circle((2.2, 7), 0.6, facecolor='none',
                       edgecolor=COLORS['antimatter'], linewidth=2))
    ax.add_patch(Circle((2, 7), 0.2, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=1))
    ax.annotate('', xy=(3, 7), xytext=(2.5, 7),
               arrowprops=dict(arrowstyle='->', color='blue', lw=1))
    ax.text(2, 6, 'Electron cloud\nshifts', fontsize=8, ha='center')

    # Ionic polarization
    ax.text(6, 8.5, 'Ionic', fontsize=10, fontweight='bold', ha='center')
    # Before
    ax.add_patch(Circle((5.3, 7.3), 0.25, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=1))
    ax.text(5.3, 7.3, '+', fontsize=8, ha='center', va='center', color='white')
    ax.add_patch(Circle((6.0, 7.3), 0.25, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=1))
    ax.text(6.0, 7.3, '-', fontsize=8, ha='center', va='center', color='white')
    ax.add_patch(Circle((6.7, 7.3), 0.25, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=1))
    ax.text(6.7, 7.3, '+', fontsize=8, ha='center', va='center', color='white')
    # After
    ax.add_patch(Circle((5.1, 6.5), 0.25, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=1, alpha=0.5))
    ax.add_patch(Circle((6.0, 6.5), 0.25, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=1, alpha=0.5))
    ax.add_patch(Circle((6.9, 6.5), 0.25, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=1, alpha=0.5))
    ax.text(6, 5.8, 'Ions shift\npositions', fontsize=8, ha='center')

    # Orientational polarization
    ax.text(10, 8.5, 'Orientational', fontsize=10, fontweight='bold', ha='center')
    # Random dipoles rotating
    angles = [45, 120, -30]
    for i, angle in enumerate(angles):
        x = 9.2 + i * 0.8
        dx = 0.25 * np.cos(np.radians(angle))
        dy = 0.25 * np.sin(np.radians(angle))
        ax.plot([x - dx, x + dx], [7.3 - dy, 7.3 + dy], 'gray', linewidth=2, alpha=0.5)
        ax.plot([x - 0.25, x + 0.25], [6.5, 6.5], 'k-', linewidth=2)
    ax.text(10, 5.8, 'Dipoles align\nwith field', fontsize=8, ha='center')

    # Frequency response
    ax.plot([1, 11], [4, 4], 'k-', linewidth=1)
    ax.text(6, 4.3, 'Frequency response', fontsize=9, ha='center')
    ax.text(2, 3.5, 'Electronic:\n~10^15 Hz', fontsize=8, ha='center')
    ax.text(6, 3.5, 'Ionic:\n~10^12 Hz', fontsize=8, ha='center')
    ax.text(10, 3.5, 'Orientational:\n~10^9 Hz', fontsize=8, ha='center')

    # Panel 3: Ferroelectric hysteresis
    ax = axes[1, 0]

    E = np.linspace(-2, 2, 500)
    P_up = np.tanh(2 * (E + 0.4))
    P_down = np.tanh(2 * (E - 0.4))

    ax.plot(E[E <= 0], P_up[E <= 0], 'b-', linewidth=2)
    ax.plot(E[E >= 0], P_down[E >= 0], 'b-', linewidth=2)
    ax.plot(E[E >= 0], P_up[E >= 0], 'r-', linewidth=2)
    ax.plot(E[E <= 0], P_down[E <= 0], 'r-', linewidth=2)

    ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
    ax.axvline(x=0, color='gray', linestyle='-', linewidth=0.5)

    ax.plot(0, np.tanh(0.8), 'ko', markersize=8)
    ax.annotate('Pr', xy=(0.15, np.tanh(0.8)), fontsize=10)

    ax.plot(-0.4, 0, 'ko', markersize=8)
    ax.annotate('-Ec', xy=(-0.6, 0.15), fontsize=10)

    ax.set_xlabel('Electric Field E', fontsize=10)
    ax.set_ylabel('Polarization P', fontsize=10)
    ax.set_title('Ferroelectric Hysteresis', fontsize=11, fontweight='bold')
    ax.set_xlim(-2, 2)
    ax.set_ylim(-1.5, 1.5)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    ax.text(1.3, -1.2, 'Pr = Remnant polarization\nEc = Coercive field', fontsize=9)

    # Panel 4: TRD interpretation
    ax = axes[1, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('TRD View of Polarization', fontsize=11, fontweight='bold')

    # Show flux displacement
    for i in range(3):
        x = 2 + i * 2
        # Nucleus (positive)
        ax.add_patch(Circle((x, 7.5), 0.3, facecolor=COLORS['matter'],
                           edgecolor='black', linewidth=1))
        ax.text(x, 7.5, '+', fontsize=10, ha='center', va='center', color='white')

        # Electron cloud (displaced)
        ellipse = Ellipse((x + 0.3, 7.5), 1.2, 0.8, fill=False,
                         edgecolor=COLORS['antimatter'], linewidth=2)
        ax.add_patch(ellipse)

        # Dipole moment arrow
        ax.annotate('', xy=(x + 0.5, 6.7), xytext=(x, 6.7),
                   arrowprops=dict(arrowstyle='->', color='green', lw=2))

    ax.text(4, 6.2, 'Induced dipole moment p', fontsize=9, ha='center', color='green')

    trd_text = (
        "TRD Polarization:\n\n"
        "P = epsilon_0 chi_e E\n"
        "chi_e = electric susceptibility\n\n"
        "- Electronic: flux cloud deformation\n"
        "- Ionic: flux center displacement\n"
        "- Orientational: flux dipole rotation\n\n"
        "Ferroelectrics:\n"
        "- Spontaneous flux asymmetry\n"
        "- Stable above Curie temperature\n"
        "- Hysteresis from domain switching\n\n"
        "D = epsilon_0 E + P (constitutive relation)"
    )
    ax.text(5, 4.5, trd_text, fontsize=9, ha='center', va='top',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    fig.suptitle('Electric Polarization: Charge Displacement in Fields',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_electric_polarization()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_24_electric_polarization.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
