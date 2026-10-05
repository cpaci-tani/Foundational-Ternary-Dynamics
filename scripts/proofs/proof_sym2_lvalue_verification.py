#!/usr/bin/env python3
"""
F1 closure — In-repo reproduction of the symmetric-square L-value behind the master-quadratic
trace coefficient, 16 G*^2 = 512 L(Sym^2 E, 1), for E : y^2 = x^3 - x (LMFDB 32.a3, Cremona 32a2).

Companion to scripts/proofs/proof_lvalue_deligne_verification.py, which reproduces L(E,1) = varpi/4
but leaves the Sym^2 value as a cited import. This script reproduces the Sym^2 value from its own
Dirichlet series instead of substituting the closed form.

NORMALIZATION (stated, because the identity depends on it). Arithmetic normalization: at a good
prime p, with alpha + beta = a_p(E), alpha beta = p and T = p^-s, the local factor is
1/((1 - alpha^2 T)(1 - p T)(1 - beta^2 T)); the conductor is 64, the functional equation relates
s and 3 - s, the critical points are s = 1 and s = 2, and the centre is s = 3/2 (so s = 1 is a
critical point but NOT the central one). The local factor at p = 2 is 1. In the analytic
normalization (s and 1 - s), "s = 1" is the arithmetic s = 2, a different number.

ROUTE (each step force-computed this run with mpmath; nothing recalled):
  (1) g = eta(4 tau)^6 = q prod (1 - q^(4n))^6, the weight-3 CM newform of level 16 with character
      chi_-4. Check b(p) = a_p(E)^2 - 2p for p = 1 mod 4 and b(p) = 0 for p = 3 mod 4, a_p(E) from
      point counts. (Equality of the two forms follows from the Sturm bound 6 of S_3(Gamma0(16),
      chi_-4); the check here runs far past it.)
  (2) Coefficient identity L(Sym^2 E, s) = L(g, s) L(chi_-4, s - 1): the Dirichlet coefficients
      built from the Sym^2 Euler product (from a_p(E) alone) equal sum_{d | n} b(n/d) chi_-4(d) d.
  (3) Root number of g is +1: Lambda(s) = (2/pi)^s Gamma(s) L(g, s) computed by splitting the
      Mellin integral at y = 1 and at y = 1.3 must agree. (Proof: eta(-1/tau) = sqrt(tau/i) eta(tau)
      gives g(-1/(16 tau)) = 64 i tau^3 g(tau), so F(y) = g(iy/4) obeys F(1/y) = y^3 F(y).)
  (4) L(g, 1), L(g, 2) from the rapidly convergent split series; L(chi_-4, 0) = 1/2.
  (5) L(Sym^2 E, 1) = L(g, 1) L(chi_-4, 0) against varpi^2/(8 pi) = G*^2/32 = Gamma(1/4)^4/(64 pi^2),
      and 512 L(Sym^2 E, 1) against 16 G*^2. Also L(g, 2) against Gamma(1/4)^4/(64 pi), the exact
      value proved by Rogers, Wan and Zucker (Ramanujan J. 2015, arXiv:1303.2259, Theorem 5), which
      together with step (3) gives an exact proof of the Sym^2 value.
  (6) Analytic-normalization caveat: the arithmetic value at s = 2 is L(g, 2) L(chi_-4, 1)
      = Gamma(1/4)^4/256 = (pi^2/4) L(Sym^2 E, 1), so the identity fails there.
  (7) Optional cross-check with PARI/GP lfunsympow if cypari2 is installed (skipped otherwise).

This is a verification (reproducing a known identity), NOT a look-elsewhere search.
Run: python scripts/proofs/proof_sym2_lvalue_verification.py
"""

from __future__ import annotations

import sys

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

import mpmath as mp

mp.mp.dps = 50

PI = mp.pi
VARPI = mp.gamma(mp.mpf(1) / 4) ** 2 / (2 * mp.sqrt(2 * PI))   # lemniscate constant
GSTAR = 2 * VARPI / mp.sqrt(PI)                                 # = Gamma(1/4)/Gamma(3/4)
G14_4 = mp.gamma(mp.mpf(1) / 4) ** 4
TOL = mp.mpf("1e-40")


def sieve_primes(limit: int):
    s = [True] * (limit + 1)
    s[0] = s[1] = False
    for i in range(2, int(limit ** 0.5) + 1):
        if s[i]:
            for j in range(i * i, limit + 1, i):
                s[j] = False
    return [i for i in range(2, limit + 1) if s[i]]


def ap_pointcount(p: int) -> int:
    """a_p(E) = p + 1 - #E(F_p) for an odd prime p, E: y^2 = x^3 - x."""
    affine = 0
    for x in range(p):
        rhs = (x * x * x - x) % p
        if rhs == 0:
            affine += 1
        elif pow(rhs, (p - 1) // 2, p) == 1:
            affine += 2
    return p - affine


def eta4_6_coefficients(nmax: int):
    """b(n), n = 0..nmax, for g = q * prod_{n>=1} (1 - q^(4n))^6."""
    poly = [0] * (nmax + 1)
    poly[0] = 1
    for n in range(1, nmax // 4 + 1):
        for _ in range(6):
            for i in range(nmax, 4 * n - 1, -1):
                poly[i] -= poly[i - 4 * n]
    return [0] + poly[:nmax]


def chi_m4(n: int) -> int:
    return 0 if n % 2 == 0 else (1 if n % 4 == 1 else -1)


def sym2_coefficients(nmax: int, ap: dict):
    """Dirichlet coefficients of L(Sym^2 E, s) (arithmetic normalization, trivial factor at 2),
    built from a_p(E) alone: at odd p the local series is 1/(1 - e1 T + e2 T^2 - e3 T^3) with
    e1 = a_p^2 - p, e2 = p (a_p^2 - p), e3 = p^3."""
    c = [0] * (nmax + 1)
    c[1] = 1
    local = {}
    for p, a in ap.items():
        if p == 2:
            continue
        e1, e2, e3 = a * a - p, p * (a * a - p), p ** 3
        h = [1]
        k = 1
        while p ** k <= nmax:
            hk = e1 * h[k - 1] - (e2 * h[k - 2] if k >= 2 else 0) + (e3 * h[k - 3] if k >= 3 else 0)
            h.append(hk)
            k += 1
        local[p] = h
    for n in range(2, nmax + 1):
        m, val = n, 1
        if m % 2 == 0:
            c[n] = 0
            continue
        for p in local:
            if p * p > m:
                break
            if m % p == 0:
                e = 0
                while m % p == 0:
                    m //= p
                    e += 1
                val *= local[p][e]
        if m > 1:
            val *= local[m][1]
        c[n] = val
    return c


def Lambda(s, t0, b, nb: int):
    """Lambda(s) = (2/pi)^s Gamma(s) L(g, s), split at y = t0, root number +1, weight 3, level 16."""
    total = mp.mpf(0)
    for n in range(1, nb + 1):
        if b[n]:
            x = PI * n / 2
            total += b[n] * (x ** (-s) * mp.gammainc(s, x * t0) + x ** (-(3 - s)) * mp.gammainc(3 - s, x / t0))
    return total


def check(label, ok, detail=""):
    print(f"  {'PASS' if ok else 'FAIL'}  {label}{('  ' + detail) if detail else ''}")
    return bool(ok)


def main():
    print("=" * 78)
    print("F1 closure: L(Sym^2 E, 1) for E: y^2 = x^3 - x (32.a3), arithmetic normalization")
    print(f"   dps = {mp.mp.dps}")
    print("=" * 78)
    results = []

    # (1) the CM form g and its link to a_p(E)
    nmax_b = 2000
    b = eta4_6_coefficients(nmax_b)
    primes = sieve_primes(nmax_b)
    ap = {p: (0 if p == 2 else ap_pointcount(p)) for p in primes}
    ok1 = b[2] == 0 and all(b[p] == (ap[p] ** 2 - 2 * p if p % 4 == 1 else 0) for p in primes if p > 2)
    print("\n(1) g = eta(4 tau)^6 and E")
    results.append(check(f"b(p) = a_p(E)^2 - 2p (p = 1 mod 4), 0 (p = 3 mod 4, p = 2), all primes < {nmax_b}", ok1,
                         f"b(1..6) = {b[1:7]}"))

    # (2) coefficient identity for the factorization
    c = sym2_coefficients(nmax_b, ap)
    rhs = [0] * (nmax_b + 1)
    for d in range(1, nmax_b + 1):
        cd = chi_m4(d) * d
        if cd == 0:
            continue
        for m in range(d, nmax_b + 1, d):
            rhs[m] += cd * b[m // d]
    ok2 = all(c[n] == rhs[n] for n in range(1, nmax_b + 1))
    print("\n(2) L(Sym^2 E, s) = L(g, s) L(chi_-4, s - 1), coefficient by coefficient")
    results.append(check(f"Sym^2 Euler-product coefficients = (b * chi_-4(d) d)(n) for all n <= {nmax_b}", ok2))

    # (3) root number +1
    nb = 200
    lam1_a, lam1_b = Lambda(1, mp.mpf(1), b, nb), Lambda(1, mp.mpf("1.3"), b, nb)
    print("\n(3) Functional equation Lambda(s) = Lambda(3 - s) (root number +1)")
    results.append(check("Lambda(1) split at y = 1 equals Lambda(1) split at y = 1.3",
                         abs(lam1_a - lam1_b) < TOL, f"diff {mp.nstr(abs(lam1_a - lam1_b), 3)}"))

    # (4) L(g, 1), L(g, 2), L(chi_-4, 0)
    Lg1 = lam1_a / (2 / PI)
    Lg2 = Lambda(2, mp.mpf(1), b, nb) / ((2 / PI) ** 2)
    Lchi0 = mp.dirichlet(0, [0, 1, 0, -1])
    Lchi1 = mp.dirichlet(1, [0, 1, 0, -1])
    print("\n(4) Values")
    print(f"     L(g, 1)        = {mp.nstr(Lg1, 30)}")
    print(f"     L(g, 2)        = {mp.nstr(Lg2, 30)}")
    results.append(check("L(chi_-4, 0) = 1/2", abs(Lchi0 - mp.mpf(1) / 2) < TOL))
    results.append(check("L(g, 1) = (2/pi) L(g, 2)", abs(Lg1 - 2 * Lg2 / PI) < TOL))

    # (5) the Sym^2 value and the master-quadratic trace
    LS2 = Lg1 * Lchi0
    print("\n(5) L(Sym^2 E, 1) reproduced (not substituted)")
    print(f"     L(Sym^2 E, 1)  = {mp.nstr(LS2, 30)}")
    results.append(check("L(Sym^2 E, 1) = varpi^2/(8 pi)", abs(LS2 - VARPI ** 2 / (8 * PI)) < TOL))
    results.append(check("L(Sym^2 E, 1) = G*^2/32 = Gamma(1/4)^4/(64 pi^2)",
                         abs(LS2 - GSTAR ** 2 / 32) < TOL and abs(LS2 - G14_4 / (64 * PI ** 2)) < TOL))
    results.append(check("512 L(Sym^2 E, 1) = 16 G*^2", abs(512 * LS2 - 16 * GSTAR ** 2) < 512 * TOL,
                         f"16 G*^2 = {mp.nstr(16 * GSTAR ** 2, 20)}"))
    results.append(check("L(g, 2) = Gamma(1/4)^4/(64 pi)  (Rogers-Wan-Zucker, Theorem 5)",
                         abs(Lg2 - G14_4 / (64 * PI)) < TOL))
    LE1 = VARPI / 4
    results.append(check("L(Sym^2 E, 1) = 2 L(E,1)^2/pi, with L(E,1) = varpi/4", abs(LS2 - 2 * LE1 ** 2 / PI) < TOL))

    # (6) analytic-normalization caveat
    LS2_at2 = Lg2 * Lchi1
    print("\n(6) Normalization caveat")
    print(f"     arithmetic value at s = 2: {mp.nstr(LS2_at2, 20)}  (the analytic 's = 1')")
    results.append(check("value at s = 2 = Gamma(1/4)^4/256 = (pi^2/4) L(Sym^2 E, 1)",
                         abs(LS2_at2 - G14_4 / 256) < TOL and abs(LS2_at2 - PI ** 2 / 4 * LS2) < TOL))

    # (7) optional PARI/GP cross-check
    print("\n(7) PARI/GP cross-check (optional)")
    try:
        import cypari2  # type: ignore
        pari = cypari2.Pari()
        pari.set_real_precision(60)
        val = pari("lfun(lfunsympow(ellinit([0,0,0,-1,0]), 2), 1)")
        pv = mp.mpf(str(val).replace(" E", "e"))
        results.append(check("PARI lfunsympow(E, 2) at s = 1 equals the value in (5)", abs(pv - LS2) < mp.mpf("1e-45"),
                             f"PARI {mp.nstr(pv, 20)}"))
    except ImportError:
        print("  SKIP  cypari2 not installed")

    ok = all(results)
    print("\n" + "=" * 78)
    print(f"VERDICT: {'ALL CHECKS PASS' if ok else 'CHECK FAILED'} ({sum(results)}/{len(results)})")
    print("  - L(Sym^2 E, 1) = varpi^2/(8 pi) = G*^2/32 is REPRODUCED from the Dirichlet series of")
    print("    eta(4 tau)^6 and chi_-4 (not substituted), so 16 G*^2 = 512 L(Sym^2 E, 1) joins")
    print("    16 G*^3 = 2^13 L(E,1)^3/pi^(3/2) as import-reproduced in-repo.")
    print("  - Exact value: CM factorization + root number +1 + Rogers-Wan-Zucker Theorem 5.")
    print("  - Normalization: arithmetic (s <-> 3 - s); s = 1 is critical, not central.")
    print("=" * 78)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
