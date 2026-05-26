"""
Map the shape of the lemniscatic CM motive structure visible in the G*/G_G
compendium. Produces five core figures + two summary/detail diagrams:

  Fig 1: weight-vs-G_G-exponent diagonal (the universal "weight = exponent" line)
  Fig 2: vanishing pattern E_k(i) (the |Aut|=4 fingerprint)
  Fig 3: R_n family + master-quadratic roots (the (R_n, x_+, x_-) curves)
  Fig 4: 3D bigrading (weight, G_G-power, pi-power) showing the dual constants
  Fig 5: the "natural form" coefficient pattern (Bernoulli numerator denominators)
  Fig 6: the "unit period" doubling tower (eta(i) family)
  Fig 7: SUMMARY shape diagram (the unified algebraic-analytic dichotomy)

Output: Stunning high-DPI, dark-mode PNG files in docs/papers/figures/.
"""

import os
import matplotlib.pyplot as plt
import numpy as np
from mpmath import mp, mpf, pi, gamma, sqrt, exp, agm

mp.dps = 40

# Setup output directory
OUTDIR = "docs/papers/figures"
os.makedirs(OUTDIR, exist_ok=True)

# Set global matplotlib style parameters for a premium look
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['DejaVu Serif', 'Georgia', 'Times New Roman', 'serif']
plt.rcParams['text.color'] = '#e0e0f0'
plt.rcParams['axes.labelcolor'] = '#d0d0e0'
plt.rcParams['xtick.color'] = '#a0a0b8'
plt.rcParams['ytick.color'] = '#a0a0b8'

# Constants
G_G = float(1 / agm(1, sqrt(2)))
G_star = float(gamma(mpf(1) / 4) / gamma(mpf(3) / 4))
EULER_GAMMA = float(mp.euler)

# Core Palette (HSL-aligned premium colors)
COLOR_ANALYTIC = '#00e5ff'   # Electric Cyan (G_G-natural)
COLOR_ALGEBRAIC = '#ffb300'  # Warm Honey Gold (G*-natural)
COLOR_VANISHING = '#ff1744'  # Radiant Hot Red (zero points)
COLOR_BRIDGE = '#d500f9'     # Neon Purple/Magenta (the analytic-algebraic bridge)
COLOR_SUPPORT = '#4fc3f7'    # Ice Blue for guides/curves
COLOR_BG_DARK = '#0a0a14'    # Midnight Black-Blue (Figure background)
COLOR_PANEL_DARK = '#06060c' # Deeper Dark (Plot axis background)
COLOR_BORDER = '#2a2a3d'     # Border/Spine gray
COLOR_GRID = '#1e1e35'       # Gridline dark purple-gray

# ---------------------------------------------------------------------------
# Helper styling functions
# ---------------------------------------------------------------------------

def apply_premium_dark_theme(fig, ax, title="", xlabel="", ylabel="", xlim=None, ylim=None, grid=True):
    """Applies high-end dark aesthetics with clear grids, thin borders, and glowing titles."""
    fig.patch.set_facecolor(COLOR_BG_DARK)
    ax.set_facecolor(COLOR_PANEL_DARK)

    # Grid lines
    if grid:
        ax.grid(True, color=COLOR_GRID, linestyle=':', linewidth=0.8, alpha=0.8)

    # Borders (spines)
    for spine in ax.spines.values():
        spine.set_color(COLOR_BORDER)
        spine.set_linewidth(1.0)

    # Labels and Titles
    if title:
        ax.set_title(title, fontsize=12, fontweight='bold', pad=15, color='#ffffff')
    if xlabel:
        ax.set_xlabel(xlabel, fontsize=10, fontweight='normal', labelpad=8)
    if ylabel:
        ax.set_ylabel(ylabel, fontsize=10, fontweight='normal', labelpad=8)

    # View bounds
    if xlim:
        ax.set_xlim(xlim)
    if ylim:
        ax.set_ylim(ylim)

def plot_glowing_line(ax, x, y, color, label=None, linewidth=2.0, alpha_base=0.12, ls='-'):
    """Renders a continuous line with multi-layered translucent neon glows."""
    # Neon Glow Halo
    for glow_w, glow_a in [(linewidth * 5.0, alpha_base * 0.15),
                           (linewidth * 3.0, alpha_base * 0.4),
                           (linewidth * 1.8, alpha_base * 0.75)]:
        ax.plot(x, y, color=color, linewidth=glow_w, alpha=glow_a, linestyle=ls, zorder=2)
    # Intense core line
    return ax.plot(x, y, color=color, linewidth=linewidth, linestyle=ls, zorder=3, label=label)[0]

def scatter_glowing(ax, x, y, color, size=60, label=None, marker='o', edgecolors='#ffffff'):
    """Renders data points with glowing atmospheric rings and sharp centers."""
    # Outer glow rings
    ax.scatter(x, y, color=color, s=size * 4.5, alpha=0.08, marker=marker, zorder=3)
    ax.scatter(x, y, color=color, s=size * 2.2, alpha=0.22, marker=marker, zorder=3)
    # Bright solid core
    return ax.scatter(x, y, color=color, s=size, edgecolors=edgecolors, linewidths=0.7, marker=marker, zorder=4, label=label)

# ---------------------------------------------------------------------------
# Data: identities with (weight, G_G exponent, rational coefficient, π exponent, label)
# ---------------------------------------------------------------------------

identities = [
    # (weight, G_G_exp, coeff (in clean form), pi_exp_in_G_G_form, label, type)
    # Types: 'analytic' (blue-cyan), 'algebraic' (gold-orange)
    (0.5, 0.5, 1 / 2 ** 0.25, 0, r"$\eta(i)$", 'analytic'),
    (1.0, 1.0, 1 / np.sqrt(2), 0, r"$\eta(i)^2$", 'analytic'),
    (2.0, 2.0, 0.5, 0, r"$\eta(i)^4$", 'analytic'),
    (4.0, 4.0, 0.25, 0, r"$\eta(i)^8$", 'analytic'),
    (6.0, 6.0, 0.125, 0, r"$\eta(i)^{12}$", 'analytic'),
    (12.0, 12.0, 1 / 64.0, 0, r"$\eta(i)^{24} = \Delta(i)$", 'analytic'),

    # Eisenstein values
    (4.0, 4.0, 3.0, 0, r"$E_4(i)$", 'analytic'),
    (8.0, 8.0, 9.0, 0, r"$E_8(i)$", 'analytic'),
    (12.0, 12.0, 11907 / 691.0, 0, r"$E_{12}(i)$", 'analytic'),
    (16.0, 16.0, 130977 / 3617.0, 0, r"$E_{16}(i)$", 'analytic'),
    (20.0, 20.0, 12966723 / 174611.0, 0, r"$E_{20}(i)$", 'analytic'),
    (24.0, 24.0, 36216057339 / 236364091.0, 0, r"$E_{24}(i)$", 'analytic'),

    # Watson lattice
    (2.0, 2.0, 2.0, 0, r"$W_{\mathrm{BCC}}^{(3)}$", 'analytic'),

    # Periods (analytic side)
    (1.0, 1.0, 2 * np.pi, 0, r"$\omega_E$ (2$\pi$)", 'analytic'),

    # Gamma(1/4)^(2k) — algebraic side, expressed in G_G coordinates
    (1.0, 1.0, (2 * np.pi) ** 1.5, 0.5, r"$\Gamma(1/4)^2$", 'algebraic'),
    (2.0, 2.0, (2 * np.pi) ** 3.0, 1.0, r"$\Gamma(1/4)^4$", 'algebraic'),
    (4.0, 4.0, (2 * np.pi) ** 6.0, 2.0, r"$\Gamma(1/4)^8$", 'algebraic'),
]

# ---------------------------------------------------------------------------
# Fig 1: weight = G_G exponent diagonal
# ---------------------------------------------------------------------------

print("Fig 1: Universal diagonal weight = G_G exponent")
fig, ax = plt.subplots(figsize=(10, 8))

# Plot universal diagonal line
xs = np.linspace(-1, 26, 200)
plot_glowing_line(ax, xs, xs, COLOR_SUPPORT, label=r"$y = x$ (Universal Diagonal)", linewidth=1.5, ls='--')

# Filter and plot by type
weights_an = [id[0] for id in identities if id[5] == 'analytic']
exps_an = [id[1] for id in identities if id[5] == 'analytic']
labels_an = [id[4] for id in identities if id[5] == 'analytic']

weights_al = [id[0] for id in identities if id[5] == 'algebraic']
exps_al = [id[1] for id in identities if id[5] == 'algebraic']
labels_al = [id[4] for id in identities if id[5] == 'algebraic']

scatter_glowing(ax, weights_an, exps_an, COLOR_ANALYTIC, size=75, label=r"Analytic ($G_{\mathrm{G}}$-natural)")
scatter_glowing(ax, weights_al, exps_al, COLOR_ALGEBRAIC, size=75, label=r"Algebraic ($G^*$-natural)")

# Annotate points with delicate glow text
for w, e, l, t in zip(
    [id[0] for id in identities],
    [id[1] for id in identities],
    [id[4] for id in identities],
    [id[5] for id in identities]
):
    offset = (7, 4) if t == 'analytic' else (-10, -12)
    color = COLOR_ANALYTIC if t == 'analytic' else COLOR_ALGEBRAIC
    ax.annotate(
        l, (w, e), textcoords="offset points", xytext=offset,
        fontsize=9, color=color, fontweight='semibold',
        bbox=dict(boxstyle="round,pad=0.1", facecolor='#06060c90', edgecolor='none')
    )

apply_premium_dark_theme(
    fig, ax,
    title=r"The Universal Motive Pattern: $\mathrm{Weight}\ k = \mathrm{Exponent}\ d$ of $G_{\mathrm{G}}$",
    xlabel="Modular weight $k$ of the quantity",
    ylabel="Exponent of fundamental period constant in natural form",
    xlim=(-0.5, 25.5), ylim=(-0.5, 25.5)
)
ax.set_aspect("equal", adjustable="box")
legend = ax.legend(loc="upper left", frameon=True, facecolor=COLOR_PANEL_DARK, edgecolor=COLOR_BORDER)
plt.setp(legend.get_texts(), color='#e0e0f0')

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig1_weight_diagonal.png"), dpi=180, facecolor=COLOR_BG_DARK)
plt.close()
print("  saved shape_fig1_weight_diagonal.png")

# ---------------------------------------------------------------------------
# Fig 2: Vanishing pattern (E_k at tau=i)
# ---------------------------------------------------------------------------

print("\nFig 2: Vanishing pattern")
fig, ax = plt.subplots(figsize=(12, 6.5))

ks = list(range(4, 26, 2))  # 4, 6, 8, ..., 24
values = []
for k in ks:
    if k % 4 == 0:
        if k == 4: values.append(3.0)
        elif k == 8: values.append(9.0)
        elif k == 12: values.append(11907 / 691.0)
        elif k == 16: values.append(130977 / 3617.0)
        elif k == 20: values.append(12966723 / 174611.0)
        elif k == 24: values.append(36216057339 / 236364091.0)
        else: values.append(0.0)
    else:
        values.append(0.0)

# Custom premium bar colors with gradient look (darker base, glowing edge)
bar_colors = [COLOR_ANALYTIC if v > 0 else COLOR_VANISHING for v in values]

# Custom grid lines behind bars
ax.set_axisbelow(True)

bars = ax.bar(
    ks, values, color=bar_colors, edgecolor='#ffffff', linewidth=0.8,
    width=1.3, alpha=0.85, zorder=3
)

# Render glowing top markers for active bars and elegant labels
for k, v, bar in zip(ks, values, bars):
    if v > 0:
        # Subtle horizontal glow marker on top of the bar
        ax.plot([k - 0.65, k + 0.65], [v, v], color='#ffffff', linewidth=2.0, zorder=4)
        ax.text(
            k, v + max(values) * 0.025, f"{v:.4f}",
            ha="center", fontsize=9, color=COLOR_ANALYTIC, fontweight='bold'
        )
        # Fraction label below the decimal label
        if k == 12: frac = r"$\frac{11907}{691}$"
        elif k == 16: frac = r"$\frac{130977}{3617}$"
        elif k == 20: frac = r"$\frac{12966723}{174611}$"
        elif k == 24: frac = r"$\frac{36216057339}{236364091}$"
        else: frac = f"${int(v)}$"
        ax.text(
            k, v / 2.0, frac, ha="center", va="center",
            fontsize=8.5, color='#ffffff', fontweight='bold', rotation=90,
            bbox=dict(boxstyle="round,pad=0.2", facecolor='#06060cd0', edgecolor='none')
        )
    else:
        # A glowing X for vanishing forms
        ax.scatter([k], [max(values) * 0.05], marker='x', s=80, color=COLOR_VANISHING, linewidths=2.0, zorder=4)
        ax.text(k, max(values) * 0.08, "0", ha="center", fontsize=11, color=COLOR_VANISHING, fontweight='bold')

ax.set_xticks(ks)
ax.set_xticklabels([str(k) for k in ks])

apply_premium_dark_theme(
    fig, ax,
    title=r"Lattice Fingerprint: Analytic Vanishing Pattern of $E_k(i)$ under $|Aut(E)| = 4$ Symmetry",
    xlabel="Modular weight $k$ of Eisenstein series $E_k$",
    ylabel=r"Normalized value $E_k(i)\ /\ G_{\mathrm{G}}^{\,k}$",
    ylim=(-2, max(values) * 1.15)
)

# Custom legend
from matplotlib.patches import Patch
legend = ax.legend(
    handles=[
        Patch(facecolor=COLOR_ANALYTIC, edgecolor='#ffffff', alpha=0.85, label=r"Active ($k \equiv 0\text{ mod }4$; integer dimension of $M_k(\Gamma(1))$)"),
        Patch(facecolor=COLOR_VANISHING, edgecolor='#ffffff', alpha=0.85, label=r"Vanishing ($k \equiv 2\text{ mod }4$; direct algebraic reflection obstruction)")
    ],
    loc="upper left", frameon=True, facecolor=COLOR_PANEL_DARK, edgecolor=COLOR_BORDER
)
plt.setp(legend.get_texts(), color='#e0e0f0')

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig2_vanishing_pattern.png"), dpi=180, facecolor=COLOR_BG_DARK)
plt.close()
print("  saved shape_fig2_vanishing_pattern.png")

# ---------------------------------------------------------------------------
# Fig 3: R_n family + master quadratic roots
# ---------------------------------------------------------------------------

print("\nFig 3: R_n family and master quadratic roots")
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6.8))

ns = list(range(2, 30))
Rn_vals = []
xplus_vals = []
xminus_vals = []

for n in ns:
    Rn = float(gamma(mpf(1) / n) / gamma(mpf(n - 1) / n))
    Rn_vals.append(Rn)
    disc = (16.0 * Rn**2) ** 2 - 4.0 * 16.0 * Rn**3
    if disc < 0:
        xplus_vals.append(np.nan)
        xminus_vals.append(np.nan)
    else:
        xp = (16.0 * Rn**2 + np.sqrt(disc)) / 2.0
        xm = (16.0 * Rn**2 - np.sqrt(disc)) / 2.0
        xplus_vals.append(xp)
        xminus_vals.append(xm)

# Left Subplot: R_n linear growth and asymptotic convergence
plot_glowing_line(ax1, ns, Rn_vals, COLOR_ANALYTIC, label=r"Lemniscatic family $R_n = \frac{\Gamma(1/n)}{\Gamma(1 - 1/n)}$", linewidth=2.2)
plot_glowing_line(ax1, ns, [n - 2.0 * EULER_GAMMA for n in ns], COLOR_SUPPORT, label=r"Asymptotic linear limit $n - 2\gamma$", linewidth=1.2, ls='--')

# Highlight R_4 = G* with a glowing gold star
scatter_glowing(ax1, [4], [G_star], COLOR_ALGEBRAIC, size=240, marker='*', edgecolors='#ffffff')
ax1.annotate(
    r"$R_4 = G^* \approx 2.9587$",
    (4, G_star),
    xytext=(15, -5),
    textcoords="offset points",
    fontsize=10.5,
    color=COLOR_ALGEBRAIC,
    fontweight='bold',
    bbox=dict(boxstyle="round,pad=0.2", facecolor='#06060ce0', edgecolor=COLOR_BORDER)
)

apply_premium_dark_theme(
    fig, ax1,
    title=r"Linear Growth of the $\Gamma$-Quotient Family $R_n$",
    xlabel="Lattice dimension coordinate $n$",
    ylabel="Ratio $R_n$"
)
legend1 = ax1.legend(loc="upper left", frameon=True, facecolor=COLOR_PANEL_DARK, edgecolor=COLOR_BORDER)
plt.setp(legend1.get_texts(), color='#e0e0f0')

# Right Subplot: Master quadratic roots x_+ and x_- (Log plot)
plot_glowing_line(ax2, Rn_vals, xplus_vals, COLOR_ANALYTIC, label=r"$x_+(R_n)$ (large root - dispositional energy)", linewidth=2.2)
plot_glowing_line(ax2, Rn_vals, xminus_vals, COLOR_ALGEBRAIC, label=r"$x_-(R_n)$ (small root - manifested charge)", linewidth=2.2)

# Asymptotics
plot_glowing_line(ax2, Rn_vals, [16.0 * r**2 for r in Rn_vals], COLOR_ANALYTIC, label=r"Asymptotic limit $16 R_n^2$", linewidth=1.0, ls=':')
plot_glowing_line(ax2, Rn_vals, [r + 0.0625 for r in Rn_vals], COLOR_ALGEBRAIC, label=r"Asymptotic limit $R_n + 1/16$", linewidth=1.0, ls=':')

# Highlight x_+(R_4) ≈ 1/alpha and x_-(R_4) ≈ N_c
scatter_glowing(ax2, [G_star], [137.035999], COLOR_ANALYTIC, size=180, marker='*', edgecolors='#ffffff')
scatter_glowing(ax2, [G_star], [3.024], COLOR_ALGEBRAIC, size=180, marker='*', edgecolors='#ffffff')

ax2.annotate(
    r"$x_+(R_4) \approx 137.036\ (1/\alpha)$ [Conjecture]",
    (G_star, 137.035999),
    xytext=(10, -12),
    textcoords="offset points",
    fontsize=9.5,
    color=COLOR_ANALYTIC,
    fontweight='bold',
    bbox=dict(boxstyle="round,pad=0.2", facecolor='#06060ce0', edgecolor=COLOR_BORDER)
)
ax2.annotate(
    r"$x_-(R_4) \approx 3.024\ (N_c \approx 3)$ [Selection]",
    (G_star, 3.024),
    xytext=(10, 8),
    textcoords="offset points",
    fontsize=9.5,
    color=COLOR_ALGEBRAIC,
    fontweight='bold',
    bbox=dict(boxstyle="round,pad=0.2", facecolor='#06060ce0', edgecolor=COLOR_BORDER)
)

ax2.set_yscale("log")
apply_premium_dark_theme(
    fig, ax2,
    title=r"Master-Quadratic Roots $x_\pm(R_n) = 8R_n^2 \pm 4R_n^{3/2}\sqrt{4R_n - 1}$",
    xlabel="Lemniscatic constant $R_n$",
    ylabel=r"Roots $x_{\pm}$ (Log Scale)"
)
legend2 = ax2.legend(loc="lower right", frameon=True, facecolor=COLOR_PANEL_DARK, edgecolor=COLOR_BORDER)
plt.setp(legend2.get_texts(), color='#e0e0f0')

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig3_Rn_family.png"), dpi=180, facecolor=COLOR_BG_DARK)
plt.close()
print("  saved shape_fig3_Rn_family.png")

# ---------------------------------------------------------------------------
# Fig 4: 3D bigrading
# ---------------------------------------------------------------------------

print("\nFig 4: 3D bigrading (weight, G_G power, pi power)")
fig = plt.figure(figsize=(12, 10))
ax = fig.add_subplot(111, projection="3d")

identities_3d = [
    # G_G natural side (pi exponent 0)
    (1.0, 1.0, 0.0, r"$\eta(i)^2$", 'analytic'),
    (2.0, 2.0, 0.0, r"$\eta(i)^4$", 'analytic'),
    (4.0, 4.0, 0.0, r"$E_4(i) = 3G_G^4$", 'analytic'),
    (8.0, 8.0, 0.0, r"$E_8(i) = 9G_G^8$", 'analytic'),
    (12.0, 12.0, 0.0, r"$\Delta(i) = \frac{G_G^{12}}{64}$", 'analytic'),
    (2.0, 2.0, 0.0, r"$W_{\mathrm{BCC}}^{(3)} = 2G_G^2$", 'analytic'),

    # G* natural side (pi exponent k/2 in G_G coordinates)
    (1.0, 1.0, 0.5, r"$\Gamma(1/4)^2$", 'algebraic'),
    (2.0, 2.0, 1.0, r"$\Gamma(1/4)^4$", 'algebraic'),
    (4.0, 4.0, 2.0, r"$\Gamma(1/4)^8$", 'algebraic'),
    (3.0, 3.0, 1.5, r"MQ: $G^{*3}$ term", 'algebraic'),
    (2.0, 2.0, 1.0, r"MQ: $G^{*2}$ term", 'algebraic'),
]

# Extract values
xs_3d = [id[0] for id in identities_3d]
ys_3d = [id[1] for id in identities_3d]
zs_3d = [id[2] for id in identities_3d]
colors_3d = [COLOR_ANALYTIC if id[4] == 'analytic' else COLOR_ALGEBRAIC for id in identities_3d]

# Plot transparent planes for G_G and G* natural families
xx, yy = np.meshgrid(np.linspace(0, 13, 15), np.linspace(0, 13, 15))
zz_floor = np.zeros_like(xx)
zz_tilted = xx / 2.0

# Base panels / backgrounds for 3D plot
fig.patch.set_facecolor(COLOR_BG_DARK)
ax.set_facecolor(COLOR_PANEL_DARK)

# Render transparent sheets with thin glowing borders
ax.plot_surface(xx, yy, zz_floor, alpha=0.15, color=COLOR_ANALYTIC, edgecolor='#00e5ff30', linewidth=0.5, zorder=1)
ax.plot_surface(xx, yy, zz_tilted, alpha=0.15, color=COLOR_ALGEBRAIC, edgecolor='#ffb30030', linewidth=0.5, zorder=1)

# Diagonal guides
diag_x = np.linspace(0, 13, 100)
ax.plot(diag_x, diag_x, np.zeros_like(diag_x), color=COLOR_ANALYTIC, linestyle="--", alpha=0.6, linewidth=1.5, zorder=2)
ax.plot(diag_x, diag_x, diag_x / 2.0, color=COLOR_ALGEBRAIC, linestyle="--", alpha=0.6, linewidth=1.5, zorder=2)

# Scatter 3D points with glowing halo
for x, y, z, c in zip(xs_3d, ys_3d, zs_3d, colors_3d):
    # Core point
    ax.scatter([x], [y], [z], color=c, s=120, edgecolors='#ffffff', linewidths=0.8, depthshade=False, zorder=4)
    # Glow ring
    ax.scatter([x], [y], [z], color=c, s=350, alpha=0.15, depthshade=False, zorder=3)

# Text labels with clean offsets
for x, y, z, l, t in identities_3d:
    c = COLOR_ANALYTIC if t == 'analytic' else COLOR_ALGEBRAIC
    ax.text(x, y, z + 0.08, l, color=c, fontsize=8.5, fontweight='bold', zorder=5)

# Styling details for 3D Axes
ax.xaxis.pane.fill = True
ax.yaxis.pane.fill = True
ax.zaxis.pane.fill = True
ax.xaxis.pane.set_facecolor('#040409')
ax.yaxis.pane.set_facecolor('#040409')
ax.zaxis.pane.set_facecolor('#040409')

ax.xaxis.line.set_color(COLOR_BORDER)
ax.yaxis.line.set_color(COLOR_BORDER)
ax.zaxis.line.set_color(COLOR_BORDER)

ax.tick_params(colors='#a0a0b8', which='both', labelsize=9)
ax.set_xlabel("Modular Weight $k$", fontsize=10, labelpad=12)
ax.set_ylabel(r"$G_{\mathrm{G}}$ Exponent $d$", fontsize=10, labelpad=12)
ax.set_zlabel(r"$\pi$ Exponent $p$ (in $G_{\mathrm{G}}$ units)", fontsize=10, labelpad=12)

ax.set_title(
    "The 3D Bigraded Motive Shape: Analytic vs Algebraic Planes\n"
    + r"(Analytic Floor: $p \equiv 0\text{ mod }1$ | Algebraic Ceiling: $p = k/2$ bridged by $G^* = 2\sqrt{\pi} G_{\mathrm{G}}$)",
    fontsize=12, fontweight='bold', color='#ffffff', pad=25
)

# Rotate to a pristine viewing perspective
ax.view_init(elev=22, azim=-62)

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig4_bigrading_3d.png"), dpi=180, facecolor=COLOR_BG_DARK)
plt.close()
print("  saved shape_fig4_bigrading_3d.png")

# ---------------------------------------------------------------------------
# Fig 5: Bernoulli numerator pattern
# ---------------------------------------------------------------------------

print("\nFig 5: Bernoulli numerator pattern in Eisenstein denominators")
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6.8))

ms = [1, 2, 3, 4, 5, 6]
weights_4m = [4 * m for m in ms]
numerators = [3, 9, 11907, 130977, 12966723, 36216057339]
B_nums = [1, 1, 691, 3617, 174611, 236364091]
coefficients = [n / b for n, b in zip(numerators, B_nums)]

# Left: Log of Bernoulli numerator vs weight
plot_glowing_line(ax1, weights_4m, B_nums, COLOR_ANALYTIC, label=r"Bernoulli numerator $|B_{4m}^{\mathrm{num}}|$", linewidth=2.2)
scatter_glowing(ax1, weights_4m, B_nums, COLOR_ANALYTIC, size=100)

for w, b in zip(weights_4m, B_nums):
    ax1.annotate(
        f"{b:,}", (w, b), textcoords="offset points", xytext=(8, 5),
        fontsize=9, color=COLOR_ANALYTIC, fontweight='bold',
        bbox=dict(boxstyle="round,pad=0.2", facecolor='#06060cd0', edgecolor='none')
    )

apply_premium_dark_theme(
    fig, ax1,
    title=r"Modular Denominators are exactly Bernoulli Numerators",
    xlabel="Modular weight $k = 4m$",
    ylabel=r"Absolute Bernoulli Numerator $|B_{k}^{\mathrm{num}}|$ (Log Scale)"
)
ax1.set_yscale("log")
legend1 = ax1.legend(loc="upper left", frameon=True, facecolor=COLOR_PANEL_DARK, edgecolor=COLOR_BORDER)
plt.setp(legend1.get_texts(), color='#e0e0f0')

# Right: Actual coefficients (Rational scaling factors)
plot_glowing_line(ax2, weights_4m, coefficients, COLOR_ALGEBRAIC, label=r"Value ratio $E_{4m}(i)\ /\ G_{\mathrm{G}}^{4m}$", linewidth=2.2)
scatter_glowing(ax2, weights_4m, coefficients, COLOR_ALGEBRAIC, size=100)

for w, c, n, b in zip(weights_4m, coefficients, numerators, B_nums):
    if b == 1:
        label = f"${int(c)}$"
    else:
        label = rf"$\frac{{{n:,}}}{{{b:,}}}$"
    ax2.annotate(
        label, (w, c), textcoords="offset points", xytext=(8, -5),
        fontsize=9.5, color=COLOR_ALGEBRAIC, fontweight='bold',
        bbox=dict(boxstyle="round,pad=0.2", facecolor='#06060cd0', edgecolor='none')
    )

apply_premium_dark_theme(
    fig, ax2,
    title=r"Rational Scaling Coefficients of $E_{4m}(i)/G_{\mathrm{G}}^{4m}$",
    xlabel="Modular weight $k = 4m$",
    ylabel="Rational Coefficient (Log Scale)"
)
ax2.set_yscale("log")
legend2 = ax2.legend(loc="upper left", frameon=True, facecolor=COLOR_PANEL_DARK, edgecolor=COLOR_BORDER)
plt.setp(legend2.get_texts(), color='#e0e0f0')

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig5_bernoulli_pattern.png"), dpi=180, facecolor=COLOR_BG_DARK)
plt.close()
print("  saved shape_fig5_bernoulli_pattern.png")

# ---------------------------------------------------------------------------
# Fig 6: The "unit period" decomposition (eta(i) tower and Delta hierarchy)
# ---------------------------------------------------------------------------

print("\nFig 6: eta(i) doubling tower")
fig, ax = plt.subplots(figsize=(10, 7.5))

ks = np.linspace(0.5, 14.0, 200)
eta_logs = ks * np.log(G_G) - (ks / 2.0) * np.log(2.0)

ks_pts = [1.0, 2.0, 4.0, 6.0, 12.0]
eta_pts = [G_G**k / 2.0 ** (k / 2.0) for k in ks_pts]
labels_pts = [r"$\eta(i)^2$", r"$\eta(i)^4$", r"$\eta(i)^8$", r"$\eta(i)^{12}$", r"$\eta(i)^{24} = \Delta(i)$"]

# Plot smooth exponential curve
plot_glowing_line(ax, ks, np.exp(eta_logs), COLOR_ANALYTIC, label=r"Analytic tower curve $G_{\mathrm{G}}^k\ /\ 2^{k/2}$", linewidth=2.0)
scatter_glowing(ax, ks_pts, eta_pts, COLOR_ALGEBRAIC, size=110, label="Exact modular forms at $\tau = i$")

for k, e, l in zip(ks_pts, eta_pts, labels_pts):
    ax.annotate(
        l, (k, e), textcoords="offset points", xytext=(12, 5),
        fontsize=10.5, color='#ffffff', fontweight='bold',
        bbox=dict(boxstyle="round,pad=0.25", facecolor='#06060ce0', edgecolor=COLOR_BORDER)
    )

ax.set_yscale("log")
apply_premium_dark_theme(
    fig, ax,
    title=r"The Doubling Tower: $\eta(i)^{2k} = G_{\mathrm{G}}^k / 2^{k/2}$",
    xlabel=r"Tower weight parameter $k$ (For fractional exponents $\eta(i)^{2k}$)",
    ylabel=r"Absolute Value at $\tau = i$ (Log Scale)"
)
legend = ax.legend(loc="upper right", frameon=True, facecolor=COLOR_PANEL_DARK, edgecolor=COLOR_BORDER)
plt.setp(legend.get_texts(), color='#e0e0f0')

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig6_doubling_tower.png"), dpi=180, facecolor=COLOR_BG_DARK)
plt.close()
print("  saved shape_fig6_doubling_tower.png")

# ---------------------------------------------------------------------------
# Fig 7: SUMMARY — the "shape" as a single unified picture
# ---------------------------------------------------------------------------

print("\nFig 7: SUMMARY shape diagram")
fig, ax = plt.subplots(figsize=(14, 9.5))

# Conceptual layout
ax.axhline(y=0.0, color=COLOR_BORDER, linewidth=1.5, alpha=0.8, zorder=1)
ax.text(25.5, 0.05, "Trivial Parity Plane (Weight axis)", fontsize=10, color=COLOR_BORDER, va="bottom", ha="right", fontweight='bold')

# Data elements
gg_identities = [
    (0.5, 0.6, r"$\eta(i) = 2^{-1/4} G_{\mathrm{G}}^{1/2}$"),
    (1.0, 1.1, r"$\eta^2 = G_{\mathrm{G}}/\sqrt{2}$"),
    (2.0, 1.5, r"$\eta^4 = G_{\mathrm{G}}^2/2$"),
    (2.0, 1.8, r"$W_{\mathrm{BCC}}^{(3)} = 2 G_{\mathrm{G}}^2$"),
    (4.0, 2.1, r"$E_4 = 3 G_{\mathrm{G}}^4$"),
    (8.0, 2.4, r"$E_8 = 9 G_{\mathrm{G}}^8$"),
    (12.0, 2.7, r"$\Delta = G_{\mathrm{G}}^{12}/64$"),
    (12.0, 3.0, r"$E_{12}$"),
    (16.0, 3.3, r"$E_{16}$"),
    (20.0, 3.6, r"$E_{20}$"),
    (24.0, 3.9, r"$E_{24}$"),
]

gstar_identities = [
    (1.0, -0.6, r"$\Gamma(1/4)^2 = \pi\sqrt{2}\, G^*$"),
    (2.0, -1.1, r"$\Gamma(1/4)^4 = 2\pi^2 G^{*2}$"),
    (1.0, -1.5, r"$\omega_E = G^* \sqrt{\pi}$"),
    (1.0, -1.9, r"$B(1/4,1/4) = \sqrt{2\pi}\, G^*$"),
    (2.0, -2.3, r"MQ Coefficient: $-16G^{*2}$"),
    (3.0, -2.7, r"MQ Constant: $16G^{*3}$"),
    (0.0, -3.1, r"$\Gamma(1/4)\Gamma(3/4) = \pi\sqrt{2}$ [Ref.]"),
]

zero_weights = [
    (6.0, 0.4, "$E_6 = 0$"),
    (10.0, 0.4, "$E_{10} = 0$"),
    (14.0, 0.4, "$E_{14} = 0$"),
    (18.0, 0.4, "$E_{18} = 0$"),
    (22.0, 0.4, "$E_{22} = 0$")
]

# Plot G_G (analytic) points in glowing cyan
for w, y, l in gg_identities:
    scatter_glowing(ax, [w], [y], COLOR_ANALYTIC, size=90)
    ax.annotate(
        l, (w, y), textcoords="offset points", xytext=(10, -2),
        fontsize=9, color=COLOR_ANALYTIC, fontweight='bold',
        bbox=dict(boxstyle="round,pad=0.15", facecolor='#06060cd0', edgecolor='none')
    )

# Plot G* (algebraic) points in glowing orange/gold
for w, y, l in gstar_identities:
    scatter_glowing(ax, [w], [y], COLOR_ALGEBRAIC, size=90)
    ax.annotate(
        l, (w, y), textcoords="offset points", xytext=(10, -2),
        fontsize=9, color=COLOR_ALGEBRAIC, fontweight='bold',
        bbox=dict(boxstyle="round,pad=0.15", facecolor='#06060cd0', edgecolor='none')
    )

# Plot zero weights as glowing neon red crosses
for w, y, l in zero_weights:
    ax.scatter([w], [y], s=140, c=COLOR_PANEL_DARK, edgecolors=COLOR_VANISHING, marker="X", linewidths=1.5, zorder=4)
    # Glow halo for crosses
    ax.scatter([w], [y], s=350, c=COLOR_VANISHING, alpha=0.1, marker="X", zorder=3)
    ax.annotate(
        l, (w, y), textcoords="offset points", xytext=(0, -18),
        fontsize=8.5, color=COLOR_VANISHING, fontweight='bold', ha="center",
        bbox=dict(boxstyle="round,pad=0.15", facecolor='#06060cd0', edgecolor='none')
    )

# Draw elegant shaded background regions for the two branches (Glassmorphism look)
from matplotlib.patches import FancyBboxPatch
ax.add_patch(FancyBboxPatch((-0.5, 0.25), 25.5, 4.0, boxstyle="round,pad=0.1", facecolor='#00e5ff07', edgecolor='#00e5ff1b', linewidth=1.0, zorder=0))
ax.add_patch(FancyBboxPatch((-0.5, -3.6), 25.5, 3.4, boxstyle="round,pad=0.1", facecolor='#ffb30007', edgecolor='#ffb3001b', linewidth=1.0, zorder=0))

# Shading branch titles
ax.text(12.5, 4.35, r"ANALYTIC BRANCH ($G_{\mathrm{G}}$-Natural / Period Lattice Floor / Transcendental Integrals)", fontsize=11, ha="center", color=COLOR_ANALYTIC, fontweight='bold', zorder=2)
ax.text(12.5, -3.95, r"ALGEBRAIC BRANCH ($G^*$-Natural / CM Motive Reflection Plane / Arithmetic $L$-Functions)", fontsize=11, ha="center", color=COLOR_ALGEBRAIC, fontweight='bold', zorder=2)

# Draw the unified motive bridge
ax.annotate(
    "", xy=(0.5, -0.2), xytext=(0.5, 0.2),
    arrowprops=dict(arrowstyle="<->", color=COLOR_BRIDGE, lw=2.5, mutation_scale=15)
)
ax.text(
    1.1, 0.0, r"$\mathrm{UNIFIED\ BRIDGE}:\ G^* = 2\sqrt{\pi}\, G_{\mathrm{G}}$",
    fontsize=11, color=COLOR_BRIDGE, fontweight='bold', va="center",
    bbox=dict(boxstyle="round,pad=0.3", facecolor='#06060cf0', edgecolor=COLOR_BRIDGE, linewidth=1.0)
)

apply_premium_dark_theme(
    fig, ax,
    title=r"The Motive Shape: $\chi_{-4}$ Algebraic-Analytic Dichotomy of Lemniscatic Integrals",
    xlabel="Modular Weight $k$",
    ylabel="", grid=False
)

ax.set_yticks([])
ax.set_xlim(-1, 26)
ax.set_ylim(-4.2, 5.0)

# Custom vertical grid lines
ax.xaxis.grid(True, color=COLOR_GRID, linestyle=':', linewidth=0.8, alpha=0.7)

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig7_summary.png"), dpi=180, facecolor=COLOR_BG_DARK)
plt.close()
print("  saved shape_fig7_summary.png")

print("\n" + "=" * 70)
print("All premium, dark-mode figures successfully updated in docs/papers/figures/")
print("=" * 70)
for f in sorted(os.listdir(OUTDIR)):
    if f.startswith("shape_"):
        print(f"  [UPDATED] {f}")
