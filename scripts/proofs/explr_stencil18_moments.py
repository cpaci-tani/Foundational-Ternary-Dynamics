"""explr_stencil18_moments.py — exact moments of the 18-point (SC+FCC)/2
lattice symbol, computed by meet-in-the-middle (B0(ii), the hard push).

M_n := CT_k[ (2A + B)^n ]  (exact non-negative integer), where
    2A = 2*sum_i (x_i + 1/x_i)            (6 nearest-neighbour steps, weight 2)
    B  = sum_{i<j}(x_i+1/x_i)(x_j+1/x_j)  (12 next-nearest steps, weight 1)
so 24*sigma_18(k) = 2A + B as an integer Laurent operator T on Z^3, and
M_n = (T^n)_{0,0} = the weighted count of length-n closed walks from origin.
The rational moment of the generating function F(z) = sum m_n z^n is
m_n = M_n / 24^n.

Meet-in-the-middle: with v_k := T^k delta_0 and T symmetric,
    M_n = <v_a, v_{n-a}>  for any split a  (T^n_{0,0} = sum_p (T^a)_{p,0}(T^{n-a})_{p,0}).
So propagating only to depth ceil(N/2) yields ALL moments up to N:
    M_{2k}   = <v_k, v_k>,   M_{2k-1} = <v_{k-1}, v_k>.
This is ~16x faster than the naive length-N propagation and needs only two
vectors in memory. NO floats, NO symmetry approximation — exact bigints.

Output: writes the exact integer moments M_0..M_N (decimal, one per line) to
scripts/proofs/_stencil18_moments.txt, for the reconstruction stage.

Usage:
    python scripts/proofs/explr_stencil18_moments.py [N]     (default N=160)
"""

from __future__ import annotations

import os
import sys
import time
from collections import defaultdict

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

N = int(sys.argv[1]) if len(sys.argv) > 1 else 160
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                   "_stencil18_moments.txt")


def build_steps():
    steps = []
    axes = [(1, 0, 0), (0, 1, 0), (0, 0, 1)]
    for ax in axes:                       # 2A: nearest neighbour, weight 2
        for s in (1, -1):
            steps.append((s * ax[0], s * ax[1], s * ax[2], 2))
    for i in range(3):                    # B: next-nearest (e_i +- e_j), weight 1
        for j in range(i + 1, 3):
            for si in (1, -1):
                for sj in (1, -1):
                    v = [0, 0, 0]
                    v[i] += si
                    v[j] += sj
                    steps.append((v[0], v[1], v[2], 1))
    return steps


STEPS = build_steps()
assert len(STEPS) == 18


def step(v: dict) -> dict:
    nv = defaultdict(int)
    for (x, y, z), c in v.items():
        for (dx, dy, dz, w) in STEPS:
            nv[(x + dx, y + dy, z + dz)] += w * c
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
    print(f"18-pt exact moments via meet-in-the-middle, target N={N}")
    M = [1]                                # M_0 = <v_0, v_0> = 1
    vk_minus_1 = {(0, 0, 0): 1}            # v_0
    k = 1
    while len(M) <= N:
        vk = step(vk_minus_1)              # v_k
        if 2 * k - 1 <= N:
            M.append(inner(vk_minus_1, vk))   # M_{2k-1}
        if 2 * k <= N:
            M.append(inner(vk, vk))           # M_{2k}
        vk_minus_1 = vk
        if k % 5 == 0 or 2 * k >= N:
            print(f"  depth k={k}  (moments up to n={min(2*k, N)}, "
                  f"support={len(vk)}, {time.time()-t0:.0f}s)", flush=True)
        k += 1

    M = M[:N + 1]
    # sanity: first few (should match B0: 24^n m_n = 1,0,36,336,6588,110880,2106720)
    print(f"  M_0..M_6 = {M[:7]}")
    with open(OUT, "w", encoding="utf-8") as f:
        f.write(f"# 18-pt (SC+FCC)/2 exact integer moments M_n = CT[(2A+B)^n]; divisor 24\n")
        f.write(f"# N={N}\n")
        for n, m in enumerate(M):
            f.write(f"{n} {m}\n")
    print(f"  wrote {len(M)} moments to {OUT}")
    print(f"  wall time: {time.time()-t0:.0f}s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
