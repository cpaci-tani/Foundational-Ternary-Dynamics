"""
Figure 1.3: Three-State Transitions
====================================
Triangle diagram showing the three voxel states and their transitions.

States:
- Void (0): Unmanifested substrate
- Matter (+1): Positive manifestation
- Antimatter (-1): Negative manifestation

Transitions:
- Genesis: 0 → +1 or 0 → -1
- Evaporation: +1 → 0 or -1 → 0
- Annihilation: +1 + (-1) → 0 + 0
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, FancyBboxPatch
from matplotlib.patches import Arc
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def draw_curved_arrow(ax, start, end, color, label=None, rad=0.3, label_offset=(0, 0)):
    """Draw a curved arrow between two points."""
    arrow = FancyArrowPatch(
        start, end,
        connectionstyle=f"arc3,rad={rad}",
        arrowstyle="->,head_length=10,head_width=6",
        color=color,
        linewidth=2,
        zorder=3
    )
    ax.add_patch(arrow)

    if label:
        # Calculate midpoint of arc for label placement
        mid = ((start[0] + end[0])/2 + label_offset[0],
               (start[1] + end[1])/2 + label_offset[1])
        ax.text(mid[0], mid[1], label, fontsize=10, ha='center', va='center',
                color=color, fontweight='bold',
                bbox=dict(boxstyle='round,pad=0.2', facecolor='white', alpha=0.8))


def generate_three_states():
    """
    Generate the three-state transition diagram.

    Equilateral triangle with:
    - Void at top
    - Matter at bottom-left
    - Antimatter at bottom-right
    - Transition arrows between states
    """
    fig, ax = plt.subplots(figsize=(10, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.2, 1.4)
    ax.set_aspect('equal')
    ax.axis('off')

    # Triangle vertices (equilateral)
    void_pos = (0, 0.9)
    matter_pos = (-0.85, -0.5)
    antimatter_pos = (0.85, -0.5)

    # Node radius
    node_radius = 0.22

    # =========================================================================
    # Draw state nodes
    # =========================================================================

    # Void node (gray)
    void_circle = Circle(void_pos, node_radius, facecolor=COLORS['void'],
                         edgecolor='black', linewidth=3, zorder=5)
    ax.add_patch(void_circle)
    ax.text(void_pos[0], void_pos[1], '0', fontsize=24, ha='center', va='center',
            fontweight='bold', color='white', zorder=6)
    ax.text(void_pos[0], void_pos[1] + 0.35, 'VOID', fontsize=14, ha='center',
            fontweight='bold', color=COLORS['void'])
    ax.text(void_pos[0], void_pos[1] + 0.5, 'Unmanifested', fontsize=10,
            ha='center', style='italic', color='gray')

    # Matter node (red)
    matter_circle = Circle(matter_pos, node_radius, facecolor=COLORS['matter'],
                           edgecolor='black', linewidth=3, zorder=5)
    ax.add_patch(matter_circle)
    ax.text(matter_pos[0], matter_pos[1], '+1', fontsize=24, ha='center', va='center',
            fontweight='bold', color='white', zorder=6)
    ax.text(matter_pos[0], matter_pos[1] - 0.35, 'MATTER', fontsize=14, ha='center',
            fontweight='bold', color=COLORS['matter'])
    ax.text(matter_pos[0], matter_pos[1] - 0.5, 'Positive', fontsize=10,
            ha='center', style='italic', color='gray')

    # Antimatter node (blue)
    antimatter_circle = Circle(antimatter_pos, node_radius, facecolor=COLORS['antimatter'],
                               edgecolor='black', linewidth=3, zorder=5)
    ax.add_patch(antimatter_circle)
    ax.text(antimatter_pos[0], antimatter_pos[1], '-1', fontsize=24, ha='center', va='center',
            fontweight='bold', color='white', zorder=6)
    ax.text(antimatter_pos[0], antimatter_pos[1] - 0.35, 'ANTIMATTER', fontsize=14, ha='center',
            fontweight='bold', color=COLORS['antimatter'])
    ax.text(antimatter_pos[0], antimatter_pos[1] - 0.5, 'Negative', fontsize=10,
            ha='center', style='italic', color='gray')

    # =========================================================================
    # Draw transition arrows
    # =========================================================================

    # Offset arrows from node centers
    arrow_offset = 0.25

    # Genesis: Void → Matter (downward left)
    start = (void_pos[0] - 0.15, void_pos[1] - node_radius - 0.05)
    end = (matter_pos[0] + 0.1, matter_pos[1] + node_radius + 0.05)
    draw_curved_arrow(ax, start, end, COLORS['accent1'],
                     label='Genesis\n(0 → +1)', rad=0.2, label_offset=(-0.25, 0.1))

    # Genesis: Void → Antimatter (downward right)
    start = (void_pos[0] + 0.15, void_pos[1] - node_radius - 0.05)
    end = (antimatter_pos[0] - 0.1, antimatter_pos[1] + node_radius + 0.05)
    draw_curved_arrow(ax, start, end, COLORS['accent1'],
                     label='Genesis\n(0 → -1)', rad=-0.2, label_offset=(0.25, 0.1))

    # Evaporation: Matter → Void (upward)
    start = (matter_pos[0] + 0.15, matter_pos[1] + node_radius + 0.05)
    end = (void_pos[0] - 0.1, void_pos[1] - node_radius - 0.05)
    draw_curved_arrow(ax, start, end, COLORS['accent2'],
                     label='Evaporation\n(+1 → 0)', rad=0.2, label_offset=(-0.35, -0.05))

    # Evaporation: Antimatter → Void (upward)
    start = (antimatter_pos[0] - 0.15, antimatter_pos[1] + node_radius + 0.05)
    end = (void_pos[0] + 0.1, void_pos[1] - node_radius - 0.05)
    draw_curved_arrow(ax, start, end, COLORS['accent2'],
                     label='Evaporation\n(-1 → 0)', rad=-0.2, label_offset=(0.35, -0.05))

    # Annihilation: Matter ↔ Antimatter (bottom, bidirectional)
    # Draw double-headed arrow for mutual annihilation
    start = (matter_pos[0] + node_radius + 0.05, matter_pos[1])
    end = (antimatter_pos[0] - node_radius - 0.05, antimatter_pos[1])

    # Draw line with arrows on both ends
    ax.annotate('', xy=end, xytext=start,
               arrowprops=dict(arrowstyle='<->', color=COLORS['highlight'],
                              lw=3, mutation_scale=15))

    ax.text(0, matter_pos[1] - 0.15, 'Annihilation\n(+1) + (-1) → 0 + 0',
           fontsize=11, ha='center', va='top', fontweight='bold',
           color=COLORS['highlight'],
           bbox=dict(boxstyle='round,pad=0.3', facecolor='#fff8dc', edgecolor=COLORS['highlight']))

    # =========================================================================
    # Add title and explanation
    # =========================================================================

    ax.set_title('Ternary State Transitions', fontsize=18, fontweight='bold', pad=20)

    # Transition rules box
    rules_text = (
        "Transition Rules:\n\n"
        "• Genesis: Void → ±1 when |J| > KB\n"
        "• Evaporation: ±1 → Void when |J| < KB\n"
        "• Annihilation: +1 adjacent to -1 → both become 0\n\n"
        "Note: Direct +1 ↔ -1 transitions are forbidden"
    )
    ax.text(0, -1.0, rules_text, fontsize=10, ha='center', va='top',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     edgecolor='gray', alpha=0.95))

    # Legend
    legend_elements = [
        mpatches.Patch(facecolor=COLORS['accent1'], label='Genesis'),
        mpatches.Patch(facecolor=COLORS['accent2'], label='Evaporation'),
        mpatches.Patch(facecolor=COLORS['highlight'], label='Annihilation'),
    ]
    ax.legend(handles=legend_elements, loc='upper right', fontsize=10)

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_three_states()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_1_3_three_state_transitions.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
