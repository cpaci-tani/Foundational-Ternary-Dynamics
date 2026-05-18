#!/usr/bin/env python3
"""
EXPLORATION · FQCR Observer Operator: Single-Electron Test
===========================================================

Tests whether adding a single phenomenological electron-loop term

    B^(e)(t) = c_e * log(1 + (mu(t)/m_e)^2)

to the FQCR Model V response function

    R^ext(t) = 1 + lambda_N(4it) + A_N(t) + B^(e)(t)

reproduces QED's electron-only one-loop running of alpha^(-1)(mu) across
the moderate-mu region mu in [m_e, m_mu], under heat-kernel Map A
(mu = m_e / t).

The structural prediction from PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md §4-5:
the asymptotic slope dB/d(log mu) -> 2 c_e at large mu must satisfy
2 c_e = +2 delta / (3 pi G*) = +0.0686 (for one active lepton).
So the predicted c_e = +0.0343.

But: the existing FQCR machinery already runs FASTER than QED. Adding
a positive B^(e) makes R increase further with mu, which makes x_+
DECREASE further with mu — i.e., it accelerates the already-too-fast
running. So if the structural prediction is right, it's pushing in
the WRONG direction for matching QED.

This script:
  1. Computes x_+(t; c_e) for a grid of c_e values.
  2. Computes QED alpha^(-1)(mu) under Map A.
  3. Finds the c_e that minimizes residuals across the electron-only
     region t in [0.005, 1].
  4. Reports whether the best-fit c_e equals the structural prediction.

Three outcomes per the proposal §7:
  (A) Match with c_e = +0.0343 -> structural normalization is real.
  (B) Match with some other c_e -> [SELECTION], not structural.
  (C) No c_e matches -> ansatz is wrong.

Status: [EXPLORATORY]. No tag changes regardless of outcome.

Usage:
    python scripts/exploration/explore_fqcr_observer_term.py
"""

from __future__ import annotations

import math
from dataclasses import dataclass


# ----------------------------------------------------------------------------
# FQCR machinery
# ----------------------------------------------------------------------------

def G_N_star(N: int) -> float:
    prod = 1.0
    for n in range(N + 1):
        prod *= (n + 0.75) / (n + 0.25)
    return prod / math.sqrt(N + 1)


def lambda_4it(t: float, N: int = 200) -> float:
    q = math.exp(-4.0 * math.pi * t)
    s2 = sum(q ** ((n + 0.5) ** 2) for n in range(N + 1))
    s3 = sum(q ** (n * n) for n in range(1, N + 1))
    theta2 = 2.0 * s2
    theta3 = 1.0 + 2.0 * s3
    return (theta2 / theta3) ** 4


def A_N(t: float, N: int = 200) -> float:
    Q = math.exp(-2.0 * math.pi * t)
    s1 = 0.0
    s2 = 0.0
    for n in range(1, N + 1):
        Q4n = Q ** (4 * n)
        Q3n = Q ** (3 * n)
        if Q4n < 1.0:
            s1 += n * Q4n / (1.0 - Q4n)
        if Q3n < 1.0:
            s2 += n * Q3n / (1.0 - Q3n)
    return 16.0 * math.pi * s1 - 4.0 * math.pi * s2


# ----------------------------------------------------------------------------
# Observer operator extension
# ----------------------------------------------------------------------------

def B_electron(t: float, c_e: float) -> float:
    """Phenomenological electron-loop term under Map A (mu = m_e/t).

    B^(e)(t) = c_e * log(1 + (mu/m_e)^2) = c_e * log(1 + 1/t^2).

    At t = 1: B = c_e * log(2) ~= 0.693 c_e.
    At t -> 0 (mu >> m_e): B ~= c_e * log(1/t^2) = -2 c_e log(t).
    At t -> inf (mu << m_e): B -> 0 (decoupling).
    """
    return c_e * math.log(1.0 + 1.0 / (t * t))


def x_plus_extended(t: float, c_e: float, N: int = 1024) -> float | None:
    g_n = G_N_star(N)
    R = 1.0 + lambda_4it(t) + A_N(t) + B_electron(t, c_e)
    disc = 4.0 * g_n - R
    if disc < 0:
        return None
    return 8.0 * g_n * g_n + 4.0 * (g_n ** 1.5) * math.sqrt(disc)


# ----------------------------------------------------------------------------
# QED reference
# ----------------------------------------------------------------------------

M_E = 0.51099895
M_MU = 105.6583755
M_TAU = 1776.86
ALPHA_INV_AT_ME = 137.0359990840


def alpha_inv_QED(mu_MeV: float) -> float:
    """One-loop QED electron + muon + tau. Returns the running coupling."""
    if mu_MeV <= M_E:
        return ALPHA_INV_AT_ME
    delta = (2.0 / (3.0 * math.pi)) * math.log(mu_MeV / M_E)
    if mu_MeV > M_MU:
        delta += (2.0 / (3.0 * math.pi)) * math.log(mu_MeV / M_MU)
    if mu_MeV > M_TAU:
        delta += (2.0 / (3.0 * math.pi)) * math.log(mu_MeV / M_TAU)
    return ALPHA_INV_AT_ME - delta


def mu_of_t(t: float) -> float:
    """Map A: mu = m_e / t."""
    return M_E / t


# ----------------------------------------------------------------------------
# Residual analysis
# ----------------------------------------------------------------------------

def residual_at(t: float, c_e: float) -> float | None:
    """x_+(t; c_e) - alpha^(-1)(mu(t))."""
    x = x_plus_extended(t, c_e)
    if x is None:
        return None
    return x - alpha_inv_QED(mu_of_t(t))


def total_residual(c_e: float, t_grid: list[float]) -> float:
    """Sum of squared residuals across t_grid (skipping complex values)."""
    total = 0.0
    count = 0
    for t in t_grid:
        r = residual_at(t, c_e)
        if r is not None:
            total += r * r
            count += 1
    if count == 0:
        return float('inf')
    return math.sqrt(total / count)


# ----------------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------------

def main() -> None:
    print("=" * 80)
    print("  FQCR Observer Operator: Single-Electron Test")
    print("=" * 80)
    print()

    # Structural prediction from PROPOSAL §4
    g_star = 2.95867511957511
    delta = math.sqrt((4 * g_star - 1) / (4 * g_star))
    c_e_structural = 1.0 / 2.0 * 2 * delta / (3 * math.pi * g_star)
    # = delta / (3 pi G*) — derived from 2 c_e = 2 delta / (3 pi G*)
    print(f"Structural prediction:")
    print(f"  delta = sqrt((4G* - 1)/(4G*))   = {delta:.6f}")
    print(f"  G*/delta                        = {g_star/delta:.6f}")
    print(f"  c_e_structural = delta/(3 pi G*) = {c_e_structural:.6f}")
    print()

    # Test grid: t in [0.005, 5] covers (mu_e, ~mu_mu/2) under Map A.
    # Above muon threshold (t < 1/207 = 0.0048) is excluded since QED
    # would activate the muon loop.
    t_grid_e_only = [0.01, 0.02, 0.05, 0.10, 0.20, 0.30, 0.50, 0.65, 0.80, 1.00, 1.50, 2.00, 5.00]

    # Cell 1: at structural c_e
    print("=" * 80)
    print(f"  Test A — at structural c_e = {c_e_structural:+.4f}")
    print("=" * 80)
    print()
    print(f"{'t':>6} {'mu (MeV)':>10} {'x_+ (FQCR+B)':>14} {'alpha^-1 (QED)':>16} {'residual':>12}")
    for t in t_grid_e_only:
        x = x_plus_extended(t, c_e_structural)
        mu = mu_of_t(t)
        a_inv = alpha_inv_QED(mu)
        x_str = f"{x:.6f}" if x is not None else "(complex)"
        r_str = f"{x - a_inv:+.4f}" if x is not None else "n/a"
        print(f"{t:>6.3f} {mu:>10.4f} {x_str:>14} {a_inv:>16.6f} {r_str:>12}")
    print()

    # Cell 2: scan c_e
    print("=" * 80)
    print("  Test B — scan c_e for best fit (RMS residual minimization)")
    print("=" * 80)
    print()
    print(f"{'c_e':>10} {'RMS residual':>14}  {'note':>30}")
    c_e_grid = [-0.50, -0.30, -0.20, -0.10, -0.05, -0.0343, 0.0, 0.0343, 0.05, 0.10]
    for c_e in c_e_grid:
        rms = total_residual(c_e, t_grid_e_only)
        note = "= structural prediction" if abs(c_e - c_e_structural) < 1e-3 else \
               "= -structural" if abs(c_e + c_e_structural) < 1e-3 else ""
        print(f"{c_e:>+10.4f} {rms:>14.4f}  {note:>30}")
    print()

    # Cell 3: fine-grained search for the minimum RMS
    print("=" * 80)
    print("  Test C — golden-section search for optimal c_e")
    print("=" * 80)
    print()
    # bracket near 0
    a, b = -0.5, 0.5
    phi = (math.sqrt(5) - 1) / 2
    for _ in range(50):
        x1 = b - phi * (b - a)
        x2 = a + phi * (b - a)
        if total_residual(x1, t_grid_e_only) < total_residual(x2, t_grid_e_only):
            b = x2
        else:
            a = x1
    c_opt = 0.5 * (a + b)
    rms_opt = total_residual(c_opt, t_grid_e_only)
    print(f"  Optimal c_e (least-squares):    {c_opt:+.6f}")
    print(f"  Optimal RMS residual:           {rms_opt:.6f}")
    print(f"  Structural prediction:          {c_e_structural:+.6f}")
    print(f"  Ratio optimal/structural:       {c_opt/c_e_structural:+.4f}")
    print()

    # Cell 4: per-point comparison at optimal c_e
    print("=" * 80)
    print(f"  Test D — per-point comparison at optimal c_e = {c_opt:+.4f}")
    print("=" * 80)
    print()
    print(f"{'t':>6} {'mu (MeV)':>10} {'x_+ (best)':>12} {'alpha^-1 (QED)':>16} {'residual':>12}")
    for t in t_grid_e_only:
        x = x_plus_extended(t, c_opt)
        mu = mu_of_t(t)
        a_inv = alpha_inv_QED(mu)
        x_str = f"{x:.6f}" if x is not None else "(complex)"
        r_str = f"{x - a_inv:+.4f}" if x is not None else "n/a"
        print(f"{t:>6.3f} {mu:>10.4f} {x_str:>12} {a_inv:>16.6f} {r_str:>12}")
    print()

    # Cell 5: comparison with FQCR base (no B term)
    print("=" * 80)
    print("  Test E — FQCR base (c_e = 0) vs FQCR+B (optimal) vs QED")
    print("=" * 80)
    print()
    print(f"{'t':>6} {'mu (MeV)':>10} {'FQCR base':>12} {'FQCR+B':>12} {'QED':>12} "
          f"{'base-QED':>10} {'+B-QED':>9}")
    for t in t_grid_e_only:
        x_base = x_plus_extended(t, 0.0)
        x_ext = x_plus_extended(t, c_opt)
        mu = mu_of_t(t)
        a_inv = alpha_inv_QED(mu)
        s_base = f"{x_base:.4f}" if x_base is not None else "(complex)"
        s_ext = f"{x_ext:.4f}" if x_ext is not None else "(complex)"
        d_base = f"{x_base - a_inv:+.3f}" if x_base is not None else "n/a"
        d_ext = f"{x_ext - a_inv:+.3f}" if x_ext is not None else "n/a"
        print(f"{t:>6.3f} {mu:>10.4f} {s_base:>12} {s_ext:>12} {a_inv:>12.4f} "
              f"{d_base:>10} {d_ext:>9}")
    print()

    # Test F: REPLACE lambda + A with B (clean separation hypothesis)
    print("=" * 80)
    print("  Test F — clean separation: R = 1 + B^(e) only (no lambda, A)")
    print("=" * 80)
    print()
    print("  Hypothesis: lambda + A in the existing FQCR provide structural content")
    print("  at t=1 (the 5.58e-5 correction) but their RUNNING away from t=1 is")
    print("  NOT QED-faithful. Removing them and substituting B^(e) with structural")
    print("  c_e tests whether QED running emerges from the operator stack alone.")
    print()
    print("  Pinning: B(t) = c * (log(1 + 1/t^2) - log 2), so B(t=1) = 0 and")
    print("  x_+(t=1) = tree-level master-quadratic value 137.0362 (1.26 ppm off CODATA).")
    print()

    def x_plus_replace(t: float, c: float) -> float | None:
        g_n = G_N_star(1024)
        B = c * (math.log(1.0 + 1.0/(t*t)) - math.log(2.0))
        R = 1.0 + B
        disc = 4.0 * g_n - R
        if disc < 0:
            return None
        return 8.0 * g_n * g_n + 4.0 * (g_n ** 1.5) * math.sqrt(disc)

    print(f"{'t':>6} {'mu (MeV)':>10} {'x_+ (replace)':>14} {'alpha^-1 QED':>14} {'resid':>9}")
    test_grid = [0.005, 0.010, 0.020, 0.050, 0.10, 0.20, 0.30, 0.50, 0.65, 0.80, 1.00, 2.00, 5.00]
    for t in test_grid:
        x = x_plus_replace(t, c_e_structural)
        mu = mu_of_t(t)
        a_inv = alpha_inv_QED(mu)
        if x is not None:
            print(f"{t:>6.3f} {mu:>10.4f} {x:>14.6f} {a_inv:>14.6f} {x-a_inv:>+9.4f}")
        else:
            print(f"{t:>6.3f} {mu:>10.4f}      (complex) {a_inv:>14.6f}       n/a")
    print()

    # Slope check under Test F
    print("  Slope check under Test F (c = structural):")
    print(f"  {'t':>6} {'mu (MeV)':>10} {'d x_+/d log mu':>17} {'QED slope':>12} {'rel err':>10}")
    for t_eval in [0.05, 0.10, 0.30, 0.50, 0.80]:
        dt = 0.001
        x1 = x_plus_replace(t_eval - dt, c_e_structural)
        x2 = x_plus_replace(t_eval + dt, c_e_structural)
        mu1 = mu_of_t(t_eval - dt)
        mu2 = mu_of_t(t_eval + dt)
        if x1 and x2:
            slope = (x2 - x1) / (math.log(mu2) - math.log(mu1))
            mu_eval = mu_of_t(t_eval)
            qed_slope = -2.0 / (3.0 * math.pi) if mu_eval > M_E else 0.0
            if mu_eval > M_MU:
                qed_slope -= 2.0 / (3.0 * math.pi)
            rel_err = abs(slope - qed_slope) / abs(qed_slope) * 100 if qed_slope != 0 else float('nan')
            print(f"  {t_eval:>6.3f} {mu_eval:>10.4f} {slope:>+17.4f} {qed_slope:>+12.4f} {rel_err:>9.2f}%")
    print()

    # Verdict
    print("=" * 80)
    print("  Verdict")
    print("=" * 80)
    print()
    rel_match = abs(c_opt - c_e_structural) / abs(c_e_structural) if c_e_structural != 0 else float('inf')
    if rel_match < 0.05:
        print(f"  (A) Optimal c_e matches structural prediction within 5%.")
        print(f"      Structural normalization delta/(3 pi G*) is empirically supported.")
        print(f"      C1 path is open: extend to muon, tau loops next.")
    elif abs(c_opt) < 0.001:
        print(f"  (~null) Optimal c_e is near zero.")
        print(f"      Best-fit B^(e) does NOT improve the FQCR base. Single-lepton")
        print(f"      ansatz is wrong, or the t-scale map is wrong, or both.")
    elif rel_match > 5:
        print(f"  (C) Optimal c_e is far from structural prediction (ratio > 5).")
        print(f"      Either the form B = c log(1 + (mu/m_e)^2) is wrong,")
        print(f"      or the structural derivation in PROPOSAL §4 is wrong.")
    else:
        print(f"  (B) Optimal c_e is non-zero but does not match structural.")
        print(f"      Ratio optimal/structural = {c_opt/c_e_structural:.3f}.")
        print(f"      The match is empirical [SELECTION], not derived.")
    print()


if __name__ == "__main__":
    main()
