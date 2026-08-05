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


def cubic_is_master_embedding(n2: int, p2: int, n1: int, p1: int,
                              n0: int, p0: int, rel_tol: float = 1e-9) -> bool:
    """Is x³ − a₂x² + a₁x − a₀ exactly divisible by the master quadratic?

    [ADDED 2026-08-04] The EXT-B result was previously reported as
    "0 genuinely-new cubic dual-matchers (all are master quadratic × linear
    factor; not independent)" — but that was a hardcoded literal in a print
    statement, and `matchers_b` was never added to `genuine_unique`. No code
    tested the claim, and it is false: the four EXT-B matchers are
    x·P_master(x) − a₀ with a₀ ∈ {1, G*, 2, 3}, which leave a nonzero
    remainder. This function performs the test that was asserted.

    Synthetic division of (1, −a₂, a₁, −a₀) by (1, −16G*², 16G*³).
    """
    a2 = n2 * (G_STAR ** p2)
    a1 = n1 * (G_STAR ** p1)
    a0 = n0 * (G_STAR ** p0)
    b1, b0 = -MASTER_N * G_STAR ** MASTER_P, MASTER_M * G_STAR ** MASTER_Q
    c2, c1, c0 = -a2, a1, -a0
    q0 = c2 - b1                      # quotient is x + q0
    r1 = c1 - b0 - q0 * b1            # remainder r1·x + r0
    r0 = c0 - q0 * b0
    scale = max(1.0, abs(a2), abs(a1), abs(a0))
    return abs(r1) / scale < rel_tol and abs(r0) / scale < rel_tol


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
    # The report below uses ∈, ×, ≤ and →. On a cp1252 console (Windows
    # default) printing them raised UnicodeEncodeError at the first EXT-A
    # header, so this runner aborted before scanning anything. Fixed
    # 2026-08-04; matches the pattern already used in scripts/verification.
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")

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

    # ─────────────────────────────────────────────────────────────────
    # Classify matchers by structural equivalence class
    # ─────────────────────────────────────────────────────────────────
    print("STRUCTURAL CLASSIFICATION OF MATCHERS")
    print()

    # EXT-A: filter for unique (n/d_n, m/d_m) ratio + (p, q). Fraction-
    # equivalent representations of the master quadratic do not count
    # as new dual-matchers.
    unique_a = set()
    for n, d_n, m, d_m, p, q, _, _ in matchers_a:
        # Reduce n/d_n and m/d_m to lowest terms
        from math import gcd
        gn, gm = gcd(n, d_n), gcd(m, d_m)
        unique_a.add((n // gn, d_n // gn, m // gm, d_m // gm, p, q))
    print(f"  EXT-A unique (after reducing fraction redundancy): {len(unique_a)}")
    for entry in unique_a:
        print(f"    (n, d_n, m, d_m, p, q) = {entry}")

    # EXT-B: cubic dual-matchers are the master quadratic × (x − r) for
    # some small r. Verify by checking that the third root is close to
    # 1 / (linear coeff) · (constant coeff)^(-1) ≈ 1/16G*³ ≈ 0.0024.
    print()
    # [FIX 2026-08-04] this block previously asserted "Each cubic factorizes
    # as master_quadratic × (x − r) ... NOT independent dual-matchers".
    # That is false, and self-inconsistent: matching the constant term needs
    # r = a_0/(16G*³), while matching the x² term needs r = 0. Both hold only
    # for a_0 = 0, and the scan starts at n_0 = 1. The cubics are
    # x·P_master(x) − a_0, which leaves a nonzero remainder; the third root is
    # only APPROXIMATELY a_0/(16G*³), and the other two roots are perturbed
    # correspondingly (that is why x_+ ≠ 137.0361715 for all four).
    print(f"  EXT-B cubics: {len(matchers_b)} found — form x·P_master(x) − a_0.")
    print(f"    Third root ≈ a_0/(16 G*³) ≈ {1.0 / (16 * G_STAR ** 3):.4f} for a_0=1,")
    print(f"    but the factorization is APPROXIMATE, not exact: matching the")
    print(f"    constant term needs r = a_0/(16G*³) while the x² term needs")
    print(f"    r = 0. Divisibility is tested per-cubic below.")

    # EXT-C: filter master quadratic out of Gaussian family
    print()
    nontrivial_c = [
        (m, k, x_p, x_m) for m, k, x_p, x_m in matchers_c_gauss
        if not (m == 2 and k == 4)  # master quadratic is at (m=2, k=4)
    ]
    print(f"  EXT-C Gaussian non-master-quadratic: {len(nontrivial_c)}")
    print(f"  EXT-C Eisenstein non-master-quadratic: {len(matchers_c)}")
    print()

    # ─────────────────────────────────────────────────────────────────
    # Genuine count
    # ─────────────────────────────────────────────────────────────────
    # [FIX 2026-08-04] EXT-B was excluded from this count by a hardcoded
    # literal, never by a test — and the asserted reason ("all are master
    # quadratic × linear factor") is false. Now computed.
    ext_b_embeddings = [e for e in matchers_b if cubic_is_master_embedding(*e[:6])]
    ext_b_genuine = [e for e in matchers_b if not cubic_is_master_embedding(*e[:6])]

    genuine_unique = (
        len({tuple(e[4:]) for e in unique_a})  # distinct (p, q) in EXT-A — should be 1 (= master quadratic)
        + len(ext_b_genuine)
        + len(nontrivial_c)
        + len(matchers_c)
    )
    print("=" * 72)
    print("GENUINE DUAL-MATCHER COUNT (after classification)")
    print("=" * 72)
    print(f"  EXT-A unique reduced fractions (modulo fraction redundancy): "
          f"{len({(e[0], e[1], e[2], e[3], e[4], e[5]) for e in unique_a})}")
    print(f"  EXT-B cubic dual-matchers: {len(matchers_b)} total — "
          f"{len(ext_b_embeddings)} exact master-quadratic embeddings, "
          f"{len(ext_b_genuine)} GENUINELY NEW")
    for e in ext_b_genuine:
        a0 = e[4] * G_STAR ** e[5]
        print(f"        x·P_master(x) − {a0:.6f}  →  "
              f"x_+={e[6]:.9f}, x_-={e[7]:.9f}  (nonzero remainder)")
    print(f"  EXT-C non-master-quadratic in Gaussian family: {len(nontrivial_c)}")
    print(f"  EXT-C non-master-quadratic in Eisenstein family: {len(matchers_c)}")
    print()

    print("  Measured:")
    print(f"    Scan size:       {total_scanned:,} polynomials/multipliers")
    print(f"                     vs. 147,456 in original FTD-0121 scan")
    print(f"                     → ~{total_scanned / 147456:.1f}× larger search space")
    print(f"    Distinct dual-matchers: {genuine_unique}")
    print()

    print("CONCLUSION:")
    print()
    print("CONCLUSION:")
    print()
    # [FIX 2026-08-04] genuine_unique includes the master quadratic itself
    # (the single EXT-A survivor), so reporting it as the count "beyond the
    # master quadratic" overstated by one. Reported separately now.
    beyond_master = len(ext_b_genuine) + len(nontrivial_c) + len(matchers_c)
    print(f"  {genuine_unique} distinct dual-matchers in the declared search space:")
    print(f"  the master quadratic, plus {beyond_master} beyond it and its")
    print(f"  trivial equivalences.")
    print()
    print(f"  The 'uniquely dual-selective' headline does NOT hold as stated.")
    print(f"  All {beyond_master} additional matchers are cubics of the form")
    print(f"  x·P_master(x) − a_0, a_0 ∈ FTD's own n·G*^p class — inside the")
    print(f"  declared EXT-B family, not outside it.")
    print()
    print("  Each genuinely-new matcher must be analyzed for its")
    print("  structural relationship to the master quadratic. If they")
    print("  are all algebraically equivalent (e.g., all share the same")
    print("  Galois closure), the structural-uniqueness argument survives.")
    print("  If they are independent algebraic objects, FTD-0121")
    print("  [SYNTHESIS] Bayes factor weakens proportionally.")
    print()
    # [AUDIT 2026-06-24] the ~4e5:1 Bayes figure cited downstream is NOT
    # computed here; this script yields a ~19x scan-size factor
    # (total_scanned // 147456 == 19; docstring says ~20,000:1). The
    # dual/rigidity 'unique matcher' result is also tolerance-conditioned
    # (asymmetric x+ ppm gate vs x- 1% gate); under a symmetric 1% gate
    # ~32 dual-matchers appear across 11 constants. Treat the single-x+
    # ppm-fit as [NUMERICAL FACT], not as a 4e5:1 structural Bayes result.
    #
    # [FIX 2026-08-04] the audit comment above was added on 2026-06-24 but
    # the print statements under it were left asserting the retracted
    # figure anyway, so every run kept emitting it. Corrected to state the
    # retraction and the base-rate result. Scan logic and all counts above
    # are unchanged.
    print("  Evidential weight — READ BEFORE CITING:")
    print("    • The ~4×10^5:1 Bayes figure is RETRACTED (spine audit")
    print("      2026-06-24). It was never computed by this runner, which")
    print(f"      yields only a ~{total_scanned // 147456}× scan-size factor applied to an")
    print("      unverified prior. Do not restate it.")
    print("    • The zero count is NOT evidence of structural uniqueness.")
    print("      Base-rate control (PREREG_OT33_BASERATE_v1, 2026-08-04,")
    print("      scripts/experiments/verify_ot33_baserate.py): under")
    print("      displaced targets this family yields N_null = 0.0014")
    print("      dual-matchers on average, P(>=1) = 0.0009. Finding zero")
    print("      others is exactly what chance predicts — OUTCOME B.")
    print("    • The x_- leg eliminates nothing at the registered gate")
    print("      (16 pass x_+, 16 pass both). 'Dual-match' and 'match' are")
    print("      the same predicate here, and x_- ↔ N_c is RETIRED")
    print("      (FTD-0014). Same defect FTD-0791 found in FTD-0319.")
    print("    • Tolerance-conditioned: under a symmetric 1% gate ~32")
    print("      dual-matchers appear across 11 constants.")
    print("    • The target IS the master quadratic's own root, so it")
    print("      matches at residual 0 by construction. This scan cannot")
    print("      measure the specialness of the object defining its target.")
    print()
    print("  What survives, as [NUMERICAL FACT]: across the stated domain the")
    print("  master quadratic is the only reduced QUADRATIC in the family that")
    print("  lands in the gate. That is a narrower claim than 'uniquely")
    print("  dual-selective across the extended search space' — the cubic")
    print("  extension the scan was built to test contributes 4 more.")
    print("  FTD-0001's empirical identification x_+ = 1/α is untouched by")
    print("  this runner and remains [SMC].")
    return 0


if __name__ == "__main__":
    sys.exit(main())
