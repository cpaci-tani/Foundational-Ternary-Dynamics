"""
Figure 2.16: Phase Transitions
==============================
Visualizes phase transitions (solid/liquid/gas) in TRD as
changes in the flux field ordering and particle mobility.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_phase_transitions():
    """
    Generate the phase transitions visualization.

    Shows:
    1. Three panels: Solid, Liquid, Gas arrangements
    2. Phase diagram
    """
    fig = plt.figure(figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    gs = fig.add_gridspec(2, 3, height_ratios=[1.2, 1], hspace=0.3, wspace=0.2)

    ax_solid = fig.add_subplot(gs[0, 0])
    ax_liquid = fig.add_subplot(gs[0, 1])
    ax_gas = fig.add_subplot(gs[0, 2])
    ax_diagram = fig.add_subplot(gs[1, :])

    np.random.seed(42)

    # =========================================================================
    # Solid Phase
    # =========================================================================
    ax_solid.set_xlim(-0.5, 4.5)
    ax_solid.set_ylim(-0.5, 4.5)
    ax_solid.set_aspect('equal')
    ax_solid.set_facecolor('#e8f4f8')

    # Regular lattice positions with small vibrations
    for i in range(5):
        for j in range(5):
            # Small random displacement (vibration)
            dx = np.random.normal(0, 0.05)
            dy = np.random.normal(0, 0.05)

            atom = Circle((i + dx, j + dy), 0.2, facecolor=COLORS['highlight'],
                         edgecolor='black', linewidth=1)
            ax_solid.add_patch(atom)

            # Draw bonds to neighbors
            if i < 4:
                ax_solid.plot([i + dx, i + 1], [j + dy, j], 'k-',
                             linewidth=1, alpha=0.5)
            if j < 4:
                ax_solid.plot([i + dx, i], [j + dy, j + 1], 'k-',
                             linewidth=1, alpha=0.5)

    ax_solid.set_title('SOLID\n(Fixed positions, ordered)',
                       fontsize=12, fontweight='bold')
    ax_solid.set_xlabel('Flux fields locked')
    ax_solid.axis('off')

    # Temperature indicator
    ax_solid.text(2, -0.3, 'Low T', fontsize=10, ha='center',
                  color=COLORS['antimatter'], fontweight='bold')

    # =========================================================================
    # Liquid Phase
    # =========================================================================
    ax_liquid.set_xlim(-0.5, 4.5)
    ax_liquid.set_ylim(-0.5, 4.5)
    ax_liquid.set_aspect('equal')
    ax_liquid.set_facecolor('#fff8e8')

    # Random but dense positions
    n_atoms = 25
    positions = []

    # Generate positions with some clustering but not ordered
    for _ in range(n_atoms):
        while True:
            x = np.random.uniform(0.3, 3.7)
            y = np.random.uniform(0.3, 3.7)
            # Check not too close to existing
            valid = True
            for px, py in positions:
                if np.sqrt((x - px)**2 + (y - py)**2) < 0.5:
                    valid = False
                    break
            if valid:
                positions.append((x, y))
                break

    for x, y in positions:
        atom = Circle((x, y), 0.18, facecolor=COLORS['highlight'],
                      edgecolor='black', linewidth=1)
        ax_liquid.add_patch(atom)

        # Draw some temporary bonds (nearby atoms)
        for x2, y2 in positions:
            dist = np.sqrt((x - x2)**2 + (y - y2)**2)
            if 0 < dist < 0.8:
                ax_liquid.plot([x, x2], [y, y2], 'k-',
                              linewidth=0.5, alpha=0.3)

    ax_liquid.set_title('LIQUID\n(Mobile, short-range order)',
                        fontsize=12, fontweight='bold')
    ax_liquid.set_xlabel('Flux fields dynamic')
    ax_liquid.axis('off')

    ax_liquid.text(2, -0.3, 'Medium T', fontsize=10, ha='center',
                   color=COLORS['highlight'], fontweight='bold')

    # =========================================================================
    # Gas Phase
    # =========================================================================
    ax_gas.set_xlim(-0.5, 4.5)
    ax_gas.set_ylim(-0.5, 4.5)
    ax_gas.set_aspect('equal')
    ax_gas.set_facecolor('#ffe8e8')

    # Sparse, random positions with velocity arrows
    n_gas = 12
    for _ in range(n_gas):
        x = np.random.uniform(0.5, 4)
        y = np.random.uniform(0.5, 4)

        atom = Circle((x, y), 0.15, facecolor=COLORS['highlight'],
                      edgecolor='black', linewidth=1)
        ax_gas.add_patch(atom)

        # Velocity arrow
        vx = np.random.normal(0, 0.4)
        vy = np.random.normal(0, 0.4)
        ax_gas.arrow(x, y, vx, vy, head_width=0.1, head_length=0.05,
                    fc='black', ec='black', alpha=0.5)

    ax_gas.set_title('GAS\n(Free motion, no order)',
                     fontsize=12, fontweight='bold')
    ax_gas.set_xlabel('Flux fields decoupled')
    ax_gas.axis('off')

    ax_gas.text(2, -0.3, 'High T', fontsize=10, ha='center',
                color=COLORS['matter'], fontweight='bold')

    # =========================================================================
    # Phase Diagram
    # =========================================================================
    # Simplified phase diagram
    T = np.linspace(0, 10, 100)

    # Phase boundaries (simplified)
    # Solid-Liquid boundary (nearly vertical, slight slope)
    T_sl = np.linspace(0, 6, 50)
    P_sl = 2 + 0.1 * T_sl

    # Liquid-Gas boundary (curved, ending at critical point)
    T_lg = np.linspace(2, 8, 50)
    P_lg = 2 + 1.5 * (T_lg - 2) - 0.1 * (T_lg - 2)**2

    # Solid-Gas boundary (sublimation)
    T_sg = np.linspace(0, 2, 20)
    P_sg = 2 * np.exp(-0.5 * (2 - T_sg))

    ax_diagram.plot(T_sl, P_sl, 'k-', linewidth=2, label='Solid-Liquid')
    ax_diagram.plot(T_lg, P_lg, 'k-', linewidth=2, label='Liquid-Gas')
    ax_diagram.plot(T_sg, P_sg, 'k-', linewidth=2, label='Solid-Gas')

    # Mark triple point
    ax_diagram.plot(2, 2, 'ko', markersize=10)
    ax_diagram.annotate('Triple Point', xy=(2, 2), xytext=(0.5, 3),
                        fontsize=10, arrowprops=dict(arrowstyle='->'))

    # Mark critical point
    ax_diagram.plot(8, 8, 'ko', markersize=10)
    ax_diagram.annotate('Critical Point', xy=(8, 8), xytext=(6, 9),
                        fontsize=10, arrowprops=dict(arrowstyle='->'))

    # Label regions
    ax_diagram.text(1, 6, 'SOLID', fontsize=14, fontweight='bold',
                    color=COLORS['antimatter'])
    ax_diagram.text(5, 5, 'LIQUID', fontsize=14, fontweight='bold',
                    color=COLORS['highlight'])
    ax_diagram.text(6, 1, 'GAS', fontsize=14, fontweight='bold',
                    color=COLORS['matter'])

    ax_diagram.set_xlabel('Temperature (T)', fontsize=12)
    ax_diagram.set_ylabel('Pressure (P)', fontsize=12)
    ax_diagram.set_title('Phase Diagram', fontsize=13, fontweight='bold')
    ax_diagram.set_xlim(0, 10)
    ax_diagram.set_ylim(0, 10)
    ax_diagram.grid(True, alpha=0.3)

    # TRD interpretation
    ax_diagram.text(8, 3, 'TRD: Phase boundaries\noccur where flux field\nordering changes',
                    fontsize=9, ha='center',
                    bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Phase Transitions: Flux Field Ordering Changes with Temperature',
                fontsize=16, fontweight='bold', y=0.98)

    explanation = (
        "In TRD, phases differ by the degree of flux field locking between particles.\n"
        "Solid: locked flux (ordered) | Liquid: dynamic flux (local order) | Gas: decoupled flux (disorder)"
    )
    fig.text(0.5, 0.01, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_phase_transitions()
    output_path = Path(__file__).parent.parent / 'ch04' / 'fig_2_16_phase_transitions.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
