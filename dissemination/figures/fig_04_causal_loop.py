"""
Figure 1.4: 13-Step Causal Loop
===============================
Circular flowchart showing the 13 steps of the universal tick.

Steps are arranged in a circle with arrows showing the cyclic flow:
1. TIME GATE → 2. DECAY → 3. EXISTENCE → 4. PROPAGATE → 5. SUPERPOSE →
6. FIELDS → 7. FORCES → 8. INTEGRATE → 9. MOVE → 10. COLLIDE →
11. TRANSMUTE → 12. BIND → 13. INCREMENT → (back to TIME GATE)

Color-coded by phase category:
- Temporal (orange): TIME GATE, INCREMENT
- Existence (blue): DECAY, EXISTENCE
- Propagation (green): PROPAGATE, SUPERPOSE, FIELDS
- Forces (purple): FORCES, INTEGRATE
- Motion (yellow): MOVE, COLLIDE, TRANSMUTE, BIND
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Circle, Wedge
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, PHASE_COLORS, apply_trd_style
from utils.physics_constants import CAUSAL_LOOP_STEPS


def generate_causal_loop():
    """
    Generate the 13-step causal loop circular flowchart.

    Steps arranged in a clock-like circle with arrows
    showing the cyclic progression.
    """
    fig, ax = plt.subplots(figsize=(12, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)
    ax.set_aspect('equal')
    ax.axis('off')

    n_steps = 13
    radius = 1.0  # Radius of the circle
    box_width = 0.28
    box_height = 0.15

    # Calculate positions for each step (starting at top, going clockwise)
    positions = []
    for i in range(n_steps):
        angle = np.pi/2 - 2 * np.pi * i / n_steps  # Start at top, go clockwise
        x = radius * np.cos(angle)
        y = radius * np.sin(angle)
        positions.append((x, y, angle))

    # Draw steps
    for i, step in enumerate(CAUSAL_LOOP_STEPS):
        x, y, angle = positions[i]
        phase_color = PHASE_COLORS[step['phase']]

        # Draw box
        box = FancyBboxPatch(
            (x - box_width/2, y - box_height/2),
            box_width, box_height,
            boxstyle="round,pad=0.02,rounding_size=0.02",
            facecolor=phase_color,
            edgecolor='black',
            linewidth=2,
            zorder=10
        )
        ax.add_patch(box)

        # Step number (small circle)
        num_x = x - box_width/2 - 0.06
        num_y = y + box_height/2 - 0.03
        num_circle = Circle((num_x, num_y), 0.04, facecolor='white',
                            edgecolor='black', linewidth=1, zorder=11)
        ax.add_patch(num_circle)
        ax.text(num_x, num_y, str(i+1), fontsize=7, ha='center', va='center',
                fontweight='bold', zorder=12)

        # Step name
        ax.text(x, y, step['name'], fontsize=8, ha='center', va='center',
                fontweight='bold', zorder=11)

        # Draw arrow to next step
        next_i = (i + 1) % n_steps
        next_x, next_y, _ = positions[next_i]

        # Calculate arrow start and end points (on box edges)
        dx = next_x - x
        dy = next_y - y
        dist = np.sqrt(dx**2 + dy**2)

        # Offset start point from current box edge
        start_offset = 0.17
        start_x = x + start_offset * dx / dist
        start_y = y + start_offset * dy / dist

        # Offset end point from next box edge
        end_offset = 0.17
        end_x = next_x - end_offset * dx / dist
        end_y = next_y - end_offset * dy / dist

        # Draw curved arrow
        rad = 0.15  # Curvature
        arrow = FancyArrowPatch(
            (start_x, start_y), (end_x, end_y),
            connectionstyle=f"arc3,rad={rad}",
            arrowstyle="->,head_length=6,head_width=4",
            color='#555555',
            linewidth=1.5,
            zorder=5
        )
        ax.add_patch(arrow)

    # Central circle with title
    center_circle = Circle((0, 0), 0.35, facecolor='white',
                           edgecolor='black', linewidth=2, zorder=3)
    ax.add_patch(center_circle)
    ax.text(0, 0.08, 'TICK', fontsize=14, ha='center', va='center', fontweight='bold')
    ax.text(0, -0.08, 't → t+1', fontsize=11, ha='center', va='center',
            style='italic', color='gray')

    # Title
    ax.set_title('The 13-Step Causal Loop\n(One Universal Tick)',
                fontsize=16, fontweight='bold', pad=20)

    # Legend for phases
    legend_elements = [
        mpatches.Patch(facecolor=PHASE_COLORS['temporal'], edgecolor='black',
                       label='Temporal'),
        mpatches.Patch(facecolor=PHASE_COLORS['existence'], edgecolor='black',
                       label='Existence'),
        mpatches.Patch(facecolor=PHASE_COLORS['propagation'], edgecolor='black',
                       label='Propagation'),
        mpatches.Patch(facecolor=PHASE_COLORS['forces'], edgecolor='black',
                       label='Forces'),
        mpatches.Patch(facecolor=PHASE_COLORS['motion'], edgecolor='black',
                       label='Motion'),
    ]
    ax.legend(handles=legend_elements, loc='lower right',
              fontsize=9, title='Phase Categories', title_fontsize=10,
              bbox_to_anchor=(1.35, 0))

    # Step descriptions (outer ring)
    desc_radius = 1.35
    for i, step in enumerate(CAUSAL_LOOP_STEPS):
        x, y, angle = positions[i]
        desc_x = desc_radius * np.cos(np.pi/2 - 2 * np.pi * i / n_steps)
        desc_y = desc_radius * np.sin(np.pi/2 - 2 * np.pi * i / n_steps)

        # Rotate text to be readable
        if -np.pi/2 < angle < np.pi/2:
            rotation = np.degrees(angle)
            ha = 'left'
        else:
            rotation = np.degrees(angle) + 180
            ha = 'right'

        ax.text(desc_x, desc_y, step['description'], fontsize=7,
                ha=ha, va='center', rotation=rotation,
                color='gray', style='italic')

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_causal_loop()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_1_4_causal_loop.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
