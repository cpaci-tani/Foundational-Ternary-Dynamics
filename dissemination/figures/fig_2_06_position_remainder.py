"""
Figure 2.6: Position Remainder
==============================
Shows how fractional positions accumulate until they cause
a discrete lattice jump.

In TRD, particles have:
- Integer position (which voxel they occupy)
- Fractional remainder (sub-lattice position)

When |remainder| >= 1, the particle jumps to an adjacent voxel.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Rectangle, Circle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_position_remainder():
    """
    Generate the position remainder visualization.

    Shows:
    1. Top: Time series of remainder accumulation
    2. Bottom: Lattice view of particle jumping
    """
    fig, (ax_top, ax_bottom) = plt.subplots(2, 1, figsize=(12, 10), dpi=150,
                                             height_ratios=[1, 1.2])
    fig.patch.set_facecolor(COLORS['background'])

    # =========================================================================
    # Top: Remainder accumulation over time
    # =========================================================================
    ticks = np.arange(0, 25)
    velocity = 0.15  # Fractional velocity per tick

    remainder = np.zeros(len(ticks))
    position = np.zeros(len(ticks))

    current_pos = 0
    current_rem = 0

    for i, t in enumerate(ticks):
        current_rem += velocity
        if current_rem >= 1.0:
            current_pos += 1
            current_rem -= 1.0
        elif current_rem <= -1.0:
            current_pos -= 1
            current_rem += 1.0

        remainder[i] = current_rem
        position[i] = current_pos

    # Plot remainder
    ax_top.fill_between(ticks, 0, remainder, alpha=0.3, color=COLORS['highlight'])
    ax_top.plot(ticks, remainder, 'o-', color=COLORS['highlight'],
                linewidth=2, markersize=6)

    # Mark threshold crossings
    ax_top.axhline(y=1.0, color=COLORS['matter'], linestyle='--', linewidth=2,
                   label='Jump threshold (+1)')
    ax_top.axhline(y=0, color='gray', linestyle='-', linewidth=1, alpha=0.5)

    # Mark jump events
    jump_ticks = [6, 13, 20]  # Approximate tick numbers where jumps occur
    for jt in jump_ticks:
        if jt < len(ticks):
            ax_top.axvline(x=jt, color=COLORS['matter'], linestyle=':',
                          linewidth=2, alpha=0.7)
            ax_top.annotate('JUMP!', xy=(jt, 0.9), fontsize=9,
                           color=COLORS['matter'], fontweight='bold')

    ax_top.set_xlim(-1, 25)
    ax_top.set_ylim(-0.2, 1.2)
    apply_trd_style(ax_top, title='Fractional Remainder Accumulation',
                    xlabel='Tick (t)', ylabel='Remainder')

    ax_top.text(0.02, 0.95, f'Velocity = {velocity} per tick',
                transform=ax_top.transAxes, fontsize=10,
                bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Bottom: Lattice view
    # =========================================================================
    ax_bottom.set_xlim(-0.5, 6.5)
    ax_bottom.set_ylim(-0.5, 4)
    ax_bottom.set_aspect('equal')
    ax_bottom.axis('off')

    # Draw lattice grid
    for x in range(7):
        for y_offset in [0.5, 2.5]:
            rect = Rectangle((x - 0.4, y_offset - 0.4), 0.8, 0.8,
                             facecolor='#f0f0f0', edgecolor='gray',
                             linewidth=1)
            ax_bottom.add_patch(rect)
            ax_bottom.text(x, y_offset - 0.55, str(x), fontsize=9,
                          ha='center', color='gray')

    # Time labels
    ax_bottom.text(-0.5, 0.5, 't=0', fontsize=10, ha='right', va='center')
    ax_bottom.text(-0.5, 2.5, 't=7', fontsize=10, ha='right', va='center')

    # Show particle at different states
    # t=0: particle at position 0 with remainder 0
    circle0 = Circle((0, 0.5), 0.25, facecolor=COLORS['matter'],
                     edgecolor='black', linewidth=2)
    ax_bottom.add_patch(circle0)
    ax_bottom.text(0, 0.9, 'r=0.0', fontsize=8, ha='center')

    # t=7: particle jumped to position 1, remainder reset
    circle7 = Circle((1, 2.5), 0.25, facecolor=COLORS['matter'],
                     edgecolor='black', linewidth=2)
    ax_bottom.add_patch(circle7)
    ax_bottom.text(1, 2.9, 'r=0.05', fontsize=8, ha='center')

    # Show the jump arrow
    ax_bottom.annotate('', xy=(1, 2.0), xytext=(0, 1.0),
                      arrowprops=dict(arrowstyle='->', color=COLORS['accent1'],
                                     lw=3, connectionstyle='arc3,rad=0.2'))
    ax_bottom.text(0.5, 1.5, 'Jump when\nr ≥ 1.0', fontsize=10, ha='center',
                  color=COLORS['accent1'], fontweight='bold')

    # Remainder indicator (conceptual bar)
    bar_x = 3.5
    ax_bottom.add_patch(Rectangle((bar_x, 0.1), 0.3, 0.8,
                                   facecolor='#e0e0e0', edgecolor='gray'))
    ax_bottom.add_patch(Rectangle((bar_x, 0.1), 0.3, 0.75 * 0.8,
                                   facecolor=COLORS['highlight'], edgecolor='none'))
    ax_bottom.text(bar_x + 0.15, 0.95, 'r=0.75', fontsize=8, ha='center')
    ax_bottom.text(bar_x + 0.15, 0.0, 'Remainder', fontsize=8, ha='center')

    # Equation box
    eq_text = (
        "Position Update:\n"
        "r ← r + velocity\n"
        "if r ≥ 1: pos += 1, r -= 1\n"
        "if r ≤ -1: pos -= 1, r += 1"
    )
    ax_bottom.text(5.5, 2.5, eq_text, fontsize=10, ha='left', va='center',
                  family='monospace',
                  bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                           edgecolor='gray'))

    ax_bottom.set_title('Lattice Position Jumps', fontsize=14, fontweight='bold',
                        pad=10)

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Position Remainder: Discrete Motion from Continuous Velocity',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "Particles accumulate fractional position until they have 'enough' to move one lattice unit.\n"
        "This bridges continuous velocity with discrete lattice structure."
    )
    fig.text(0.5, 0.02, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_position_remainder()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_2_6_position_remainder.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
