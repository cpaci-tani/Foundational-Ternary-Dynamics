"""proof_reflection_flow_parity.py — FTD-0367 / first-order flow parity of the
reflection product and ratio.

Claim (FTD-0367, [THEOREM — classical identities assembled] + reading tagged
separately in the doc):
    The two branches of the Euler reflection formula satisfy first-order
    linear ODEs whose coefficients split by parity, by differential
    algebraicity, and by value-class at the lemniscatic point:

        P(z) = Gamma(z)*Gamma(1-z):   P' = c_P(z)*P,  c_P = psi(z) - psi(1-z)
                                               = -pi*cot(pi*z)
        R(z) = Gamma(z)/Gamma(1-z):   R' = c_R(z)*R,  c_R = psi(z) + psi(1-z)

    (i)   Parity: the product's coefficient is the ODD digamma combination
          (elementary, by the differentiated reflection formula); the ratio's
          is the EVEN combination (non-elementary).
    (ii)  Differential algebraicity: c_P satisfies its own autonomous
          first-order algebraic ODE  c_P' = pi^2 + c_P^2  (Riccati form);
          c_R = 2*psi(z) - c_P is HYPERTRANSCENDENTAL — it satisfies no
          algebraic differential equation, by Holder's theorem (1887: Gamma
          satisfies no ADE) plus the closure of differentially algebraic
          functions under field operations, exp, and antidifferentiation.
    (iii) Value class at z = 1/4:  c_P(1/4) = -pi  (pi-world);
          c_R(1/4) = psi(1/4) + psi(3/4) = -2*(EulerGamma + 3*ln 2)
          (the gamma/log boundary class of the FTD-0127 L'(s, chi_{-4})
          identities).  R(1/4) = G*, P(1/4) = sqrt(2)*pi.

What this script does:
    (F1) Verifies the reflection formula and its differentiated form
         symbolically for symbolic z (gammasimp), plus 50-digit numeric spot
         checks at non-special points.
    (F2) Verifies c_R = psi(z) + psi(1-z) by direct symbolic differentiation
         of log R, plus numerics.
    (F3) Verifies the lemniscatic-point values exactly (sympy Gauss-digamma
         evaluation) and numerically (dps=50), including a finite-difference
         cross-check of (log R)'(1/4).
    (F4) Verifies the autonomous ADE  c_P' = pi^2 + c_P^2  symbolically.
    (F5) Verifies the reduction identities  c_R + c_P = 2*psi(z)  and
         c_R - c_P = 2*psi(1-z)  symbolically — the algebraic step that,
         combined with Holder's theorem (cited, not machine-checkable here),
         yields the hypertranscendence of c_R.
    (F6) Pins the branch values R(1/4) = G* and P(1/4) = sqrt(2)*pi.

What this script is NOT:
    - NOT a new theorem of FTD's: every identity here is classical
      (differentiated Euler reflection; Gauss's digamma theorem; Holder
      1887). The assembly and the frontier reading are the FTD contribution,
      tagged in the companion doc at [coherent-interpretation].
    - NOT an alpha route. The hypertranscendence of the ratio branch's flow
      coefficient is, if anything, the ODE-level face of the standing wall:
      no promotion, no derivation of any physical constant. x+ = 1/alpha
      stays [SMC] (FTD-0013); MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION].

Usage:
    python scripts/proofs/proof_reflection_flow_parity.py
"""

from __future__ import annotations

import os
import sys
import time

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

import mpmath as mpm
import sympy as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite, G_STAR as G_STAR_FLOAT64  # noqa: E402

mpm.mp.dps = 50
TOL = mpm.mpf(10) ** (-40)

z = sp.Symbol("z")
P = sp.gamma(z) * sp.gamma(1 - z)
R = sp.gamma(z) / sp.gamma(1 - z)
cP = sp.digamma(z) - sp.digamma(1 - z)
cR = sp.digamma(z) + sp.digamma(1 - z)

suite = ProofSuite("Reflection flow parity (product vs ratio first-order ODEs)")


def check_f1() -> None:
    # Reflection formula, symbolic z (gammasimp performs the reflection):
    refl = sp.simplify(sp.gammasimp(P) - sp.pi / sp.sin(sp.pi * z))
    suite.assert_true("F1 reflection: Gamma(z)Gamma(1-z) = pi/sin(pi z) (symbolic)",
                      refl == 0, tag="[THEOREM]")
    # Differentiated form: (log P)' = cP and also = -pi*cot(pi*z).
    lhs = sp.simplify(sp.diff(sp.log(P), z) - cP)
    suite.assert_true("F1 (log P)' = psi(z) - psi(1-z) (symbolic)",
                      lhs == 0, tag="[THEOREM]")
    target = sp.diff(sp.log(sp.pi / sp.sin(sp.pi * z)), z)  # = -pi*cot(pi*z)
    suite.assert_true("F1 target: d/dz log(pi/sin(pi z)) = -pi*cot(pi z)",
                      sp.simplify(target + sp.pi * sp.cot(sp.pi * z)) == 0,
                      tag="[THEOREM]")
    # Numeric spot checks of cP = -pi*cot(pi*z) at non-special points.
    for zz in ("0.31", "0.171", "0.44"):
        zv = mpm.mpf(zz)
        got = mpm.digamma(zv) - mpm.digamma(1 - zv)
        want = -mpm.pi * mpm.cot(mpm.pi * zv)
        suite.assert_true(f"F1 numeric c_P({zz}) = -pi*cot(pi z) (dps=50)",
                          mpm.fabs(got - want) < TOL, tag="[THEOREM]")


def check_f2() -> None:
    lhs = sp.simplify(sp.diff(sp.log(R), z) - cR)
    suite.assert_true("F2 (log R)' = psi(z) + psi(1-z) (symbolic)",
                      lhs == 0, tag="[THEOREM]")
    for zz in ("0.31", "0.171"):
        zv = mpm.mpf(zz)
        got = mpm.diff(lambda t: mpm.log(mpm.gamma(t) / mpm.gamma(1 - t)), zv)
        want = mpm.digamma(zv) + mpm.digamma(1 - zv)
        suite.assert_true(f"F2 numeric (log R)'({zz}) = psi+psi (dps=50)",
                          mpm.fabs(got - want) < TOL, tag="[THEOREM]")


def check_f3() -> None:
    # Exact values at the lemniscatic point z = 1/4 (Gauss digamma theorem).
    cP14 = sp.simplify(cP.subs(z, sp.Rational(1, 4)))
    suite.assert_true("F3 c_P(1/4) = -pi (exact)",
                      sp.simplify(cP14 + sp.pi) == 0, tag="[THEOREM]")
    cR14 = sp.simplify(cR.subs(z, sp.Rational(1, 4)))
    want = -2 * (sp.EulerGamma + 3 * sp.log(2))
    suite.assert_true("F3 c_R(1/4) = psi(1/4)+psi(3/4) = -2(gamma + 3 ln 2) (exact)",
                      sp.simplify(cR14 - want) == 0, tag="[THEOREM]")
    # Numeric + finite-difference cross-check of the ratio slope at 1/4.
    num = mpm.digamma(mpm.mpf(1) / 4) + mpm.digamma(mpm.mpf(3) / 4)
    ref = -2 * (mpm.euler + 3 * mpm.log(2))
    fd = mpm.diff(lambda t: mpm.log(mpm.gamma(t) / mpm.gamma(1 - t)),
                  mpm.mpf(1) / 4)
    ok = mpm.fabs(num - ref) < TOL and mpm.fabs(fd - ref) < TOL
    suite.assert_true("F3 numeric slope (log R)'(1/4) = -5.3133... (two routes, dps=50)",
                      bool(ok), tag="[THEOREM]")


def check_f4() -> None:
    # Autonomous algebraic ODE for the product coefficient:
    #   c_P = -pi*cot(pi*z)  satisfies  c_P' = pi^2 + c_P^2.
    c = -sp.pi * sp.cot(sp.pi * z)
    ade = sp.simplify(sp.diff(c, z) - (sp.pi**2 + c**2))
    suite.assert_true("F4 c_P' = pi^2 + c_P^2 (autonomous ADE, symbolic)",
                      ade == 0, tag="[THEOREM]")


def check_f5() -> None:
    # Reduction identities behind the hypertranscendence argument:
    #   c_R + c_P = 2 psi(z),   c_R - c_P = 2 psi(1-z).
    # DA functions are closed under field ops, exp, and antidifferentiation;
    # if c_R were DA then psi = (c_R + c_P)/2 would be DA (c_P is DA by F4),
    # hence log Gamma and Gamma would be DA — contradicting Holder (1887).
    # Holder's theorem is cited classical mathematics, not machine-checked.
    ok1 = sp.simplify(cR + cP - 2 * sp.digamma(z)) == 0
    ok2 = sp.simplify(cR - cP - 2 * sp.digamma(1 - z)) == 0
    suite.assert_true("F5 c_R + c_P = 2 psi(z) and c_R - c_P = 2 psi(1-z) (symbolic)",
                      ok1 and ok2, tag="[THEOREM]")


def check_f6() -> None:
    # P(1/4) = sqrt(2)*pi symbolically (reflection at z = 1/4):
    p14 = sp.simplify(sp.gammasimp(P.subs(z, sp.Rational(1, 4)))
                      - sp.sqrt(2) * sp.pi)
    suite.assert_true("F6 P(1/4) = sqrt(2)*pi (symbolic reflection)",
                      p14 == 0, tag="[THEOREM]")
    # R(1/4) = Gamma(1/4)/Gamma(3/4) is definitional; pin its value against
    # the corpus's canonical G_STAR (float64 sanity, matching the FTD-0366
    # verifier's C8 convention).
    g_num = mpm.gamma(mpm.mpf(1) / 4) / mpm.gamma(mpm.mpf(3) / 4)
    suite.assert_true("F6 R(1/4) numeric agrees with common.py G_STAR (1e-12)",
                      abs(float(g_num) - G_STAR_FLOAT64) < 1e-12,
                      tag="[THEOREM]")


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  FTD-0367 - Reflection flow parity: P' = c_P P vs R' = c_R R")
    print("  c_P = psi(z)-psi(1-z) = -pi cot(pi z)   [odd, elementary, DA]")
    print("  c_R = psi(z)+psi(1-z)                   [even, hypertranscendental]")
    print("=" * 70)

    check_f1()
    check_f2()
    check_f3()
    check_f4()
    check_f5()
    check_f6()

    suite.print_summary()
    print(f"\n  Wall time: {time.time() - t0:.1f}s")
    print("\n  STANDING INVARIANTS (unchanged):")
    print("  - Every identity above is classical (differentiated reflection,")
    print("    Gauss digamma, Holder 1887); the assembly/reading is FTD's,")
    print("    tagged [coherent-interpretation] in the companion doc.")
    print("  - No alpha content; x+ = 1/alpha stays [SMC]; MC-T4.3 stays")
    print("    [FOUNDATIONAL OBSTRUCTION]. The hypertranscendence of c_R is")
    print("    the ODE-level face of the wall, not a route through it.")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
