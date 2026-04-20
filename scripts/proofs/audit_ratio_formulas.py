#!/usr/bin/env python3
"""
audit_ratio_formulas.py — rigidity tests on FTD's integer-rational mass
and coupling claims.

Formulas audited:

  m_p / m_e = N_eff/alpha + N_base * N_eff + N_c  [DERIVED per catalog]
            = 13/alpha + 4*13 + 3 = 1836.47
  experimental = 1836.15267344 -> 174 ppm

  alpha_s(M_Z) = 7/59  [DERIVED, claim 0.6%]
  experimental = 0.1179(10)
  7/59 = 0.11864... -> 0.6% agreement

  sin^2 theta_W = 3/13  [THEOREM per catalog, N_c/N_eff]
  experimental (on-shell) = 0.22290
  3/13 = 0.23077 -> 3.5% agreement

  PMNS angles:
    sin^2 theta_12 = 3/10    (exp 0.307, claim 0.300 -> 2.3%)
    sin^2 theta_23 = 16/29   (exp 0.546, claim 0.552 -> 1.1%)
    sin^2 theta_13 = 1/52    (exp 0.0220, claim 0.0192 -> 13%)
    Dm^2_31 / Dm^2_21 = 100/3 (exp 32.8, claim 33.3 -> 1.5%)

For each, rigidity test: among rationals p/q with p, q <= 30 (or appropriate
range), how many hit the experimental value within various tolerances?
Is the FTD formula unique, most-precise, or one of many?
"""
from __future__ import annotations
import math
from typing import Sequence

# Experimental targets (PDG 2024 central values where applicable)
CLAIMS = [
    # (label, formula, claim_formula, exp_value, precision_ppm_exp)
    ("m_p / m_e",        "N_eff/alpha + N_base*N_eff + N_c", 1836.47, 1836.15267344, 30),
    ("alpha_s(M_Z)",     "7/59",                             7/59,    0.1179, 8500),
    ("sin^2 theta_W",    "3/13",                             3/13,    0.22290, 20),
    ("sin^2 theta_12",   "3/10",                             3/10,    0.307, 6500),
    ("sin^2 theta_23",   "16/29",                            16/29,   0.546, 6400),
    ("sin^2 theta_13",   "1/52",                             1/52,    0.0220, 3400),
    ("Dm^2_31 / Dm^2_21", "100/3",                            100/3,   32.8, 9000),
]

def count_hits_within_tolerance(target: float, tol_list: Sequence[float],
                                 p_max: int = 200, q_max: int = 60) -> None:
    """For each tolerance, count how many rationals p/q hit the target."""
    tallies = {t: [] for t in tol_list}
    total = 0
    for q in range(1, q_max + 1):
        for p in range(1, p_max + 1):
            if math.gcd(p, q) != 1:
                continue
            total += 1
            val = p / q
            err = abs(val - target) / target
            for t in tol_list:
                if err < t:
                    tallies[t].append((p, q, val, err))
    for t in sorted(tol_list):
        hits = tallies[t]
        label = f"{t*100:.2f}%"
        print(f"    within {label:>8}:  {len(hits):>4d} hits", end="")
        if hits and t == min(tol_list):
            hits.sort(key=lambda x: x[3])
            print("   top: " + ", ".join(f"{p}/{q}={v:.4f}" for p,q,v,_ in hits[:3]))
        else:
            print()


def main() -> None:
    print("=" * 78)
    print("  RIGIDITY TESTS on FTD's rational-integer claims")
    print("=" * 78)
    for label, formula, claim, exp, prec_ppm in CLAIMS:
        err_abs = abs(claim - exp) / exp
        err_ppm = err_abs * 1e6
        precision_tier = "TIGHT" if err_ppm < 1000 else ("LOOSE" if err_ppm < 50000 else "VERY LOOSE")
        print(f"\n  === {label} === ")
        print(f"    formula (FTD)  = {formula}  =  {claim:.6f}")
        print(f"    experimental   =  {exp:.6f}   (exp. precision ~{prec_ppm} ppm)")
        print(f"    FTD err        =  {err_abs:.3e}  =  {err_ppm:.0f} ppm  [{precision_tier}]")
        print(f"    How many rationals p/q with p≤200, q≤60 hit this value?")
        if label.startswith("m_p"):
            # This is a different formula (not a simple rational); skip rigidity
            print(f"    (formula has different shape; skip rigidity scan)")
            continue
        count_hits_within_tolerance(
            exp, [err_abs, err_abs * 2, err_abs * 5, 0.01, 0.05],
            p_max=60, q_max=30
        )
    print()
    print("=" * 78)
    print("  INTERPRETATION")
    print("=" * 78)
    print("""
  For a rational p/q to be a structurally genuine derivation, it should be
  UNIQUE among small rationals at its precision tier. If the FTD ratio hits
  the experimental value within X ppm but many OTHER small rationals also hit
  within X ppm, then the FTD choice is essentially a fit from within a set of
  competitors.

  If FTD is uniquely precise among its family, structural derivation survives.

  Rule of thumb:
    - 0 competitors within same tolerance: STRONG structural claim
    - 1-2 competitors: MARGINALLY structural
    - >5 competitors: FIT dressed as derivation
""")


if __name__ == "__main__":
    main()
