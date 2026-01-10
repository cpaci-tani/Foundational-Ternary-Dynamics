"""
Figure 3.7: Spin States
=======================
Visualizes spin-1/2 particles and their states in TRD,
showing how spin emerges from framed flux orientation.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, Wedge
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_spin_states():
    """Generate the spin states visualization."""
    fig, axes = plt.subplots(1, 3, figsize=(14, 5), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Left: Spin up
    ax = axes[0]
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2, 2)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Spin Up |+>', fontsize=12, fontweight='bold')

    # Particle
    ax.add_patch(Circle((0, 0), 0.5, facecolor=COLORS['matter'], edgecolor='black',
                        linewidth=2))
    # Spin arrow up
    ax.annotate('', xy=(0, 1.5), xytext=(0, 0.6),
               arrowprops=dict(arrowstyle='->', color='black', lw=3))
    ax.text(0, 1.7, 'S', fontsize=12, ha='center', fontweight='bold')

    # Magnetic field alignment
    ax.text(0, -1.5, 'Aligned with B-field', fontsize=10, ha='center')

    # Middle: Spin down
    ax = axes[1]
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2, 2)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Spin Down |->', fontsize=12, fontweight='bold')

    # Particle
    ax.add_patch(Circle((0, 0), 0.5, facecolor=COLORS['matter'], edgecolor='black',
                        linewidth=2))
    # Spin arrow down
    ax.annotate('', xy=(0, -1.5), xytext=(0, -0.6),
               arrowprops=dict(arrowstyle='->', color='black', lw=3))
    ax.text(0, -1.7, 'S', fontsize=12, ha='center', fontweight='bold')

    # Magnetic field alignment
    ax.text(0, 1.5, 'Anti-aligned with B-field', fontsize=10, ha='center')

    # Right: Superposition (TRD interpretation)
    ax = axes[2]
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2, 2)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('TRD: Framed Flux', fontsize=12, fontweight='bold')

    # Particle with rotating flux arrows
    ax.add_patch(Circle((0, 0), 0.5, facecolor=COLORS['highlight'], edgecolor='black',
                        linewidth=2))

    # Multiple flux direction indicators
    for angle in [0, 60, 120, 180, 240, 300]:
        rad = np.radians(angle)
        x1 = 0.6 * np.cos(rad)
        y1 = 0.6 * np.sin(rad)
        x2 = 1.2 * np.cos(rad)
        y2 = 1.2 * np.sin(rad)
        ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                   arrowprops=dict(arrowstyle='->', color=COLORS['accent1'], lw=1.5,
                                  alpha=0.7))

    ax.text(0, -1.7, 'Spin = net rotation\nof flux frame', fontsize=10, ha='center')

    fig.suptitle('Spin-1/2: Intrinsic Angular Momentum from Flux Orientation',
                fontsize=14, fontweight='bold', y=0.98)

    # Bottom explanation
    fig.text(0.5, 0.02, 'In TRD, spin emerges from the handedness of the flux frame. '
            '720 degree rotation returns to original state (spinor behavior).',
            fontsize=10, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='gray'))

    plt.tight_layout(rect=[0, 0.08, 1, 0.93])
    return fig


if __name__ == '__main__':
    fig = generate_spin_states()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_3_7_spin_states.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
