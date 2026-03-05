"""
SCRIPT 4: UNIFICATION TEST - The Decisive Experiment
=====================================================

PURPOSE: Systematically test whether any combination of modular objects
at CM points yields 1/alpha = 137.035999...

This is THE decisive test. We evaluate:
1. Eta-quotients at level 60 = lcm(4, 15) at CM points
2. Products/sums of CM values from disc -4 and disc -15
3. Period ratios of the Cremona 15a1 curve
4. The correction delta = p(pi) - x_+ expressed via L-values

KEY YES/NO QUESTIONS:
- Does any eta-quotient of level 60 at a CM point give 1/alpha?
- Does any product f(i) * g(tau_1) = 1/alpha for standard modular objects?
- Can delta/alpha^2 be expressed as a ratio of L-values?

Author: Claude Code Investigation
Date: 2026-02-07
"""

import sys, os
if sys.platform == 'win32':
    os.environ['PYTHONIOENCODING'] = 'utf-8'
    sys.stdout.reconfigure(encoding='utf-8')

import mpmath
mpmath.mp.dps = 50  # 50 decimal places

import itertools
import time
import math

# ============================================================================
# CONSTANTS
# ============================================================================

pi = mpmath.pi
sqrt2 = mpmath.sqrt(2)
sqrt3 = mpmath.sqrt(3)
sqrt5 = mpmath.sqrt(5)
sqrt15 = mpmath.sqrt(15)
gamma_quarter = mpmath.gamma(mpmath.mpf('0.25'))
varpi = gamma_quarter**2 / (2 * mpmath.sqrt(2 * pi))
G_star = sqrt2 * gamma_quarter**2 / (2 * pi)
phi = (1 + sqrt5) / 2

# CODATA 2022
alpha_inv_exp = mpmath.mpf('137.035999177')
alpha_exp = 1 / alpha_inv_exp

# FTD framework integers
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13

# FTD master quadratic roots: x^2 - 16*G*^2*x + 16*G*^3 = 0
c = G_star
disc_mq = 256*c**4 - 64*c**3
x_plus = (16*c**2 + mpmath.sqrt(disc_mq)) / 2
x_minus = (16*c**2 - mpmath.sqrt(disc_mq)) / 2

# RFT polynomial
def p_rft(x):
    return 4*x**3 + x**2 + x

p_pi = p_rft(pi)

# The correction delta
delta = p_pi - x_plus

SEP = "=" * 80
SUB = "-" * 60

def fmt(x, digits=25):
    if isinstance(x, mpmath.mpc):
        return mpmath.nstr(x, digits)
    return mpmath.nstr(x, digits)

def ppm_error(derived, experimental):
    return abs(derived - experimental) / abs(experimental) * mpmath.mpf('1e6')

def ppt_error(derived, experimental):
    return abs(derived - experimental) / abs(experimental) * mpmath.mpf('1e12')

def flag_if_close(value, target=alpha_inv_exp, threshold_ppm=100):
    """Flag values within threshold_ppm of 1/alpha."""
    if abs(value) < 1e-30:
        return ""
    ppm = float(ppm_error(value, target))
    if ppm < threshold_ppm:
        return f"  *** MATCH: {ppm:.2f} ppm ***"
    return ""

# ============================================================================
# SECTION 1: Compute CM values (from scratch — don't rely on Script 1 output)
# ============================================================================

print(f"\n{SEP}")
print("  SECTION 1: COMPUTING CM VALUES AT ALL RELEVANT POINTS")
print(f"{SEP}\n")

# CM points
tau_i = mpmath.mpc(0, 1)                              # disc -4, form (1,0,1)
tau1 = mpmath.mpc(-mpmath.mpf('0.5'), sqrt15/2)       # disc -15, form (1,1,4)
tau2 = mpmath.mpc(-mpmath.mpf('0.25'), sqrt15/4)      # disc -15, form (2,1,2)

print(f"  tau_i   = i                         [disc -4,  form (1,0,1)]")
print(f"  tau_1   = (-1+sqrt(-15))/2          [disc -15, form (1,1,4)]")
print(f"  tau_2   = (-1+sqrt(-15))/4          [disc -15, form (2,1,2)]")
print()

# ----- Dedekind eta function -----
def eta(tau, terms=300):
    """Compute Dedekind eta function using product formula."""
    q = mpmath.exp(2 * pi * mpmath.mpc(0, 1) * tau)
    result = mpmath.power(q, mpmath.mpf(1)/24)
    for n in range(1, terms+1):
        qn = mpmath.power(q, n)
        result *= (1 - qn)
    return result

# ----- Eisenstein series -----
def sigma_k(n, k):
    s = mpmath.mpf(0)
    for d in range(1, n+1):
        if n % d == 0:
            s += mpmath.power(d, k)
    return s

def E4(tau, terms=200):
    q = mpmath.exp(2 * pi * mpmath.mpc(0, 1) * tau)
    result = mpmath.mpf(1)
    qn = mpmath.mpf(1)
    for n in range(1, terms+1):
        qn *= q
        result += 240 * sigma_k(n, 3) * qn
    return result

def E6(tau, terms=200):
    q = mpmath.exp(2 * pi * mpmath.mpc(0, 1) * tau)
    result = mpmath.mpf(1)
    qn = mpmath.mpf(1)
    for n in range(1, terms+1):
        qn *= q
        result -= 504 * sigma_k(n, 5) * qn
    return result

def j_invariant(tau, terms=200):
    e4 = E4(tau, terms)
    e6 = E6(tau, terms)
    return 1728 * e4**3 / (e4**3 - e6**2)

# ----- Jacobi theta functions -----
def theta2(tau, terms=200):
    q = mpmath.exp(mpmath.mpc(0, 1) * pi * tau)
    result = mpmath.mpf(0)
    for n in range(0, terms):
        result += mpmath.power(q, (n + mpmath.mpf('0.5'))**2)
    return 2 * result

def theta3(tau, terms=200):
    q = mpmath.exp(mpmath.mpc(0, 1) * pi * tau)
    result = mpmath.mpf(1)
    for n in range(1, terms):
        result += 2 * mpmath.power(q, n**2)
    return result

def theta4(tau, terms=200):
    q = mpmath.exp(mpmath.mpc(0, 1) * pi * tau)
    result = mpmath.mpf(1)
    for n in range(1, terms):
        result += 2 * (-1)**n * mpmath.power(q, n**2)
    return result

# Compute all values at all three CM points
print("  Computing modular values... (this may take a few minutes)")
print()

start_time = time.time()

cm_points = {
    'i': tau_i,
    'tau1': tau1,
    'tau2': tau2,
}

cm_values = {}
for name, tau in cm_points.items():
    print(f"  Processing {name}...")
    t0 = time.time()

    eta_val = eta(tau)
    e4_val = E4(tau, 100)  # fewer terms for speed
    e6_val = E6(tau, 100)
    j_val = 1728 * e4_val**3 / (e4_val**3 - e6_val**2)
    th2 = theta2(tau, 100)
    th3 = theta3(tau, 100)
    th4 = theta4(tau, 100)

    cm_values[name] = {
        'eta': eta_val,
        'E4': e4_val,
        'E6': e6_val,
        'j': j_val,
        'theta2': th2,
        'theta3': th3,
        'theta4': th4,
        'eta24': eta_val**24,
        'j_over_1728': j_val / 1728,
        'lambda': (th2/th3)**4 if abs(th3) > 1e-30 else mpmath.mpf(0),
    }

    elapsed = time.time() - t0
    print(f"    eta({name}) = {fmt(eta_val, 15)}")
    print(f"    j({name})   = {fmt(j_val, 15)}")
    print(f"    ({elapsed:.1f}s)")
    print()

total_time = time.time() - start_time
print(f"  Total CM computation time: {total_time:.1f}s")

# ============================================================================
# SECTION 2: SYSTEMATIC SEARCH — Products and Sums of CM Values
# ============================================================================

print(f"\n{SEP}")
print("  SECTION 2: PRODUCTS AND SUMS OF CM VALUES")
print(f"{SUB}")
print("  Testing f(i) * g(tau_k) and f(i) + g(tau_k) against 1/alpha")
print(f"{SEP}\n")

# Collect real-valued quantities from CM data
def collect_real_quantities(cm_vals, name):
    """Extract real (or real-part) quantities from CM values."""
    items = []
    for key, val in cm_vals.items():
        if isinstance(val, mpmath.mpc):
            if abs(mpmath.im(val)) < 1e-30:
                items.append((f"{key}({name})", mpmath.re(val)))
            else:
                items.append((f"Re({key}({name}))", mpmath.re(val)))
                items.append((f"|{key}({name})|", abs(val)))
        else:
            items.append((f"{key}({name})", val))
    return items

# Also add key mathematical constants
math_constants = [
    ("pi", pi),
    ("varpi", varpi),
    ("G_star", G_star),
    ("gamma(1/4)", gamma_quarter),
    ("phi", phi),
    ("sqrt(2)", sqrt2),
    ("sqrt(3)", sqrt3),
    ("sqrt(5)", sqrt5),
    ("sqrt(15)", sqrt15),
    ("4", mpmath.mpf(4)),
    ("3", mpmath.mpf(3)),
    ("7", mpmath.mpf(7)),
    ("13", mpmath.mpf(13)),
    ("16", mpmath.mpf(16)),
    ("47", mpmath.mpf(47)),
    ("64", mpmath.mpf(64)),
    ("1728", mpmath.mpf(1728)),
]

# Get FTD quantities
ftd_quantities = collect_real_quantities(cm_values['i'], 'i')
rft_quantities_1 = collect_real_quantities(cm_values['tau1'], 'tau1')
rft_quantities_2 = collect_real_quantities(cm_values['tau2'], 'tau2')

print(f"  FTD quantities (at i): {len(ftd_quantities)} values")
print(f"  RFT quantities (at tau1): {len(rft_quantities_1)} values")
print(f"  RFT quantities (at tau2): {len(rft_quantities_2)} values")
print(f"  Math constants: {len(math_constants)} values")
print()

# Combine all non-FTD quantities
rft_all = rft_quantities_1 + rft_quantities_2 + math_constants
ftd_all = ftd_quantities + math_constants

matches_found = []

# Test products: f_FTD * g_RFT
print(f"  Testing f_FTD * g_RFT combinations ({len(ftd_all)} x {len(rft_all)})...")
for name_f, f_val in ftd_all:
    for name_g, g_val in rft_all:
        if abs(f_val) < 1e-30 or abs(g_val) < 1e-30:
            continue
        product = f_val * g_val
        flag = flag_if_close(product, alpha_inv_exp, 100)
        if flag:
            matches_found.append(('product', f"{name_f} * {name_g}", product, flag))

# Test ratios: f_FTD / g_RFT
print(f"  Testing f_FTD / g_RFT combinations...")
for name_f, f_val in ftd_all:
    for name_g, g_val in rft_all:
        if abs(f_val) < 1e-30 or abs(g_val) < 1e-30:
            continue
        ratio = f_val / g_val
        flag = flag_if_close(ratio, alpha_inv_exp, 100)
        if flag:
            matches_found.append(('ratio', f"{name_f} / {name_g}", ratio, flag))
        # Also check inverse
        ratio_inv = g_val / f_val
        flag_inv = flag_if_close(ratio_inv, alpha_inv_exp, 100)
        if flag_inv:
            matches_found.append(('ratio', f"{name_g} / {name_f}", ratio_inv, flag_inv))

# Test sums: f_FTD + g_RFT
print(f"  Testing f_FTD + g_RFT combinations...")
for name_f, f_val in ftd_all:
    for name_g, g_val in rft_all:
        if abs(f_val) < 1e-30 or abs(g_val) < 1e-30:
            continue
        sumval = f_val + g_val
        flag = flag_if_close(sumval, alpha_inv_exp, 100)
        if flag:
            matches_found.append(('sum', f"{name_f} + {name_g}", sumval, flag))

# Test power combinations: f^a * g^b for small a,b
print(f"  Testing f^a * g^b for small integer powers...")
small_ints = [-3, -2, -1, 1, 2, 3]
# Only use a subset of quantities for power search (avoid combinatorial explosion)
ftd_key = [(n, v) for n, v in ftd_quantities if abs(v) > 1e-30 and abs(v) < 1e30][:8]
rft_key = [(n, v) for n, v in rft_quantities_1 if abs(v) > 1e-30 and abs(v) < 1e30][:8]

for name_f, f_val in ftd_key:
    for name_g, g_val in rft_key:
        for a in small_ints:
            for b in small_ints:
                try:
                    combo = mpmath.power(abs(f_val), a) * mpmath.power(abs(g_val), b)
                    flag = flag_if_close(combo, alpha_inv_exp, 50)
                    if flag:
                        matches_found.append(('power', f"|{name_f}|^{a} * |{name_g}|^{b}", combo, flag))
                except:
                    pass

print()
print(f"  RESULTS: {len(matches_found)} matches found within 100 ppm of 1/alpha")
print()

if matches_found:
    # Sort by accuracy
    matches_found.sort(key=lambda x: abs(float(ppm_error(x[2], alpha_inv_exp))))
    for mtype, formula, value, flag in matches_found[:30]:  # Top 30
        ppm = float(ppm_error(value, alpha_inv_exp))
        print(f"  [{mtype:8s}] {formula}")
        print(f"            = {fmt(value, 20)}  ({ppm:.2f} ppm)")
        print()
else:
    print("  >>> NO MATCHES FOUND within 100 ppm <<<")
    print()

# ============================================================================
# SECTION 3: ETA-QUOTIENTS AT LEVEL 60
# ============================================================================

print(f"\n{SEP}")
print("  SECTION 3: ETA-QUOTIENTS AT LEVEL 60 = lcm(4, 15)")
print(f"{SUB}")
print("  Evaluate eta-quotients prod eta(d*tau)^r_d at CM points")
print(f"{SEP}\n")

# Level 60 divisors: 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60
divisors_60 = [1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60]
print(f"  Divisors of 60: {divisors_60}")
print(f"  Number of divisors: {len(divisors_60)}")
print()

# For weight-0 eta-quotients: sum(r_d) = 0
# For holomorphicity: sum(d * r_d) >= 0 at each cusp
# We'll search with |r_d| <= 2 to keep search manageable

# First, precompute eta(d*tau) for each divisor and CM point
print("  Precomputing eta(d*tau) values...")
eta_cache = {}
for name, tau in cm_points.items():
    eta_cache[name] = {}
    for d in divisors_60:
        try:
            val = eta(d * tau, 200)
            eta_cache[name][d] = val
            # Only print a few for debugging
            if d <= 5:
                print(f"    eta({d}*{name}) = {fmt(val, 12)}")
        except Exception as e:
            print(f"    eta({d}*{name}) = ERROR: {e}")
            eta_cache[name][d] = None
    print()

# Enumerate weight-0 eta-quotients with bounded exponents
# sum(r_d) = 0, |r_d| <= 2
# This is a constraint satisfaction problem over 12 variables
# We'll use a greedy approach: try quotients with few nonzero exponents

print("  Searching eta-quotients with at most 4 nonzero exponents...")
print("  (weight-0 constraint: sum of exponents = 0)")
print()

eta_matches = []

# Strategy: pick 2, 3, or 4 divisors and vary exponents
# 2-divisor quotients: eta(d1)^r / eta(d2)^r
for i, d1 in enumerate(divisors_60):
    for d2 in divisors_60[i+1:]:
        for r in [1, 2, 3, 4, 5, 6]:
            # eta(d1*tau)^r * eta(d2*tau)^(-r)  [weight 0: r + (-r) = 0]
            for name in ['i', 'tau1', 'tau2']:
                e1 = eta_cache[name].get(d1)
                e2 = eta_cache[name].get(d2)
                if e1 is None or e2 is None or abs(e2) < 1e-100:
                    continue
                try:
                    val = mpmath.power(e1, r) / mpmath.power(e2, r)
                    if isinstance(val, mpmath.mpc):
                        val_r = abs(val)
                    else:
                        val_r = abs(val)
                    flag = flag_if_close(val_r, alpha_inv_exp, 100)
                    if flag:
                        desc = f"(eta({d1}*t)/eta({d2}*t))^{r} at {name}"
                        eta_matches.append((desc, val_r, flag))
                    # Also check the reciprocal
                    val_inv = 1/val_r if val_r > 1e-100 else mpmath.mpf(0)
                    flag2 = flag_if_close(val_inv, alpha_inv_exp, 100)
                    if flag2:
                        desc = f"(eta({d2}*t)/eta({d1}*t))^{r} at {name}"
                        eta_matches.append((desc, val_inv, flag2))
                except:
                    pass

# 3-divisor quotients: eta(d1)^a * eta(d2)^b * eta(d3)^c with a+b+c=0
# Try a=1, b=1, c=-2 and permutations
triplet_exponents = [
    (1, 1, -2), (1, -2, 1), (-2, 1, 1),
    (2, 1, -3), (2, -3, 1), (-3, 2, 1),
    (1, 2, -3), (-3, 1, 2), (1, -3, 2),
    (2, 2, -4), (2, -4, 2), (-4, 2, 2),
    (1, -1, 0),  # not weight 0 unless we skip zeros
]
# Filter to weight-0
triplet_exponents = [(a,b,c) for a,b,c in triplet_exponents if a+b+c == 0]

print(f"  Searching 3-divisor quotients ({len(triplet_exponents)} exponent patterns)...")

for i, d1 in enumerate(divisors_60):
    for j, d2 in enumerate(divisors_60):
        if j <= i:
            continue
        for d3 in divisors_60[j+1:]:
            for (a, b, c) in triplet_exponents:
                for name in ['i', 'tau1', 'tau2']:
                    e1 = eta_cache[name].get(d1)
                    e2 = eta_cache[name].get(d2)
                    e3 = eta_cache[name].get(d3)
                    if any(x is None for x in [e1, e2, e3]):
                        continue
                    if any(abs(x) < 1e-100 for x in [e1, e2, e3]):
                        continue
                    try:
                        val = mpmath.power(e1, a) * mpmath.power(e2, b) * mpmath.power(e3, c)
                        val_r = abs(val)
                        flag = flag_if_close(val_r, alpha_inv_exp, 100)
                        if flag:
                            desc = f"eta({d1})^{a}*eta({d2})^{b}*eta({d3})^{c} at {name}"
                            eta_matches.append((desc, val_r, flag))
                    except:
                        pass

print()
print(f"  ETA-QUOTIENT RESULTS: {len(eta_matches)} matches within 100 ppm")
print()

if eta_matches:
    eta_matches.sort(key=lambda x: abs(float(ppm_error(x[1], alpha_inv_exp))))
    for desc, val, flag in eta_matches[:20]:
        ppm = float(ppm_error(val, alpha_inv_exp))
        print(f"  {desc}")
        print(f"    = {fmt(val, 20)}  ({ppm:.2f} ppm)")
        print()
else:
    print("  >>> NO ETA-QUOTIENT MATCHES within 100 ppm <<<")
    print()

# ============================================================================
# SECTION 4: THE DELTA ANALYSIS
# ============================================================================

print(f"\n{SEP}")
print("  SECTION 4: ANALYZING delta = p(pi) - x_+")
print(f"{SEP}\n")

print(f"  p(pi) = 4*pi^3 + pi^2 + pi = {fmt(p_pi, 30)}")
print(f"  x_+   = (master quadratic)  = {fmt(x_plus, 30)}")
print(f"  delta = p(pi) - x_+         = {fmt(delta, 30)}")
print()

# Express delta in various units
print(f"  delta / alpha^2    = {fmt(delta * alpha_inv_exp**2, 15)}")
print(f"  delta / alpha      = {fmt(delta * alpha_inv_exp, 15)}")
print(f"  delta * alpha_inv  = {fmt(delta * alpha_inv_exp, 15)}")
print(f"  delta * 137        = {fmt(delta * 137, 15)}")
print(f"  delta * 1728       = {fmt(delta * 1728, 15)}")
print()

# Continued fraction of delta/alpha^2
print(f"  Continued fraction analysis of delta/alpha^2:")
da2 = delta / alpha_exp**2
print(f"  delta/alpha^2 = {fmt(da2, 20)}")

# Get continued fraction coefficients
try:
    cf = mpmath.identify(da2)
    if cf:
        print(f"  mpmath.identify: {cf}")
except:
    pass

# Manual continued fraction
def continued_fraction(x, n_terms=12):
    """Compute continued fraction representation."""
    cfs = []
    for _ in range(n_terms):
        a = mpmath.floor(x)
        cfs.append(int(a))
        frac = x - a
        if abs(frac) < mpmath.mpf('1e-40'):
            break
        x = 1 / frac
    return cfs

print(f"  CF of delta/alpha^2: {continued_fraction(da2)}")
print()

# Check if delta/alpha^2 is close to simple rationals
print("  Checking delta/alpha^2 against simple rationals:")
for p in range(1, 30):
    for q in range(1, 30):
        ratio = mpmath.mpf(p) / q
        if abs(da2 - ratio) / abs(da2) < 0.01:  # within 1%
            error_pct = float(abs(da2 - ratio) / abs(da2) * 100)
            print(f"    delta/alpha^2 ~ {p}/{q} = {float(ratio):.6f}  (error: {error_pct:.3f}%)")
print()

# Check against L-values
print("  Computing Dirichlet L-values for comparison:")
print()

# L(chi_{-4}, s) — Dirichlet character for disc -4
# chi_{-4}(n) = Kronecker symbol (-4/n) = (-1)^((n-1)/2) for odd n, 0 for even
def chi_minus4(n):
    if n % 2 == 0:
        return 0
    return (-1)**((n - 1) // 2)

# L(chi_{-15}, s) — Dirichlet character for disc -15
# Kronecker symbol (-15/n)
def chi_minus15(n):
    if n == 0:
        return 0
    # Use quadratic reciprocity / direct computation
    # (-15/n) = (-1/n)(3/n)(5/n)
    # For simplicity, compute directly mod 15
    if math.gcd(n, 15) > 1:
        return 0
    # Compute Kronecker symbol (-15/n)
    # (-15/n) = (-1/n)(3/n)(5/n)
    # Use lookup table for (-15/n) mod 15
    # The character is periodic with period 15 (conductor divides 15)
    n_mod = n % 15
    # Precomputed: (-15/n) for n = 0..14 coprime to 15
    table = {1: 1, 2: 1, 4: 1, 7: -1, 8: -1, 11: -1, 13: -1, 14: 1}
    return table.get(n_mod, 0)

def L_function(chi, s, terms=50000):
    """Compute L(chi, s) by direct summation."""
    result = mpmath.mpf(0)
    for n in range(1, terms + 1):
        result += chi(n) / mpmath.power(n, s)
    return result

print("  L(chi_{-4}, 1):")
L_minus4_1 = L_function(chi_minus4, 1)
print(f"    = {fmt(L_minus4_1, 20)}")
print(f"    Expected: pi/4 = {fmt(pi/4, 20)}")
print(f"    Match: {abs(L_minus4_1 - pi/4) < 1e-3}")
print()

print("  L(chi_{-15}, 1):")
L_minus15_1 = L_function(chi_minus15, 1)
print(f"    = {fmt(L_minus15_1, 20)}")

# Class number formula: h(-15) * 2*pi / (w * sqrt(15)) where w = 2
# h(-15) = 2, w = 2
# L(chi_{-15}, 1) = 2 * 2 * pi / (2 * sqrt(15)) = 2*pi/sqrt(15)
L_minus15_expected = 2 * pi / sqrt15
print(f"    Expected: 2*pi/sqrt(15) = {fmt(L_minus15_expected, 20)}")
print(f"    Match: {abs(L_minus15_1 - L_minus15_expected) < 1e-2}")
print()

# Now test delta against L-values
print("  Testing delta against L-value expressions:")
candidates_delta = [
    ("delta / L(-4,1)", delta / L_minus4_1),
    ("delta / L(-15,1)", delta / L_minus15_1),
    ("delta * L(-4,1)", delta * L_minus4_1),
    ("delta * L(-15,1)", delta * L_minus15_1),
    ("delta / (L(-4,1) * L(-15,1))", delta / (L_minus4_1 * L_minus15_1)),
    ("delta / alpha^2 / L(-4,1)", da2 / L_minus4_1),
    ("delta / alpha^2 / L(-15,1)", da2 / L_minus15_1),
    ("delta / alpha^2 / (L(-4,1)*L(-15,1))", da2 / (L_minus4_1 * L_minus15_1)),
    ("delta / alpha^2 * L(-4,1)/L(-15,1)", da2 * L_minus4_1 / L_minus15_1),
    ("delta / alpha^2 * L(-15,1)/L(-4,1)", da2 * L_minus15_1 / L_minus4_1),
]

for name, val in candidates_delta:
    cf = continued_fraction(abs(val), 8)
    print(f"  {name} = {fmt(val, 15)}")
    print(f"    CF: {cf}")
    # Check if near simple rational
    for p in range(1, 20):
        for q in range(1, 20):
            if abs(abs(val) - mpmath.mpf(p)/q) < 0.05:
                print(f"    ~ {p}/{q}")
    print()

# ============================================================================
# SECTION 5: PERIOD RATIOS FROM CREMONA 15a1
# ============================================================================

print(f"\n{SEP}")
print("  SECTION 5: CREMONA 15a1 ELLIPTIC CURVE PERIODS")
print(f"{SEP}\n")

# Cremona 15a1: y^2 + xy + y = x^3 + x^2 - 10x - 10
# Minimal Weierstrass model: compute periods via AGM

# Standard form: y^2 = 4x^3 - g2*x - g3
# First convert to standard form
# y^2 + xy + y = x^3 + x^2 - 10x - 10
# Complete the square: (y + x/2 + 1/2)^2 = x^3 + x^2 + x^2/4 - 10x + 1/4 - 10
# = x^3 + 5x^2/4 - 10x - 39/4
# Let Y = y + x/2 + 1/2
# Y^2 = x^3 + (5/4)x^2 - 10x - 39/4

# To convert to short Weierstrass: substitute x = X - 5/12
# Need Tate's algorithm or just use the known invariants
# For 15a1: c4 = 21, c6 = -99, Delta = -15^2 = -225... wait

# Actually, from Cremona tables:
# 15a1: [1,1,1,−10,−10]  (a1=1, a2=1, a3=1, a4=-10, a6=-10)
# Invariants: c4 = 21, c6 = -99

a1, a2, a3, a4, a6 = 1, 1, 1, -10, -10

# Standard invariants
b2 = a1**2 + 4*a2
b4 = a1*a3 + 2*a4
b6 = a3**2 + 4*a6
b8 = a1**2 * a6 - a1*a3*a4 + a2*a6 + a2*a3**2/4 - a4**2  # should be integer

# Wait, use exact formulas
b2_val = a1**2 + 4*a2  # 1 + 4 = 5
b4_val = a1*a3 + 2*a4  # 1 - 20 = -19
b6_val = a3**2 + 4*a6  # 1 - 40 = -39
b8_val = a1**2 * a6 - a1*a3*a4 + a2*a6 + a2*a3**2 - a4**2
# = -10 - 1*1*(-10) + 1*(-10) + 1*1 - 100 = -10 + 10 - 10 + 1 - 100 = -109

c4 = b2_val**2 - 24*b4_val  # 25 + 456 = 481... hmm
# Let me recheck: c4 = b2^2 - 24*b4
c4_val = b2_val**2 - 24*b4_val  # 25 - 24*(-19) = 25 + 456 = 481
# But Cremona says c4 = 21... Let me check the formulas more carefully

# For a general Weierstrass model y^2 + a1*xy + a3*y = x^3 + a2*x^2 + a4*x + a6
# The standard b-invariants are:
# b2 = a1^2 + 4a2
# b4 = 2a4 + a1*a3
# b6 = a3^2 + 4a6
# b8 = a1^2*a6 + 4a2*a6 - a1*a3*a4 + a2*a3^2 - a4^2

print(f"  15a1: [a1,a2,a3,a4,a6] = [{a1},{a2},{a3},{a4},{a6}]")
print(f"  b2 = {b2_val}, b4 = {b4_val}, b6 = {b6_val}")

# c4 = b2^2 - 24b4
c4_val = b2_val**2 - 24*b4_val
c6_val = -b2_val**3 + 36*b2_val*b4_val - 216*b6_val
disc_val = -b2_val**2 * b8_val - 8*b4_val**3 - 27*b6_val**2 + 9*b2_val*b4_val*b6_val

print(f"  c4 = {c4_val}")
print(f"  c6 = {c6_val}")
print()

# The j-invariant of 15a1
j_15a1 = mpmath.mpf(c4_val)**3 / mpmath.mpf(disc_val) if disc_val != 0 else None
if j_15a1:
    print(f"  j(15a1) = c4^3/Delta = {c4_val}^3 / {disc_val}")
    print(f"          = {fmt(j_15a1, 15)}")
print()

# Compute periods using numerical integration
# For the curve in short Weierstrass form: Y^2 = 4X^3 - g2*X - g3
# g2 = (c4)/12, g3 = (c6)/216
g2 = mpmath.mpf(c4_val) / 12
g3 = mpmath.mpf(c6_val) / 216
print(f"  g2 = {fmt(g2, 15)}")
print(f"  g3 = {fmt(g3, 15)}")
print()

# Period lattice via AGM or elliptic integrals
# For Y^2 = 4X^3 - g2*X - g3
# Roots of 4x^3 - g2*x - g3 = 0
# Use Cardano's formula or numerical roots
coeffs = [4, 0, -g2, -g3]  # 4x^3 + 0x^2 - g2*x - g3
roots = mpmath.polyroots(coeffs)
print(f"  Roots of 4x^3 - g2*x - g3:")
for k, r in enumerate(roots):
    print(f"    e{k+1} = {fmt(r, 15)}")
print()

# Sort roots: e1 > e2 > e3 (if all real)
real_roots = sorted([mpmath.re(r) for r in roots if abs(mpmath.im(r)) < 1e-20], reverse=True)
if len(real_roots) == 3:
    e1, e2, e3 = [mpmath.mpf(r) for r in real_roots]

    # Periods via complete elliptic integrals
    # omega1 = 2 * integral from e1 to inf of dx/sqrt(4x^3 - g2*x - g3)
    # Using the standard formula:
    # omega1 = 2K(k) / sqrt(e1 - e3)  where k^2 = (e2 - e3)/(e1 - e3)

    k_sq = (e2 - e3) / (e1 - e3)
    k_val = mpmath.sqrt(k_sq)
    kp_sq = (e1 - e2) / (e1 - e3)

    K_k = mpmath.ellipk(k_sq)
    K_kp = mpmath.ellipk(kp_sq)

    omega1 = 2 * K_k / mpmath.sqrt(e1 - e3)
    omega2 = 2 * mpmath.mpc(0, 1) * K_kp / mpmath.sqrt(e1 - e3)

    print(f"  Period omega1 = {fmt(omega1, 20)}")
    print(f"  Period omega2 = {fmt(omega2, 20)}")
    print(f"  tau = omega2/omega1 = {fmt(omega2/omega1, 20)}")
    print()

    # Compare to varpi
    print(f"  omega1 / varpi = {fmt(omega1/varpi, 15)}")
    print(f"  omega1 / pi    = {fmt(omega1/pi, 15)}")
    print(f"  omega1 / G*    = {fmt(omega1/G_star, 15)}")
    print(f"  omega1 * alpha_inv = {fmt(omega1 * alpha_inv_exp, 15)}")
    print()

    # Check if period ratios involve 137
    if abs(omega1) > 1e-30:
        ratio = alpha_inv_exp / omega1
        print(f"  137.036 / omega1 = {fmt(ratio, 15)}")
        cf_ratio = continued_fraction(abs(ratio), 8)
        print(f"    CF: {cf_ratio}")
    print()
else:
    print("  Not all roots are real — complex multiplication present")
    print(f"  (roots: {[fmt(r, 12) for r in roots]})")
    print()

# ============================================================================
# SECTION 6: MIXED FTD-RFT FORMULAS
# ============================================================================

print(f"\n{SEP}")
print("  SECTION 6: MIXED FTD-RFT FORMULAS")
print(f"{SUB}")
print("  Testing if combining FTD and RFT objects gives EXACT 1/alpha")
print(f"{SEP}\n")

# Key idea: Can we write 1/alpha = F(G*, pi, {3,4,7,13}) exactly?
# We already know:
#   FTD:  x_+ = 8G*^2 + 8G*^2*sqrt(1 - 1/G*) ≈ 137.0362 (1.26 ppm)
#   RFT:  4*pi^3 + pi^2 + pi ≈ 137.0363 (2.22 ppm)
#   Both are ABOVE the experimental value

# The correction:
eps_ftd = x_plus - alpha_inv_exp
eps_rft = p_pi - alpha_inv_exp

print(f"  eps_FTD = x_+ - 1/alpha_exp    = {fmt(eps_ftd, 20)}")
print(f"  eps_RFT = p(pi) - 1/alpha_exp  = {fmt(eps_rft, 20)}")
print(f"  delta   = p(pi) - x_+          = {fmt(delta, 20)}")
print()

# Weighted average
# If both have similar accuracy, try linear combination
# w1 * x_+ + w2 * p(pi) = 1/alpha
# Constraint: w1 + w2 = 1 (weighted avg)
# w1 * x_+ + (1-w1) * p(pi) = alpha_inv_exp
# w1 * (x_+ - p_pi) = alpha_inv_exp - p_pi
# w1 = (alpha_inv_exp - p_pi) / (x_+ - p_pi) = -eps_rft / (x_+ - p_pi)
w1 = -eps_rft / (x_plus - p_pi)
w2 = 1 - w1

print(f"  Weighted average: w1*x_+ + w2*p(pi) = 1/alpha")
print(f"    w1 = {fmt(w1, 15)}")
print(f"    w2 = {fmt(w2, 15)}")
print(f"    w1/(w1+w2) = {fmt(w1/(w1+w2), 15)}")
print(f"    Result: {fmt(w1 * x_plus + w2 * p_pi, 25)}")
print(f"    Target: {fmt(alpha_inv_exp, 25)}")
print()

# Check if w1 or w2 are nice rationals
print(f"  CF of w1: {continued_fraction(w1, 10)}")
print(f"  CF of w2: {continued_fraction(w2, 10)}")
print()

# Try: alpha_inv = a * x_+ + b * p(pi) for integers a, b
# This is overdetermined for a single equation, but try:
# alpha_inv = x_+ - c * delta  where c is the FTD mixing coefficient
c_mix = (x_plus - alpha_inv_exp) / delta
print(f"  alpha_inv = x_+ - c * delta")
print(f"    c = eps_FTD / delta = {fmt(c_mix, 20)}")
print(f"    CF of c: {continued_fraction(c_mix, 10)}")
print()

# Try to match c to framework expressions
c_candidates = [
    ("N_c / N_base", mpmath.mpf(N_c) / N_base),
    ("N_base / (N_base + N_c)", mpmath.mpf(N_base) / (N_base + N_c)),
    ("b_3 / N_eff", mpmath.mpf(b_3) / N_eff),
    ("(N_c + N_base) / N_eff", mpmath.mpf(N_c + N_base) / N_eff),
    ("N_c^2 / 47", mpmath.mpf(N_c**2) / 47),
    ("9/47", mpmath.mpf(9) / 47),
    ("alpha", alpha_exp),
    ("alpha * pi", alpha_exp * pi),
    ("alpha * N_base", alpha_exp * N_base),
    ("1/pi^2", 1 / pi**2),
    ("alpha/pi", alpha_exp / pi),
]

print("  Matching c against framework expressions:")
for name, val in c_candidates:
    err = float(abs(c_mix - val) / abs(c_mix) * 100)
    marker = " ***" if err < 5 else ""
    print(f"    {name:30s} = {fmt(val, 12):>18s}  (err: {err:.2f}%){marker}")
print()

# ============================================================================
# SECTION 7: THE GEOMETRIC MEAN TEST
# ============================================================================

print(f"\n{SEP}")
print("  SECTION 7: GEOMETRIC MEAN AND OTHER MEANS")
print(f"{SEP}\n")

# Arithmetic mean
am = (x_plus + p_pi) / 2
# Geometric mean
gm = mpmath.sqrt(x_plus * p_pi)
# Harmonic mean
hm = 2 * x_plus * p_pi / (x_plus + p_pi)

print(f"  x_+    = {fmt(x_plus, 25)}")
print(f"  p(pi)  = {fmt(p_pi, 25)}")
print(f"  1/alpha= {fmt(alpha_inv_exp, 25)}")
print()
print(f"  Arithmetic mean = {fmt(am, 25)}  ({float(ppm_error(am, alpha_inv_exp)):.2f} ppm)")
print(f"  Geometric mean  = {fmt(gm, 25)}  ({float(ppm_error(gm, alpha_inv_exp)):.2f} ppm)")
print(f"  Harmonic mean   = {fmt(hm, 25)}  ({float(ppm_error(hm, alpha_inv_exp)):.2f} ppm)")
print()

# Power mean: M_p = ((x_+^p + p_pi^p)/2)^(1/p)
print("  Power means M_p:")
for p in [-3, -2, -1, -0.5, 0.5, 1, 2, 3]:
    if p == 0:
        pm = gm
    else:
        pm = ((x_plus**p + p_pi**p)/2)**(1/p)
    err_ppm = float(ppm_error(pm, alpha_inv_exp))
    marker = " ***" if err_ppm < 0.5 else ""
    print(f"    M_{p:5.1f} = {fmt(pm, 20)}  ({err_ppm:.3f} ppm){marker}")
print()

# Weighted power mean
print("  Searching for weighted mean w*x_+ + (1-w)*p(pi) that matches 1/alpha...")
# Already done above, but let's also check:
# ((w*x_+^p + (1-w)*p_pi^p))^(1/p) for various p, w
best_match = (1e10, 0, 0)
for p in [x/10 for x in range(-30, 31)]:
    if abs(p) < 0.01:
        continue
    for w_num in range(1, 20):
        for w_den in range(1, 20):
            w = mpmath.mpf(w_num) / w_den
            if w <= 0 or w >= 1:
                continue
            try:
                pm = (w * x_plus**p + (1 - w) * p_pi**p)**(1/p)
                err = float(abs(pm - alpha_inv_exp))
                if err < best_match[0]:
                    best_match = (err, p, w)
            except:
                pass

if best_match[0] < 1e-6:
    p_best, w_best = best_match[1], best_match[2]
    pm_best = (w_best * x_plus**p_best + (1 - w_best) * p_pi**p_best)**(1/p_best)
    print(f"  Best match: p={p_best:.1f}, w={fmt(w_best, 8)}")
    print(f"    Value = {fmt(pm_best, 25)}")
    print(f"    Error = {float(ppm_error(pm_best, alpha_inv_exp)):.4f} ppm")
print()

# ============================================================================
# SECTION 8: THE GRAND SUMMARY
# ============================================================================

print(f"\n{SEP}")
print("  GRAND SUMMARY: DECISIVE YES/NO ANSWERS")
print(f"{SEP}\n")

print("  Q1: Does any eta-quotient of level 60 at a CM point give 1/alpha?")
if eta_matches:
    print(f"       YES — {len(eta_matches)} matches found!")
    print(f"       Best: {eta_matches[0][0]}")
    print(f"              = {fmt(eta_matches[0][1], 15)} ({float(ppm_error(eta_matches[0][1], alpha_inv_exp)):.2f} ppm)")
else:
    print("       NO — No eta-quotient within 100 ppm of 1/alpha")
print()

print("  Q2: Does any product f(i) * g(tau_1) = 1/alpha?")
product_matches = [m for m in matches_found if m[0] in ('product', 'ratio', 'power')]
if product_matches:
    print(f"       YES — {len(product_matches)} matches found!")
    best = product_matches[0]
    print(f"       Best: {best[1]}")
    print(f"              = {fmt(best[2], 15)} ({float(ppm_error(best[2], alpha_inv_exp)):.2f} ppm)")
else:
    print("       NO — No product/ratio within 100 ppm of 1/alpha")
print()

print("  Q3: Can delta/alpha^2 be expressed as a simple rational or L-value ratio?")
# Check if any of our candidates were close to simple rationals
print(f"       delta/alpha^2 = {fmt(da2, 15)}")
print(f"       CF: {continued_fraction(da2, 10)}")
# If first few CF coefficients suggest a rational...
cf_da2 = continued_fraction(da2, 5)
if len(cf_da2) >= 2:
    # Convergents
    p0, p1 = cf_da2[0], cf_da2[0]*cf_da2[1] + 1 if len(cf_da2) > 1 else cf_da2[0]
    q0, q1 = 1, cf_da2[1] if len(cf_da2) > 1 else 1
    print(f"       First convergent: {p0}/{q0}")
    print(f"       Second convergent: {p1}/{q1}")
print()

print("  Q4: Is there an exact formula combining FTD and RFT?")
print(f"       Mixing coefficient c = {fmt(c_mix, 15)}")
print(f"       alpha_inv = x_+ - c * (p(pi) - x_+)")
print(f"       Best framework match for c: see above")
print()

print("  Q5: Does any mean of x_+ and p(pi) match 1/alpha?")
print(f"       Geometric mean: {float(ppm_error(gm, alpha_inv_exp)):.3f} ppm")
print(f"       Arithmetic mean: {float(ppm_error(am, alpha_inv_exp)):.3f} ppm")
print(f"       Harmonic mean: {float(ppm_error(hm, alpha_inv_exp)):.3f} ppm")
print()

print(f"\n{SEP}")
print("  END OF UNIFICATION TEST")
print(f"{SEP}")
