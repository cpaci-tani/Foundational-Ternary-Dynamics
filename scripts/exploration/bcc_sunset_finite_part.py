#!/usr/bin/env python3
"""
bcc_sunset_finite_part.py -- extract the two-loop BCC sunset's log coefficient
A_s and finite part B from the EXACT series c_N=d_N/8^N by a PARITY-AWARE
least-squares fit of the coefficient asymptotics (robust regression, NOT
high-degree extrapolation).

Singularity analysis: F(y)=sum c_N y^N has singularities at BOTH y=1 (the
physical mu^2->0 point) and y=-1 (the bipartite BCC walk), so
    c_N = (A_s + A_o(-1)^N)/N + (s1+o1(-1)^N) N^{-3/2}
             + (s2+o2(-1)^N) N^{-2} + (s3+o3(-1)^N) N^{-5/2} + ...
Only the SMOOTH y=1 part sets the mu^2->0 log:  I=-A_s log(mu^2)+B+... .  The
finite part (Abel/transfer theorem):
    B = c_0 + sum_{N>=1}(c_N - A_s/N)
      = c_0 + sum_{N=1}^{M}(c_N - A_s/N) + TAIL(M),
    TAIL(M) = A_o*Salt(1,M) + sum_{j>=1}[ s_j*zeta(1+j/2,M+1)
                                          + o_j*Salt(1+j/2,M) ],
where Salt(sN,M)=sum_{N>M}(-1)^N N^{-sN} = (-1)^{M+1} lerchphi(-1,sN,M+1).

Fit coeffs by LS over a wide window (well-conditioned); cross-validate over
windows & orders; report B with a spread-based error.  No PSLQ, no fishing.

Usage: python3 bcc_sunset_finite_part.py [cN_file] [dps]
"""
from __future__ import annotations

import sys
from mpmath import mp, mpf, matrix, lu_solve, zeta, lerchphi, gamma, pi

CN = sys.argv[1] if len(sys.argv) > 1 else "scripts/exploration/_bcc_sunset_cN.txt"
mp.dps = int(sys.argv[2]) if len(sys.argv) > 2 else 60


def load_c():
    d = []
    for line in open(CN):
        if line.startswith("#") or not line.strip():
            continue
        d.append(int(line.split()[1]))
    p8 = mpf(8)
    return [mpf(d[N]) / p8 ** N for N in range(len(d))]


# smooth exponents 1, 3/2, 2, 5/2, 3, ...  and osc versions with (-1)^N
def basis_exps(Ks, Ko):
    s = [mpf(1) + mpf(j) / 2 for j in range(Ks)]       # 1,3/2,2,...
    o = [mpf(1) + mpf(j) / 2 for j in range(Ko)]
    return s, o


def fit(c, Nlo, Nhi, Ks, Ko):
    """LS fit c_N over [Nlo,Nhi]; returns dict of coeffs keyed ('s',exp)/('o',exp)."""
    sexp, oexp = basis_exps(Ks, Ko)
    cols = [("s", e) for e in sexp] + [("o", e) for e in oexp]
    Ns = list(range(Nlo, Nhi + 1))
    F = matrix(len(Ns), len(cols))
    b = matrix(len(Ns), 1)
    for i, N in enumerate(Ns):
        sign = 1 if N % 2 == 0 else -1
        for jc, (kind, e) in enumerate(cols):
            F[i, jc] = (sign if kind == "o" else 1) * mpf(N) ** (-e)
        b[i] = c[N]
    # normal equations F^T F a = F^T b (well-conditioned for a wide window)
    FT = F.T
    a = lu_solve(FT * F, FT * b)
    return {cols[k]: a[k] for k in range(len(cols))}


def Salt(sN, M):
    """sum_{N>M} (-1)^N N^{-sN} = (-1)^{M+1} lerchphi(-1, sN, M+1)."""
    return ((-1) ** (M + 1)) * lerchphi(-1, sN, M + 1)


def finite_part(c, coef, M):
    A_s = coef[("s", mpf(1))]
    A_o = coef.get(("o", mpf(1)), mpf(0))
    s = c[0]
    for N in range(1, M + 1):
        s += c[N] - A_s / N
    tail = A_o * Salt(mpf(1), M)
    for (kind, e), v in coef.items():
        if e == mpf(1):
            continue  # the 1/N pieces handled (smooth cancels, osc in A_o*Salt)
        if kind == "s":
            tail += v * zeta(e, M + 1)
        else:
            tail += v * Salt(e, M)
    return A_s, s + tail


def main():
    c = load_c()
    Nmax = len(c) - 1
    print(f"loaded {Nmax+1} exact terms (N=0..{Nmax}); dps={mp.dps}")

    print("\n== parity-aware LS fits (A_s = smooth 1/N coeff = log coefficient) ==")
    configs = [
        (Nmax // 3, Nmax, 6, 5),
        (Nmax // 2, Nmax, 6, 5),
        (Nmax // 3, Nmax, 7, 6),
        (2 * Nmax // 5, Nmax, 8, 6),
    ]
    Bs, As = [], []
    for (Nlo, Nhi, Ks, Ko) in configs:
        try:
            coef = fit(c, Nlo, Nhi, Ks, Ko)
            A_s, B = finite_part(c, coef, Nmax)
            As.append(A_s); Bs.append(B)
            print(f"  win[{Nlo:4d},{Nhi}] Ks={Ks} Ko={Ko}:  A_s={mp.nstr(A_s,14)}"
                  f"  A_o={mp.nstr(coef.get(('o',mpf(1)),mpf(0)),8)}  B={mp.nstr(B,16)}")
        except Exception as e:  # noqa: BLE001
            print(f"  win[{Nlo},{Nhi}] Ks={Ks} Ko={Ko}: FAIL {e}")
    if not Bs:
        return 1
    # spread across configs = error proxy
    Bmid = Bs[len(Bs) // 2]
    spread = max(Bs) - min(Bs)
    Aspread = max(As) - min(As)
    print(f"\n  A_s ~ {mp.nstr(sum(As)/len(As),12)}  (spread {mp.nstr(Aspread,3)})")
    print(f"  B   ~ {mp.nstr(Bmid,14)}  (spread across configs {mp.nstr(spread,3)})")
    print(f"  [cross-check] GPU sqrt-ansatz gave B ~ 0.97 -> "
          f"{'CONSISTENT' if abs(Bmid-mpf('0.97'))<0.03 else 'CHECK'}")

    # reference CM-family monomials (for a separate PSLQ if precision allows)
    G14, G13 = gamma(mpf(1)/4), gamma(mpf(1)/3)
    W3 = G14**4/(4*pi**3)
    refs = {"W3": W3, "W3^2/2": W3**2/2, "G14^4/(2pi^4)": G14**4/(2*pi**4),
            "G13^6/(4pi^4)": G13**6/(4*pi**4), "G13^3/(2pi^2)": G13**3/(2*pi**2)}
    print("\n== B / reference (NOT a claim; separate PSLQ decides) ==")
    for n_, v in refs.items():
        print(f"   {n_:16s}={mp.nstr(v,12)}   B/val={mp.nstr(Bmid/v,12)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
