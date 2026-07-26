"""proof_coat_external_anchor.py — FTD-0568: quadratic-coat external anchor
(companion AUDIT_QUADRATIC_COAT_EXTERNAL_ANCHOR.md; import ledger IMP-C5).

Verifies, convention-for-convention, that the FTD-0541-0551 quadratic-coat
identities are instances of the standard tensor-product B-spline de Rham
complex (Buffa-Sangalli-Vazquez, CMAME 199:1143-1152, 2010;
Buffa-Rivas-Sangalli-Vazquez, SIAM J. Numer. Anal. 49(2):818-844, 2011;
framework: Arnold-Falk-Winther FEEC, Acta Numerica 15:1-155, 2006; explicit
coefficient-level form: GEMPIC, Kraus-Kormann-Morrison-Sonnendrucker,
J. Plasma Phys. 83, 2017, arXiv:1609.03053, eqs. 3.29-3.47, 4.51).

Checks:
  A1  the univariate derivative identity B2'(u) = B1(u+1/2) - B1(u-1/2)
      (FTD-0550 eq. 6 == the B-spline derivative recurrence, GEMPIC 3.37,
      recentred to cardinal splines). Both sides are piecewise linear with
      breakpoints in {-3/2,-1/2,1/2,3/2}; agreement at >= 2 interior rational
      points per piece proves equality piece-by-piece.
  A2  the centered quadratic B-spline integer-offset weights are
      (1/8, 3/4, 1/8) and sum with partition of unity (the FTD-0540
      cardinality-loss values == the standard quadratic-spline values).
  A3  the univariate commutation d/dx sum_i a_i B2(x-i)
      = sum_i (a_{i+1}-a_i) B1(x-i-1/2) with FORWARD-difference
      coefficients — the 1D core of FTD-0550 eq. (7)-(8), symbolic in the
      coefficients (linearity => coefficient-wise proof), sampled per piece.
  A4  the assembled 3D wiring: (curl A)_x of the face/1-form interpolant
      (components: B1 own-direction at half-integer stagger, B2 transverse)
      equals the edge/2-form interpolant of the incidence-matrix coefficients
      A_z[i,j+1,k]-A_z[i,j,k] - A_y[i,j,k+1]+A_y[i,j,k] (FTD-0550 eq. 7),
      on a periodic N=4 lattice in EXACT rational arithmetic, with the
      analytic per-piece derivative of B2 (independent of A1, so the check
      is not circular).

This instrument reconciles conventions; it introduces no theorem beyond the
external literature's and moves no tag. FTD-0541/0550 [SELECTION] rows stand;
the arc's in-house [THEOREM]s keep their own proofs.

Usage:
    python scripts/proofs/proof_coat_external_anchor.py
"""

from __future__ import annotations

import os
import random
import sys
import time
from fractions import Fraction

import sympy as sp

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

suite = ProofSuite("FTD-0568 — quadratic-coat external anchor "
                   "(spline de Rham complex conventions)")

X = sp.symbols('x')


def bspline(p, u):
    """Centered cardinal B-spline of degree p (sympy Piecewise), via the
    convolution recurrence B_p(x) = int_{x-1/2}^{x+1/2} B_{p-1}(t) dt."""
    if p == 0:
        return sp.Piecewise(
            (1, sp.And(u >= sp.Rational(-1, 2), u < sp.Rational(1, 2))),
            (0, True))
    t = sp.symbols('t', real=True)
    return sp.integrate(bspline(p - 1, t),
                        (t, u - sp.Rational(1, 2), u + sp.Rational(1, 2)))


def check_a1() -> None:
    b2 = bspline(2, X)
    lhs = sp.diff(b2, X)
    rhs = (bspline(1, X + sp.Rational(1, 2))
           - bspline(1, X - sp.Rational(1, 2)))
    kinks = {sp.Rational(k, 2) for k in (-3, -1, 1, 3)}
    samples = [sp.Rational(k, 8) for k in range(-16, 17)]
    ok = all(sp.simplify(lhs.subs(X, s) - rhs.subs(X, s)) == 0
             for s in samples if s not in kinks)
    suite.assert_true(
        "A1 derivative identity B2'(u) = B1(u+1/2) - B1(u-1/2) "
        "(FTD-0550 eq. 6 == B-spline derivative recurrence, GEMPIC 3.37): "
        "piecewise-linear both sides, >= 2 interior samples per piece",
        ok, tag="[DERIVED]")


def check_a2() -> None:
    b2 = bspline(2, X)
    w = [b2.subs(X, v) for v in (-1, 0, 1)]
    target = [sp.Rational(1, 8), sp.Rational(3, 4), sp.Rational(1, 8)]
    generic = sp.Rational(3, 10)
    pou = sp.simplify(sum(b2.subs(X, generic - i)
                          for i in range(-2, 3))) == 1
    suite.assert_true(
        f"A2 B2 integer-offset weights = {w} == (1/8, 3/4, 1/8) "
        "(FTD-0540 cardinality-loss values) + partition of unity",
        w == target and pou, tag="[DERIVED]")


def check_a3() -> None:
    n = 7
    a = sp.symbols('a0:7')
    field = sum(a[i] * bspline(2, X - i) for i in range(n))
    dfield = sp.diff(field, X)
    staggered = sum((a[(i + 1) % n] - a[i])
                    * bspline(1, X - i - sp.Rational(1, 2))
                    for i in range(n))
    pts = [sp.Rational(16, 10) + sp.Rational(k, 10) for k in range(0, 29)]
    ok = all(sp.simplify(dfield.subs(X, p_) - staggered.subs(X, p_)) == 0
             for p_ in pts)
    suite.assert_true(
        "A3 1D commutation d/dx interp0(a) = interp1(forward-diff a) "
        "with coefficient a_{i+1} - a_i (the sign convention of "
        "FTD-0550 eq. 7), symbolic in all 7 periodic coefficients",
        ok, tag="[DERIVED]")


# ---- exact-rational (Fraction) B-splines for the 3D wiring check ----------

def b1_frac(u: Fraction) -> Fraction:
    au = abs(u)
    return Fraction(1) - au if au <= 1 else Fraction(0)


def b2_frac(u: Fraction) -> Fraction:
    au = abs(u)
    if au <= Fraction(1, 2):
        return Fraction(3, 4) - u * u
    if au <= Fraction(3, 2):
        d = Fraction(3, 2) - au
        return d * d / 2
    return Fraction(0)


def db2_frac(u: Fraction) -> Fraction:
    """Analytic per-piece derivative of B2 (C1, well-defined everywhere);
    written directly from the quadratic pieces, NOT via the A1 identity."""
    if abs(u) <= Fraction(1, 2):
        return -2 * u
    if Fraction(1, 2) < u <= Fraction(3, 2):
        return -(Fraction(3, 2) - u)
    if Fraction(-3, 2) <= u < Fraction(-1, 2):
        return Fraction(3, 2) + u
    return Fraction(0)


def check_a4() -> None:
    n = 4
    rng = random.Random(56841)  # deterministic

    def rand_coeffs():
        return {(i, j, k): Fraction(rng.randint(-9, 9), rng.randint(1, 7))
                for i in range(n) for j in range(n) for k in range(n)}

    ay, az = rand_coeffs(), rand_coeffs()

    def wrap(i):
        return i % n

    def per(u, n_=n):
        """Reduce a real offset to the periodic fundamental domain around 0
        by shifting with multiples of n (supports are width <= 3 < n)."""
        while u > Fraction(n_, 2):
            u -= n_
        while u < -Fraction(n_, 2):
            u += n_
        return u

    def a_y(x, y, z):
        # face/1-form y-component: B2(x-i) B1(y-j-1/2) B2(z-k)
        s = Fraction(0)
        for (i, j, k), c in ay.items():
            s += c * b2_frac(per(x - i)) * b1_frac(per(y - j - Fraction(1, 2))) \
                * b2_frac(per(z - k))
        return s

    def curl_x_lhs(x, y, z):
        # (curl A)_x = dy A_z - dz A_y with analytic per-piece derivatives
        s = Fraction(0)
        for (i, j, k), c in az.items():
            s += c * b2_frac(per(x - i)) * db2_frac(per(y - j)) \
                * b1_frac(per(z - k - Fraction(1, 2)))
        for (i, j, k), c in ay.items():
            s -= c * b2_frac(per(x - i)) * b1_frac(per(y - j - Fraction(1, 2))) \
                * db2_frac(per(z - k))
        return s

    def curl_x_rhs(x, y, z):
        # edge/2-form x-component interpolant of the incidence coefficients
        # A_z[i,j+1,k] - A_z[i,j,k] - A_y[i,j,k+1] + A_y[i,j,k]  (FTD-0550 eq.7)
        s = Fraction(0)
        for i in range(n):
            for j in range(n):
                for k in range(n):
                    coeff = (az[(i, wrap(j + 1), k)] - az[(i, j, k)]
                             - ay[(i, j, wrap(k + 1))] + ay[(i, j, k)])
                    s += coeff * b2_frac(per(x - i)) \
                        * b1_frac(per(y - j - Fraction(1, 2))) \
                        * b1_frac(per(z - k - Fraction(1, 2)))
        return s

    pts = [(Fraction(rng.randint(0, 8 * n - 1), 8) + Fraction(1, 16),
            Fraction(rng.randint(0, 8 * n - 1), 8) + Fraction(1, 16),
            Fraction(rng.randint(0, 8 * n - 1), 8) + Fraction(1, 16))
           for _ in range(30)]
    worst_ok = all(curl_x_lhs(*p) == curl_x_rhs(*p) for p in pts)
    suite.assert_true(
        "A4 3D wiring, exact rationals, periodic N=4: (curl A)_x of the "
        "face/1-form interpolant == edge/2-form interpolant of the "
        "FTD-0550 eq. (7) incidence coefficients, at 30 sample points "
        "(analytic B2 pieces; independent of A1)",
        worst_ok, tag="[NUMERICAL FACT — exact rational]")


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  FTD-0568 — quadratic-coat external anchor: convention checks")
    print("  Coat identities vs the B-spline de Rham complex of IGA/FEEC.")
    print("=" * 70)
    check_a1()
    check_a2()
    check_a3()
    check_a4()
    suite.print_summary()
    print(f"\n  Wall time: {time.time() - t0:.2f}s")
    print("\n  Reconciliation instrument: no tag moves. FTD-0541/0550")
    print("  [SELECTION] stand; in-house [THEOREM]s keep their own proofs;")
    print("  the external literature supplies prior existence + p-general")
    print("  guarantees (commuting diagram, exactness, adjointness).")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
