"""explr_square2d_moments.py — exact moments of the 2D SQUARE-lattice Green's
function, computed by meet-in-the-middle (same method as FTD-0372's 18-pt run,
adapted to 2D).

The 2D square lattice nearest-neighbour symbol (4 steps) is
    sigma_2(k) = (1/4)(cos kx + cos ky) after normalisation, but for the LGF
    generating function it is cleanest to keep the integer walk operator
        T = sum over the 4 unit steps (+-e1, +-e2), each weight 1,
so 4*sigma_2(k) = 2(cos kx + cos ky) as a Laurent operator, and the exact
integer return moments are
    M_n := CT_k[ T^n ]_{0,0}  = number of length-n closed walks from origin
                             = ( n choose n/2 )^2  for n even, 0 for n odd
                             (the classic 2D central-binomial-squared count).
The generating function F(z) = sum_n m_n z^n with m_n = M_n / 4^n is the
lattice Green's function at the origin, = (2/pi) K(4z) up to normalisation:
this is the ELLIPTIC / complete-elliptic-integral period, structurally a
hypergeometric 2F1(1/2,1/2;1;.) — an order-2 (elliptic) LGF.

Meet-in-the-middle: with v_k := T^k delta_0 and T symmetric,
    M_n = <v_a, v_{n-a}>  for any split a.
So propagating only to depth ceil(N/2) yields ALL moments up to N. Exact
bigints, no floats.

Output: writes exact integer moments M_0..M_N to _square2d_moments.txt.

Usage:
    python scripts/proofs/explr_square2d_moments.py [N]     (default N=120)
"""

from __future__ import annotations

import os
import sys
import time
from collections import defaultdict
from math import comb

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

N = int(sys.argv[1]) if len(sys.argv) > 1 else 120
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                   "_square2d_moments.txt")

# 4 nearest-neighbour unit steps on Z^2, each weight 1
STEPS = [(1, 0, 1), (-1, 0, 1), (0, 1, 1), (0, -1, 1)]
DIVISOR = 4  # 4*sigma normalisation


def step(v: dict) -> dict:
    nv = defaultdict(int)
    for (x, y), c in v.items():
        for (dx, dy, w) in STEPS:
            nv[(x + dx, y + dy)] += w * c
    return nv


def inner(u: dict, v: dict) -> int:
    if len(u) > len(v):
        u, v = v, u
    s = 0
    vg = v.get
    for k, c in u.items():
        d = vg(k)
        if d:
            s += c * d
    return s


def main() -> int:
    t0 = time.time()
    print(f"2D square-lattice exact moments via meet-in-the-middle, N={N}")
    M = [1]
    vk_minus_1 = {(0, 0): 1}
    k = 1
    while len(M) <= N:
        vk = step(vk_minus_1)
        if 2 * k - 1 <= N:
            M.append(inner(vk_minus_1, vk))
        if 2 * k <= N:
            M.append(inner(vk, vk))
        vk_minus_1 = vk
        if k % 10 == 0 or 2 * k >= N:
            print(f"  depth k={k} (n up to {min(2*k, N)}, "
                  f"support={len(vk)}, {time.time()-t0:.0f}s)", flush=True)
        k += 1

    M = M[:N + 1]
    # closed-form cross-check: M_{2m} = C(2m, m)^2, M_odd = 0
    ok = all(M[2 * m] == comb(2 * m, m) ** 2 for m in range(N // 2 + 1)) and \
        all(M[2 * m + 1] == 0 for m in range((N - 1) // 2 + 1))
    print(f"  M_0..M_6 = {M[:7]}  (expect 1,0,4,0,36,0,400)")
    print(f"  closed-form check M_2m == C(2m,m)^2, M_odd==0: {'PASS' if ok else 'FAIL'}")
    with open(OUT, "w", encoding="utf-8") as f:
        f.write("# 2D square-lattice exact integer moments M_n = CT[T^n]; divisor 4\n")
        f.write(f"# N={N}\n")
        for n, m in enumerate(M):
            f.write(f"{n} {m}\n")
    print(f"  wrote {len(M)} moments to {OUT}  ({time.time()-t0:.0f}s)")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
