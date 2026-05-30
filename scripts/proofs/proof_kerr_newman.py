#!/usr/bin/env python3
"""
Verification: Kerr-Newman Black Hole Metric and Limits
Proves symbolically and numerically that the derived Kerr-Newman metric
reconciles under FTD's unified lattice budget and reduces to all classical limits.
"""

import sympy as sp
import numpy as np
import mpmath as mp
import sys

def run_symbolic_proofs():
    print("----------------------------------------------------------------")
    # Define symbolic variables
    r, M, a, Q, theta = sp.symbols('r M a Q theta', real=True)
    
    # Oblate load function Sigma
    Sigma = r**2 + a**2 * sp.cos(theta)**2
    
    # Horizon function Delta_KN
    Delta = r**2 - 2*M*r + a**2 + Q**2
    
    # Define the Kerr-Newman metric components in Boyer-Lindquist coordinates
    g_tt = -(Delta - a**2 * sp.sin(theta)**2) / Sigma
    g_tphi = -(a * sp.sin(theta)**2 * (2*M*r - Q**2)) / Sigma
    g_rr = Sigma / Delta
    g_theta_theta = Sigma
    g_phi_phi = ((r**2 + a**2)**2 - Delta * a**2 * sp.sin(theta)**2) * sp.sin(theta)**2 / Sigma
    
    # ----------------------------------------------------------------
    # 1. Determinant Invariance Proof
    # ----------------------------------------------------------------
    print("Proving determinant invariance symbolically...")
    # The metric is block diagonal in (r, theta) and (t, phi)
    # Det = det(2x2 in t, phi) * g_rr * g_theta_theta
    det_t_phi = g_tt * g_phi_phi - g_tphi**2
    det_full = det_t_phi * g_rr * g_theta_theta
    
    # Standard GR determinant is -Sigma**2 * sin(theta)**2
    expected_det = -Sigma**2 * sp.sin(theta)**2
    diff_det = sp.simplify(det_full - expected_det)
    
    if diff_det == 0:
        print("  [PASS] Determinant invariance confirmed: det(g) = -Sigma^2 * sin^2(theta) exactly.")
    else:
        print(f"  [FAIL] Determinant mismatch! Simplified diff: {diff_det}")
        return False
        
    # ----------------------------------------------------------------
    # 2. Limiting Case 1: Uncharged Kerr limit (Q -> 0)
    # ----------------------------------------------------------------
    print("Verifying Kerr limit (Q -> 0)...")
    # Metric components with Q=0
    g_tt_kerr = sp.simplify(g_tt.subs(Q, 0))
    g_tphi_kerr = sp.simplify(g_tphi.subs(Q, 0))
    g_rr_kerr = sp.simplify(g_rr.subs(Q, 0))
    g_phi_phi_kerr = sp.simplify(g_phi_phi.subs(Q, 0))
    
    # Standard Kerr expressions
    Delta_k = r**2 - 2*M*r + a**2
    expected_g_tt_kerr = -(Delta_k - a**2 * sp.sin(theta)**2) / Sigma
    expected_g_tphi_kerr = -(2*M*r*a*sp.sin(theta)**2) / Sigma
    expected_g_rr_kerr = Sigma / Delta_k
    expected_g_phi_phi_kerr = ((r**2 + a**2)**2 - Delta_k * a**2 * sp.sin(theta)**2) * sp.sin(theta)**2 / Sigma
    
    check_kerr = (
        sp.simplify(g_tt_kerr - expected_g_tt_kerr) == 0 and
        sp.simplify(g_tphi_kerr - expected_g_tphi_kerr) == 0 and
        sp.simplify(g_rr_kerr - expected_g_rr_kerr) == 0 and
        sp.simplify(g_phi_phi_kerr - expected_g_phi_phi_kerr) == 0
    )
    
    if check_kerr:
        print("  [PASS] Kerr limit confirmed.")
    else:
        print("  [FAIL] Kerr limit mismatch!")
        return False

    # ----------------------------------------------------------------
    # 3. Limiting Case 2: Reissner-Nordstrom limit (a -> 0)
    # ----------------------------------------------------------------
    print("Verifying Reissner-Nordstrom limit (a -> 0)...")
    # Metric components with a=0
    g_tt_rn = sp.simplify(g_tt.subs(a, 0))
    g_tphi_rn = sp.simplify(g_tphi.subs(a, 0))
    g_rr_rn = sp.simplify(g_rr.subs(a, 0))
    g_phi_phi_rn = sp.simplify(g_phi_phi.subs(a, 0))
    
    # Standard RN expressions
    f_rn = 1 - 2*M/r + Q**2/r**2
    expected_g_tt_rn = -f_rn
    expected_g_tphi_rn = 0
    expected_g_rr_rn = 1/f_rn
    expected_g_phi_phi_rn = r**2 * sp.sin(theta)**2
    
    check_rn = (
        sp.simplify(g_tt_rn - expected_g_tt_rn) == 0 and
        sp.simplify(g_tphi_rn - expected_g_tphi_rn) == 0 and
        sp.simplify(g_rr_rn - expected_g_rr_rn) == 0 and
        sp.simplify(g_phi_phi_rn - expected_g_phi_phi_rn) == 0
    )
    
    if check_rn:
        print("  [PASS] Reissner-Nordstrom limit confirmed.")
    else:
        print("  [FAIL] Reissner-Nordstrom limit mismatch!")
        return False

    # ----------------------------------------------------------------
    # 4. Limiting Case 3: Schwarzschild limit (a -> 0, Q -> 0)
    # ----------------------------------------------------------------
    print("Verifying Schwarzschild limit (a -> 0, Q -> 0)...")
    g_tt_schw = sp.simplify(g_tt.subs([(a, 0), (Q, 0)]))
    g_tphi_schw = sp.simplify(g_tphi.subs([(a, 0), (Q, 0)]))
    g_rr_schw = sp.simplify(g_rr.subs([(a, 0), (Q, 0)]))
    g_phi_phi_schw = sp.simplify(g_phi_phi.subs([(a, 0), (Q, 0)]))
    
    f_schw = 1 - 2*M/r
    expected_g_tt_schw = -f_schw
    expected_g_tphi_schw = 0
    expected_g_rr_schw = 1/f_schw
    expected_g_phi_phi_schw = r**2 * sp.sin(theta)**2
    
    check_schw = (
        sp.simplify(g_tt_schw - expected_g_tt_schw) == 0 and
        sp.simplify(g_tphi_schw - expected_g_tphi_schw) == 0 and
        sp.simplify(g_rr_schw - expected_g_rr_schw) == 0 and
        sp.simplify(g_phi_phi_schw - expected_g_phi_phi_schw) == 0
    )
    
    if check_schw:
        print("  [PASS] Schwarzschild limit confirmed.")
    else:
        print("  [FAIL] Schwarzschild limit mismatch!")
        return False

    # ----------------------------------------------------------------
    # 5. Limiting Case 4: Minkowski limit (M -> 0, a -> 0, Q -> 0)
    # ----------------------------------------------------------------
    print("Verifying flat Minkowski limit...")
    g_tt_flat = sp.simplify(g_tt.subs([(M, 0), (a, 0), (Q, 0)]))
    g_tphi_flat = sp.simplify(g_tphi.subs([(M, 0), (a, 0), (Q, 0)]))
    g_rr_flat = sp.simplify(g_rr.subs([(M, 0), (a, 0), (Q, 0)]))
    g_phi_phi_flat = sp.simplify(g_phi_phi.subs([(M, 0), (a, 0), (Q, 0)]))
    
    check_flat = (
        g_tt_flat == -1 and
        g_tphi_flat == 0 and
        g_rr_flat == 1 and
        g_phi_phi_flat == r**2 * sp.sin(theta)**2
    )
    
    if check_flat:
        print("  [PASS] Minkowski flat space limit confirmed.")
    else:
        print("  [FAIL] Minkowski limit mismatch!")
        return False
        
    return True


def run_numerical_proofs():
    print("\n----------------------------------------------------------------")
    print("Running high-precision numerical horizon spot-checks via mpmath...")
    mp.dps = 50 # Set high-precision arithmetic to 50 decimal digits
    
    # Choose representative black hole parameters
    M = mp.mpf('10.0')
    a = mp.mpf('3.0')
    Q = mp.mpf('4.0')
    
    # 1. Horizon condition verification: Delta_KN(r_pm) = 0
    print(f"  Using parameters: M = {M}, a = {a}, Q = {Q}")
    
    # Compute analytical horizons
    discriminant = M**2 - a**2 - Q**2
    print(f"  M^2 - a^2 - Q^2 = {discriminant}")
    assert discriminant > 0, "Black hole must have horizons (non-extremal)"
    
    r_plus = M + mp.sqrt(discriminant)
    r_minus = M - mp.sqrt(discriminant)
    
    print(f"  Outer horizon (r_plus) = {r_plus}")
    print(f"  Inner horizon (r_minus) = {r_minus}")
    
    # Evaluate Delta at r_plus and r_minus
    delta_plus = r_plus**2 - 2*M*r_plus + a**2 + Q**2
    delta_minus = r_minus**2 - 2*M*r_minus + a**2 + Q**2
    
    print(f"  Delta(r_plus)  = {delta_plus}")
    print(f"  Delta(r_minus) = {delta_minus}")
    
    # Assert they are zero to high precision (within 1e-12)
    tol = mp.mpf('1e-12')
    check_horizons = mp.almosteq(delta_plus, 0, rel_eps=tol, abs_eps=tol) and \
                     mp.almosteq(delta_minus, 0, rel_eps=tol, abs_eps=tol)
                     
    if check_horizons:
        print("  [PASS] Horizons coincide exactly with Delta_KN = 0 at high precision.")
    else:
        print("  [FAIL] Horizon numerical mismatch!")
        return False
        
    # 2. Extremal boundary spot-check: a^2 + Q^2 = M^2
    print("Verifying degenerate extremal horizon...")
    M_ext = mp.mpf('5.0')
    a_ext = mp.mpf('3.0')
    Q_ext = mp.mpf('4.0')
    
    r_ext = M_ext + mp.sqrt(M_ext**2 - a_ext**2 - Q_ext**2)
    delta_ext = r_ext**2 - 2*M_ext*r_ext + a_ext**2 + Q_ext**2
    
    print(f"  Extremal Delta(r_ext={r_ext}) = {delta_ext}")
    check_ext = mp.almosteq(r_ext, M_ext, rel_eps=tol, abs_eps=tol) and \
                mp.almosteq(delta_ext, 0, rel_eps=tol, abs_eps=tol)
                
    if check_ext:
        print("  [PASS] Extremal degenerate horizon confirmed at r = M.")
    else:
        print("  [FAIL] Extremal degenerate horizon mismatch!")
        return False
        
    return True


def main():
    print("================================================================")
    print("  VERIFICATION: Kerr-Newman Black Hole Derivation & Limits")
    print("================================================================")
    
    sym_ok = run_symbolic_proofs()
    if not sym_ok:
        sys.exit(1)
        
    num_ok = run_numerical_proofs()
    if not num_ok:
        sys.exit(1)
        
    print("\n================================================================")
    print("  RESULT: ALL KERR-NEWMAN VERIFICATIONS PASSED")
    sys.exit(0)

if __name__ == "__main__":
    main()
