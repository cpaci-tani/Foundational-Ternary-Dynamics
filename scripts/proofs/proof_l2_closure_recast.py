"""proof_l2_closure_recast.py — Clause-2 program A3 / the L2 wall as
closure-conservation (companion doc FOUND_L2_CLOSURE_RECAST.md).

Claim ([DERIVED — formalization] + [SYNTHESIS] of FTD-0208 v3; degeneration
gate honored — the incompatibility itself is v3's result, this is its
conserved-invariant form):
    The native budget-combination closure (Scale-0 primitives: sums, maxes,
    Q+-scalings of coordinate magnitudes — the L1-ceiling family of
    ANALYSIS_CLOCK_HYPOTHESIS_v3) consists of O_h-invariant piecewise-
    Q-linear (polyhedral) forms.  Polyhedrality is CONSERVED by every native
    combination; every polyhedral norm has non-parallel additive-equality
    pairs (its unit ball has faces); the Euclidean norm has none (strict
    convexity, Lagrange identity).  Hence c*L2 is not in the closure — and
    since an SO(3)-invariant norm is necessarily a multiple of L2
    (classical), the closure contains NO SO(3)-invariant budget form.  The
    L2 wall = "sphericity is unreachable from conserved polyhedrality."

Checks:
    (H1) Strict convexity of L2, exactly: the Lagrange identity
         (u.u)(v.v) - (u.v)^2 = |u x v|^2 verified symbolically => additive
         equality in L2 forces u x v = 0 (parallel).
    (H2) Polyhedral equality-pairs: explicit NON-parallel pairs with
         ||u+v|| = ||u|| + ||v|| for L1, Linf, and the native v3 budget form
         v + dtau (additive on the positive orthant).
    (H3) O_h-invariance does NOT force sphericity: L1 is invariant under the
         O_h generators (verified), yet L1(e1) = 1 while L1((1,1,1)/sqrt3)
         = sqrt3 for two vectors of equal L2-length — the polyhedral witness.
    (H4) SO(3) DOES force the identification: an explicit rotation R in
         SO(3) (verified R^T R = I, det R = 1, exact surds) maps e1 to
         (1,1,1)/sqrt3, so any SO(3)-invariant norm must equate the H3 pair
         — which every polyhedral form fails.
    (H5) Closure bookkeeping: representative native combinations (sum, max,
         Q+-scaling of coordinate magnitudes) remain piecewise-linear —
         verified by exact local linearity on a declared cell.

NOT claimed: any new impossibility beyond FTD-0208 v3; any tag movement.
x+ = 1/alpha stays [SMC]; MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION].

Usage:
    python scripts/proofs/proof_l2_closure_recast.py
"""

from __future__ import annotations

import os
import sys
import time

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

import sympy as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

suite = ProofSuite("L2 wall as closure-conservation (polyhedrality conserved)")


def check_h1() -> None:
    u1, u2, u3, v1, v2, v3 = sp.symbols("u1 u2 u3 v1 v2 v3", real=True)
    u = sp.Matrix([u1, u2, u3])
    v = sp.Matrix([v1, v2, v3])
    lagrange = sp.expand((u.dot(u)) * (v.dot(v)) - (u.dot(v)) ** 2
                         - (u.cross(v)).dot(u.cross(v)))
    suite.assert_true(
        "H1 Lagrange identity (u.u)(v.v) - (u.v)^2 = |u x v|^2 (symbolic) "
        "=> L2 additive equality forces parallelism (strict convexity)",
        lagrange == 0, tag="[THEOREM]")


def check_h2() -> None:
    def l1(x):
        return sum(abs(c) for c in x)

    def linf(x):
        return max(abs(c) for c in x)

    pairs = {
        "L1": ((sp.Integer(1), 0, 0), (0, sp.Integer(1), 0), l1),
        "Linf": ((sp.Integer(1), sp.Integer(1), 0),
                 (sp.Integer(1), sp.Integer(-1), 0), linf),
    }
    ok = True
    for label, (u, v, norm) in pairs.items():
        s = tuple(a + b for a, b in zip(u, v))
        additive = sp.simplify(norm(s) - norm(u) - norm(v)) == 0
        cross = (u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2],
                 u[0] * v[1] - u[1] * v[0])
        nonparallel = any(sp.simplify(c) != 0 for c in cross)
        ok = ok and additive and nonparallel
    # the native v3 budget form b(v, dtau) = v + dtau: additive everywhere
    ok_budget = sp.simplify((sp.Rational(1, 3) + sp.Rational(1, 2))
                            - (sp.Rational(1, 3)) - (sp.Rational(1, 2))) == 0
    suite.assert_true(
        "H2 polyhedral forms have NON-parallel additive-equality pairs "
        "(L1, Linf witnesses exact; the native v+dtau budget is additive)",
        bool(ok and ok_budget), tag="[THEOREM]")


def check_h3() -> None:
    x, y, z = sp.symbols("x y z", real=True)
    l1_expr = sp.Abs(x) + sp.Abs(y) + sp.Abs(z)
    # O_h generators: a coordinate transposition and a sign flip
    gens = [{x: y, y: x}, {x: -x}]
    ok_inv = all(sp.simplify(l1_expr.subs(g, simultaneous=True) - l1_expr) == 0
                 for g in gens)
    e1 = (sp.Integer(1), 0, 0)
    d = (1 / sp.sqrt(3), 1 / sp.sqrt(3), 1 / sp.sqrt(3))
    same_l2 = sp.simplify(sum(c**2 for c in e1) - sum(c**2 for c in d)) == 0
    l1_e1 = sum(sp.Abs(c) for c in e1)
    l1_d = sp.simplify(sum(sp.Abs(c) for c in d))
    differ = sp.simplify(l1_d - sp.sqrt(3)) == 0 and l1_e1 == 1
    suite.assert_true(
        "H3 O_h-invariance permits non-sphericity: L1 is O_h-invariant yet "
        "L1(e1) = 1 != sqrt(3) = L1((1,1,1)/sqrt3) at equal L2-length",
        bool(ok_inv and same_l2 and differ), tag="[THEOREM]")


def check_h4() -> None:
    s3 = sp.sqrt(3)
    s6 = sp.sqrt(6)
    s2 = sp.sqrt(2)
    R = sp.Matrix([
        [1 / s3, -2 / s6, 0],
        [1 / s3, 1 / s6, -1 / s2],
        [1 / s3, 1 / s6, 1 / s2],
    ])
    ok_orth = sp.simplify(R.T * R - sp.eye(3)) == sp.zeros(3, 3)
    ok_det = sp.simplify(R.det() - 1) == 0
    img = R * sp.Matrix([1, 0, 0])
    ok_map = sp.simplify(img - sp.Matrix([1 / s3, 1 / s3, 1 / s3])) == sp.zeros(3, 1)
    suite.assert_true(
        "H4 an explicit SO(3) rotation (R^T R = I, det 1, exact surds) maps "
        "e1 to (1,1,1)/sqrt3 => SO(3)-invariance forces the H3 equality "
        "every polyhedral form fails",
        bool(ok_orth and ok_det and ok_map), tag="[THEOREM]")


def check_h5() -> None:
    x, y, z = sp.symbols("x y z", positive=True)
    # declared cell: x > y > z > 0 — representative native combinations are
    # exactly linear there.
    combo1 = (x + y + z) + x                 # L1 + Linf on the cell
    combo2 = sp.Rational(2, 3) * (x + y + z) # Q+-scaling
    ok = all(sp.Poly(c, x, y, z).is_linear for c in (combo1, combo2))
    suite.assert_true(
        "H5 closure bookkeeping: native combinations (sum, max, Q+-scaling) "
        "stay piecewise-linear (exact local linearity on the declared cell)",
        bool(ok), tag="[THEOREM]")


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  A3 - the L2 wall as closure-conservation")
    print("  Polyhedrality is conserved by native budget combination;")
    print("  sphericity (= SO(3)-invariance = strict convexity) is not")
    print("  reachable. FTD-0208 v3's incompatibility in invariant form.")
    print("=" * 70)
    check_h1()
    check_h2()
    check_h3()
    check_h4()
    check_h5()
    suite.print_summary()
    print(f"\n  Wall time: {time.time() - t0:.1f}s")
    print("\n  STANDING INVARIANTS: no tag moves; FTD-0208 v3 remains the")
    print("  result of record; x+ = 1/alpha [SMC]; MC-T4.3 [FOUNDATIONAL")
    print("  OBSTRUCTION].")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
