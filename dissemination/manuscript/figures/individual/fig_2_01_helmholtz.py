"""
Figure 2.1: Helmholtz Decomposition
===================================
Visualizes how any vector field J can be decomposed into:
- J_L (longitudinal/curl-free component): ∇ × J_L = 0
- J_T (transverse/divergence-free component): ∇ · J_T = 0

J = J_L + J_T

This is fundamental to understanding gauge symmetry emergence in TRD.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Circle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_helmholtz():
    """
    Generate the Helmholtz decomposition visualization.

    Shows three panels:
    1. Original field J
    2. Longitudinal component J_L (radial/irrotational)
    3. Transverse component J_T (circular/solenoidal)
    """
    fig, axes = plt.subplots(1, 3, figsize=(15, 5), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Grid for vector field
    x = np.linspace(-2, 2, 12)
    y = np.linspace(-2, 2, 12)
    X, Y = np.meshgrid(x, y)

    # =========================================================================
    # Panel 1: Original field J (combination)
    # =========================================================================
    ax1 = axes[0]

    # Combined field: radial + rotational
    r = np.sqrt(X**2 + Y**2) + 0.1
    # Longitudinal (radial outward)
    Jx_L = X / r**2
    Jy_L = Y / r**2
    # Transverse (rotational)
    Jx_T = -Y / r**1.5
    Jy_T = X / r**1.5

    # Combined
    Jx = Jx_L + Jx_T
    Jy = Jy_L + Jy_T

    # Normalize for display
    mag = np.sqrt(Jx**2 + Jy**2)
    Jx_norm = Jx / (mag + 0.1)
    Jy_norm = Jy / (mag + 0.1)

    ax1.quiver(X, Y, Jx_norm, Jy_norm, mag, cmap='viridis',
               scale=25, width=0.008, alpha=0.8)
    ax1.set_xlim(-2.5, 2.5)
    ax1.set_ylim(-2.5, 2.5)
    ax1.set_aspect('equal')
    ax1.set_title(r'Original Field $\mathbf{J}$', fontsize=14, fontweight='bold')
    ax1.set_xlabel('x')
    ax1.set_ylabel('y')

    # Add equation
    ax1.text(0, -2.2, r'$\mathbf{J} = \mathbf{J}_L + \mathbf{J}_T$',
             fontsize=12, ha='center',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='white', alpha=0.9))

    # =========================================================================
    # Panel 2: Longitudinal component J_L (curl-free)
    # =========================================================================
    ax2 = axes[1]

    # Normalize
    mag_L = np.sqrt(Jx_L**2 + Jy_L**2)
    Jx_L_norm = Jx_L / (mag_L + 0.1)
    Jy_L_norm = Jy_L / (mag_L + 0.1)

    ax2.quiver(X, Y, Jx_L_norm, Jy_L_norm, mag_L, cmap='Reds',
               scale=25, width=0.008, alpha=0.8)

    # Add source point at center
    ax2.scatter([0], [0], c=COLORS['matter'], s=200, zorder=10,
                edgecolors='black', linewidths=2, marker='o')
    ax2.text(0.2, 0.2, 'Source', fontsize=10)

    ax2.set_xlim(-2.5, 2.5)
    ax2.set_ylim(-2.5, 2.5)
    ax2.set_aspect('equal')
    ax2.set_title(r'Longitudinal $\mathbf{J}_L$', fontsize=14,
                  fontweight='bold', color=COLORS['matter'])
    ax2.set_xlabel('x')

    # Properties
    props_L = r'$\nabla \times \mathbf{J}_L = 0$' + '\n' + r'(curl-free / irrotational)'
    ax2.text(0, -2.2, props_L, fontsize=10, ha='center',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='#ffcccc', alpha=0.9))

    # =========================================================================
    # Panel 3: Transverse component J_T (divergence-free)
    # =========================================================================
    ax3 = axes[2]

    # Normalize
    mag_T = np.sqrt(Jx_T**2 + Jy_T**2)
    Jx_T_norm = Jx_T / (mag_T + 0.1)
    Jy_T_norm = Jy_T / (mag_T + 0.1)

    ax3.quiver(X, Y, Jx_T_norm, Jy_T_norm, mag_T, cmap='Blues',
               scale=25, width=0.008, alpha=0.8)

    # Add circulation indicator
    theta = np.linspace(0, 2*np.pi, 100)
    circle_r = 1.0
    ax3.plot(circle_r * np.cos(theta), circle_r * np.sin(theta),
             'b--', linewidth=2, alpha=0.5)
    ax3.annotate('', xy=(0, 1.0), xytext=(0.3, 0.95),
                arrowprops=dict(arrowstyle='->', color='blue', lw=2))

    ax3.set_xlim(-2.5, 2.5)
    ax3.set_ylim(-2.5, 2.5)
    ax3.set_aspect('equal')
    ax3.set_title(r'Transverse $\mathbf{J}_T$', fontsize=14,
                  fontweight='bold', color=COLORS['antimatter'])
    ax3.set_xlabel('x')

    # Properties
    props_T = r'$\nabla \cdot \mathbf{J}_T = 0$' + '\n' + r'(divergence-free / solenoidal)'
    ax3.text(0, -2.2, props_T, fontsize=10, ha='center',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='#ccccff', alpha=0.9))

    # =========================================================================
    # Overall annotations
    # =========================================================================
    fig.suptitle('Helmholtz Decomposition of Vector Fields',
                fontsize=16, fontweight='bold', y=1.02)

    # Explanation
    explanation = (
        "Any vector field can be uniquely decomposed into curl-free and divergence-free parts.\n"
        "In TRD, this explains U(1) gauge symmetry: J_L is constrained by charge, J_T has 2 physical modes (photon polarizations)."
    )
    fig.text(0.5, -0.02, explanation, ha='center', fontsize=10,
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                      edgecolor='gray', alpha=0.9))

    plt.tight_layout(rect=[0, 0.05, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_helmholtz()
    output_path = Path(__file__).parent.parent / 'ch00' / 'fig_2_1_helmholtz.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
