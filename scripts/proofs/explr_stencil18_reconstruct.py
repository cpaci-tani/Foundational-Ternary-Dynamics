"""explr_stencil18_reconstruct.py — B0(ii) the hard push: find the minimal
annihilating ODE of the 18-point (SC+FCC)/2 lattice Green's function from its
exact moments, by MODULAR rank detection over a wide (order, degree) grid,
then extract the exact rational operator at the minimal size.

Method:
  1. Load exact integer moments M_n (from explr_stencil18_moments.py).
  2. The GF F(z) = sum m_n z^n, m_n = M_n / 24^n, is holonomic (Lipshitz 1988).
     An order-r, degree-d ODE  sum_{r} p_r(z) F^(r)(z) = 0, deg p_r <= d,
     gives, for each power n, the linear relation
        sum_{r=0..R} sum_{d=0..D} c_{r,d} * falling(n-d+r, r) * m_{n-d+r} = 0,
     complete for n in [D, N-R]  (n_eqs = N-R-D+1 equations, (R+1)(D+1) unknowns).
  3. DETECTION over F_p: since gcd(24,p)=1, m_n mod p = M_n * 24^{-n} mod p is a
     machine int -> build the matrix mod a 61-bit prime and compute its rank by
     Gaussian elimination.  nullity = unknowns - rank; a genuine operator gives
     nullity >= 1 with large surplus (n_eqs - unknowns).  Cross-check TWO primes
     to reject unlucky rank drops.  Scan order 2..RMAX, and for each order the
     smallest degree with nullity>=1 & surplus>=SURPLUS -> the MINIMAL operator.
  4. EXACT extraction at the minimal (R,D): solve the nullspace over Q with
     Fraction arithmetic (one solve), clear denominators to integer polynomial
     coefficients, and save the operator to _stencil18_operator.json for the
     classification stage.

Declared search bounds: order R in [2, 12], degree D in [1, DMAX(N)], surplus
>= 12. NO PSLQ, NO closed-form fishing -- this reconstructs a STRUCTURAL object
(a linear ODE) from exact data, the standard LGF method.

Usage:
    python scripts/proofs/explr_stencil18_reconstruct.py
"""

from __future__ import annotations

import json
import os
import sys
import time
from fractions import Fraction

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

HERE = os.path.dirname(os.path.abspath(__file__))
MOMENTS = os.path.join(HERE, "_stencil18_moments.txt")
OPOUT = os.path.join(HERE, "_stencil18_operator.json")

RMAX = 12
SURPLUS = 12
PRIMES = [2305843009213693951, 2305843009213693669]  # two 61-bit primes (2^61-1 and near)


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


def falling(m: int, r: int) -> int:
    p = 1
    for i in range(r):
        p *= (m - i)
    return p


def rank_mod_p(M, order, degree, p):
    """Rank over F_p of the (order,degree) relation matrix. Returns
    (rank, n_eqs, unknowns) or None if not enough equations."""
    N = len(M) - 1
    unknowns = (order + 1) * (degree + 1)
    lo, hi = degree, N - order
    n_eqs = hi - lo + 1
    if n_eqs < unknowns + SURPLUS:
        return None
    inv24 = pow(24, p - 2, p)
    inv24n = [pow(inv24, n, p) for n in range(N + 1)]
    Mp = [ (m % p) for m in M ]
    mp = [ (Mp[n] * inv24n[n]) % p for n in range(N + 1) ]
    # falling(k, r) mod p, precompute per r as needed inline
    rows = []
    for n in range(lo, hi + 1):
        row = [0] * unknowns
        col = 0
        for r in range(order + 1):
            for d in range(degree + 1):
                idx = n - d + r
                # idx always in [0,N] for n in [lo,hi]
                ff = falling(idx, r) % p
                row[col] = (ff * mp[idx]) % p
                col += 1
        rows.append(row)
    # Gaussian elimination mod p
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
    """Solve the nullspace over Q at (order,degree); return one vector
    (list of Fraction) or None."""
    N = len(M) - 1
    unknowns = (order + 1) * (degree + 1)
    lo, hi = degree, N - order
    m = [Fraction(M[n], 24 ** n) for n in range(N + 1)]
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
    piv_set = list(zip(pivots, range(len(pivots))))
    for pc, pi in reversed(piv_set):
        s = -sum(rows[pi][c] * vec[c] for c in range(pc + 1, ncols))
        vec[pc] = s
    return vec


def main() -> int:
    t0 = time.time()
    M = load_moments()
    N = len(M) - 1
    print("=" * 70)
    print(f"  B0(ii) reconstruction — {N+1} exact moments loaded")
    print(f"  M_0..M_6 = {M[:7]}  (expect 1,0,36,336,6588,110880,2106720)")
    print(f"  grid: order 2..{RMAX}, surplus>={SURPLUS}, two-prime cross-check")
    print("=" * 70)

    found = None
    for order in range(2, RMAX + 1):
        Dmax = (N - order - SURPLUS) // 1  # generous; loop breaks on infeasible
        hit_degree = None
        for degree in range(1, Dmax + 1):
            res = rank_mod_p(M, order, degree, PRIMES[0])
            if res is None:
                break  # not enough equations for this order at this/higher degree
            rank, n_eqs, unknowns = res
            nullity = unknowns - rank
            if nullity >= 1:
                # cross-check second prime
                res2 = rank_mod_p(M, order, degree, PRIMES[1])
                null2 = res2[2] - res2[0]
                if null2 >= 1:
                    hit_degree = degree
                    print(f"  CANDIDATE order={order} degree={degree}: "
                          f"nullity={nullity}/{null2} (2 primes), "
                          f"n_eqs={n_eqs}, unknowns={unknowns}, "
                          f"surplus={n_eqs-unknowns}  [{time.time()-t0:.0f}s]")
                    break
        if hit_degree is not None:
            found = (order, hit_degree)
            break
        else:
            print(f"  order={order}: no operator up to degree "
                  f"{Dmax} within moment budget  [{time.time()-t0:.0f}s]")

    if found is None:
        print(f"\n  VERDICT: no ODE of order <= {RMAX} within the moment budget "
              f"(N={N}). Need more moments or higher order.")
        return 0

    order, degree = found
    print(f"\n  MINIMAL OPERATOR: order={order}, degree={degree}. "
          f"Extracting exact rational coefficients...")
    vec = exact_nullspace(M, order, degree)
    if vec is None:
        print("  exact solve found no nullspace (prime/exact mismatch!) — abort")
        return 1
    # organize into p_r(z) = sum_d c_{r,d} z^d ; clear to integer coeffs
    from math import gcd
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
        coeffs = ints[i:i + degree + 1]
        i += degree + 1
        polys.append(coeffs)
    # trailing/leading trims for readability
    print("  operator p_r(z) coefficients (low->high degree):")
    for r, coeffs in enumerate(polys):
        if any(coeffs):
            hi_deg = max(j for j, c in enumerate(coeffs) if c)
            print(f"    p_{r}(z): deg {hi_deg}, coeffs {coeffs[:hi_deg+1]}")
    with open(OPOUT, "w", encoding="utf-8") as f:
        json.dump({"order": order, "degree": degree, "polys": polys,
                   "N_moments": N}, f)
    print(f"\n  saved operator to {OPOUT}")
    print(f"  wall time: {time.time()-t0:.0f}s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
