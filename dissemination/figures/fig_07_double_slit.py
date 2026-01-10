"""
Figure 1.7: Double-Slit Pattern
===============================
Demonstrates the double-slit interference pattern in TRD.

Shows:
- Schematic of two slits with wave fronts
- Resulting interference pattern on screen
- Intensity profile showing bright/dark fringes
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Circle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_double_slit():
    """
    Generate the double-slit experiment visualization.

    Layout:
    - Left: Experimental geometry with wave fronts
    - Right: Interference pattern (2D intensity)
    - Bottom: Intensity profile along detection screen
    """
    fig = plt.figure(figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Create grid spec for layout
    gs = fig.add_gridspec(2, 2, height_ratios=[3, 1], hspace=0.25, wspace=0.2)

    ax_geom = fig.add_subplot(gs[0, 0])   # Geometry
    ax_pattern = fig.add_subplot(gs[0, 1])  # 2D pattern
    ax_profile = fig.add_subplot(gs[1, :])  # Intensity profile

    # =========================================================================
    # Parameters
    # =========================================================================
    wavelength = 1.0
    slit_separation = 5.0  # Distance between slits
    screen_distance = 20.0  # Distance to screen
    slit_width = 0.5

    # =========================================================================
    # Panel 1: Geometry (left)
    # =========================================================================
    ax_geom.set_xlim(-5, 25)
    ax_geom.set_ylim(-15, 15)
    ax_geom.set_aspect('equal')
    ax_geom.axis('off')
    ax_geom.set_title('Experimental Geometry', fontsize=12, fontweight='bold')

    # Draw barrier with slits
    barrier_x = 0
    ax_geom.add_patch(Rectangle((barrier_x - 0.3, -15), 0.6, 15 - slit_separation/2 - slit_width/2,
                                 facecolor='#333333', edgecolor='none'))
    ax_geom.add_patch(Rectangle((barrier_x - 0.3, slit_separation/2 + slit_width/2), 0.6, 15 - slit_separation/2 - slit_width/2,
                                 facecolor='#333333', edgecolor='none'))
    ax_geom.add_patch(Rectangle((barrier_x - 0.3, -slit_separation/2 + slit_width/2), 0.6, slit_separation - slit_width,
                                 facecolor='#333333', edgecolor='none'))

    # Draw slits (gaps)
    ax_geom.annotate('', xy=(0.5, slit_separation/2), xytext=(-0.5, slit_separation/2),
                     arrowprops=dict(arrowstyle='<->', color=COLORS['highlight'], lw=2))
    ax_geom.text(0, slit_separation/2 + 1, 'Slit 1', fontsize=10, ha='center')
    ax_geom.annotate('', xy=(0.5, -slit_separation/2), xytext=(-0.5, -slit_separation/2),
                     arrowprops=dict(arrowstyle='<->', color=COLORS['highlight'], lw=2))
    ax_geom.text(0, -slit_separation/2 - 1.5, 'Slit 2', fontsize=10, ha='center')

    # Draw circular wave fronts from each slit
    for r in np.arange(2, 22, 2):
        # From slit 1
        theta = np.linspace(-np.pi/2, np.pi/2, 100)
        x1 = barrier_x + r * np.cos(theta)
        y1 = slit_separation/2 + r * np.sin(theta)
        ax_geom.plot(x1, y1, color=COLORS['matter'], alpha=0.3, linewidth=1)

        # From slit 2
        x2 = barrier_x + r * np.cos(theta)
        y2 = -slit_separation/2 + r * np.sin(theta)
        ax_geom.plot(x2, y2, color=COLORS['antimatter'], alpha=0.3, linewidth=1)

    # Draw detection screen
    screen_x = screen_distance
    ax_geom.add_patch(Rectangle((screen_x - 0.2, -12), 0.4, 24,
                                 facecolor='#666666', edgecolor='black'))
    ax_geom.text(screen_x + 1, 0, 'Screen', fontsize=10, ha='left', va='center', rotation=90)

    # Draw incoming flux (from left)
    for y_pos in [-8, -4, 0, 4, 8]:
        ax_geom.annotate('', xy=(barrier_x - 0.5, y_pos), xytext=(-4, y_pos),
                        arrowprops=dict(arrowstyle='->', color=COLORS['accent1'], lw=1.5))

    ax_geom.text(-4.5, 10, 'Incoming\nFlux', fontsize=10, ha='center')

    # =========================================================================
    # Panel 2: 2D Interference Pattern (right)
    # =========================================================================
    # Calculate interference pattern
    screen_y = np.linspace(-12, 12, 200)
    screen_x_grid = np.linspace(5, 20, 100)
    X, Y = np.meshgrid(screen_x_grid, screen_y)

    # Distance from each slit to each point
    r1 = np.sqrt(X**2 + (Y - slit_separation/2)**2)
    r2 = np.sqrt(X**2 + (Y + slit_separation/2)**2)

    # Phase difference and interference
    k = 2 * np.pi / wavelength
    phase1 = k * r1
    phase2 = k * r2

    # Intensity from superposition (simplified)
    intensity = np.cos((phase1 - phase2) / 2)**2

    # Apply envelope for visibility
    envelope = np.exp(-Y**2 / 100)
    intensity *= envelope

    im = ax_pattern.imshow(intensity, extent=[5, 20, -12, 12],
                           origin='lower', aspect='auto',
                           cmap='hot', vmin=0, vmax=1)
    ax_pattern.set_xlabel('Distance from slits', fontsize=10)
    ax_pattern.set_ylabel('Position on screen', fontsize=10)
    ax_pattern.set_title('Interference Pattern', fontsize=12, fontweight='bold')

    # Add colorbar
    cbar = plt.colorbar(im, ax=ax_pattern, shrink=0.8)
    cbar.set_label('Intensity $I = |J_1 + J_2|^2$', fontsize=10)

    # =========================================================================
    # Panel 3: Intensity Profile (bottom)
    # =========================================================================
    # Calculate intensity at screen
    y_screen = np.linspace(-12, 12, 500)
    r1_screen = np.sqrt(screen_distance**2 + (y_screen - slit_separation/2)**2)
    r2_screen = np.sqrt(screen_distance**2 + (y_screen + slit_separation/2)**2)

    phase_diff = k * (r1_screen - r2_screen)
    I_screen = np.cos(phase_diff / 2)**2

    # Apply single-slit envelope
    beta = k * slit_width * y_screen / (2 * screen_distance)
    envelope = np.sinc(beta / np.pi)**2
    I_screen *= envelope

    ax_profile.fill_between(y_screen, 0, I_screen, alpha=0.3, color=COLORS['highlight'])
    ax_profile.plot(y_screen, I_screen, color=COLORS['text'], linewidth=2)

    # Mark bright and dark fringes
    ax_profile.axhline(y=1, color=COLORS['accent1'], linestyle='--', alpha=0.5, linewidth=1)
    ax_profile.axhline(y=0, color=COLORS['void'], linestyle='-', alpha=0.3, linewidth=1)

    ax_profile.set_xlim(-12, 12)
    ax_profile.set_ylim(0, 1.2)

    apply_trd_style(ax_profile,
                    title='Intensity Profile at Screen',
                    xlabel='Position y', ylabel='Intensity $I$')

    # Add annotations
    ax_profile.annotate('Central\nMaximum', xy=(0, 1), xytext=(0, 1.1),
                        fontsize=9, ha='center', color=COLORS['accent1'])

    # Formula box
    formula_text = (
        r'$I(y) = I_0 \cos^2\left(\frac{\pi d \sin\theta}{\lambda}\right)$' + '\n'
        r'$d$ = slit separation, $\lambda$ = wavelength'
    )
    ax_profile.text(0.98, 0.95, formula_text, transform=ax_profile.transAxes,
                    fontsize=10, verticalalignment='top', horizontalalignment='right',
                    bbox=dict(boxstyle='round,pad=0.3', facecolor='white', alpha=0.9))

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_double_slit()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_1_7_double_slit.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
