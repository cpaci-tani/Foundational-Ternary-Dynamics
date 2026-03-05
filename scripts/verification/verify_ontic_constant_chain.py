#!/usr/bin/env python3
"""
Verification Script: The Ontic Constant Chain
==============================================

Verifies the mathematical relationships in the proposed ontic foundation chain:

    gamma  -->  varpi  -->  M (AGM)  -->  pi  -->  G*

where:
    gamma  = Euler-Mascheroni constant           ~ 0.5772156649
    varpi  = lemniscate constant                  ~ 2.6220575542
    M      = AGM(1, sqrt(2))                      ~ 1.1981402347
    pi     = ratio of circumference to diameter   ~ 3.1415926536
    G*     = lemniscatic constant                  ~ 2.9586751192

The chain traces how discrete counting (integers, harmonic series) connects
through gamma to the elliptic/lemniscatic world (varpi, G*) via the
arithmetic-geometric mean (M) and pi.

Sections:
    1. Compute all five constants to 100+ digit precision
    2. Verify exact algebraic relationships among varpi, M, pi, G*
    3. Explore gamma's role as the discrete-to-continuous bridge
    4. Logarithmic scaling and the Weierstrass product for 1/Gamma(z)
    5. Potential new relationships involving gamma
    6. The "decimal inversion" property: gamma as counting-to-scaling converter
    7. Summary table

Framework: Foundational Ternary Dynamics v5.17
Date: February 2026
"""

from mpmath import (
    mp, mpf, pi, euler, sqrt, log, exp, gamma, digamma,
    agm, zeta, inf, nsum, fac, harmonic, power, fraction,
    nstr,
)

mp.dps = 120  # 120 decimal digits of working precision


# =============================================================================
# UTILITY
# =============================================================================

def banner(title):
    """Print a section banner."""
    width = 88
    print()
    print("=" * width)
    print(f"  {title}")
    print("=" * width)
    print()


def sub_banner(title):
    """Print a sub-section banner."""
    width = 88
    print()
    print("-" * width)
    print(f"  {title}")
    print("-" * width)
    print()


def show(label, value, digits=50):
    """Print a labeled value to the specified number of digits."""
    print(f"  {label:42s} = {nstr(value, digits)}")


def check(label, lhs, rhs, tol_digits=90):
    """Check whether two values agree to tol_digits decimal places."""
    diff = abs(lhs - rhs)
    tol = power(10, -tol_digits)
    ok = diff < tol
    status = "EXACT MATCH" if ok else f"DIFF = {nstr(diff, 10)}"
    tag = "[PASS]" if ok else "[FAIL]"
    print(f"  {tag} {label:50s}  {status}")
    return ok


# =============================================================================
# SECTION 1: COMPUTE ALL FIVE CONSTANTS
# =============================================================================

banner("SECTION 1: The Five Ontic Constants (120-digit precision)")

# 1a. Euler-Mascheroni constant gamma
gamma_const = euler
show("gamma (Euler-Mascheroni)", gamma_const)

# 1b. Gamma(1/4) — the bridge quantity
Gamma_quarter = gamma(mpf(1) / 4)
show("Gamma(1/4)", Gamma_quarter)

# 1c. Lemniscate constant varpi
#     varpi = Gamma(1/4)^2 / (2 * sqrt(2*pi))
varpi = Gamma_quarter**2 / (2 * sqrt(2 * pi))
show("varpi (lemniscate constant)", varpi)

# 1d. M = AGM(1, sqrt(2))  — the Gauss AGM constant
M_agm = agm(1, sqrt(2))
show("M = AGM(1, sqrt(2))", M_agm)

# Also show 1/M = Gauss constant
gauss_const = 1 / M_agm
show("1/M (Gauss constant)", gauss_const)

# 1e. pi
show("pi", pi)

# 1f. G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)
G_star = sqrt(mpf(2)) * Gamma_quarter**2 / (2 * pi)
show("G* (lemniscatic constant)", G_star)

print()
print("  The proposed ontic ordering by 'foundational depth':")
print(f"    gamma  = {nstr(gamma_const, 30)}")
print(f"    varpi  = {nstr(varpi, 30)}")
print(f"    M      = {nstr(M_agm, 30)}")
print(f"    pi     = {nstr(pi, 30)}")
print(f"    G*     = {nstr(G_star, 30)}")


# =============================================================================
# SECTION 2: VERIFY EXACT ALGEBRAIC RELATIONSHIPS
# =============================================================================

banner("SECTION 2: Exact Algebraic Relationships")

pass_count = 0
total_count = 0

# 2a. varpi = pi / AGM(1, sqrt(2)) = pi / M
sub_banner("Identity: varpi = pi / M")
varpi_from_M = pi / M_agm
show("varpi (direct)", varpi)
show("pi / M", varpi_from_M)
total_count += 1
if check("varpi == pi / M", varpi, varpi_from_M):
    pass_count += 1

# 2b. G* = 2 * varpi / sqrt(pi)
sub_banner("Identity: G* = 2 * varpi / sqrt(pi)")
G_star_from_varpi = 2 * varpi / sqrt(pi)
show("G* (direct)", G_star)
show("2 * varpi / sqrt(pi)", G_star_from_varpi)
total_count += 1
if check("G* == 2 * varpi / sqrt(pi)", G_star, G_star_from_varpi):
    pass_count += 1

# 2c. G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)
sub_banner("Identity: G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)")
G_star_from_Gamma = sqrt(mpf(2)) * Gamma_quarter**2 / (2 * pi)
show("G* (direct)", G_star)
show("sqrt(2)*Gamma(1/4)^2/(2*pi)", G_star_from_Gamma)
total_count += 1
if check("G* == sqrt(2)*Gamma(1/4)^2/(2*pi)", G_star, G_star_from_Gamma):
    pass_count += 1

# 2d. pi = 4 * varpi^2 / G*^2  (algebraic rearrangement)
sub_banner("Identity: pi = 4 * varpi^2 / G*^2")
pi_from_varpi_Gstar = 4 * varpi**2 / G_star**2
show("pi (direct)", pi)
show("4 * varpi^2 / G*^2", pi_from_varpi_Gstar)
total_count += 1
if check("pi == 4 * varpi^2 / G*^2", pi, pi_from_varpi_Gstar):
    pass_count += 1

# 2e. M = pi / varpi  (rearrangement of 2a)
sub_banner("Identity: M = pi / varpi")
M_from_pi_varpi = pi / varpi
show("M (AGM)", M_agm)
show("pi / varpi", M_from_pi_varpi)
total_count += 1
if check("M == pi / varpi", M_agm, M_from_pi_varpi):
    pass_count += 1

# 2f. G*^2 = 4 * varpi^2 / pi  (rearranged from 2d)
sub_banner("Identity: G*^2 = 4 * varpi^2 / pi")
Gstar_sq_direct = G_star**2
Gstar_sq_from_varpi = 4 * varpi**2 / pi
show("G*^2 (direct)", Gstar_sq_direct)
show("4 * varpi^2 / pi", Gstar_sq_from_varpi)
total_count += 1
if check("G*^2 == 4 * varpi^2 / pi", Gstar_sq_direct, Gstar_sq_from_varpi):
    pass_count += 1

print()
print(f"  Algebraic identities: {pass_count}/{total_count} passed")


# =============================================================================
# SECTION 3: GAMMA AS THE DISCRETE-TO-CONTINUOUS BRIDGE
# =============================================================================

banner("SECTION 3: Gamma as the Discrete-to-Continuous Bridge")

# 3a. gamma = lim(n->inf) [H_n - ln(n)]
sub_banner("3a. gamma = lim [H_n - ln(n)]  (definition)")
print("  gamma connects the harmonic series (discrete) to the logarithm (continuous).")
print("  H_n = 1 + 1/2 + 1/3 + ... + 1/n  ~  ln(n) + gamma")
print()
for n in [10, 100, 1000, 10000]:
    H_n = harmonic(n)
    approx_gamma = H_n - log(n)
    err = abs(approx_gamma - gamma_const)
    print(f"  n = {n:6d}:  H_n - ln(n) = {nstr(approx_gamma, 20)},  "
          f"error = {nstr(err, 6)}")
print()
print(f"  Exact gamma              = {nstr(gamma_const, 40)}")

# 3b. Laurent expansion of Gamma(s) near s = 0: Gamma(s) = 1/s - gamma + O(s)
sub_banner("3b. Gamma(s) near s = 0: Gamma(s) ~ 1/s - gamma + ...")
print("  Check: s * Gamma(s) -> 1  and  s * Gamma(s) - 1 -> -gamma * s")
for s_val in [mpf('0.001'), mpf('0.0001'), mpf('0.00001')]:
    Gamma_s = gamma(s_val)
    residue = s_val * Gamma_s
    correction = (s_val * Gamma_s - 1) / s_val
    print(f"  s = {nstr(s_val, 6)}:  s*Gamma(s) = {nstr(residue, 20)},  "
          f"(s*Gamma(s)-1)/s = {nstr(correction, 20)}")
print(f"  Expected limit: -gamma  = {nstr(-gamma_const, 20)}")

# 3c. Digamma function: psi(1) = -gamma
sub_banner("3c. Digamma: psi(1) = -gamma")
psi_1 = digamma(1)
show("psi(1) = digamma(1)", psi_1)
show("-gamma", -gamma_const)
total_count += 1
if check("psi(1) == -gamma", psi_1, -gamma_const):
    pass_count += 1

# 3d. psi(1/4) = -gamma - pi/2 - 3*ln(2)
sub_banner("3d. Digamma at 1/4: psi(1/4) = -gamma - pi/2 - 3*ln(2)")
psi_quarter = digamma(mpf(1) / 4)
psi_quarter_formula = -gamma_const - pi / 2 - 3 * log(2)
show("psi(1/4) computed", psi_quarter)
show("-gamma - pi/2 - 3*ln(2)", psi_quarter_formula)
total_count += 1
if check("psi(1/4) == -gamma - pi/2 - 3*ln(2)", psi_quarter, psi_quarter_formula):
    pass_count += 1

print()
print("  Connection chain:  gamma  -->  psi(1/4)  -->  Gamma(1/4)  -->  varpi  -->  G*")
print("  gamma appears in the digamma psi(1/4), which relates to d/ds[ln Gamma(s)] at s=1/4.")
print("  Gamma(1/4) is the value of the Gamma function at 1/4, and from it varpi and G* follow.")


# =============================================================================
# SECTION 4: LOGARITHMIC SCALING AND THE WEIERSTRASS PRODUCT
# =============================================================================

banner("SECTION 4: Weierstrass Product — Gamma Embeds gamma into Gamma(z)")

# The Weierstrass product formula:
#   1/Gamma(z) = z * exp(gamma * z) * prod_{n=1}^{inf} [(1 + z/n) * exp(-z/n)]
#
# gamma is literally the exponential growth rate inside 1/Gamma(z).

sub_banner("4a. Weierstrass product for 1/Gamma(z)")
print("  1/Gamma(z) = z * exp(gamma*z) * prod_{n=1}^{inf} [(1+z/n)*exp(-z/n)]")
print()
print("  Significance: gamma is the EXPONENTIAL RATE controlling how the")
print("  Gamma function (and hence Gamma(1/4), varpi, G*) scales with z.")
print()

# Verify the Weierstrass product numerically at z = 1/4
z_test = mpf(1) / 4
# Compute partial product
inv_Gamma_direct = 1 / gamma(z_test)

N_terms = 500
partial_product = z_test * exp(gamma_const * z_test)
for n in range(1, N_terms + 1):
    partial_product *= (1 + z_test / n) * exp(-z_test / n)

print(f"  At z = 1/4:")
show("1/Gamma(1/4) exact", inv_Gamma_direct, 30)
show(f"Weierstrass product ({N_terms} terms)", partial_product, 30)
print(f"  Relative error: {nstr(abs(partial_product - inv_Gamma_direct) / abs(inv_Gamma_direct), 6)}")

sub_banner("4b. exp(gamma) and exp(-gamma)")
show("exp(gamma)", exp(gamma_const))
show("exp(-gamma)", exp(-gamma_const))
print()
print("  exp(-gamma) ~ 0.5615 is the 'discount factor' by which the discrete")
print("  harmonic world undershoots the continuous logarithmic world.")
print("  It controls the rate at which Gamma(1/4), and hence varpi and G*, scale.")


# =============================================================================
# SECTION 5: POTENTIAL NEW RELATIONSHIPS INVOLVING GAMMA
# =============================================================================

banner("SECTION 5: Exploring Relationships Involving gamma")

sub_banner("5a. Products and ratios with gamma")
show("gamma * varpi", gamma_const * varpi)
show("gamma * pi", gamma_const * pi)
show("gamma * G*", gamma_const * G_star)
show("exp(-gamma) * G*", exp(-gamma_const) * G_star)
show("exp(-gamma) * varpi", exp(-gamma_const) * varpi)
show("gamma / ln(2)", gamma_const / log(2))
show("gamma * sqrt(pi)", gamma_const * sqrt(pi))
show("gamma^2 * pi", gamma_const**2 * pi)

print()
print("  Notable near-matches:")
val_1 = exp(-gamma_const) * G_star
print(f"    exp(-gamma) * G*  = {nstr(val_1, 30)}   (~ 1.661, close to 5/3 = {nstr(mpf(5)/3, 10)})")
val_2 = gamma_const * pi
print(f"    gamma * pi        = {nstr(val_2, 30)}   (~ 1.813)")
val_3 = gamma_const / log(2)
print(f"    gamma / ln(2)     = {nstr(val_3, 30)}   (~ 0.8327, close to 1/M = {nstr(gauss_const, 10)})")
print(f"      Difference: gamma/ln(2) - 1/M = {nstr(val_3 - gauss_const, 15)}")
print(f"      (Not exact, but remarkably close: ~ 0.002 relative)")

sub_banner("5b. gamma + 1/(12*gamma)  — Bernoulli connection")
bernoulli_approx = gamma_const + 1 / (12 * gamma_const)
show("gamma + 1/(12*gamma)", bernoulli_approx)
print(f"    (~ {nstr(bernoulli_approx, 15)}, compare to 1/sqrt(2*pi*e) ~ {nstr(1/sqrt(2*pi*exp(1)), 15)})")

sub_banner("5c. Harmonic numbers at FTD integers {3, 4, 7, 13}")
ftd_integers = [3, 4, 7, 13]
print(f"  {'n':>4s}  {'H_n':>30s}  {'H_n - ln(n)':>20s}  {'H_n - gamma':>20s}")
print(f"  {'':>4s}  {'':>30s}  {'(-> gamma)':>20s}  {'(= ln(n) approx)':>20s}")
print("  " + "-" * 80)
for n in ftd_integers:
    H_n = harmonic(n)
    diff_from_gamma = H_n - log(n)
    excess = H_n - gamma_const
    print(f"  {n:4d}  {nstr(H_n, 30):>30s}  {nstr(diff_from_gamma, 15):>20s}  "
          f"{nstr(excess, 15):>20s}")

print()
print("  Interesting combinations of harmonic numbers at FTD integers:")
H3 = harmonic(3)
H4 = harmonic(4)
H7 = harmonic(7)
H13 = harmonic(13)
show("H_3", H3, 20)
show("H_4", H4, 20)
show("H_7", H7, 20)
show("H_13", H13, 20)
show("H_13 - H_3", H13 - H3, 20)
show("H_7 / H_3", H7 / H3, 20)
show("H_4 * H_7", H4 * H7, 20)
show("(H_13 - H_3) * pi", (H13 - H3) * pi, 20)
show("H_13 / gamma", H13 / gamma_const, 20)

print()
# Check if any combination lands near alpha or 1/alpha
alpha_ftd = mpf(1) / mpf('137.035999177')
show("alpha (CODATA)", alpha_ftd, 20)
show("H_3 * H_4 / (H_7 * H_13)", H3 * H4 / (H7 * H13), 20)
show("exp(H_4 - gamma) (should ~ 4)", exp(H4 - gamma_const), 20)
show("exp(H_7 - gamma) (should ~ 7)", exp(H7 - gamma_const), 20)
show("exp(H_13 - gamma) (should ~ 13)", exp(H13 - gamma_const), 20)

sub_banner("5d. Basel problem: zeta(2) = pi^2/6  (integers -> pi)")
zeta_2 = zeta(2)
pi_sq_over_6 = pi**2 / 6
show("zeta(2)", zeta_2, 30)
show("pi^2 / 6", pi_sq_over_6, 30)
total_count += 1
if check("zeta(2) == pi^2/6", zeta_2, pi_sq_over_6):
    pass_count += 1

sub_banner("5e. Gamma in zeta: zeta(1+eps) ~ 1/eps + gamma + O(eps)")
print("  The pole of zeta(s) at s=1 has residue 1 and the next term is gamma.")
for eps_val in [mpf('0.01'), mpf('0.001'), mpf('0.0001')]:
    zeta_val = zeta(1 + eps_val)
    estimated_gamma = zeta_val - 1 / eps_val
    print(f"  eps = {nstr(eps_val, 5)}:  zeta(1+eps) - 1/eps = {nstr(estimated_gamma, 20)}")
print(f"  Expected: gamma = {nstr(gamma_const, 20)}")

sub_banner("5f. Connecting zeta values to the chain")
show("zeta(2) = pi^2/6", zeta_2, 20)
show("zeta(4) = pi^4/90", zeta(4), 20)
# Since pi = 4*varpi^2/G*^2, we have pi^2 = 16*varpi^4/G*^4
# So zeta(2) = pi^2/6 = 16*varpi^4 / (6*G*^4)
zeta2_from_varpi = 16 * varpi**4 / (6 * G_star**4)
show("zeta(2) via varpi, G*: 16*varpi^4/(6*G*^4)", zeta2_from_varpi, 20)
total_count += 1
if check("zeta(2) == 16*varpi^4/(6*G*^4)  [pi^2/6 with pi=4*varpi^2/G*^2]",
         zeta_2, zeta2_from_varpi):
    pass_count += 1


# =============================================================================
# SECTION 6: GAMMA AS THE COUNTING-TO-SCALING CONVERTER
# =============================================================================

banner("SECTION 6: Gamma as the Counting <-> Scaling Inversion Constant")

sub_banner("6a. The fundamental asymptotic: H_n ~ ln(n) + gamma")
print("  The harmonic number H_n = sum_{k=1}^{n} 1/k counts the 'harmonic weight'")
print("  of the first n integers.  Its continuous analog is ln(n).")
print("  The offset between them is EXACTLY gamma in the limit.")
print()
print("  This means: gamma is the universal correction term that bridges")
print("  DISCRETE COUNTING (integers, sums) and CONTINUOUS SCALING (logarithms).")

sub_banner("6b. The inversion: exp(H_n - gamma) ~ n")
print("  Rearranging H_n ~ ln(n) + gamma:")
print("    H_n - gamma ~ ln(n)")
print("    exp(H_n - gamma) ~ n")
print()
print("  Without gamma, you CANNOT invert from continuous back to discrete.")
print()
print(f"  {'n':>6s}  {'exp(H_n - gamma)':>25s}  {'ratio to n':>15s}")
print("  " + "-" * 52)
for n in [1, 2, 3, 4, 5, 7, 10, 13, 50, 100, 137, 1000]:
    H_n = harmonic(n)
    recovered = exp(H_n - gamma_const)
    ratio = recovered / n
    print(f"  {n:6d}  {nstr(recovered, 18):>25s}  {nstr(ratio, 12):>15s}")
print()
print("  The ratio -> 1 as n -> inf, but the correction terms are O(1/(2n)).")
print("  For small n, the discrepancy reveals sub-asymptotic structure.")

sub_banner("6c. exp(-gamma) as the 'decimal discount factor'")
exp_neg_gamma = exp(-gamma_const)
show("exp(-gamma)", exp_neg_gamma, 30)
print()
print("  exp(-gamma) ~ 0.5615 controls the 'conversion rate' between the")
print("  discrete harmonic world and the continuous logarithmic world:")
print()
print("    prod_{n=1}^{N} (1/n) * exp(ln N)  =  N * 1/(N!)^{1/...}")
print()
print("  More precisely, the Mertens theorem states:")
print("    prod_{p <= N, p prime} (1 - 1/p) ~ exp(-gamma) / ln(N)")
print()
print("  So exp(-gamma) even governs the distribution of PRIMES among integers.")

sub_banner("6d. The complete chain: gamma -> Gamma(1/4) -> varpi -> G* -> alpha")
print("  gamma enters through the Weierstrass product of 1/Gamma(z):")
print("    1/Gamma(z) = z * exp(gamma*z) * prod_{n>=1} [(1+z/n)*exp(-z/n)]")
print()
print("  At z = 1/4:")
print("    1/Gamma(1/4) = (1/4) * exp(gamma/4) * prod_{n>=1} [(1+1/(4n))*exp(-1/(4n))]")
print()
# Show the numerical chain
show("gamma", gamma_const, 30)
show("exp(gamma/4)", exp(gamma_const / 4), 30)
show("Gamma(1/4)", Gamma_quarter, 30)
show("varpi = Gamma(1/4)^2 / (2*sqrt(2*pi))", varpi, 30)
show("G* = 2*varpi / sqrt(pi)", G_star, 30)

# The master quadratic
print()
print("  The FTD master quadratic: x^2 - 16*G*^2 * x + 16*G*^3 = 0")
c = G_star
disc = (16 * c**2)**2 - 4 * 16 * c**3
x_plus = (16 * c**2 + sqrt(disc)) / 2
x_minus = (16 * c**2 - sqrt(disc)) / 2
show("x_+ (-> 1/alpha)", x_plus, 30)
show("x_- (-> N_c)", x_minus, 30)
show("1/alpha (CODATA 2022)", mpf('137.035999177'), 20)
show("Discrepancy x_+ vs 1/alpha", abs(x_plus - mpf('137.035999177')), 10)
print()
print("  Complete chain:")
print("    gamma (discrete<->continuous)")
print("      |-> Gamma(1/4) (via Weierstrass product)")
print("          |-> varpi (lemniscate constant)")
print("              |-> G* (lemniscatic constant)")
print("                  |-> master quadratic -> 1/alpha = 137.036...")


# =============================================================================
# SECTION 7: SUMMARY TABLE
# =============================================================================

banner("SECTION 7: Summary")

sub_banner("The Five Ontic Constants")
print(f"  {'Constant':15s}  {'Symbol':8s}  {'Value (30 digits)':>40s}  {'Role'}")
print("  " + "-" * 100)
print(f"  {'Euler-Masch.':15s}  {'gamma':8s}  {nstr(gamma_const, 30):>40s}  "
      f"Discrete <-> continuous bridge")
print(f"  {'Lemniscate':15s}  {'varpi':8s}  {nstr(varpi, 30):>40s}  "
      f"pi of the lemniscate curve")
print(f"  {'Gauss AGM':15s}  {'M':8s}  {nstr(M_agm, 30):>40s}  "
      f"AGM(1, sqrt(2)); links varpi to pi")
print(f"  {'Archimedes':15s}  {'pi':8s}  {nstr(pi, 30):>40s}  "
      f"Circular geometry")
print(f"  {'Lemniscatic':15s}  {'G*':8s}  {nstr(G_star, 30):>40s}  "
      f"FTD master constant -> alpha")

sub_banner("Verified Exact Relations")
relations = [
    ("varpi = pi / M", "EXACT", "Links lemniscate to circle via AGM"),
    ("G* = 2 * varpi / sqrt(pi)", "EXACT", "Connects G* to varpi"),
    ("G* = sqrt(2)*Gamma(1/4)^2/(2*pi)", "EXACT", "Definition of G*"),
    ("pi = 4 * varpi^2 / G*^2", "EXACT", "Algebraic rearrangement"),
    ("M = pi / varpi", "EXACT", "Rearrangement"),
    ("G*^2 = 4 * varpi^2 / pi", "EXACT", "Rearrangement"),
    ("psi(1) = -gamma", "EXACT", "Digamma at 1"),
    ("psi(1/4) = -gamma - pi/2 - 3*ln(2)", "EXACT", "Digamma at 1/4"),
    ("zeta(2) = pi^2 / 6", "EXACT", "Basel problem"),
    ("zeta(2) = 16*varpi^4/(6*G*^4)", "EXACT", "Substituting pi^2 = 16*varpi^4/G*^4"),
]
print(f"  {'Relation':42s}  {'Status':8s}  {'Interpretation'}")
print("  " + "-" * 100)
for rel, status, interp in relations:
    print(f"  {rel:42s}  {status:8s}  {interp}")

sub_banner("Gamma's Role: Discrete <-> Continuous Inversion")
print("  1. gamma = lim [H_n - ln(n)]        -- defines the offset")
print("  2. exp(H_n - gamma) ~ n              -- inverts continuous back to discrete")
print("  3. Weierstrass: gamma enters Gamma(z) -- scales the Gamma function")
print("  4. psi(1/4) involves gamma           -- connects to Gamma(1/4)")
print("  5. Gamma(1/4) -> varpi -> G*         -- algebraic chain")
print("  6. G* -> master quadratic -> alpha   -- physics emerges")
print()
print("  Without gamma, there is no bridge from counting (1,2,3,...) to the")
print("  elliptic/lemniscatic world from which the fine structure constant arises.")
print()
print("  gamma is the FIRST constant in the ontic chain: it is where the discrete")
print("  integer world first makes contact with the continuous analytical world.")

sub_banner("Near-Miss Observations (Not Exact)")
print(f"  gamma / ln(2)         = {nstr(gamma_const / log(2), 20)}")
print(f"  1/M (Gauss constant)  = {nstr(gauss_const, 20)}")
print(f"    Relative diff:  {nstr(abs(gamma_const/log(2) - gauss_const)/gauss_const, 6)}")
print()
print(f"  exp(-gamma) * G*      = {nstr(exp(-gamma_const) * G_star, 20)}")
print(f"    (~ 1.661, suggestively close to 5/3 = 1.6667)")
print()
print(f"  exp(H_4 - gamma)      = {nstr(exp(H4 - gamma_const), 20)}")
print(f"    (should be ~ 4; actual ~ {nstr(exp(H4 - gamma_const), 8)}, "
      f"off by {nstr(abs(exp(H4 - gamma_const) - 4), 4)})")

sub_banner(f"Final Score: {pass_count}/{total_count} exact identities verified")
if pass_count == total_count:
    print("  All exact algebraic relationships confirmed to 90+ digit precision.")
else:
    print(f"  {total_count - pass_count} relationship(s) did not match to required precision.")

print()
print("=" * 88)
print("  END OF VERIFICATION")
print("=" * 88)
print()
