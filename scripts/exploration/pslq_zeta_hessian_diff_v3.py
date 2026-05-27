"""
pslq_zeta_hessian_diff_v3.py — v2 surfaced the Adamchik-Glaisher Barnes G
identity (a known result) but not c's closed form. v3 drops the redundant
Barnes G basis element and broadens products in G_Catalan, since the
Adamchik formula for ζ''(0, a) at a = 1/4 involves Catalan-G in the
non-trivial part.

Discovery from v2 (for the record): The Adamchik-Glaisher identity
  8·log[G_Barnes(1/4)/G_Barnes(3/4)] = -4·log G* - log 2 - 2·log π - 4·G_Catalan/π
or equivalently
  log G_Barnes(1/4) - log G_Barnes(3/4) = -(log G*)/2 - (log 2)/8 - (log π)/4 - G_Catalan/(2π)

So Barnes G is expressible in {log G*, log 2, log π, G_Catalan/π}.
Drop it.

Run: python scripts/exploration/pslq_zeta_hessian_diff_v3.py
"""

from mpmath import mp, mpf, gamma, zeta, log, diff, pi, catalan, pslq, euler, glaisher

mp.dps = 200  # head-room

print("Computing target c = ζ''(0,1/4) − ζ''(0,3/4) at 200-digit precision…")
zeta2_q  = diff(lambda s: zeta(s, mpf('0.25')), 0, 2)
zeta2_3q = diff(lambda s: zeta(s, mpf('0.75')), 0, 2)
c = zeta2_q - zeta2_3q
print(f"  c = {c}")
print()

# ── Non-redundant basis with more product terms ───────────────────────
G_star    = gamma(mpf('0.25')) / gamma(mpf('0.75'))
log_Gstar = log(G_star)
log_2     = log(mpf(2))
log_pi    = log(pi)
G_cat     = catalan
gamma_em  = euler

# Drop Barnes G (redundant per v2 discovery).
# Add products involving G_Catalan since Adamchik ζ''(0,a) for a=1/4 includes
# Catalan-type terms in its non-trivial sector.
basis = {
    '1':                          mpf(1),
    'G_Catalan':                  G_cat,
    'log G*':                     log_Gstar,
    'log 2':                      log_2,
    'log π':                      log_pi,
    'γ':                          gamma_em,
    '(log G*)²':                  log_Gstar**2,
    'log G* · log 2':             log_Gstar * log_2,
    'log G* · log π':             log_Gstar * log_pi,
    'γ · log G*':                 gamma_em * log_Gstar,
    'γ · log 2':                  gamma_em * log_2,
    'γ · log π':                  gamma_em * log_pi,
    'γ²':                         gamma_em**2,
    'π²':                         pi**2,
    'G_Catalan / π':              G_cat / pi,
    'G_Catalan · log G*':         G_cat * log_Gstar,
    'G_Catalan · log 2':          G_cat * log_2,
    'G_Catalan · log π':          G_cat * log_pi,
    'G_Catalan · γ':              G_cat * gamma_em,
    'π · log G*':                 pi * log_Gstar,
    '(log 2)²':                   log_2**2,
    '(log π)²':                   log_pi**2,
    'log 2 · log π':              log_2 * log_pi,
    'log Glaisher (A)':           log(glaisher),  # Glaisher-Kinkelin
}

names = list(basis.keys())
vals  = [basis[n] for n in names]

# ── Iterative PSLQ: find a relation, eliminate it, repeat until c is reached ──
def fmt_relation(rel, basis_names, c_label='c'):
    """Format a PSLQ relation as a readable string."""
    terms = []
    if rel[0] != 0:
        terms.append(f"{rel[0]}·{c_label}")
    for i, coeff in enumerate(rel[1:]):
        if coeff != 0:
            terms.append(f"{coeff:+d}·{basis_names[i]}")
    return " + ".join(terms) + " = 0"

print(f"PSLQ pass 1 (full basis, c included) at maxcoeff=50000…")
full_inputs = [c] + vals
try:
    rel1 = pslq(full_inputs, tol=mpf(10)**(-120), maxcoeff=50000, maxsteps=20000)
    print(f"  Result: {rel1}")
    print(f"  Largest |coeff|: {max(abs(r) for r in rel1)}")
    print(f"  Verbalised: {fmt_relation(rel1, names)}")
    print()
except Exception as e:
    print(f"  PSLQ failed: {e}")
    rel1 = None

if rel1 is not None and rel1[0] != 0:
    # Reconstruct c
    reconstructed = -sum(rel1[i+1] * vals[i] for i in range(len(vals))) / rel1[0]
    resid = c - reconstructed
    print(f"  c (target)        = {c}")
    print(f"  c (reconstructed) = {reconstructed}")
    print(f"  residual          = {resid}")
    print(f"  |residual|        = {float(abs(resid)):.3e}")
    if abs(resid) < mpf(10)**(-100):
        print()
        print("  ✓ CLOSED FORM FOUND.")
        print()
        # Pretty-print
        sign_c = 1 if rel1[0] < 0 else -1  # we want c = positive sum
        denom = abs(rel1[0])
        print(f"  Closed form (denom = {denom}):")
        print(f"  c = ", end='')
        first = True
        for name, coeff in zip(names, rel1[1:]):
            if coeff == 0: continue
            num = sign_c * coeff
            if denom == 1:
                term = f"{num:+d}·{name}"
            else:
                term = f"{num:+d}/{denom}·{name}"
            if first:
                print(term, end='')
                first = False
            else:
                print(f" {term}", end='')
        print()
    else:
        print()
        print("  Relation found but residual large — likely a degenerate basis sub-relation.")

elif rel1 is not None and rel1[0] == 0:
    # Trivial basis dependency. Identify and remove.
    print("  PSLQ found a basis sub-relation (rel[0]=0):")
    print(f"    {fmt_relation(rel1, names)}")
    # Find the basis element with the largest coefficient and eliminate it
    nonzero_idx = [i for i, coeff in enumerate(rel1[1:]) if coeff != 0]
    if nonzero_idx:
        largest_i = max(nonzero_idx, key=lambda i: abs(rel1[i+1]))
        print(f"  Eliminating basis element with largest |coeff|: '{names[largest_i]}'")
        # Solve for vals[largest_i] in terms of others, then drop it
        # (This is a manual step; for now, drop and re-run.)
        new_names = [names[i] for i in range(len(names)) if i != largest_i]
        new_vals  = [vals[i] for i in range(len(vals)) if i != largest_i]
        print()
        print(f"PSLQ pass 2 (after dropping '{names[largest_i]}') at maxcoeff=50000…")
        try:
            rel2 = pslq([c] + new_vals, tol=mpf(10)**(-120), maxcoeff=50000, maxsteps=20000)
            print(f"  Result: {rel2}")
            print(f"  Largest |coeff|: {max(abs(r) for r in rel2)}")
            print(f"  Verbalised: {fmt_relation(rel2, new_names)}")
            print()
            if rel2[0] != 0:
                reconstructed = -sum(rel2[i+1] * new_vals[i] for i in range(len(new_vals))) / rel2[0]
                resid = c - reconstructed
                print(f"  residual = {resid}")
                print(f"  |resid|  = {float(abs(resid)):.3e}")
                if abs(resid) < mpf(10)**(-100):
                    print("  ✓ CLOSED FORM FOUND (pass 2).")
        except Exception as e:
            print(f"  PSLQ pass 2 failed: {e}")
