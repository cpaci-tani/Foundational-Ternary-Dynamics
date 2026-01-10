"""
Figure 2.9: Entanglement Origin
===============================
Visualizes how entanglement arises from shared-origin pair production.

In TRD, entangled particles are those that manifested together from
the same flux fluctuation, inheriting correlated properties from
their common origin.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, Wedge, Rectangle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_entanglement_origin():
    """
    Generate the entanglement origin visualization.

    Shows:
    1. Top: Pair production from flux fluctuation
    2. Middle: Separation while maintaining correlation
    3. Bottom: Correlation preserved at measurement
    """
    fig = plt.figure(figsize=(14, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Create custom grid
    gs = fig.add_gridspec(3, 2, height_ratios=[1, 1, 1.2], hspace=0.3, wspace=0.2)

    # Top spans full width
    ax_creation = fig.add_subplot(gs[0, :])
    # Middle panels
    ax_separation = fig.add_subplot(gs[1, 0])
    ax_properties = fig.add_subplot(gs[1, 1])
    # Bottom spans full width
    ax_measurement = fig.add_subplot(gs[2, :])

    # =========================================================================
    # Top: Pair Creation from Flux
    # =========================================================================
    ax_creation.set_xlim(0, 10)
    ax_creation.set_ylim(0, 4)
    ax_creation.set_aspect('equal')
    ax_creation.axis('off')

    # Stage 1: High-density flux
    ax_creation.text(1.5, 3.5, 'Stage 1: High-Density Flux', fontsize=11,
                     fontweight='bold')

    # Draw flux concentration
    theta = np.linspace(0, 2*np.pi, 100)
    for r in [0.3, 0.5, 0.7, 0.9]:
        alpha = 0.6 - 0.5 * r
        ax_creation.plot(1.5 + r * np.cos(theta), 1.8 + r * np.sin(theta),
                        color=COLORS['highlight'], linewidth=2, alpha=alpha)

    ax_creation.text(1.5, 0.5, 'density > KB', fontsize=9, ha='center',
                     style='italic')

    # Arrow to stage 2
    ax_creation.annotate('', xy=(3.5, 1.8), xytext=(2.7, 1.8),
                        arrowprops=dict(arrowstyle='->', color='black', lw=2))

    # Stage 2: Genesis event
    ax_creation.text(5, 3.5, 'Stage 2: Simultaneous Genesis', fontsize=11,
                     fontweight='bold')

    # Two particles appearing
    circle_pos = Circle((4.5, 1.8), 0.35, facecolor=COLORS['matter'],
                        edgecolor='black', linewidth=2)
    circle_neg = Circle((5.5, 1.8), 0.35, facecolor=COLORS['antimatter'],
                        edgecolor='black', linewidth=2)
    ax_creation.add_patch(circle_pos)
    ax_creation.add_patch(circle_neg)

    ax_creation.text(4.5, 1.8, '+', fontsize=14, ha='center', va='center',
                     color='white', fontweight='bold')
    ax_creation.text(5.5, 1.8, '-', fontsize=14, ha='center', va='center',
                     color='white', fontweight='bold')

    # Wavy line connecting them (entanglement)
    x_wave = np.linspace(4.85, 5.15, 50)
    y_wave = 1.8 + 0.1 * np.sin(20 * (x_wave - 4.85))
    ax_creation.plot(x_wave, y_wave, color='purple', linewidth=2, linestyle='-')

    ax_creation.text(5, 0.5, 'Shared UUID assigned', fontsize=9, ha='center',
                     style='italic')

    # Arrow to stage 3
    ax_creation.annotate('', xy=(7.3, 1.8), xytext=(6.2, 1.8),
                        arrowprops=dict(arrowstyle='->', color='black', lw=2))

    # Stage 3: Properties assigned
    ax_creation.text(8.5, 3.5, 'Stage 3: Correlated Properties', fontsize=11,
                     fontweight='bold')

    # Two particles with property indicators
    circle_pos2 = Circle((8, 1.8), 0.35, facecolor=COLORS['matter'],
                         edgecolor='black', linewidth=2)
    circle_neg2 = Circle((9, 1.8), 0.35, facecolor=COLORS['antimatter'],
                         edgecolor='black', linewidth=2)
    ax_creation.add_patch(circle_pos2)
    ax_creation.add_patch(circle_neg2)

    # Spin arrows
    ax_creation.annotate('', xy=(8, 2.4), xytext=(8, 2.0),
                        arrowprops=dict(arrowstyle='->', color='white', lw=2))
    ax_creation.annotate('', xy=(9, 1.4), xytext=(9, 1.8),
                        arrowprops=dict(arrowstyle='->', color='white', lw=2))

    ax_creation.text(8, 2.6, 'spin up', fontsize=8, ha='center')
    ax_creation.text(9, 1.2, 'spin down', fontsize=8, ha='center')

    ax_creation.text(8.5, 0.5, 'Total spin = 0\n(conserved)', fontsize=9,
                     ha='center', style='italic')

    ax_creation.set_title('Pair Production: The Origin of Entanglement',
                         fontsize=14, fontweight='bold')

    # =========================================================================
    # Middle Left: Separation
    # =========================================================================
    ax_separation.set_xlim(0, 10)
    ax_separation.set_ylim(0, 6)
    ax_separation.axis('off')

    ax_separation.set_title('Spatial Separation', fontsize=12, fontweight='bold')

    # Show particles moving apart
    positions = [(1, 3), (3, 3), (5, 3), (7, 3), (9, 3)]
    for i, (x, _) in enumerate(positions):
        alpha = 0.2 + 0.2 * i
        # Left particle (moving left)
        ax_separation.add_patch(Circle((5 - 0.8*i, 3), 0.25,
                                       facecolor=COLORS['matter'],
                                       edgecolor='black', alpha=alpha))
        # Right particle (moving right)
        ax_separation.add_patch(Circle((5 + 0.8*i, 3), 0.25,
                                       facecolor=COLORS['antimatter'],
                                       edgecolor='black', alpha=alpha))

    # Dashed line showing they came from same origin
    ax_separation.plot([1.8, 8.2], [3, 3], 'k--', linewidth=1, alpha=0.5)

    ax_separation.text(5, 1.5, 'Particles separate but\nshare same UUID',
                       fontsize=10, ha='center')

    ax_separation.text(5, 0.5,
                       'No signal exchanged -\ncorrelation is from shared origin',
                       fontsize=9, ha='center', style='italic', color='gray')

    # =========================================================================
    # Middle Right: Property Correlation Table
    # =========================================================================
    ax_properties.set_xlim(0, 10)
    ax_properties.set_ylim(0, 6)
    ax_properties.axis('off')

    ax_properties.set_title('Correlated Properties', fontsize=12, fontweight='bold')

    # Table header
    ax_properties.text(5, 5.2, 'Property', fontsize=10, ha='center',
                       fontweight='bold')
    ax_properties.text(2, 5.2, 'Particle A', fontsize=10, ha='center',
                       fontweight='bold', color=COLORS['matter'])
    ax_properties.text(8, 5.2, 'Particle B', fontsize=10, ha='center',
                       fontweight='bold', color=COLORS['antimatter'])

    # Table rows
    rows = [
        ('Charge', '+q', '-q'),
        ('Spin', 'up', 'down'),
        ('Momentum', '+p', '-p'),
        ('UUID', 'shared', 'shared'),
    ]

    for i, (prop, val_a, val_b) in enumerate(rows):
        y = 4.3 - i * 0.9
        ax_properties.text(5, y, prop, fontsize=10, ha='center')
        ax_properties.text(2, y, val_a, fontsize=10, ha='center',
                          color=COLORS['matter'])
        ax_properties.text(8, y, val_b, fontsize=10, ha='center',
                          color=COLORS['antimatter'])
        ax_properties.axhline(y=y-0.4, xmin=0.1, xmax=0.9, color='gray',
                             linewidth=0.5, alpha=0.5)

    ax_properties.text(5, 0.5, 'Conservation laws enforced\nat creation time',
                       fontsize=9, ha='center', style='italic')

    # =========================================================================
    # Bottom: Measurement Correlation
    # =========================================================================
    ax_measurement.set_xlim(0, 14)
    ax_measurement.set_ylim(0, 5)
    ax_measurement.axis('off')

    ax_measurement.set_title('Measurement: Correlations Revealed, Not Created',
                            fontsize=14, fontweight='bold')

    # Left detector
    detector_left = Rectangle((0.5, 1.5), 2, 2.5, facecolor='#e0e0e0',
                              edgecolor='black', linewidth=2)
    ax_measurement.add_patch(detector_left)
    ax_measurement.text(1.5, 4.3, 'Detector A', fontsize=10, ha='center',
                        fontweight='bold')

    # Particle A approaching
    ax_measurement.add_patch(Circle((3.5, 2.75), 0.3, facecolor=COLORS['matter'],
                                   edgecolor='black', linewidth=2))
    ax_measurement.annotate('', xy=(2.7, 2.75), xytext=(3.2, 2.75),
                           arrowprops=dict(arrowstyle='->', color='black', lw=2))

    # Measurement result
    ax_measurement.text(1.5, 2.75, 'Spin UP', fontsize=11, ha='center',
                        fontweight='bold', color=COLORS['matter'])

    # Right detector
    detector_right = Rectangle((11.5, 1.5), 2, 2.5, facecolor='#e0e0e0',
                               edgecolor='black', linewidth=2)
    ax_measurement.add_patch(detector_right)
    ax_measurement.text(12.5, 4.3, 'Detector B', fontsize=10, ha='center',
                        fontweight='bold')

    # Particle B approaching
    ax_measurement.add_patch(Circle((10.5, 2.75), 0.3, facecolor=COLORS['antimatter'],
                                   edgecolor='black', linewidth=2))
    ax_measurement.annotate('', xy=(11.3, 2.75), xytext=(10.8, 2.75),
                           arrowprops=dict(arrowstyle='->', color='black', lw=2))

    # Measurement result
    ax_measurement.text(12.5, 2.75, 'Spin DOWN', fontsize=11, ha='center',
                        fontweight='bold', color=COLORS['antimatter'])

    # Center explanation
    ax_measurement.text(7, 3.5, 'Spacelike separated\n(no communication possible)',
                        fontsize=10, ha='center',
                        bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    ax_measurement.text(7, 1.8, 'Correlation exists because both particles\n'
                               'inherited their properties from a common source.',
                        fontsize=10, ha='center', style='italic')

    ax_measurement.text(7, 0.5, 'TRD: No "spooky action" - just shared origin.',
                        fontsize=11, ha='center', fontweight='bold',
                        color=COLORS['accent1'])

    # =========================================================================
    # Overall
    # =========================================================================
    fig.suptitle('Entanglement in TRD: Correlation from Common Origin',
                fontsize=16, fontweight='bold', y=0.99)

    plt.tight_layout(rect=[0, 0, 1, 0.97])

    return fig


if __name__ == '__main__':
    fig = generate_entanglement_origin()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_2_9_entanglement_origin.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
