#!/usr/bin/env python3
"""
TRD Evidence Visuals Generator
==============================
Generates an evidence-focused addendum of figures (20+ visuals).

Outputs (PNG + SVG):
  `dissemination/manuscript/figures/ch14/fig-evidence-*.png`

Usage:
  `python dissemination/manuscript/figures/generate_evidence_figures.py --list`
  `python dissemination/manuscript/figures/generate_evidence_figures.py --all --dpi print`
  `python dissemination/manuscript/figures/generate_evidence_figures.py --figure E01`
"""

from __future__ import annotations

import argparse
import math
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Iterable, Literal

import matplotlib.pyplot as plt
import numpy as np
import mpmath as mp
from scipy import special

THIS_DIR = Path(__file__).resolve().parent
MANUSCRIPT_DIR = THIS_DIR.parent
sys.path.insert(0, str(THIS_DIR))

from utils.style import COLORS, DPI as DPI_MAP, apply_trd_style, setup_matplotlib_defaults  # noqa: E402
from utils.physics_constants import FREQUENCIES, X_AMPLITUDES, Y_AMPLITUDES, SCALE_FACTOR  # noqa: E402


EXPERIMENT = {
    "alpha_inv": 137.035999177,  # CODATA 2022
    "sin2_theta_w": 0.2312,
    "alpha_s_mz": 0.1179,
}


def compute_g_star_mpmath(dps: int = 80) -> float:
    mp.mp.dps = dps
    g_star = mp.sqrt(2) * mp.gamma(mp.mpf(1) / 4) ** 2 / (2 * mp.pi)
    return float(g_star)


def compute_g_star_scipy() -> float:
    gamma_quarter = special.gamma(0.25)
    return float(np.sqrt(2) * gamma_quarter**2 / (2 * np.pi))


def solve_master_quadratic(g_star: float, coeff: float = 16.0) -> tuple[float, float]:
    b = -coeff * g_star**2
    c = coeff * g_star**3
    disc = b * b - 4.0 * c
    if disc < 0:
        return (float("nan"), float("nan"))
    sqrt_disc = math.sqrt(disc)
    x_plus = (-b + sqrt_disc) / 2.0
    x_minus = (-b - sqrt_disc) / 2.0
    return x_plus, x_minus


def ppm_error(pred: float, ref: float) -> float:
    return abs(pred - ref) / ref * 1e6


def pct_error(pred: float, ref: float) -> float:
    return abs(pred - ref) / ref * 100.0


def lemniscate_curve_xy(n_samples: int) -> tuple[np.ndarray, np.ndarray]:
    t = np.linspace(0, 2 * np.pi, int(n_samples), dtype=np.float64)
    x = np.zeros_like(t)
    y = np.zeros_like(t)
    for idx, freq in enumerate(FREQUENCIES):
        x += X_AMPLITUDES[idx] * np.cos(freq * t)
        y += Y_AMPLITUDES[idx] * np.sin(freq * t)
    return x, y


def arc_length_polyline(x: np.ndarray, y: np.ndarray) -> float:
    dx = np.diff(x)
    dy = np.diff(y)
    return float(np.sum(np.sqrt(dx * dx + dy * dy)))


def agm_sequence(a0: float, b0: float, n_iter: int) -> np.ndarray:
    a = float(a0)
    b = float(b0)
    a_vals = [a]
    for _ in range(int(n_iter)):
        a_next = 0.5 * (a + b)
        b_next = math.sqrt(a * b)
        a, b = a_next, b_next
        a_vals.append(a)
    return np.array(a_vals)


def elliptic_K_agm(k: float, n_iter: int) -> tuple[np.ndarray, np.ndarray]:
    k = float(k)
    a0 = 1.0
    b0 = math.sqrt(max(0.0, 1.0 - k * k))
    a_vals = agm_sequence(a0, b0, n_iter=n_iter)
    K_approx = math.pi / (2.0 * a_vals)
    return np.arange(len(K_approx)), K_approx


def save_png_svg(fig: plt.Figure, output_png: Path, dpi: Literal["web", "print", "preview"]) -> None:
    output_png.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_png, dpi=DPI_MAP[dpi], bbox_inches="tight", facecolor=COLORS["background"], edgecolor="none")
    fig.savefig(output_png.with_suffix(".svg"), bbox_inches="tight", facecolor=COLORS["background"], edgecolor="none")
    plt.close(fig)


@dataclass(frozen=True)
class MassRow:
    category: str
    name: str
    predicted: float
    measured: float


def _parse_number(token: str) -> float:
    token = token.strip().replace(",", "")
    m = re.search(r"[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?", token)
    if not m:
        raise ValueError(f"Could not parse number from: {token!r}")
    return float(m.group(0))


def iter_md_tables(lines: list[str]) -> Iterable[tuple[int, list[str], list[dict[str, str]]]]:
    i = 0
    while i < len(lines) - 1:
        if not (lines[i].startswith("|") and lines[i + 1].startswith("|") and "---" in lines[i + 1]):
            i += 1
            continue
        header = [c.strip() for c in lines[i].strip().strip("|").split("|")]
        start_idx = i
        i += 2
        rows: list[dict[str, str]] = []
        while i < len(lines) and lines[i].startswith("|"):
            cells = [c.strip() for c in lines[i].strip().strip("|").split("|")]
            if len(cells) == len(header):
                rows.append(dict(zip(header, cells)))
            i += 1
        yield start_idx, header, rows


def _nearest_heading(lines: list[str], idx: int) -> str:
    for j in range(idx, -1, -1):
        line = lines[j].strip()
        if line.startswith("### "):
            return line.removeprefix("### ").strip()
    return "Mass ratios"


def parse_mass_ratio_rows() -> list[MassRow]:
    qmd_path = MANUSCRIPT_DIR / "chapters" / "14.1-constants-reference.qmd"
    lines = qmd_path.read_text(encoding="utf-8").splitlines()
    rows: list[MassRow] = []
    for start_idx, header, table_rows in iter_md_tables(lines):
        if not {"Predicted", "Measured", "Error"}.issubset(set(header)):
            continue
        if header[0] not in {"Particle", "Quantity"}:
            continue
        category = _nearest_heading(lines, start_idx)
        for tr in table_rows:
            name = tr.get(header[0], "").strip()
            if not name:
                continue
            try:
                pred = _parse_number(tr["Predicted"])
                meas = _parse_number(tr["Measured"])
            except Exception:
                continue
            rows.append(MassRow(category=category, name=name, predicted=pred, measured=meas))
    return rows


# -----------------------------------------------------------------------------
# Figure functions (E01..)
# -----------------------------------------------------------------------------


def fig_e01_alpha_match() -> plt.Figure:
    g_star = compute_g_star_mpmath()
    x_plus, _ = solve_master_quadratic(g_star, coeff=16.0)
    ref = EXPERIMENT["alpha_inv"]
    err_ppm = ppm_error(x_plus, ref)

    fig, ax = plt.subplots(figsize=(10, 3.2))
    fig.patch.set_facecolor(COLORS["background"])

    ax.set_title("Fine Structure Constant from Master Quadratic (Evidence View)")
    ax.grid(True, alpha=0.25, color=COLORS["grid"])
    ax.scatter([0], [ref], s=120, color=COLORS["accent1"], label="CODATA 2022 1/α")
    ax.scatter([1], [x_plus], s=120, color=COLORS["matter"], label="TRD quadratic x+")
    ax.plot([0, 1], [ref, x_plus], color=COLORS["text"], alpha=0.35, linewidth=1)
    ax.set_xticks([0, 1], ["Experiment", "TRD"])
    ax.set_ylabel("1/α")
    ax.set_ylim(min(ref, x_plus) - 0.0003, max(ref, x_plus) + 0.0003)
    ax.legend(loc="upper right", framealpha=0.95)
    ax.text(
        0.5,
        0.08,
        f"G*={g_star:.10f}  |  x+={x_plus:.9f}  |  Δ={err_ppm:.2f} ppm",
        transform=ax.transAxes,
        ha="center",
        va="bottom",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="#fff7e6", edgecolor=COLORS["highlight"], alpha=0.95),
    )
    return fig


def fig_e02_master_quadratic_with_residuals() -> plt.Figure:
    g_star = compute_g_star_mpmath()
    x_plus, x_minus = solve_master_quadratic(g_star, coeff=16.0)
    ref = EXPERIMENT["alpha_inv"]

    def q(x: np.ndarray) -> np.ndarray:
        return x * x - 16 * (g_star**2) * x + 16 * (g_star**3)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 4.2))
    fig.patch.set_facecolor(COLORS["background"])

    x_full = np.linspace(-10, 155, 1500)
    ax1.plot(x_full, q(x_full), color=COLORS["text"], linewidth=2)
    ax1.axhline(0, color=COLORS["grid"], linestyle="--", linewidth=1)
    ax1.scatter([x_minus, x_plus], [0, 0], s=110, color=[COLORS["antimatter"], COLORS["matter"]], zorder=5)
    apply_trd_style(ax1, title="Master Quadratic (Full View)", xlabel="x", ylabel="q(x)")
    ax1.set_ylim(-6000, 8000)

    x_zoom = np.linspace(ref - 0.015, ref + 0.015, 1200)
    ax2.plot(x_zoom, q(x_zoom), color=COLORS["text"], linewidth=2)
    ax2.axhline(0, color=COLORS["grid"], linestyle="--", linewidth=1)
    ax2.axvline(ref, color=COLORS["accent1"], linestyle=":", linewidth=2, alpha=0.9, label="CODATA 2022 1/α")
    ax2.scatter([x_plus], [0], s=130, color=COLORS["matter"], zorder=5, label="TRD x+")
    apply_trd_style(ax2, title="Zoom Near 1/α", xlabel="x", ylabel="q(x)")
    ax2.legend(loc="upper left", framealpha=0.95)

    ax2.text(
        0.02,
        0.98,
        f"x+ = {x_plus:.9f}\nCODATA = {ref:.9f}\nΔ = {ppm_error(x_plus, ref):.2f} ppm",
        transform=ax2.transAxes,
        ha="left",
        va="top",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )

    fig.suptitle("q(x) = x² − 16(G*)² x + 16(G*)³   (G* from Γ(1/4))", fontsize=12, y=1.02)
    fig.tight_layout()
    return fig


def fig_e03_coeff_sweep() -> plt.Figure:
    g_star = compute_g_star_mpmath()
    coeffs = np.linspace(8, 30, 800)
    x_plus = np.array([solve_master_quadratic(g_star, float(c))[0] for c in coeffs])
    ref = EXPERIMENT["alpha_inv"]

    fig, ax = plt.subplots(figsize=(10, 5))
    fig.patch.set_facecolor(COLORS["background"])

    ax.plot(coeffs, x_plus, color=COLORS["text"], linewidth=2)
    ax.axhline(ref, color=COLORS["accent1"], linestyle="--", linewidth=2, alpha=0.9, label="CODATA 2022 1/α")
    ax.axvline(16, color=COLORS["highlight"], linestyle=":", linewidth=2, alpha=0.9, label="Coefficient = 16")

    c_star = float(coeffs[np.argmin(np.abs(x_plus - ref))])
    ax.scatter([c_star], [float(np.interp(c_star, coeffs, x_plus))], s=80, color=COLORS["accent2"], zorder=5)

    apply_trd_style(ax, title="Sensitivity: x+ vs Quadratic Coefficient", xlabel="Coefficient", ylabel="x+")
    ax.legend(loc="best", framealpha=0.95)
    ax.text(
        0.02,
        0.02,
        f"Closest coefficient in sweep: {c_star:.3f}",
        transform=ax.transAxes,
        ha="left",
        va="bottom",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )
    return fig


def fig_e04_gstar_perturbation_sensitivity() -> plt.Figure:
    g0 = compute_g_star_mpmath()
    ref = EXPERIMENT["alpha_inv"]

    deltas = np.linspace(-200, 200, 1001)  # ppm perturbations of G*
    g_vals = g0 * (1.0 + deltas * 1e-6)
    x_plus = np.array([solve_master_quadratic(float(g), 16.0)[0] for g in g_vals])

    best_idx = int(np.argmin(np.abs(x_plus - ref)))
    best_delta_ppm = float(deltas[best_idx])

    fig, ax = plt.subplots(figsize=(10, 5))
    fig.patch.set_facecolor(COLORS["background"])

    ax.plot(deltas, x_plus, color=COLORS["text"], linewidth=2)
    ax.axhline(ref, color=COLORS["accent1"], linestyle="--", linewidth=2, alpha=0.9, label="CODATA 2022 1/α")
    ax.axvline(0, color=COLORS["grid"], linestyle=":", linewidth=1.5, alpha=0.8)
    ax.scatter([best_delta_ppm], [x_plus[best_idx]], s=90, color=COLORS["accent2"], zorder=5)

    apply_trd_style(ax, title="Sensitivity: x+ vs G* Perturbation", xlabel="ΔG* (ppm)", ylabel="x+")
    ax.text(
        0.02,
        0.02,
        f"Best ΔG* (within sweep): {best_delta_ppm:.1f} ppm",
        transform=ax.transAxes,
        ha="left",
        va="bottom",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )
    ax.legend(loc="best", framealpha=0.95)
    return fig


def fig_e05_arc_length_convergence() -> plt.Figure:
    g_exact = compute_g_star_mpmath()
    ref = EXPERIMENT["alpha_inv"]

    ns = np.unique(np.round(np.logspace(2, 5.2, 26)).astype(int))
    g_est = []
    x_est = []
    for n in ns:
        x, y = lemniscate_curve_xy(int(n))
        L = arc_length_polyline(x, y)
        g = L * SCALE_FACTOR
        g_est.append(g)
        x_est.append(solve_master_quadratic(g, 16.0)[0])

    g_est = np.array(g_est)
    x_est = np.array(x_est)

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 7.5), sharex=True)
    fig.patch.set_facecolor(COLORS["background"])

    ax1.plot(ns, g_est, color=COLORS["text"], linewidth=2)
    ax1.axhline(g_exact, color=COLORS["accent1"], linestyle="--", linewidth=2, alpha=0.9, label="Exact G* (Γ(1/4))")
    apply_trd_style(ax1, title="Numerical Stability: Arc-Length → G*", ylabel="G* estimate")
    ax1.set_xscale("log")
    ax1.legend(loc="best", framealpha=0.95)

    ax2.plot(ns, x_est, color=COLORS["text"], linewidth=2)
    ax2.axhline(ref, color=COLORS["accent1"], linestyle="--", linewidth=2, alpha=0.9, label="CODATA 2022 1/α")
    apply_trd_style(ax2, title="Propagated Effect: G* Estimate → Quadratic Root x+", xlabel="Curve samples (log scale)", ylabel="x+")
    ax2.set_xscale("log")
    ax2.legend(loc="best", framealpha=0.95)

    fig.tight_layout()
    return fig


def fig_e06_gstar_method_precision() -> plt.Figure:
    g_mp = compute_g_star_mpmath(dps=120)
    g_sp = compute_g_star_scipy()

    mp.mp.dps = 120
    g_ref = float(mp.sqrt(2) * mp.gamma(mp.mpf(1) / 4) ** 2 / (2 * mp.pi))

    methods = ["SciPy Γ(1/4)", "mpmath Γ(1/4)"]
    values = [g_sp, g_mp]
    abs_err = [abs(g_sp - g_ref), abs(g_mp - g_ref)]

    fig, ax = plt.subplots(figsize=(10, 4.6))
    fig.patch.set_facecolor(COLORS["background"])

    y = np.arange(len(methods))
    ax.barh(
        y,
        [-math.log10(e) if e > 0 else 50 for e in abs_err],
        color=[COLORS["antimatter"], COLORS["matter"]],
        alpha=0.9,
    )
    ax.set_yticks(y, methods)
    ax.set_xlabel(r"$-\log_{10}(|G^*_{method} - G^*_{ref}|)$  (bigger is better)")
    ax.set_title("Numerical Agreement of G* Computation Methods")
    ax.grid(True, axis="x", alpha=0.25, color=COLORS["grid"])

    for yi, v, e in zip(y, values, abs_err):
        ax.text(0.02, yi, f"  G*={v:.12f}  |  abs err={e:.2e}", va="center", ha="left", fontsize=10, color=COLORS["text"])

    fig.tight_layout()
    return fig


def fig_e07_agm_convergence() -> plt.Figure:
    k = 1.0 / math.sqrt(2.0)
    iters, K_vals = elliptic_K_agm(k, n_iter=12)

    mp.mp.dps = 80
    K_ref = float(mp.ellipk(k * k))
    errs = np.abs(K_vals - K_ref)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12.5, 4.8))
    fig.patch.set_facecolor(COLORS["background"])

    ax1.plot(iters, K_vals, marker="o", color=COLORS["text"], linewidth=2)
    ax1.axhline(K_ref, color=COLORS["accent1"], linestyle="--", linewidth=2, alpha=0.9, label="mpmath ellipk(m)")
    apply_trd_style(ax1, title="AGM Convergence for K(k), k=1/√2", xlabel="Iteration", ylabel="K(k)")
    ax1.legend(loc="best", framealpha=0.95)

    ax2.semilogy(iters, np.maximum(errs, 1e-30), marker="o", color=COLORS["highlight"], linewidth=2)
    apply_trd_style(ax2, title="Absolute Error vs Iteration", xlabel="Iteration", ylabel="|K_AGM - K_ref|")

    fig.tight_layout()
    return fig


def fig_e08_dof_breakdown_2x2x2() -> plt.Figure:
    total = 24
    gauss = 7
    gauge = 1
    physical = total - gauss - gauge

    labels = ["Total flux DoF", "Gauss constraints", "Gauge mode", "Physical DoF"]
    values = [total, gauss, gauge, physical]
    colors = [COLORS["text"], COLORS["antimatter"], COLORS["accent2"], COLORS["accent1"]]

    fig, ax = plt.subplots(figsize=(10, 4.6))
    fig.patch.set_facecolor(COLORS["background"])

    ax.bar(labels, values, color=colors, alpha=0.9)
    apply_trd_style(ax, title="2×2×2 Minimal Lattice: Degrees-of-Freedom Accounting", ylabel="Count")
    ax.set_ylim(0, max(values) * 1.15)
    for i, v in enumerate(values):
        ax.text(i, v + 0.5, str(v), ha="center", va="bottom", fontsize=11, fontweight="bold")

    ax.text(
        0.5,
        0.02,
        "24 − 7 − 1 = 16   (coefficient used in the master quadratic)",
        transform=ax.transAxes,
        ha="center",
        va="bottom",
        fontsize=11,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="#f4faff", edgecolor=COLORS["grid"], alpha=0.95),
    )

    fig.tight_layout()
    return fig


def fig_e09_cm_points_tau_plane() -> plt.Figure:
    tau_i = 0 + 1j
    tau_w = 0.5 + (math.sqrt(3) / 2) * 1j  # exp(i*pi/3)

    fig, ax = plt.subplots(figsize=(7.4, 6.2))
    fig.patch.set_facecolor(COLORS["background"])

    ax.set_xlim(-0.2, 1.2)
    ax.set_ylim(0.0, 1.4)
    ax.set_aspect("equal")

    ax.scatter([tau_i.real], [tau_i.imag], s=140, color=COLORS["matter"], zorder=5)
    ax.scatter([tau_w.real], [tau_w.imag], s=140, color=COLORS["antimatter"], zorder=5)

    ax.annotate(
        "τ = i\n(j = 1728, Aut order 4)",
        (tau_i.real, tau_i.imag),
        xytext=(0.15, 1.15),
        textcoords="data",
        arrowprops=dict(arrowstyle="->", color=COLORS["matter"], lw=1.5),
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["matter"], alpha=0.95),
    )
    ax.annotate(
        "τ = e^{iπ/3}\n(j = 0, Aut order 6)",
        (tau_w.real, tau_w.imag),
        xytext=(0.72, 1.15),
        textcoords="data",
        arrowprops=dict(arrowstyle="->", color=COLORS["antimatter"], lw=1.5),
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["antimatter"], alpha=0.95),
    )

    apply_trd_style(ax, title="CM Points in the Upper Half-Plane (Selection Space Sketch)", xlabel="Re(τ)", ylabel="Im(τ)")
    ax.grid(True, alpha=0.25, color=COLORS["grid"])
    ax.text(
        0.5,
        0.03,
        "Heuristic: cubic lattice symmetries include order-4 rotations (τ=i) but not order-6.",
        transform=ax.transAxes,
        ha="center",
        va="bottom",
        fontsize=9.5,
        bbox=dict(boxstyle="round,pad=0.3", facecolor="#fff7e6", edgecolor=COLORS["grid"], alpha=0.95),
    )
    fig.tight_layout()
    return fig


def _sigma_powers(n: int, p: int) -> int:
    s = 0
    for d in range(1, n + 1):
        if n % d == 0:
            s += d**p
    return s


def j_invariant_qseries(tau: complex, n_terms: int) -> complex:
    q = mp.e ** (2 * mp.pi * 1j * tau)
    E4 = mp.mpf(1)
    E6 = mp.mpf(1)
    for n in range(1, int(n_terms) + 1):
        E4 += 240 * _sigma_powers(n, 3) * (q**n)
        E6 += -504 * _sigma_powers(n, 5) * (q**n)
    Delta = (E4**3 - E6**2) / 1728
    return complex(E4**3 / Delta)


def fig_e10_j_invariant_convergence() -> plt.Figure:
    mp.mp.dps = 80
    tau_i = 0 + 1j
    tau_w = 0.5 + (math.sqrt(3) / 2) * 1j

    terms = np.array([1, 2, 3, 5, 8, 13, 21, 34, 55], dtype=int)
    j_i = np.array([j_invariant_qseries(tau_i, int(t)).real for t in terms], dtype=float)
    j_w = np.array([abs(j_invariant_qseries(tau_w, int(t))) for t in terms], dtype=float)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12.8, 4.8))
    fig.patch.set_facecolor(COLORS["background"])

    ax1.plot(terms, j_i, marker="o", color=COLORS["matter"], linewidth=2)
    ax1.axhline(1728, color=COLORS["accent1"], linestyle="--", linewidth=2, alpha=0.9, label="1728")
    apply_trd_style(ax1, title="j(τ=i) via q-series (real part)", xlabel="q-series terms", ylabel="j")
    ax1.legend(loc="best", framealpha=0.95)

    ax2.semilogy(terms, np.maximum(j_w, 1e-30), marker="o", color=COLORS["antimatter"], linewidth=2)
    apply_trd_style(ax2, title="|j(τ=e^{iπ/3})| via q-series", xlabel="q-series terms", ylabel="|j|")
    ax2.text(
        0.02,
        0.98,
        "Expected: j=0 (equianharmonic)\nSmall |j| supports convergence",
        transform=ax2.transAxes,
        ha="left",
        va="top",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )

    fig.tight_layout()
    return fig


def fig_e11_integer_heatmap_sin2thetaW() -> plt.Figure:
    target = EXPERIMENT["sin2_theta_w"]
    nc_vals = np.arange(2, 7)
    neff_vals = np.arange(7, 21)
    err = np.zeros((len(nc_vals), len(neff_vals)))
    for i, nc in enumerate(nc_vals):
        for j, neff in enumerate(neff_vals):
            err[i, j] = pct_error(nc / neff, target)

    fig, ax = plt.subplots(figsize=(11, 4.8))
    fig.patch.set_facecolor(COLORS["background"])
    im = ax.imshow(err, aspect="auto", cmap="viridis", origin="lower")
    ax.set_xticks(np.arange(len(neff_vals)), [str(v) for v in neff_vals])
    ax.set_yticks(np.arange(len(nc_vals)), [str(v) for v in nc_vals])
    ax.set_xlabel("N_eff")
    ax.set_ylabel("N_c")
    ax.set_title(r"Integer Robustness Scan: $\sin^2\theta_W = N_c / N_{eff}$  (% error)")

    i0 = int(np.where(nc_vals == 3)[0][0])
    j0 = int(np.where(neff_vals == 13)[0][0])
    ax.scatter([j0], [i0], s=140, facecolors="none", edgecolors="white", linewidths=2)
    ax.text(j0 + 0.3, i0 + 0.2, "3/13", color="white", fontsize=11, weight="bold")

    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label("% error vs 0.2312")
    fig.tight_layout()
    return fig


def fig_e12_integer_heatmap_alpha_s() -> plt.Figure:
    target = EXPERIMENT["alpha_s_mz"]
    b3_vals = np.arange(4, 13)
    nc_vals = np.arange(2, 7)
    err = np.zeros((len(b3_vals), len(nc_vals)))
    for i, b3 in enumerate(b3_vals):
        for j, nc in enumerate(nc_vals):
            pred = b3 / (b3 * b3 + b3 + nc)
            err[i, j] = pct_error(pred, target)

    fig, ax = plt.subplots(figsize=(9.6, 5.2))
    fig.patch.set_facecolor(COLORS["background"])
    im = ax.imshow(err, aspect="auto", cmap="viridis", origin="lower")
    ax.set_xticks(np.arange(len(nc_vals)), [str(v) for v in nc_vals])
    ax.set_yticks(np.arange(len(b3_vals)), [str(v) for v in b3_vals])
    ax.set_xlabel("N_c")
    ax.set_ylabel("b3")
    ax.set_title(r"Integer Robustness Scan: $\alpha_s = b_3/(b_3^2+b_3+N_c)$  (% error)")

    i0 = int(np.where(b3_vals == 7)[0][0])
    j0 = int(np.where(nc_vals == 3)[0][0])
    ax.scatter([j0], [i0], s=140, facecolors="none", edgecolors="white", linewidths=2)
    ax.text(j0 + 0.2, i0 + 0.2, "7/59", color="white", fontsize=11, weight="bold")

    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label("% error vs 0.1179")
    fig.tight_layout()
    return fig


def _mass_rows_to_arrays(rows: list[MassRow]) -> tuple[list[str], np.ndarray, np.ndarray, np.ndarray]:
    names: list[str] = []
    pred = []
    meas = []
    err = []
    for r in rows:
        names.append(r.name)
        pred.append(r.predicted)
        meas.append(r.measured)
        err.append(pct_error(r.predicted, r.measured))
    return names, np.array(pred), np.array(meas), np.array(err)


def fig_e13_mass_pred_vs_measured() -> plt.Figure:
    rows = parse_mass_ratio_rows()
    names, pred, meas, err = _mass_rows_to_arrays(rows)

    fig, ax = plt.subplots(figsize=(7.2, 7.2))
    fig.patch.set_facecolor(COLORS["background"])

    ax.loglog(meas, pred, "o", markersize=6, color=COLORS["text"], alpha=0.85)
    lo = min(meas.min(), pred.min()) * 0.8
    hi = max(meas.max(), pred.max()) * 1.2
    ax.loglog([lo, hi], [lo, hi], linestyle="--", color=COLORS["accent1"], linewidth=2, alpha=0.9, label="y=x")

    apply_trd_style(ax, title="Mass Ratios: Predicted vs Measured (log-log)", xlabel="Measured (ratio to electron)", ylabel="Predicted (ratio)")
    ax.legend(loc="upper left", framealpha=0.95)

    worst = int(np.argmax(err))
    ax.annotate(
        f"Worst: {names[worst]}\n{err[worst]:.2f}%",
        xy=(meas[worst], pred[worst]),
        xytext=(0.05, 0.95),
        textcoords="axes fraction",
        ha="left",
        va="top",
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
        arrowprops=dict(arrowstyle="->", color=COLORS["highlight"], lw=1.5),
    )

    fig.tight_layout()
    return fig


def fig_e14_mass_errors_sorted() -> plt.Figure:
    rows = parse_mass_ratio_rows()
    names, _, _, err = _mass_rows_to_arrays(rows)
    order = np.argsort(err)

    fig, ax = plt.subplots(figsize=(11.5, 6.0))
    fig.patch.set_facecolor(COLORS["background"])

    ax.bar(np.arange(len(order)), err[order], color=COLORS["highlight"], alpha=0.9)
    ax.set_xticks(np.arange(len(order)), [names[i] for i in order], rotation=45, ha="right", fontsize=9)
    apply_trd_style(ax, title="Mass Ratio Errors (sorted)", ylabel="% error")
    ax.set_ylim(0, float(np.max(err)) * 1.2)
    ax.text(
        0.02,
        0.98,
        f"Count: {len(err)}  |  mean: {float(np.mean(err)):.3f}%  |  median: {float(np.median(err)):.3f}%",
        transform=ax.transAxes,
        ha="left",
        va="top",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )
    fig.tight_layout()
    return fig


def fig_e15_mass_error_histogram() -> plt.Figure:
    rows = parse_mass_ratio_rows()
    _, _, _, err = _mass_rows_to_arrays(rows)

    fig, ax = plt.subplots(figsize=(10, 4.8))
    fig.patch.set_facecolor(COLORS["background"])
    ax.hist(err, bins=12, color=COLORS["text"], alpha=0.85, edgecolor="white")
    apply_trd_style(ax, title="Distribution of Mass Ratio Errors", xlabel="% error", ylabel="count")
    ax.grid(True, alpha=0.25, color=COLORS["grid"])
    fig.tight_layout()
    return fig


def fig_e16_headline_scoreboard() -> plt.Figure:
    g_star = compute_g_star_mpmath()
    x_plus, _ = solve_master_quadratic(g_star, 16.0)
    alpha = 1.0 / x_plus

    items: list[tuple[str, float]] = []
    items.append(("1/α (ppm)", ppm_error(x_plus, EXPERIMENT["alpha_inv"])))
    items.append((r"$\sin^2\theta_W$ (%)", pct_error(3 / 13, EXPERIMENT["sin2_theta_w"])))
    items.append((r"$\alpha_s$ (%)", pct_error(7 / 59, EXPERIMENT["alpha_s_mz"])))

    for r in parse_mass_ratio_rows():
        items.append((f"{r.name} (%)", pct_error(r.predicted, r.measured)))

    exp_pred = -math.log10(alpha**57)
    items.append(("Λ exponent (%)", pct_error(exp_pred, 122.0)))

    items = items[:20]  # 3 couplings + 16 mass ratios + 1 cosmology
    labels = [t[0] for t in items]
    errors = np.array([t[1] for t in items], dtype=float)

    order = np.argsort(errors)
    fig, ax = plt.subplots(figsize=(12.5, 6.2))
    fig.patch.set_facecolor(COLORS["background"])
    ax.bar(np.arange(len(order)), errors[order], color=COLORS["accent1"], alpha=0.85)
    ax.set_xticks(np.arange(len(order)), [labels[i] for i in order], rotation=55, ha="right", fontsize=8.5)
    ax.set_yscale("log")
    apply_trd_style(ax, title="Headline Derivations: Error Scoreboard (log scale)", ylabel="error (ppm or %)")
    ax.text(
        0.02,
        0.98,
        "Note: 1/α shown in ppm; others shown in % (log axis).",
        transform=ax.transAxes,
        ha="left",
        va="top",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )
    fig.tight_layout()
    return fig


def fig_e17_couplings_comparison() -> plt.Figure:
    g_star = compute_g_star_mpmath()
    x_plus, _ = solve_master_quadratic(g_star, 16.0)

    labels = ["1/α", r"$\sin^2\theta_W$", r"$\alpha_s(M_Z)$"]
    pred = np.array([x_plus, 3 / 13, 7 / 59], dtype=float)
    meas = np.array([EXPERIMENT["alpha_inv"], EXPERIMENT["sin2_theta_w"], EXPERIMENT["alpha_s_mz"]], dtype=float)

    fig, ax = plt.subplots(figsize=(10.5, 4.8))
    fig.patch.set_facecolor(COLORS["background"])
    x = np.arange(len(labels))
    w = 0.35
    ax.bar(x - w / 2, meas, width=w, color=COLORS["accent1"], alpha=0.85, label="Measured")
    ax.bar(x + w / 2, pred, width=w, color=COLORS["matter"], alpha=0.85, label="TRD")
    ax.set_xticks(x, labels)
    apply_trd_style(ax, title="Couplings: TRD vs Measured", ylabel="value")
    ax.legend(loc="best", framealpha=0.95)
    for xi, p, m in zip(x, pred, meas):
        err = ppm_error(p, m) if xi == 0 else pct_error(p, m)
        ax.text(xi, max(p, m) * 1.02, f"{err:.2f} {'ppm' if xi == 0 else '%'}", ha="center", va="bottom", fontsize=10)
    fig.tight_layout()
    return fig


def fig_e18_cosmological_exponent_scan() -> plt.Figure:
    g_star = compute_g_star_mpmath()
    x_plus, _ = solve_master_quadratic(g_star, 16.0)
    alpha = 1.0 / x_plus

    n = np.arange(0, 81)
    log10_vals = np.log10(alpha**n)

    fig, ax = plt.subplots(figsize=(10.5, 4.8))
    fig.patch.set_facecolor(COLORS["background"])
    ax.plot(n, log10_vals, color=COLORS["text"], linewidth=2)
    ax.axvline(57, color=COLORS["highlight"], linestyle=":", linewidth=2, alpha=0.9, label="n=57")
    ax.axhline(-122.0, color=COLORS["accent1"], linestyle="--", linewidth=2, alpha=0.9, label="Measured ~ -122")
    apply_trd_style(ax, title=r"Exponent Scan: $\log_{10}(\alpha^n)$", xlabel="n", ylabel=r"$\log_{10}(\alpha^n)$")
    ax.legend(loc="best", framealpha=0.95)
    ax.text(
        0.02,
        0.02,
        f"log10(alpha^57) = {float(math.log10(alpha**57)):.1f}",
        transform=ax.transAxes,
        ha="left",
        va="bottom",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )
    fig.tight_layout()
    return fig


def run_look_elsewhere_monte_carlo(
    n_samples: int,
    g_variation_pct: float,
    coeff_variation: float,
    seed: int,
) -> tuple[np.ndarray, np.ndarray]:
    rng = np.random.default_rng(seed)
    g0 = compute_g_star_scipy()
    g = g0 * (1.0 + rng.uniform(-g_variation_pct / 100.0, g_variation_pct / 100.0, size=n_samples))
    coeff = rng.uniform(16.0 - coeff_variation, 16.0 + coeff_variation, size=n_samples)

    b = -coeff * g * g
    c = coeff * g * g * g
    disc = np.maximum(b * b - 4.0 * c, 0.0)
    x_plus = (-b + np.sqrt(disc)) / 2.0
    ppm = np.abs(x_plus - EXPERIMENT["alpha_inv"]) / EXPERIMENT["alpha_inv"] * 1e6
    return x_plus, ppm


def fig_e19_look_elsewhere_hist(samples: int = 50_000) -> plt.Figure:
    _, ppm = run_look_elsewhere_monte_carlo(n_samples=int(samples), g_variation_pct=1.0, coeff_variation=2.0, seed=12345)
    threshold = 1.26
    frac = float(np.mean(ppm <= threshold))

    fig, ax = plt.subplots(figsize=(11.0, 4.8))
    fig.patch.set_facecolor(COLORS["background"])
    ax.hist(ppm, bins=80, color=COLORS["text"], alpha=0.85)
    ax.axvline(threshold, color=COLORS["highlight"], linestyle="--", linewidth=2, alpha=0.9, label="1.26 ppm threshold")
    apply_trd_style(ax, title="Look-Elsewhere Monte Carlo: PPM Discrepancy Distribution", xlabel="ppm discrepancy to CODATA 1/α", ylabel="count")
    ax.legend(loc="best", framealpha=0.95)
    ax.text(
        0.98,
        0.98,
        f"samples={int(samples):,}\nmatch fraction={frac:.2e}",
        transform=ax.transAxes,
        ha="right",
        va="top",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )
    fig.tight_layout()
    return fig


def fig_e20_look_elsewhere_heatmap(samples_per_cell: int = 8000) -> plt.Figure:
    g_vars = [0.1, 0.3, 0.6, 1.0, 2.0]
    c_vars = [0.5, 1.0, 1.5, 2.0, 3.0]
    threshold = 1.26

    grid = np.zeros((len(g_vars), len(c_vars)))
    base_seed = 24680
    for i, gv in enumerate(g_vars):
        for j, cv in enumerate(c_vars):
            _, ppm = run_look_elsewhere_monte_carlo(
                n_samples=int(samples_per_cell),
                g_variation_pct=float(gv),
                coeff_variation=float(cv),
                seed=base_seed + i * 100 + j,
            )
            grid[i, j] = float(np.mean(ppm <= threshold))

    fig, ax = plt.subplots(figsize=(10.5, 4.8))
    fig.patch.set_facecolor(COLORS["background"])
    im = ax.imshow(grid, origin="lower", aspect="auto", cmap="magma")
    ax.set_xticks(np.arange(len(c_vars)), [str(v) for v in c_vars])
    ax.set_yticks(np.arange(len(g_vars)), [str(v) for v in g_vars])
    ax.set_xlabel("Coefficient variation (±)")
    ax.set_ylabel("G* variation (%)")
    ax.set_title("Look-Elsewhere Heatmap: Match Fraction within 1.26 ppm")
    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label("match fraction")
    fig.tight_layout()
    return fig


def fig_e21_look_elsewhere_best_of_n() -> plt.Figure:
    base_samples = 20000
    _, ppm = run_look_elsewhere_monte_carlo(n_samples=base_samples, g_variation_pct=1.0, coeff_variation=2.0, seed=999)
    threshold = 1.26
    p1 = float(np.mean(ppm <= threshold))

    Ns = np.array([1, 2, 5, 10, 20, 50, 100, 200, 500, 1000], dtype=int)
    pN = 1.0 - (1.0 - p1) ** Ns

    fig, ax = plt.subplots(figsize=(10.0, 4.6))
    fig.patch.set_facecolor(COLORS["background"])
    ax.semilogx(Ns, pN, marker="o", color=COLORS["text"], linewidth=2)
    apply_trd_style(ax, title="Look-Elsewhere: Best-of-N Trials Amplification", xlabel="independent tries (log)", ylabel="P(at least one match)")
    ax.grid(True, which="both", alpha=0.25, color=COLORS["grid"])
    ax.text(
        0.02,
        0.02,
        f"Single-try p ≈ {p1:.2e}  (estimated from {base_samples:,} samples)",
        transform=ax.transAxes,
        ha="left",
        va="bottom",
        fontsize=10,
        bbox=dict(boxstyle="round,pad=0.35", facecolor="white", edgecolor=COLORS["grid"], alpha=0.95),
    )
    fig.tight_layout()
    return fig


def fig_e22_root_joint_distribution(samples: int = 60_000) -> plt.Figure:
    rng = np.random.default_rng(777)
    g0 = compute_g_star_scipy()
    g = g0 * (1.0 + rng.uniform(-0.01, 0.01, size=samples))  # ±1%
    coeff = rng.uniform(14.0, 18.0, size=samples)

    b = -coeff * g * g
    c = coeff * g * g * g
    disc = np.maximum(b * b - 4.0 * c, 0.0)
    sqrt_disc = np.sqrt(disc)
    x_plus = (-b + sqrt_disc) / 2.0
    x_minus = (-b - sqrt_disc) / 2.0

    fig, ax = plt.subplots(figsize=(7.4, 6.4))
    fig.patch.set_facecolor(COLORS["background"])
    ax.scatter(x_minus, x_plus, s=3, alpha=0.2, color=COLORS["text"])
    ax.scatter([3.0], [EXPERIMENT["alpha_inv"]], s=120, color=COLORS["accent1"], label="(N_c=3, CODATA 1/α)")
    apply_trd_style(ax, title="Joint Distribution of Quadratic Roots under Variations", xlabel="x−", ylabel="x+")
    ax.legend(loc="lower right", framealpha=0.95)
    fig.tight_layout()
    return fig


# -----------------------------------------------------------------------------
# Registry + CLI
# -----------------------------------------------------------------------------


FigureFunc = Callable[..., plt.Figure]

EVIDENCE_FIGURES: dict[str, dict[str, object]] = {
    "E01": {"description": "Alpha match (ppm)", "output": "ch14/fig-evidence-01-alpha-match.png", "fn": fig_e01_alpha_match},
    "E02": {"description": "Master quadratic + residual zoom", "output": "ch14/fig-evidence-02-master-quadratic-residuals.png", "fn": fig_e02_master_quadratic_with_residuals},
    "E03": {"description": "Sensitivity: coefficient sweep", "output": "ch14/fig-evidence-03-coefficient-sweep.png", "fn": fig_e03_coeff_sweep},
    "E04": {"description": "Sensitivity: G* perturbation", "output": "ch14/fig-evidence-04-gstar-perturbation.png", "fn": fig_e04_gstar_perturbation_sensitivity},
    "E05": {"description": "Arc length convergence -> G* -> x+", "output": "ch14/fig-evidence-05-arc-length-convergence.png", "fn": fig_e05_arc_length_convergence},
    "E06": {"description": "G* computation method precision", "output": "ch14/fig-evidence-06-gstar-method-precision.png", "fn": fig_e06_gstar_method_precision},
    "E07": {"description": "AGM convergence for K(k)", "output": "ch14/fig-evidence-07-agm-convergence.png", "fn": fig_e07_agm_convergence},
    "E08": {"description": "2x2x2 DoF accounting -> 16", "output": "ch14/fig-evidence-08-dof-breakdown.png", "fn": fig_e08_dof_breakdown_2x2x2},
    "E09": {"description": "CM points in tau-plane", "output": "ch14/fig-evidence-09-cm-tau-plane.png", "fn": fig_e09_cm_points_tau_plane},
    "E10": {"description": "j-invariant q-series convergence", "output": "ch14/fig-evidence-10-j-invariant-convergence.png", "fn": fig_e10_j_invariant_convergence},
    "E11": {"description": "Integer scan: sin2thetaW", "output": "ch14/fig-evidence-11-integer-scan-sin2thetaW.png", "fn": fig_e11_integer_heatmap_sin2thetaW},
    "E12": {"description": "Integer scan: alpha_s", "output": "ch14/fig-evidence-12-integer-scan-alpha-s.png", "fn": fig_e12_integer_heatmap_alpha_s},
    "E13": {"description": "Mass ratios: pred vs meas", "output": "ch14/fig-evidence-13-mass-pred-vs-measured.png", "fn": fig_e13_mass_pred_vs_measured},
    "E14": {"description": "Mass errors sorted", "output": "ch14/fig-evidence-14-mass-errors-sorted.png", "fn": fig_e14_mass_errors_sorted},
    "E15": {"description": "Mass error histogram", "output": "ch14/fig-evidence-15-mass-error-hist.png", "fn": fig_e15_mass_error_histogram},
    "E16": {"description": "Headline derivations scoreboard", "output": "ch14/fig-evidence-16-headline-scoreboard.png", "fn": fig_e16_headline_scoreboard},
    "E17": {"description": "Couplings comparison", "output": "ch14/fig-evidence-17-couplings-comparison.png", "fn": fig_e17_couplings_comparison},
    "E18": {"description": "Cosmological exponent scan", "output": "ch14/fig-evidence-18-cosmological-exponent-scan.png", "fn": fig_e18_cosmological_exponent_scan},
    "E19": {"description": "Look-elsewhere: ppm histogram", "output": "ch14/fig-evidence-19-look-elsewhere-hist.png", "fn": fig_e19_look_elsewhere_hist},
    "E20": {"description": "Look-elsewhere: heatmap", "output": "ch14/fig-evidence-20-look-elsewhere-heatmap.png", "fn": fig_e20_look_elsewhere_heatmap},
    "E21": {"description": "Look-elsewhere: best-of-N amplification", "output": "ch14/fig-evidence-21-look-elsewhere-best-of-n.png", "fn": fig_e21_look_elsewhere_best_of_n},
    "E22": {"description": "Root joint distribution under variations", "output": "ch14/fig-evidence-22-root-joint-distribution.png", "fn": fig_e22_root_joint_distribution},
}


def list_figures() -> None:
    print("Evidence figures:")
    for fig_id, cfg in EVIDENCE_FIGURES.items():
        print(f"  {fig_id}: {cfg['description']} -> {cfg['output']}")


def generate_one(fig_id: str, dpi: Literal["web", "print", "preview"], output_dir: Path, samples: int) -> bool:
    cfg = EVIDENCE_FIGURES.get(fig_id)
    if not cfg:
        print(f"[FAIL] Unknown figure: {fig_id}")
        return False

    fn = cfg["fn"]
    assert callable(fn)

    if fig_id == "E19":
        fig = fn(samples=samples)  # type: ignore[arg-type]
    elif fig_id == "E20":
        fig = fn(samples_per_cell=max(2000, samples // 25))  # type: ignore[arg-type]
    elif fig_id == "E22":
        fig = fn(samples=samples)  # type: ignore[arg-type]
    else:
        fig = fn()  # type: ignore[misc]

    out_png = output_dir / str(cfg["output"])
    save_png_svg(fig, out_png, dpi=dpi)
    print(f"[OK] {fig_id} -> {out_png}")
    return True


def main() -> int:
    setup_matplotlib_defaults()
    parser = argparse.ArgumentParser(description="Generate evidence-focused TRD figures")
    parser.add_argument("--list", action="store_true", help="List available evidence figures")
    parser.add_argument("--all", action="store_true", help="Generate all evidence figures")
    parser.add_argument("--figure", "-f", type=str, help="Generate specific figure (e.g., E01)")
    parser.add_argument("--dpi", choices=["web", "print", "preview"], default="web", help="Resolution setting")
    parser.add_argument("--output", "-o", type=str, default=None, help="Output directory (default: this figures dir)")
    parser.add_argument("--samples", type=int, default=50_000, help="Monte Carlo sample count for E19/E22 (default 50k)")
    args = parser.parse_args()

    if args.list:
        list_figures()
        return 0

    output_dir = Path(args.output) if args.output else THIS_DIR

    if args.figure:
        return 0 if generate_one(args.figure.strip().upper(), args.dpi, output_dir, args.samples) else 1

    # Default: generate all.
    ok = 0
    fail = 0
    for fig_id in EVIDENCE_FIGURES.keys():
        if generate_one(fig_id, args.dpi, output_dir, args.samples):
            ok += 1
        else:
            fail += 1
    print(f"Done. ok={ok} fail={fail}")
    return 0 if fail == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
