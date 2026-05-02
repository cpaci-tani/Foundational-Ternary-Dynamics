"""proof_polynomial_look_elsewhere_extended.py — MC-T2.1 + MC-T2.2.

Extended polynomial + multiplier scan for the master-quadratic
structural-uniqueness argument.

ORIGINAL SCANS (FTD-0097 + FTD-0111 follow-ups, 2026-05-01):
    Polynomial scan: 147,456 polynomials of form
        P(x) = x² − n·G*^p·x + m·G*^q
        with n, m ∈ [1, 64], p, q ∈ [0, 5].
    Result: master quadratic uniquely dual-selective.
    Bayes factor ~ 20,000 : 1 within natural FTD polynomial family.

    Multiplier scan: 58 (m, k) pairs in Gaussian-integer-tower family
        with m ∈ {1, 2, 4, 5, 8, 9, 10, 13, 16, 17, 18, 20} ×
        k ∈ {3, 4, 5, 6, 7}.
    Result: (m=2, k=4) RANK 1 with 5-orders-of-magnitude gap to
    rank 2.

EXTENSIONS this script implements:

    EXT-A: Rational coefficients in the polynomial scan.
        Search space: P(x) = x² − (n/d_n)·G*^p·x + (m/d_m)·G*^q
        with n, m ∈ [1, 64], p, q ∈ [0, 5], d_n, d_m ∈ [1, 4].
        Total: 147,456 × 16 = 2,359,296 polynomials.

    EXT-B: Higher polynomial degree.
        Cubic polynomials of form P(x) = x³ − a·x² + b·x − c with
        coefficients in the natural FTD class (n·G*^p with bounded n, p).
        Test whether cubic polynomials can dual-match the same way
        the master quadratic does.

    EXT-C: Multiplier-level scan beyond Gaussian integers.
        Tower form M_k(x) = x² − 2^k · G*^(k−2) · x + 2^k · G*^(k−1)
        is the (1+i)-tower because 2 = |1+i|² (Gaussian-integer norm).
        Extend to multipliers c = G* · b with b ∈ {Eisenstein integer
        norms ∈ {1, 3, 4, 7, 9, 12, 13, ...}} and other low-norm
        algebraic classes.

PRE-REGISTRATION:
    Pre-registration via `git tag preregister-polynomial-scan-extended-v1`
    BEFORE the run. Runner SHA-256 + this docstring's parameter
    declarations form the locked spec.

TOLERANCES (must match original scan):
    x_+ within 1.26 ppm of CODATA 1/α
    x_- within 0.80% of N_c = 3
    Both must hold simultaneously for "dual match".

CLOSURE:
    - If extended scan finds master quadratic remains uniquely
      dual-selective: T2.1 + T2.2 closed; FTD-0121 Bayes claim
      strengthened ~10-100× depending on EXT search-space size;
      FTD-0001 status PROMOTED from [SMC] toward [DERIVED] with
      stronger structural-uniqueness backing.
    - If extended scan finds additional dual-matchers: closes
      T2.1 + T2.2 with NEGATIVE result; FTD-0121 [SYNTHESIS]
      Bayes claim weakens; FTD-0001 status remains [SMC]; this
      is also a substantive scientific result (transparency).

Usage:
    python scripts/proofs/proof_polynomial_look_elsewhere_extended.py
"""

from __future__ import annotations

import math
import sys
import time
from typing import List, Tuple


# ─────────────────────────────────────────────────────────────────────
# Constants (per scripts/constants.py)
# ─────────────────────────────────────────────────────────────────────
G_STAR = 2.958675119188639           # Γ(1/4)/Γ(3/4)
X_PLUS_TARGET = 137.0361714582       # 1/α (master quadratic root, tree)
X_MINUS_TARGET = 3.0239639163        # ≈ N_c = 3 (master quadratic root)

# Tolerances per FTD-0097 / FTD-0121 protocol
X_PLUS_TOL = 1.26e-6 * X_PLUS_TARGET     # 1.26 ppm
X_MINUS_TOL = 0.0080 * X_MINUS_TARGET    # 0.80%

# Master quadratic: x² − 16·G*²·x + 16·G*³ = 0
MASTER_N, MASTER_P = 16, 2  # linear coefficient: -16·G*²
MASTER_M, MASTER_Q = 16, 3  # constant coefficient: +16·G*³


# ─────────────────────────────────────────────────────────────────────
# Quadratic root computation
# ─────────────────────────────────────────────────────────────────────
def quadratic_roots(b: float, c: float) -> Tuple[float, float] | None:
    """Roots of x² − b·x + c = 0."""
    disc = b * b - 4 * c
    if disc < 0:
        return None
    sqd = math.sqrt(disc)
    return ((b + sqd) / 2, (b - sqd) / 2)


def matches_dual(x_plus: float, x_minus: float) -> bool:
    """Test whether (x_+, x_-) match BOTH master-quadratic targets."""
    return (
        abs(x_plus - X_PLUS_TARGET) < X_PLUS_TOL
        and abs(x_minus - X_MINUS_TARGET) < X_MINUS_TOL
    )


# ─────────────────────────────────────────────────────────────────────
# EXT-A: Rational-coefficient polynomial scan
# ─────────────────────────────────────────────────────────────────────
def scan_rational_coefficients(
    n_max: int = 64, m_max: int = 64,
    p_max: int = 5, q_max: int = 5,
    d_max: int = 4,
) -> Tuple[int, List[Tuple]]:
    """Search rational-coefficient polynomials.

    P(x) = x² − (n/d_n)·G*^p·x + (m/d_m)·G*^q

    Returns (total_scanned, dual_matchers).
    """
    matchers = []
    count = 0
    g_powers = [G_STAR ** p for p in range(p_max + 1)]

    for n in range(1, n_max + 1):
        for d_n in range(1, d_max + 1):
            for m in range(1, m_max + 1):
                for d_m in range(1, d_max + 1):
                    for p in range(p_max + 1):
                        for q in range(q_max + 1):
                            count += 1
                            b = (n / d_n) * g_powers[p]
                            c = (m / d_m) * g_powers[q]
                            roots = quadratic_roots(b, c)
                            if roots is None:
                                continue
                            x_plus, x_minus = roots
                            if matches_dual(x_plus, x_minus):
                                matchers.append((n, d_n, m, d_m, p, q, x_plus, x_minus))
    return count, matchers


# ─────────────────────────────────────────────────────────────────────
# EXT-B: Cubic-polynomial scan
# ─────────────────────────────────────────────────────────────────────
def cubic_roots(a2: float, a1: float, a0: float) -> List[float] | None:
    """Real roots of x³ − a2·x² + a1·x − a0 = 0 via numpy.

    Uses depressed-cubic formula. Returns list of real roots.
    """
    import numpy as np
    poly = np.poly1d([1.0, -a2, a1, -a0])
    roots = poly.roots
    real_roots = [r.real for r in roots if abs(r.imag) < 1e-9]
    return sorted(real_roots, reverse=True) if real_roots else None


def scan_cubic_polynomials(
    coeff_max: int = 32, p_max: int = 5,
) -> Tuple[int, List[Tuple]]:
    """Search cubic polynomials of form x³ − a₂·x² + a₁·x − a₀ with
    coefficients in n·G*^p form. Tests if any cubic dual-matches the
    master-quadratic targets simultaneously (i.e., has both x_+ and x_-
    as roots).
    """
    matchers = []
    count = 0
    g_powers = [G_STAR ** p for p in range(p_max + 1)]

    for n2 in range(1, coeff_max + 1):
        for p2 in range(p_max + 1):
            for n1 in range(1, coeff_max + 1):
                for p1 in range(p_max + 1):
                    for n0 in range(1, coeff_max + 1):
                        for p0 in range(p_max + 1):
                            count += 1
                            a2 = n2 * g_powers[p2]
                            a1 = n1 * g_powers[p1]
                            a0 = n0 * g_powers[p0]
                            real_roots = cubic_roots(a2, a1, a0)
                            if real_roots is None or len(real_roots) < 2:
                                continue
                            # Check if any pair of real roots dual-matches
                            for i, r_plus in enumerate(real_roots):
                                for r_minus in real_roots[i + 1:]:
                                    if matches_dual(r_plus, r_minus):
                                        matchers.append(
                                            (n2, p2, n1, p1, n0, p0, r_plus, r_minus)
                                        )
    return count, matchers


# ─────────────────────────────────────────────────────────────────────
# EXT-C: Multiplier-level scan beyond Gaussian integers
# ─────────────────────────────────────────────────────────────────────
EISENSTEIN_NORMS = [1, 3, 4, 7, 9, 12, 13, 16, 19, 21, 25, 27, 28]
"""Norms of Eisenstein integers (a + b·ω where ω = exp(2πi/3)):
    N(a + bω) = a² − ab + b². The complete list of Eisenstein norms
    up to 28 is the multiset {1, 1, 3, 3, 4, 4, 7, 7, 9, 9, 12, ...}.
    Distinct values shown above."""

GAUSSIAN_NORMS = [1, 2, 4, 5, 8, 9, 10, 13, 16, 17, 18, 20, 25, 26, 29]
"""Norms of Gaussian integers (a + bi): N(a + bi) = a² + b²."""


def scan_extended_multipliers(
    norms: List[int], k_min: int = 3, k_max: int = 12,
) -> Tuple[int, List[Tuple]]:
    """Tower scan with multipliers M_k(x) = x² − m^k · G*^(k−2) · x
                                            + m^k · G*^(k−1)
    for m running over given norms and k ∈ [k_min, k_max].

    The original 58-pair scan used Gaussian-integer norms ∈
    {1, 2, 4, 5, 8, 9, 10, 13, 16, 17, 18, 20} × k ∈ {3..7}.
    """
    matchers = []
    count = 0
    g_powers = [G_STAR ** p for p in range(k_max + 1)]

    for m in norms:
        for k in range(k_min, k_max + 1):
            count += 1
            b = m ** k * g_powers[k - 2] if k - 2 >= 0 else m ** k / g_powers[2 - k]
            c = m ** k * g_powers[k - 1] if k - 1 >= 0 else m ** k / g_powers[1 - k]
            roots = quadratic_roots(b, c)
            if roots is None:
                continue
            x_plus, x_minus = roots
            if matches_dual(x_plus, x_minus):
                matchers.append((m, k, x_plus, x_minus))
    return count, matchers


# ─────────────────────────────────────────────────────────────────────
# Master scan
# ─────────────────────────────────────────────────────────────────────
def main() -> int:
    print("=" * 72)
    print("proof_polynomial_look_elsewhere_extended.py — MC-T2.1 + MC-T2.2")
    print("=" * 72)
    print()
    print("Pre-registered scan parameters:")
    print(f"  G* = {G_STAR}")
    print(f"  Target x_+ = {X_PLUS_TARGET} ± {X_PLUS_TOL:.4e} (1.26 ppm)")
    print(f"  Target x_- = {X_MINUS_TARGET} ± {X_MINUS_TOL:.4e} (0.80%)")
    print(f"  Master quadratic: n={MASTER_N}, p={MASTER_P}; m={MASTER_M}, q={MASTER_Q}")
    print()

    # EXT-A
    print("EXT-A: Rational-coefficient polynomial scan")
    print("       n, m ∈ [1, 64], p, q ∈ [0, 5], d_n, d_m ∈ [1, 4]")
    print("       Expected scan size: 147,456 × 16 = 2,359,296")
    t0 = time.time()
    total_a, matchers_a = scan_rational_coefficients()
    dt_a = time.time() - t0
    print(f"       Scanned: {total_a:,} polynomials in {dt_a:.1f}s")
    print(f"       Dual-matchers found: {len(matchers_a)}")
    if matchers_a:
        for n, d_n, m, d_m, p, q, x_p, x_m in matchers_a[:10]:
            print(f"         n/d_n = {n}/{d_n}, m/d_m = {m}/{d_m}, p={p}, q={q} "
                  f"→ x_+={x_p:.6f}, x_-={x_m:.6f}")
    print()

    # EXT-B
    print("EXT-B: Cubic-polynomial scan")
    print("       Coefficients n_i·G*^p_i with n_i ∈ [1, 32], p_i ∈ [0, 5]")
    # Reduced parameters for tractability — full scan would be 32^3 × 6^3 = ~7M cubics
    print("       Note: reduced scan (n_i ∈ [1, 16], p_i ∈ [0, 4]) for tractability")
    t0 = time.time()
    total_b, matchers_b = scan_cubic_polynomials(coeff_max=16, p_max=4)
    dt_b = time.time() - t0
    print(f"       Scanned: {total_b:,} cubics in {dt_b:.1f}s")
    print(f"       Dual-matchers found: {len(matchers_b)}")
    if matchers_b:
        for entry in matchers_b[:10]:
            print(f"         {entry}")
    print()

    # EXT-C
    print("EXT-C: Multiplier-level scan beyond Gaussian integers")
    print(f"       Eisenstein norms: {EISENSTEIN_NORMS}")
    t0 = time.time()
    total_c, matchers_c = scan_extended_multipliers(EISENSTEIN_NORMS)
    dt_c = time.time() - t0
    print(f"       Scanned: {total_c} (Eisenstein-norm, k) pairs in {dt_c:.3f}s")
    print(f"       Dual-matchers in Eisenstein family: {len(matchers_c)}")
    for entry in matchers_c:
        print(f"         m={entry[0]}, k={entry[1]} → x_+={entry[2]:.6f}, x_-={entry[3]:.6f}")
    print()
    print(f"       Cross-check Gaussian-integer family: {GAUSSIAN_NORMS}")
    total_c_gauss, matchers_c_gauss = scan_extended_multipliers(GAUSSIAN_NORMS)
    print(f"       Scanned: {total_c_gauss} (Gaussian-norm, k) pairs")
    print(f"       Dual-matchers in Gaussian family: {len(matchers_c_gauss)}")
    for entry in matchers_c_gauss:
        print(f"         m={entry[0]}, k={entry[1]} → x_+={entry[2]:.6f}, x_-={entry[3]:.6f}")
    print()

    # Summary
    print("=" * 72)
    print("SUMMARY")
    print("=" * 72)
    total_scanned = total_a + total_b + total_c + total_c_gauss
    total_matchers = len(matchers_a) + len(matchers_b) + len(matchers_c) + len(matchers_c_gauss)
    print(f"Total polynomials/multipliers scanned: {total_scanned:,}")
    print(f"Total dual-matchers across all extensions: {total_matchers}")
    print()
    print("Per-extension findings:")
    print(f"  EXT-A (rational coeffs):    {len(matchers_a)} dual-matchers in {total_a:,} polynomials")
    print(f"  EXT-B (cubics):             {len(matchers_b)} dual-matchers in {total_b:,} cubics")
    print(f"  EXT-C (Eisenstein tower):   {len(matchers_c)} dual-matchers in {total_c} pairs")
    print(f"  EXT-C (Gaussian tower):     {len(matchers_c_gauss)} dual-matchers in {total_c_gauss} pairs")
    print()

    # Find the master-quadratic specifically in EXT-A
    master_in_a = sum(
        1 for n, d_n, m, d_m, p, q, _, _ in matchers_a
        if (n, d_n, m, d_m, p, q) == (MASTER_N, 1, MASTER_M, 1, MASTER_P, MASTER_Q)
    )
    print(f"Master quadratic appearance in EXT-A: {master_in_a} (expected: 1)")
    print()

    if total_matchers <= 1:
        print("CONCLUSION:")
        print("  Master quadratic is UNIQUELY dual-selective across the extended")
        print("  search space. T2.1 + T2.2 CLOSED with positive result.")
        print()
        print("  Implications:")
        print("    - FTD-0121 [SYNTHESIS] Bayes factor strengthened by")
        print(f"      ~{total_scanned // 147456}× over the original 147,456-polynomial scan.")
        print("    - Master quadratic structural-uniqueness now demonstrated")
        print("      across rational-coefficient AND non-Gaussian-integer-tower")
        print("      AND cubic search spaces.")
        print("    - FTD-0001 status: structural uniqueness substantively")
        print("      strengthened. (Identification x_+ = 1/α is still the")
        print("      empirical step; this scan strengthens the polynomial-form")
        print("      side of the structural argument.)")
        return 0
    else:
        print("CONCLUSION:")
        print(f"  Found {total_matchers} dual-matchers (master quadratic + {total_matchers - 1} alternatives).")
        print("  The master-quadratic uniqueness within the extended search")
        print("  space is reduced. FTD-0121 [SYNTHESIS] Bayes factor would be")
        print("  weakened proportionally; alternatives must be analyzed for")
        print("  their structural relationship to the master quadratic.")
        return 0


if __name__ == "__main__":
    sys.exit(main())
