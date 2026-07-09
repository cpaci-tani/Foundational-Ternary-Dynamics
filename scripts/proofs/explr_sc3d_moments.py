"""explr_sc3d_moments.py — exact moments of the 3D SIMPLE-CUBIC lattice Green's
function, meet-in-the-middle (pipeline validation for FTD-0372's method: the
minimal ODE MUST come out order 3, Joyce 1973 / Glasser-Zucker).

6 nearest-neighbour unit steps on Z^3, each weight 1; 6*sigma_SC = 2 sum cos k_i.
M_n = CT[T^n]_{0,0} = number of length-n closed walks; m_n = M_n / 6^n.
Expected first moments: 1, 0, 6, 0, 90, 0, 1860, ... (OEIS A002896).

Usage: python scripts/proofs/explr_sc3d_moments.py [N]   (default 90)
"""
from __future__ import annotations
import os, sys, time
from collections import defaultdict

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

N = int(sys.argv[1]) if len(sys.argv) > 1 else 90
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "_sc3d_moments.txt")
STEPS = [(1,0,0),( -1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]
DIVISOR = 6

def step(v):
    nv = defaultdict(int)
    for (x,y,z), c in v.items():
        for (dx,dy,dz) in STEPS:
            nv[(x+dx,y+dy,z+dz)] += c
    return nv

def inner(u,v):
    if len(u) > len(v): u,v = v,u
    s = 0; vg = v.get
    for k,c in u.items():
        d = vg(k)
        if d: s += c*d
    return s

def main():
    t0 = time.time()
    print(f"3D SC exact moments, N={N}")
    M = [1]; vkm1 = {(0,0,0):1}; k = 1
    while len(M) <= N:
        vk = step(vkm1)
        if 2*k-1 <= N: M.append(inner(vkm1, vk))
        if 2*k   <= N: M.append(inner(vk, vk))
        vkm1 = vk
        if k % 10 == 0 or 2*k >= N:
            print(f"  depth k={k} (n<={min(2*k,N)}, support={len(vk)}, {time.time()-t0:.0f}s)", flush=True)
        k += 1
    M = M[:N+1]
    print(f"  M_0..M_6 = {M[:7]}  (expect 1,0,6,0,90,0,1860)")
    with open(OUT,"w",encoding="utf-8") as f:
        f.write("# 3D SC exact integer moments; divisor 6\n# N=%d\n" % N)
        for n,m in enumerate(M): f.write(f"{n} {m}\n")
    print(f"  wrote {len(M)} moments to {OUT}  ({time.time()-t0:.0f}s)")
    return 0

if __name__ == "__main__":
    sys.exit(main())
