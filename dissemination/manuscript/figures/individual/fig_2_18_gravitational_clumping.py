"""
Figure 2.18: Gravitational Clumping
===================================
Visualizes how gravitational attraction leads to structure formation:
density perturbations grow into stars, galaxies, clusters.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_gravitational_clumping():
    """
    Generate the gravitational clumping visualization.

    Shows:
    1. Top: Time evolution of density field
    2. Bottom: Power spectrum of clustering
    """
    fig = plt.figure(figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    gs = fig.add_gridspec(2, 4, height_ratios=[1.2, 1], hspace=0.3, wspace=0.15)

    ax_t0 = fig.add_subplot(gs[0, 0])
    ax_t1 = fig.add_subplot(gs[0, 1])
    ax_t2 = fig.add_subplot(gs[0, 2])
    ax_t3 = fig.add_subplot(gs[0, 3])
    ax_power = fig.add_subplot(gs[1, :])

    np.random.seed(42)

    # =========================================================================
    # Top Row: Density Field Evolution
    # =========================================================================
    times = [('t = 0', 0), ('t = 1 Gyr', 1), ('t = 5 Gyr', 5), ('t = 13 Gyr', 13)]
    axes = [ax_t0, ax_t1, ax_t2, ax_t3]

    # Grid for density field
    n_grid = 50
    x = np.linspace(0, 10, n_grid)
    y = np.linspace(0, 10, n_grid)
    X, Y = np.meshgrid(x, y)

    # Initial random density fluctuations
    np.random.seed(123)
    initial_density = np.random.normal(1, 0.1, (n_grid, n_grid))

    # Simple smoothing (without scipy) using numpy convolution
    def simple_smooth(arr, size=5):
        """Simple box filter smoothing."""
        kernel = np.ones((size, size)) / (size * size)
        # Pad array
        pad = size // 2
        padded = np.pad(arr, pad, mode='reflect')
        result = np.zeros_like(arr)
        for i in range(arr.shape[0]):
            for j in range(arr.shape[1]):
                result[i, j] = np.sum(padded[i:i+size, j:j+size] * kernel)
        return result

    initial_density = simple_smooth(initial_density, size=5)

    # Add some initial seeds
    for _ in range(5):
        cx = np.random.randint(10, 40)
        cy = np.random.randint(10, 40)
        initial_density[max(0,cy-2):min(n_grid,cy+2), max(0,cx-2):min(n_grid,cx+2)] += 0.3

    for ax, (label, t) in zip(axes, times):
        ax.set_xlim(0, 10)
        ax.set_ylim(0, 10)
        ax.set_aspect('equal')

        # Evolve density field (simplified model)
        if t == 0:
            density = initial_density.copy()
        else:
            # Gravitational growth amplifies overdensities
            growth_factor = 1 + 0.3 * t
            density = 1 + (initial_density - 1) * growth_factor

            # Collapse to points (above threshold become compact)
            collapse_threshold = 1.3 + 0.05 * t
            density = np.where(density > collapse_threshold,
                              density * (1 + 0.2 * t), density)

        # Normalize for display
        vmin = 0.5
        vmax = 2 + 0.3 * t

        im = ax.imshow(density, extent=[0, 10, 0, 10], origin='lower',
                       cmap='inferno', vmin=vmin, vmax=vmax)

        # Mark collapsed objects (peaks)
        if t > 0:
            threshold = 1.5 + 0.1 * t
            peaks_y, peaks_x = np.where(density > threshold)
            # Sample some peaks to display
            if len(peaks_x) > 0:
                n_show = min(10, len(peaks_x))
                indices = np.random.choice(len(peaks_x), n_show, replace=False)
                for idx in indices:
                    px = peaks_x[idx] * 10 / n_grid
                    py = peaks_y[idx] * 10 / n_grid
                    ax.plot(px, py, 'w*', markersize=3 + t/3)

        ax.set_title(label, fontsize=11, fontweight='bold')
        ax.axis('off')

    # Add colorbar
    cbar_ax = fig.add_axes([0.92, 0.55, 0.015, 0.3])
    cbar = plt.colorbar(im, cax=cbar_ax)
    cbar.set_label('Density', fontsize=10)

    # =========================================================================
    # Bottom: Power Spectrum Evolution
    # =========================================================================
    k = np.logspace(-2, 1, 100)  # Wavenumber (inverse scale)

    # Power spectrum at different times
    # P(k) ~ k^n * growth^2, with suppression at small scales
    def power_spectrum(k, t, n_s=0.96):
        # Transfer function (suppression at small scales)
        k_eq = 0.1
        T = 1 / (1 + (k / k_eq)**2)
        # Growth factor
        growth = 1 + 0.3 * t
        # Primordial spectrum
        P = k**n_s * T**2 * growth**2
        return P / P.max()

    for t, color, label in [(0, 'blue', 't = 0'), (1, 'green', 't = 1 Gyr'),
                            (5, 'orange', 't = 5 Gyr'), (13, 'red', 't = 13 Gyr')]:
        P = power_spectrum(k, t)
        ax_power.loglog(k, P, color=color, linewidth=2, label=label)

    ax_power.set_xlabel('Wavenumber k (inverse scale)', fontsize=12)
    ax_power.set_ylabel('Power P(k) (normalized)', fontsize=12)
    ax_power.set_title('Matter Power Spectrum: Growth of Structure',
                       fontsize=13, fontweight='bold')
    ax_power.legend(loc='lower left', fontsize=10)
    ax_power.grid(True, alpha=0.3, which='both')

    # Annotate scales
    ax_power.axvline(x=0.01, color='gray', linestyle=':', alpha=0.5)
    ax_power.text(0.01, 0.001, 'Clusters', fontsize=9, rotation=90, va='bottom')

    ax_power.axvline(x=0.1, color='gray', linestyle=':', alpha=0.5)
    ax_power.text(0.1, 0.001, 'Galaxies', fontsize=9, rotation=90, va='bottom')

    ax_power.axvline(x=1, color='gray', linestyle=':', alpha=0.5)
    ax_power.text(1, 0.001, 'Stars', fontsize=9, rotation=90, va='bottom')

    # TRD note
    ax_power.text(0.05, 0.5, 'TRD: Gravitational flux gradients\namplify density perturbations',
                  fontsize=10, transform=ax_power.transAxes,
                  bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Gravitational Clumping: From Primordial Fluctuations to Cosmic Structure',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "In TRD, gravity-like flux gradients cause density perturbations to grow over time.\n"
        "Small initial variations become stars, galaxies, and clusters through gravitational collapse."
    )
    fig.text(0.5, 0.01, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 0.91, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_gravitational_clumping()
    output_path = Path(__file__).parent.parent / 'ch05' / 'fig_2_18_gravitational_clumping.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
