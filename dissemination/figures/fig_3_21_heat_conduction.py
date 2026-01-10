"""
Figure 3.21: Heat Conduction
============================
Shows thermal energy transport via phonon and
electron mechanisms in TRD framework.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, Rectangle
from matplotlib.colors import LinearSegmentedColormap
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_heat_conduction():
    """Generate the heat conduction visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Custom hot-cold colormap
    colors_hot = ['#0000FF', '#00FFFF', '#00FF00', '#FFFF00', '#FF0000']
    hot_cmap = LinearSegmentedColormap.from_list('hot_cold', colors_hot)

    # Panel 1: Temperature gradient in lattice
    ax = axes[0, 0]
    ax.set_xlim(-1, 12)
    ax.set_ylim(-1, 6)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Temperature Gradient in Lattice', fontsize=11, fontweight='bold')

    # Draw atoms with temperature coloring
    n_x, n_y = 12, 6
    for i in range(n_x):
        for j in range(n_y):
            # Temperature decreases from left to right
            temp = 1 - i / (n_x - 1)
            color = hot_cmap(temp)

            # Vibration amplitude proportional to temperature
            amplitude = 0.15 * temp
            offset_x = amplitude * np.random.randn()
            offset_y = amplitude * np.random.randn()

            ax.add_patch(Circle((i + offset_x, j + offset_y), 0.2,
                               facecolor=color, edgecolor='black', linewidth=0.5))

    ax.text(0, -0.7, 'HOT', fontsize=10, ha='center', color='red', fontweight='bold')
    ax.text(11, -0.7, 'COLD', fontsize=10, ha='center', color='blue', fontweight='bold')

    ax.annotate('', xy=(11, 5.5), xytext=(0, 5.5),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))
    ax.text(5.5, 5.8, 'Heat flow Q', fontsize=9, ha='center')

    # Panel 2: Phonon transport mechanism
    ax = axes[0, 1]
    ax.set_xlim(-1, 12)
    ax.set_ylim(-1, 4)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Phonon Heat Transport', fontsize=11, fontweight='bold')

    # Draw lattice atoms
    for i in range(12):
        for j in range(4):
            ax.add_patch(Circle((i, j), 0.2, facecolor=COLORS['void'],
                               edgecolor='black', linewidth=0.5))

    # Draw phonon wave packet traveling
    packet_center = 5
    packet_width = 2
    for i in range(12):
        if abs(i - packet_center) < packet_width:
            y_offset = 0.3 * np.exp(-0.5 * ((i - packet_center) / 0.8)**2) * np.sin(3 * i)
            ax.add_patch(Circle((i, 2 + y_offset), 0.22,
                               facecolor='orange', edgecolor='black', linewidth=1))
            ax.annotate('', xy=(i + 0.3, 2 + y_offset), xytext=(i - 0.1, 2 + y_offset),
                       arrowprops=dict(arrowstyle='->', color='orange', lw=1))

    ax.text(5, 3.5, 'Phonon wave packet', fontsize=9, ha='center', color='orange')
    ax.text(5, -0.5, 'Carries vibrational energy from hot to cold',
            fontsize=9, ha='center')

    # Panel 3: Electron transport in metals
    ax = axes[1, 0]
    ax.set_xlim(-1, 12)
    ax.set_ylim(-1, 4)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Electron Heat Transport (Metals)', fontsize=11, fontweight='bold')

    # Draw ion cores
    for i in range(12):
        for j in range(4):
            ax.add_patch(Circle((i, j), 0.25, facecolor=COLORS['matter'],
                               edgecolor='black', linewidth=1))
            ax.text(i, j, '+', fontsize=8, ha='center', va='center', color='white')

    # Draw electrons moving from hot to cold
    np.random.seed(42)
    for _ in range(8):
        x = np.random.uniform(1, 10)
        y = np.random.uniform(0.5, 3.5)
        # Drift velocity toward cold end
        ax.add_patch(Circle((x, y), 0.12, facecolor=COLORS['antimatter'],
                           edgecolor='black', linewidth=0.5))
        ax.annotate('', xy=(x + 0.4, y + np.random.uniform(-0.1, 0.1)),
                   xytext=(x + 0.15, y),
                   arrowprops=dict(arrowstyle='->', color=COLORS['antimatter'], lw=1))

    ax.text(5, -0.5, 'Fast electrons carry kinetic energy from hot region',
            fontsize=9, ha='center')

    # Panel 4: Thermal conductivity comparison
    ax = axes[1, 1]

    materials = ['Diamond', 'Copper', 'Silicon', 'Glass', 'Air']
    k_values = [2000, 400, 150, 1, 0.025]  # W/(m·K) approximate
    colors = [COLORS['accent1'], COLORS['matter'], COLORS['antimatter'],
              COLORS['void'], 'white']

    bars = ax.barh(materials, k_values, color=colors, edgecolor='black')
    ax.set_xscale('log')
    ax.set_xlabel('Thermal Conductivity k [W/(m·K)]', fontsize=10)
    ax.set_title('Thermal Conductivity Comparison', fontsize=11, fontweight='bold')

    # Annotate mechanism
    ax.annotate('Phonon\ndominated', xy=(2000, 0), xytext=(500, -0.5),
               fontsize=8, ha='center')
    ax.annotate('Electron\ndominated', xy=(400, 1), xytext=(100, 0.5),
               fontsize=8, ha='center')

    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    # TRD interpretation box
    trd_text = (
        "TRD Heat Transport:\n"
        "Q = -k grad(T)\n\n"
        "k = (1/3) C v l\n"
        "C = heat capacity (flux energy)\n"
        "v = carrier velocity\n"
        "l = mean free path\n\n"
        "Discrete lattice naturally limits l"
    )
    ax.text(0.95, 0.95, trd_text, transform=ax.transAxes, fontsize=8,
            va='top', ha='right',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    fig.suptitle('Heat Conduction: Energy Transport via Flux Carriers',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_heat_conduction()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_21_heat_conduction.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
