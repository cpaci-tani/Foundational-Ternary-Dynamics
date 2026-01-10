"""
Figure 3.30: Large Scale Structure
==================================
Shows cosmic web, filaments, voids, and galaxy
clustering from TRD hierarchical organization.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
from matplotlib.colors import LinearSegmentedColormap
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_large_scale_structure():
    """Generate the large scale structure visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Cosmic web simulation-like
    ax = axes[0, 0]
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Cosmic Web Structure', fontsize=11, fontweight='bold')

    # Create filamentary structure
    np.random.seed(42)

    # Cluster nodes
    n_clusters = 15
    cluster_x = np.random.uniform(10, 90, n_clusters)
    cluster_y = np.random.uniform(10, 90, n_clusters)

    # Draw filaments between nearby clusters
    for i in range(n_clusters):
        for j in range(i+1, n_clusters):
            dist = np.sqrt((cluster_x[i] - cluster_x[j])**2 +
                          (cluster_y[i] - cluster_y[j])**2)
            if dist < 35:  # Connect nearby clusters
                # Draw filament as multiple points
                n_points = int(dist / 2)
                for k in range(n_points):
                    t = k / n_points
                    x = cluster_x[i] + t * (cluster_x[j] - cluster_x[i])
                    y = cluster_y[i] + t * (cluster_y[j] - cluster_y[i])
                    # Add some scatter
                    x += np.random.normal(0, 1)
                    y += np.random.normal(0, 1)
                    size = 10 + 20 * np.exp(-((t - 0.5)**2) / 0.1)
                    ax.scatter(x, y, s=size, c='blue', alpha=0.3)

    # Draw cluster cores
    for i in range(n_clusters):
        # Cluster with galaxies
        for _ in range(20):
            gx = cluster_x[i] + np.random.normal(0, 3)
            gy = cluster_y[i] + np.random.normal(0, 3)
            ax.scatter(gx, gy, s=30, c='yellow', alpha=0.8, edgecolors='orange')

    ax.text(50, -5, 'Each yellow dot = galaxy cluster (~1000 galaxies)',
           fontsize=8, ha='center')

    # Panel 2: Hierarchy of scales
    ax = axes[0, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('Hierarchy of Cosmic Structure', fontsize=11, fontweight='bold')

    # Draw nested boxes with labels
    levels = [
        (0.5, 0.5, 9, 9, 'Observable Universe\n(~93 Gly)', 'lightblue'),
        (1, 1, 8, 7, 'Supercluster\n(~100 Mpc)', 'lightgreen'),
        (2, 1.5, 6, 5, 'Galaxy Cluster\n(~5 Mpc)', 'lightyellow'),
        (3, 2, 4, 3.5, 'Galaxy Group\n(~2 Mpc)', 'lightsalmon'),
        (4, 2.5, 2, 2, 'Galaxy\n(~100 kpc)', 'plum'),
    ]

    for x, y, w, h, label, color in levels:
        rect = plt.Rectangle((x, y), w, h, facecolor=color, edgecolor='black',
                             linewidth=1, alpha=0.5)
        ax.add_patch(rect)
        ax.text(x + w/2, y + h + 0.2, label, fontsize=8, ha='center')

    # Scale bar
    ax.text(5, 0.2, 'Each level ~ 10-100x larger than previous', fontsize=9, ha='center')

    # Panel 3: Void and wall structure
    ax = axes[1, 0]
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Voids and Walls', fontsize=11, fontweight='bold')

    # Create Voronoi-like void structure
    np.random.seed(123)

    # Void centers
    n_voids = 8
    void_x = np.random.uniform(15, 85, n_voids)
    void_y = np.random.uniform(15, 85, n_voids)

    # Draw voids as empty circles
    for vx, vy in zip(void_x, void_y):
        void_r = np.random.uniform(15, 25)
        ax.add_patch(Circle((vx, vy), void_r, facecolor='#1a1a2e', alpha=0.3,
                           edgecolor='none'))

    # Draw galaxies concentrated at void edges (walls)
    for _ in range(500):
        # Random position
        x = np.random.uniform(0, 100)
        y = np.random.uniform(0, 100)

        # Check if in any void
        in_void = False
        for vx, vy in zip(void_x, void_y):
            if np.sqrt((x - vx)**2 + (y - vy)**2) < 15:
                in_void = True
                break

        if not in_void:
            ax.scatter(x, y, s=5, c='yellow', alpha=0.8)

    ax.text(50, -5, 'Galaxies cluster on void boundaries (walls and filaments)',
           fontsize=8, ha='center')

    # Annotations
    ax.annotate('Void', xy=(void_x[0], void_y[0]), xytext=(void_x[0]+20, void_y[0]+15),
               arrowprops=dict(arrowstyle='->', color='white'),
               fontsize=10, color='white')

    # Panel 4: TRD interpretation
    ax = axes[1, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('TRD Large-Scale Structure', fontsize=11, fontweight='bold')

    # Show hierarchical aggregation
    # Single voxel
    ax.add_patch(Circle((1, 8), 0.2, facecolor=COLORS['matter'], edgecolor='black'))
    ax.text(1, 7.3, 'Voxel', fontsize=8, ha='center')

    # Particle
    for i in range(3):
        ax.add_patch(Circle((3 + i*0.3, 8), 0.15, facecolor=COLORS['matter'],
                           edgecolor='black'))
    ax.text(3.3, 7.3, 'Triad', fontsize=8, ha='center')

    # Atom
    ax.add_patch(Circle((6, 8), 0.5, facecolor='yellow', alpha=0.5,
                       edgecolor='black'))
    ax.text(6, 7.3, 'Atom', fontsize=8, ha='center')

    # Star
    ax.add_patch(Circle((8.5, 8), 0.6, facecolor='orange',
                       edgecolor='red', linewidth=2))
    ax.text(8.5, 7.3, 'Star', fontsize=8, ha='center')

    # Arrows
    for x in [1.8, 4.2, 7]:
        ax.annotate('', xy=(x + 0.5, 8), xytext=(x, 8),
                   arrowprops=dict(arrowstyle='->', color='gray', lw=1))

    # Lower row: larger scales
    ax.add_patch(Circle((2, 5), 0.8, facecolor='yellow', alpha=0.3,
                       edgecolor='black'))
    ax.text(2, 3.8, 'Galaxy', fontsize=8, ha='center')

    # Multiple galaxies (cluster)
    for i in range(5):
        angle = i * 2 * np.pi / 5
        r = 0.5
        ax.add_patch(Circle((5.5 + r*np.cos(angle), 5 + r*np.sin(angle)), 0.25,
                           facecolor='yellow', alpha=0.5, edgecolor='black'))
    ax.text(5.5, 3.8, 'Cluster', fontsize=8, ha='center')

    # Cosmic web node
    ax.scatter([8.5], [5], s=200, c='yellow', alpha=0.3)
    ax.plot([7.5, 9.5], [5, 5], 'b-', linewidth=2, alpha=0.5)
    ax.plot([8.5, 8.5], [4, 6], 'b-', linewidth=2, alpha=0.5)
    ax.text(8.5, 3.8, 'Node', fontsize=8, ha='center')

    # Arrows
    ax.annotate('', xy=(3.5, 5), xytext=(3, 5),
               arrowprops=dict(arrowstyle='->', color='gray', lw=1))
    ax.annotate('', xy=(7, 5), xytext=(6.5, 5),
               arrowprops=dict(arrowstyle='->', color='gray', lw=1))

    trd_text = (
        "TRD Hierarchical Structure:\n\n"
        "- Same flux dynamics at all scales\n"
        "- Gravity: density gradient attraction\n"
        "- Larger scales = more time to aggregate\n\n"
        "Structure formation:\n"
        "1. Initial flux density fluctuations\n"
        "2. Gravitational amplification\n"
        "3. Hierarchical clustering\n"
        "4. Void expansion (low density)\n\n"
        "The cosmic web is the natural\n"
        "endpoint of flux gradient dynamics\n"
        "acting over cosmic time."
    )
    ax.text(5, 2.5, trd_text, fontsize=8, ha='center', va='top',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    fig.suptitle('Large-Scale Structure: The Cosmic Web',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_large_scale_structure()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_30_large_scale_structure.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
