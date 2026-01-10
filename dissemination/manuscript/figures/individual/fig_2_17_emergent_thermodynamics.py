"""
Figure 2.17: Emergent Thermodynamics
====================================
Visualizes how thermodynamic behavior emerges from TRD dynamics:
entropy, temperature, and the arrow of time.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Rectangle, FancyArrowPatch
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_emergent_thermodynamics():
    """
    Generate the emergent thermodynamics visualization.

    Shows:
    1. Top: Low entropy to high entropy evolution
    2. Middle: Temperature as average kinetic flux
    3. Bottom: Second law emergence
    """
    fig = plt.figure(figsize=(14, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    gs = fig.add_gridspec(3, 3, height_ratios=[1, 1, 1.2], hspace=0.35, wspace=0.25)

    ax_low_s = fig.add_subplot(gs[0, 0])
    ax_mid_s = fig.add_subplot(gs[0, 1])
    ax_high_s = fig.add_subplot(gs[0, 2])
    ax_temp = fig.add_subplot(gs[1, :])
    ax_arrow = fig.add_subplot(gs[2, :])

    np.random.seed(42)

    # =========================================================================
    # Top Row: Entropy Evolution
    # =========================================================================
    for ax, title, disorder, t in [
        (ax_low_s, 'Low Entropy\n(Ordered)', 0.05, 't = 0'),
        (ax_mid_s, 'Medium Entropy\n(Spreading)', 0.2, 't = 100'),
        (ax_high_s, 'High Entropy\n(Equilibrium)', 0.5, 't = 1000'),
    ]:
        ax.set_xlim(-0.5, 4.5)
        ax.set_ylim(-0.5, 4.5)
        ax.set_aspect('equal')
        ax.axis('off')

        # Draw box
        box = Rectangle((0, 0), 4, 4, facecolor='white',
                        edgecolor='black', linewidth=2)
        ax.add_patch(box)

        # Draw particles
        n_particles = 20

        if disorder < 0.1:
            # Clustered in corner
            for i in range(n_particles):
                x = 0.5 + np.random.uniform(0, 1)
                y = 0.5 + np.random.uniform(0, 1)
                p = Circle((x, y), 0.12, facecolor=COLORS['matter'],
                          edgecolor='black', linewidth=0.5)
                ax.add_patch(p)
        elif disorder < 0.3:
            # Spreading
            for i in range(n_particles):
                x = 0.5 + np.random.uniform(0, 2)
                y = 0.5 + np.random.uniform(0, 2)
                p = Circle((x, y), 0.12, facecolor=COLORS['matter'],
                          edgecolor='black', linewidth=0.5)
                ax.add_patch(p)
        else:
            # Uniform
            for i in range(n_particles):
                x = np.random.uniform(0.3, 3.7)
                y = np.random.uniform(0.3, 3.7)
                p = Circle((x, y), 0.12, facecolor=COLORS['matter'],
                          edgecolor='black', linewidth=0.5)
                ax.add_patch(p)

        ax.set_title(title, fontsize=11, fontweight='bold')
        ax.text(2, -0.3, t, fontsize=10, ha='center', style='italic')

    # Arrows between panels
    fig.patches.extend([
        FancyArrowPatch((0.34, 0.78), (0.38, 0.78),
                        arrowstyle='->', mutation_scale=15, color='gray',
                        lw=2, transform=fig.transFigure, figure=fig),
        FancyArrowPatch((0.64, 0.78), (0.68, 0.78),
                        arrowstyle='->', mutation_scale=15, color='gray',
                        lw=2, transform=fig.transFigure, figure=fig),
    ])

    # =========================================================================
    # Middle: Temperature as Average Kinetic Flux
    # =========================================================================
    ax_temp.set_xlim(0, 10)
    ax_temp.set_ylim(0, 5)

    # Draw two systems at different temperatures
    # Cold system (left)
    cold_box = Rectangle((0.5, 1), 3, 3, facecolor='#e0e0ff',
                         edgecolor='black', linewidth=2)
    ax_temp.add_patch(cold_box)
    ax_temp.text(2, 4.3, 'Cold (Low T)', fontsize=11, ha='center', fontweight='bold')

    # Particles with small velocity arrows
    for _ in range(8):
        x = np.random.uniform(0.8, 3.2)
        y = np.random.uniform(1.3, 3.7)
        p = Circle((x, y), 0.12, facecolor=COLORS['antimatter'],
                   edgecolor='black', linewidth=0.5)
        ax_temp.add_patch(p)
        # Small velocity
        vx = np.random.normal(0, 0.15)
        vy = np.random.normal(0, 0.15)
        ax_temp.arrow(x, y, vx, vy, head_width=0.05, head_length=0.03,
                     fc='black', ec='black')

    # Hot system (right)
    hot_box = Rectangle((6.5, 1), 3, 3, facecolor='#ffe0e0',
                        edgecolor='black', linewidth=2)
    ax_temp.add_patch(hot_box)
    ax_temp.text(8, 4.3, 'Hot (High T)', fontsize=11, ha='center', fontweight='bold')

    # Particles with large velocity arrows
    for _ in range(8):
        x = np.random.uniform(6.8, 9.2)
        y = np.random.uniform(1.3, 3.7)
        p = Circle((x, y), 0.12, facecolor=COLORS['matter'],
                   edgecolor='black', linewidth=0.5)
        ax_temp.add_patch(p)
        # Large velocity
        vx = np.random.normal(0, 0.4)
        vy = np.random.normal(0, 0.4)
        ax_temp.arrow(x, y, vx, vy, head_width=0.08, head_length=0.05,
                     fc='black', ec='black')

    # Center: formula
    ax_temp.text(5, 2.5, r'$T \propto \langle |\mathbf{J}|^2 \rangle$' + '\n\n' +
                 'Temperature = Average\nkinetic flux energy',
                 fontsize=11, ha='center', va='center',
                 bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                          edgecolor='gray'))

    ax_temp.set_title('Temperature: Average Kinetic Flux Density',
                      fontsize=13, fontweight='bold')
    ax_temp.axis('off')

    # =========================================================================
    # Bottom: Second Law Emergence
    # =========================================================================
    ax_arrow.set_xlim(0, 10)
    ax_arrow.set_ylim(0, 5)
    ax_arrow.axis('off')

    ax_arrow.set_title('Second Law: Entropy Increases (Statistical Tendency)',
                       fontsize=13, fontweight='bold')

    # Time arrow
    ax_arrow.annotate('', xy=(9, 2.5), xytext=(1, 2.5),
                     arrowprops=dict(arrowstyle='->', color='black', lw=3))
    ax_arrow.text(5, 2, 'Time', fontsize=12, ha='center', fontweight='bold')

    # Entropy curve
    t = np.linspace(1, 9, 100)
    S = 1 + 2 * (1 - np.exp(-0.5 * (t - 1)))
    ax_arrow.plot(t, S + 0.5, color=COLORS['accent1'], linewidth=3)
    ax_arrow.text(9.2, S[-1] + 0.5, 'S', fontsize=14, color=COLORS['accent1'],
                  fontweight='bold')

    # Equilibrium line
    ax_arrow.axhline(y=3.5, xmin=0.7, xmax=0.95, color='gray',
                    linestyle='--', linewidth=1.5)
    ax_arrow.text(8.5, 3.7, 'S_max', fontsize=10, color='gray')

    # Explanation boxes
    ax_arrow.text(2, 4.3, 'Many microstates\nlead to disorder',
                  fontsize=9, ha='center',
                  bbox=dict(boxstyle='round,pad=0.3', facecolor='#fff8e8'))

    ax_arrow.text(5, 4.3, 'Overwhelmingly\nprobable evolution',
                  fontsize=9, ha='center',
                  bbox=dict(boxstyle='round,pad=0.3', facecolor='#fff8e8'))

    ax_arrow.text(8, 4.3, 'Equilibrium:\nmaximum entropy',
                  fontsize=9, ha='center',
                  bbox=dict(boxstyle='round,pad=0.3', facecolor='#fff8e8'))

    # TRD note
    ax_arrow.text(5, 0.7, 'In TRD: The second law emerges statistically from deterministic update rules.\n'
                         'Ordered states are rare; disordered states are overwhelmingly common.',
                  fontsize=10, ha='center',
                  bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                           edgecolor=COLORS['accent1'], linewidth=2))

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Emergent Thermodynamics: Statistical Behavior from Deterministic Rules',
                fontsize=16, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0, 1, 0.96])

    return fig


if __name__ == '__main__':
    fig = generate_emergent_thermodynamics()
    output_path = Path(__file__).parent.parent / 'ch04' / 'fig_2_17_emergent_thermodynamics.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
