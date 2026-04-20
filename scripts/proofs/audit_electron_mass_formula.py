#!/usr/bin/env python3
"""
audit_electron_mass_formula.py — Phase I Option 4 sub-audit.

FTD's headline mass formula (catalogued as [DERIVED]):

    m_e = m_P · √(2π) · (16/3) · α^11

claims to derive the electron mass from the Planck mass m_P, the fine-
structure constant α, and the Moore-neighborhood integers {N_base=4,
N_eff=13, N_c=3}. The "(16/3)" factor is presented as a structural
coefficient; the 11th power of α is where it gets suspicious.

This script does a rigidity test: among integer exponents n ∈ [5, 15]
and rational prefactors p/q with small p, q, how many combinations of
the form

    m_candidate = m_P · √(2π) · (p/q) · α^n

match the measured m_e = 0.51099895 MeV within various tolerances?
If many combinations hit, the claim is a fit; if only (p/q, n) =
(16/3, 11) hits, it is structurally forced.
"""
from __future__ import annotations
import math
from fractions import Fraction

# Experimental values (CODATA 2022, SI/natural):
M_PLANCK_GEV = 1.220890e19        # GeV
M_E_MEV      = 0.51099895069      # MeV
M_E_GEV      = M_E_MEV * 1e-3
ALPHA        = 1.0 / 137.035999177

SQRT_2PI = math.sqrt(2.0 * math.pi)


def candidate_mass(coeff: float, n: int) -> float:
    """m_candidate / m_P at exponent n and prefactor coeff, in GeV."""
    return M_PLANCK_GEV * SQRT_2PI * coeff * (ALPHA ** n)


def rel_err(m: float) -> float:
    return abs(m - M_E_GEV) / M_E_GEV


def main() -> None:
    print("=" * 78)
    print("  RIGIDITY TEST:  m_e = m_P · √(2π) · (p/q) · α^n")
    print("=" * 78)
    target = M_E_GEV
    print(f"  m_e (CODATA) = {M_E_GEV:.6e} GeV")
    print(f"  m_P          = {M_PLANCK_GEV:.4e} GeV")
    print(f"  α            = {ALPHA:.10e}")
    print(f"  sqrt(2π)     = {SQRT_2PI:.6f}")

    # FTD's claim: coeff = 16/3, n = 11
    ftd_coeff = 16.0 / 3.0
    ftd_n = 11
    ftd_mass = candidate_mass(ftd_coeff, ftd_n)
    print(f"\n  FTD claim (16/3, n=11):  m = {ftd_mass:.6e} GeV")
    print(f"    rel err vs m_e       = {rel_err(ftd_mass):.3e}  ({rel_err(ftd_mass)*100:.3f}%)")

    # For each integer n, solve for the "exact" prefactor that hits m_e
    # Then check if it's close to a small rational.
    print(f"\n  For each integer n, the 'exact' prefactor that hits m_e:")
    print(f"  {'n':>3} {'coeff_exact':>15} {'nearest small p/q (p,q≤30)':>35} {'rel err':>12}")
    print("  " + "-" * 70)
    best_n_by_precision = []
    for n in range(5, 18):
        coeff_exact = M_E_GEV / (M_PLANCK_GEV * SQRT_2PI * (ALPHA ** n))
        # Find nearest rational p/q with p, q <= 100
        best_pq = None
        best_err = math.inf
        for q in range(1, 101):
            p_raw = coeff_exact * q
            for p in [int(math.floor(p_raw)), int(math.ceil(p_raw)), round(p_raw)]:
                if p <= 0 or p > 10000:
                    continue
                candidate_coeff = p / q
                err = abs(candidate_coeff - coeff_exact) / coeff_exact
                if err < best_err:
                    best_err = err
                    best_pq = (p, q, candidate_coeff)
        if best_pq is None:
            # Skip — exact coefficient outside any reasonable rational range
            print(f"  {n:>3d} {coeff_exact:>15.6e}  (no small p/q found)")
            continue
        p, q, cc = best_pq
        m = candidate_mass(cc, n)
        rel = rel_err(m)
        marker = "  <-- FTD" if (p, q, n) == (16, 3, 11) else ""
        print(f"  {n:>3d} {coeff_exact:>15.6e} {p:>8d} / {q:<6d}  "
              f"{rel:>12.3e}{marker}")
        best_n_by_precision.append((n, best_err, (p, q), cc, rel))

    print()
    print("  Ranked by rel_err (tight rationals that hit m_e):")
    best_n_by_precision.sort(key=lambda t: t[4])
    print(f"  {'rank':>4} {'n':>3} {'p/q':>12} {'rel err':>12}")
    for i, (n, rerr, pq, cc, rel) in enumerate(best_n_by_precision[:8]):
        marker = "  <-- FTD" if pq == (16, 3) and n == 11 else ""
        print(f"  {i+1:>4d} {n:>3d} {pq[0]:>5d}/{pq[1]:<5d} {rel:>12.3e}{marker}")

    # Now the key test: does (16/3, n=11) have unusually low error
    # compared to nearby (p, q, n) combinations?
    print()
    print("=" * 78)
    print("  Full small-coefficient scan: (p/q, n) with p, q ≤ 50, n ∈ [8, 14]")
    print("=" * 78)
    hits_within: dict[float, list] = {1e-3: [], 1e-2: [], 0.05: []}
    total = 0
    for n in range(8, 15):
        for p in range(1, 51):
            for q in range(1, 31):
                if math.gcd(p, q) != 1:
                    continue
                total += 1
                m = candidate_mass(p / q, n)
                err = rel_err(m)
                for cutoff in hits_within:
                    if err < cutoff:
                        hits_within[cutoff].append((p, q, n, m, err))

    print(f"\n  Scanned {total} coprime (p, q, n) triples.\n")
    for cutoff in sorted(hits_within):
        hits = hits_within[cutoff]
        label = {1e-3: "< 0.1% (1000 ppm)",
                 1e-2: "< 1%",
                 0.05: "< 5%"}[cutoff]
        print(f"  Within {label}: {len(hits)} combinations")
        hits.sort(key=lambda t: t[4])
        for (p, q, n, m, err) in hits[:10]:
            marker = "  <-- FTD" if (p, q, n) == (16, 3, 11) else ""
            print(f"    p/q = {p:>3d}/{q:<3d}, n = {n}:  m = {m:.4e}, "
                  f"err = {err:.3e}{marker}")
        if len(hits) > 10:
            print(f"    ... ({len(hits) - 10} more)")
        print()

    # Scan ONLY n=11, let p,q float -- how many rationals hit within 1000 ppm?
    print("  For n=11 fixed (the FTD exponent), how many p/q hit < 1000 ppm?")
    hits_n11 = []
    for p in range(1, 51):
        for q in range(1, 31):
            if math.gcd(p, q) != 1:
                continue
            m = candidate_mass(p / q, 11)
            err = rel_err(m)
            if err < 1e-3:
                hits_n11.append((p, q, err))
    hits_n11.sort(key=lambda t: t[2])
    print(f"    Hits within 1000 ppm at n=11: {len(hits_n11)}")
    for p, q, err in hits_n11:
        marker = "  <-- FTD" if (p, q) == (16, 3) else ""
        print(f"      p/q = {p:>3d}/{q:<3d}:  err = {err:.3e}{marker}")


if __name__ == "__main__":
    main()
