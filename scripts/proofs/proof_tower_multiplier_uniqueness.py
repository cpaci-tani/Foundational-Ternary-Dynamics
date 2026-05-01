"""
Proof — (1+i)-tower multiplier uniqueness scan
================================================

FTD-0111 (Theorem 8) introduces the (1+i)-tower of master quadratics:
    M_k(x) = x² − 2^k · G*^(k−2) · x + 2^k · G*^(k−1)
At level k=4, the larger root x_+ ≈ 137.036, conjecturally = 1/α.

The factor 2 = |1+i|² is the smallest non-trivial Gaussian-integer norm.
The level k=4 = N_base is the number of O_h orbits in the 27-block.

Both selections have structural justification (CM ring = Z[i] from
Theorem 3; N_base from O_h representation theory). But is the COMBINATION
(m=2, k=4) UNIQUE in giving x_+ ≈ 137.036, or are there other (m, k)
pairs that come close?

This script scans:
  - Gaussian-integer multipliers a + bi with a²+b² ∈ {1, 2, 4, 5, 8, 9, 10, ...}
  - Levels k ∈ {3, 4, 5, 6, 7}
For each (m=a²+b², k):
    M_{m,k}(x) = x² − m^k · G*^(k−2) · x + m^k · G*^(k−1)
    x_+ = (m^k · G*^(k−2) + √(m^(2k) · G*^(2k−4) − 4 · m^k · G*^(k−1))) / 2

The harmonic invariant 1/y_+ + 1/y_- = 1 (with y = x/G*) holds
generically: y_+ + y_- = m^k · G*^(k−3), y_+·y_- = m^k · G*^(k−4),
so 1/y_+ + 1/y_- = (y_+ + y_-)/(y_+·y_-) = G*. Wait, this gives G*, not 1.

Let me recompute. x_+ + x_- = c_k = m^k · G*^(k−2). x_+·x_- = m^k · G*^(k−1).
y_± = x_±/G*. y_+ + y_- = (m^k · G*^(k−2))/G* = m^k · G*^(k−3).
y_+·y_- = (m^k · G*^(k−1))/G*² = m^k · G*^(k−3).

So 1/y_+ + 1/y_- = (y_+ + y_-)/(y_+·y_-) = (m^k · G*^(k−3))/(m^k · G*^(k−3)) = 1. ✓

The harmonic invariant holds for ALL (m, k) with this normalization.
This is a generic feature of the tower, not a (1+i)-specific property.

What IS (1+i)-specific is the actual root values at each level. We test
which (m, k) gives x_+ closest to 137.036 (= 1/α empirical).

Provenance: docs/theory/03_derivations/THEOREM_HARMONIC_INVARIANT_TOWER.md
LEDGER: FTD-0111 follow-up (uniqueness sub-question Q1 from FTD-0111).

Usage:
    python scripts/proofs/proof_tower_multiplier_uniqueness.py
"""

import math
from itertools import product

import numpy as np


# ---------------------------------------------------------------------------
# G* canonical (Gamma(1/4)/Gamma(3/4))
# ---------------------------------------------------------------------------
G_STAR = math.gamma(0.25) / math.gamma(0.75)   # 2.9586751192...
ALPHA_INV_CODATA = 137.035999084               # CODATA 2022


# ---------------------------------------------------------------------------
# Tower roots at given (m, k)
# ---------------------------------------------------------------------------

def tower_roots(m, k, G=G_STAR):
    """For multiplier with norm m and level k, compute roots of:
        M_{m,k}(x) = x² − m^k · G^(k−2) · x + m^k · G^(k−1) = 0
    Returns (x_+, x_-) or None if discriminant < 0.
    """
    c1 = m ** k * G ** (k - 2)
    c0 = m ** k * G ** (k - 1)
    disc = c1 ** 2 - 4 * c0
    if disc < 0:
        return None
    sq = math.sqrt(disc)
    x_plus = (c1 + sq) / 2
    x_minus = (c1 - sq) / 2
    return x_plus, x_minus


# ---------------------------------------------------------------------------
# Gaussian-integer norms (smallest representatives)
# ---------------------------------------------------------------------------

def gaussian_norms(max_norm=20):
    """Distinct Gaussian-integer norms a²+b² ≤ max_norm with at least one
    nonzero (a, b) pair. Returns sorted list with sample (a, b) representative.
    """
    norms = {}
    for a in range(-int(math.sqrt(max_norm)) - 1, int(math.sqrt(max_norm)) + 2):
        for b in range(-int(math.sqrt(max_norm)) - 1, int(math.sqrt(max_norm)) + 2):
            n = a * a + b * b
            if 0 < n <= max_norm and n not in norms:
                norms[n] = (a, b)
    return sorted(norms.items())


# ---------------------------------------------------------------------------
# Main scan
# ---------------------------------------------------------------------------

def main():
    print('=' * 78)
    print('PROOF: (1+i)-tower multiplier uniqueness scan (FTD-0111 follow-up Q1)')
    print('=' * 78)
    print(f'G* = Γ(1/4)/Γ(3/4) = {G_STAR:.10f}')
    print(f'CODATA 1/α        = {ALPHA_INV_CODATA:.6f}')
    print()
    print('Scanning Gaussian-integer norms m and tower levels k ∈ {3, 4, 5, 6, 7}:')
    print()

    norms = gaussian_norms(max_norm=20)
    levels = [3, 4, 5, 6, 7]

    # Collect all (m, k, x_+) entries
    entries = []
    for m_norm, (a, b) in norms:
        for k in levels:
            roots = tower_roots(m_norm, k)
            if roots is None:
                continue
            x_plus, x_minus = roots
            entries.append((m_norm, (a, b), k, x_plus, x_minus))

    # Header
    print(f'  {"m=|a+bi|²":>10s} | {"(a,b)":>9s} | {"k":>2s} | '
          f'{"x_+":>14s} | {"x_-":>10s} | {"|x_+ - 137.036|":>16s} | {"rel ppm":>10s}')
    print('  ' + '-' * 95)

    # Sort by |x_+ - 137.036| ascending
    sorted_entries = sorted(entries, key=lambda e: abs(e[3] - ALPHA_INV_CODATA))
    for m_norm, (a, b), k, x_plus, x_minus in sorted_entries[:25]:
        diff = abs(x_plus - ALPHA_INV_CODATA)
        rel_ppm = diff / ALPHA_INV_CODATA * 1e6
        marker = '  <-- (1+i, k=4) [master quadratic]' if (m_norm == 2 and k == 4) else ''
        print(f'  {m_norm:>10d} | {f"({a},{b})":>9s} | {k:>2d} | '
              f'{x_plus:>14.6f} | {x_minus:>10.6f} | {diff:>16.6e} | {rel_ppm:>10.2f}{marker}')

    print()
    print('STRUCTURAL OBSERVATIONS:')
    # Find rank of (m=2, k=4)
    target_idx = None
    for i, (m_norm, (a, b), k, x_plus, x_minus) in enumerate(sorted_entries):
        if m_norm == 2 and k == 4:
            target_idx = i
            break
    if target_idx is not None:
        print(f'  (1+i, k=4) ranks #{target_idx + 1} of {len(sorted_entries)} (m, k) pairs')
        print(f'  by closeness of x_+ to 137.036.')
    print()

    # Specifically check (m=2, k=4) and nearby alternatives
    print('SPECIFIC COMPARISON: (1+i, k=4) vs neighbors')
    print(f'  {"(m, k)":>10s} | {"x_+":>14s} | {"|Δ| ppm":>12s}')
    print('  ' + '-' * 45)
    candidates = [
        (2, 4),    # (1+i)-tower at master quadratic
        (1, 4),    # trivial
        (4, 4),    # (2)-tower
        (5, 4),    # (2+i)-tower
        (2, 3),    # (1+i)-tower at k=3
        (2, 5),    # (1+i)-tower at k=5
        (8, 4),    # (2+2i)-tower
        (9, 4),    # (3)-tower
        (10, 4),   # (3+i)-tower
        (13, 4),   # (3+2i) or (2+3i)-tower
        (16, 4),   # (4)-tower
    ]
    for m, k in candidates:
        roots = tower_roots(m, k)
        if roots is None:
            print(f'  ({m}, {k}) | (no real roots)')
            continue
        x_plus = roots[0]
        diff = abs(x_plus - ALPHA_INV_CODATA)
        rel = diff / ALPHA_INV_CODATA * 1e6
        marker = '  <-- master quadratic' if (m == 2 and k == 4) else ''
        print(f'  ({m:>2d}, {k}) | {x_plus:>14.6f} | {rel:>12.4f}{marker}')

    print()
    print('VERDICT:')
    print(f'  Among {len(sorted_entries)} (m, k) pairs scanned, (m=2, k=4) is at rank '
          f'{target_idx + 1 if target_idx is not None else "???"} for closeness to 137.036.')
    print()
    print('STRUCTURAL JUSTIFICATIONS:')
    print('  m = 2 = |1+i|² selected by:')
    print('    (a) Z[i] = CM ring of E: y² = x³ - x (Theorem 3)')
    print('    (b) Smallest non-trivial Gaussian integer norm')
    print('  k = 4 selected by:')
    print('    (a) k = N_base = 4 = number of O_h orbits in 27-block (Theorem 4 connection)')
    print('    (b) k = 4 = mult(A_{1g}) of O_h on 27-block')
    print()
    print('Both selections are structural; their COMBINATION yields x_+ ≈ 137.036.')


if __name__ == '__main__':
    main()
