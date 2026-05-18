#!/usr/bin/env python3
"""
EXPLORATION · FQCR t <-> Scale Map Candidates
==============================================

Tests three candidate t <-> scale maps for the FQCR Model V branch readout
x_+(t), and compares each against QED's experimentally-measured running of
alpha^-1(mu).

The four open questions raised in the 2026-05-08 operator-stack discussion
were addressed in:
  - Q1 (R-law):   EXPLR_FQCR_RESPONSE_LAW_TEST.md (no decisive winner;
                   additive [SELECTION] stands)
  - Q2 (16=4^2):  EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md (rank 1, 5-orders gap)
  - Q3 ((4,6;3,2)): FTD-0143 pre-registered (formal scan still pending)
  - Q4 (t=1 phys): THIS SCRIPT — what does t correspond to physically?

This script proposes:

  Map A (heat-kernel time):
     mu(t) = m_* / t
     where m_* is the FTD natural mass unit (m_e in the dimensional map)
     -> t=1 corresponds to mu=m_*, small t is UV (large mu), large t is IR.

  Map B (logarithmic / RG):
     mu(t) = m_* * exp((1 - t) / c)
     for some natural c. Linear in log(mu).

  Map C (lattice cutoff):
     a(t) = a_phys * sqrt(t)
     mu(t) = 1 / a(t) = (1 / a_phys) * t^(-1/2)
     -> t=1 corresponds to mu = 1/a_phys = m_P (Planck), small t is sub-Planck.

For each map, the script computes x_+(t) and compares to one-loop QED running
alpha^-1(mu) = alpha^-1(m_e) - (2/(3 pi)) sum_{leptons l: m_l < mu} log(mu/m_l)

The comparison uses lepton-only one-loop running (electron + muon + tau) for
mu in [m_e, ~10 GeV]. This is a clean comparison up to the hadronic threshold.

Key observation: the FQCR-EM branch has a "Landau-pole-like" structural point
where R(t) = 4 G_N* and the discriminant vanishes (x_+ = x_-). At that point
the EM and color branches coalesce. If the t-scale map is correct, this point
must correspond to a physical scale where something specific happens.

Status: [EXPLORATORY]. No tag promotions. Does not close the t-scale map
question; surfaces structural mismatches that any final map must resolve.

Usage:
    python scripts/exploration/explore_fqcr_t_scale_map.py
"""

from __future__ import annotations

import math
from dataclasses import dataclass


# ----------------------------------------------------------------------------
# FQCR machinery (replicating explore_fqcr_response_laws.py at converged N)
# ----------------------------------------------------------------------------

def G_N_star(N: int) -> float:
    prod = 1.0
    for n in range(N + 1):
        prod *= (n + 0.75) / (n + 0.25)
    return prod / math.sqrt(N + 1)


def lambda_4it(t: float, N: int = 200) -> float:
    """lambda_N(4it) = (theta_2(0|4it)/theta_3(0|4it))^4."""
    q = math.exp(-4.0 * math.pi * t)
    s2 = 0.0
    for n in range(N + 1):
        s2 += q ** ((n + 0.5) ** 2)
    theta2 = 2.0 * s2

    s3 = 0.0
    for n in range(1, N + 1):
        s3 += q ** (n * n)
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


def x_plus_FQCR(t: float, N: int = 1024) -> float | None:
    """The FQCR Model V branch readout at parameter t."""
    g_n = G_N_star(N)
    R = 1.0 + lambda_4it(t, N=200) + A_N(t, N=200)
    disc = 4.0 * g_n - R
    if disc < 0:
        return None  # Landau-like
    return 8.0 * g_n * g_n + 4.0 * (g_n ** 1.5) * math.sqrt(disc)


# ----------------------------------------------------------------------------
# QED one-loop running (electron + muon + tau)
# ----------------------------------------------------------------------------

# Lepton masses in MeV (CODATA 2022)
M_E = 0.51099895
M_MU = 105.6583755
M_TAU = 1776.86

ALPHA_INV_AT_ME = 137.0359990840  # CODATA at low energy (Thomson)


def alpha_inv_QED_lepton(mu_MeV: float) -> float:
    """One-loop QED running with electron + muon + tau contributions.

    alpha^-1(mu) = alpha^-1(m_e) - (2/(3 pi)) sum_{leptons l: m_l < mu} log(mu/m_l)

    Below electron threshold: no running (electron decoupled at m_e).
    Above electron, below muon: electron loop only.
    Above muon, below tau: + muon loop.
    Above tau: + tau loop.
    """
    if mu_MeV <= M_E:
        return ALPHA_INV_AT_ME

    delta = 0.0
    coeff = 2.0 / (3.0 * math.pi)
    delta += coeff * math.log(mu_MeV / M_E)
    if mu_MeV > M_MU:
        delta += coeff * math.log(mu_MeV / M_MU)
    if mu_MeV > M_TAU:
        delta += coeff * math.log(mu_MeV / M_TAU)
    return ALPHA_INV_AT_ME - delta


# ----------------------------------------------------------------------------
# Three candidate t <-> mu maps
# ----------------------------------------------------------------------------

def mu_heat_kernel(t: float, m_ref_MeV: float = M_E) -> float:
    """Map A: mu(t) = m_ref / t (heat-kernel reading; t=1 <-> mu=m_ref)."""
    return m_ref_MeV / t


def mu_rg_log(t: float, m_ref_MeV: float = M_E, c: float = 1.0) -> float:
    """Map B: mu(t) = m_ref * exp((1-t)/c) (RG-log reading)."""
    return m_ref_MeV * math.exp((1.0 - t) / c)


def mu_lattice_cutoff(t: float, m_lattice_MeV: float = 1.221e22) -> float:
    """Map C: mu(t) = m_lattice * t^(-1/2) (lattice-cutoff reading;
    m_lattice = 1/a_phys = m_P in MeV ~ 1.221e22)."""
    return m_lattice_MeV / math.sqrt(t)


# ----------------------------------------------------------------------------
# Main comparison
# ----------------------------------------------------------------------------

@dataclass
class Comparison:
    t: float
    x_FQCR: float | None
    map_name: str
    mu_MeV: float
    alpha_inv_QED: float
    diff: float | None  # x_FQCR - alpha_inv_QED


def compare_at_t(t: float, map_name: str, mu_fn) -> Comparison:
    x = x_plus_FQCR(t)
    mu = mu_fn(t)
    a_inv = alpha_inv_QED_lepton(mu)
    diff = x - a_inv if x is not None else None
    return Comparison(t=t, x_FQCR=x, map_name=map_name, mu_MeV=mu,
                       alpha_inv_QED=a_inv, diff=diff)


def main() -> None:
    print("=" * 80)
    print("  FQCR t <-> Scale Map Candidates: numerical comparison vs QED")
    print("=" * 80)

    t_grid = [0.10, 0.30, 0.50, 0.80, 1.00, 1.50, 2.00, 3.00, 5.00, 10.0, 50.0]

    for map_name, mu_fn in [
        ("A (heat-kernel: mu=m_e/t)",       lambda t: mu_heat_kernel(t)),
        ("B (RG-log:    mu=m_e*exp(1-t))",  lambda t: mu_rg_log(t)),
        ("C (lattice:   mu=m_P/sqrt(t))",   lambda t: mu_lattice_cutoff(t)),
    ]:
        print(f"\n--- Map {map_name} ---")
        print(f"{'t':>6} {'mu (MeV)':>14} {'x_+ (FQCR)':>13} "
              f"{'alpha^-1 (QED)':>16} {'FQCR - QED':>13}")
        for t in t_grid:
            c = compare_at_t(t, map_name, mu_fn)
            x_str = f"{c.x_FQCR:.6f}" if c.x_FQCR is not None else "(complex)"
            d_str = f"{c.diff:+.4f}" if c.diff is not None else "n/a"
            mu_str = f"{c.mu_MeV:.3e}" if c.mu_MeV > 1000 else f"{c.mu_MeV:.4f}"
            print(f"{c.t:>6.2f} {mu_str:>14} {x_str:>13} {c.alpha_inv_QED:>16.6f} {d_str:>13}")

    # Where is the FQCR Landau-pole-like point?
    print()
    print("=" * 80)
    print("  FQCR structural boundary: where R(t) = 4 G_N* (x_+ merges with x_-)")
    print("=" * 80)
    print()
    print("This is the FQCR analog of a Landau pole. The t-scale map must")
    print("explain what physical scale this corresponds to.")
    print()

    # Find t where R hits 4G*
    g_n = G_N_star(1024)
    target = 4.0 * g_n
    t_low = 0.01
    t_high = 1.0
    for _ in range(60):
        t_mid = 0.5 * (t_low + t_high)
        R_mid = 1.0 + lambda_4it(t_mid) + A_N(t_mid)
        if R_mid > target:
            t_low = t_mid
        else:
            t_high = t_mid
    t_pole = 0.5 * (t_low + t_high)

    print(f"  FQCR Landau-like point: t* = {t_pole:.5f}  (R(t*) = {1+lambda_4it(t_pole)+A_N(t_pole):.4f}, 4G* = {target:.4f})")
    print()
    print(f"  Map A (mu=m_e/t):      mu* = {mu_heat_kernel(t_pole):.4f} MeV  "
          f"= {mu_heat_kernel(t_pole) / M_E:.2f} m_e")
    print(f"  Map B (RG-log):        mu* = {mu_rg_log(t_pole):.4e} MeV")
    print(f"  Map C (lattice):       mu* = {mu_lattice_cutoff(t_pole):.4e} MeV")
    print()
    print(f"  QED Landau pole (electron-only one-loop):")
    print(f"     mu_L = m_e * exp(3 pi / (2 alpha)) = m_e * exp({3*math.pi*ALPHA_INV_AT_ME/2:.1f})")
    print(f"     = ~10^{int(math.log10(M_E * math.exp(3*math.pi*ALPHA_INV_AT_ME/2))):d} MeV  (astronomical)")
    print()

    # QED running coefficient comparison
    print("=" * 80)
    print("  QED-style running: dx_+/d(log mu) under each map (at moderate t)")
    print("=" * 80)
    print()
    print("  QED prediction (electron + muon + tau, between thresholds):")
    print(f"     d(alpha^-1)/d(log mu) = -2/(3 pi) * N_active = -{2/(3*math.pi):.4f} per active lepton")
    print(f"     = -{2*1/(3*math.pi):.4f} (electron only)")
    print(f"     = -{2*2/(3*math.pi):.4f} (electron + muon)")
    print(f"     = -{2*3/(3*math.pi):.4f} (all three)")
    print()

    # Numerically estimate dx/d(log mu) at t=0.5 and t=1 under Map A
    for t_eval in [0.3, 0.5, 0.8, 1.0, 1.5]:
        dt = 0.001
        x1 = x_plus_FQCR(t_eval - dt)
        x2 = x_plus_FQCR(t_eval + dt)
        if x1 is not None and x2 is not None:
            dx = x2 - x1
            mu1 = mu_heat_kernel(t_eval - dt)
            mu2 = mu_heat_kernel(t_eval + dt)
            d_log_mu = math.log(mu2) - math.log(mu1)
            slope = dx / d_log_mu
            print(f"  Map A at t={t_eval:.2f}: dx_+/d(log mu) = {slope:+.4f}")

    print()
    print("=" * 80)
    print("Done.")


if __name__ == "__main__":
    main()
