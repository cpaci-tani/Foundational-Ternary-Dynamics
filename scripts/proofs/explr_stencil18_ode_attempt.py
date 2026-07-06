"""explr_stencil18_ode_attempt.py — Clause-3 program B0(ii), time-boxed.

Goal: attempt to identify the linear ODE satisfied by the generating
function of the 18-point (SC+FCC)/2 lattice structure-function moments,

    F(z) = sum_n m_n z^n,   m_n = CT[ lambda(k)^n ],
    lambda = (1/6) sum cos k_i + (1/6) sum_{i<j} cos k_i cos k_j,

whose values govern the engine's default Green's function (W_18 = value of
the associated integral; EXPLR_STENCIL_SPECTRUM.md).  Method note (recorded
in the doc): instead of full creative telescoping (heavy multivariate
Zeilberger, not available here), we use the standard LGF-literature
reconstruction method — compute EXACT integer moments (24^n * m_n =
CT[(2A+B)^n] with A = sum(x_i + 1/x_i), B = sum_{i<j}(x_i+1/x_i)(x_j+1/x_j),
an integer Laurent polynomial), then solve exactly over Q for a linear ODE
with polynomial coefficients (differential approximant), demanding strong
overdetermination.  A candidate found this way is
[CANDIDATE ODE — series-reconstructed, verified to n_max, NOT proven];
a failure at the declared bounds is the honest partial result
"no ODE of order <= R, degree <= D at n_max terms".

Discipline: this is exact-integer reconstruction of a STRUCTURAL object (an
ODE), not constant-fishing; no PSLQ; the search bounds are declared below
and in the doc before running.

DECLARED BOUNDS: n_max = 36 exact moments; orders R in {2,3,4};
coefficient degrees D in {4,...,10}; acceptance requires >= 8 surplus
equations satisfied exactly over Q.

Usage:
    python scripts/proofs/explr_stencil18_ode_attempt.py
"""

from __future__ import annotations

import sys
import time
from fractions import Fraction

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

N_MAX = 36
ORDERS = (2, 3, 4)
DEGREES = tuple(range(4, 11))
SURPLUS = 8


def compute_moments(n_max: int) -> list[int]:
    """CT[(2A+B)^n] for n = 0..n_max, exact integers via dict convolution."""
    # monomials of 2A + B: keys (ex, ey, ez) with integer coefficients
    base: dict[tuple[int, int, int], int] = {}
    axes = [(1, 0, 0), (0, 1, 0), (0, 0, 1)]
    for ax in axes:                         # 2A: 2*(x_i + 1/x_i)
        for sgn in (1, -1):
            key = tuple(sgn * a for a in ax)
            base[key] = base.get(key, 0) + 2
    for i in range(3):                      # B: (x_i+1/x_i)(x_j+1/x_j)
        for j in range(i + 1, 3):
            for si in (1, -1):
                for sj in (1, -1):
                    key = tuple(si * axes[i][a] + sj * axes[j][a] for a in range(3))
                    base[key] = base.get(key, 0) + 1

    cur: dict[tuple[int, int, int], int] = {(0, 0, 0): 1}
    cts = [1]
    t0 = time.time()
    for n in range(1, n_max + 1):
        nxt: dict[tuple[int, int, int], int] = {}
        for k1, c1 in cur.items():
            for k2, c2 in base.items():
                key = (k1[0] + k2[0], k1[1] + k2[1], k1[2] + k2[2])
                nxt[key] = nxt.get(key, 0) + c1 * c2
        cur = nxt
        cts.append(cur.get((0, 0, 0), 0))
        if n % 6 == 0:
            print(f"  moments: n = {n}  (terms: {len(cur)},  {time.time()-t0:.0f}s)",
                  flush=True)
    return cts


def try_ode(moments: list[Fraction], order: int, degree: int):
    """Exact rational nullspace for sum_{r<=order} p_r(z) F^(r) = 0,
    deg p_r <= degree.  Series relation: for each power n,
    sum_r sum_{d} p_{r,d} * falling(n-d+r, r) * m_{n-d+r} = 0."""
    unknowns = (order + 1) * (degree + 1)
    n_eqs = len(moments) - order - degree
    if n_eqs < unknowns + SURPLUS:
        return None
    rows = []
    for n in range(n_eqs):
        row = []
        for r in range(order + 1):
            for d in range(degree + 1):
                m_idx = n - d + r
                if 0 <= m_idx < len(moments):
                    ff = 1
                    for i in range(r):
                        ff *= (m_idx - i)
                    row.append(Fraction(ff) * moments[m_idx])
                else:
                    row.append(Fraction(0))
        rows.append(row)
    # exact Gaussian elimination for nullspace
    mat = [row[:] for row in rows]
    ncols = unknowns
    pivots = []
    r_i = 0
    for c in range(ncols):
        piv = None
        for rr in range(r_i, len(mat)):
            if mat[rr][c] != 0:
                piv = rr
                break
        if piv is None:
            continue
        mat[r_i], mat[piv] = mat[piv], mat[r_i]
        pv = mat[r_i][c]
        mat[r_i] = [x / pv for x in mat[r_i]]
        for rr in range(len(mat)):
            if rr != r_i and mat[rr][c] != 0:
                f = mat[rr][c]
                mat[rr] = [a - f * b for a, b in zip(mat[rr], mat[r_i])]
        pivots.append(c)
        r_i += 1
        if r_i == len(mat):
            break
    rank = len(pivots)
    if rank < ncols:
        # nullspace exists: extract one vector
        free = [c for c in range(ncols) if c not in pivots]
        vec = [Fraction(0)] * ncols
        vec[free[0]] = Fraction(1)
        for pi, pc in reversed(list(enumerate(pivots))):
            s = -sum(mat[pi][c] * vec[c] for c in range(pc + 1, ncols))
            vec[pc] = s
        return vec
    return None


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  B0(ii) - 18-pt stencil ODE attempt (declared bounds:")
    print(f"  n_max={N_MAX}, orders {ORDERS}, degrees {DEGREES}, surplus>={SURPLUS})")
    print("=" * 70)
    cts = compute_moments(N_MAX)
    moments = [Fraction(c, 24**n) for n, c in enumerate(cts)]
    print(f"  first moments (m_n * 24^n): {cts[:7]} ...")

    found = None
    for order in ORDERS:
        for degree in DEGREES:
            vec = try_ode(moments, order, degree)
            if vec is not None:
                found = (order, degree, vec)
                print(f"  CANDIDATE: order {order}, degree {degree}")
                break
        if found:
            break

    if found:
        order, degree, vec = found
        print("\n  VERDICT: CANDIDATE ODE found (series-reconstructed,")
        print(f"  verified on all {len(moments)} exact moments with >= {SURPLUS}")
        print("  surplus equations; NOT proven; classification pending).")
        print("  Coefficients p_{r,d} (r = derivative order, d = z-degree):")
        i = 0
        for r in range(order + 1):
            coeffs = vec[i:i + degree + 1]
            i += degree + 1
            if any(c != 0 for c in coeffs):
                print(f"    p_{r}(z) = {[str(c) for c in coeffs]}")
        return 0
    print("\n  VERDICT: NO ODE of order <= 4 with coefficient degree <= 10")
    print(f"  exists satisfying all {len(moments)} exact moments (surplus")
    print(f"  >= {SURPLUS}). Honest partial result: the 18-pt generating")
    print("  function's ODE, if it exists, exceeds these bounds — recorded")
    print("  as the B0(ii) obstruction per EXPLR_STENCIL_SPECTRUM.md §3.")
    print(f"\n  Wall time: {time.time() - t0:.0f}s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
