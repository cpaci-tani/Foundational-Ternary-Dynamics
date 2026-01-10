"""
Figure 1.2: Master Quadratic Roots
==================================
Visualizes the master quadratic equation whose roots give:
- x₊ = 137.036 (inverse fine structure constant, 1/α)
- x₋ = 3.024 (approximately N_c, the number of color charges)

The equation: x² - 16G*²x + 16G*³ = 0
where G* ≈ 2.9587 is the lemniscatic constant.
"""

import numpy as np
import matplotlib.pyplot as plt
import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, FONTS, apply_trd_style, create_figure
from utils.physics_constants import G_STAR, ALPHA_INV, QUADRATIC_X_PLUS, QUADRATIC_X_MINUS


def generate_master_quadratic():
    """
    Generate the master quadratic visualization.

    Shows:
    - Two panels: full view and zoomed view near x₋
    - Parabola with roots clearly marked
    - Annotations explaining physical meaning of roots
    """
    # Quadratic coefficients: x² - 16G*²x + 16G*³ = 0
    a = 1
    b = -16 * G_STAR ** 2
    c = 16 * G_STAR ** 3

    # Create two-panel figure
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Full view showing both roots
    x_full = np.linspace(-20, 160, 1000)
    y_full = a * x_full**2 + b * x_full + c

    ax1.plot(x_full, y_full, color=COLORS['text'], linewidth=2.5)
    ax1.axhline(y=0, color=COLORS['grid'], linestyle='--', alpha=0.7, linewidth=1)
    ax1.axvline(x=0, color=COLORS['grid'], linestyle='--', alpha=0.7, linewidth=1)

    # Mark the roots
    ax1.scatter([QUADRATIC_X_MINUS], [0], color=COLORS['antimatter'],
                s=150, zorder=5, edgecolors='white', linewidths=2)
    ax1.scatter([QUADRATIC_X_PLUS], [0], color=COLORS['matter'],
                s=150, zorder=5, edgecolors='white', linewidths=2)

    # Annotations for roots
    ax1.annotate(f'$x_- = {QUADRATIC_X_MINUS:.3f}$\n$(\\approx N_c)$',
                 xy=(QUADRATIC_X_MINUS, 0), xytext=(30, 3000),
                 fontsize=11, ha='center',
                 arrowprops=dict(arrowstyle='->', color=COLORS['antimatter'], lw=1.5),
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor=COLORS['antimatter']))

    ax1.annotate(f'$x_+ = {QUADRATIC_X_PLUS:.3f}$\n$(= 1/\\alpha)$',
                 xy=(QUADRATIC_X_PLUS, 0), xytext=(110, 3000),
                 fontsize=11, ha='center',
                 arrowprops=dict(arrowstyle='->', color=COLORS['matter'], lw=1.5),
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor=COLORS['matter']))

    # Mark vertex
    vertex_x = -b / (2 * a)
    vertex_y = a * vertex_x**2 + b * vertex_x + c
    ax1.scatter([vertex_x], [vertex_y], color=COLORS['highlight'],
                s=100, zorder=5, marker='s')
    ax1.annotate(f'Vertex\n$x = {vertex_x:.1f}$',
                 xy=(vertex_x, vertex_y), xytext=(vertex_x, vertex_y - 3000),
                 fontsize=9, ha='center',
                 arrowprops=dict(arrowstyle='->', color=COLORS['highlight'], lw=1))

    apply_trd_style(ax1, title='Master Quadratic: Full View',
                    xlabel='$x$', ylabel='$y = x^2 - 16G^{*2}x + 16G^{*3}$')
    ax1.set_xlim(-20, 160)
    ax1.set_ylim(-6000, 8000)

    # Panel 2: Zoomed view near x₋ (N_c root)
    x_zoom = np.linspace(-2, 12, 500)
    y_zoom = a * x_zoom**2 + b * x_zoom + c

    ax2.plot(x_zoom, y_zoom, color=COLORS['text'], linewidth=2.5)
    ax2.axhline(y=0, color=COLORS['grid'], linestyle='--', alpha=0.7, linewidth=1)
    ax2.axvline(x=0, color=COLORS['grid'], linestyle='--', alpha=0.7, linewidth=1)

    # Mark x₋ root
    ax2.scatter([QUADRATIC_X_MINUS], [0], color=COLORS['antimatter'],
                s=200, zorder=5, edgecolors='white', linewidths=2)

    # Mark N_c = 3 for comparison
    ax2.axvline(x=3, color=COLORS['accent1'], linestyle=':', alpha=0.8, linewidth=2)
    ax2.annotate('$N_c = 3$\n(exact integer)',
                 xy=(3, 500), fontsize=10, ha='center', color=COLORS['accent1'])

    ax2.annotate(f'$x_- = {QUADRATIC_X_MINUS:.4f}$\n(from master quadratic)',
                 xy=(QUADRATIC_X_MINUS, 0), xytext=(7, 2000),
                 fontsize=11, ha='center',
                 arrowprops=dict(arrowstyle='->', color=COLORS['antimatter'], lw=1.5),
                 bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor=COLORS['antimatter']))

    apply_trd_style(ax2, title='Zoomed: The $N_c$ Root',
                    xlabel='$x$', ylabel='$y$')
    ax2.set_xlim(-2, 12)
    ax2.set_ylim(-1000, 4000)

    # Add equation box at the top
    eq_text = f'Master Quadratic: $x^2 - 16G^{{*2}}x + 16G^{{*3}} = 0$\n$G^* = {G_STAR:.4f}$'
    fig.text(0.5, 0.96, eq_text, ha='center', va='top',
             fontsize=12, fontweight='bold',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='#f0f0f0', edgecolor='gray'))

    plt.tight_layout(rect=[0, 0, 1, 0.92])

    return fig


if __name__ == '__main__':
    fig = generate_master_quadratic()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_1_2_master_quadratic.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
