"""
Precision Formula Deep Dive — Part 2

Now that we know R(5) = 20, explore:
1. WHY truncate at index 5? What's special about 5?
2. Can the coefficients be derived from theta function structure?
3. What determines the signs?
4. Is there a generating function for the series?
5. What does the 5th term predict?
"""

import numpy as np
from scipy.special import gamma
from itertools import product as iterproduct

# Constants
G14 = gamma(0.25)
Gstar = np.sqrt(2) * G14**2 / (2 * np.pi)
varpi = G14**2 / (2 * np.sqrt(2 * np.pi))
W3 = G14**4 / (4 * np.pi**3)

K = 16 * Gstar**2
Delta = K**2 - 4*K*Gstar
xp = (K + np.sqrt(Delta)) / 2
xm = (K - np.sqrt(Delta)) / 2

alpha_inv_codata = 137.035999177
eps = np.exp(np.pi) - np.pi - 20
eps_abs = abs(eps)

Nc, Nb, b3, Neff = 3, 4, 7, 13
D = Nb**2 * Nc - 1  # 47

# r_2(n) for n = 0..50
def r2(n):
    """Number of representations of n as sum of two squares."""
    count = 0
    for a in range(-n, n+1):
        for b in range(-n, n+1):
            if a*a + b*b == n:
                count += 1
    return count

r2_vals = [r2(n) for n in range(51)]
R_cumulative = [0]
for n in range(1, 51):
    R_cumulative.append(R_cumulative[-1] + r2_vals[n])

# =====================================================
print("=" * 70)
print("PART I: WHY INDEX 5? WHAT'S SPECIAL ABOUT R(5)?")
print("=" * 70)
print()

print("r_2(n) and cumulative R(N) for n = 1..25:")
print(f"  {'n':>3s}  {'r_2(n)':>6s}  {'R(n)':>6s}  Notes")
for n in range(1, 26):
    notes = ""
    if R_cumulative[n] == 4: notes = "= N_base"
    elif R_cumulative[n] == 8: notes = "= BCC = 2^D"
    elif R_cumulative[n] == 12: notes = "= FCC"
    elif R_cumulative[n] == 20: notes = "= b_3 + N_eff"
    elif R_cumulative[n] == 24: notes = "= |O|"
    elif R_cumulative[n] == 26: notes = "= Moore"
    elif R_cumulative[n] == 48: notes = "= |O_h|"

    plateau = ""
    if n > 1 and r2_vals[n] == 0:
        plateau = " (plateau)"
    print(f"  {n:3d}  {r2_vals[n]:6d}  {R_cumulative[n]:6d}  {notes}{plateau}")

print()
print("KEY OBSERVATIONS:")
print("  1. R(5) = 20 and R(6) = R(7) = 20 (plateau: r_2(6) = r_2(7) = 0)")
print("     6 and 7 cannot be written as sums of two squares.")
print("     So R(5) = R(6) = R(7) = 20.")
print("     The plateau at 20 is the LONGEST zero-gap in the early r_2 sequence.")
print()
print("  2. The zero-gap {6, 7} corresponds to n = N_f and n = b_3")
print(f"     6 = 2*N_c = N_f (number of quark flavors)")
print(f"     7 = b_3 (QCD beta coefficient)")
print(f"     BOTH framework integers lie in the representation gap!")
print()

# Why can't 6 and 7 be written as sums of two squares?
# A number n can be written as a sum of two squares iff
# every prime factor of n of the form 4k+3 appears to an even power.
# 6 = 2 * 3. Since 3 = 4*0 + 3 appears to odd power (1), 6 fails.
# 7 = 7 = 4*1 + 3, a prime of form 4k+3 to odd power, so 7 fails.

print("  3. NUMBER THEORY: n is a sum of two squares iff every prime")
print("     factor of form 4k+3 appears to an even power.")
print("     6 = 2*3: prime 3 (form 4k+3) appears once. Fails.")
print("     7 = 7:   prime 7 (form 4k+3) appears once. Fails.")
print()
print("     The representation gap at {6,7} is forced by the")
print("     Fermat two-square theorem. It's a theorem of number theory,")
print("     not a selection.")

# =====================================================
print()
print("=" * 70)
print("PART II: THE PLATEAU STRUCTURE")
print("=" * 70)
print()

# Where are the plateaus (consecutive zeros of r_2)?
plateaus = []
in_plateau = False
start = 0
for n in range(1, 51):
    if r2_vals[n] == 0:
        if not in_plateau:
            start = n
            in_plateau = True
    else:
        if in_plateau:
            plateaus.append((start, n-1, R_cumulative[start-1]))
            in_plateau = False

print("Plateaus (consecutive n with r_2(n) = 0):")
print(f"  {'Start':>5s}-{'End':>3s}  {'Length':>6s}  {'R value':>7s}")
for s, e, R in plateaus:
    length = e - s + 1
    notes = ""
    if R == 20: notes = " <-- 20 = b_3 + N_eff"
    print(f"  {s:5d}-{e:3d}  {length:6d}  {R:7d}{notes}")

print()
print("  The {6,7} plateau (length 2) is the FIRST multi-point plateau.")
print("  Single gaps occur at n=3 (r_2(3)=0), but {6,7} is the first PAIR.")
print()

# What about the NEXT plateau?
# After {6,7}, single zeros at n=11,12,14,15,...
# These are not plateaus of length > 1 until later.

# =====================================================
print()
print("=" * 70)
print("PART III: COEFFICIENT STRUCTURE FROM THETA FUNCTION")
print("=" * 70)
print()

# The theta function squared is:
# theta_3(q)^2 = sum_{n=0}^inf r_2(n) * q^n

# G* = sqrt(2*pi) * theta_3(q)^2 where q = e^{-pi}
# So G* = sqrt(2*pi) * sum r_2(n) * q^n
# = sqrt(2*pi) * [1 + 4q + 4q^2 + 4q^4 + 8q^5 + 4q^8 + 4q^9 + 8q^10 + ...]

# The correction to G* from TRUNCATING the theta series at N terms:
# G*_N = sqrt(2*pi) * sum_{n=0}^{N} r_2(n) * q^n
# G*_inf - G*_N = sqrt(2*pi) * sum_{n=N+1}^{inf} r_2(n) * q^n

q = np.exp(-np.pi)
Gstar_terms = []
cumulative_Gstar = 0
for n in range(20):
    term = r2_vals[n] * q**n
    cumulative_Gstar += term
    Gstar_n = np.sqrt(2*np.pi) * cumulative_Gstar
    error = abs(Gstar_n - Gstar)
    if r2_vals[n] > 0 or n < 10:
        print(f"  G*_{n:2d} = sqrt(2pi) * R_cum = {Gstar_n:.15f}  error = {error:.2e}")

print(f"  G*_exact = {Gstar:.15f}")
print()

# The corrections to x+ from corrections to G*:
# x+ depends on G* through K = 16*G*^2 and KG* = 16*G*^3
# Small correction delta_G to G* produces delta_x+ = ?
# From x+ = 8G*^2 + 4G*sqrt(4G* - 1):  (using the root formula)
# dx+/dG* = 16G* + 4*sqrt(4G*-1) + 4G**(2/sqrt(4G*-1))
#         = 16G* + 4*sqrt(4G*-1) + 8G*/sqrt(4G*-1)

sqrt_term = np.sqrt(4*Gstar - 1)
dxp_dGstar = 16*Gstar + 4*sqrt_term + 8*Gstar/sqrt_term
print(f"dx+/dG* = {dxp_dGstar:.6f}")
print()

# =====================================================
print()
print("=" * 70)
print("PART IV: SEARCHING FOR COEFFICIENT DERIVATION")
print("=" * 70)
print()

# The coefficients are: c1 = 9/47, c2 = 5/64, c3 = 4/141, c4 = 141/11
# D = 47 = 16*3 - 1 = |O_h| - 1
# 141 = 3*47 = 3*(|O_h| - 1) = 3*|O_h| - 3

# Can we express the coefficients in terms of r_2?
print("Coefficients vs r_2 values:")
print(f"  c1 = 9/47 = N_c^2 / D")
print(f"     9 = N_c^2 = r_2(1) + r_2(4) + ... ? No, r_2(1) = 4, r_2(4) = 4")
print(f"     9 = sum of first two nonzero r_2: r_2(1) + r_2(2) + 1 = 4+4+1? No.")
print(f"     9 = r_2(1) + r_2(5) + 1... no pattern.")
print()

# Let me check: which r_2 values appear?
r2_nonzero = [(n, r2_vals[n]) for n in range(1, 20) if r2_vals[n] > 0]
print("  Non-zero r_2 values in order:")
for n, r in r2_nonzero:
    print(f"    r_2({n:2d}) = {r}")
print()

# Can the coefficient NUMERATORS and DENOMINATORS be related to r_2?
print("Coefficient numerators and denominators:")
coeffs = [(9, 47, "c1"), (5, 64, "c2"), (4, 141, "c3"), (141, 11, "c4")]
for num, den, name in coeffs:
    # Check if num or den appears in r_2 cumulative
    r2_match_num = [n for n in range(51) if R_cumulative[n] == num]
    r2_match_den = [n for n in range(51) if R_cumulative[n] == den]
    print(f"  {name} = {num}/{den}")
    if r2_match_num:
        print(f"    {num} = R({r2_match_num[0]})")
    if r2_match_den:
        print(f"    {den} = R({r2_match_den[0]})")

print()

# =====================================================
print()
print("=" * 70)
print("PART V: THE GENERATING FUNCTION HYPOTHESIS")
print("=" * 70)
print()

# If the correction series has a generating function, it should be
# related to the modular properties of theta_3(q)^2

# Key identity: G* = sqrt(2*pi) * theta_3(q)^2
# So theta_3(q)^2 = G* / sqrt(2*pi)

# The modular equation at tau = i:
# theta_3(q) is invariant under tau -> -1/tau (Poisson summation)
# This self-duality at tau = i is what DEFINES the self-dual point

# The Eisenstein series at tau = i:
# E_2(i) involves q-derivatives of theta_3
# E_4(i) = 1 + 240*sum sigma_3(n)*q^n = ... evaluated at q = e^{-pi}
# E_6(i) = 0 (because i is a zero of E_6 -- this is related to j(i) = 1728)

print("Modular forms at tau = i (q = e^{-pi}):")
print()

# E_4(i) = 3*(varpi/pi)^4 * (1/12) ...
# Actually, the Eisenstein series E_4 is related to theta functions:
# E_4 = (theta_2^8 + theta_3^8 + theta_4^8) / 2
# At tau = i: theta_2(q) = theta_4(q) (by Jacobi identity at self-dual point)
# And theta_3(q) is known.

# Let me compute E_4(i) directly from its q-expansion
def sigma_k(n, k):
    """Sum of k-th powers of divisors of n."""
    return sum(d**k for d in range(1, n+1) if n % d == 0)

E4_q = 1
for n in range(1, 30):
    E4_q += 240 * sigma_k(n, 3) * q**n

print(f"  E_4(i) = {E4_q:.10f}")
print(f"  3*G*^4/(4*pi^2) = {3*Gstar**4/(4*np.pi**2):.10f}")
# Actually E_4(tau) = (2*K(k)/pi)^4 * (1 - k^2 + k^4) at the corresponding k
# At k = 1/sqrt(2): 1 - 1/2 + 1/4 = 3/4
# K(1/sqrt(2)) = G14^2/(4*sqrt(pi))
# (2*K/pi)^4 = (G14^2/(2*pi*sqrt(pi)))^4 = G14^8/(16*pi^4*pi^2) = G14^8/(16*pi^6)
# E_4(i) = G14^8/(16*pi^6) * 3/4 = 3*G14^8/(64*pi^6)

E4_exact = 3 * G14**8 / (64 * np.pi**6)
print(f"  E_4(i) exact = 3*Gamma(1/4)^8/(64*pi^6) = {E4_exact:.10f}")
print(f"  Match: {abs(E4_q - E4_exact)/E4_exact:.2e}")
print()

# E_6(i) = 0 because i is a CM point with j = 1728 = 1728*E_4^3/(E_4^3 - E_6^2)
# j = 1728 => E_6 = 0
E6_q = 1
for n in range(1, 30):
    E6_q -= 504 * sigma_k(n, 5) * q**n

print(f"  E_6(i) = {E6_q:.10f}")
print(f"  Expected: 0 (because j(i) = 1728 requires E_6 = 0)")
print(f"  Close to zero: {abs(E6_q) < 0.01}")
print()

# The discriminant modular form:
# Delta(tau) = (E_4^3 - E_6^2) / 1728
# At tau = i: Delta = E_4^3 / 1728 (since E_6 = 0)
Delta_mod = E4_exact**3 / 1728
print(f"  Delta(i) = E_4(i)^3 / 1728 = {Delta_mod:.10e}")
print()

# Dedekind eta function:
# eta(i) = Gamma(1/4) / (2 * pi^(3/4))
eta_i = G14 / (2 * np.pi**0.75)
print(f"  eta(i) = Gamma(1/4)/(2*pi^(3/4)) = {eta_i:.10f}")
print(f"  eta(i)^24 = {eta_i**24:.10e}")
print(f"  Delta(i) from eta: eta(i)^24 * (2*pi)^12 = ... ")
# Actually Delta(tau) = (2*pi)^12 * eta(tau)^24
Delta_from_eta = (2*np.pi)**12 * eta_i**24
print(f"  = {Delta_from_eta:.10e}")
print(f"  Match with E_4 method: {abs(Delta_mod - Delta_from_eta)/Delta_mod:.2e}")
print()

# =====================================================
print()
print("=" * 70)
print("PART VI: THE CORRECTION AS MODULAR DEVIATION")
print("=" * 70)
print()

# The master quadratic root x+ deviates from 1/alpha by delta ~ 1.72e-4
# epsilon = e^pi - pi - 20 ~ -9e-4
# The ratio delta/|epsilon| = c1 = 9/47 at leading order

# Can we express the correction as a MODULAR FORM evaluation?
#
# The gap: x+ - 1/alpha = 1.72e-4
# = c1 * |eps| (at leading order)
# = (9/47) * |e^pi - pi - 20|
#
# Now: 9/47 = N_c^2 / (|O_h| - 1)
# And: |eps| = |1/q - pi - R(5)|
#
# So: x+ - 1/alpha ~ N_c^2/(|O_h|-1) * |1/q - pi - R(5)|
# = N_c^2/(|O_h|-1) * |modular deviation at tau=i|

print("Leading correction structure:")
print(f"  x+ - 1/alpha = c1 * |eps| = (N_c^2/(|O_h|-1)) * |1/q - pi - R(5)|")
print(f"               = ({Nc}^2/({48}-1)) * |e^pi - pi - 20|")
print(f"               = (9/47) * {eps_abs:.10e}")
print(f"               = {9/47 * eps_abs:.10e}")
print(f"  Actual gap    = {xp - alpha_inv_codata:.10e}")
print(f"  Residual after c1 = {xp - alpha_inv_codata - 9/47*eps_abs:.2e}")
print()

# =====================================================
print()
print("=" * 70)
print("PART VII: CAN WE PREDICT c5?")
print("=" * 70)
print()

# If there's a pattern in {c1, c2, c3, c4}, can we predict c5?
# c1 = 9/47    = N_c^2 / D
# c2 = 5/64    = (N_eff - 2*N_base) / N_base^3
# c3 = 4/141   = N_base / (N_c * D)
# c4 = 141/11  = (N_c * D) / (b_3 + N_base)

# Products: c1*c2 = 45/3008 = ...not clean
# c3*c4 = 4/11 [CLEAN]
# c1*c4 = 9*141/(47*11) = 1269/517 = 9*141/(47*11) = 9*3/11 = 27/11
print("Coefficient products:")
print(f"  c1 * c2 = {9*5}/{47*64} = {9*5/(47*64):.10f}")
print(f"  c1 * c3 = {9*4}/{47*141} = {36}/{6627} = {36/6627:.10f}")
print(f"  c1 * c4 = {9*141}/{47*11} = {9*141}/{47*11} = {9*141/(47*11):.10f}")
# 9*141 = 1269, 47*11 = 517
# 1269/517 = ?  1269 = 3*423 = 3*3*141 = 9*141. 517 = 11*47.
# So c1*c4 = 9*141/(11*47) = 9*3*47/(11*47) = 27/11
print(f"  c1 * c4 = 27/11 = {27/11:.10f} [CLEAN: N_c^3 / (b_3 + N_base)]")
print()

# c1*c4 = 27/11 = N_c^3 / (b_3 + N_base)
# c3*c4 = 4/11 = N_base / (b_3 + N_base)
# Both have denominator 11 = b_3 + N_base

# The pattern: c1 and c3 share numerator structure with D=47
# c4 is always the "inversion" partner with denominator b_3 + N_base = 11

# For c5, we might expect denominator involving the NEXT framework quantity
# The denominators so far: 47, 64, 141, 11
# 47 = |O_h| - 1
# 64 = N_base^3 = 4^3
# 141 = N_c * D = 3 * 47
# 11 = b_3 + N_base = 7 + 4

# What's left?
# 13 = N_eff (not yet used as denominator)
# 23 = N_c^3 - N_base = 27 - 4 (used in Higgs sector: lambda = 3/23)
# 3 = N_c (not yet used as denominator)

print("Denominators used: {47, 64, 141, 11}")
print("Framework denominators not yet used: {3, 13, 23}")
print()

# Pattern hypothesis: denominators cycle through framework quantities
# and their products. Numerators do the same.

# Let me check if c5 can be constrained by the RESIDUAL after 4 terms

# Compute the residual after 4 terms with high precision
c = [9/47, 5/64, 4/141, 141/11]
signs = [-1, +1, -1, -1]

result = xp
for i in range(4):
    result += signs[i] * c[i] * eps_abs**(i+1)

residual_4 = result - alpha_inv_codata
print(f"Residual after 4 terms: {residual_4:.6e}")
print(f"This must equal c5 * |eps|^5 * sign5")
print(f"|eps|^5 = {eps_abs**5:.6e}")
print(f"c5 * sign5 = residual / |eps|^5 = {residual_4 / eps_abs**5:.6f}")
print()

c5_estimate = residual_4 / eps_abs**5
print(f"If sign5 = +1: c5 = {c5_estimate:.6f}")
print(f"If sign5 = -1: c5 = {-c5_estimate:.6f}")
print()

# Check if c5 ~ a simple rational from framework integers
# c5 ~ 70.1 (if positive)
# 70 = 2 * 5 * 7 = 2 * 5 * b_3
# 141/2 = 70.5
# N_eff * (N_eff - 2*N_c) / (N_c - 1) ... let me search

print("Searching for rational expressions near c5 = {:.4f}:".format(abs(c5_estimate)))
target = abs(c5_estimate)
best_matches = []
for a in range(1, 200):
    for b in range(1, 200):
        if abs(a/b - target) < 0.01 * target:
            # Check if a and b are expressible from {3,4,7,13,47}
            best_matches.append((a, b, abs(a/b - target)))

best_matches.sort(key=lambda x: x[2])
print("  Best rational approximations (within 1%):")
for a, b, err in best_matches[:10]:
    print(f"    {a}/{b} = {a/b:.6f}  error = {err:.6f}")

print()

# =====================================================
print()
print("=" * 70)
print("SUMMARY: WHAT IS DERIVED vs WHAT IS OBSERVED")
print("=" * 70)
print()

print("DERIVED from CM curve E: y^2 = x^3 - x:")
print("  - e^pi = 1/q (inverse nome at tau = i)        [THEOREM]")
print("  - pi = 4*varpi^2/G*^2 (triad relation)        [THEOREM]")
print("  - R(5) = sum r_2(1..5) = 20                    [THEOREM]")
print("  - R(5) = R(6) = R(7) due to Fermat 2-square   [THEOREM]")
print("  - R(1) = 4 = N_base                            [THEOREM]")
print("  - R(2) = 8 = BCC = 2^D                         [THEOREM]")
print("  - R(16) = 48 = |O_h|                            [THEOREM]")
print()
print("OBSERVED but not derived:")
print("  - WHY truncate R at index 5 (not 4 or 8)?      [OPEN]")
print("  - WHY these specific rational coefficients?     [OPEN]")
print("  - WHY the sign pattern {-,+,-,-}?               [OPEN]")
print("  - The 5th coefficient (predicted ~70)            [PREDICTION]")
print()
print("STRUCTURAL INSIGHT:")
print("  epsilon = (inverse nome) - (circular period) - R(5)")
print("  = 1/q - pi - sum_{n=1}^{5} r_2(n)")
print("  All three terms come from the CM curve E.")
print("  The expansion parameter is INTERNAL to the curve's arithmetic,")
print("  not externally chosen.")
