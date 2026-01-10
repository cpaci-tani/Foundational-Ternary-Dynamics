"""
Figure 3.27: Black Holes
========================
Shows event horizon, singularity concepts, and Hawking
radiation from TRD flux perspective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge, FancyArrowPatch
from matplotlib.colors import LinearSegmentedColormap
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_black_holes():
    """Generate the black hole visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 12), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Black hole anatomy
    ax = axes[0, 0]
    ax.set_xlim(-6, 6)
    ax.set_ylim(-6, 6)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Black Hole Structure', fontsize=11, fontweight='bold')

    # Singularity (center)
    ax.add_patch(Circle((0, 0), 0.3, facecolor='black', edgecolor='white',
                       linewidth=2))
    ax.text(0, 0, '?', fontsize=12, ha='center', va='center', color='white')

    # Event horizon
    event_horizon = Circle((0, 0), 2, fill=False, edgecolor='red',
                          linewidth=3, linestyle='-')
    ax.add_patch(event_horizon)
    ax.text(2.3, 0, 'Event\nHorizon', fontsize=9, color='red')

    # Photon sphere
    photon_sphere = Circle((0, 0), 3, fill=False, edgecolor='yellow',
                          linewidth=2, linestyle='--')
    ax.add_patch(photon_sphere)
    ax.text(3.3, 0, 'Photon\nSphere', fontsize=9, color='orange')

    # Accretion disk (simplified)
    for r in np.linspace(3.5, 5.5, 10):
        alpha = 0.3 * (1 - (r - 3.5) / 2)
        ellipse = plt.matplotlib.patches.Ellipse(
            (0, 0), 2*r, 0.5*r, facecolor='orange', alpha=alpha)
        ax.add_patch(ellipse)

    # Infalling matter
    for angle in [45, 135, 225, 315]:
        x1 = 5 * np.cos(np.radians(angle))
        y1 = 5 * np.sin(np.radians(angle))
        x2 = 2.2 * np.cos(np.radians(angle))
        y2 = 2.2 * np.sin(np.radians(angle))
        ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                   arrowprops=dict(arrowstyle='->', color='cyan', lw=1.5))

    ax.text(0, -5.5, 'r_s = 2GM/c^2 (Schwarzschild radius)', fontsize=9, ha='center')

    # Panel 2: Escape velocity vs radius
    ax = axes[0, 1]

    r = np.linspace(0.5, 5, 100)
    r_s = 1  # Schwarzschild radius = 1

    # Newtonian escape velocity (v/c)
    v_escape = np.sqrt(r_s / r)

    ax.plot(r, v_escape, 'b-', linewidth=2, label='v_escape / c')
    ax.axhline(y=1, color='red', linestyle='--', linewidth=2, label='Speed of light')
    ax.axvline(x=r_s, color='gray', linestyle=':', linewidth=2)

    ax.fill_between(r[r < r_s], 0, v_escape[r < r_s], color='black', alpha=0.3)
    ax.text(0.75, 0.5, 'Nothing\nescapes', fontsize=9, ha='center', color='white')

    ax.set_xlabel('Radius r / r_s', fontsize=10)
    ax.set_ylabel('Escape velocity v / c', fontsize=10)
    ax.set_title('Escape Velocity Profile', fontsize=11, fontweight='bold')
    ax.legend(loc='upper right', fontsize=9)
    ax.set_xlim(0, 5)
    ax.set_ylim(0, 2)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    # Panel 3: Hawking radiation
    ax = axes[1, 0]
    ax.set_xlim(-5, 5)
    ax.set_ylim(-5, 5)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Hawking Radiation', fontsize=11, fontweight='bold')

    # Black hole
    ax.add_patch(Circle((0, 0), 2, facecolor='black', edgecolor='white',
                       linewidth=2))

    # Virtual pair creation near horizon
    pair_positions = [(2.2, 1), (2.1, -0.5), (2.3, 0.3), (-2.2, 0.8)]

    for x, y in pair_positions:
        # Pair created
        ax.add_patch(Circle((x, y), 0.15, facecolor=COLORS['matter'],
                           edgecolor='black', linewidth=1))
        ax.add_patch(Circle((x - 0.3, y), 0.15, facecolor=COLORS['antimatter'],
                           edgecolor='black', linewidth=1))

        # One falls in, one escapes
        if x > 0:
            ax.annotate('', xy=(x + 1, y + 0.5), xytext=(x + 0.2, y),
                       arrowprops=dict(arrowstyle='->', color='red', lw=1))
            ax.annotate('', xy=(1.5, y - 0.3), xytext=(x - 0.4, y),
                       arrowprops=dict(arrowstyle='->', color='blue', lw=1))

    ax.text(3.5, 2, 'Escaping\nradiation', fontsize=9, ha='center', color='red')
    ax.text(0, 3.5, 'Virtual pairs near horizon:\none escapes, one falls in',
           fontsize=9, ha='center')

    ax.text(0, -4, 'T = hbar c^3 / (8 pi G M k_B)', fontsize=9, ha='center')
    ax.text(0, -4.7, '(Temperature inversely proportional to mass)', fontsize=8, ha='center')

    # Panel 4: TRD interpretation
    ax = axes[1, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('TRD View of Black Holes', fontsize=11, fontweight='bold')

    # Flux density visualization
    for r in np.linspace(4, 0.5, 15):
        alpha = 0.1 + 0.4 * (1 - r/4)
        ax.add_patch(Circle((3, 5), r, facecolor='purple', alpha=alpha))

    ax.text(3, 5, 'Maximum\nflux density', fontsize=8, ha='center', va='center',
           color='white')

    # Arrows showing flux gradient
    for angle in np.linspace(0, 2*np.pi, 8, endpoint=False):
        x1 = 3 + 4 * np.cos(angle)
        y1 = 5 + 4 * np.sin(angle)
        x2 = 3 + 2 * np.cos(angle)
        y2 = 5 + 2 * np.sin(angle)
        ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                   arrowprops=dict(arrowstyle='->', color=COLORS['accent1'], lw=1.5))

    trd_text = (
        "TRD Black Hole Model:\n\n"
        "- Event horizon: flux density exceeds\n"
        "  maximum propagation capacity\n\n"
        "- Singularity: undefined in TRD\n"
        "  (lattice prevents true singularity)\n\n"
        "- Hawking radiation: vacuum flux\n"
        "  fluctuations at horizon boundary\n\n"
        "- Information: encoded in boundary\n"
        "  flux configuration (holographic)\n\n"
        "- No true 'black' - always some\n"
        "  flux leakage at Planck scale"
    )
    ax.text(8, 5, trd_text, fontsize=8, ha='left', va='center',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    fig.suptitle('Black Holes: Extreme Flux Concentration',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_black_holes()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_27_black_holes.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
