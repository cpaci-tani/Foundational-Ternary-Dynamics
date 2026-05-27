"""
pslq_zeta_hessian_diff.py — find a closed form for ζ''(0,1/4) − ζ''(0,3/4).

The 2026-05-27 check (check_zeta_hessian_gstar2.py) showed:
    c := ζ''(0,1/4) − ζ''(0,3/4) ≈ 1.8138033412487484856943…

This is NOT log(16·G*²) (the naive J-chain extension); the residual is at
order 1 and no simple combination from a 10-target sweep lands on c.

This script runs mpmath.pslq against a basis of natural transcendentals
(Catalan's constant, log G*, log(2π), log 2, π, etc.) to search for an
integer relation. Adamchik's identity for ζ''(0, a) at rational a involves
the Barnes G function and Catalan-type Dirichlet L-values; the result for
a = 1/4 (vs 3/4) should be expressible in elementary closed form via
reflection identities. PSLQ will find any small-integer relation that
exists.

Honest prior:
- High probability: c lies in span(G_Catalan, log G*, log(2π), log 2, log G*·log(2π))
  with small-integer coefficients (the Adamchik-Coffey territory).
- The residual c - 2·G_Catalan ≈ -0.01813 (per first check) is suspicious —
  could be log G* · (small rational) or similar.

Run: python scripts/exploration/pslq_zeta_hessian_diff.py
"""

from mpmath import mp, mpf, gamma, zeta, log, diff, pi, catalan, pslq

mp.dps = 120  # 120-digit precision for PSLQ (need head-room for noise floor)

print("Computing target c = ζ''(0,1/4) − ζ''(0,3/4) at 120-digit precision…")
zeta2_q  = diff(lambda s: zeta(s, mpf('0.25')), 0, 2)
zeta2_3q = diff(lambda s: zeta(s, mpf('0.75')), 0, 2)
c = zeta2_q - zeta2_3q
print(f"  c = {c}")
print()

# ── Basis of natural transcendentals ──────────────────────────────────
G_star = gamma(mpf('0.25')) / gamma(mpf('0.75'))
log_Gstar = log(G_star)
log_2pi   = log(2 * pi)
log_2     = log(mpf(2))
log_pi    = log(pi)
G_cat     = catalan  # Catalan's constant

basis = {
    '1':                  mpf(1),
    'G_Catalan':          G_cat,
    'log G*':             log_Gstar,
    'log(2π)':            log_2pi,
    'log 2':              log_2,
    'log π':              log_pi,
    'π':                  pi,
    '(log G*)²':          log_Gstar**2,
    'log G* · log(2π)':   log_Gstar * log_2pi,
    'log G* · log 2':     log_Gstar * log_2,
    '(log(2π))²':         log_2pi**2,
    'π · log G*':         pi * log_Gstar,
    'G_Catalan / π':      G_cat / pi,
    'π · G_Catalan':      pi * G_cat,
}

names = list(basis.keys())
vals  = [basis[n] for n in names]

# ── PSLQ search ──────────────────────────────────────────────────────
# pslq([c, x_1, ..., x_n]) returns coefficients [a_0, ..., a_n] such that
# a_0·c + a_1·x_1 + ... + a_n·x_n ≈ 0 with small integers.
print("Running PSLQ on [c, basis...] with 120-digit precision…")
print()

def try_pslq(input_vals, input_names, tol_dps=80, maxcoeff=1000):
    """Run pslq with the given tolerance; return the relation if found."""
    try:
        rel = pslq(input_vals, tol=mpf(10)**(-tol_dps), maxcoeff=maxcoeff, maxsteps=2000)
        return rel
    except Exception as e:
        print(f"  pslq failed: {e}")
        return None

# Try the full basis first
full_inputs = [c] + vals
full_names  = ['c'] + names

rel = try_pslq(full_inputs, full_names, tol_dps=80, maxcoeff=10000)
if rel is not None:
    print("Full-basis PSLQ result:")
    print(f"  Integer coefficients: {rel}")
    print(f"  Largest |coeff|:      {max(abs(r) for r in rel)}")
    print()
    if rel[0] != 0:
        # Express c as a linear combination
        print(f"  c = {-rel[0]}^-1 · (linear combination):")
        print()
        terms = []
        for i, (name, coeff) in enumerate(zip(full_names[1:], rel[1:])):
            if coeff != 0:
                # We want c = -(sum_i rel[i+1] * basis_i) / rel[0]
                # So coeff_in_c = -rel[i+1] / rel[0]
                terms.append((name, -coeff, rel[0]))
        for name, num, den in terms:
            sign = '+' if num * den > 0 else '-'
            if abs(num) == abs(den):
                disp = f"{sign} {name}"
            elif abs(den) == 1:
                disp = f"{sign} {abs(num)} · {name}"
            else:
                disp = f"{sign} ({abs(num)}/{abs(den)}) · {name}"
            print(f"    {disp}")
        # Verify
        reconstructed = sum(-rel[i+1] / rel[0] * vals[i] for i in range(len(vals)))
        resid = c - reconstructed
        print()
        print(f"  reconstructed c = {reconstructed}")
        print(f"  residual        = {resid}")
        print(f"  |resid|         = {float(abs(resid)):.3e}")
    else:
        print("  rel[0] = 0; relation is among basis only (degenerate). Trying smaller basis…")
        rel = None

# If full basis didn't find anything, try smaller subsets
if rel is None:
    print()
    print("─── Trying smaller bases ─────────────────────────────────────")
    small_bases = [
        ('Catalan + log G* + log(2π)',     ['G_Catalan', 'log G*', 'log(2π)']),
        ('Catalan + log G* + log 2',       ['G_Catalan', 'log G*', 'log 2']),
        ('Catalan + log G* + log(2π) + 1', ['G_Catalan', 'log G*', 'log(2π)', '1']),
        ('Catalan + log G* + log 2 + 1',   ['G_Catalan', 'log G*', 'log 2', '1']),
        ('Catalan + (log G*)² + log G*',   ['G_Catalan', '(log G*)²', 'log G*']),
        ('Catalan + log G*·log(2π)',       ['G_Catalan', 'log G* · log(2π)']),
        ('Catalan only',                    ['G_Catalan']),
        ('Catalan + π',                     ['G_Catalan', 'π']),
        ('Catalan + log G* + 1',            ['G_Catalan', 'log G*', '1']),
    ]
    for label, sub_names in small_bases:
        sub_vals = [basis[n] for n in sub_names]
        sub_inputs = [c] + sub_vals
        sub_input_names = ['c'] + sub_names
        rel = try_pslq(sub_inputs, sub_input_names, tol_dps=60, maxcoeff=100000)
        if rel is not None and rel[0] != 0:
            reconstructed = sum(-rel[i+1] / rel[0] * sub_vals[i] for i in range(len(sub_vals)))
            resid = c - reconstructed
            tight = abs(resid) < mpf('1e-50')
            marker = " ← CLOSED FORM" if tight else ""
            print(f"  [{label}]: {rel}{marker}")
            print(f"    residual = {float(abs(resid)):.3e}")
            if tight:
                print(f"    relation: c = {reconstructed}")
                terms = []
                for name, coeff in zip(sub_names, rel[1:]):
                    if coeff != 0:
                        if rel[0] == -1:
                            terms.append(f"{coeff} · {name}")
                        else:
                            terms.append(f"({coeff}/{-rel[0]}) · {name}")
                print(f"    c = {' + '.join(terms)}")
                break

print()
print("─── Done ─────────────────────────────────────────────────────")
