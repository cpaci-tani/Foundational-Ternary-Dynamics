#!/usr/bin/env python3
"""
biquadratic_bridge.py  --  Script 3 of 5 (Modular Forms Investigation)

Construct and analyze the biquadratic field Q(i, sqrt(15)) as the algebraic
structure unifying FTD (built on Q(i), disc -4) and RFT (built on Q(sqrt(-15)),
disc -15).

Covers:
  Part 1 - The three quadratic subfields
  Part 2 - Dirichlet characters and L-functions
  Part 3 - Complete factorization of 137
  Part 4 - Galois action on prime ideals
  Part 5 - Frobenius elements
  Part 6 - Products of L-values and alpha
  Part 7 - Class number product formula
"""

import sys, os
if sys.platform == 'win32':
    os.environ['PYTHONIOENCODING'] = 'utf-8'
    sys.stdout.reconfigure(encoding='utf-8')

import mpmath
mpmath.mp.dps = 50

from mpmath import mp, mpf, mpc, pi, sqrt, log, gamma, zeta, fabs, floor, power, fmod

# ==============================================================================
# Utility helpers
# ==============================================================================

def header(title):
    """Print a prominent section header."""
    bar = "=" * 78
    print(f"\n{bar}")
    print(f"  {title}")
    print(bar)


def subheader(title):
    print(f"\n--- {title} ---")


def jacobi_symbol(a, n):
    """
    Compute the Jacobi symbol (a/n) for odd positive n.
    Standard algorithm using quadratic reciprocity.
    """
    a = int(a)
    n = int(n)
    if n <= 0 or n % 2 == 0:
        raise ValueError(f"n must be odd and positive, got {n}")
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
    if n == 1:
        return result
    return 0


def kronecker_symbol(D, n):
    """
    Compute the Kronecker symbol (D/n), extending Jacobi to all integers.
    This is the Dirichlet character chi_D for fundamental discriminant D.
    """
    n = int(n)
    if n == 0:
        return 1 if abs(D) == 1 else 0

    # Handle sign of n
    result = 1
    if n < 0:
        n = -n
        if D < 0:
            result = -1

    # Factor out 2s
    v = 0
    while n % 2 == 0:
        v += 1
        n //= 2

    if v > 0:
        # (D/2) symbol
        D_mod8 = int(D) % 8
        if D_mod8 < 0:
            D_mod8 += 8
        if D_mod8 % 2 == 0:
            kr2 = 0
        elif D_mod8 in (1, 7):
            kr2 = 1
        else:  # 3, 5
            kr2 = -1
        result *= kr2 ** v
        if result == 0:
            return 0

    if n == 1:
        return result

    # Now n is odd > 1
    return result * jacobi_symbol(int(D), n)


def chi_minus4(n):
    """Dirichlet character chi_{-4}(n) = Kronecker(-4, n)."""
    return kronecker_symbol(-4, n)


def chi_minus15(n):
    """Dirichlet character chi_{-15}(n) = Kronecker(-15, n)."""
    return kronecker_symbol(-15, n)


def chi_60(n):
    """Dirichlet character chi_{60}(n) = Kronecker(60, n)."""
    return kronecker_symbol(60, n)


def L_function_partial(chi_func, s, N=100000):
    """Compute L(chi, s) = sum_{n=1}^{N} chi(n)/n^s using mpmath."""
    s = mpf(s)
    total = mpf(0)
    for n in range(1, N + 1):
        c = chi_func(n)
        if c != 0:
            total += mpf(c) / power(mpf(n), s)
    return total


def gcd(a, b):
    a, b = abs(int(a)), abs(int(b))
    while b:
        a, b = b, a % b
    return a


def euler_phi(n):
    n = int(n)
    result = 0
    for k in range(1, n + 1):
        if gcd(k, n) == 1:
            result += 1
    return result


def multiplicative_order(a, n):
    """Order of a in (Z/nZ)*."""
    a = int(a) % int(n)
    if gcd(a, n) != 1:
        return None  # not coprime
    order = 1
    current = a
    while current != 1:
        current = (current * a) % n
        order += 1
        if order > n:
            return None
    return order


# ==============================================================================
# PART 1 : The Three Quadratic Subfields
# ==============================================================================

header("PART 1: The Three Quadratic Subfields of Q(i, sqrt(15))")

print("""
The biquadratic field K = Q(i, sqrt(15)) is a degree-4 extension of Q
with Galois group V4 = Z/2 x Z/2.

It contains exactly three quadratic subfields, corresponding to the
three non-trivial elements of V4:
""")

# Subfield 1: Q(i)
print("Subfield 1: Q(i) = Q(sqrt(-1))")
print(f"  Discriminant:  -4")
print(f"  Class number:  h(-4) = 1   (principal, Z[i] is a PID)")
print(f"  Ring of integers: Z[i]  (Gaussian integers)")
print(f"  Roots of unity:  w = 4  ({{1, i, -1, -i}})")
print()

# Subfield 2: Q(sqrt(-15))
print("Subfield 2: Q(sqrt(-15))")
disc_15 = -15
print(f"  Discriminant:  -15  (since -15 = 1 mod 4)")
print(f"  Class number:  h(-15) = 2")
print(f"  Ring of integers: Z[(1+sqrt(-15))/2]")
print(f"  Roots of unity:  w = 2  ({{1, -1}})")
print(f"  Ideal classes:  [O] = principal, [p2] = non-principal")
print(f"    Principal form:    (1, 1, 4)  ->  x^2 + xy + 4y^2")
print(f"    Non-principal form: (2, 1, 2)  ->  2x^2 + xy + 2y^2")
print()

# Subfield 3: Q(sqrt(15))
print("Subfield 3: Q(sqrt(15))")
disc_60 = 60
# Fundamental unit of Q(sqrt(15))
# sqrt(15) ~ 3.873..., so look for x^2 - 15*y^2 = +/- 1
# 4^2 - 15*1^2 = 16 - 15 = 1 => fundamental unit = 4 + sqrt(15)
fund_unit = mpf(4) + sqrt(mpf(15))
fund_unit_conj = mpf(4) - sqrt(mpf(15))
print(f"  Discriminant:  60  (since 15 = 3 mod 4, disc = 4*15 = 60)")
print(f"  Class number:  h(60) = 2  (narrow class number)")
print(f"  Fundamental unit:  epsilon = 4 + sqrt(15) = {fund_unit}")
print(f"    Norm(epsilon) = (4+sqrt(15))(4-sqrt(15)) = 16 - 15 = {int(fund_unit * fund_unit_conj + mpf('0.5'))}")
print(f"  Ring of integers: Z[sqrt(15)]  (since 15 = 3 mod 4)")
print(f"  Regulator: R = log(epsilon) = {log(fund_unit)}")
print()

# Verification of field structure
subheader("Verification of biquadratic structure")
print(f"  (-1) * (-15) = {(-1)*(-15)} = 15   (product gives the third square-free part)")
print(f"  (-1) * 15   = {(-1)*15} = -15  (confirming Q(i,sqrt(15)) = Q(i,sqrt(-15)))")
print(f"  disc(-4) * disc(-15) = {(-4)*(-15)} = {60}  = disc(Q(sqrt(15)))")
print(f"  Field discriminant of K/Q: disc(K) = 60^2 = {60**2}")
print(f"  [K : Q] = 4  (biquadratic)")
print()

# Table of the three subfields
print("  +-------------------+---------+----+---+---------------------------+")
print("  | Subfield          | Disc    | h  | w | Ring of integers          |")
print("  +-------------------+---------+----+---+---------------------------+")
print("  | Q(i)              |      -4 |  1 | 4 | Z[i]                      |")
print("  | Q(sqrt(-15))      |     -15 |  2 | 2 | Z[(1+sqrt(-15))/2]        |")
print("  | Q(sqrt(15))       |      60 |  2 | - | Z[sqrt(15)]               |")
print("  +-------------------+---------+----+---+---------------------------+")


# ==============================================================================
# PART 2 : Dirichlet Characters and L-functions
# ==============================================================================

header("PART 2: Dirichlet Characters and L-functions")

# ---- chi_{-4} ----
subheader("Character chi_{-4} (Kronecker symbol for disc -4)")
print("  Period: 4")
print("  Values for n = 1..12:")
vals_4 = []
for n in range(1, 13):
    c = chi_minus4(n)
    vals_4.append(c)
    print(f"    chi_{{-4}}({n:2d}) = {c:+d}")

# ---- chi_{-15} ----
subheader("Character chi_{-15} (Kronecker symbol for disc -15)")
print("  Period: 15")
print("  Values for n = 1..30:")
vals_15 = []
for n in range(1, 31):
    c = chi_minus15(n)
    vals_15.append(c)
    print(f"    chi_{{-15}}({n:2d}) = {c:+d}")

# ---- chi_{60} ----
subheader("Character chi_{60} (Kronecker symbol for disc 60)")
print("  Period: 60")
print("  Values for n = 1..60:")
vals_60 = []
for n in range(1, 61):
    c = chi_60(n)
    vals_60.append(c)
print("  n : chi_60(n)")
for i in range(0, 60, 10):
    row = "  "
    for j in range(10):
        n = i + j + 1
        if n <= 60:
            row += f"{n:3d}:{chi_60(n):+d}  "
    print(row)

# ---- L-function computations ----
subheader("L-function values at s = 1")

# Exact values from class number formula
# L(chi_{-4}, 1) = pi/4
L_m4_exact = pi / 4
print(f"\n  L(chi_{{-4}}, 1) = pi/4 = {L_m4_exact}")

# L(chi_{-15}, 1) = 2*pi*h(-15)/(w*sqrt(15)) = 2*pi*2/(2*sqrt(15)) = 2*pi/sqrt(15)
L_m15_exact = 2 * pi / sqrt(mpf(15))
print(f"  L(chi_{{-15}}, 1) = 2*pi/sqrt(15) = {L_m15_exact}")

# Numerical verification via partial sums
print("\n  Numerical verification (N = 100000 partial sums):")
L_m4_num = L_function_partial(chi_minus4, 1, 100000)
L_m15_num = L_function_partial(chi_minus15, 1, 100000)
L_60_num = L_function_partial(chi_60, 1, 100000)

print(f"    L(chi_{{-4}}, 1)  numeric = {L_m4_num}")
print(f"    L(chi_{{-4}}, 1)  exact   = {L_m4_exact}")
print(f"    difference = {fabs(L_m4_num - L_m4_exact)}")
print()
print(f"    L(chi_{{-15}}, 1) numeric = {L_m15_num}")
print(f"    L(chi_{{-15}}, 1) exact   = {L_m15_exact}")
print(f"    difference = {fabs(L_m15_num - L_m15_exact)}")
print()

# For chi_60, compute with more terms for better convergence
L_60_num_2 = L_function_partial(chi_60, 1, 100000)
print(f"    L(chi_{{60}}, 1)  numeric = {L_60_num_2}")

# For the real character chi_60, L(chi_60, 1) = 2*h(60)*R / sqrt(60)
# where R = log(epsilon), h(60) = 2, epsilon = 4 + sqrt(15)
R_60 = log(fund_unit)
L_60_exact = 2 * 2 * R_60 / sqrt(mpf(60))
print(f"    L(chi_{{60}}, 1)  exact   = 2*h*R/sqrt(60) = {L_60_exact}")
print(f"    difference = {fabs(L_60_num_2 - L_60_exact)}")

# L-values at s = 2
subheader("L-function values at s = 2")
L_m4_s2 = L_function_partial(chi_minus4, 2, 100000)
L_m15_s2 = L_function_partial(chi_minus15, 2, 100000)
L_60_s2 = L_function_partial(chi_60, 2, 100000)
# L(chi_{-4}, 2) = Catalan's constant G = sum (-1)^n/(2n+1)^2
# Actually L(chi_{-4}, 2) = Catalan's constant = 0.9159655941772190...
print(f"  L(chi_{{-4}}, 2)  = {L_m4_s2}")
print(f"  Catalan's G    = {mpmath.catalan}")
print(f"  L(chi_{{-15}}, 2) = {L_m15_s2}")
print(f"  L(chi_{{60}}, 2)  = {L_60_s2}")


# ==============================================================================
# PART 3 : Complete Factorization of 137
# ==============================================================================

header("PART 3: Complete Factorization of 137")

# ---- In Z[i] ----
subheader("Factorization in Z[i] (Gaussian integers)")
a, b = 4, 11
norm = a**2 + b**2
print(f"  137 = (4 + 11i)(4 - 11i)")
print(f"  Verification: 4^2 + 11^2 = {a**2} + {b**2} = {norm}")
print(f"  N(4 + 11i) = {norm}")
print()
print(f"  *** Key FTD numerology: ***")
print(f"    4  = N_base (from FTD integer set)")
print(f"    11 = N_base + b_3 = 4 + 7")
print(f"    137 = N_base^2 + (N_base + b_3)^2")

# ---- In O_{-15} = Z[(1+sqrt(-15))/2] ----
subheader("Factorization in ring of integers of Q(sqrt(-15))")
print()
print("  Ring of integers: O = Z[(1+sqrt(-15))/2]  (since -15 = 1 mod 4)")
print("  Norm form for principal ideals: N(a + b*omega) = a^2 + ab + 4b^2")
print("    where omega = (1+sqrt(-15))/2")
print()

# Check: is 137 represented by x^2 + xy + 4y^2?
print("  Searching for x^2 + xy + 4y^2 = 137 ...")
found_principal = False
for x in range(-20, 21):
    for y in range(-20, 21):
        if x*x + x*y + 4*y*y == 137:
            print(f"    FOUND: x={x}, y={y} gives {x*x + x*y + 4*y*y}")
            found_principal = True
if not found_principal:
    print("    NO SOLUTION FOUND.")
    print("    => 137 is NOT a norm from principal ideals.")
    print("    => (137) factors into NON-PRINCIPAL ideal classes.")

print()
# Check: is 137 represented by 2x^2 + xy + 2y^2?
print("  Searching for 2x^2 + xy + 2y^2 = 137 ...")
found_nonprincipal = False
for x in range(-20, 21):
    for y in range(-20, 21):
        val = 2*x*x + x*y + 2*y*y
        if val == 137:
            print(f"    FOUND: x={x}, y={y} gives 2*{x}^2 + {x}*{y} + 2*{y}^2 = {val}")
            found_nonprincipal = True
if not found_nonprincipal:
    print("    NO SOLUTION FOUND for non-principal form either.")

print()
print("  Interpretation:")
print("    h(-15) = 2, so there are two ideal classes: [O] and [p].")
if found_nonprincipal and not found_principal:
    print("    Since 137 is represented by (2,1,2) but NOT by (1,1,4),")
    print("    the ideal (137) = p * p_bar where p is in the non-principal class.")
    print("    Connection: 137 lives in the non-trivial ideal class of disc -15!")
elif found_principal:
    print("    137 IS represented by the principal form (1,1,4).")
    print("    So (137) = p * p_bar with p principal.")
elif not found_principal and not found_nonprincipal:
    print("    137 is not represented by either form -- checking if 137 is inert...")
    # 137 is inert in Q(sqrt(-15)) iff chi_{-15}(137) = -1
    print(f"    chi_{{-15}}(137) = {chi_minus15(137)}")
    if chi_minus15(137) == -1:
        print("    => 137 is INERT in Q(sqrt(-15)).")
    elif chi_minus15(137) == 1:
        print("    => 137 SPLITS in Q(sqrt(-15)). Searching wider range...")
        for x in range(-50, 51):
            for y in range(-50, 51):
                if x*x + x*y + 4*y*y == 137:
                    print(f"    FOUND (wider): x={x}, y={y}")
                val2 = 2*x*x + x*y + 2*y*y
                if val2 == 137:
                    print(f"    FOUND non-principal (wider): x={x}, y={y}")

# ---- In the biquadratic field ----
subheader("Factorization in Q(i, sqrt(15)) -- the biquadratic field")

print()
print("  Step 1: Does 137 split completely in K = Q(i, sqrt(15))?")
print("  A prime p splits completely in a biquadratic Q(sqrt(d1), sqrt(d2))")
print("  iff it splits in ALL three quadratic subfields.")
print()
print(f"  chi_{{-4}}(137)  = {chi_minus4(137)}")
print(f"  chi_{{-15}}(137) = {chi_minus15(137)}")
print(f"  chi_{{60}}(137)  = {chi_60(137)}")
print()

c4 = chi_minus4(137)
c15 = chi_minus15(137)
c60 = chi_60(137)

if c4 == 1 and c15 == 1 and c60 == 1:
    print("  ALL characters evaluate to +1 at 137.")
    print("  => 137 SPLITS COMPLETELY into 4 prime ideals in K.")
elif c4 == 1 and c15 == -1:
    print("  chi_{-15}(137) = -1; 137 is INERT in Q(sqrt(-15)).")
    print("  => 137 does NOT split completely in K.")
    print("  Splitting pattern depends on which subfields see splitting.")
elif c4 == 1:
    print(f"  chi_{{-4}}(137) = 1 (splits in Q(i))")
    print(f"  chi_{{-15}}(137) = {c15}")
    print(f"  chi_{{60}}(137) = {c60}")
else:
    print(f"  chi_{{-4}}(137) = {c4}")

# Check if 15 is a QR mod 137
print()
print("  Step 2: Does sqrt(15) exist mod 137?")
# Compute 15^((137-1)/2) mod 137 = 15^68 mod 137
legendre_15_137 = pow(15, 68, 137)
sqrt15_mod137 = None
if legendre_15_137 == 1:
    print(f"  15^68 mod 137 = {legendre_15_137} => 15 IS a quadratic residue mod 137.")
    # Find sqrt(15) mod 137 by brute force (137 is small)
    for x in range(137):
        if (x * x) % 137 == 15:
            sqrt15_mod137 = x
            break
    if sqrt15_mod137 is not None:
        other = 137 - sqrt15_mod137
        print(f"  sqrt(15) mod 137 = {sqrt15_mod137}  (or {other})")
        print(f"  Verification: {sqrt15_mod137}^2 = {sqrt15_mod137**2} = {sqrt15_mod137**2 % 137} mod 137")
elif legendre_15_137 == 136:  # = -1 mod 137
    print(f"  15^68 mod 137 = {legendre_15_137} = -1 mod 137 => 15 is NOT a QR mod 137.")
else:
    print(f"  15^68 mod 137 = {legendre_15_137} (unexpected)")

print()
print("  Step 3: Detailed splitting in the biquadratic ring")
print()
print("  In Z[i]:  (137) = (4+11i)(4-11i)")
print()

# The four prime ideals in the biquadratic
if sqrt15_mod137 is not None:
    r = sqrt15_mod137
    print(f"  Since sqrt(15) = {r} mod 137, and 137 = (4+11i)(4-11i) in Z[i],")
    print(f"  each Gaussian prime further splits in Q(i, sqrt(15)):")
    print()
    print(f"  (4+11i) = P1 * P2   in O_K")
    print(f"  (4-11i) = P3 * P4   in O_K")
    print()
    print(f"  where:")
    print(f"    P1 = (4+11i, sqrt(15) - {r})")
    print(f"    P2 = (4+11i, sqrt(15) + {r})")
    print(f"    P3 = (4-11i, sqrt(15) - {r})")
    print(f"    P4 = (4-11i, sqrt(15) + {r})")
    print()
    print(f"  (137) = P1 * P2 * P3 * P4   in O_K")
    print(f"  Each P_j has norm 137^(1) in O_K (residue field F_137).")
else:
    print("  sqrt(15) does not exist mod 137; Gaussian primes do NOT further split.")
    print("  (137) = Q1 * Q2  in O_K  (two primes of degree 2)")


# ==============================================================================
# PART 4 : Galois Action
# ==============================================================================

header("PART 4: Galois Action on Prime Ideals")

print("""
  Gal(K/Q) = V4 = {{id, sigma, tau, sigma*tau}} where:
    sigma:     i -> -i,     sqrt(15) -> sqrt(15)    (fixes Q(sqrt(15)))
    tau:       i -> i,      sqrt(15) -> -sqrt(15)   (fixes Q(i))
    sigma*tau: i -> -i,     sqrt(15) -> -sqrt(15)   (fixes Q(sqrt(-15)))
""")

if c4 == 1 and c15 == 1 and c60 == 1:
    print("  137 splits completely: (137) = P1 * P2 * P3 * P4")
    print()
    print("  Action on the four prime ideals above 137:")
    print()
    print("    id:         P1 -> P1,  P2 -> P2,  P3 -> P3,  P4 -> P4")
    print()
    print("    sigma:      P1 -> P3,  P2 -> P4,  P3 -> P1,  P4 -> P2")
    print("      (swaps conjugate Z[i] primes: 4+11i <-> 4-11i)")
    print()
    print("    tau:        P1 -> P2,  P2 -> P1,  P3 -> P4,  P4 -> P3")
    print("      (swaps within each Z[i] prime: sqrt(15) <-> -sqrt(15))")
    print()
    print("    sigma*tau:  P1 -> P4,  P2 -> P3,  P3 -> P2,  P4 -> P1")
    print("      (diagonal swap)")
    print()
    print("  The three pairings correspond to the three quadratic subfields:")
    print()
    print("  +--------------------+----------------+----------------------------+")
    print("  | Fixed by           | Subfield       | Pairing of primes          |")
    print("  +--------------------+----------------+----------------------------+")
    print("  | sigma (i -> -i)    | Q(sqrt(15))    | {P1,P3} and {P2,P4}       |")
    print("  | tau (sqrt15->-sq15)| Q(i)           | {P1,P2} and {P3,P4}       |")
    print("  | sigma*tau          | Q(sqrt(-15))   | {P1,P4} and {P2,P3}       |")
    print("  +--------------------+----------------+----------------------------+")
    print()
    print("  FTD / RFT interpretation:")
    print("    FTD world (Q(i)):       sigma orbits   = {P1,P3}, {P2,P4}")
    print("    RFT world (Q(sqrt-15)): sigma*tau orbits = {P1,P4}, {P2,P3}")
    print()
    print("    The biquadratic field BRIDGES these two worlds via the tau")
    print("    automorphism, which permutes between the FTD and RFT pairings.")
    print()
    print("    Explicitly: tau * sigma = sigma*tau (since V4 is abelian)")
    print("    Composing the FTD symmetry (sigma) with the bridge (tau)")
    print("    gives the RFT symmetry (sigma*tau).")
else:
    print("  137 does NOT split completely in K.")
    print(f"  Splitting characters: chi_{{-4}}={c4}, chi_{{-15}}={c15}, chi_{{60}}={c60}")
    print()
    # Determine actual splitting pattern
    splits_in = []
    inert_in = []
    if c4 == 1:
        splits_in.append("Q(i)")
    elif c4 == -1:
        inert_in.append("Q(i)")
    if c15 == 1:
        splits_in.append("Q(sqrt(-15))")
    elif c15 == -1:
        inert_in.append("Q(sqrt(-15))")
    if c60 == 1:
        splits_in.append("Q(sqrt(15))")
    elif c60 == -1:
        inert_in.append("Q(sqrt(15))")
    print(f"  137 splits in: {', '.join(splits_in) if splits_in else 'none'}")
    print(f"  137 is inert in: {', '.join(inert_in) if inert_in else 'none'}")
    print()

    # Determine splitting in biquadratic
    num_split = sum(1 for c in [c4, c15, c60] if c == 1)
    num_inert = sum(1 for c in [c4, c15, c60] if c == -1)

    if num_split == 3:
        print("  => (137) = P1*P2*P3*P4 (4 primes of degree 1)")
    elif num_split == 1 and num_inert == 2:
        print("  => (137) = Q1*Q2 (2 primes of degree 2)")
        print("  The splitting subfield determines the pairing.")
        if c4 == 1:
            print("  Q(i) is the splitting subfield.")
            print("  Q1 lies over (4+11i), Q2 lies over (4-11i)")
            print("  Each Q_j is inert when extended to K from Q(i)")
        elif c15 == 1:
            print("  Q(sqrt(-15)) is the splitting subfield.")
        elif c60 == 1:
            print("  Q(sqrt(15)) is the splitting subfield.")
    elif num_inert == 3:
        print("  => 137 is inert in K (stays prime)")

    print()
    print("  GALOIS ACTION on the prime ideals:")
    if c4 == 1 and c15 == -1 and c60 == -1:
        print("    (137) = Q1 * Q2 in K, where Q1|Q2 are swapped by sigma")
        print("    tau and sigma*tau each fix both Q1 and Q2")
        print()
        print("  FTD/RFT interpretation:")
        print("    In Q(i):       137 = (4+11i)(4-11i) -- two principal primes")
        print("    In Q(sqrt-15): 137 remains prime -- inert!")
        print("    The non-splitting in Q(sqrt(-15)) means 137 'resists' RFT factorization.")
        print("    It splits only in the FTD subfield Q(i).")
        print()
        print("  This is PROFOUND for the bridge:")
        print("    137 = 1/alpha lives 'naturally' in the FTD world Q(i)")
        print("    but is algebraically invisible (inert) in the RFT world Q(sqrt(-15)).")
        print("    The biquadratic field Q(i,sqrt(15)) sees BOTH perspectives:")
        print("    the FTD factorization AND the RFT inertness coexist in K.")


# ==============================================================================
# PART 5 : Frobenius Elements
# ==============================================================================

header("PART 5: Frobenius Elements")

print()
if c4 == 1 and c15 == 1 and c60 == 1:
    print("  Since 137 splits completely in K = Q(i, sqrt(15)),")
    print("  Frob_137 = identity in Gal(K/Q).")
else:
    print("  137 does NOT split completely in K.")
    print("  Frob_137 is a non-trivial element of Gal(K/Q).")
    # Determine which element
    if c4 == 1 and c15 == -1:
        print("  Frob_137 = sigma*tau (the element fixing Q(i), acting as -1 on both sqrt(15) and sqrt(-15))")
    elif c4 == -1 and c15 == 1:
        print("  Frob_137 = tau (fixes Q(sqrt(-15)))")
    elif c4 == -1 and c60 == 1:
        print("  Frob_137 = sigma (fixes Q(sqrt(15)))")

print()
print("  Frobenius in cyclotomic extensions:")
print()

# 137 mod 4
r4 = 137 % 4
o4 = multiplicative_order(137, 4)
print(f"  137 mod 4  = {r4}")
print(f"    Order of 137 in (Z/4Z)* = {o4}")
if r4 == 1:
    print(f"    Frob_137 in Gal(Q(zeta_4)/Q) = identity (since 137 = 1 mod 4)")
else:
    print(f"    Frob_137 in Gal(Q(zeta_4)/Q) has order {o4}")
print(f"    Consistent with chi_{{-4}}(137) = {chi_minus4(137)}")
print()

# 137 mod 15
r15 = 137 % 15
o15 = multiplicative_order(137, 15)
print(f"  137 mod 15 = {r15}")
print(f"    Order of 137 in (Z/15Z)* = {o15}")
# Powers of 137 mod 15 = powers of 2 mod 15
print(f"    Powers of {r15} mod 15: ", end="")
val = r15
powers = [val]
for _ in range(10):
    val = (val * r15) % 15
    powers.append(val)
    if val == 1:
        break
print(" -> ".join(str(p) for p in powers))
print(f"    chi_{{-15}}(137) = {chi_minus15(137)}")
print()

# 137 mod 60
r60 = 137 % 60
o60 = multiplicative_order(137, 60)
print(f"  137 mod 60 = {r60}")
print(f"    Order of 137 in (Z/60Z)* = {o60}")
# Powers of 137 mod 60 = powers of 17 mod 60
print(f"    Powers of {r60} mod 60: ", end="")
val = r60
powers = [val]
for _ in range(15):
    val = (val * r60) % 60
    powers.append(val)
    if val == 1:
        break
print(" -> ".join(str(p) for p in powers))
print(f"    chi_{{60}}(137) = {chi_60(137)}")

print()
print("  Summary of Artin symbols:")
print("  +----------+---------------+-------+----------+")
print("  | Modulus   | 137 mod m     | Order | chi(137) |")
print("  +----------+---------------+-------+----------+")
o4_str = str(o4) if o4 else 'N/A'
o15_str = str(o15) if o15 else 'N/A'
o60_str = str(o60) if o60 else 'N/A'
print(f"  | 4        | {137%4:13d} | {o4_str:>5s} | {chi_minus4(137):+8d} |")
print(f"  | 15       | {137%15:13d} | {o15_str:>5s} | {chi_minus15(137):+8d} |")
print(f"  | 60       | {137%60:13d} | {o60_str:>5s} | {chi_60(137):+8d} |")
print("  +----------+---------------+-------+----------+")

if c4 == 1 and c15 == 1 and c60 == 1:
    print()
    print("  All Artin symbols are trivial (chi = +1), confirming")
    print("  that 137 splits completely in K = Q(i, sqrt(15)).")
else:
    print()
    print("  Not all characters are +1; 137 does not split completely.")
    print("  The non-trivial characters indicate partial splitting.")


# ==============================================================================
# PART 6 : Products of L-values and Alpha
# ==============================================================================

header("PART 6: Products of L-values and Alpha")

alpha_phys = mpf(1) / mpf('137.035999177')
inv_alpha = mpf('137.035999177')

# Use exact values where possible
L1 = L_m4_exact      # pi/4
L2 = L_m15_exact     # 2*pi/sqrt(15)
L3 = L_60_exact      # 2*h(60)*R/sqrt(60)

print(f"\n  L-function values at s = 1 (exact):")
print(f"    L1 = L(chi_{{-4}}, 1)  = pi/4            = {L1}")
print(f"    L2 = L(chi_{{-15}}, 1) = 2*pi/sqrt(15)   = {L2}")
print(f"    L3 = L(chi_{{60}}, 1)  = 4*R/sqrt(60)    = {L3}")
print()
print(f"  Reference values:")
print(f"    alpha       = {alpha_phys}")
print(f"    1/alpha     = {inv_alpha}")
print(f"    pi          = {pi}")
print(f"    sqrt(15)    = {sqrt(mpf(15))}")

subheader("Basic products")
prod_12 = L1 * L2
prod_123 = L1 * L2 * L3
prod_12_sq = L1**2 * L2**2
sum_sq = L1**2 + L2**2

print(f"  L1 * L2           = {prod_12}")
print(f"  L1 * L2 * L3      = {prod_123}")
print(f"  L1^2 * L2^2       = {prod_12_sq}")
print(f"  L1^2 + L2^2       = {sum_sq}")

# L1 * L2 = (pi/4)*(2*pi/sqrt(15)) = pi^2/(2*sqrt(15))
print(f"\n  L1 * L2 = pi^2/(2*sqrt(15)) = {pi**2 / (2*sqrt(mpf(15)))}")

subheader("Ratios with 1/alpha = 137.036...")

def check_ratio(name, value):
    """Check ratio of value to inverse alpha and flag if close to simple number."""
    ratio = value / inv_alpha
    inv_ratio = inv_alpha / value
    print(f"  {name:30s} = {value}")
    print(f"    ratio to 1/alpha = {ratio}")
    print(f"    1/alpha / value  = {inv_ratio}")
    # Check closeness to small integers and simple fractions
    for target_name, target in [("1", mpf(1)), ("2", mpf(2)), ("pi", pi),
                                  ("pi^2", pi**2), ("4*pi", 4*pi),
                                  ("pi/2", pi/2), ("2*pi", 2*pi),
                                  ("sqrt(15)", sqrt(mpf(15))),
                                  ("pi^2/6", pi**2/6)]:
        r = value / target
        if fabs(r) > mpf('1e-20'):
            nearest_int = mpmath.nint(r)
            if nearest_int != 0 and fabs(r - nearest_int) < mpf('0.01') * fabs(nearest_int):
                print(f"    *** Near {target_name} * {int(nearest_int)} ***")
    print()

check_ratio("L1*L2", prod_12)
check_ratio("L1*L2*L3", prod_123)
check_ratio("L1^2*L2^2", prod_12_sq)
check_ratio("L1^2+L2^2", sum_sq)

subheader("Systematic scan: L1^a * L2^b * L3^c vs multiples of alpha")

print()
print("  Scanning a, b, c in {-3, ..., 3} for closeness to alpha-related quantities...")
print()

alpha_targets = {
    "alpha":       alpha_phys,
    "1/alpha":     inv_alpha,
    "sqrt(alpha)": sqrt(alpha_phys),
    "alpha^2":     alpha_phys**2,
    "pi*alpha":    pi * alpha_phys,
    "pi/alpha":    pi * inv_alpha,
    "2*pi*alpha":  2 * pi * alpha_phys,
}

hits = []
for a in range(-3, 4):
    for b in range(-3, 4):
        for c in range(-3, 4):
            if a == 0 and b == 0 and c == 0:
                continue
            try:
                val = L1**a * L2**b * L3**c
            except Exception:
                continue
            if not mpmath.isfinite(val) or val <= 0:
                continue
            for tname, tval in alpha_targets.items():
                if tval <= 0:
                    continue
                ratio = val / tval
                if fabs(ratio) > mpf('1e-10'):
                    # Check if ratio is close to a simple fraction p/q with small p,q
                    for p in range(1, 20):
                        for q in range(1, 20):
                            frac = mpf(p)/mpf(q)
                            if fabs(ratio - frac) < mpf('0.01') * frac:
                                pct = float(fabs(ratio - frac) / frac) * 100
                                hits.append((pct, a, b, c, tname, p, q, float(val)))

# Sort by closeness
hits.sort(key=lambda x: x[0])

# Print top 20 hits
print(f"  Top hits (within 1% of (p/q) * target):")
print(f"  {'a':>3s} {'b':>3s} {'c':>3s}  {'target':>12s}  {'p/q':>6s}  {'value':>20s}  {'error%':>8s}")
printed = set()
count = 0
for pct, a, b, c, tname, p, q, val in hits:
    key = (a, b, c, tname)
    if key not in printed:
        printed.add(key)
        print(f"  {a:+3d} {b:+3d} {c:+3d}  {tname:>12s}  {p:d}/{q:d}    {val:20.12f}  {pct:8.4f}%")
        count += 1
        if count >= 25:
            break

subheader("FTD-RFT delta connection")
delta_ftd_rft = mpf('1.32e-4')
print(f"  FTD-RFT delta = {delta_ftd_rft}")
print()

# Various differences
diff_L = fabs(L1 - L2)
print(f"  |L1 - L2|         = {diff_L}")
print(f"  |L1 - L2| / alpha = {diff_L / alpha_phys}")
print(f"  |L1 - L2| * alpha = {diff_L * alpha_phys}")

# L3 related
print(f"\n  L3 = {L3}")
print(f"  L3 / pi = {L3 / pi}")
print(f"  L3^2    = {L3**2}")
print(f"  L3 / L1 = {L3 / L1}")
print(f"  L3 / L2 = {L3 / L2}")

# The residue of zeta_K at s=1
subheader("Residue of zeta_K(s) at s = 1")
R_60 = log(fund_unit)
residue = L1 * L2 * L3
print(f"  Res_{{s=1}} zeta_K(s) = L1 * L2 * L3 = {residue}")

# Simplify
simplified = pi**2 * R_60 / 15
print(f"  Simplified: pi^2 * R / 15 = {simplified}")
print(f"  Verification match: {fabs(residue - simplified) < mpf('1e-40')}")
print()
print(f"  Ratio: Res / (1/alpha) = {residue / inv_alpha}")
print(f"  Ratio: (1/alpha) / Res = {inv_alpha / residue}")


# ==============================================================================
# PART 7 : Class Number Product Formula
# ==============================================================================

header("PART 7: Class Number Product Formula")

h_m4 = 1
h_m15 = 2
h_60_val = 2
w_K = 4  # roots of unity in Q(i, sqrt(15)): at least {1, i, -1, -i}

print(f"\n  Class numbers of subfields:")
print(f"    h(-4)  = {h_m4}")
print(f"    h(-15) = {h_m15}")
print(f"    h(60)  = {h_60_val}")
print(f"    w(K)   = {w_K}  (roots of unity: {{1, i, -1, -i}})")
print()

# For a totally complex biquadratic: signature (r1, r2) = (0, 2)
r1 = 0
r2 = 2
dK = 3600

print(f"  Biquadratic field K = Q(i, sqrt(15)):")
print(f"    Degree [K:Q] = 4")
print(f"    Signature (r1, r2) = ({r1}, {r2})  (totally complex)")
print(f"    |disc(K)| = {dK} = 60^2")
print(f"    w(K) = {w_K}")
print()

# Analytic class number formula:
# h(K) * R(K) = w(K) * sqrt(|d(K)|) / (2^{r1} * (2*pi)^{r2}) * Res_{s=1} zeta_K(s)
#
# For r1=0, r2=2:
# h(K) * R(K) = w(K) * sqrt(|d(K)|) / (2*pi)^2 * L1*L2*L3
# = 4 * 60 / (4*pi^2) * L1*L2*L3
# = 60/pi^2 * L1*L2*L3

print(f"  Analytic class number formula:")
print(f"    h(K)*R(K) = w(K)*sqrt(|d_K|) / (2*pi)^{{r2}} * Res_{{s=1}} zeta_K")
hR = w_K * sqrt(mpf(dK)) / (2*pi)**r2 * residue
print(f"              = {w_K} * {int(sqrt(mpf(dK)))} / (2*pi)^2 * {residue}")
print(f"              = {hR}")
print(f"    4*R       = {4*R_60}")
print(f"    Match: {fabs(hR - 4*R_60) < mpf('1e-40')}")
print()

# Unit rank and regulator
print(f"  Unit rank of K: r1 + r2 - 1 = 0 + 2 - 1 = 1")
print(f"  Fundamental unit: epsilon = 4 + sqrt(15)  (from Q(sqrt(15)))")
print(f"  Regulator R(K) = log(4 + sqrt(15)) = {R_60}")
print()

h_K = hR / R_60
print(f"  => h(K) = h(K)*R(K) / R(K) = {hR} / {R_60}")
print(f"         = {h_K}")
print(f"  Rounded: h(K) = {int(float(h_K) + 0.5)}")
print()

# Cross-check with product formula
print("  Cross-check via subfield class numbers:")
prod_h = h_m4 * h_m15 * h_60_val
print(f"    h(-4) * h(-15) * h(60) = {h_m4} * {h_m15} * {h_60_val} = {prod_h}")
h_K_int = int(float(h_K) + 0.5)
if prod_h != 0 and h_K_int != 0:
    Q_unit = prod_h // h_K_int
    print(f"    h(K) = {h_K_int}")
    print(f"    Hasse unit index Q = h_sub_product / h(K) = {prod_h} / {h_K_int} = {Q_unit}")
print()

# Connection to 137
subheader("Connections between h(K) and 137")
print(f"  h(K) = {h_K_int}")
print(f"  137 mod h(K) = {137 % h_K_int}")
print(f"  137 = {h_K_int} * {137 // h_K_int} + {137 % h_K_int}")
print(f"  (137 - 1) / h(K) = {(137-1) / h_K_int}")
print()

# Final summary
header("GRAND SUMMARY: The Biquadratic Bridge")

print(f"""
  The biquadratic field K = Q(i, sqrt(15)) unifies three algebraic worlds:

  1. FTD world:  Q(i)          disc = -4,  h = 1  (Gaussian integers, Z[i] is PID)
  2. RFT world:  Q(sqrt(-15))  disc = -15, h = 2  (non-principal class carries 137)
  3. Real bridge: Q(sqrt(15))  disc = 60,  h = 2  (fundamental unit 4+sqrt(15))

  Key results:

  A. FACTORIZATION OF 137:
     In Z[i]:           137 = (4+11i)(4-11i)     [splits into principal primes]
     chi_{{-4}}(137)  = {c4}
     chi_{{-15}}(137) = {c15}
     chi_{{60}}(137)  = {c60}
""")

if c4 == 1 and c15 == 1 and c60 == 1:
    print("     In K:            137 = P1*P2*P3*P4   [splits completely!]")
elif c4 == 1 and c15 == -1:
    print("     In Q(sqrt(-15)): 137 is INERT        [does not split]")
    print("     In K:            (137) = Q1*Q2        [two primes of degree 2]")
    print()
    print("     CRUCIAL FINDING:")
    print("     137 splits in the FTD world Q(i) but is INERT in the RFT world Q(sqrt(-15)).")
    print("     This means alpha = 1/137 is algebraically 'native' to FTD")
    print("     but 'foreign' to the RFT discriminant -15 structure.")

print(f"""
  B. L-FUNCTION FACTORIZATION:
     zeta_K(s) = zeta(s) * L(-4,s) * L(-15,s) * L(60,s)

     L(chi_{{-4}},  1) = pi/4           = {float(L1):.15f}
     L(chi_{{-15}}, 1) = 2*pi/sqrt(15)  = {float(L2):.15f}
     L(chi_{{60}},  1) = 4*R/sqrt(60)   = {float(L3):.15f}

     Residue at s=1: pi^2 * log(4+sqrt(15)) / 15 = {float(residue):.15f}

  C. CLASS NUMBER:
     h(K) = {h_K_int}
     Regulator R(K) = log(4+sqrt(15)) = {float(R_60):.15f}

  D. CONNECTION TO 137:
     137 = 4^2 + 11^2     (Gaussian norm, with 4=N_base, 11=N_base+b_3)
     137 = 1 mod 4         (splits in Q(i))
     137 mod 15 = {137 % 15}
     Legendre (15/137) = {legendre_15_137}
""")

print("=" * 78)
print("  END OF BIQUADRATIC BRIDGE ANALYSIS")
print("=" * 78)
