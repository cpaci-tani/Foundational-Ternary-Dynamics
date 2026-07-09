#!/usr/bin/env python3
"""
bcc_sunset_pslq.py -- consolidate the two-loop BCC sunset constants and run the
pre-registered M2 PSLQ (SCOPE_DISCRETE_FEYNMAN_PROGRAM.md sec4).

Inputs: exact c_N=d_N/8^N (lattice_two_loop_bcc_series.py).  Establishes:
  * A_s (log coefficient) -- PSLQ vs {1/pi^2}: expect A_s = 4/pi^2 (genus-0).
  * B (finite part) -- computed with A_s FIXED EXACT (removes the A_s*H_M error),
    tail from a parity-aware LS of the residual; convergence over M = error bar.
  * PSLQ B against the pre-registered CM basis to test the falsifier
    (lemniscatic Gamma(1/4) vs equianharmonic Gamma(1/3)) -- honestly reported;
    at the achieved precision a null result means "no low-height relation".
No fishing: a fixed pre-registered basis, low-height only, residual at the floor.
"""
from __future__ import annotations

import sys
from mpmath import mp, mpf, matrix, lu_solve, zeta, lerchphi, gamma, pi, pslq, log

CN = "scripts/exploration/_bcc_sunset_cN.txt"
mp.dps = int(sys.argv[1]) if len(sys.argv) > 1 else 40


def load_c():
    d = []
    for line in open(CN):
        if line.startswith("#") or not line.strip():
            continue
        d.append(int(line.split()[1]))
    return [mpf(d[N]) / mpf(8) ** N for N in range(len(d))]


def Salt(sN, M):
    return ((-1) ** (M + 1)) * lerchphi(-1, sN, M + 1)


def fit_residual(c, A_s, Nlo, Nhi, Ks, Ko):
    """Fit r_N = c_N - A_s/N over [Nlo,Nhi] to smooth {N^-3/2,N^-2,...} +
    osc (-1)^N{N^-3/2,...}.  Returns coeff dict keyed ('s'/'o', exp)."""
    sexp = [mpf(3) / 2 + mpf(j) / 2 for j in range(Ks)]   # 3/2,2,5/2,...
    oexp = [mpf(3) / 2 + mpf(j) / 2 for j in range(Ko)]
    cols = [("s", e) for e in sexp] + [("o", e) for e in oexp]
    Ns = list(range(Nlo, Nhi + 1))
    F = matrix(len(Ns), len(cols)); b = matrix(len(Ns), 1)
    for i, N in enumerate(Ns):
        sgn = 1 if N % 2 == 0 else -1
        for jc, (k, e) in enumerate(cols):
            F[i, jc] = (sgn if k == "o" else 1) * mpf(N) ** (-e)
        b[i] = c[N] - A_s / N
    FT = F.T; a = lu_solve(FT * F, FT * b)
    return {cols[k]: a[k] for k in range(len(cols))}


def B_of_M(c, A_s, coef, M):
    s = c[0] + sum(c[N] - A_s / N for N in range(1, M + 1))
    tail = mpf(0)
    for (k, e), v in coef.items():
        tail += v * (zeta(e, M + 1) if k == "s" else Salt(e, M))
    return s + tail


def main():
    c = load_c(); Nmax = len(c) - 1
    print(f"loaded {Nmax+1} exact terms; dps={mp.dps}")

    # --- A_s: fit, then PSLQ vs 1/pi^2 ---
    # quick fit for A_s (smooth 1/N) parity-aware
    from bcc_sunset_finite_part import fit as fullfit  # reuse
    coefA = fullfit(c, Nmax // 3, Nmax, 6, 5)
    A_fit = coefA[("s", mpf(1))]
    print(f"\nA_s (fit)   = {mp.nstr(A_fit, 14)}")
    print(f"4/pi^2      = {mp.nstr(4/pi**2, 14)}   diff = {mp.nstr(A_fit-4/pi**2,3)}")
    rel = pslq([A_fit, 1 / pi**2], maxcoeff=10**6, maxsteps=10**5)
    print(f"PSLQ[A_s, 1/pi^2] = {rel}  -> "
          f"{'A_s = 4/pi^2' if rel and abs(rel[0])==1 else 'see coeffs'}")

    # --- B with A_s = 4/pi^2 EXACT; convergence over M ---
    A_s = 4 / pi**2
    coef = fit_residual(c, A_s, 2 * Nmax // 5, Nmax, 7, 6)
    print("\nB with A_s=4/pi^2 exact, convergence over M:")
    Bvals = []
    for M in (Nmax, Nmax - 100, Nmax - 200, Nmax - 400):
        B = B_of_M(c, A_s, coef, M); Bvals.append(B)
        print(f"   M={M:5d}: B = {mp.nstr(B, 18)}")
    B = Bvals[0]
    err = max(abs(Bvals[i] - Bvals[0]) for i in range(1, len(Bvals)))
    print(f"   -> B = {mp.nstr(B, 16)}   (M-convergence err ~ {mp.nstr(err,3)})")

    # --- pre-registered PSLQ of B against the CM basis ---
    G14, G13 = gamma(mpf(1)/4), gamma(mpf(1)/3)
    W3 = G14**4/(4*pi**3)
    print("\n== pre-registered PSLQ of B (falsifier) ==")
    tests = {
        "lemniscatic {1, B, W3, W3^2, 1/pi^2}":
            [mpf(1), B, W3, W3**2, 1/pi**2],
        "equianharm  {1, B, G13^6/pi^4, G13^3/pi^2, 1/pi^2}":
            [mpf(1), B, G13**6/pi**4, G13**3/pi**2, 1/pi**2],
        "mixed+log2+zeta3 {1,B,W3^2,G13^6/pi^4,log2/pi^2,zeta3/pi^3}":
            [mpf(1), B, W3**2, G13**6/pi**4, log(2)/pi**2, zeta(3)/pi**3],
    }
    for name, vec in tests.items():
        r = pslq(vec, maxcoeff=10**5, maxsteps=10**5, tol=mpf(10)**(-(mp.dps-6)))
        print(f"   {name}\n      -> {r}")
    print("\n(NOTE: with B to ~9-11 digits a multi-term PSLQ is UNDERPOWERED; a"
          " null / high-height result means no low-height CM relation at this"
          " precision -- consistent with a two-loop period that is NOT a simple"
          " Gamma-quotient. A decisive verdict needs the certified order-18"
          " operator's connection constants.)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
