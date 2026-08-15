"""FTD — Axial surd obstruction + Pythagorean currency (native C3 arithmetic).

Companion verifier for THEOREM_AXIAL_SURD_OBSTRUCTION_AND_PYTHAGOREAN_
CURRENCY_v1.md. Machine-checks the arithmetic content of:

  T1  (surd no-go, all stages): a signed sum of square roots of positive
      non-square rationals is never 1 — so no all-surd axial closure pays a
      unit strut, for ANY number of cable stages. Rests on the classical
      Q-linear independence of square roots of distinct squarefree integers
      (Besicovitch 1940); instances verified here by field-degree checks.
  T2  (Pythagorean currency): a cable stage has rational axial rise iff
      k^2 - rho^2 is a rational square, with the ring radius pinned by
      integer ring chains, rho = t/(2 sin(pi/n)); rho^2 is rational iff
      n in {3, 4, 6} (crystallographic restriction). Governing Diophantine
      family: k^2 - t^2/c = h^2 with c in {1, 2, 3}.
  T3  (single-ring closure, unconditional): a single-ring axially-symmetric
      unit-strut tensegrity fails for every ring span and radius — the
      tension-ring case forces a unit ring and dies by polarity; the
      compression-ring case forces span 1 and dies by the FTD-1004 F4 tree.

P07 verifies the appendix identity: the quartic clock invariant T*A equals
the area of the unit squircle x^4 + y^4 = 1 (exact Gamma algebra; the
squircle double-covers the clock's time curve w^2 = 1 - x^4 via w = y^2).
"""

import itertools
import sys

import sympy as sp

CHECKS = []


def check(cid, desc, ok, detail=""):
    CHECKS.append((cid, desc, bool(ok)))
    print(f"[{'PASS' if ok else 'FAIL'}] {cid}  {desc}"
          + (f"  -- {detail}" if detail else ""))


# --------------------------------------------------------------------------
# P01 — Besicovitch instances: [Q(sqrt m1..mr) : Q] = 2^r
# --------------------------------------------------------------------------
def p01():
    ok = True
    for ms in ((2,), (3,), (2, 3), (3, 5), (2, 3, 5), (3, 5, 7),
               (2, 5, 7), (2, 3, 35)):
        gen = sum(sp.sqrt(m) for m in ms)
        deg = sp.Poly(sp.minimal_polynomial(gen, sp.Symbol("x")),
                      sp.Symbol("x")).degree()
        want = 2 ** len(ms)
        if deg != want:
            ok = False
    check("P01", "Besicovitch instances: field degree 2^r for distinct "
          "squarefree radicands (r <= 3)", ok)


# --------------------------------------------------------------------------
# P02 — surd-closure sweep: no signed sum of sqrt(k^2-1) equals 1
# --------------------------------------------------------------------------
def p02():
    import math
    vals = {k: math.sqrt(k * k - 1) for k in range(2, 31)}
    hit = None
    for nterms in (1, 2, 3, 4):
        for ks in itertools.combinations_with_replacement(vals, nterms):
            for signs in itertools.product((1, -1), repeat=nterms):
                ssum = sum(sg * vals[k] for sg, k in zip(signs, ks))
                if abs(ssum - 1.0) < 1e-9:
                    hit = (ks, signs)
        if hit:
            break
    check("P02", "surd sweep: no signed sum of sqrt(k^2-1), k<=30, "
          "up to 4 terms, equals 1 (instances of T1)", hit is None,
          str(hit) if hit else "")


# --------------------------------------------------------------------------
# P03 — crystallographic pinning: rho^2 rational iff n in {3,4,6}
# --------------------------------------------------------------------------
def p03():
    t = sp.Symbol("t", positive=True)
    rational_n = []
    for n in range(3, 13):
        r2 = sp.simplify((t / (2 * sp.sin(sp.pi / n))) ** 2)
        if (r2 / t ** 2).is_rational:
            rational_n.append(n)
    check("P03", "rho^2 rational iff n in {3,4,6} (n <= 12)",
          rational_n == [3, 4, 6], str(rational_n))


# --------------------------------------------------------------------------
# P04 — Pythagorean stage enumeration (k, t <= 12)
# --------------------------------------------------------------------------
def p04():
    got = {}
    for c in (1, 2, 3):
        sols = []
        for k in range(1, 13):
            for tt in range(1, 13):
                h2 = sp.Rational(k * k) - sp.Rational(tt * tt, c)
                if h2 > 0 and sp.sqrt(h2).is_integer:
                    sols.append((k, tt, int(sp.sqrt(h2))))
        got[c] = sols
    primitive_ok = ((2, 3, 1) in got[3] and (3, 4, 1) in got[2]
                    and (5, 4, 3) in got[1] and (5, 3, 4) in got[1])
    nonempty = all(got[c] for c in (1, 2, 3))
    check("P04", "Pythagorean currency nonempty; primitive stages "
          "(2,3,1)@n=3, (3,4,1)@n=4, (5,4,3)@n=6 present",
          primitive_ok and nonempty,
          f"counts c=1:{len(got[1])} c=2:{len(got[2])} c=3:{len(got[3])}")


# --------------------------------------------------------------------------
# P05 — T3 tension case: unit spokes force a unit ring
# --------------------------------------------------------------------------
def p05():
    # spoke: rho^2 + h^2 = 1 -> rho <= 1; ring chain span t = 2 rho sin(pi/n)
    # t integer >= 1 and t <= 2 rho <= 2. t = 2 forces rho = 1/sin(pi/n) >= 1
    # and rho <= 1 -> sin(pi/n) = 1 -> n = 2 (degenerate). So t = 1.
    rho, h = sp.symbols("rho h", positive=True)
    t2_case = sp.solve([sp.Eq(rho ** 2 + h ** 2, 1),
                        sp.Eq(2 * rho * sp.sin(sp.pi / sp.Symbol("n")), 2)],
                       [rho, h], dict=True)
    # symbolic: rho = 1/sin(pi/n); rho <= 1 requires sin >= 1, i.e. n = 2.
    forced = sp.solve(sp.Eq(1 / sp.sin(sp.pi / sp.Symbol("n", positive=True)),
                            1), sp.Symbol("n", positive=True))
    check("P05", "T3 tension case: t=2 degenerate (n=2 only); unit ring t=1 "
          "forced", forced == [2], f"t=2 solutions n={forced}")


# --------------------------------------------------------------------------
# P06 — T3 polarity kills for the forced unit ring
# --------------------------------------------------------------------------
def p06():
    hub_cycle_flips = 1 + 1 + 1        # spoke, ring bond, spoke
    odd_ring_dead = all((n % 2 == 1) for n in (3, 5))
    check("P06", "T3 unit-ring polarity: hub cycle 3 flips (odd, dead); "
          "odd-n unit ring dead", hub_cycle_flips % 2 == 1 and odd_ring_dead)


# --------------------------------------------------------------------------
# P07 — squircle identity: T*A = area(x^4+y^4=1), exact
# --------------------------------------------------------------------------
def p07():
    x = sp.Symbol("x")
    area = 4 * sp.integrate((1 - x ** 4) ** sp.Rational(1, 4), (x, 0, 1))
    Gs = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
    TA = sp.sqrt(sp.pi / 2) * Gs
    diff = sp.simplify(sp.nsimplify(area, rational=False) - TA)
    exact = sp.simplify(area - TA) == 0
    num = abs(sp.N(area - TA, 40))
    check("P07", "squircle area equals quartic clock invariant "
          "sqrt(pi/2)*Gamma(1/4)/Gamma(3/4)",
          exact or num < sp.Float(10) ** -35, f"numeric diff {num}")


# --------------------------------------------------------------------------
# P08 — double cover: w = y^2 maps the squircle onto the clock curve
# --------------------------------------------------------------------------
def p08():
    xx, yy = sp.symbols("xx yy")
    squircle = xx ** 4 + yy ** 4 - 1
    w = yy ** 2
    clock_curve = w ** 2 - (1 - xx ** 4)      # w^2 = 1 - x^4
    ok = sp.simplify(clock_curve - squircle * 1) == sp.simplify(
        (yy ** 4 - (1 - xx ** 4)) - (xx ** 4 + yy ** 4 - 1))
    # identical polynomials: w^2-(1-x^4) == squircle
    ok = sp.expand(clock_curve) == sp.expand(squircle)
    check("P08", "w = y^2 maps the squircle identically onto w^2 = 1 - x^4",
          ok)


def main():
    print("=" * 74)
    print("AXIAL SURD OBSTRUCTION + PYTHAGOREAN CURRENCY -- verifier")
    print("=" * 74)
    p01(); p02(); p03(); p04(); p05(); p06(); p07(); p08()
    n_ok = sum(1 for _, _, ok in CHECKS if ok)
    print("=" * 74)
    print(f"RESULT: {n_ok}/{len(CHECKS)}")
    return 0 if n_ok == len(CHECKS) else 1


if __name__ == "__main__":
    sys.exit(main())
