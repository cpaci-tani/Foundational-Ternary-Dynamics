"""
proof_barnes_g_quarter_identity.py — verifies the Adamchik–Kinkelin Barnes G
ratio identity at quarter-integer arguments in FTD-canonical form.

Identity (PSLQ-discovered 2026-05-27, classical in literature):

    log G(1/4) − log G(3/4)
        = −(1/2)·log G* − (1/8)·log 2 − (1/4)·log π − G_Catalan/(2π)

Equivalently:
    G(3/4)/G(1/4) = (G*)^(1/2) · 2^(1/8) · π^(1/4) · exp(G_Catalan/(2π))

where:
    G(z) is the Barnes G-function
    G* = Γ(1/4)/Γ(3/4) (FTD lemniscatic constant, Theorem 1)
    G_Catalan ≈ 0.9159655941… (Catalan's constant)

The identity ties G* into the multiple-gamma hierarchy at the natural CM
point τ = i (cf. Theta-nullwert and Parity-twist readings of G* in
EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md Parts B and C). It positions G* one
level higher than its role in the classical Γ-ratio: at the Barnes G level,
G* combines with Catalan's constant via the explicit closed-form coefficient
1/(2π).

This identity is a synthesis of classical material (Kinkelin 1860; Adamchik
1998 "Multiple Gamma Function"; Choi 2003 "Some integral representations of
the Clausen function") rewritten in FTD's canonical basis where G* — not
Γ(1/4) — is the fundamental constant. It is NOT new mathematics. It is filed
to the algebraic-identity catalog because the FTD-canonical form makes
visible the additive role of G_Catalan/(2π) alongside the multiplicative
role of G* in the Barnes G ratio.

Tags: [SYNTHESIS] (operationally useful re-statement of classical material).

Run: python scripts/proofs/proof_barnes_g_quarter_identity.py
Expected: residual = 0 to at least 200 decimal digits.
"""

import sys
import io

# Ensure UTF-8 output
if sys.stdout.encoding != 'utf-8':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

from mpmath import mp, mpf, gamma, barnesg, log, pi, catalan

mp.dps = 250  # 250-digit precision; identity verified at this level

# ── Constants ─────────────────────────────────────────────────────────
G_star    = gamma(mpf('0.25')) / gamma(mpf('0.75'))
log_Gstar = log(G_star)
log_2     = log(mpf(2))
log_pi    = log(pi)
G_cat     = catalan

# ── Barnes G evaluations ──────────────────────────────────────────────
logBarnes_q  = log(barnesg(mpf('0.25')))
logBarnes_3q = log(barnesg(mpf('0.75')))
lhs = logBarnes_q - logBarnes_3q

# ── Predicted RHS ─────────────────────────────────────────────────────
rhs = (-log_Gstar / 2
       - log_2     / 8
       - log_pi    / 4
       - G_cat / (2 * pi))

residual = lhs - rhs

# ── Report ────────────────────────────────────────────────────────────
print("Identity:")
print("  log G(1/4) - log G(3/4)")
print("    = -(1/2)·log G* - (1/8)·log 2 - (1/4)·log π - G_Catalan/(2π)")
print()
print(f"Precision: mp.dps = {mp.dps}")
print()
print(f"LHS  = log G(1/4) - log G(3/4)")
print(f"     = {lhs}")
print()
print(f"RHS  = -(1/2)·log G* - (1/8)·log 2 - (1/4)·log π - G_Catalan/(2π)")
print(f"     = {rhs}")
print()
print(f"residual = LHS - RHS")
print(f"         = {residual}")
print()
print(f"|residual|         = {float(abs(residual)):.3e}")
print(f"Precision floor    = 10^-{mp.dps}")
print()

# Pass/fail with threshold at 10^(-200) to be conservative
threshold = mpf(10)**(-200)
if abs(residual) < threshold:
    print(f"  ✓ PASS — identity holds to better than 10^-200.")
    print()
    print("Equivalent multiplicative form:")
    print("  G(3/4)/G(1/4) = (G*)^(1/2) · 2^(1/8) · π^(1/4) · exp(G_Catalan/(2π))")
    print()
    # Verify multiplicative form too
    from mpmath import exp as mp_exp
    mult_lhs = barnesg(mpf('0.75')) / barnesg(mpf('0.25'))
    mult_rhs = (G_star**(mpf('0.5'))
                * mpf(2)**(mpf('1')/mpf('8'))
                * pi**(mpf('1')/mpf('4'))
                * mp_exp(G_cat / (2*pi)))
    mult_resid = mult_lhs - mult_rhs
    print(f"  LHS = G(3/4)/G(1/4)        = {mult_lhs}")
    print(f"  RHS = (G*)^(1/2)·2^(1/8)·π^(1/4)·exp(G_Catalan/(2π))")
    print(f"                              = {mult_rhs}")
    print(f"  residual (multiplicative) = {mult_resid}")
    print(f"  |residual|                 = {float(abs(mult_resid)):.3e}")
else:
    print(f"  ✗ FAIL — residual {float(abs(residual)):.3e} exceeds 10^-200 threshold.")
