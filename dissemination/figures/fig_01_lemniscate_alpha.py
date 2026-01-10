"""
Figure 1.1: Lemniscate-Alpha Curve
==================================
The fundamental curve from which α = 1/137.036 is derived.

Parametric equations:
x(t) = Σ aₖ cos(2^k t) for k = 0,1,2,3,4
y(t) = Σ bₖ sin(2^k t) for k = 0,1,2,3,4

Where:
- a = [1, 1/2, 1/2, 2/5, 1/16]
- b = [1, -1/2, 1/2, -7/20, 1/16]

Shows:
- Combined curve in black
- Individual harmonic modes in distinct colors
- Arc length annotation
"""

import numpy as np
import matplotlib.pyplot as plt
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, MODE_COLORS, apply_trd_style
from utils.physics_constants import (
    FREQUENCIES, X_AMPLITUDES, Y_AMPLITUDES,
    ARC_LENGTH, G_STAR, ALPHA_INV
)


def calculate_arc_length(x, y):
    """Calculate approximate arc length of parametric curve."""
    dx = np.diff(x)
    dy = np.diff(y)
    return np.sum(np.sqrt(dx**2 + dy**2))


def generate_lemniscate_alpha():
    """
    Generate the Lemniscate-Alpha curve figure.

    Shows:
    - Full parametric curve in black
    - Individual harmonic modes in distinct colors
    - Key mathematical annotations
    """
    fig, ax = plt.subplots(figsize=(10, 8), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Parameter range
    t = np.linspace(0, 2 * np.pi, 2000)

    # Calculate full curve
    x_full = np.zeros_like(t)
    y_full = np.zeros_like(t)

    for j, freq in enumerate(FREQUENCIES):
        x_full += X_AMPLITUDES[j] * np.cos(freq * t)
        y_full += Y_AMPLITUDES[j] * np.sin(freq * t)

    # Plot individual harmonic modes (faded)
    for j, freq in enumerate(FREQUENCIES):
        x_mode = X_AMPLITUDES[j] * np.cos(freq * t)
        y_mode = Y_AMPLITUDES[j] * np.sin(freq * t)
        ax.plot(x_mode, y_mode, color=MODE_COLORS[freq],
                alpha=0.4, linewidth=1.5,
                label=f'Mode {freq}: $a_{j}$={X_AMPLITUDES[j]:.3f}')

    # Plot combined curve (bold)
    ax.plot(x_full, y_full, 'k-', linewidth=3, label='Combined curve', zorder=10)

    # Mark origin
    ax.scatter([0], [0], color=COLORS['highlight'], s=100, zorder=15,
               edgecolors='black', linewidths=1)
    ax.annotate('Origin', xy=(0, 0), xytext=(0.2, 0.2),
                fontsize=10, arrowprops=dict(arrowstyle='->', lw=1))

    # Mark extrema points
    max_x_idx = np.argmax(x_full)
    min_x_idx = np.argmin(x_full)
    ax.scatter([x_full[max_x_idx]], [y_full[max_x_idx]],
               color=COLORS['matter'], s=80, zorder=15, marker='>')
    ax.scatter([x_full[min_x_idx]], [y_full[min_x_idx]],
               color=COLORS['antimatter'], s=80, zorder=15, marker='<')

    # Calculate arc length
    arc_len = calculate_arc_length(x_full, y_full)

    # Configure axes
    ax.set_xlim(-2.5, 2.5)
    ax.set_ylim(-2, 2)
    ax.set_aspect('equal')

    apply_trd_style(ax,
                    title='The Lemniscate-Alpha Curve',
                    xlabel='$x(t)$', ylabel='$y(t)$')

    # Legend for harmonic modes
    legend = ax.legend(loc='upper right', fontsize=9, framealpha=0.95,
                       title='Harmonic Modes', title_fontsize=10)

    # Parametric equations box
    eq_text = (
        r'$x(t) = \sum_{k=0}^{4} a_k \cos(2^k t)$' + '\n'
        r'$y(t) = \sum_{k=0}^{4} b_k \sin(2^k t)$' + '\n\n'
        r'$a = [1, \frac{1}{2}, \frac{1}{2}, \frac{2}{5}, \frac{1}{16}]$' + '\n'
        r'$b = [1, -\frac{1}{2}, \frac{1}{2}, -\frac{7}{20}, \frac{1}{16}]$'
    )
    ax.text(0.02, 0.98, eq_text, transform=ax.transAxes,
            fontsize=10, verticalalignment='top',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     alpha=0.95, edgecolor='gray'))

    # Derivation chain box
    derivation_text = (
        f'Arc Length: $L \\approx {arc_len:.4f}$\n'
        f'$G^* = L \\times \\frac{{91}}{{732}} \\approx {G_STAR:.4f}$\n'
        f'Master Quadratic $\\rightarrow$ $1/\\alpha = {ALPHA_INV:.3f}$'
    )
    ax.text(0.98, 0.02, derivation_text, transform=ax.transAxes,
            fontsize=10, verticalalignment='bottom', horizontalalignment='right',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#ffffd0',
                     alpha=0.95, edgecolor=COLORS['highlight']))

    # Color legend for modes
    mode_labels = ['n=1 (fundamental)', 'n=2 (1st harmonic)',
                   'n=4 (2nd harmonic)', 'n=8 (3rd harmonic)',
                   'n=16 (4th harmonic)']

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_lemniscate_alpha()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_1_1_lemniscate_alpha.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
