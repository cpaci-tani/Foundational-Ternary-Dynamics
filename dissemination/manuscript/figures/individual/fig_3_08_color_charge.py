"""
Figure 3.8: Color Charge Visualization
======================================
Shows the three color charges (red, green, blue) and how
they combine to form color-neutral hadrons.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge, RegularPolygon
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_color_charge():
    """Generate the color charge visualization."""
    fig, axes = plt.subplots(1, 3, figsize=(14, 5), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Color definitions
    red = '#FF4444'
    green = '#44FF44'
    blue = '#4444FF'
    antired = '#00BBBB'  # Cyan
    antigreen = '#FF00FF'  # Magenta
    antiblue = '#FFFF00'  # Yellow

    # Left: Three color charges
    ax = axes[0]
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2.5, 2)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Quark Color Charges', fontsize=12, fontweight='bold')

    # Three quarks in triangle
    positions = [(-1, -1), (1, -1), (0, 0.73)]
    colors = [red, green, blue]
    labels = ['Red', 'Green', 'Blue']

    for pos, color, label in zip(positions, colors, labels):
        ax.add_patch(Circle(pos, 0.4, facecolor=color, edgecolor='black', linewidth=2))
        ax.text(pos[0], pos[1] - 0.7, label, fontsize=10, ha='center')

    ax.text(0, 1.7, 'N_c = 3 colors', fontsize=11, ha='center', fontweight='bold')

    # Middle: Color-neutral combinations
    ax = axes[1]
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2.5, 2)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Color-Neutral Hadrons', fontsize=12, fontweight='bold')

    # Baryon (RGB = white)
    ax.text(0, 1.7, 'Baryon: R + G + B = White', fontsize=10, ha='center')

    # Draw overlapping circles
    for pos, color in zip([(-0.3, 0.7), (0.3, 0.7), (0, 1.2)],
                          [red, green, blue]):
        ax.add_patch(Circle(pos, 0.3, facecolor=color, edgecolor='black',
                           linewidth=1, alpha=0.7))

    # Meson (color + anticolor)
    ax.text(0, -0.3, 'Meson: Color + Anticolor', fontsize=10, ha='center')
    ax.add_patch(Circle((-0.3, -1), 0.3, facecolor=red, edgecolor='black',
                       linewidth=1))
    ax.add_patch(Circle((0.3, -1), 0.3, facecolor=antired, edgecolor='black',
                       linewidth=1))
    ax.text(0, -1.8, 'R + Anti-R = Neutral', fontsize=9, ha='center')

    # Right: TRD interpretation
    ax = axes[2]
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2.5, 2)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('TRD: Flux Axis Alignment', fontsize=12, fontweight='bold')

    # Show 3D axes
    origin = (0, 0)
    ax.annotate('', xy=(1.5, 0), xytext=origin,
               arrowprops=dict(arrowstyle='->', color=red, lw=3))
    ax.text(1.7, 0, 'x (Red)', fontsize=9, color=red, va='center')

    ax.annotate('', xy=(0, 1.5), xytext=origin,
               arrowprops=dict(arrowstyle='->', color=green, lw=3))
    ax.text(0, 1.7, 'y (Green)', fontsize=9, color=green, ha='center')

    ax.annotate('', xy=(-1, -0.8), xytext=origin,
               arrowprops=dict(arrowstyle='->', color=blue, lw=3))
    ax.text(-1.3, -1, 'z (Blue)', fontsize=9, color=blue)

    ax.text(0, -1.8, 'Color = primary flux axis\nN_c = 3.024 from curve geometry',
            fontsize=9, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    fig.suptitle('Color Charge: SU(3) from Spatial Geometry',
                fontsize=14, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0, 1, 0.93])
    return fig


if __name__ == '__main__':
    fig = generate_color_charge()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_3_8_color_charge.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
