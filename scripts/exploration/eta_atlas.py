"""
Compute |η(τ_K)|^{2 w_K} for all 9 class-number-one IQ fields and verify
the Chowla-Selberg formula in the form
  |η(τ_K)|^{2 w_K} = c_K · G_K^{w_K/2}
for explicit c_K depending on |d_K|, π, and elementary factors.

This is the η-tower extension of Paper A's Δ(i) = G_G^12/64 and
Paper A §15's Δ(ρ) = -G_ρ^12/6912 to the full atlas.
"""

from mpmath import mp, mpf, mpc, pi, gamma, sqrt, exp, agm, log, pslq

mp.dps = 50

def eta(tau, n_terms=400):
    q = exp(2j * pi * tau)
    val = mpc(1)
    for n in range(1, n_terms):
        val *= (1 - q**n)
    return q**(mpf(1)/24) * val

def kronecker_chi(d_abs, a):
    if d_abs == 4:
        if a % 2 == 0: return 0
        return 1 if a % 4 == 1 else -1
    if d_abs == 8:
        if a % 2 == 0: return 0
        m = a % 8
        return 1 if m in (1, 3) else -1
    if d_abs == 3:
        m = a % 3
        return 0 if m == 0 else (1 if m == 1 else -1)
    if d_abs % 2 == 1:
        p = d_abs
        if a % p == 0: return 0
        e = pow(a, (p - 1) // 2, p)
        return 1 if e == 1 else (-1 if e == p - 1 else 0)
    return 0

def G_K_const(d_abs, w_K):
    result = mpf(1)
    for a in range(1, d_abs):
        chi = kronecker_chi(d_abs, a)
        if chi != 0:
            result *= gamma(mpf(a) / d_abs) ** (chi * w_K / mpf(4))
    return result

# Atlas: (name, d, |d_K|, w_K, tau_K)
atlas = [
    ("Q(i)",      1,  4,  4, mpc(0, 1)),
    ("Q(sqrt-2)", 2,  8,  2, mpc(0, sqrt(2))),
    ("Q(rho)",    3,  3,  6, mpc(mpf(1)/2, sqrt(3)/2)),
    ("Q(sqrt-7)", 7,  7,  2, mpc(mpf(1)/2, sqrt(7)/2)),
    ("Q(sqrt-11)",11, 11, 2, mpc(mpf(1)/2, sqrt(11)/2)),
    ("Q(sqrt-19)",19, 19, 2, mpc(mpf(1)/2, sqrt(19)/2)),
    ("Q(sqrt-43)",43, 43, 2, mpc(mpf(1)/2, sqrt(43)/2)),
    ("Q(sqrt-67)",67, 67, 2, mpc(mpf(1)/2, sqrt(67)/2)),
    ("Q(sqrt-163)",163,163,2,mpc(mpf(1)/2, sqrt(163)/2)),
]

print("=" * 90)
print("η-tower across the class-number-one atlas")
print("=" * 90)
print()

# Compute |η(τ_K)|^{2 w_K} and G_K, and find the ratio
# Chowla-Selberg in clean form: |η(τ_K)|^{2 w_K / h_K} · |d_K|^{w_K/(4 h_K)} · (2π)^{-w_K/(2 h_K)}
#                              = const · G_K^{w_K/2 / h_K}
# At h_K=1: |η|^{2w} · |d_K|^{w/4} / (2π)^{w/2} = const · G_K^{w/2}

# Strategy: compute LHS = |η(τ_K)|^{2 w_K} · |d_K|^{w_K/4} / (2π)^{w_K/2}
# Compute RHS = G_K^{w_K/2}
# Ratio LHS/RHS should be a clean rational constant

print(f"{'K':<14}  {'w':>2}  {'|η(τ_K)|^(2w)':>30}  {'G_K^(w/2)':>30}  {'ratio':>20}")
print("-" * 110)
for name, d, d_K, w_K, tau in atlas:
    eta_tau = eta(tau)
    abs_eta_2w = abs(eta_tau) ** (2 * w_K)
    G_K = G_K_const(d_K, w_K)
    G_K_pow = G_K ** (mpf(w_K) / 2)
    # Normalised LHS
    LHS = abs_eta_2w * mpf(d_K) ** (mpf(w_K) / 4) / (2 * pi) ** (mpf(w_K) / 2)
    ratio = LHS / G_K_pow
    print(f"{name:<14}  {w_K:>2}  {float(abs_eta_2w):>30.15e}  {float(G_K_pow):>30.15e}  {float(ratio):>20.15f}")

print()

# For d=1: w_K = 4, so |η(i)|^8 · 4 / (2π)^2 = const · G_K^2
# |η(i)|^8 = (G_G/2^(1/2))^4 = G_G^4/4 — wait, |η(i)| is real positive, |η(i)|^2 = G_G/2^(1/2)... let me recompute
# η(i) = G_G^(1/2)/2^(1/4), so |η(i)| = G_G^(1/2)/2^(1/4)
# |η(i)|^8 = G_G^4 / 4
# G_K(d=1) = G* = 2√π G_G
# G_K^2 = 4π G_G^2
# LHS = |η(i)|^8 · 4 / (4π^2) = (G_G^4/4) · 4 / (4π^2) = G_G^4 / (4π^2)
# RHS = G_K^2 = 4π G_G^2
# Ratio LHS/RHS = G_G^4/(4π^2) / (4π G_G^2) = G_G^2 / (16 π^3)

print("VERIFY d=1: theoretical ratio = G_G^2/(16 π^3)")
G_G = 1/agm(1, sqrt(2))
theoretical_ratio_d1 = G_G**2 / (16 * pi**3)
print(f"  G_G^2/(16 π^3) = {float(theoretical_ratio_d1):.15f}")
print()

# The Chowla-Selberg formula doesn't give a CONSTANT ratio across the atlas
# (the η-power and G_K-power are different at each w_K), so this isn't quite right.

# Better approach: for each field, derive the EXPLICIT closed form of |η(τ_K)|
# directly via the Chowla-Selberg formula:
#   |η(τ_K)|^{2 w_K} = (2π)^{w_K/2} · |d_K|^{-w_K/4} · const_K · G_K^{w_K/2}
# where const_K is the "Chowla-Selberg residue" (depends on |d_K|, computed via Gauss sums)

# For h_K = 1, the Chowla-Selberg constant has the explicit form (Selberg-Chowla 1967):
#   |η(τ_K)|^2 = (2π)^{1/2} · |d_K|^{-1/4} · (prod_a Γ(a/|d|)^{chi(a)/(2 w_K)}) · h_K^{...}

# Simpler: just record the empirical closed forms for each field via PSLQ.

print("=" * 90)
print("Empirical closed-form search: |η(τ_K)|^{2 w_K} as polynomial in (G_K, π, |d_K|)")
print("=" * 90)
print()

mp.dps = 60
for name, d, d_K, w_K, tau in atlas[:5]:
    eta_tau = eta(tau)
    target = abs(eta_tau) ** (2 * w_K)
    G_K = G_K_const(d_K, w_K)

    # Try to match target = G_K^a · π^b · |d_K|^c with small rationals a, b, c
    # Search over log-basis
    log_target = log(target)
    log_GK = log(G_K)
    log_pi = log(pi)
    log_dK = log(mpf(d_K))

    rel = pslq([log_target, log_GK, log_pi, log_dK, log(mpf(2))], maxcoeff=10**4)
    if rel is not None and rel[0] != 0:
        a0, a1, a2, a3, a4 = rel
        # target = G_K^(-a1/a0) · π^(-a2/a0) · d_K^(-a3/a0) · 2^(-a4/a0)
        print(f"{name}:")
        print(f"  |η(τ_K)|^{2*w_K} = G_K^{-mpf(a1)/a0} · π^{-mpf(a2)/a0} · d_K^{-mpf(a3)/a0} · 2^{-mpf(a4)/a0}")
        # Verify
        val = G_K**(-mpf(a1)/a0) * pi**(-mpf(a2)/a0) * mpf(d_K)**(-mpf(a3)/a0) * mpf(2)**(-mpf(a4)/a0)
        err = abs(target - val) / target
        print(f"  Relative error: {float(err):.2e}")
        print()
    else:
        print(f"{name}: no clean closed form found at maxcoeff=10^4")
        print()
