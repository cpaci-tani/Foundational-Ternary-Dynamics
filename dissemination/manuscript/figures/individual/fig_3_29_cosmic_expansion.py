"""
Figure 3.29: Cosmic Expansion
=============================
Shows Hubble expansion, redshift, and cosmological
implications from TRD lattice perspective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_cosmic_expansion():
    """Generate the cosmic expansion visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Expanding universe (balloon analogy)
    ax = axes[0, 0]
    ax.set_xlim(-5, 5)
    ax.set_ylim(-5, 5)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Expansion: Raisin Bread Analogy', fontsize=11, fontweight='bold')

    # Earlier time (smaller)
    center_small = (-2.5, 0)
    r_small = 1.5
    ax.add_patch(Circle(center_small, r_small, facecolor='wheat', alpha=0.5,
                       edgecolor='brown', linewidth=2))

    # Galaxies in small universe
    np.random.seed(42)
    galaxy_angles = np.random.uniform(0, 2*np.pi, 6)
    galaxy_radii = np.random.uniform(0.3, 0.9, 6)

    for angle, r in zip(galaxy_angles, galaxy_radii):
        x = center_small[0] + r * r_small * np.cos(angle)
        y = center_small[1] + r * r_small * np.sin(angle)
        ax.plot(x, y, 'r*', markersize=8)

    ax.text(-2.5, -2.3, 'Earlier\n(t = t1)', fontsize=9, ha='center')

    # Later time (larger)
    center_large = (2.5, 0)
    r_large = 2.5
    ax.add_patch(Circle(center_large, r_large, facecolor='wheat', alpha=0.5,
                       edgecolor='brown', linewidth=2))

    # Same galaxies, more spread out
    for angle, r in zip(galaxy_angles, galaxy_radii):
        x = center_large[0] + r * r_large * np.cos(angle)
        y = center_large[1] + r * r_large * np.sin(angle)
        ax.plot(x, y, 'r*', markersize=8)

    ax.text(2.5, -3.3, 'Later\n(t = t2 > t1)', fontsize=9, ha='center')

    # Arrow showing time
    ax.annotate('', xy=(1, 0), xytext=(-0.5, 0),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))
    ax.text(0.25, 0.5, 'Time', fontsize=10, ha='center')

    ax.text(0, -4.5, 'Space itself expands, carrying galaxies apart',
           fontsize=9, ha='center')

    # Panel 2: Hubble law
    ax = axes[0, 1]

    # Mock data
    distances = np.array([10, 20, 35, 50, 75, 100, 150, 200])
    velocities = 70 * distances  # H0 ~ 70 km/s/Mpc

    # Add scatter
    np.random.seed(42)
    velocities_obs = velocities + np.random.normal(0, 500, len(distances))

    ax.scatter(distances, velocities_obs, c='blue', s=50, alpha=0.7)

    # Hubble fit
    d_fit = np.linspace(0, 220, 100)
    v_fit = 70 * d_fit
    ax.plot(d_fit, v_fit, 'r-', linewidth=2, label='v = H0 x d')

    ax.set_xlabel('Distance (Mpc)', fontsize=10)
    ax.set_ylabel('Recession Velocity (km/s)', fontsize=10)
    ax.set_title("Hubble's Law", fontsize=11, fontweight='bold')
    ax.legend(loc='upper left', fontsize=9)

    ax.text(150, 5000, 'H0 ~ 70 km/s/Mpc', fontsize=10)
    ax.text(150, 3500, '(Hubble constant)', fontsize=9)

    ax.set_xlim(0, 220)
    ax.set_ylim(0, 16000)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    # Panel 3: Redshift visualization
    ax = axes[1, 0]
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 8)
    ax.axis('off')
    ax.set_title('Cosmological Redshift', fontsize=11, fontweight='bold')

    # Emitted wave (at source)
    t_emit = np.linspace(0, 3, 100)
    y_emit = 6 + 0.5 * np.sin(10 * t_emit)
    ax.plot(t_emit, y_emit, 'b-', linewidth=2)
    ax.text(1.5, 7, 'Emitted wavelength', fontsize=9, ha='center', color='blue')

    # Received wave (stretched)
    t_recv = np.linspace(0, 3, 100)
    y_recv = 2 + 0.5 * np.sin(6 * t_recv)  # Lower frequency
    ax.plot(t_recv + 6, y_recv, 'r-', linewidth=2)
    ax.text(7.5, 3, 'Observed wavelength\n(redshifted)', fontsize=9, ha='center', color='red')

    # Arrow showing stretching
    ax.annotate('', xy=(6, 4), xytext=(3, 4),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))
    ax.text(4.5, 4.5, 'Space expands', fontsize=9, ha='center')

    # Formula
    ax.text(6, 0.5, 'z = (lambda_obs - lambda_emit) / lambda_emit = (a_now - a_emit) / a_emit',
           fontsize=9, ha='center')

    # Panel 4: TRD cosmology
    ax = axes[1, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('TRD Cosmological View', fontsize=11, fontweight='bold')

    # Lattice expansion visualization
    # Small lattice
    for i in range(3):
        for j in range(3):
            ax.add_patch(Circle((1 + i*0.8, 7 + j*0.8), 0.15,
                               facecolor=COLORS['matter'], edgecolor='black'))

    ax.add_patch(Rectangle((0.5, 6.5), 2.4, 2.4, fill=False,
                           edgecolor='gray', linewidth=1, linestyle='--'))
    ax.text(1.7, 6, 't = early', fontsize=9, ha='center')

    # Large lattice (expanded)
    for i in range(3):
        for j in range(3):
            ax.add_patch(Circle((6 + i*1.2, 7 + j*1.2), 0.15,
                               facecolor=COLORS['matter'], edgecolor='black'))

    ax.add_patch(Rectangle((5.3, 6.3), 3.6, 3.6, fill=False,
                           edgecolor='gray', linewidth=1, linestyle='--'))
    ax.text(7.1, 5.8, 't = now', fontsize=9, ha='center')

    # Arrow
    ax.annotate('', xy=(5, 7.8), xytext=(3.5, 7.8),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))

    trd_text = (
        "TRD Expansion Model:\n\n"
        "Two possibilities:\n\n"
        "1. Lattice growth:\n"
        "   - New voxels created at boundary\n"
        "   - Total lattice size increases\n"
        "   - Content unchanged, space grows\n\n"
        "2. Scale factor change:\n"
        "   - Voxel size (in physical units)\n"
        "     increases with cosmic time\n"
        "   - Light wavelength stretches\n"
        "   - Local physics unchanged\n\n"
        "Cosmological constant Lambda:\n"
        "   Lambda ~ alpha^57 ~ 10^(-122)\n"
        "   (derived from TRD framework)"
    )
    ax.text(5, 4.5, trd_text, fontsize=8, ha='center', va='top',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    fig.suptitle('Cosmic Expansion: The Stretching of Space',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_cosmic_expansion()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_29_cosmic_expansion.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
