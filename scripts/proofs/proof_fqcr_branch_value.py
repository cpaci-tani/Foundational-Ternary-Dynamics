"""
proof_fqcr_branch_value.py
==========================

Verifies the trilogy's depth-4 five-harmonic model numerical content as a
sanity check for the QCR-trilogy-bridge documentation (FTD-0144).

Reference: compatibility_branch_curve_spectra_authorless.pdf (2026-05-07),
Example 7.1 + Numerical Observations 7.2/7.3 + Proposition 7.4.

What is verified
----------------
1. Exact rational identity (compatibility paper Proposition 7.4):

       Area(gamma_4) = pi * sum_{n=0..4} 2^n * a_n * b_n = 177*pi/400

   Computed in fractions.Fraction (exact arithmetic).

2. Independent numerical cross-check via Green's-formula integration:

       Area = (1/2) * int_0^{2*pi} ( x_4(t) * y_4'(t) - y_4(t) * x_4'(t) ) dt

   Computed with mpmath at 50-digit precision; asserted to agree with the
   rational closed form to better than 1e-40.

3. Reciprocal projective branches sanity (compatibility paper Prop 2.6):

       z^2 - s*z + 1 = 0  =>  z_+ * z_- = 1

   Verified for sample s values including s = 4*sqrt(G*) which is the
   FTD-master-quadratic-relevant scale.

What is NOT verified
--------------------
- This script does not verify any FTD-side claim. The master quadratic's
  algebraic rigidity (FTD-0014), the operator-theoretic provenance of G*
  (FTD-0141), and the finite-N attractor (FTD-0142) are verified by their
  own proof scripts. This file only verifies the trilogy's published
  numerical content as a methodological sanity check before integrating
  the trilogy's framework into FTD's documentation.

Output
------
On success: prints PASS for each of three checks and a final OK summary.
On any failure: AssertionError with a short diagnostic.

Runtime: well under 1 second.
"""

from __future__ import annotations

from fractions import Fraction

import mpmath as mp


# ----------------------------------------------------------------------
# The depth-4 five-harmonic model coefficients (compatibility paper §7)
# ----------------------------------------------------------------------

# x_4(t) = sum_{n=0..4} a_n cos(2^n t)
A = [Fraction(1, 1),     # a_0
     Fraction(1, 2),     # a_1
     Fraction(1, 2),     # a_2
     Fraction(2, 5),     # a_3
     Fraction(1, 16)]    # a_4

# y_4(t) = sum_{n=0..4} b_n sin(2^n t)
B = [Fraction(1, 1),     # b_0
     Fraction(-1, 2),    # b_1
     Fraction(1, 2),     # b_2
     Fraction(-7, 20),   # b_3
     Fraction(1, 16)]    # b_4

DEPTH_N = 4
EXPECTED_AREA_RATIONAL = Fraction(177, 400)  # Area / pi


# ----------------------------------------------------------------------
# Check 1: Exact rational identity
# ----------------------------------------------------------------------

def check_rational_area() -> Fraction:
    """Sum 2^n * a_n * b_n in exact rational arithmetic and assert == 177/400."""
    s = Fraction(0)
    for n in range(DEPTH_N + 1):
        s += Fraction(1 << n) * A[n] * B[n]

    assert s == EXPECTED_AREA_RATIONAL, (
        f"Rational area mismatch: got {s}, expected {EXPECTED_AREA_RATIONAL}. "
        f"Trilogy Proposition 7.4 fails — investigate immediately."
    )
    return s


# ----------------------------------------------------------------------
# Check 2: Independent Green's-formula integration
# ----------------------------------------------------------------------

def green_area_numerical() -> mp.mpf:
    """
    Compute Area = (1/2) integral_0^{2*pi} ( x*y' - y*x' ) dt at 50 dps.

    Using compatibility paper §3 Definition + Green's identity:
        Area = (1/2) integral_0^{2*pi} ( x_4 * y_4' - y_4 * x_4' ) dt
    """
    mp.mp.dps = 50

    A_mpf = [mp.mpf(a.numerator) / mp.mpf(a.denominator) for a in A]
    B_mpf = [mp.mpf(b.numerator) / mp.mpf(b.denominator) for b in B]

    def x(t):
        return sum(A_mpf[n] * mp.cos((1 << n) * t) for n in range(DEPTH_N + 1))

    def y(t):
        return sum(B_mpf[n] * mp.sin((1 << n) * t) for n in range(DEPTH_N + 1))

    def xp(t):
        # x'(t) = -sum_{n} 2^n * a_n * sin(2^n t)
        return -sum(mp.mpf(1 << n) * A_mpf[n] * mp.sin((1 << n) * t)
                    for n in range(DEPTH_N + 1))

    def yp(t):
        # y'(t) = sum_{n} 2^n * b_n * cos(2^n t)
        return sum(mp.mpf(1 << n) * B_mpf[n] * mp.cos((1 << n) * t)
                   for n in range(DEPTH_N + 1))

    def integrand(t):
        return x(t) * yp(t) - y(t) * xp(t)

    integral = mp.quad(integrand, [0, 2 * mp.pi])
    return integral / mp.mpf(2)


def check_numerical_area():
    """Assert the Green's-formula integration agrees with 177*pi/400."""
    mp.mp.dps = 50
    expected = mp.pi * mp.mpf(177) / mp.mpf(400)
    measured = green_area_numerical()
    diff = abs(measured - expected)
    tol = mp.mpf("1e-40")
    assert diff < tol, (
        f"Numerical area disagrees with rational closed form by {diff} "
        f"(tol {tol}). Either the trilogy's identity is wrong, the script "
        f"has a bug, or the integrator failed to converge."
    )
    return measured, expected, diff


# ----------------------------------------------------------------------
# Check 3: Reciprocal projective branches
# ----------------------------------------------------------------------

def check_reciprocal_branches():
    """
    For the symmetric recurrence u_{m+1} + u_{m-1} = s u_m, the projective
    fixed branches satisfy z_+ * z_- = 1 (compatibility paper Prop 2.6).

    Verify for sample s, including the FTD-master-quadratic-relevant
    s_master = 4 * sqrt(G_star) where G_star = Gamma(1/4)/Gamma(3/4).
    """
    mp.mp.dps = 50
    g_star = mp.gamma(mp.mpf("0.25")) / mp.gamma(mp.mpf("0.75"))

    s_samples = [
        mp.mpf("1.5"),                    # below the |s| > 2 stability threshold
        mp.mpf(2),                        # boundary (degenerate)
        mp.mpf(4),                        # |s| > 2, real branches
        mp.mpf(8),
        mp.mpf(16),
        4 * mp.sqrt(g_star),              # master-quadratic relevant scale
    ]

    tol = mp.mpf("1e-45")

    for s in s_samples:
        if s == 2:
            # degenerate: z_+ = z_- = 1; product still 1.
            z_plus = z_minus = mp.mpf(1)
        else:
            disc = mp.sqrt(s * s - 4) if s * s > 4 else mp.sqrt(mp.mpc(s * s - 4))
            z_plus = (s + disc) / 2
            z_minus = (s - disc) / 2

        prod = z_plus * z_minus
        diff = abs(prod - 1)
        assert diff < tol, (
            f"Branch product z_+ * z_- != 1 for s={s}: got {prod}, "
            f"diff {diff} (tol {tol})."
        )
    return s_samples


# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------

def main():
    print("=" * 70)
    print("FQCR branch-value sanity check — trilogy compatibility paper §7")
    print("=" * 70)

    # Check 1
    rational_sum = check_rational_area()
    print(f"\nCheck 1 (rational arithmetic):")
    print(f"  sum_{{n=0..4}} 2^n * a_n * b_n = {rational_sum} = "
          f"{rational_sum.numerator}/{rational_sum.denominator}")
    print(f"  Expected: 177/400")
    print(f"  PASS")

    # Check 2
    measured, expected, diff = check_numerical_area()
    print(f"\nCheck 2 (Green's-formula integration, 50 dps):")
    print(f"  Numerical Area    = {measured}")
    print(f"  Rational closed   = {expected}")
    print(f"  |numerical - rational| = {diff}")
    print(f"  Tolerance         = 1e-40")
    print(f"  PASS")

    # Check 3
    s_samples = check_reciprocal_branches()
    print(f"\nCheck 3 (reciprocal projective branches z_+ * z_- = 1):")
    print(f"  Verified for {len(s_samples)} sample s values.")
    print(f"  Includes s_master = 4*sqrt(G*) (FTD master-quadratic scale).")
    print(f"  PASS")

    print("\n" + "=" * 70)
    print("OK — trilogy depth-4 numerical content reproduces correctly.")
    print("This script does NOT verify any FTD-side claim. It is a sanity")
    print("check on the trilogy's own published numbers; cited from")
    print("REF_QCR_TRILOGY_BRIDGE.md §7 (FTD-0144).")
    print("=" * 70)


if __name__ == "__main__":
    main()
