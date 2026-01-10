"""
Figure 3.4: Update Order Effects
================================
Shows how changing the order of update phases can
affect simulation outcomes.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Circle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_update_order():
    """Generate the update order effects visualization."""
    fig, axes = plt.subplots(1, 2, figsize=(14, 6), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Left: Standard order
    ax = axes[0]
    ax.set_xlim(0, 6)
    ax.set_ylim(0, 8)
    ax.axis('off')
    ax.set_title('Standard Order\n(Forces before Movement)', fontsize=12,
                fontweight='bold')

    steps_standard = [
        ('1. Compute Forces', '#4ECDC4'),
        ('2. Update Velocity', '#FFE66D'),
        ('3. Move Particles', '#FF6B6B'),
    ]

    for i, (step, color) in enumerate(steps_standard):
        y = 6 - i * 2
        box = FancyBboxPatch((0.5, y - 0.5), 5, 1.2,
                            boxstyle="round,pad=0.02",
                            facecolor=color, edgecolor='black', linewidth=2)
        ax.add_patch(box)
        ax.text(3, y, step, fontsize=11, ha='center', va='center', fontweight='bold')
        if i < len(steps_standard) - 1:
            ax.annotate('', xy=(3, y - 0.8), xytext=(3, y - 0.6),
                       arrowprops=dict(arrowstyle='->', color='black', lw=2))

    ax.text(3, 1, 'Result: Stable orbits,\ncorrect energy conservation',
            fontsize=10, ha='center', color='green',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # Right: Altered order
    ax = axes[1]
    ax.set_xlim(0, 6)
    ax.set_ylim(0, 8)
    ax.axis('off')
    ax.set_title('Altered Order\n(Movement before Forces)', fontsize=12,
                fontweight='bold')

    steps_altered = [
        ('1. Move Particles', '#FF6B6B'),
        ('2. Compute Forces', '#4ECDC4'),
        ('3. Update Velocity', '#FFE66D'),
    ]

    for i, (step, color) in enumerate(steps_altered):
        y = 6 - i * 2
        box = FancyBboxPatch((0.5, y - 0.5), 5, 1.2,
                            boxstyle="round,pad=0.02",
                            facecolor=color, edgecolor='black', linewidth=2)
        ax.add_patch(box)
        ax.text(3, y, step, fontsize=11, ha='center', va='center', fontweight='bold')
        if i < len(steps_altered) - 1:
            ax.annotate('', xy=(3, y - 0.8), xytext=(3, y - 0.6),
                       arrowprops=dict(arrowstyle='->', color='black', lw=2))

    ax.text(3, 1, 'Result: Energy drift,\nunstable dynamics',
            fontsize=10, ha='center', color='red',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    fig.suptitle('Update Order Matters: Same Rules, Different Sequence = Different Physics',
                fontsize=14, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_update_order()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_3_4_update_order.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
