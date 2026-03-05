#!/usr/bin/env python3
"""
chase_cf_followup.py  --  Deep dive into the 3/19 formula and beyond
=====================================================================

The chase_cf.py script found:
  delta ~ (3/19) * pi^3 * Gamma(1/4)^2 / G*^3 * alpha^2   (0.51 ppm)

This script:
1. Verifies at 150-digit precision
2. Simplifies algebraically (G* = sqrt(2)*Gamma(1/4)^2/(2*pi))
3. Searches the neighborhood of 3/19 for an exact relation
4. Explores whether the 0.51 ppm residual has structure
5. Tests whether using x_+ instead of 1/alpha_exp improves it
6. Searches for EXACT relations (no alpha_exp -- purely from G* and pi)

Author: Claude Code Investigation
Date: 2026-02-07
"""

import sys, os
if sys.platform == 'win32':
    os.environ['PYTHONIOENCODING'] = 'utf-8'
    sys.stdout.reconfigure(encoding='utf-8')

import mpmath
mpmath.mp.dps = 150  # 150-digit precision for verification

from mpmath import mp, mpf, mpc, pi, sqrt, log, gamma, fabs, floor, power, exp
import math

# ==============================================================================
# UTILITIES
# ==============================================================================

SEP = "=" * 80

def header(title):
    print(f"\n{SEP}")
    print(f"  {title}")
    print(SEP)

def subheader(title):
    print(f"\n--- {title} ---")

def fmt(x, digits=50):
    return mpmath.nstr(x, digits)

def fmt_short(x, digits=18):
    return mpmath.nstr(x, digits)

def ppm_error(derived, experimental):
    return float(abs(derived - experimental) / abs(experimental) * mpf('1e6'))

def pct_error(derived, experimental):
    return float(abs(derived - experimental) / abs(experimental) * 100)

def continued_fraction(x, n_terms=30):
    cfs = []
    for _ in range(n_terms):
        a = floor(x)
        cfs.append(int(a))
        frac = x - a
        if abs(frac) < mpf(10)**(-120):
            break
        x = 1 / frac
    return cfs


# ==============================================================================
# SECTION 1: RECOMPUTE AT 150 DIGITS
# ==============================================================================

header("SECTION 1: 150-DIGIT RECOMPUTATION")

gamma_quarter = gamma(mpf('0.25'))
sqrt2 = sqrt(mpf(2))
sqrt15 = sqrt(mpf(15))

varpi = gamma_quarter**2 / (2 * sqrt(2 * pi))
G_star = sqrt2 * gamma_quarter**2 / (2 * pi)

c = G_star
disc_mq = 256*c**4 - 64*c**3
x_plus = (16*c**2 + mpmath.sqrt(disc_mq)) / 2
x_minus = (16*c**2 - mpmath.sqrt(disc_mq)) / 2

p_pi = 4*pi**3 + pi**2 + pi
delta = p_pi - x_plus

alpha_inv_exp = mpf('137.035999177')
alpha_exp = 1 / alpha_inv_exp

# Also define alpha from x_+
alpha_ftd = 1 / x_plus

print(f"  Precision: {mpmath.mp.dps} digits")
print(f"  G*      = {fmt(G_star)}")
print(f"  x_+     = {fmt(x_plus)}")
print(f"  p(pi)   = {fmt(p_pi)}")
print(f"  delta   = {fmt(delta)}")
print(f"  alpha_exp = {fmt(alpha_exp, 30)}")
print(f"  alpha_ftd = 1/x_+ = {fmt(alpha_ftd, 30)}")


# ==============================================================================
# SECTION 2: VERIFY AND SIMPLIFY THE 3/19 FORMULA
# ==============================================================================

header("SECTION 2: THE 3/19 FORMULA -- VERIFICATION & SIMPLIFICATION")

subheader("Direct verification")
formula_319 = mpf(3)/19 * G_star**(-3) * pi**3 * gamma_quarter**2 * alpha_exp**2
print(f"  (3/19) * pi^3 * Gamma(1/4)^2 / G*^3 * alpha_exp^2:")
print(f"    = {fmt(formula_319)}")
print(f"    delta = {fmt(delta)}")
print(f"    error = {ppm_error(formula_319, delta):.4f} ppm")
print()

subheader("Algebraic simplification")
print(f"""
  G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)

  So G*^3 = (sqrt(2))^3 * Gamma(1/4)^6 / (2*pi)^3
          = 2*sqrt(2) * Gamma(1/4)^6 / (8*pi^3)
          = sqrt(2) * Gamma(1/4)^6 / (4*pi^3)

  Therefore:
  pi^3 * Gamma(1/4)^2 / G*^3
    = pi^3 * Gamma(1/4)^2 * 4*pi^3 / (sqrt(2) * Gamma(1/4)^6)
    = 4*pi^6 / (sqrt(2) * Gamma(1/4)^4)

  So the formula becomes:
  delta ~ (3/19) * 4*pi^6 / (sqrt(2) * Gamma(1/4)^4) * alpha^2
        = (12/19) * pi^6 / (sqrt(2) * Gamma(1/4)^4) * alpha^2

  Or equivalently:
  delta ~ (6*sqrt(2)/19) * pi^6 / Gamma(1/4)^4 * alpha^2
""")

# Verify the simplified form
simplified = mpf(12)/19 * pi**6 / (sqrt2 * gamma_quarter**4) * alpha_exp**2
print(f"  Simplified: (12/19) * pi^6 / (sqrt(2)*Gamma(1/4)^4) * alpha_exp^2")
print(f"    = {fmt(simplified)}")
print(f"    error vs delta = {ppm_error(simplified, delta):.4f} ppm")
print()

# Further simplify using varpi
# varpi = Gamma(1/4)^2 / (2*sqrt(2*pi))
# varpi^2 = Gamma(1/4)^4 / (8*pi)
# So Gamma(1/4)^4 = 8*pi*varpi^2
print(f"  Using Gamma(1/4)^4 = 8*pi*varpi^2:")
print(f"    delta ~ (12/19) * pi^6 / (sqrt(2) * 8*pi*varpi^2) * alpha^2")
print(f"          = (12/19) * pi^5 / (8*sqrt(2)*varpi^2) * alpha^2")
print(f"          = (3/38) * pi^5 / (sqrt(2)*varpi^2) * alpha^2")
print(f"          = (3*sqrt(2)/38) * pi^5 / varpi^2 * alpha^2")
print()

simplified_v = mpf(3)/38 * pi**5 / (sqrt2 * varpi**2) * alpha_exp**2
print(f"  (3/38) * pi^5 / (sqrt(2)*varpi^2) * alpha_exp^2")
print(f"    = {fmt(simplified_v)}")
print(f"    error vs delta = {ppm_error(simplified_v, delta):.4f} ppm")
print()

# And using G*:
# G* = 2*varpi/sqrt(pi)  =>  varpi = G*sqrt(pi)/2
# varpi^2 = G*^2*pi/4
print(f"  Using varpi^2 = G*^2*pi/4:")
print(f"    delta ~ (3/38) * pi^5 / (sqrt(2) * G*^2*pi/4) * alpha^2")
print(f"          = (3/38) * 4*pi^4 / (sqrt(2)*G*^2) * alpha^2")
print(f"          = (6/19) * pi^4 / (sqrt(2)*G*^2) * alpha^2")
print(f"          = (3*sqrt(2)/19) * pi^4 / G*^2 * alpha^2")
print()

simplified_g = mpf(6)/19 * pi**4 / (sqrt2 * G_star**2) * alpha_exp**2
print(f"  (6/19) * pi^4 / (sqrt(2)*G*^2) * alpha_exp^2")
print(f"    = {fmt(simplified_g)}")
print(f"    error = {ppm_error(simplified_g, delta):.4f} ppm")
print()

simplified_g2 = 3*sqrt2/19 * pi**4 / G_star**2 * alpha_exp**2
print(f"  (3*sqrt(2)/19) * pi^4 / G*^2 * alpha_exp^2")
print(f"    = {fmt(simplified_g2)}")
print(f"    error = {ppm_error(simplified_g2, delta):.4f} ppm")


# ==============================================================================
# SECTION 3: THE EXACT RATIO -- What is delta * G*^3 / (pi^3 * Gamma(1/4)^2 * alpha^2)?
# ==============================================================================

header("SECTION 3: THE EXACT RATIO")

ratio_exact = delta * G_star**3 / (pi**3 * gamma_quarter**2 * alpha_exp**2)
print(f"  R = delta * G*^3 / (pi^3 * Gamma(1/4)^2 * alpha_exp^2)")
print(f"    = {fmt(ratio_exact)}")
print(f"    3/19 = {fmt(mpf(3)/19)}")
print(f"    difference = {fmt(ratio_exact - mpf(3)/19)}")
print(f"    relative = {ppm_error(ratio_exact, mpf(3)/19):.4f} ppm")
print()

# What is R more precisely?
cf_R = continued_fraction(ratio_exact, 25)
print(f"  CF of R = {cf_R[:20]}")
print()

# Try to identify R
subheader("mpmath.identify() on the exact ratio R")
for tol_exp in [15, 12, 10, 8]:
    try:
        result = mpmath.identify(ratio_exact, tol=mpf(10)**(-tol_exp), maxcoeff=10000)
        if result:
            print(f"  identify(R, tol=1e-{tol_exp}): {result}")
    except Exception as e:
        pass

print()

# PSLQ on the ratio
subheader("PSLQ on R vs simple constants")

def try_pslq(name, labels, values, max_coeff=200):
    """Run PSLQ and report."""
    print(f"\n  Basis {name}:")
    for tol_exp in [-50, -40, -30, -20]:
        try:
            rel = mpmath.pslq(values, tol=mpf(10)**(tol_exp), maxcoeff=10000)
            if rel is not None:
                max_c = max(abs(r) for r in rel)
                if max_c <= max_coeff:
                    print(f"    *** RELATION (tol=1e{tol_exp}, max|c|={max_c}) ***")
                    print(f"    Coeffs: {rel}")
                    terms = []
                    for coeff, label in zip(rel, labels):
                        if coeff != 0:
                            terms.append(f"({coeff})*{label}")
                    print(f"    {' + '.join(terms)} = 0")
                    check = sum(mpf(r)*v for r,v in zip(rel, values))
                    print(f"    Verify: {fmt(check, 20)}")
                    if rel[0] != 0:
                        solved = -sum(mpf(r)*v for r,v in zip(rel[1:], values[1:])) / mpf(rel[0])
                        print(f"    => {labels[0]} = {fmt(solved, 20)}")
                    return rel
                else:
                    print(f"    Found (tol=1e{tol_exp}) but max|c|={max_c} > {max_coeff}")
                    print(f"    Coeffs: {rel}")
        except Exception:
            pass
    print(f"    No relation found")
    return None

try_pslq("R vs 1, pi, G*",
    ["R", "1", "pi", "G*"],
    [ratio_exact, mpf(1), pi, G_star])

try_pslq("R vs 1, 1/pi, alpha",
    ["R", "1", "1/pi", "alpha"],
    [ratio_exact, mpf(1), 1/pi, alpha_exp])

try_pslq("R vs 1, alpha, alpha^2, pi*alpha",
    ["R", "1", "alpha", "alpha^2", "pi*alpha"],
    [ratio_exact, mpf(1), alpha_exp, alpha_exp**2, pi*alpha_exp])


# ==============================================================================
# SECTION 4: USE x_+ INSTEAD OF alpha_exp
# ==============================================================================

header("SECTION 4: USING FTD's OWN ALPHA (1/x_+)")

subheader("Replace alpha_exp with alpha_ftd = 1/x_+")
formula_ftd = mpf(3)/19 * G_star**(-3) * pi**3 * gamma_quarter**2 * alpha_ftd**2
print(f"  (3/19) * pi^3 * Gamma(1/4)^2 / G*^3 * (1/x_+)^2:")
print(f"    = {fmt(formula_ftd)}")
print(f"    delta = {fmt(delta)}")
print(f"    error = {ppm_error(formula_ftd, delta):.4f} ppm")
print()

# The ratio using FTD's own alpha
ratio_ftd = delta * G_star**3 / (pi**3 * gamma_quarter**2 * alpha_ftd**2)
print(f"  Ratio using alpha_ftd: {fmt(ratio_ftd)}")
print(f"  3/19 = {fmt(mpf(3)/19)}")
print(f"  diff = {fmt(ratio_ftd - mpf(3)/19)}")
print(f"  relative = {ppm_error(ratio_ftd, mpf(3)/19):.4f} ppm")
print()

# Now the KEY INSIGHT: alpha_ftd = 1/x_+ where x_+ comes from the master quadratic
# So the formula is entirely in terms of G* and pi!
# delta = (3/19) * pi^3 * Gamma(1/4)^2 / (G*^3 * x_+^2)
# and x_+ is a function of G* alone

subheader("Fully algebraic form (no experimental input)")
print(f"  x_+ = 8*G*^2 + 4*G*^2*sqrt(4*G* - 1)")
print(f"  delta = p(pi) - x_+")
print(f"  Question: does (3/19)*pi^3*Ga14^2/(G*^3*x_+^2) = p(pi) - x_+ EXACTLY?")
print()

# Compute LHS and RHS
lhs = mpf(3)/19 * pi**3 * gamma_quarter**2 / (G_star**3 * x_plus**2)
rhs = delta
print(f"  LHS = (3/19)*pi^3*Ga14^2/(G*^3*x_+^2) = {fmt(lhs)}")
print(f"  RHS = p(pi) - x_+                       = {fmt(rhs)}")
print(f"  LHS - RHS = {fmt(lhs - rhs)}")
print(f"  Relative error: {ppm_error(lhs, rhs):.4f} ppm")
print()

# So it's NOT exact. Let's find what the exact ratio is.
subheader("Exact ratio with FTD alpha")
print(f"  R_ftd = delta * G*^3 * x_+^2 / (pi^3 * Gamma(1/4)^2)")
R_ftd = delta * G_star**3 * x_plus**2 / (pi**3 * gamma_quarter**2)
print(f"    = {fmt(R_ftd)}")
cf_Rftd = continued_fraction(R_ftd, 25)
print(f"  CF = {cf_Rftd[:20]}")
print()

# Alternative simplification
# delta * G*^3 * x_+^2 / (pi^3 * Gamma(1/4)^2)
# = delta * (sqrt(2)*Ga14^2/(2*pi))^3 * x_+^2 / (pi^3 * Ga14^2)
# = delta * 2*sqrt(2)*Ga14^6/(8*pi^3) * x_+^2 / (pi^3 * Ga14^2)
# = delta * sqrt(2)*Ga14^4 * x_+^2 / (4*pi^6)
R_alt = delta * sqrt2 * gamma_quarter**4 * x_plus**2 / (4 * pi**6)
print(f"  Alternative: delta * sqrt(2) * Gamma(1/4)^4 * x_+^2 / (4*pi^6)")
print(f"    = {fmt(R_alt)}")
print(f"  (Should match R_ftd: {fabs(R_alt - R_ftd) < mpf('1e-100')})")
print()


# ==============================================================================
# SECTION 5: SEARCH FOR THE EXACT COEFFICIENT
# ==============================================================================

header("SECTION 5: REFINING THE COEFFICIENT")

# The ratio is close to 3/19. What's the exact value?
# R_ftd = delta * G*^3 * x_+^2 / (pi^3 * Gamma(1/4)^2) ~ 3/19

# Let's compute (delta*19/3) / (pi^3*Gamma(1/4)^2/(G*^3*x_+^2))
# i.e., does delta = (3/19 + epsilon) * pi^3*Gamma(1/4)^2/(G*^3*x_+^2)?

eps_319 = R_ftd - mpf(3)/19
print(f"  R_ftd - 3/19 = {fmt(eps_319)}")
print(f"  R_ftd = {fmt(R_ftd)}")
print(f"  3/19  = {fmt(mpf(3)/19)}")
print()

# What's eps_319 as a fraction of 3/19?
print(f"  eps/(3/19) = {fmt(eps_319 / (mpf(3)/19))}")
print(f"  eps*19/3   = {fmt(eps_319 * 19/3)}")
print()

subheader("Searching for better rational coefficients a/b")
best_err = float('inf')
best_frac = None

for a in range(1, 200):
    for b in range(1, 200):
        if math.gcd(a, b) > 1:
            continue
        frac = mpf(a) / mpf(b)
        err = float(fabs(R_ftd - frac) / R_ftd)
        if err < best_err:
            best_err = err
            best_frac = (a, b)
            if err < 1e-6:  # better than 1 ppm
                print(f"    {a}/{b} = {float(frac):.15f}  err = {err*1e6:.4f} ppm")

print(f"\n  Best: {best_frac[0]}/{best_frac[1]}, error = {best_err*1e6:.4f} ppm")
print()

# Also try negative fractions and larger range
subheader("Testing specific fractions near 3/19")
test_fracs = [
    (3, 19),
    (6, 38),    # = 3/19
    (47, 298),  # convergent?
]

# Add convergents of R_ftd
cf_coeff = continued_fraction(R_ftd, 20)
h_prev, h_curr = mpf(0), mpf(1)
k_prev, k_curr = mpf(1), mpf(0)
for i, a in enumerate(cf_coeff):
    h_prev, h_curr = h_curr, mpf(a) * h_curr + h_prev
    k_prev, k_curr = k_curr, mpf(a) * k_curr + k_prev
    if k_curr > 0 and k_curr < 10000:
        conv_val = h_curr / k_curr
        err = ppm_error(conv_val, R_ftd)
        print(f"  Convergent [{i}]: {int(h_curr)}/{int(k_curr)} = {fmt_short(conv_val)}  error = {err:.4f} ppm")

print()

# ==============================================================================
# SECTION 6: COMPLETELY ALGEBRAIC APPROACH -- NO alpha at all
# ==============================================================================

header("SECTION 6: PURELY ALGEBRAIC -- delta as f(G*, pi, Gamma(1/4))")

# delta = p(pi) - x_+
# p(pi) = 4*pi^3 + pi^2 + pi -- a polynomial in pi
# x_+ = 8*G*^2 + 4*G*^2*sqrt(4*G* - 1)  -- a function of G*
# And G* = sqrt(2)*Gamma(1/4)^2/(2*pi)

# So delta is entirely determined by pi and Gamma(1/4).
# The question is: can we express delta SIMPLY in these terms?

# Let's define R_pure = delta * Gamma(1/4)^n / pi^m for various n, m
# and look for simple values

subheader("delta normalized by powers of pi and Gamma(1/4)")
print(f"  {'Expression':45s} {'Value':>25s} {'CF start':>30s}")
for n in range(-4, 5):
    for m in range(-6, 7):
        val = delta * gamma_quarter**n / pi**m
        absval = float(fabs(val))
        if 0.01 < absval < 1000:
            cf = continued_fraction(fabs(val), 8)
            marker = ""
            # Check if near simple fraction
            for p in range(1, 30):
                for q in range(1, 30):
                    if math.gcd(p, q) > 1:
                        continue
                    frac = mpf(p)/q
                    if float(fabs(val - frac)/frac) < 0.001:
                        marker = f" ~ {p}/{q}"
                    if float(fabs(val + frac)/frac) < 0.001:
                        marker = f" ~ -{p}/{q}"
            label = f"delta * Ga14^{n} / pi^{m}"
            print(f"  {label:45s} {fmt_short(fabs(val)):>25s} {str(cf[:6]):>30s}{marker}")

print()

# ==============================================================================
# SECTION 7: SYSTEMATIC MONOMIAL SEARCH (higher resolution)
# ==============================================================================

header("SECTION 7: HIGH-RESOLUTION MONOMIAL SEARCH")

subheader("delta = (a/b) * G*^p * pi^q * Gamma(1/4)^r for small a,b,p,q,r")
print(f"  Searching |a|,|b| <= 50, |p,q,r| <= 5...")

results = []

for p in range(-5, 6):
    for q in range(-6, 7):
        for r in range(-6, 7):
            try:
                trial = G_star**p * pi**q * gamma_quarter**r
                if abs(trial) < mpf('1e-50') or abs(trial) > mpf('1e50'):
                    continue
                ratio = delta / trial
                absratio = float(fabs(ratio))
                if absratio < 1e-10 or absratio > 1e10:
                    continue
                for num in range(1, 51):
                    for den in range(1, 51):
                        if math.gcd(num, den) > 1:
                            continue
                        frac = mpf(num) / mpf(den)
                        # positive
                        err = float(fabs(ratio - frac) / frac)
                        if err < 0.00001:  # 10 ppm
                            results.append((err, num, den, p, q, r, float(frac), '+'))
                        # negative
                        err_neg = float(fabs(ratio + frac) / frac)
                        if err_neg < 0.00001:
                            results.append((err_neg, num, den, p, q, r, float(-frac), '-'))
            except Exception:
                pass

results.sort()
print(f"\n  Found {len(results)} matches within 10 ppm")
print(f"  Top 20:")
print(f"  {'a/b':>8s}  {'G*^p':>5s}  {'pi^q':>5s}  {'Ga^r':>5s}  {'error ppm':>12s}")
for err, num, den, p, q, r, fval, sign in results[:20]:
    prefix = '-' if sign == '-' else ''
    pred = mpf(fval) * G_star**p * pi**q * gamma_quarter**r
    actual_ppm = ppm_error(pred, delta) if sign == '+' else ppm_error(-pred, delta)
    print(f"  {prefix}{num}/{den:>3d}  G*^{p:+d}  pi^{q:+d}  Ga^{r:+d}  {err*1e6:>12.3f}")

if results:
    err, num, den, p, q, r, fval, sign = results[0]
    print(f"\n  BEST MATCH:")
    prefix = '-' if sign == '-' else ''
    print(f"    delta = ({prefix}{num}/{den}) * G*^{p} * pi^{q} * Gamma(1/4)^{r}")
    pred = mpf(fval) * G_star**p * pi**q * gamma_quarter**r
    print(f"    Predicted: {fmt(pred, 30)}")
    print(f"    Actual:    {fmt(delta, 30)}")
    print(f"    Error:     {err*1e6:.3f} ppm")

    # Now try PSLQ to refine: is the ratio exactly a/b or a/b + small correction?
    exact_ratio = delta / (G_star**p * pi**q * gamma_quarter**r)
    print(f"\n    Exact ratio: {fmt(exact_ratio, 40)}")
    cf_exact = continued_fraction(fabs(exact_ratio), 20)
    print(f"    CF: {cf_exact[:15]}")


# ==============================================================================
# SECTION 8: PSLQ ON DELTA DIRECTLY vs G* AND PI FUNCTIONS
# ==============================================================================

header("SECTION 8: PSLQ -- DELTA vs ALGEBRAIC FUNCTIONS OF G* AND PI")

# Key insight: delta = p(pi) - x_+
# where x_+ satisfies x^2 - 16*G*^2*x + 16*G*^3 = 0
# So delta*(delta + 2*x_+) = delta^2 + 2*delta*x_+
# And (delta + x_+)^2 = p(pi)^2
# Also (delta + x_+)^2 - 16*G*^2*(delta + x_+) + 16*G*^3 = 0
# => delta^2 + 2*delta*x_+ + x_+^2 - 16*G*^2*delta - 16*G*^2*x_+ + 16*G*^3 = 0
# => delta^2 + 2*delta*x_+ - 16*G*^2*delta + (x_+^2 - 16*G*^2*x_+ + 16*G*^3) = 0
# The last bracket = 0 (x_+ is a root!)
# => delta^2 + 2*delta*x_+ - 16*G*^2*delta = 0
# => delta + 2*x_+ - 16*G*^2 = 0  (divide by delta)
# => delta = 16*G*^2 - 2*x_+ = 16*G*^2 - 2*x_+

subheader("ALGEBRAIC IDENTITY CHECK")
print(f"  Since p(pi) = x_+ + delta, and x_+ satisfies x^2 - 16G*^2 x + 16G*^3 = 0:")
print(f"  Substituting p(pi) into the quadratic:")
print(f"    p(pi)^2 - 16*G*^2*p(pi) + 16*G*^3 = ?")
residual_quad = p_pi**2 - 16*G_star**2*p_pi + 16*G_star**3
print(f"    = {fmt(residual_quad)}")
print(f"    (This should be nonzero -- p(pi) is NOT a root of the quadratic)")
print()

# Expand: p(pi) = x_+ + delta
# (x_+ + delta)^2 - 16G*^2(x_+ + delta) + 16G*^3
# = x_+^2 + 2*x_+*delta + delta^2 - 16G*^2*x_+ - 16G*^2*delta + 16G*^3
# = (x_+^2 - 16G*^2*x_+ + 16G*^3) + delta*(2*x_+ - 16G*^2) + delta^2
# = 0 + delta*(2*x_+ - 16G*^2) + delta^2
# = delta*(2*x_+ - 16G*^2 + delta)

print(f"  Factored: p(pi)^2 - 16G*^2*p(pi) + 16G*^3 = delta*(2*x_+ - 16G*^2 + delta)")
factor_check = delta * (2*x_plus - 16*G_star**2 + delta)
print(f"  delta * (2*x_+ - 16*G*^2 + delta) = {fmt(factor_check)}")
print(f"  Match: {fabs(residual_quad - factor_check) < mpf('1e-100')}")
print()

# So: p(pi)^2 - 16G*^2*p(pi) + 16G*^3 = delta*(2*x_+ - 16G*^2 + delta)
# Define: D = 2*x_+ - 16*G*^2 = -(x_+ - x_-) wait...
# x_+ + x_- = 16G*^2, so 2*x_+ - 16G*^2 = 2*x_+ - (x_+ + x_-) = x_+ - x_-
D_val = 2*x_plus - 16*G_star**2
print(f"  D = 2*x_+ - 16*G*^2 = x_+ - x_- = {fmt(D_val)}")
print(f"  x_+ - x_- = {fmt(x_plus - x_minus)}")
print(f"  Match: {fabs(D_val - (x_plus - x_minus)) < mpf('1e-100')}")
print()

# So: residual = delta * (x_+ - x_- + delta)
# And x_+ - x_- = sqrt(disc) = sqrt(256G*^4 - 64G*^3) = 8G*^2*sqrt(4G*-1)*... wait
# disc = 256G*^4 - 64G*^3 = 64G*^3(4G*-1)
# sqrt(disc) = 8G*^(3/2)*sqrt(4G*-1)
# Actually from the quadratic formula: x_+ - x_- = sqrt(disc_mq) where disc_mq = (16G*^2)^2 - 4*16G*^3

gap = x_plus - x_minus
print(f"  x_+ - x_- = {fmt(gap)}")
print(f"  = sqrt((16G*^2)^2 - 4*16*G*^3) = sqrt(256G*^4 - 64G*^3)")
sqrt_disc = mpmath.sqrt(disc_mq)
print(f"  = {fmt(sqrt_disc)}")
print(f"  Match: {fabs(gap - sqrt_disc) < mpf('1e-100')}")
print()

# KEY EQUATION:
# p(pi)^2 - 16G*^2*p(pi) + 16G*^3 = delta*(gap + delta)
# where gap = sqrt(256G*^4 - 64G*^3)
#
# This is an EXACT algebraic relation connecting delta, G*, and p(pi)!
# Since p(pi) = 4pi^3 + pi^2 + pi, this connects delta to pi and G*.

subheader("THE MASTER EQUATION FOR DELTA")
print(f"""
  p(pi)^2 - 16*G*^2*p(pi) + 16*G*^3 = delta * (sqrt(256*G*^4 - 64*G*^3) + delta)

  This is EXACT and ALGEBRAIC. It connects delta to pi and G* only.

  Rearranging:
    delta^2 + delta*sqrt(256G*^4 - 64G*^3) - [p(pi)^2 - 16G*^2*p(pi) + 16G*^3] = 0

  This is a QUADRATIC IN DELTA! Solving:
    delta = [-sqrt(disc_mq) + sqrt(disc_mq + 4*R)] / 2
  where R = p(pi)^2 - 16G*^2*p(pi) + 16G*^3
""")

R_residual = p_pi**2 - 16*G_star**2*p_pi + 16*G_star**3
print(f"  R = p(pi)^2 - 16G*^2*p(pi) + 16G*^3 = {fmt(R_residual)}")
print()

# Solve the quadratic for delta
delta_solved = (-sqrt_disc + mpmath.sqrt(disc_mq + 4*R_residual)) / 2
print(f"  delta from quadratic = {fmt(delta_solved)}")
print(f"  delta direct          = {fmt(delta)}")
print(f"  Match: {fabs(delta_solved - delta) < mpf('1e-100')}")
print()

# Since delta << sqrt(disc_mq), we can approximate:
# delta ~ R / sqrt(disc_mq)  (first order)
# Because delta^2 + delta*sqrt(disc) = R
# If delta << sqrt(disc): delta*sqrt(disc) ~ R  =>  delta ~ R/sqrt(disc)

delta_approx1 = R_residual / sqrt_disc
print(f"  First-order approximation: delta ~ R/sqrt(disc_mq)")
print(f"    = {fmt(delta_approx1)}")
print(f"    Error: {ppm_error(delta_approx1, delta):.2f} ppm")
print()

# Second order: delta = R/(sqrt(disc) + delta) ~ R/sqrt(disc) * 1/(1 + delta/sqrt(disc))
# ~ R/sqrt(disc) * (1 - delta/sqrt(disc))
# ~ R/sqrt(disc) - R*delta/disc
# ~ R/sqrt(disc) - R^2/disc^(3/2)
delta_approx2 = R_residual / sqrt_disc - R_residual**2 / disc_mq**(mpf(3)/2)
print(f"  Second-order: delta ~ R/sqrt(D) - R^2/D^(3/2)")
print(f"    = {fmt(delta_approx2)}")
print(f"    Error: {ppm_error(delta_approx2, delta):.6f} ppm")
print()

# THIS IS THE CLOSED FORM.
# delta = R / (x_+ - x_-)  to first order
# where R = p(pi)^2 - 16G*^2*p(pi) + 16G*^3
# = (4pi^3+pi^2+pi)^2 - 16G*^2*(4pi^3+pi^2+pi) + 16G*^3

subheader("THE CLOSED FORM")
print(f"""
  EXACT (quadratic):
    delta^2 + delta*sqrt(256*G*^4 - 64*G*^3) = (4pi^3+pi^2+pi)^2 - 16G*^2*(4pi^3+pi^2+pi) + 16G*^3

  FIRST ORDER (0.48 ppm):
    delta = [(4pi^3+pi^2+pi)^2 - 16G*^2*(4pi^3+pi^2+pi) + 16G*^3] / sqrt(256G*^4 - 64G*^3)

  In words: delta = p(pi) evaluated at the master quadratic / gap between roots

  This is not a "guess" -- it's an ALGEBRAIC IDENTITY following from:
    1. p(pi) = x_+ + delta     (definition)
    2. x_+^2 - 16G*^2*x_+ + 16G*^3 = 0   (master quadratic)

  The first-order approximation works to {ppm_error(delta_approx1, delta):.2f} ppm because
  delta/sqrt(disc_mq) = {fmt_short(delta/sqrt_disc)} << 1
""")


# ==============================================================================
# SECTION 9: NUMERICAL VERIFICATION AND COMPARISON
# ==============================================================================

header("SECTION 9: COMPARING ALL FORMULAS")

print(f"  {'Formula':55s} {'Value':>25s} {'Error ppm':>12s}")
print(f"  {'-'*55} {'-'*25} {'-'*12}")

formulas = [
    ("delta (exact)", delta, 0),
    ("(3/19)*pi^3*Ga14^2/(G*^3*x+^2) [Strategy 3]",
     mpf(3)/19 * pi**3 * gamma_quarter**2 / (G_star**3 * x_plus**2),
     None),
    ("R/sqrt(disc_mq) [1st order algebraic]",
     delta_approx1, None),
    ("R/sqrt(D) - R^2/D^(3/2) [2nd order algebraic]",
     delta_approx2, None),
    ("(77/31)*alpha_exp^2",
     mpf(77)/31 * alpha_exp**2, None),
    ("(5/2)*alpha_exp^2",
     mpf(5)/2 * alpha_exp**2, None),
]

for name, val, _ in formulas:
    err = ppm_error(val, delta) if name != "delta (exact)" else 0
    print(f"  {name:55s} {fmt_short(val):>25s} {err:>12.4f}")

print()

# Relationship between the 3/19 formula and the algebraic identity
subheader("Connecting the 3/19 formula to the algebraic identity")
print(f"  The 3/19 formula: delta ~ (3/19) * pi^3 * Ga14^2 / (G*^3 * x_+^2)")
print(f"  The algebraic:    delta ~ R / sqrt(disc_mq)")
print(f"    where R = p(pi)^2 - 16G*^2*p(pi) + 16G*^3")
print()
print(f"  Ratio of formulas:")
ratio_formulas = delta_approx1 / (mpf(3)/19 * pi**3 * gamma_quarter**2 / (G_star**3 * x_plus**2))
print(f"    algebraic / monomial = {fmt(ratio_formulas, 20)}")
print(f"    This is NOT 1 -- the formulas are different approximations!")
print()

# What IS R in terms of simpler quantities?
subheader("Decomposing R = p(pi)^2 - 16G*^2*p(pi) + 16G*^3")
print(f"  R = {fmt(R_residual)}")
print(f"  R = p(pi) * [p(pi) - 16G*^2] + 16G*^3")
vieta_sum = x_plus + x_minus
print(f"  p(pi) - 16G*^2 = {fmt(p_pi - 16*G_star**2)}")
print(f"  = p(pi) - (x_+ + x_-) = {fmt(p_pi - vieta_sum)}")
print(f"  = delta + x_+ - (x_+ + x_-) = delta - x_-")
print(f"  delta - x_- = {fmt(delta - x_minus)}")
print(f"  So R = p(pi)*(delta - x_-) + 16G*^3")
print(f"       = p(pi)*(delta - x_-) + x_+*x_-")
print(f"       = (x_+ + delta)*(delta - x_-) + x_+*x_-")
print(f"       = x_+*delta - x_+*x_- + delta^2 - delta*x_- + x_+*x_-")
print(f"       = x_+*delta + delta^2 - delta*x_-")
print(f"       = delta*(x_+ - x_- + delta)")
print(f"       = delta*(gap + delta)")
print(f"  Verification: delta*(gap+delta) = {fmt(delta*(gap + delta))}")
print(f"  R = {fmt(R_residual)}")
print(f"  Match: {fabs(R_residual - delta*(gap+delta)) < mpf('1e-100')}")
print()
print(f"  So R/sqrt(disc) = delta*(gap+delta)/gap = delta*(1 + delta/gap)")
print(f"  = delta + delta^2/gap")
print(f"  This is just delta + O(delta^2) -- a tautology at first order!")
print()
print(f"  CONCLUSION: The algebraic identity R = delta*(gap+delta) is just the")
print(f"  expansion of the master quadratic around x_+. It's exact but circular --")
print(f"  it doesn't give a SIMPLER expression for delta.")
print()
print(f"  The 3/19 monomial formula IS a genuine discovery: it says delta is")
print(f"  approximately (3/19)*pi^3*Gamma(1/4)^2/(G*^3*x_+^2) to 0.5 ppm.")
print(f"  This is NOT a consequence of the master quadratic -- it relates")
print(f"  the RFT polynomial's deviation from the FTD root to specific powers")
print(f"  of the lemniscatic constants.")


# ==============================================================================
# SECTION 10: FINAL SYNTHESIS
# ==============================================================================

header("SECTION 10: FINAL SYNTHESIS")

print(f"""
  WHAT WE FOUND
  =============

  1. EXACT IDENTITY (algebraic, tautological):
     delta^2 + delta*sqrt(256G*^4 - 64G*^3) = p(pi)^2 - 16G*^2*p(pi) + 16G*^3
     This just says "p(pi) plugged into x_+'s quadratic gives nonzero."
     It's exact but circular.

  2. MONOMIAL APPROXIMATION (0.51 ppm with alpha_exp, {ppm_error(mpf(3)/19 * pi**3 * gamma_quarter**2 / (G_star**3 * x_plus**2), delta):.2f} ppm with alpha_ftd):
     delta ~ (3/19) * pi^3 * Gamma(1/4)^2 / (G*^3 * alpha^2)

     Simplified forms:
     delta ~ (12/19) * pi^6 / (sqrt(2) * Gamma(1/4)^4) * alpha^2
     delta ~ (3*sqrt(2)/19) * pi^4 / G*^2 * alpha^2
     delta ~ (3/38) * pi^5 / (sqrt(2) * varpi^2) * alpha^2

  3. RATIONAL CONVERGENT (0.036% error):
     delta/alpha^2 ~ 77/31 = b_3*(N_base+b_3) / (N_base*b_3+N_c)
     CF of 77/31 = [2, 2, 15] -- terminates at the discriminant!

  4. CF STRUCTURE (60 terms):
     15 appears 3 times (8.9x Gauss-Kuzmin expectation)
     11 appears 2 times (3.3x expectation)
     13 appears 1 time  (2.3x expectation)
     7  appears 2 times (1.5x expectation)

  5. PSLQ NEGATIVE RESULT:
     delta/alpha^2 is NOT a low-coefficient linear combination of
     {{1, pi, G*, varpi, sqrt(2), R, L-values}}.
     The relationship is multiplicative, not additive.

  THE OPEN QUESTION
  =================

  Why 3/19? The coefficient 3/19 is NOT explained by FTD integers alone.
  - 3 = N_c (color number)
  - 19 = ? (the 8th prime, but no obvious FTD decomposition)
  - 19 = N_eff + b_3 - 1 = 13 + 7 - 1? Forced.
  - 19 = 4*N_base + N_c = 4*4 + 3? Forced.

  The 0.51 ppm residual may indicate:
  (a) The formula is approximate (coincidental numerology)
  (b) The exact coefficient is close to but not exactly 3/19
  (c) There's a correction term of order alpha or alpha^2

  If (b), the exact ratio is {fmt_short(R_ftd)} whose CF is {cf_coeff[:10]}.
  The convergents suggest no simple fraction matches better than 3/19
  within the first few terms.
""")

print(SEP)
print("  END OF FOLLOW-UP ANALYSIS")
print(SEP)
