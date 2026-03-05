"""
FTD Biophysics Master Figures
Generates a 4-panel figure for the biological thermodynamics paper.

Panels:
  A) FTD Thermodynamic Symmetry Ladder (G* baseline + quantized gradients)
  B) Kleiber's Law (3/4 topological routing tax)
  C) Deep-Sea Gigantism (Ricci flow dilution)
  D) 20-Thermal Conformal Metabolic Tax
"""
from pathlib import Path

_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'

import numpy as np
import matplotlib.pyplot as plt
from scipy.special import gamma


def generate_biophysics_figures():
    """Generates the 4-panel biophysical proofs for FTD."""
    plt.style.use('default')
    plt.rcParams.update({
        'font.family': 'serif',
        'mathtext.fontset': 'cm',
        'font.size': 10,
        'axes.titlesize': 12,
        'axes.labelsize': 11,
        'legend.fontsize': 9,
        'axes.linewidth': 0.8,
        'xtick.direction': 'in',
        'ytick.direction': 'in',
    })

    # FTD Constants
    G_star = np.sqrt(2) * (gamma(0.25)**2) / (2 * np.pi)
    theta_T = 100.0  # Kelvin bandwidth
    abs_zero = -273.15

    fig = plt.figure(figsize=(14, 11))
    fig.suptitle('Geometric Biophysics: Algorithmic Thermoregulation & Deep-Sea Scaling',
                 fontsize=16, fontweight='bold', y=0.96)

    # =========================================================================
    # PANEL A: The FTD Thermodynamic Symmetry Ladder
    # =========================================================================
    ax1 = fig.add_subplot(221)

    gradients = [
        ('Avian Overclock (+3/16)', 3/16, '#CC0000'),
        ('Mammalian Baseline (+1/7)', 1/7, '#FF3300'),
        ('Reptilian Passive (+1/13)', 1/13, '#FF9933'),
        (r'Ambient Baseline ($G^*$)', 0, '#228B22'),
        ('Ocean Surface (-1/13)', -1/13, '#3399FF'),
        ('Abyssal Max Density (-3/16)', -3/16, '#0000CC'),
        ('Seawater Crash (-1/4)', -1/4, '#000066'),
    ]

    for i, (label, frac, color) in enumerate(gradients):
        temp_C = (G_star + frac) * theta_T + abs_zero
        ax1.axhline(y=temp_C, color=color, linestyle='-', lw=2, alpha=0.8)
        ax1.plot(0.02, temp_C, 'o', color=color, markersize=8, markeredgecolor='k', zorder=5)
        align = 'bottom' if frac >= 0 else 'top'
        offset = 0.6 if frac >= 0 else -0.9
        ax1.text(0.06, temp_C + offset, f"{temp_C:.2f}\u00b0C : {label}",
                 color=color, fontweight='bold', va=align, fontsize=8.5)

    ax1.set_xlim(-0.05, 1)
    ax1.set_ylim(-6, 46)
    ax1.set_xticks([])
    ax1.set_ylabel(r"Temperature ($^\circ$C)")
    ax1.set_title(r"A) Symmetrical FTD Biosphere Gradient ($\Theta_T \equiv 100$ K)")
    ax1.grid(True, linestyle='--', alpha=0.3, axis='y')

    # =========================================================================
    # PANEL B: Kleiber's Law (The 3/4 Topological Routing Tax)
    # =========================================================================
    ax2 = fig.add_subplot(222)

    masses = np.logspace(-1.5, 4, 200)
    basal_metabolism = masses**(3/4)
    surface_area_limit = masses**(2/3)

    ax2.loglog(masses, basal_metabolism, '#003366', lw=3,
               label=r'FTD Topological Limit ($M^{3/4}$)')
    ax2.loglog(masses, surface_area_limit, 'r--', lw=2,
               label=r'Classical Surface Limit ($M^{2/3}$)')

    animal_data = [
        (0.02, 'Mouse'), (0.3, 'Rat'), (3, 'Cat'),
        (70, 'Human'), (400, 'Horse'), (3000, 'Elephant'),
    ]
    for m, name in animal_data:
        b = m**(3/4) * (0.95 + 0.1 * np.random.RandomState(hash(name) % 2**31).random())
        ax2.scatter(m, b, color='black', s=30, zorder=5)
        ax2.annotate(name, (m, b), xytext=(5, -8), textcoords='offset points',
                     fontsize=8, fontstyle='italic')

    ax2.set_xlabel("Body Mass (kg)")
    ax2.set_ylabel("Basal Metabolic Rate (Normalized)")
    ax2.set_title(r"B) Topological Impedance: Kleiber's Law ($N_c / N_{base}$)")
    ax2.grid(True, which="both", linestyle='--', alpha=0.3)
    ax2.legend(loc='upper left')

    # =========================================================================
    # PANEL C: Deep-Sea Gigantism (Discrete Ricci Flow)
    # =========================================================================
    ax3 = fig.add_subplot(223)

    depths = np.linspace(0, 11000, 200)
    pressure = 1 + (depths / 10)  # rough atm

    k_standard = 0.015 * pressure
    k_giant = 0.015 + np.log1p(pressure) * 0.15

    ax3.plot(depths, k_standard, 'r--', lw=2, label='Standard Volume ($V = 1$)')
    ax3.plot(depths, k_giant, '#0066CC', lw=3, label='Gigantism (Volumetric Scaling)')
    ax3.axhline(y=1.352, color='k', linestyle=':', lw=2,
                label=r'Topological Crash ($k_{crit} \approx 1.352$)')

    ax3.fill_between(depths, 1.352, np.clip(k_standard, 0, 2),
                     where=k_standard > 1.352, color='red', alpha=0.1)

    # Annotate key organisms
    ax3.annotate('Giant Isopod\n(~2000 m)', xy=(2000, 0.35), fontsize=8,
                 fontstyle='italic', color='#0066CC')
    ax3.annotate('Colossal Squid\n(~1000 m)', xy=(1000, 0.22), fontsize=8,
                 fontstyle='italic', color='#0066CC')
    ax3.annotate('Giant Tube Worm\n(~2500 m)', xy=(3500, 0.50), fontsize=8,
                 fontstyle='italic', color='#0066CC')

    ax3.set_xlabel('Ocean Depth (Meters)')
    ax3.set_ylabel(r'Local Geometric Tension ($k$)')
    ax3.set_ylim(0, 2.0)
    ax3.set_title('C) Deep-Sea Gigantism (Ricci Flow Dilution)')
    ax3.grid(True, linestyle='--', alpha=0.3)
    ax3.legend(loc='upper left', fontsize=8)

    # =========================================================================
    # PANEL D: The 20-Thermal Conformal Metabolic Tax
    # =========================================================================
    ax4 = fig.add_subplot(224)

    days = np.linspace(0, 5, 200)
    energy_used = 20 * days

    ax4.plot(days, energy_used, 'k-', lw=3,
             label=r'Metabolic Tax ($20\,\Theta_E$ / day)')
    ax4.fill_between(days, 0, energy_used, color='#FF9900', alpha=0.15)

    ax4.text(2.5, 22, r'$1\,\Theta_E \equiv 100$ kcal',
             ha='center', fontsize=11, fontweight='bold', color='#CC0000')
    ax4.text(2.5, 12, r'$c_{Dirac}^{-1} = N_{eff} + b_3 = 13 + 7 = 20$',
             ha='center', fontsize=10, fontstyle='italic', color='#333333')

    # Mark daily milestones
    for d in range(1, 6):
        ax4.plot(d, 20 * d, 'ko', markersize=5)
        ax4.annotate(f'{20*d*100:,} kcal', (d, 20 * d), xytext=(5, 5),
                     textcoords='offset points', fontsize=8)

    ax4.set_xlabel("Time (Days)")
    ax4.set_ylabel(r"Cumulative Energy Expended ($\Theta_E$)")
    ax4.set_title(r"D) The Human Conformal Anomaly ($2{,}000$ kcal/day)")
    ax4.grid(True, linestyle='--', alpha=0.3)
    ax4.legend(loc='upper left')

    plt.tight_layout()
    fig.subplots_adjust(top=0.92)
    plt.savefig(_FIGDIR / 'FTD_Biophysics_Master.pdf', format='pdf', dpi=600, bbox_inches='tight')
    plt.savefig(_FIGDIR / 'FTD_Biophysics_Master.png', format='png', dpi=300, bbox_inches='tight')
    print(f"Saved to {_FIGDIR}")
    plt.close()


if __name__ == "__main__":
    generate_biophysics_figures()
