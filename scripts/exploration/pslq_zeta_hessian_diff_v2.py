"""
pslq_zeta_hessian_diff_v2.py — second pass after v1 found only a trivial dependency.

v1 found log(2π) = log 2 + log π (trivial); didn't relate to c.
v2 drops log(2π) and adds Barnes G(1/4)/G(3/4) difference, Euler γ, and γ²,
which is what Adamchik's identity for ζ''(0, a) actually involves.

Run: python scripts/exploration/pslq_zeta_hessian_diff_v2.py
"""

from mpmath import mp, mpf, gamma, zeta, log, diff, pi, catalan, pslq, barnesg, euler

mp.dps = 150  # head-room for PSLQ

print("Computing target c = ζ''(0,1/4) − ζ''(0,3/4) at 150-digit precision…")
zeta2_q  = diff(lambda s: zeta(s, mpf('0.25')), 0, 2)
zeta2_3q = diff(lambda s: zeta(s, mpf('0.75')), 0, 2)
c = zeta2_q - zeta2_3q
print(f"  c = {c}")
print()

# ── Build a non-redundant basis ──────────────────────────────────────
G_star      = gamma(mpf('0.25')) / gamma(mpf('0.75'))
log_Gstar   = log(G_star)
log_2       = log(mpf(2))
log_pi      = log(pi)
G_cat       = catalan
gamma_em    = euler  # Euler-Mascheroni γ
logBarnes_q  = log(barnesg(mpf('0.25')))
logBarnes_3q = log(barnesg(mpf('0.75')))
logBarnes_diff = logBarnes_q - logBarnes_3q

basis = {
    '1':                     mpf(1),
    'G_Catalan':             G_cat,
    'log G*':                log_Gstar,
    'log 2':                 log_2,
    'log π':                 log_pi,
    'γ':                     gamma_em,
    'log G(1/4) - log G(3/4)': logBarnes_diff,
    '(log G*)²':             log_Gstar**2,
    'log G* · log 2':        log_Gstar * log_2,
    'log G* · log π':        log_Gstar * log_pi,
    'γ · log G*':            gamma_em * log_Gstar,
    'γ · log 2':             gamma_em * log_2,
    'γ · log π':             gamma_em * log_pi,
    'γ²':                    gamma_em**2,
    'π²':                    pi**2,
    'G_Catalan / π':         G_cat / pi,
}

names = list(basis.keys())
vals  = [basis[n] for n in names]

print("Basis values:")
for n, v in zip(names, vals):
    print(f"  {n:35} = {float(v):>20.10f}")
print()

# ── PSLQ search ──────────────────────────────────────────────────────
print("Running PSLQ on [c, basis…] at 150-digit precision, maxcoeff=10000…")
print()

try:
    full_inputs = [c] + vals
    rel = pslq(full_inputs, tol=mpf(10)**(-100), maxcoeff=10000, maxsteps=5000)
    print(f"PSLQ result: {rel}")
    print(f"Largest |coeff|: {max(abs(r) for r in rel)}")
    print()
    if rel[0] != 0:
        # Express c
        print(f"Relation found: {rel[0]}·c + (sum over basis) = 0")
        print()
        # Reconstruct
        reconstructed = mpf(0)
        for i, (name, coeff) in enumerate(zip(names, rel[1:])):
            if coeff != 0:
                reconstructed += coeff * vals[i]
        # c = -reconstructed / rel[0]
        c_pred = -reconstructed / rel[0]
        resid = c - c_pred
        print(f"  c (computed)    = {c}")
        print(f"  c (reconstructed)= {c_pred}")
        print(f"  residual         = {resid}")
        print(f"  |resid|          = {float(abs(resid)):.3e}")
        if abs(resid) < mpf(10)**(-50):
            print()
            print("  ✓ CLOSED FORM FOUND.")
            terms = []
            for name, coeff in zip(names, rel[1:]):
                if coeff != 0:
                    num = -coeff
                    den = rel[0]
                    if abs(num) == abs(den):
                        sign = '+' if num*den > 0 else '-'
                        terms.append(f"{sign} {name}")
                    elif abs(den) == 1:
                        sign = '+' if num*den > 0 else '-'
                        terms.append(f"{sign} {abs(num)} · {name}")
                    else:
                        sign = '+' if num*den > 0 else '-'
                        from math import gcd
                        g = gcd(abs(num), abs(den))
                        if g > 1:
                            num, den = num // g, den // g
                        if abs(den) == 1:
                            terms.append(f"{sign} {abs(num)} · {name}")
                        else:
                            terms.append(f"{sign} ({abs(num)}/{abs(den)}) · {name}")
            print(f"  c = {' '.join(terms)}")
        else:
            print()
            print(f"  PSLQ found a relation but residual is large — likely a degenerate")
            print(f"  basis dependency rather than a closed form for c.")
            print(f"  Inspect coefficients: {rel}")
    else:
        print("rel[0] = 0; PSLQ found dependencies in the basis but not involving c.")
        print(f"Coefficient pattern: {rel}")
        for i, coeff in enumerate(rel[1:]):
            if coeff != 0:
                print(f"  {names[i]}: coeff = {coeff}")

except Exception as e:
    print(f"PSLQ failed: {e}")
    import traceback
    traceback.print_exc()

print()
print("─── Targeted Adamchik-form trial (independent of PSLQ) ────────")
# Adamchik's identity for ζ''(0,a) involves Barnes G via:
#   ζ''(0,a) = (log Γ(a))² - 2 log Γ(a) · log(2π)^(1/2) + (γ + log(2π))·log Γ(a) - 2 log G(a) + const
# The difference at 1/4 and 3/4 eliminates the const term.
# Compute the standard Adamchik-form prediction:
logG_q  = log(gamma(mpf('0.25')))
logG_3q = log(gamma(mpf('0.75')))
log2pi  = log(2*pi)

# Try: c =? (log Γ(1/4))² - (log Γ(3/4))² + (γ + log(2π)) · log G* - 2·(log G(1/4) - log G(3/4))
term1 = logG_q**2 - logG_3q**2
term2 = (gamma_em + log2pi) * log_Gstar
term3 = -2 * logBarnes_diff
adamchik_pred = term1 + term2 + term3

print(f"  (log Γ(1/4))² - (log Γ(3/4))²      = {term1}")
print(f"  (γ + log(2π)) · log G*              = {term2}")
print(f"  -2 · (log G(1/4) - log G(3/4))      = {term3}")
print(f"  Adamchik-form prediction            = {adamchik_pred}")
print(f"  c (target)                          = {c}")
print(f"  residual                            = {c - adamchik_pred}")
print(f"  |residual|                          = {float(abs(c - adamchik_pred)):.3e}")
