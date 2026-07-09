"""explr_lgf_reconstruct.py — general LGF minimal-ODE reconstruction, the same
modular-rank meet-in-the-middle method as explr_stencil18_reconstruct.py
(FTD-0372), parameterised by a moments file + divisor so it serves the 2D
square lattice, SC, BCC, FCC and the 18-pt stencil uniformly.

Method (identical to FTD-0372):
  1. Load exact integer moments M_n; m_n = M_n / DIVISOR^n.
  2. F(z) = sum m_n z^n is holonomic (Lipshitz 1988). An order-R, degree-D ODE
     gives for each n the linear relation
        sum_{r,d} c_{r,d} * falling(n-d+r, r) * m_{n-d+r} = 0.
  3. DETECT over two 61-bit primes (gcd(DIVISOR,p)=1): build the (order,degree)
     relation matrix mod p, rank by Gaussian elimination; minimal (order, then
     degree) with nullity>=1 and surplus>=SURPLUS on BOTH primes = the operator.
  4. EXTRACT exact rational coefficients at the minimal (R,D) over Q, clear to
     integer coeffs, save to <stem>_operator.json.

NO PSLQ, NO closed-form fishing — reconstructs a STRUCTURAL object (a linear
ODE) from exact data, the standard LGF method.

Usage:
    python scripts/proofs/explr_lgf_reconstruct.py <moments.txt> <DIVISOR> [RMAX]
"""

from __future__ import annotations

import json
import os
import sys
import time
from fractions import Fraction
from math import gcd

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

MOMENTS = sys.argv[1]
DIVISOR = int(sys.argv[2])
RMAX = int(sys.argv[3]) if len(sys.argv) > 3 else 8
SURPLUS = 12
PRIMES = [2305843009213693951, 2305843009213693669]
OPOUT = os.path.splitext(MOMENTS)[0].replace("_moments", "") + "_operator.json"


def load_moments():
    M = []
    with open(MOMENTS, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            n, m = line.split()
            assert int(n) == len(M)
            M.append(int(m))
    return M


def falling(m, r):
    p = 1
    for i in range(r):
        p *= (m - i)
    return p


def rank_mod_p(M, order, degree, p):
    N = len(M) - 1
    unknowns = (order + 1) * (degree + 1)
    lo, hi = degree, N - order
    n_eqs = hi - lo + 1
    if n_eqs < unknowns + SURPLUS:
        return None
    invD = pow(DIVISOR, p - 2, p)
    invDn = [pow(invD, n, p) for n in range(N + 1)]
    Mp = [(m % p) for m in M]
    mp = [(Mp[n] * invDn[n]) % p for n in range(N + 1)]
    rows = []
    for n in range(lo, hi + 1):
        row = [0] * unknowns
        col = 0
        for r in range(order + 1):
            for d in range(degree + 1):
                idx = n - d + r
                ff = falling(idx, r) % p
                row[col] = (ff * mp[idx]) % p
                col += 1
        rows.append(row)
    rank = 0
    ncols = unknowns
    r_i = 0
    nrows = len(rows)
    for c in range(ncols):
        piv = -1
        for rr in range(r_i, nrows):
            if rows[rr][c] != 0:
                piv = rr
                break
        if piv < 0:
            continue
        rows[r_i], rows[piv] = rows[piv], rows[r_i]
        inv = pow(rows[r_i][c], p - 2, p)
        prow = rows[r_i]
        prow[:] = [(x * inv) % p for x in prow]
        for rr in range(nrows):
            if rr != r_i and rows[rr][c] != 0:
                f = rows[rr][c]
                orow = rows[rr]
                orow[:] = [(a - f * b) % p for a, b in zip(orow, prow)]
        r_i += 1
        rank += 1
        if r_i == nrows:
            break
    return rank, n_eqs, unknowns


def exact_nullspace(M, order, degree):
    N = len(M) - 1
    unknowns = (order + 1) * (degree + 1)
    lo, hi = degree, N - order
    m = [Fraction(M[n], DIVISOR ** n) for n in range(N + 1)]
    rows = []
    for n in range(lo, hi + 1):
        row = []
        for r in range(order + 1):
            for d in range(degree + 1):
                idx = n - d + r
                row.append(Fraction(falling(idx, r)) * m[idx])
        rows.append(row)
    ncols = unknowns
    pivots = []
    r_i = 0
    for c in range(ncols):
        piv = None
        for rr in range(r_i, len(rows)):
            if rows[rr][c] != 0:
                piv = rr
                break
        if piv is None:
            continue
        rows[r_i], rows[piv] = rows[piv], rows[r_i]
        pv = rows[r_i][c]
        rows[r_i] = [x / pv for x in rows[r_i]]
        for rr in range(len(rows)):
            if rr != r_i and rows[rr][c] != 0:
                f = rows[rr][c]
                rows[rr] = [a - f * b for a, b in zip(rows[rr], rows[r_i])]
        pivots.append(c)
        r_i += 1
        if r_i == len(rows):
            break
    free = [c for c in range(ncols) if c not in pivots]
    if not free:
        return None
    vec = [Fraction(0)] * ncols
    vec[free[0]] = Fraction(1)
    for pc, pi in reversed(list(zip(pivots, range(len(pivots))))):
        s = -sum(rows[pi][c] * vec[c] for c in range(pc + 1, ncols))
        vec[pc] = s
    return vec


def main():
    t0 = time.time()
    M = load_moments()
    N = len(M) - 1
    print("=" * 70)
    print(f"  LGF reconstruction — {N+1} exact moments, divisor {DIVISOR}")
    print(f"  M_0..M_6 = {M[:7]}")
    print(f"  grid order 2..{RMAX}, surplus>={SURPLUS}, two-prime cross-check")
    print("=" * 70)

    found = None
    for order in range(1, RMAX + 1):
        Dmax = N - order - SURPLUS
        hit = None
        for degree in range(0, Dmax + 1):
            res = rank_mod_p(M, order, degree, PRIMES[0])
            if res is None:
                break
            rank, n_eqs, unknowns = res
            if unknowns - rank >= 1:
                res2 = rank_mod_p(M, order, degree, PRIMES[1])
                if res2[2] - res2[0] >= 1:
                    hit = degree
                    print(f"  CANDIDATE order={order} degree={degree}: "
                          f"nullity>=1 (2 primes), n_eqs={n_eqs}, "
                          f"unknowns={unknowns}, surplus={n_eqs-unknowns} "
                          f"[{time.time()-t0:.0f}s]")
                    break
        if hit is not None:
            found = (order, hit)
            break
        print(f"  order={order}: none up to degree {Dmax} [{time.time()-t0:.0f}s]")

    if found is None:
        print(f"\n  VERDICT: no ODE of order <= {RMAX} within moment budget")
        return 0

    order, degree = found
    print(f"\n  MINIMAL OPERATOR: order={order}, degree={degree}. Extracting exact...")
    vec = exact_nullspace(M, order, degree)
    if vec is None:
        print("  no exact nullspace — abort")
        return 1
    dens = [c.denominator for c in vec if c != 0]
    lcm = 1
    for d in dens:
        lcm = lcm * d // gcd(lcm, d)
    ints = [int(c * lcm) for c in vec]
    g = 0
    for x in ints:
        g = gcd(g, abs(x))
    if g:
        ints = [x // g for x in ints]
    polys = []
    i = 0
    for r in range(order + 1):
        polys.append(ints[i:i + degree + 1])
        i += degree + 1
    print("  operator p_r(z) (low->high degree):")
    for r, coeffs in enumerate(polys):
        if any(coeffs):
            hd = max(j for j, c in enumerate(coeffs) if c)
            print(f"    p_{r}(z): deg {hd}, coeffs {coeffs[:hd+1]}")
    with open(OPOUT, "w", encoding="utf-8") as f:
        json.dump({"order": order, "degree": degree, "polys": polys,
                   "N_moments": N, "divisor": DIVISOR}, f)
    print(f"\n  saved operator to {OPOUT}  ({time.time()-t0:.0f}s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
