"""
Figure 3.25: Star Formation
===========================
Shows gravitational collapse and fusion ignition
from TRD flux accumulation perspective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge
from matplotlib.colors import LinearSegmentedColormap
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_star_formation():
    """Generate the star formation visualization."""
    fig, axes = plt.subplots(2, 3, figsize=(15, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Custom colormap for density
    density_colors = ['#000033', '#003366', '#006699', '#3399CC',
                     '#66CCFF', '#FFFF99', '#FFCC00', '#FF6600', '#FF0000']
    density_cmap = LinearSegmentedColormap.from_list('density', density_colors)

    titles = [
        '1. Molecular Cloud',
        '2. Fragmentation',
        '3. Protostellar Core',
        '4. Accretion Disk',
        '5. T-Tauri Phase',
        '6. Main Sequence Star'
    ]

    for idx, ax in enumerate(axes.flat):
        ax.set_xlim(-5, 5)
        ax.set_ylim(-5, 5)
        ax.set_aspect('equal')
        ax.axis('off')
        ax.set_title(titles[idx], fontsize=11, fontweight='bold')

        if idx == 0:
            # Molecular cloud - diffuse gas
            np.random.seed(42)
            for _ in range(200):
                x = np.random.normal(0, 2)
                y = np.random.normal(0, 2)
                size = np.random.uniform(0.05, 0.15)
                alpha = np.random.uniform(0.1, 0.4)
                ax.add_patch(Circle((x, y), size, facecolor='gray', alpha=alpha))

            ax.text(0, -4.5, 'Diffuse gas and dust\n(~10-100 particles/cm^3)',
                   fontsize=8, ha='center')

        elif idx == 1:
            # Fragmentation - multiple clumps
            np.random.seed(123)
            centers = [(-2, 1.5), (1.5, 2), (-1, -1.5), (2, -1)]
            for cx, cy in centers:
                for _ in range(50):
                    x = np.random.normal(cx, 0.6)
                    y = np.random.normal(cy, 0.6)
                    r = np.sqrt((x - cx)**2 + (y - cy)**2)
                    alpha = 0.5 * np.exp(-r / 0.8)
                    ax.add_patch(Circle((x, y), 0.08, facecolor='blue', alpha=alpha))

            ax.text(0, -4.5, 'Jeans instability\ncauses fragmentation',
                   fontsize=8, ha='center')

        elif idx == 2:
            # Protostellar core - dense central region
            for r in np.linspace(3, 0.1, 30):
                alpha = 0.1 + 0.5 * (1 - r/3)
                color = density_cmap(1 - r/3)
                ax.add_patch(Circle((0, 0), r, facecolor=color, alpha=0.3))

            # Infall arrows
            for angle in np.linspace(0, 2*np.pi, 8, endpoint=False):
                x1, y1 = 3.5 * np.cos(angle), 3.5 * np.sin(angle)
                x2, y2 = 2 * np.cos(angle), 2 * np.sin(angle)
                ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                           arrowprops=dict(arrowstyle='->', color='cyan', lw=1.5))

            ax.text(0, -4.5, 'Gravitational collapse\n(~10^4 particles/cm^3)',
                   fontsize=8, ha='center')

        elif idx == 3:
            # Accretion disk
            # Disk (ellipse viewed at angle)
            for r in np.linspace(3, 0.5, 15):
                ellipse = plt.matplotlib.patches.Ellipse(
                    (0, 0), 2*r, 0.6*r, facecolor=density_cmap(1 - r/3),
                    alpha=0.3)
                ax.add_patch(ellipse)

            # Central protostar
            ax.add_patch(Circle((0, 0), 0.4, facecolor='yellow',
                               edgecolor='orange', linewidth=2))

            # Bipolar outflows
            ax.annotate('', xy=(0, 4), xytext=(0, 0.5),
                       arrowprops=dict(arrowstyle='->', color='red', lw=2))
            ax.annotate('', xy=(0, -4), xytext=(0, -0.5),
                       arrowprops=dict(arrowstyle='->', color='red', lw=2))
            ax.text(0.3, 3, 'Jets', fontsize=9, color='red')

            ax.text(0, -4.5, 'Conservation of\nangular momentum',
                   fontsize=8, ha='center')

        elif idx == 4:
            # T-Tauri phase - irregular variability
            # Star with variability
            ax.add_patch(Circle((0, 0), 1, facecolor='orange',
                               edgecolor='red', linewidth=3))

            # Irregular brightness symbols
            for angle in np.linspace(0, 2*np.pi, 12, endpoint=False):
                r = 1.5 + 0.3 * np.sin(5 * angle)
                x, y = r * np.cos(angle), r * np.sin(angle)
                ax.plot([0, x], [0, y], 'yellow', linewidth=1, alpha=0.5)

            # Remaining disk
            for r in np.linspace(3, 2, 5):
                ellipse = plt.matplotlib.patches.Ellipse(
                    (0, 0), 2*r, 0.4*r, facecolor='brown', alpha=0.2)
                ax.add_patch(ellipse)

            ax.text(0, -4.5, 'Variable brightness\nactive accretion',
                   fontsize=8, ha='center')

        else:  # idx == 5
            # Main sequence star
            ax.add_patch(Circle((0, 0), 2, facecolor='yellow',
                               edgecolor='orange', linewidth=3))

            # Core
            ax.add_patch(Circle((0, 0), 0.6, facecolor='white', alpha=0.8))
            ax.text(0, 0, 'H -> He', fontsize=9, ha='center', va='center')

            # Radiation
            for angle in np.linspace(0, 2*np.pi, 16, endpoint=False):
                x1, y1 = 2.2 * np.cos(angle), 2.2 * np.sin(angle)
                x2, y2 = 3.5 * np.cos(angle), 3.5 * np.sin(angle)
                ax.plot([x1, x2], [y1, y2], 'yellow', linewidth=2, alpha=0.7)

            ax.text(0, -4.5, 'Hydrostatic equilibrium\nfusion ignited',
                   fontsize=8, ha='center')

    fig.suptitle('Star Formation: From Cloud to Fusion',
                fontsize=14, fontweight='bold', y=0.98)

    fig.text(0.5, 0.02, 'TRD: Gravitational flux accumulation increases density '
            'until fusion threshold (KB) is reached.',
            fontsize=10, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='gray'))

    plt.tight_layout(rect=[0, 0.05, 1, 0.93])
    return fig


if __name__ == '__main__':
    fig = generate_star_formation()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_25_star_formation.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
