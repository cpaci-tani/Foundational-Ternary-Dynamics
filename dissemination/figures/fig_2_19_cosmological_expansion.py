"""
Figure 2.19: Cosmological Expansion
===================================
Visualizes the expansion of the universe as lattice scaling in TRD,
showing how "distance" grows while local physics is preserved.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_cosmological_expansion():
    """
    Generate the cosmological expansion visualization.

    Shows:
    1. Top: Lattice at three epochs (expanding)
    2. Bottom: Hubble diagram (velocity vs distance)
    """
    fig = plt.figure(figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    gs = fig.add_gridspec(2, 3, height_ratios=[1.2, 1], hspace=0.3, wspace=0.2)

    ax_early = fig.add_subplot(gs[0, 0])
    ax_middle = fig.add_subplot(gs[0, 1])
    ax_late = fig.add_subplot(gs[0, 2])
    ax_hubble = fig.add_subplot(gs[1, :])

    # =========================================================================
    # Top Row: Expanding Lattice
    # =========================================================================
    epochs = [
        (ax_early, 'Early Universe\n(t = 380,000 yr)', 1.0, 0.3),
        (ax_middle, 'Middle Age\n(t = 5 Gyr)', 0.5, 1.5),
        (ax_late, 'Today\n(t = 13.8 Gyr)', 0.3, 3.0),
    ]

    for ax, title, density, spacing in epochs:
        ax.set_xlim(-0.5, 5.5)
        ax.set_ylim(-0.5, 5.5)
        ax.set_aspect('equal')
        ax.axis('off')

        # Draw lattice points with spacing
        n_points = int(5 / spacing) + 1
        for i in range(n_points):
            for j in range(n_points):
                x = i * spacing
                y = j * spacing
                if x <= 5 and y <= 5:
                    # Galaxy/cluster as a point
                    size = 0.1 / spacing
                    galaxy = Circle((x, y), size, facecolor=COLORS['highlight'],
                                   edgecolor='black', linewidth=0.5, alpha=0.8)
                    ax.add_patch(galaxy)

                    # Draw recession arrows for later epochs
                    if spacing > 1:
                        # Arrow pointing outward from center
                        cx, cy = 2.5, 2.5
                        dx = (x - cx) * 0.1
                        dy = (y - cy) * 0.1
                        if dx != 0 or dy != 0:
                            ax.arrow(x, y, dx, dy, head_width=0.08,
                                    head_length=0.04, fc='red', ec='red',
                                    alpha=0.5)

        # Scale indicator
        ax.plot([0.2, 0.2 + spacing], [5.3, 5.3], 'k-', linewidth=2)
        ax.text(0.2 + spacing/2, 5.5, f'a = {spacing:.1f}', fontsize=9, ha='center')

        ax.set_title(title, fontsize=11, fontweight='bold')

        # Background color based on temperature
        bg_color = plt.cm.hot(1 - density)[:3]
        ax.set_facecolor((*bg_color, 0.2))

    # =========================================================================
    # Bottom: Hubble Diagram
    # =========================================================================
    # Distance (Mpc)
    d = np.linspace(0, 500, 100)

    # Hubble's law: v = H0 * d
    H0 = 70  # km/s/Mpc
    v = H0 * d

    ax_hubble.plot(d, v, 'b-', linewidth=2.5, label='Hubble Law: v = H$_0$d')

    # Add some scatter points (galaxies)
    np.random.seed(42)
    n_gal = 30
    d_gal = np.random.uniform(10, 450, n_gal)
    v_gal = H0 * d_gal + np.random.normal(0, 500, n_gal)
    ax_hubble.scatter(d_gal, v_gal, c=COLORS['highlight'], s=50, edgecolors='black',
                      linewidth=0.5, label='Observed galaxies', zorder=5)

    ax_hubble.set_xlabel('Distance (Mpc)', fontsize=12)
    ax_hubble.set_ylabel('Recession Velocity (km/s)', fontsize=12)
    ax_hubble.set_title('Hubble Diagram: Recession Velocity vs Distance',
                        fontsize=13, fontweight='bold')
    ax_hubble.legend(loc='upper left', fontsize=10)
    ax_hubble.grid(True, alpha=0.3)

    # Mark H0
    ax_hubble.text(400, 15000, f'H$_0$ = {H0} km/s/Mpc', fontsize=12,
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # TRD interpretation
    ax_hubble.text(0.7, 0.15, 'TRD: Lattice spacing a(t) increases with time.\n'
                            'Galaxies recede as space between them grows.',
                   fontsize=10, transform=ax_hubble.transAxes,
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='#fff8e8'))

    # Redshift scale on right axis
    ax_hubble2 = ax_hubble.twinx()
    z_max = v.max() / 3e5  # Approximate z for non-relativistic
    ax_hubble2.set_ylim(0, z_max)
    ax_hubble2.set_ylabel('Redshift z (approx)', fontsize=11, color='gray')
    ax_hubble2.tick_params(axis='y', labelcolor='gray')

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Cosmological Expansion: Universe Grows as Lattice Spacing Increases',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "In TRD, cosmic expansion is modeled as increasing lattice spacing a(t).\n"
        "Galaxies are not moving through space - space itself is expanding between them."
    )
    fig.text(0.5, 0.01, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_cosmological_expansion()
    output_path = Path(__file__).parent.parent / 'ch05' / 'fig_2_19_cosmological_expansion.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
