"""
check_zeta_hessian_gstar2.py — Path A first computation per 2026-05-27 plan.

Tests whether the second-order J-chain identity holds in closed form:

   ζ''(0, 1/4) - ζ''(0, 3/4)  =?  ± log(16 G*²)

The first-order identity is theorem-grade (FTD-0002, MATH_LOG_GSTAR_IDENTITY.md):

   ζ'(0, 1/4) - ζ'(0, 3/4) = log G*    where G* = Γ(1/4)/Γ(3/4)

The second-order test asks whether the analogous Hurwitz-ζ second derivative
lands on the master quadratic's sum-of-roots S = 16·G*² in closed form. A
PASS opens the multi-week Path A program toward deriving (S, P_*) = (16G*²,
16G*³) independently of the master quadratic's algebraic construction. A FAIL
closes the naive Hessian-route hypothesis and pushes to K_2-regulator-of-E
machinery (Beilinson conjectures, Brunault–Zudilin's Mahler-measure monograph).

Honest prior: 60/40 FAIL. Either outcome is informative.

Run: python scripts/exploration/check_zeta_hessian_gstar2.py
"""

from mpmath import mp, mpf, gamma, zeta, log, diff, pi, euler, mpc

mp.dps = 80  # 80-digit precision

# ── Reference constants ───────────────────────────────────────────────
G_star = gamma(mpf('0.25')) / gamma(mpf('0.75'))
log_Gstar_direct = log(G_star)

print(f"G* = Γ(1/4)/Γ(3/4)")
print(f"  G*                  = {G_star}")
print(f"  log G* (direct)     = {log_Gstar_direct}")
print()

# ── Sanity check: first-order J-chain identity ────────────────────────
print("─── First-order identity (sanity check) ───────────────────────")
zeta1_q  = diff(lambda s: zeta(s, mpf('0.25')), 0, 1)
zeta1_3q = diff(lambda s: zeta(s, mpf('0.75')), 0, 1)

candidate_1 = zeta1_q - zeta1_3q
residual_1  = candidate_1 - log_Gstar_direct

print(f"  ζ'(0, 1/4)          = {zeta1_q}")
print(f"  ζ'(0, 3/4)          = {zeta1_3q}")
print(f"  ζ'(0,1/4) - ζ'(0,3/4) = {candidate_1}")
print(f"  log G* (via ζ')     = {candidate_1}")
print(f"  log G* (direct)     = {log_Gstar_direct}")
print(f"  residual            = {residual_1}")
print(f"  |residual|/|log G*|  = {float(abs(residual_1) / abs(log_Gstar_direct)):.3e}")
print()

# ── Second-order test ─────────────────────────────────────────────────
print("─── Second-order test (the new computation) ───────────────────")
zeta2_q  = diff(lambda s: zeta(s, mpf('0.25')), 0, 2)
zeta2_3q = diff(lambda s: zeta(s, mpf('0.75')), 0, 2)

candidate_2     =  (zeta2_q - zeta2_3q)
candidate_2_neg = -(zeta2_q - zeta2_3q)

target_S       = log(16 * G_star**2)         # log(16 G*²) = log S
target_S_neg   = -target_S
target_log16Gstar2 = 2 * log_Gstar_direct + log(16)  # equivalent form

print(f"  ζ''(0, 1/4)              = {zeta2_q}")
print(f"  ζ''(0, 3/4)              = {zeta2_3q}")
print(f"  ζ''(0,1/4) - ζ''(0,3/4)  = {candidate_2}")
print(f"  -(above)                 = {candidate_2_neg}")
print()
print(f"  target log(16·G*²)       = {target_S}")
print(f"  target -log(16·G*²)      = {target_S_neg}")
print()

# Try multiple natural target forms
targets = {
    'log(16·G*²)':           target_S,
    '-log(16·G*²)':         -target_S,
    'log(16) + 2 log G*':    log(16) + 2 * log_Gstar_direct,
    'log(G*²)':              2 * log_Gstar_direct,
    '-log(G*²)':            -2 * log_Gstar_direct,
    'log(8·G*²)':            log(8 * G_star**2),
    '(log G*)²':             log_Gstar_direct**2,
    'log(16·G*³)':           log(16 * G_star**3),
    '-log(2π)':             -log(2 * pi),                  # constant term in ζ''(0)
    'log(G*) · log(2)':      log_Gstar_direct * log(2),
}

candidates = {
    'ζ''-diff':       candidate_2,
    '-(ζ''-diff)':    candidate_2_neg,
}

print("─── Residual table (candidate vs target) ──────────────────────")
print(f"{'candidate':25} {'target':25} {'residual':>30}")
print("─" * 82)
best = None
for c_name, c_val in candidates.items():
    for t_name, t_val in targets.items():
        residual = c_val - t_val
        rel = abs(residual) / max(abs(t_val), mpf('1e-80'))
        marker = " ← MATCH" if rel < mpf('1e-50') else (" ← near-match" if rel < mpf('1e-10') else "")
        line = f"{c_name:25} {t_name:25} {residual!s:>30}{marker}"
        if rel < mpf('1e-10'):
            print(line)
        if best is None or rel < best[0]:
            best = (rel, c_name, t_name, residual)
print()
print(f"Best fit: {best[1]} vs {best[2]}")
print(f"  residual    = {best[3]}")
print(f"  relative    = {float(best[0]):.3e}")
print()

# ── Verdict ────────────────────────────────────────────────────────────
print("─── Verdict ───────────────────────────────────────────────────")
if best[0] < mpf('1e-50'):
    print(f"  PASS — closed-form second-order J-chain identity found:")
    print(f"         {best[1]}  =  {best[2]}")
    print(f"  Path A multi-week program is on the right track.")
elif best[0] < mpf('1e-10'):
    print(f"  PARTIAL — near-match found, residual {float(best[0]):.3e} suggests")
    print(f"            additional small correction term.")
else:
    print(f"  FAIL — no closed-form match in the naive target list.")
    print(f"         Best residual {float(best[0]):.3e} is far above closed-form precision.")
    print(f"  Path A naive Hessian-identity hypothesis is CLOSED-NEGATIVE.")
    print(f"  Next layer: K_2-regulator-of-E machinery (Beilinson conjectures).")
print()
print(f"  Numerical reference (for the PREREG §3 status note):")
print(f"    G*                            = {G_star.real if isinstance(G_star, mpc) else G_star}")
print(f"    log(16·G*²)                   = {target_S}")
print(f"    ζ''(0,1/4) - ζ''(0,3/4)       = {candidate_2}")
print(f"    -(ζ''(0,1/4) - ζ''(0,3/4))    = {candidate_2_neg}")
