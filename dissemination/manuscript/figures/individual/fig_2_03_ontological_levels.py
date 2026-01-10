"""
Figure 2.3: Ontological Levels
==============================
Visualizes the graded monism hierarchy in TRD:
Void (substrate) → Flux (disposition) → Manifestation (actualized)

This represents the three-layer ontology where:
- Void is the foundational substance
- Flux represents dispositional properties (potentialities)
- Manifestation is the actualization of those dispositions
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Circle, Ellipse
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, apply_trd_style


def generate_ontological_levels():
    """
    Generate the ontological levels hierarchy diagram.

    Shows three layers with increasing specificity:
    1. Void (foundational substrate)
    2. Flux (dispositional field)
    3. Manifestation (actualized states)
    """
    fig, ax = plt.subplots(figsize=(12, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis('off')

    # Layer positions (bottom to top)
    layers = [
        {
            'y': 0.15,
            'height': 0.2,
            'name': 'VOID',
            'subtitle': 'Foundational Substrate',
            'color': '#e0e0e0',
            'border': COLORS['void'],
            'description': [
                'State: s = 0',
                'Present but null',
                'Awaiting activation',
                'The "ditto" of ontology',
            ],
            'analogy': 'Like a blank canvas or stem cell',
        },
        {
            'y': 0.42,
            'height': 0.2,
            'name': 'FLUX',
            'subtitle': 'Dispositional Field',
            'color': '#fff4cc',
            'border': COLORS['highlight'],
            'description': [
                r'$\mathbf{J} \in \mathbb{R}^3$',
                'Energy density: |J|',
                'Tendencies of substrate',
                'Potentialities encoded',
            ],
            'analogy': 'Like a wave function or probability field',
        },
        {
            'y': 0.69,
            'height': 0.2,
            'name': 'MANIFESTATION',
            'subtitle': 'Actualized States',
            'color': '#ffe0e0',
            'border': COLORS['matter'],
            'description': [
                'States: s = +1 or -1',
                'Observable particles',
                'Definite properties',
                'Discrete, countable',
            ],
            'analogy': 'Like collapsed wave function or differentiated cell',
        },
    ]

    # Draw layers
    for i, layer in enumerate(layers):
        # Main box
        box = FancyBboxPatch(
            (0.15, layer['y']), 0.7, layer['height'],
            boxstyle="round,pad=0.02",
            facecolor=layer['color'],
            edgecolor=layer['border'],
            linewidth=3,
            zorder=2
        )
        ax.add_patch(box)

        # Name and subtitle
        ax.text(0.5, layer['y'] + layer['height'] - 0.03,
                layer['name'], fontsize=16, fontweight='bold',
                ha='center', va='top', color=layer['border'])
        ax.text(0.5, layer['y'] + layer['height'] - 0.065,
                layer['subtitle'], fontsize=11,
                ha='center', va='top', style='italic', color='gray')

        # Description bullets (left side)
        for j, desc in enumerate(layer['description']):
            ax.text(0.2, layer['y'] + layer['height'] - 0.09 - j*0.035,
                    f'• {desc}', fontsize=9, ha='left', va='top')

        # Analogy (right side)
        ax.text(0.82, layer['y'] + layer['height']/2,
                layer['analogy'], fontsize=9, ha='left', va='center',
                style='italic', color='gray',
                bbox=dict(boxstyle='round,pad=0.2', facecolor='white', alpha=0.7))

    # Arrows between layers
    arrow_style = dict(arrowstyle='->', color='#555555', lw=3,
                       connectionstyle='arc3,rad=0')

    # Void → Flux
    ax.annotate('', xy=(0.5, 0.42), xytext=(0.5, 0.35),
               arrowprops=arrow_style)
    ax.text(0.53, 0.385, 'Disposition\n(potential)', fontsize=9,
            ha='left', va='center', color='#555555')

    # Flux → Manifestation
    ax.annotate('', xy=(0.5, 0.69), xytext=(0.5, 0.62),
               arrowprops=arrow_style)
    ax.text(0.53, 0.655, 'Actualization\n(|J| > KB)', fontsize=9,
            ha='left', va='center', color='#555555')

    # Downward arrows (evaporation)
    ax.annotate('', xy=(0.35, 0.35), xytext=(0.35, 0.42),
               arrowprops=dict(arrowstyle='->', color='#999999', lw=2, ls='--'))
    ax.annotate('', xy=(0.35, 0.62), xytext=(0.35, 0.69),
               arrowprops=dict(arrowstyle='->', color='#999999', lw=2, ls='--'))
    ax.text(0.32, 0.385, 'Decay', fontsize=8, ha='right', color='#999999')
    ax.text(0.32, 0.655, 'Evaporation', fontsize=8, ha='right', color='#999999')

    # Title
    ax.text(0.5, 0.97, 'Ontological Hierarchy: Graded Monism',
            fontsize=18, fontweight='bold', ha='center', va='top')

    # Philosophical note
    note = (
        "TRD posits a single substance (void) with dispositional properties (flux)\n"
        "that can actualize into definite states (manifestation). This is graded monism:\n"
        "one substance, multiple modes of being."
    )
    ax.text(0.5, 0.03, note, fontsize=10, ha='center', va='bottom',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white',
                     edgecolor='gray', alpha=0.9))

    # Side labels
    ax.text(0.05, 0.25, 'LESS\nACTUAL', fontsize=10, ha='center', va='center',
            color='gray', fontweight='bold')
    ax.text(0.05, 0.79, 'MORE\nACTUAL', fontsize=10, ha='center', va='center',
            color='gray', fontweight='bold')
    ax.annotate('', xy=(0.05, 0.85), xytext=(0.05, 0.35),
               arrowprops=dict(arrowstyle='->', color='gray', lw=2))

    return fig


if __name__ == '__main__':
    fig = generate_ontological_levels()
    output_path = Path(__file__).parent.parent / 'ch00' / 'fig_2_3_ontological_levels.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
