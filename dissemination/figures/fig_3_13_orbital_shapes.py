"""
Figure 3.13: Orbital Shapes
===========================
Visualizes s, p, and d orbital shapes as flux probability densities.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Ellipse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_orbital_shapes():
    """Generate the orbital shapes visualization."""
    fig, axes = plt.subplots(2, 3, figsize=(14, 9), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # s orbitals
    ax = axes[0, 0]
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('1s Orbital', fontsize=12, fontweight='bold')

    # Spherical s orbital
    for r in [2.5, 2.0, 1.5, 1.0, 0.5]:
        ax.add_patch(Circle((0, 0), r, facecolor=COLORS['antimatter'],
                           edgecolor='none', alpha=0.15))
    ax.add_patch(Circle((0, 0), 0.1, facecolor='black'))  # nucleus
    ax.text(0, -2.7, 'l = 0, m = 0\nSpherical', fontsize=9, ha='center')

    # 2s orbital (with node)
    ax = axes[0, 1]
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('2s Orbital', fontsize=12, fontweight='bold')

    # Outer shell
    for r in [2.5, 2.2, 1.9]:
        ax.add_patch(Circle((0, 0), r, facecolor=COLORS['antimatter'],
                           edgecolor='none', alpha=0.15))
    # Node (white ring)
    ax.add_patch(Circle((0, 0), 1.2, facecolor='white', edgecolor='none'))
    # Inner shell
    for r in [0.8, 0.5]:
        ax.add_patch(Circle((0, 0), r, facecolor=COLORS['antimatter'],
                           edgecolor='none', alpha=0.2))
    ax.add_patch(Circle((0, 0), 0.1, facecolor='black'))
    ax.text(0, -2.7, 'l = 0, m = 0\nWith radial node', fontsize=9, ha='center')

    # p orbital
    ax = axes[0, 2]
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('2p Orbital (pz)', fontsize=12, fontweight='bold')

    # Two lobes
    ax.add_patch(Ellipse((0, 1.2), 1.5, 2, facecolor=COLORS['antimatter'],
                        edgecolor='black', linewidth=1, alpha=0.7))
    ax.add_patch(Ellipse((0, -1.2), 1.5, 2, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=1, alpha=0.7))
    ax.add_patch(Circle((0, 0), 0.1, facecolor='black'))
    ax.text(0, 1.2, '+', fontsize=14, ha='center', va='center', color='white')
    ax.text(0, -1.2, '-', fontsize=14, ha='center', va='center', color='white')
    ax.text(0, -2.7, 'l = 1, m = 0\nDumbbell shape', fontsize=9, ha='center')

    # d orbital (dz2)
    ax = axes[1, 0]
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('3d Orbital (dz2)', fontsize=12, fontweight='bold')

    # Two lobes on z-axis
    ax.add_patch(Ellipse((0, 1.5), 1, 1.5, facecolor=COLORS['antimatter'],
                        edgecolor='black', linewidth=1, alpha=0.7))
    ax.add_patch(Ellipse((0, -1.5), 1, 1.5, facecolor=COLORS['antimatter'],
                        edgecolor='black', linewidth=1, alpha=0.7))
    # Torus in middle
    ax.add_patch(Ellipse((0, 0), 2.5, 0.6, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=1, alpha=0.5))
    ax.add_patch(Circle((0, 0), 0.1, facecolor='black'))
    ax.text(0, -2.7, 'l = 2, m = 0\nLobes + torus', fontsize=9, ha='center')

    # d orbital (dx2-y2)
    ax = axes[1, 1]
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('3d Orbital (dx2-y2)', fontsize=12, fontweight='bold')

    # Four lobes on axes
    for angle in [0, 90, 180, 270]:
        rad = np.radians(angle)
        x = 1.2 * np.cos(rad)
        y = 1.2 * np.sin(rad)
        color = COLORS['antimatter'] if angle in [0, 180] else COLORS['matter']
        ax.add_patch(Ellipse((x, y), 1.5, 1, angle=angle,
                            facecolor=color, edgecolor='black', alpha=0.7))
    ax.add_patch(Circle((0, 0), 0.1, facecolor='black'))
    ax.text(0, -2.7, 'l = 2, m = +/-2\nFour-leaf clover', fontsize=9, ha='center')

    # Summary
    ax = axes[1, 2]
    ax.set_xlim(0, 3)
    ax.set_ylim(0, 3)
    ax.axis('off')
    ax.set_title('Orbital Summary', fontsize=12, fontweight='bold')

    summary = (
        "Angular Momentum l:\n\n"
        "l = 0: s (sphere)\n"
        "l = 1: p (dumbbell)\n"
        "l = 2: d (cloverleaf)\n"
        "l = 3: f (complex)\n\n"
        "TRD: Orbitals are\nflux standing waves"
    )
    ax.text(1.5, 1.5, summary, fontsize=10, ha='center', va='center',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     edgecolor='gray'))

    fig.suptitle('Atomic Orbital Shapes: Flux Probability Distributions',
                fontsize=14, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_orbital_shapes()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_13_orbital_shapes.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
