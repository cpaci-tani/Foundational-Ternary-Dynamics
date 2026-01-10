"""
Figure 2.2: Discrete vs Continuous
==================================
Side-by-side comparison showing how continuous calculus operations
are approximated on the discrete TRD lattice.

Shows:
- Gradient (∇f)
- Divergence (∇·J)
- Curl (∇×J)
- Laplacian (∇²f)
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Rectangle, Circle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_discrete_continuous():
    """
    Generate the discrete vs continuous comparison figure.

    Four rows showing each differential operator in both
    continuous and discrete forms.
    """
    fig, axes = plt.subplots(4, 2, figsize=(12, 14), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Column headers
    axes[0, 0].set_title('Continuous', fontsize=14, fontweight='bold',
                         color=COLORS['antimatter'])
    axes[0, 1].set_title('Discrete (TRD)', fontsize=14, fontweight='bold',
                         color=COLORS['matter'])

    # =========================================================================
    # Row 1: Gradient
    # =========================================================================
    ax_c1, ax_d1 = axes[0]

    # Continuous gradient
    ax_c1.text(0.5, 0.7, r'$\nabla f = \left(\frac{\partial f}{\partial x}, \frac{\partial f}{\partial y}, \frac{\partial f}{\partial z}\right)$',
               fontsize=12, ha='center', va='center', transform=ax_c1.transAxes)
    ax_c1.text(0.5, 0.3, 'Smooth derivative\nat every point',
               fontsize=10, ha='center', va='center', transform=ax_c1.transAxes,
               style='italic', color='gray')
    ax_c1.axis('off')
    ax_c1.set_ylabel('GRADIENT', fontsize=12, fontweight='bold', rotation=0,
                     labelpad=60, va='center')

    # Discrete gradient (finite difference)
    ax_d1.text(0.5, 0.7, r'$(\nabla f)_i = \frac{f(v + e_i) - f(v - e_i)}{2}$',
               fontsize=12, ha='center', va='center', transform=ax_d1.transAxes)

    # Draw lattice diagram
    for dx in [-1, 0, 1]:
        c = COLORS['highlight'] if dx == 0 else COLORS['void']
        circle = Circle((0.5 + dx*0.15, 0.25), 0.04, facecolor=c,
                        edgecolor='black', transform=ax_d1.transAxes)
        ax_d1.add_patch(circle)
    ax_d1.annotate('', xy=(0.65, 0.25), xytext=(0.35, 0.25),
                   xycoords=ax_d1.transAxes,
                   arrowprops=dict(arrowstyle='<->', color=COLORS['matter'], lw=2))
    ax_d1.text(0.5, 0.12, 'f(v+1) - f(v-1)', fontsize=9, ha='center',
               transform=ax_d1.transAxes)
    ax_d1.axis('off')

    # =========================================================================
    # Row 2: Divergence
    # =========================================================================
    ax_c2, ax_d2 = axes[1]

    ax_c2.text(0.5, 0.7, r'$\nabla \cdot \mathbf{J} = \frac{\partial J_x}{\partial x} + \frac{\partial J_y}{\partial y} + \frac{\partial J_z}{\partial z}$',
               fontsize=11, ha='center', va='center', transform=ax_c2.transAxes)
    ax_c2.text(0.5, 0.3, 'Measures flux\nout of point',
               fontsize=10, ha='center', va='center', transform=ax_c2.transAxes,
               style='italic', color='gray')
    ax_c2.axis('off')
    ax_c2.set_ylabel('DIVERGENCE', fontsize=12, fontweight='bold', rotation=0,
                     labelpad=60, va='center')

    ax_d2.text(0.5, 0.7, r'$\nabla \cdot \mathbf{J} = \sum_i \frac{J_i(v+e_i) - J_i(v-e_i)}{2}$',
               fontsize=11, ha='center', va='center', transform=ax_d2.transAxes)

    # Draw arrows pointing outward from center
    center = (0.5, 0.25)
    for angle, label in [(0, '+x'), (np.pi, '-x'), (np.pi/2, '+y'), (-np.pi/2, '-y')]:
        dx = 0.1 * np.cos(angle)
        dy = 0.1 * np.sin(angle)
        ax_d2.annotate('', xy=(center[0]+dx*1.3, center[1]+dy*1.3),
                      xytext=center, xycoords=ax_d2.transAxes,
                      arrowprops=dict(arrowstyle='->', color=COLORS['accent1'], lw=1.5))
    circle = Circle(center, 0.03, facecolor=COLORS['highlight'],
                    edgecolor='black', transform=ax_d2.transAxes)
    ax_d2.add_patch(circle)
    ax_d2.text(0.5, 0.08, 'Sum of flux differences', fontsize=9, ha='center',
               transform=ax_d2.transAxes)
    ax_d2.axis('off')

    # =========================================================================
    # Row 3: Curl
    # =========================================================================
    ax_c3, ax_d3 = axes[2]

    ax_c3.text(0.5, 0.7, r'$(\nabla \times \mathbf{J})_i = \epsilon_{ijk}\frac{\partial J_k}{\partial x_j}$',
               fontsize=12, ha='center', va='center', transform=ax_c3.transAxes)
    ax_c3.text(0.5, 0.3, 'Measures rotation\naround point',
               fontsize=10, ha='center', va='center', transform=ax_c3.transAxes,
               style='italic', color='gray')
    ax_c3.axis('off')
    ax_c3.set_ylabel('CURL', fontsize=12, fontweight='bold', rotation=0,
                     labelpad=60, va='center')

    ax_d3.text(0.5, 0.75, r'$(\nabla \times \mathbf{J})_z = $',
               fontsize=11, ha='center', va='center', transform=ax_d3.transAxes)
    ax_d3.text(0.5, 0.55, r'$\frac{J_y(v+e_x) - J_y(v-e_x)}{2} - \frac{J_x(v+e_y) - J_x(v-e_y)}{2}$',
               fontsize=9, ha='center', va='center', transform=ax_d3.transAxes)

    # Draw circulation arrows
    theta = np.linspace(0, 2*np.pi, 50)
    r = 0.08
    cx, cy = 0.5, 0.2
    ax_d3.plot(cx + r*np.cos(theta), cy + r*np.sin(theta),
               transform=ax_d3.transAxes, color=COLORS['accent2'], linewidth=2)
    ax_d3.annotate('', xy=(cx, cy+r), xytext=(cx+0.02, cy+r),
                  xycoords=ax_d3.transAxes,
                  arrowprops=dict(arrowstyle='->', color=COLORS['accent2'], lw=2))
    ax_d3.text(0.5, 0.05, 'Circulation around loop', fontsize=9, ha='center',
               transform=ax_d3.transAxes)
    ax_d3.axis('off')

    # =========================================================================
    # Row 4: Laplacian
    # =========================================================================
    ax_c4, ax_d4 = axes[3]

    ax_c4.text(0.5, 0.7, r'$\nabla^2 f = \frac{\partial^2 f}{\partial x^2} + \frac{\partial^2 f}{\partial y^2} + \frac{\partial^2 f}{\partial z^2}$',
               fontsize=11, ha='center', va='center', transform=ax_c4.transAxes)
    ax_c4.text(0.5, 0.3, 'Measures curvature\n(diffusion operator)',
               fontsize=10, ha='center', va='center', transform=ax_c4.transAxes,
               style='italic', color='gray')
    ax_c4.axis('off')
    ax_c4.set_ylabel('LAPLACIAN', fontsize=12, fontweight='bold', rotation=0,
                     labelpad=60, va='center')

    ax_d4.text(0.5, 0.75, r'$\nabla^2 f(v) = \sum_{n \in N_6} f(n) - 6f(v)$',
               fontsize=11, ha='center', va='center', transform=ax_d4.transAxes)
    ax_d4.text(0.5, 0.55, '(6 face neighbors minus 6× center)',
               fontsize=9, ha='center', va='center', transform=ax_d4.transAxes,
               style='italic', color='gray')

    # Draw 6-neighbor stencil
    center = (0.5, 0.25)
    neighbors = [(0.1, 0), (-0.1, 0), (0, 0.08), (0, -0.08)]
    for dx, dy in neighbors:
        circle = Circle((center[0]+dx, center[1]+dy), 0.025,
                        facecolor=COLORS['antimatter'], edgecolor='black',
                        transform=ax_d4.transAxes, alpha=0.7)
        ax_d4.add_patch(circle)
    circle = Circle(center, 0.03, facecolor=COLORS['matter'],
                    edgecolor='black', transform=ax_d4.transAxes)
    ax_d4.add_patch(circle)
    ax_d4.text(0.5, 0.08, '6-neighbor stencil', fontsize=9, ha='center',
               transform=ax_d4.transAxes)
    ax_d4.axis('off')

    # =========================================================================
    # Overall title
    # =========================================================================
    fig.suptitle('Continuous vs Discrete Differential Operators',
                fontsize=16, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0.1, 0, 1, 0.96])

    return fig


if __name__ == '__main__':
    fig = generate_discrete_continuous()
    output_path = Path(__file__).parent.parent / 'ch00' / 'fig_2_2_discrete_continuous.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
