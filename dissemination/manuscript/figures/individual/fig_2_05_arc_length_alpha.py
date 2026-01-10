"""
Figure 2.5: Arc Length to Alpha
===============================
Shows the derivation chain from the Lemniscate-Alpha curve
to the fine structure constant:

L (arc length) × 91/732 = G* → Master Quadratic → α = 1/137.036

This is the heart of the TRD constant derivation.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Circle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, MODE_COLORS, apply_trd_style
from utils.physics_constants import (
    FREQUENCIES, X_AMPLITUDES, Y_AMPLITUDES,
    ARC_LENGTH, G_STAR, ALPHA_INV, T_13
)


def generate_arc_length_alpha():
    """
    Generate the arc length to alpha derivation visualization.

    Shows the complete derivation chain with the curve and
    mathematical transformations.
    """
    fig = plt.figure(figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Create grid layout
    gs = fig.add_gridspec(2, 3, height_ratios=[1.2, 1], hspace=0.3, wspace=0.25)

    # Top row: The curve spanning two columns
    ax_curve = fig.add_subplot(gs[0, :2])

    # Top right: Arc length calculation
    ax_arc = fig.add_subplot(gs[0, 2])

    # Bottom row: Three derivation steps
    ax_step1 = fig.add_subplot(gs[1, 0])
    ax_step2 = fig.add_subplot(gs[1, 1])
    ax_step3 = fig.add_subplot(gs[1, 2])

    # =========================================================================
    # The Lemniscate-Alpha Curve
    # =========================================================================
    t = np.linspace(0, 2 * np.pi, 1000)

    x_full = np.zeros_like(t)
    y_full = np.zeros_like(t)
    for j, freq in enumerate(FREQUENCIES):
        x_full += X_AMPLITUDES[j] * np.cos(freq * t)
        y_full += Y_AMPLITUDES[j] * np.sin(freq * t)

    ax_curve.plot(x_full, y_full, 'k-', linewidth=2.5)
    ax_curve.fill(x_full, y_full, alpha=0.1, color=COLORS['highlight'])

    # Mark arc length along curve
    n_markers = 20
    indices = np.linspace(0, len(t)-1, n_markers).astype(int)
    ax_curve.scatter(x_full[indices], y_full[indices],
                     c=COLORS['highlight'], s=30, zorder=5)

    ax_curve.set_xlim(-2.5, 2.5)
    ax_curve.set_ylim(-2, 2)
    ax_curve.set_aspect('equal')
    ax_curve.set_title('The Lemniscate-Alpha Curve', fontsize=14, fontweight='bold')
    ax_curve.set_xlabel('x(t)')
    ax_curve.set_ylabel('y(t)')
    ax_curve.grid(True, alpha=0.3)

    # Parametric equations
    eq_text = (
        r'$x(t) = \sum_{k=0}^{4} a_k \cos(2^k t)$' + '\n'
        r'$y(t) = \sum_{k=0}^{4} b_k \sin(2^k t)$'
    )
    ax_curve.text(0.02, 0.98, eq_text, transform=ax_curve.transAxes,
                  fontsize=10, va='top',
                  bbox=dict(boxstyle='round,pad=0.3', facecolor='white', alpha=0.9))

    # =========================================================================
    # Arc Length Calculation
    # =========================================================================
    ax_arc.axis('off')
    ax_arc.set_xlim(0, 1)
    ax_arc.set_ylim(0, 1)

    ax_arc.text(0.5, 0.9, 'Arc Length', fontsize=14, fontweight='bold',
                ha='center')

    # Arc length formula
    ax_arc.text(0.5, 0.7, r'$L = \oint \sqrt{dx^2 + dy^2}$',
                fontsize=12, ha='center')

    ax_arc.text(0.5, 0.5, f'L ≈ {ARC_LENGTH:.4f}', fontsize=16,
                ha='center', fontweight='bold', color=COLORS['highlight'],
                bbox=dict(boxstyle='round,pad=0.3', facecolor='#fff8dc'))

    ax_arc.text(0.5, 0.3, '(Total path length\naround the curve)',
                fontsize=10, ha='center', style='italic', color='gray')

    # =========================================================================
    # Step 1: L × scaling = G*
    # =========================================================================
    ax_step1.axis('off')
    ax_step1.set_xlim(0, 1)
    ax_step1.set_ylim(0, 1)

    # Box
    box1 = FancyBboxPatch((0.05, 0.1), 0.9, 0.8, boxstyle="round,pad=0.02",
                          facecolor='#e8f4f8', edgecolor=COLORS['accent1'], linewidth=2)
    ax_step1.add_patch(box1)

    ax_step1.text(0.5, 0.85, 'STEP 1', fontsize=12, fontweight='bold',
                  ha='center', color=COLORS['accent1'])
    ax_step1.text(0.5, 0.7, 'Scaling Factor', fontsize=11, ha='center')

    ax_step1.text(0.5, 0.5, r'$G^* = L \times \frac{91}{732}$',
                  fontsize=13, ha='center',
                  bbox=dict(boxstyle='round,pad=0.2', facecolor='white'))

    ax_step1.text(0.5, 0.32, f'= {ARC_LENGTH:.4f} × {T_13}/732',
                  fontsize=10, ha='center')

    ax_step1.text(0.5, 0.18, f'G* = {G_STAR:.4f}', fontsize=14,
                  ha='center', fontweight='bold', color=COLORS['accent1'])

    # =========================================================================
    # Step 2: Master Quadratic
    # =========================================================================
    ax_step2.axis('off')
    ax_step2.set_xlim(0, 1)
    ax_step2.set_ylim(0, 1)

    # Box
    box2 = FancyBboxPatch((0.05, 0.1), 0.9, 0.8, boxstyle="round,pad=0.02",
                          facecolor='#fff0f0', edgecolor=COLORS['matter'], linewidth=2)
    ax_step2.add_patch(box2)

    ax_step2.text(0.5, 0.85, 'STEP 2', fontsize=12, fontweight='bold',
                  ha='center', color=COLORS['matter'])
    ax_step2.text(0.5, 0.7, 'Master Quadratic', fontsize=11, ha='center')

    ax_step2.text(0.5, 0.5, r'$x^2 - 16G^{*2}x + 16G^{*3} = 0$',
                  fontsize=12, ha='center',
                  bbox=dict(boxstyle='round,pad=0.2', facecolor='white'))

    ax_step2.text(0.5, 0.32, 'Roots:', fontsize=10, ha='center')
    ax_step2.text(0.5, 0.18, f'x₊ = {ALPHA_INV:.3f}    x₋ = 3.024',
                  fontsize=12, ha='center', fontweight='bold', color=COLORS['matter'])

    # =========================================================================
    # Step 3: Fine Structure Constant
    # =========================================================================
    ax_step3.axis('off')
    ax_step3.set_xlim(0, 1)
    ax_step3.set_ylim(0, 1)

    # Box
    box3 = FancyBboxPatch((0.05, 0.1), 0.9, 0.8, boxstyle="round,pad=0.02",
                          facecolor='#f0fff0', edgecolor=COLORS['accent1'], linewidth=2)
    ax_step3.add_patch(box3)

    ax_step3.text(0.5, 0.85, 'STEP 3', fontsize=12, fontweight='bold',
                  ha='center', color=COLORS['accent1'])
    ax_step3.text(0.5, 0.7, 'Fine Structure Constant', fontsize=11, ha='center')

    ax_step3.text(0.5, 0.5, r'$\alpha = \frac{1}{x_+}$',
                  fontsize=14, ha='center',
                  bbox=dict(boxstyle='round,pad=0.2', facecolor='white'))

    ax_step3.text(0.5, 0.32, f'α = 1/{ALPHA_INV:.3f}', fontsize=12, ha='center')

    ax_step3.text(0.5, 0.18, 'α ≈ 0.00729735',
                  fontsize=14, ha='center', fontweight='bold', color=COLORS['accent1'])

    # =========================================================================
    # Connecting arrows
    # =========================================================================
    # Arrow from curve to arc length
    fig.patches.extend([
        FancyArrowPatch((0.63, 0.72), (0.68, 0.72),
                        arrowstyle='->', mutation_scale=15,
                        color='gray', lw=2,
                        transform=fig.transFigure, figure=fig)
    ])

    # Arrows between steps
    for x_start, x_end in [(0.35, 0.37), (0.64, 0.66)]:
        fig.patches.extend([
            FancyArrowPatch((x_start, 0.25), (x_end, 0.25),
                            arrowstyle='->', mutation_scale=15,
                            color='gray', lw=2,
                            transform=fig.transFigure, figure=fig)
        ])

    # =========================================================================
    # Title and summary
    # =========================================================================
    fig.suptitle('From Arc Length to the Fine Structure Constant',
                 fontsize=18, fontweight='bold', y=0.98)

    summary = (
        "The curve's arc length, through a simple rational scaling factor, "
        "determines G*, which encodes α = 1/137.036 as a quadratic root."
    )
    fig.text(0.5, 0.02, summary, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    return fig


if __name__ == '__main__':
    fig = generate_arc_length_alpha()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_2_5_arc_length_alpha.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
