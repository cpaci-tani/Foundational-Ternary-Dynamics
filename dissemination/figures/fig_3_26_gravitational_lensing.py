"""
Figure 3.26: Gravitational Lensing
==================================
Shows light bending around massive objects
from TRD flux gradient perspective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge, Arc
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_gravitational_lensing():
    """Generate the gravitational lensing visualization."""
    fig, axes = plt.subplots(1, 3, figsize=(15, 5), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Light bending around mass
    ax = axes[0]
    ax.set_xlim(-8, 8)
    ax.set_ylim(-5, 5)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Light Deflection by Mass', fontsize=11, fontweight='bold')

    # Central mass (lens)
    ax.add_patch(Circle((0, 0), 1, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=2))
    ax.text(0, 0, 'M', fontsize=12, ha='center', va='center', color='white')

    # Light rays bending
    for y_start in [-3, -1.5, 1.5, 3]:
        # Incoming ray (straight)
        ax.plot([-7, -3], [y_start, y_start], 'y-', linewidth=2)

        # Bent path around mass
        t = np.linspace(-3, 3, 50)
        # Simple approximation of bending
        bend = 0.8 * np.sign(y_start) / (1 + abs(y_start)/2)
        y = y_start + bend * np.exp(-t**2/3)
        ax.plot(t, y, 'y-', linewidth=2)

        # Outgoing ray
        y_end = y_start + 2 * bend
        ax.plot([3, 7], [y[-1], y_end], 'y-', linewidth=2)

    # Observer
    ax.plot(7, 0, 'ko', markersize=10)
    ax.text(7, -0.8, 'Observer', fontsize=9, ha='center')

    # Source
    ax.plot(-7, 0, 'y*', markersize=15)
    ax.text(-7, -0.8, 'Source', fontsize=9, ha='center')

    ax.text(0, -4.5, 'Deflection angle = 4GM / (c^2 b)', fontsize=9, ha='center')

    # Panel 2: Einstein ring
    ax = axes[1]
    ax.set_xlim(-5, 5)
    ax.set_ylim(-5, 5)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Einstein Ring', fontsize=11, fontweight='bold')

    # Central lens (galaxy)
    ax.add_patch(Circle((0, 0), 0.8, facecolor=COLORS['matter'],
                       edgecolor='black', linewidth=2))
    ax.text(0, 0, 'Lens', fontsize=9, ha='center', va='center', color='white')

    # Einstein ring (distorted source image)
    ring = plt.Circle((0, 0), 2.5, fill=False, edgecolor='yellow',
                      linewidth=4)
    ax.add_patch(ring)

    ax.text(0, 3.5, 'Einstein radius:', fontsize=9, ha='center')
    ax.text(0, 3, 'theta_E = sqrt(4GM D_LS / (c^2 D_L D_S))', fontsize=8, ha='center')

    ax.text(0, -4, 'Perfect alignment creates\ncomplete ring', fontsize=9, ha='center')

    # Panel 3: Microlensing light curve
    ax = axes[2]

    t = np.linspace(-3, 3, 200)

    # Magnification for point source, point lens
    u0 = 0.3  # Impact parameter
    u = np.sqrt(u0**2 + t**2)
    A = (u**2 + 2) / (u * np.sqrt(u**2 + 4))

    ax.plot(t, A, 'b-', linewidth=2)
    ax.axhline(y=1, color='gray', linestyle='--', alpha=0.5)

    ax.fill_between(t, 1, A, alpha=0.2, color='blue')

    ax.set_xlabel('Time (relative to closest approach)', fontsize=10)
    ax.set_ylabel('Magnification A', fontsize=10)
    ax.set_title('Microlensing Light Curve', fontsize=11, fontweight='bold')

    ax.text(0, 3, f'u_0 = {u0}', fontsize=10, ha='center')
    ax.text(0, 2.5, 'A_max = {:.2f}'.format((u0**2 + 2)/(u0 * np.sqrt(u0**2 + 4))),
           fontsize=10, ha='center')

    ax.set_xlim(-3, 3)
    ax.set_ylim(0.5, 4)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    fig.suptitle('Gravitational Lensing: Light Bent by Mass',
                fontsize=14, fontweight='bold', y=0.98)

    # TRD interpretation
    fig.text(0.5, 0.02, 'TRD: Mass creates flux density gradient that '
            'bends photon trajectories (null geodesics in flux field).',
            fontsize=10, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor='gray'))

    plt.tight_layout(rect=[0, 0.08, 1, 0.92])
    return fig


if __name__ == '__main__':
    fig = generate_gravitational_lensing()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_26_gravitational_lensing.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
