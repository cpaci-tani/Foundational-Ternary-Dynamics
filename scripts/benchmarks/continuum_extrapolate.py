#!/usr/bin/env python3
"""
continuum_extrapolate.py ? Fit alpha(r_max, L) -> alpha(L->inf) for multiple scaling laws.

Uses the r_max-per-L data points from Phases 2-4 + Day-2 Thread 1a:
  L=64, r=20,  alpha_r = 0.030
  L=128, r=40, alpha_r = 0.028
  L=256, r=84, alpha_r = 0.010

Fits alpha(L) = alpha_inf + coeff ? f(L)  for three candidate f's:
  - 1/L   (linear finite-size, appropriate for Coulomb in periodic box)
  - 1/L^2  (standard lattice-dispersion finite-size)
  - 1/L^p (free exponent, 3-point best fit)

Outputs all three extrapolations with residuals. Let the data choose.
"""

from __future__ import annotations
import math
from typing import List, Tuple

# r_max alpha_r data, pairing each L with the alpha measured at its largest r-probe
DATA: List[Tuple[int, int, float]] = [
    # (L, r_max, alpha_r)
    (64, 20, 0.030),
    (128, 40, 0.028),
    (256, 84, 0.010),
]
ALPHA_REF = 1.0 / 137.035999177

def fit_linear(xs, ys):
    """Fit y = a + b?x by OLS. Return (a, b, r2)."""
    n = len(xs)
    sx = sum(xs); sy = sum(ys)
    sxx = sum(x*x for x in xs); sxy = sum(x*y for x, y in zip(xs, ys))
    denom = n*sxx - sx*sx
    if abs(denom) < 1e-30:
        return None, None, None
    b = (n*sxy - sx*sy) / denom
    a = (sy - b*sx) / n
    ybar = sy/n
    ss_tot = sum((y-ybar)**2 for y in ys)
    ss_res = sum((y - (a + b*x))**2 for x, y in zip(xs, ys))
    r2 = 1.0 - ss_res/ss_tot if ss_tot > 0 else 1.0
    return a, b, r2

def fit_power_law(Ls, ys):
    """Fit y = alpha_inf + c ? L^(-p) via log-linear on (y - min(y)) vs 1/L.
    Returns (alpha_inf, c, p, r2) for a 2-parameter free-p fit.
    Simple grid-search over p ? [0.5, 4.0]."""
    best_p = None
    best_a = None
    best_b = None
    best_r2 = -math.inf
    for p_int in range(5, 41):
        p = p_int / 10.0
        xs = [1.0/L**p for L in Ls]
        a, b, r2 = fit_linear(xs, ys)
        if r2 is not None and r2 > best_r2:
            best_p = p; best_a = a; best_b = b; best_r2 = r2
    return best_a, best_b, best_p, best_r2

def main():
    Ls = [d[0] for d in DATA]
    ys = [d[2] for d in DATA]

    print("=" * 60)
    print("  Continuum Extrapolation: alpha(r_max, L) -> alpha(L->inf)")
    print("  Reference alpha = 1/137.036 =", ALPHA_REF)
    print("=" * 60)
    print()
    print("Raw data (r_max-per-L):")
    for L, r_max, alpha in DATA:
        print(f"  L={L:4d}  r_max={r_max:3d}  alpha_r={alpha:.4f}  "
              f"ratio = {alpha/ALPHA_REF:.2f}? alpha_ref")
    print()

    # 1/L fit
    xs_1 = [1.0/L for L in Ls]
    a1, b1, r2_1 = fit_linear(xs_1, ys)
    print("1/L fit (appropriate for Coulomb-in-periodic-box finite-size):")
    print(f"  alpha(L->inf) = {a1:.6f}  (ratio {a1/ALPHA_REF:.3f}? alpha_ref)")
    print(f"  1/L coeff = {b1:.4f}")
    print(f"  R^2 = {r2_1:.5f}")
    print()

    # 1/L^2 fit
    xs_2 = [1.0/L/L for L in Ls]
    a2, b2, r2_2 = fit_linear(xs_2, ys)
    print("1/L^2 fit (standard lattice-artefact finite-size):")
    print(f"  alpha(L->inf) = {a2:.6f}  (ratio {a2/ALPHA_REF:.3f}? alpha_ref)")
    print(f"  1/L^2 coeff = {b2:.2f}")
    print(f"  R^2 = {r2_2:.5f}")
    print()

    # Free-p fit
    a_p, b_p, p, r2_p = fit_power_law(Ls, ys)
    print(f"Free 1/L^p fit (best p over [0.5, 4.0]):")
    print(f"  best p = {p}")
    print(f"  alpha(L->inf) = {a_p:.6f}  (ratio {a_p/ALPHA_REF:.3f}? alpha_ref)")
    print(f"  coeff = {b_p:.4f}")
    print(f"  R^2 = {r2_p:.5f}")
    print()

    # Predictions at L=512
    L_extrap = 512
    pred_1 = a1 + b1 / L_extrap
    pred_2 = a2 + b2 / (L_extrap**2)
    pred_p = a_p + b_p / (L_extrap**p) if p is not None else None
    print("Predicted alpha(L=512, r_max) for each scaling law:")
    print(f"  1/L fit    : {pred_1:.5f}  (ratio {pred_1/ALPHA_REF:.3f}?)")
    print(f"  1/L^2 fit   : {pred_2:.5f}  (ratio {pred_2/ALPHA_REF:.3f}?)")
    if pred_p is not None:
        print(f"  1/L^{p} fit: {pred_p:.5f}  (ratio {pred_p/ALPHA_REF:.3f}?)")

    print()
    print("Interpretation:")
    best_ratio = min(a1/ALPHA_REF, a2/ALPHA_REF, a_p/ALPHA_REF if a_p > 0 else 1e10)
    worst_ratio = max(a1/ALPHA_REF, a2/ALPHA_REF, a_p/ALPHA_REF if a_p > 0 else 0)
    print(f"  Continuum alpha_inf is between {best_ratio:.2f}? and {worst_ratio:.2f}? alpha_ref,")
    print(f"  depending on scaling-law assumption. Three data points cannot")
    print(f"  discriminate; L=512 measurement would pin it down.")


if __name__ == "__main__":
    main()
