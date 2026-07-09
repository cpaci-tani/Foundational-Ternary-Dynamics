#!/usr/bin/env python3
"""
lattice_two_loop_bcc_modp.py -- FAST mod-p computation of the two-loop BCC sunset
series c_N = d_N/8^N, for guessing/certifying the holonomic operator at large N.

Same object as lattice_two_loop_bcc_series.py (exact), but mod a prime p < 2^20
so all intermediates fit float64's exact-integer range (< 2^53) and the O(N^4)
triple sum becomes BLAS matmuls:

  d_N = sum_{n1+n2+n3=N} U(n1,n2,n3)^3,  U = sum_x b_n1(x)b_n2(x)b_n3(x),
  b_n(x) = C(n,(n+x)/2).

For each n1, form W[n2,x] = wt[x]*b_n1(x)*b_n2(x) (x>=0, wt=1 at 0 else 2 by the
x<->-x symmetry), then U1 = W @ B^T gives U(n1,n2,n3) for ALL (n2,n3) in one
matmul; cube mod p (in two <2^53 steps); anti-diagonal sum (n2+n3=s) via bincount
and scatter into d[n1+s].  ORDERED triples (independent n1,n2,n3) -- matches the
exact build.  c_N mod p = d_N * inv8^N.

Usage: python3 lattice_two_loop_bcc_modp.py <Nmax> <prime> [--validate]
"""
from __future__ import annotations

import sys
import time
import numpy as np

Nmax = int(sys.argv[1]) if len(sys.argv) > 1 else 200
P = int(sys.argv[2]) if len(sys.argv) > 2 else 1048573   # prime < 2^20
VALIDATE = "--validate" in sys.argv
assert P < (1 << 20), "need p < 2^20 so support*p^2 < 2^53 (float64 exact)"

OUT = f"scripts/exploration/_bcc_sunset_cN_modp_{P}.txt"


def build_B(Nmax, P):
    """B[n, x] = C(n,(n+x)/2) mod P for x=0..Nmax (x same parity as n, else 0)."""
    # Pascal by rows in the (n, x>=0) layout: b_n(x) from b_{n-1}(x-1)+b_{n-1}(x+1)
    B = np.zeros((Nmax + 1, Nmax + 1), dtype=np.float64)
    B[0, 0] = 1.0
    for n in range(1, Nmax + 1):
        prev = B[n - 1]
        row = B[n]
        # b_n(x) = b_{n-1}(x-1) + b_{n-1}(x+1); with x>=0 folding, b(-1)=b(1)
        row[1:] += prev[:-1]          # x-1 contribution: row[x]+=prev[x-1], x>=1
        row[:-1] += prev[1:]          # x+1 contribution: row[x]+=prev[x+1]
        row[0] = 2.0 * prev[1]        # b_n(0)=b_{n-1}(-1)+b_{n-1}(1)=2 b_{n-1}(1)
        B[n] %= P
    return B


def build_dN(Nmax, P):
    B = build_B(Nmax, P)
    wt = np.full(Nmax + 1, 2.0); wt[0] = 1.0     # x<->-x symmetry weight
    BT = B.T.copy()                              # (x, n3)
    d = np.zeros(2 * Nmax + 1 + Nmax, dtype=np.float64)  # index up to 3*Nmax
    # index helper for anti-diagonal (n2+n3=s) via bincount on a fixed grid
    n2n3 = (np.arange(Nmax + 1)[:, None] + np.arange(Nmax + 1)[None, :]).ravel()
    t0 = time.time()
    for n1 in range(Nmax + 1):
        W = (wt * B[n1]) * B                     # W[n2,x] = wt*b_n1(x)*b_n2(x)
        W %= P
        U1 = (W @ BT) % P                        # U1[n2,n3] = sum_x wt b_n1 b_n2 b_n3
        U3 = ((U1 * U1) % P * U1) % P            # cube mod p, each step < 2^53
        anti = np.bincount(n2n3, weights=U3.ravel(), minlength=2 * Nmax + 1)
        s = slice(n1, n1 + 2 * Nmax + 1)
        d[s] += anti[:2 * Nmax + 1]
        if n1 % 200 == 0 or n1 == Nmax:
            print(f"  n1={n1}/{Nmax}  ({time.time()-t0:.0f}s)", flush=True)
    d = np.mod(d[:Nmax + 1], P).astype(np.int64)
    return d


def main():
    print(f"mod-p two-loop BCC c_N: Nmax={Nmax}, p={P}")
    d = build_dN(Nmax, P)
    inv8 = pow(8, P - 2, P)
    c = [(int(d[N]) * pow(inv8, N, P)) % P for N in range(Nmax + 1)]
    print(f"  d_0..d_8 mod p = {[int(x) for x in d[:9]]}")
    print(f"  (exact d_0..d_8 = 1,0,24,8,840,648,35368,41496,1651272)")

    if VALIDATE:
        # compare c_N mod p against the exact build's c_N mod p
        exact = []
        try:
            for line in open("scripts/exploration/_bcc_sunset_cN.txt"):
                if line.startswith("#") or not line.strip():
                    continue
                exact.append(int(line.split()[1]))
        except FileNotFoundError:
            exact = []
        if exact:
            ncmp = min(len(exact) - 1, Nmax)
            bad = []
            for N in range(ncmp + 1):
                cx = (exact[N] % P) * pow(inv8, N, P) % P
                if cx != c[N]:
                    bad.append(N)
            print(f"  [validate] vs exact d_N mod p over N=0..{ncmp}: "
                  f"{'PASS' if not bad else 'FAIL at ' + str(bad[:5])}")

    with open(OUT, "w", encoding="utf-8") as f:
        f.write(f"# c_N mod p ; p={P} ; Nmax={Nmax}\n")
        for N in range(Nmax + 1):
            f.write(f"{N} {c[N]}\n")
    print(f"  wrote c_N mod p to {OUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
