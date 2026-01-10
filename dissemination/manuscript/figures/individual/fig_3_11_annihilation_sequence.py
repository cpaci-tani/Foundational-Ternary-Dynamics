"""
Figure 3.11: Annihilation Sequence
==================================
Step-by-step visualization of matter-antimatter annihilation
in TRD.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_annihilation_sequence():
    """Generate the annihilation sequence visualization."""
    fig, axes = plt.subplots(1, 4, figsize=(14, 4), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    stages = [
        ('t = 0', 'Approach'),
        ('t = 1', 'Adjacent'),
        ('t = 2', 'Annihilation'),
        ('t = 3', 'Photons'),
    ]

    for ax, (time, stage) in zip(axes, stages):
        ax.set_xlim(-2, 2)
        ax.set_ylim(-2, 2)
        ax.set_aspect('equal')
        ax.axis('off')
        ax.set_title(f'{time}: {stage}', fontsize=11, fontweight='bold')

    # Stage 1: Approach
    ax = axes[0]
    ax.add_patch(Circle((-1, 0), 0.3, facecolor=COLORS['matter'], edgecolor='black',
                        linewidth=2))
    ax.text(-1, 0, '+', fontsize=14, ha='center', va='center', color='white',
            fontweight='bold')
    ax.add_patch(Circle((1, 0), 0.3, facecolor=COLORS['antimatter'], edgecolor='black',
                        linewidth=2))
    ax.text(1, 0, '-', fontsize=14, ha='center', va='center', color='white',
            fontweight='bold')
    ax.annotate('', xy=(-0.5, 0), xytext=(-0.7, 0),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))
    ax.annotate('', xy=(0.5, 0), xytext=(0.7, 0),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))

    # Stage 2: Adjacent
    ax = axes[1]
    ax.add_patch(Circle((-0.35, 0), 0.3, facecolor=COLORS['matter'], edgecolor='black',
                        linewidth=2))
    ax.text(-0.35, 0, '+', fontsize=14, ha='center', va='center', color='white',
            fontweight='bold')
    ax.add_patch(Circle((0.35, 0), 0.3, facecolor=COLORS['antimatter'], edgecolor='black',
                        linewidth=2))
    ax.text(0.35, 0, '-', fontsize=14, ha='center', va='center', color='white',
            fontweight='bold')
    # Interaction indicator
    ax.plot([0, 0], [-0.5, 0.5], color='yellow', linewidth=3, alpha=0.7)
    ax.text(0, -1, 'Collision\ndetected', fontsize=9, ha='center')

    # Stage 3: Annihilation
    ax = axes[2]
    # Flash effect
    for r in [0.3, 0.5, 0.7, 0.9]:
        ax.add_patch(Circle((0, 0), r, facecolor='yellow', edgecolor='none',
                           alpha=0.8 - r*0.6))
    # Void particles
    ax.add_patch(Circle((-0.3, 0), 0.25, facecolor=COLORS['void'], edgecolor='black',
                        linewidth=1, alpha=0.5))
    ax.add_patch(Circle((0.3, 0), 0.25, facecolor=COLORS['void'], edgecolor='black',
                        linewidth=1, alpha=0.5))
    ax.text(0, -1.2, 'Both -> state 0\nFlux burst emitted', fontsize=9, ha='center')

    # Stage 4: Photons
    ax = axes[3]
    # Two photon waves going opposite directions
    t = np.linspace(0, 1.5, 75)

    # Photon 1 (left)
    x1 = -t
    y1 = 0.2 * np.sin(15 * t)
    ax.plot(x1, y1, color=COLORS['highlight'], linewidth=2)
    ax.annotate('', xy=(-1.7, 0), xytext=(-1.5, 0),
               arrowprops=dict(arrowstyle='->', color=COLORS['highlight'], lw=2))

    # Photon 2 (right)
    x2 = t
    y2 = 0.2 * np.sin(15 * t)
    ax.plot(x2, y2, color=COLORS['highlight'], linewidth=2)
    ax.annotate('', xy=(1.7, 0), xytext=(1.5, 0),
               arrowprops=dict(arrowstyle='->', color=COLORS['highlight'], lw=2))

    ax.text(0, -1.2, 'Energy -> 2 photons\n(511 keV each)', fontsize=9, ha='center')

    fig.suptitle('Annihilation: e+ + e- -> 2 Photons (511 keV each)',
                fontsize=14, fontweight='bold', y=1.02)

    plt.tight_layout()
    return fig


if __name__ == '__main__':
    fig = generate_annihilation_sequence()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_3_11_annihilation_sequence.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
