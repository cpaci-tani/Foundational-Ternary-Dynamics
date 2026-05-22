#!/usr/bin/env python3
"""
EXPLORATION · FQCR Multi-Lepton Observer Operator (C1 stage 2)
===============================================================

Extends the single-electron observer-term test (EXPLR_FQCR_OBSERVER_TERM_TEST)
to all three charged leptons (electron, muon, tau), under the structural
prediction that each lepton contributes with the SAME coefficient

    c_l = delta / (3 pi G*) ~ 0.0343    (l = e, mu, tau)

derived from the chain-rule constraint dx_+/dR|_{R=1} = -G*/delta.

Configuration: clean separation (Configuration II from EXPLR doc 9.44),
i.e., R = 1 + sum_l B^(l)(t)  WITHOUT the existing lambda + A.

Per-lepton observer term:
    B^(l)(t) = c * [log(1 + (mu(t)/m_l)^2) - log(1 + (m_e/m_l)^2)]

Pinned so each B^(l)(t=1) = 0 -> R(t=1) = 1 -> x_+(t=1) = 137.0362
(the master quadratic tree-level value, 1.26 ppm off CODATA Thomson limit).

Map: mu(t) = m_e / t (heat-kernel Map A from EXPLR doc 9.42).

Test points span four regimes:
  - mu < m_e (deep IR, no leptons active in QED): t > 1
  - m_e < mu < m_mu (electron-only): t in [m_e/m_mu, 1] ~ [0.005, 1]
  - m_mu < mu < m_tau (e + mu): t in [m_e/m_tau, m_e/m_mu] ~ [0.00029, 0.005]
  - mu > m_tau (all three): t in [0, 0.00029]

QED reference: lepton-only one-loop running of alpha^-1(mu).

Predictions (structural; if confirmed, supports C1 path closability):
  - At mu in [5, 50] m_e: slope = -0.212 per active lepton (electron only)
  - At mu in [m_mu, m_tau]: slope = -0.424 (e + mu)
  - At mu > m_tau: slope = -0.637 (all three)

Status: [EXPLORATORY]. No tag changes regardless of outcome.

Usage:
    python scripts/exploration/explore_fqcr_multilepton.py
"""

from __future__ import annotations

import math


# ---------- Constants (FTD + lepton masses in MeV) -----------------------

G_STAR = 2.95867511957511
DELTA = math.sqrt((4 * G_STAR - 1) / (4 * G_STAR))
C_STRUCTURAL = DELTA / (3 * math.pi * G_STAR)  # ~ 0.034313

M_E = 0.51099895
M_MU = 105.6583755
M_TAU = 1776.86
ALPHA_INV_AT_LOWE = 137.0359991770  # CODATA 2022, Thomson limit


# ---------- FQCR pieces --------------------------------------------------

def G_N_star(N: int) -> float:
    prod = 1.0
    for n in range(N + 1):
        prod *= (n + 0.75) / (n + 0.25)
    return prod / math.sqrt(N + 1)


def mu_of_t(t: float) -> float:
    """Map A: mu = m_e / t (in MeV)."""
    return M_E / t


def B_lepton(t: float, m_l_MeV: float, c: float) -> float:
    """Observer-term contribution from lepton with mass m_l.

    B^(l)(t) = c * [log(1 + (mu/m_l)^2) - log(1 + (m_e/m_l)^2)]

    Pinned so B^(l)(t=1) = 0 (since mu(t=1) = m_e).
    """
    mu = mu_of_t(t)
    pin = c * math.log(1.0 + (M_E / m_l_MeV) ** 2)
    return c * math.log(1.0 + (mu / m_l_MeV) ** 2) - pin


def R_extended(t: float, c_e: float, c_mu: float, c_tau: float) -> float:
    """R(t) = 1 + B^(e) + B^(mu) + B^(tau). No lambda, no A."""
    return (1.0
            + B_lepton(t, M_E, c_e)
            + B_lepton(t, M_MU, c_mu)
            + B_lepton(t, M_TAU, c_tau))


def x_plus(t: float, c_e: float = C_STRUCTURAL,
           c_mu: float = C_STRUCTURAL, c_tau: float = C_STRUCTURAL,
           N: int = 1024) -> float | None:
    g = G_N_star(N)
    R = R_extended(t, c_e, c_mu, c_tau)
    disc = 4.0 * g - R
    if disc < 0:
        return None
    return 8.0 * g * g + 4.0 * (g ** 1.5) * math.sqrt(disc)


# ---------- QED reference ------------------------------------------------

def alpha_inv_QED_lepton(mu_MeV: float) -> float:
    """One-loop lepton-only QED running."""
    if mu_MeV <= M_E:
        return ALPHA_INV_AT_LOWE
    delta_qed = (2.0 / (3.0 * math.pi)) * math.log(mu_MeV / M_E)
    if mu_MeV > M_MU:
        delta_qed += (2.0 / (3.0 * math.pi)) * math.log(mu_MeV / M_MU)
    if mu_MeV > M_TAU:
        delta_qed += (2.0 / (3.0 * math.pi)) * math.log(mu_MeV / M_TAU)
    return ALPHA_INV_AT_LOWE - delta_qed


def qed_slope(mu_MeV: float) -> float:
    """Predicted slope: d alpha^-1 / d log mu."""
    coeff = -2.0 / (3.0 * math.pi)
    n = 0
    if mu_MeV > M_E: n += 1
    if mu_MeV > M_MU: n += 1
    if mu_MeV > M_TAU: n += 1
    return n * coeff


# ---------- Slope estimator (centered finite difference + chain rule) ---

def slope_at(t_eval: float, c: float = C_STRUCTURAL) -> tuple[float, float]:
    """Return (FD slope, chain-rule slope) at t = t_eval."""
    # Use a relative step that stays well inside the domain
    dt = t_eval * 1e-3
    x1 = x_plus(t_eval - dt, c, c, c)
    x2 = x_plus(t_eval + dt, c, c, c)
    mu1 = mu_of_t(t_eval - dt)
    mu2 = mu_of_t(t_eval + dt)
    if x1 is None or x2 is None:
        return float('nan'), float('nan')
    fd = (x2 - x1) / (math.log(mu2) - math.log(mu1))

    # Chain rule
    g = G_N_star(1024)
    R = R_extended(t_eval, c, c, c)
    dx_dR = -2.0 * (g ** 1.5) / math.sqrt(4.0 * g - R)
    mu = mu_of_t(t_eval)
    # dB/d log mu = sum_l 2 c (mu/m_l)^2 / (1 + (mu/m_l)^2)
    dR_dlogmu = sum(
        2.0 * c * (mu / m_l) ** 2 / (1.0 + (mu / m_l) ** 2)
        for m_l in (M_E, M_MU, M_TAU)
    )
    chain = dx_dR * dR_dlogmu
    return fd, chain


# ---------- Main ---------------------------------------------------------

def main() -> None:
    print("=" * 80)
    print("  FQCR Multi-Lepton Observer Test (Configuration II)")
    print(f"  R(t) = 1 + B^(e) + B^(mu) + B^(tau);  c_l = {C_STRUCTURAL:.6f} (structural)")
    print("=" * 80)
    print()

    # Test grid spans all four regimes
    t_grid = [
        # mu < m_e (deep IR, all decoupled)
        5.0, 2.0, 1.5,
        # m_e < mu < m_mu (electron only active)
        1.0, 0.5, 0.1, 0.05, 0.01, 0.006,
        # m_mu < mu < m_tau (e + mu)
        0.004, 0.002, 0.001, 0.0005,
        # mu > m_tau (all three)
        0.0002, 0.0001, 0.00005, 0.00001,
    ]

    print("--- Per-point comparison (FQCR vs lepton-only QED) ---")
    print(f"{'t':>10} {'mu (MeV)':>10} {'#act':>5} {'x_+(FQCR)':>12} {'a^-1(QED)':>12} {'resid':>8}")
    for t in t_grid:
        x = x_plus(t)
        mu = mu_of_t(t)
        a_inv = alpha_inv_QED_lepton(mu)
        n_active = sum(1 for m in (M_E, M_MU, M_TAU) if mu > m)
        if x is None:
            print(f"{t:>10.5f} {mu:>10.2e} {n_active:>5} {'(complex)':>12} {a_inv:>12.6f}    n/a")
        else:
            print(f"{t:>10.5f} {mu:>10.2e} {n_active:>5} {x:>12.6f} {a_inv:>12.6f} {x-a_inv:>+8.4f}")
    print()

    # Slope check at points safely inside each regime
    print("--- Slope at log mu (FQCR vs QED) ---")
    print(f"{'t':>10} {'mu (MeV)':>11} {'regime':>10} {'FD slope':>11} "
          f"{'chain':>11} {'QED':>11} {'rel err':>10}")
    slope_test_points = [
        (0.05,  "e only"),    # mu = 10.2 MeV
        (0.01,  "e only"),    # mu = 51 MeV
        (0.003, "e + mu"),    # mu = 170 MeV (just above muon)
        (0.001, "e + mu"),    # mu = 511 MeV
        (0.0003, "e + mu"),   # mu = 1.7 GeV (just below tau)
        (0.0001, "all 3"),    # mu = 5.1 GeV
        (0.00003, "all 3"),   # mu = 17 GeV
    ]
    for t_eval, regime in slope_test_points:
        mu = mu_of_t(t_eval)
        fd, chain = slope_at(t_eval)
        q = qed_slope(mu)
        if math.isnan(fd):
            print(f"{t_eval:>10.5f} {mu:>11.2e} {regime:>10}     (complex)")
        else:
            err = abs((fd - q) / q) * 100 if q != 0 else float('nan')
            print(f"{t_eval:>10.5f} {mu:>11.2e} {regime:>10} {fd:>+11.4f} "
                  f"{chain:>+11.4f} {q:>+11.4f} {err:>9.2f}%")
    print()

    # Where is the FQCR Landau-like point under multi-lepton?
    g = G_N_star(1024)
    print(f"--- FQCR Landau-like point (where R = 4G* = {4*g:.4f}) ---")
    # Bisect
    t_low, t_high = 1e-200, 1.0
    for _ in range(2000):
        t_mid = math.sqrt(t_low * t_high) if t_low > 0 else (t_low + t_high) / 2
        R_mid = R_extended(t_mid, C_STRUCTURAL, C_STRUCTURAL, C_STRUCTURAL)
        if R_mid > 4 * g:
            t_low = t_mid
        else:
            t_high = t_mid
    t_pole = math.sqrt(t_low * t_high) if t_low > 0 else (t_low + t_high) / 2
    mu_pole = mu_of_t(t_pole)
    print(f"  t_* = {t_pole:.3e}, mu_* = {mu_pole:.3e} MeV")
    print(f"  Convert to GeV: {mu_pole/1000:.3e} GeV")
    print(f"  QED Landau pole (lepton-only one-loop est): "
          f"{M_E * math.exp(3*math.pi*ALPHA_INV_AT_LOWE/(2*3)):.3e} MeV")
    # The lepton-only QED Landau pole is ~10^93 MeV with 3-lepton b_0
    # (Real QED has Landau pole at ~10^286 MeV including all SM contributions)
    print()


if __name__ == "__main__":
    main()
