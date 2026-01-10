"""
Figure 2.7: Quantum Foam
========================
Visualizes virtual pair production at the Planck scale.

In TRD, the void substrate is never truly "empty" - flux fluctuations
constantly produce transient +1/-1 pairs that annihilate before
propagating macroscopically.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch
from matplotlib.collections import PatchCollection
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_quantum_foam():
    """
    Generate the quantum foam visualization.

    Shows:
    1. Left: Snapshot of virtual pair bubbles
    2. Right: Lifecycle of a virtual pair
    """
    fig, (ax_foam, ax_lifecycle) = plt.subplots(1, 2, figsize=(14, 7), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # =========================================================================
    # Left: Quantum foam snapshot
    # =========================================================================
    ax_foam.set_xlim(-5, 5)
    ax_foam.set_ylim(-5, 5)
    ax_foam.set_aspect('equal')
    ax_foam.set_facecolor('#f8f8f8')

    # Draw many virtual pair "bubbles" at random positions
    np.random.seed(42)
    n_pairs = 30

    for _ in range(n_pairs):
        # Random position
        x = np.random.uniform(-4.5, 4.5)
        y = np.random.uniform(-4.5, 4.5)

        # Random size (lifetime proxy)
        size = np.random.uniform(0.1, 0.5)
        alpha = 0.3 + 0.4 * (1 - size / 0.5)  # Smaller = more solid (newer)

        # Draw the pair as two touching circles
        offset = size * 0.6
        angle = np.random.uniform(0, 2 * np.pi)
        dx, dy = offset * np.cos(angle), offset * np.sin(angle)

        # Positive particle
        circle_pos = Circle((x + dx, y + dy), size * 0.4,
                           facecolor=COLORS['matter'], alpha=alpha,
                           edgecolor='black', linewidth=0.5)
        ax_foam.add_patch(circle_pos)

        # Negative particle
        circle_neg = Circle((x - dx, y - dy), size * 0.4,
                           facecolor=COLORS['antimatter'], alpha=alpha,
                           edgecolor='black', linewidth=0.5)
        ax_foam.add_patch(circle_neg)

    # Add some "annihilation flashes"
    for _ in range(8):
        x = np.random.uniform(-4, 4)
        y = np.random.uniform(-4, 4)
        # Starburst pattern
        for angle in np.linspace(0, 2*np.pi, 8, endpoint=False):
            dx = 0.3 * np.cos(angle)
            dy = 0.3 * np.sin(angle)
            ax_foam.plot([x, x+dx], [y, y+dy], color=COLORS['highlight'],
                        linewidth=1.5, alpha=0.7)

    ax_foam.set_title('Quantum Foam Snapshot\n(Virtual Pairs at Planck Scale)',
                      fontsize=14, fontweight='bold')
    ax_foam.set_xlabel('x (Planck units)')
    ax_foam.set_ylabel('y (Planck units)')

    # Grid for scale reference
    ax_foam.grid(True, alpha=0.2, linestyle='--')

    # Legend
    legend_elements = [
        mpatches.Patch(facecolor=COLORS['matter'], edgecolor='black',
                      label='+1 (virtual matter)'),
        mpatches.Patch(facecolor=COLORS['antimatter'], edgecolor='black',
                      label='-1 (virtual antimatter)'),
        plt.Line2D([0], [0], color=COLORS['highlight'], linewidth=2,
                   label='Annihilation flash'),
    ]
    ax_foam.legend(handles=legend_elements, loc='upper right', fontsize=9)

    # =========================================================================
    # Right: Virtual pair lifecycle
    # =========================================================================
    ax_lifecycle.set_xlim(0, 10)
    ax_lifecycle.set_ylim(0, 8)
    ax_lifecycle.set_aspect('equal')
    ax_lifecycle.axis('off')

    # Stage 1: Vacuum fluctuation
    y1 = 6.5
    ax_lifecycle.text(1, y1, '1. Flux Fluctuation', fontsize=11, fontweight='bold')
    # Wavy line representing flux
    t = np.linspace(0, 2, 100)
    wave_y = y1 - 0.8 + 0.15 * np.sin(8 * t)
    ax_lifecycle.plot(t + 3, wave_y, color=COLORS['highlight'], linewidth=2)
    ax_lifecycle.text(6, y1 - 0.8, 'density > KB', fontsize=9, style='italic')

    # Stage 2: Pair creation
    y2 = 4.5
    ax_lifecycle.text(1, y2, '2. Genesis (Pair)', fontsize=11, fontweight='bold')
    # Two circles appearing
    circle_p = Circle((3.5, y2 - 0.7), 0.3, facecolor=COLORS['matter'],
                      edgecolor='black', linewidth=2)
    circle_n = Circle((4.5, y2 - 0.7), 0.3, facecolor=COLORS['antimatter'],
                      edgecolor='black', linewidth=2)
    ax_lifecycle.add_patch(circle_p)
    ax_lifecycle.add_patch(circle_n)
    ax_lifecycle.text(3.5, y2 - 0.7, '+', fontsize=12, ha='center', va='center',
                      color='white', fontweight='bold')
    ax_lifecycle.text(4.5, y2 - 0.7, '-', fontsize=12, ha='center', va='center',
                      color='white', fontweight='bold')
    ax_lifecycle.text(6, y2 - 0.7, 'lifetime ~ h/E', fontsize=9, style='italic')

    # Stage 3: Brief separation
    y3 = 2.5
    ax_lifecycle.text(1, y3, '3. Brief Separation', fontsize=11, fontweight='bold')
    circle_p2 = Circle((3.2, y3 - 0.7), 0.3, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=2, alpha=0.7)
    circle_n2 = Circle((4.8, y3 - 0.7), 0.3, facecolor=COLORS['antimatter'],
                       edgecolor='black', linewidth=2, alpha=0.7)
    ax_lifecycle.add_patch(circle_p2)
    ax_lifecycle.add_patch(circle_n2)
    # Attraction arrow
    ax_lifecycle.annotate('', xy=(4.5, y3 - 0.7), xytext=(3.5, y3 - 0.7),
                         arrowprops=dict(arrowstyle='<->', color='gray', lw=2))
    ax_lifecycle.text(6, y3 - 0.7, 'Coulomb attraction', fontsize=9, style='italic')

    # Stage 4: Annihilation
    y4 = 0.5
    ax_lifecycle.text(1, y4, '4. Annihilation', fontsize=11, fontweight='bold')
    # Starburst
    cx, cy = 4, y4 - 0.5
    for angle in np.linspace(0, 2*np.pi, 12, endpoint=False):
        dx = 0.5 * np.cos(angle)
        dy = 0.5 * np.sin(angle)
        ax_lifecycle.plot([cx, cx+dx], [cy, cy+dy], color=COLORS['highlight'],
                         linewidth=2.5)
    ax_lifecycle.text(6, y4 - 0.5, 'flux returns to void', fontsize=9, style='italic')

    ax_lifecycle.set_title('Virtual Pair Lifecycle', fontsize=14, fontweight='bold')

    # =========================================================================
    # Bottom explanation
    # =========================================================================
    explanation = (
        "Virtual pairs constantly appear and annihilate in the void substrate.\n"
        "Their brief existence creates the 'quantum foam' texture at Planck scale.\n"
        "Energy-time uncertainty: pairs with energy E exist for time ~ h/E."
    )
    fig.text(0.5, 0.02, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    fig.suptitle('Quantum Foam: Virtual Pair Production',
                fontsize=16, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0.08, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_quantum_foam()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_2_7_quantum_foam.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
