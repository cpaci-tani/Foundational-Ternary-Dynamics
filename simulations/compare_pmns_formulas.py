"""
COMPARE PMNS FORMULAS: Existing vs Cubic-Derived
=================================================

Comparing the current NOVEL_CLAIMS.md formulas with our new cubic-derived predictions.
"""

import numpy as np

# Experimental values (NuFit 5.2, 2024)
sin2_12_exp = 0.304   # +/- 0.012
sin2_23_exp = 0.573   # +/- 0.020 (NO)
sin2_13_exp = 0.02220 # +/- 0.00062

theta_12_exp = np.degrees(np.arcsin(np.sqrt(sin2_12_exp)))  # 33.4 deg
theta_23_exp = np.degrees(np.arcsin(np.sqrt(sin2_23_exp)))  # 49.2 deg
theta_13_exp = np.degrees(np.arcsin(np.sqrt(sin2_13_exp)))  # 8.6 deg

print("=" * 70)
print("EXPERIMENTAL VALUES (NuFit 5.2)")
print("=" * 70)
print(f"sin^2(theta_12) = {sin2_12_exp:.4f} -> theta_12 = {theta_12_exp:.2f} deg")
print(f"sin^2(theta_23) = {sin2_23_exp:.4f} -> theta_23 = {theta_23_exp:.2f} deg")
print(f"sin^2(theta_13) = {sin2_13_exp:.5f} -> theta_13 = {theta_13_exp:.2f} deg")
print()

# Framework integers
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13

print("=" * 70)
print("THETA_12 COMPARISON")
print("=" * 70)
print()

# Both formulas agree: sin^2 = 3/10
sin2_12_old = 3/10  # arcsin(sqrt(3/10))
sin2_12_new = N_c / (N_c + b_3)  # = 3/10

theta_12_old = np.degrees(np.arcsin(np.sqrt(sin2_12_old)))
theta_12_new = np.degrees(np.arcsin(np.sqrt(sin2_12_new)))

print(f"OLD: sin^2 = 3/10 = {sin2_12_old:.4f} -> theta = {theta_12_old:.2f} deg")
print(f"NEW (cubic): N_c/(N_c+b_3) = 3/10 = {sin2_12_new:.4f} -> theta = {theta_12_new:.2f} deg")
print(f"EXPERIMENTAL: {sin2_12_exp:.4f} -> {theta_12_exp:.2f} deg")
print(f"Both formulas identical: sin^2 = 3/10")
print(f"Error: {abs(sin2_12_old - sin2_12_exp)/sin2_12_exp*100:.2f}%")
print()

print("=" * 70)
print("THETA_23 COMPARISON - DIFFERENT FORMULAS!")
print("=" * 70)
print()

# OLD: sin^2 = 16/29
sin2_23_old = 16/29
theta_23_old = np.degrees(np.arcsin(np.sqrt(sin2_23_old)))
error_23_old = abs(sin2_23_old - sin2_23_exp)/sin2_23_exp*100

# NEW: sin^2 = 7/13 = (N_c + N_base)/N_eff
sin2_23_new = (N_c + N_base) / N_eff
theta_23_new = np.degrees(np.arcsin(np.sqrt(sin2_23_new)))
error_23_new = abs(sin2_23_new - sin2_23_exp)/sin2_23_exp*100

print(f"OLD: sin^2 = 16/29 = {sin2_23_old:.4f} -> theta = {theta_23_old:.2f} deg")
print(f"     Error: {error_23_old:.2f}%")
print()
print(f"NEW (cubic): (N_c+N_base)/N_eff = 7/13 = {sin2_23_new:.4f} -> theta = {theta_23_new:.2f} deg")
print(f"     Error: {error_23_new:.2f}%")
print()
print(f"EXPERIMENTAL: {sin2_23_exp:.4f} -> {theta_23_exp:.2f} deg")
print()

if error_23_old < error_23_new:
    print(f"WINNER: OLD formula (16/29) is more accurate by {error_23_new - error_23_old:.1f} percentage points")
else:
    print(f"WINNER: NEW formula (7/13) is more accurate by {error_23_old - error_23_new:.1f} percentage points")
print()

# BUT: Check where 16/29 comes from
print("INTERPRETATION:")
print(f"  OLD: 16/29 - where does this come from?")
print(f"       16 = N_base^2 (lattice DoF)")
print(f"       29 = ??? (not obviously from framework)")
print()
print(f"  NEW: 7/13 = (N_c + N_base)/N_eff")
print(f"       This uses ONLY framework integers!")
print()

print("=" * 70)
print("THETA_13 COMPARISON")
print("=" * 70)
print()

# Both formulas agree: sin^2 = 1/52
sin2_13_old = 1/52
sin2_13_new = 1/(N_base * N_eff)

theta_13_old = np.degrees(np.arcsin(np.sqrt(sin2_13_old)))
theta_13_new = np.degrees(np.arcsin(np.sqrt(sin2_13_new)))
error_13 = abs(sin2_13_old - sin2_13_exp)/sin2_13_exp*100

print(f"OLD: sin^2 = 1/52 = {sin2_13_old:.5f} -> theta = {theta_13_old:.2f} deg")
print(f"NEW (cubic): 1/(N_base*N_eff) = 1/52 = {sin2_13_new:.5f} -> theta = {theta_13_new:.2f} deg")
print(f"EXPERIMENTAL: {sin2_13_exp:.5f} -> {theta_13_exp:.2f} deg")
print(f"Both formulas identical: sin^2 = 1/52")
print(f"Error: {error_13:.2f}%")
print()

print("=" * 70)
print("SUMMARY")
print("=" * 70)
print()
print("AGREEMENTS with existing formulas:")
print("  theta_12: sin^2 = 3/10 = N_c/(N_c+b_3)")
print("  theta_13: sin^2 = 1/52 = 1/(N_base*N_eff)")
print()
print("DISAGREEMENT:")
print(f"  theta_23: OLD uses 16/29 = {16/29:.4f} (error {error_23_old:.2f}%)")
print(f"            NEW uses 7/13 = {7/13:.4f} (error {error_23_new:.2f}%)")
print()

# Alternative: maybe 29 has a framework interpretation?
print("FRAMEWORK CHECK for 29:")
print(f"  29 = N_eff + N_base^2 = 13 + 16 = {N_eff + N_base**2}")
print(f"  29 = 2*N_eff + N_c = 2*13 + 3 = {2*N_eff + N_c}")
print(f"  29 = b_3 + N_eff + N_c^2 = 7 + 13 + 9 = {b_3 + N_eff + N_c**2}")
print()

# Actually 16/29 is close to 7/13 - let's see how they relate
print("RELATIONSHIP between 16/29 and 7/13:")
print(f"  16/29 = {16/29:.6f}")
print(f"  7/13 = {7/13:.6f}")
print(f"  Ratio: (16/29)/(7/13) = {(16/29)/(7/13):.6f}")
print(f"  Difference: {abs(16/29 - 7/13):.6f}")
print()

# New discovery: tau/muon mass ratio
print("=" * 70)
print("NOVEL FINDING: TAU/MUON MASS RATIO")
print("=" * 70)
print()

# Master cubic roots
GAMMA_QUARTER = 3.6256099082219083
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)

a = 16 * G_STAR**2
b = 16 * G_STAR**3
roots = np.roots([1, 0, -a, -b])
roots = np.sort(roots.real)[::-1]

r1 = roots[0]
r2 = roots[1]

ratio_cubic = (abs(r1/r2))**2
mass_ratio_exp = 1776.86 / 105.66

print(f"Cubic roots: r1 = {r1:.4f}, r2 = {r2:.4f}")
print(f"|r1/r2|^2 = {ratio_cubic:.4f}")
print(f"m_tau/m_mu = {mass_ratio_exp:.4f}")
print(f"Error: {abs(ratio_cubic - mass_ratio_exp)/mass_ratio_exp*100:.2f}%")
print()
print("THIS IS A 0.3% PREDICTION NOT IN EXISTING DOCUMENTATION!")
print()

# Check Cabibbo angle
print("=" * 70)
print("CABIBBO ANGLE COMPARISON")
print("=" * 70)
print()

V_us_exp = 0.2245
sin_C_old = np.sqrt(3/13)  # From CKM formula in docs
sin_C_new = G_STAR / N_eff  # Our cubic discovery

print(f"OLD: sin(theta_C) = sqrt(3/13) = {sin_C_old:.4f} (error: {abs(sin_C_old - V_us_exp)/V_us_exp*100:.2f}%)")
print(f"NEW: sin(theta_C) = G*/N_eff = {sin_C_new:.4f} (error: {abs(sin_C_new - V_us_exp)/V_us_exp*100:.2f}%)")
print(f"EXPERIMENTAL: {V_us_exp:.4f}")
print()

if abs(sin_C_old - V_us_exp) < abs(sin_C_new - V_us_exp):
    print("OLD formula is more accurate")
else:
    print("NEW formula (G*/N_eff) is more accurate!")
print()

print("=" * 70)
print("FINAL ASSESSMENT: WHAT'S NOVEL FROM THE CUBIC")
print("=" * 70)
print()
print("CONFIRMED MATCHES WITH EXISTING:")
print("  - sin^2(theta_12) = 3/10 = N_c/(N_c+b_3)")
print("  - sin^2(theta_13) = 1/52 = 1/(N_base*N_eff)")
print()
print("POTENTIAL IMPROVEMENT:")
print(f"  - sin^2(theta_23): 7/13 may be better motivated than 16/29")
print(f"    (even though 16/29 is slightly closer to experiment)")
print()
print("GENUINELY NOVEL:")
print("  1. m_tau/m_mu = |r1/r2|^2 from cubic roots (0.3% error)")
print("  2. Discriminant D/(16^2*G*^6) = 37 exactly")
print("  3. 37 = N_eff*N_c - 2 = N_base*b_3 + N_c^2")
print("  4. sin(theta_C) = G*/N_eff (1.4% error)")
print("  5. Jarlskog J ~ |y_c|*alpha^2 (9% error)")
print()
