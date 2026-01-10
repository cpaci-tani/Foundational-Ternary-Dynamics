"""
Figure 3.5: Sparse vs Dense Representation
==========================================
Compares sparse (dictionary-based) and dense (array-based)
representations for the TRD lattice.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, FancyBboxPatch
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_sparse_dense():
    """Generate the sparse vs dense representation visualization."""
    fig, axes = plt.subplots(1, 2, figsize=(14, 7), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Left: Dense representation
    ax = axes[0]
    ax.set_xlim(-0.5, 8.5)
    ax.set_ylim(-1, 8)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Dense Representation\n(Full 3D Array)', fontsize=12, fontweight='bold')

    # Draw 8x8 grid, most cells empty
    np.random.seed(42)
    occupied = [(1, 2), (3, 5), (6, 1), (4, 4), (7, 6)]

    for i in range(8):
        for j in range(8):
            if (i, j) in occupied:
                color = COLORS['matter']
            else:
                color = '#e0e0e0'
            rect = Rectangle((i, j), 0.9, 0.9, facecolor=color,
                            edgecolor='gray', linewidth=0.5)
            ax.add_patch(rect)

    ax.text(4, -0.7, 'Memory: O(N^3) for N^3 lattice\nMost cells empty (wasted)',
            fontsize=10, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#ffe0e0'))

    # Right: Sparse representation
    ax = axes[1]
    ax.set_xlim(-0.5, 8.5)
    ax.set_ylim(-1, 8)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Sparse Representation\n(Dictionary of Occupied Sites)', fontsize=12,
                fontweight='bold')

    # Draw only the occupied cells with their coordinates
    for i, (x, y) in enumerate(occupied):
        rect = Rectangle((1, 6.5 - i * 1.2), 6, 1, facecolor=COLORS['matter'],
                         edgecolor='black', linewidth=1)
        ax.add_patch(rect)
        ax.text(4, 7 - i * 1.2, f'({x}, {y}) -> Voxel Data', fontsize=10,
                ha='center', va='center', color='white', fontweight='bold')

    ax.text(4, -0.7, 'Memory: O(M) for M particles\nOnly store what exists',
            fontsize=10, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#e0ffe0'))

    # Comparison table
    fig.text(0.5, 0.02, 'For 1000^3 lattice with 1000 particles:  '
            'Dense = 1 billion entries  |  Sparse = 1000 entries  |  '
            'Ratio: 1,000,000x more efficient',
            fontsize=11, ha='center', fontweight='bold',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white', edgecolor='gray'))

    fig.suptitle('Data Structure Choice: Sparse is Essential for Large Simulations',
                fontsize=14, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0.08, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_sparse_dense()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_3_5_sparse_dense.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
