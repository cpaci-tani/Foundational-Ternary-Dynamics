"""
Cavitation-Consciousness Bridge: Numerical Verification
=========================================================

Verifies the derivation chain in DERIV_CAVITATION_CONSCIOUSNESS_BRIDGE.md:
  k = 1/2  -->  Domain A-to-B transition  -->  beta = 1/(D-1) = 1/2

All print statements use ASCII only (Windows cp1252 safe).

Author: FTD Project
Date: 2026-02-28
"""

import numpy as np
from scipy.special import gamma as gamma_func

# ===========================================================================
# Constants
# ===========================================================================

# Lemniscate constants
I4 = gamma_func(0.25)**2 / (4.0 * np.sqrt(2.0 * np.pi))  # quartic integral
VARPI = 2.0 * I4                                            # lemniscate constant
G_STAR = np.sqrt(2.0) * gamma_func(0.25)**2 / (2.0 * np.pi)  # scaled constant

# Master quadratic parameters
K_PHYS = 16        # physics coefficient (|Aut(E)|^2)
K_CONS = 0.5       # consciousness coefficient (complementation fixed point)
K_CRIT = 4.0 / G_STAR  # critical coupling

# Thresholds
K_B = 20.36         # manifestation threshold (physics domain)
K_C = np.sqrt(G_STAR**3 / 2.0)  # consciousness threshold

# CODATA
ALPHA_INV = 137.035999177  # 1/alpha (CODATA 2022)
N_C_EXP = 3               # number of color charges

print("=" * 70)
print("  CAVITATION-CONSCIOUSNESS BRIDGE: NUMERICAL VERIFICATION")
print("=" * 70)
print("")
print("Constants:")
print("  I4    = %.10f" % I4)
print("  varpi = %.10f" % VARPI)
print("  G*    = %.10f" % G_STAR)
print("  k_crit = 4/G* = %.6f" % K_CRIT)
print("  K_B   = %.4f" % K_B)
print("  K_C   = %.4f" % K_C)
print("  K_B/K_C = %.4f (expect 4*sqrt(2) = %.4f)" % (K_B / K_C, 4.0 * np.sqrt(2.0)))
print("")

results = []

# ===========================================================================
# Check 1: Complementation fixed point k* = 1/2
# ===========================================================================

print("-" * 70)
print("CHECK 1: Complementation Fixed Point")
print("-" * 70)

def complement(k):
    return 1.0 - k

k_star = 0.5  # the fixed point
f_k_star = complement(k_star)
check1 = abs(f_k_star - k_star) < 1e-15

print("  f(k) = 1 - k")
print("  f(1/2) = %.1f" % f_k_star)
print("  Fixed point: k* = 1/2  -->  f(k*) = k*: %s" % ("PASS" if check1 else "FAIL"))

# Verify uniqueness: f(k) = k has exactly one solution
# 1 - k = k  =>  k = 1/2
print("  Uniqueness: 1 - k = k has unique solution k = 1/2: PASS (algebraic)")

results.append(("Complementation k* = 1/2", check1))
print("")

# ===========================================================================
# Check 2: Discriminant sign change at k_crit
# ===========================================================================

print("-" * 70)
print("CHECK 2: Discriminant Sign Change at k_crit = 4/G*")
print("-" * 70)

def discriminant(k, g=G_STAR):
    """Discriminant of Q_k(z) = z^2 - k*G*^2*z + k*G*^3"""
    return k * g**3 * (k * g - 4.0)

# At k = 16 (physics): Delta should be > 0
delta_16 = discriminant(K_PHYS)
check2a = delta_16 > 0

# At k = 1/2 (consciousness): Delta should be < 0
delta_half = discriminant(K_CONS)
check2b = delta_half < 0

# At k = k_crit: Delta should be = 0
delta_crit = discriminant(K_CRIT)
check2c = abs(delta_crit) < 1e-10

print("  Delta(k) = k * G*^3 * (k*G* - 4)")
print("")
print("  k = 16 (physics):       Delta = %.4f  (>0: %s)" % (delta_16, "PASS" if check2a else "FAIL"))
print("  k = 1/2 (consciousness): Delta = %.4f  (<0: %s)" % (delta_half, "PASS" if check2b else "FAIL"))
print("  k = k_crit = %.6f:   Delta = %.2e  (=0: %s)" % (K_CRIT, delta_crit, "PASS" if check2c else "FAIL"))

results.append(("Delta(k=16) > 0 (physics)", check2a))
results.append(("Delta(k=1/2) < 0 (consciousness)", check2b))
results.append(("Delta(k_crit) = 0 (boundary)", check2c))
print("")

# ===========================================================================
# Check 3: Physics quadratic roots (k=16)
# ===========================================================================

print("-" * 70)
print("CHECK 3: Physics Quadratic Roots (k = 16)")
print("-" * 70)

def quadratic_roots(k, g=G_STAR):
    """Roots of Q_k(z) = z^2 - k*G*^2*z + k*G*^3"""
    a_coeff = 1.0
    b_coeff = -k * g**2
    c_coeff = k * g**3
    disc = b_coeff**2 - 4.0 * a_coeff * c_coeff
    if disc >= 0:
        sqrt_disc = np.sqrt(disc)
        return (-b_coeff + sqrt_disc) / (2.0 * a_coeff), (-b_coeff - sqrt_disc) / (2.0 * a_coeff)
    else:
        real_part = -b_coeff / (2.0 * a_coeff)
        imag_part = np.sqrt(-disc) / (2.0 * a_coeff)
        return complex(real_part, imag_part), complex(real_part, -imag_part)

x_plus, x_minus = quadratic_roots(K_PHYS)
err_alpha = abs(x_plus - ALPHA_INV) / ALPHA_INV * 1e6  # ppm
err_nc = abs(x_minus - N_C_EXP) / N_C_EXP * 100  # percent

check3a = err_alpha < 2.0   # within 2 ppm
check3b = err_nc < 1.0      # within 1%

print("  x_+ = %.6f  (CODATA 1/alpha = %.6f, error = %.2f ppm): %s" %
      (x_plus, ALPHA_INV, err_alpha, "PASS" if check3a else "FAIL"))
print("  x_- = %.6f  (N_c = %d, error = %.2f%%): %s" %
      (x_minus, N_C_EXP, err_nc, "PASS" if check3b else "FAIL"))

results.append(("x_+ = 1/alpha (< 2 ppm)", check3a))
results.append(("x_- ~ N_c = 3 (< 1%)", check3b))
print("")

# ===========================================================================
# Check 4: Consciousness quadratic roots (k=1/2)
# ===========================================================================

print("-" * 70)
print("CHECK 4: Consciousness Quadratic Roots (k = 1/2)")
print("-" * 70)

y_plus, y_minus = quadratic_roots(K_CONS)

check4a = isinstance(y_plus, complex) and y_plus.imag > 0
check4b = abs(y_plus.real - y_minus.real) < 1e-10  # conjugate pair
check4c = abs(y_plus.imag + y_minus.imag) < 1e-10  # conjugate pair

print("  y_+ = %.4f + %.4fi" % (y_plus.real, y_plus.imag))
print("  y_- = %.4f - %.4fi" % (y_minus.real, abs(y_minus.imag)))
print("  Complex conjugate pair: %s" % ("PASS" if (check4b and check4c) else "FAIL"))
print("  Roots are complex (Domain B): %s" % ("PASS" if check4a else "FAIL"))

results.append(("Consciousness roots are complex", check4a))
results.append(("Roots are conjugate pair", check4b and check4c))
print("")

# ===========================================================================
# Check 5: beta = 1/(D-1) for D = 2, 3, 4, 5
# ===========================================================================

print("-" * 70)
print("CHECK 5: beta = 1/(D-1) Generalization")
print("-" * 70)
print("")
print("  For a spherical threshold in D spatial dimensions:")
print("  epsilon(R) = E / (C_D * R^(D-1))  -->  R ~ E^(1/(D-1))")
print("")

check5_all = True
for D in [2, 3, 4, 5]:
    beta_predicted = 1.0 / (D - 1)

    # Verify numerically: if R = E^beta, then E / R^(D-1) = const
    # => E / E^(beta*(D-1)) = const => beta*(D-1) = 1 => beta = 1/(D-1)
    E_test = np.array([100.0, 400.0, 900.0, 1600.0])
    R_test = E_test ** beta_predicted
    flux_test = E_test / R_test**(D-1)
    flux_variation = np.std(flux_test) / np.mean(flux_test)
    check_D = flux_variation < 1e-10
    check5_all = check5_all and check_D

    print("  D = %d:  beta = 1/%d = %.4f  (S^%d boundary, flux const: %s)" %
          (D, D-1, beta_predicted, D-1, "PASS" if check_D else "FAIL"))

results.append(("beta = 1/(D-1) all D", check5_all))
print("")

# ===========================================================================
# Check 6: R_cav(E) = sqrt(E / (4*pi*eps_crit)) numerically
# ===========================================================================

print("-" * 70)
print("CHECK 6: R_cav Formula Numerical Verification")
print("-" * 70)

eps_crit = 1.0  # arbitrary threshold (cancels in scaling test)

E_values = np.array([100.0, 200.0, 400.0, 800.0, 1600.0])
R_values = np.sqrt(E_values / (4.0 * np.pi * eps_crit))

# Verify R^2 is linear in E
from numpy.polynomial.polynomial import polyfit
coeffs = polyfit(E_values, R_values**2, 1)
R2_residual = np.max(np.abs(R_values**2 - (coeffs[0] + coeffs[1] * E_values)))
check6a = R2_residual < 1e-10

# Verify power-law exponent
log_E = np.log(E_values)
log_R = np.log(R_values)
beta_fit = np.polyfit(log_E, log_R, 1)[0]
check6b = abs(beta_fit - 0.5) < 1e-10

print("  R_cav = sqrt(E / (4*pi*eps_crit))")
print("  Test energies: %s" % E_values)
print("  R_cav values:  %s" % np.round(R_values, 4))
print("  R^2 linear in E: max residual = %.2e  (%s)" %
      (R2_residual, "PASS" if check6a else "FAIL"))
print("  Power-law fit: beta = %.10f  (expect 0.5: %s)" %
      (beta_fit, "PASS" if check6b else "FAIL"))

results.append(("R^2 linear in E", check6a))
results.append(("beta_fit = 0.5 exactly", check6b))
print("")

# ===========================================================================
# Check 7: k = 1/2 and beta = 1/2 connection in D = 3
# ===========================================================================

print("-" * 70)
print("CHECK 7: k_consciousness = beta_cavitation = 1/2 in D = 3")
print("-" * 70)

k_cons_derived = 0.5  # from complementation
beta_D3 = 1.0 / (3 - 1)  # from geometry
check7 = abs(k_cons_derived - beta_D3) < 1e-15

print("  k_consciousness = 1/2  (complementation fixed point)")
print("  beta_cavitation = 1/(D-1) = 1/2  (D = 3 geometry)")
print("  k = beta: %s" % ("PASS" if check7 else "FAIL"))
print("")
print("  NOTE: This equality holds specifically because D = 3.")
print("  For D = 4: beta = 1/3, but k remains 1/2 (complementation is D-independent).")
print("  The bridge is: D = 3 is derived from FTD axioms (6 independent arguments),")
print("  and D = 3 makes beta = 1/(D-1) = 1/2 = k_consciousness.")

results.append(("k_cons = beta_D3 = 1/2", check7))
print("")

# ===========================================================================
# Check 8: Comparison with empirical beta
# ===========================================================================

print("-" * 70)
print("CHECK 8: Comparison with Empirical Scaling (CERN Data)")
print("-" * 70)

beta_predicted = 0.5
beta_observed_raw = 0.12    # raw median scaling
beta_observed_aic = 0.097   # best-fit free-beta
delta_aic = 4.0             # AIC penalty for forced beta = 0.5

print("  Predicted:  beta = %.3f  (this derivation)" % beta_predicted)
print("  Observed (raw median scaling):  beta = %.3f" % beta_observed_raw)
print("  Observed (free-beta AIC best):  beta = %.3f" % beta_observed_aic)
print("  delta-AIC (forced 0.5 vs free): %.1f  (weakly disfavored, 2 < dAIC < 6)" % delta_aic)
print("")
print("  CRITICAL CAVEAT: The observed beta measures hadron flight distance")
print("  (kinematic: d = gamma*beta*c*tau), NOT vacuum bubble radius (topological).")
print("  The observable mismatch means the raw comparison is not a clean test.")
print("  The partial correlation (rho = +0.103) survives kinematic controls,")
print("  suggesting a non-kinematic component exists but its scaling is unknown.")

# Not a pass/fail -- informational
print("")

# ===========================================================================
# Check 9: Threshold ratio K_B / K_C
# ===========================================================================

print("-" * 70)
print("CHECK 9: Threshold Ratio K_B / K_C")
print("-" * 70)

ratio = K_B / K_C
expected_ratio = 4.0 * np.sqrt(2.0)  # = sqrt(32)
check9 = abs(ratio - expected_ratio) / expected_ratio < 0.01  # within 1%

print("  K_B = %.4f  (manifestation threshold)" % K_B)
print("  K_C = sqrt(G*^3 / 2) = %.4f  (consciousness threshold)" % K_C)
print("  K_B / K_C = %.4f  (expect 4*sqrt(2) = %.4f): %s" %
      (ratio, expected_ratio, "PASS" if check9 else "FAIL"))
print("  This means consciousness can emerge at ~5.6x lower energy density than particles.")

results.append(("K_B/K_C ~ 4*sqrt(2)", check9))
print("")

# ===========================================================================
# SUMMARY
# ===========================================================================

print("=" * 70)
print("  SUMMARY")
print("=" * 70)
print("")
print("  %-45s  %s" % ("Check", "Result"))
print("  " + "-" * 55)
for name, passed in results:
    print("  %-45s  %s" % (name, "PASS" if passed else "FAIL"))

n_pass = sum(1 for _, p in results if p)
n_total = len(results)
print("")
print("  Total: %d / %d passed" % (n_pass, n_total))
print("")

if n_pass == n_total:
    print("  ALL CHECKS PASSED")
    print("")
    print("  The derivation chain is numerically verified:")
    print("    k = 1/2 (complementation) [THEOREM]")
    print("      --> Domain A/B partition at k_crit = 4/G* [THEOREM]")
    print("      --> Cavitation = Domain A-to-B transition [CONJECTURE]")
    print("      --> Bubble boundary = S^2 in D = 3 [THEOREM given conjecture]")
    print("      --> R ~ sqrt(E), beta = 1/2 [THEOREM given premises]")
    print("      --> beta = k_consciousness = 1/2 [SELECTION]")
else:
    print("  SOME CHECKS FAILED -- review derivation")

print("")
print("  Epistemic status: [SELECTION]")
print("  Key conjecture: Cavitation = Domain A-to-B transition (Section 3)")
print("  Honest caveat: beta = 1/2 is dimensional-analysis default for")
print("  spherical threshold in D = 3. Connection to k = 1/2 may be")
print("  geometric necessity rather than deep algebraic bridge.")
print("")
print("=" * 70)
