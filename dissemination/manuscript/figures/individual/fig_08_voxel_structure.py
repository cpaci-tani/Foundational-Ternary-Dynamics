"""
Figure 1.8: Voxel Data Structure
================================
Visualizes the internal data structure of a TRD voxel.

Shows a 3D cube with labeled sections for each data category:
- Identity: pos, uuid, partner_uuid
- State: state, charge, shell_n
- Energy: flux, density, frequency
- Mechanics: force_accumulator, position_remainder, wave_velocity
- Temporal: phase
- Stability: is_locked
"""

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, VOXEL_CATEGORY_COLORS
from utils.physics_constants import VOXEL_STRUCTURE


def generate_voxel_structure():
    """
    Generate the voxel data structure visualization.

    Shows a 3D exploded view of voxel with labeled data fields.
    """
    fig = plt.figure(figsize=(14, 10), dpi=150)

    # Create main 3D axis for the cube
    ax3d = fig.add_subplot(121, projection='3d')

    # Create 2D axis for the data table
    ax2d = fig.add_subplot(122)

    fig.patch.set_facecolor(COLORS['background'])

    # =========================================================================
    # 3D Cube Visualization
    # =========================================================================

    # Draw a simple cube to represent the voxel
    cube_size = 1.0

    # Vertices of the cube
    vertices = np.array([
        [0, 0, 0], [1, 0, 0], [1, 1, 0], [0, 1, 0],  # Bottom face
        [0, 0, 1], [1, 0, 1], [1, 1, 1], [0, 1, 1]   # Top face
    ]) - 0.5  # Center at origin

    # Faces with different colors for each category
    faces_data = [
        ([0, 1, 2, 3], 'identity', 'Bottom'),      # Bottom
        ([4, 5, 6, 7], 'state', 'Top'),            # Top
        ([0, 1, 5, 4], 'energy', 'Front'),         # Front
        ([2, 3, 7, 6], 'mechanics', 'Back'),       # Back
        ([0, 3, 7, 4], 'temporal', 'Left'),        # Left
        ([1, 2, 6, 5], 'stability', 'Right'),      # Right
    ]

    for indices, category, face_name in faces_data:
        face = [vertices[i] for i in indices]
        color = VOXEL_CATEGORY_COLORS[category]

        poly = Poly3DCollection([face], alpha=0.7, facecolor=color,
                               edgecolor='black', linewidth=2)
        ax3d.add_collection3d(poly)

        # Add label to face center
        center = np.mean([vertices[i] for i in indices], axis=0)
        # Offset label outward
        offset = center * 1.3
        ax3d.text(offset[0], offset[1], offset[2],
                 category.upper(), fontsize=9, fontweight='bold',
                 ha='center', va='center', color='black')

    # Set 3D axis properties
    ax3d.set_xlim(-1.5, 1.5)
    ax3d.set_ylim(-1.5, 1.5)
    ax3d.set_zlim(-1.5, 1.5)
    ax3d.set_xlabel('X')
    ax3d.set_ylabel('Y')
    ax3d.set_zlabel('Z')
    ax3d.view_init(elev=25, azim=45)
    ax3d.set_xticks([])
    ax3d.set_yticks([])
    ax3d.set_zticks([])
    ax3d.set_title('Voxel Structure\n(3D View)', fontsize=14, fontweight='bold')

    # =========================================================================
    # 2D Data Table
    # =========================================================================
    ax2d.axis('off')
    ax2d.set_xlim(0, 1)
    ax2d.set_ylim(0, 1)

    ax2d.set_title('Voxel Data Fields', fontsize=14, fontweight='bold', pad=10)

    # Create table layout
    y_start = 0.95
    row_height = 0.13
    box_height = 0.11

    for i, (category, data) in enumerate(VOXEL_STRUCTURE.items()):
        y = y_start - i * row_height

        # Category header box
        header_box = mpatches.FancyBboxPatch(
            (0.02, y - box_height/2), 0.25, box_height,
            boxstyle="round,pad=0.01",
            facecolor=data['color'],
            edgecolor='black',
            linewidth=2
        )
        ax2d.add_patch(header_box)
        ax2d.text(0.15, y, category.upper(), fontsize=10, ha='center', va='center',
                 fontweight='bold')

        # Fields
        fields_text = ', '.join(data['fields'])
        ax2d.text(0.3, y, fields_text, fontsize=9, ha='left', va='center',
                 style='italic')

    # Add summary box
    summary_y = 0.08
    summary_box = mpatches.FancyBboxPatch(
        (0.02, summary_y - 0.05), 0.96, 0.1,
        boxstyle="round,pad=0.02",
        facecolor='white',
        edgecolor='gray',
        linewidth=1
    )
    ax2d.add_patch(summary_box)
    ax2d.text(0.5, summary_y, 'Each voxel: ~50 bytes in sparse representation',
             fontsize=10, ha='center', va='center', style='italic', color='gray')

    # Legend explanation
    legend_text = (
        "State ∈ {-1, 0, +1}\n"
        "Flux ∈ ℝ³\n"
        "is_locked ∈ {True, False}"
    )
    ax2d.text(0.85, 0.5, legend_text, fontsize=9, ha='center', va='center',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='#f0f0f0', edgecolor='gray'))

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_voxel_structure()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_1_8_voxel_structure.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
