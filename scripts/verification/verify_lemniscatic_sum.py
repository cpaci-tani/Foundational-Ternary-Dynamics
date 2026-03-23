#!/usr/bin/env python3
"""
Verification Script: The Discrete-Continuous Bridge Through the Lemniscatic Lens
================================================================================

Tests all algebraic identities from the paper "An FTD Analysis of Sum 1/(n^2+1)^2",
including the new Epstein zeta factorization and row decomposition results.

All tests use print-based output (no pytest).
Run: python scripts/verification/verify_lemniscatic_sum.py
"""

from mpmath import (mp, mpf, pi, gamma, sqrt, exp, coth, csch, floor,
                    fsum, catalan, zeta)


def sigma3(n):
    """Sum of cubes of divisors of n."""
    n = int(n)
    s = mpf(0)
    for d in range(1, n + 1):
        if n % d == 0:
            s += mpf(d)**3
    return s

mp.dps = 50  # 50 decimal places

# =============================================================================
# CONSTANTS (computed from scratch, not imported)
# =============================================================================

Gamma_quarter = gamma(mpf('0.25'))
varpi = Gamma_quarter**2 / (2 * sqrt(2 * pi))           # lemniscate constant
G_star = sqrt(mpf(2)) * Gamma_quarter**2 / (2 * pi)     # bridge constant
lam = varpi**2 / G_star**2                               # ontic ratio lambda

TOL = mpf('1e-14')

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
# PART A: CORE SUM IDENTITY (3 tests)
# =============================================================================

print("=" * 70)
print("PART A: CORE SUM IDENTITY")
print("=" * 70)
print()

# Test A1: S via direct summation (10^5 terms)
N_terms = 100000
S_numerical = fsum(mpf(1) / (mpf(n)**2 + 1)**2 for n in range(N_terms))

# Test A2: S via closed form (Eq 11)
S_closed = (2 + pi * coth(pi) + pi**2 * csch(pi)**2) / 4

err_A1 = abs(S_numerical - S_closed) / S_closed
record("A1: S numerical vs closed form",
       err_A1 < mpf('1e-9'),
       f"S_num = {float(S_numerical):.15f}, S_closed = {float(S_closed):.15f}, "
       f"rel err = {float(err_A1):.2e}")

# Test A3: S via FTD form (Eq 13)
S_ftd = (2 + 4 * lam * coth(4 * lam) + 16 * lam**2 * csch(4 * lam)**2) / 4

err_A2 = abs(S_closed - S_ftd) / S_closed
record("A2: S closed form vs FTD form",
       err_A2 < TOL,
       f"S_closed = {float(S_closed):.15f}, S_ftd = {float(S_ftd):.15f}, "
       f"rel err = {float(err_A2):.2e}")

err_A3 = abs(S_numerical - S_ftd) / S_ftd
record("A3: S numerical vs FTD form",
       err_A3 < mpf('1e-9'),
       f"rel err = {float(err_A3):.2e}")

print()

# =============================================================================
# PART B: ONTIC RATIO IDENTITIES (2 tests)
# =============================================================================

print("=" * 70)
print("PART B: ONTIC RATIO IDENTITIES")
print("=" * 70)
print()

# Test B1: pi = 4 * varpi^2 / G*^2
pi_from_ontic = 4 * varpi**2 / G_star**2
err_B1 = abs(pi_from_ontic - pi) / pi
record("B1: pi = 4 varpi^2 / G*^2",
       err_B1 < TOL,
       f"pi_ontic = {float(pi_from_ontic):.15f}, pi = {float(pi):.15f}, "
       f"rel err = {float(err_B1):.2e}")

# Test B2: Gelfond's constant e^pi = e^{4 varpi^2 / G*^2}
gelfond_std = exp(pi)
gelfond_ontic = exp(4 * varpi**2 / G_star**2)
err_B2 = abs(gelfond_std - gelfond_ontic) / gelfond_std
record("B2: Gelfond's constant e^pi = e^{4 varpi^2/G*^2}",
       err_B2 < TOL,
       f"e^pi = {float(gelfond_std):.10f}, ontic = {float(gelfond_ontic):.10f}, "
       f"rel err = {float(err_B2):.2e}")

print()

# =============================================================================
# PART C: EISENSTEIN SERIES (2 tests)
# =============================================================================

print("=" * 70)
print("PART C: EISENSTEIN SERIES E4(i)")
print("=" * 70)
print()

# Test C1: E4(i) via q-expansion (500 terms)
q = exp(-2 * pi)
E4_qexp = mpf(1) + 240 * fsum(sigma3(n) * q**n for n in range(1, 501))

# Test C2: E4(i) = 3 G*^8 / (256 varpi^4)
E4_ontic = 3 * G_star**8 / (256 * varpi**4)

# Also compute via classical: 3 * (varpi/pi)^4
E4_classical = 3 * (varpi / pi)**4

err_C1 = abs(E4_qexp - E4_ontic) / E4_ontic
record("C1: E4(i) q-expansion vs ontic form",
       err_C1 < mpf('1e-12'),
       f"E4_q = {float(E4_qexp):.15f}, E4_ontic = {float(E4_ontic):.15f}, "
       f"rel err = {float(err_C1):.2e}")

err_C2 = abs(E4_classical - E4_ontic) / E4_ontic
record("C2: E4(i) classical 3(varpi/pi)^4 vs 3G*^8/(256 varpi^4)",
       err_C2 < TOL,
       f"classical = {float(E4_classical):.15f}, ontic = {float(E4_ontic):.15f}, "
       f"rel err = {float(err_C2):.2e}")

print()

# =============================================================================
# PART D: EPSTEIN ZETA FACTORIZATION (3 tests)
# =============================================================================

print("=" * 70)
print("PART D: EPSTEIN ZETA FACTORIZATION")
print("=" * 70)
print()

# Test D1: Z_{Z[i]}(2) via direct 2D lattice sum
# Z = sum'_{(m,n)} (m^2 + n^2)^{-2}
# Use |m|,|n| <= L, exclude (0,0)
L = 500
Z_direct = mpf(0)
for m in range(-L, L + 1):
    for n in range(-L, L + 1):
        if m == 0 and n == 0:
            continue
        Z_direct += mpf(1) / (mpf(m)**2 + mpf(n)**2)**2

# Test D2: Z_{Z[i]}(2) = 4 * zeta(2) * L(2, chi_{-4})
# L(2, chi_{-4}) = Catalan's constant G
# zeta(2) = pi^2/6
G_catalan = catalan  # mpmath provides Catalan's constant
zeta_2 = zeta(2)
Z_factored = 4 * zeta_2 * G_catalan

err_D1 = abs(Z_direct - Z_factored) / Z_factored
record("D1: Z_{Z[i]}(2) lattice sum vs 4*zeta(2)*G",
       err_D1 < mpf('1e-5'),
       f"Z_direct = {float(Z_direct):.12f}, Z_factored = {float(Z_factored):.12f}, "
       f"rel err = {float(err_D1):.2e} (finite lattice)")

# Test D3: Z_{Z[i]}(2) in ontic form = 32 varpi^4 G / (3 G*^4)
Z_ontic = 32 * varpi**4 * G_catalan / (3 * G_star**4)

err_D2 = abs(Z_factored - Z_ontic) / Z_factored
record("D2: Z_{Z[i]}(2) factored vs ontic 32*varpi^4*G/(3*G*^4)",
       err_D2 < TOL,
       f"Z_fact = {float(Z_factored):.15f}, Z_ontic = {float(Z_ontic):.15f}, "
       f"rel err = {float(err_D2):.2e}")

# Test D3: zeta(2) in ontic form
zeta2_ontic = 8 * varpi**4 / (3 * G_star**4)
err_D3 = abs(zeta_2 - zeta2_ontic) / zeta_2
record("D3: zeta(2) = 8*varpi^4 / (3*G*^4)",
       err_D3 < TOL,
       f"zeta(2) = {float(zeta_2):.15f}, ontic = {float(zeta2_ontic):.15f}, "
       f"rel err = {float(err_D3):.2e}")

print()

# =============================================================================
# PART E: ROW DECOMPOSITION (2 tests)
# =============================================================================

print("=" * 70)
print("PART E: ROW DECOMPOSITION")
print("=" * 70)
print()

# Test E1: Z_{Z[i]}(2) via row-by-row coth summation
# Z = 2*zeta(4) + 2 * sum_{n=1}^{infty} sum_{m=-infty}^{infty} (m^2+n^2)^{-2}
# The bilateral row sum for fixed n:
#   sum_{m=-infty}^{infty} 1/(m^2+n^2)^2
#   = -d/da [pi*coth(pi*sqrt(a))/(2*sqrt(a)) + 1/(2a)] evaluated at a=n^2
# We use the formula: sum = (2 + pi/n * coth(n*pi) + (pi/n)^2 * csch(n*pi)^2) / (4*n^2)
# Wait, let's derive properly. For general a:
#   sum_{m=-infty}^{infty} 1/(m^2+a)^2 = -d/da [pi*coth(pi*sqrt(a))/sqrt(a)]
# = [pi^2 * csch^2(pi*sqrt(a)) / (2a) + pi*coth(pi*sqrt(a)) / (2*a^{3/2})] (taking positive)
# Actually from the paper: g'(a) formula gives the unilateral sum.
# For the bilateral sum at a = n^2:
#   sum_{m} 1/(m^2+n^2)^2 = pi^2*csch^2(n*pi)/(2*n^2) + pi*coth(n*pi)/(2*n^3)

N_rows = 500
zeta_4 = zeta(4)  # = pi^4/90
Z_row = 2 * zeta_4  # n=0 row: sum'_m m^{-4} = 2*zeta(4)

for n in range(1, N_rows + 1):
    n_mp = mpf(n)
    # Bilateral row sum: sum_{m=-infty}^{infty} (m^2 + n^2)^{-2}
    # Using differentiation of the bilateral coth formula
    bilateral_row = pi**2 * csch(n_mp * pi)**2 / (2 * n_mp**2) \
                  + pi * coth(n_mp * pi) / (2 * n_mp**3)
    Z_row += 2 * bilateral_row  # factor 2 for n and -n

# Tail correction: for large n, coth(n*pi) -> 1, csch(n*pi) -> 0
# so bilateral_row -> pi/(2*n^3). The missing tail is:
#   2 * sum_{n=N+1}^{infty} pi/(2*n^3) = pi * [zeta(3) - sum_{n=1}^{N} 1/n^3]
zeta_3 = zeta(3)
partial_zeta3 = fsum(mpf(1) / mpf(n)**3 for n in range(1, N_rows + 1))
Z_row += pi * (zeta_3 - partial_zeta3)

err_E1 = abs(Z_row - Z_factored) / Z_factored
record("E1: Z_{Z[i]}(2) row decomposition vs factored",
       err_E1 < mpf('1e-10'),
       f"Z_row = {float(Z_row):.15f}, Z_fact = {float(Z_factored):.15f}, "
       f"rel err = {float(err_E1):.2e}")

# Test E2: S embedded in Z_{Z[i]}(2): bilateral n=1 row = 2S - 1
bilateral_n1 = pi**2 * csch(pi)**2 / 2 + pi * coth(pi) / 2
embed_check = 2 * S_closed - 1

err_E2 = abs(bilateral_n1 - embed_check) / abs(embed_check)
record("E2: bilateral n=1 row = 2S - 1",
       err_E2 < TOL,
       f"bilateral = {float(bilateral_n1):.15f}, 2S-1 = {float(embed_check):.15f}, "
       f"rel err = {float(err_E2):.2e}")

print()

# =============================================================================
# PART F: THREE-TERM DECOMPOSITION (1 test)
# =============================================================================

print("=" * 70)
print("PART F: THREE-TERM DECOMPOSITION")
print("=" * 70)
print()

# The numerator of S decomposes as: constant + linear + quadratic in lambda
term_const = mpf(2) / 4
term_linear = 4 * lam * coth(4 * lam) / 4
term_quad = 16 * lam**2 * csch(4 * lam)**2 / 4
total = term_const + term_linear + term_quad

share_const = term_const / total * 100
share_linear = term_linear / total * 100
share_quad = term_quad / total * 100

err_F1 = abs(total - S_closed) / S_closed
record("F1: Three-term decomposition sums to S",
       err_F1 < TOL,
       f"sum = {float(total):.15f}, S = {float(S_closed):.15f}")

print(f"\n  Decomposition shares:")
print(f"    Constant (discrete residue):  {float(share_const):.1f}%")
print(f"    Linear (G* bridge):           {float(share_linear):.1f}%")
print(f"    Quadratic (curvature):        {float(share_quad):.1f}%")

print()

# =============================================================================
# PART G: NOME AND EXPONENTIAL SCALES (2 tests)
# =============================================================================

print("=" * 70)
print("PART G: NOME AND EXPONENTIAL SCALES")
print("=" * 70)
print()

# Test G1: nome q = e^{-2pi} = e^{-8 varpi^2/G*^2}
q_std = exp(-2 * pi)
q_ontic = exp(-8 * varpi**2 / G_star**2)
err_G1 = abs(q_std - q_ontic) / abs(q_std)
record("G1: nome q = e^{-8 varpi^2/G*^2}",
       err_G1 < TOL,
       f"q = {float(q_std):.6e}, ontic = {float(q_ontic):.6e}, "
       f"rel err = {float(err_G1):.2e}")

# Test G2: smallness of q controls csch^2 correction
csch_correction_pct = float(share_quad)
record("G2: csch^2 correction < 2% (nome suppression)",
       csch_correction_pct < 2.0,
       f"csch^2 share = {csch_correction_pct:.1f}%, "
       f"nome q = {float(q_std):.6e}")

print()

# =============================================================================
# SUMMARY
# =============================================================================

print("=" * 70)
print("SUMMARY")
print("=" * 70)
print()

n_pass = sum(1 for _, p, _ in results if p)
n_fail = sum(1 for _, p, _ in results if not p)
print(f"  Total: {len(results)} tests")
print(f"  Passed: {n_pass}")
print(f"  Failed: {n_fail}")
print()

if n_fail > 0:
    print("  FAILED TESTS:")
    for name, passed, detail in results:
        if not passed:
            print(f"    - {name}")
            if detail:
                print(f"      {detail}")
    print()

print("=" * 70)
print(f"  Key numerical values:")
print(f"    varpi            = {float(varpi):.15f}")
print(f"    G*               = {float(G_star):.15f}")
print(f"    lambda           = {float(lam):.15f}")
print(f"    pi (from ontic)  = {float(pi_from_ontic):.15f}")
print(f"    S                = {float(S_closed):.15f}")
print(f"    E4(i)            = {float(E4_ontic):.15f}")
print(f"    Z_{{Z[i]}}(2)      = {float(Z_factored):.15f}")
print(f"    Catalan G        = {float(G_catalan):.15f}")
print(f"    nome q           = {float(q_std):.6e}")
print("=" * 70)
