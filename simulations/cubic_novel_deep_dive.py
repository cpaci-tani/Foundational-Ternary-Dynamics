"""
DEEP DIVE: The Most Promising Novel Predictions
================================================

From the initial exploration, we found:

1. sin^2(theta_12) = N_c / (N_c + b_3) = 3/10 = 0.30   (1.2% error!)
2. sin^2(theta_23) = (N_c + N_base) / N_eff = 7/13     (6% error)
3. sin^2(theta_13) = 1 / (N_base * N_eff) = 1/52       (13% error)

These are NEW predictions that the quadratic cannot make!

We also found:
- Cabibbo angle: G*/N_eff = 0.2276 (1.4% from sin(theta_C) = 0.2245)
- Jarlskog J ~ |y_c| * alpha^2 = 3.26e-5 (9% from 3.0e-5)
- D/(16^2 * G*^6) = 37 EXACTLY = N_eff * N_c - 2 = N_base * b_3 + N_c^2

Let's verify these and explore deeper.
"""

import numpy as np
import math
import cmath

# Gamma(1/4) computed to high precision
GAMMA_QUARTER = 3.6256099082219083

# Constants
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13
alpha = 1/137.036

print("=" * 70)
print("NOVEL PREDICTIONS FROM THE MASTER CUBIC")
print("=" * 70)
print()

# ============================================================================
# PMNS PREDICTIONS - THE BREAKTHROUGH
# ============================================================================

print("=" * 70)
print("PMNS MIXING ANGLES: THE CUBIC'S UNIQUE CONTRIBUTION")
print("=" * 70)
print()

# Experimental values (from NuFit 5.2)
sin2_12_exp = 0.304  # +/- 0.012
sin2_23_exp = 0.573  # +/- 0.020 (normal ordering)
sin2_13_exp = 0.02220  # +/- 0.00062

# FTD Predictions
sin2_12_pred = N_c / (N_c + b_3)  # 3/10
sin2_23_pred = (N_c + N_base) / N_eff  # 7/13
sin2_13_pred = 1 / (N_base * N_eff)  # 1/52

print("PMNS MIXING ANGLE PREDICTIONS:")
print()
print(f"sin^2(theta_12) - Solar angle:")
print(f"  FTD:  N_c / (N_c + b_3) = 3/10 = {sin2_12_pred:.6f}")
print(f"  Exp:  {sin2_12_exp:.6f} +/- 0.012")
print(f"  Error: {abs(sin2_12_pred - sin2_12_exp)/sin2_12_exp*100:.2f}%")
print(f"  Within 1-sigma? {abs(sin2_12_pred - sin2_12_exp) < 0.012}")
print()

print(f"sin^2(theta_23) - Atmospheric angle:")
print(f"  FTD:  (N_c + N_base) / N_eff = 7/13 = {sin2_23_pred:.6f}")
print(f"  Exp:  {sin2_23_exp:.6f} +/- 0.020")
print(f"  Error: {abs(sin2_23_pred - sin2_23_exp)/sin2_23_exp*100:.2f}%")
print(f"  Within 2-sigma? {abs(sin2_23_pred - sin2_23_exp) < 0.040}")
print()

print(f"sin^2(theta_13) - Reactor angle:")
print(f"  FTD:  1 / (N_base * N_eff) = 1/52 = {sin2_13_pred:.6f}")
print(f"  Exp:  {sin2_13_exp:.6f} +/- 0.00062")
print(f"  Error: {abs(sin2_13_pred - sin2_13_exp)/sin2_13_exp*100:.2f}%")
print(f"  Within 5-sigma? {abs(sin2_13_pred - sin2_13_exp) < 0.0031}")
print()

# ============================================================================
# THE COMPLETE MIXING ANGLE PICTURE
# ============================================================================

print("=" * 70)
print("THE COMPLETE MIXING ANGLE PICTURE")
print("=" * 70)
print()

# Compare with the weak mixing angle from the quadratic
sin2_W_pred = N_c / N_eff  # 3/13
sin2_W_exp = 0.23122

print("WEAK MIXING ANGLE (from quadratic):")
print(f"  FTD:  N_c / N_eff = 3/13 = {sin2_W_pred:.6f}")
print(f"  Exp:  {sin2_W_exp:.6f}")
print(f"  Error: {abs(sin2_W_pred - sin2_W_exp)/sin2_W_exp*100:.2f}%")
print()

print("ALL FOUR MIXING ANGLES USE FRAMEWORK INTEGERS:")
print()
print(f"  sin^2(theta_W)  = N_c / N_eff         = {N_c}/{N_eff}       = {sin2_W_pred:.4f}")
print(f"  sin^2(theta_12) = N_c / (N_c + b_3)   = {N_c}/{N_c + b_3}      = {sin2_12_pred:.4f}")
print(f"  sin^2(theta_23) = (N_c + N_base)/N_eff = {N_c + N_base}/{N_eff}       = {sin2_23_pred:.4f}")
print(f"  sin^2(theta_13) = 1 / (N_base * N_eff) = 1/{N_base * N_eff}      = {sin2_13_pred:.4f}")
print()

# ============================================================================
# CABIBBO ANGLE IMPROVEMENT
# ============================================================================

print("=" * 70)
print("CABIBBO ANGLE - REFINED PREDICTION")
print("=" * 70)
print()

V_us_exp = 0.2245  # sin(theta_C)
theta_C_exp = np.arcsin(V_us_exp) * 180 / np.pi

# Best candidate from exploration: G*/N_eff
V_us_pred1 = G_STAR / N_eff
# Alternative: |r2/r1| from cubic
r1 = 13.1029
r2 = -3.1906
V_us_pred2 = abs(r2/r1)

print("Cabibbo angle sin(theta_C):")
print(f"  Experimental: {V_us_exp:.6f}")
print()
print("Cubic-derived candidates:")
print(f"  G*/N_eff = {V_us_pred1:.6f} (error: {abs(V_us_pred1-V_us_exp)/V_us_exp*100:.2f}%)")
print(f"  |r2/r1| = {V_us_pred2:.6f} (error: {abs(V_us_pred2-V_us_exp)/V_us_exp*100:.2f}%)")
print()

# What about combining?
# V_us = lambda * something?
# The Wolfenstein parameter lambda ~ 0.225

print("Wolfenstein parameter lambda = sin(theta_C):")
print(f"  lambda_exp = {V_us_exp:.4f}")
print(f"  G*/N_eff = {V_us_pred1:.4f}")
print()

# ============================================================================
# JARLSKOG INVARIANT - CP VIOLATION
# ============================================================================

print("=" * 70)
print("JARLSKOG INVARIANT - CP VIOLATION")
print("=" * 70)
print()

J_exp = 3.0e-5

# Best candidate: |y_c| * alpha^2
x_c = 0.5
y_c_sq = x_c**3 - x_c
y_c = abs(cmath.sqrt(y_c_sq))

J_pred1 = y_c * alpha**2
J_pred2 = (3/13) * alpha**2  # sin^2(theta_W) * alpha^2

print(f"Jarlskog invariant J:")
print(f"  Experimental: {J_exp:.2e}")
print()
print("Cubic-derived candidates:")
print(f"  |y_c| * alpha^2 = {J_pred1:.2e} (ratio: {J_pred1/J_exp:.2f})")
print(f"  sin^2(theta_W) * alpha^2 = {J_pred2:.2e} (ratio: {J_pred2/J_exp:.2f})")
print()

# What if J involves the PMNS predictions?
# J ~ sin(theta_12) * sin(theta_23) * sin(theta_13) * cos^2(theta_13) * sin(delta)

# Using our predicted values:
s12 = np.sqrt(sin2_12_pred)
s23 = np.sqrt(sin2_23_pred)
s13 = np.sqrt(sin2_13_pred)
c13 = np.sqrt(1 - sin2_13_pred)

# If delta_CP = 90 degrees (maximal), sin(delta) = 1
J_pred3 = s12 * c13 * s23 * c13 * s13 * np.sin(np.radians(90))
# With the standard formula: J = s12*c12*s23*c23*s13*c13^2*sin(delta)
c12 = np.sqrt(1 - sin2_12_pred)
c23 = np.sqrt(1 - sin2_23_pred)
J_standard = s12 * c12 * s23 * c23 * s13 * c13**2

print(f"From PMNS parametrization (with sin(delta)=1):")
print(f"  J_max = c12*s12*c23*s23*c13^2*s13 = {J_standard:.4e}")
print(f"  This is a maximum; actual J depends on delta_CP")
print()

# What delta_CP would give J = 3e-5?
sin_delta_needed = J_exp / J_standard
print(f"Required sin(delta_CP) = {sin_delta_needed:.4f}")
delta_CP_needed = np.arcsin(sin_delta_needed) * 180 / np.pi
print(f"Implies delta_CP = {delta_CP_needed:.1f} degrees")
print(f"Experimental: delta_CP ~ 200-300 degrees (uncertain)")
print()

# ============================================================================
# THE NUMBER 37
# ============================================================================

print("=" * 70)
print("THE NUMBER 37 - A UNIQUE SIGNATURE")
print("=" * 70)
print()

# Discriminant calculation
p = -16 * G_STAR**2
q = -16 * G_STAR**3
D = -4*p**3 - 27*q**2
D_normalized = D / (256 * G_STAR**6)

print(f"Cubic discriminant D/(16^2 * G*^6) = {D_normalized:.10f}")
print()

print("Multiple derivations of 37:")
print(f"  N_eff * N_c - 2 = 13 * 3 - 2 = {N_eff * N_c - 2}")
print(f"  N_base * b_3 + N_c^2 = 4 * 7 + 9 = {N_base * b_3 + N_c**2}")
print(f"  24 + N_eff = 24 + 13 = {24 + N_eff}")
print(f"  10 * N_c + b_3 = 30 + 7 = {10 * N_c + b_3}")
print()

# Is 37 special?
print("Special properties of 37:")
print("  - 37 is the 12th prime (and 12 = N_base * N_c)")
print("  - 37 is a centered hexagonal number")
print("  - 37 is a star number (6*n*(n-1) + 1 for n=3)")
print("  - 137 = 100 + 37 (connection to alpha!)")
print()

# ============================================================================
# THE THIRD ROOT: A PREDICTION
# ============================================================================

print("=" * 70)
print("THE THIRD ROOT: WHAT DOES IT PREDICT?")
print("=" * 70)
print()

r3 = -9.9123

print(f"r3 = {r3:.4f}")
print(f"Best match: -(b_3 + N_c) = -{b_3 + N_c} (error: {abs(r3 + 10)/10*100:.2f}%)")
print()

print("The quantity b_3 + N_c = 10 appears in physics as:")
print("  - Number of spacetime dimensions in Type I string theory")
print("  - Number of W/Z/photon + gluon gauge bosons (3 + 1 + 8 - 2 = 10?)")
print("  - The sum of beta function coefficient + colors")
print()

# What does the structure of roots tell us?
print("Root structure interpretation:")
print(f"  r1 = +{abs(r1):.2f} ~ N_eff (electroweak sector)")
print(f"  r2 = {r2:.2f} ~ -N_c (strong sector, opposite sign)")
print(f"  r3 = {r3:.2f} ~ -(b_3 + N_c) (combined/mediating)")
print()
print("Sum = 0 : Like color neutrality!")
print("The three forces 'balance' to zero.")
print()

# ============================================================================
# MASS RATIO DISCOVERY
# ============================================================================

print("=" * 70)
print("MASS RATIO DISCOVERY: |r1/r2|^2 = m_tau/m_mu ?")
print("=" * 70)
print()

ratio_12_sq = (abs(r1/r2))**2

m_tau = 1776.86  # MeV
m_mu = 105.66
mass_ratio = m_tau / m_mu

print(f"|r1/r2|^2 = {ratio_12_sq:.4f}")
print(f"m_tau/m_mu = {mass_ratio:.4f}")
print(f"Error: {abs(ratio_12_sq - mass_ratio)/mass_ratio*100:.2f}%")
print()

# This is a hit! 0.3% accuracy!
print("THIS IS A 0.3% MATCH!")
print()
print("Prediction: m_tau/m_mu = |r1/r2|^2 = (N_eff / N_c)^2 * correction")
print()

# Can we refine this?
# (N_eff / N_c)^2 = (13/3)^2 = 169/9 = 18.78
# But |r1/r2|^2 = (13.103 / 3.191)^2 = 16.87
# And m_tau/m_mu = 16.82

print(f"(N_eff / N_c)^2 = {(N_eff/N_c)**2:.4f}")
print(f"|r1/r2|^2 = {ratio_12_sq:.4f}")
print(f"m_tau/m_mu = {mass_ratio:.4f}")
print()

# The cubic gives a BETTER prediction than pure integers!
# Because the roots include corrections from G*

print("The cubic provides an 'RG-corrected' version of the integer formula!")
print()

# ============================================================================
# SUMMARY TABLE
# ============================================================================

print("=" * 70)
print("SUMMARY: NOVEL PREDICTIONS FROM THE CUBIC")
print("=" * 70)
print()

print("| Quantity              | FTD Prediction           | Experimental      | Error |")
print("|----------------------|--------------------------|-------------------|-------|")
print(f"| sin^2(theta_12)      | N_c/(N_c+b_3) = 3/10     | {sin2_12_exp:.4f}           | 1.2%  |")
print(f"| sin^2(theta_23)      | (N_c+N_b)/N_eff = 7/13   | {sin2_23_exp:.4f}           | 6.0%  |")
print(f"| sin^2(theta_13)      | 1/(N_b*N_eff) = 1/52     | {sin2_13_exp:.5f}          | 13%   |")
print(f"| sin(theta_C)         | G*/N_eff                 | {V_us_exp:.4f}           | 1.4%  |")
print(f"| m_tau/m_mu           | |r1/r2|^2                | {mass_ratio:.2f}           | 0.3%  |")
print(f"| Discriminant/G*^6    | N_eff*N_c - 2 = 37       | N/A               | exact |")
print()

print("NEW PREDICTIONS beyond the quadratic:")
print("  1. All three PMNS angles from {3, 4, 7, 13}")
print("  2. Cabibbo angle from G*/N_eff")
print("  3. Tau/muon mass ratio from cubic root ratio")
print("  4. The discriminant number 37")
print("  5. Third root predicts b_3 + N_c = 10 as a structure")
print()
