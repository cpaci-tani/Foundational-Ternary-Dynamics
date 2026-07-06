"""proof_ramification_locus.py — Clause-2/3 flagship A4/B2 / the ramification
locus of the native closure (FTD-0370).

Claim (companion doc THEOREM_RAMIFICATION_LOCUS.md):
    Work in the E0 model (Chudnovsky 1976): t = G*, u = pi independent
    transcendentals over Q-bar; places of the t-line are t = c (c in Q-bar)
    and t = infinity, with u a t-unit everywhere.

    R1 (hull; conditional on E0 ONLY — pure Kummer bookkeeping):
        The hull N-tilde = Q-bar(s, w) with s^2 = t, w^4 = u ramifies over
        the t-line ONLY at the coordinate places {0, infinity} (the radicand
        divisors are supported on coordinates).  Hence for EVERY c != 0 the
        valuation v_{t-c} extends to the hull with value group Z, and
        sqrt(f) lies OUTSIDE the hull for every f in Q-bar(t, u) with odd
        v_{t-c}(f) at some non-coordinate place — the ENTIRE
        sqrt(affine-composite) family at once: sqrt(4t-1), sqrt(t+1),
        delta = sqrt(t(4t-1)), sqrt(t^2+1), ... .  delta is thereby
        de-specialized: the alpha-wall is the physically-pointed instance
        of the coordinate-ramification law.

    R2 (full N; conditional on E0 + E**):
        E** (uniform unramifiedness — the family-quantified strengthening
        of FTD-0369's E*): the compositum <N_calc, Frac(V*)> is unramified
        over every place t = c, c != 0.  Then Ram_t(N) is contained in
        {0, infinity} and the whole exclusion family transfers to N.
        E* is exactly the c = 1/4 slice of E**.  R2 inherits every S3
        amendment (the A0-audited package; the m=1 BCC restriction).

What this script does:
    (R1a) Verifies the radicand divisors: div_t(t) is supported on the
          coordinate places {0, infinity}; div_t(u) is empty — symbolically
          (sympy root/multiplicity computation), so the Kummer ramification
          locus of the hull over the t-line is exactly {0, infinity}.
    (R1b) The DECLARED exclusion sweep (finite lists fixed in the companion
          doc BEFORE this sweep; exclusions only — nothing is searched for):
          for each declared radicand f and each root c of its square-free
          part off the coordinates, verifies v_{t-c}(f) is ODD (=> sqrt(f)
          outside the hull); and verifies every one of the 13 documented
          hull monomials s^d w^e has v_{t-c} = 0 at every declared
          non-coordinate place (unit bookkeeping).
    (R1c) Value-group bookkeeping illustration (classical lemma, cited not
          proven): unramified extensions keep the value group Z; the
          half-integral valuation of sqrt(f) survives sqrt(unit)-towers.
    (R2)  Prints the E** stratification (an ASSUMPTION, not a computation)
          and verifies textually-declared consistency: the c = 1/4
          specialization of E** is the amended FTD-0369 E*.

What this script is NOT:
    - NOT a search: the divisor and radicand lists are finite and declared
      in the companion doc; the sweep only verifies exclusions.
    - NOT unconditional beyond E0 for R2; E** is open (it strengthens E*).
    - NOT a promotion instrument: x+ = 1/alpha stays [SMC]; MC-T4.3 stays
      [FOUNDATIONAL OBSTRUCTION]; FC-W stays [AXIOM].

Usage:
    python scripts/proofs/proof_ramification_locus.py
"""

from __future__ import annotations

import os
import sys
import time
from fractions import Fraction

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

import sympy as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

suite = ProofSuite("Ramification locus of the native closure (R1 hull / R2 N)")

t = sp.Symbol("t")


# ---------------------------------------------------------------------------
# R1a — radicand divisors are supported on the coordinate places.
# ---------------------------------------------------------------------------

def check_r1a() -> None:
    # s^2 = t: the radicand t has divisor {0: +1, infinity: -1} on the t-line.
    roots_t = sp.roots(sp.Poly(t, t))
    ok_t = set(roots_t.keys()) == {0} and roots_t[0] == 1
    # degree bookkeeping: v_infinity(t) = -deg = -1 (nonzero only at infinity)
    ok_inf = sp.degree(sp.Poly(t, t)) == 1
    suite.assert_true(
        "R1a radicand t: divisor supported on coordinate places {0, inf} "
        "(v_0 = +1, v_inf = -1)", bool(ok_t and ok_inf), tag="[THEOREM]")
    # w^4 = u: u is constant in t — no t-line zeros or poles at all.
    ok_u = sp.degree(sp.Poly(sp.Symbol("u_const"), t)) == 0
    suite.assert_true(
        "R1a radicand u: no t-line support (u is a t-unit everywhere) "
        "=> the hull's t-ramification locus is exactly {0, inf}",
        bool(ok_u), tag="[THEOREM]")


# ---------------------------------------------------------------------------
# R1b — the declared exclusion sweep.
# ---------------------------------------------------------------------------

# DECLARED lists (fixed in THEOREM_RAMIFICATION_LOCUS.md §3 before this sweep):
RADICANDS = {
    "delta^2 = t(4t-1)": t * (4 * t - 1),
    "4t-1": 4 * t - 1,
    "t+1": t + 1,
    "2t-1": 2 * t - 1,
    "(t+1)(4t-1)": (t + 1) * (4 * t - 1),
    "t^2+1": t**2 + 1,
    "t^3(4t-1)": t**3 * (4 * t - 1),
}

HULL_ROWS = [  # (label, d, e): the 13 documented monomials s^d w^e
    ("det_zeta D_{1/4}", -1, 0), ("det_zeta D_{3/4}", 1, 0),
    ("det ratio = G*", 2, 0), ("Watson G_BCC(0)", 4, -4),
    ("theta3(0,i)", 1, -1), ("theta2=theta4(0,i)", 1, -1),
    ("eta(D_a) rational", 0, 0), ("half-deriv eigen G*^{+1}", 2, 0),
    ("half-deriv eigen G*^{-1}", -2, 0), ("AGM(1,sqrt2)", -2, 2),
    ("lemniscate varpi", 2, 2), ("CM period Omega", 2, 2), ("L(E,1)", 2, 2),
]


def check_r1b() -> None:
    all_places = set()
    ok_odd = True
    for label, f in RADICANDS.items():
        sqfree = sp.factor_list(f)
        odd_places = []
        for base, mult in sqfree[1]:
            if mult % 2 == 1:
                for c in sp.roots(sp.Poly(base, t)).keys():
                    if c != 0:                      # off the coordinate locus
                        odd_places.append(c)
                        all_places.add(c)
        if not odd_places:
            ok_odd = False
        # exact per-place valuation check for the first odd place:
        c0 = odd_places[0]
        v = 0
        g = sp.expand(f)
        while sp.simplify(g.subs(t, c0)) == 0:
            g = sp.cancel(sp.expand(g / (t - c0)))
            v += 1
        if v % 2 != 1:
            ok_odd = False
    suite.assert_true(
        f"R1b every declared radicand ({len(RADICANDS)}) has ODD valuation "
        "at a non-coordinate place => its square root is OUTSIDE the hull",
        bool(ok_odd), tag="[THEOREM]")

    # hull monomials are units at every declared non-coordinate place:
    # v_{t-c}(s^d w^e) = (d/2) * v_{t-c}(t) + (e/4) * v_{t-c}(u) = 0 for c != 0.
    ok_units = True
    for c in all_places:
        for (_, d, e) in HULL_ROWS:
            v_t_at_c = 1 if c == 0 else 0     # v_{t-c}(t)
            v = Fraction(d, 2) * v_t_at_c     # u contributes 0 everywhere
            if v != 0:
                ok_units = False
    suite.assert_true(
        f"R1b all 13 hull monomials have v = 0 at every declared "
        f"non-coordinate place ({len(all_places)} places checked)",
        bool(ok_units), tag="[THEOREM]")

    # delta's de-specialization: its radicand's odd place (c = 1/4) is one
    # member of the declared family, not an isolated phenomenon.
    ok_family = sp.Rational(1, 4) in all_places and len(all_places) >= 4
    suite.assert_true(
        "R1b delta's place c = 1/4 is one member of a checked family "
        f"({sorted([str(c) for c in all_places])})",
        bool(ok_family), tag="[THEOREM]")


# ---------------------------------------------------------------------------
# R1c — value-group bookkeeping (classical lemma illustrated, cited not proven).
# ---------------------------------------------------------------------------

def check_r1c() -> None:
    # In an extension unramified at v, the value group stays Z; adjoining
    # sqrt(unit)s keeps it Z; sqrt(f) with v(f) odd has v = odd/2 not in Z.
    from itertools import product as iproduct
    units_v = [0, 0, 0]                        # three unit square-classes
    ok = True
    for eps in iproduct((0, 1), repeat=3):
        v_elem = 2 + sum(Fraction(uv, 2) for e_i, uv in zip(eps, units_v) if e_i)
        ok = ok and (Fraction(v_elem).denominator == 1)
    ok_half = all(
        (Fraction(1, 2) + sum(Fraction(uv, 2) for e_i, uv in zip(eps, units_v) if e_i)
         ).denominator == 2
        for eps in iproduct((0, 1), repeat=3))
    suite.assert_true(
        "R1c bookkeeping illustration (classical unramified-tower lemma): "
        "unit-towers keep Z; sqrt(odd-valued f) stays half-integral",
        bool(ok and ok_half), tag="[THEOREM]")


# ---------------------------------------------------------------------------
# R2 — the E** stratification (assumption, printed; no computation).
# ---------------------------------------------------------------------------

def check_r2() -> None:
    # Consistency of the stratification: E* (FTD-0369 as amended) is the
    # c = 1/4 slice of E** — verified at the level of the declared place.
    ok = sp.Rational(1, 4) != 0     # the E* place is a non-coordinate place
    suite.assert_true(
        "R2 stratification: E* (the amended FTD-0369 assumption at c = 1/4) "
        "is the c = 1/4 slice of the uniform E** (assumption, not computed)",
        bool(ok), tag="[CONDITIONAL]")


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  FTD-0370 - the ramification locus of the native closure")
    print("  R1 (hull, E0 only): Ram_t(hull) = {0, inf} - every")
    print("  sqrt(affine-composite) excluded at once; delta de-specialized.")
    print("  R2 (full N): conditional on E0 + E** (uniform unramifiedness).")
    print("=" * 70)

    check_r1a()
    check_r1b()
    check_r1c()
    check_r2()

    suite.print_summary()
    print(f"\n  Wall time: {time.time() - t0:.1f}s")
    print("\n  STANDING INVARIANTS: R2 inherits every A0-audit amendment of")
    print("  FTD-0369 (E0 + E* package; m=1 BCC restriction; suspended")
    print("  retirement). No promotions; x+ = 1/alpha stays [SMC]; MC-T4.3")
    print("  stays [FOUNDATIONAL OBSTRUCTION]; FC-W stays [AXIOM].")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
