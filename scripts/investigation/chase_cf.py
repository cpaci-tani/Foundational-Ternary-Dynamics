#!/usr/bin/env python3
"""
chase_cf.py  --  Chase the Continued Fraction: Finding a Closed Form for delta
================================================================================

The invariance base established that delta = p(pi) - x_+ is a Level 0
quantity (universally invariant under V4). The CF of delta/alpha^2 =
[2, 2, 15, 1, 11, 1, 5, 1, 1, 15, 1, 1, 1, 7, 3, ...] is packed with
FTD integers and the discriminant 15.

This script uses mpmath's PSLQ integer relation algorithm and identify()
function to find a closed form for delta in terms of the minimal base
{pi, Gamma(1/4), 3, 4, 7}.

TOOLS:
  - mpmath.pslq()    : Integer relation algorithm
  - mpmath.identify() : Symbolic constant identification
  - Continued fractions at 100-digit precision

Author: Claude Code Investigation
Date: 2026-02-07
"""

import sys, os
if sys.platform == 'win32':
    os.environ['PYTHONIOENCODING'] = 'utf-8'
    sys.stdout.reconfigure(encoding='utf-8')

import mpmath
mpmath.mp.dps = 100  # 100-digit precision for PSLQ reliability

from mpmath import mp, mpf, mpc, pi, sqrt, log, gamma, fabs, floor, power, exp

# ==============================================================================
# UTILITIES
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
    return float(abs(derived - experimental) / abs(experimental) * mpf('1e6'))

def pct_error(derived, experimental):
    return float(abs(derived - experimental) / abs(experimental) * 100)

def continued_fraction(x, n_terms=60):
    """Compute continued fraction representation to n_terms."""
    cfs = []
    for _ in range(n_terms):
        a = floor(x)
        cfs.append(int(a))
        frac = x - a
        if abs(frac) < mpf(10)**(-80):
            break
        x = 1 / frac
    return cfs

def convergents(cfs):
    """Compute all convergents p_n/q_n from CF coefficients."""
    h_prev, h_curr = mpf(0), mpf(1)
    k_prev, k_curr = mpf(1), mpf(0)
    results = []
    for a in cfs:
        h_prev, h_curr = h_curr, mpf(a) * h_curr + h_prev
        k_prev, k_curr = k_curr, mpf(a) * k_curr + k_prev
        if k_curr > 0:
            results.append((int(h_curr), int(k_curr), h_curr / k_curr))
    return results


# ==============================================================================
# SECTION 1: SETUP — Compute delta to 100 digits
# ==============================================================================

header("SECTION 1: SETUP -- 100-DIGIT PRECISION")

# --- Fundamental transcendentals ---
gamma_quarter = gamma(mpf('0.25'))
sqrt2 = sqrt(mpf(2))
sqrt3 = sqrt(mpf(3))
sqrt15 = sqrt(mpf(15))

# --- Lemniscatic constant and G* ---
varpi = gamma_quarter**2 / (2 * sqrt(2 * pi))
G_star = sqrt2 * gamma_quarter**2 / (2 * pi)

# --- Master quadratic: x^2 - 16*G*^2*x + 16*G*^3 = 0 ---
c = G_star
disc_mq = 256*c**4 - 64*c**3
x_plus = (16*c**2 + mpmath.sqrt(disc_mq)) / 2
x_minus = (16*c**2 - mpmath.sqrt(disc_mq)) / 2

# --- RFT polynomial ---
p_pi = 4*pi**3 + pi**2 + pi

# --- Gap ---
delta = p_pi - x_plus

# --- CODATA 2022 ---
alpha_inv_exp = mpf('137.035999177')
alpha_exp = 1 / alpha_inv_exp

# --- FTD framework integers ---
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13

# --- Key derived quantities ---
epsilon = mpf(4) + sqrt15
R_reg = log(epsilon)

# --- L-function values ---
L1 = pi / 4                    # L(chi_{-4}, 1)
L2 = 2 * pi / sqrt15           # L(chi_{-15}, 1)
L3 = 4 * R_reg / sqrt(mpf(60)) # L(chi_{60}, 1)

# --- Scaled quantities ---
da = delta / alpha_exp
da2 = delta / alpha_exp**2
da3 = delta / alpha_exp**3
da4 = delta / alpha_exp**4
d137 = delta * 137
d137sq = delta * 137**2

print(f"  Precision: {mpmath.mp.dps} digits")
print()
print(f"  G*           = {fmt(G_star)}")
print(f"  x_+          = {fmt(x_plus)}")
print(f"  x_-          = {fmt(x_minus)}")
print(f"  p(pi)        = {fmt(p_pi)}")
print(f"  delta        = {fmt(delta)}")
print(f"  alpha_exp    = {fmt(alpha_exp)}")
print()
print(f"  delta/alpha   = {fmt(da)}")
print(f"  delta/alpha^2 = {fmt(da2)}")
print(f"  delta/alpha^3 = {fmt(da3)}")
print(f"  delta*137     = {fmt(d137)}")
print(f"  delta*137^2   = {fmt(d137sq)}")
print()
print(f"  varpi         = {fmt(varpi)}")
print(f"  sqrt(15)      = {fmt(sqrt15)}")
print(f"  R = log(4+s15)= {fmt(R_reg)}")
print(f"  L1 = pi/4     = {fmt(L1)}")
print(f"  L2 = 2pi/s15  = {fmt(L2)}")
print(f"  L3 = 4R/s60   = {fmt(L3)}")


# ==============================================================================
# SECTION 2: EXTENDED CONTINUED FRACTION — 50+ terms
# ==============================================================================

header("SECTION 2: EXTENDED CONTINUED FRACTIONS")

subheader("CF of delta/alpha^2 (target: 50+ terms)")
cf_da2 = continued_fraction(da2, 60)
print(f"  {len(cf_da2)} terms computed:")
print(f"  {cf_da2}")
print()

# Identify FTD integers
ftd_ints = {3, 4, 7, 13}
disc_val = 15
print(f"  Appearances of FTD integers {{3,4,7,13}} and disc 15:")
for i, a in enumerate(cf_da2):
    if a in ftd_ints:
        print(f"    a[{i}] = {a}  (FTD integer)")
    if a == disc_val:
        print(f"    a[{i}] = {a}  *** DISCRIMINANT 15 ***")
    if a == 11:
        print(f"    a[{i}] = {a}  (= N_base + b_3 = 4+7)")

# Periodicity check
print()
subheader("Periodicity check")
# Check if any substring of length L repeats
for period_len in range(2, min(len(cf_da2)//2, 15)):
    chunk1 = cf_da2[:period_len]
    chunk2 = cf_da2[period_len:2*period_len]
    if chunk1 == chunk2:
        print(f"  PERIODIC with period {period_len}: {chunk1}")
        break
else:
    print(f"  No periodicity detected in first {len(cf_da2)} terms")
    print(f"  (Periodicity would indicate a quadratic irrational)")

# Compute convergents
print()
subheader("Convergents of delta/alpha^2")
convs = convergents(cf_da2)
print(f"  {'n':>3s}  {'p_n':>20s}  {'q_n':>15s}  {'p_n/q_n':>20s}  {'error %':>15s}")
for i, (p, q, val) in enumerate(convs[:20]):
    err = pct_error(val, da2)
    marker = ""
    if i < len(cf_da2):
        a = cf_da2[i]
        if a in ftd_ints or a == 15 or a == 11:
            marker = f" <-- a={a}"
    print(f"  {i:3d}  {p:>20d}  {q:>15d}  {fmt_short(val):>20s}  {err:>15.10f}{marker}")

# CFs of other delta combinations
print()
subheader("CFs of other delta combinations")

other_cfs = {
    "delta/alpha":    da,
    "delta/alpha^3":  da3,
    "delta*137":      d137,
    "delta*137^2":    d137sq,
    "delta/G*":       delta / G_star,
    "delta/varpi":    delta / varpi,
    "delta*137/pi":   d137 / pi,
    "delta*137^2/pi^2": d137sq / pi**2,
}

for name, val in other_cfs.items():
    cf = continued_fraction(val, 20)
    print(f"  {name:25s} = {fmt_short(val):>20s}  CF = {cf[:15]}")


# ==============================================================================
# SECTION 3: mpmath.identify() — Let mpmath guess
# ==============================================================================

header("SECTION 3: mpmath.identify() -- AUTOMATED IDENTIFICATION")

identify_targets = {
    "delta":            delta,
    "delta/alpha^2":    da2,
    "delta/alpha":      da,
    "delta*137":        d137,
    "delta*137^2":      d137sq,
    "delta/pi":         delta / pi,
    "delta/varpi":      delta / varpi,
    "delta/G*":         delta / G_star,
    "delta*137^2/pi^2": d137sq / pi**2,
    "delta*137/pi":     d137 / pi,
    "delta/alpha^2/pi": da2 / pi,
    "delta/alpha^3":    da3,
}

for name, val in identify_targets.items():
    # Try identify with different tolerances
    result = None
    for tol_exp in [15, 12, 10, 8]:
        try:
            result = mpmath.identify(val, tol=mpf(10)**(-tol_exp), maxcoeff=1000)
            if result:
                break
        except Exception:
            pass

    if result:
        print(f"  {name:25s} = {result}")
    else:
        print(f"  {name:25s} = [no identification found]")

# Also try identify on the 77/31 residual
eps_77_31 = da2 - mpf(77)/31
print()
print(f"  epsilon = delta/alpha^2 - 77/31:")
print(f"    epsilon = {fmt(eps_77_31)}")
result = None
for tol_exp in [15, 12, 10, 8]:
    try:
        result = mpmath.identify(eps_77_31, tol=mpf(10)**(-tol_exp), maxcoeff=1000)
        if result:
            break
    except Exception:
        pass
if result:
    print(f"    identified as: {result}")
else:
    print(f"    [no identification found]")


# ==============================================================================
# SECTION 4: PSLQ — Integer Relation Search
# ==============================================================================

header("SECTION 4: PSLQ -- INTEGER RELATION SEARCH")

def try_pslq(basis_name, labels, values, max_coeff=100, tol_exp=-30):
    """Run PSLQ on a basis and report results."""
    print(f"\n  Basis {basis_name}: [{', '.join(labels)}]")

    # Normalize: scale so all values are O(1)
    max_val = max(abs(v) for v in values)
    min_nonzero = min(abs(v) for v in values if abs(v) > 0)

    # If dynamic range is too large, normalize
    scale = 1
    if max_val / min_nonzero > 1000:
        # Don't normalize — let PSLQ handle it with varying entries
        pass

    try:
        rel = mpmath.pslq(values, tol=mpf(10)**(tol_exp), maxcoeff=10000)
    except Exception as e:
        print(f"    PSLQ failed: {e}")
        return None

    if rel is None:
        # Try with relaxed tolerance
        for relaxed in [-25, -20, -15]:
            try:
                rel = mpmath.pslq(values, tol=mpf(10)**(relaxed), maxcoeff=10000)
                if rel is not None:
                    print(f"    (Found with relaxed tol = 10^{relaxed})")
                    break
            except Exception:
                pass

    if rel is None:
        print(f"    No relation found (tol = 10^{tol_exp})")
        return None

    # Check coefficient size
    max_c = max(abs(r) for r in rel)
    if max_c > max_coeff:
        print(f"    Relation found but max |coeff| = {max_c} > {max_coeff} (likely spurious)")
        print(f"    Coefficients: {rel}")
        return rel

    print(f"    *** RELATION FOUND! ***")
    print(f"    Coefficients: {rel}")

    # Print the relation
    terms = []
    for coeff, label in zip(rel, labels):
        if coeff != 0:
            terms.append(f"({coeff})*{label}")
    print(f"    Relation: {' + '.join(terms)} = 0")

    # Verify
    check = sum(mpf(r) * v for r, v in zip(rel, values))
    print(f"    Verification: sum = {fmt(check)}")

    # Express as equation for delta (or first variable)
    if rel[0] != 0:
        print(f"    => {labels[0]} = ", end="")
        terms = []
        for coeff, label in zip(rel[1:], labels[1:]):
            if coeff != 0:
                c = -mpf(coeff) / mpf(rel[0])
                if c == int(c):
                    terms.append(f"({int(c)})*{label}")
                else:
                    terms.append(f"({-coeff}/{rel[0]})*{label}")
        print(" + ".join(terms))

    return rel

# ---- Basis A: delta vs powers of alpha and simple constants ----
try_pslq("A (delta vs alpha powers)",
    ["delta", "alpha^2", "alpha^3", "alpha^4", "pi*alpha^2", "G**alpha^2"],
    [delta, alpha_exp**2, alpha_exp**3, alpha_exp**4, pi*alpha_exp**2, G_star*alpha_exp**2])

# ---- Basis B: delta/alpha^2 vs algebraic numbers ----
try_pslq("B (delta/alpha^2 vs constants)",
    ["delta/a^2", "1", "1/137", "1/137^2", "pi/137", "G*/137"],
    [da2, mpf(1), mpf(1)/137, mpf(1)/137**2, pi/137, G_star/137])

# ---- Basis C: full minimal base ----
try_pslq("C (full minimal base)",
    ["delta", "alpha^2", "pi*a^2", "Ga14*a^2", "s2*a^2", "varpi*a^2", "G**a^2"],
    [delta, alpha_exp**2, pi*alpha_exp**2, gamma_quarter*alpha_exp**2,
     sqrt2*alpha_exp**2, varpi*alpha_exp**2, G_star*alpha_exp**2])

# ---- Basis D: delta/alpha^2 vs FTD integers & constants (normalized) ----
# Normalize all to O(1) range
try_pslq("D (FTD integers & constants)",
    ["delta/a^2", "1", "pi", "1/pi", "G*", "varpi", "sqrt(2)"],
    [da2, mpf(1), pi, 1/pi, G_star, varpi, sqrt2])

# ---- Basis E: mixing FTD and RFT with higher powers ----
try_pslq("E1 (pi powers)",
    ["delta/a^2", "1", "pi", "pi^2", "pi^3", "1/pi", "1/pi^2"],
    [da2, mpf(1), pi, pi**2, pi**3, 1/pi, 1/pi**2])

try_pslq("E2 (G* powers)",
    ["delta/a^2", "1", "G*", "G*^2", "G*^3", "1/G*", "1/G*^2"],
    [da2, mpf(1), G_star, G_star**2, G_star**3, 1/G_star, 1/G_star**2])

try_pslq("E3 (mixed pi and G*)",
    ["delta/a^2", "1", "pi", "G*", "pi*G*", "pi/G*", "G*/pi"],
    [da2, mpf(1), pi, G_star, pi*G_star, pi/G_star, G_star/pi])

# ---- Basis F: L-values ----
try_pslq("F (L-values)",
    ["delta/a^2", "1", "L1", "L2", "L3", "L1*L2", "L1*L3"],
    [da2, mpf(1), L1, L2, L3, L1*L2, L1*L3])

# ---- Basis G: regulator/period ratios ----
try_pslq("G (regulator/period ratios)",
    ["delta/a^2", "1", "R", "R/pi", "varpi/pi", "G*/pi", "R*G*"],
    [da2, mpf(1), R_reg, R_reg/pi, varpi/pi, G_star/pi, R_reg*G_star])

# ---- Additional targeted bases ----
# Basis H: delta directly vs Gamma(1/4) powers
try_pslq("H (Gamma(1/4) powers)",
    ["delta", "Ga14^2/pi", "Ga14^4/pi^2", "Ga14^2/pi^2", "alpha^2", "1"],
    [delta, gamma_quarter**2/pi, gamma_quarter**4/pi**2,
     gamma_quarter**2/pi**2, alpha_exp**2, mpf(1)])

# Basis I: delta/alpha^2 vs varpi and Gamma relations
try_pslq("I (varpi and Gamma combinations)",
    ["delta/a^2", "1", "varpi", "varpi^2", "varpi/pi", "Ga14^2/(2pi)", "varpi*sqrt2"],
    [da2, mpf(1), varpi, varpi**2, varpi/pi, gamma_quarter**2/(2*pi), varpi*sqrt2])

# Basis J: try involving sqrt(15), R, and disc-specific quantities
try_pslq("J (disc-specific: sqrt(15), R)",
    ["delta/a^2", "1", "sqrt(15)", "R", "pi/sqrt(15)", "R/sqrt(15)", "sqrt(15)/pi"],
    [da2, mpf(1), sqrt15, R_reg, pi/sqrt15, R_reg/sqrt15, sqrt15/pi])

# Basis K: powers of alpha alone
try_pslq("K (pure alpha powers)",
    ["delta", "a^2", "a^3", "a^4", "a^5", "a^6"],
    [delta, alpha_exp**2, alpha_exp**3, alpha_exp**4, alpha_exp**5, alpha_exp**6])

# Basis L: delta vs single-term products G*^a * pi^b
try_pslq("L (delta vs G*^a pi^b products)",
    ["delta", "G*^2*pi/137^2", "G*^3/137^2", "pi^3/137^2", "pi^2*G*/137^2", "1/137^2"],
    [delta, G_star**2*pi/137**2, G_star**3/137**2, pi**3/137**2, pi**2*G_star/137**2, mpf(1)/137**2])


# ==============================================================================
# SECTION 5: TARGETED CLOSED-FORM SEARCH
# ==============================================================================

header("SECTION 5: TARGETED CLOSED-FORM SEARCH")

subheader("Strategy 1: delta = (5/2)*alpha^2 + correction")
c2_from_5_2 = (delta - mpf(5)/2 * alpha_exp**2) / alpha_exp**3
print(f"  delta - (5/2)*alpha^2 = {fmt(delta - mpf(5)/2 * alpha_exp**2)}")
print(f"  c2 = (delta - (5/2)*a^2) / a^3 = {fmt(c2_from_5_2)}")
print()

# PSLQ on c2
try_pslq("5a (c2 from 5/2 expansion)",
    ["c2", "1", "pi", "G*", "varpi", "sqrt(2)", "1/pi"],
    [c2_from_5_2, mpf(1), pi, G_star, varpi, sqrt2, 1/pi])

subheader("Strategy 2: delta = (77/31)*alpha^2 + correction")
c2_from_77_31 = (delta - mpf(77)/31 * alpha_exp**2) / alpha_exp**4
print(f"  delta - (77/31)*alpha^2 = {fmt(delta - mpf(77)/31 * alpha_exp**2)}")
print(f"  c2 = (delta - (77/31)*a^2) / a^4 = {fmt(c2_from_77_31)}")
print()

try_pslq("5b (c2 from 77/31 expansion)",
    ["c2", "1", "pi", "G*", "varpi", "sqrt(2)", "1/pi"],
    [c2_from_77_31, mpf(1), pi, G_star, varpi, sqrt2, 1/pi])

subheader("Strategy 3: Systematic search delta = a * G*^p * pi^q * Gamma(1/4)^r * alpha^s")
print(f"  Searching -5 <= p,q,r,s <= 5 for integer a...")

best_match = None
best_error = float('inf')
best_params = None

for p in range(-4, 5):
    for q in range(-4, 5):
        for r in range(-4, 5):
            for s in range(0, 6):  # alpha^s with s >= 0
                if p == 0 and q == 0 and r == 0 and s == 0:
                    continue
                try:
                    trial = G_star**p * pi**q * gamma_quarter**r * alpha_exp**s
                    if abs(trial) < mpf('1e-50') or abs(trial) > mpf('1e50'):
                        continue
                    ratio = delta / trial
                    # Check if ratio is close to a small integer or simple fraction
                    for num in range(-20, 21):
                        if num == 0:
                            continue
                        for den in range(1, 21):
                            frac = mpf(num) / mpf(den)
                            if frac == 0:
                                continue
                            err = float(abs(ratio - frac) / abs(frac))
                            if err < best_error and err < 0.0001:  # better than 0.01%
                                best_error = err
                                best_match = frac
                                best_params = (num, den, p, q, r, s)
                except Exception:
                    pass

if best_params:
    num, den, p, q, r, s = best_params
    print(f"  BEST MATCH:")
    print(f"    delta = ({num}/{den}) * G*^{p} * pi^{q} * Gamma(1/4)^{r} * alpha^{s}")
    predicted = mpf(num)/den * G_star**p * pi**q * gamma_quarter**r * alpha_exp**s
    print(f"    Predicted: {fmt(predicted)}")
    print(f"    Actual:    {fmt(delta)}")
    print(f"    Error:     {pct_error(predicted, delta):.8f}%")
    print(f"    ppm:       {ppm_error(predicted, delta):.2f}")
else:
    print(f"  No match found within 0.01% with |a| <= 20, |p,q,r| <= 4, s <= 5")

# Try with more structure: a/b * G*^p * pi^q  (ignoring Gamma and alpha)
subheader("Strategy 3b: delta = (a/b) * G*^p * pi^q (no Gamma, no alpha)")
best2 = None
best2_err = float('inf')

for p in range(-5, 6):
    for q in range(-5, 6):
        if p == 0 and q == 0:
            continue
        try:
            trial = G_star**p * pi**q
            if abs(trial) < mpf('1e-50') or abs(trial) > mpf('1e50'):
                continue
            ratio = delta / trial
            for num in range(-50, 51):
                if num == 0:
                    continue
                for den in range(1, 51):
                    frac = mpf(num) / mpf(den)
                    err = float(abs(ratio - frac) / abs(frac))
                    if err < best2_err and err < 0.001:
                        best2_err = err
                        best2 = (num, den, p, q, err)
        except Exception:
            pass

if best2:
    num, den, p, q, err = best2
    predicted = mpf(num)/den * G_star**p * pi**q
    print(f"  BEST: delta ~ ({num}/{den}) * G*^{p} * pi^{q}")
    print(f"    Error: {err*100:.6f}%  ({ppm_error(predicted, delta):.2f} ppm)")
else:
    print(f"  No match found")


# ==============================================================================
# SECTION 6: THE 77/31 RESIDUAL
# ==============================================================================

header("SECTION 6: THE 77/31 RESIDUAL")

eps = da2 - mpf(77)/31
print(f"  delta/alpha^2 = {fmt(da2)}")
print(f"  77/31         = {fmt(mpf(77)/31)}")
print(f"  epsilon       = delta/alpha^2 - 77/31 = {fmt(eps)}")
print(f"  |epsilon|/|delta/alpha^2| = {pct_error(mpf(77)/31, da2):.6f}%")
print()

subheader("CF of epsilon")
cf_eps = continued_fraction(abs(eps), 30)
print(f"  CF(|epsilon|) = {cf_eps[:20]}")
print()

# Check for FTD integers in CF
print(f"  FTD integers in CF of epsilon:")
for i, a in enumerate(cf_eps[:20]):
    if a in ftd_ints or a == 15 or a == 11:
        print(f"    a[{i}] = {a}")

# PSLQ on epsilon
subheader("PSLQ on epsilon = delta/alpha^2 - 77/31")

try_pslq("6a (epsilon vs basic constants)",
    ["eps", "1", "alpha", "alpha^2", "pi", "G*", "varpi"],
    [eps, mpf(1), alpha_exp, alpha_exp**2, pi, G_star, varpi])

try_pslq("6b (epsilon vs alpha powers)",
    ["eps", "alpha", "alpha^2", "alpha^3", "alpha^4", "alpha^5"],
    [eps, alpha_exp, alpha_exp**2, alpha_exp**3, alpha_exp**4, alpha_exp**5])

try_pslq("6c (epsilon vs L-values)",
    ["eps", "1", "L1", "L2", "L3", "alpha", "alpha^2"],
    [eps, mpf(1), L1, L2, L3, alpha_exp, alpha_exp**2])

# Also check: is epsilon itself a simple alpha power?
print()
subheader("Is epsilon a simple function of alpha?")
for n in range(1, 8):
    ratio = eps / alpha_exp**n
    print(f"  epsilon/alpha^{n} = {fmt_short(ratio)}")
    # Check CF
    cf_r = continued_fraction(abs(ratio), 10)
    print(f"    CF = {cf_r[:8]}")

# Check the 5/2 residual too
subheader("Alternative: residual from 5/2")
eps2 = da2 - mpf(5)/2
print(f"  delta/alpha^2 - 5/2 = {fmt(eps2)}")
print(f"  CF = {continued_fraction(abs(eps2), 15)[:12]}")
try_pslq("6d (5/2 residual)",
    ["eps2", "1", "alpha", "pi", "G*", "varpi", "sqrt2"],
    [eps2, mpf(1), alpha_exp, pi, G_star, varpi, sqrt2])


# ==============================================================================
# SECTION 7: NUMEROLOGY CHECK — Is 77/31 significant?
# ==============================================================================

header("SECTION 7: NUMEROLOGY CHECK -- 77/31")

print(f"  77 = 7 * 11 = b_3 * (N_base + b_3)")
print(f"     = {b_3} * {N_base + b_3}")
print(f"  31 is prime")
print()

# Kronecker symbols for 31
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

print(f"  Kronecker symbols for 31:")
print(f"    chi_{{-4}}(31)  = {kronecker_symbol(-4, 31):+d}")
print(f"    chi_{{-15}}(31) = {kronecker_symbol(-15, 31):+d}")
print(f"    chi_{{60}}(31)  = {kronecker_symbol(60, 31):+d}")
print()

print(f"  31 mod 4  = {31 % 4}")
print(f"  31 mod 15 = {31 % 15}")
print(f"  31 mod 3  = {31 % 3}")
print(f"  31 mod 7  = {31 % 7}")
print(f"  31 mod 8  = {31 % 8}")
print()

# Is 31 representable by binary quadratic forms?
print(f"  Binary quadratic forms:")
# Q(i): 31 = a^2 + b^2?
for a in range(10):
    for b in range(a, 10):
        if a*a + b*b == 31:
            print(f"    31 = {a}^2 + {b}^2  => splits in Q(i) as ({a}+{b}i)({a}-{b}i)")
# Q(sqrt(-15)): 31 = a^2 + ab + 4b^2  or  2a^2 + ab + 2b^2?
for a in range(-10, 11):
    for b in range(-10, 11):
        if a*a + a*b + 4*b*b == 31:
            print(f"    31 = {a}^2 + {a}*{b} + 4*{b}^2  (principal form of disc -15)")
        if 2*a*a + a*b + 2*b*b == 31:
            print(f"    31 = 2*{a}^2 + {a}*{b} + 2*{b}^2  (non-principal form of disc -15)")
print()

# 77/31 in terms of FTD decomposition
print(f"  77/31 as a number:")
print(f"    = {float(mpf(77)/31):.10f}")
print(f"    CF of 77/31 = {continued_fraction(mpf(77)/31, 10)}")
print(f"    77/31 = 2 + 15/31 = 2 + 15/31")
print(f"    Note: 15 = disc of Q(sqrt(-15)), 31 is prime")
print()

# Check: 31 = 4*7 + 3 = 4*b_3 + N_c
print(f"  FTD decomposition of 31:")
print(f"    31 = 4*7 + 3 = N_base * b_3 + N_c")
print(f"    31 = 3*7 + 10 = N_c * b_3 + (N_c + b_3)")
print(f"    31 = 2*13 + 5 = 2*N_eff + (N_base + 1)")
print(f"    31 = 3*13 - 8 = N_c*N_eff - 2*N_base")
print()

# So 77/31 = b_3 * (N_base + b_3) / (N_base * b_3 + N_c)
print(f"  77/31 = b_3*(N_base+b_3) / (N_base*b_3 + N_c)")
print(f"        = 7*11 / (28+3)")
print(f"        = 7*(4+7) / (4*7+3)")
print(f"        = {7*11}/{4*7+3} = {float(7*11/(4*7+3)):.10f}")


# ==============================================================================
# SECTION 8: GRAND RESULTS TABLE
# ==============================================================================

header("SECTION 8: GRAND RESULTS")

subheader("Best rational approximations to delta/alpha^2")
print(f"  {'Approximation':25s} {'Value':>20s} {'Error %':>15s} {'From':>20s}")
print(f"  {'-'*25} {'-'*20} {'-'*15} {'-'*20}")

approxes = [
    ("2/1",      mpf(2),      "CF[0]"),
    ("5/2",      mpf(5)/2,    "CF[0,1]"),
    ("77/31",    mpf(77)/31,  "CF[0,1,2]"),
    ("82/33",    mpf(82)/33,  "CF[0,1,2,3]"),
]

# Add more convergents from the computed list
for p, q, val in convs[:10]:
    label = f"{p}/{q}"
    if label not in [a[0] for a in approxes]:
        approxes.append((label, val, f"convergent"))

for label, val, source in approxes:
    err = pct_error(val, da2)
    print(f"  {label:25s} {fmt_short(val):>20s} {err:>15.8f} {source:>20s}")

print()
subheader("PSLQ Results Summary")
print(f"  (See Section 4 above for details of each basis)")
print()

subheader("CF Structure Analysis")
print(f"  delta/alpha^2 CF = {cf_da2[:20]}...")
print(f"  Total terms computed: {len(cf_da2)}")
print()

# Count FTD appearances
ftd_count = sum(1 for a in cf_da2 if a in ftd_ints)
disc_count = sum(1 for a in cf_da2 if a == 15)
eleven_count = sum(1 for a in cf_da2 if a == 11)
print(f"  FTD integers {{3,4,7,13}}: {ftd_count} appearances in {len(cf_da2)} terms")
print(f"  Discriminant 15:           {disc_count} appearances")
print(f"  Value 11 (=4+7):           {eleven_count} appearances")
print()

# Statistical test: are FTD integers overrepresented?
# In a "random" CF, large partial quotients are less common (Gauss-Kuzmin)
# P(a_n = k) ~ log_2(1 + 1/(k(k+2)))
import math
print(f"  Gauss-Kuzmin expected frequencies (for comparison):")
for k in [1, 2, 3, 4, 5, 7, 11, 13, 15]:
    prob = math.log2(1 + 1/(k*(k+2)))
    expected = prob * len(cf_da2)
    actual = sum(1 for a in cf_da2 if a == k)
    ratio_str = f"{actual/expected:.1f}x" if expected > 0.01 else "N/A"
    print(f"    a_n = {k:3d}: P = {prob:.4f}, expected ~ {expected:.1f}, actual = {actual}, ratio = {ratio_str}")

print()
subheader("Key Number: 77/31")
print(f"  77/31 = b_3*(N_base+b_3) / (N_base*b_3+N_c)")
print(f"        = 7*11 / 31")
print(f"  Error vs delta/alpha^2: {pct_error(mpf(77)/31, da2):.6f}%")
print()

print(f"  Residual: delta/alpha^2 - 77/31 = {fmt(eps)}")
print(f"  This residual is O(alpha) = O(1/137)")
print()

# Check if residual is proportional to alpha
eps_over_alpha = eps / alpha_exp
print(f"  epsilon / alpha = {fmt(eps_over_alpha)}")
cf_eoa = continued_fraction(abs(eps_over_alpha), 15)
print(f"  CF of |eps/alpha| = {cf_eoa[:12]}")

# Final summary
print()
subheader("FINAL SUMMARY")
print(f"""
  TARGET:  delta = p(pi) - x_+ = {fmt_short(delta)}

  This is the gap between RFT's 4*pi^3+pi^2+pi and FTD's x_+ root.
  Both expressions approximate 1/alpha to ppm accuracy.

  delta/alpha^2 = {fmt_short(da2)}

  Best rational:  77/31 ({pct_error(mpf(77)/31, da2):.6f}% error)
  where 77 = 7*11 = b_3*(N_base+b_3)
  and   31 = N_base*b_3 + N_c

  CF structure: [{', '.join(str(a) for a in cf_da2[:20])}...]
  Contains: 15 (discriminant, x{disc_count}), 7 (b_3), 3 (N_c), 11 (=4+7)
""")

print(SEP)
print("  END OF CONTINUED FRACTION CHASE")
print(SEP)
