#!/usr/bin/env python3
"""
lattice_two_loop_bcc_series.py -- the holonomic (exact-series) reformulation of
the M2 two-loop BCC sunset, and its validation against an independent FFT.

Derivation (exact).  With the massive BCC propagator P(k)=1/(1+mu^2-cx cy cz),
    1/(1+mu^2-sigma) = int_0^inf e^{-t(1+mu^2)} e^{t sigma} dt ,  sigma=cx cy cz,
and e^{t cx cy cz} = sum_n (t^n/n!) cx^n cy^n cz^n (the product FACTORIZES over
axes), the position-space propagator is
    G(x1,x2,x3) = sum_n g_n a_n(x1) a_n(x2) a_n(x3),  g_n = 1/(1+mu^2)^{n+1},
    a_n(x) = CT_k[e^{ikx} cos(k)^n] = C(n,(n+x)/2)/2^n = b_n(x)/2^n.
The sunset at external p=0 then collapses to a ONE-dimensional series:
    I(mu^2) = sum_x G(x)^3
            = sum_{n,m,p} T(n,m,p)^3 / (1+mu^2)^{n+m+p+3},   T=U/2^{n+m+p},
    U(n,m,p) = sum_x b_n(x) b_m(x) b_p(x)   (integer 3-walk overlap),
so  I(mu^2) = sum_N c_N / (1+mu^2)^{N+3},  c_N = d_N/8^N,
    d_N = sum_{n+m+p=N} U(n,m,p)^3   (exact integer).
This is a diagonal of a rational function -> holonomic (the FTD-0372 world).
The finite part B of I = -A log(mu^2) + B + C sqrt(mu^2) + ... is then obtained
by RIGOROUS y->1 (mu^2->0) singularity analysis of F(y)=sum_N c_N y^N,
y=1/(1+mu^2) -- no fit-model ambiguity.  This file does STEP 1: build c_N
exactly and VALIDATE the reformulation against an independent numpy FFT.

NO PSLQ, NO closed-form fishing here -- this is an exact structural identity
check.  Promotes nothing.
"""
from __future__ import annotations

import sys
import time
from fractions import Fraction
from math import comb

import numpy as np   # numpy 1.24.x (the Sage-pinned system numpy); CPU FFT only

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

NMAX = int(sys.argv[1]) if len(sys.argv) > 1 else 90


# ----------------------------------------------------------------------
# exact integer walk amplitudes and the c_N sequence
# ----------------------------------------------------------------------
def bvec(n):
    """b_n(x) = C(n,(n+x)/2) for x = -n,-n+2,...,n (same parity as n)."""
    return {x: comb(n, (n + x) // 2) for x in range(-n, n + 1, 2)}


def build_cN(nmax):
    """Return d_N (int) and c_N = Fraction(d_N, 8^N) for N=0..nmax."""
    B = [bvec(n) for n in range(nmax + 1)]
    d = [0] * (nmax + 1)
    for N in range(nmax + 1):
        tot = 0
        # symmetric enumeration n<=m<=p, n+m+p=N, all same parity
        for n in range(0, N // 3 + 1):
            for m in range(n, (N - n) // 2 + 1):
                p = N - n - m
                if p < m:
                    continue
                if (n % 2 != m % 2) or (m % 2 != p % 2):
                    continue  # U=0 unless all same parity
                bn, bm, bp = B[n], B[m], B[p]
                lo = max(min(bn), min(bm), min(bp))
                hi = min(max(bn), max(bm), max(bp))
                U = 0
                for x in range(lo, hi + 1, 2):
                    U += bn[x] * bm[x] * bp[x]
                U3 = U ** 3
                # permutation multiplicity
                if n == m == p:
                    mult = 1
                elif n == m or m == p or n == p:
                    mult = 3
                else:
                    mult = 6
                tot += mult * U3
        d[N] = tot
    c = [Fraction(d[N], 8 ** N) for N in range(nmax + 1)]
    return d, c


def I_series(c, mu2, nmax):
    """I(mu^2) = sum_{N=0}^{nmax} c_N / (1+mu^2)^{N+3}, high-precision float."""
    inv = 1.0 / (1.0 + mu2)
    s = 0.0
    p = inv ** 3
    for N in range(nmax + 1):
        s += float(c[N]) * p
        p *= inv
    return s


# ----------------------------------------------------------------------
# independent check: I(mu^2) = sum_x G(x)^3 via CPU FFT (infinite-lattice
# limit reached for mu^2 >~ 0.1 at L=128 -- finite-size ~ exp(-L*mu))
# ----------------------------------------------------------------------
def I_fft(L, mu2):
    j = np.arange(L)
    c = np.cos(2.0 * np.pi * j / L)
    sig = c[:, None, None] * c[None, :, None] * c[None, None, :]
    G = np.fft.ifftn(1.0 / (1.0 - sig + mu2)).real
    return float(np.sum(G ** 3))


def main():
    t0 = time.time()
    print(f"== M2 holonomic reformulation: build c_N (N<= {NMAX}) + validate ==")
    d, c = build_cN(NMAX)
    print(f"  built c_N in {time.time()-t0:.1f}s")
    print("  d_N (integer)  N=0..8 :", d[:9])
    print("  c_N=d_N/8^N    N=0..8 :", [f"{float(x):.6f}" for x in c[:9]])
    # one-loop cross-check: c_0 = T(0,0,0)^3 = 1  (U(0,0,0)=1)
    ok0 = (c[0] == 1)
    print(f"  [check] c_0 == 1 (U(0,0,0)=1): {'PASS' if ok0 else 'FAIL'}")

    print("\n  reformulation vs independent FFT (L=128):")
    all_ok = ok0
    for mu2 in (0.30, 0.20, 0.15):
        Is = I_series(c, mu2, NMAX)
        If = I_fft(128, mu2)
        rel = abs(Is - If) / abs(If)
        tag = "PASS" if rel < 1e-6 else ("~ok" if rel < 1e-3 else "FAIL")
        if rel >= 1e-6:
            all_ok = False
        print(f"    mu^2={mu2:.2f}:  series={Is:.10f}  fft={If:.10f}  "
              f"rel={rel:.2e}  [{tag}]")
    print("    (series truncation at N<=%d limits the lowest mu^2; higher mu^2"
          " converges faster)" % NMAX)

    # persist the exact sequence for the recurrence-guess / singularity step
    out = "scripts/exploration/_bcc_sunset_cN.txt"
    with open(out, "w", encoding="utf-8") as f:
        f.write("# N  d_N (c_N = d_N / 8^N); two-loop BCC sunset series\n")
        for N in range(NMAX + 1):
            f.write(f"{N} {d[N]}\n")
    print(f"\n  wrote d_N (N<= {NMAX}) to {out}")
    print(f"  VALIDATION {'PASSED' if all_ok else 'INCOMPLETE'} "
          f"({time.time()-t0:.1f}s)")
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
