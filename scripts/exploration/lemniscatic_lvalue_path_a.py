"""
lemniscatic_lvalue_path_a.py — Path A K_2-regulator probe.

Goal: compute L(E, 2) for the lemniscatic curve E: y² = x³ − x to high precision,
then PSLQ-test whether L(E, 2) ∈ Q[π², G*², Catalan]. If yes, the
Bloch-Beilinson K_2-regulator route lands cleanly and Path A advances by
one layer. If no, the K_2-regulator route is also closed-negative at the
elementary-rationals level.

E: y² = x³ − x has:
  - Conductor N = 32 (LMFDB label 32.a3 / Cremona 32a1)
  - CM by Z[i]
  - Modular form: f(τ) = η(4τ)²·η(8τ)² (weight 2, level 32)
  - Rank 0; torsion (Z/2)²
  - L(E, 1) = Γ(1/4)² / (4·√(2π))   (Damerell formula)

Method:
  1. Compute a_n coefficients via η-product q-expansion up to N_max = 5000
  2. Compute L(E, 2) = sum_{n=1..N_max} a_n / n² with mpmath at 50+ dps
  3. PSLQ-test L(E, 2) against {1, π², G*², G*², π²·G*², G_Catalan, π·G_Catalan, …}
  4. Report verdict: closed form in Q[G*²] or CLOSED-NEGATIVE for this layer

Run: python scripts/exploration/lemniscatic_lvalue_path_a.py
"""

from mpmath import mp, mpf, gamma, pi, catalan, log, sqrt, pslq, fsum

mp.dps = 60  # precision

# ── Step 1: compute a_n coefficients for the modular form ────────────
# f(τ) = η(4τ)² · η(8τ)²
# η(z) = q^(1/24) ∏(1 - q^n) with q = e^(2πiz)
# η(4z)² · η(8z)² = q^(1/3 + 2/3) · ∏(1 - q^{4n})² · ∏(1 - q^{8n})²
#                 = q · ∏(1 - q^{4n})² · ∏(1 - q^{8n})²
#
# So f(τ) starts with q^1; we need q-expansion to extract a_n.

def compute_a_coefficients(N_max):
    """Compute a_n for n = 1..N_max from the η-product expansion."""
    # We compute prod_{k=1}^∞ (1 - q^{4k})² · (1 - q^{8k})² as a power series
    # up to degree N_max - 1 (since f = q · this).
    #
    # Strategy: build the power series multiplicatively.
    # Initialize A[d] = coefficient of q^d in the running product.

    # Start with A = [1] (constant series 1)
    deg = N_max  # We need degree up to N_max - 1 after multiplying by q
    A = [mpf(0)] * (deg + 1)
    A[0] = mpf(1)

    # Multiply by (1 - q^{4k})² for k = 1, 2, ...
    # (1 - q^{4k})² = 1 - 2 q^{4k} + q^{8k}
    k = 1
    while 4 * k <= deg:
        # multiply A by (1 - 2 q^{4k} + q^{8k}) in place
        # B[d] = A[d] - 2 A[d - 4k] + A[d - 8k]
        new_A = list(A)
        for d in range(deg, -1, -1):
            val = A[d]
            if d - 4*k >= 0:
                val = val - 2 * A[d - 4*k]
            if d - 8*k >= 0:
                val = val + A[d - 8*k]
            new_A[d] = val
        A = new_A
        k += 1

    # Multiply by (1 - q^{8k})² for k = 1, 2, ...
    # (1 - q^{8k})² = 1 - 2 q^{8k} + q^{16k}
    k = 1
    while 8 * k <= deg:
        new_A = list(A)
        for d in range(deg, -1, -1):
            val = A[d]
            if d - 8*k >= 0:
                val = val - 2 * A[d - 8*k]
            if d - 16*k >= 0:
                val = val + A[d - 16*k]
            new_A[d] = val
        A = new_A
        k += 1

    # f(τ) = q · A(q), so a_n = A[n-1]
    a = [mpf(0)] * (N_max + 1)
    for n in range(1, N_max + 1):
        if n - 1 <= deg:
            a[n] = A[n - 1]
    return a

# Quick sanity test: a_1=1, a_2=a_3=0, a_5=-2, a_13=6, a_17=2 per LMFDB
print("Computing a_n coefficients via η(4z)²·η(8z)² q-expansion (slow but exact)…")
print("(Using small N_max = 200 for sanity check first)")
a_test = compute_a_coefficients(200)
expected = {1: 1, 2: 0, 3: 0, 5: -2, 7: 0, 9: -3, 13: 6, 17: 2, 25: -1, 29: -10, 37: -2, 41: 10, 49: -7}
print("Sanity check vs LMFDB E_32a1:")
all_ok = True
for n, exp_val in expected.items():
    got = int(a_test[n])
    ok = (got == exp_val)
    all_ok = all_ok and ok
    print(f"  a_{n} = {got:>4} (expected {exp_val:>4}) {'✓' if ok else '✗'}")
if not all_ok:
    print("Coefficients do not match LMFDB — η-product expansion is wrong.")
    print("Aborting.")
    import sys
    sys.exit(1)
print()

# Now compute at larger N for L(E, 2)
N_max = 5000
print(f"Computing a_n for n = 1..{N_max}…")
a = compute_a_coefficients(N_max)
print(f"  a_{N_max} = {int(a[N_max])} (sanity: should be small integer)")
print()

# ── Step 2: compute L(E, 2) ──────────────────────────────────────────
print(f"Summing L(E, 2) = Σ_{{n=1..{N_max}}} a_n / n²…")
L_E2 = fsum(a[n] / mpf(n)**2 for n in range(1, N_max + 1))
print(f"  L(E, 2) ≈ {L_E2}")
print()

# Sanity check: L(E, 1) for comparison (Damerell)
print("Sanity check: L(E, 1) via partial sum vs Damerell closed form…")
L_E1_partial = fsum(a[n] / mpf(n) for n in range(1, N_max + 1))
gamma_quarter = gamma(mpf('0.25'))
L_E1_damerell = gamma_quarter**2 / (4 * sqrt(2 * pi))
print(f"  L(E, 1) partial sum (N={N_max}) = {L_E1_partial}")
print(f"  L(E, 1) Damerell formula      = {L_E1_damerell}")
print(f"  Note: L(E,1) converges slowly at s=1 (boundary of critical strip); s=2 converges much faster")
print()

# ── Step 3: PSLQ search ──────────────────────────────────────────────
print("─── PSLQ: does L(E, 2) live in Q[π², G*², Catalan]? ────────────")
G_star    = gamma_quarter / gamma(mpf('0.75'))
G_star_sq = G_star**2
pi_sq     = pi**2
G_cat     = catalan

basis = {
    '1':                  mpf(1),
    'π²':                 pi_sq,
    'G*²':                G_star_sq,
    'G*':                 G_star,
    'π':                  pi,
    'G_Catalan':          G_cat,
    'π²·G*²':             pi_sq * G_star_sq,
    'π·G_Catalan':        pi * G_cat,
    'π²·G_Catalan':       pi_sq * G_cat,
    'G*²·G_Catalan':      G_star_sq * G_cat,
    'G*²/π':              G_star_sq / pi,
    'π²/G*²':             pi_sq / G_star_sq,
    'G_Catalan/π':        G_cat / pi,
    'G_Catalan/π²':       G_cat / pi_sq,
    'log G*':             log(G_star),
}

names = list(basis.keys())
vals  = [basis[n] for n in names]

print("Basis:")
for n, v in zip(names, vals):
    print(f"  {n:25} = {float(v):>20.10f}")
print()

print(f"Running PSLQ on [L(E,2)] + basis at {mp.dps}-dps, maxcoeff=10000…")
try:
    inputs = [L_E2] + vals
    rel = pslq(inputs, tol=mpf(10)**(-(mp.dps - 10)), maxcoeff=10000, maxsteps=5000)
    if rel is None:
        print("  PSLQ: no integer relation found at maxcoeff=10000.")
    else:
        print(f"  Relation: {rel}")
        print(f"  Largest |coeff|: {max(abs(r) for r in rel)}")
        if rel[0] != 0:
            reconstructed = -sum(rel[i+1] * vals[i] for i in range(len(vals))) / rel[0]
            resid = L_E2 - reconstructed
            print(f"  L(E,2) reconstructed = {reconstructed}")
            print(f"  residual             = {resid}")
            print(f"  |residual|           = {float(abs(resid)):.3e}")
            if abs(resid) < mpf(10)**(-40):
                print()
                print("  ✓ STRONG MATCH — L(E, 2) ∈ Q[basis]")
                print("  Path A advances: K_2-regulator route delivers a closed form")
                # Pretty-print
                terms = []
                for name, coeff in zip(names, rel[1:]):
                    if coeff != 0:
                        num = -coeff
                        den = rel[0]
                        from math import gcd
                        g = gcd(abs(num), abs(den))
                        if g > 1:
                            num, den = num // g, den // g
                        if abs(den) == 1:
                            terms.append(f"{num:+d}·{name}")
                        else:
                            terms.append(f"({num:+d}/{den})·{name}")
                print(f"  L(E, 2) = {' '.join(terms)}")
            else:
                print()
                print("  PSLQ found a relation but residual is not closed-form tight.")
                print("  This suggests a degenerate basis or a near-miss.")
        else:
            print(f"  rel[0] = 0; basis sub-relation, not L(E,2) closed form.")
except Exception as e:
    print(f"  PSLQ failed: {e}")
    import traceback
    traceback.print_exc()

print()
print("─── Done ─────────────────────────────────────────────────────")
print()
print("Note on precision: L(E, 2) computed by truncating at N_max = 5000.")
print(f"Truncation error is ~ a_n_max / n_max² ~ 1/{N_max}² ≈ {float(mpf(1)/N_max**2):.3e}")
print("If PSLQ residual is much smaller than truncation error, the relation")
print("is tight. If comparable, increase N_max and re-run.")
