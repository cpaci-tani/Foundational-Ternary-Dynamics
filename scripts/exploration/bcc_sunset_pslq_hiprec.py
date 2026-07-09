#!/usr/bin/env python3
"""
bcc_sunset_pslq_hiprec.py -- PSLQ the certified two-loop BCC sunset finite part B
(~230 digits, from bcc_sunset_connection_sage.py) against a PRINCIPLED lemniscatic
(Gamma(1/4)-centric) basis. The genus signature (y=1 exponents +-1/4 = j=1728) and
the log coefficient A_s=4/pi^2 motivate a Gamma(1/4)/pi + standard-2-loop basis.

Discipline: a small PRE-SPECIFIED nested sequence of bases (not a fishing sweep);
accept a relation only if it is LOW-HEIGHT and its residual sits at the ~10^-220
precision floor. A null / high-height outcome => B has no low-height closed form
in this basis (a marked boundary), consistent with a higher non-Gamma-quotient
period. Reports the relation, max|coeff|, and residual for each basis.
"""
from mpmath import (mp, mpf, mpmathify, pi, gamma, zeta, catalan, log, sqrt,
                    pslq, fabs)

mp.dps = 215

# load certified B
Bs = None
for line in open("scripts/exploration/_bcc_sunset_B.txt"):
    if not line.startswith("#") and line.strip():
        Bs = line.strip(); break
B = mpmathify(Bs)
print(f"B = {mp.nstr(B, 40)} ... ({mp.dps} dps)")

G14 = gamma(mpf(1) / 4)
W3 = G14 ** 4 / (4 * pi ** 3)          # one-loop BCC period = G*^2/2pi (lemniscatic)
A_s = 4 / pi ** 2                        # known log coefficient (cross-ref)

# named constants for the basis (all lemniscatic/standard, weight-graded)
C = {
    "1":            mpf(1),
    "1/pi^2":       1 / pi ** 2,
    "1/pi^4":       1 / pi ** 4,
    "W3":           W3,
    "W3^2":         W3 ** 2,
    "W3/pi":        W3 / pi,
    "W3/pi^2":      W3 / pi ** 2,
    "zeta3":        zeta(3),
    "zeta3/pi":     zeta(3) / pi,
    "zeta3/pi^2":   zeta(3) / pi ** 2,
    "log2/pi^2":    log(2) / pi ** 2,
    "Catalan/pi^2": catalan / pi ** 2,
    "1/pi^3":       1 / pi ** 3,
    # BCC-intrinsic sqrt(2) constants (arcsinh(1)=log(1+sqrt2))
    "sqrt2/pi^2":   sqrt(2) / pi ** 2,
    "L2/pi^2":      log(1 + sqrt(2)) / pi ** 2,
    "L2/pi^3":      log(1 + sqrt(2)) / pi ** 3,
    "sqrt2*L2/pi^2": sqrt(2) * log(1 + sqrt(2)) / pi ** 2,
}

# pre-specified nested bases (most-motivated first)
BASES = [
    ["1", "1/pi^2", "W3^2"],
    ["1", "1/pi^2", "W3", "W3^2"],
    ["1/pi^2", "W3^2", "zeta3", "1"],
    ["1/pi^2", "1/pi^4", "W3^2", "W3/pi", "zeta3/pi", "1"],
    ["1/pi^2", "1/pi^4", "W3^2", "W3/pi^2", "zeta3/pi^2", "log2/pi^2",
     "Catalan/pi^2", "1"],
    # BCC sqrt(2)-flavored rounds (motivated: BCC = sqrt(2) lattice)
    ["1/pi^2", "W3^2", "L2/pi^2", "sqrt2/pi^2", "1"],
    ["1/pi^2", "1/pi^4", "W3^2", "L2/pi^2", "L2/pi^3", "sqrt2*L2/pi^2", "1"],
]

tol = mpf(10) ** (-(mp.dps - 15))
print(f"\nPSLQ tol ~ 1e-{mp.dps-15}; accept only low-height + residual at floor\n")
for names in BASES:
    vec = [B] + [C[n] for n in names]
    rel = pslq(vec, tol=tol, maxcoeff=10 ** 25, maxsteps=10 ** 6)
    tag = "basis {" + ", ".join(names) + "}"
    if rel is None:
        print(f"  {tag}: no relation (maxcoeff 1e25)")
        continue
    # residual of the found relation
    resid = fabs(mpf(int(rel[0])) * B +
                 sum(mpf(int(rel[i + 1])) * C[names[i]] for i in range(len(names))))
    hmax = max(abs(int(x)) for x in rel)
    # express B = -(1/rel0) * sum rel_i c_i  if rel0 != 0
    print(f"  {tag}:")
    print(f"     relation coeffs [B, *basis] = {[int(x) for x in rel]}")
    print(f"     max|coeff| = {hmax}   residual = {mp.nstr(resid, 5)}")
    if rel[0] != 0 and hmax < 10 ** 6 and resid < tol * 100:
        c0 = mpf(int(rel[0]))
        terms = "  ".join(f"({-int(rel[i+1])}/{int(rel[0])})*{names[i]}"
                          for i in range(len(names)))
        print(f"     >>> B = {terms}   [LOW-HEIGHT CANDIDATE]")
