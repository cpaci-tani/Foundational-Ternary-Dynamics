"""
Figure 2.20: Dark Matter and Dark Energy
========================================
Visualizes how TRD might account for dark matter (unmanifested
flux concentrations) and dark energy (background flux pressure).
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge, Rectangle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_dark_matter_energy():
    """
    Generate the dark matter and dark energy visualization.

    Shows:
    1. Left: Galaxy rotation curve and dark matter halo
    2. Right: Cosmic pie chart and dark energy effect
    """
    fig = plt.figure(figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    gs = fig.add_gridspec(2, 2, height_ratios=[1.2, 1], hspace=0.3, wspace=0.25)

    ax_galaxy = fig.add_subplot(gs[0, 0])
    ax_curve = fig.add_subplot(gs[0, 1])
    ax_pie = fig.add_subplot(gs[1, 0])
    ax_expansion = fig.add_subplot(gs[1, 1])

    # =========================================================================
    # Top Left: Galaxy with Dark Matter Halo
    # =========================================================================
    ax_galaxy.set_xlim(-8, 8)
    ax_galaxy.set_ylim(-8, 8)
    ax_galaxy.set_aspect('equal')
    ax_galaxy.axis('off')

    # Dark matter halo (invisible, shown as outline)
    for r in [7, 6, 5, 4]:
        halo = Circle((0, 0), r, facecolor='none',
                      edgecolor=COLORS['void'], linewidth=1.5,
                      linestyle='--', alpha=0.5 - r/20)
        ax_galaxy.add_patch(halo)

    # Dark matter halo fill (barely visible)
    halo_fill = Circle((0, 0), 7, facecolor=COLORS['void'],
                        edgecolor='none', alpha=0.1)
    ax_galaxy.add_patch(halo_fill)

    # Visible galaxy (spiral arms suggestion)
    galaxy_disk = Circle((0, 0), 3, facecolor=COLORS['highlight'],
                         edgecolor='black', linewidth=2, alpha=0.6)
    ax_galaxy.add_patch(galaxy_disk)

    # Spiral arms
    theta = np.linspace(0, 4*np.pi, 200)
    for offset in [0, np.pi/2, np.pi, 3*np.pi/2]:
        r_arm = 0.5 + 0.2 * theta
        x_arm = r_arm * np.cos(theta + offset)
        y_arm = r_arm * np.sin(theta + offset)
        mask = r_arm < 3
        ax_galaxy.plot(x_arm[mask], y_arm[mask], color=COLORS['matter'],
                       linewidth=2, alpha=0.7)

    # Central bulge
    bulge = Circle((0, 0), 0.5, facecolor=COLORS['matter'],
                   edgecolor='black', linewidth=2)
    ax_galaxy.add_patch(bulge)

    ax_galaxy.set_title('Galaxy with Dark Matter Halo\n(Dashed = invisible halo)',
                        fontsize=12, fontweight='bold')

    # Labels
    ax_galaxy.text(0, -4.5, 'Visible\nGalaxy', fontsize=9, ha='center',
                   color=COLORS['matter'])
    ax_galaxy.text(5.5, 5, 'Dark\nMatter\nHalo', fontsize=9, ha='center',
                   color=COLORS['void'])

    # =========================================================================
    # Top Right: Rotation Curve
    # =========================================================================
    r = np.linspace(0.1, 10, 100)

    # Expected Keplerian rotation (no dark matter)
    v_keplerian = 200 / np.sqrt(r)

    # Observed flat rotation (with dark matter)
    v_observed = 200 * np.tanh(r / 2) + 50

    # Dark matter contribution
    v_dm = np.sqrt(v_observed**2 - (200 / np.sqrt(r + 0.5))**2)
    v_dm = np.maximum(v_dm, 0)  # Can't be negative

    ax_curve.plot(r, v_keplerian, 'b--', linewidth=2,
                  label='Expected (no DM)')
    ax_curve.plot(r, v_observed, 'r-', linewidth=2.5,
                  label='Observed')
    ax_curve.fill_between(r, v_keplerian, v_observed, alpha=0.2,
                          color=COLORS['void'], label='Dark matter contribution')

    ax_curve.set_xlabel('Radius (kpc)', fontsize=11)
    ax_curve.set_ylabel('Rotation velocity (km/s)', fontsize=11)
    ax_curve.set_title('Galaxy Rotation Curve',
                       fontsize=12, fontweight='bold')
    ax_curve.legend(loc='lower right', fontsize=9)
    ax_curve.grid(True, alpha=0.3)
    ax_curve.set_xlim(0, 10)
    ax_curve.set_ylim(0, 300)

    # TRD note
    ax_curve.text(5, 280, 'TRD: "Dark matter" = unmanifested\nflux concentrations (state 0, high density)',
                  fontsize=9, ha='center',
                  bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Bottom Left: Cosmic Composition Pie Chart
    # =========================================================================
    sizes = [68.3, 26.8, 4.9]
    labels = ['Dark Energy\n(68.3%)', 'Dark Matter\n(26.8%)', 'Ordinary Matter\n(4.9%)']
    colors = ['#9b59b6', COLORS['void'], COLORS['matter']]
    explode = (0.05, 0.02, 0)

    wedges, texts, autotexts = ax_pie.pie(sizes, explode=explode, labels=labels,
                                          colors=colors, autopct='',
                                          startangle=90, textprops={'fontsize': 10})

    ax_pie.set_title('Cosmic Composition', fontsize=12, fontweight='bold')

    # TRD interpretation
    ax_pie.text(0, -1.5, 'TRD interpretation:\n'
                        'Dark Energy = Background void flux pressure\n'
                        'Dark Matter = Unmanifested flux (state 0)',
                fontsize=9, ha='center',
                bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Bottom Right: Dark Energy Effect on Expansion
    # =========================================================================
    t = np.linspace(0, 13.8, 100)  # Time in Gyr

    # Scale factor evolution
    # Matter-only universe: a ~ t^(2/3)
    a_matter = (t / 13.8) ** (2/3)

    # With dark energy (accelerating): a ~ exp(H*t) at late times
    a_de = a_matter * (1 + 0.3 * np.exp(0.2 * (t - 8)))
    a_de = a_de / a_de[-1]  # Normalize to 1 today

    ax_expansion.plot(t, a_matter, 'b--', linewidth=2, label='Matter only (decelerating)')
    ax_expansion.plot(t, a_de, 'r-', linewidth=2.5, label='With dark energy (accelerating)')

    # Mark transition
    ax_expansion.axvline(x=7.5, color='gray', linestyle=':', linewidth=1.5)
    ax_expansion.text(7.7, 0.3, 'Acceleration\nbegins', fontsize=9, color='gray')

    ax_expansion.set_xlabel('Time since Big Bang (Gyr)', fontsize=11)
    ax_expansion.set_ylabel('Scale factor a(t)', fontsize=11)
    ax_expansion.set_title('Cosmic Expansion History',
                          fontsize=12, fontweight='bold')
    ax_expansion.legend(loc='upper left', fontsize=9)
    ax_expansion.grid(True, alpha=0.3)
    ax_expansion.set_xlim(0, 14)
    ax_expansion.set_ylim(0, 1.1)

    # Today marker
    ax_expansion.axvline(x=13.8, color='green', linestyle='-', linewidth=2)
    ax_expansion.text(13.5, 1.05, 'Today', fontsize=9, color='green', ha='right')

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Dark Matter and Dark Energy: The Invisible 95% of the Universe',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "In TRD, 'dark matter' is unmanifested flux (state 0 with high density - gravitationally active but invisible).\n"
        "'Dark energy' arises from background void flux pressure that drives accelerating expansion."
    )
    fig.text(0.5, 0.01, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_dark_matter_energy()
    output_path = Path(__file__).parent.parent / 'ch05' / 'fig_2_20_dark_matter_energy.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
