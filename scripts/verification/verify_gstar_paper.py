"""
Pedantic verification of every numerical and algebraic claim in
docs/papers/PAPER_GSTAR_INTRODUCTION.tex at 40-digit precision.

Output: pass/fail table for each claim, with computed vs. displayed values
and absolute differences. Anything that doesn't match to the claimed
precision is flagged.

This is a one-shot verification script, not a unit test.
"""
from __future__ import annotations
from mpmath import (
    mp, mpf, mpc, gamma, pi, sqrt, agm, ellipk, ellipe,
    exp, log, sin, cos, hyper, fabs, euler, fsum, fprod, binomial,
)

def hyp4f3(a1, a2, a3, a4, b1, b2, b3, z):
    return hyper([a1, a2, a3, a4], [b1, b2, b3], z)

def hyp5f4(a1, a2, a3, a4, a5, b1, b2, b3, b4, z):
    return hyper([a1, a2, a3, a4, a5], [b1, b2, b3, b4], z)

mp.dps = 40

# ----------------------------------------------------------------------
# Foundational constants
# ----------------------------------------------------------------------

Gstar  = gamma(mpf(1)/4) / gamma(mpf(3)/4)              # G*
G_G    = 1 / agm(1, sqrt(2))                            # Gauss
varpi  = pi * G_G                                        # lemniscate constant
G_rho  = gamma(mpf(1)/3) * gamma(mpf(1)/6) / (2*pi*sqrt(pi))  # equianharmonic Gauss analog

# helper for verification
results = []  # list of (id, label, status, computed, paper_claim, abs_diff)
def check(label: str, computed, claim, tol=mpf(10)**(-20), note=""):
    diff = fabs(computed - claim) if isinstance(claim, (int, float, mpf, mpc)) else None
    status = "PASS" if (diff is not None and diff <= tol) else ("FAIL" if diff is not None else "INFO")
    results.append((label, status, str(computed)[:50], str(claim)[:50], str(diff)[:25] if diff is not None else "", note))

# ----------------------------------------------------------------------
# A. Foundational constants
# ----------------------------------------------------------------------

# 1. G* numerical value claimed: 2.95867511918863889231...
check("A1 G* = Gamma(1/4)/Gamma(3/4)", Gstar, mpf("2.95867511918863889231"), tol=mpf(10)**(-19))
# 2. G_G numerical value claimed: 0.83462684167407318628...
check("A2 G_G = 1/AGM(1, sqrt 2)", G_G, mpf("0.83462684167407318628"), tol=mpf(10)**(-19))
# 3. Bridge: G* = 2 sqrt(pi) G_G
check("A3 Bridge G* = 2 sqrt(pi) G_G", Gstar, 2*sqrt(pi)*G_G, tol=mpf(10)**(-35))
# 4. varpi = pi G_G  (verified via direct definition)
check("A4 varpi = pi G_G", varpi, pi*G_G, tol=mpf(10)**(-35))
# 5. varpi = Gamma(1/4)^2 / (2 sqrt(2 pi))
check("A5 varpi from Gamma def", varpi, gamma(mpf(1)/4)**2 / (2*sqrt(2*pi)), tol=mpf(10)**(-35))
# Cross-check varpi numerical: ~2.62206
check("A5b varpi numeric", varpi, mpf("2.62205755429211981046"), tol=mpf(10)**(-19))

# ----------------------------------------------------------------------
# B. Reflection identities
# ----------------------------------------------------------------------
check("B1 Gamma(1/4)*Gamma(3/4) = pi sqrt 2",
      gamma(mpf(1)/4) * gamma(mpf(3)/4), pi*sqrt(2), tol=mpf(10)**(-35))
check("B2 Gamma(1/4)^2 = pi sqrt 2 * G*",
      gamma(mpf(1)/4)**2, pi*sqrt(2)*Gstar, tol=mpf(10)**(-30))
check("B3 Gamma(3/4)^2 = pi sqrt 2 / G*",
      gamma(mpf(3)/4)**2, pi*sqrt(2)/Gstar, tol=mpf(10)**(-30))
check("B4 Gamma(1/4)^4 = 2 pi^2 G*^2",
      gamma(mpf(1)/4)**4, 2*pi**2*Gstar**2, tol=mpf(10)**(-25))
check("B5 Gamma(1/4)^8 = 4 pi^4 G*^4",
      gamma(mpf(1)/4)**8, 4*pi**4*Gstar**4, tol=mpf(10)**(-20))

# ----------------------------------------------------------------------
# C. Master quadratic roots
# ----------------------------------------------------------------------
# x^2 - 16 G*^2 x + 16 G*^3 = 0
# x_+ = 8 G*^2 + 4 G*^{3/2} sqrt(4 G* - 1)
x_plus  = 8*Gstar**2 + 4*Gstar**(mpf(3)/2) * sqrt(4*Gstar - 1)
x_minus = 8*Gstar**2 - 4*Gstar**(mpf(3)/2) * sqrt(4*Gstar - 1)

check("C1 x_+ = 137.03617145815548388",
      x_plus, mpf("137.03617145815548388"), tol=mpf(10)**(-17))
check("C2 x_- = 3.02396391633902100",
      x_minus, mpf("3.02396391633902100"), tol=mpf(10)**(-17))
check("C3 Vieta: x_+ * x_- = 16 G*^3",
      x_plus * x_minus, 16*Gstar**3, tol=mpf(10)**(-25))
check("C4 Vieta: 1/x_+ + 1/x_- = 1/G*",
      1/x_plus + 1/x_minus, 1/Gstar, tol=mpf(10)**(-30))
# Discriminant: 256 G*^4 - 64 G*^3 = 64 G*^3 (4 G* - 1)
disc = (16*Gstar**2)**2 - 4*16*Gstar**3
check("C5 Disc = 64 G*^3 (4 G* - 1)",
      disc, 64*Gstar**3*(4*Gstar - 1), tol=mpf(10)**(-30))

# ----------------------------------------------------------------------
# D. R_n family
# ----------------------------------------------------------------------
def R(n):
    return gamma(mpf(1)/n) / gamma(mpf(n-1)/n)

R2  = R(2)
R3  = R(3)
R4  = R(4)
R5  = R(5)
R6  = R(6)
R8  = R(8)
R12 = R(12)

check("D1 R_2 = 1", R2, 1, tol=mpf(10)**(-35))
check("D2 R_3 = 1.9783642596 (claim)",
      R3, mpf("1.9783642596"), tol=mpf(10)**(-9))
check("D3 R_4 = G* = 2.9586751192",
      R4, Gstar, tol=mpf(10)**(-35))
check("D4 R_5 = 3.9432456137",
      R5, mpf("3.9432456137"), tol=mpf(10)**(-9))
check("D5 R_6 = 4.9312366764",
      R6, mpf("4.9312366764"), tol=mpf(10)**(-9))
# Both R_8 references now consistent at 6.9140781897 after fix
check("D6 R_8 = 6.9140781897 (corrected line 503)",
      R8, mpf("6.9140781897"), tol=mpf(10)**(-9))
check("D6b R_8 ~ 6.91408 (line 596 asymptotic table)",
      R8, mpf("6.91408"), tol=mpf(10)**(-4))
check("D7 R_12 = 10.89429 (line 597 claim)",
      R12, mpf("10.89429"), tol=mpf(10)**(-4))
# Verify Euler-reflection alt form: R_n = Gamma(1/n)^2 sin(pi/n)/pi
for n in [3, 4, 5, 6, 8, 12]:
    Rn_alt = gamma(mpf(1)/n)**2 * sin(pi/n) / pi
    check(f"D8.{n} R_{n} via Gamma^2 sin/pi", R(n), Rn_alt, tol=mpf(10)**(-30))

# R_n ~ n - 2*gamma asymptotic check at n=12 (large-n regime)
check("D9 R_12 ~ 12 - 2*gamma (rough)",
      R12 - (12 - 2*euler), 0, tol=mpf("0.1"),
      note="asymptotic accuracy expected ~1/n")

# ----------------------------------------------------------------------
# E. Period and Beta
# ----------------------------------------------------------------------
omega_E_int = mpf(2) * mp.quad(lambda x: 1/sqrt(x*(x-1)*(x+1)), [1, mp.inf])
# CAREFUL: integral domain. The paper says "2 int_{-1}^{0}". Equivalent integral
# in absolute value over (1, inf) by symmetry of y^2 = x^3 - x.
omega_E_gamma = gamma(mpf(1)/4)**2 / sqrt(2*pi)

check("E1 omega_E via integral",
      omega_E_int, omega_E_gamma, tol=mpf(10)**(-15))
check("E2 omega_E = G* sqrt(pi)",
      omega_E_gamma, Gstar*sqrt(pi), tol=mpf(10)**(-30))
check("E3 omega_E = 2 pi G_G",
      omega_E_gamma, 2*pi*G_G, tol=mpf(10)**(-30))
# Beta(1/4, 1/4)
B_quarter = gamma(mpf(1)/4)**2 / gamma(mpf(1)/2)
check("E4 B(1/4,1/4) = sqrt(2pi) G*",
      B_quarter, sqrt(2*pi)*Gstar, tol=mpf(10)**(-30))
check("E5 B(1/4,1/4) = 2 sqrt(2) pi G_G",
      B_quarter, 2*sqrt(2)*pi*G_G, tol=mpf(10)**(-30))

# ----------------------------------------------------------------------
# F. Equianharmonic Gauss analog
# ----------------------------------------------------------------------
check("F1 G_rho ~ 1.33899",
      G_rho, mpf("1.33899"), tol=mpf(10)**(-4))
omega_rho = gamma(mpf(1)/3) * gamma(mpf(1)/6) / sqrt(3*pi)
# Paper claims G_rho = sqrt(3) * omega_rho / (2 pi)
check("F2 G_rho = sqrt(3) * omega_rho / (2 pi)",
      G_rho, sqrt(3)*omega_rho/(2*pi), tol=mpf(10)**(-30))
check("F3 Gamma(1/3)*Gamma(2/3) = 2 pi / sqrt(3)",
      gamma(mpf(1)/3)*gamma(mpf(2)/3), 2*pi/sqrt(3), tol=mpf(10)**(-30))

# ----------------------------------------------------------------------
# G. Watson constants
# ----------------------------------------------------------------------
W3 = 2 * G_G**2
check("G1 W^(3)_BCC = 2 G_G^2 = 1.39320392968567685918",
      W3, mpf("1.39320392968567685918"), tol=mpf(10)**(-19))
# W^(3) = (4/pi^2) K(1/sqrt 2)^2
K_half = ellipk(mpf(1)/2)  # mpmath ellipk(m) takes m = k^2; for k=1/sqrt(2), m=1/2
check("G2 W^(3) = (4/pi^2) K(1/sqrt 2)^2",
      W3, (4/pi**2) * K_half**2, tol=mpf(10)**(-25))
# W^(4) = 4F3(1/2,1/2,1/2,1/2; 1,1,1; 1)
W4_raw = hyp4f3(mpf(1)/2, mpf(1)/2, mpf(1)/2, mpf(1)/2, 1, 1, 1, 1)
check("G3 W^(4) = 4F3 = 1.11863638716418706835",
      W4_raw, mpf("1.11863638716418706835"), tol=mpf(10)**(-18))
# W^(4) Watson-normalised (the conjecture form):
W4_watson = (2/pi)**2 * W4_raw
check("G3b W4_watson = (2/pi)^2 * 4F3 ~ 0.4534",
      W4_watson, mpf("0.4534"), tol=mpf(10)**(-4))
# W^(5)
W5_raw = hyp5f4(mpf(1)/2, mpf(1)/2, mpf(1)/2, mpf(1)/2, mpf(1)/2, 1, 1, 1, 1, 1)
check("G4 W^(5) = 5F4 = 1.04682554983350000525",
      W5_raw, mpf("1.04682554983350000525"), tol=mpf(10)**(-18))

# ----------------------------------------------------------------------
# H. Theta functions at tau = i (Jacobi)
# ----------------------------------------------------------------------
# Use mpmath's jtheta. jtheta(j, z, q) where q = exp(i pi tau). At tau = i, q = exp(-pi).
# Conventions: theta_j(0|tau) = jtheta(j, 0, q) in mpmath for j = 1, 2, 3, 4.
from mpmath import jtheta
q_i = exp(-pi)
theta2_i = jtheta(2, 0, q_i)
theta3_i = jtheta(3, 0, q_i)
theta4_i = jtheta(4, 0, q_i)

check("H1 theta_2(0|i)^2 = G_G", theta2_i**2, G_G, tol=mpf(10)**(-25))
check("H2 theta_4(0|i)^2 = G_G", theta4_i**2, G_G, tol=mpf(10)**(-25))
check("H3 theta_3(0|i)^2 = sqrt(2) G_G", theta3_i**2, sqrt(2)*G_G, tol=mpf(10)**(-25))
# Jacobi identity at tau=i: theta_3^4 = theta_2^4 + theta_4^4  -> 2 G_G^2 = G_G^2 + G_G^2
check("H4 Jacobi: theta_3^4 = theta_2^4 + theta_4^4",
      theta3_i**4, theta2_i**4 + theta4_i**4, tol=mpf(10)**(-25))
# lambda(i) = theta_2^4 / theta_3^4 = 1/2
check("H5 lambda(i) = 1/2",
      theta2_i**4 / theta3_i**4, mpf(1)/2, tol=mpf(10)**(-25))

# ----------------------------------------------------------------------
# I. Eta tower
# ----------------------------------------------------------------------
# eta(tau) = q^{1/24} prod_{n>=1} (1 - q^n) where q = exp(2 pi i tau).
# For tau = i, q = exp(-2 pi); for tau = i/2, q = exp(-pi); for tau = 2i, q = exp(-4 pi).
def eta(tau):
    q = exp(2j*pi*tau)
    # Use Euler pentagonal-number form or just direct product (converges fast for |q| small).
    # Direct: q^{1/24} prod (1 - q^n)
    result = q**(mpf(1)/24)
    for n in range(1, 200):
        result *= (1 - q**n)
        # cut off when |q|^n is negligible
        if fabs(q**n) < mpf(10)**(-50):
            break
    return result

eta_i      = eta(mpc(0, 1))
eta_i_half = eta(mpc(0, mpf(1)/2))
eta_2i     = eta(mpc(0, 2))

# These are complex but with tiny imaginary parts; eta at purely imaginary tau is real
check("I1 eta(i) = G_G^(1/2)/2^(1/4) = 0.7683 (real part)",
      eta_i.real, G_G**(mpf(1)/2) / 2**(mpf(1)/4), tol=mpf(10)**(-25))
check("I2 |eta(i)|^16 = G_G^8/16",
      fabs(eta_i)**16, G_G**8 / 16, tol=mpf(10)**(-20))
check("I3 |eta(i)|^24 = G_G^12/64",
      fabs(eta_i)**24, G_G**12 / 64, tol=mpf(10)**(-18))
check("I4 |eta(i)|^2 = G_G/sqrt(2)",
      fabs(eta_i)**2, G_G/sqrt(2), tol=mpf(10)**(-25))
check("I5 |eta(i)|^4 = G_G^2/2",
      fabs(eta_i)**4, G_G**2/2, tol=mpf(10)**(-25))
check("I6 |eta(i)|^8 = G_G^4/4",
      fabs(eta_i)**8, G_G**4/4, tol=mpf(10)**(-22))
check("I7 |eta(i/2)|^2 = G_G/2^(1/4)",
      fabs(eta_i_half)**2, G_G / 2**(mpf(1)/4), tol=mpf(10)**(-25))
check("I8 |eta(2i)|^2 = G_G/2^(5/4)",
      fabs(eta_2i)**2, G_G / 2**(mpf(5)/4), tol=mpf(10)**(-25))
# eta(i/2) = sqrt 2 * eta(2i) modular S-transform
check("I9 eta(i/2) = sqrt(2) * eta(2i) (real parts)",
      eta_i_half.real, sqrt(2)*eta_2i.real, tol=mpf(10)**(-25))
# Watson-eta bridge: W^(3) = 4 eta(i)^4
check("I10 W^(3) = 4 eta(i)^4",
      W3, 4 * fabs(eta_i)**4, tol=mpf(10)**(-25))

# ----------------------------------------------------------------------
# J. L-value
# ----------------------------------------------------------------------
# L(E_lemn, 1) = varpi/4 = pi G_G / 4
check("J1 L(E,1) = varpi/4 (paper claims ~0.6555144)",
      varpi/4, mpf("0.6555144"), tol=mpf(10)**(-6),
      note="paper truncates at 7 digits with ellipsis; full computed value should round to 0.6555144")
check("J2 L(E,1) = pi G_G / 4",
      varpi/4, pi*G_G/4, tol=mpf(10)**(-35))
# Omega = 2 varpi
Omega_BSD = 2 * varpi
check("J3 Omega = 2 varpi", Omega_BSD, 2*varpi, tol=mpf(10)**(-35))
# BSD: L(E,1) = Omega * prod c_p * |Sha| / |E(Q)_tors|^2
# = (2 varpi * 2 * 1) / 16 = varpi/4
L_BSD = Omega_BSD * 2 * 1 / 16
check("J4 BSD: (2*varpi * c_2 * 1) / |tors|^2 = varpi/4",
      L_BSD, varpi/4, tol=mpf(10)**(-35))

# ----------------------------------------------------------------------
# K. Elliptic integrals at k = 1/sqrt(2)
# ----------------------------------------------------------------------
# mpmath ellipk(m) takes parameter m = k^2; for k = 1/sqrt(2), m = 1/2
K_half = ellipk(mpf(1)/2)
E_half = ellipe(mpf(1)/2)

check("K1 K(1/sqrt 2) = pi G_G / sqrt(2)",
      K_half, pi*G_G/sqrt(2), tol=mpf(10)**(-25))
check("K2 E(1/sqrt 2) = (sqrt 2 /4)(1/G_G + pi G_G)",
      E_half, (sqrt(2)/4)*(1/G_G + pi*G_G), tol=mpf(10)**(-25))
check("K3 K E = (pi + varpi^2)/4",
      K_half * E_half, (pi + varpi**2)/4, tol=mpf(10)**(-25))
check("K4 G_G^2 = (4 K E - pi)/pi^2",
      G_G**2, (4*K_half*E_half - pi)/pi**2, tol=mpf(10)**(-25))
# Legendre relation: 2 K E - K^2 = pi/2 at k = k' = 1/sqrt 2
check("K5 Legendre: 2 K E - K^2 = pi/2",
      2*K_half*E_half - K_half**2, pi/2, tol=mpf(10)**(-25))

# ----------------------------------------------------------------------
# L. Modular forms at tau = i (Eisenstein series)
# ----------------------------------------------------------------------
# Compute E_{2k}(tau) = 1 - (4k/B_{2k}) sum_{n>=1} sigma_{2k-1}(n) q^n
# where q = exp(2 pi i tau), B_{2k} are Bernoulli numbers.
from mpmath import bernoulli

def sigma(s, n):
    """sum of d^s over divisors of n"""
    return sum(d**s for d in range(1, n+1) if n % d == 0)

def E_2k(k, tau, max_n=200):
    """Eisenstein series E_{2k}(tau), 2k >= 4. For 2k=2 use quasi-modular value separately."""
    q = exp(2j*pi*tau)
    B = bernoulli(2*k)
    coef = -mpf(4*k) / B
    s = mpf(0)
    for n in range(1, max_n+1):
        term = sigma(2*k-1, n) * q**n
        s += term
        if fabs(term) < mpf(10)**(-50):
            break
    return 1 + coef * s

# at tau = i
tau_i = mpc(0, 1)
E4_i  = E_2k(2, tau_i)
E6_i  = E_2k(3, tau_i)
E8_i  = E_2k(4, tau_i)
E10_i = E_2k(5, tau_i)
E12_i = E_2k(6, tau_i)
E14_i = E_2k(7, tau_i)
E16_i = E_2k(8, tau_i)
E20_i = E_2k(10, tau_i)
E24_i = E_2k(12, tau_i)

check("L1 E_4(i) = 3 G_G^4", E4_i.real, 3*G_G**4, tol=mpf(10)**(-20))
check("L2 E_6(i) = 0", E6_i.real, 0, tol=mpf(10)**(-15))
check("L3 E_8(i) = 9 G_G^8", E8_i.real, 9*G_G**8, tol=mpf(10)**(-18))
check("L4 E_10(i) = 0", E10_i.real, 0, tol=mpf(10)**(-15))
check("L5 E_12(i) = (11907/691) G_G^12",
      E12_i.real, mpf(11907)/691 * G_G**12, tol=mpf(10)**(-15))
check("L6 E_14(i) = 0", E14_i.real, 0, tol=mpf(10)**(-12))
check("L7 E_16(i) = (130977/3617) G_G^16",
      E16_i.real, mpf(130977)/3617 * G_G**16, tol=mpf(10)**(-12))
check("L8 E_20(i) = (12966723/174611) G_G^20",
      E20_i.real, mpf(12966723)/174611 * G_G**20, tol=mpf(10)**(-10))
check("L9 E_24(i) = (36216057339/236364091) G_G^24",
      E24_i.real, mpf(36216057339)/236364091 * G_G**24, tol=mpf(10)**(-8))
# Delta(i) = G_G^12 / 64
Delta_i = (E4_i**3 - E6_i**2) / 1728
check("L10 Delta(i) = G_G^12/64",
      Delta_i.real, G_G**12/64, tol=mpf(10)**(-15))
# j(i) = E_4^3 / Delta = 1728
j_i = E4_i**3 / Delta_i
check("L11 j(i) = 1728", j_i.real, 1728, tol=mpf(10)**(-12))

# ----------------------------------------------------------------------
# M. Quasi-modular E_2 and corrected G_G^4 formula
# ----------------------------------------------------------------------
# E_2(i) = 3/pi (quasi-modular)
# Compute directly: E_2 = 1 - 24 sum sigma_1(n) q^n
def E_2_qm(tau, max_n=200):
    q = exp(2j*pi*tau)
    s = mpf(0)
    for n in range(1, max_n+1):
        term = sigma(1, n) * q**n
        s += term
        if fabs(term) < mpf(10)**(-50):
            break
    return 1 - 24*s

E2_i = E_2_qm(tau_i)
check("M1 E_2(i) = 3/pi", E2_i.real, 3/pi, tol=mpf(10)**(-25))

# E_2(rho) = 2 sqrt(3) / pi
tau_rho = mpc(mpf(1)/2, sqrt(3)/2)
E2_rho = E_2_qm(tau_rho)
check("M2 E_2(rho) = 2 sqrt(3)/pi", fabs(E2_rho), 2*sqrt(3)/pi, tol=mpf(10)**(-20))

# Corrected G_G^4 formula
check("M3 G_G^4 = Gamma(1/4)^8 / (64 pi^6) [CORRECTED]",
      G_G**4, gamma(mpf(1)/4)**8 / (64*pi**6), tol=mpf(10)**(-30))

# Anti-regression: verify the WRONG formula from v1.0 is NOT equal to G_G^4
# This test passes if the wrong formula correctly differs by 4 pi^4 factor
wrong_GG4 = gamma(mpf(1)/4)**8 / (16*pi**2)
expected_ratio = 4 * pi**4
check("M4 [ANTI-REGRESSION] wrong v1.0 formula / G_G^4 == 4 pi^4",
      wrong_GG4 / G_G**4, expected_ratio, tol=mpf(10)**(-25),
      note="confirms the v1.0 formula was off by exactly 4 pi^4 ~ 389.6")

# E_12 basis decomposition coefficients (verify 441/691 + 250/691 = 1)
check("M5 Basis decomp: 441 + 250 = 691",
      mpf(441) + mpf(250), 691, tol=mpf(10)**(-35))
# E_12(i) via direct basis decomp: 441/691 * E_4(i)^3 + 250/691 * E_6(i)^2
# = 441/691 * 27 G_G^12 + 0
E12_i_basis = mpf(441)/691 * 27 * G_G**12
check("M6 E_12(i) via basis decomp = 441/691 * 27 * G_G^12 = 11907/691 * G_G^12",
      E12_i.real, E12_i_basis, tol=mpf(10)**(-15))

# ----------------------------------------------------------------------
# N. Equianharmonic at tau = rho
# ----------------------------------------------------------------------
E4_rho  = E_2k(2, tau_rho)
E6_rho  = E_2k(3, tau_rho)
E12_rho = E_2k(6, tau_rho)

check("N1 E_4(rho) = 0", fabs(E4_rho), 0, tol=mpf(10)**(-12))
check("N2 E_6(rho) = G_rho^6 / 2",
      fabs(E6_rho), G_rho**6/2, tol=mpf(10)**(-15))
check("N3 E_12(rho) = (125/1382) G_rho^12",
      fabs(E12_rho), mpf(125)/1382 * G_rho**12, tol=mpf(10)**(-12))
# Delta(rho) = -G_rho^12 / 6912
Delta_rho = (E4_rho**3 - E6_rho**2) / 1728
check("N4 Delta(rho) = -G_rho^12 / 6912",
      Delta_rho.real, -G_rho**12/6912, tol=mpf(10)**(-12))
# 6912 = 2^8 * 27
check("N5 6912 = 2^8 * 27", mpf(6912), 256*27, tol=0)
# |eta(rho)|^24 = |Delta(rho)|
eta_rho = eta(tau_rho)
check("N6 |eta(rho)|^24 = G_rho^12/6912",
      fabs(eta_rho)**24, G_rho**12/6912, tol=mpf(10)**(-12))

# ----------------------------------------------------------------------
# O. Asymptotic d_n coefficients
# ----------------------------------------------------------------------
def d_n(n):
    return binomial(2*n, n) / ((2*n-1) * mpf(2)**(4*n-3))

check("O1 d_1 = 1", d_n(1), 1, tol=0)
check("O2 d_2 = 1/16", d_n(2), mpf(1)/16, tol=0)
check("O3 d_3 = 1/128", d_n(3), mpf(1)/128, tol=0)
check("O4 d_4 = 5/4096", d_n(4), mpf(5)/4096, tol=0)
check("O5 d_5 = 7/32768", d_n(5), mpf(7)/32768, tol=0)

# Verify the x_-(R_n) - R_n approaches 1/16 from above for R_n = R(8) and R(12)
for n in [3, 4, 5, 6, 8, 12]:
    Rn = R(n)
    x_minus_Rn = 8*Rn**2 - 4*Rn**(mpf(3)/2) * sqrt(4*Rn - 1)
    check(f"O6.{n} x_-(R_{n}) - R_{n} > 1/16",
          x_minus_Rn - Rn, mpf("0.0625"), tol=mpf("0.01"),
          note=f"actual diff = {float(x_minus_Rn - Rn):.5f}")

# ----------------------------------------------------------------------
# P. chi_{-4} structure
# ----------------------------------------------------------------------
# chi_{-4}(n) = Im(i^n) = sin(pi n / 2)
for n in [0, 1, 2, 3, 4, 5, 6, 7]:
    expected = 0 if n % 2 == 0 else (1 if n % 4 == 1 else -1)
    chi_imag = mpc(0, 1)**n
    check(f"P1.{n} chi_{{-4}}({n}) = Im(i^{n}) = sin(pi*{n}/2)",
          chi_imag.imag, expected, tol=mpf(10)**(-30),
          note=f"and = {float(sin(pi*n/2)):.3f}")

# Trivial chi_{-4}-twisted Gamma product evaluation
# prod Gamma(a/4)^{chi(a)} for a=1,2,3 = Gamma(1/4)^1 * Gamma(2/4)^0 * Gamma(3/4)^{-1} = Gamma(1/4)/Gamma(3/4) = G*
check("P2 chi_{-4}-twisted Gamma product = G*",
      gamma(mpf(1)/4) / gamma(mpf(3)/4), Gstar, tol=mpf(10)**(-35))

# ----------------------------------------------------------------------
# Q. P2 closure: cubic-AGM expression for G_rho (Theorem thm:cubic-AGM-Grho)
# ----------------------------------------------------------------------

def M3(a, b, max_iter=200, tol_dps=40):
    """Borwein-Borwein cubic AGM. Iteration:
       a_{n+1} = (a_n + 2 b_n) / 3
       b_{n+1} = (b_n (a_n^2 + a_n b_n + b_n^2) / 3)^(1/3)
    """
    a, b = mpf(a), mpf(b)
    tol = mpf(10)**(-tol_dps)
    for _ in range(max_iter):
        if fabs(a - b) < tol * fabs(a):
            return (a + b) / 2
        a_new = (a + 2*b) / 3
        b_new = (b * (a**2 + a*b + b**2) / 3) ** (mpf(1)/3)
        a, b = a_new, b_new
    return (a + b) / 2

# Step (i): equianharmonic period reduces to 2^(2/3) Gamma(1/3)^3 / (2 pi)
omega_rho_direct  = gamma(mpf(1)/3) * gamma(mpf(1)/6) / sqrt(3*pi)
omega_rho_reduced = mpf(2)**(mpf(2)/3) * gamma(mpf(1)/3)**3 / (2*pi)
check("Q1 omega_rho = 2^(2/3) Gamma(1/3)^3 / (2 pi)",
      omega_rho_direct, omega_rho_reduced, tol=mpf(10)**(-30))

# Step (ii): G_rho explicit Gamma form
G_rho_explicit = sqrt(3) * mpf(2)**(mpf(2)/3) * gamma(mpf(1)/3)**3 / (4*pi**2)
check("Q2 G_rho = sqrt(3) * 2^(2/3) * Gamma(1/3)^3 / (4 pi^2)",
      G_rho, G_rho_explicit, tol=mpf(10)**(-30))

# Step (iii): 2F1(1/3, 2/3; 1; 1/2) closed form
F213_direct = hyper([mpf(1)/3, mpf(2)/3], [1], mpf(1)/2)
F213_closed = (3 * mpf(2)**(mpf(2)/3)) / (8*pi**2) * gamma(mpf(1)/3)**3
check("Q3 2F1(1/3, 2/3; 1; 1/2) = (3 * 2^(2/3) / 8 pi^2) Gamma(1/3)^3",
      F213_direct, F213_closed, tol=mpf(10)**(-30))

# Step (iv): Borwein-Borwein cubic AGM correspondence at x = 2^(-1/3)
M3_at = M3(1, mpf(2)**(-mpf(1)/3))
check("Q4 1/M_3(1, 2^(-1/3)) = 2F1(1/3, 2/3; 1; 1/2)",
      1/M3_at, F213_direct, tol=mpf(10)**(-30))

# Final: G_rho cubic-AGM theorem
check("Q5 G_rho = (2/sqrt 3) / M_3(1, 2^(-1/3))",
      G_rho, (2/sqrt(3)) / M3_at, tol=mpf(10)**(-30))
check("Q6 G_rho = (2/sqrt 3) * 2F1(1/3, 2/3; 1; 1/2)",
      G_rho, (2/sqrt(3)) * F213_direct, tol=mpf(10)**(-30))

# ----------------------------------------------------------------------
# REPORT
# ----------------------------------------------------------------------
print(f"\n{'='*80}")
print(f"VERIFICATION REPORT — {mp.dps} decimal digit precision")
print(f"{'='*80}\n")
print(f"{'#':<4} {'Status':<6} {'Label':<55} {'|diff|':<25} {'Note'}")
print(f"{'-'*120}")

n_pass = 0
n_fail = 0
n_info = 0
for i, (label, status, comp, claim, diff, note) in enumerate(results):
    print(f"{i+1:<4} {status:<6} {label:<55} {diff:<25} {note}")
    if status == "PASS": n_pass += 1
    elif status == "FAIL": n_fail += 1
    else: n_info += 1

print(f"\n{'='*80}")
print(f"TOTAL: {n_pass} PASS, {n_fail} FAIL, {n_info} INFO -- {len(results)} checks")
print(f"{'='*80}")

if n_fail > 0:
    print(f"\nFAILURES (showing computed vs claim):\n")
    for label, status, comp, claim, diff, note in results:
        if status == "FAIL":
            print(f"  {label}")
            print(f"    computed:    {comp}")
            print(f"    paper claim: {claim}")
            print(f"    |diff|:      {diff}")
            if note:
                print(f"    note:        {note}")
            print()
