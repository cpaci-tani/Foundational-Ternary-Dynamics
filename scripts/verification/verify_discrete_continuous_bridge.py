"""
Verification Script: The Discrete-Continuous Bridge

Tests the algebraic identities and structural results from
DERIV_DISCRETE_CONTINUOUS_BRIDGE.md.

All tests use print-based output (no pytest).
Run: python scripts/verification/verify_discrete_continuous_bridge.py
"""

import numpy as np
from scipy.special import gamma

# =============================================================================
# CONSTANTS
# =============================================================================

# Framework integers
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13

# Mathematical constants
GAMMA_QUARTER = gamma(0.25)  # Gamma(1/4)
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)
VARPI = GAMMA_QUARTER**2 / (2 * np.sqrt(2 * np.pi))  # lemniscate half-period
PF = np.pi / 4  # circle-in-square packing fraction
M_AGM = np.pi / VARPI  # AGM(1, sqrt(2)) = pi/varpi

# Constraint dimension
D = N_c * N_base**2 - 1  # = 47

# Tolerance
TOL = 1e-12

# =============================================================================
# TEST INFRASTRUCTURE
# =============================================================================

results = []


def record(name, passed, detail=""):
    """Record a test result."""
    status = "[PASS]" if passed else "[FAIL]"
    results.append((name, passed, detail))
    print(f"  {status} {name}")
    if detail:
        print(f"         {detail}")


# =============================================================================
# PART A: CORE PF FORM OF MASTER QUADRATIC (4 tests)
# =============================================================================

print("=" * 70)
print("PART A: CORE PF FORM OF MASTER QUADRATIC")
print("=" * 70)
print()

# Test 1: PF-form quadratic gives same roots as standard form (DCB-1)
# Standard: x^2 - 16*G*^2*x + 16*G*^3 = 0
a_std = 1
b_std = -16 * G_STAR**2
c_std = 16 * G_STAR**3

disc_std = b_std**2 - 4 * a_std * c_std
x_plus_std = (-b_std + np.sqrt(disc_std)) / 2
x_minus_std = (-b_std - np.sqrt(disc_std)) / 2

# PF form: x^2 - (16*varpi^2/PF)*x + (16*varpi^3/PF^{3/2}) = 0
b_pf = -16 * VARPI**2 / PF
c_pf = 16 * VARPI**3 / PF**1.5

disc_pf = b_pf**2 - 4 * c_pf
x_plus_pf = (-b_pf + np.sqrt(disc_pf)) / 2
x_minus_pf = (-b_pf - np.sqrt(disc_pf)) / 2

err_plus = abs(x_plus_std - x_plus_pf)
err_minus = abs(x_minus_std - x_minus_pf)
record(
    "DCB-1: PF-form roots match standard roots",
    err_plus < TOL and err_minus < TOL,
    f"x+ diff = {err_plus:.2e}, x- diff = {err_minus:.2e}"
)

# Test 2: Vieta sum = 16*varpi^2/PF (DCB-2)
vieta_sum_std = 16 * G_STAR**2
vieta_sum_pf = 16 * VARPI**2 / PF
vieta_sum_roots = x_plus_std + x_minus_std
record(
    "DCB-2: Vieta sum = 16*varpi^2/PF",
    abs(vieta_sum_std - vieta_sum_pf) < TOL and abs(vieta_sum_std - vieta_sum_roots) < TOL,
    f"16G*^2 = {vieta_sum_std:.8f}, 16*varpi^2/PF = {vieta_sum_pf:.8f}, "
    f"x+ + x- = {vieta_sum_roots:.8f}"
)

# Test 3: Vieta product = 16*varpi^3/PF^{3/2} (DCB-3)
vieta_prod_std = 16 * G_STAR**3
vieta_prod_pf = 16 * VARPI**3 / PF**1.5
vieta_prod_roots = x_plus_std * x_minus_std
record(
    "DCB-3: Vieta product = 16*varpi^3/PF^{3/2}",
    abs(vieta_prod_std - vieta_prod_pf) < TOL * 100
    and abs(vieta_prod_std - vieta_prod_roots) < TOL * 100,
    f"16G*^3 = {vieta_prod_std:.8f}, 16*varpi^3/PF^(3/2) = {vieta_prod_pf:.8f}, "
    f"x+ * x- = {vieta_prod_roots:.8f}"
)

# Test 4: G* = 2*sqrt(varpi/M) where M = AGM(1, sqrt(2)) (DCB-10)
gstar_agm = 2 * np.sqrt(VARPI / M_AGM)
record(
    "DCB-10: G* = 2*sqrt(varpi/M)",
    abs(gstar_agm - G_STAR) < TOL,
    f"2*sqrt(varpi/M) = {gstar_agm:.10f}, G* = {G_STAR:.10f}"
)

print()

# =============================================================================
# PART B: ROOT RATIO AND PF BEHAVIOR (3 tests)
# =============================================================================

print("=" * 70)
print("PART B: ROOT RATIO AND PF BEHAVIOR")
print("=" * 70)
print()

# Test 5: Root ratio from PF expression matches direct (DCB-4)
inner = np.sqrt(1 - np.sqrt(PF) / (4 * VARPI))
ratio_pf_form = (1 + inner) / (1 - inner)
ratio_direct = x_plus_std / x_minus_std
record(
    "DCB-4: Root ratio from PF expression",
    abs(ratio_pf_form - ratio_direct) < TOL,
    f"PF form: {ratio_pf_form:.8f}, direct: {ratio_direct:.8f}"
)

# Test 6: Root ratio changes when PF varies (PF does NOT cancel)
pf_test_values = [0.5, 0.6, PF, 0.8, 0.9]
ratios = []
for pf_val in pf_test_values:
    g_test = VARPI / np.sqrt(pf_val)
    b_test = -16 * g_test**2
    c_test = 16 * g_test**3
    d_test = b_test**2 - 4 * c_test
    if d_test > 0:
        xp = (-b_test + np.sqrt(d_test)) / 2
        xm = (-b_test - np.sqrt(d_test)) / 2
        ratios.append(xp / xm)
    else:
        ratios.append(None)

# Check that ratios are NOT all the same
all_same = all(
    r is not None and abs(r - ratios[0]) < TOL
    for r in ratios
)
record(
    "PF does NOT cancel in root ratio (cross-sector)",
    not all_same,
    f"Ratios for PF={pf_test_values}: {[f'{r:.4f}' if r else 'None' for r in ratios]}"
)

# Test 7: Vieta product/sum = G* (coefficient ratio, Theorem 1.2)
coeff_ratio = vieta_prod_std / vieta_sum_std
record(
    "Coefficient ratio: product/sum = G*",
    abs(coeff_ratio - G_STAR) < TOL,
    f"16G*^3 / 16G*^2 = {coeff_ratio:.10f}, G* = {G_STAR:.10f}"
)

print()

# =============================================================================
# PART C: DISCRIMINANT (2 tests)
# =============================================================================

print("=" * 70)
print("PART C: DISCRIMINANT IN PF FORM")
print("=" * 70)
print()

# Test 8: Discriminant in PF form matches standard (DCB-5)
disc_standard = 64 * G_STAR**3 * (4 * G_STAR - 1)
disc_pf_form = 64 * VARPI**3 / PF**1.5 * (4 * VARPI / np.sqrt(PF) - 1)
record(
    "DCB-5: Discriminant PF form matches standard",
    abs(disc_standard - disc_pf_form) < TOL * 1e4,  # looser for large numbers
    f"Standard: {disc_standard:.4f}, PF form: {disc_pf_form:.4f}"
)

# Test 9: Delta > 0 for physical PF = pi/4
record(
    "Delta > 0 for physical PF = pi/4",
    disc_standard > 0,
    f"Delta = {disc_standard:.4f} > 0 (real roots confirmed)"
)

print()

# =============================================================================
# PART D: PRECISION FORMULA DECOMPOSITION (3 tests)
# =============================================================================

print("=" * 70)
print("PART D: PRECISION FORMULA DECOMPOSITION")
print("=" * 70)
print()

# Test 10: epsilon = e^pi - N_base*PF - (b_3 + N_eff) matches e^pi - pi - 20 (DCB-6)
eps_standard = np.exp(np.pi) - np.pi - 20
eps_decomposed = np.exp(np.pi) - N_base * PF - (b_3 + N_eff)
record(
    "DCB-6: epsilon decomposition",
    abs(eps_standard - eps_decomposed) < 1e-15,
    f"Standard: {eps_standard:.15e}, Decomposed: {eps_decomposed:.15e}"
)

# Test 11: Coefficient exact rational check
c1 = N_c**2 / D                           # 9/47
c2 = (N_eff - 2 * N_base) / N_base**3     # 5/64
c3 = N_base / (N_c * D)                   # 4/141
c4 = (N_c * D) / (b_3 + N_base)           # 141/11

record(
    "Coefficients are exact integer ratios",
    abs(c1 - 9/47) < 1e-15
    and abs(c2 - 5/64) < 1e-15
    and abs(c3 - 4/141) < 1e-15
    and abs(c4 - 141/11) < 1e-15,
    f"c1 = {c1} = 9/47, c2 = {c2} = 5/64, c3 = {c3:.10f} = 4/141, "
    f"c4 = {c4:.10f} = 141/11"
)

# Test 12: 4-term formula reproduces CODATA value
eps = abs(eps_standard)
x_plus_precision = x_plus_std - c1*eps + c2*eps**2 - c3*eps**3 - c4*eps**4
alpha_inv_codata = 137.035999177
err_ppt = abs(x_plus_precision - alpha_inv_codata) / alpha_inv_codata * 1e12
record(
    "4-term formula reproduces CODATA",
    err_ppt < 1.0,  # < 1 ppt
    f"FTD: {x_plus_precision:.12f}, CODATA: {alpha_inv_codata:.12f}, "
    f"error: {err_ppt:.4f} ppt"
)

print()

# =============================================================================
# PART E: THETA FUNCTION AND SELF-DUALITY (3 tests)
# =============================================================================

print("=" * 70)
print("PART E: THETA FUNCTION AND SELF-DUALITY")
print("=" * 70)
print()


def theta3(q, N=30):
    """Compute theta_3(q) = 1 + 2*sum(q^{n^2}, n=1..N)."""
    return 1 + 2 * sum(q**(n**2) for n in range(1, N + 1))


# Test 13: G* = sqrt(2*pi) * theta_3(e^{-pi})^2 (DCB-8)
q_lem = np.exp(-np.pi)
th3 = theta3(q_lem)
gstar_theta = np.sqrt(2 * np.pi) * th3**2
record(
    "DCB-8: G* = sqrt(2*pi) * theta_3(e^{-pi})^2",
    abs(gstar_theta - G_STAR) < TOL,
    f"Theta form: {gstar_theta:.10f}, G*: {G_STAR:.10f}, "
    f"diff: {abs(gstar_theta - G_STAR):.2e}"
)

# Test 14: Theta_3 self-duality: theta_3(e^{-pi*t}) = (1/sqrt(t)) * theta_3(e^{-pi/t})
all_self_dual = True
t_values = [0.5, 1.0, 2.0]
for t in t_values:
    lhs = theta3(np.exp(-np.pi * t))
    rhs = (1 / np.sqrt(t)) * theta3(np.exp(-np.pi / t))
    if abs(lhs - rhs) / abs(lhs) > 1e-10:
        all_self_dual = False
record(
    "Theta_3 self-duality verified",
    all_self_dual,
    f"theta_3(e^(-pi*t)) = (1/sqrt(t))*theta_3(e^(-pi/t)) for t = {t_values}"
)

# Test 15: G* = 2*sqrt(varpi/M) verified independently via AGM iteration
# Compute AGM(1, sqrt(2)) directly via iteration
a, g = 1.0, np.sqrt(2)
for _ in range(20):
    a, g = (a + g) / 2, np.sqrt(a * g)
m_agm_direct = a  # converged AGM value

gstar_from_agm = 2 * np.sqrt(VARPI / m_agm_direct)
record(
    "DCB-10: G* from direct AGM iteration",
    abs(gstar_from_agm - G_STAR) < TOL,
    f"AGM(1,sqrt(2)) = {m_agm_direct:.12f}, pi/varpi = {M_AGM:.12f}, "
    f"2*sqrt(varpi/M) = {gstar_from_agm:.10f}"
)

print()

# =============================================================================
# SUMMARY
# =============================================================================

print("=" * 70)
print("SUMMARY")
print("=" * 70)
print()
print(f"{'Test':<55} {'Status':<8}")
print("-" * 63)

n_pass = 0
n_fail = 0
for name, passed, detail in results:
    status = "PASS" if passed else "FAIL"
    if passed:
        n_pass += 1
    else:
        n_fail += 1
    print(f"  {name:<53} {status:<8}")

print("-" * 63)
print(f"\nResults: {n_pass}/{n_pass + n_fail} passed, {n_fail} failed")
print()

if n_fail == 0:
    print("ALL TESTS PASSED")
    print()
    print("Key results verified:")
    print(f"  x+ = {x_plus_std:.6f}, x- = {x_minus_std:.6f}")
    print(f"  Vieta sum = 16*varpi^2/PF = {vieta_sum_pf:.6f}")
    print(f"  Vieta product = 16*varpi^3/PF^(3/2) = {vieta_prod_pf:.6f}")
    print(f"  Root ratio x+/x- = {ratio_direct:.6f} (PF-dependent)")
    print(f"  G* = 2*sqrt(varpi/M) = {gstar_agm:.10f}")
    print(f"  G* = sqrt(2*pi)*theta_3^2 = {gstar_theta:.10f}")
    print(f"  epsilon decomposition verified: {eps_decomposed:.15e}")
    print(f"  4-term precision: {x_plus_precision:.12f} ({err_ppt:.4f} ppt from CODATA)")
else:
    print(f"WARNING: {n_fail} test(s) FAILED")
