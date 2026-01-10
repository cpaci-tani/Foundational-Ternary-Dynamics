"""
Figure 1.11: Force Comparison
=============================
Log-log plot comparing the four fundamental forces by:
- Relative strength (y-axis)
- Range (x-axis)

Shows:
- Strong force: strength=1, range=10⁻¹⁵ m
- Electromagnetic: strength=α≈1/137, range=∞
- Weak: strength=10⁻⁶, range=10⁻¹⁸ m
- Gravity: strength=10⁻⁴⁰, range=∞
"""

import numpy as np
import matplotlib.pyplot as plt
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, FORCE_COLORS, FONTS, apply_trd_style, create_figure
from utils.physics_constants import FORCES, ALPHA


def generate_force_comparison():
    """
    Generate the log-log force comparison chart.

    Four forces plotted as large markers with range bars
    showing characteristic interaction distances.
    """
    fig, ax = plt.subplots(figsize=(10, 8), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    # Force data
    forces = {
        'Strong': {
            'strength': 1.0,
            'range': 1e-15,
            'color': FORCE_COLORS['strong'],
            'carrier': 'gluons',
            'marker': 'o',
        },
        'Electromagnetic': {
            'strength': ALPHA,  # ~1/137 ≈ 0.0073
            'range': 1e10,  # Use large but finite for plotting
            'color': FORCE_COLORS['electromagnetic'],
            'carrier': 'photons',
            'marker': 's',
        },
        'Weak': {
            'strength': 1e-6,
            'range': 1e-18,
            'color': FORCE_COLORS['weak'],
            'carrier': 'W/Z bosons',
            'marker': '^',
        },
        'Gravity': {
            'strength': 1e-40,
            'range': 1e10,  # Use large but finite for plotting
            'color': FORCE_COLORS['gravity'],
            'carrier': 'gravitons',
            'marker': 'D',
        },
    }

    # Plot each force
    for name, props in forces.items():
        ax.scatter(
            props['range'],
            props['strength'],
            c=props['color'],
            s=400,
            marker=props['marker'],
            label=f"{name}\n({props['carrier']})",
            zorder=5,
            edgecolors='white',
            linewidths=2
        )

        # Add force name annotation
        offset_x = 1.5 if props['range'] < 1 else 0.3
        offset_y = 3 if props['strength'] > 1e-20 else 0.1

        ax.annotate(
            name,
            xy=(props['range'], props['strength']),
            xytext=(props['range'] * offset_x, props['strength'] * offset_y),
            fontsize=12,
            fontweight='bold',
            color=props['color'],
            ha='left' if props['range'] < 1 else 'center',
        )

    # Add "infinite range" arrows for EM and Gravity
    for name in ['Electromagnetic', 'Gravity']:
        props = forces[name]
        ax.annotate(
            '',
            xy=(1e12, props['strength']),
            xytext=(props['range'], props['strength']),
            arrowprops=dict(arrowstyle='->', color=props['color'], lw=2, ls='--')
        )

    # Shaded regions for short vs long range
    ax.axvspan(1e-20, 1e-10, alpha=0.1, color='blue', label='Short-range')
    ax.axvspan(1e-5, 1e15, alpha=0.1, color='green', label='Long-range')

    # Configure axes
    ax.set_xscale('log')
    ax.set_yscale('log')

    ax.set_xlim(1e-20, 1e15)
    ax.set_ylim(1e-45, 1e2)

    apply_trd_style(ax,
                    title='The Four Fundamental Forces',
                    xlabel='Characteristic Range (meters)',
                    ylabel='Relative Strength (Strong = 1)')

    # Add strength comparison lines
    ax.axhline(y=1, color=COLORS['grid'], linestyle='-', alpha=0.5, linewidth=1)
    ax.axhline(y=ALPHA, color=FORCE_COLORS['electromagnetic'], linestyle=':', alpha=0.5, linewidth=1)
    ax.axhline(y=1e-6, color=FORCE_COLORS['weak'], linestyle=':', alpha=0.5, linewidth=1)

    # Legend
    ax.legend(loc='upper right', fontsize=10, framealpha=0.95,
              markerscale=0.6, handletextpad=0.5)

    # Add annotation box explaining the plot
    info_text = (
        "Strong: Binds quarks in nucleons\n"
        "EM: Binds atoms, chemistry\n"
        "Weak: Radioactive decay\n"
        "Gravity: Cosmic structure"
    )
    ax.text(0.02, 0.02, info_text, transform=ax.transAxes,
            fontsize=9, verticalalignment='bottom',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='white', alpha=0.9, edgecolor='gray'))

    plt.tight_layout()

    return fig


if __name__ == '__main__':
    fig = generate_force_comparison()
    output_path = Path(__file__).parent.parent / 'ch01' / 'fig_1_11_force_comparison.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
