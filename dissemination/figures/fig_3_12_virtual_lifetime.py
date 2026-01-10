"""
Figure 3.12: Virtual Particle Lifetime
======================================
Shows the relationship between virtual particle energy
and lifetime (Heisenberg uncertainty).
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_virtual_lifetime():
    """Generate the virtual particle lifetime visualization."""
    fig, (ax_left, ax_right) = plt.subplots(1, 2, figsize=(14, 6), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Left: Energy-time uncertainty plot
    ax = ax_left
    E = np.linspace(0.1, 10, 100)
    # delta_t ~ hbar / delta_E
    hbar = 1  # Natural units
    delta_t = hbar / E

    ax.loglog(E, delta_t, color=COLORS['accent1'], linewidth=2.5)
    ax.fill_between(E, delta_t, alpha=0.2, color=COLORS['accent1'])

    ax.set_xlabel('Energy (arbitrary units)', fontsize=11)
    ax.set_ylabel('Lifetime (arbitrary units)', fontsize=11)
    ax.set_title('Energy-Time Uncertainty Relation', fontsize=12, fontweight='bold')

    # Mark some key points
    points = [(1, 1, 'Virtual photon'), (0.5, 2, 'Low energy'), (5, 0.2, 'High energy')]
    for x, y, label in points:
        ax.plot(x, y, 'ko', markersize=8)
        ax.annotate(label, xy=(x, y), xytext=(x*1.5, y*1.5),
                   fontsize=9, arrowprops=dict(arrowstyle='->', color='gray'))

    ax.grid(True, alpha=0.3, which='both')
    ax.text(0.5, 0.1, r'$\Delta E \cdot \Delta t \geq \hbar/2$', fontsize=12,
            transform=ax.transAxes,
            bbox=dict(boxstyle='round,pad=0.3', facecolor='white'))

    # Right: Visual timeline
    ax = ax_right
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 6)
    ax.axis('off')
    ax.set_title('Virtual Particle Lifetimes', fontsize=12, fontweight='bold')

    # Time arrow
    ax.annotate('', xy=(9.5, 0.5), xytext=(0.5, 0.5),
               arrowprops=dict(arrowstyle='->', color='black', lw=2))
    ax.text(5, 0.1, 'Time', fontsize=10, ha='center')

    # Different virtual particles with different lifetimes
    particles = [
        ('W/Z boson', 0.3, 5.5, COLORS['matter']),
        ('Gluon', 0.8, 4.5, '#44AA44'),
        ('Virtual e+e-', 1.5, 3.5, COLORS['antimatter']),
        ('Virtual photon', 3.0, 2.5, COLORS['highlight']),
        ('Low-energy fluct.', 6.0, 1.5, COLORS['void']),
    ]

    for name, width, y, color in particles:
        # Lifetime bar
        ax.add_patch(Rectangle((1, y - 0.2), width * 1.2, 0.4,
                               facecolor=color, edgecolor='black', linewidth=1))
        ax.text(1 + width * 0.6, y, name, fontsize=9, ha='center', va='center',
                color='white' if color != COLORS['void'] else 'black',
                fontweight='bold')

        # Mass/energy label
        if 'W/Z' in name:
            ax.text(0.8, y, '~80 GeV', fontsize=8, ha='right', va='center')
        elif 'Gluon' in name:
            ax.text(0.8, y, '~1 GeV', fontsize=8, ha='right', va='center')
        elif 'e+e-' in name:
            ax.text(0.8, y, '~1 MeV', fontsize=8, ha='right', va='center')
        elif 'photon' in name:
            ax.text(0.8, y, '~keV', fontsize=8, ha='right', va='center')
        else:
            ax.text(0.8, y, '~eV', fontsize=8, ha='right', va='center')

    ax.text(5, 5.8, 'Higher energy = Shorter lifetime', fontsize=11,
            ha='center', fontweight='bold')

    fig.suptitle('Virtual Particles: Borrowed Energy, Limited Time',
                fontsize=14, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    return fig


if __name__ == '__main__':
    fig = generate_virtual_lifetime()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_3_12_virtual_lifetime.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
