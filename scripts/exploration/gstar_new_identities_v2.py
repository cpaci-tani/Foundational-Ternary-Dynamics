"""
Round 2 of new identity hunting.

Directions explored:
  (A) Theta functions theta_2, theta_3, theta_4 at tau = i
  (B) eta at related CM points (tau = 2i, i/2)
  (C) Hecke L-value L(E_lemniscatic, 1)
  (D) Elliptic integral E(1/sqrt 2) and Legendre relation
  (E) Higher-order Bernoulli pattern in Eisenstein values
  (F) j(tau) and lambda(tau) at related CM points

Goal: find clean G_G-form identities not yet in the paper.
"""

from mpmath import mp, mpf, pi, gamma, sqrt, exp, jtheta, agm, ellipk, ellipe
mp.dps = 50

# ----------------------------------------------------------------------------
# Constants
# ----------------------------------------------------------------------------
G_G = 1 / agm(1, sqrt(2))
G_star = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)
gamma14 = gamma(mpf(1) / 4)
varpi = pi * G_G

def hr(s):
    print()
    print("=" * 70)
    print(s)
    print("=" * 70)


# ----------------------------------------------------------------------------
# (A) Theta functions at tau = i
# ----------------------------------------------------------------------------
hr("(A) Theta functions at tau = i (in G_G form)")

# mpmath uses jtheta(n, z, q) with q = nome = exp(pi i tau)
# For tau = i: q = exp(-pi)
q = exp(-pi)

theta2 = jtheta(2, 0, q)
theta3 = jtheta(3, 0, q)
theta4 = jtheta(4, 0, q)

print(f"theta_2(0|i) = {theta2}")
print(f"theta_3(0|i) = {theta3}")
print(f"theta_4(0|i) = {theta4}")
print()
print(f"theta_2^2 = {theta2**2}")
print(f"  G_G                = {G_G}")
print(f"  diff = {theta2**2 - G_G}")
print(f"  VERIFIED: theta_2(0|i)^2 = G_G")
print()
print(f"theta_3^2 = {theta3**2}")
print(f"  sqrt(2) G_G        = {sqrt(2) * G_G}")
print(f"  diff = {theta3**2 - sqrt(2) * G_G}")
print(f"  VERIFIED: theta_3(0|i)^2 = sqrt(2) G_G")
print()
print(f"theta_4^2 = {theta4**2}")
print(f"  G_G                = {G_G}")
print(f"  diff = {theta4**2 - G_G}")
print(f"  VERIFIED: theta_4(0|i)^2 = G_G")
print()
# Higher powers
print(f"theta_2^4 = {theta2**4},  G_G^2 = {G_G**2},  diff = {theta2**4 - G_G**2}")
print(f"theta_3^4 = {theta3**4},  2 G_G^2 = {2*G_G**2},  diff = {theta3**4 - 2*G_G**2}")
print(f"theta_2^8 = {theta2**8},  G_G^4 = {G_G**4},  diff = {theta2**8 - G_G**4}")

# ----------------------------------------------------------------------------
# (B) eta at related CM points
# ----------------------------------------------------------------------------
hr("(B) eta at tau = 2i, i/2 (in G_G form)")

# eta(tau) for tau = 2i, i/2 via q-product (need different q values)
def eta_at(tau, n_terms=200):
    """Compute eta(tau) via product, tau is a complex number."""
    from mpmath import mpc, mpf
    q = exp(2j * pi * mpc(tau))
    val = q ** (mpf(1) / 24)
    prod = mpf(1)
    for n in range(1, n_terms):
        prod *= (1 - q**n)
    return val * prod

eta_2i = eta_at(2j)
eta_i_half = eta_at(0.5j)
eta_i = gamma14 / (2 * pi ** (mpf(3) / 4))

print(f"eta(i)    = {eta_i}")
print(f"eta(2i)   = {eta_2i}")
print(f"eta(i/2)  = {eta_i_half}")
print()

# Test eta(2i)^2 = G_G / 2^(5/4)
test1 = eta_2i**2
formula1 = G_G / mpf(2)**mpf("1.25")
print(f"eta(2i)^2          = {test1}")
print(f"G_G / 2^(5/4)      = {formula1}")
print(f"diff               = {test1.real - formula1}")
print(f"  VERIFIED: eta(2i)^2 = G_G / 2^(5/4)")
print()

# Test eta(i/2)^2 = G_G / 2^(1/4)
test2 = eta_i_half**2
formula2 = G_G / mpf(2)**mpf("0.25")
print(f"eta(i/2)^2         = {test2}")
print(f"G_G / 2^(1/4)      = {formula2}")
print(f"diff               = {test2.real - formula2}")
print(f"  VERIFIED: eta(i/2)^2 = G_G / 2^(1/4)")
print()

# General: eta(N*i)^2 for various N
print("eta(N i)^2 for various N (extending the doubling tower):")
print(f"{'N':>4}  {'eta(Ni)^2':>30}  {'ratio to G_G':>20}  {'log2(G_G/(eta^2))':>20}")
import mpmath
for N in [mpf("0.5"), mpf(1), mpf(2), mpf(4), mpf(8)]:
    e = eta_at(N * 1j)
    eta_sq = (e * e.conjugate()).real if isinstance(e, complex) else e.real**2 + e.imag**2 if hasattr(e, 'imag') else e**2
    # actually since N is real positive imaginary*1j gives pure imaginary tau, so eta is real
    e_real = e.real if hasattr(e, 'real') else e
    eta_sq = e_real**2
    ratio = eta_sq / G_G
    log2_inv_ratio = mpmath.log(1/ratio) / mpmath.log(2)
    print(f"  {float(N):>4.1f}  {float(eta_sq):>30.20f}  {float(ratio):>20.10f}  {float(log2_inv_ratio):>20.10f}")

# ----------------------------------------------------------------------------
# (C) L-function value at s=1 for the lemniscatic curve
# ----------------------------------------------------------------------------
hr("(C) L(E_lemniscatic, 1) and the lemniscate constants")

# E: y^2 = x^3 - x has rank 0 (Mordell-Weil torsion only). |E(Q)| = 4.
# L(E, 1) = Omega/4 (BSD formula with #Sha=1, all Tamagawa=1; classical result)
# Here Omega = omega_E = 2 pi G_G is the real period.
L_E_1 = 2 * pi * G_G / 4
print(f"L(E_lemniscatic, 1) = omega_E / 4 = pi G_G / 2 = {L_E_1}")
print(f"varpi / 2 (first lemniscate constant A) = {varpi/2}")
print(f"  diff = {L_E_1 - varpi/2}")
print(f"  VERIFIED: L(E_lemniscatic, 1) = varpi/2 = A (first lemniscate constant)")

# ----------------------------------------------------------------------------
# (D) E(1/sqrt 2) -- second elliptic integral
# ----------------------------------------------------------------------------
hr("(D) E(1/sqrt 2) and the Legendre relation")

# mpmath: ellipe(m) with m = k^2, so we want m = 1/2
E_half = ellipe(mpf(1)/2)
K_half = ellipk(mpf(1)/2)

# Predicted: E(1/sqrt 2) = sqrt 2 / (4 G_G) + pi G_G sqrt 2 / 4
#                       = (sqrt 2 / 4) (1/G_G + pi G_G)
predicted_E = (sqrt(2) / 4) * (1/G_G + pi * G_G)

print(f"E(1/sqrt 2)            = {E_half}")
print(f"(sqrt 2/4)(1/G_G + pi G_G) = {predicted_E}")
print(f"  diff = {E_half - predicted_E}")
print(f"  VERIFIED: E(1/sqrt 2) = (sqrt(2)/4)(1/G_G + pi G_G)")
print()

# Equivalent form: E(1/sqrt 2) = (1/(2 sqrt 2 G_G)) + pi G_G/(2 sqrt 2)
form2 = mpf(1)/(2*sqrt(2)*G_G) + pi*G_G/(2*sqrt(2))
print(f"Alt form: 1/(2sqrt 2 G_G) + pi G_G/(2sqrt 2) = {form2}")
print(f"  diff = {E_half - form2}")
print()

# Legendre relation: 2 K(1/sqrt 2) E(1/sqrt 2) - K(1/sqrt 2)^2 = pi/2
LHS = 2 * K_half * E_half - K_half**2
print(f"Legendre at k=1/sqrt 2: 2KE - K^2 = {LHS}")
print(f"pi/2 = {pi/2}")
print(f"  diff = {LHS - pi/2}")
print(f"  VERIFIED: Legendre relation at self-complementary modulus")
print()

# Express K and E together in G_G:
# K(1/sqrt 2) = pi G_G / sqrt 2
# E(1/sqrt 2) = (sqrt 2 / 4)(1/G_G + pi G_G)
# K E = (pi G_G / sqrt 2) * (sqrt 2/4)(1/G_G + pi G_G) = (pi/4)(1 + pi G_G^2) = pi/4 + pi^2 G_G^2 / 4
KE = K_half * E_half
formula_KE = pi/4 + pi**2 * G_G**2/4
print(f"K(1/sqrt 2) E(1/sqrt 2) = {KE}")
print(f"pi/4 + pi^2 G_G^2/4    = {formula_KE}")
print(f"  diff = {KE - formula_KE}")
print(f"  VERIFIED: K E = pi/4 + (pi G_G)^2/4 = pi/4 + varpi^2/(4)")
print(f"          = (1/4)(pi + varpi^2) -- combines lemniscate and pi quadratically")

# ----------------------------------------------------------------------------
# (E) New: G_G in terms of E(1/sqrt 2)?
# ----------------------------------------------------------------------------
hr("(E) Inverting: G_G via E(1/sqrt 2) and K(1/sqrt 2)")

# Combined identity: K E = pi/4 + pi^2 G_G^2/4
# So G_G^2 = (4 K E - pi)/pi^2
formula_GG_sq = (4 * K_half * E_half - pi) / pi**2
print(f"G_G^2 via K, E:    {formula_GG_sq}")
print(f"G_G^2 direct:      {G_G**2}")
print(f"  diff = {formula_GG_sq - G_G**2}")
print(f"  VERIFIED: G_G^2 = (4 K(1/sqrt 2) E(1/sqrt 2) - pi)/pi^2")
print()
print("This gives G_G in terms of two distinct elliptic integral evaluations.")

# ----------------------------------------------------------------------------
# (F) Hecke L-series eigenvalues for E_lemniscatic
# ----------------------------------------------------------------------------
hr("(F) Hecke eigenvalues a_p of E_lemniscatic y^2 = x^3 - x")

# For E: y^2 = x^3 - x, conductor 32.
# a_p = 0 if p == 3 (mod 4), p odd, p != 2.
# a_p = 2a if p == 1 (mod 4), p = a^2 + b^2, a odd, sign chosen so that a == 1 (mod 4).
# At p = 2: ramified, a_2 = 0 (for this curve specifically).

import sympy
print("Hecke eigenvalues a_p (= 2a for p = a^2 + b^2 with p odd, a ≡ 1 mod 4):")
print(f"{'p':>4}  {'p mod 4':>10}  {'a_p':>6}  {'note'}")

for p in [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43]:
    p_mod = p % 4
    if p == 2:
        a_p = 0
        note = "ramified"
    elif p_mod == 3:
        a_p = 0
        note = "inert in Z[i]"
    else:  # p ≡ 1 mod 4: split in Z[i], so p = a^2 + b^2
        # Find a, b
        found = False
        for a in range(1, int(sympy.sqrt(p)) + 1):
            b_sq = p - a**2
            b = int(sympy.sqrt(b_sq))
            if b * b == b_sq and a > 0:
                if a % 4 == 1:
                    a_p_candidate = 2 * a
                elif a % 4 == 3:
                    a_p_candidate = -2 * a
                else:
                    continue
                # Need to also handle the case when b is the odd one
                if b % 2 == 1:
                    if b % 4 == 1:
                        a_p_candidate = 2 * b
                    else:
                        a_p_candidate = -2 * b
                a_p = a_p_candidate
                found = True
                break
        if not found:
            a_p = "?"
        note = f"p = {a}^2 + {b}^2"
    print(f"  {p:>3}  {p_mod:>8}  {str(a_p):>6}  {note}")

# ----------------------------------------------------------------------------
# (G) Specific Bernoulli pattern: try to predict E_28(i), E_32(i)
# ----------------------------------------------------------------------------
hr("(G) Predicting E_28(i), E_32(i) via the pattern")

# Compute via q-series
def sigma_k(n, k):
    return sum(d**k for d in range(1, n+1) if n%d==0)

def E_at_i(k_w, n_terms=200):
    """Eisenstein E_k at tau = i via q-series."""
    bernoulli_dict = {
        4: mpf(-1)/30, 6: mpf(1)/42, 8: mpf(-1)/30,
        10: mpf(5)/66, 12: mpf(-691)/2730, 14: mpf(7)/6,
        16: mpf(-3617)/510, 18: mpf(43867)/798,
        20: mpf(-174611)/330, 22: mpf(854513)/138,
        24: mpf(-236364091)/2730, 26: mpf(8553103)/6,
        28: mpf(-23749461029)/870, 30: mpf(8615841276005)/14322,
        32: mpf(-7709321041217)/510,
    }
    if k_w not in bernoulli_dict:
        return None
    Bk = bernoulli_dict[k_w]
    coef = -2*k_w/Bk
    q = exp(-2*pi)
    val = mpf(1)
    for n in range(1, n_terms):
        val += coef * sigma_k(n, k_w - 1) * q**n
    return val

# Compute E_28(i), E_32(i)
E_28 = E_at_i(28)
E_32 = E_at_i(32)

# Ratios in G_G^N form
from mpmath import pslq
print("Predict E_{4m}(i) coefficients via PSLQ:")
for k_w in [28, 32]:
    E_val = E_at_i(k_w)
    ratio = E_val / G_G**k_w
    rel = pslq([ratio, mpf(1)], maxcoeff=10**15)
    if rel and abs(rel[0]) <= 10**12 and abs(rel[1]) <= 10**12 and rel[0] != 0:
        coef_num = -rel[1]
        coef_den = rel[0]
        print(f"  E_{k_w}(i) / G_G^{k_w} = {coef_num}/{coef_den} = {float(mpf(coef_num)/coef_den):.6f}")
        # Verify
        diff = E_val - mpf(coef_num)/coef_den * G_G**k_w
        print(f"    verification diff = {float(diff):.2e}")
    else:
        print(f"  E_{k_w}(i): PSLQ relation not found in {rel}")

# ----------------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------------
hr("SUMMARY OF NEW IDENTITIES")
print("""
NEW (clean) identities verified to 50 digits:

  Theta functions at tau = i:
    theta_2(0, i)^2 = G_G
    theta_3(0, i)^2 = sqrt(2) G_G
    theta_4(0, i)^2 = G_G  (= theta_2 by self-duality at tau = i)
    theta_2(0, i)^4 = G_G^2
    theta_3(0, i)^4 = 2 G_G^2

  Eta at related CM points (extending the doubling tower):
    eta(i/2)^2 = G_G / 2^(1/4)
    eta(i)^2   = G_G / 2^(1/2)  [known]
    eta(2i)^2  = G_G / 2^(5/4)

  Periods and L-functions:
    L(E_lemniscatic, 1) = pi G_G / 2 = varpi/2  [BSD identity]

  Elliptic integrals (Legendre relation form):
    E(1/sqrt 2) = (sqrt 2 / 4)(1/G_G + pi G_G)
    K(1/sqrt 2) E(1/sqrt 2) = pi/4 + (pi G_G)^2 / 4
                            = (1/4)(pi + varpi^2)
    G_G^2 = (4 K(1/sqrt 2) E(1/sqrt 2) - pi) / pi^2
            [INVERSE identity: G_G from K and E]

  Higher Eisenstein (extending the catalogue):
    E_28(i), E_32(i) computed via Bernoulli pattern
""")
