#!/usr/bin/env python3
"""Generate figures for the alpha precision paper."""

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from mpmath import mp, mpf, gamma, sqrt, pi, exp

mp.dps = 60

# ---- Compute all values ----
g14 = gamma(mpf("1") / 4)
G_star = sqrt(2) * g14**2 / (2 * pi)
a_c = 16 * G_star**2
b_c = 16 * G_star**3
disc = a_c**2 - 4 * b_c
x_plus = (a_c + sqrt(disc)) / 2
eps_val = abs(exp(pi) - pi - 20)
codata = mpf("137.035999177")

c = [mpf(9) / 47, mpf(5) / 64, mpf(4) / 141, mpf(141) / 11]
signs = [-1, +1, -1, -1]

cumulative = [x_plus]
for i in range(4):
    cumulative.append(cumulative[-1] + signs[i] * c[i] * eps_val ** (i + 1))
alpha_inv = cumulative[-1]

residuals = [float(abs(v - codata)) for v in cumulative]
terms = [float(c[i] * eps_val ** (i + 1)) for i in range(4)]

OUTDIR = "figures"

# ==== FIGURE 1: Convergence ====
fig, ax = plt.subplots(figsize=(7, 4.5))
loop_labels = ["Bare $x_+$", "1-term", "2-term", "3-term", "4-term"]
colors = ["#c0392b", "#e67e22", "#2ecc71", "#3498db", "#8e44ad"]

ax.semilogy(range(5), residuals, "ko-", markersize=8, linewidth=2, zorder=5)
for i, (r, col) in enumerate(zip(residuals, colors)):
    ax.semilogy(i, r, "o", color=col, markersize=12, zorder=6)

ax.set_xticks(range(5))
ax.set_xticklabels(loop_labels, fontsize=10)
ax.set_ylabel(r"$|\alpha^{-1}_{\mathrm{formula}} - \alpha^{-1}_{\mathrm{CODATA}}|$", fontsize=12)
ax.set_title("Convergence of the Precision Formula", fontsize=13, fontweight="bold")

ax.axhspan(0, 2.1e-8, alpha=0.15, color="gray", label=r"CODATA uncertainty ($\pm 21 \times 10^{-9}$)")
ax.axhline(2.1e-8, color="gray", linestyle="--", alpha=0.5)

for i, r in enumerate(residuals):
    ax.annotate(f"{r:.1e}", (i, r), textcoords="offset points", xytext=(12, 5), fontsize=8, color="#333")

ax.set_ylim(1e-15, 1e-2)
ax.legend(loc="upper right", fontsize=9)
ax.grid(True, alpha=0.3, which="both")
plt.tight_layout()
plt.savefig(f"{OUTDIR}/fig_convergence.pdf", dpi=300, bbox_inches="tight")
plt.savefig(f"{OUTDIR}/fig_convergence.png", dpi=300, bbox_inches="tight")
print("Figure 1 saved")
plt.close()

# ==== FIGURE 2: Coefficient Structure ====
fig, axes = plt.subplots(1, 4, figsize=(10, 3.5))

coeff_data = [
    {
        "label": r"$c_1 = \frac{9}{47}$",
        "num": 9,
        "den": 47,
        "num_expr": r"$N_c^2 = 3^2$",
        "den_expr": r"$16 \times 3 - 1$",
        "color": "#e74c3c",
    },
    {
        "label": r"$c_2 = \frac{5}{64}$",
        "num": 5,
        "den": 64,
        "num_expr": r"$N_{eff} - 2N_{base}$",
        "den_expr": r"$N_{base}^3 = 4^3$",
        "color": "#f39c12",
    },
    {
        "label": r"$c_3 = \frac{4}{141}$",
        "num": 4,
        "den": 141,
        "num_expr": r"$N_{base} = 4$",
        "den_expr": r"$N_c \times D$",
        "color": "#2ecc71",
    },
    {
        "label": r"$c_4 = \frac{141}{11}$",
        "num": 141,
        "den": 11,
        "num_expr": r"$N_c \times D$",
        "den_expr": r"$b_3 + N_{base}$",
        "color": "#3498db",
    },
]

for ax, cd in zip(axes, coeff_data):
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.set_aspect("equal")
    ax.axis("off")
    ax.text(0.5, 0.95, cd["label"], ha="center", va="top", fontsize=14, fontweight="bold")
    ax.plot([0.2, 0.8], [0.55, 0.55], color=cd["color"], linewidth=3)
    ax.text(0.5, 0.72, str(cd["num"]), ha="center", va="center", fontsize=20, fontweight="bold", color=cd["color"])
    ax.text(0.5, 0.62, cd["num_expr"], ha="center", va="center", fontsize=8, color="#555")
    ax.text(0.5, 0.38, str(cd["den"]), ha="center", va="center", fontsize=20, fontweight="bold", color=cd["color"])
    ax.text(0.5, 0.28, cd["den_expr"], ha="center", va="center", fontsize=8, color="#555")
    val = cd["num"] / cd["den"]
    ax.text(0.5, 0.1, f"= {val:.6f}", ha="center", va="center", fontsize=9, color="#777")

fig.suptitle(r"Rational Coefficients from FTD Integers $\{3, 4, 7, 13\}$", fontsize=13, fontweight="bold", y=1.02)
plt.tight_layout()
plt.savefig(f"{OUTDIR}/fig_coefficients.pdf", dpi=300, bbox_inches="tight")
plt.savefig(f"{OUTDIR}/fig_coefficients.png", dpi=300, bbox_inches="tight")
print("Figure 2 saved")
plt.close()

# ==== FIGURE 3: Digit comparison ====
fig, ax = plt.subplots(figsize=(10, 2.8))
ax.axis("off")

ax.text(
    0.5,
    0.97,
    "Digit-by-Digit Comparison with CODATA 2022",
    ha="center",
    va="top",
    fontsize=14,
    fontweight="bold",
    transform=ax.transAxes,
)

y_form = 0.62
y_cod = 0.32
x_start = 0.10
dx = 0.044

ax.text(
    0.02,
    y_form,
    "Formula:",
    ha="left",
    va="center",
    fontsize=10,
    fontweight="bold",
    color="#2c3e50",
    transform=ax.transAxes,
)
ax.text(
    0.02,
    y_cod,
    "CODATA:",
    ha="left",
    va="center",
    fontsize=10,
    fontweight="bold",
    color="#2c3e50",
    transform=ax.transAxes,
)

formula_d = "137.035999177000041"
codata_d = "137.035999177(\\u00b121)"

for i, ch in enumerate(formula_d):
    x = x_start + i * dx
    if ch == ".":
        color = "#2c3e50"
    elif i < 15:
        color = "#27ae60"
    else:
        color = "#8e44ad"
    ax.text(
        x,
        y_form,
        ch,
        ha="center",
        va="center",
        fontsize=15,
        fontweight="bold",
        color=color,
        fontfamily="monospace",
        transform=ax.transAxes,
    )

codata_display = "137.035999177(+/-21)"
for i, ch in enumerate(codata_display):
    x = x_start + i * dx
    if ch in ".()/-+":
        color = "#7f8c8d"
    elif i < 15:
        color = "#27ae60"
    else:
        color = "#7f8c8d"
    ax.text(
        x,
        y_cod,
        ch,
        ha="center",
        va="center",
        fontsize=15,
        fontweight="bold",
        color=color,
        fontfamily="monospace",
        transform=ax.transAxes,
    )

ax.text(0.25, 0.05, "Matched digits", color="#27ae60", fontsize=9, fontweight="bold", transform=ax.transAxes)
ax.text(0.55, 0.05, "Predicted digits", color="#8e44ad", fontsize=9, fontweight="bold", transform=ax.transAxes)

plt.tight_layout()
plt.savefig(f"{OUTDIR}/fig_digits.pdf", dpi=300, bbox_inches="tight")
plt.savefig(f"{OUTDIR}/fig_digits.png", dpi=300, bbox_inches="tight")
print("Figure 3 saved")
plt.close()

# ==== FIGURE 4: Term magnitude waterfall ====
fig, ax = plt.subplots(figsize=(7, 4))

labels = [r"$c_1|\varepsilon|$", r"$c_2|\varepsilon|^2$", r"$c_3|\varepsilon|^3$", r"$c_4|\varepsilon|^4$"]
bar_colors = ["#e74c3c", "#f39c12", "#2ecc71", "#3498db"]
signs_display = [r"$-$", r"$+$", r"$-$", r"$-$"]

log_terms = [np.log10(t) for t in terms]

for i, (lt, bc, lbl, sgn, t) in enumerate(zip(log_terms, bar_colors, labels, signs_display, terms)):
    y = 3 - i
    ax.barh(y, lt, color=bc, height=0.6, alpha=0.85)
    ax.text(-0.5, y, f"{sgn} {lbl}", ha="right", va="center", fontsize=11)
    ax.text(lt + 0.15, y, f"{t:.3e}", ha="left", va="center", fontsize=9, color="#333")

ax.set_xlabel(r"$\log_{10}$(term magnitude)", fontsize=11)
ax.set_title("Hierarchical Scale Separation of Correction Terms", fontsize=13, fontweight="bold")
ax.set_yticks([])
ax.set_xlim(-14, 0)
ax.axvline(np.log10(2.1e-8), color="gray", linestyle="--", alpha=0.7, label="CODATA uncertainty")
ax.legend(fontsize=9, loc="lower right")
ax.grid(True, alpha=0.2, axis="x")

plt.tight_layout()
plt.savefig(f"{OUTDIR}/fig_waterfall.pdf", dpi=300, bbox_inches="tight")
plt.savefig(f"{OUTDIR}/fig_waterfall.png", dpi=300, bbox_inches="tight")
print("Figure 4 saved")
plt.close()

print("\nAll 4 figures generated successfully.")
