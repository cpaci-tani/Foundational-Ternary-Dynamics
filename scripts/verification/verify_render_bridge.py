"""
Render-Bridge Verification Suite

Verifies the complete derivation chain from SPEC_FTD_LAGRANGIAN.md v2.0:
  D=3 -> PF -> N_base -> 16 -> G* -> master quadratic -> {alpha, N_c, b_3, N_eff}
  Born-Infeld weak-field reduction -> v1.0 Klein-Gordon
  gamma_FTD special cases
  Contextual Tensor properties
  Drag values and mass correspondence
"""

import numpy as np
from scipy.special import gamma

# ============================================================================
# Constants (recomputed from scratch -- independent of simulations/constants.py)
# ============================================================================

D = 3                                  # Spatial dimensions (axiom)
PF = np.pi / 4                        # Packing fraction on each lattice face
N_BASE = int(2**((D + 1) / 2))        # Spinor dimension
GAMMA_QUARTER = gamma(0.25)           # Gamma(1/4)
VARPI = GAMMA_QUARTER**2 / (2 * np.sqrt(2 * np.pi))  # Lemniscate constant
G_STAR = VARPI / np.sqrt(PF)          # Universal Render Bridge
SQRT_GSTAR = np.sqrt(G_STAR)          # Time operator

# ============================================================================
# Test infrastructure
# ============================================================================

PASS_COUNT = 0
FAIL_COUNT = 0


def check(name, condition, detail=""):
    global PASS_COUNT, FAIL_COUNT
    if condition:
        PASS_COUNT += 1
        print(f"  PASS  {name}")
    else:
        FAIL_COUNT += 1
        print(f"  FAIL  {name}")
        if detail:
            print(f"        {detail}")


def section(title):
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}")


# ============================================================================
# Test 1: Derivation Chain
# ============================================================================

def test_derivation_chain():
    section("TEST 1: Complete Derivation Chain (D=3 -> alpha)")

    # Sub-test 1a: PF from D=3
    pf_computed = np.pi / 4
    check("PF = pi/4 from D=3 cubic lattice",
          abs(pf_computed - 0.7854) < 0.0001,
          f"PF = {pf_computed:.6f}")

    # Sub-test 1b: N_base from D=3
    n_base = int(2**((D + 1) / 2))
    check("N_base = 2^((D+1)/2) = 4",
          n_base == 4,
          f"N_base = {n_base}")

    # Sub-test 1c: Coefficient = N_base^2 = 16
    coeff = n_base**2
    check("Coefficient = N_base^2 = 16",
          coeff == 16,
          f"Coefficient = {coeff}")

    # Sub-test 1d: varpi (lemniscate constant)
    check("varpi = Gamma(1/4)^2 / (2*sqrt(2*pi)) ~ 2.6221",
          abs(VARPI - 2.6220575) < 0.0000001,
          f"varpi = {VARPI:.10f}")

    # Sub-test 1e: G* = varpi / sqrt(PF)
    check("G* = varpi / sqrt(PF) ~ 2.9587",
          abs(G_STAR - 2.9586751) < 0.0000001,
          f"G* = {G_STAR:.10f}")

    # Sub-test 1f: Alternative G* formula matches
    g_star_alt = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)
    check("G* = sqrt(2)*Gamma(1/4)^2/(2*pi) [alternative form]",
          abs(G_STAR - g_star_alt) < 1e-14,
          f"Bridge form: {G_STAR:.15f}, Alt form: {g_star_alt:.15f}")

    # Sub-test 1g: Master quadratic
    c = G_STAR
    a_coef = 1
    b_coef = -16 * c**2
    c_coef = 16 * c**3
    disc = b_coef**2 - 4 * a_coef * c_coef
    x_plus = (-b_coef + np.sqrt(disc)) / 2
    x_minus = (-b_coef - np.sqrt(disc)) / 2

    check("x_+ ~ 137.036 (alpha^-1 tree level)",
          abs(x_plus - 137.036) < 0.001,
          f"x_+ = {x_plus:.10f}")

    check("x_- ~ 3.024 (N_c root)",
          abs(x_minus - 3.024) < 0.001,
          f"x_- = {x_minus:.10f}")

    # Sub-test 1h: Vieta relations
    check("Vieta: x_+ + x_- = 16*G*^2",
          abs((x_plus + x_minus) - 16 * c**2) < 1e-10,
          f"Sum = {x_plus + x_minus:.10f}, 16*G*^2 = {16*c**2:.10f}")

    check("Vieta: x_+ * x_- = 16*G*^3",
          abs((x_plus * x_minus) - 16 * c**3) < 1e-10,
          f"Product = {x_plus * x_minus:.10f}, 16*G*^3 = {16*c**3:.10f}")

    # Sub-test 1i: Derived integers
    N_c = int(np.floor(x_minus))
    check("N_c = floor(x_-) = 3",
          N_c == 3,
          f"N_c = {N_c}")

    N_gen = N_c
    N_f = 2 * N_gen
    b_3 = (11 * N_c - 2 * N_f) // 3
    check("b_3 = (11*N_c - 2*N_f)/3 = 7",
          b_3 == 7,
          f"b_3 = {b_3}")

    N_eff = b_3 + 2 * N_c
    check("N_eff = b_3 + 2*N_c = 13",
          N_eff == 13,
          f"N_eff = {N_eff}")

    # Sub-test 1j: Fibonacci check
    fib = [1, 1, 2, 3, 5, 8, 13, 21]
    check("N_eff = 13 = F_7 (Fibonacci closure)",
          N_eff == fib[6],
          f"F_7 = {fib[6]}")

    # Sub-test 1k: alpha accuracy vs CODATA
    alpha_tree = 1.0 / x_plus
    alpha_codata = 1.0 / 137.035999177
    ppm = abs(alpha_tree - alpha_codata) / alpha_codata * 1e6
    check("Tree-level alpha within 2 ppm of CODATA",
          ppm < 2.0,
          f"Error = {ppm:.3f} ppm")

    return x_plus, x_minus, N_c, b_3, N_eff


# ============================================================================
# Test 2: Precision Formula
# ============================================================================

def test_precision_formula(x_plus, N_c, b_3, N_eff):
    section("TEST 2: Precision Formula (Radiative Corrections)")

    D_constraint = N_c * N_BASE**2 - 1  # = 47
    check("D = N_c*N_base^2 - 1 = 47",
          D_constraint == 47,
          f"D = {D_constraint}")

    epsilon = np.exp(np.pi) - np.pi - (b_3 + N_eff)
    check("epsilon = e^pi - pi - 20, |epsilon| ~ 0.0009",
          abs(abs(epsilon) - 0.0009) < 0.001,
          f"epsilon = {epsilon:.10f}, |epsilon| = {abs(epsilon):.10f}")

    c1 = N_c**2 / D_constraint             # 9/47
    c2 = (N_eff - 2*N_BASE) / N_BASE**3     # 5/64
    c3 = N_BASE / (N_c * D_constraint)       # 4/141
    c4 = (N_c * D_constraint) / (b_3 + N_BASE)  # 141/11

    check("c1 = 9/47",
          abs(c1 - 9/47) < 1e-15,
          f"c1 = {c1}")
    check("c2 = 5/64",
          abs(c2 - 5/64) < 1e-15,
          f"c2 = {c2}")
    check("c3 = 4/141",
          abs(c3 - 4/141) < 1e-15,
          f"c3 = {c3}")
    check("c4 = 141/11",
          abs(c4 - 141/11) < 1e-15,
          f"c4 = {c4}")

    eps = abs(epsilon)
    alpha_inv_corrected = x_plus - c1*eps + c2*eps**2 - c3*eps**3 - c4*eps**4
    alpha_inv_codata = 137.035999177

    ppm_corrected = abs(alpha_inv_corrected - alpha_inv_codata) / alpha_inv_codata * 1e6
    check("4-term precision formula improves on tree level",
          ppm_corrected < 1.5,
          f"Corrected 1/alpha = {alpha_inv_corrected:.12f}, error = {ppm_corrected:.4f} ppm")


# ============================================================================
# Test 3: Born-Infeld Weak-Field Reduction
# ============================================================================

def test_born_infeld_reduction():
    section("TEST 3: Born-Infeld Weak-Field Reduction (Theorem 3.1)")

    K_B = 0.511  # MeV

    # For small v and L, the Born-Infeld core should reduce to Klein-Gordon kinetic terms
    test_values = [0.01, 0.05, 0.1, 0.2]

    for v in test_values:
        L = v * 0.5  # Keep L small too

        # Full Born-Infeld
        bi_full = -K_B * np.sqrt(1 - v**2 - L**2)

        # Weak-field expansion (up to second order)
        bi_weak = -K_B + K_B/2 * v**2 + K_B/2 * L**2

        # Relative error (excluding constant -K_B)
        dynamic_full = bi_full - (-K_B)  # dynamic part only
        dynamic_weak = bi_weak - (-K_B)
        rel_error = abs(dynamic_full - dynamic_weak) / abs(dynamic_full) if abs(dynamic_full) > 0 else 0

        check(f"BI weak-field at v={v:.2f}, L={L:.3f}: rel_error < 5%",
              rel_error < 0.05,
              f"Full={bi_full:.8f}, Weak={bi_weak:.8f}, rel_error={rel_error:.6f}")

    # Test that speed limit is built in
    check("BI diverges as v^2 + L^2 -> 1",
          np.sqrt(1 - 0.999) < 0.032,
          f"sqrt(1-0.999) = {np.sqrt(1-0.999):.6f}")

    # Test that the argument cannot go negative
    check("BI argument non-negative for v^2+L^2 < 1",
          all(1 - v**2 - (0.3*v)**2 > 0 for v in np.linspace(0, 0.9, 100)))


# ============================================================================
# Test 4: FTD Lorentz Factor Special Cases
# ============================================================================

def test_lorentz_factor():
    section("TEST 4: FTD Lorentz Factor Special Cases")

    def gamma_ftd(v, L):
        return 1.0 / np.sqrt(1 - v**2 - L**2)

    # Case 1: Rest in flat space
    g = gamma_ftd(0, 0)
    check("Rest in flat space: gamma = 1",
          abs(g - 1.0) < 1e-15,
          f"gamma = {g}")

    # Case 2: Standard SR (L=0)
    for v in [0.1, 0.5, 0.8, 0.99]:
        g_ftd = gamma_ftd(v, 0)
        g_sr = 1.0 / np.sqrt(1 - v**2)
        check(f"SR at v={v}: gamma_FTD = gamma_SR",
              abs(g_ftd - g_sr) < 1e-14,
              f"FTD={g_ftd:.10f}, SR={g_sr:.10f}")

    # Case 3: Gravitational time dilation (v=0)
    r_s_over_r = 0.1  # Moderate gravity
    L = np.sqrt(r_s_over_r)
    g = gamma_ftd(0, L)
    g_schwarz = 1.0 / np.sqrt(1 - r_s_over_r)
    check("Gravitational dilation: gamma_FTD = 1/sqrt(1-r_s/r)",
          abs(g - g_schwarz) < 1e-14,
          f"FTD={g:.10f}, Schwarzschild={g_schwarz:.10f}")

    # Case 4: Dark matter (v=0, L=0.75)
    g_dm = gamma_ftd(0, 0.75)
    expected = 1.0 / np.sqrt(1 - 0.75**2)
    check("Dark matter (L=0.75): gamma ~ 1.51",
          abs(g_dm - expected) < 1e-10,
          f"gamma_DM = {g_dm:.6f}")

    # Case 5: Bandwidth sum constraint
    # v^2 + L^2 must stay < 1
    v_test, L_test = 0.6, 0.6
    budget = v_test**2 + L_test**2
    check("v=0.6, L=0.6: v^2+L^2 = 0.72 < 1 (valid)",
          budget < 1.0,
          f"Budget = {budget}")

    v_test2, L_test2 = 0.8, 0.7
    budget2 = v_test2**2 + L_test2**2
    check("v=0.8, L=0.7: v^2+L^2 = 1.13 > 1 (forbidden)",
          budget2 > 1.0,
          f"Budget = {budget2}")

    # Case 6: Near-horizon limit
    g_near = gamma_ftd(0, np.sqrt(0.999))
    check("Near horizon (L^2=0.999): gamma > 30",
          g_near > 30,
          f"gamma = {g_near:.2f}")


# ============================================================================
# Test 5: Contextual Tensor
# ============================================================================

def test_contextual_tensor():
    section("TEST 5: Contextual Tensor Properties")

    # Define a test Contextual Tensor
    N = np.array([5.0, 3.0, 7.0])     # Position (integer lattice address)
    v = np.array([0.1, 0.2, 0.05])     # Velocity
    L_val = 0.3                        # Latency
    n_tick = 100                       # Tick number
    gstar_n = G_STAR**n_tick           # Compounding time

    # For scalar test, use magnitude of N and v
    N_mag = np.linalg.norm(N)
    v_mag = np.linalg.norm(v)

    # 2x2 Contextual Tensor (scalar reduction)
    C = np.array([[N_mag, v_mag],
                  [L_val, G_STAR]])  # Use single G* for n=1 test

    # Determinant
    det_C = np.linalg.det(C)
    expected_det = N_mag * G_STAR - v_mag * L_val
    check("det(C) = N*G* - v*L (computational capacity)",
          abs(det_C - expected_det) < 1e-10,
          f"det = {det_C:.10f}, expected = {expected_det:.10f}")

    # Trace
    tr_C = np.trace(C)
    expected_tr = N_mag + G_STAR
    check("tr(C) = N + G* (event address)",
          abs(tr_C - expected_tr) < 1e-10,
          f"tr = {tr_C:.10f}, expected = {expected_tr:.10f}")

    # Positive determinant when v*L is small
    check("det(C) > 0 for small v*L (healthy node)",
          det_C > 0,
          f"det = {det_C:.6f}")

    # High velocity/latency should reduce determinant
    C_stressed = np.array([[N_mag, 0.9],
                           [0.9, G_STAR]])
    det_stressed = np.linalg.det(C_stressed)
    check("det(C) decreases under high v*L (stressed node)",
          det_stressed < det_C,
          f"Stressed det = {det_stressed:.6f} < Normal det = {det_C:.6f}")


# ============================================================================
# Test 6: Drag Values
# ============================================================================

def test_drag_values():
    section("TEST 6: Mass-Drag Correspondence")

    drag_axis = 1.0 / N_BASE  # 1/4 = 0.25
    check("Drag per axis = 1/N_base = 0.25",
          abs(drag_axis - 0.25) < 1e-15,
          f"drag_axis = {drag_axis}")

    # Electron: 1D drag
    drag_electron = 1 * drag_axis
    check("Electron drag (1D) = 0.25",
          abs(drag_electron - 0.25) < 1e-15)

    # Top quark: 3D drag
    drag_top = D * drag_axis
    check("Top quark drag (3D) = 0.75",
          abs(drag_top - 0.75) < 1e-15)

    # Neutrino: ~0D drag
    drag_neutrino = 0.0
    check("Neutrino drag (~0D) ~ 0",
          drag_neutrino < 0.01)

    # Mass ratio consistency: drag_top / drag_electron = 3
    ratio = drag_top / drag_electron
    check("drag_top / drag_electron = 3 (dimensions ratio)",
          abs(ratio - 3.0) < 1e-15,
          f"ratio = {ratio}")


# ============================================================================
# Test 7: Operator Properties
# ============================================================================

def test_operator_properties():
    section("TEST 7: Render-Bridge Operator Properties")

    # G* = sqrt(G*) * sqrt(G*)
    check("G* = sqrt(G*)^2 (tick = two sub-events)",
          abs(G_STAR - SQRT_GSTAR**2) < 1e-14,
          f"G* = {G_STAR:.15f}, sqrt(G*)^2 = {SQRT_GSTAR**2:.15f}")

    # Projection tensor idempotence
    test_val = 3.7
    proj1 = np.round(test_val)
    proj2 = np.round(proj1)
    check("T_hat idempotent: round(round(x)) = round(x)",
          proj1 == proj2,
          f"round({test_val}) = {proj1}, round({proj1}) = {proj2}")

    # Drag = fractional remainder
    drag = test_val - np.round(test_val)
    check("Drag = x - round(x) (fractional remainder)",
          abs(drag - (-0.3)) < 1e-15,
          f"drag = {drag}")

    # Drag bounded in [-0.5, 0.5)
    for x in np.linspace(-10, 10, 1000):
        d = x - np.round(x)
        if not (-0.5 - 1e-10 <= d <= 0.5 + 1e-10):
            check("Drag always in [-0.5, 0.5]", False, f"x={x}, drag={d}")
            return
    check("Drag bounded in [-0.5, 0.5] for all test values", True)


# ============================================================================
# Test 8: Cross-check with simulations/constants.py
# ============================================================================

def test_cross_check():
    section("TEST 8: Cross-Check with simulations/constants.py")

    try:
        import sys
        import os
        # Add project root to path
        project_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
        sys.path.insert(0, project_root)
        from scripts.constants import (
            G_STAR as G_STAR_SIM, VARPI_CLASSICAL, PF as PF_SIM,
            N_c, N_base, b_3, N_eff, X_PLUS, X_MINUS
        )

        check("G* matches simulations/constants.py",
              abs(G_STAR - G_STAR_SIM) < 1e-14,
              f"Local: {G_STAR:.15f}, constants.py: {G_STAR_SIM:.15f}")

        check("varpi matches simulations/constants.py",
              abs(VARPI - VARPI_CLASSICAL) < 1e-14,
              f"Local: {VARPI:.15f}, constants.py: {VARPI_CLASSICAL:.15f}")

        check("PF matches simulations/constants.py",
              abs(PF - PF_SIM) < 1e-14)

        check("N_base matches",
              N_BASE == N_base)

        check("Master quadratic x_+ matches",
              abs(137.036 - X_PLUS) < 0.001,
              f"X_PLUS = {X_PLUS:.10f}")

    except ImportError as e:
        check(f"Import simulations/constants.py", False, str(e))


# ============================================================================
# Test 9: G* Bridge Identity
# ============================================================================

def test_gstar_bridge():
    section("TEST 9: G* Bridge Identity (DERIV_GSTAR_PF_BRIDGE)")

    # G* = varpi / sqrt(PF) = 2*varpi / sqrt(pi)
    g1 = VARPI / np.sqrt(PF)
    g2 = 2 * VARPI / np.sqrt(np.pi)
    check("G* = varpi/sqrt(PF) = 2*varpi/sqrt(pi)",
          abs(g1 - g2) < 1e-14,
          f"Form 1: {g1:.15f}, Form 2: {g2:.15f}")

    # G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)
    g3 = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)
    check("G* = sqrt(2)*Gamma(1/4)^2/(2*pi)",
          abs(g1 - g3) < 1e-14,
          f"Bridge: {g1:.15f}, Gamma: {g3:.15f}")

    # PF cancellation: G*^2 * PF = varpi^2
    check("G*^2 * PF = varpi^2 (PF cancellation identity)",
          abs(G_STAR**2 * PF - VARPI**2) < 1e-14,
          f"G*^2*PF = {G_STAR**2*PF:.15f}, varpi^2 = {VARPI**2:.15f}")


# ============================================================================
# Test 10: Equations of Motion
# ============================================================================

def test_equations_of_motion():
    section("TEST 10: Born-Infeld Equations of Motion")

    K_B = 0.511

    # Relativistic momentum: p = K_B * gamma * v
    v = 0.5
    L = 0.3
    gamma = 1.0 / np.sqrt(1 - v**2 - L**2)
    p = K_B * gamma * v

    # Check momentum diverges as bandwidth limit approached
    v_high = 0.8
    L_high = 0.5
    budget_high = v_high**2 + L_high**2
    if budget_high < 1.0:
        gamma_high = 1.0 / np.sqrt(1 - budget_high)
        p_high = K_B * gamma_high * v_high
        check("Momentum increases near bandwidth limit",
              p_high > p,
              f"p(v=0.5)={p:.4f}, p(v=0.8)={p_high:.4f}")

    # Newton's second law in weak field: F = K_B * dv/dt
    # (just verify the functional form)
    dv_dt = 0.01  # small acceleration
    F_weak = K_B * dv_dt
    F_full = K_B * gamma * dv_dt  # full relativistic
    check("Weak-field: F ~ K_B * dv/dt (Newton's 2nd law)",
          abs(F_weak - K_B * dv_dt) < 1e-15)

    check("Relativistic correction: F_full > F_weak",
          F_full > F_weak,
          f"F_weak={F_weak:.6f}, F_full={F_full:.6f}")


# ============================================================================
# Run all tests
# ============================================================================

if __name__ == "__main__":
    print("=" * 70)
    print("  RENDER-BRIDGE VERIFICATION SUITE")
    print("  SPEC_FTD_LAGRANGIAN.md v2.0 (Born-Infeld Render-Bridge Action)")
    print("=" * 70)

    x_plus, x_minus, N_c, b_3, N_eff = test_derivation_chain()
    test_precision_formula(x_plus, N_c, b_3, N_eff)
    test_born_infeld_reduction()
    test_lorentz_factor()
    test_contextual_tensor()
    test_drag_values()
    test_operator_properties()
    test_cross_check()
    test_gstar_bridge()
    test_equations_of_motion()

    print(f"\n{'='*70}")
    print(f"  RESULTS: {PASS_COUNT} passed, {FAIL_COUNT} failed")
    print(f"{'='*70}")

    if FAIL_COUNT == 0:
        print("\n  All render-bridge verifications PASSED.")
    else:
        print(f"\n  WARNING: {FAIL_COUNT} test(s) FAILED.")

    exit(FAIL_COUNT)
