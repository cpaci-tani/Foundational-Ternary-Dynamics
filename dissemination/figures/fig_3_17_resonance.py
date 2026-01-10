"""
Figure 3.17: Resonance Structures
=================================
Shows resonance in molecules like benzene,
with delocalized electron flux.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, RegularPolygon
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_resonance():
    """Generate the resonance structures visualization."""
    fig, axes = plt.subplots(1, 3, figsize=(14, 5), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    def draw_benzene(ax, alternating=True, structure=1):
        """Draw benzene ring."""
        ax.set_xlim(-3, 3)
        ax.set_ylim(-3, 3)
        ax.set_aspect('equal')
        ax.axis('off')

        # Hexagon vertices
        angles = np.linspace(0, 2*np.pi, 7)[:-1]
        r = 1.5
        vertices = [(r * np.cos(a + np.pi/6), r * np.sin(a + np.pi/6)) for a in angles]

        # Draw carbon atoms
        for x, y in vertices:
            ax.add_patch(Circle((x, y), 0.2, facecolor='gray', edgecolor='black'))

        # Draw bonds
        for i in range(6):
            x1, y1 = vertices[i]
            x2, y2 = vertices[(i+1) % 6]
            ax.plot([x1, x2], [y1, y2], 'k-', linewidth=2)

            if alternating:
                # Double bonds on alternate positions
                if structure == 1 and i in [0, 2, 4]:
                    # Inner double bond
                    dx = (x2 - x1) * 0.15
                    dy = (y2 - y1) * 0.15
                    cx = (x1 + x2) / 2
                    cy = (y1 + y2) / 2
                    # Offset toward center
                    nx = -dy / np.sqrt(dx**2 + dy**2) * 0.15
                    ny = dx / np.sqrt(dx**2 + dy**2) * 0.15
                    ax.plot([x1 + nx, x2 + nx], [y1 + ny, y2 + ny], 'k-', linewidth=2)
                elif structure == 2 and i in [1, 3, 5]:
                    dx = (x2 - x1) * 0.15
                    dy = (y2 - y1) * 0.15
                    nx = -dy / np.sqrt(dx**2 + dy**2) * 0.15
                    ny = dx / np.sqrt(dx**2 + dy**2) * 0.15
                    ax.plot([x1 + nx, x2 + nx], [y1 + ny, y2 + ny], 'k-', linewidth=2)

        # Draw H atoms
        for x, y in vertices:
            # H position (outward)
            hx = x * 1.5
            hy = y * 1.5
            ax.plot([x, hx], [y, hy], 'k-', linewidth=1)
            ax.add_patch(Circle((hx, hy), 0.15, facecolor='white', edgecolor='black'))

    # Structure 1
    ax = axes[0]
    draw_benzene(ax, alternating=True, structure=1)
    ax.set_title('Structure 1', fontsize=11, fontweight='bold')

    # Double-headed arrow
    fig.text(0.39, 0.5, '<-->', fontsize=20, ha='center', va='center',
             transform=fig.transFigure)

    # Structure 2
    ax = axes[1]
    draw_benzene(ax, alternating=True, structure=2)
    ax.set_title('Structure 2', fontsize=11, fontweight='bold')

    # Equals sign
    fig.text(0.69, 0.5, '=', fontsize=24, ha='center', va='center',
             transform=fig.transFigure)

    # True structure (delocalized)
    ax = axes[2]
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_aspect('equal')
    ax.axis('off')

    # Draw with inner circle representing delocalization
    angles = np.linspace(0, 2*np.pi, 7)[:-1]
    r = 1.5
    vertices = [(r * np.cos(a + np.pi/6), r * np.sin(a + np.pi/6)) for a in angles]

    for x, y in vertices:
        ax.add_patch(Circle((x, y), 0.2, facecolor='gray', edgecolor='black'))

    for i in range(6):
        x1, y1 = vertices[i]
        x2, y2 = vertices[(i+1) % 6]
        ax.plot([x1, x2], [y1, y2], 'k-', linewidth=2)

        hx = x1 * 1.5
        hy = y1 * 1.5
        ax.plot([x1, hx], [y1, hy], 'k-', linewidth=1)
        ax.add_patch(Circle((hx, hy), 0.15, facecolor='white', edgecolor='black'))

    # Delocalized electron ring
    circle = plt.Circle((0, 0), 0.8, fill=False, color=COLORS['antimatter'],
                        linewidth=3)
    ax.add_patch(circle)
    ax.set_title('True Structure\n(Delocalized)', fontsize=11, fontweight='bold')

    fig.suptitle('Resonance: Electron Delocalization in Benzene', fontsize=14,
                fontweight='bold', y=0.98)

    fig.text(0.5, 0.02, 'TRD: Electrons are not localized in fixed bonds but '
            'form a continuous flux ring around the molecule.',
            fontsize=10, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='gray'))

    plt.tight_layout(rect=[0, 0.08, 1, 0.92])
    return fig


if __name__ == '__main__':
    fig = generate_resonance()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_17_resonance.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
