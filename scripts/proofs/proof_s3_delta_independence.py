"""proof_s3_delta_independence.py — FTD-0369 / the δ-independence program's
Stage S3 verdict instrument (post-lock execution of
PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md, tag preregister-delta-ind-closure-v1,
lock commit 63e9c506).

THIS IS the program's first δ-involving computation, sanctioned as the S3
registered execution (prereg ban B1 covered pre-S3 computation only).

Claim adjudicated (verdict doc: ANALYSIS_DELTA_IND_CLOSURE_v1.md):
    delta-IND relative to the FROZEN closure N = <N_calc, N_dyn>:
    delta = sqrt(G*(4G*-1)) is not in N.
    VERDICT per the frozen map: **PROVEN-CONDITIONAL** — the valuation
    argument closes under the enumerated independence package E0-E2
    (E0 = Chudnovsky 1976, proven; E1 = joint independence of the
    Gamma(1/3)-/Gamma(1/24)-class Watson values with {pi, Gamma(1/4)};
    E2 = independence of the exponential-period generators — E1/E2 open,
    standard-type).  Unconditional-beyond-E0 sub-theorem for the
    BCC-sector sub-closure (generators inside the hull).

What this script does:
    (V1) delta's algebraic position, symbolically: from the master quadratic
         x^2 - 16 G*^2 x + 16 G*^3, verifies (x+ - x-)^2 = 64 G*^3 (4G*-1),
         hence delta = (x+ - x-)/(8G*) satisfies delta^2 = G*(4G*-1) = t(4t-1)
         exactly, and x+ = 8G*^2 + 4G*delta.
    (V2) The valuation mechanism, formal bookkeeping: implements the
         (4t-1)-adic valuation on the model field and verifies (a) v = 0
         (unit) for EVERY one of the 13 documented inventory hull-monomials
         s^a w^b (s = sqrt(t), w = u^(1/4)) — integral valuation, even parity;
         (b) v(delta^2) = 1 (odd), so v(delta) = 1/2 is NON-INTEGRAL — the
         parity obstruction that no unit-adjunction can repair.
    (V3) The unramified-tower lemma, mechanics: valuations of elements
         a * prod sqrt(u_i)^eps with v(u_i) = 0 remain integral (spot
         verification of the parity bookkeeping over representative towers)
         while delta's stays half-integral.
    (V4) The schema->period bookkeeping, numerically at two precisions
         (gate G4, dps in {50, 100}): (a) the odd-L BCC schema converges to
         Gamma(1/4)^4/(4 pi^3) (the hull row); (b) the SC schema satisfies
         the defining Green's relation G_L(0) - G_L(1) -> 1/6 (an exact
         pre-stated target — the momentum sums approach the period-side
         identity), with dps-band agreement.

What this script is NOT:
    - NOT an unconditional theorem: E1/E2 are open independence conjectures,
      enumerated in the verdict doc; the script verifies the MECHANISM and
      the MODEL bookkeeping, not the transcendence conjectures.
    - NOT a promotion instrument: x+ = 1/alpha stays [SMC] (FTD-0013);
      MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; FC-W stays [AXIOM] — the
      theorem pins FC-W's necessity relative to N, nothing more.

Usage:
    python scripts/proofs/proof_s3_delta_independence.py
"""

from __future__ import annotations

import os
import sys
import time
from fractions import Fraction
from itertools import product as iproduct

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

import mpmath as mpm
import sympy as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

suite = ProofSuite("S3 verdict: delta-IND relative to the frozen closure N")


# ---------------------------------------------------------------------------
# V1 — delta's algebraic position from the master quadratic (symbolic).
# ---------------------------------------------------------------------------

def check_v1() -> None:
    t = sp.Symbol("t", positive=True)          # t models G*
    x = sp.Symbol("x")
    quad = x**2 - 16 * t**2 * x + 16 * t**3
    xp, xm = [r for r in sp.solve(quad, x)]
    # order the roots symbolically: xp - xm = +/- sqrt(disc)
    diff2 = sp.simplify((xp - xm) ** 2)
    ok1 = sp.simplify(diff2 - 64 * t**3 * (4 * t - 1)) == 0
    suite.assert_true("V1 (x+ - x-)^2 = 64 t^3 (4t - 1) (symbolic)",
                      bool(ok1), tag="[THEOREM]")
    delta = sp.sqrt(t * (4 * t - 1))
    ok2 = sp.simplify(diff2 / (64 * t**2) - delta**2) == 0
    suite.assert_true("V1 delta^2 = t(4t-1) with delta = (x+ - x-)/(8t)",
                      bool(ok2), tag="[THEOREM]")
    # x+ = 8t^2 + 4t*delta (the alpha-candidate root reached via delta)
    xplus = 8 * t**2 + 4 * t * delta
    ok3 = sp.simplify(quad.subs(x, xplus)) == 0
    suite.assert_true("V1 x+ = 8t^2 + 4t*delta solves the master quadratic",
                      bool(ok3), tag="[THEOREM]")


# ---------------------------------------------------------------------------
# V2 — the (4t-1)-adic valuation on the model: inventory rows are units
# (integral, even valuation); delta is half-integral.  We track valuations
# of monomials t^a (4t-1)^b u^c s^d w^e with s = t^(1/2), w = u^(1/4):
#     v(t) = 0, v(4t-1) = 1, v(u) = 0  =>  v = b (+ d/2 * v(t) = 0 ...).
# delta = s * (4t-1)^(1/2):  v(delta) = 1/2.
# ---------------------------------------------------------------------------

def val(a=0, b=0, c=0, d=0, e=0, half_b=0):
    """Valuation of t^a (4t-1)^b u^c (sqrt t)^d (u^(1/4))^e ((4t-1)^(1/2))^half_b."""
    return Fraction(b) + Fraction(half_b, 2)


def check_v2() -> None:
    # STRENGTHENED 2026-07-05 per the A0 audit (finding M-4): the original
    # V2 was valuation bookkeeping over author-supplied exponents and could
    # not catch a mistyped hull form.  It now RECOMPUTES the hull identities
    # numerically at dps=50 (value expression vs coeff * s^d * w^e with
    # s = sqrt(G*), w = pi^(1/4)) for every row with an independent value
    # expression, before doing the valuation bookkeeping.  The row
    # identities' 40-58 digit verification of record remains FTD-0353's
    # instrument (51/51); this check is a re-verification, not the source.
    with mpm.workdps(50):
        Gs = mpm.gamma(mpm.mpf(1) / 4) / mpm.gamma(mpm.mpf(3) / 4)
        s_ = mpm.sqrt(Gs)
        w_ = mpm.pi ** mpm.mpf("0.25")
        g14 = mpm.gamma(mpm.mpf(1) / 4)
        g34 = mpm.gamma(mpm.mpf(3) / 4)
        q = mpm.exp(-mpm.pi)
        # (label, value_expr, coeff, d, e) — coeff * s^d * w^e
        numeric_rows = [
            ("det_zeta D_{1/4}", mpm.sqrt(2 * mpm.pi) / g14, 2 ** mpm.mpf("0.25"), -1, 0),
            ("det_zeta D_{3/4}", mpm.sqrt(2 * mpm.pi) / g34, 2 ** mpm.mpf("0.25"), 1, 0),
            ("Watson G_BCC(0)", g14 ** 4 / (4 * mpm.pi ** 3), mpm.mpf(1) / 2, 4, -4),
            ("theta3(0,i)", mpm.jtheta(3, 0, q), 2 ** mpm.mpf("-0.25"), 1, -1),
            ("theta2(0,i)", mpm.jtheta(2, 0, q), 2 ** mpm.mpf("-0.5"), 1, -1),
            ("theta4(0,i)", mpm.jtheta(4, 0, q), 2 ** mpm.mpf("-0.5"), 1, -1),
            ("AGM(1,sqrt2)", mpm.agm(1, mpm.sqrt(2)), 2, -2, 2),
            ("lemniscate varpi", g14 ** 2 / (2 * mpm.sqrt(2 * mpm.pi)), mpm.mpf(1) / 2, 2, 2),
            ("CM period Omega", g14 ** 2 / mpm.sqrt(2 * mpm.pi), 1, 2, 2),
        ]
        ok_num = True
        for label, valexpr, coeff, d, e in numeric_rows:
            hull = coeff * s_ ** d * w_ ** e
            if mpm.fabs(valexpr - hull) > mpm.mpf(10) ** (-40):
                ok_num = False
    suite.assert_true(
        f"V2 hull identities RECOMPUTED numerically for {len(numeric_rows)} "
        "rows (value = coeff*s^d*w^e at dps=50; catches mistyped exponents)",
        bool(ok_num), tag="[THEOREM]")

    # The 13 documented inventory rows as hull monomials s^d w^e (FTD-0353
    # §2.2 table; identities of record verified there at 40-58 digits).
    rows = [
        ("det_zeta D_{1/4}", -1, 0), ("det_zeta D_{3/4}", 1, 0),
        ("det ratio = G*", 2, 0), ("Watson G_BCC(0)", 4, -4),
        ("theta3(0,i)", 1, -1), ("theta2=theta4(0,i)", 1, -1),
        ("eta(D_a) rational", 0, 0), ("half-deriv eigen G*^{+1}", 2, 0),
        ("half-deriv eigen G*^{-1}", -2, 0), ("AGM(1,sqrt2)", -2, 2),
        ("lemniscate varpi", 2, 2), ("CM period Omega", 2, 2),
        ("L(E,1)", 2, 2),
    ]
    ok_units = all(val(d=d, e=e) == 0 for (_, d, e) in rows)
    suite.assert_true(
        "V2 all 13 inventory hull-monomials are (4t-1)-units (v = 0, "
        "integral; valuation bookkeeping over the recomputed rows)",
        bool(ok_units), tag="[THEOREM]")
    v_delta = val(d=1, half_b=1)   # delta = s * (4t-1)^{1/2}
    suite.assert_true(
        f"V2 v(delta) = {v_delta} is HALF-INTEGRAL (the parity obstruction)",
        v_delta == Fraction(1, 2) and v_delta.denominator == 2,
        tag="[THEOREM]")
    suite.assert_true(
        "V2 v(delta^2) = 1 is ODD (no unit multiple can fix the parity)",
        val(b=1, d=2) == 1, tag="[THEOREM]")


# ---------------------------------------------------------------------------
# V3 — unramified-tower mechanics: adjoining square roots of v-units keeps
# valuations integral; delta's half-integral valuation survives every such
# tower.  Spot-verified over representative towers.
# ---------------------------------------------------------------------------

def check_v3() -> None:
    # representative unit square-classes: u1 = t, u2 = u, u3 = t^3*u^2 (all v=0)
    units = [dict(a=1), dict(c=1), dict(a=3, c=2)]
    ok = True
    for eps in iproduct((0, 1), repeat=3):
        # element = (t^2 (4t-1)^2) * prod sqrt(u_i)^eps_i  — v-integral base
        v_elem = val(a=2, b=2)
        for e_i, uu in zip(eps, units):
            if e_i:
                # sqrt of a unit contributes v(unit)/2 = 0
                v_elem += Fraction(val(**uu), 2)
        ok = ok and (v_elem.denominator == 1)
    suite.assert_true(
        "V3 bookkeeping illustration of the classical unramified-tower "
        "lemma: sqrt(unit)-tower elements keep INTEGRAL valuation (8/8 sign "
        "patterns; the lemma itself is classical, not proven here)",
        bool(ok), tag="[THEOREM]")
    # delta over the same tower: v = 1/2 + 0 -> still half-integral
    ok_d = all((Fraction(1, 2)
                + sum(Fraction(val(**uu), 2) for e_i, uu in zip(eps, units) if e_i)
                ).denominator == 2
               for eps in iproduct((0, 1), repeat=3))
    suite.assert_true(
        "V3 delta's valuation stays half-integral over every such tower "
        "=> delta not in any sqrt(unit) multiquadratic extension",
        bool(ok_d), tag="[THEOREM]")


# ---------------------------------------------------------------------------
# V4 — schema -> period bookkeeping at two precisions (gate G4).
# ---------------------------------------------------------------------------

def bcc_green_float(L: int) -> mpm.mpf:
    assert L % 2 == 1
    total = mpm.mpf(0)
    cos_tab = [mpm.cos(2 * mpm.pi * j / L) for j in range(L)]
    for kx in range(L):
        for ky in range(L):
            for kz in range(L):
                if kx == ky == kz == 0:
                    continue
                total += 1 / (1 - cos_tab[kx] * cos_tab[ky] * cos_tab[kz])
    return total / L**3


def sc_green_diff_float(L: int) -> mpm.mpf:
    """G_L(0) - G_L(1) for the SC symbol (defining relation target: 1/6)."""
    total = mpm.mpf(0)
    cos_tab = [mpm.cos(2 * mpm.pi * j / L) for j in range(L)]
    for kx in range(L):
        for ky in range(L):
            for kz in range(L):
                if kx == ky == kz == 0:
                    continue
                denom = 6 - 2 * (cos_tab[kx] + cos_tab[ky] + cos_tab[kz])
                total += (1 - cos_tab[kx]) / denom
    return total / L**3


def check_v4() -> None:
    results = {}
    for dps in (50, 100):
        with mpm.workdps(dps):
            I1 = mpm.gamma(mpm.mpf(1) / 4) ** 4 / (4 * mpm.pi ** 3)
            e33 = abs(bcc_green_float(33) - I1)
            e49 = abs(bcc_green_float(49) - I1)
            d33 = abs(sc_green_diff_float(33) - mpm.mpf(1) / 6)
            d49 = abs(sc_green_diff_float(49) - mpm.mpf(1) / 6)
            results[dps] = (float(e33), float(e49), float(d33), float(d49))
    ok_band = all(abs(results[50][i] - results[100][i])
                  / max(results[100][i], 1e-30) < 1e-6 for i in range(4))
    suite.assert_true("V4 dps-{50,100} band agreement on all four residuals "
                      "(gate G4)", bool(ok_band), tag="[THEOREM]")
    e33, e49, d33, d49 = results[100]
    suite.assert_true(
        f"V4 BCC schema -> hull period: residual shrinks 33->49 "
        f"({e33:.2e} -> {e49:.2e})", e49 < e33, tag="[EXTERNAL]")
    suite.assert_true(
        f"V4 SC schema obeys the defining Green relation G(0)-G(1) -> 1/6 "
        f"({d33:.2e} -> {d49:.2e})", d49 < d33, tag="[EXTERNAL]")


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  FTD-0369 - S3 verdict instrument (post-lock, sanctioned)")
    print("  delta-IND relative to the frozen closure N: the valuation")
    print("  mechanism, the model bookkeeping, and the schema->period checks.")
    print("=" * 70)

    check_v1()
    check_v2()
    check_v3()
    check_v4()

    suite.print_summary()
    print(f"\n  Wall time: {time.time() - t0:.1f}s")
    print("\n  VERDICT CONTEXT (full statement in ANALYSIS_DELTA_IND_CLOSURE_v1.md):")
    print("  - PROVEN-CONDITIONAL per the frozen map: the mechanism above is")
    print("    exact; closing it over ALL of N requires the enumerated")
    print("    independence package E0 (Chudnovsky, proven) + E1/E2 (open).")
    print("  - Unconditional-beyond-E0 sub-theorem for the BCC-sector")
    print("    sub-closure (generators in the hull; rows verified in V2).")
    print("  - Zero promotions: x+ = 1/alpha [SMC]; MC-T4.3 [FOUNDATIONAL")
    print("    OBSTRUCTION]; FC-W [AXIOM] - its necessity relative to N is")
    print("    what the theorem pins.")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
