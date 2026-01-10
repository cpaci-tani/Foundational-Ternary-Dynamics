"""
Figure 3.28: Dark Matter
========================
Shows evidence for dark matter and potential TRD
interpretations as non-luminous flux structures.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_dark_matter():
    """Generate the dark matter visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Galaxy rotation curves
    ax = axes[0, 0]

    r = np.linspace(0.1, 15, 100)

    # Keplerian expectation (v ~ 1/sqrt(r))
    v_keplerian = 200 / np.sqrt(r)

    # Observed (flat rotation curve)
    v_observed = 200 * np.ones_like(r)
    v_observed[:20] = 200 * r[:20] / r[19]  # Rising in center

    # With dark matter halo
    v_halo = 150 * r / np.sqrt(r**2 + 5**2)
    v_disk = 200 / np.sqrt(r + 0.5)
    v_total = np.sqrt(v_halo**2 + v_disk**2 * 0.5)

    ax.plot(r, v_keplerian, 'b--', linewidth=2, label='Expected (visible matter)')
    ax.plot(r, v_observed, 'r-', linewidth=2, label='Observed')
    ax.fill_between(r, v_keplerian, v_observed, alpha=0.2, color='purple',
                   label='Dark matter contribution')

    ax.set_xlabel('Radius from galactic center (kpc)', fontsize=10)
    ax.set_ylabel('Rotation velocity (km/s)', fontsize=10)
    ax.set_title('Galaxy Rotation Curves', fontsize=11, fontweight='bold')
    ax.legend(loc='upper right', fontsize=8)
    ax.set_xlim(0, 15)
    ax.set_ylim(0, 300)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    # Panel 2: Dark matter halo
    ax = axes[0, 1]
    ax.set_xlim(-8, 8)
    ax.set_ylim(-8, 8)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Dark Matter Halo Structure', fontsize=11, fontweight='bold')

    # Dark matter halo (outer diffuse)
    for r in np.linspace(7, 3, 20):
        alpha = 0.05 + 0.1 * (1 - r/7)
        ax.add_patch(Circle((0, 0), r, facecolor='purple', alpha=alpha))

    # Galaxy (inner bright region)
    ax.add_patch(Circle((0, 0), 2, facecolor='yellow', alpha=0.8))

    # Spiral arms suggestion
    theta = np.linspace(0, 4*np.pi, 200)
    r_spiral = 0.5 + 0.3 * theta
    for offset in [0, np.pi]:
        x = r_spiral * np.cos(theta + offset) * 0.8
        y = r_spiral * np.sin(theta + offset) * 0.8
        valid = r_spiral < 2
        ax.plot(x[valid], y[valid], 'orange', linewidth=2, alpha=0.7)

    ax.text(0, -7.5, 'Visible galaxy embedded in\nmuch larger dark matter halo',
           fontsize=9, ha='center')

    # Annotations
    ax.annotate('Visible matter\n(~5%)', xy=(0, 0), xytext=(4, 3),
               arrowprops=dict(arrowstyle='->', color='black'),
               fontsize=9, ha='center')
    ax.annotate('Dark matter\nhalo (~27%)', xy=(5, 0), xytext=(6, -4),
               arrowprops=dict(arrowstyle='->', color='purple'),
               fontsize=9, ha='center', color='purple')

    # Panel 3: Gravitational lensing evidence
    ax = axes[1, 0]
    ax.set_xlim(-6, 6)
    ax.set_ylim(-6, 6)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Lensing Evidence for Dark Matter', fontsize=11, fontweight='bold')

    # Galaxy cluster (visible)
    cluster_positions = [(0, 0), (1, 0.5), (-0.5, -0.3), (0.3, -0.5)]
    for x, y in cluster_positions:
        ax.add_patch(Circle((x, y), 0.3, facecolor='orange', alpha=0.8))

    # Dark matter concentration (inferred from lensing)
    ax.add_patch(Circle((0, 0), 3, facecolor='purple', alpha=0.15))
    ax.text(0, 3.5, 'Dark matter distribution\n(from lensing)', fontsize=9,
           ha='center', color='purple')

    # Distorted background galaxies
    bg_angles = np.linspace(0, 2*np.pi, 12, endpoint=False)
    for angle in bg_angles:
        x = 4.5 * np.cos(angle)
        y = 4.5 * np.sin(angle)

        # Tangential stretching (weak lensing shear)
        stretch_angle = angle + np.pi/2
        dx = 0.4 * np.cos(stretch_angle)
        dy = 0.4 * np.sin(stretch_angle)

        ellipse = plt.matplotlib.patches.Ellipse(
            (x, y), 0.8, 0.3, angle=np.degrees(stretch_angle),
            facecolor='cyan', alpha=0.6)
        ax.add_patch(ellipse)

    ax.text(0, -5.5, 'Background galaxies stretched\ntangentially by lensing',
           fontsize=9, ha='center')

    # Panel 4: TRD interpretation
    ax = axes[1, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('TRD Dark Matter Hypotheses', fontsize=11, fontweight='bold')

    # Visualization: non-luminous flux structures
    # Show void-state regions with flux but no manifestation

    for i in range(5):
        for j in range(5):
            x = 1 + i * 1.2
            y = 6 + j * 0.8

            # Some voxels with flux but state=0
            if (i + j) % 3 == 0:
                # Void with flux (dark matter candidate)
                ax.add_patch(Circle((x, y), 0.2, facecolor='purple', alpha=0.4,
                                   edgecolor='purple', linestyle='--'))
            else:
                # Normal void
                ax.add_patch(Circle((x, y), 0.2, facecolor=COLORS['void'],
                                   alpha=0.3))

    ax.text(4, 5.2, 'Purple: flux-carrying void\n(gravitates but does not radiate)',
           fontsize=9, ha='center', color='purple')

    trd_text = (
        "TRD Dark Matter Possibilities:\n\n"
        "1. Void-state flux:\n"
        "   - Voxels with flux density < KB\n"
        "   - Gravitates but doesn't manifest\n"
        "   - No EM interaction (no charge)\n\n"
        "2. Coherent flux structures:\n"
        "   - Stable non-manifesting patterns\n"
        "   - Similar to solitons in void\n\n"
        "3. Relict flux from early universe:\n"
        "   - Never reached manifestation\n"
        "   - Frozen in expansion\n\n"
        "Key prediction: DM interacts via\n"
        "flux gradient (gravity) only"
    )
    ax.text(5, 3.5, trd_text, fontsize=8, ha='center', va='top',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor='purple', linewidth=2))

    fig.suptitle('Dark Matter: Evidence and TRD Interpretation',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_dark_matter()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_28_dark_matter.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
