"""
P2 investigation: search for a cubic-AGM expression for G_rho.

The paper asks: find an explicit expression of G_rho = Gamma(1/3) Gamma(1/6) / (2 pi sqrt pi)
in terms of the Borwein-Borwein cubic AGM M_3(a, b), defined by
  a_{n+1} = (a_n + 2 b_n)/3
  b_{n+1} = ( b_n * (a_n^2 + a_n*b_n + b_n^2) / 3 )^(1/3)

The lemniscatic analog is G_G = 1/AGM(1, sqrt 2).

Strategy:
1. Compute G_rho at 40-digit precision.
2. Implement M_3 carefully.
3. Compute M_3(1, x) at candidate cubic-special values (1/2, 2^{-1/3}, 3^{-1/3},
   sin(pi/3), cos(pi/9), etc.).
4. Test simple candidates G_rho =? 1/M_3(1, x_i) or M_3(1, x_i)^{1/3} / something
5. PSLQ on log-basis to detect relations.

This is an investigation, not a proof. Any positive finding lands as
[CONJECTURE -- numerical evidence] until a derivation is produced.
"""

from mpmath import (
    mp, mpf, mpc, gamma, pi, sqrt, agm, exp, log, fabs, pslq,
    hyper, ellipk
)

mp.dps = 50

# -------------------------------------------------------------------------
# Target: G_rho
# -------------------------------------------------------------------------

G_rho = gamma(mpf(1)/3) * gamma(mpf(1)/6) / (2*pi*sqrt(pi))
inv_G_rho = 1 / G_rho

print(f'G_rho     = {G_rho}')
print(f'1/G_rho   = {inv_G_rho}')
print()

# -------------------------------------------------------------------------
# Borwein-Borwein cubic AGM
# -------------------------------------------------------------------------

def M3(a, b, max_iter=200, tol_dps=45):
    """Borwein-Borwein cubic arithmetic-geometric mean.

    a_{n+1} = (a_n + 2 b_n) / 3
    b_{n+1} = ( b_n (a_n^2 + a_n*b_n + b_n^2) / 3 )^(1/3)

    Converges cubically when a, b > 0.
    """
    a, b = mpf(a), mpf(b)
    tol = mpf(10)**(-tol_dps)
    for k in range(max_iter):
        if fabs(a - b) < tol * fabs(a):
            return (a + b) / 2
        a_new = (a + 2*b) / 3
        # cube root taken with positive real branch (a, b > 0)
        b_new = (b * (a**2 + a*b + b**2) / 3) ** (mpf(1)/3)
        a, b = a_new, b_new
    return (a + b) / 2

# Sanity: M_3(1, 1) = 1
print(f'M_3(1, 1) = {M3(1, 1)}  (should be 1)')
# And M_3 should converge cubically; check rate informally
print()

# -------------------------------------------------------------------------
# Connection to 2F1: 1/M_3(1, (1-x^3)^(1/3)) = 2F1(1/3, 2/3; 1; x^3)
# (Borwein-Borwein 1991, cubic theta correspondence)
# Verify this identity numerically at a test value
# -------------------------------------------------------------------------

def F213(x3):
    """2F1(1/3, 2/3; 1; x^3) -- the canonical cubic-AGM hypergeometric."""
    return hyper([mpf(1)/3, mpf(2)/3], [1], x3)

# Test: take x = 1/2, so x^3 = 1/8; b = (1 - 1/8)^(1/3) = (7/8)^(1/3)
x_test = mpf(1)/2
x3_test = x_test**3
b_test = (1 - x3_test)**(mpf(1)/3)
lhs = 1 / M3(1, b_test)
rhs = F213(x3_test)
print(f'Borwein BB identity check at x = 1/2:')
print(f'  1/M_3(1, (1-1/8)^(1/3)) = {lhs}')
print(f'  2F1(1/3, 2/3; 1; 1/8)   = {rhs}')
print(f'  match: {fabs(lhs - rhs) < mpf(10)**(-40)}')
print()

# -------------------------------------------------------------------------
# Candidate cubic-AGM expressions to test against G_rho
# -------------------------------------------------------------------------
# Key candidate values for the second argument b:
candidates_b = {
    '1/2'           : mpf(1)/2,
    '1/sqrt(2)'     : 1/sqrt(2),
    '1/3^(1/3)'     : 1/mpf(3)**(mpf(1)/3),
    '1/2^(1/3)'     : 1/mpf(2)**(mpf(1)/3),
    '2^(-1/3)'      : mpf(2)**(-mpf(1)/3),
    '(1/2)^(1/3)'   : (mpf(1)/2)**(mpf(1)/3),
    'sqrt(3)/2'     : sqrt(3)/2,
    'cos(pi/6)'     : (sqrt(3)/2),
    '(7/8)^(1/3)'   : (mpf(7)/8)**(mpf(1)/3),
    '(1/4)^(1/3)'   : (mpf(1)/4)**(mpf(1)/3),
    '(3/4)^(1/3)'   : (mpf(3)/4)**(mpf(1)/3),
    'sin(pi/3)'     : (sqrt(3)/2),
}

print('Search: M_3(1, b) and 1/M_3(1, b) for candidate b values vs G_rho')
print(f'{"b":<20} {"M_3(1,b)":<25} {"1/M_3(1,b)":<25} {"closest match to G_rho or 1/G_rho?"}')
print('-' * 120)

best_diff = mpf('inf')
best_match = None

for label, b in candidates_b.items():
    if b <= 0 or b > 1.001:  # skip non-physical
        continue
    m3 = M3(1, b)
    inv = 1 / m3
    d1 = min(fabs(m3 - G_rho), fabs(inv - G_rho))
    d2 = min(fabs(m3 - inv_G_rho), fabs(inv - inv_G_rho))
    d  = min(d1, d2)
    note = ""
    if d < mpf("1e-30"):
        note = "*** EXACT MATCH ***"
    elif d < mpf("0.01"):
        note = f"|diff|={float(d):.6e}"
    if d < best_diff:
        best_diff = d
        best_match = (label, b, m3, inv)
    print(f'{label:<20} {float(m3):<25.15f} {float(inv):<25.15f} {note}')

print()
print(f'Closest candidate: b = {best_match[0]}')
print(f'  M_3(1, b)      = {best_match[2]}')
print(f'  1/M_3(1, b)    = {best_match[3]}')
print(f'  |closest - G_rho or 1/G_rho|: {best_diff}')

# -------------------------------------------------------------------------
# More structured search: relate G_rho to specific cubic-modular values
# Hypothesis: at the cubic-special modulus, M_3(1, x*) should give G_rho up to
# elementary factors of sqrt(3), pi, etc.
# -------------------------------------------------------------------------

print()
print('Structured search: G_rho =? c * M_3(1, b)^k or G_rho =? c / M_3(1, b)^k')
print('for c in {1, sqrt(3), 3, 1/sqrt(3), 1/3, sqrt(3)/2, ...} and k in {1, 1/3, 2/3, 1/2}')
print()

c_candidates = {
    '1'         : mpf(1),
    'sqrt(3)'   : sqrt(3),
    '1/sqrt(3)' : 1/sqrt(3),
    '3'         : mpf(3),
    '1/3'       : mpf(1)/3,
    '3^(1/4)'   : mpf(3)**(mpf(1)/4),
    '3^(1/6)'   : mpf(3)**(mpf(1)/6),
    'sqrt(3)/2' : sqrt(3)/2,
    '2/sqrt(3)' : 2/sqrt(3),
    '3^(2/3)'   : mpf(3)**(mpf(2)/3),
    '3^(-2/3)'  : mpf(3)**(-mpf(2)/3),
    '3^(3/4)'   : mpf(3)**(mpf(3)/4),
}

k_candidates = {
    '1'   : mpf(1),
    '1/3' : mpf(1)/3,
    '2/3' : mpf(2)/3,
    '1/2' : mpf(1)/2,
    '3/2' : mpf(3)/2,
    '-1'  : -mpf(1),
    '-1/3': -mpf(1)/3,
}

found = []
for b_label, b in candidates_b.items():
    if b <= 0 or b > 1.001:
        continue
    m3 = M3(1, b)
    for c_label, c in c_candidates.items():
        for k_label, k in k_candidates.items():
            try:
                val = c * m3**k
                d = fabs(val - G_rho)
                if d < mpf("1e-30"):
                    found.append((b_label, c_label, k_label, val, d))
            except Exception:
                pass

if found:
    print('CANDIDATE EXPRESSIONS for G_rho:')
    for b_label, c_label, k_label, val, d in found:
        print(f'  G_rho =? ({c_label}) * M_3(1, {b_label})^({k_label})')
        print(f'    value: {val}')
        print(f'    |diff|: {d}')
else:
    print('No simple cubic-AGM match found in {c * M_3(1, b)^k} family.')
    print('Trying broader 2F1 approach: G_rho via 2F1(1/3, 2/3; 1; t).')

# -------------------------------------------------------------------------
# Approach via 2F1 directly
# -------------------------------------------------------------------------

print()
print('2F1 approach: G_rho =? c * 2F1(1/3, 2/3; 1; t)^k for candidate t values')
print()

t_candidates = {
    '1/2'    : mpf(1)/2,
    '1/4'    : mpf(1)/4,
    '3/4'    : mpf(3)/4,
    '1/3'    : mpf(1)/3,
    '2/3'    : mpf(2)/3,
    '1/8'    : mpf(1)/8,
    '7/8'    : mpf(7)/8,
    '1/27'   : mpf(1)/27,
    '26/27'  : mpf(26)/27,
}

for t_label, t in t_candidates.items():
    val = F213(t)
    for c_label, c in c_candidates.items():
        for k_label, k in k_candidates.items():
            try:
                cand = c * val**k
                d = fabs(cand - G_rho)
                if d < mpf("1e-30"):
                    print(f'  G_rho = ({c_label}) * 2F1(1/3, 2/3; 1; {t_label})^({k_label})')
                    print(f'    value: {cand}')
                    print(f'    |diff|: {d}')
            except Exception:
                pass

# -------------------------------------------------------------------------
# PSLQ search: find integer relation between log G_rho and {log M_3(1, b), log c}
# -------------------------------------------------------------------------

print()
print('PSLQ: integer relations of log G_rho against log M_3(1, b), log of small constants')

# Take a basis: log G_rho, log M_3(1, 1/2), log M_3(1, 1/2^{1/3}), log 2, log 3, log pi, log sqrt(3)
basis_labels = ['log G_rho', 'log M_3(1, 1/2)', 'log M_3(1, 2^(-1/3))', 'log 2', 'log 3', 'log pi', 'log sqrt(3)']
basis_vals = [
    log(G_rho),
    log(M3(1, mpf(1)/2)),
    log(M3(1, mpf(2)**(-mpf(1)/3))),
    log(mpf(2)),
    log(mpf(3)),
    log(pi),
    log(sqrt(3)),
]

try:
    rel = pslq(basis_vals, tol=mpf(10)**(-35), maxcoeff=mpf(10)**12)
    if rel:
        print(f'Found relation (coefficients):')
        for c, lbl in zip(rel, basis_labels):
            print(f'  {c:>8d} * {lbl}')
        # Reformulate as expression for G_rho
        if rel[0] != 0:
            print(f'\nThis implies log G_rho = (-1/{rel[0]}) * sum_i (rel[i] * basis_vals[i] for i>=1)')
    else:
        print('  No integer relation at coefficient bound 10^12')
except Exception as e:
    print(f'  PSLQ raised: {e}')
