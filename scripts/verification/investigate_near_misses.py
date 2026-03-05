#!/usr/bin/env python3
"""
Deep Investigation of the Two Most Compelling Near-Misses
==========================================================

Near-miss 1: ln(M)/ln(G*) ≈ 1/6  (0.011% deviation = 109 ppm)
    If exact: M = G*^(1/6) — the AGM constant is the sixth root of G*

Near-miss 2: exp(ψ(1/3) - ψ(1/4)) ≈ 3 = N_c  (0.32% deviation)
    If exact: the digamma difference at the two primary FTD integers
    would encode the number of color charges

This script investigates:
  A. Whether correction terms from known identities close either gap
  B. The exact deviation and whether it matches any framework expression
  C. Cross-connections between the two near-misses
  D. Higher-order or compositional relationships that might be exact

Author: Claude Code
Date: February 10, 2026
"""

from mpmath import (mp, mpf, pi, euler, gamma as gammafunc, sqrt, log, exp,
                    agm, zeta, digamma, nstr, fabs, power, bernoulli,
                    loggamma, psi as polygamma_raw, harmonic, floor)

mp.dps = 300  # Very high precision for detecting subtle structure

def banner(title):
    print()
    print("=" * 100)
    print(f"  {title}")
    print("=" * 100)
    print()

def sub_banner(title):
    print()
    print("-" * 100)
    print(f"  {title}")
    print("-" * 100)
    print()

def show(label, value, digits=35):
    print(f"  {label:65s} = {nstr(value, digits)}")

# =============================================================================
# CONSTANTS at 300-digit precision
# =============================================================================

gamma_const = euler
Gamma_quarter = gammafunc(mpf(1)/4)
varpi = Gamma_quarter**2 / (2 * sqrt(2 * pi))
M_agm = agm(1, sqrt(2))
gauss_const = 1 / M_agm
G_star = sqrt(2) * Gamma_quarter**2 / (2 * pi)

N_c, N_base, b_3, N_eff = 3, 4, 7, 13

disc = (16 * G_star**2)**2 - 4 * 16 * G_star**3
x_plus = (16 * G_star**2 + sqrt(disc)) / 2
x_minus = (16 * G_star**2 - sqrt(disc)) / 2

alpha_inv = x_plus
alpha = 1 / alpha_inv

# =============================================================================
# NEAR-MISS 1: ln(M)/ln(G*) ≈ 1/6
# =============================================================================

banner("NEAR-MISS 1: ln(M)/ln(G*) and 1/6")

ln_M = log(M_agm)
ln_G = log(G_star)
ratio = ln_M / ln_G

show("ln(M) = ln(AGM(1,sqrt(2)))", ln_M, 50)
show("ln(G*)", ln_G, 50)
show("ln(M)/ln(G*)", ratio, 50)
show("1/6", mpf(1)/6, 50)
print()

delta_1 = ratio - mpf(1)/6
show("delta_1 = ln(M)/ln(G*) - 1/6", delta_1, 50)
show("|delta_1| relative", fabs(delta_1) / (mpf(1)/6), 20)
print(f"  This is {float(fabs(delta_1) / (mpf(1)/6) * 1e6):.1f} ppm")
print()

sub_banner("1a. What WOULD ln(M)/ln(G*) = 1/6 mean?")
print("  If exact: M = G*^(1/6)")
print("  Check: G*^(1/6) vs M:")
G_sixth = G_star**(mpf(1)/6)
show("G*^(1/6)", G_sixth, 40)
show("M = AGM(1,sqrt(2))", M_agm, 40)
show("G*^(1/6) - M", G_sixth - M_agm, 50)
show("relative difference", fabs(G_sixth - M_agm)/M_agm, 20)
print()

# What would this imply for varpi?
# M = pi/varpi, so pi/varpi = G*^(1/6)
# → varpi = pi · G*^(-1/6)
# But also G* = 2*varpi/sqrt(pi)
# → varpi = pi · (2*varpi/sqrt(pi))^(-1/6)
# → varpi = pi · (2^(-1/6)) · varpi^(-1/6) · pi^(1/12)
# → varpi^(7/6) = 2^(-1/6) · pi^(13/12)
# → varpi = (2^(-1/6) · pi^(13/12))^(6/7) = 2^(-1/7) · pi^(13/14)
print("  If M = G*^(1/6), then combining with G* = 2*varpi/sqrt(pi):")
print("    varpi would satisfy varpi^(7/6) = 2^(-1/6) * pi^(13/12)")
print("    → varpi = 2^(-1/7) * pi^(13/14)")
varpi_test = 2**(mpf(-1)/7) * pi**(mpf(13)/14)
show("2^(-1/7) * pi^(13/14)", varpi_test, 40)
show("actual varpi", varpi, 40)
show("difference", varpi_test - varpi, 50)
show("relative", fabs(varpi_test - varpi)/varpi, 20)
print()

sub_banner("1b. The exact deviation: does delta_1 have structure?")
print("  delta_1 = ln(M)/ln(G*) - 1/6")
show("delta_1", delta_1, 50)
print()

# Check if delta_1 relates to framework constants
for name, val in [
    ("alpha", alpha),
    ("alpha^2", alpha**2),
    ("gamma/100", gamma_const/100),
    ("gamma^2", gamma_const**2),
    ("1/(6*x_+)", 1/(6*x_plus)),
    ("alpha/6", alpha/6),
    ("1/(6*x_+^2)", 1/(6*x_plus**2)),
    ("gamma*alpha", gamma_const*alpha),
    ("exp(-x_+)/6", exp(-x_plus)/6),
    ("pi/(6*x_+^2)", pi/(6*x_plus**2)),
    ("ln(2)/(6*x_+)", log(2)/(6*x_plus)),
    ("gamma/(6*x_+)", gamma_const/(6*x_plus)),
    ("1/(36*G*^2)", 1/(36*G_star**2)),
]:
    ratio_test = delta_1 / val
    if 0.5 < fabs(ratio_test) < 5:
        print(f"  delta_1 / ({name}) = {nstr(ratio_test, 20)}")
    elif fabs(fabs(ratio_test) - round(float(fabs(ratio_test)))) < 0.05 and round(float(fabs(ratio_test))) < 50:
        n = round(float(ratio_test))
        print(f"  delta_1 / ({name}) = {nstr(ratio_test, 20)}  (near integer {n})")

print()

# Check if delta_1 is a known constant times a small rational
for p in range(-6, 7):
    for q in range(1, 13):
        test = delta_1 * q / (gamma_const**p if p != 0 else 1)
        if fabs(test - round(float(test))) < 0.01 and 0 < fabs(round(float(test))) < 20:
            n = round(float(test))
            print(f"  delta_1 * {q} / gamma^{p} ~ {n}  (err = {nstr(fabs(test - n), 10)})")

for p in range(-4, 5):
    for q in range(1, 13):
        test = delta_1 * q / (pi**p if p != 0 else 1)
        if fabs(test - round(float(test))) < 0.01 and 0 < fabs(round(float(test))) < 50:
            n = round(float(test))
            print(f"  delta_1 * {q} / pi^{p} ~ {n}  (err = {nstr(fabs(test - n), 10)})")

sub_banner("1c. Nearby simple fractions")
print("  What simple fraction p/q is closest to ln(M)/ln(G*)?")
# Continued fraction expansion
val = ratio
for n_terms in [5, 10, 15, 20]:
    cf = []
    v = val
    for _ in range(n_terms):
        a = int(floor(v))
        cf.append(a)
        frac = v - a
        if fabs(frac) < mpf('1e-100'):
            break
        v = 1/frac

    # Compute convergents
    h_prev, h_curr = 0, 1
    k_prev, k_curr = 1, 0
    convergents = []
    for a in cf:
        h_prev, h_curr = h_curr, a * h_curr + h_prev
        k_prev, k_curr = k_curr, a * k_curr + k_prev
        convergents.append((h_curr, k_curr))

    print(f"  Continued fraction coefficients ({n_terms} terms): {cf}")
    print(f"  Best convergents:")
    for h, k in convergents[:12]:
        approx = mpf(h) / k
        err = fabs(approx - ratio) / ratio
        print(f"    {h}/{k} = {nstr(approx, 20)}  (rel_err = {nstr(err, 8)})")
    print()


# =============================================================================
# NEAR-MISS 2: exp(psi(1/3) - psi(1/4)) ≈ 3
# =============================================================================

banner("NEAR-MISS 2: exp(psi(1/3) - psi(1/4)) and N_c = 3")

psi_third = digamma(mpf(1)/3)
psi_quarter = digamma(mpf(1)/4)
diff_psi = psi_third - psi_quarter

show("psi(1/3)", psi_third, 50)
show("psi(1/4)", psi_quarter, 50)
show("psi(1/3) - psi(1/4)", diff_psi, 50)
show("ln(3)", log(3), 50)
print()

# The exact closed forms:
# psi(1/3) = -gamma - (3/2)ln3 - pi/(2*sqrt(3))
# psi(1/4) = -gamma - pi/2 - 3*ln2
# psi(1/3) - psi(1/4) = 3*ln2 + pi/2 - (3/2)*ln3 - pi/(2*sqrt(3))
exact_diff = 3*log(2) + pi/2 - mpf(3)/2*log(3) - pi/(2*sqrt(3))
show("3*ln2 + pi/2 - (3/2)*ln3 - pi/(2*sqrt(3))", exact_diff, 50)
print()

exp_diff = exp(diff_psi)
show("exp(psi(1/3) - psi(1/4))", exp_diff, 50)
show("N_c = 3", mpf(3), 50)
print()

delta_2 = exp_diff - 3
show("delta_2 = exp(diff) - 3", delta_2, 50)
show("|delta_2| / 3", fabs(delta_2)/3, 20)
print(f"  This is {float(fabs(delta_2)/3 * 100):.4f}%")
print()

sub_banner("2a. What WOULD exp(psi(1/3) - psi(1/4)) = 3 mean?")
print("  It would mean: psi(1/3) - psi(1/4) = ln(3)")
print("  i.e., 3*ln2 + pi/2 - (3/2)*ln3 - pi/(2*sqrt(3)) = ln(3)")
print("  i.e., 3*ln2 + pi/2 - (5/2)*ln3 = pi/(2*sqrt(3))")
print("  i.e., pi*(1/2 - 1/(2*sqrt(3))) = (5/2)*ln3 - 3*ln2")
print("  i.e., pi*(sqrt(3)-1)/(2*sqrt(3)) = ln(243/8)")
print()
lhs_test = pi * (sqrt(3) - 1) / (2*sqrt(3))
rhs_test = log(mpf(243)/8)
show("pi*(sqrt(3)-1)/(2*sqrt(3))", lhs_test, 40)
show("ln(243/8) = ln(3^5/2^3)", rhs_test, 40)
show("difference", lhs_test - rhs_test, 50)
print()
print("  These are NOT equal. The difference is the deviation from N_c = 3.")

sub_banner("2b. The exact deviation: what is exp(diff) - 3?")
show("delta_2 = exp(diff) - 3", delta_2, 50)
print()

# Check if delta_2 matches framework expressions
for name, val in [
    ("alpha", alpha),
    ("3*alpha", 3*alpha),
    ("alpha/pi", alpha/pi),
    ("gamma*alpha", gamma_const*alpha),
    ("3*gamma*alpha", 3*gamma_const*alpha),
    ("1/x_+", 1/x_plus),
    ("x_-/x_+", x_minus/x_plus),
    ("3/x_+", 3/x_plus),
    ("3*alpha/pi", 3*alpha/pi),
    ("gamma/100", gamma_const/100),
    ("pi/1000", pi/1000),
    ("1/100", mpf(1)/100),
]:
    r = delta_2 / val
    print(f"  delta_2 / ({name:20s}) = {nstr(r, 20)}")
    if fabs(r - round(float(r))) < 0.1 and fabs(round(float(r))) < 50 and round(float(r)) != 0:
        print(f"      ** near integer {round(float(r))} **")

print()

sub_banner("2c. Is 3 - exp(diff) expressible in closed form?")
neg_delta = -delta_2  # = 3 - exp(diff) ≈ 0.009562...
show("3 - exp(diff)", neg_delta, 50)
show("= 3*(1 - exp(diff)/3)", 3*(1 - exp_diff/3), 50)
print()

# Interesting: 3 - exp(diff) ≈ 0.00956. Is this related to alpha?
show("3 - exp(diff)", neg_delta, 30)
show("alpha = 1/137.036...", alpha, 30)
show("(3 - exp(diff)) / alpha", neg_delta / alpha, 30)
print(f"  Ratio (3-exp(diff))/alpha = {float(neg_delta/alpha):.6f}")
print()

# Check pi*alpha, etc.
for name, val in [
    ("alpha", alpha),
    ("pi*alpha/2", pi*alpha/2),
    ("alpha*ln(3)", alpha*log(3)),
    ("3*alpha/2", 3*alpha/2),
    ("(1/2)*alpha^(1/2)", alpha**(mpf(1)/2)/2),
]:
    r = neg_delta / val
    print(f"  (3-exp(diff)) / ({name:20s}) = {nstr(r, 20)}")

print()

sub_banner("2d. The ratio (psi(1/3)-psi(1/4))/ln(3)")
ratio_2 = diff_psi / log(3)
show("(psi(1/3)-psi(1/4))/ln(3)", ratio_2, 50)
show("1", mpf(1), 50)
show("deviation from 1", ratio_2 - 1, 50)
print()

# What fraction p/q is this closest to?
print("  If not exactly 1, what simple fraction is it?")
val2 = ratio_2
cf2 = []
v2 = val2
for _ in range(20):
    a = int(floor(v2))
    cf2.append(a)
    frac = v2 - a
    if fabs(frac) < mpf('1e-100'):
        break
    v2 = 1/frac

h_prev, h_curr = 0, 1
k_prev, k_curr = 1, 0
convergents2 = []
for a in cf2:
    h_prev, h_curr = h_curr, a * h_curr + h_prev
    k_prev, k_curr = k_curr, a * k_curr + k_prev
    convergents2.append((h_curr, k_curr))

print(f"  CF: {cf2}")
for h, k in convergents2[:10]:
    approx = mpf(h) / k
    err = fabs(approx - ratio_2)
    print(f"    {h}/{k} = {nstr(approx, 20)}  (abs_err = {nstr(err, 10)})")


# =============================================================================
# CROSS-CONNECTIONS between the two near-misses
# =============================================================================

banner("CROSS-CONNECTIONS: Are the two near-misses related?")

sub_banner("Do the deviations relate to each other?")
show("delta_1 = ln(M)/ln(G*) - 1/6", delta_1, 40)
show("delta_2 = exp(psi(1/3)-psi(1/4)) - 3", delta_2, 40)
show("delta_2 / delta_1", delta_2 / delta_1, 30)
show("delta_1 / delta_2", delta_1 / delta_2, 30)
print()

# Are they connected through framework constants?
for name, val in [("alpha", alpha), ("gamma", gamma_const), ("pi", pi),
                   ("G*", G_star), ("varpi", varpi), ("x_-", x_minus)]:
    r = delta_2 / (delta_1 * val)
    print(f"  delta_2 / (delta_1 * {name:8s}) = {nstr(r, 20)}")

print()

sub_banner("Composite near-misses: do corrections using one fix the other?")
# If we correct x_+ by replacing exact 1/6 with the true ratio:
# x_+ involves G*, which involves varpi and pi
# M = G*^(ratio*6/6) = G*^(ratio) ... but that's tautological

# Instead: does exp(diff) * (some correction involving delta_1) = 3 exactly?
for name, val in [
    ("1 + delta_1", 1 + delta_1),
    ("1 - delta_1", 1 - delta_1),
    ("1 + 6*delta_1", 1 + 6*delta_1),
    ("exp(delta_1)", exp(delta_1)),
    ("exp(-delta_1)", exp(-delta_1)),
    ("1 + delta_1*ln(G*)", 1 + delta_1*log(G_star)),
]:
    test = exp_diff * val
    err = fabs(test - 3)
    print(f"  exp(diff) * ({name:25s}) = {nstr(test, 20)}  (err from 3: {nstr(err, 10)})")


# =============================================================================
# BROADER SEARCH: other simple expressions near integers/framework values
# =============================================================================

banner("BROADER SEARCH: Other near-integer expressions")

sub_banner("Testing exp(psi(1/a) - psi(1/b)) for all FTD integer pairs")
for a in [3, 4, 7, 13]:
    for b in [3, 4, 7, 13]:
        if a != b:
            diff_ab = digamma(mpf(1)/a) - digamma(mpf(1)/b)
            exp_ab = exp(diff_ab)
            # Check if near a simple number
            for target_name, target in [
                ("a/b", mpf(a)/b), ("b/a", mpf(b)/a),
                ("a", mpf(a)), ("b", mpf(b)),
                (f"{a}+{b}", mpf(a+b)), (f"{a}*{b}", mpf(a*b)),
                ("N_c", mpf(3)), ("N_base", mpf(4)),
                ("b_3", mpf(7)), ("N_eff", mpf(13)),
            ]:
                rel = fabs(exp_ab - target) / target if target != 0 else fabs(exp_ab)
                if rel < mpf('0.05'):
                    print(f"  exp(psi(1/{a})-psi(1/{b})) = {nstr(exp_ab, 12)}"
                          f"  ~ {target_name} = {nstr(target, 8)}"
                          f"  (rel_err = {nstr(rel, 6)})")

sub_banner("Testing M^n vs G*^m for small n,m")
print("  Looking for M^n = G*^m with n/m close to but not exactly 1/6:")
print()
for n in range(1, 13):
    for m in range(1, 13):
        ratio_nm = fabs(M_agm**n - G_star**m) / G_star**m
        if ratio_nm < mpf('0.01'):
            print(f"  M^{n} ~ G*^{m}  (rel_err = {nstr(ratio_nm, 10)}, n/m = {n}/{m} = {nstr(mpf(n)/m, 12)})")


print()
print("=" * 100)
print("  END OF NEAR-MISS INVESTIGATION")
print("=" * 100)
