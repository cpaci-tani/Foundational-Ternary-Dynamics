"""
FTD Master Figures — Publication-Quality Visual Proofs
Generates:
  1) A 4-panel composite figure (FTD_Master_Figures.pdf/png)
  2) Individual standalone figures for per-paper embedding

Panels:
  A) Lemniscate-Alpha Topological Blueprint (Feigenbaum Cascade)
  B) PT-Symmetry Phase Transition (Sonoluminescence Threshold)
  C) Manifestation Operator (Softplus → ReLU thermodynamic gate)
  D) 137-Lobe Spin-2 Moiré Asymmetry (Quadrupolar Diode)
"""

from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt

_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'
from matplotlib.patches import FancyArrowPatch
from scipy.special import gamma


# =============================================================================
# Shared constants and styling
# =============================================================================
G_STAR = np.sqrt(2) * (gamma(0.25)**2) / (2 * np.pi)
K_CRIT = 4 / G_STAR
K_B = 1.0  # Normalized manifestation threshold

STYLE = {
    'font.family': 'serif',
    'mathtext.fontset': 'cm',
    'font.size': 10,
    'axes.titlesize': 12,
    'axes.labelsize': 11,
    'legend.fontsize': 9,
    'axes.linewidth': 0.8,
    'xtick.direction': 'in',
    'ytick.direction': 'in',
    'xtick.major.size': 4,
    'ytick.major.size': 4,
}

# Color palette
NAVY = '#003366'
RED = '#CC0000'
CYAN = '#0099CC'
TEAL = '#006699'


def softplus(z, beta, threshold=K_B):
    return (1 / beta) * np.log1p(np.exp(np.clip(beta * (z - threshold), -500, 500)))


# =============================================================================
# Panel A: Lemniscate-Alpha Topological Blueprint
# =============================================================================
def plot_lemniscate(ax, show_title=True):
    t = np.linspace(0, 2 * np.pi, 8000)

    x = (np.cos(t) + 0.5 * np.cos(2 * t) + 0.5 * np.cos(4 * t)
         + 0.4 * np.cos(8 * t) + 0.0625 * np.cos(16 * t))
    y = (np.sin(t) - 0.5 * np.sin(2 * t) + 0.5 * np.sin(4 * t)
         - 0.35 * np.sin(8 * t) + 0.0625 * np.sin(16 * t))

    exclusion_radius = (G_STAR**2) / 32

    ax.plot(x, y, color=NAVY, lw=1.5, label=r'Lemniscate-$\alpha$ Curve')

    circle = plt.Circle(
        (0, 0), exclusion_radius, color=RED, alpha=0.15,
        label=rf'Euclidean Void ($r \approx {exclusion_radius:.3f}$)',
    )
    ax.add_artist(circle)
    circle_edge = plt.Circle((0, 0), exclusion_radius, fill=False,
                             edgecolor=RED, lw=1.0, linestyle='--', alpha=0.6)
    ax.add_artist(circle_edge)
    ax.plot(0, 0, 'kx', markersize=7, mew=1.5)

    if show_title:
        ax.set_title(r"A) Topological Quasicrystal Blueprint ($w = -2$)")
    ax.set_xlabel(r"$\mathrm{Re}(z)$")
    ax.set_ylabel(r"$\mathrm{Im}(z)$")
    ax.set_aspect('equal')
    ax.grid(True, linestyle='--', alpha=0.3)
    ax.legend(loc='upper right', framealpha=0.9)


# =============================================================================
# Panel B: PT-Symmetry Phase Transition
# =============================================================================
def plot_pt_transition(ax, show_title=True):
    k_vals = np.linspace(0.01, 4.0, 2000)
    real_r1 = np.empty_like(k_vals)
    real_r2 = np.empty_like(k_vals)
    imag_r = np.empty_like(k_vals)

    for i, k in enumerate(k_vals):
        b = -k * (G_STAR**2)
        c = k * (G_STAR**3)
        disc = b**2 - 4 * c
        if disc >= 0:
            real_r1[i] = (-b + np.sqrt(disc)) / 2
            real_r2[i] = (-b - np.sqrt(disc)) / 2
            imag_r[i] = 0
        else:
            real_r1[i] = -b / 2
            real_r2[i] = -b / 2
            imag_r[i] = np.sqrt(-disc) / 2

    c_mask = k_vals < K_CRIT
    r_mask = k_vals >= K_CRIT

    # Complex (Euclidean) regime
    ax.plot(k_vals[c_mask], imag_r[c_mask], color=RED, lw=2,
            label=r'$+\mathrm{Im}(x)$ (ZPE Polling)')
    ax.plot(k_vals[c_mask], -imag_r[c_mask], color=RED, lw=2, linestyle='--',
            label=r'$-\mathrm{Im}(x)$')
    ax.fill_between(k_vals[c_mask], imag_r[c_mask], -imag_r[c_mask],
                    color=RED, alpha=0.08)

    # Real (Physical) regime
    ax.plot(k_vals[r_mask], real_r1[r_mask], color=NAVY, lw=2,
            label=r'$x_+ \to 1/\alpha$ (Physics)')
    ax.plot(k_vals[r_mask], real_r2[r_mask], color=CYAN, lw=2, linestyle='--',
            label=r'$x_- \to N_c$ (Internal)')

    # Threshold line
    ax.axvline(x=K_CRIT, color='k', linestyle=':', lw=1.5,
               label=rf'$k_{{crit}} = 4/G^* \approx {K_CRIT:.3f}$')

    # Annotation at the exceptional point
    ax.annotate(
        r'$\mathcal{PT}$-breaking',
        xy=(K_CRIT, 0), xytext=(K_CRIT + 0.4, -4),
        fontsize=9, fontstyle='italic',
        arrowprops=dict(arrowstyle='->', color='k', lw=1),
    )

    if show_title:
        ax.set_title(r"B) $\mathcal{PT}$-Symmetry Phase Transition")
    ax.set_xlabel(r"Local Geometric Tension ($k$)")
    ax.set_ylabel("Eigenvalue Magnitude")
    ax.grid(True, linestyle='--', alpha=0.3)
    ax.legend(loc='upper left', framealpha=0.9, fontsize=8)


# =============================================================================
# Panel C: Manifestation Operator (Softplus → ReLU)
# =============================================================================
def plot_manifestation(ax, show_title=True):
    z_vals = np.linspace(-2, 4, 500)

    # Three temperature regimes
    ax.plot(z_vals, softplus(z_vals, beta=1.5), color=RED, linestyle='--', lw=2,
            label=r'Sonoluminescence ($\beta_{th} = 1.5$)')
    ax.plot(z_vals, softplus(z_vals, beta=5), color='#CC6600', lw=2,
            label=r'Room Temp ($\beta_{th} = 5$)')
    relu = np.maximum(0, z_vals - K_B)
    ax.plot(z_vals, relu, 'k-', lw=3,
            label=r'Cryogenic Limit ($T \to 0$, ReLU)')

    # Fill the sub-threshold region
    ax.fill_between(z_vals[z_vals <= K_B], 0, softplus(z_vals[z_vals <= K_B], beta=1.5),
                    color=RED, alpha=0.06)

    ax.axvline(x=K_B, color='gray', linestyle=':', lw=1.2,
               label=r'Threshold $K_B$')

    # Label the regimes
    ax.text(-0.5, 1.5, r'$\ker(\mathcal{M})$' + '\n(Null Space)',
            fontsize=9, ha='center', color=RED, fontstyle='italic')
    ax.text(2.8, 0.3, 'Physical\nManifold', fontsize=9, ha='center',
            color=NAVY, fontstyle='italic')

    if show_title:
        ax.set_title(r"C) Manifestation Operator $\mathcal{M}(z)$")
    ax.set_xlabel(r"Local Complex Flux ($z$)")
    ax.set_ylabel("Manifested Physical Data")
    ax.set_ylim(-0.2, 3.2)
    ax.grid(True, linestyle='--', alpha=0.3)
    ax.legend(loc='upper left', framealpha=0.9, fontsize=8)


# =============================================================================
# Panel D: 137-Lobe Spin-2 Moiré Pattern
# =============================================================================
def plot_moire(ax_polar, show_title=True):
    # Very high resolution for 137 lobes
    theta = np.linspace(0, 4 * np.pi, 200000)

    r = np.abs(np.cos(137 * theta / 2))
    modulation = 1 + 0.15 * np.cos(2 * theta)
    r_modulated = r * modulation

    ax_polar.plot(theta, r_modulated, color=TEAL, lw=0.15, alpha=0.7)
    ax_polar.plot(theta, modulation, 'k--', lw=2,
                  label='Spin-2 Quadrupole\nEnvelope')

    # Mark the quadrupolar axis of asymmetry
    for angle in [0, np.pi]:
        ax_polar.annotate('', xy=(angle, 1.2), xytext=(angle, 0.6),
                          arrowprops=dict(arrowstyle='->', color=RED, lw=1.5))

    if show_title:
        ax_polar.set_title(r"D) 137-Lobe Spin-2 Moiré Asymmetry", pad=20)
    ax_polar.set_xticks([0, np.pi / 2, np.pi, 3 * np.pi / 2])
    ax_polar.set_xticklabels([r'$0$', r'$\pi/2$', r'$\pi$', r'$3\pi/2$'])
    ax_polar.set_yticks([])
    ax_polar.grid(True, linestyle='--', alpha=0.3)
    ax_polar.legend(loc='lower right', bbox_to_anchor=(1.15, -0.08), fontsize=8)


# =============================================================================
# Master 4-panel figure
# =============================================================================
def generate_master_figure():
    plt.style.use('default')
    plt.rcParams.update(STYLE)

    fig = plt.figure(figsize=(14, 12))
    fig.suptitle(
        'Foundational Ternary Dynamics: Topological & Thermodynamic Mechanics',
        fontsize=16, fontweight='bold', y=0.97,
    )

    ax1 = fig.add_subplot(221)
    ax2 = fig.add_subplot(222)
    ax3 = fig.add_subplot(223)
    ax4 = fig.add_subplot(224, projection='polar')

    plot_lemniscate(ax1)
    plot_pt_transition(ax2)
    plot_manifestation(ax3)
    plot_moire(ax4)

    plt.tight_layout()
    fig.subplots_adjust(top=0.93, hspace=0.28, wspace=0.25)
    plt.savefig(_FIGDIR / 'FTD_Master_Figures.pdf', format='pdf', dpi=600, bbox_inches='tight')
    plt.savefig(_FIGDIR / 'FTD_Master_Figures.png', format='png', dpi=300, bbox_inches='tight')
    print(f"Saved to {_FIGDIR}")
    plt.close()


# =============================================================================
# Standalone per-paper figures
# =============================================================================
def generate_standalone_figures():
    plt.style.use('default')
    plt.rcParams.update(STYLE)

    # --- Figure 1: Lemniscate Blueprint (for paper.tex) ---
    fig1, ax1 = plt.subplots(figsize=(7, 6))
    plot_lemniscate(ax1, show_title=False)
    ax1.set_title(r"Lemniscate-$\alpha$ Topological Blueprint ($w = -2$)")
    plt.tight_layout()
    fig1.savefig(_FIGDIR / 'fig_lemniscate.pdf', format='pdf', dpi=600, bbox_inches='tight')
    fig1.savefig(_FIGDIR / 'fig_lemniscate.png', format='png', dpi=300, bbox_inches='tight')
    print("Saved fig_lemniscate")
    plt.close(fig1)

    # --- Figure 2: PT-Symmetry Phase Transition (for sonoluminescence.tex) ---
    fig2, ax2 = plt.subplots(figsize=(7, 5))
    plot_pt_transition(ax2, show_title=False)
    ax2.set_title(r"$\mathcal{PT}$-Symmetry Breaking: Sonoluminescence Threshold")
    plt.tight_layout()
    fig2.savefig(_FIGDIR / 'fig_pt_transition.pdf', format='pdf', dpi=600, bbox_inches='tight')
    fig2.savefig(_FIGDIR / 'fig_pt_transition.png', format='png', dpi=300, bbox_inches='tight')
    print("Saved fig_pt_transition")
    plt.close(fig2)

    # --- Figure 3: Manifestation Operator (for casimir_ratchet.tex) ---
    fig3, ax3 = plt.subplots(figsize=(7, 5))
    plot_manifestation(ax3, show_title=False)
    ax3.set_title(r"Manifestation Operator $\mathcal{M}(z)$: Softplus $\to$ ReLU")
    plt.tight_layout()
    fig3.savefig(_FIGDIR / 'fig_manifestation.pdf', format='pdf', dpi=600, bbox_inches='tight')
    fig3.savefig(_FIGDIR / 'fig_manifestation.png', format='png', dpi=300, bbox_inches='tight')
    print("Saved fig_manifestation")
    plt.close(fig3)

    # --- Figure 4: 137-Lobe Moiré (for casimir_ratchet.tex) ---
    fig4, ax4 = plt.subplots(figsize=(7, 7), subplot_kw={'projection': 'polar'})
    plot_moire(ax4, show_title=False)
    ax4.set_title(r"137-Lobe Spin-2 Moiré Asymmetry", pad=20)
    plt.tight_layout()
    fig4.savefig(_FIGDIR / 'fig_moire_137.pdf', format='pdf', dpi=600, bbox_inches='tight')
    fig4.savefig(_FIGDIR / 'fig_moire_137.png', format='png', dpi=300, bbox_inches='tight')
    print("Saved fig_moire_137")
    plt.close(fig4)

    # --- Figure 5: Casimir Ratchet 2-panel (PT + Manifestation) ---
    fig5, (ax5a, ax5b) = plt.subplots(1, 2, figsize=(14, 5.5))
    plot_pt_transition(ax5a, show_title=True)
    ax5a.set_title(r"A) $\mathcal{PT}$-Symmetry Phase Transition")
    plot_manifestation(ax5b, show_title=True)
    ax5b.set_title(r"B) Manifestation Operator $\mathcal{M}(z)$")
    plt.tight_layout()
    fig5.savefig(_FIGDIR / 'fig_casimir_ratchet_panels.pdf', format='pdf', dpi=600, bbox_inches='tight')
    fig5.savefig(_FIGDIR / 'fig_casimir_ratchet_panels.png', format='png', dpi=300, bbox_inches='tight')
    print("Saved fig_casimir_ratchet_panels")
    plt.close(fig5)


if __name__ == "__main__":
    generate_master_figure()
    generate_standalone_figures()
    print("\nAll figures generated successfully.")
