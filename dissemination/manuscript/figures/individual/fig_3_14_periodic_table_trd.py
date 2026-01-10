"""
Figure 3.14: Periodic Table TRD View
====================================
Periodic table highlighting TRD-relevant properties:
shell filling, derived masses, and patterns.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_periodic_table_trd():
    """Generate the TRD periodic table visualization."""
    fig, ax = plt.subplots(figsize=(16, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])
    ax.set_xlim(0, 18.5)
    ax.set_ylim(0, 10)
    ax.axis('off')

    # Element data (abbreviated - just first 20 elements)
    elements = [
        # Row 1
        (1, 1, 'H', 1, 's'),
        (18, 1, 'He', 2, 's'),
        # Row 2
        (1, 2, 'Li', 3, 's'),
        (2, 2, 'Be', 4, 's'),
        (13, 2, 'B', 5, 'p'),
        (14, 2, 'C', 6, 'p'),
        (15, 2, 'N', 7, 'p'),
        (16, 2, 'O', 8, 'p'),
        (17, 2, 'F', 9, 'p'),
        (18, 2, 'Ne', 10, 'p'),
        # Row 3
        (1, 3, 'Na', 11, 's'),
        (2, 3, 'Mg', 12, 's'),
        (13, 3, 'Al', 13, 'p'),
        (14, 3, 'Si', 14, 'p'),
        (15, 3, 'P', 15, 'p'),
        (16, 3, 'S', 16, 'p'),
        (17, 3, 'Cl', 17, 'p'),
        (18, 3, 'Ar', 18, 'p'),
        # Row 4 (partial)
        (1, 4, 'K', 19, 's'),
        (2, 4, 'Ca', 20, 's'),
    ]

    # Color by block
    block_colors = {
        's': '#FF9999',  # Red for s-block
        'p': '#99FF99',  # Green for p-block
        'd': '#9999FF',  # Blue for d-block
        'f': '#FFFF99',  # Yellow for f-block
    }

    cell_size = 0.9
    y_offset = 9

    for col, row, symbol, Z, block in elements:
        x = col - 0.5
        y = y_offset - row

        # Draw cell
        rect = Rectangle((x, y), cell_size, cell_size,
                         facecolor=block_colors[block],
                         edgecolor='black', linewidth=1)
        ax.add_patch(rect)

        # Element symbol
        ax.text(x + cell_size/2, y + cell_size/2 + 0.1, symbol,
                fontsize=12, fontweight='bold', ha='center', va='center')

        # Atomic number
        ax.text(x + 0.1, y + cell_size - 0.15, str(Z),
                fontsize=7, ha='left', va='top')

    # Legend
    ax.text(0.5, 0.8, 'Block Colors:', fontsize=10, fontweight='bold')
    for i, (block, color) in enumerate(block_colors.items()):
        ax.add_patch(Rectangle((2 + i*2, 0.5), 0.8, 0.5, facecolor=color,
                               edgecolor='black'))
        ax.text(2.4 + i*2, 0.75, f'{block}-block', fontsize=9, ha='left', va='center')

    # TRD interpretation
    trd_text = (
        "TRD Interpretation:\n"
        "- Shell structure from flux equilibria\n"
        "- Block patterns from angular momentum l\n"
        "- Periodicity from n^2 shell capacity"
    )
    ax.text(10, 1.5, trd_text, fontsize=10, ha='left',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    ax.set_title('Periodic Table: TRD Shell Structure View',
                fontsize=14, fontweight='bold')

    return fig


if __name__ == '__main__':
    fig = generate_periodic_table_trd()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_14_periodic_table_trd.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
