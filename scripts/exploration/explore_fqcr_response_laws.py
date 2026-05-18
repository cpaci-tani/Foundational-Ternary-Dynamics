#!/usr/bin/env python3
"""
EXPLORATION · FQCR Response-Law Comparison
==========================================

Tests three candidate response laws for R_N(t) in the FQCR Model V transfer
matrix, as proposed in §8 of the 2026-05-08 operator-stack discussion:

    R_add  = 1 + lambda_N(4it) + A_N(t)
    R_mult = (1 + lambda_N) * (1 + A_N)
    R_exp  = exp(lambda_N + A_N)

For each law, computes:

    x_+(N, t) = 8 (G_N*)^2 + 4 (G_N*)^(3/2) sqrt(4 G_N* - R(t))

across t in [0.3, 3.0] for N in {32, 128, 512}, and reports:

  (a) real-domain validity: does R(t) < 4 G_N* hold across the range?
  (b) smoothness:           is x_+(t) C^1 (no kinks)?
  (c) monotonicity:         is dx_+/dt sign-stable?
  (d) finite-N convergence: do the curves converge as N grows?
  (e) parsimony:            do the laws differ at the t=1 base point?

Pre-registration note: this is exploratory — it tests structural-stability
properties of the additive law (currently [SELECTION] per SPEC_FQCR §3.3),
not its numerical agreement with experiment. The CODATA precision match at
t=1 is itself contingent on the additive selection (as documented in
EXPLR_GSTAR_FLUX_TIME and the 2026-05-08 audit) and would be confirmation
bias if used as the criterion here.

Status: [EXPLORATORY]. Not promoted to LEDGER.

Usage:
    python scripts/exploration/explore_fqcr_response_laws.py
"""

from __future__ import annotations

import math
from dataclasses import dataclass


# ----------------------------------------------------------------------------
# Stack components (per the §1-§5 operator stack from 2026-05-08 discussion)
# ----------------------------------------------------------------------------

# Layer 1: G_N* via quarter-determinant ratio (canonical, FTD-0142)
def G_N_star(N: int) -> float:
    """G_N* = (N+1)^(-1/2) * prod_{n=0..N} (n+3/4)/(n+1/4)."""
    prod = 1.0
    for n in range(N + 1):
        prod *= (n + 0.75) / (n + 0.25)
    return prod / math.sqrt(N + 1)


# Layer 4: lambda_N(4it) = (theta_{2,N}/theta_{3,N})^4 at tau = 4it
# Convention: theta with q = e^(i pi tau), so tau = 4it -> q = e^(-4 pi t)
def theta2_N(t: float, N: int) -> float:
    """theta_2(0|4it) truncated to N terms.
    theta_2(0|tau) = 2 * sum_{n=0..inf} q^((n+1/2)^2)  where q = e^(i pi tau).
    At tau = 4it, q = e^(-4 pi t)."""
    q = math.exp(-4.0 * math.pi * t)
    s = 0.0
    for n in range(N + 1):
        s += q ** ((n + 0.5) ** 2)
    return 2.0 * s


def theta3_N(t: float, N: int) -> float:
    """theta_3(0|4it) truncated to N terms.
    theta_3(0|tau) = 1 + 2 * sum_{n=1..inf} q^(n^2)."""
    q = math.exp(-4.0 * math.pi * t)
    s = 0.0
    for n in range(1, N + 1):
        s += q ** (n * n)
    return 1.0 + 2.0 * s


def lambda_N(t: float, N: int) -> float:
    """lambda_N(4it) = (theta_2/theta_3)^4."""
    return (theta2_N(t, N) / theta3_N(t, N)) ** 4


# Layer 3: A_N(t) anomaly pressure
# A_N(t) = 16 pi sum n Q^(4n) / (1 - Q^(4n)) - 4 pi sum n Q^(3n) / (1 - Q^(3n))
# where Q = e^(-2 pi t)
def A_N(t: float, N: int) -> float:
    Q = math.exp(-2.0 * math.pi * t)
    s1 = 0.0  # sum_{n=1..N} n * Q^(4n) / (1 - Q^(4n))
    s2 = 0.0  # sum_{n=1..N} n * Q^(3n) / (1 - Q^(3n))
    for n in range(1, N + 1):
        Q4n = Q ** (4 * n)
        Q3n = Q ** (3 * n)
        if Q4n < 1.0:  # avoid div-by-zero (only matters at t -> 0)
            s1 += n * Q4n / (1.0 - Q4n)
        if Q3n < 1.0:
            s2 += n * Q3n / (1.0 - Q3n)
    return 16.0 * math.pi * s1 - 4.0 * math.pi * s2


# ----------------------------------------------------------------------------
# The three response laws
# ----------------------------------------------------------------------------

def R_add(lam: float, a: float) -> float:
    return 1.0 + lam + a


def R_mult(lam: float, a: float) -> float:
    return (1.0 + lam) * (1.0 + a)


def R_exp(lam: float, a: float) -> float:
    return math.exp(lam + a)


# ----------------------------------------------------------------------------
# x_+(N, t) under a given response law
# ----------------------------------------------------------------------------

@dataclass
class Result:
    t: float
    G_N: float
    lam: float
    a: float
    R_add: float
    R_mult: float
    R_exp: float
    x_add: float | None
    x_mult: float | None
    x_exp: float | None


def compute(t: float, N: int) -> Result:
    g_n = G_N_star(N)
    lam = lambda_N(t, N)
    a = A_N(t, N)
    r_a = R_add(lam, a)
    r_m = R_mult(lam, a)
    r_e = R_exp(lam, a)

    def x_plus(R: float) -> float | None:
        # Real-domain check: R < 4 G_N*
        disc = 4.0 * g_n - R
        if disc < 0:
            return None
        return 8.0 * g_n * g_n + 4.0 * (g_n ** 1.5) * math.sqrt(disc)

    return Result(
        t=t, G_N=g_n, lam=lam, a=a,
        R_add=r_a, R_mult=r_m, R_exp=r_e,
        x_add=x_plus(r_a), x_mult=x_plus(r_m), x_exp=x_plus(r_e),
    )


# ----------------------------------------------------------------------------
# Stability metrics
# ----------------------------------------------------------------------------

def stability_report(results: list[Result], law_name: str, x_attr: str) -> dict:
    """Compute (a)-(d) stability metrics for a single law."""
    xs = [getattr(r, x_attr) for r in results]
    real_count = sum(1 for x in xs if x is not None)
    real_domain_ok = real_count == len(xs)

    # Monotonicity: dx/dt should be sign-stable across the range
    # Compute first differences
    valid_idx = [i for i, x in enumerate(xs) if x is not None]
    if len(valid_idx) < 2:
        mono = "n/a"
        signs_changes = -1
    else:
        diffs = [xs[valid_idx[i + 1]] - xs[valid_idx[i]] for i in range(len(valid_idx) - 1)]
        signs = [1 if d > 0 else (-1 if d < 0 else 0) for d in diffs]
        signs_changes = sum(1 for i in range(len(signs) - 1) if signs[i] * signs[i + 1] < 0)
        mono = "yes" if signs_changes == 0 else f"no ({signs_changes} sign changes)"

    # Smoothness: max relative second difference
    if len([x for x in xs if x is not None]) >= 3:
        valid_xs = [xs[i] for i in valid_idx]
        second = [valid_xs[i + 2] - 2 * valid_xs[i + 1] + valid_xs[i]
                  for i in range(len(valid_xs) - 2)]
        max_curvature = max(abs(s) for s in second) if second else 0.0
    else:
        max_curvature = float('nan')

    return {
        "law": law_name,
        "real_domain_ok": real_domain_ok,
        "real_points": real_count,
        "total_points": len(xs),
        "monotonic": mono,
        "max_2nd_diff": max_curvature,
    }


# ----------------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------------

def main() -> None:
    print("=" * 78)
    print("  FQCR Response-Law Comparison")
    print("  R_add(t)  = 1 + lambda(4it) + A(t)")
    print("  R_mult(t) = (1 + lambda)(1 + A)")
    print("  R_exp(t)  = exp(lambda + A)")
    print("=" * 78)
    print()

    # t-grid: dense around t=1 (the SELECTION point), wider around it
    t_grid = [0.30, 0.40, 0.50, 0.65, 0.80, 0.95, 1.00, 1.05, 1.20,
              1.40, 1.60, 1.80, 2.00, 2.40, 2.80, 3.00]

    for N in [32, 128, 512]:
        print(f"--- N = {N} -----------------------------------------------------------")
        print(f"{'t':>5} {'G_N*':>10} {'lambda':>10} {'A':>10} {'R_add':>11} "
              f"{'x_add':>10} {'x_mult':>10} {'x_exp':>10}")

        results = []
        for t in t_grid:
            r = compute(t, N)
            results.append(r)
            xa = f"{r.x_add:.6f}" if r.x_add is not None else "(complex)"
            xm = f"{r.x_mult:.6f}" if r.x_mult is not None else "(complex)"
            xe = f"{r.x_exp:.6f}" if r.x_exp is not None else "(complex)"
            print(f"{r.t:>5.2f} {r.G_N:>10.6f} {r.lam:>10.4e} {r.a:>10.4e} "
                  f"{r.R_add:>11.6f} {xa:>10} {xm:>10} {xe:>10}")
        print()

        # Stability summary
        for law_name, x_attr in [("additive", "x_add"),
                                  ("multiplicative", "x_mult"),
                                  ("exponential", "x_exp")]:
            rep = stability_report(results, law_name, x_attr)
            print(f"  [{rep['law']:>15}] real-domain: {rep['real_points']}/{rep['total_points']}  "
                  f"monotonic: {rep['monotonic']}  max-2nd-diff: {rep['max_2nd_diff']:.4f}")
        print()

    # Cross-N convergence at the SELECTION point t=1
    print("=" * 78)
    print("  Cross-N convergence at t = 1 (the SELECTION base point)")
    print("=" * 78)
    print()
    print(f"{'N':>5} {'G_N*':>14} {'lambda':>14} {'A':>14} "
          f"{'x_add':>14} {'x_mult':>14} {'x_exp':>14}")
    for N in [16, 32, 64, 128, 256, 512, 1024]:
        r = compute(1.0, N)
        print(f"{N:>5} {r.G_N:>14.10f} {r.lam:>14.6e} {r.a:>14.6e} "
              f"{r.x_add:>14.9f} {r.x_mult:>14.9f} {r.x_exp:>14.9f}")
    print()

    # CODATA reference for context
    codata = 137.035999084
    print(f"CODATA 2022 alpha^-1 = {codata:.9f}")
    print()

    # Pairwise differences at t=1
    print("--- Pairwise difference at t=1, N=512 (test of law-distinguishability) ---")
    r = compute(1.0, 512)
    print(f"  x_add  - x_mult = {(r.x_add - r.x_mult):.3e}")
    print(f"  x_add  - x_exp  = {(r.x_add - r.x_exp):.3e}")
    print(f"  x_mult - x_exp  = {(r.x_mult - r.x_exp):.3e}")
    print()

    # Pairwise differences at t=0.3 (away-from-base test)
    print("--- Pairwise difference at t=0.3, N=512 (laws should diverge here) ---")
    r = compute(0.3, 512)
    if r.x_add is not None and r.x_mult is not None and r.x_exp is not None:
        print(f"  x_add  = {r.x_add:.6f}")
        print(f"  x_mult = {r.x_mult:.6f}")
        print(f"  x_exp  = {r.x_exp:.6f}")
        print(f"  x_add  - x_mult = {(r.x_add - r.x_mult):.4e}")
        print(f"  x_add  - x_exp  = {(r.x_add - r.x_exp):.4e}")
        print(f"  x_mult - x_exp  = {(r.x_mult - r.x_exp):.4e}")
    else:
        print("  At least one law went complex at t=0.3 (R > 4 G*)")
    print()

    # Where does R > 4 G* first occur? (real-domain failure threshold)
    print("--- Real-domain failure threshold (smallest t where R > 4 G_N*) ---")
    for N in [32, 128, 512]:
        g_n = G_N_star(N)
        for law_name, R_fn in [("R_add", R_add), ("R_mult", R_mult), ("R_exp", R_exp)]:
            t_test = 0.30
            while t_test > 0.0:
                lam = lambda_N(t_test, N)
                a = A_N(t_test, N)
                R = R_fn(lam, a)
                if R >= 4.0 * g_n:
                    break
                t_test -= 0.01
            if t_test <= 0.0:
                print(f"  N={N:4d}  {law_name}: real-valid down to t=0.01 (no failure in range)")
            else:
                print(f"  N={N:4d}  {law_name}: real-failure at t={t_test:.2f} "
                      f"(R={R:.4f}, 4G*={4*g_n:.4f})")
    print()

    print("Done.")


if __name__ == "__main__":
    main()
