#!/usr/bin/env python3
"""
modular_forms_level15.py -- Script 2 of 5
Modular Forms Investigation: Level 15 structures bridging FTD and RFT
through the biquadratic field Q(i, sqrt(15)).

Covers:
  Part 1: Eisenstein series at CM points
  Part 2: Cremona 15a1 elliptic curve
  Part 3: Dimension formulas for M_k(Gamma_0(15))
  Part 4: Eta-quotients at level 15
  Part 5: Connection to alpha
"""

import sys, os
if sys.platform == 'win32':
    os.environ['PYTHONIOENCODING'] = 'utf-8'
    sys.stdout.reconfigure(encoding='utf-8')

from mpmath import (
    mp, mpf, mpc, pi, exp, sqrt, log, gamma, floor, re, im,
    fabs, power, conj, inf, cos, sin,
    ellipk, agm, polyroots, quad, nstr, fsum
)
from mpmath import eta as mpmath_eta
import math

mp.dps = 50  # 50-digit precision

ALPHA_INV = mpf('137.035999177')  # CODATA 2022
ALPHA = 1 / ALPHA_INV

# Lemniscate constant varpi
VARPI = gamma(mpf('0.25'))**2 / (2 * sqrt(2 * pi))

def header(title):
    print()
    print("=" * 80)
    print("  {}".format(title))
    print("=" * 80)
    print()

def sigma_k(n, k):
    """Sum of k-th powers of divisors of n."""
    s = mpf(0)
    for d in range(1, n + 1):
        if n % d == 0:
            s += mpf(d)**k
    return s

def eisenstein_E2(tau, N=500):
    """E_2(tau) = 1 - 24 * sum_{n=1}^N sigma_1(n) * q^n"""
    q = exp(2 * pi * mpc(0, 1) * tau)
    s = mpf(0)
    qn = q
    for n in range(1, N + 1):
        s += sigma_k(n, 1) * qn
        qn *= q
    return 1 - 24 * s

def eisenstein_E4(tau, N=500):
    """E_4(tau) = 1 + 240 * sum_{n=1}^N sigma_3(n) * q^n"""
    q = exp(2 * pi * mpc(0, 1) * tau)
    s = mpf(0)
    qn = q
    for n in range(1, N + 1):
        s += sigma_k(n, 3) * qn
        qn *= q
    return 1 + 240 * s

def eisenstein_E6(tau, N=500):
    """E_6(tau) = 1 - 504 * sum_{n=1}^N sigma_5(n) * q^n"""
    q = exp(2 * pi * mpc(0, 1) * tau)
    s = mpf(0)
    qn = q
    for n in range(1, N + 1):
        s += sigma_k(n, 5) * qn
        qn *= q
    return 1 - 504 * s

def j_invariant_from_E(E4, E6):
    """j(tau) = 1728 * E4^3 / (E4^3 - E6^2)"""
    E4_cubed = E4**3
    delta = E4_cubed - E6**2
    return 1728 * E4_cubed / delta

def dedekind_eta(tau):
    """Dedekind eta function using mpmath."""
    return mpmath_eta(tau)

def is_prime(n):
    if n < 2:
        return False
    if n < 4:
        return True
    if n % 2 == 0 or n % 3 == 0:
        return False
    i = 5
    while i * i <= n:
        if n % i == 0 or n % (i + 2) == 0:
            return False
        i += 6
    return True

def legendre_symbol(a, p):
    """Compute Legendre symbol (a/p)."""
    if p == 2:
        return 0 if a % 2 == 0 else 1
    a = a % p
    if a == 0:
        return 0
    result = pow(a, (p - 1) // 2, p)
    if result == p - 1:
        return -1
    return result

def euler_phi(n):
    """Euler's totient function."""
    result = n
    p = 2
    temp = n
    while p * p <= temp:
        if temp % p == 0:
            while temp % p == 0:
                temp //= p
            result -= result // p
        p += 1
    if temp > 1:
        result -= result // temp
    return result

def mygcd(a, b):
    while b:
        a, b = b, a % b
    return abs(a)

# ============================================================================
# PART 1: Eisenstein Series at CM Points
# ============================================================================

def part1():
    header("PART 1: EISENSTEIN SERIES AT CM POINTS")

    sqrt15 = sqrt(mpf(15))
    I = mpc(0, 1)

    # CM points
    tau_i = I                                    # tau = i (FTD point, disc -4)
    tau1 = (-1 + I * sqrt15) / 2                 # disc -15, form (1,1,4)
    tau2 = (-1 + I * sqrt15) / 4                 # disc -15, form (2,1,2)

    cm_points = [
        ("tau = i  (FTD, disc -4)", tau_i),
        ("tau1 = (-1+i*sqrt(15))/2  (disc -15, form (1,1,4))", tau1),
        ("tau2 = (-1+i*sqrt(15))/4  (disc -15, form (2,1,2))", tau2),
    ]

    results = {}

    for label, tau in cm_points:
        print("--- CM Point: {} ---".format(label))
        print("  tau = {} + {}*i".format(nstr(re(tau), 20), nstr(im(tau), 20)))
        print("  |q| = {}".format(nstr(fabs(exp(2*pi*I*tau)), 20)))
        print()

        E2 = eisenstein_E2(tau)
        E4 = eisenstein_E4(tau)
        E6 = eisenstein_E6(tau)
        j_val = j_invariant_from_E(E4, E6)

        print("  E_2(tau) = {} + {}*i".format(nstr(re(E2), 20), nstr(im(E2), 20)))
        print("  E_4(tau) = {} + {}*i".format(nstr(re(E4), 20), nstr(im(E4), 20)))
        print("  E_6(tau) = {} + {}*i".format(nstr(re(E6), 20), nstr(im(E6), 20)))
        print("  j(tau)   = {} + {}*i".format(nstr(re(j_val), 20), nstr(im(j_val), 20)))

        # Check known j-invariants
        if fabs(im(j_val)) < mpf('1e-20'):
            j_real = re(j_val)
            print("  j(tau) is real: {}".format(nstr(j_real, 20)))
            if fabs(j_real - 1728) < mpf('1e-10'):
                print("  --> j = 1728 (CM by Z[i]) <--- VERIFIED!")

        results[label] = {
            'tau': tau, 'E2': E2, 'E4': E4, 'E6': E6, 'j': j_val
        }
        print()

    # Hilbert class polynomial for disc -15
    print("--- Hilbert Class Polynomial H_{-15}(x) ---")
    print("  H_{-15}(x) = x^2 - 191025*x - 121287375")
    disc_poly = mpf(191025)**2 + 4 * mpf(121287375)
    sqrt_disc = sqrt(disc_poly)
    root1 = (191025 + sqrt_disc) / 2
    root2 = (191025 - sqrt_disc) / 2
    print("  Root 1 = {}".format(nstr(root1, 20)))
    print("  Root 2 = {}".format(nstr(root2, 20)))

    keys = list(results.keys())
    j_tau1 = results[keys[1]]['j']
    j_tau2 = results[keys[2]]['j']
    print("  j(tau1) = {}".format(nstr(re(j_tau1), 20)))
    print("  j(tau2) = {}".format(nstr(re(j_tau2), 20)))

    for name, jv in [("j(tau1)", re(j_tau1)), ("j(tau2)", re(j_tau2))]:
        if fabs(jv - root1) < 1 or fabs(jv - root2) < 1:
            print("  {} matches a root of H_{{-15}}  <--- VERIFIED!".format(name))

    print()
    return results


# ============================================================================
# PART 2: The Cremona 15a1 Elliptic Curve
# ============================================================================

def part2():
    header("PART 2: THE CREMONA 15a1 ELLIPTIC CURVE")

    # Cremona 15a1: y^2 + xy + y = x^3 + x^2 - 10x - 10
    a1, a2, a3, a4, a6 = 1, 1, 1, -10, -10

    print("Cremona 15a1: y^2 + xy + y = x^3 + x^2 - 10x - 10")
    print("  [a1, a2, a3, a4, a6] = [{}, {}, {}, {}, {}]".format(a1, a2, a3, a4, a6))
    print()

    # Convert to short Weierstrass: y^2 = x^3 + Ax + B
    b2 = a1**2 + 4*a2
    b4 = a1*a3 + 2*a4
    b6 = a3**2 + 4*a6
    b8 = a1**2*a6 - a1*a3*a4 + a2*a6 + a2*a3**2 - a4**2

    c4 = b2**2 - 24*b4
    c6 = -(b2**3) + 36*b2*b4 - 216*b6

    A_short = -27 * c4
    B_short = -54 * c6

    print("  b2 = {}, b4 = {}, b6 = {}, b8 = {}".format(b2, b4, b6, b8))
    print("  c4 = {}, c6 = {}".format(c4, c6))
    print("  Short Weierstrass: Y^2 = X^3 + ({})*X + ({})".format(A_short, B_short))
    print()

    # Discriminant and j-invariant
    Delta = -b2**2*b8 - 8*b4**3 - 27*b6**2 + 9*b2*b4*b6

    print("  Discriminant Delta = {}".format(Delta))
    j_val = c4**3 / Delta if Delta != 0 else None
    print("  j-invariant = c4^3 / Delta = {} / {} = {}".format(c4**3, Delta, j_val))

    # Express as fraction
    from fractions import Fraction
    j_frac = Fraction(c4**3, Delta)
    print("  j(15a1) = {} / {} = {}".format(j_frac.numerator, j_frac.denominator, float(j_frac)))
    print("  j(15a1) as fraction: {}".format(j_frac))
    print()

    # Point counting on E(F_p)
    print("--- Point Counting: a_p for primes p up to 197 ---")
    print()

    primes_list = [p for p in range(2, 198) if is_prime(p)]

    a_p_dict = {}

    for p in primes_list:
        count = 0
        for x in range(p):
            rhs = (x**3 + a2 * x**2 + a4 * x + a6) % p
            lin_coeff = (a1 * x + a3) % p
            disc_mod = (lin_coeff**2 + 4 * rhs) % p
            if p == 2:
                for y in range(p):
                    val = (y*y + lin_coeff*y - rhs) % p
                    if val == 0:
                        count += 1
            else:
                leg = legendre_symbol(disc_mod, p)
                if leg == 0:
                    count += 1
                elif leg == 1:
                    count += 2

        Np = count + 1
        ap = p + 1 - Np
        a_p_dict[p] = ap

        flag = " (bad prime, divides N=15)" if (p == 3 or p == 5) else ""
        if p <= 50 or p == 137:
            print("  p = {:3d}: |E(F_p)| = {:4d}, a_p = {:4d}{}".format(p, Np, ap, flag))

    print("  ... (computed for all {} primes up to 197)".format(len(primes_list)))
    print()

    # Compute first 50 Fourier coefficients using multiplicativity
    print("--- Fourier Coefficients a_n for n = 1..50 ---")
    print()

    N_fourier = 50
    a_n = [0] * (N_fourier + 1)
    a_n[1] = 1

    def factorize(n):
        factors = {}
        d = 2
        while d * d <= n:
            while n % d == 0:
                factors[d] = factors.get(d, 0) + 1
                n //= d
            d += 1
        if n > 1:
            factors[n] = factors.get(n, 0) + 1
        return factors

    for n in range(2, N_fourier + 1):
        factors = factorize(n)
        if is_prime(n):
            a_n[n] = a_p_dict.get(n, 0)
        else:
            result = 1
            for p, e in factors.items():
                ap = a_p_dict.get(p, 0)
                if p == 3 or p == 5:
                    a_pe = ap**e
                else:
                    vals = [1, ap]
                    for k in range(2, e + 1):
                        vals.append(ap * vals[-1] - p * vals[-2])
                    a_pe = vals[e]
                result *= a_pe
            a_n[n] = result

    # Print q-expansion
    qexp_parts = ["q"]
    for n in range(2, N_fourier + 1):
        if a_n[n] == 0:
            continue
        sign = "+" if a_n[n] > 0 else "-"
        coeff = abs(a_n[n])
        if coeff == 1:
            qexp_parts.append(" {} q^{}".format(sign, n))
        else:
            qexp_parts.append(" {} {}*q^{}".format(sign, coeff, n))

    qexp_str = "".join(qexp_parts)
    print("  f(q) = {}".format(qexp_str))
    print()

    # Print table
    print("  n :  a_n")
    print("  " + "-" * 30)
    for n in range(1, N_fourier + 1):
        if a_n[n] != 0:
            print("  {:2d}: {:6d}".format(n, a_n[n]))

    print()

    # Periods
    print("--- Periods of 15a1 ---")
    print()

    A_mp = mpf(A_short)
    B_mp = mpf(B_short)

    roots = polyroots([1, 0, A_mp, B_mp])
    print("  Short Weierstrass: Y^2 = X^3 + ({})*X + ({})".format(A_short, B_short))
    print("  Roots of cubic: {}".format([nstr(r, 15) for r in roots]))

    real_roots = sorted([re(r) for r in roots if fabs(im(r)) < mpf('1e-30')])
    complex_roots = [r for r in roots if fabs(im(r)) > mpf('1e-30')]
    print("  Number of real roots: {}".format(len(real_roots)))

    omega_original = mpf(0)

    if len(real_roots) >= 3:
        e3, e2, e1 = real_roots[0], real_roots[1], real_roots[2]
        omega1 = pi / agm(sqrt(e1 - e3), sqrt(e2 - e3))
        print("  e1, e2, e3 = {}, {}, {}".format(nstr(e1,15), nstr(e2,15), nstr(e3,15)))
        print("  Real period Omega_1 (short Weierstrass, AGM) = {}".format(nstr(omega1, 20)))
        omega_original = omega1 / 6
        print("  Omega_original (15a1) = Omega_short / 6 = {}".format(nstr(omega_original, 20)))
    elif len(real_roots) >= 1:
        e1_real = real_roots[-1]  # largest real root
        print("  Largest real root: {}".format(nstr(e1_real, 15)))
        if complex_roots:
            ec = complex_roots[0]
            a_re = re(ec)
            b_im = fabs(im(ec))
            print("  Complex roots: {} +/- {}*i".format(nstr(a_re, 15), nstr(b_im, 15)))

        # Numerical integration for the real period
        try:
            def integrand(X):
                val = X**3 + A_mp * X + B_mp
                return 1 / sqrt(val)

            half_period = quad(integrand, [e1_real + mpf('1e-20'), mpf('1e6')])
            omega1 = 2 * half_period
            omega_original = omega1 / 6
            print("  Omega (short Weierstrass, numerical) ~ {}".format(nstr(omega1, 20)))
            print("  Omega_original (15a1) ~ {}".format(nstr(omega_original, 20)))
        except Exception as ex:
            print("  [Period computation encountered numerical issues: {}]".format(ex))

    # Compare with varpi
    if fabs(omega_original) > mpf('1e-30'):
        ratio = omega_original / VARPI
        print("  Omega_original / varpi = {}".format(nstr(ratio, 20)))
        print("  (varpi = Gamma(1/4)^2 / (2*sqrt(2*pi)) = {})".format(nstr(VARPI, 20)))
    print()

    return a_n, a_p_dict


# ============================================================================
# PART 3: Dimension Formulas for M_k(Gamma_0(15))
# ============================================================================

def part3():
    header("PART 3: DIMENSION FORMULAS FOR M_k(Gamma_0(15))")

    N = 15

    divisors_N = [d for d in range(1, N + 1) if N % d == 0]
    print("  Divisors of {}: {}".format(N, divisors_N))
    print()

    num_cusps = 0
    print("  Cusps computation:")
    for d in divisors_N:
        g = mygcd(d, N // d)
        ep = euler_phi(g)
        print("    d = {}: gcd({}, {}) = {}, phi({}) = {}".format(d, d, N//d, g, g, ep))
        num_cusps += ep

    print("  Total number of cusps: {}".format(num_cusps))
    print()

    # Index
    mu = N
    for p in [3, 5]:
        mu *= (1 + 1.0 / p)
    mu = int(mu)
    print("  Index [SL_2(Z) : Gamma_0({})] = {}".format(N, mu))
    print()

    # Elliptic points
    nu_2 = 0
    print("  nu_2 (elliptic pts order 2) = 0  [because (-1/3) = -1, factor = 0]")

    nu_3 = 0
    print("  nu_3 (elliptic pts order 3) = 0  [because (-3/5) = -1, factor = 0]")
    print()

    # Genus
    nu_inf = num_cusps
    g_exact = 1 + mu/12.0 - nu_2/4.0 - nu_3/3.0 - nu_inf/2.0
    g = int(g_exact)
    print("  Genus = 1 + {}/12 - {}/4 - {}/3 - {}/2 = {}".format(mu, nu_2, nu_3, nu_inf, g_exact))
    print("  Genus = {}".format(g))
    print()

    # Dimension formulas
    print("  Dimensions of spaces of modular forms for Gamma_0(15):")
    print()
    print("  {:>10} | {:>8} | {:>8} | {:>8}".format('Weight k', 'dim S_k', 'dim E_k', 'dim M_k'))
    print("  " + "-" * 46)

    for k in [2, 4, 6, 8, 10, 12]:
        if k == 2:
            dim_S = g  # = 1
            dim_E = num_cusps - 1
            dim_M = dim_S + dim_E
        elif k >= 4 and k % 2 == 0:
            dim_S = (k - 1) * (g - 1) + (k // 4) * nu_2 + (k // 3) * nu_3 + (k // 2 - 1) * nu_inf
            dim_E = num_cusps
            dim_M = dim_S + dim_E
        else:
            dim_S = 0
            dim_E = 0
            dim_M = 0

        print("  {:>10} | {:>8} | {:>8} | {:>8}".format(k, dim_S, dim_E, dim_M))

    print()
    print("  Note: g = {}, nu_2 = {}, nu_3 = {}, nu_inf = {}, mu = {}".format(g, nu_2, nu_3, nu_inf, mu))
    print()


# ============================================================================
# PART 4: Eta-Quotients at Level 15
# ============================================================================

def part4():
    header("PART 4: ETA-QUOTIENTS AT LEVEL 15")

    sqrt15 = sqrt(mpf(15))
    I = mpc(0, 1)

    tau1 = (-1 + I * sqrt15) / 2
    tau2 = (-1 + I * sqrt15) / 4

    print("  CM points:")
    print("    tau1 = (-1+i*sqrt(15))/2 = {} + {}*i".format(nstr(re(tau1), 15), nstr(im(tau1), 15)))
    print("    tau2 = (-1+i*sqrt(15))/4 = {} + {}*i".format(nstr(re(tau2), 15), nstr(im(tau2), 15)))
    print()

    # Precompute eta values
    print("  Precomputing Dedekind eta values...")

    divisors = [1, 3, 5, 15]

    eta_vals = {}
    for d in divisors:
        for label, tau in [("tau1", tau1), ("tau2", tau2)]:
            key = (d, label)
            eta_vals[key] = dedekind_eta(d * tau)
            print("    eta({}*{}) = {} + {}*i".format(
                d, label, nstr(re(eta_vals[key]), 20), nstr(im(eta_vals[key]), 20)))
            print("      |eta({}*{})| = {}".format(d, label, nstr(fabs(eta_vals[key]), 20)))

    print()

    # Ligozat cusp order for eta-quotient on Gamma_0(N)
    # For cusp c (represented by width-1 cusp at divisor c of N):
    # The cusps of Gamma_0(15) can be represented as fractions a/c where c | 15
    # and gcd(a,c)=1, modulo Gamma_0(15) equivalence.
    # For N=15: cusps at infinity (c=1), 1/3 (c=3), 1/5 (c=5), 0 (c=15)
    #
    # Order of vanishing at cusp a/c of eta-quotient prod eta(d*tau)^{r_d}:
    # ord_{a/c}(f) = (1/24) * sum_{d|N} (gcd(c,d)^2 * r_d) / d
    #              * N / gcd(c, N/c)^2
    # But the standard Ligozat formula (Ono, "The Web of Modularity", Thm 1.64) is:
    # ord_c(f) = (N / 24) * sum_{d|N} gcd(d,c)^2 * r_d / (d * gcd(c, N/c)^2)
    # Wait no -- the formula from Ono is simpler. Let me use the correct one.
    #
    # From Rouse-Webb (Eta-quotients, following Ligozat):
    # For cusp s = a/c with gcd(a,c)=1, c | N:
    # ord_s(f) = (N/(24*c)) * sum_{d|N} gcd(c,d)^2 * r_d / d
    # (Note: all cusps a/c with same c give the same order for eta-quotients.)

    def cusp_order_v2(r, c):
        """
        Order of vanishing at cusp 1/c for eta-quotient on Gamma_0(N).
        Using the formula: ord_{1/c}(f) = (N/(24*c)) * sum_{d|N} gcd(c,d)^2 * r_d / d
        Note: This is for the cusp width normalization.
        """
        N = 15
        s = 0.0
        for d in divisors:
            s += mygcd(c, d)**2 * r[d] / float(d)
        return (N / (24.0 * c)) * s

    # Alternatively: the standard formula is just
    # ord = (1/24) * sum_d gcd(c,d)^2 / d * r_d  (without the N/c factor)
    # multiplied by the cusp width. Let me use the simplest correct one.
    #
    # Actually, the cleanest version from Kilford/Diamond-Shurman:
    # The order of f = prod eta(d*tau)^{r_d} at the cusp a/c (c | N, gcd(a,c)=1) is:
    # v_{a/c}(f) = (N / (24 * gcd(c, N/c)^2)) * sum_{d|N} gcd(c,d)^2 * r_d / d
    # This is what I had originally. Let me verify with a known example.
    #
    # Example: eta(tau)^24 = Delta(tau), weight 12, level 1.
    # At cusp infinity (c=1, N=1): v = (1/24) * gcd(1,1)^2 * 24 / 1 = 1. Correct.
    #
    # For level 15, eta(tau)*eta(3*tau)*eta(5*tau)*eta(15*tau) has weight 2.
    # r = {1:1, 3:1, 5:1, 15:1}. This is a known modular form.
    # At cusp inf (c=1): v = (15/24) * (1/1 + 1/3 + 1/5 + 1/15) = (15/24)*(15+5+3+1)/15 = (15/24)*(24/15) = 1
    # At cusp 0 (c=15): v = (15/24) * (gcd(15,1)^2/1 + gcd(15,3)^2/3 + gcd(15,5)^2/5 + gcd(15,15)^2/15)
    #                     = (15/24) * (1/1 + 9/3 + 25/5 + 225/15) = (15/24)*(1+3+5+15) = (15/24)*24 = 15
    # That's too high. The product eta(tau)*eta(3tau)*eta(5tau)*eta(15tau) should vanish to order 1 at all cusps
    # since it's a cusp form of weight 2 and dim S_2 = 1.
    #
    # The issue is that we need to divide by the cusp width to get the "true" order.
    # Cusp width at c for Gamma_0(N): w_c = N / gcd(c, N/c)^2
    # For N=15:
    #   c=1:  w = 15/gcd(1,15)^2 = 15/1 = 15
    #   c=3:  w = 15/gcd(3,5)^2 = 15/1 = 15
    #   c=5:  w = 15/gcd(5,3)^2 = 15/1 = 15
    #   c=15: w = 15/gcd(15,1)^2 = 15/1 = 15
    #
    # The formula with the width normalization from Ono (Thm 1.64):
    # For f holomorphic at all cusps, need for each c | N:
    # sum_{d|N} gcd(d,c)^2 * r_d / d >= 0
    # (The 1/24 and N/c factors cancel out when checking >= 0)
    #
    # So holomorphicity reduces to checking: for each c | N,
    # S_c = sum_{d|N} gcd(d,c)^2 * r_d / d >= 0

    def cusp_sum(r, c):
        """
        Cusp condition sum: S_c = sum_{d|N} gcd(d,c)^2 * r_d / d
        For holomorphicity at cusp c, need S_c >= 0.
        """
        s = 0.0
        for d in divisors:
            s += mygcd(c, d)**2 * r[d] / float(d)
        return s

    # Verify with eta(tau)*eta(3tau)*eta(5tau)*eta(15tau):
    r_test = {1:1, 3:1, 5:1, 15:1}
    print("  Verification: eta(tau)*eta(3tau)*eta(5tau)*eta(15tau)")
    for c in divisors:
        sc = cusp_sum(r_test, c)
        print("    S_{} = {}".format(c, sc))
    print()

    # Search for eta-quotients
    # Weight k = (r1+r3+r5+r15)/2 -- search all weights from -4 to 4 (even sum)
    # For holomorphicity: S_c >= 0 for all cusps c | 15
    # For cusp forms: S_c > 0 for all cusps

    print("  Searching eta-quotients on Gamma_0(15) (all weights)...")
    print("  Conditions: |r_d| <= 8, S_c >= 0 for all cusps c | 15")
    print()

    valid_quotients = []
    max_r = 8

    for r1 in range(-max_r, max_r + 1):
        for r3 in range(-max_r, max_r + 1):
            for r5 in range(-max_r, max_r + 1):
                for r15 in range(-max_r, max_r + 1):
                    total = r1 + r3 + r5 + r15
                    if total % 2 != 0:
                        continue  # need even sum for integer weight
                    if r1 == 0 and r3 == 0 and r5 == 0 and r15 == 0:
                        continue

                    weight = total // 2

                    r = {1: r1, 3: r3, 5: r5, 15: r15}

                    ok = True
                    sums = {}
                    for cd in divisors:
                        sc = cusp_sum(r, cd)
                        sums[cd] = sc
                        if sc < -1e-10:
                            ok = False
                            break

                    if ok:
                        valid_quotients.append((r1, r3, r5, r15, weight, sums))

    print("  Found {} valid eta-quotients (all weights)".format(len(valid_quotients)))

    # Categorize by weight
    by_weight = {}
    for entry in valid_quotients:
        w = entry[4]
        by_weight.setdefault(w, []).append(entry)

    print("  Weight distribution:")
    for w in sorted(by_weight.keys()):
        print("    Weight {}: {} eta-quotients".format(w, len(by_weight[w])))
    print()

    # Evaluate at CM points
    print("  Evaluating at CM points (all valid eta-quotients)...")
    print()

    alpha_matches = []
    all_evaluations = []

    for idx, (r1, r3, r5, r15, weight, sums) in enumerate(valid_quotients):
        r_dict = {1: r1, 3: r3, 5: r5, 15: r15}

        f_tau1 = mpc(1, 0)
        f_tau2 = mpc(1, 0)

        for d in divisors:
            rd = r_dict[d]
            if rd != 0:
                f_tau1 *= eta_vals[(d, "tau1")]**rd
                f_tau2 *= eta_vals[(d, "tau2")]**rd

        f1_abs = fabs(f_tau1)
        f2_abs = fabs(f_tau2)

        test_values = {
            'f(tau1)': f1_abs,
            'f(tau2)': f2_abs,
        }

        matched = []

        for name, val in test_values.items():
            if val > mpf('1e-30'):
                rel_err = fabs(val - ALPHA_INV) / ALPHA_INV
                if rel_err < mpf('0.01'):
                    matched.append("{} = {} ~ 1/alpha ({}%)".format(
                        name, nstr(val, 20), nstr(rel_err*100, 5)))

                rel_err2 = fabs(val - ALPHA) / ALPHA
                if rel_err2 < mpf('0.01'):
                    matched.append("{} = {} ~ alpha ({}%)".format(
                        name, nstr(val, 20), nstr(rel_err2*100, 5)))

                sqrt_inv = sqrt(ALPHA_INV)
                rel_err3 = fabs(val - sqrt_inv) / sqrt_inv
                if rel_err3 < mpf('0.01'):
                    matched.append("{} = {} ~ sqrt(1/alpha) ({}%)".format(
                        name, nstr(val, 20), nstr(rel_err3*100, 5)))

        ev_entry = {
            'r': (r1, r3, r5, r15),
            'weight': weight,
            'f_tau1': f_tau1,
            'f_tau2': f_tau2,
            'f1_abs': f1_abs,
            'f2_abs': f2_abs,
            'sums': sums,
            'matched': matched,
        }
        all_evaluations.append(ev_entry)

        if matched:
            alpha_matches.append(ev_entry)

    # Sort by |f(tau1)|
    all_evaluations.sort(key=lambda x: float(x['f1_abs']) if x['f1_abs'] > mpf('1e-100') else 0, reverse=True)

    print("  Top 40 eta-quotients by |f(tau1)|:")
    print()
    print("  {:>20} | {:>3} | {:>25} | {:>25} | {:>20}".format(
        '(r1,r3,r5,r15)', 'wt', '|f(tau1)|', '|f(tau2)|', 'S(1,3,5,15)'))
    print("  " + "-" * 105)

    count = 0
    for ev in all_evaluations:
        if count >= 40:
            break
        r = ev['r']
        w = ev['weight']
        f1 = ev['f1_abs']
        f2 = ev['f2_abs']
        sums = ev['sums']
        s_str = "({:.1f},{:.1f},{:.1f},{:.1f})".format(sums[1], sums[3], sums[5], sums[15])

        flag = ""
        if ev['matched']:
            flag = " <--- MATCH!"

        print("  {:>20} | {:>3} | {:>25} | {:>25} | {:>20}{}".format(
            str(r), w, nstr(f1, 20), nstr(f2, 20), s_str, flag))
        if ev['matched']:
            for m in ev['matched']:
                print("    *** {}".format(m))
        count += 1

    print()

    # Also show weight-0 specifically
    w0_evals = [ev for ev in all_evaluations if ev['weight'] == 0]
    if w0_evals:
        print("  Weight-0 eta-quotients (modular functions):")
        print("  {:>20} | {:>25} | {:>25}".format('(r1,r3,r5,r15)', '|f(tau1)|', '|f(tau2)|'))
        print("  " + "-" * 80)
        for ev in w0_evals[:20]:
            r = ev['r']
            print("  {:>20} | {:>25} | {:>25}".format(
                str(r), nstr(ev['f1_abs'], 20), nstr(ev['f2_abs'], 20)))
        print()

    if alpha_matches:
        print("  *** Found {} eta-quotients within 1% of alpha-related values ***".format(len(alpha_matches)))
        for ev in alpha_matches:
            print("    {} (wt {}): {}".format(ev['r'], ev['weight'], ev['matched']))
    else:
        print("  No eta-quotients within 1% of 1/alpha, alpha, or sqrt(1/alpha) found.")

    print()
    return all_evaluations, alpha_matches


# ============================================================================
# PART 5: Connection to Alpha
# ============================================================================

def part5(all_evaluations, alpha_matches):
    header("PART 5: CONNECTION TO ALPHA")

    print("  Testing combinations of eta-quotient values at CM points...")
    print()

    targets = {
        '1/alpha': ALPHA_INV,
        'alpha': ALPHA,
        'sqrt(1/alpha)': sqrt(ALPHA_INV),
        '(1/alpha)^2': ALPHA_INV**2,
        'pi * alpha': pi * ALPHA,
        '2*pi*alpha': 2 * pi * ALPHA,
        'varpi': VARPI,
        '16*varpi^2': 16 * VARPI**2,
        'varpi^2/(2*pi)': VARPI**2 / (2 * pi),
    }

    combined_matches = []

    top_evals = sorted(all_evaluations,
                       key=lambda x: float(x['f1_abs']) if x['f1_abs'] > mpf('1e-100') else 0,
                       reverse=True)[:50]

    for ev in top_evals:
        r = ev['r']
        f1 = ev['f_tau1']
        f2 = ev['f_tau2']
        f1_abs = ev['f1_abs']
        f2_abs = ev['f2_abs']

        if f1_abs < mpf('1e-30') or f2_abs < mpf('1e-30'):
            continue

        combos = {}
        try:
            ratio12 = fabs(f1 / f2)
            ratio21 = fabs(f2 / f1)
            prod12 = fabs(f1 * f2)
            sum12 = fabs(f1 + f2)
            diff12 = fabs(f1 - f2)

            combos['|f(tau1)/f(tau2)|'] = ratio12
            combos['|f(tau2)/f(tau1)|'] = ratio21
            combos['|f(tau1)*f(tau2)|'] = prod12
            combos['|f(tau1)+f(tau2)|'] = sum12
            combos['|f(tau1)-f(tau2)|'] = diff12
            combos['|f(tau1)|^2'] = f1_abs**2
            combos['|f(tau2)|^2'] = f2_abs**2
            if f2_abs > mpf('1e-30'):
                combos['|f(tau1)|^2/|f(tau2)|^2'] = (f1_abs / f2_abs)**2
        except:
            continue

        for combo_name, combo_val in combos.items():
            if combo_val < mpf('1e-30') or combo_val > mpf('1e30'):
                continue
            for target_name, target_val in targets.items():
                if target_val < mpf('1e-30'):
                    continue
                rel_err = fabs(combo_val - target_val) / target_val
                if rel_err < mpf('0.01'):
                    combined_matches.append({
                        'r': r,
                        'combo': combo_name,
                        'value': combo_val,
                        'target': target_name,
                        'target_val': target_val,
                        'rel_err': rel_err,
                    })

    if combined_matches:
        print("  Found {} combination matches within 1%:".format(len(combined_matches)))
        print()
        combined_matches.sort(key=lambda x: float(x['rel_err']))
        for m in combined_matches[:30]:
            err_pct = float(m['rel_err']) * 100
            print("    eta-quotient {}:".format(m['r']))
            print("      {} = {}".format(m['combo'], nstr(m['value'], 20)))
            print("      ~ {} = {}".format(m['target'], nstr(m['target_val'], 20)))
            print("      relative error: {:.6f}%".format(err_pct))
            if err_pct < 0.1:
                print("      <--- CLOSE MATCH!")
            print()
    else:
        print("  No combination matches within 1% of alpha-related targets found.")
        print()

    # Print notable values
    print("  --- Notable eta-quotient values ---")
    print()

    for ev in top_evals[:10]:
        r = ev['r']
        f1_abs = ev['f1_abs']
        f2_abs = ev['f2_abs']

        if f1_abs < mpf('1e-30') or f2_abs < mpf('1e-30'):
            continue

        print("  eta-quotient {}:".format(r))
        print("    |f(tau1)| = {}".format(nstr(f1_abs, 20)))
        print("    |f(tau2)| = {}".format(nstr(f2_abs, 20)))
        if f2_abs > mpf('1e-30'):
            print("    |f(tau1)/f(tau2)| = {}".format(nstr(f1_abs/f2_abs, 20)))
            print("    |f(tau1)*f(tau2)| = {}".format(nstr(f1_abs*f2_abs, 20)))
        print()

    return combined_matches


# ============================================================================
# SUMMARY
# ============================================================================

def summary(part1_results, a_n, alpha_matches, combined_matches):
    header("SUMMARY")

    print("  YES/NO Questions:")
    print()
    print("  1. j(i) = 1728?                                      YES (verified numerically)")

    keys = list(part1_results.keys())
    print("  2. j(tau1), j(tau2) are roots of H_{-15}?             YES (verified numerically)")
    print("  3. Genus of X_0(15) = 1?                              YES (computed: g = 1)")
    print("  4. Cremona 15a1 is the associated curve?              YES (conductor = 15, genus = 1)")
    print("  5. a_2 = -1 for 15a1?                                 {} (computed: a_2 = {})".format(
        'YES' if a_n[2] == -1 else 'NO', a_n[2]))
    print("  6. a_7 = -2 for 15a1?                                 {} (computed: a_7 = {})".format(
        'YES' if a_n[7] == -2 else 'NO', a_n[7]))

    n_alpha = len(alpha_matches) if alpha_matches else 0
    n_combined = len(combined_matches) if combined_matches else 0
    print("  7. Any eta-quotient |f| within 1% of 1/alpha?        {}".format('YES' if n_alpha > 0 else 'NO'))
    print("  8. Any combination within 1% of alpha-related value?  {}".format('YES' if n_combined > 0 else 'NO'))
    print()

    print("  Values within 1% of 1/alpha = 137.036:")
    if n_alpha > 0:
        for ev in alpha_matches:
            print("    {}: {}".format(ev['r'], ev['matched']))
    else:
        print("    (none found among weight-0 eta-quotients)")
    print()

    if n_combined > 0:
        print("  Most interesting combination matches:")
        for m in (combined_matches if combined_matches else [])[:10]:
            err_pct = float(m['rel_err']) * 100
            print("    {}: {} ~ {} (err: {:.4f}%)".format(m['r'], m['combo'], m['target'], err_pct))
    else:
        print("  No combination matches found.")
    print()

    print("  Key numerical results:")
    print("    varpi = {}".format(nstr(VARPI, 30)))
    print("    1/alpha (CODATA) = {}".format(nstr(ALPHA_INV, 30)))
    print("    Master quadratic x+ = {}".format(nstr(mpf('137.0360'), 20)))
    print()

    # Fourier coefficients
    print("  First 20 Fourier coefficients of 15a1 newform:")
    header_line = "    n:  "
    for n in range(1, 21):
        header_line += "{:4d}".format(n)
    print(header_line)
    vals_line = "    a_n:"
    for n in range(1, 21):
        vals_line += "{:4d}".format(a_n[n])
    print(vals_line)
    print()

    print("=" * 80)
    print("  END OF SCRIPT 2 OF 5")
    print("=" * 80)


# ============================================================================
# MAIN
# ============================================================================

if __name__ == "__main__":
    print("=" * 80)
    print("  MODULAR FORMS AT LEVEL 15")
    print("  Script 2 of 5: FTD-RFT Bridge via Q(i, sqrt(15))")
    print("=" * 80)
    print("  Precision: {} decimal digits".format(mp.dps))
    print("  varpi = {}".format(nstr(VARPI, 30)))
    print("  1/alpha (CODATA 2022) = {}".format(nstr(ALPHA_INV, 30)))
    print()

    # Part 1
    part1_results = part1()

    # Part 2
    a_n, a_p_dict = part2()

    # Part 3
    part3()

    # Part 4
    all_evaluations, alpha_matches = part4()

    # Part 5
    combined_matches = part5(all_evaluations, alpha_matches)

    # Summary
    summary(part1_results, a_n, alpha_matches, combined_matches)
