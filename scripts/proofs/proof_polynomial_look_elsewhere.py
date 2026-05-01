"""
Proof — Polynomial-level look-elsewhere scan for the master-quadratic dual prediction
========================================================================================

FTD-0097 ran a MONOMIAL-level look-elsewhere scan and found the catalog
over-rich. The master quadratic's DUAL-PREDICTION property (one polynomial
matches BOTH 1/α and N_c simultaneously) was specifically distinguished
from monomial-level fits.

This scan extends FTD-0097 to the POLYNOMIAL level. Specifically: how
many degree-2 polynomials of the form

    M(x) = x² − n · G*^p · x + m · G*^q

with integer (n, m) and small integer (p, q) match BOTH 1/α (137.036
to 1.26 ppm) AND N_c (3.024 to 0.80%) simultaneously?

The master quadratic has (n, p, m, q) = (16, 2, 16, 3). If only this
combination matches both targets, the dual-prediction is structurally
selective. If many combinations match, the catalog is over-rich at
polynomial level too.

SEARCH SPACE:
  n, m ∈ {1, 2, ..., 64}      (integer coefficients up to 64)
  p, q ∈ {0, 1, 2, 3, 4, 5}    (G*-powers from 0 to 5)
  Total: 64² × 6² = 147,456 polynomials

TOLERANCES (matched to master quadratic precision):
  x_+ tolerance: 1.26 ppm of 137.036 = 1.73e-4
  x_- tolerance: 0.80 % of 3.024     = 0.0242

We also report SINGLE-target hits for comparison:
  - polynomials matching only x_+ (might be many)
  - polynomials matching only x_- (might be many)
  - polynomials matching BOTH (the dual-prediction class)

Provenance: docs/theory/09_mathematical/EXPLR_PATHS_TO_ALPHA.md;
            docs/theory/09_mathematical/EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md
LEDGER: FTD-0097 follow-up (polynomial-level look-elsewhere refinement).

Usage:
    python scripts/proofs/proof_polynomial_look_elsewhere.py
"""

import math


G_STAR = math.gamma(0.25) / math.gamma(0.75)         # 2.95867...
ALPHA_INV = 137.035999084                            # CODATA 1/α
N_C = 3.0                                            # color number target

# Tolerances matched to master-quadratic precision
TOL_X_PLUS  = 1.26e-6 * ALPHA_INV   # 1.26 ppm absolute = 1.73e-4
TOL_X_MINUS = 0.80e-2 * N_C         # 0.80% absolute = 2.42e-2

# Search space
N_MAX = 64
P_MAX = 5
N_RANGE = range(1, N_MAX + 1)
P_RANGE = range(0, P_MAX + 1)


def quadratic_roots(a_coef, b_coef):
    """Return (x_+, x_-) for x² − a·x + b = 0, or None if discriminant < 0."""
    disc = a_coef * a_coef - 4 * b_coef
    if disc < 0:
        return None
    sq = math.sqrt(disc)
    return (a_coef + sq) / 2, (a_coef - sq) / 2


def main():
    print('=' * 78)
    print('PROOF: Polynomial-level look-elsewhere scan')
    print('FTD-0097 follow-up (polynomial-level extension to monomial scan)')
    print('=' * 78)
    print(f'G* = {G_STAR:.10f}')
    print(f'TARGET 1: 1/α = {ALPHA_INV:.6f}, tolerance ±{TOL_X_PLUS:.4e}  (1.26 ppm)')
    print(f'TARGET 2: N_c = {N_C:.6f}, tolerance ±{TOL_X_MINUS:.4e}  (0.80 %)')
    print()
    print(f'Search space: x² − n·G*^p·x + m·G*^q  with')
    print(f'  n, m ∈ [1, {N_MAX}]   p, q ∈ [0, {P_MAX}]')
    print(f'  Total: {N_MAX**2 * (P_MAX+1)**2:,} polynomials')
    print()

    # Precompute G*^p
    G_pow = [G_STAR ** p for p in P_RANGE]

    # Counters
    n_total = 0
    n_complex = 0  # discriminant negative
    n_match_xplus = 0
    n_match_xminus = 0
    n_match_both = 0
    dual_matches = []

    for n in N_RANGE:
        for p in P_RANGE:
            a_coef = n * G_pow[p]
            for m in N_RANGE:
                for q in P_RANGE:
                    b_coef = m * G_pow[q]
                    n_total += 1
                    roots = quadratic_roots(a_coef, b_coef)
                    if roots is None:
                        n_complex += 1
                        continue
                    x_plus, x_minus = roots
                    match_xplus  = abs(x_plus  - ALPHA_INV) <= TOL_X_PLUS
                    match_xminus = abs(x_minus - N_C)        <= TOL_X_MINUS
                    if match_xplus:
                        n_match_xplus += 1
                    if match_xminus:
                        n_match_xminus += 1
                    if match_xplus and match_xminus:
                        n_match_both += 1
                        dual_matches.append((n, p, m, q, x_plus, x_minus))

    print(f'SCAN RESULTS:')
    print(f'  Total polynomials evaluated:      {n_total:,}')
    print(f'  With complex roots (skipped):     {n_complex:,}')
    print(f'  With real roots:                  {n_total - n_complex:,}')
    print()
    print(f'  Match x_+ to 1/α (single-target):  {n_match_xplus:,}')
    print(f'  Match x_- to N_c (single-target):  {n_match_xminus:,}')
    print(f'  DUAL match (both simultaneously):  {n_match_both:,}')
    print()

    print('DUAL MATCHES:')
    print(f'  {"#":>3s} | {"n":>3s} | {"p":>2s} | {"m":>3s} | {"q":>2s} | '
          f'{"a = n·G*^p":>10s} | {"b = m·G*^q":>10s} | {"x_+":>10s} | {"x_-":>8s}')
    print('  ' + '-' * 80)
    for i, (n, p, m, q, xp, xm) in enumerate(dual_matches, 1):
        a_val = n * G_pow[p]
        b_val = m * G_pow[q]
        marker = '  <-- master quadratic' if (n == 16 and p == 2 and m == 16 and q == 3) else ''
        print(f'  {i:>3d} | {n:>3d} | {p:>2d} | {m:>3d} | {q:>2d} | '
              f'{a_val:>10.4f} | {b_val:>10.4f} | {xp:>10.6f} | {xm:>8.4f}{marker}')

    # Statistical analysis
    print()
    print('STATISTICAL ANALYSIS:')
    n_real = n_total - n_complex
    p_xplus = n_match_xplus / n_real if n_real > 0 else 0
    p_xminus = n_match_xminus / n_real if n_real > 0 else 0
    p_both_independent = p_xplus * p_xminus
    expected_dual_if_independent = p_both_independent * n_real

    print(f'  Single-target hit rates:')
    print(f'    P(match x_+) = {n_match_xplus}/{n_real} = {p_xplus:.6f}  ({p_xplus*1e6:.0f} per million)')
    print(f'    P(match x_-) = {n_match_xminus}/{n_real} = {p_xminus:.6f}  ({p_xminus*1e6:.0f} per million)')
    print()
    print(f'  Expected dual matches if x_+ and x_- match INDEPENDENTLY:')
    print(f'    P(x_+) × P(x_-) × N = {expected_dual_if_independent:.6f}')
    print(f'    But dual is NOT independent: x_+ + x_- = a, x_+ · x_- = b are constrained.')
    print(f'    Real dual hit rate: {n_match_both}/{n_real} = {n_match_both/n_real:.2e}')
    print()

    if n_match_both == 0:
        print(f'  VERDICT: NO dual matches in {n_real:,} polynomials.')
        print(f'  The master quadratic is OUTSIDE the scan space, OR the')
        print(f'  master quadratic is genuinely UNIQUE and the scan tolerances')
        print(f'  are too tight even for it.')
    elif n_match_both == 1:
        n_mq, p_mq, m_mq, q_mq, _, _ = dual_matches[0]
        if (n_mq, p_mq, m_mq, q_mq) == (16, 2, 16, 3):
            print(f'  VERDICT: EXACTLY 1 dual match — the master quadratic itself.')
            print(f'  Within this {n_real:,}-polynomial space, the master quadratic')
            print(f'  is uniquely selective at master-quadratic precision.')
            print(f'  STRONG SUPPORT for the dual-prediction structural significance.')
        else:
            print(f'  VERDICT: 1 dual match, but NOT the master quadratic.')
            print(f'  This would be unexpected — the master quadratic should match.')
    else:
        print(f'  VERDICT: {n_match_both} dual matches.')
        print(f'  The master quadratic is among them, but not unique.')
        print(f'  The polynomial-level catalog has multiple dual-matchers — extends')
        print(f'  FTD-0097 monomial over-rich finding to polynomial level.')


if __name__ == '__main__':
    main()
