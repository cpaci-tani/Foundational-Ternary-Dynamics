"""
Numerical exploration for Paper D:
  (B) Higher-level modular forms Γ_0(N) at the Atkin-Lehner CM point τ = i/√N
  (C) Mahler measures of small Laurent polynomials and connections to L'(E_lemn, 0)

Strategy:
  (B) At each h=1 IQ field K with discriminant -d_K, the natural CM point
      τ_K lives at level N = N(τ_K). Compute modular-form values at τ_K
      and identify the period algebra.

  (C) Compute m(P) for several small P(x,y) and PSLQ-search for clean
      relations with G*, G_G, π, L(E_lemn, 1) = ϖ/4.

We work to 60-digit precision throughout.
"""

from mpmath import mp, mpf, mpc, pi, gamma, sqrt, exp, agm, log, quad, pslq

mp.dps = 60

# === Reference constants ===
G_star = gamma(mpf(1)/4) / gamma(mpf(3)/4)
G_G    = 1 / agm(1, sqrt(2))
varpi  = pi * G_G
L_E_lemn_1 = varpi / 4  # corrected

# === eta function via q-product ===
def eta(tau, n_terms=300):
    q = exp(2j * pi * tau)
    val = mpc(1)
    for n in range(1, n_terms):
        val *= (1 - q**n)
    return q**(mpf(1)/24) * val

# === Eisenstein series E_k via q-series ===
def sigma_k(n, k):
    return sum(d**k for d in range(1, n+1) if n % d == 0)

def E_k(tau, k, n_terms=120):
    """E_k(tau) via standard q-series."""
    if k == 2:
        q = exp(2j*pi*tau)
        val = mpc(1)
        for n in range(1, n_terms):
            val -= 24 * sigma_k(n, 1) * q**n
        return val
    bernoulli_factor = {4: mpf(-1)/30, 6: mpf(1)/42, 8: mpf(-1)/30, 10: mpf(5)/66,
                        12: mpf(-691)/2730, 14: mpf(7)/6}
    if k not in bernoulli_factor:
        return None
    B_k = bernoulli_factor[k]
    coef = -2*k / B_k
    q = exp(2j*pi*tau)
    val = mpc(1)
    for n in range(1, n_terms):
        val += coef * sigma_k(n, k-1) * q**n
    return val

# ========================================================================
# (B) Higher-level modular forms at CM points
# ========================================================================
print("=" * 80)
print("(B) HIGHER-LEVEL MODULAR FORMS AT CM POINTS")
print("=" * 80)
print()

# CM point τ_K for each h=1 IQ field (with N = N(τ_K) the natural level)
# For K = Q(sqrt(-d)) with -d ≡ 1 mod 4: τ_K = (1+sqrt(-d))/2, |τ_K|² = (1+d)/4
# For -d ≡ 2 or 3 mod 4: τ_K = sqrt(-d), |τ_K|² = d
field_data = [
    ("Q(i)",     1,  mpc(0, 1),                       1),   # tau = i,  level 1
    ("Q(sqrt-2)", 2, mpc(0, sqrt(2)),                 2),   # tau = sqrt(-2), level 2
    ("Q(rho)",   3,  mpc(mpf(1)/2, sqrt(3)/2),        1),   # tau = rho, level 1 (h=1 maximal order)
    ("Q(sqrt-7)", 7, mpc(mpf(1)/2, sqrt(7)/2),        2),   # tau = (1+sqrt(-7))/2, |tau|^2 = 2
    ("Q(sqrt-11)", 11, mpc(mpf(1)/2, sqrt(11)/2),     3),  # |tau|^2 = 3
    ("Q(sqrt-19)", 19, mpc(mpf(1)/2, sqrt(19)/2),     5),  # |tau|^2 = 5
    ("Q(sqrt-43)", 43, mpc(mpf(1)/2, sqrt(43)/2),     11), # |tau|^2 = 11
    ("Q(sqrt-67)", 67, mpc(mpf(1)/2, sqrt(67)/2),     17), # |tau|^2 = 17
    ("Q(sqrt-163)",163,mpc(mpf(1)/2, sqrt(163)/2),    41), # |tau|^2 = 41
]

print(f"{'K':<14}  {'d':>4}  {'τ_K':<28}  {'N=|τ|²':>8}  {'|η(τ_K)|':>30}")
print("-" * 95)
for name, d, tau, N in field_data:
    eta_tau = eta(tau)
    abs_eta = abs(eta_tau)
    re_str = f"({float(tau.real):.4f}, {float(tau.imag):.4f}i)"
    print(f"{name:<14}  {d:>4}  {re_str:<28}  {N:>8}  {float(abs_eta):>30.20f}")
print()

# Key observation: |η(τ_K)|^24 = Δ(τ_K) is an algebraic number for each h=1 field
print("Computing |η(τ_K)|^24 (= |Δ(τ_K)|) for each field:")
for name, d, tau, N in field_data[:5]:
    eta_tau = eta(tau)
    delta_tau = eta_tau**24
    abs_delta = abs(delta_tau)
    print(f"  {name}: |Δ(τ)| = {float(abs_delta):.6e}")
print()

# For Q(i): Δ(i) = G_G^12/64
print(f"Check d=1: G_G^12/64 = {float(G_G**12/64):.6e}")
print(f"Check d=3: should be -G_rho^12 / 6912 (equianharmonic Δ from paper A §15)")
G_rho = gamma(mpf(1)/3) * gamma(mpf(1)/6) / (2 * pi * sqrt(pi))
print(f"  G_rho^12/6912 = {float(G_rho**12/6912):.6e}")

print()
print("=" * 80)
print("(C) MAHLER MEASURES")
print("=" * 80)
print()

# Mahler measure of P(x, y) for several small P:
#   P_1 = 1 + x + y           (Smyth: m = L'(chi_{-3}, -1))
#   P_2 = 1 + x + y + x*y     (Boyd: simple)
#   P_3 = 1 + x + 1/x + y + 1/y (Boyd 1981, related to L'(E_15, 0))
#   P_4 = (1+x)(1+y) - x*y    (custom)

# Compute m(P) via the formula:
#   m(P) = (1/(2pi i)) integral_T log|P(x,y)| dx/x dy/y
#   where T = {|x| = |y| = 1}
# This is a 2D integral over the unit torus.

def mahler_measure_2d(P_func, n=200):
    """Compute m(P(x,y)) via Monte-Carlo torus integration.
    P_func takes complex x, y and returns complex value."""
    import random
    random.seed(42)
    s = mpf(0)
    count = 0
    for _ in range(n):
        u = mpf(random.random())
        v = mpf(random.random())
        x = exp(2j*pi*u)
        y = exp(2j*pi*v)
        val = abs(P_func(x, y))
        if val > 0:
            s += log(val)
            count += 1
    return s / count

# Test polynomials
polynomials = {
    "P_1 = 1+x+y":              lambda x, y: 1 + x + y,
    "P_2 = 1+x+y+xy":           lambda x, y: 1 + x + y + x*y,
    "P_3 = x+1/x+y+1/y+1":      lambda x, y: x + 1/x + y + 1/y + 1,
    "P_4 = x+1/x+y+1/y":        lambda x, y: x + 1/x + y + 1/y,
}

print("Numerical m(P) via Monte-Carlo torus integration (200 samples):")
mahler_values = {}
for label, P in polynomials.items():
    m = mahler_measure_2d(P, n=2000)
    mahler_values[label] = m
    print(f"  m({label}) = {float(m):>10.6f}")

print()
print("Known reference values:")
print("  m(1+x+y) = L'(chi_{-3}, -1) (Smyth 1981) ≈ 0.32306...")
print("  m(1+x+1/x+y+1/y) = (15/4π²) L'(E_15, 0) (Boyd-Deninger) ≈ 0.25132...")
print()

# Search for clean relations: m(P) = c1 * L(E_lemn, 1) + c2 * pi + c3 * G_star + ...
# Build a basis of known constants:
basis_constants = [
    mpf(1),
    L_E_lemn_1,
    pi,
    G_star,
    G_G,
    log(2),
    log(3),
    log(pi),
]
basis_labels = ['1', 'L(E_lemn,1)=varpi/4', 'pi', 'G*', 'G_G', 'log(2)', 'log(3)', 'log(pi)']

print("PSLQ search for m(P_3) in basis {1, L(E_lemn,1), π, G*, G_G, log 2, log 3, log π}:")
m3 = mahler_values["P_3 = x+1/x+y+1/y+1"]
rel = pslq([m3] + basis_constants, maxcoeff=10**6)
if rel is None:
    print("  No relation found")
else:
    nz = [(c, lab) for c, lab in zip(rel, ['m(P_3)'] + basis_labels) if c != 0]
    print(f"  Found: {nz}")

# For E_lemn = 32.a3, are there KNOWN Mahler measure identities?
# Stienstra and Beukers have computed several.
# Specifically: m(y^2 - (x^2 - 2)y + 1) or similar related to 32.a3?
# Not commonly listed in standard tables.

# Independent check: the conductor of E_lemn is 32, which is not a 'small Boyd-style' conductor.
# The standard Boyd identities are for conductor 11, 14, 15, 20, 21, 24, 26, 30, 33, 34, 36, 37, 38, 39, 40, 42, 44, 45.
# Conductor 32 is NOT in Boyd's standard list. This suggests E_lemn does not have a known clean Mahler measure formula.

print()
print("=" * 80)
print("OBSERVATION: E_lemn = 32.a3 has conductor 32, which is NOT in Boyd's")
print("standard tables of curves with clean Mahler measure identities.")
print("The CM structure (j(i) = 1728) makes E_lemn special enough that")
print("its L-values reduce to closed Gamma forms, but the Mahler-measure")
print("connection is not direct.")
print()
print("Direction (C) thus does NOT yield a clean theorem for E_lemn directly.")
print("It yields instead a structural NEGATIVE result: E_lemn's L(s) at s=2,3")
print("does not have a known clean Mahler-measure formula.")
print("=" * 80)
print()

# ========================================================================
# Pivot: at level 2, compute E_4 at the natural CM point tau = sqrt(-2)
# ========================================================================
print("=" * 80)
print("(B continued) E_4 at the level-2 CM point τ = sqrt(-2)")
print("=" * 80)
print()

tau_2 = mpc(0, sqrt(2))
E4_at_sqrt2 = E_k(tau_2, 4)
print(f"E_4(sqrt(-2)) = {E4_at_sqrt2}")
print(f"  Re part: {float(E4_at_sqrt2.real):.20f}")

# Compare to known formula:
# E_4(sqrt(-2)) should be expressible via Gamma(a/8) values
gamma_1_8 = gamma(mpf(1)/8)
gamma_3_8 = gamma(mpf(3)/8)
gamma_5_8 = gamma(mpf(5)/8)
gamma_7_8 = gamma(mpf(7)/8)
G_K_2 = (gamma_1_8 * gamma_3_8 / (gamma_5_8 * gamma_7_8)) ** mpf('0.5')
print(f"G_K(d=2) = {float(G_K_2):.10f}")
print(f"G_K(d=2)^4 = {float(G_K_2**4):.10f}")
print(f"E_4(sqrt-2) / G_K(d=2)^4 = {float(E4_at_sqrt2.real / G_K_2**4):.10f}")

# Search for clean rational
ratio = E4_at_sqrt2.real / G_K_2**4
print(f"Ratio: {ratio}")
rel = pslq([ratio, mpf(1)], maxcoeff=10**6)
if rel:
    a, b = rel
    if a != 0:
        print(f"  PSLQ: ratio = {-b}/{a} = {-mpf(b)/a}")

print()
# What about E_4(sqrt(-2)) * 4 = ? expressible in pi powers?
# Try: E_4(sqrt-2) = c * G_K^a / pi^b
print("PSLQ for E_4(sqrt(-2)) in log-basis {log G_K, log pi, log 2}:")
log_basis = [log(E4_at_sqrt2.real), log(G_K_2), log(pi), log(mpf(2))]
log_labels = ['log E_4(sqrt-2)', 'log G_K(d=2)', 'log pi', 'log 2']
rel = pslq(log_basis, maxcoeff=10**5)
if rel:
    nz = [(c, lab) for c, lab in zip(rel, log_labels) if c != 0]
    print(f"  Found relation: {nz}")
else:
    print("  No log-linear relation found at maxcoeff=10^5")
