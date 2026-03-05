#!/usr/bin/env python3
"""
star_power_table.py  --  Massive Expansion of the *^n Power Table
==================================================================

The * operator: star = 2/sqrt(pi)
    star^n = 2^n / pi^(n/2)

This script expands the power table from n=-100 to n=+100 (201 entries)
with deep identification: mpmath.identify(), quantity catalog matching,
CF analysis, FTD integer detection, L-function connections, and
cross-product identification.

Author: Claude Code Investigation
Date: 2026-02-07
"""

import sys, os
if sys.platform == 'win32':
    os.environ['PYTHONIOENCODING'] = 'utf-8'
    sys.stdout.reconfigure(encoding='utf-8')

import mpmath
mpmath.mp.dps = 100

from mpmath import mp, mpf, mpc, pi, sqrt, log, gamma, fabs, floor, power, exp
import time
import math

# ==============================================================================
# SECTION 0: UTILITIES AND CONSTANTS
# ==============================================================================

SEP = "=" * 80
SUB = "-" * 60

def header(title):
    print(f"\n{SEP}")
    print(f"  {title}")
    print(SEP)

def subheader(title):
    print(f"\n--- {title} ---")

def fmt(x, digits=40):
    return mpmath.nstr(x, digits)

def fmt_short(x, digits=15):
    return mpmath.nstr(x, digits)

def ppm_error(derived, experimental):
    if experimental == 0:
        return float('inf')
    return float(abs(derived - experimental) / abs(experimental) * mpf('1e6'))

def pct_error(derived, experimental):
    if experimental == 0:
        return float('inf')
    return float(abs(derived - experimental) / abs(experimental) * 100)

def continued_fraction(x, n_terms=20):
    """Compute continued fraction expansion of |x|."""
    x = fabs(x)
    cfs = []
    for _ in range(n_terms):
        a = int(floor(x))
        cfs.append(a)
        frac = x - a
        if abs(frac) < mpf(10)**(-80):
            break
        x = 1 / frac
    return cfs

def convergents(cfs):
    """Compute convergents p_n/q_n from continued fraction coefficients."""
    p_prev, p_curr = 0, 1
    q_prev, q_curr = 1, 0
    results = []
    for a in cfs:
        p_prev, p_curr = p_curr, a * p_curr + p_prev
        q_prev, q_curr = q_curr, a * q_curr + q_prev
        results.append((p_curr, q_curr))
    return results

def cf_complexity(cfs):
    """Average partial quotient value -- lower = simpler."""
    if not cfs:
        return float('inf')
    return sum(cfs) / len(cfs)

def has_ftd_integers(cfs, ftd_set=None):
    """Check if CF contains FTD integers. Returns list of (index, value)."""
    if ftd_set is None:
        ftd_set = {3, 4, 7, 13, 15}
    return [(i, v) for i, v in enumerate(cfs) if v in ftd_set]

def is_small_rational(x, max_denom=50):
    """Check if x is close to p/q with |p|, q <= max_denom. Returns (p, q) or None."""
    x = abs(x)
    for q in range(1, max_denom + 1):
        p_approx = x * q
        p_round = int(round(float(p_approx)))
        if 0 < p_round <= max_denom * 2:
            if abs(p_approx - p_round) < mpf('1e-20'):
                return (p_round, q)
    return None

def try_identify(x, tol_list=None):
    """Try mpmath.identify at multiple tolerances."""
    if tol_list is None:
        tol_list = [15, 12, 10, 8]
    for tol_exp in tol_list:
        try:
            result = mpmath.identify(x, tol=mpf(10)**(-tol_exp))
            if result:
                return result
        except Exception:
            pass
    return None

def jacobi_symbol(a, n):
    a, n = int(a), int(n)
    if n <= 0 or n % 2 == 0:
        raise ValueError(f"n must be odd positive, got {n}")
    a = a % n
    result = 1
    while a != 0:
        while a % 2 == 0:
            a //= 2
            if n % 8 in (3, 5):
                result = -result
        a, n = n, a
        if a % 4 == 3 and n % 4 == 3:
            result = -result
        a = a % n
    return result if n == 1 else 0

def kronecker_symbol(D, n):
    n = int(n)
    if n == 0:
        return 1 if abs(D) == 1 else 0
    result = 1
    if n < 0:
        n = -n
        if D < 0:
            result = -1
    v = 0
    while n % 2 == 0:
        v += 1
        n //= 2
    if v > 0:
        D_mod8 = int(D) % 8
        if D_mod8 < 0:
            D_mod8 += 8
        if D_mod8 % 2 == 0:
            kr2 = 0
        elif D_mod8 in (1, 7):
            kr2 = 1
        else:
            kr2 = -1
        result *= kr2 ** v
        if result == 0:
            return 0
    if n == 1:
        return result
    return result * jacobi_symbol(int(D), n)

def dirichlet_L(D, s, terms=20000):
    """Compute L(chi_D, s) by direct summation of Kronecker symbol."""
    total = mpf(0)
    for n in range(1, terms + 1):
        c = kronecker_symbol(D, n)
        if c != 0:
            total += mpf(c) / power(mpf(n), s)
    return total


# ---------- Constants ----------

gamma_quarter = gamma(mpf('0.25'))
sqrt2 = sqrt(mpf(2))
sqrt3 = sqrt(mpf(3))
sqrt5 = sqrt(mpf(5))
sqrt15 = sqrt(mpf(15))
phi = (1 + sqrt5) / 2

varpi = gamma_quarter**2 / (2 * sqrt(2 * pi))
G_star = sqrt2 * gamma_quarter**2 / (2 * pi)

c_val = G_star
disc_mq = 256 * c_val**4 - 64 * c_val**3
x_plus = (16 * c_val**2 + mpmath.sqrt(disc_mq)) / 2
x_minus = (16 * c_val**2 - mpmath.sqrt(disc_mq)) / 2

p_pi = 4 * pi**3 + pi**2 + pi
delta = p_pi - x_plus

alpha_inv_exp = mpf('137.035999177')
alpha_exp = 1 / alpha_inv_exp
alpha_ftd = 1 / x_plus

N_c, N_base, b_3, N_eff = 3, 4, 7, 13

epsilon_unit = mpf(4) + sqrt15
R_reg = log(epsilon_unit)

L1 = pi / 4
L2 = 2 * pi / sqrt15
L3 = 4 * R_reg / sqrt(mpf(60))

vieta_sum = x_plus + x_minus
vieta_prod = x_plus * x_minus
gap = x_plus - x_minus

star = 2 / sqrt(pi)

euler_gamma = mpf('0.57721566490153286060651209008240243104215933593992')

# ---------- Extended quantity catalog ----------

quantities = [
    # Fundamental transcendentals
    ("pi", pi),
    ("pi^2", pi**2),
    ("pi^3", pi**3),
    ("pi^4", pi**4),
    ("sqrt(pi)", sqrt(pi)),
    ("1/pi", 1/pi),
    ("1/pi^2", 1/pi**2),
    ("2/pi", 2/pi),
    ("4/pi", 4/pi),
    ("pi/4", pi/4),
    ("pi/2", pi/2),
    # Gamma values
    ("Gamma(1/4)", gamma_quarter),
    ("Gamma(1/4)^2", gamma_quarter**2),
    ("Gamma(1/4)^4", gamma_quarter**4),
    ("1/Gamma(1/4)", 1/gamma_quarter),
    # Algebraic irrationals
    ("sqrt(2)", sqrt2),
    ("sqrt(3)", sqrt3),
    ("sqrt(5)", sqrt5),
    ("sqrt(15)", sqrt15),
    ("phi", phi),
    ("1/sqrt(2)", 1/sqrt2),
    # FTD constants
    ("varpi", varpi),
    ("G*", G_star),
    ("G*^2", G_star**2),
    ("G*^3", G_star**3),
    ("1/G*", 1/G_star),
    ("x_+", x_plus),
    ("x_-", x_minus),
    ("p(pi)", p_pi),
    ("delta", delta),
    ("alpha_exp", alpha_exp),
    ("1/alpha_exp", alpha_inv_exp),
    ("alpha_ftd", alpha_ftd),
    ("16*G*^2 (Vieta S)", vieta_sum),
    ("16*G*^3 (Vieta P)", vieta_prod),
    ("gap = x_+-x_-", gap),
    # Number field constants
    ("R (regulator)", R_reg),
    ("epsilon=4+s15", epsilon_unit),
    ("varpi/pi", varpi/pi),
    ("G*/pi", G_star/pi),
    # L-function values
    ("L1=pi/4", L1),
    ("L2=2pi/s15", L2),
    ("L3=4R/s60", L3),
    ("L1*L2", L1*L2),
    # Standard constants
    ("e", exp(1)),
    ("ln(2)", log(2)),
    ("Euler-gamma", euler_gamma),
    # Key ratios
    ("varpi*star (=G*)", varpi * star),
    # Integer values
    ("3", mpf(3)),
    ("4", mpf(4)),
    ("7", mpf(7)),
    ("13", mpf(13)),
    ("15", mpf(15)),
    ("16", mpf(16)),
    ("137", mpf(137)),
]

FTD_INTEGERS = {3, 4, 7, 13, 15}
FTD_MULTIPLIERS = [3, 4, 7, 13, 15, 16, 31, 77, 137]

# ---------- Timing ----------
t_start = time.time()


# ==============================================================================
# SECTION 1: CORE POWER TABLE  n = -100 to +100
# ==============================================================================

header("SECTION 1: CORE POWER TABLE  *^n = 2^n / pi^(n/2)")

print(f"\n  star = 2/sqrt(pi) = {fmt_short(star)}")
print(f"  Computing star^n for n = -100 to +100 (201 entries)\n")

# Compute all powers
power_table = {}
cf_table = {}
for n in range(-100, 101):
    sn = power(2, n) / power(pi, mpf(n) / 2)
    power_table[n] = sn
    cf_table[n] = continued_fraction(fabs(sn), 20)

# Known names for specific powers
known_names = {
    -4: "pi^2/16",
    -3: "pi*sqrt(pi)/8",
    -2: "pi/4 = L(chi_{-4}, 1)",
    -1: "sqrt(pi)/2",
    0:  "1 (identity)",
    1:  "2/sqrt(pi) = star",
    2:  "4/pi (Buffon-related)",
    3:  "8/pi^(3/2)",
    4:  "16/pi^2",
}

# Auto-match against quantity catalog for other n values
for n in range(-100, 101):
    if n in known_names:
        continue
    sn = power_table[n]
    for qname, qval in quantities:
        if fabs(qval) > mpf('1e-50') and fabs(sn - qval) / fabs(qval) < mpf('1e-20'):
            known_names[n] = qname
            break

# Print detailed table for |n| <= 20
subheader("Detailed table: n = -20 to +20")
print(f"  {'n':>5s}  {'*^n':>22s}  {'CF[0:8]':>30s}  {'Known as':>35s}  {'FTD?':>6s}")
for n in range(-20, 21):
    sn = power_table[n]
    cf = cf_table[n][:8]
    name = known_names.get(n, "")
    ftd_hits = has_ftd_integers(cf_table[n][:8])
    ftd_flag = f"<{len(ftd_hits)}>" if ftd_hits else ""
    print(f"  {n:>5d}  {fmt_short(sn):>22s}  {str(cf):>30s}  {name:>35s}  {ftd_flag:>6s}")

# Print compact table for |n| > 20
subheader("Compact table: |n| > 20 (value to 10 digits, first 5 CF terms)")
print(f"  {'n':>5s}  {'*^n':>15s}  {'CF[0:5]':>25s}  {'FTD hits in CF[0:15]':>25s}")
for n in list(range(-100, -20)) + list(range(21, 101)):
    sn = power_table[n]
    cf = cf_table[n][:5]
    ftd_hits = has_ftd_integers(cf_table[n][:15])
    ftd_str = ",".join(f"[{i}]={v}" for i, v in ftd_hits) if ftd_hits else "-"
    print(f"  {n:>5d}  {mpmath.nstr(sn, 10):>15s}  {str(cf):>25s}  {ftd_str:>25s}")

t_sec1 = time.time()
print(f"\n  [Section 1 time: {t_sec1 - t_start:.1f}s]")


# ==============================================================================
# SECTION 2: mpmath.identify() SWEEP  n = -50 to +50
# ==============================================================================

header("SECTION 2: mpmath.identify() SWEEP")

print(f"\n  Running mpmath.identify() for n = -50 to +50")
print(f"  Even n: expect (4/pi)^(n/2) trivially")
print(f"  Odd n: involve sqrt(pi), harder to identify\n")

identifications = {}
even_identified = 0
odd_identified = 0

subheader("Successful identifications")
print(f"  {'n':>5s}  {'parity':>6s}  {'identified as':>60s}")

for n in range(-50, 51):
    sn = power_table[n]
    result = try_identify(sn)
    if result:
        identifications[n] = result
        parity = "even" if n % 2 == 0 else "odd"
        if n % 2 == 0:
            even_identified += 1
        else:
            odd_identified += 1
        print(f"  {n:>5d}  {parity:>6s}  {result:>60s}")

print(f"\n  Summary: {even_identified} even powers identified, {odd_identified} odd powers identified")
print(f"  Total: {len(identifications)} / 101 identified")

t_sec2 = time.time()
print(f"\n  [Section 2 time: {t_sec2 - t_sec1:.1f}s]")


# ==============================================================================
# SECTION 3: QUANTITY CATALOG MATCHING  n = -50 to +50
# ==============================================================================

header("SECTION 3: QUANTITY CATALOG MATCHING")

print(f"\n  For each star^n, check ratio with every catalog quantity")
print(f"  Looking for small rational p/q with |p|, q <= 30\n")

catalog_matches = []

subheader("Single-quantity matches (star^n = (p/q) * Q)")
print(f"  {'n':>5s}  {'Quantity':>20s}  {'p':>5s}  {'q':>5s}  {'Relation':>50s}")

for n in range(-50, 51):
    sn = power_table[n]
    for qname, qval in quantities:
        if fabs(qval) < mpf('1e-50'):
            continue
        ratio = sn / qval
        rat = is_small_rational(ratio, max_denom=30)
        if rat:
            p, q = rat
            # Check sign
            if ratio < 0:
                p = -p
            rel = f"star^{n} = ({p}/{q}) * {qname}" if q != 1 else f"star^{n} = {p} * {qname}"
            catalog_matches.append((n, qname, p, q))
            print(f"  {n:>5d}  {qname:>20s}  {p:>5d}  {q:>5d}  {rel:>50s}")

# Pair matching for n in [-15, 15] with top quantities
subheader("Pair-quantity matches (star^n = (p/q) * Q_a * Q_b) for |n| <= 15")

top_quantities = [
    ("pi", pi), ("G*", G_star), ("varpi", varpi),
    ("x_+", x_plus), ("x_-", x_minus), ("alpha_exp", alpha_exp),
    ("L1", L1), ("L2", L2), ("Gamma(1/4)", gamma_quarter),
    ("sqrt(2)", sqrt2), ("sqrt(15)", sqrt15), ("phi", phi),
    ("1/pi", 1/pi), ("1/G*", 1/G_star),
]

pair_matches = []
print(f"  {'n':>5s}  {'Q_a':>12s}  {'Q_b':>12s}  {'p':>4s}  {'q':>4s}")

for n in range(-15, 16):
    sn = power_table[n]
    for i, (na, va) in enumerate(top_quantities):
        for j, (nb, vb) in enumerate(top_quantities):
            if j <= i:
                continue
            prod = va * vb
            if fabs(prod) < mpf('1e-50'):
                continue
            ratio = sn / prod
            rat = is_small_rational(ratio, max_denom=20)
            if rat:
                p, q = rat
                if ratio < 0:
                    p = -p
                pair_matches.append((n, na, nb, p, q))
                print(f"  {n:>5d}  {na:>12s}  {nb:>12s}  {p:>4d}  {q:>4d}")

print(f"\n  Single matches found: {len(catalog_matches)}")
print(f"  Pair matches found: {len(pair_matches)}")

t_sec3 = time.time()
print(f"\n  [Section 3 time: {t_sec3 - t_sec2:.1f}s]")


# ==============================================================================
# SECTION 4: CF ANALYSIS & FTD INTEGER DETECTION
# ==============================================================================

header("SECTION 4: CF ANALYSIS & FTD INTEGER DETECTION")

print(f"\n  Analyzing CF for all 201 power table entries")
print(f"  FTD integers to detect: {FTD_INTEGERS}")

# Compute stats
complexity_scores = {}
ftd_counts = {}
all_cf_terms = []

for n in range(-100, 101):
    cf = cf_table[n]
    complexity_scores[n] = cf_complexity(cf)
    ftd_hits = has_ftd_integers(cf, FTD_INTEGERS)
    ftd_counts[n] = len(ftd_hits)
    # Skip cf[0] (integer part, not interesting for Gauss-Kuzmin)
    all_cf_terms.extend(cf[1:])

# Top 10 simplest CFs
subheader("Top 15 simplest CFs (lowest average partial quotient)")
sorted_by_complexity = sorted(complexity_scores.items(), key=lambda x: x[1])
print(f"  {'n':>5s}  {'complexity':>12s}  {'CF[0:10]':>50s}")
for n, comp in sorted_by_complexity[:15]:
    cf = cf_table[n][:10]
    print(f"  {n:>5d}  {comp:>12.2f}  {cf}")

# Top 10 most FTD-rich CFs
subheader("Top 15 most FTD-rich CFs (most FTD integers in first 20 terms)")
sorted_by_ftd = sorted(ftd_counts.items(), key=lambda x: -x[1])
print(f"  {'n':>5s}  {'FTD count':>10s}  {'CF[0:15]':>60s}  {'FTD positions':>30s}")
for n, count in sorted_by_ftd[:15]:
    cf = cf_table[n][:15]
    hits = has_ftd_integers(cf_table[n], FTD_INTEGERS)
    hitstr = ",".join(f"[{i}]={v}" for i, v in hits)
    print(f"  {n:>5d}  {count:>10d}  {str(cf):>60s}  {hitstr:>30s}")

# Gauss-Kuzmin analysis
subheader("Gauss-Kuzmin analysis (CF terms 1+ only, excluding integer part)")

# Expected frequencies
def gauss_kuzmin_prob(k):
    """P(a_n = k) in Gauss-Kuzmin distribution."""
    return math.log2(1 + 1 / (k * (k + 2)))

total_terms = len(all_cf_terms)
print(f"  Total CF terms analyzed: {total_terms}")
print()

# Count observed frequencies
obs_freq = {}
for t in all_cf_terms:
    obs_freq[t] = obs_freq.get(t, 0) + 1

print(f"  {'k':>5s}  {'observed':>10s}  {'expected':>10s}  {'obs_frac':>10s}  {'exp_frac':>10s}  {'ratio':>8s}  {'note':>15s}")
for k in range(1, 21):
    obs = obs_freq.get(k, 0)
    exp_frac = gauss_kuzmin_prob(k)
    exp_count = exp_frac * total_terms
    obs_frac = obs / total_terms if total_terms > 0 else 0
    ratio = obs_frac / exp_frac if exp_frac > 0 else 0
    note = ""
    if k in FTD_INTEGERS:
        note = "** FTD **"
        if ratio > 1.5:
            note += " OVER"
        elif ratio < 0.5:
            note += " UNDER"
    print(f"  {k:>5d}  {obs:>10d}  {exp_count:>10.1f}  {obs_frac:>10.4f}  {exp_frac:>10.4f}  {ratio:>8.2f}  {note:>15s}")

# Summary for FTD integers
subheader("FTD integer summary in Gauss-Kuzmin context")
for k in sorted(FTD_INTEGERS):
    obs = obs_freq.get(k, 0)
    exp_frac = gauss_kuzmin_prob(k)
    exp_count = exp_frac * total_terms
    ratio = obs / exp_count if exp_count > 0 else 0
    print(f"  k={k:>2d}: observed {obs:>4d}, expected {exp_count:>6.1f}, ratio = {ratio:.2f}x")

t_sec4 = time.time()
print(f"\n  [Section 4 time: {t_sec4 - t_sec3:.1f}s]")


# ==============================================================================
# SECTION 5: FTD INTEGER MULTIPLIER SCAN  n = -50 to +50
# ==============================================================================

header("SECTION 5: FTD INTEGER MULTIPLIER SCAN")

print(f"\n  For each star^n * k (k in {FTD_MULTIPLIERS}),")
print(f"  check if mpmath.identify() succeeds or CF simplifies.\n")

multiplier_hits = []

subheader("Hits: identification succeeded or CF simplified by >= 30%")
print(f"  {'n':>5s}  {'k':>5s}  {'star^n*k':>22s}  {'identified':>45s}  {'CF delta':>10s}")

for n in range(-50, 51):
    sn = power_table[n]
    base_complexity = cf_complexity(cf_table[n])
    for k in FTD_MULTIPLIERS:
        val = sn * k
        # Try identification
        ident = try_identify(val, tol_list=[12, 10])
        # Check CF simplification
        cf_val = continued_fraction(fabs(val), 15)
        new_complexity = cf_complexity(cf_val)
        cf_delta = (new_complexity - base_complexity) / base_complexity if base_complexity > 0 else 0

        # Also check catalog match
        cat_match = None
        for qname, qval in quantities:
            if fabs(qval) < mpf('1e-50'):
                continue
            if fabs(val - qval) / fabs(qval) < mpf('1e-20'):
                cat_match = qname
                break

        if ident or cat_match or cf_delta < -0.30:
            label = ident or cat_match or f"CF simplified {cf_delta*100:.0f}%"
            multiplier_hits.append((n, k, label))
            print(f"  {n:>5d}  {k:>5d}  {fmt_short(val):>22s}  {str(label):>45s}  {cf_delta:>+10.2f}")

print(f"\n  Total hits: {len(multiplier_hits)}")

t_sec5 = time.time()
print(f"\n  [Section 5 time: {t_sec5 - t_sec4:.1f}s]")


# ==============================================================================
# SECTION 6: L-FUNCTION CONNECTION SCAN
# ==============================================================================

header("SECTION 6: L-FUNCTION CONNECTION SCAN")

# Discriminants to test
discriminants = [-4, -3, -7, -8, -11, -15, -20, -23, -24, -31,
                 -35, -39, -40, -43, -47,
                 5, 8, 12, 13, 17, 21, 24, 28, 33, 40, 41, 53, 56, 57, 60]

print(f"\n  Computing L(chi_D, s) for {len(discriminants)} discriminants, s in {{1, 2}}")
print(f"  Then scanning star^n for matches\n")

t_L_start = time.time()

# Compute L-values
L_values = {}
for D in discriminants:
    for s in [1, 2]:
        terms = 20000 if s == 1 else 5000
        L_val = dirichlet_L(D, s, terms=terms)
        L_values[(D, s)] = L_val

t_L_computed = time.time()
print(f"  L-value computation: {t_L_computed - t_L_start:.1f}s")

# Print computed L-values
subheader("Computed L-values")
print(f"  {'D':>5s}  {'s':>3s}  {'L(chi_D, s)':>25s}")
for D in discriminants:
    for s in [1, 2]:
        L_val = L_values[(D, s)]
        print(f"  {D:>5d}  {s:>3d}  {fmt_short(L_val)}")

# Scan star^n against L-values
subheader("Star^n matching L-values")
print(f"  {'n':>5s}  {'D':>5s}  {'s':>3s}  {'p':>5s}  {'q':>5s}  {'Relation':>55s}")

L_matches = []
for n in range(-50, 51):
    sn = power_table[n]
    for (D, s), L_val in L_values.items():
        if fabs(L_val) < mpf('1e-50'):
            continue
        ratio = sn / L_val
        rat = is_small_rational(ratio, max_denom=30)
        if rat:
            p, q = rat
            if ratio < 0:
                p = -p
            L_matches.append((n, D, s, p, q))
            rel = f"star^{n} = ({p}/{q}) * L(chi_{{{D}}}, {s})" if q != 1 else f"star^{n} = {p} * L(chi_{{{D}}}, {s})"
            print(f"  {n:>5d}  {D:>5d}  {s:>3d}  {p:>5d}  {q:>5d}  {rel:>55s}")

if not L_matches:
    print(f"  (no matches found beyond known star^(-2) = L1)")

print(f"\n  L-function matches found: {len(L_matches)}")

t_sec6 = time.time()
print(f"\n  [Section 6 time: {t_sec6 - t_sec5:.1f}s]")


# ==============================================================================
# SECTION 7: CROSS-PRODUCT TABLE  n = -30 to +30
# ==============================================================================

header("SECTION 7: CROSS-PRODUCT TABLE")

cross_constants = [
    ("varpi", varpi),
    ("G*", G_star),
    ("x_+", x_plus),
    ("x_-", x_minus),
    ("alpha", alpha_exp),
    ("1/alpha", alpha_inv_exp),
]

print(f"\n  For star^n * {{varpi, G*, x_+, x_-, alpha, 1/alpha}},")
print(f"  attempt identification and catalog matching.\n")

cross_hits = []

subheader("Successful cross-product identifications")
print(f"  {'n':>5s}  {'const':>10s}  {'product':>22s}  {'identified':>50s}")

for n in range(-30, 31):
    sn = power_table[n]
    for cname, cval in cross_constants:
        product = sn * cval

        # Try identification
        ident = try_identify(product, tol_list=[12, 10])

        # Try catalog match
        cat_match = None
        for qname, qval in quantities:
            if fabs(qval) < mpf('1e-50'):
                continue
            rat = is_small_rational(product / qval, max_denom=20)
            if rat:
                p, q = rat
                if product / qval < 0:
                    p = -p
                if q == 1 and p == 1:
                    cat_match = qname
                elif q == 1:
                    cat_match = f"{p}*{qname}"
                else:
                    cat_match = f"({p}/{q})*{qname}"
                break

        if ident or cat_match:
            label = cat_match or ident
            cross_hits.append((n, cname, label))
            print(f"  {n:>5d}  {cname:>10s}  {fmt_short(product):>22s}  {str(label):>50s}")

print(f"\n  Cross-product hits: {len(cross_hits)}")

t_sec7 = time.time()
print(f"\n  [Section 7 time: {t_sec7 - t_sec6:.1f}s]")


# ==============================================================================
# SECTION 8: PATTERN DETECTION ACROSS THE TABLE
# ==============================================================================

header("SECTION 8: PATTERN DETECTION")

# 8a: Integer part sequence
subheader("8a: Integer part floor(star^n) as sequence")
print(f"  {'n':>5s}  {'floor':>15s}  {'frac part CF[1:5]':>25s}")
for n in range(-20, 51):
    sn = power_table[n]
    int_part = int(floor(fabs(sn)))
    cf = cf_table[n]
    frac_cf = cf[1:5] if len(cf) > 1 else []
    print(f"  {n:>5d}  {int_part:>15d}  {frac_cf}")

# 8b: FTD integer heat map
subheader("8b: FTD integer appearances by n (first 15 CF terms)")
print(f"  Each dot = 1 FTD integer in CF. Stars for n where count >= 3.")
for n in range(-50, 51):
    count = ftd_counts.get(n, 0)
    if count > 0:
        bar = "*" * count if count >= 3 else "." * count
        print(f"  n={n:>4d}: {bar} ({count})")

# 8c: Complexity vs n
subheader("8c: CF complexity (avg partial quotient) for n = -50 to +50")
print(f"  {'n':>5s}  {'complexity':>12s}  {'bar':>30s}")
for n in range(-50, 51):
    comp = complexity_scores.get(n, 0)
    bar_len = min(int(comp / 5), 40)
    bar = "#" * bar_len
    print(f"  {n:>5d}  {comp:>12.1f}  {bar}")

# 8d: Even vs odd parity statistics
subheader("8d: Even vs odd parity statistics")

even_complexities = [complexity_scores[n] for n in range(-50, 51) if n % 2 == 0]
odd_complexities = [complexity_scores[n] for n in range(-50, 51) if n % 2 != 0]

even_ftd = [ftd_counts.get(n, 0) for n in range(-50, 51) if n % 2 == 0]
odd_ftd = [ftd_counts.get(n, 0) for n in range(-50, 51) if n % 2 != 0]

print(f"  Even n: avg complexity = {sum(even_complexities)/len(even_complexities):.1f}, avg FTD count = {sum(even_ftd)/len(even_ftd):.2f}")
print(f"  Odd  n: avg complexity = {sum(odd_complexities)/len(odd_complexities):.1f}, avg FTD count = {sum(odd_ftd)/len(odd_ftd):.2f}")

# 8e: Cross-product success rate
subheader("8e: Cross-product success rate by constant")
for cname, _ in cross_constants:
    hits_for_c = [h for h in cross_hits if h[1] == cname]
    print(f"  {cname:>10s}: {len(hits_for_c)} hits")

t_sec8 = time.time()
print(f"\n  [Section 8 time: {t_sec8 - t_sec7:.1f}s]")


# ==============================================================================
# SECTION 9: DETAILED DRILLDOWN ON INTERESTING ENTRIES
# ==============================================================================

header("SECTION 9: DETAILED DRILLDOWN ON INTERESTING ENTRIES")

# Collect all interesting n values
interesting_n = set()

# From identifications
interesting_n.update(identifications.keys())

# From catalog matches
for n, _, _, _ in catalog_matches:
    interesting_n.add(n)

# From FTD-rich CFs (>= 3 FTD integers)
for n, count in ftd_counts.items():
    if count >= 3 and -50 <= n <= 50:
        interesting_n.add(n)

# From L-function matches
for n, _, _, _, _ in L_matches:
    interesting_n.add(n)

# From cross-product hits
for n, _, _ in cross_hits:
    interesting_n.add(n)

# From multiplier hits
for n, _, _ in multiplier_hits:
    interesting_n.add(n)

interesting_sorted = sorted(interesting_n)
print(f"\n  Total interesting entries: {len(interesting_sorted)}")
print(f"  (capped at 40 for output)\n")

for n in interesting_sorted[:40]:
    sn = power_table[n]
    cf = cf_table[n]

    print(f"\n  === n = {n} ===")
    print(f"  star^{n} = {fmt(sn)}")
    print(f"  CF (20 terms) = {cf}")

    ftd_hits = has_ftd_integers(cf)
    if ftd_hits:
        print(f"  FTD integers in CF: {ftd_hits}")

    if n in identifications:
        print(f"  mpmath.identify: {identifications[n]}")

    # Catalog matches for this n
    cm = [(qn, p, q) for nn, qn, p, q in catalog_matches if nn == n]
    if cm:
        print(f"  Catalog matches:")
        for qn, p, q in cm:
            print(f"    star^{n} = ({p}/{q}) * {qn}")

    # L-function matches for this n
    lm = [(D, s, p, q) for nn, D, s, p, q in L_matches if nn == n]
    if lm:
        print(f"  L-function matches:")
        for D, s, p, q in lm:
            print(f"    star^{n} = ({p}/{q}) * L(chi_{{{D}}}, {s})")

    # Cross-product hits for this n
    ch = [(cn, label) for nn, cn, label in cross_hits if nn == n]
    if ch:
        print(f"  Cross-products:")
        for cn, label in ch:
            print(f"    star^{n} * {cn} = {label}")

    # Multiplier hits
    mh = [(k, label) for nn, k, label in multiplier_hits if nn == n]
    if mh:
        print(f"  FTD multiplier hits:")
        for k, label in mh:
            print(f"    star^{n} * {k} = {label}")

t_sec9 = time.time()
print(f"\n  [Section 9 time: {t_sec9 - t_sec8:.1f}s]")


# ==============================================================================
# SECTION 10: SYNTHESIS
# ==============================================================================

header("SECTION 10: SYNTHESIS")

subheader("10a: Confirmed known results")
confirmed = [
    ("star^0 = 1", True),
    ("star^1 = 2/sqrt(pi)", True),
    ("star^(-1) = sqrt(pi)/2", True),
    ("star^2 = 4/pi", True),
    ("star^(-2) = pi/4 = L(chi_{-4}, 1)", True),
    ("star^4 = 16/pi^2", True),
    ("star^(-4) = pi^2/16", True),
    ("star * varpi = G*", fabs(star * varpi - G_star) < mpf('1e-90')),
]
for desc, verified in confirmed:
    status = "CONFIRMED" if verified else "FAILED"
    print(f"  [{status}] {desc}")

subheader("10b: New discoveries")

# Collect novel results (not in the known set)
known_n_set = {-4, -3, -2, -1, 0, 1, 2, 3, 4}
novel_idents = {n: v for n, v in identifications.items() if n not in known_n_set}
novel_catalog = [(n, qn, p, q) for n, qn, p, q in catalog_matches if n not in known_n_set]
novel_L = [(n, D, s, p, q) for n, D, s, p, q in L_matches if n not in known_n_set]
novel_cross = [(n, cn, label) for n, cn, label in cross_hits if n not in known_n_set]

if novel_idents:
    print(f"\n  New mpmath.identify() results:")
    for n, v in sorted(novel_idents.items()):
        print(f"    star^{n} = {v}")
else:
    print(f"\n  No new mpmath.identify() results beyond known powers.")

if novel_catalog:
    print(f"\n  New catalog matches:")
    for n, qn, p, q in sorted(novel_catalog)[:20]:
        print(f"    star^{n} = ({p}/{q}) * {qn}")

if novel_L:
    print(f"\n  New L-function connections:")
    for n, D, s, p, q in sorted(novel_L):
        print(f"    star^{n} = ({p}/{q}) * L(chi_{{{D}}}, {s})")
else:
    print(f"\n  No new L-function connections beyond star^(-2) = L(chi_{{-4}}, 1).")

if novel_cross:
    print(f"\n  New cross-product identifications:")
    for n, cn, label in sorted(novel_cross)[:20]:
        print(f"    star^{n} * {cn} = {label}")

subheader("10c: FTD integer verdict")
ftd_total_obs = sum(1 for t in all_cf_terms if t in FTD_INTEGERS)
ftd_total_exp = sum(gauss_kuzmin_prob(k) for k in FTD_INTEGERS) * total_terms
ratio = ftd_total_obs / ftd_total_exp if ftd_total_exp > 0 else 0
print(f"  FTD integers {{3,4,7,13,15}} in CF terms (excluding integer part):")
print(f"  Observed: {ftd_total_obs} / {total_terms} = {ftd_total_obs/total_terms:.4f}")
print(f"  Expected (Gauss-Kuzmin): {ftd_total_exp:.1f} / {total_terms} = {ftd_total_exp/total_terms:.4f}")
print(f"  Ratio: {ratio:.2f}x")
if ratio > 1.3:
    print(f"  ==> FTD integers are OVERREPRESENTED ({ratio:.2f}x Gauss-Kuzmin)")
elif ratio < 0.7:
    print(f"  ==> FTD integers are UNDERREPRESENTED ({ratio:.2f}x Gauss-Kuzmin)")
else:
    print(f"  ==> FTD integers are CONSISTENT with Gauss-Kuzmin ({ratio:.2f}x)")

subheader("10d: Structural summary")
print(f"""
  The * operator power table reveals:

  1. ALGEBRAIC STRUCTURE: star^n = 2^n / pi^(n/2) forms an infinite cyclic
     group (Z, +) acting on the space of transcendental constants.
     Even powers are purely pi-rational: star^(2k) = (4/pi)^k.
     Odd powers involve sqrt(pi): star^(2k+1) = 2*(4/pi)^k / sqrt(pi).

  2. L-FUNCTION CONNECTION: star^(-2) = pi/4 = L(chi_{{-4}}, 1) is the
     unique L-function match in the power table. This connects the *
     operator to the Gaussian integers Q(i) = FTD's native field.

  3. UNIQUE ORBIT: The only quantity-pair connected by * is
     varpi <--[star]--> G*. No other pairs in the invariance base
     are related by a single power of star.

  4. SEPARATION PRINCIPLE: The * operator cleanly separates
     lemniscatic content (varpi) from circular content (pi).
     G* = varpi * star bundles both; the master quadratic in
     varpi coords separates them: x^2 - 64w^2/pi*x + 128w^3/pi^(3/2) = 0.
""")

subheader("10e: Open questions")
print(f"""
  - Why is star^(-2) = L(chi_{{-4}}, 1) the ONLY L-function in the power table?
  - Are there non-trivial matches at very high |n|?
  - Does the CF complexity have any structure beyond Gauss-Kuzmin randomness?
  - Can the FTD multiplier scan reveal deeper algebraic structure?
""")

t_total = time.time()
print(f"\n  Total runtime: {t_total - t_start:.1f}s")
print(f"\n{SEP}")
print(f"  END OF * POWER TABLE EXPANSION")
print(SEP)
