"""
Figure 2.4: Force Gradients
===========================
Shows how each of the four forces emerges from gradients
of different field quantities in TRD.

- Gravity: ∇ρ (density gradient)
- Electromagnetic: ∇q (charge gradient) + ∇×J (curl)
- Strong: Yukawa-form from flux exchange
- Weak: Stress threshold for transmutation
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Circle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, FORCE_COLORS, apply_trd_style


def generate_force_gradients():
    """
    Generate the force gradients visualization.

    Four panels showing how each force emerges from field gradients.
    """
    fig, axes = plt.subplots(2, 2, figsize=(14, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    forces = [
        {
            'ax': axes[0, 0],
            'name': 'GRAVITY',
            'color': FORCE_COLORS['gravity'],
            'equation': r'$\mathbf{F}_g = G_{bias} \cdot \nabla\bar{\rho}$',
            'field': 'Density ρ = |J|',
            'mechanism': 'Attraction toward\nhigh-density regions',
            'gradient_type': 'scalar',
        },
        {
            'ax': axes[0, 1],
            'name': 'ELECTROMAGNETIC',
            'color': FORCE_COLORS['electromagnetic'],
            'equation': r'$\mathbf{F}_{em} = -q\nabla\bar{q} + \beta(\nabla \times \mathbf{J}) \times \hat{J}$',
            'field': 'Charge q and Flux J',
            'mechanism': 'Electric: charge gradient\nMagnetic: flux curl',
            'gradient_type': 'vector',
        },
        {
            'ax': axes[1, 0],
            'name': 'STRONG',
            'color': FORCE_COLORS['strong'],
            'equation': r'$F_s = g^2 \frac{e^{-mr}(1+mr)}{r^2}$',
            'field': 'Flux exchange',
            'mechanism': 'Short-range binding\nvia Yukawa potential',
            'gradient_type': 'yukawa',
        },
        {
            'ax': axes[1, 1],
            'name': 'WEAK',
            'color': FORCE_COLORS['weak'],
            'equation': r'stress $= |\nabla \cdot J| + |\nabla \times J| + |\nabla\rho|$',
            'field': 'Field stress',
            'mechanism': 'Polarity flip when\nstress > threshold',
            'gradient_type': 'threshold',
        },
    ]

    for force in forces:
        ax = force['ax']
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)
        ax.axis('off')

        # Background box
        box = FancyBboxPatch(
            (0.02, 0.02), 0.96, 0.96,
            boxstyle="round,pad=0.02",
            facecolor='white',
            edgecolor=force['color'],
            linewidth=3
        )
        ax.add_patch(box)

        # Force name
        ax.text(0.5, 0.92, force['name'], fontsize=16, fontweight='bold',
                ha='center', color=force['color'])

        # Equation
        ax.text(0.5, 0.78, force['equation'], fontsize=11, ha='center',
                bbox=dict(boxstyle='round,pad=0.3', facecolor='#f0f0f0'))

        # Field label
        ax.text(0.5, 0.65, f"Field: {force['field']}", fontsize=10,
                ha='center', style='italic')

        # Gradient visualization
        if force['gradient_type'] == 'scalar':
            # Density gradient - show particles moving toward high density
            x = np.linspace(0.2, 0.8, 20)
            y = np.linspace(0.25, 0.55, 10)
            X, Y = np.meshgrid(x, y)
            # Density increases to the right
            density = X
            ax.contourf(X, Y, density, levels=10, cmap='Oranges', alpha=0.6)
            # Arrows pointing right (toward high density)
            ax.quiver([0.3, 0.4, 0.5], [0.4, 0.4, 0.4],
                     [0.1, 0.1, 0.1], [0, 0, 0],
                     color=force['color'], scale=1.5, width=0.015)
            ax.text(0.75, 0.35, 'High ρ', fontsize=9, ha='center')
            ax.text(0.25, 0.35, 'Low ρ', fontsize=9, ha='center')

        elif force['gradient_type'] == 'vector':
            # EM - show both charge gradient and curl
            # Left: charge gradient (like charges repel)
            ax.scatter([0.25], [0.4], c=COLORS['matter'], s=200, zorder=5)
            ax.scatter([0.45], [0.4], c=COLORS['matter'], s=200, zorder=5)
            ax.annotate('', xy=(0.18, 0.4), xytext=(0.25, 0.4),
                       arrowprops=dict(arrowstyle='->', color=force['color'], lw=2))
            ax.annotate('', xy=(0.52, 0.4), xytext=(0.45, 0.4),
                       arrowprops=dict(arrowstyle='->', color=force['color'], lw=2))
            ax.text(0.35, 0.5, 'Repel', fontsize=9, ha='center')

            # Right: curl (magnetic)
            theta = np.linspace(0, 2*np.pi, 50)
            r = 0.08
            cx, cy = 0.7, 0.4
            ax.plot(cx + r*np.cos(theta), cy + r*np.sin(theta),
                   color=force['color'], linewidth=2)
            ax.annotate('', xy=(cx, cy+r), xytext=(cx+0.02, cy+r),
                       arrowprops=dict(arrowstyle='->', color=force['color'], lw=2))
            ax.text(0.7, 0.28, 'Curl', fontsize=9, ha='center')

        elif force['gradient_type'] == 'yukawa':
            # Yukawa - show attractive binding at medium range
            r = np.linspace(0.2, 0.8, 100)
            # Normalize to fit in display
            y_base = 0.4
            potential = -0.1 * np.exp(-5*(r-0.2)) / (r - 0.15)
            potential = np.clip(potential, -0.15, 0.15)
            ax.fill_between(r, y_base, y_base + potential,
                           where=(potential < 0), alpha=0.4, color=force['color'])
            ax.plot(r, y_base + potential, color=force['color'], linewidth=2)
            ax.axhline(y=y_base, color='gray', linestyle='--', alpha=0.5)
            ax.text(0.25, 0.28, 'Repulsive', fontsize=8, ha='center')
            ax.text(0.55, 0.28, 'Attractive', fontsize=8, ha='center')
            ax.text(0.5, 0.55, 'r', fontsize=10, ha='center')

        elif force['gradient_type'] == 'threshold':
            # Weak - show stress threshold
            stress_levels = [0.2, 0.4, 0.6, 0.8]
            for i, s in enumerate(stress_levels):
                x = 0.2 + i * 0.18
                height = s * 0.25
                color = COLORS['matter'] if s < 0.7 else COLORS['antimatter']
                bar = FancyBboxPatch(
                    (x, 0.3), 0.12, height,
                    boxstyle="round,pad=0.01",
                    facecolor=color,
                    edgecolor='black'
                )
                ax.add_patch(bar)
            ax.axhline(y=0.45, color=force['color'], linestyle='--', linewidth=2)
            ax.text(0.85, 0.45, 'Threshold', fontsize=8, ha='left', color=force['color'])
            ax.text(0.5, 0.28, 'Stress level', fontsize=9, ha='center')
            ax.text(0.75, 0.52, '+1→-1', fontsize=8, ha='center', color=COLORS['antimatter'])

        # Mechanism description
        ax.text(0.5, 0.12, force['mechanism'], fontsize=10, ha='center',
                va='center', style='italic')

    fig.suptitle('Force Emergence from Field Gradients',
                fontsize=18, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_force_gradients()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_2_4_force_gradients.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
