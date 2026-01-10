"""
Figure 3.22: Superconductivity
==============================
Shows Cooper pairing and flux quantization
from TRD perspective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, Ellipse, Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_superconductivity():
    """Generate the superconductivity visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Normal vs Superconducting resistance
    ax = axes[0, 0]

    T = np.linspace(0, 20, 200)
    T_c = 10  # Critical temperature

    # Normal metal: R ~ T at low T (phonon scattering)
    R_normal = 0.5 + 0.03 * T

    # Superconductor: R drops to zero below Tc
    R_super = np.where(T < T_c, 0, 0.5 + 0.03 * (T - T_c))

    ax.plot(T, R_normal, 'b-', linewidth=2, label='Normal metal')
    ax.plot(T, R_super, 'r-', linewidth=2, label='Superconductor')
    ax.axvline(x=T_c, color='gray', linestyle='--', alpha=0.5)
    ax.text(T_c + 0.5, 0.7, 'Tc', fontsize=10)

    ax.set_xlabel('Temperature T', fontsize=10)
    ax.set_ylabel('Resistance R', fontsize=10)
    ax.set_title('Resistance vs Temperature', fontsize=11, fontweight='bold')
    ax.legend(loc='upper left', fontsize=9)
    ax.set_xlim(0, 20)
    ax.set_ylim(0, 1.2)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    # Panel 2: Cooper pair formation
    ax = axes[0, 1]
    ax.set_xlim(-4, 4)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Cooper Pair: Phonon-Mediated Attraction', fontsize=11, fontweight='bold')

    # Draw lattice deformation
    for i in range(-3, 4):
        for j in range(-2, 3):
            # Slight deformation near electron 1 position
            dist1 = np.sqrt((i + 1.5)**2 + j**2)
            offset = 0.15 / (1 + dist1) if dist1 > 0 else 0

            ax.add_patch(Circle((i - offset * np.sign(i + 1.5), j), 0.15,
                               facecolor=COLORS['matter'], edgecolor='black',
                               linewidth=0.5, alpha=0.7))

    # Electron 1
    ax.add_patch(Circle((-1.5, 0), 0.25, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=2))
    ax.text(-1.5, 0, 'e-', fontsize=9, ha='center', va='center', color='white')

    # Electron 2
    ax.add_patch(Circle((1.5, 0), 0.25, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=2))
    ax.text(1.5, 0, 'e-', fontsize=9, ha='center', va='center', color='white')

    # Phonon exchange (wavy line)
    t = np.linspace(-1.2, 1.2, 50)
    wave = 0.3 * np.sin(8 * t)
    ax.plot(t, wave, 'orange', linewidth=2)
    ax.text(0, 0.6, 'phonon', fontsize=9, ha='center', color='orange')

    # Correlation ellipse
    ellipse = Ellipse((0, 0), 4, 1.5, fill=False, edgecolor='green',
                      linestyle='--', linewidth=2)
    ax.add_patch(ellipse)
    ax.text(0, -1.2, 'Coherence length xi', fontsize=9, ha='center', color='green')

    ax.text(0, -2.5, 'Lattice deformation creates attractive potential',
            fontsize=9, ha='center')

    # Panel 3: Meissner effect
    ax = axes[1, 0]
    ax.set_xlim(-4, 4)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Meissner Effect: Flux Expulsion', fontsize=11, fontweight='bold')

    # Superconductor (gray rectangle)
    rect = Rectangle((-2, -1.5), 4, 3, facecolor='lightgray',
                    edgecolor='black', linewidth=2)
    ax.add_patch(rect)
    ax.text(0, 0, 'Superconductor\nB = 0 inside', fontsize=10, ha='center', va='center')

    # External field lines bending around
    for y_start in np.linspace(-2.5, 2.5, 7):
        # Field line bends around superconductor
        if abs(y_start) < 1.3:
            # Lines that would go through - bend around
            x = np.linspace(-4, -2, 20)
            y = np.full_like(x, y_start)
            ax.plot(x, y, 'b-', linewidth=1)

            # Bend around top or bottom
            theta = np.linspace(np.pi, 0 if y_start > 0 else 2*np.pi, 20)
            r = 1.5 + abs(y_start) * 0.3
            x_arc = r * np.cos(theta)
            y_arc = (1.5 if y_start > 0 else -1.5) + 0.5 * np.sin(theta) * np.sign(y_start)
            ax.plot(x_arc, y_arc, 'b-', linewidth=1)

            x = np.linspace(2, 4, 20)
            ax.plot(x, np.full_like(x, y_start), 'b-', linewidth=1)
        else:
            # Lines that pass above/below
            ax.plot([-4, 4], [y_start, y_start], 'b-', linewidth=1)

    ax.annotate('', xy=(3.5, 2), xytext=(3.5, 1),
               arrowprops=dict(arrowstyle='->', color='blue', lw=1.5))
    ax.text(3.8, 1.5, 'B', fontsize=10, color='blue')

    # Panel 4: TRD interpretation
    ax = axes[1, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('TRD View of Superconductivity', fontsize=11, fontweight='bold')

    # Draw Cooper pair as correlated flux entities
    ax.add_patch(Circle((3, 7), 0.4, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=2))
    ax.add_patch(Circle((5, 7), 0.4, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=2))

    # Flux correlation
    t = np.linspace(3.4, 4.6, 30)
    wave = 7 + 0.3 * np.sin(15 * (t - 3.4))
    ax.plot(t, wave, color=COLORS['accent1'], linewidth=2)

    ax.text(4, 7.8, 'Correlated flux', fontsize=9, ha='center', color=COLORS['accent1'])
    ax.text(3, 6.3, 'k up', fontsize=8, ha='center')
    ax.text(5, 6.3, '-k down', fontsize=8, ha='center')

    # Energy gap diagram
    ax.plot([1, 7], [4, 4], 'k-', linewidth=2)
    ax.plot([1, 7], [3, 3], 'k-', linewidth=2)
    ax.fill_between([1, 7], [3, 3], [4, 4], color=COLORS['void'], alpha=0.3)
    ax.text(4, 3.5, 'Energy Gap 2Delta', fontsize=9, ha='center')
    ax.text(4, 4.3, 'Excited states', fontsize=8, ha='center')
    ax.text(4, 2.6, 'Cooper pair condensate', fontsize=8, ha='center')

    trd_text = (
        "TRD Superconductivity:\n\n"
        "1. Cooper pairs: correlated flux entities\n"
        "   (opposite k, opposite spin)\n\n"
        "2. Gap: minimum energy to break pair\n"
        "   Delta ~ KB × exp(-1/N(0)V)\n\n"
        "3. Meissner: flux coherence excludes\n"
        "   external field perturbations\n\n"
        "4. Zero resistance: coherent flux flow\n"
        "   has no scattering channels"
    )
    ax.text(5, 1.5, trd_text, fontsize=8, ha='center', va='bottom',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    fig.suptitle('Superconductivity: Coherent Flux Condensation',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_superconductivity()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_22_superconductivity.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
