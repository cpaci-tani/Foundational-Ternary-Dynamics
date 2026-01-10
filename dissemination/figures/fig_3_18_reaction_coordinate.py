"""
Figure 3.18: Reaction Coordinate
================================
Energy diagram showing reaction progress from
reactants through transition state to products.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS


def generate_reaction_coordinate():
    """Generate the reaction coordinate visualization."""
    fig, ax = plt.subplots(figsize=(12, 8), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Reaction coordinate
    x = np.linspace(0, 10, 200)

    # Energy profile (double well with barrier)
    # Reactant well at x~2, product well at x~8, barrier at x~5
    E = 3 - 2*np.exp(-0.5*(x-2)**2) - 2.5*np.exp(-0.5*(x-8)**2) + 4*np.exp(-0.5*(x-5)**2)

    ax.plot(x, E, color=COLORS['accent1'], linewidth=3)
    ax.fill_between(x, 0, E, alpha=0.1, color=COLORS['accent1'])

    # Mark key points
    # Reactants
    ax.axhline(y=1, xmin=0.1, xmax=0.3, color='black', linewidth=2)
    ax.text(2, 0.5, 'Reactants', fontsize=11, ha='center')

    # Transition state
    ax.plot(5, 5, 'ro', markersize=12)
    ax.text(5, 5.5, 'Transition State', fontsize=11, ha='center', fontweight='bold')

    # Products
    ax.axhline(y=0.5, xmin=0.7, xmax=0.9, color='black', linewidth=2)
    ax.text(8, 0, 'Products', fontsize=11, ha='center')

    # Activation energy arrow
    ax.annotate('', xy=(3.5, 5), xytext=(3.5, 1),
               arrowprops=dict(arrowstyle='<->', color='red', lw=2))
    ax.text(3.2, 3, 'Ea', fontsize=12, color='red', fontweight='bold')

    # Energy change arrow
    ax.annotate('', xy=(7, 0.5), xytext=(7, 1),
               arrowprops=dict(arrowstyle='<->', color='blue', lw=2))
    ax.text(7.3, 0.75, 'Delta H', fontsize=10, color='blue')

    # Labels
    ax.set_xlabel('Reaction Coordinate', fontsize=12)
    ax.set_ylabel('Energy', fontsize=12)
    ax.set_title('Reaction Energy Profile', fontsize=14, fontweight='bold')

    ax.set_xlim(0, 10)
    ax.set_ylim(-0.5, 7)

    # TRD interpretation
    trd_text = (
        "TRD Interpretation:\n\n"
        "- Reactants: stable flux configuration\n"
        "- Transition state: unstable flux saddle point\n"
        "- Products: new stable flux configuration\n"
        "- Ea: flux barrier that must be overcome"
    )
    ax.text(8.5, 5, trd_text, fontsize=9,
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     edgecolor=COLORS['accent1'], linewidth=2))

    # Remove top and right spines
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    plt.tight_layout()
    return fig


if __name__ == '__main__':
    fig = generate_reaction_coordinate()
    output_path = Path(__file__).parent.parent / 'ch03' / 'fig_3_18_reaction_coordinate.png'
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
