"""
Figure 1.14: sLoop Mechanism
============================
Visualizes the sLoop (self-Loop) mechanism that explains how
TRD can reproduce Bell inequality violations.

Shows:
- Left: Conceptual diagram of substrate overlap
- Right: Bell parameter S as function of overlap fraction f
  S(f) = 2 + (2√2 - 2) × g(f)
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Ellipse, FancyArrowPatch, Arc
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_sloop_mechanism():
    """
    Generate the sLoop mechanism visualization.

    Two panels:
    1. Conceptual diagram showing apparatus embedding in substrate
    2. Bell parameter S vs overlap fraction f
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # =========================================================================
    # Panel 1: Conceptual Diagram
    # =========================================================================
    ax1.set_xlim(-1.5, 1.5)
    ax1.set_ylim(-1.2, 1.2)
    ax1.set_aspect('equal')
    ax1.axis('off')
    ax1.set_title('The sLoop: Substrate Overlap', fontsize=14, fontweight='bold', pad=10)

    # Draw the shared flux substrate (large ellipse)
    substrate = Ellipse((0, 0), 2.4, 1.8, facecolor='#e8f4f8',
                        edgecolor=COLORS['accent1'], linewidth=3, alpha=0.5)
    ax1.add_patch(substrate)
    ax1.text(0, 0.75, 'Shared Flux Substrate', fontsize=11,
             ha='center', fontweight='bold', color=COLORS['accent1'])

    # Entangled pair source (center)
    source = Circle((0, 0), 0.12, facecolor=COLORS['highlight'],
                    edgecolor='black', linewidth=2, zorder=10)
    ax1.add_patch(source)
    ax1.text(0, -0.25, 'Pair\nSource', fontsize=9, ha='center')

    # Particle paths (wavy lines representing flux)
    t = np.linspace(0, 1, 50)
    # Left particle path
    x_left = -0.1 - 0.8 * t
    y_left = 0.05 * np.sin(10 * np.pi * t)
    ax1.plot(x_left, y_left, color=COLORS['matter'], linewidth=2, zorder=5)
    ax1.scatter([-0.9], [0], c=COLORS['matter'], s=100, zorder=6, marker='o')
    ax1.text(-0.9, 0.15, r'$\psi_1$', fontsize=12, ha='center', fontweight='bold')

    # Right particle path
    x_right = 0.1 + 0.8 * t
    y_right = -0.05 * np.sin(10 * np.pi * t)
    ax1.plot(x_right, y_right, color=COLORS['antimatter'], linewidth=2, zorder=5)
    ax1.scatter([0.9], [0], c=COLORS['antimatter'], s=100, zorder=6, marker='o')
    ax1.text(0.9, 0.15, r'$\psi_2$', fontsize=12, ha='center', fontweight='bold')

    # Measurement apparatuses (boxes embedded in substrate)
    # Left apparatus
    left_box = mpatches.FancyBboxPatch((-1.3, -0.3), 0.4, 0.6,
                                        boxstyle="round,pad=0.05",
                                        facecolor='#ffcccc',
                                        edgecolor=COLORS['matter'],
                                        linewidth=2, zorder=8)
    ax1.add_patch(left_box)
    ax1.text(-1.1, 0, 'A', fontsize=14, ha='center', va='center', fontweight='bold')
    ax1.text(-1.1, -0.5, 'Detector A', fontsize=9, ha='center')

    # Right apparatus
    right_box = mpatches.FancyBboxPatch((0.9, -0.3), 0.4, 0.6,
                                         boxstyle="round,pad=0.05",
                                         facecolor='#ccccff',
                                         edgecolor=COLORS['antimatter'],
                                         linewidth=2, zorder=8)
    ax1.add_patch(right_box)
    ax1.text(1.1, 0, 'B', fontsize=14, ha='center', va='center', fontweight='bold')
    ax1.text(1.1, -0.5, 'Detector B', fontsize=9, ha='center')

    # Curved arrows showing correlation through substrate
    arc1 = Arc((0, -0.6), 1.6, 0.8, angle=0, theta1=20, theta2=160,
               color=COLORS['accent2'], linewidth=2, linestyle='--')
    ax1.add_patch(arc1)
    ax1.annotate('', xy=(-0.75, -0.55), xytext=(-0.7, -0.7),
                arrowprops=dict(arrowstyle='->', color=COLORS['accent2'], lw=2))

    ax1.text(0, -0.95, 'Correlation via shared substrate', fontsize=10,
             ha='center', style='italic', color=COLORS['accent2'])

    # Key insight box
    insight_text = (
        "The sLoop insight:\n"
        "Apparatus and particles share\n"
        "the same flux field.\n\n"
        "Correlations aren't transmitted—\n"
        "they're inherited."
    )
    ax1.text(0, -1.1, insight_text, fontsize=9, ha='center', va='top',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    # =========================================================================
    # Panel 2: Bell Parameter Graph
    # =========================================================================
    # Overlap fraction
    f = np.linspace(0, 1, 100)

    # Bell parameter: S(f) = 2 + (2√2 - 2) × g(f)
    # Using linear g(f) = f for simplicity
    S_classical = 2  # Classical limit
    S_quantum = 2 * np.sqrt(2)  # Quantum limit (~2.828)
    S = S_classical + (S_quantum - S_classical) * f

    # Plot
    ax2.fill_between(f, S_classical, S, alpha=0.3, color=COLORS['accent1'],
                     label='Quantum advantage')
    ax2.plot(f, S, color=COLORS['text'], linewidth=3, label='$S(f)$')

    # Reference lines
    ax2.axhline(y=S_classical, color=COLORS['matter'], linestyle='--',
                linewidth=2, label=f'Classical limit (S=2)')
    ax2.axhline(y=S_quantum, color=COLORS['accent1'], linestyle='--',
                linewidth=2, label=f'Quantum limit (S=2√2≈{S_quantum:.3f})')

    # Mark key points
    ax2.scatter([0], [S_classical], color=COLORS['matter'], s=100, zorder=5)
    ax2.scatter([1], [S_quantum], color=COLORS['accent1'], s=100, zorder=5)

    # Annotations
    ax2.annotate('No overlap:\nClassical', xy=(0, S_classical),
                xytext=(0.15, 2.2), fontsize=10,
                arrowprops=dict(arrowstyle='->', lw=1.5))

    ax2.annotate('Full overlap:\nQuantum', xy=(1, S_quantum),
                xytext=(0.75, 2.6), fontsize=10,
                arrowprops=dict(arrowstyle='->', lw=1.5))

    # Shade forbidden regions
    ax2.axhspan(0, S_classical, alpha=0.1, color='gray')
    ax2.text(0.5, 1.8, 'Classical regime', fontsize=10, ha='center',
             color='gray', style='italic')

    apply_trd_style(ax2,
                    title='Bell Parameter vs Substrate Overlap',
                    xlabel='Overlap fraction $f$',
                    ylabel='Bell parameter $S$')

    ax2.set_xlim(-0.05, 1.05)
    ax2.set_ylim(1.5, 3.0)

    # Formula box
    formula_text = (
        r'$S(f) = 2 + (2\sqrt{2} - 2) \cdot g(f)$' + '\n\n'
        r'$f=0$: Classical (S=2)' + '\n'
        r'$f=1$: Quantum (S=$2\sqrt{2}$)' + '\n\n'
        'Bell violation: $S > 2$'
    )
    ax2.text(0.02, 0.98, formula_text, transform=ax2.transAxes,
             fontsize=10, verticalalignment='top',
             bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                      alpha=0.95, edgecolor='gray'))

    ax2.legend(loc='lower right', fontsize=9)

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_sloop_mechanism()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_1_14_sloop_mechanism.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
