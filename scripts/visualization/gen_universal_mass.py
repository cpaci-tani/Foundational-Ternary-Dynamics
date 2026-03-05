"""
FTD Universal Mass Figures — Scale-Invariant Gravimetrics
Generates a 6-panel figure spanning atomic to cosmological scales.

Panels:
  A) Gravity as Algorithmic Deduplication (Ontology diagram)
  B) Micro-Scale: Nuclear Packing (Iron-56 + Feynman Limit)
  C) Meso-Scale: Newton's G & Proton Mass derivation
  D) Planetary Scale: Earth/Moon harmonic resonance
  E) Anthropomorphic Scale: Biological anchor
  F) Cosmological Scale: Chandrasekhar + Black Hole entropy
"""
from pathlib import Path

_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches


def generate_universal_mass_figures():
    """Generates a 6-panel comprehensive proof of FTD Scale-Invariant Gravimetrics."""
    plt.style.use('default')
    plt.rcParams.update({
        'font.family': 'serif',
        'mathtext.fontset': 'cm',
        'font.size': 9,
        'axes.titlesize': 11,
        'axes.labelsize': 10,
        'legend.fontsize': 8,
        'axes.linewidth': 0.8,
        'xtick.direction': 'in',
        'ytick.direction': 'in',
    })

    fig = plt.figure(figsize=(16, 11))
    fig.suptitle(
        'The Grand Unified Theory of Mass: FTD Scale-Invariant Algorithmic Deduplication',
        fontsize=16, fontweight='bold', y=0.97,
    )

    # =========================================================================
    # PANEL A: Gravity as Algorithmic Deduplication (Ontology)
    # =========================================================================
    ax1 = fig.add_subplot(231)
    ax1.set_xlim(0, 1)
    ax1.set_ylim(0, 1)
    ax1.axis('off')
    ax1.set_title("A) Ontology: Gravity as Data Compression", pad=10)

    # Pre-Gravity: High Memory (Separate Boundaries)
    c1 = patches.Circle((0.3, 0.78), 0.15, fill=True, color='#003366', alpha=0.9)
    c2 = patches.Circle((0.7, 0.78), 0.15, fill=True, color='#003366', alpha=0.9)
    ax1.add_patch(c1)
    ax1.add_patch(c2)
    ax1.text(0.3, 0.78, "High\nMemory",
             ha='center', va='center', fontsize=8, color='white', fontweight='bold')
    ax1.text(0.7, 0.78, "High\nMemory",
             ha='center', va='center', fontsize=8, color='white', fontweight='bold')

    # Arrow down
    ax1.annotate('', xy=(0.5, 0.52), xytext=(0.5, 0.60),
                 arrowprops=dict(facecolor='black', shrink=0.05, width=2, headwidth=8))
    ax1.text(0.73, 0.56, "Discrete Ricci Flow\n(Gravitational Pull)",
             va='center', fontsize=8, fontstyle='italic', color='#333333')

    # Post-Gravity: Low Memory (Shared Boundary)
    c3 = patches.Circle((0.40, 0.33), 0.15, fill=True, color='#CC0000', alpha=0.9)
    c4 = patches.Circle((0.60, 0.33), 0.15, fill=True, color='#CC0000', alpha=0.9)
    ax1.add_patch(c3)
    ax1.add_patch(c4)
    ax1.text(0.40, 0.33, "Shared\nBoundary",
             ha='center', va='center', fontsize=8, color='white', fontweight='bold')
    ax1.text(0.60, 0.33, "Low\nCost",
             ha='center', va='center', fontsize=8, color='white', fontweight='bold')

    ax1.text(0.5, 0.10, r"$F_{grav} = -\nabla(\mathrm{Memory\ Cost})$",
             ha='center', fontsize=10, fontstyle='italic', color='#003366')

    rect = patches.FancyBboxPatch((0.02, 0.02), 0.96, 0.96, boxstyle="round,pad=0.02",
                                   linewidth=1, edgecolor='gray', facecolor='none', alpha=0.3)
    ax1.add_patch(rect)

    # =========================================================================
    # PANEL B: The Micro-Scale (Nuclear Stability & Feynman Limit)
    # =========================================================================
    ax2 = fig.add_subplot(232)

    A = np.linspace(1, 260, 500)
    # Simplified smooth binding energy curve
    BE_smooth = 8.8 * (1 - np.exp(-A / 8)) - 0.0007 * (A - 56)**2
    BE_smooth = np.clip(BE_smooth, 0, 9)

    ax2.plot(A, BE_smooth, 'k-', lw=2)
    ax2.axvline(x=56, color='#CC0000', linestyle='--', lw=1.5)
    ax2.plot(56, BE_smooth[np.argmin(np.abs(A - 56))], 'ro', markersize=8, zorder=5)
    ax2.text(64, 8.3, r'$^{56}$Fe (Max Saturation)' + '\n'
             + r'$14_{faces} \times 4_{dim} = \mathbf{56}$',
             color='#CC0000', fontsize=8.5, fontweight='bold')

    ax2.axvline(x=137, color='#003366', linestyle=':', lw=2)
    ax2.text(143, 3.5, 'Feynman Limit\n' + r'$Z = \lfloor 1/\alpha \rfloor = 137$',
             color='#003366', fontsize=8.5, fontweight='bold')

    ax2.set_xlim(0, 260)
    ax2.set_ylim(0, 9.5)
    ax2.set_xlabel("Mass Number ($A$) / Atomic Number ($Z$)")
    ax2.set_ylabel("Binding Energy per Nucleon (MeV)")
    ax2.set_title("B) Micro-Scale: Nuclear Packing Geometry")
    ax2.grid(True, linestyle='--', alpha=0.3)

    # =========================================================================
    # PANEL C: The Meso-Scale (Deriving G and Proton Mass)
    # =========================================================================
    ax3 = fig.add_subplot(233)
    ax3.set_xlim(0, 1)
    ax3.set_ylim(0, 1)
    ax3.axis('off')
    ax3.set_title(r"C) Meso-Scale: Newton's $G$ & The Proton", pad=10)

    # --- Newton's G block (upper half) ---
    ax3.text(0.50, 0.88, r"$\mathbf{Newton's\ Gravitational\ Constant}$",
             ha='center', fontsize=9.5, fontweight='bold')

    ax3.text(0.50, 0.77, r"$G_{FTD} = \frac{c_{Dirac}^{-1}}{N_c}"
             r"\!\left(1 + \frac{\alpha}{2\pi}\right)"
             r"= \frac{20}{3}(1 + 0.00116)$",
             ha='center', fontsize=9.5)

    ax3.text(0.50, 0.67, r"$= \mathbf{6.6744 \times 10^{-11}}$"
             r"  m$^3$ kg$^{-1}$ s$^{-2}$",
             ha='center', fontsize=10, color='#003366', fontweight='bold')

    ax3.text(0.50, 0.60, r"Error: $0.0016\%$ vs CODATA 2018",
             ha='center', fontsize=8.5, color='#555555')

    # Separator
    ax3.plot([0.10, 0.90], [0.53, 0.53], 'k-', lw=0.8, alpha=0.4)

    # --- Proton Mass block (lower half) ---
    ax3.text(0.50, 0.46, r"$\mathbf{Proton\text{-}to\text{-}Electron\ Mass\ Ratio}$",
             ha='center', fontsize=9.5, fontweight='bold')

    ax3.text(0.50, 0.35, r"$\mu = (N_c \times N_{base}) \times (x_+ + k_{dim})$",
             ha='center', fontsize=9.5)

    ax3.text(0.50, 0.25, r"$= 12 \times 153.036 = \mathbf{1836.43}$",
             ha='center', fontsize=10, color='#003366', fontweight='bold')

    ax3.text(0.50, 0.18, r"Error: $0.015\%$ vs CODATA 2018",
             ha='center', fontsize=8.5, color='#555555')

    # Background box
    rect_c = patches.FancyBboxPatch((0.03, 0.10), 0.94, 0.84, boxstyle="round,pad=0.02",
                                     linewidth=1.2, edgecolor='black', facecolor='#f5f5f0')
    ax3.add_patch(rect_c)
    # Re-draw text on top by setting zorder (patches default lower)
    for child in ax3.get_children():
        if hasattr(child, 'set_zorder') and not isinstance(child, patches.FancyBboxPatch):
            child.set_zorder(10)

    # =========================================================================
    # PANEL D: The Planetary Scale (Earth & Moon)
    # =========================================================================
    ax4 = fig.add_subplot(234)

    categories = [r"Earth $g$" + "\n" + r"$(\alpha^{-1}/14)$",
                  r"Earth/Moon" + "\n" + r"$(3^4)$"]
    ftd_vals = [137.036 / 14, 3**4]
    emp_vals = [9.780, 81.3]

    x = np.arange(len(categories))
    w = 0.3
    bars1 = ax4.bar(x - w/2, ftd_vals, w, label='FTD Derivation', color='#006699', edgecolor='k', lw=0.5)
    bars2 = ax4.bar(x + w/2, emp_vals, w, label='Empirical', color='#999999', edgecolor='k', lw=0.5)

    for bar, val in zip(bars1, ftd_vals):
        ax4.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1.5,
                 f'{val:.3f}', ha='center', fontsize=8, color='#006699', fontweight='bold')
    for bar, val in zip(bars2, emp_vals):
        ax4.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1.5,
                 f'{val:.1f}', ha='center', fontsize=8, color='#555555')

    ax4.set_xticks(x)
    ax4.set_xticklabels(categories, fontsize=9)
    ax4.set_ylabel("Value")
    ax4.set_ylim(0, 95)
    ax4.set_title("D) Planetary Scale: Harmonic Resonance")
    ax4.legend(loc='upper left', fontsize=8)
    ax4.grid(True, axis='y', linestyle='--', alpha=0.3)

    # =========================================================================
    # PANEL E: The Anthropomorphic Scale (The Biological Anchor)
    # Split into two sub-groups with separate y-axes for clarity
    # =========================================================================
    ax5 = fig.add_subplot(235)

    # Use grouped bar chart but with log scale to handle the 50x difference
    categories_e = ['Reference\nHuman (kg)', 'Human\nBrain (kg)']
    ftd_e = [137.036 / 2, 4 / 2.95867]
    emp_e = [68.5, 1.35]

    x_e = np.arange(len(categories_e))
    bars5a = ax5.bar(x_e - w/2, ftd_e, w, label='FTD Derivation', color='#006699', edgecolor='k', lw=0.5)
    bars5b = ax5.bar(x_e + w/2, emp_e, w, label='Empirical', color='#999999', edgecolor='k', lw=0.5)

    # Annotate with values and FTD formulas
    ax5.text(bars5a[0].get_x() + bars5a[0].get_width()/2, ftd_e[0] + 2,
             f'{ftd_e[0]:.2f}', ha='center', fontsize=8, color='#006699', fontweight='bold')
    ax5.text(bars5b[0].get_x() + bars5b[0].get_width()/2, emp_e[0] + 2,
             f'{emp_e[0]:.1f}', ha='center', fontsize=8, color='#555555')

    ax5.text(bars5a[1].get_x() + bars5a[1].get_width()/2, ftd_e[1] + 2,
             f'{ftd_e[1]:.3f}', ha='center', fontsize=8, color='#006699', fontweight='bold')
    ax5.text(bars5b[1].get_x() + bars5b[1].get_width()/2, emp_e[1] + 2,
             f'{emp_e[1]:.2f}', ha='center', fontsize=8, color='#555555')

    # FTD formula annotations
    ax5.text(0, -8, r'$\alpha^{-1}/2$', ha='center', fontsize=9, color='#003366', fontstyle='italic')
    ax5.text(1, -8, r'$4/G^*$', ha='center', fontsize=9, color='#003366', fontstyle='italic')

    ax5.set_xticks(x_e)
    ax5.set_xticklabels(categories_e, fontsize=9)
    ax5.set_ylabel("Mass (kg)")
    ax5.set_ylim(0, 82)
    ax5.set_title("E) Anthropomorphic Scale: Biological Anchor")
    ax5.legend(loc='upper right', fontsize=8)
    ax5.grid(True, axis='y', linestyle='--', alpha=0.3)

    # Inset for the brain mass detail (too small to see on main axis)
    ax5_inset = ax5.inset_axes([0.52, 0.35, 0.42, 0.45])
    ax5_inset.bar([0 - w/2], [ftd_e[1]], w, color='#006699', edgecolor='k', lw=0.5)
    ax5_inset.bar([0 + w/2], [emp_e[1]], w, color='#999999', edgecolor='k', lw=0.5)
    ax5_inset.set_ylim(0, 1.8)
    ax5_inset.set_xlim(-0.5, 0.5)
    ax5_inset.set_xticks([0])
    ax5_inset.set_xticklabels(['Brain Mass'], fontsize=7)
    ax5_inset.set_ylabel('kg', fontsize=7)
    ax5_inset.tick_params(labelsize=7)
    ax5_inset.text(-w/2, ftd_e[1] + 0.05, f'{ftd_e[1]:.3f}', ha='center', fontsize=7,
                   color='#006699', fontweight='bold')
    ax5_inset.text(w/2, emp_e[1] + 0.05, f'{emp_e[1]:.2f}', ha='center', fontsize=7,
                   color='#555555')
    ax5_inset.set_title('Detail', fontsize=8)
    ax5_inset.grid(True, axis='y', linestyle='--', alpha=0.3)

    # =========================================================================
    # PANEL F: The Cosmological Scale (Singularities & Expansion)
    # Redesigned: separate the two scale regimes visually
    # =========================================================================
    ax6 = fig.add_subplot(236)

    # Three bars with a broken-style visual: first two share k_crit scale,
    # third is on a different scale entirely
    scales = ['Brain Mass\n(kg)', 'Chandrasekhar\n' + r'($M_\odot$)',
              r'$\Omega_\Lambda$' + '\n(%)']
    values = [1.352, 1.35, 68.5]
    colors_f = ['#FF9900', '#660099', '#006699']

    bars_f = ax6.bar(range(3), values, color=colors_f, width=0.5, edgecolor='k', lw=0.5)

    # Value labels above bars
    for i, bar in enumerate(bars_f):
        ax6.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1.2,
                 f"{values[i]}", ha='center', fontweight='bold', fontsize=9.5,
                 color=colors_f[i])

    # k_crit reference line for the first two
    ax6.plot([-0.4, 1.4], [4/2.95867, 4/2.95867], 'k--', lw=1.5)
    ax6.text(0.5, 4/2.95867 + 2.5, r'$k_{crit} = 4/G^* \approx 1.352$',
             ha='center', fontsize=9, fontstyle='italic',
             bbox=dict(facecolor='white', alpha=0.8, edgecolor='none', pad=1))

    # alpha/2 reference line for the third
    ax6.plot([1.6, 2.4], [137.036/2, 137.036/2], 'k:', lw=1.5)
    ax6.text(2.0, 137.036/2 + 2.5, r'$\alpha^{-1}/2 = 68.518$',
             ha='center', fontsize=9, fontstyle='italic',
             bbox=dict(facecolor='white', alpha=0.8, edgecolor='none', pad=1))

    ax6.set_xticks(range(3))
    ax6.set_xticklabels(scales, fontsize=8.5)
    ax6.set_ylim(0, 82)
    ax6.set_title(r"F) Cosmo-Scale: Scale-Invariant Singularities")

    # Black hole entropy box at top
    ax6.text(1.0, 77, r"Bekenstein-Hawking: $S = A/N_{base} = A/4$",
             ha='center', fontsize=9,
             bbox=dict(facecolor='#f0f0f0', alpha=0.95, edgecolor='black',
                       boxstyle='round,pad=0.4'))

    ax6.grid(True, axis='y', linestyle='--', alpha=0.3)

    # =========================================================================
    # Final layout
    # =========================================================================
    fig.subplots_adjust(top=0.92, bottom=0.06, left=0.05, right=0.97,
                        wspace=0.28, hspace=0.35)
    plt.savefig(_FIGDIR / 'FTD_Universal_Mass.pdf', format='pdf', dpi=600, bbox_inches='tight')
    plt.savefig(_FIGDIR / 'FTD_Universal_Mass.png', format='png', dpi=300, bbox_inches='tight')
    print(f"Saved to {_FIGDIR}")
    plt.close()


if __name__ == "__main__":
    generate_universal_mass_figures()
