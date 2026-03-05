#!/usr/bin/env python3
"""
Testing c4 = 141/11 Discovery!
==============================

The search found that c4 ~ 12.88 is very close to 141/11 = 12.818...

141 = N_c * D = 3 * 47 (the denominator of c3!)
11 = b_3 + N_base = 7 + 4 (framework sum!)

This would give: c4 = (N_c * D) / (b_3 + N_base) = 141/11

Author: Claude Code
Date: January 31, 2026
"""

from mpmath import mp, mpf, pi, e, gamma, sqrt, exp
from fractions import Fraction

mp.dps = 100

# Framework integers
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13
D = N_c * N_base**2 - 1  # = 47

print("=" * 80)
print("TESTING c4 = 141/11 = (N_c * D) / (b_3 + N_base)")
print("=" * 80)
print()

# Verify the identity
c4_num = N_c * D
c4_den = b_3 + N_base
c4_frac = Fraction(c4_num, c4_den)

print(f"c4 = (N_c * D) / (b_3 + N_base)")
print(f"   = ({N_c} * {D}) / ({b_3} + {N_base})")
print(f"   = {c4_num} / {c4_den}")
print(f"   = {c4_frac}")
print(f"   = {float(c4_frac):.10f}")
print()

# Setup constants
Gamma_quarter = gamma(mpf('0.25'))
G_star = sqrt(mpf(2)) * Gamma_quarter**2 / (2 * pi)

b_coef = -16 * G_star**2
c_coef = 16 * G_star**3
discriminant = b_coef**2 - 4 * c_coef
x_plus = (-b_coef + sqrt(discriminant)) / 2

epsilon = exp(pi) - pi - 20
eps = abs(epsilon)

alpha_inv_exp = mpf('137.035999177')

# Coefficients
c1 = mpf(9) / mpf(47)      # N_c^2 / D
c2 = mpf(5) / mpf(64)      # (N_eff - 2*N_base) / N_base^3
c3 = mpf(4) / mpf(141)     # N_base / (N_c * D)
c4 = mpf(141) / mpf(11)    # (N_c * D) / (b_3 + N_base)

print("-" * 80)
print("ALL FOUR COEFFICIENTS")
print("-" * 80)
print()
print(f"c1 = 9/47  = N_c^2 / D                 = {float(c1):.10f}")
print(f"c2 = 5/64  = (N_eff-2N_base) / N_base^3 = {float(c2):.10f}")
print(f"c3 = 4/141 = N_base / (N_c*D)          = {float(c3):.10f}")
print(f"c4 = 141/11 = (N_c*D) / (b_3+N_base)   = {float(c4):.10f}")
print()

# Test formulas
print("-" * 80)
print("PRECISION COMPARISON")
print("-" * 80)
print()

# 2-term
alpha_2 = x_plus - c1*eps + c2*eps**2
error_2 = abs(alpha_2 - alpha_inv_exp) / alpha_inv_exp * mpf('1e12')

# 3-term
alpha_3 = x_plus - c1*eps + c2*eps**2 - c3*eps**3
error_3 = abs(alpha_3 - alpha_inv_exp) / alpha_inv_exp * mpf('1e12')

# 4-term with c4 = 141/11
alpha_4 = x_plus - c1*eps + c2*eps**2 - c3*eps**3 + c4*eps**4
error_4 = abs(alpha_4 - alpha_inv_exp) / alpha_inv_exp * mpf('1e12')

print(f"2-term formula: error = {float(error_2):.3f} ppt")
print(f"3-term formula: error = {float(error_3):.3f} ppt")
print(f"4-term formula: error = {float(error_4):.3f} ppt")
print()

# Hmm, the 4-term got worse. Let's try negative c4
alpha_4neg = x_plus - c1*eps + c2*eps**2 - c3*eps**3 - c4*eps**4
error_4neg = abs(alpha_4neg - alpha_inv_exp) / alpha_inv_exp * mpf('1e12')

print(f"4-term (c4 negative): error = {float(error_4neg):.3f} ppt")
print()

# The exact c4 needed was NEGATIVE ~-12.88
# So we need -c4, not +c4

print("-" * 80)
print("CORRECTED: 4-TERM FORMULA WITH -c4")
print("-" * 80)
print()

# The pattern is: alternating signs
# 1/alpha = x_+ - c1*eps + c2*eps^2 - c3*eps^3 + ???
# If the pattern continues: - c4*eps^4

# But wait, -12.88 means we need:
# 1/alpha = x_+ - c1*eps + c2*eps^2 - c3*eps^3 - c4*eps^4
# where c4 = +141/11

print("Full 4-term formula:")
print("  1/alpha = x_+ - (9/47)|eps| + (5/64)|eps|^2 - (4/141)|eps|^3 - (141/11)|eps|^4")
print()
print(f"  Predicted: {float(alpha_4neg):.15f}")
print(f"  CODATA:    {float(alpha_inv_exp):.15f}")
print(f"  Error:     {float(error_4neg):.3f} ppt")
print()

# Let me check the exact value needed
residual_3 = alpha_inv_exp - alpha_3
c4_exact = residual_3 / eps**4
print(f"Exact c4 needed: {float(c4_exact):.6f}")
print(f"141/11 = {float(mpf(141)/mpf(11)):.6f}")
print(f"Difference: {float(c4_exact - mpf(141)/mpf(11)):.6f}")
print()

# =============================================================================
# WAIT - let me reconsider the sign pattern
# =============================================================================

print("=" * 80)
print("SIGN PATTERN ANALYSIS")
print("=" * 80)
print()

# The exact c4 is ~ -12.88
# This is NEGATIVE, so the term is -c4*eps^4 = -(-12.88)*eps^4 = +12.88*eps^4
# No wait, if c4_exact = -12.88, then the term IS c4_exact*eps^4 = -12.88*eps^4

# But 141/11 = +12.818, so we need -141/11 = -12.818

# Check: does -(141/11) work as c4?
c4_test = -mpf(141) / mpf(11)
alpha_test = x_plus - c1*eps + c2*eps**2 - c3*eps**3 + c4_test*eps**4
error_test = abs(alpha_test - alpha_inv_exp) / alpha_inv_exp * mpf('1e12')

print(f"Using c4 = -141/11:")
print(f"  1/alpha = x_+ - (9/47)|eps| + (5/64)|eps|^2 - (4/141)|eps|^3 + (-141/11)|eps|^4")
print(f"         = x_+ - (9/47)|eps| + (5/64)|eps|^2 - (4/141)|eps|^3 - (141/11)|eps|^4")
print(f"  Error:   {float(error_test):.3f} ppt")
print()

# That's way worse. The sign pattern must be different.

# Let's think about this more carefully
# Current formula gives 0.062 ppt at order 3
# We need to ADD a negative correction of ~-8e-12

print("-" * 80)
print("UNDERSTANDING THE CORRECTION NEEDED")
print("-" * 80)
print()

print(f"Current 3-term value:  {float(alpha_3):.15f}")
print(f"CODATA value:          {float(alpha_inv_exp):.15f}")
print(f"Residual (need to add): {float(residual_3):.15e}")
print()

# residual_3 is NEGATIVE, so we need to subtract something
# residual_3 / eps^4 ~ -12.88
# So we need to add c4*eps^4 where c4 = -12.88

# The formula should be:
# 1/alpha = x_+ - c1*eps + c2*eps^2 - c3*eps^3 + c4*eps^4
# with c4 = -12.88 (NEGATIVE)

# Can -12.88 be expressed as a ratio?
# -141/11 = -12.818 is close!

# BUT 141/11 reduces! Let's check
print(f"141 = 3 * 47")
print(f"11 = 7 + 4 = b_3 + N_base")
print(f"gcd(141, 11) = 1, so 141/11 is in lowest terms")
print()

# The difference between exact and 141/11:
diff_c4 = float(c4_exact) - (-141.0/11.0)
print(f"c4_exact = {float(c4_exact):.6f}")
print(f"-141/11  = {-141.0/11.0:.6f}")
print(f"Difference: {diff_c4:.6f}")
print()

# This difference of ~0.063 at the 4th order level contributes:
contrib = diff_c4 * float(eps**4)
print(f"Contribution to 1/alpha: {contrib:.3e}")
print(f"In ppt: {contrib/float(alpha_inv_exp)*1e12:.3f} ppt")
print()

# =============================================================================
# FINAL VERIFICATION
# =============================================================================

print("=" * 80)
print("FINAL 4-TERM FORMULA TEST")
print("=" * 80)
print()

c4_framework = mpf(N_c * D) / mpf(b_3 + N_base)  # 141/11

# The exact c4 is negative
alpha_4_final = x_plus - c1*eps + c2*eps**2 - c3*eps**3 - c4_framework*eps**4
error_4_final = abs(alpha_4_final - alpha_inv_exp) / alpha_inv_exp * mpf('1e12')

print("Formula: 1/alpha = x_+ - (9/47)|eps| + (5/64)|eps|^2 - (4/141)|eps|^3 - (141/11)|eps|^4")
print()
print("Where:")
print(f"  9/47   = N_c^2 / D")
print(f"  5/64   = (N_eff - 2N_base) / N_base^3")
print(f"  4/141  = N_base / (N_c * D)")
print(f"  141/11 = (N_c * D) / (b_3 + N_base)")
print()
print(f"Predicted 1/alpha: {float(alpha_4_final):.15f}")
print(f"CODATA 1/alpha:    {float(alpha_inv_exp):.15f}")
print(f"Error:             {float(error_4_final):.3f} ppt")
print()

if error_4_final < error_3:
    print("[IMPROVEMENT] 4-term formula is better!")
else:
    print("[NO IMPROVEMENT] 3-term formula remains best")
    print(f"  3-term: {float(error_3):.3f} ppt")
    print(f"  4-term: {float(error_4_final):.3f} ppt")
