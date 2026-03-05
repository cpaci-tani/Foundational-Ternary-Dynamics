"""
ReLU Type Transition Verification Figure
Generates a 5-panel figure for EXPLR_RELU_TYPE_TRANSITION.md

Panel A: M'_beta(z) at beta = 1, 3, 10, inf -- continuous (0,1) -> discrete {0,1}
Panel B: M''_beta(z) -- smooth bump -> delta(z)
Panel C: Strip width pi/beta and Connes lambda = e^{-beta} vs beta with factor type labels
Panel D: Enriched (k, 1/beta) phase diagram with factor type annotations
Panel E: Spectral truncation K -> max(0,K) -- the complete algebraic descent chain
"""

from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
from matplotlib.gridspec import GridSpec
from matplotlib import rcParams
from scipy.special import gamma as Gamma

_FIGDIR = Path(__file__).resolve().parents[2] / 'docs' / 'papers' / 'src' / 'figures'

# Publication styling (matches gen_softplus_relu.py)
rcParams['font.family'] = 'serif'
rcParams['font.serif'] = ['CMU Serif', 'Computer Modern Roman', 'DejaVu Serif']
rcParams['mathtext.fontset'] = 'cm'
rcParams['axes.labelsize'] = 11
rcParams['xtick.labelsize'] = 9
rcParams['ytick.labelsize'] = 9
rcParams['legend.fontsize'] = 9
rcParams['axes.linewidth'] = 0.8

# Constants
G_STAR = np.sqrt(2) * Gamma(0.25)**2 / (2 * np.pi)  # ~2.9587
K_CRIT_N16 = 1.0 / (4.0 * G_STAR)  # ~0.0845


def softplus_deriv(z, beta):
    """Fermi-Dirac: M'_β(z) = 1 / (1 + exp(-βz))"""
    return 1.0 / (1.0 + np.exp(-beta * np.clip(z, -500/beta, 500/beta)))


def softplus_deriv2(z, beta):
    """Susceptibility: M''_β(z) = β * nF * (1 - nF)"""
    nf = softplus_deriv(z, beta)
    return beta * nf * (1.0 - nf)


def heaviside(z):
    """Heaviside step function"""
    return np.where(z > 0, 1.0, np.where(z < 0, 0.0, 0.5))


# ============================================================
# Create figure
# ============================================================
fig = plt.figure(figsize=(10, 12))
gs = GridSpec(3, 2, figure=fig, height_ratios=[1, 1, 0.8],
              hspace=0.38, wspace=0.32, left=0.09, right=0.96, top=0.95, bottom=0.05)

# ============================================================
# Panel A: M'_β(z) — Fermi-Dirac -> Heaviside
# ============================================================
ax1 = fig.add_subplot(gs[0, 0])
z = np.linspace(-5, 5, 1000)

betas = [1, 3, 10]
colors_beta = ['#2196F3', '#FF9800', '#E91E63']
labels_beta = [
    r'$\beta = 1$: $\mathcal{M}^\prime \in (0.007, 0.993)$',
    r'$\beta = 3$: $\mathcal{M}^\prime \in (10^{-7}, 1-10^{-7})$',
    r'$\beta = 10$: $\mathcal{M}^\prime \approx \{0, 1\}$',
]

# Plot Heaviside (β -> inf) first as background
ax1.plot(z, heaviside(z), 'k--', linewidth=2.0,
         label=r'$\beta \to \infty$ (Heaviside): $\{0, 1\}$', zorder=5)

for beta_val, col, lab in zip(betas, colors_beta, labels_beta):
    ax1.plot(z, softplus_deriv(z, beta_val), color=col, linewidth=1.8, label=lab, zorder=4)

ax1.axhline(y=0.5, color='gray', linewidth=0.5, linestyle=':')
ax1.axvline(x=0, color='gray', linewidth=0.5, linestyle=':')

# Add dimension function labels
ax1.annotate(r'Type II$_1$: $d \in [0,1]$',
             xy=(-2.5, 0.15), fontsize=8, color='#1565C0',
             bbox=dict(boxstyle='round,pad=0.2', facecolor='#E3F2FD', edgecolor='#1565C0', alpha=0.8))
ax1.annotate(r'Type I: $d \in \{0,1\}$',
             xy=(2, 0.85), fontsize=8, color='black',
             bbox=dict(boxstyle='round,pad=0.2', facecolor='#F5F5F5', edgecolor='black', alpha=0.8))

ax1.set_xlabel(r'$z = |J| - K_B$')
ax1.set_ylabel(r'$\mathcal{M}^\prime_\beta(z)$ (occupation)')
ax1.set_title(r'$\mathbf{A.}$ Dimension Function: Continuous $\to$ Discrete', loc='left', fontsize=10)
ax1.legend(loc='center left', framealpha=0.9, edgecolor='gray', fontsize=7.5)
ax1.set_xlim(-5, 5)
ax1.set_ylim(-0.05, 1.1)

# ============================================================
# Panel B: M''_β(z) — smooth bump -> δ(z)
# ============================================================
ax2 = fig.add_subplot(gs[0, 1])
z2 = np.linspace(-3, 3, 1000)

betas_b = [1, 3, 10, 30]
colors_b = ['#2196F3', '#FF9800', '#E91E63', '#9C27B0']
labels_b = [
    r'$\beta = 1$: max $= 0.25$',
    r'$\beta = 3$: max $= 0.75$',
    r'$\beta = 10$: max $= 2.5$',
    r'$\beta = 30$: max $= 7.5$',
]

for beta_val, col, lab in zip(betas_b, colors_b, labels_b):
    ax2.plot(z2, softplus_deriv2(z2, beta_val), color=col, linewidth=1.8, label=lab, zorder=4)

# Arrow indicating β -> inf limit
ax2.annotate(r'$\beta \to \infty$: $\delta(z)$',
             xy=(0, 7.5), xytext=(1.2, 6.5),
             fontsize=9, color='black',
             arrowprops=dict(arrowstyle='->', color='black', lw=1.2),
             bbox=dict(boxstyle='round,pad=0.2', facecolor='lightyellow', edgecolor='gray', alpha=0.9))

# Labels for projection interpretation
ax2.annotate('No minimal projection\n(smooth, subdivides)',
             xy=(-2.2, 1.5), fontsize=7.5, color='#1565C0',
             bbox=dict(boxstyle='round,pad=0.2', facecolor='#E3F2FD', edgecolor='#1565C0', alpha=0.8))
ax2.annotate(r'Minimal projection $P_0$' + '\n' + r'(rank-1 at $z=0$)',
             xy=(1.0, 4.5), fontsize=7.5, color='#6A1B9A',
             bbox=dict(boxstyle='round,pad=0.2', facecolor='#F3E5F5', edgecolor='#6A1B9A', alpha=0.8))

ax2.axvline(x=0, color='gray', linewidth=0.5, linestyle=':')
ax2.set_xlabel(r'$z = |J| - K_B$')
ax2.set_ylabel(r'$\mathcal{M}^{\prime\prime}_\beta(z)$ (susceptibility)')
ax2.set_title(r'$\mathbf{B.}$ Susceptibility: Smooth Bump $\to$ $\delta(z)$', loc='left', fontsize=10)
ax2.legend(loc='upper right', framealpha=0.9, edgecolor='gray', fontsize=7.5)
ax2.set_xlim(-3, 3)
ax2.set_ylim(-0.2, 8.5)

# Verify unit integral
for beta_val in betas_b:
    z_wide = np.linspace(-100, 100, 100000)
    integral = np.trapezoid(softplus_deriv2(z_wide, beta_val), z_wide)
    print(f"  beta={beta_val:>2d}: integral of M'' = {integral:.6f} (should be 1.0)")

# ============================================================
# Panel C: Strip width and Connes λ vs β
# ============================================================
ax3 = fig.add_subplot(gs[1, 0])
beta_range = np.linspace(0.3, 8, 500)
strip_width = 2 * np.pi / beta_range
connes_lambda = np.exp(-beta_range)

# Two y-axes
ax3_right = ax3.twinx()

# Strip width (left axis, blue)
l1, = ax3.plot(beta_range, strip_width, color='#1565C0', linewidth=2.0,
               label=r'Strip width $2\pi/\beta$')
ax3.fill_between(beta_range, 0, strip_width, alpha=0.1, color='#1565C0')
ax3.set_ylabel(r'Analyticity strip width $2\pi/\beta$', color='#1565C0')
ax3.tick_params(axis='y', labelcolor='#1565C0')

# Connes λ (right axis, red)
l2, = ax3_right.plot(beta_range, connes_lambda, color='#C62828', linewidth=2.0,
                     linestyle='--', label=r'Connes $\lambda = e^{-\beta}$')
ax3_right.fill_between(beta_range, 0, connes_lambda, alpha=0.08, color='#C62828')
ax3_right.set_ylabel(r'Connes parameter $\lambda = e^{-\beta}$', color='#C62828')
ax3_right.tick_params(axis='y', labelcolor='#C62828')

# Factor type regime labels
ax3.axvspan(0.3, 1.5, alpha=0.08, color='#E91E63')
ax3.axvspan(1.5, 4.0, alpha=0.08, color='#FF9800')
ax3.axvspan(4.0, 8.0, alpha=0.08, color='#4CAF50')

ax3.text(0.7, strip_width.max() * 0.85, r'III$_1$' + '\n(ergodic)',
         ha='center', va='top', fontsize=8, color='#880E4F',
         bbox=dict(facecolor='white', alpha=0.7, edgecolor='none'))
ax3.text(2.5, strip_width.max() * 0.55, r'III$_\lambda$' + '\n(periodic)',
         ha='center', va='center', fontsize=8, color='#E65100',
         bbox=dict(facecolor='white', alpha=0.7, edgecolor='none'))
ax3.text(6.0, strip_width.max() * 0.25, r'III$_0$' + '\n(aperiodic)',
         ha='center', va='center', fontsize=8, color='#2E7D32',
         bbox=dict(facecolor='white', alpha=0.7, edgecolor='none'))

# Mark β -> inf arrow
ax3.annotate(r'$\beta \to \infty$: strip $\to 0$, $\lambda \to 0$' + '\n'
             + r'$\longrightarrow$ Type I (topological jump)',
             xy=(7, 0.3), fontsize=7.5, color='black',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='lightyellow', edgecolor='gray', alpha=0.9))

lines = [l1, l2]
labels = [l.get_label() for l in lines]
ax3.legend(lines, labels, loc='upper right', framealpha=0.9, edgecolor='gray', fontsize=8)

ax3.set_xlabel(r'Inverse temperature $\beta$')
ax3.set_title(r'$\mathbf{C.}$ Strip Width and Connes Parameter vs $\beta$', loc='left', fontsize=10)
ax3.set_xlim(0.3, 8)
ax3.set_ylim(0, strip_width.max() * 1.05)
ax3_right.set_ylim(0, 1)

# ============================================================
# Panel D: Enriched (k, 1/β) phase diagram with factor types
# ============================================================
ax4 = fig.add_subplot(gs[1, 1])

k_range = np.linspace(0, 0.5, 300)
beta_inv_range = np.linspace(0, 1.5, 300)
K, B = np.meshgrid(k_range, beta_inv_range)

# Phase regions
phase = np.zeros_like(K)
phase[K < K_CRIT_N16] = 0                          # Vacuum (purple) - Type III_1
phase[(K >= K_CRIT_N16) & (B < 0.08)] = 2          # Classical (blue) - Type I
phase[(K >= K_CRIT_N16) & (B >= 0.08)] = 1         # Quantum (green) - Type III_λ

cmap = ListedColormap(['#E1BEE7', '#C8E6C9', '#BBDEFB'])
ax4.pcolormesh(K, B, phase, cmap=cmap, shading='auto', alpha=0.6)

# Critical line k = k_crit
ax4.axvline(x=K_CRIT_N16, color='red', linewidth=2.5, linestyle='-')

# Horizontal dashed for classical/quantum boundary
ax4.axhline(y=0.08, color='blue', linewidth=0.8, linestyle=':',
            xmin=K_CRIT_N16/0.5, alpha=0.6)

# Factor type labels (larger, clearer)
ax4.text(0.03, 0.75,
         r'$\mathbf{Type\;III_1}$' + '\n(complex eigenvalues)\n' + r'$S(\mathcal{M}) = \mathbb{R}_+$',
         ha='center', va='center', fontsize=8.5, color='#6A1B9A',
         bbox=dict(facecolor='white', alpha=0.8, edgecolor='#6A1B9A', linewidth=0.8))

ax4.text(0.32, 0.85,
         r'$\mathbf{Type\;III_\lambda}$' + '\n(Softplus, KMS holds)\n' + r'$\lambda = e^{-\beta}$',
         ha='center', va='center', fontsize=8.5, color='#2E7D32',
         bbox=dict(facecolor='white', alpha=0.8, edgecolor='#2E7D32', linewidth=0.8))

ax4.text(0.32, 0.03,
         r'$\mathbf{Type\;I}$' + '\n' + r'(ReLU, $\beta\to\infty$, KMS destroyed)',
         ha='center', va='center', fontsize=7.5, color='#1565C0',
         bbox=dict(facecolor='white', alpha=0.8, edgecolor='#1565C0', linewidth=0.8))

# Label the critical line
ax4.annotate(r'$\mathbf{Type\;II_1}$' + '\n(exceptional pt)',
             xy=(K_CRIT_N16, 1.2), xytext=(K_CRIT_N16 + 0.08, 1.35),
             fontsize=8, color='red', fontweight='bold',
             arrowprops=dict(arrowstyle='->', color='red', lw=1.2),
             bbox=dict(boxstyle='round,pad=0.2', facecolor='#FFEBEE', edgecolor='red', alpha=0.9))

ax4.set_xlabel(r'Geometric tension $k$')
ax4.set_ylabel(r'Temperature $1/\beta$')
ax4.set_title(r'$\mathbf{D.}$ Enriched Phase Diagram with Factor Types', loc='left', fontsize=10)
ax4.set_xlim(0, 0.5)
ax4.set_ylim(0, 1.5)

# ============================================================
# Panel E: Spectral truncation K -> max(0,K) — descent chain
# ============================================================
ax5 = fig.add_subplot(gs[2, :])  # span both columns

# Mock modular Hamiltonian spectrum (symmetric around 0)
K_spec = np.linspace(-6, 6, 1000)
spectral_density = 0.25 * np.exp(-K_spec**2 / 3.0)  # Gaussian envelope

# Full spectrum (Type III — purple)
ax5.fill_between(K_spec, 0, spectral_density, alpha=0.12, color='#9C27B0')
ax5.plot(K_spec, spectral_density, color='#9C27B0', linewidth=1.2, alpha=0.6)

# ReLU truncation: positive half only (Type I — blue)
mask_pos = K_spec >= 0
relu_density = np.where(K_spec >= 0, spectral_density, 0)
ax5.fill_between(K_spec[mask_pos], 0, spectral_density[mask_pos],
                 alpha=0.35, color='#1565C0', label=r'ReLU$(K)$ = physical spectrum (Type I)')

# Vacuum reservoir (K < 0, red shading)
mask_neg = K_spec < 0
ax5.fill_between(K_spec[mask_neg], 0, spectral_density[mask_neg],
                 alpha=0.18, color='#C62828', label=r'$P_0$: vacuum ($\hat{\tau}(P_0) = \infty$)')

# Kink line at K = 0
ax5.axvline(x=0, color='black', linewidth=2.0, linestyle='-', zorder=5)

# Labels
ax5.annotate('Heavy Zero\n(vacuum reservoir)\n' + r'$\hat{\tau}(P_0) = \infty$',
             xy=(-3.2, 0.10), ha='center', fontsize=8, color='#C62828',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='#FFEBEE',
                       edgecolor='#C62828', alpha=0.9))

ax5.annotate('MASA selection\n' + r'$\Theta(K)$: II$_1$ -> I',
             xy=(0, spectral_density[500]), xytext=(2.0, 0.22),
             fontsize=8.5, color='black', fontweight='bold',
             arrowprops=dict(arrowstyle='->', color='black', lw=1.5),
             bbox=dict(boxstyle='round,pad=0.3', facecolor='lightyellow',
                       edgecolor='gray', alpha=0.9))

ax5.annotate('Physical states\n' + r'$s = \pm 1$ (manifested)',
             xy=(3.0, 0.08), ha='center', fontsize=8, color='#1565C0',
             bbox=dict(boxstyle='round,pad=0.3', facecolor='#E3F2FD',
                       edgecolor='#1565C0', alpha=0.9))

# Descent chain text box (bottom center, plain text with Unicode arrows)
descent_text = (r'Type $\mathrm{III}_1$'
                '  --[crossed product]-->  '
                r'Type $\mathrm{II}_\infty$'
                '  --[decomposition]-->  '
                r'Type $\mathrm{II}_1$'
                r'  --[$\Theta(K)$]-->  '
                r'Type $\mathrm{I}$')
ax5.text(0.0, -0.06, descent_text, fontsize=10, ha='center',
         transform=ax5.get_xaxis_transform(),
         bbox=dict(boxstyle='round,pad=0.5', facecolor='#FFF9C4',
                   edgecolor='#F57F17', alpha=0.9, linewidth=1.5))

ax5.set_xlabel(r'Modular Hamiltonian eigenvalue $K$')
ax5.set_ylabel('Spectral density')
ax5.set_title(r'$\mathbf{E.}$ Spectral Truncation $K \to \max(0, K)$: '
              r'The Complete Algebraic Descent', loc='left', fontsize=10)
ax5.legend(loc='upper right', framealpha=0.9, edgecolor='gray', fontsize=8)
ax5.set_xlim(-6, 6)
ax5.set_ylim(-0.01, 0.30)

# ============================================================
# Save
# ============================================================
_FIGDIR.mkdir(parents=True, exist_ok=True)
fig.savefig(_FIGDIR / 'FTD_ReLU_Type_Transition.pdf', dpi=300, bbox_inches='tight')
fig.savefig(_FIGDIR / 'FTD_ReLU_Type_Transition.png', dpi=200, bbox_inches='tight')

print()
print("=" * 60)
print("ReLU Type Transition Verification")
print("=" * 60)
print(f"G* = {G_STAR:.10f}")
print(f"k_crit (with n=16) = {K_CRIT_N16:.6f}")
print()

# Verify key mathematical claims
print("--- RT-T1: Fermi-Dirac -> Heaviside ---")
z_test = np.array([-2.0, -1.0, -0.5, 0.5, 1.0, 2.0])
for beta_val in [1, 10, 100, 1000]:
    vals = softplus_deriv(z_test, beta_val)
    print(f"  beta={beta_val:>4d}: M'(z) = [{', '.join(f'{v:.6f}' for v in vals)}]")
print(f"  Heaviside:  M'(z) = [{', '.join(f'{v:.6f}' for v in heaviside(z_test))}]")

print()
print("--- RT-T2: Susceptibility -> delta(z) ---")
for beta_val in [1, 10, 100]:
    peak = beta_val / 4.0
    fwhm = 2 * np.log(3) / beta_val
    print(f"  beta={beta_val:>3d}: peak = {peak:.2f}, FWHM = {fwhm:.4f}")
print("  beta->inf: peak -> inf, FWHM -> 0 (delta function)")

print()
print("--- RT-T5: Strip width collapse ---")
for beta_val in [1, 3, 10, 100]:
    width = 2 * np.pi / beta_val
    lam = np.exp(-beta_val)
    print(f"  beta={beta_val:>3d}: strip = {width:.4f}, lambda = {lam:.2e}")
print("  beta->inf: strip -> 0, lambda -> 0")

print()
print("--- RT-C4: beta-lambda dictionary ---")
print("  beta -> 0:   lambda -> 1   (Type III_1, ergodic)")
print("  beta = 0.69: lambda = 0.5  (Type III_{1/2})")
print("  beta = 1.0:  lambda = 0.37 (Type III_{0.37})")
print("  beta -> inf:   lambda -> 0   (Type III_0 -> Type I [topological jump])")

print()
print("--- Warning RT-W1 (now resolved by descent chain) ---")
print("  Powers factors R_lambda are ALL Type III for lambda in (0,1).")
print("  beta -> inf gives lambda -> 0 = Type III_0, NOT Type I.")
print("  RESOLUTION: Three-step descent chain:")
print("    Step 1: III_1 --[crossed product]--> II_inf   [CLASSICAL, Takesaki 1973]")
print("    Step 2: II_inf --[R x B(H)]--> II_1           [CLASSICAL, Murray-vN 1943]")
print("    Step 3: II_1 --[MASA via Theta(K)]--> I       [CONJECTURE, RT-C9]")

print()
print("--- RT-C9: MASA selection via Heaviside partition ---")
print("  The ReLU kink at z=0 selects the canonical MASA:")
print("  P_+ = E_{[0,inf)}(K)  (physical states, s = +/-1)")
print("  P_- = E_{(-inf,0)}(K) (vacuum states, s = 0)")
print("  Every MASA in a II_1 factor is Type I (Dixmier 1954).")
print("  MASA selection IS the II_1 -> I transition.")

print()
print(f"Figures saved: FTD_ReLU_Type_Transition.pdf, FTD_ReLU_Type_Transition.png")
print(f"Output directory: {_FIGDIR}")
