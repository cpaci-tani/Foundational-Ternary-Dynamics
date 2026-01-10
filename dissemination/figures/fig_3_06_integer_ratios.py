"""
Figure 3.6: Integer Ratios Table
================================
Table showing the key integer ratios that appear
throughout TRD derivations.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_integer_ratios():
    """Generate the integer ratios table visualization."""
    fig, ax = plt.subplots(figsize=(12, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 10)
    ax.axis('off')

    # Title
    ax.text(6, 9.5, 'Key Integer Ratios in TRD', fontsize=16, fontweight='bold',
            ha='center')

    # Table data
    headers = ['Ratio', 'Value', 'Physical Meaning']
    rows = [
        ['91/732', '0.1243...', 'Arc length scaling to G*'],
        ['1/137', '0.00729...', 'Fine structure constant alpha'],
        ['3/13', '0.2308...', 'Weinberg angle sin^2(theta_W)'],
        ['7/59', '0.1186...', 'Strong coupling alpha_s'],
        ['1/100', '0.01', 'Gravitational coupling (1/(7+3)^2)'],
        ['70/1', '70', 'Electron mass factor (7 x 10 x alpha)'],
        ['13/1', '13', 'Effective dimension n_eff'],
        ['7/1', '7', 'QCD beta coefficient b_3'],
        ['3/1', '3', 'Color charges N_c'],
        ['4/1', '4', 'Base harmonic modes N_base'],
    ]

    # Draw table
    col_widths = [2.5, 2.5, 5]
    row_height = 0.7
    x_start = 1
    y_start = 8.5

    # Header row
    x = x_start
    for j, (header, width) in enumerate(zip(headers, col_widths)):
        rect = Rectangle((x, y_start), width, row_height,
                         facecolor=COLORS['highlight'], edgecolor='black', linewidth=2)
        ax.add_patch(rect)
        ax.text(x + width/2, y_start + row_height/2, header,
                fontsize=11, fontweight='bold', ha='center', va='center')
        x += width

    # Data rows
    for i, row in enumerate(rows):
        y = y_start - (i + 1) * row_height
        x = x_start

        # Alternate row colors
        bg_color = 'white' if i % 2 == 0 else '#f5f5f5'

        for j, (cell, width) in enumerate(zip(row, col_widths)):
            rect = Rectangle((x, y), width, row_height,
                             facecolor=bg_color, edgecolor='black', linewidth=1)
            ax.add_patch(rect)

            # Format ratio column differently
            if j == 0:
                ax.text(x + width/2, y + row_height/2, cell,
                       fontsize=10, ha='center', va='center',
                       fontweight='bold', color=COLORS['accent1'])
            else:
                ax.text(x + width/2, y + row_height/2, cell,
                       fontsize=10, ha='center', va='center')
            x += width

    # Key insight box
    ax.text(6, 0.8, 'Key Insight: All fundamental constants derive from\n'
                   'combinations of just 4 integers: {7, 3, 13, 4}',
            fontsize=11, ha='center', fontweight='bold',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#e8f4f8',
                     edgecolor=COLORS['accent1'], linewidth=2))

    return fig


if __name__ == '__main__':
    fig = generate_integer_ratios()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_3_6_integer_ratios.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
