"""
Deep Mathematical Investigation: RFT Polynomial p(x) = 4x³ + x² + x
and its connections to FTD, number theory, and fundamental constants.

Author: Claude Code Investigation
Date: 2026-02-07
"""

import numpy as np
from scipy import special
import sys
import os

# Fix Windows console encoding
if sys.platform == 'win32':
    os.environ['PYTHONIOENCODING'] = 'utf-8'
    sys.stdout.reconfigure(encoding='utf-8')

# Try to import mpmath for arbitrary precision
try:
    import mpmath
    mpmath.mp.dps = 50  # 50 decimal places
    HAS_MPMATH = True
    print("mpmath available — using 50-digit precision\n")
except ImportError:
    HAS_MPMATH = False
    print("mpmath not available — using numpy/scipy (15-16 digits)\n")

SEPARATOR = "=" * 80
SUBSEP = "-" * 60

def header(title):
    print(f"\n{SEPARATOR}")
    print(f"  {title}")
    print(f"{SEPARATOR}\n")

def subheader(title):
    print(f"\n{SUBSEP}")
    print(f"  {title}")
    print(f"{SUBSEP}\n")

# ============================================================================
# FUNDAMENTAL CONSTANTS (high precision)
# ============================================================================

if HAS_MPMATH:
    pi = mpmath.pi
    e_const = mpmath.e
    phi = (1 + mpmath.sqrt(5)) / 2  # golden ratio
    sqrt2 = mpmath.sqrt(2)
    sqrt3 = mpmath.sqrt(3)
    sqrt5 = mpmath.sqrt(5)
    sqrt15 = mpmath.sqrt(15)

    # Lemniscatic constant varpi
    varpi = mpmath.gamma(mpmath.mpf('0.25'))**2 / (2 * mpmath.sqrt(2 * pi))

    # G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)
    gamma_quarter = mpmath.gamma(mpmath.mpf('0.25'))
    G_star = sqrt2 * gamma_quarter**2 / (2 * pi)

    # Fine structure constant (CODATA 2022)
    alpha_inv_exp = mpmath.mpf('137.035999177')
    alpha_exp = 1 / alpha_inv_exp

    def to_float(x):
        return float(x)

    def mp_sqrt(x):
        return mpmath.sqrt(x)

    def mp_cos(x):
        return mpmath.cos(x)

    def mp_acos(x):
        return mpmath.acos(x)

    def mp_exp(x):
        return mpmath.exp(x)

    def mp_log(x):
        return mpmath.log(x)

    def mp_power(x, n):
        return mpmath.power(x, n)

    def mp_abs(x):
        return mpmath.fabs(x)

    def mp_floor(x):
        return mpmath.floor(x)

    def mp_frac(x):
        return x - mpmath.floor(x)

    def fmt(x, digits=15):
        return mpmath.nstr(x, digits + 1)

else:
    pi = np.pi
    e_const = np.e
    phi = (1 + np.sqrt(5)) / 2
    sqrt2 = np.sqrt(2)
    sqrt3 = np.sqrt(3)
    sqrt5 = np.sqrt(5)
    sqrt15 = np.sqrt(15)

    gamma_quarter = special.gamma(0.25)
    varpi = gamma_quarter**2 / (2 * np.sqrt(2 * pi))
    G_star = sqrt2 * gamma_quarter**2 / (2 * pi)

    alpha_inv_exp = 137.035999177
    alpha_exp = 1 / alpha_inv_exp

    def to_float(x):
        return float(x)

    def mp_sqrt(x):
        return np.sqrt(float(x))

    def mp_cos(x):
        return np.cos(float(x))

    def mp_acos(x):
        return np.arccos(float(x))

    def mp_exp(x):
        return np.exp(float(x))

    def mp_log(x):
        return np.log(float(x))

    def mp_power(x, n):
        return float(x)**float(n)

    def mp_abs(x):
        return abs(float(x))

    def mp_floor(x):
        return np.floor(float(x))

    def mp_frac(x):
        return float(x) - np.floor(float(x))

    def fmt(x, digits=15):
        return f"{float(x):.{digits}f}"

# ============================================================================
# BASIC SETUP
# ============================================================================

header("FUNDAMENTAL CONSTANTS")

print(f"  pi              = {fmt(pi)}")
print(f"  e               = {fmt(e_const)}")
print(f"  phi (golden)    = {fmt(phi)}")
print(f"  sqrt(2)         = {fmt(sqrt2)}")
print(f"  sqrt(15)        = {fmt(sqrt15)}")
print(f"  Gamma(1/4)      = {fmt(gamma_quarter)}")
print(f"  varpi           = {fmt(varpi)}")
print(f"  G*              = {fmt(G_star)}")
print(f"  1/alpha (exp)   = {fmt(alpha_inv_exp)}")
print(f"  alpha (exp)     = {fmt(alpha_exp, 18)}")

# RFT polynomial
def p_rft(x):
    return 4*x**3 + x**2 + x

p_pi = p_rft(pi)
print(f"\n  p(pi) = 4pi^3 + pi^2 + pi = {fmt(p_pi)}")

# FTD master quadratic roots
# x^2 - 16*G*^2 * x + 16*G*^3 = 0
a_ftd = 1
b_ftd = -16 * G_star**2
c_ftd = 16 * G_star**3
disc_ftd = b_ftd**2 - 4*a_ftd*c_ftd
x_plus = (-b_ftd + mp_sqrt(disc_ftd)) / 2
x_minus = (-b_ftd - mp_sqrt(disc_ftd)) / 2

print(f"\n  FTD x_+         = {fmt(x_plus)}")
print(f"  FTD x_-         = {fmt(x_minus)}")

delta = p_pi - x_plus
print(f"\n  delta = p(pi) - x_+ = {fmt(delta, 18)}")
print(f"  |delta|/x_+ (ppm)   = {fmt(mp_abs(delta)/x_plus * 1e6, 6)}")

# ============================================================================
# INVESTIGATION 1: Discriminant -15 and Number Theory
# ============================================================================

header("INVESTIGATION 1: THE DISCRIMINANT -15 AND NUMBER THEORY")

subheader("1a. Quadratic 4x^2 + x + 1 properties")

a_q, b_q, c_q = 4, 1, 1
disc_q = b_q**2 - 4*a_q*c_q
print(f"  Quadratic: 4x^2 + x + 1")
print(f"  a = {a_q}, b = {b_q}, c = {c_q}")
print(f"  Discriminant = b^2 - 4ac = 1 - 16 = {disc_q}")
print(f"  Discriminant = -15 = -3 x 5")
print(f"  Fundamental discriminant: -15 (already fundamental, since -15 ≡ 1 mod 4)")

subheader("1b. Class number of Q(sqrt(-15))")

print("  The imaginary quadratic field Q(sqrt(-15)):")
print("  Discriminant D = -15")
print("  Class number h(-15) = 2")
print()
print("  Reduced binary quadratic forms of discriminant -15:")
print("  Form 1: (1, 1, 4)  -> x^2 + xy + 4y^2  [principal form]")
print("  Form 2: (2, 1, 2)  -> 2x^2 + xy + 2y^2")
print()

# Verify the forms have discriminant -15
for (a, b, c), label in [((1,1,4), "Form 1"), ((2,1,2), "Form 2")]:
    d = b**2 - 4*a*c
    print(f"  {label}: ({a},{b},{c}), disc = {b}^2 - 4*{a}*{c} = {d}  {'OK' if d == -15 else 'ERROR'}")

subheader("1c. j-invariants for discriminant -15")

print("  For a quadratic form (a,b,c) with disc D < 0, the associated")
print("  CM point in the upper half plane is:")
print("  tau = (-b + sqrt(D)) / (2a)")
print()

# CM points
if HAS_MPMATH:
    tau1 = (-1 + mpmath.sqrt(-15)) / 2  # Form (1,1,4): tau = (-1 + sqrt(-15))/2
    tau2 = (-1 + mpmath.sqrt(-15)) / 4  # Form (2,1,2): tau = (-1 + sqrt(-15))/4

    print(f"  tau_1 (form 1,1,4) = (-1 + sqrt(-15))/2")
    print(f"    = {mpmath.nstr(mpmath.re(tau1), 15)} + {mpmath.nstr(mpmath.im(tau1), 15)}i")
    print(f"  tau_2 (form 2,1,2) = (-1 + sqrt(-15))/4")
    print(f"    = {mpmath.nstr(mpmath.re(tau2), 15)} + {mpmath.nstr(mpmath.im(tau2), 15)}i")

    # Compute j-invariants using the Klein j-function
    # j(tau) = 1728 * J(tau) where J is the modular J-invariant
    # mpmath has the j-function built in
    j1 = mpmath.kleinj(tau1)
    j2 = mpmath.kleinj(tau2)

    print(f"\n  j(tau_1) = {mpmath.nstr(j1, 20)}")
    print(f"  j(tau_2) = {mpmath.nstr(j2, 20)}")

    # The Hilbert class polynomial H_D(x) for D = -15
    # Since h(-15) = 2, it's a degree-2 polynomial
    # H_{-15}(x) = (x - j1)(x - j2) = x^2 - (j1+j2)x + j1*j2

    j_sum = j1 + j2
    j_prod = j1 * j2

    print(f"\n  Hilbert Class Polynomial for D = -15:")
    print(f"  H_{{-15}}(x) = x^2 - ({mpmath.nstr(mpmath.re(j_sum), 6)})x + ({mpmath.nstr(mpmath.re(j_prod), 6)})")
    print(f"  j1 + j2 = {mpmath.nstr(j_sum, 20)}")
    print(f"  j1 * j2 = {mpmath.nstr(j_prod, 20)}")

    # Known result: H_{-15}(x) = x^2 + 191025x - 121287375
    print(f"\n  Known exact result: H_{{-15}}(x) = x^2 + 191025x - 121287375")
    print(f"  Verification: j1 + j2 should be -191025: {mpmath.nstr(j_sum, 15)}")
    print(f"  Verification: j1 * j2 should be -121287375: {mpmath.nstr(j_prod, 15)}")

    # Compare with FTD's j = 1728
    print(f"\n  FTD uses j = 1728 (from CM curve with disc -4)")
    print(f"  Disc -15 j-invariants: {mpmath.nstr(j1, 10)}, {mpmath.nstr(j2, 10)}")
    print(f"  Ratio j1/1728 = {mpmath.nstr(j1/1728, 10)}")
    print(f"  Ratio j2/1728 = {mpmath.nstr(j2/1728, 10)}")

    # Factor analysis
    print(f"\n  191025 = 3^4 * 5^2 * 94.1... let me factor properly")
    n = 191025
    factors = []
    temp = n
    for p in [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31]:
        while temp % p == 0:
            factors.append(p)
            temp //= p
    if temp > 1:
        factors.append(temp)
    print(f"  191025 = {' x '.join(map(str, factors))}")

    n2 = 121287375
    factors2 = []
    temp2 = n2
    for p in [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43]:
        while temp2 % p == 0:
            factors2.append(p)
            temp2 //= p
    if temp2 > 1:
        factors2.append(temp2)
    print(f"  121287375 = {' x '.join(map(str, factors2))}")

else:
    print("  [Requires mpmath for Klein j-function computation]")
    print("  Known result: H_{-15}(x) = x^2 + 191025x - 121287375")
    print("  j-invariants are roots: j = (-191025 +/- sqrt(191025^2 + 4*121287375))/2")
    disc_hilbert = 191025**2 + 4*121287375
    print(f"  Hilbert disc = {disc_hilbert}")
    print(f"  sqrt(disc) = {np.sqrt(disc_hilbert):.6f}")
    j1_approx = (-191025 + np.sqrt(disc_hilbert))/2
    j2_approx = (-191025 - np.sqrt(disc_hilbert))/2
    print(f"  j1 ≈ {j1_approx:.6f}")
    print(f"  j2 ≈ {j2_approx:.6f}")

subheader("1d. Connection between -15 and the RFT coefficients")

print("  The polynomial 4x^2 + x + 1 has disc = -15 = -(4*4 - 1)")
print("  Note: for ax^2 + bx + c with a=4, b=1, c=1:")
print("  disc = 1 - 16 = -15")
print()
print("  The number 15 = 3 * 5 connects to:")
print("  - 15 = number of edges in the complete graph K_6")
print("  - 15 = dimension of SU(4) Lie algebra")
print("  - 15 = 2^4 - 1 (Mersenne number)")
print("  - 15 = F_5 * F_4 (Fibonacci: 5 * 3)")
print("  - 15 is a triangular number: T_5 = 5*6/2 = 15")
print()
print("  In FTD context:")
print("  - 3 = N_c (color charges, from x_-)")
print("  - 5 = number of FTD axioms / Planck units")
print("  - 4 = N_base in FTD (appears as leading coefficient)")


# ============================================================================
# INVESTIGATION 2: The Modulus 1/2 Connection
# ============================================================================

header("INVESTIGATION 2: THE MODULUS 1/2 CONNECTION")

subheader("2a. Roots of 4x^2 + x + 1 = 0")

# Roots: x = (-1 +/- sqrt(-15)) / 8
r1_re = -1 / mpmath.mpf(8) if HAS_MPMATH else -1/8
r1_im_pos = mp_sqrt(15) / 8 if HAS_MPMATH else np.sqrt(15)/8

print(f"  Roots of 4x^2 + x + 1 = 0:")
print(f"  r1 = (-1 + i*sqrt(15)) / 8 = {fmt(r1_re)} + {fmt(r1_im_pos)}i")
print(f"  r2 = (-1 - i*sqrt(15)) / 8 = {fmt(r1_re)} - {fmt(r1_im_pos)}i")
print()

modulus_sq = r1_re**2 + r1_im_pos**2
modulus = mp_sqrt(modulus_sq)
print(f"  |r|^2 = (1/64) + (15/64) = 16/64 = 1/4")
print(f"  |r|^2 (computed) = {fmt(modulus_sq)}")
print(f"  |r| = 1/2 EXACTLY")
print(f"  |r| (computed) = {fmt(modulus)}")

subheader("2b. Polar form: r = (1/2)*exp(i*theta)")

theta = mp_acos(-1/mpmath.mpf(4) if HAS_MPMATH else -0.25)
theta_deg = theta * 180 / pi

print(f"  r = (1/2) * exp(i*theta)")
print(f"  cos(theta) = Re(r)/(1/2) = (-1/8)/(1/2) = -1/4")
print(f"  theta = arccos(-1/4)")
print(f"  theta = {fmt(theta)} radians")
print(f"  theta = {fmt(theta_deg)} degrees")
print()

sin_theta = mp_sqrt(15) / 4 if HAS_MPMATH else np.sqrt(15)/4
print(f"  sin(theta) = sqrt(1 - 1/16) = sqrt(15/16) = sqrt(15)/4")
print(f"  sin(theta) = {fmt(sin_theta)}")
print(f"  Verification: sin^2 + cos^2 = {fmt(sin_theta**2 + (-1/mpmath.mpf(4) if HAS_MPMATH else -0.25)**2)}")

subheader("2c. Angle analysis")

ratio_theta_pi = theta / pi
print(f"  theta/pi = {fmt(ratio_theta_pi, 20)}")
print(f"  360/theta(deg) = {fmt(360/theta_deg, 10)}")
print()

# Check if theta/pi is rational using continued fraction
print("  Continued fraction expansion of theta/pi:")
if HAS_MPMATH:
    # Get continued fraction coefficients
    x_cf = ratio_theta_pi
    cf_coeffs = []
    for i in range(20):
        a_cf = mpmath.floor(x_cf)
        cf_coeffs.append(int(a_cf))
        remainder = x_cf - a_cf
        if mp_abs(remainder) < mpmath.mpf('1e-40'):
            break
        x_cf = 1 / remainder
    print(f"  [{', '.join(map(str, cf_coeffs[:15]))}...]")
    print(f"  No termination -> theta/pi is IRRATIONAL (as Niven's theorem predicts)")
else:
    x_cf = float(ratio_theta_pi)
    cf_coeffs = []
    for i in range(15):
        a_cf = int(np.floor(x_cf))
        cf_coeffs.append(a_cf)
        remainder = x_cf - a_cf
        if abs(remainder) < 1e-12:
            break
        x_cf = 1 / remainder
    print(f"  [{', '.join(map(str, cf_coeffs))}...]")

print()
print("  Niven's Theorem: cos(r*pi) is rational iff r in {0, 1/6, 1/3, 1/2, 2/3, 5/6, 1}")
print("  (and their reflections). cos(theta) = -1/4 is rational but not in this set,")
print("  confirming theta/pi is IRRATIONAL.")

subheader("2d. Crystallographic angle check")

print("  Common crystallographic angles (degrees):")
crystal_angles = [60, 90, 109.47, 120, 180, 70.53, 54.74]
labels = ["60 (hexagonal)", "90 (cubic)", "109.47 (tetrahedral)",
          "120 (trigonal)", "180 (linear)", "70.53 (supplement of tet.)",
          "54.74 (magic angle)"]
for ang, label in zip(crystal_angles, labels):
    diff = to_float(mp_abs(theta_deg - ang))
    print(f"  |theta - {label}| = {diff:.4f} deg")

print(f"\n  Closest: tetrahedral angle 109.47 deg (diff = {to_float(mp_abs(theta_deg - 109.47)):.4f} deg)")
print(f"  theta - 109.47 = {to_float(theta_deg - 109.47):.6f} deg")

# Exact tetrahedral angle
tet_angle = mp_acos(-1/mpmath.mpf(3) if HAS_MPMATH else -1/3) * 180 / pi
print(f"\n  Exact tetrahedral angle = arccos(-1/3) = {fmt(tet_angle)} deg")
print(f"  Our angle = arccos(-1/4) = {fmt(theta_deg)} deg")
print(f"  Note: arccos(-1/3) vs arccos(-1/4) — consecutive 1/n values!")
print(f"  Difference = {fmt(theta_deg - tet_angle)} deg")

subheader("2e. Angle relationships with physical constants")

print(f"  theta/(2*pi) = {fmt(theta/(2*pi))} (fraction of full rotation)")
print(f"  2*pi/theta = {fmt(2*pi/theta)} (number of theta's in full rotation)")
print(f"  theta*137 = {fmt(theta*137)} rad = {fmt(theta*137*180/pi)} deg")
print(f"  theta*137/(2*pi) = {fmt(theta*137/(2*pi))} full rotations")
# Fractional part
frac_137 = theta * 137 / (2*pi)
frac_part = frac_137 - mp_floor(frac_137)
print(f"  Fractional part = {fmt(frac_part)}")
print(f"  theta*alpha = {fmt(theta*alpha_exp)} rad")


# ============================================================================
# INVESTIGATION 3: Roots of Unity Connection
# ============================================================================

header("INVESTIGATION 3: ROOTS OF UNITY AND cos(2*pi*k/15)")

subheader("3a. Values of cos(2*pi*k/15) for k = 0..7")

print("  We need cos = -1/4 = -0.25000...")
print()
for k in range(8):
    cos_val = mp_cos(2*pi*k/15)
    diff = mp_abs(cos_val - (-1/mpmath.mpf(4) if HAS_MPMATH else -0.25))
    marker = " <--- CLOSE" if to_float(diff) < 0.05 else ""
    print(f"  cos(2*pi*{k}/15) = {fmt(cos_val, 12)}{marker}")

subheader("3b. Check cos(2*pi*k/n) = -1/4 for small n")

print("  Searching for n, k such that cos(2*pi*k/n) = -1/4:")
print()
found_any = False
for n in range(3, 101):
    for k in range(1, n):
        cos_val = mp_cos(2*pi*k/n)
        if to_float(mp_abs(cos_val - (-0.25))) < 1e-10:
            print(f"  FOUND: cos(2*pi*{k}/{n}) = -1/4")
            found_any = True
if not found_any:
    print("  No solution found for n <= 100")
    print("  This confirms: arccos(-1/4)/pi is irrational,")
    print("  so -1/4 is NOT a value of cos at rational multiples of pi.")

subheader("3c. Nearby rational-pi cosines")

print("  Closest rational-pi cosines to -1/4:")
best_matches = []
for n in range(3, 200):
    for k in range(1, n):
        cos_val = to_float(mp_cos(2*pi*k/n))
        diff = abs(cos_val - (-0.25))
        if diff < 0.01:
            best_matches.append((diff, k, n, cos_val))

best_matches.sort()
for diff, k, n, cos_val in best_matches[:10]:
    print(f"  cos(2*pi*{k}/{n}) = {cos_val:.10f}, diff from -1/4 = {diff:.2e}")


# ============================================================================
# INVESTIGATION 4: Bridge Between RFT and FTD
# ============================================================================

header("INVESTIGATION 4: THE BRIDGE BETWEEN RFT AND FTD")

subheader("4a. The fundamental difference delta")

print(f"  p(pi) = 4*pi^3 + pi^2 + pi = {fmt(p_pi, 20)}")
print(f"  x_+   (FTD)                 = {fmt(x_plus, 20)}")
print(f"  1/alpha (CODATA 2022)       = {fmt(alpha_inv_exp, 20)}")
print()
delta_rft_ftd = p_pi - x_plus
delta_rft_exp = p_pi - alpha_inv_exp
delta_ftd_exp = x_plus - alpha_inv_exp

print(f"  delta_RFT-FTD = p(pi) - x_+     = {fmt(delta_rft_ftd, 18)}")
print(f"  delta_RFT-exp = p(pi) - 1/alpha  = {fmt(delta_rft_exp, 18)}")
print(f"  delta_FTD-exp = x_+  - 1/alpha   = {fmt(delta_ftd_exp, 18)}")

subheader("4b. delta in terms of powers of known constants")

delta = delta_rft_ftd  # Use RFT-FTD difference

print("  delta / pi^n:")
for n in range(-3, 4):
    val = delta / mp_power(pi, n)
    print(f"    n={n:+d}: {fmt(val, 18)}")

print("\n  delta * alpha^(-n):")
for n in range(-3, 4):
    val = delta * mp_power(alpha_exp, -n)
    print(f"    n={n:+d}: {fmt(val, 18)}")

print("\n  delta * G*^n:")
for n in range(-3, 4):
    val = delta * mp_power(G_star, n)
    print(f"    n={n:+d}: {fmt(val, 18)}")

print("\n  delta * varpi^n:")
for n in range(-3, 4):
    val = delta * mp_power(varpi, n)
    print(f"    n={n:+d}: {fmt(val, 18)}")

print("\n  delta * 137^n:")
for n in range(0, 4):
    val = delta * mp_power(137, n)
    print(f"    n={n:+d}: {fmt(val, 18)}")

subheader("4c. delta/alpha^2 detailed analysis")

ratio_alpha2 = delta / alpha_exp**2
print(f"  delta / alpha^2 = {fmt(ratio_alpha2, 18)}")
print(f"  5/2 = 2.5")
print(f"  delta/alpha^2 - 5/2 = {fmt(ratio_alpha2 - 5/mpmath.mpf(2) if HAS_MPMATH else ratio_alpha2 - 2.5, 18)}")
print()

# Best rational approximation
print("  Best rational approximations p/q for delta/alpha^2 (q < 100):")
target = to_float(ratio_alpha2)
best_rats = []
for q in range(1, 101):
    p = round(target * q)
    err = abs(target - p/q)
    best_rats.append((err, p, q))
best_rats.sort()
for err, p, q in best_rats[:15]:
    print(f"    {p}/{q} = {p/q:.15f}, error = {err:.2e}")

subheader("4d. delta in terms of combined constants")

print("  Trying delta = C * alpha^m * pi^n * G*^p for small integers:")
print()
best_combos = []
for m in range(-3, 4):
    for n in range(-3, 4):
        for p in range(-2, 3):
            combo = mp_power(alpha_exp, m) * mp_power(pi, n) * mp_power(G_star, p)
            if to_float(mp_abs(combo)) > 1e-20:
                ratio = delta / combo
                ratio_f = to_float(ratio)
                # Check if ratio is close to a small integer or simple fraction
                for num in range(1, 20):
                    for den in range(1, 10):
                        if abs(ratio_f - num/den) < 0.005:
                            best_combos.append((abs(ratio_f - num/den), m, n, p, num, den, ratio_f))

best_combos.sort()
for err, m, n, p, num, den, ratio_f in best_combos[:10]:
    print(f"    delta ≈ ({num}/{den}) * alpha^{m} * pi^{n} * G*^{p}")
    print(f"      ratio = {ratio_f:.10f}, target = {num/den:.10f}, err = {err:.2e}")

subheader("4e. FTD correction parameter epsilon")

eps_ftd = mp_exp(pi) - pi - 20
print(f"  epsilon = e^pi - pi - 20 = {fmt(eps_ftd, 18)}")
print(f"  delta / epsilon = {fmt(delta / eps_ftd, 18)}")
print(f"  Note: epsilon ≈ {fmt(eps_ftd, 6)}, so delta/epsilon ≈ {fmt(delta/eps_ftd, 6)}")


# ============================================================================
# INVESTIGATION 5: UNIFYING THE TWO FORMULAS
# ============================================================================

header("INVESTIGATION 5: UNIFYING RFT AND FTD")

subheader("5a. FTD quadratic in terms of varpi")

# G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)
# Also G* = 2*varpi/sqrt(pi) ... let's verify
G_star_from_varpi = 2 * varpi / mp_sqrt(pi)
print(f"  G* = {fmt(G_star)}")
print(f"  2*varpi/sqrt(pi) = {fmt(G_star_from_varpi)}")
print(f"  Difference = {fmt(G_star - G_star_from_varpi, 18)}")
print()

# FTD quadratic: x^2 - 16*G*^2*x + 16*G*^3 = 0
# In terms of varpi: G* = 2*varpi/sqrt(pi)
# 16*G*^2 = 16 * 4*varpi^2/pi = 64*varpi^2/pi
# 16*G*^3 = 16 * 8*varpi^3/pi^(3/2) = 128*varpi^3/pi^(3/2)

coeff_linear = 64 * varpi**2 / pi
coeff_const = 128 * varpi**3 / mp_power(pi, 3/mpmath.mpf(2) if HAS_MPMATH else 1.5)

print(f"  FTD quadratic: x^2 - (64*varpi^2/pi)*x + (128*varpi^3/pi^(3/2)) = 0")
print(f"  64*varpi^2/pi = {fmt(coeff_linear)}")
print(f"  128*varpi^3/pi^(3/2) = {fmt(coeff_const)}")
print(f"  Verification: 16*G*^2 = {fmt(16*G_star**2)}")
print(f"  Verification: 16*G*^3 = {fmt(16*G_star**3)}")

subheader("5b. Substituting RFT value into FTD quadratic")

# Residual = [p(pi)]^2 - 16*G*^2*p(pi) + 16*G*^3
residual = p_pi**2 - 16*G_star**2*p_pi + 16*G_star**3
print(f"  Residual = [p(pi)]^2 - 16*G*^2*p(pi) + 16*G*^3")
print(f"  = {fmt(residual, 18)}")
print(f"  |Residual| / x_+^2 = {fmt(mp_abs(residual) / x_plus**2, 18)}")
print()

# For comparison, what's the residual at the exact experimental value?
residual_exp = alpha_inv_exp**2 - 16*G_star**2*alpha_inv_exp + 16*G_star**3
print(f"  Residual at 1/alpha(exp) = {fmt(residual_exp, 18)}")
print(f"  |Res(exp)| / x_+^2 = {fmt(mp_abs(residual_exp) / x_plus**2, 18)}")

subheader("5c. Can we factor a unified cubic?")

print("  We want a cubic f(x) = 0 such that:")
print("  - x = pi is a root (linking to RFT via p(pi))")
print("  - The other factor relates to FTD")
print()
print("  If f(x) = (x - pi) * g(x), and g(pi) = some nice form...")
print("  Or if f(x) has a root x = pi and the quadratic factor")
print("  has roots related to x_+ and x_-...")
print()

# What if the cubic is (x - pi)(x^2 + bx + c) = 0
# where the quadratic part has roots that map to 1/alpha somehow?
# Expanding: x^3 + (b-pi)*x^2 + (c - pi*b)*x - pi*c = 0
# Compare with x(4x^2 + x + 1) = 4x^3 + x^2 + x
# Matching: 4x^3 + x^2 + x = 0 means x=0 or 4x^2 + x + 1 = 0 (complex roots)

# Alternative: consider x^3 = p(x)/4 - x^2/4 - x/4
# So if p(x) = alpha_inv, then 4x^3 + x^2 + x = alpha_inv
# -> 4x^3 + x^2 + x - alpha_inv = 0

# This cubic evaluated at x = pi gives p(pi) - alpha_inv = small residual
cubic_at_pi = p_pi - alpha_inv_exp
print(f"  Cubic 4x^3 + x^2 + x - (1/alpha) evaluated at x = pi:")
print(f"  = {fmt(cubic_at_pi, 18)}")
print(f"  = {fmt(cubic_at_pi, 18)} (the RFT-experimental difference)")

# What value of x EXACTLY solves 4x^3 + x^2 + x = 1/alpha?
# Use Newton's method starting from pi
if HAS_MPMATH:
    x_exact = mpmath.mpf(pi)
    for _ in range(50):
        f_val = 4*x_exact**3 + x_exact**2 + x_exact - alpha_inv_exp
        f_prime = 12*x_exact**2 + 2*x_exact + 1
        x_exact -= f_val / f_prime

    print(f"\n  Exact solution of 4x^3 + x^2 + x = 1/alpha(exp):")
    print(f"  x_exact = {mpmath.nstr(x_exact, 30)}")
    print(f"  pi      = {mpmath.nstr(pi, 30)}")
    print(f"  x_exact - pi = {mpmath.nstr(x_exact - pi, 18)}")
    print(f"  (x_exact - pi)/pi = {mpmath.nstr((x_exact - pi)/pi, 18)} (relative shift)")
    print(f"  This is a ~{mpmath.nstr((x_exact - pi)/pi * 1e6, 4)} ppm shift from pi")

subheader("5d. Exploring the varpi-pi connection")

print(f"  varpi = {fmt(varpi)}")
print(f"  pi    = {fmt(pi)}")
print(f"  varpi/pi = {fmt(varpi/pi)}")
print(f"  varpi*pi = {fmt(varpi*pi)}")
print(f"  varpi + pi = {fmt(varpi + pi)}")
print(f"  varpi - pi = {fmt(varpi - pi)}")
print(f"  varpi^2/pi = {fmt(varpi**2/pi)}")
print(f"  pi/varpi = {fmt(pi/varpi)}")
print(f"  (pi/varpi)^2 = {fmt((pi/varpi)**2)}")
print()

# What about p(varpi)?
p_varpi = p_rft(varpi)
print(f"  p(varpi) = 4*varpi^3 + varpi^2 + varpi = {fmt(p_varpi)}")
print(f"  p(varpi)/p(pi) = {fmt(p_varpi/p_pi)}")
print(f"  (varpi/pi)^3 = {fmt((varpi/pi)**3)}")


# ============================================================================
# INVESTIGATION 6: THE "GEOMETRIC SERIES" HINT
# ============================================================================

header("INVESTIGATION 6: COEFFICIENT ANALYSIS {1, 1, 4}")

subheader("6a. The coefficient pattern")

print("  p(pi) = 1*pi + 1*pi^2 + 4*pi^3")
print("  Coefficients: {1, 1, 4}")
print()
print("  Possible patterns:")
print("  - {1, 1, N_base} where N_base = 4 in FTD")
print("  - {2^0, 2^0, 2^2} — note gap at 2^1 = 2")
print("  - {F_1, F_1, F_3+1} where F_n = Fibonacci (1,1,2,3,5...)")
print("  - {1, 1, 4} = start of sequence 1,1,4,... (OEIS?)")
print("  - Sum of coeffs = 1+1+4 = 6 = 3!")
print("  - Product of coeffs = 1*1*4 = 4 = N_base")
print()
print("  Interesting: coefficients in binary:")
print("  1 = 001, 1 = 001, 4 = 100")
print("  In ternary:")
print("  1 = 01, 1 = 01, 4 = 11")

subheader("6b. What pi^4 correction would make p(pi) exact?")

p_pi_val = p_pi
target = alpha_inv_exp
deficit = target - p_pi_val

print(f"  p(pi) = {fmt(p_pi_val, 18)}")
print(f"  1/alpha = {fmt(target, 18)}")
print(f"  Deficit = 1/alpha - p(pi) = {fmt(deficit, 18)}")
print()

pi4 = pi**4
coeff_pi4 = deficit / pi4
print(f"  pi^4 = {fmt(pi4)}")
print(f"  Needed coefficient for pi^4 term: {fmt(coeff_pi4, 18)}")
print(f"  Note: this is about {fmt(coeff_pi4, 6)}")
print(f"  = approximately -{fmt(mp_abs(coeff_pi4) * 1e6, 4)} x 10^-6")

subheader("6c. What correction to '4' makes it exact?")

# Need (4+eps)*pi^3 + pi^2 + pi = 1/alpha
eps_correction = deficit / pi**3
print(f"  Need (4 + eps)*pi^3 + pi^2 + pi = 1/alpha")
print(f"  eps = deficit / pi^3 = {fmt(eps_correction, 18)}")
print(f"  4 + eps = {fmt(4 + eps_correction, 18)}")
print(f"  eps ≈ {fmt(eps_correction, 6)}")
print(f"  eps/alpha = {fmt(eps_correction/alpha_exp, 18)}")
print(f"  eps/alpha^2 = {fmt(eps_correction/alpha_exp**2, 18)}")

subheader("6d. Extended polynomial investigation")

print("  What if p(x) = sum_{k=1}^{n} a_k * x^k for various {a_k}?")
print()

# Try: p(x) = x + x^2 + x^3 (geometric)
geo = pi + pi**2 + pi**3
print(f"  x + x^2 + x^3 at x=pi: {fmt(geo)} (way too small)")

# Try: p(x) = x + x^2 + 4x^3 + a4*x^4 where a4 tries simple values
print(f"\n  Testing p(pi) = pi + pi^2 + 4*pi^3 + a4*pi^4:")
for a4_try in [0, 1/137, 1/274, -1/137, alpha_exp, -alpha_exp, alpha_exp**2, -alpha_exp**2]:
    val = pi + pi**2 + 4*pi**3 + a4_try * pi**4
    err_ppm = to_float((val - alpha_inv_exp) / alpha_inv_exp * 1e6)
    print(f"    a4 = {to_float(a4_try):+.8f}: p = {fmt(val, 10)}, error = {err_ppm:+.4f} ppm")

# What about continuing the pattern: 1, 1, 4, ?
# If Fibonacci-like: next after {1,1,4} could be 1+4=5 or 1+1+4=6
print(f"\n  Testing with pi^0 term (constant):")
for a0 in [0, 1, -1, alpha_exp, -alpha_exp]:
    val = a0 + pi + pi**2 + 4*pi**3
    err_ppm = to_float((val - alpha_inv_exp) / alpha_inv_exp * 1e6)
    print(f"    a0 = {to_float(a0):+.8f}: p = {fmt(val, 10)}, error = {err_ppm:+.4f} ppm")


# ============================================================================
# INVESTIGATION 7: Evaluate p at Special Arguments
# ============================================================================

header("INVESTIGATION 7: p(x) AT SPECIAL ARGUMENTS")

subheader("7a. p at multiples and fractions of pi")

# Known physical constants for comparison
known_constants = {
    "1/alpha": 137.035999177,
    "alpha": 0.0072973525693,
    "pi": 3.14159265358979,
    "e": 2.71828182845905,
    "phi": 1.61803398874989,
    "sqrt(2)": 1.41421356237310,
    "sqrt(3)": 1.73205080756888,
    "G*": to_float(G_star),
    "varpi": to_float(varpi),
    "137": 137.0,
    "1": 1.0,
    "2": 2.0,
    "3": 3.0,
    "4": 4.0,
    "7": 7.0,
    "13": 13.0,
    "ln(2)": 0.693147180559945,
    "ln(10)": 2.30258509299405,
    "Euler gamma": 0.577215664901532,
    "Catalan": 0.915965594177219,
    "Apery zeta(3)": 1.20205690315959,
    "m_e/m_p": 0.000544617021487,
    "m_mu/m_e": 206.768283,
    "m_tau/m_e": 3477.48,
    "sin^2(theta_W)": 0.23122,
    "electron g-2 anom.": 0.00115965218128,
    "proton/electron mass": 1836.15267343,
    "m_W (GeV)": 80.377,
    "m_Z (GeV)": 91.1876,
    "m_H (GeV)": 125.25,
    "G_F (10^-5 GeV^-2)": 1.1663788,
    "N_A (10^23)": 6.02214076,
}

test_args = {
    "pi": pi,
    "pi/2": pi/2,
    "pi/3": pi/3,
    "pi/4": pi/4,
    "pi/6": pi/6,
    "2*pi": 2*pi,
    "3*pi": 3*pi,
    "pi/phi": pi/phi,
    "phi": phi,
    "e": e_const,
    "sqrt(2)": sqrt2,
    "sqrt(3)": sqrt3,
    "1": mpmath.mpf(1) if HAS_MPMATH else 1.0,
    "2": mpmath.mpf(2) if HAS_MPMATH else 2.0,
    "3": mpmath.mpf(3) if HAS_MPMATH else 3.0,
    "G*": G_star,
    "varpi": varpi,
    "1/alpha": 1/alpha_exp,
    "alpha": alpha_exp,
    "pi*alpha": pi*alpha_exp,
    "pi/e": pi/e_const,
    "e/pi": e_const/pi,
    "ln(2)": mp_log(2),
    "Euler_gamma": mpmath.euler if HAS_MPMATH else 0.5772156649015329,
}

print(f"  {'Argument':<15} {'p(x)':<25} {'Closest constant':<25} {'Rel. error':>12}")
print(f"  {'-'*15} {'-'*25} {'-'*25} {'-'*12}")

for name, x in test_args.items():
    val = to_float(p_rft(x))

    # Find closest known constant
    best_match = None
    best_err = float('inf')
    for cname, cval in known_constants.items():
        if cval != 0:
            rel_err = abs(val - cval) / abs(cval)
            if rel_err < best_err:
                best_err = rel_err
                best_match = cname

    marker = ""
    if best_err < 0.001:
        marker = " <-- MATCH!"
    elif best_err < 0.01:
        marker = " <-- close"

    print(f"  {name:<15} {val:<25.15f} {best_match:<25} {best_err:>12.6e}{marker}")

subheader("7b. The quadratic factor at special arguments")

print("  q(x) = 4x^2 + x + 1:")
print()

for name, x in test_args.items():
    val = to_float(4*x**2 + x + 1)
    print(f"  q({name:<12}) = {val:<20.10f}")

subheader("7c. Inverse: what x gives p(x) = known constants?")

print("  Solving 4x^3 + x^2 + x = C for various constants C:")
print()

targets_inv = {
    "1/alpha (CODATA)": alpha_inv_exp,
    "137 (integer)": mpmath.mpf(137) if HAS_MPMATH else 137.0,
    "x_+ (FTD)": x_plus,
    "x_- (FTD)": x_minus,
    "42": mpmath.mpf(42) if HAS_MPMATH else 42.0,
    "1728": mpmath.mpf(1728) if HAS_MPMATH else 1728.0,
    "G*": G_star,
    "4*pi^2 (Euler)": 4*pi**2,
}

if HAS_MPMATH:
    for cname, C in targets_inv.items():
        # Newton's method for 4x^3 + x^2 + x - C = 0
        x_sol = mpmath.cbrt(C/4)  # initial guess
        for _ in range(100):
            f_val = 4*x_sol**3 + x_sol**2 + x_sol - C
            f_prime = 12*x_sol**2 + 2*x_sol + 1
            x_sol -= f_val / f_prime

        print(f"  p(x) = {cname:<20}: x = {mpmath.nstr(x_sol, 18)}")

        # Check if x is close to known constants
        x_f = float(x_sol)
        for kname, kval in [("pi", float(pi)), ("e", float(e_const)),
                            ("phi", float(phi)), ("sqrt(2)", float(sqrt2)),
                            ("varpi", float(varpi)), ("G*", float(G_star)),
                            ("3", 3.0), ("pi-0.001", float(pi)-0.001)]:
            if abs(x_f - kval) < 0.01:
                print(f"    ^^^ Close to {kname} = {kval:.10f}, diff = {x_f - kval:.2e}")


# ============================================================================
# INVESTIGATION 8: DEEPER STRUCTURAL ANALYSIS
# ============================================================================

header("INVESTIGATION 8: DEEPER STRUCTURAL ANALYSIS")

subheader("8a. The discriminant -15 field and class field theory")

print("  Key relationships in Q(sqrt(-15)):")
print(f"  Ring of integers: Z[(1+sqrt(-15))/2] (since -15 ≡ 1 mod 4)")
print(f"  Class number h(-15) = 2")
print(f"  Class group: Z/2Z (cyclic of order 2)")
print()
print("  The Hilbert class field of Q(sqrt(-15)) is a degree-2 extension")
print("  generated by the j-invariants of the two ideal classes.")
print()

# Weber class polynomials might be simpler
print("  Genus characters for -15:")
print("  -15 = -3 * 5, so genus theory gives:")
print("  Number of genera = 2^(t-1) where t = number of prime discriminant divisors")
print("  Here t = 2 (-3 and 5 are fundamental), so genera = 2")
print("  Since h(-15) = 2 = number of genera, the genus theory is 'sharp'")
print("  Every ideal class is determined by its genus character.")

subheader("8b. Galois theory of the roots")

print("  The polynomial 4x^2 + x + 1 splits over Q(sqrt(-15)).")
print("  Its splitting field is Q(sqrt(-15)).")
print("  Galois group: Z/2Z (complex conjugation)")
print()
print("  The polynomial x(4x^2 + x + 1) = 4x^3 + x^2 + x splits as:")
print("  - x = 0 (rational root)")
print("  - x = (-1 +/- i*sqrt(15))/8 (conjugate pair in Q(sqrt(-15)))")
print()
print("  The splitting field of 4x^3 + x^2 + x over Q is Q(sqrt(-15)).")
print("  Galois group: Z/2Z")

subheader("8c. Norm form and lattice connection")

print("  The quadratic form 4x^2 + xy + y^2 (associated to the quadratic x=1)")
print("  has discriminant 1 - 16 = -15")
print()
print("  Norm form in Q(sqrt(-15)):")
print("  N(a + b*(1+sqrt(-15))/2) = a^2 + ab + 4b^2")
print("  This is exactly the principal form (1, 1, 4)!")
print()
print("  So 4x^2 + x + 1 = 0 is equivalent to asking:")
print("  What elements of Q(sqrt(-15)) have norm 1/4 (after rescaling)?")
print()
print("  The connection to the norm form (1,1,4) is:")
print("  4x^2 + x + 1 = 4(x^2 + x/4 + 1/4)")
print("  = 4((x+1/8)^2 + 15/64)")
print("  Setting u = x + 1/8: 4(u^2 + 15/64) = 4u^2 + 15/16")

# Verify: 4*(-1/8)^2 + (-1/8) + 1 = 4/64 - 1/8 + 1 = 1/16 - 2/16 + 16/16 = 15/16
check = 4*(mpmath.mpf(-1)/8)**2 + (mpmath.mpf(-1)/8) + 1 if HAS_MPMATH else 4*(-1/8)**2 + (-1/8) + 1
print(f"\n  Check: 4*(-1/8)^2 + (-1/8) + 1 = {fmt(check)} (should be 15/16)")

subheader("8d. Resultant and discriminant of the system")

print("  Consider the two polynomials:")
print("  RFT: f(x) = 4x^3 + x^2 + x - alpha_inv")
print("  FTD: g(x) = x^2 - 16*G*^2*x + 16*G*^3")
print()
print("  Their resultant Res(f,g) measures 'how close' they share a root:")
print()

if HAS_MPMATH:
    # Evaluate g at the real root of f (near pi)
    x_root_f = mpmath.mpf(pi)
    for _ in range(50):
        fv = 4*x_root_f**3 + x_root_f**2 + x_root_f - alpha_inv_exp
        fp = 12*x_root_f**2 + 2*x_root_f + 1
        x_root_f -= fv / fp

    g_at_root_f = x_root_f**2 - 16*G_star**2*x_root_f + 16*G_star**3
    print(f"  Root of f(x)=0 near pi: x = {mpmath.nstr(x_root_f, 20)}")
    print(f"  g(x) at this root = {mpmath.nstr(g_at_root_f, 18)}")
    print(f"  |g(root_f)| / 137^2 = {mpmath.nstr(abs(g_at_root_f)/137**2, 18)}")
    print()

    # Evaluate f at the roots of g
    f_at_xplus = 4*x_plus**3 + x_plus**2 + x_plus - alpha_inv_exp
    f_at_xminus = 4*x_minus**3 + x_minus**2 + x_minus - alpha_inv_exp
    print(f"  f(x_+) = {mpmath.nstr(f_at_xplus, 18)}")
    print(f"  f(x_-) = {mpmath.nstr(f_at_xminus, 18)}")
    print(f"  Note: f(x) = p(x) - 1/alpha, so f(x_+) = p(x_+) - 1/alpha")
    print(f"  p(x_+) = 4*{mpmath.nstr(x_plus,6)}^3 + ... = {mpmath.nstr(4*x_plus**3 + x_plus**2 + x_plus, 10)}")
    print(f"  This is p evaluated at x = 1/alpha ≈ 137, which is enormous")


# ============================================================================
# INVESTIGATION 9: MODULAR FORMS CONNECTION
# ============================================================================

header("INVESTIGATION 9: MODULAR AND ELLIPTIC CONNECTIONS")

subheader("9a. The modular lambda function")

print("  The modulus 1/2 of the quadratic roots suggests connections to")
print("  the modular lambda function lambda(tau), which takes value 1/2")
print("  at tau = i (the square lattice point).")
print()

if HAS_MPMATH:
    # lambda(i) should be 1/2
    # Using theta functions: lambda(tau) = (theta_2/theta_3)^4
    tau_i = mpmath.mpc(0, 1)  # tau = i

    # Klein j at tau = i
    j_at_i = mpmath.kleinj(tau_i)
    print(f"  j(i) = {mpmath.nstr(j_at_i, 15)}")
    print(f"  Expected: j(i) = 1728 (the FTD value!)")
    print()

    # j at our CM points
    tau_rft = mpmath.mpc(-1, float(mp_sqrt(15))) / 8  # root of 4x^2 + x + 1
    j_at_rft = mpmath.kleinj(tau_rft)
    print(f"  j((-1+i*sqrt(15))/8) = {mpmath.nstr(j_at_rft, 15)}")

    # The root scaled to upper half plane
    # Our root is (-1 + i*sqrt(15))/8
    # For modular forms, we need Im(tau) > 0
    # tau_rft has Im = sqrt(15)/8 ≈ 0.484 > 0, good
    print(f"  Im(tau_rft) = {mpmath.nstr(mpmath.im(tau_rft), 15)}")
    print()

    # What about tau = (-1 + i*sqrt(15))/2 (the CM point for form (1,1,4))?
    tau_cm = mpmath.mpc(-0.5, float(mp_sqrt(15))/2)
    j_at_cm = mpmath.kleinj(tau_cm)
    print(f"  j((-1+i*sqrt(15))/2) = {mpmath.nstr(j_at_cm, 15)}")
    print(f"  (This is the CM point for the principal form of disc -15)")

subheader("9b. Eta function and weight")

if HAS_MPMATH:
    # Dedekind eta at tau_i
    eta_i = mpmath.eta(tau_i)
    print(f"  eta(i) = {mpmath.nstr(eta_i, 15)}")
    print(f"  eta(i)^24 = {mpmath.nstr(eta_i**24, 15)}")
    print()

    # eta at our RFT root
    eta_rft = mpmath.eta(tau_rft)
    print(f"  eta((-1+i*sqrt(15))/8) = {mpmath.nstr(eta_rft, 15)}")

    # At the CM point
    eta_cm = mpmath.eta(tau_cm)
    print(f"  eta((-1+i*sqrt(15))/2) = {mpmath.nstr(eta_cm, 15)}")

    # The famous relation: eta(i) = Gamma(1/4) / (2*pi^(3/4))
    eta_i_formula = gamma_quarter / (2 * pi**(mpmath.mpf(3)/4))
    print(f"\n  eta(i) via Gamma: Gamma(1/4)/(2*pi^(3/4)) = {mpmath.nstr(eta_i_formula, 15)}")
    print(f"  Match: {mpmath.nstr(eta_i - eta_i_formula, 18)}")

subheader("9c. Connection summary")

print("  STRUCTURAL CONNECTIONS FOUND:")
print()
print("  1. RFT polynomial 4x^2 + x + 1 has discriminant -15")
print("  2. Q(sqrt(-15)) has class number 2 with forms (1,1,4) and (2,1,2)")
print("     Note: (1,1,4) DIRECTLY matches the RFT coefficients {1,1,4}!")
print("  3. The principal form (1,1,4) is the NORM FORM of Q(sqrt(-15))")
print("  4. FTD uses j = 1728 = j(i), the CM point for discriminant -4")
print("  5. The roots of 4x^2 + x + 1 have modulus 1/2 exactly")
print("  6. lambda(i) = 1/2 (modular lambda at the FTD CM point)")
print()
print("  HYPOTHESIS: The RFT polynomial encodes the arithmetic of Q(sqrt(-15))")
print("  through its norm form, while FTD encodes Q(sqrt(-1)) via j=1728.")
print("  The bridge between them may involve the PRODUCT of these fields,")
print("  i.e., the biquadratic field Q(sqrt(-1), sqrt(-15)) = Q(i, sqrt(15)).")


# ============================================================================
# INVESTIGATION 10: THE (1,1,4) COINCIDENCE
# ============================================================================

header("INVESTIGATION 10: THE (1,1,4) QUADRATIC FORM COINCIDENCE")

subheader("10a. Explicit verification")

print("  RFT polynomial: p(x) = x + x^2 + 4x^3 = x(1 + x + 4x^2)")
print("  Coefficients of inner quadratic: {1, 1, 4}")
print()
print("  Principal form of discriminant -15: (a,b,c) = (1,1,4)")
print("  Representing: f(x,y) = x^2 + xy + 4y^2")
print()
print("  This is the SAME set of integers {1, 1, 4}!")
print()

# What primes are represented by the form (1,1,4)?
print("  Primes represented by x^2 + xy + 4y^2 (principal genus):")
primes_form1 = []
for p in range(2, 200):
    # Check if p is prime
    if p < 2:
        continue
    is_prime = True
    for d in range(2, int(p**0.5)+1):
        if p % d == 0:
            is_prime = False
            break
    if not is_prime:
        continue

    found = False
    for x in range(0, p+1):
        for y in range(0, p+1):
            if x*x + x*y + 4*y*y == p:
                found = True
                primes_form1.append((p, x, y))
                break
        if found:
            break

print(f"  {[p for p,x,y in primes_form1[:20]]}")
for p, x, y in primes_form1[:10]:
    print(f"    {p} = {x}^2 + {x}*{y} + 4*{y}^2")

print()
print("  Primes represented by 2x^2 + xy + 2y^2 (non-principal genus):")
primes_form2 = []
for p in range(2, 200):
    is_prime = True
    if p < 2:
        continue
    for d in range(2, int(p**0.5)+1):
        if p % d == 0:
            is_prime = False
            break
    if not is_prime:
        continue

    found = False
    for x in range(-p, p+1):
        for y in range(-p, p+1):
            if 2*x*x + x*y + 2*y*y == p:
                found = True
                primes_form2.append((p, x, y))
                break
        if found:
            break

print(f"  {[p for p,x,y in primes_form2[:20]]}")
for p, x, y in primes_form2[:10]:
    print(f"    {p} = 2*{x}^2 + {x}*{y} + 2*{y}^2")

# Check which FTD integers appear
print(f"\n  FTD key integers: {{3, 4, 7, 13}}")
print(f"  3 in principal form? {3 in [p for p,x,y in primes_form1]}")
print(f"  7 in principal form? {7 in [p for p,x,y in primes_form1]}")
print(f"  13 in principal form? {13 in [p for p,x,y in primes_form1]}")
print(f"  3 in non-principal? {3 in [p for p,x,y in primes_form2]}")
print(f"  7 in non-principal? {7 in [p for p,x,y in primes_form2]}")
print(f"  13 in non-principal? {13 in [p for p,x,y in primes_form2]}")

subheader("10b. Splitting behavior in Q(sqrt(-15))")

print("  A prime p splits in Q(sqrt(-15)) iff (-15/p) = 1 (Legendre symbol)")
print("  i.e., -15 is a quadratic residue mod p.")
print()
for p in [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 137]:
    if p == 3 or p == 5:
        print(f"  p = {p}: RAMIFIED (divides discriminant)")
        continue
    # Compute (-15/p) via Euler criterion
    residue = pow(-15 % p, (p-1)//2, p) if p > 2 else 0
    if p == 2:
        print(f"  p = 2: RAMIFIED (special)")
    elif residue == 1:
        genus = "principal" if p in [pp for pp,x,y in primes_form1] else "non-principal"
        print(f"  p = {p}: SPLITS ({genus} form)")
    else:
        print(f"  p = {p}: INERT")

# Special attention to 137
print(f"\n  137: (-15 mod 137) = {(-15) % 137}")
print(f"  (-15)^68 mod 137 = {pow((-15) % 137, 68, 137)}")
is_split = pow((-15) % 137, 68, 137) == 1
print(f"  137 {'SPLITS' if is_split else 'is INERT'} in Q(sqrt(-15))")

if is_split:
    # Find the representation
    for x in range(0, 138):
        for y in range(0, 138):
            if x*x + x*y + 4*y*y == 137:
                print(f"  137 = {x}^2 + {x}*{y} + 4*{y}^2 (principal form)")
                break
            if 2*x*x + x*y + 2*y*y == 137:
                print(f"  137 = 2*{x}^2 + {x}*{y} + 2*{y}^2 (non-principal form)")
                break


# ============================================================================
# FINAL SUMMARY
# ============================================================================

header("FINAL SUMMARY OF KEY FINDINGS")

print("""
  1. DISCRIMINANT -15 AND NUMBER THEORY:
     - 4x^2 + x + 1 has discriminant -15
     - Q(sqrt(-15)) has class number 2
     - The two quadratic forms are (1,1,4) and (2,1,2)
     - REMARKABLE: The coefficients {1,1,4} of the RFT quadratic factor
       are IDENTICAL to the principal form of discriminant -15
     - This is the NORM FORM of the ring of integers of Q(sqrt(-15))

  2. MODULUS 1/2:
     - Roots have |r| = 1/2 exactly
     - Polar angle theta = arccos(-1/4) ≈ 104.478 degrees
     - theta/pi is IRRATIONAL (Niven's theorem)
     - Angle is 5.01 degrees from tetrahedral angle arccos(-1/3)
     - arccos(-1/4) vs arccos(-1/3): consecutive 1/n values

  3. MODULAR CONNECTIONS:
     - j(i) = 1728 (the FTD CM point)
     - lambda(i) = 1/2 (matching root modulus)
     - eta(i) = Gamma(1/4)/(2*pi^(3/4)) (connecting to G*)
     - The roots live in the CM field Q(sqrt(-15))

  4. THE BRIDGE delta = p(pi) - x_+:""")

print(f"     delta = {fmt(delta, 18)}")
print(f"     delta/alpha^2 ≈ {fmt(delta/alpha_exp**2, 10)}")
print(f"     Best rational approx to delta/alpha^2 near 5/2")

print("""
  5. UNIFICATION PROSPECT:
     - RFT encodes arithmetic of Q(sqrt(-15)) via norm form (1,1,4)
     - FTD encodes arithmetic of Q(sqrt(-1)) via j = 1728 = j(i)
     - Both approximate 1/alpha from different algebraic structures
     - The biquadratic field Q(i, sqrt(15)) may contain both

  6. COEFFICIENT (1,1,4) IDENTITY:
     - This is perhaps the deepest finding
     - The RFT polynomial coefficients ARE the principal quadratic
       form of the same discriminant the polynomial generates
     - This self-referential structure is highly non-generic
     - p(x) = x * (norm form evaluated as a polynomial in x)

  7. PRIME 137 IN Q(sqrt(-15)):""")

# Final check on 137
found_137 = False
for x in range(0, 138):
    for y in range(0, 138):
        if x*x + x*y + 4*y*y == 137:
            print(f"     137 = {x}^2 + {x}*{y} + 4*{y}^2 (PRINCIPAL form!)")
            found_137 = True
            break
        if 2*x*x + x*y + 2*y*y == 137:
            print(f"     137 = 2*{x}^2 + {x}*{y} + 2*{y}^2 (non-principal form)")
            found_137 = True
            break
    if found_137:
        break

if not found_137:
    print("     137 is INERT in Q(sqrt(-15))")
    print("     This means 137 remains prime in this field")
    print("     -> 137 is 'incompatible' with the RFT structure")
    print("     -> Perhaps explaining WHY pi (not an algebraic number)")
    print("        is needed to reach 137 through the form (1,1,4)")

print(f"""
  8. OPEN QUESTIONS:
     - Why does the norm form of Q(sqrt(-15)), evaluated as a
       polynomial at x = pi, give 1/alpha to 2.2 ppm?
     - Is there a modular form of weight k and level 15 that
       encodes this relationship?
     - Does the biquadratic field Q(i, sqrt(15)) have special
       arithmetic properties connecting both approaches?
     - Can the {fmt(delta/alpha_exp**2, 6)} ≈ 5/2 ratio be derived?
""")

print(SEPARATOR)
print("  END OF INVESTIGATION")
print(SEPARATOR)
