"""
Map the shape of the lemniscatic CM motive structure visible in the G*/G_G
compendium. Produces five figures:

  Fig 1: weight-vs-G_G-exponent diagonal (the universal "weight = exponent" line)
  Fig 2: vanishing pattern E_k(i) (the |Aut|=4 fingerprint)
  Fig 3: R_n family + master-quadratic roots (the (R_n, x_+, x_-) curves)
  Fig 4: 3D bigrading (weight, G_G-power, pi-power) showing the dual constants
  Fig 5: the "natural form" coefficient pattern (Bernoulli numerator denominators)

Output: PNG files in docs/papers/figures/.
"""

import os
import matplotlib.pyplot as plt
import numpy as np
from mpmath import mp, mpf, pi, gamma, sqrt, exp, agm

mp.dps = 30

# Setup output directory
OUTDIR = "docs/papers/figures"
os.makedirs(OUTDIR, exist_ok=True)

# Constants
G_G = float(1 / agm(1, sqrt(2)))
G_star = float(gamma(mpf(1) / 4) / gamma(mpf(3) / 4))
EULER_GAMMA = float(mp.euler)

# ---------------------------------------------------------------------------
# Data: identities with (weight, G_G exponent, rational coefficient, π exponent, label)
# ---------------------------------------------------------------------------

identities = [
    # (weight, G_G_exp, coeff (in clean form), pi_exp_in_G_G_form, label)
    # Modular forms at tau=i (clean in G_G)
    (0.5, 0.5, 1 / 2 ** 0.25, 0, r"$\eta(i)$"),
    (1, 1, 1 / np.sqrt(2), 0, r"$\eta(i)^2$"),
    (2, 2, 1 / 2, 0, r"$\eta(i)^4$"),
    (4, 4, 1 / 4, 0, r"$\eta(i)^8$"),
    (6, 6, 1 / 8, 0, r"$\eta(i)^{12}$"),
    (12, 12, 1 / 64, 0, r"$\eta(i)^{24} = \Delta(i)$"),
    # Eisenstein values
    (4, 4, 3.0, 0, r"$E_4(i)$"),
    (8, 8, 9.0, 0, r"$E_8(i)$"),
    (12, 12, 11907 / 691, 0, r"$E_{12}(i)$"),
    (16, 16, 130977 / 3617, 0, r"$E_{16}(i)$"),
    (20, 20, 12966723 / 174611, 0, r"$E_{20}(i)$"),
    (24, 24, 36216057339 / 236364091, 0, r"$E_{24}(i)$"),
    # Watson lattice
    (2, 2, 2.0, 0, r"$W_{BCC}^{(3)}$"),
    # Periods (analytic side)
    (1, 1, 2 * np.pi, 0, r"$\omega_E$ (2π factor)"),
    # Gamma(1/4)^(2k) — algebraic side, expressed in G_G
    # Gamma(1/4)^(2k) = (2π)^(3k/2) G_G^k
    (1, 1, (2 * np.pi) ** 1.5, 0, r"$\Gamma(1/4)^2$"),
    (2, 2, (2 * np.pi) ** 3, 0, r"$\Gamma(1/4)^4$"),
    (4, 4, (2 * np.pi) ** 6, 0, r"$\Gamma(1/4)^8$"),
]

# Vanishing weights
zero_weights_existing = [(k, 0, 0, 0, f"$E_{{{k}}}(i)=0$") for k in [6, 10, 14, 18, 22]]

# ---------------------------------------------------------------------------
# Fig 1: weight = G_G exponent diagonal
# ---------------------------------------------------------------------------

print("Fig 1: Universal diagonal weight = G_G exponent")
fig, ax = plt.subplots(figsize=(10, 8))

# Plot diagonal line
xs = np.linspace(0, 25, 100)
ax.plot(xs, xs, "k--", alpha=0.3, linewidth=1, label="$y = x$ (universal)")

# Plot identities
weights = [id[0] for id in identities]
exps = [id[1] for id in identities]
labels = [id[4] for id in identities]
ax.scatter(weights, exps, s=60, c="#1f77b4", edgecolors="navy", zorder=3)

# Labels for distinguishing
for w, e, l in zip(weights, exps, labels):
    ax.annotate(
        l, (w, e), textcoords="offset points", xytext=(7, 5), fontsize=8, alpha=0.85
    )

ax.set_xlabel("Modular weight $N$ of the quantity", fontsize=12)
ax.set_ylabel("Exponent of $G_{\\mathrm{G}}$ in the natural form", fontsize=12)
ax.set_title(
    r"The universal pattern: $\mathrm{weight} = \mathrm{exponent}\ \mathrm{of}\ G_{\mathrm{G}}$",
    fontsize=13,
)
ax.set_xlim(-0.5, 25)
ax.set_ylim(-0.5, 25)
ax.grid(True, alpha=0.3)
ax.legend(loc="upper left")
ax.set_aspect("equal", adjustable="box")

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig1_weight_diagonal.png"), dpi=150)
plt.close()
print("  saved shape_fig1_weight_diagonal.png")

# ---------------------------------------------------------------------------
# Fig 2: Vanishing pattern (E_k at tau=i)
# ---------------------------------------------------------------------------

print()
print("Fig 2: Vanishing pattern")
fig, ax = plt.subplots(figsize=(12, 6))

ks = list(range(4, 26, 2))  # 4, 6, 8, ..., 24
values = []
for k in ks:
    if k % 4 == 0:
        # Compute the coefficient
        if k == 4:
            values.append(3.0)
        elif k == 8:
            values.append(9.0)
        elif k == 12:
            values.append(11907 / 691)
        elif k == 16:
            values.append(130977 / 3617)
        elif k == 20:
            values.append(12966723 / 174611)
        elif k == 24:
            values.append(36216057339 / 236364091)
        else:
            values.append(0.0)
    else:
        values.append(0.0)

colors = ["#1f77b4" if v > 0 else "#d62728" for v in values]
bars = ax.bar(ks, values, color=colors, edgecolor="black", linewidth=0.6, width=1.5)

# Label the bars
for k, v, bar in zip(ks, values, bars):
    if v > 0:
        ax.text(
            k,
            v + max(values) * 0.02,
            f"$E_{{{k}}}(i)/G_{{\\mathrm{{G}}}}^{{{k}}}={v:.3f}$",
            ha="center",
            fontsize=8,
        )
    else:
        ax.text(k, max(values) * 0.05, "0", ha="center", fontsize=11, color="darkred")

ax.set_xticks(ks)
ax.set_xticklabels([str(k) for k in ks])
ax.set_xlabel("Weight $k$ of Eisenstein series $E_k$", fontsize=12)
ax.set_ylabel(r"$E_k(i)\,/\,G_{\mathrm{G}}^{\,k}$", fontsize=12)
ax.set_title(
    r"Vanishing pattern: $E_k(i) = 0$ for $k \not\equiv 0 $ mod $4$"
    + " (fingerprint of $|\\mathrm{Aut}(E)|=4$)",
    fontsize=12,
)
ax.set_ylim(0, max(values) * 1.15)
ax.grid(True, alpha=0.3, axis="y")

# Legend
from matplotlib.patches import Patch

ax.legend(
    handles=[
        Patch(facecolor="#1f77b4", edgecolor="black", label="non-zero (weight ≡ 0 mod 4)"),
        Patch(facecolor="#d62728", edgecolor="black", label="vanishing (weight ≡ 2 mod 4)"),
    ],
    loc="upper left",
)

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig2_vanishing_pattern.png"), dpi=150)
plt.close()
print("  saved shape_fig2_vanishing_pattern.png")

# ---------------------------------------------------------------------------
# Fig 3: R_n family + master quadratic roots
# ---------------------------------------------------------------------------

print()
print("Fig 3: R_n family and master quadratic roots")
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

# Compute R_n for many n
ns = list(range(2, 30))
Rn_vals = []
xplus_vals = []
xminus_vals = []
xminus_minus_Rn = []

for n in ns:
    Rn = float(gamma(mpf(1) / n) / gamma(mpf(n - 1) / n))
    Rn_vals.append(Rn)
    disc = (16 * Rn**2) ** 2 - 4 * 16 * Rn**3
    if disc < 0:
        xplus_vals.append(np.nan)
        xminus_vals.append(np.nan)
        xminus_minus_Rn.append(np.nan)
    else:
        xp = (16 * Rn**2 + np.sqrt(disc)) / 2
        xm = (16 * Rn**2 - np.sqrt(disc)) / 2
        xplus_vals.append(xp)
        xminus_vals.append(xm)
        xminus_minus_Rn.append(xm - Rn)

# Left: R_n vs n with asymptotic
ax1.plot(ns, Rn_vals, "o-", color="#1f77b4", markersize=6, label=r"$R_n = \Gamma(1/n)/\Gamma((n-1)/n)$")
ax1.plot(ns, [n - 2 * EULER_GAMMA for n in ns], "--", color="#7f7f7f", alpha=0.6, label=r"$n - 2\gamma$ (asymptotic)")
# Mark R_4 = G*
ax1.scatter([4], [G_star], s=200, color="red", marker="*", zorder=5, label=r"$R_4 = G^* \approx 2.9587$")
ax1.set_xlabel("$n$", fontsize=12)
ax1.set_ylabel("$R_n$", fontsize=12)
ax1.set_title(r"The family $R_n$ — linear growth", fontsize=12)
ax1.grid(True, alpha=0.3)
ax1.legend(loc="upper left")

# Right: master quadratic roots x_+ and x_- as functions of R_n
ax2.plot(Rn_vals, xplus_vals, "o-", color="#1f77b4", markersize=5, label=r"$x_+(R_n)$ (large root)")
ax2.plot(Rn_vals, xminus_vals, "o-", color="#ff7f0e", markersize=5, label=r"$x_-(R_n)$ (small root)")
ax2.plot(Rn_vals, [16 * r**2 for r in Rn_vals], "--", color="#1f77b4", alpha=0.4, label=r"$16 R_n^2$ (asymptotic for $x_+$)")
ax2.plot(Rn_vals, [r + 1 / 16 for r in Rn_vals], "--", color="#ff7f0e", alpha=0.4, label=r"$R_n + 1/16$ (asymptotic for $x_-$)")

# Mark x_+(R_4) ≈ 1/α and x_-(R_4) ≈ N_c
ax2.scatter([G_star], [137.036171], s=200, color="red", marker="*", zorder=5)
ax2.scatter([G_star], [3.024], s=200, color="red", marker="*", zorder=5)
ax2.annotate(
    r"$x_+(R_4) \approx 1/\alpha$",
    (G_star, 137.036),
    xytext=(8, -15),
    textcoords="offset points",
    fontsize=10,
    color="darkred",
)
ax2.annotate(
    r"$x_-(R_4) \approx N_c \approx 3$",
    (G_star, 3.024),
    xytext=(8, 8),
    textcoords="offset points",
    fontsize=10,
    color="darkred",
)

ax2.set_yscale("log")
ax2.set_xlabel("$R_n$", fontsize=12)
ax2.set_ylabel("Roots of master quadratic", fontsize=12)
ax2.set_title(r"Master-quadratic roots $x_\pm(R_n) = 8R_n^2 \pm 4R_n^{3/2}\sqrt{4R_n - 1}$", fontsize=11)
ax2.grid(True, alpha=0.3, which="both")
ax2.legend(loc="center right", fontsize=9)

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig3_Rn_family.png"), dpi=150)
plt.close()
print("  saved shape_fig3_Rn_family.png")

# ---------------------------------------------------------------------------
# Fig 4: 3D bigrading
# ---------------------------------------------------------------------------

print()
print("Fig 4: 3D bigrading (weight, G_G power, pi power)")
fig = plt.figure(figsize=(11, 9))
ax = fig.add_subplot(111, projection="3d")

# Each identity has natural-form coordinates:
#   (weight, G_G power, pi power) in G_G natural form -- pi power = 0
#   (weight, G* power, pi power) in G* natural form -- pi power = weight/2 (extra sqrt pi per unit)
identities_3d = [
    # G_G natural side (pi exponent 0): clean
    (1, 1, 0, r"$\eta(i)^2$"),
    (2, 2, 0, r"$\eta(i)^4$"),
    (4, 4, 0, r"$E_4(i)\!\!=\!\!3G_G^4$"),
    (8, 8, 0, r"$E_8(i)\!\!=\!\!9G_G^8$"),
    (12, 12, 0, r"$\Delta(i)\!=\!G_G^{12}/64$"),
    (2, 2, 0, r"$W_{BCC}\!\!=\!\!2G_G^2$"),
    # G* natural side (pi exponent k/2 in G_G coordinates from G* = 2 sqrt pi G_G)
    (1, 1, 0.5, r"$\Gamma(1/4)^2$"),
    (2, 2, 1, r"$\Gamma(1/4)^4$"),
    (4, 4, 2, r"$\Gamma(1/4)^8$"),
    (3, 3, 1.5, r"MQ: $G^{*3}$ term"),
    (2, 2, 1, r"MQ: $G^{*2}$ term"),
]

# Color by pi-power: 0 = analytic clean (blue), >0 = algebraic (orange)
colors_3d = ["#1f77b4" if id[2] == 0 else "#ff7f0e" for id in identities_3d]
xs_3d = [id[0] for id in identities_3d]
ys_3d = [id[1] for id in identities_3d]
zs_3d = [id[2] for id in identities_3d]
ax.scatter(xs_3d, ys_3d, zs_3d, c=colors_3d, s=70, edgecolors="black", linewidth=0.5)

# Annotate
for x, y, z, l in identities_3d:
    ax.text(x, y, z + 0.05, l, fontsize=7, alpha=0.85)

# Plot the planes:
# G_G natural: pi-exponent = 0 (the floor)
xx, yy = np.meshgrid(np.linspace(0, 13, 10), np.linspace(0, 13, 10))
zz0 = np.zeros_like(xx)
zz_diag = (
    xx / 2
)  # for G* natural form, the algebraic identities lie on this plane (z = weight/2)
ax.plot_surface(xx, yy, zz0, alpha=0.15, color="#1f77b4")  # G_G plane
ax.plot_surface(xx, yy, zz_diag, alpha=0.10, color="#ff7f0e")  # G* plane

# Also plot the y = x diagonal (the universal weight = exponent rule)
diag_x = np.linspace(0, 13, 50)
ax.plot(diag_x, diag_x, np.zeros_like(diag_x), color="black", linestyle="--", alpha=0.5, linewidth=1)
ax.plot(diag_x, diag_x, diag_x / 2, color="black", linestyle="--", alpha=0.5, linewidth=1)

ax.set_xlabel("Weight $N$", fontsize=11, labelpad=8)
ax.set_ylabel("$G_{\\mathrm{G}}$ exponent", fontsize=11, labelpad=8)
ax.set_zlabel("$\\pi$ exponent (in $G_{\\mathrm{G}}$ coords)", fontsize=11, labelpad=8)
ax.set_title(
    r"The bigraded shape: identities live on two parallel planes"
    + "\n"
    + r"(blue floor = $G_{\mathrm{G}}$-natural / analytic; orange tilt = $G^*$-natural / algebraic)",
    fontsize=11,
)

# View angle
ax.view_init(elev=20, azim=-65)

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig4_bigrading_3d.png"), dpi=150)
plt.close()
print("  saved shape_fig4_bigrading_3d.png")

# ---------------------------------------------------------------------------
# Fig 5: Bernoulli numerator pattern
# ---------------------------------------------------------------------------

print()
print("Fig 5: Bernoulli numerator pattern in Eisenstein denominators")
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

# E_{4m}(i) / G_G^{4m} = numerator / Bernoulli numerator
ms = [1, 2, 3, 4, 5, 6]
weights_4m = [4 * m for m in ms]
numerators = [3, 9, 11907, 130977, 12966723, 36216057339]
B_nums = [1, 1, 691, 3617, 174611, 236364091]
coefficients = [n / b for n, b in zip(numerators, B_nums)]

# Left: log of Bernoulli numerator vs weight
ax1.semilogy(weights_4m, B_nums, "o-", color="#1f77b4", markersize=10, linewidth=2, label="$|B_{4m}^{\\mathrm{num}}|$ (Bernoulli numerator)")
# Annotate values
for w, b in zip(weights_4m, B_nums):
    ax1.annotate(
        f"{b}",
        (w, b),
        textcoords="offset points",
        xytext=(7, 5),
        fontsize=9,
    )
ax1.set_xlabel("Weight $4m$", fontsize=12)
ax1.set_ylabel(r"$|B_{4m}^{\mathrm{num}}|$ (Bernoulli numerator)", fontsize=12)
ax1.set_title(r"Denominators of $E_{4m}(i)/G_{\mathrm{G}}^{4m}$ — exactly the Bernoulli numerators", fontsize=11)
ax1.grid(True, alpha=0.3, which="both")
ax1.legend(loc="upper left")

# Right: actual coefficients
ax2.semilogy(weights_4m, coefficients, "o-", color="#ff7f0e", markersize=10, linewidth=2)
for w, c, n, b in zip(weights_4m, coefficients, numerators, B_nums):
    if b == 1:
        ax2.annotate(
            f"{int(c)}",
            (w, c),
            textcoords="offset points",
            xytext=(7, 5),
            fontsize=9,
        )
    else:
        ax2.annotate(
            f"$\\frac{{{n}}}{{{b}}}$",
            (w, c),
            textcoords="offset points",
            xytext=(7, 5),
            fontsize=8,
        )
ax2.set_xlabel("Weight $4m$", fontsize=12)
ax2.set_ylabel(r"$E_{4m}(i)/G_{\mathrm{G}}^{4m}$", fontsize=12)
ax2.set_title(r"Rational coefficients of Eisenstein values at $\tau=i$", fontsize=11)
ax2.grid(True, alpha=0.3, which="both")

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig5_bernoulli_pattern.png"), dpi=150)
plt.close()
print("  saved shape_fig5_bernoulli_pattern.png")

# ---------------------------------------------------------------------------
# Fig 6: The "unit period" decomposition (eta(i) tower and Delta hierarchy)
# ---------------------------------------------------------------------------

print()
print("Fig 6: eta(i) doubling tower")
fig, ax = plt.subplots(figsize=(10, 7))

# eta(i)^(2k) = G_G^k / 2^(k/2)
# log eta(i)^(2k) = k log(G_G) - (k/2) log(2)
ks = np.linspace(0.5, 14, 100)
eta_logs = ks * np.log(G_G) - (ks / 2) * np.log(2)

# Points: k = 1, 2, 4, 6, 12 -> eta(i)^2, ^4, ^8, ^12, ^24
ks_pts = [1, 2, 4, 6, 12]
eta_pts = [G_G**k / 2 ** (k / 2) for k in ks_pts]
labels_pts = [r"$\eta(i)^2$", r"$\eta(i)^4$", r"$\eta(i)^8$", r"$\eta(i)^{12}$", r"$\eta(i)^{24}=\Delta(i)$"]

ax.plot(ks, np.exp(eta_logs), "k--", alpha=0.4, linewidth=1, label=r"$G_{\mathrm{G}}^k / 2^{k/2}$ (continuous)")
ax.scatter(ks_pts, eta_pts, s=120, c="#1f77b4", edgecolors="navy", zorder=3, label="known evaluations")

for k, e, l in zip(ks_pts, eta_pts, labels_pts):
    ax.annotate(l, (k, e), textcoords="offset points", xytext=(10, 8), fontsize=10)

ax.set_yscale("log")
ax.set_xlabel(r"$k$ (with $\eta(i)^{2k}$)", fontsize=12)
ax.set_ylabel(r"$\eta(i)^{2k}$", fontsize=12)
ax.set_title(
    r"The doubling tower: $\eta(i)^{2k} = G_{\mathrm{G}}^k / 2^{k/2}$"
    + " (single exponential family)",
    fontsize=12,
)
ax.grid(True, alpha=0.3, which="both")
ax.legend(loc="upper right")

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig6_doubling_tower.png"), dpi=150)
plt.close()
print("  saved shape_fig6_doubling_tower.png")

# ---------------------------------------------------------------------------
# Fig 7: SUMMARY — the "shape" as a single picture
# ---------------------------------------------------------------------------

print()
print("Fig 7: SUMMARY shape diagram")
fig, ax = plt.subplots(figsize=(13, 9))

# Conceptual diagram: weight as horizontal axis; the two "branches" (G_G and G*)
# diverge at integer weights as a kind of staircase

# Plot the weight axis
ax.axhline(y=0, color="black", linewidth=1, alpha=0.4)
ax.text(13.5, 0, "weight axis", fontsize=11, va="center")

# Plot G_G-natural identities as points above the axis
gg_identities = [
    (0.5, 0.5, r"$\eta(i) = 2^{-1/4} G_{\mathrm{G}}^{1/2}$"),
    (1, 1, r"$\eta^2 = G_{\mathrm{G}}/\sqrt{2}$"),
    (2, 1.5, r"$\eta^4 = G_{\mathrm{G}}^2/2$"),
    (2, 1.7, r"$W_{BCC} = 2 G_{\mathrm{G}}^2$"),
    (4, 2.0, r"$E_4 = 3 G_{\mathrm{G}}^4$"),
    (8, 2.3, r"$E_8 = 9 G_{\mathrm{G}}^8$"),
    (12, 2.6, r"$\Delta = G_{\mathrm{G}}^{12}/64$"),
    (12, 2.9, r"$E_{12}$"),
    (16, 3.2, r"$E_{16}$"),
    (20, 3.5, r"$E_{20}$"),
    (24, 3.8, r"$E_{24}$"),
]

# Plot G*-natural identities as points below the axis
gstar_identities = [
    (1, -0.5, r"$\Gamma(1/4)^2 = \pi\sqrt{2}\, G^*$"),
    (2, -1, r"$\Gamma(1/4)^4 = 2\pi^2 G^{*2}$"),
    (1, -1.4, r"$\omega_E = G^* \sqrt{\pi}$"),
    (1, -1.8, r"$B(1/4,1/4) = \sqrt{2\pi}\, G^*$"),
    (2, -2.2, r"MQ coeff $-16G^{*2}$"),
    (3, -2.6, r"MQ const $16G^{*3}$"),
    (0, -3.0, r"$\Gamma(1/4)\Gamma(3/4) = \pi\sqrt{2}$ (product)"),
]

# Zero weights (vanishing modular forms)
zero_weights = [(6, 0.5, "$E_6=0$"), (10, 0.5, "$E_{10}=0$"), (14, 0.5, "$E_{14}=0$"), (18, 0.5, "$E_{18}=0$"), (22, 0.5, "$E_{22}=0$")]

# Plot G_G side
for w, y, l in gg_identities:
    ax.scatter([w], [y], s=80, c="#1f77b4", edgecolors="navy", zorder=3)
    ax.annotate(l, (w, y), textcoords="offset points", xytext=(8, 0), fontsize=8, color="navy")

# Plot G* side
for w, y, l in gstar_identities:
    ax.scatter([w], [y], s=80, c="#ff7f0e", edgecolors="darkred", zorder=3)
    ax.annotate(l, (w, y), textcoords="offset points", xytext=(8, 0), fontsize=8, color="darkred")

# Plot zero weights as X marks
for w, y, l in zero_weights:
    ax.scatter([w], [y], s=100, c="white", edgecolors="red", marker="X", zorder=3)
    ax.annotate(l, (w, y), textcoords="offset points", xytext=(0, -15), fontsize=8, color="red", ha="center")

# Sketch the two regions
from matplotlib.patches import FancyBboxPatch

ax.add_patch(FancyBboxPatch((-0.5, 0.2), 25, 4, boxstyle="round,pad=0.1", facecolor="#1f77b4", alpha=0.1, edgecolor="none"))
ax.add_patch(FancyBboxPatch((-0.5, -3.5), 25, 3.3, boxstyle="round,pad=0.1", facecolor="#ff7f0e", alpha=0.1, edgecolor="none"))

ax.text(12, 4.3, r"$G_{\mathrm{G}}$-natural (analytic)", fontsize=15, ha="center", color="navy", weight="bold")
ax.text(12, -3.9, r"$G^*$-natural (algebraic)", fontsize=15, ha="center", color="darkred", weight="bold")

# Draw the bridge
ax.annotate(
    "", xy=(0.5, -0.2), xytext=(0.5, 0.2), arrowprops=dict(arrowstyle="<->", color="purple", lw=2)
)
ax.text(1, 0, r"bridge: $G^* = 2\sqrt{\pi}\, G_{\mathrm{G}}$", fontsize=10, color="purple")

ax.set_xlim(-1, 25)
ax.set_ylim(-4.5, 5)
ax.set_xlabel("Modular weight $N$", fontsize=12)
ax.set_yticks([])
ax.set_title(
    r"The shape: two parallel families ($G_{\mathrm{G}}$ above, $G^*$ below) bridged by $G^* = 2\sqrt{\pi}\, G_{\mathrm{G}}$,"
    + "\n"
    + r"with vanishing weights $k \equiv 2 $ mod $4$ marked X (fingerprint of $|\mathrm{Aut}(E)|=4$)",
    fontsize=11,
)
ax.grid(True, alpha=0.3, axis="x")

plt.tight_layout()
plt.savefig(os.path.join(OUTDIR, "shape_fig7_summary.png"), dpi=150)
plt.close()
print("  saved shape_fig7_summary.png")

print()
print("=" * 70)
print("All figures saved to docs/papers/figures/")
print("=" * 70)
print("Files:")
for f in sorted(os.listdir(OUTDIR)):
    if f.startswith("shape_"):
        print(f"  {f}")
