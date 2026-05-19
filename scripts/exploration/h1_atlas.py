"""
The class-number-one atlas: numerical computation of CM constants across
all nine class-number-one imaginary quadratic fields K = Q(sqrt(-d)).

For each field, we compute:
  1. The Chowla-Selberg constant: a Gamma-product determined by chi_{-|d|}
  2. The j-invariant at the canonical CM point (the integer Hilbert class polynomial root for h=1)
  3. The Hecke character data (split/inert prime pattern)
  4. The eta-evaluation |eta(tau_K)|
  5. The Heegner near-integer phenomenon (where applicable)

Reference:
  d=1:   |d_K|=4,   w_K=4,  j(i)=1728
  d=2:   |d_K|=8,   w_K=2,  j(sqrt(-2))=8000=20^3
  d=3:   |d_K|=3,   w_K=6,  j(rho)=0
  d=7:   |d_K|=7,   w_K=2,  j(tau_7)=-3375=-15^3
  d=11:  |d_K|=11,  w_K=2,  j(tau_11)=-32768=-32^3
  d=19:  |d_K|=19,  w_K=2,  j(tau_19)=-884736=-96^3
  d=43:  |d_K|=43,  w_K=2,  j(tau_43)=-884736000=-960^3
  d=67:  |d_K|=67,  w_K=2,  j(tau_67)=-147197952000=-5280^3
  d=163: |d_K|=163, w_K=2,  j(tau_163)=-262537412640768000=-640320^3

The famous Heegner near-integer: exp(pi sqrt(163)) ~ 640320^3 + 744 (to 12 decimal places).
"""

from mpmath import mp, mpf, mpc, pi, gamma, sqrt, exp, agm, log, jtheta

mp.dps = 60

# === Field data: discriminant |d_K|, unit count w_K, integer j-value ===
# For each squarefree d > 0, K = Q(sqrt(-d)):
# - If d == 1 mod 4: disc(K) = -d, |d_K| = d
# - If d == 2 or 3 mod 4: disc(K) = -4d, |d_K| = 4d

field_data = [
    # (d, |d_K|, w_K, j(tau_K), CM-tau)
    (1,   4,   4, 1728,                   "i"),
    (2,   8,   2, 8000,                   "sqrt(-2)"),
    (3,   3,   6, 0,                      "rho = e^(2pi i/3)"),
    (7,   7,   2, -3375,                  "(1+sqrt(-7))/2"),
    (11,  11,  2, -32768,                 "(1+sqrt(-11))/2"),
    (19,  19,  2, -884736,                "(1+sqrt(-19))/2"),
    (43,  43,  2, -884736000,             "(1+sqrt(-43))/2"),
    (67,  67,  2, -147197952000,          "(1+sqrt(-67))/2"),
    (163, 163, 2, -262537412640768000,    "(1+sqrt(-163))/2"),
]

# === Compute Chowla-Selberg constants ===
# For h_K = 1, the Chowla-Selberg formula at K (mod normalization):
#   prod_{a=1}^{|d|-1} Gamma(a/|d|)^{chi_K(a) * w_K / (4*h_K)}
# gives a quantity related to |eta(tau_K)|^4 (up to constants in d_K, pi)

def kronecker_chi(d_abs, a):
    """Kronecker character chi_{-d_abs}(a). Returns -1, 0, or +1."""
    if d_abs == 4:
        # chi_{-4}(a): +1 if a == 1 (4), -1 if a == 3 (4), 0 if even
        if a % 2 == 0: return 0
        return 1 if a % 4 == 1 else -1
    if d_abs == 8:
        # chi_{-8}(a): determined by a mod 8
        if a % 2 == 0: return 0
        m = a % 8
        return 1 if m in (1, 3) else -1
    if d_abs == 3:
        # chi_{-3}(a) = chi_3 = (a/3): +1 if a == 1, -1 if a == 2, 0 if a == 0 mod 3
        m = a % 3
        if m == 0: return 0
        return 1 if m == 1 else -1
    # For prime discriminant -p (p in {7, 11, 19, 43, 67, 163}):
    # chi_{-p}(a) = (a/p) the Legendre symbol if p odd
    if d_abs % 2 == 1:  # d_abs is an odd prime
        p = d_abs
        if a % p == 0: return 0
        # Compute Legendre symbol (a/p) via Euler's criterion: a^((p-1)/2) mod p
        e = pow(a, (p - 1) // 2, p)
        return 1 if e == 1 else (-1 if e == p - 1 else 0)
    return 0

def chowla_selberg_constant(d_abs, w_K, h_K=1):
    """The Chowla-Selberg product for K = Q(sqrt(-d_abs))."""
    exponent_factor = mpf(w_K) / (4 * h_K)
    result = mpf(1)
    for a in range(1, d_abs):
        chi = kronecker_chi(d_abs, a)
        if chi != 0:
            result *= gamma(mpf(a) / d_abs) ** (chi * exponent_factor)
    return result

print("=" * 90)
print("THE CLASS-NUMBER-ONE ATLAS: Chowla-Selberg constants at each h=1 IQ field")
print("=" * 90)
print()
print(f"{'d':>4}  {'|d_K|':>6}  {'w_K':>4}  {'C-S constant G_K':>32}  {'log G_K':>12}")
print("-" * 90)

results = []
for d, d_K, w_K, j_val, tau_descr in field_data:
    G_K = chowla_selberg_constant(d_K, w_K)
    log_G_K = log(G_K) if G_K > 0 else mpc(log(abs(G_K)), pi)
    print(f"{d:>4}  {d_K:>6}  {w_K:>4}  {float(G_K):>32.18f}  {float(log_G_K.real if hasattr(log_G_K, 'real') else log_G_K):>12.6f}")
    results.append((d, d_K, w_K, j_val, G_K, tau_descr))
print()

# Verify the d=1 (lemniscatic) case: should give something proportional to G_G^something
print("Verification of d=1 case:")
G_G = 1 / agm(1, sqrt(2))
G_star = gamma(mpf(1)/4) / gamma(mpf(3)/4)
print(f"  G_K(d=1) = {float(results[0][4]):.10f}")
print(f"  G_star  = {float(G_star):.10f}")
print(f"  G_G     = {float(G_G):.10f}")
# What's the relation?
# For d=1, w_K=4, h_K=1:
#  G_K = prod_{a=1}^{3} Gamma(a/4)^{chi(a)*1} = Gamma(1/4)/Gamma(3/4) = G_star
# So G_K(d=1) should equal G_star. Let's verify.
print(f"  G_K/G_star = {float(results[0][4] / G_star):.10f} (expect 1.0)")
print()

# Verify the d=3 (equianharmonic) case
print("Verification of d=3 case:")
R_3 = gamma(mpf(1)/3) / gamma(mpf(2)/3)
print(f"  G_K(d=3) = {float(results[2][4]):.10f}")
print(f"  R_3      = {float(R_3):.10f}")
# For d=3, w_K=6, h_K=1, the exponent factor = 6/4 = 3/2:
#  G_K = (Gamma(1/3)/Gamma(2/3))^(3/2) = R_3^(3/2)
print(f"  R_3^(3/2) = {float(R_3 ** mpf('1.5')):.10f}")
print(f"  G_K/R_3^(3/2) = {float(results[2][4] / R_3 ** mpf('1.5')):.10f}")
print()

# Verify the d=2 case (Q(sqrt(-2)))
print("Verification of d=2 case:")
print(f"  G_K(d=2) = {float(results[1][4]):.10f}")
# For d=2, d_K=8, w_K=2, exponent factor = 1/2:
# chi_{-8}: chi(1)=+1, chi(3)=+1, chi(5)=-1, chi(7)=-1
# G_K = (Gamma(1/8) Gamma(3/8) / (Gamma(5/8) Gamma(7/8)))^(1/2)
# Using reflection Gamma(z)Gamma(1-z) = pi/sin(pi z):
#   Gamma(1/8)Gamma(7/8) = pi/sin(pi/8) = 2pi/sqrt(2-sqrt(2))
#   Gamma(3/8)Gamma(5/8) = pi/sin(3pi/8) = pi*sqrt(2)/sqrt(1+1/sqrt(2)) wait
gamma_1_8 = gamma(mpf(1)/8)
gamma_3_8 = gamma(mpf(3)/8)
gamma_5_8 = gamma(mpf(5)/8)
gamma_7_8 = gamma(mpf(7)/8)
analytic = (gamma_1_8 * gamma_3_8 / (gamma_5_8 * gamma_7_8)) ** mpf('0.5')
print(f"  Direct (Gamma(1/8)Gamma(3/8)/Gamma(5/8)Gamma(7/8))^(1/2) = {float(analytic):.10f}")
print()

# === Heegner near-integers ===
print("=" * 90)
print("HEEGNER NEAR-INTEGERS: e^(pi sqrt(d)) vs -j(tau_d) + 744")
print("=" * 90)
print(f"{'d':>4}  {'e^(pi sqrt d)':>40}  {'|j| + 744':>30}  {'diff':>20}")
print("-" * 90)

for d, d_K, w_K, j_val, G_K, tau_descr in results:
    if d in (1, 2, 3):
        # tau_K = i, sqrt(-2)/(something), rho — different normalization
        continue
    # For (1 + sqrt(-d))/2 the imaginary part is sqrt(d)/2, so q = e^(2 pi i tau)
    # has |q| = e^(-pi sqrt(d)). The leading term of -j(tau) is 1/q = e^(pi sqrt(d)).
    # Specifically j(tau) ~ 1/q + 744 + 196884 q + ... for large d
    # So e^(pi sqrt d) ~ |j| - 744 (with sign flip for negative j)
    e_pi_sqrtd = exp(pi * sqrt(d))
    approx_j_plus_744 = abs(j_val) + 744
    diff = abs(e_pi_sqrtd - approx_j_plus_744)
    print(f"{d:>4}  {float(e_pi_sqrtd):>40.10f}  {approx_j_plus_744:>30}  {float(diff):>20.6f}")
print()
print("For d=163: e^(pi sqrt(163)) - (640320^3 + 744) ~ 7.5e-13 (the famous near-integer)")
print()

# === Structural observation: G_K^(1/w_K) is the natural 'base constant' ===
print("=" * 90)
print("STRUCTURAL: extracting the natural base constant G_K^(2/w_K)")
print("=" * 90)
print("This normalises the Chowla-Selberg product to a canonical 'half-weight' form.")
print()
print(f"{'d':>4}  {'|d_K|':>6}  {'G_K^(2/w_K)':>32}")
print("-" * 60)
for d, d_K, w_K, j_val, G_K, tau_descr in results:
    if G_K > 0:
        normalised = G_K ** (mpf(2) / w_K)
        print(f"{d:>4}  {d_K:>6}  {float(normalised):>32.18f}")
    else:
        print(f"{d:>4}  {d_K:>6}  (complex; not real positive)")
print()

# === Verification of e^(pi sqrt 163) to high precision ===
print("=" * 90)
print("HIGH-PRECISION CHECK: e^(pi sqrt(163))")
print("=" * 90)
mp.dps = 50
ep163 = exp(pi * sqrt(163))
target = mpf("262537412640768744")  # = 640320^3 + 744
print(f"e^(pi sqrt 163) = {ep163}")
print(f"640320^3 + 744 = {target}")
print(f"Difference     = {target - ep163}")
print(f"  ~ 7.5e-13 (the Heegner near-integer)")
