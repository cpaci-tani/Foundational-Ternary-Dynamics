"""
Figure 3.1: Tick Sequence Detail
================================
Detailed breakdown of what happens during a single simulation tick,
showing the 12 phases in order.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_tick_sequence():
    """Generate the tick sequence detail visualization."""
    fig, ax = plt.subplots(figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])
    ax.set_xlim(0, 14)
    ax.set_ylim(0, 10)
    ax.axis('off')

    # Define the 12 phases
    phases = [
        ('1. TIME GATING', 'Check phase accumulators\nMark active voxels', '#FFE66D'),
        ('2. ENTROPY', 'Apply decay to unlocked\nmanifested voxels', '#FF6B6B'),
        ('3. EXISTENCE', 'Check evaporation\nCheck genesis', '#4ECDC4'),
        ('4. WAVE PROP', 'Update wave velocities\nUpdate flux vectors', '#95E1D3'),
        ('5. FIELDS', 'Compute density\ngradient, div, curl', '#DCD6F7'),
        ('6. FORCES', 'Gravity, Coulomb\nLorentz, Strong, Weak', '#F7DC6F'),
        ('7. INTEGRATE', 'Update velocities\nAccumulate remainders', '#85C1E9'),
        ('8. MOVEMENT', 'Integer position updates\nEnforce speed limit', '#F1948A'),
        ('9. COLLISIONS', 'Empty: move\nSame: elastic\nOpposite: annihilate', '#82E0AA'),
        ('10. TRANSMUTE', 'Weak-force polarity\nflips if stressed', '#D7BDE2'),
        ('11. BINDING', 'Detect stable configs\nSet is_locked flag', '#F5B041'),
        ('12. INCREMENT', 't <- t + 1', '#AEB6BF'),
    ]

    # Layout: 3 columns, 4 rows
    cols = 3
    rows = 4
    box_width = 4
    box_height = 2
    x_spacing = 4.5
    y_spacing = 2.3

    for i, (title, desc, color) in enumerate(phases):
        col = i % cols
        row = i // cols

        x = 0.5 + col * x_spacing
        y = 9 - row * y_spacing

        # Draw box
        box = FancyBboxPatch((x, y - box_height/2), box_width, box_height,
                            boxstyle="round,pad=0.02,rounding_size=0.1",
                            facecolor=color, edgecolor='black', linewidth=2)
        ax.add_patch(box)

        # Title
        ax.text(x + box_width/2, y + 0.5, title, fontsize=10, fontweight='bold',
                ha='center', va='center')

        # Description
        ax.text(x + box_width/2, y - 0.3, desc, fontsize=8,
                ha='center', va='center', linespacing=1.2)

        # Draw arrows between phases
        if i < len(phases) - 1:
            if col < cols - 1:
                # Arrow to right
                ax.annotate('', xy=(x + box_width + 0.3, y),
                           xytext=(x + box_width + 0.1, y),
                           arrowprops=dict(arrowstyle='->', color='gray', lw=2))
            else:
                # Arrow down to next row
                ax.annotate('', xy=(0.5 + box_width/2, y - box_height/2 - 0.1),
                           xytext=(x + box_width/2, y - box_height/2 - 0.1),
                           arrowprops=dict(arrowstyle='->', color='gray', lw=2,
                                          connectionstyle='arc3,rad=0.3'))

    # Title
    ax.text(7, 9.7, 'Single Tick: 12 Update Phases', fontsize=16, fontweight='bold',
            ha='center')

    # Footer
    ax.text(7, 0.3, 'Each tick advances the simulation by one discrete time unit.\n'
                   'Order matters: changing phase sequence changes emergent behavior.',
            fontsize=10, ha='center', style='italic',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='gray'))

    return fig


if __name__ == '__main__':
    fig = generate_tick_sequence()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_3_1_tick_sequence.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
