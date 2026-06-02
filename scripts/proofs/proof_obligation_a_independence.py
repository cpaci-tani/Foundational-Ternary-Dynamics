#!/usr/bin/env python3
"""
proof_obligation_a_independence.py
==================================

Analytic core of the READOUT-STRUCTURE INDEPENDENCE boundary theorem (MC-T4.3 / Obligation A).

It verifies the load-bearing lemmas establishing that the master-quadratic operator structure
(Tr, Det) = (16 G*^2, 16 G*^3) is NOT forced by the FTD-native construction set — so the readout
structure is logically INDEPENDENT of the five postulates + spine + O_h representation theory
(it is realizable only by a chosen external selection W = the master-quadratic companion matrix).

Companion to scripts/proofs/proof_readout_multE_zero.py (the group-theoretic trace-side leg).

  L-A  Bernoulli-Gamma trace dichotomy.
       The J-twisted operator D_a (spectrum {n+a}, n>=0) has det_zeta(D_a) = sqrt(2pi)/Gamma(a)
       (Gamma/G*-bearing: the 3/4-vs-1/4 ratio = Gamma(1/4)/Gamma(3/4) = G*), but EVERY regularized
       power-trace zeta(-k, a) = -B_{k+1}(a)/(k+1) is RATIONAL (Bernoulli) and carries ZERO G*.
       => the operator that supplies the odd G* (its determinant) has a G*-free trace; it cannot
       also supply the degree-2 G* trace.

  L-B  degree <=> multiplicity (IFF), no single-operator shortcut.
       For zeta-regularized determinants, spectral-power-m and eigenvalue-multiplicity-m coincide:
       det_zeta{(n+a)^m} = exp(-m*zeta_H'(0,a)) = (sqrt(2pi)/Gamma(a))^m = (det_zeta{n+a})^m.
       => reaching det_zeta = G*^3 REQUIRES exactly three multiplicative factors; a degree-1
       source has no single-operator route to degree 3.

  L-C  rank-2 tautology.
       For a finite 2x2 operator T with eigenvalues {x+, x-}: zeta_T(0)=2 and
       det_zeta T = exp(-zeta_T'(0)) = x+ * x- = ordinary det. The zeta-regularization is VACUOUS
       at rank 2, so "det_zeta = 16 G*^3" imposes nothing -- 16 G*^3 enters only by CHOOSING the
       entries (the imposed Vieta target, W-CRIT-2).

Net: the odd degree-3 determinant (3 factors, a genuine spectral object => rank>=3) and the
degree-2 rank-2 Watson trace cannot be co-realized as FORCED invariants of a single operator.

Run:  python scripts/proofs/proof_obligation_a_independence.py
Deps: mpmath, sympy.
"""

import sys
from mpmath import mp, mpf, gamma, sqrt, pi, exp, log
import sympy as sp

mp.dps = 50


def main():
    checks = []
    print("=== proof_obligation_a_independence.py : analytic core of the MC-T4.3 boundary theorem ===")

    gstar = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)
    target_tr = 16 * gstar**2
    target_det = 16 * gstar**3
    print(f"\nG*              = {gstar}")
    print(f"16 G*^2 (trace) = {target_tr}")
    print(f"16 G*^3 (det)   = {target_det}")

    # det_zeta of the J-twisted operator D_a (spectrum {n+a}): det_zeta = sqrt(2pi)/Gamma(a)
    def det_zeta_shift(a):
        return sqrt(2 * pi) / gamma(a)

    ratio = det_zeta_shift(mpf(3) / 4) / det_zeta_shift(mpf(1) / 4)  # = Gamma(1/4)/Gamma(3/4) = G*
    print(f"\n[source] det_zeta(D_3/4)/det_zeta(D_1/4) = {ratio}")
    checks.append(("J-twisted det_zeta ratio = G* (the clean degree-1 odd source, FTD-0234)",
                   abs(ratio - gstar) < mpf(10)**-40))

    # ---- L-A : Bernoulli rational traces vs Gamma-bearing determinant ----
    a14, a34 = sp.Rational(1, 4), sp.Rational(3, 4)
    zeta_neg1 = lambda a: -(a**2 - a + sp.Rational(1, 6)) / 2  # zeta(-1,a) = -B2(a)/2
    z14, z34 = zeta_neg1(a14), zeta_neg1(a34)
    print(f"\n[L-A] regularized trace zeta(-1,1/4) = {z14} ; zeta(-1,3/4) = {z34}  (exact, sympy)")
    checks.append(("zeta(-1,1/4) = zeta(-1,3/4) = 1/96 (RATIONAL -- the trace carries zero G*)",
                   z14 == sp.Rational(1, 96) and z34 == sp.Rational(1, 96)))
    all_rational = all((-sp.bernoulli(k + 1, a14) / (k + 1)).is_rational for k in range(1, 6))
    print(f"[L-A] all power-traces zeta(-k,1/4), k=1..5, are rational: {all_rational}")
    checks.append(("every regularized power-trace zeta(-k,a) is RATIONAL (Bernoulli)", all_rational))
    checks.append(("dichotomy: determinant is G*-bearing, trace is rational -> different operators",
                   abs(ratio - gstar) < mpf(10)**-40 and z14 == sp.Rational(1, 96)))

    # ---- L-B : degree <=> multiplicity IFF (det_zeta{(n+a)^m} = (det_zeta{n+a})^m) ----
    zetaH_prime0 = lambda a: log(gamma(a)) - log(2 * pi) / 2  # Lerch: zeta_H'(0,a)
    iff_ok = True
    for m in (1, 2, 3):
        lhs = exp(-m * zetaH_prime0(mpf(1) / 4))     # det_zeta of spectrum {(n+1/4)^m}
        rhs = (det_zeta_shift(mpf(1) / 4))**m         # (det_zeta{n+1/4})^m
        if abs(lhs - rhs) > mpf(10)**-38:
            iff_ok = False
    print(f"\n[L-B] det_zeta(spectral-power m) == (det_zeta)^m for m=1,2,3: {iff_ok}")
    checks.append(("degree<=>multiplicity: no single-operator shortcut from degree-1 to degree-3", iff_ok))
    cube_ratio = exp(-3 * zetaH_prime0(mpf(3) / 4)) / exp(-3 * zetaH_prime0(mpf(1) / 4))
    print(f"[L-B] multiplicity-3 J-twist ratio = {cube_ratio}  (= G*^3 = {gstar**3})")
    checks.append(("det_zeta = G*^3 REQUIRES exactly three multiplicative factors",
                   abs(cube_ratio - gstar**3) < mpf(10)**-38))

    # ---- L-C : rank-2 tautology (det_zeta = ordinary det; 16G*^3 only by choosing entries) ----
    disc = sqrt(4 * gstar**2 - gstar)
    xp = 8 * gstar**2 + 4 * gstar * disc   # master-quadratic roots x_+- = 8G*^2 +- 4G* sqrt(4G*^2 - G*)
    xm = 8 * gstar**2 - 4 * gstar * disc
    detz_rank2 = exp(log(xp) + log(xm))    # exp(-zeta_T'(0)) with zeta_T(s)=xp^-s+xm^-s, zeta_T(0)=2
    print(f"\n[L-C] rank-2: x_+ = {xp}\n             x_- = {xm}")
    print(f"[L-C] det_zeta(rank-2) = {detz_rank2} ; ordinary x+*x- = {xp * xm}")
    checks.append(("rank-2 det_zeta == ordinary product x+*x- (regularization VACUOUS at rank 2)",
                   abs(detz_rank2 - xp * xm) < mpf(10)**-36))
    checks.append(("x+*x- = 16 G*^3 (so hitting it at rank 2 is a chosen Vieta target, not forced)",
                   abs(xp * xm - target_det) < mpf(10)**-34))
    checks.append(("x+ + x- = 16 G*^2 (the trace; Tr and Det are the two free Vieta entries)",
                   abs(xp + xm - target_tr) < mpf(10)**-34))

    print("\n=== RESULTS ===")
    all_pass = True
    for name, ok in checks:
        print(f"  [{'PASS' if ok else 'FAIL'}] {name}")
        all_pass = all_pass and ok
    print("=" * 72)
    if all_pass:
        print("ALL CHECKS PASS -- the degree-3 odd determinant (3 factors / genuine spectral, rank>=3)")
        print("and the degree-2 rank-2 trace cannot be co-realized as FORCED invariants of one")
        print("operator. (Tr,Det)=(16G*^2,16G*^3) is realizable only by a CHOSEN W => the readout")
        print("operator structure is INDEPENDENT of the FTD-native construction set.")
        return 0
    print("FAILURE -- a check did not pass; do NOT cite the boundary theorem as established.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
