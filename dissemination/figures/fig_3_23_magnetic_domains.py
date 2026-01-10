"""
Figure 3.23: Magnetic Domains
=============================
Shows domain formation, walls, and hysteresis
from TRD flux alignment perspective.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, FancyArrowPatch, Circle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_magnetic_domains():
    """Generate the magnetic domains visualization."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Panel 1: Domain structure
    ax = axes[0, 0]
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 8)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Magnetic Domain Structure', fontsize=11, fontweight='bold')

    # Draw domains with different magnetization directions
    domains = [
        (0, 0, 4, 4, 0),      # Domain 1: up
        (4, 0, 4, 4, 180),    # Domain 2: down
        (8, 0, 4, 4, 90),     # Domain 3: right
        (0, 4, 6, 4, 45),     # Domain 4: diagonal
        (6, 4, 6, 4, -45),    # Domain 5: other diagonal
    ]

    colors = ['#FFB3B3', '#B3B3FF', '#B3FFB3', '#FFFFB3', '#FFB3FF']

    for (x, y, w, h, angle), color in zip(domains, colors):
        rect = Rectangle((x, y), w, h, facecolor=color,
                         edgecolor='black', linewidth=2)
        ax.add_patch(rect)

        # Draw arrows showing magnetization
        cx, cy = x + w/2, y + h/2
        dx = 0.8 * np.cos(np.radians(angle))
        dy = 0.8 * np.sin(np.radians(angle))

        for i in range(3):
            for j in range(3):
                ax.annotate('', xy=(x + w*(i+1)/4 + dx*0.3, y + h*(j+1)/4 + dy*0.3),
                           xytext=(x + w*(i+1)/4 - dx*0.3, y + h*(j+1)/4 - dy*0.3),
                           arrowprops=dict(arrowstyle='->', color='black', lw=1))

    ax.text(6, -0.5, 'Different colors = different magnetization directions',
            fontsize=9, ha='center')

    # Panel 2: Domain wall (Bloch wall)
    ax = axes[0, 1]
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 8)
    ax.set_aspect('equal')
    ax.axis('off')
    ax.set_title('Bloch Domain Wall', fontsize=11, fontweight='bold')

    # Left domain (up)
    rect1 = Rectangle((0, 2), 4, 4, facecolor='#FFB3B3',
                      edgecolor='black', linewidth=2)
    ax.add_patch(rect1)

    # Right domain (down)
    rect2 = Rectangle((8, 2), 4, 4, facecolor='#B3B3FF',
                      edgecolor='black', linewidth=2)
    ax.add_patch(rect2)

    # Wall region (gradual rotation)
    wall = Rectangle((4, 2), 4, 4, facecolor='#E0E0E0',
                     edgecolor='black', linewidth=2)
    ax.add_patch(wall)

    # Draw rotating spins in wall
    n_spins = 8
    for i in range(n_spins):
        x = 4.5 + i * 3/8
        angle = 90 - i * 180 / (n_spins - 1)  # Rotate from up to down
        dx = 0.3 * np.cos(np.radians(angle))
        dy = 0.3 * np.sin(np.radians(angle))

        for j in range(3):
            y = 3 + j * 1.2
            ax.annotate('', xy=(x + dx, y + dy), xytext=(x - dx, y - dy),
                       arrowprops=dict(arrowstyle='->', color='red', lw=1.5))

    # Labels
    ax.text(2, 6.5, 'Domain 1', fontsize=10, ha='center')
    ax.text(10, 6.5, 'Domain 2', fontsize=10, ha='center')
    ax.text(6, 6.5, 'Wall', fontsize=10, ha='center')
    ax.text(6, 1.5, 'Spins rotate gradually across wall (width ~ 100 nm)',
            fontsize=9, ha='center')

    # Panel 3: Hysteresis loop
    ax = axes[1, 0]

    # Generate hysteresis curve
    H = np.linspace(-2, 2, 500)

    # Upper branch (increasing H)
    M_up = np.tanh(2 * (H + 0.5))

    # Lower branch (decreasing H)
    M_down = np.tanh(2 * (H - 0.5))

    ax.plot(H[H <= 0], M_up[H <= 0], 'b-', linewidth=2)
    ax.plot(H[H >= 0], M_down[H >= 0], 'b-', linewidth=2)
    ax.plot(H[H >= 0], M_up[H >= 0], 'r-', linewidth=2)
    ax.plot(H[H <= 0], M_down[H <= 0], 'r-', linewidth=2)

    ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.5)
    ax.axvline(x=0, color='gray', linestyle='-', linewidth=0.5)

    # Mark key points
    ax.plot(0, np.tanh(1), 'ko', markersize=8)
    ax.annotate('Mr', xy=(0.1, np.tanh(1)), fontsize=10)

    ax.plot(-0.5, 0, 'ko', markersize=8)
    ax.annotate('-Hc', xy=(-0.7, 0.15), fontsize=10)

    ax.plot(0.5, 0, 'ko', markersize=8)
    ax.annotate('Hc', xy=(0.55, 0.15), fontsize=10)

    ax.set_xlabel('Applied Field H', fontsize=10)
    ax.set_ylabel('Magnetization M', fontsize=10)
    ax.set_title('Hysteresis Loop', fontsize=11, fontweight='bold')
    ax.set_xlim(-2, 2)
    ax.set_ylim(-1.5, 1.5)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    ax.text(1.3, -1.2, 'Mr = Remanence\nHc = Coercivity', fontsize=9)

    # Panel 4: TRD interpretation
    ax = axes[1, 1]
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('TRD View of Magnetic Domains', fontsize=11, fontweight='bold')

    # Draw flux alignment visualization
    # Aligned domain
    for i in range(4):
        for j in range(3):
            x = 1.5 + i * 0.8
            y = 7 + j * 0.8
            ax.annotate('', xy=(x + 0.25, y), xytext=(x - 0.25, y),
                       arrowprops=dict(arrowstyle='->', color=COLORS['matter'], lw=2))

    ax.add_patch(Rectangle((1, 6.5), 4, 3, fill=False,
                           edgecolor='black', linewidth=2))
    ax.text(3, 6, 'Aligned flux (domain)', fontsize=9, ha='center')

    # Domain wall as flux rotation
    for i in range(6):
        x = 6 + i * 0.5
        angle = i * 30  # Rotate 0 to 150 degrees
        y = 8
        dx = 0.25 * np.cos(np.radians(angle))
        dy = 0.25 * np.sin(np.radians(angle))
        ax.annotate('', xy=(x + dx, y + dy), xytext=(x - dx, y - dy),
                   arrowprops=dict(arrowstyle='->', color='purple', lw=2))

    ax.text(7.5, 6, 'Flux rotation\n(wall)', fontsize=9, ha='center', color='purple')

    trd_text = (
        "TRD Magnetism:\n\n"
        "- Spin = intrinsic flux rotation\n"
        "- Domain = region of aligned flux\n"
        "- Wall = gradual flux rotation interface\n"
        "- Exchange: parallel flux preferred\n"
        "- Anisotropy: flux prefers lattice axes\n"
        "- Hysteresis: domain wall pinning\n\n"
        "Domain formation minimizes:\n"
        "E_total = E_exchange + E_anisotropy\n"
        "         + E_magnetostatic + E_wall"
    )
    ax.text(5, 4, trd_text, fontsize=9, ha='center', va='top',
            bbox=dict(boxstyle='round,pad=0.4', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    fig.suptitle('Magnetic Domains: Collective Flux Alignment',
                fontsize=14, fontweight='bold')

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_magnetic_domains()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_23_magnetic_domains.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
