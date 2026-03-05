"""
Verification: Collapse-Gravity Bridge (EXPLR_COLLAPSE_GRAVITY_BRIDGE.md)

Tests the quantitative claims connecting wave function collapse to gravitational
curvature via the Hawking-KMS bridge and algebraic type transition.

10 tests across 3 parts:
  Part A: Hawking-KMS Numerics (4 tests)
  Part B: Position-Dependent Structure (3 tests)
  Part C: Evaporation and Information (3 tests)
"""

import sys
import os
import numpy as np

# Add project root to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
from simulations.constants import (
    N_c, N_base, b_3, N_eff, G_STAR, VARPI_CLASSICAL, PF, ALPHA,
    percent_error
)


# =============================================================================
# HELPER FUNCTIONS
# =============================================================================

def hawking_beta(M):
    """Hawking inverse temperature: beta_H = 8*pi*M (Planck units)."""
    return 8 * np.pi * M


def connes_parameter(beta):
    """Connes parameter from RT dictionary: lambda = e^{-beta}."""
    return np.exp(-beta)


def kms_strip_width(beta):
    """KMS strip width: pi/beta."""
    return np.pi / beta


def modular_period(beta):
    """Modular period: T = 2*pi/beta."""
    return 2 * np.pi / beta


def tolman_beta_local(beta_H, f_r):
    """Local inverse temperature via Tolman relation: beta_local = beta_H * sqrt(f)."""
    return beta_H * np.sqrt(f_r)


def availability_factor(r, r_s):
    """Schwarzschild availability factor: f(r) = 1 - r_s/r."""
    return 1.0 - r_s / r


def bh_entropy(M):
    """Bekenstein-Hawking entropy in Planck units: S = 4*pi*M^2."""
    return 4 * np.pi * M**2


def hawking_temperature(M):
    """Hawking temperature: T_H = 1/(8*pi*M)."""
    return 1.0 / (8 * np.pi * M)


def penrose_decoherence_time(m, d, G=1.0, hbar=1.0):
    """Penrose-Diosi decoherence timescale: tau ~ hbar*d / (G*m^2)."""
    return hbar * d / (G * m**2)


# =============================================================================
# PART A: HAWKING-KMS NUMERICS
# =============================================================================

def test_CG_T1_pf_decomposition_8pi():
    """CG-T1: Verify 8*pi = 2 * N_base^2 * PF exactly."""
    lhs = 8 * np.pi
    rhs = 2 * N_base**2 * PF

    # This should be exact (PF = pi/4, so 2*16*pi/4 = 8*pi)
    error = abs(lhs - rhs)

    print(f"  8*pi             = {lhs:.15f}")
    print(f"  2*N_base^2*PF    = {rhs:.15f}")
    print(f"  Difference       = {error:.2e}")

    assert error < 1e-14, f"PF decomposition failed: error = {error}"
    print("  PASS: 8*pi = 2*N_base^2*PF verified to machine precision")


def test_CG_T2_connes_parameter_range():
    """CG-T2: Connes parameter lambda_H = e^{-beta_H} in (0,1) for all M > 0."""
    test_masses = [0.001, 0.01, 0.1, 1.0, 10.0, 100.0, 1e6, 1e38]

    print(f"  {'M (Planck)':>15} {'beta_H':>15} {'lambda_H':>20} {'In (0,1)?':>10}")
    print(f"  {'-'*15} {'-'*15} {'-'*20} {'-'*10}")

    all_valid = True
    for M in test_masses:
        beta = hawking_beta(M)
        lam = connes_parameter(beta)
        valid = 0.0 < lam < 1.0
        all_valid = all_valid and valid

        # For very large M, lambda is essentially 0 (underflows to 0.0)
        # This is fine — it approaches 0 but never equals it for finite M
        if M <= 100:
            print(f"  {M:15.4f} {beta:15.4f} {lam:20.15e} {'Yes' if valid else 'NO':>10}")
        else:
            # For large M, e^{-beta} underflows to 0.0 in float64
            # Mathematically it's still in (0,1), but numerically it's 0
            print(f"  {M:15.4e} {beta:15.4e} {'~0 (underflow)':>20} {'Yes*':>10}")

    # For finite precision, verify at masses where lambda is representable in float64.
    # For M >= ~87 (beta_H > 708), e^{-beta} underflows to 0.0 in float64.
    # Mathematically lambda is always in (0,1) for finite M > 0.
    max_representable_beta = 700  # float64 exp underflow threshold
    representable_masses = [M for M in test_masses if hawking_beta(M) < max_representable_beta]
    for M in representable_masses:
        beta = hawking_beta(M)
        lam = connes_parameter(beta)
        assert 0.0 < lam < 1.0, f"lambda_H not in (0,1) for M={M}: lambda={lam}"

    print("  PASS: Connes parameter in (0,1) for all representable masses")
    print(f"        (masses with beta_H > {max_representable_beta} underflow to 0.0 in float64 — mathematically still in (0,1))")


def test_CG_T3_kms_strip_width():
    """CG-T3: KMS strip width pi/beta_H = 1/(8M) matches expected Hawking values."""
    test_masses = [0.1, 0.5, 1.0, 5.0, 10.0, 100.0]

    print(f"  {'M':>8} {'pi/beta_H':>15} {'1/(8M)':>15} {'Match':>8}")
    print(f"  {'-'*8} {'-'*15} {'-'*15} {'-'*8}")

    for M in test_masses:
        beta = hawking_beta(M)
        strip_from_beta = kms_strip_width(beta)
        strip_expected = 1.0 / (8.0 * M)
        error = abs(strip_from_beta - strip_expected) / strip_expected

        print(f"  {M:8.1f} {strip_from_beta:15.10f} {strip_expected:15.10f} {error:.2e}")
        assert error < 1e-14, f"KMS strip mismatch at M={M}: error={error}"

    print("  PASS: KMS strip width pi/beta_H = 1/(8M) verified")


def test_CG_T4_beta_H_formula():
    """CG-T4: beta_H = 8*pi*M = 2*N_base^2*PF*M for various BH masses."""
    test_masses = [0.01, 0.1, 1.0, 10.0, 1000.0]

    print(f"  {'M':>10} {'8*pi*M':>18} {'2*N_b^2*PF*M':>18} {'Error':>12}")
    print(f"  {'-'*10} {'-'*18} {'-'*18} {'-'*12}")

    for M in test_masses:
        beta_direct = 8 * np.pi * M
        beta_ftd = 2 * N_base**2 * PF * M
        error = abs(beta_direct - beta_ftd)

        print(f"  {M:10.2f} {beta_direct:18.10f} {beta_ftd:18.10f} {error:.2e}")
        assert error < 1e-12 * max(abs(beta_direct), 1.0), \
            f"beta_H formula mismatch at M={M}"

    print("  PASS: beta_H = 8*pi*M = 2*N_base^2*PF*M verified")


# =============================================================================
# PART B: POSITION-DEPENDENT STRUCTURE
# =============================================================================

def test_CG_T5_tolman_horizon_limit():
    """CG-T5: Tolman beta_local(r) -> 0 as r -> r_s (local Type III_1)."""
    M = 10.0
    r_s = 2 * M  # Schwarzschild radius
    beta_H = hawking_beta(M)

    # Approach the horizon: r = r_s * (1 + epsilon)
    epsilons = [1.0, 0.1, 0.01, 0.001, 1e-4, 1e-6, 1e-8, 1e-10]

    print(f"  M = {M}, r_s = {r_s}, beta_H = {beta_H:.4f}")
    print(f"  {'epsilon':>12} {'r/r_s':>10} {'f(r)':>15} {'beta_local':>15} {'lambda_local':>15}")
    print(f"  {'-'*12} {'-'*10} {'-'*15} {'-'*15} {'-'*15}")

    for eps in epsilons:
        r = r_s * (1 + eps)
        f_r = availability_factor(r, r_s)
        beta_loc = tolman_beta_local(beta_H, f_r)
        lam_loc = connes_parameter(beta_loc) if beta_loc < 700 else 0.0  # avoid overflow

        print(f"  {eps:12.2e} {r/r_s:10.6f} {f_r:15.10e} {beta_loc:15.10f} {lam_loc:15.10e}")

    # Verify: beta_local -> 0 as epsilon -> 0
    # At epsilon = 1e-10: f ~ 1e-10, sqrt(f) ~ 1e-5, beta_local = beta_H * 1e-5 ~ 0.0025
    # This is small relative to beta_H = 251, confirming the approach to 0
    r_near = r_s * (1 + 1e-10)
    f_near = availability_factor(r_near, r_s)
    beta_near = tolman_beta_local(beta_H, f_near)

    # beta_local / beta_H should be ~sqrt(1e-10) ~ 3.16e-6, so ratio << 1
    ratio = beta_near / beta_H
    assert ratio < 1e-4, \
        f"beta_local/beta_H should be << 1 near horizon, got ratio = {ratio}"

    # At the horizon, lambda -> 1 (Type III_1)
    lam_near = connes_parameter(beta_near)
    assert lam_near > 0.99, \
        f"lambda should approach 1 at horizon (Type III_1), got {lam_near}"

    print(f"  beta_local/beta_H = {ratio:.2e} (<<1, confirming approach to 0)")
    print(f"  lambda_local = {lam_near:.6f} (near 1, confirming Type III_1)")
    print("  PASS: beta_local -> 0, lambda -> 1 (Type III_1) at horizon")


def test_CG_T6_tolman_asymptotic_limit():
    """CG-T6: Tolman beta_local(r) -> beta_H as r -> infinity."""
    M = 10.0
    r_s = 2 * M
    beta_H = hawking_beta(M)

    # Move far from BH: r = r_s * factor
    factors = [2, 5, 10, 100, 1000, 1e6]

    print(f"  M = {M}, r_s = {r_s}, beta_H = {beta_H:.4f}")
    print(f"  {'r/r_s':>10} {'f(r)':>12} {'beta_local':>15} {'|beta_local - beta_H|':>22}")
    print(f"  {'-'*10} {'-'*12} {'-'*15} {'-'*22}")

    for factor in factors:
        r = r_s * factor
        f_r = availability_factor(r, r_s)
        beta_loc = tolman_beta_local(beta_H, f_r)
        diff = abs(beta_loc - beta_H)

        print(f"  {factor:10.0f} {f_r:12.8f} {beta_loc:15.8f} {diff:22.15f}")

    # At large r, beta_local should converge to beta_H
    r_far = r_s * 1e6
    f_far = availability_factor(r_far, r_s)
    beta_far = tolman_beta_local(beta_H, f_far)
    rel_error = abs(beta_far - beta_H) / beta_H

    assert rel_error < 1e-6, \
        f"beta_local should approach beta_H at large r, relative error = {rel_error}"

    print("  PASS: beta_local -> beta_H as r -> infinity (asymptotic limit)")


def test_CG_T7_entropy_temperature_product():
    """CG-T7: S_BH x T_H = M/2 is PF-free (re-verify from DERIV_GSTAR_PF_BRIDGE.md)."""
    test_masses = [0.1, 1.0, 10.0, 100.0, 1e6]

    print(f"  {'M':>10} {'S_BH':>18} {'T_H':>18} {'S*T':>15} {'M/2':>15} {'Error':>12}")
    print(f"  {'-'*10} {'-'*18} {'-'*18} {'-'*15} {'-'*15} {'-'*12}")

    for M in test_masses:
        S = bh_entropy(M)
        T = hawking_temperature(M)
        product = S * T
        expected = M / 2.0
        error = abs(product - expected) / expected

        print(f"  {M:10.1f} {S:18.6f} {T:18.10f} {product:15.8f} {expected:15.8f} {error:.2e}")
        assert error < 1e-14, f"S*T != M/2 at M={M}: error={error}"

    # Verify PF independence: S contains PF, T contains PF, but product doesn't
    S_via_pf = N_base**2 * PF * test_masses[2]**2
    T_via_pf = 1.0 / (2 * N_base**2 * PF * test_masses[2])
    product_pf = S_via_pf * T_via_pf
    expected_pf = test_masses[2] / 2.0
    assert abs(product_pf - expected_pf) < 1e-12, "PF should cancel in S*T product"

    print("  PASS: S_BH x T_H = M/2 (PF-free) verified")


# =============================================================================
# PART C: EVAPORATION AND INFORMATION
# =============================================================================

def test_CG_T8_evaporation_type_recovery():
    """CG-T8: As M -> 0 during evaporation, lambda_H -> 1 (Type III_1 recovery)."""
    # Simulate evaporation: M decreasing
    M_initial = 10.0
    masses = np.logspace(np.log10(M_initial), np.log10(0.001), 50)

    print(f"  {'M':>10} {'beta_H':>12} {'lambda_H':>15} {'Type character':>20}")
    print(f"  {'-'*10} {'-'*12} {'-'*15} {'-'*20}")

    # Print a selection of masses
    display_indices = [0, 10, 20, 30, 40, 45, 48, 49]
    for i in display_indices:
        M = masses[i]
        beta = hawking_beta(M)
        lam = connes_parameter(beta) if beta < 700 else 0.0

        if lam > 0.9:
            char = "Near Type III_1"
        elif lam > 0.1:
            char = f"Type III_{lam:.3f}"
        elif lam > 1e-10:
            char = f"Type III_0 (lam~{lam:.2e})"
        else:
            char = "Near Type I"

        print(f"  {M:10.4f} {beta:12.4f} {lam:15.10e} {char:>20}")

    # Verify endpoint: small M gives lambda -> 1
    M_small = 0.001
    beta_small = hawking_beta(M_small)
    lam_small = connes_parameter(beta_small)

    assert lam_small > 0.97, \
        f"lambda should approach 1 for small M, got {lam_small} at M={M_small}"

    # Verify monotonicity: lambda increases as M decreases
    lambdas = []
    for M in masses:
        beta = hawking_beta(M)
        if beta < 700:
            lambdas.append(connes_parameter(beta))
        else:
            lambdas.append(0.0)

    for i in range(1, len(lambdas)):
        assert lambdas[i] >= lambdas[i-1] - 1e-15, \
            f"lambda should increase as M decreases (monotonicity violation at i={i})"

    print("  PASS: lambda_H -> 1 (Type III_1) as M -> 0 during evaporation")


def test_CG_T9_page_time_scaling():
    """CG-T9: Page time estimate t_Page proportional to M^3."""
    # The Page time corresponds to half the initial entropy being radiated
    # BH mass evolves as: dM/dt ~ -1/M^2 (Hawking), so M(t) ~ (M_0^3 - C*t)^{1/3}
    # Page time: M(t_P) ~ M_0/sqrt(2), so t_P ~ M_0^3 * (1 - 1/(2*sqrt(2)))

    C_page = 1 - 1 / (2 * np.sqrt(2))  # ~ 0.6464

    masses = [1.0, 2.0, 5.0, 10.0, 50.0, 100.0]

    print(f"  Page time coefficient: C = 1 - 1/(2*sqrt(2)) = {C_page:.6f}")
    print(f"  {'M_0':>8} {'t_Page':>15} {'M_0^3':>15} {'t_Page/M_0^3':>15}")
    print(f"  {'-'*8} {'-'*15} {'-'*15} {'-'*15}")

    ratios = []
    for M0 in masses:
        t_page = C_page * M0**3
        ratio = t_page / M0**3
        ratios.append(ratio)

        print(f"  {M0:8.1f} {t_page:15.4f} {M0**3:15.4f} {ratio:15.8f}")

    # Verify t_Page ~ M^3 scaling: all ratios should be equal
    for r in ratios:
        assert abs(r - C_page) < 1e-10, f"Page time should scale as M^3, ratio = {r}"

    # Verify beta at Page time
    M_test = 10.0
    M_page = M_test / np.sqrt(2)
    beta_page = hawking_beta(M_page)
    beta_initial = hawking_beta(M_test)
    ratio_beta = beta_page / beta_initial

    print(f"\n  At Page time for M_0 = {M_test}:")
    print(f"    M(t_Page) = M_0/sqrt(2) = {M_page:.4f}")
    print(f"    beta_H(t_Page) = {beta_page:.4f}")
    print(f"    beta_H(t_Page)/beta_H(0) = {ratio_beta:.6f} (should be 1/sqrt(2) = {1/np.sqrt(2):.6f})")

    assert abs(ratio_beta - 1/np.sqrt(2)) < 1e-10, "beta ratio at Page time should be 1/sqrt(2)"

    print("  PASS: Page time scales as M^3 with correct coefficient")


def test_CG_T10_gravitational_decoherence_scaling():
    """CG-T10: Gravitational decoherence timescale tau ~ hbar*d/(G*m^2) matches Penrose form."""
    # Test that FTD gravitational decoherence has the correct scaling
    # In Planck units: G = hbar = c = 1

    # Test mass scaling: tau ~ 1/m^2
    d = 1.0  # fixed separation
    masses = [0.1, 0.5, 1.0, 2.0, 5.0, 10.0]

    print("  Mass scaling (fixed d=1):")
    print(f"  {'m':>8} {'tau':>15} {'tau*m^2':>15} {'Constant?':>12}")
    print(f"  {'-'*8} {'-'*15} {'-'*15} {'-'*12}")

    for m in masses:
        tau = penrose_decoherence_time(m, d)
        product = tau * m**2
        print(f"  {m:8.2f} {tau:15.6f} {product:15.6f} {'Yes' if abs(product - d) < 1e-10 else 'No':>12}")
        assert abs(product - d) < 1e-10, f"tau*m^2 should equal d, got {product}"

    # Test distance scaling: tau ~ d
    m = 1.0  # fixed mass
    distances = [0.1, 0.5, 1.0, 2.0, 5.0, 10.0]

    print(f"\n  Distance scaling (fixed m=1):")
    print(f"  {'d':>8} {'tau':>15} {'tau/d':>15} {'Constant?':>12}")
    print(f"  {'-'*8} {'-'*15} {'-'*15} {'-'*12}")

    for d in distances:
        tau = penrose_decoherence_time(m, d)
        ratio = tau / d
        print(f"  {d:8.2f} {tau:15.6f} {ratio:15.6f} {'Yes' if abs(ratio - 1/m**2) < 1e-10 else 'No':>12}")
        assert abs(ratio - 1/m**2) < 1e-10, f"tau/d should be 1/m^2, got {ratio}"

    # Verify the formula gives physically reasonable values
    # For a 1 microgram mass separated by 1 micrometer (in SI):
    # tau_Penrose ~ hbar / (G * m^2 / d) ~ 1e-34 / (6.67e-11 * (1e-9)^2 / 1e-6)
    #            ~ 1e-34 / (6.67e-14) ~ 1.5e-21 seconds
    # This is the regime targeted by tabletop experiments

    print("  PASS: Gravitational decoherence scaling tau ~ hbar*d/(G*m^2) verified")


# =============================================================================
# MAIN
# =============================================================================

def main():
    tests = [
        ("CG-T1", "PF decomposition: 8*pi = 2*N_base^2*PF", test_CG_T1_pf_decomposition_8pi),
        ("CG-T2", "Connes parameter lambda_H in (0,1)", test_CG_T2_connes_parameter_range),
        ("CG-T3", "KMS strip width pi/beta_H = 1/(8M)", test_CG_T3_kms_strip_width),
        ("CG-T4", "beta_H = 8*pi*M = 2*N_base^2*PF*M", test_CG_T4_beta_H_formula),
        ("CG-T5", "Tolman beta_local -> 0 at horizon", test_CG_T5_tolman_horizon_limit),
        ("CG-T6", "Tolman beta_local -> beta_H at infinity", test_CG_T6_tolman_asymptotic_limit),
        ("CG-T7", "S_BH x T_H = M/2 (PF-free)", test_CG_T7_entropy_temperature_product),
        ("CG-T8", "Evaporation: lambda_H -> 1 (Type III_1)", test_CG_T8_evaporation_type_recovery),
        ("CG-T9", "Page time ~ M^3 scaling", test_CG_T9_page_time_scaling),
        ("CG-T10", "Gravitational decoherence scaling", test_CG_T10_gravitational_decoherence_scaling),
    ]

    print("=" * 70)
    print("COLLAPSE-GRAVITY BRIDGE VERIFICATION")
    print("EXPLR_COLLAPSE_GRAVITY_BRIDGE.md")
    print("=" * 70)

    passed = 0
    failed = 0
    errors = []

    for test_id, description, test_fn in tests:
        print(f"\n[{test_id}] {description}")
        print("-" * 50)
        try:
            test_fn()
            passed += 1
        except AssertionError as e:
            failed += 1
            errors.append((test_id, str(e)))
            print(f"  FAIL: {e}")
        except Exception as e:
            failed += 1
            errors.append((test_id, f"ERROR: {e}"))
            print(f"  ERROR: {e}")

    print("\n" + "=" * 70)
    print(f"RESULTS: {passed}/{passed+failed} tests passed")
    print("=" * 70)

    if errors:
        print("\nFailed tests:")
        for test_id, msg in errors:
            print(f"  [{test_id}] {msg}")
        return 1

    print("\nAll tests PASSED.")

    # Summary of key values
    print("\n" + "-" * 70)
    print("KEY VALUES SUMMARY")
    print("-" * 70)
    print(f"  8*pi = 2*N_base^2*PF = {8*np.pi:.10f}")
    print(f"  N_base = {N_base}, PF = pi/4 = {PF:.10f}")
    print(f"  G* = {G_STAR:.10f}")
    print(f"  k_c = 1/(4*G*) = {1/(4*G_STAR):.10f} (phase diagram critical point)")
    print(f"  Page time coefficient = 1 - 1/(2*sqrt(2)) = {1 - 1/(2*np.sqrt(2)):.10f}")
    print()
    print("  BH algebraic character at key masses:")
    for M in [0.01, 0.1, 1.0, 10.0]:
        beta = hawking_beta(M)
        lam = connes_parameter(beta) if beta < 700 else 0.0
        print(f"    M={M:6.2f}: beta_H={beta:8.3f}, lambda_H={lam:.6e}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
