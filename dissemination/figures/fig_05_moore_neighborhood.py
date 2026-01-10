"""
Figure 1.5: Moore Neighborhood (3D)
===================================
Visualizes the 26-neighbor Moore neighborhood in 3D.

Shows a 3×3×3 cube grid with:
- Center voxel (gold, highlighted)
- 6 face neighbors (distance 1) - red
- 12 edge neighbors (distance √2) - blue
- 8 corner neighbors (distance √3) - green

Uses matplotlib 3D projection.
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, NEIGHBOR_COLORS


def draw_cube(ax, center, size, color, alpha=0.6, edgecolor='black'):
    """Draw a 3D cube at the specified center position."""
    x, y, z = center
    s = size / 2

    # Define the 8 vertices
    vertices = np.array([
        [x-s, y-s, z-s],
        [x+s, y-s, z-s],
        [x+s, y+s, z-s],
        [x-s, y+s, z-s],
        [x-s, y-s, z+s],
        [x+s, y-s, z+s],
        [x+s, y+s, z+s],
        [x-s, y+s, z+s]
    ])

    # Define the 6 faces
    faces = [
        [vertices[0], vertices[1], vertices[2], vertices[3]],  # Bottom
        [vertices[4], vertices[5], vertices[6], vertices[7]],  # Top
        [vertices[0], vertices[1], vertices[5], vertices[4]],  # Front
        [vertices[2], vertices[3], vertices[7], vertices[6]],  # Back
        [vertices[0], vertices[3], vertices[7], vertices[4]],  # Left
        [vertices[1], vertices[2], vertices[6], vertices[5]]   # Right
    ]

    # Create and add the cube
    cube = Poly3DCollection(faces, alpha=alpha, facecolor=color,
                            edgecolor=edgecolor, linewidth=0.5)
    ax.add_collection3d(cube)


def generate_moore_neighborhood():
    """
    Generate the 3D Moore neighborhood visualization.

    27 voxels arranged in a 3×3×3 grid with color-coded
    neighbor types based on distance from center.
    """
    fig = plt.figure(figsize=(12, 10), dpi=150)
    ax = fig.add_subplot(111, projection='3d')
    fig.patch.set_facecolor(COLORS['background'])

    cube_size = 0.8  # Size of each voxel cube
    gap = 0.1  # Gap between cubes

    # Draw all 27 cubes
    for dx in [-1, 0, 1]:
        for dy in [-1, 0, 1]:
            for dz in [-1, 0, 1]:
                center = (dx * (cube_size + gap),
                         dy * (cube_size + gap),
                         dz * (cube_size + gap))

                # Calculate distance squared to determine neighbor type
                dist_sq = dx**2 + dy**2 + dz**2

                if dist_sq == 0:
                    # Center voxel
                    color = NEIGHBOR_COLORS['center']
                    alpha = 1.0
                    label = 'center'
                elif dist_sq == 1:
                    # Face neighbor (6 total)
                    color = NEIGHBOR_COLORS['face']
                    alpha = 0.8
                    label = 'face'
                elif dist_sq == 2:
                    # Edge neighbor (12 total)
                    color = NEIGHBOR_COLORS['edge']
                    alpha = 0.6
                    label = 'edge'
                else:  # dist_sq == 3
                    # Corner neighbor (8 total)
                    color = NEIGHBOR_COLORS['corner']
                    alpha = 0.5
                    label = 'corner'

                draw_cube(ax, center, cube_size, color, alpha)

    # Set axis properties
    ax.set_xlim(-2, 2)
    ax.set_ylim(-2, 2)
    ax.set_zlim(-2, 2)

    ax.set_xlabel('X', fontsize=12)
    ax.set_ylabel('Y', fontsize=12)
    ax.set_zlabel('Z', fontsize=12)

    # Set viewing angle for better visualization
    ax.view_init(elev=25, azim=45)

    # Remove axis ticks for cleaner look
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_zticks([])

    # Title
    ax.set_title('Moore Neighborhood (26 Neighbors)', fontsize=16,
                fontweight='bold', pad=20)

    # Create legend
    legend_elements = [
        mpatches.Patch(facecolor=NEIGHBOR_COLORS['center'], edgecolor='black',
                       label='Center voxel'),
        mpatches.Patch(facecolor=NEIGHBOR_COLORS['face'], edgecolor='black',
                       label='Face neighbors (6) - d=1'),
        mpatches.Patch(facecolor=NEIGHBOR_COLORS['edge'], edgecolor='black',
                       label='Edge neighbors (12) - d=√2'),
        mpatches.Patch(facecolor=NEIGHBOR_COLORS['corner'], edgecolor='black',
                       label='Corner neighbors (8) - d=√3'),
    ]
    ax.legend(handles=legend_elements, loc='upper left',
              fontsize=10, bbox_to_anchor=(0, 1))

    # Add text annotations for counts
    info_text = (
        "Total: 26 neighbors\n"
        "6 face + 12 edge + 8 corner\n\n"
        "Causality constraint:\n"
        "Updates depend only on\n"
        "these 26 neighbors"
    )
    fig.text(0.85, 0.3, info_text, fontsize=10, ha='left', va='center',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    # Distance formula
    fig.text(0.85, 0.6, '$d = \\sqrt{\\Delta x^2 + \\Delta y^2 + \\Delta z^2}$',
             fontsize=11, ha='left', va='center')

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_moore_neighborhood()
    output_path = Path(__file__).parent.parent / 'ch00' / 'fig_1_5_moore_neighborhood.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
