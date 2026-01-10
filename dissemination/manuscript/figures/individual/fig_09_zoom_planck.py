"""
Figure 1.9: Zoom from Macro to Planck Scale
============================================
Shows progressive magnification from macroscopic appearance
to the discrete voxel structure at Planck scale.

5 panels showing increasing granularity:
1. Macroscopic: Smooth, continuous
2. Microscopic: Very slight texture
3. Nanoscale: Visible structure
4. Near-Planck: Clear grid emerging
5. Planck scale: Individual voxels with states
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Circle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def add_zoom_bracket(ax, x, y, width, height, label, direction='right'):
    """Add a zoom bracket indicating magnification."""
    if direction == 'right':
        ax.annotate('', xy=(x + width + 0.1, y + height/2),
                   xytext=(x + width + 0.3, y + height/2),
                   arrowprops=dict(arrowstyle='->', color='gray', lw=1.5))


def generate_zoom_planck():
    """
    Generate the macro-to-Planck zoom sequence.

    5 panels showing progressive zoom levels with
    increasing discreteness visible.
    """
    fig, axes = plt.subplots(1, 5, figsize=(16, 4), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Scale labels and descriptions
    scales = [
        {'label': 'Macroscopic', 'scale': '~1 m', 'description': 'Smooth, continuous'},
        {'label': 'Microscopic', 'scale': '~10⁻⁶ m', 'description': 'Still smooth'},
        {'label': 'Nanoscale', 'scale': '~10⁻⁹ m', 'description': 'Atomic structure'},
        {'label': 'Sub-Planck', 'scale': '~10⁻³⁴ m', 'description': 'Grid emerging'},
        {'label': 'Planck Scale', 'scale': '~10⁻³⁵ m', 'description': 'Discrete voxels'},
    ]

    for i, (ax, scale_info) in enumerate(zip(axes, scales)):
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)
        ax.set_aspect('equal')
        ax.axis('off')

        # Panel background
        ax.add_patch(Rectangle((0, 0), 1, 1, facecolor='#f5f5f5',
                               edgecolor='black', linewidth=2))

        if i == 0:
            # Macroscopic: Perfectly smooth gradient
            x = np.linspace(0, 1, 100)
            y = np.linspace(0, 1, 100)
            X, Y = np.meshgrid(x, y)
            Z = np.sin(2 * np.pi * X) * np.cos(2 * np.pi * Y)
            ax.imshow(Z, extent=[0.05, 0.95, 0.05, 0.95], cmap='RdBu_r',
                     alpha=0.7, aspect='auto')

        elif i == 1:
            # Microscopic: Very slight noise
            x = np.linspace(0, 1, 50)
            y = np.linspace(0, 1, 50)
            X, Y = np.meshgrid(x, y)
            Z = np.sin(2 * np.pi * X) * np.cos(2 * np.pi * Y)
            Z += 0.1 * np.random.randn(*Z.shape)
            ax.imshow(Z, extent=[0.05, 0.95, 0.05, 0.95], cmap='RdBu_r',
                     alpha=0.7, aspect='auto')

        elif i == 2:
            # Nanoscale: Visible atomic-like structure
            x = np.linspace(0, 1, 30)
            y = np.linspace(0, 1, 30)
            X, Y = np.meshgrid(x, y)
            Z = np.sin(2 * np.pi * X * 3) * np.cos(2 * np.pi * Y * 3)
            ax.imshow(Z, extent=[0.05, 0.95, 0.05, 0.95], cmap='RdBu_r',
                     alpha=0.7, aspect='auto', interpolation='none')

        elif i == 3:
            # Sub-Planck: Clear grid starting to appear
            grid_size = 10
            for gx in range(grid_size):
                for gy in range(grid_size):
                    cx = 0.1 + gx * 0.08
                    cy = 0.1 + gy * 0.08
                    # Random state for visual interest
                    val = np.sin(gx * 2) * np.cos(gy * 3)
                    if val > 0.3:
                        color = COLORS['matter']
                        alpha = 0.6
                    elif val < -0.3:
                        color = COLORS['antimatter']
                        alpha = 0.6
                    else:
                        color = COLORS['void']
                        alpha = 0.3

                    rect = Rectangle((cx, cy), 0.07, 0.07,
                                     facecolor=color, edgecolor='gray',
                                     linewidth=0.5, alpha=alpha)
                    ax.add_patch(rect)

        elif i == 4:
            # Planck scale: Clear discrete voxels with states
            grid_size = 6
            voxel_size = 0.12
            offset = 0.12

            for gx in range(grid_size):
                for gy in range(grid_size):
                    cx = offset + gx * (voxel_size + 0.02)
                    cy = offset + gy * (voxel_size + 0.02)

                    # Assign states based on position (for visual pattern)
                    state_val = np.sin(gx * 1.5) * np.cos(gy * 2)
                    if state_val > 0.4:
                        color = COLORS['matter']
                        state = '+1'
                    elif state_val < -0.4:
                        color = COLORS['antimatter']
                        state = '-1'
                    else:
                        color = COLORS['void']
                        state = '0'

                    rect = Rectangle((cx, cy), voxel_size, voxel_size,
                                     facecolor=color, edgecolor='black',
                                     linewidth=1.5)
                    ax.add_patch(rect)

                    # Add state label for some voxels
                    if gx % 2 == 0 and gy % 2 == 0:
                        ax.text(cx + voxel_size/2, cy + voxel_size/2,
                               state, fontsize=8, ha='center', va='center',
                               fontweight='bold', color='white' if state != '0' else 'black')

            # Add legend
            ax.text(0.5, 0.02, 'States: +1 (matter), 0 (void), -1 (antimatter)',
                   fontsize=7, ha='center', transform=ax.transAxes)

        # Title for each panel
        ax.set_title(f"{scale_info['label']}\n{scale_info['scale']}",
                    fontsize=10, fontweight='bold', pad=5)

        # Description at bottom
        ax.text(0.5, -0.05, scale_info['description'],
               fontsize=8, ha='center', transform=ax.transAxes,
               style='italic', color='gray')

        # Add zoom arrow between panels (except last)
        if i < 4:
            ax.annotate('', xy=(1.15, 0.5), xytext=(1.0, 0.5),
                       xycoords='axes fraction',
                       arrowprops=dict(arrowstyle='->', color=COLORS['highlight'],
                                      lw=2, mutation_scale=15))
            ax.text(1.08, 0.35, '×10⁶', fontsize=8, ha='center',
                   transform=ax.transAxes, color=COLORS['highlight'])

    # Main title
    fig.suptitle('From Continuous Appearance to Discrete Reality',
                fontsize=14, fontweight='bold', y=1.02)

    # Explanation box
    explanation = (
        "At macroscopic scales, the lattice appears continuous. "
        "Zoom in far enough, and discrete voxels become visible—each in one of three states."
    )
    fig.text(0.5, -0.05, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_zoom_planck()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_1_9_zoom_planck.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
