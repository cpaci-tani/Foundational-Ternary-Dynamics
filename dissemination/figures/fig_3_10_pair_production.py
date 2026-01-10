"""
Figure 3.10: Pair Production Detail
===================================
Detailed view of electron-positron pair production
from high-energy photon interaction.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, Wedge
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_pair_production():
    """Generate the pair production detail visualization."""
    fig, ax = plt.subplots(figsize=(12, 8), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 8)
    ax.axis('off')

    # Title
    ax.set_title('Pair Production: Photon -> Electron + Positron', fontsize=14,
                fontweight='bold')

    # Stage 1: Incoming photon
    ax.text(1.5, 7, 'Stage 1: High-Energy Photon', fontsize=11, fontweight='bold')

    # Wavy photon line
    t = np.linspace(0, 2, 100)
    x_wave = 0.5 + t
    y_wave = 6 + 0.2 * np.sin(15 * t)
    ax.plot(x_wave, y_wave, color=COLORS['highlight'], linewidth=3)
    ax.annotate('', xy=(3, 6), xytext=(2.7, 6),
               arrowprops=dict(arrowstyle='->', color=COLORS['highlight'], lw=2))
    ax.text(1.5, 5.3, 'E > 2m_e c^2 = 1.022 MeV', fontsize=10, ha='center')

    # Stage 2: Nucleus interaction
    ax.text(5.5, 7, 'Stage 2: Near Nucleus', fontsize=11, fontweight='bold')

    # Nucleus
    ax.add_patch(Circle((5.5, 5.5), 0.5, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=2))
    ax.text(5.5, 5.5, 'Z', fontsize=12, ha='center', va='center', color='white',
            fontweight='bold')

    # Photon approaching
    t = np.linspace(0, 1.5, 75)
    x_wave2 = 3.5 + t
    y_wave2 = 6 + 0.15 * np.sin(15 * t)
    ax.plot(x_wave2, y_wave2, color=COLORS['highlight'], linewidth=2)

    # Electric field lines from nucleus
    for angle in np.linspace(0, 2*np.pi, 8, endpoint=False):
        dx = 1.2 * np.cos(angle)
        dy = 1.2 * np.sin(angle)
        ax.annotate('', xy=(5.5 + dx, 5.5 + dy), xytext=(5.5 + 0.6*np.cos(angle),
                   5.5 + 0.6*np.sin(angle)),
                   arrowprops=dict(arrowstyle='->', color='gray', lw=1, alpha=0.5))

    ax.text(5.5, 4, 'Nuclear field provides\nmomentum conservation', fontsize=9,
            ha='center')

    # Stage 3: Pair creation
    ax.text(9.5, 7, 'Stage 3: Pair Created', fontsize=11, fontweight='bold')

    # Electron
    ax.add_patch(Circle((8.5, 6.5), 0.3, facecolor=COLORS['antimatter'],
                        edgecolor='black', linewidth=2))
    ax.text(8.5, 6.5, 'e-', fontsize=10, ha='center', va='center', color='white')
    ax.annotate('', xy=(8, 7.2), xytext=(8.3, 6.8),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))

    # Positron
    ax.add_patch(Circle((10.5, 6.5), 0.3, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=2))
    ax.text(10.5, 6.5, 'e+', fontsize=10, ha='center', va='center', color='white')
    ax.annotate('', xy=(11, 7.2), xytext=(10.7, 6.8),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))

    ax.text(9.5, 5.5, 'Opposite charges\nOpposite momenta', fontsize=9, ha='center')

    # TRD interpretation box
    trd_text = (
        "TRD Mechanism:\n\n"
        "1. High flux density (from photon) exceeds KB\n"
        "2. Genesis condition met\n"
        "3. Divergence sign determines polarity:\n"
        "   - Positive divergence -> +1 (positron)\n"
        "   - Negative divergence -> -1 (electron)\n"
        "4. Shared UUID assigned (entanglement)"
    )
    ax.text(6, 1.8, trd_text, fontsize=10, ha='center', va='center',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2),
            family='monospace')

    return fig


if __name__ == '__main__':
    fig = generate_pair_production()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_3_10_pair_production.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
