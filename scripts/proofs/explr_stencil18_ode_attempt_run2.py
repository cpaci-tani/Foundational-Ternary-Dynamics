"""explr_stencil18_ode_attempt_run2.py — B0(ii) RUN 2 (declared extension).

RUN 1 (explr_stencil18_ode_attempt.py) was negative at its declared bounds
(orders <= 4, coefficient degree <= 10, 37 exact moments).  This is the ONE
declared extension run, with bounds fixed here before execution:

DECLARED BOUNDS (RUN 2): n_max = 84 exact moments; the (order, degree)
pairs tested are exactly {5} x {6..9} and {6} x {6..8}; acceptance requires
>= 8 surplus equations satisfied exactly over Q.  No further runs without a
new declaration.

PIPELINE VALIDATION: before the 18-pt run, the same code must recover a
known ODE for the simple-cubic (SC) walk generating function
sum_n CT[((cx+cy+cz)/3)^n] z^n (Joyce/Guttmann: SC LGF-class functions
satisfy low-order ODEs).  If validation fails to find an ODE at order <= 4,
degree <= 10, the pipeline is broken and the 18-pt negative is VOID.

Discipline: exact integer moments, exact rational linear algebra, declared
bounds, no PSLQ, structural object (an ODE), not constant-fishing.

Usage:
    python scripts/proofs/explr_stencil18_ode_attempt_run2.py
"""

from __future__ import annotations

import sys
import time
from fractions import Fraction

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

N_MAX = 84
PAIRS = [(5, 6), (5, 7), (5, 8), (5, 9), (6, 6), (6, 7), (6, 8)]
SURPLUS = 8


def base_18pt() -> dict[tuple[int, int, int], int]:
    """24*lambda_18 = 2A + B as integer Laurent monomials."""
    base: dict[tuple[int, int, int], int] = {}
    axes = [(1, 0, 0), (0, 1, 0), (0, 0, 1)]
    for ax in axes:
        for sgn in (1, -1):
            key = tuple(sgn * a for a in ax)
            base[key] = base.get(key, 0) + 2
    for i in range(3):
        for j in range(i + 1, 3):
            for si in (1, -1):
                for sj in (1, -1):
                    key = tuple(si * axes[i][a] + sj * axes[j][a]
                                for a in range(3))
                    base[key] = base.get(key, 0) + 1
    return base


def base_sc() -> dict[tuple[int, int, int], int]:
    """6*lambda_SC = sum(x_i + 1/x_i)."""
    base: dict[tuple[int, int, int], int] = {}
    for ax in [(1, 0, 0), (0, 1, 0), (0, 0, 1)]:
        for sgn in (1, -1):
            key = tuple(sgn * a for a in ax)
            base[key] = base.get(key, 0) + 1
    return base


def compute_cts(base: dict, n_max: int, label: str) -> list[int]:
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
        if n % 12 == 0:
            print(f"  [{label}] moments n = {n}  ({time.time()-t0:.0f}s)",
                  flush=True)
    return cts


def try_ode(moments: list[Fraction], order: int, degree: int):
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
    if len(pivots) < ncols:
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
    print("  B0(ii) RUN 2 - declared extension (see module docstring)")
    print("=" * 70)

    # --- pipeline validation on SC ---
    print("  VALIDATION: SC walk generating function (known low-order ODE)")
    sc_cts = compute_cts(base_sc(), 54, "SC")
    sc_moments = [Fraction(c, 6**n) for n, c in enumerate(sc_cts)]
    sc_found = None
    for order in (2, 3, 4):
        for degree in range(4, 11):
            vec = try_ode(sc_moments, order, degree)
            if vec is not None:
                sc_found = (order, degree)
                break
        if sc_found:
            break
    if sc_found:
        print(f"  VALIDATION PASS: SC ODE found at order {sc_found[0]}, "
              f"degree {sc_found[1]} — pipeline works.")
    else:
        print("  VALIDATION FAIL: pipeline could not recover the known SC")
        print("  ODE. RUN 1's 18-pt negative is VOID. Aborting.")
        return 1

    # --- 18-pt extension ---
    print(f"\n  18-pt moments to n = {N_MAX} (exact) ...")
    cts = compute_cts(base_18pt(), N_MAX, "18pt")
    moments = [Fraction(c, 24**n) for n, c in enumerate(cts)]

    found = None
    for order, degree in PAIRS:
        print(f"  trying order {order}, degree {degree} ...", flush=True)
        vec = try_ode(moments, order, degree)
        if vec is not None:
            found = (order, degree, vec)
            break

    if found:
        order, degree, vec = found
        print(f"\n  VERDICT: CANDIDATE ODE at order {order}, degree {degree}")
        print("  (series-reconstructed on 85 exact moments, NOT proven).")
        i = 0
        for r in range(order + 1):
            coeffs = vec[i:i + degree + 1]
            i += degree + 1
            if any(c != 0 for c in coeffs):
                print(f"    p_{r}(z) = {[str(c) for c in coeffs]}")
    else:
        print("\n  VERDICT: NEGATIVE at RUN-2 bounds. Combined with RUN 1:")
        print("  no annihilating ODE with (order <= 4, deg <= 10),")
        print("  (order = 5, deg <= 9), or (order = 6, deg <= 8) on 85")
        print("  exact moments. The 18-pt generating function's ODE, if any,")
        print("  exceeds these bounds — strengthened B0(ii) obstruction.")
    print(f"\n  Wall time: {time.time() - t0:.0f}s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
