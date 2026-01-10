"""
Figure 1.10: Standard Model Chart
=================================
Comprehensive chart of Standard Model particles with TRD-derived masses.

Layout:
- Quarks: 2 rows × 3 columns (generations)
- Leptons: 2 rows × 3 columns (generations)
- Gauge bosons: 4 particles
- Higgs boson: 1 particle

Each particle box shows:
- Symbol and name
- TRD mass formula
- Predicted and measured values
- Error percentage
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Rectangle
import matplotlib.patches as mpatches
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.style import COLORS, SM_COLORS
from utils.physics_constants import PARTICLE_MASSES


def draw_particle_box(ax, x, y, width, height, symbol, name, formula,
                      predicted, measured, error, particle_type):
    """Draw a single particle information box."""
    # Get color based on particle type
    colors = {
        'quark': '#FF9999',
        'lepton': '#99CCFF',
        'gauge': '#FFFF99',
        'higgs': '#CC99FF',
    }
    color = colors.get(particle_type, '#CCCCCC')

    # Draw box
    box = FancyBboxPatch(
        (x, y), width, height,
        boxstyle="round,pad=0.01,rounding_size=0.01",
        facecolor=color,
        edgecolor='black',
        linewidth=1.5
    )
    ax.add_patch(box)

    # Symbol (large)
    ax.text(x + width*0.2, y + height*0.7, symbol, fontsize=16, ha='center',
            va='center', fontweight='bold')

    # Name
    ax.text(x + width*0.2, y + height*0.4, name, fontsize=7, ha='center',
            va='center')

    # Mass/formula (right side)
    if formula:
        ax.text(x + width*0.65, y + height*0.75, formula, fontsize=5,
                ha='center', va='center')

    if predicted:
        ax.text(x + width*0.65, y + height*0.5, f'{predicted}', fontsize=6,
                ha='center', va='center')

    if error:
        ax.text(x + width*0.65, y + height*0.25, f'({error})', fontsize=5,
                ha='center', va='center', color='gray')


def generate_standard_model():
    """
    Generate the Standard Model particle chart with TRD formulas.

    Traditional grid layout with all particles and their derived masses.
    """
    fig, ax = plt.subplots(figsize=(14, 10), dpi=150)
    fig.patch.set_facecolor(COLORS['background'])

    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis('off')

    # Layout parameters
    box_w = 0.11
    box_h = 0.12
    gap_x = 0.015
    gap_y = 0.02

    # Starting positions
    quark_x = 0.05
    lepton_x = 0.05
    boson_x = 0.55
    quark_y = 0.75
    lepton_y = 0.45

    # =========================================================================
    # Quarks (2 rows × 3 columns)
    # =========================================================================
    ax.text(quark_x + 1.5*(box_w + gap_x), quark_y + box_h + 0.03,
            'QUARKS', fontsize=14, ha='center', fontweight='bold')

    quarks = [
        # Row 1: Up-type quarks
        [
            ('u', 'up', r'$N_b+\sin^2\theta$', '4.23', '0.09%'),
            ('c', 'charm', r'$n(b+N_c)(19)+15$', '2485', '0.01%'),
            ('t', 'top', r'$(\phi^2-64\alpha)m_W$', '338k', '0.12%'),
        ],
        # Row 2: Down-type quarks
        [
            ('d', 'down', r'$2N_b+1+\alpha n$', '9.10', '0.48%'),
            ('s', 'strange', r'$n(n+1)+1$', '183', '0.12%'),
            ('b', 'bottom', r'$10^3\times8+169$', '8169', '0.14%'),
        ],
    ]

    for row_idx, row in enumerate(quarks):
        for col_idx, (symbol, name, formula, mass, error) in enumerate(row):
            x = quark_x + col_idx * (box_w + gap_x)
            y = quark_y - row_idx * (box_h + gap_y)
            draw_particle_box(ax, x, y, box_w, box_h, symbol, name,
                            formula, mass, None, error, 'quark')

    # Charge labels
    ax.text(quark_x - 0.02, quark_y + box_h/2, '+2/3', fontsize=9,
            ha='right', va='center', fontweight='bold', color=COLORS['matter'])
    ax.text(quark_x - 0.02, quark_y - box_h/2 - gap_y, '-1/3', fontsize=9,
            ha='right', va='center', fontweight='bold', color=COLORS['antimatter'])

    # Generation labels
    for i, gen in enumerate(['I', 'II', 'III']):
        ax.text(quark_x + i*(box_w + gap_x) + box_w/2, quark_y + box_h + 0.01,
                gen, fontsize=10, ha='center', color='gray')

    # =========================================================================
    # Leptons (2 rows × 3 columns)
    # =========================================================================
    ax.text(quark_x + 1.5*(box_w + gap_x), lepton_y + box_h + 0.03,
            'LEPTONS', fontsize=14, ha='center', fontweight='bold')

    leptons = [
        # Row 1: Charged leptons
        [
            ('e', 'electron', r'$70\alpha \cdot m_0$', '1.00', '0.04%'),
            ('μ', 'muon', r'$3\times70-3$', '207', '0.11%'),
            ('τ', 'tau', r'$17\times207-42$', '3477', '0.01%'),
        ],
        # Row 2: Neutrinos
        [
            ('νe', 'e-neutrino', r'$\sim 0$', '<1 eV', '-'),
            ('νμ', 'μ-neutrino', r'$\sim 0$', '<1 eV', '-'),
            ('ντ', 'τ-neutrino', r'$\sim 0$', '<1 eV', '-'),
        ],
    ]

    for row_idx, row in enumerate(leptons):
        for col_idx, (symbol, name, formula, mass, error) in enumerate(row):
            x = lepton_x + col_idx * (box_w + gap_x)
            y = lepton_y - row_idx * (box_h + gap_y)
            draw_particle_box(ax, x, y, box_w, box_h, symbol, name,
                            formula, mass, None, error, 'lepton')

    # Charge labels
    ax.text(lepton_x - 0.02, lepton_y + box_h/2, '-1', fontsize=9,
            ha='right', va='center', fontweight='bold', color=COLORS['antimatter'])
    ax.text(lepton_x - 0.02, lepton_y - box_h/2 - gap_y, '0', fontsize=9,
            ha='right', va='center', fontweight='bold', color=COLORS['void'])

    # =========================================================================
    # Gauge Bosons
    # =========================================================================
    ax.text(boson_x + box_w + gap_x/2, quark_y + box_h + 0.03,
            'GAUGE BOSONS', fontsize=14, ha='center', fontweight='bold')

    gauge = [
        [
            ('γ', 'photon', 'massless', '0', '-'),
            ('g', 'gluon', 'massless', '0', '-'),
        ],
        [
            ('W', 'W boson', r'$67/(8\alpha^2)$', '157k', '0.02%'),
            ('Z', 'Z boson', r'$m_W\sqrt{13/10}$', '179k', '0.49%'),
        ],
    ]

    for row_idx, row in enumerate(gauge):
        for col_idx, (symbol, name, formula, mass, error) in enumerate(row):
            x = boson_x + col_idx * (box_w + gap_x)
            y = quark_y - row_idx * (box_h + gap_y)
            draw_particle_box(ax, x, y, box_w, box_h, symbol, name,
                            formula, mass, None, error, 'gauge')

    # =========================================================================
    # Higgs Boson
    # =========================================================================
    ax.text(boson_x + box_w/2, lepton_y + box_h + 0.03,
            'SCALAR BOSON', fontsize=12, ha='center', fontweight='bold')

    higgs_x = boson_x
    higgs_y = lepton_y
    draw_particle_box(ax, higgs_x, higgs_y, box_w*1.5, box_h*1.2,
                     'H', 'Higgs', r'$n_{eff}/\alpha^2$', '244k', None, '0.40%', 'higgs')

    # =========================================================================
    # Baryons (bonus section)
    # =========================================================================
    baryon_x = 0.75
    baryon_y = lepton_y - 0.02

    ax.text(baryon_x + box_w, baryon_y + box_h + 0.03,
            'BARYONS', fontsize=12, ha='center', fontweight='bold')

    baryons = [
        ('p', 'proton', r'$n/\alpha+T(10)$', '1836', '0.017%'),
        ('n', 'neutron', r'$m_p+\phi^2-12\alpha$', '1839', '0.017%'),
    ]

    for col_idx, (symbol, name, formula, mass, error) in enumerate(baryons):
        x = baryon_x + col_idx * (box_w + gap_x)
        y = baryon_y
        draw_particle_box(ax, x, y, box_w, box_h, symbol, name,
                        formula, mass, None, error, 'quark')  # Use quark color

    # =========================================================================
    # Title and Legend
    # =========================================================================
    fig.suptitle('Standard Model Particles with TRD Mass Derivations',
                fontsize=18, fontweight='bold', y=0.98)

    # Legend
    legend_elements = [
        mpatches.Patch(facecolor='#FF9999', edgecolor='black', label='Quarks'),
        mpatches.Patch(facecolor='#99CCFF', edgecolor='black', label='Leptons'),
        mpatches.Patch(facecolor='#FFFF99', edgecolor='black', label='Gauge Bosons'),
        mpatches.Patch(facecolor='#CC99FF', edgecolor='black', label='Higgs'),
    ]
    ax.legend(handles=legend_elements, loc='lower center', ncol=4,
              fontsize=10, bbox_to_anchor=(0.4, 0.02))

    # Summary
    summary = 'All masses in units of electron mass. Average error < 0.5%'
    ax.text(0.4, 0.08, summary, fontsize=10, ha='center',
            style='italic', color='gray')

    # Note about framework
    note = 'Derived from 4 integers: {b₃=7, Nc=3, neff=13, Nbase=4}'
    ax.text(0.4, 0.04, note, fontsize=9, ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#ffffd0',
                     edgecolor='gray'))

    plt.tight_layout(rect=[0, 0.1, 1, 0.95])

    return fig


if __name__ == '__main__':
    fig = generate_standard_model()
    output_path = Path(__file__).parent.parent / 'ch02' / 'fig_1_10_standard_model.png'
    fig.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"Saved: {output_path}")
    plt.close(fig)
