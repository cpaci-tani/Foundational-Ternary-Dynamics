#!/usr/bin/env python3
"""
proof_meaning_weights_k_mapping.py
==================================

Validation script for the Meaning Weights to k mapping derivation (FTD-0245).
It verifies the algebraic identities mapping k to the weights alpha and beta,
as well as the polar phase angles, for k = 16, 1/2, and 4/G* at 50 decimal places.

Run:
  python scripts/proofs/proof_meaning_weights_k_mapping.py
"""

import sys
from mpmath import mp, mpf, gamma, sqrt, atan, degrees, cos

mp.dps = 50


def check_k(k, gstar):
    alpha = k * gstar**2 / 2
    delta = k**2 * gstar**4 - 4 * k * gstar**3
    
    if delta >= 0:
        beta = sqrt(delta) / 2
        k_calc = (4 / gstar) * (alpha**2 / (alpha**2 - beta**2))
        domain = "A/C (Real)"
    else:
        beta = sqrt(-delta) / 2
        k_calc = (4 / gstar) * (alpha**2 / (alpha**2 + beta**2))
        domain = "B (Complex)"
        
    diff = abs(k_calc - k)
    return k_calc, beta, alpha, diff, domain


def main():
    checks = []
    print("=== proof_meaning_weights_k_mapping.py : Verification of meaning weights mapping ===")

    gstar = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)
    print(f"\nG* = {gstar}\n")

    # 1. Test k = 16 (Domain A)
    k_val = mpf(16)
    k_calc, beta, alpha, diff, domain = check_k(k_val, gstar)
    print(f"[k = 16] Domain: {domain}")
    print(f"  alpha  = {alpha}")
    print(f"  beta   = {beta}")
    print(f"  k_calc = {k_calc} (diff = {diff})")
    checks.append(("k = 16 matches exactly", diff < mpf(10)**-45))

    # 2. Test k = 1/2 (Domain B)
    k_val = mpf(1) / 2
    k_calc, beta, alpha, diff, domain = check_k(k_val, gstar)
    theta = degrees(atan(beta / alpha))
    cos2_theta = cos(atan(beta / alpha))**2
    gstar_div_8 = gstar / 8
    
    print(f"\n[k = 1/2] Domain: {domain}")
    print(f"  alpha  = {alpha}")
    print(f"  beta   = {beta}")
    print(f"  k_calc = {k_calc} (diff = {diff})")
    print(f"  theta  = {theta} degrees")
    print(f"  cos^2(theta) = {cos2_theta}")
    print(f"  G*/8         = {gstar_div_8}")
    
    checks.append(("k = 1/2 matches exactly", diff < mpf(10)**-45))
    checks.append(("theta for k=1/2 is approx 52.54 degrees", abs(theta - mpf("52.54485824")) < mpf("1e-6")))
    checks.append(("cos^2(theta) == G*/8", abs(cos2_theta - gstar_div_8) < mpf(10)**-45))

    # 3. Test k = 4/G* (Domain C)
    k_val = 4 / gstar
    k_calc, beta, alpha, diff, domain = check_k(k_val, gstar)
    print(f"\n[k = 4/G*] Domain: {domain}")
    print(f"  alpha  = {alpha}")
    print(f"  beta   = {beta}")
    print(f"  k_calc = {k_calc} (diff = {diff})")
    checks.append(("k = 4/G* matches exactly", diff < mpf(10)**-45))
    checks.append(("beta is exactly zero at boundary", abs(beta) < mpf(10)**-45))

    print("\n=== RESULTS ===")
    all_pass = True
    for name, ok in checks:
        print(f"  [{'PASS' if ok else 'FAIL'}] {name}")
        all_pass = all_pass and ok
    print("=" * 72)
    
    if all_pass:
        print("ALL CHECKS PASS -- The mapping k = (4/G*) * alpha^2 / (alpha^2 -/+ beta^2)")
        print("correctly unifies Domain A and Domain B, and recovers the theta = 52.54 phase angle.")
        return 0
    else:
        print("FAILURE -- One or more checks failed.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
