"""FTD-0566 -- The degeneracy criterion: equivariance no-selector theorem
and the split-fiber instances.

Verifies the exact content of DERIV_DEGENERACY_CRITERION.md:

  K1  Equivariance no-selector: if a structure group G acts freely and
      transitively on a fiber F and all selector inputs are G-invariant,
      every candidate selector fails equivariance -- exhaustively checked
      for G = Z/2 on 2 points and G = Sym(m), cyclic subgroup action,
      m = 3, 4 (every constant selector is moved by some group element).
  K2  The FTD-0549 schedule fiber is split, with the exact coefficient:
      s_eps(tau) = tau + eps*tau^2(1-tau)^2 matches s(tau) = tau at both
      endpoints, both endpoint velocities, AND the midpoint velocity, yet
      the first source moment differs by exactly eps/30:
      integral tau^2(1-tau)^2 dtau = 1/30 (exact rational arithmetic).
  K3  The position fiber is split: the FTD-0565 toy self-energy U(x) is
      non-constant across the subcell fiber (corrugation 1/64 at B2),
      so a variational selector exists there -- consistent with the
      supplied-section events (FTD-0551/0552) and in exact contrast to
      the protected fibers of K1.

Run:  python scripts/proofs/proof_degeneracy_criterion.py
"""

import itertools
import sys
from fractions import Fraction

PASS = []


def check(name, cond):
    PASS.append((name, bool(cond)))
    print(f"  [{'PASS' if cond else 'FAIL'}] {name}")


def k1_no_selector():
    ok = True
    # G-invariant inputs collapse to a single datum; a selector is then a
    # constant choice of fiber point. Equivariance s(g.d) = g.s(d) with
    # g.d = d forces s(d) to be a fixed point; free transitive actions
    # have none. Exhaustive witness:
    for m in (2, 3, 4):
        F = list(range(m))
        # cyclic (free, transitive) subgroup of Sym(m)
        for choice in F:  # every candidate constant selector
            moved = any(((choice + g) % m) != choice for g in range(1, m))
            ok &= moved
    check("K1 free transitive action: every invariant-input selector fails "
          "equivariance (m = 2, 3, 4, exhaustive)", ok)


def k2_schedule_split():
    # endpoint/velocity matching, exact
    def s(tau, eps):
        return tau + eps * tau**2 * (1 - tau)**2

    def sdot(tau, eps):
        return 1 + eps * (2 * tau * (1 - tau)**2 - 2 * tau**2 * (1 - tau))

    eps = Fraction(1, 7)
    ok = (s(Fraction(0), eps) == 0 and s(Fraction(1), eps) == 1
          and sdot(Fraction(0), eps) == 1 and sdot(Fraction(1), eps) == 1
          and sdot(Fraction(1, 2), eps) == 1)
    # exact first moment of the schedule difference: integral tau^2(1-tau)^2
    # = 1/3 - 2/4 + 1/5 = 1/30 (expand tau^2 - 2tau^3 + tau^4)
    moment = Fraction(1, 3) - Fraction(2, 4) + Fraction(1, 5)
    ok &= moment == Fraction(1, 30)
    check("K2 FTD-0549 fiber split: endpoints + both endpoint velocities + midpoint "
          "velocity all match, yet the source moment differs by exactly eps/30", ok)


def k3_position_split():
    # minimal standalone recomputation of the FTD-0565 landscape at 5 points
    import numpy as np

    def bspline2(t):
        a = abs(t)
        if a <= 0.5:
            return 0.75 - a * a
        if a <= 1.5:
            return 0.5 * (1.5 - a) ** 2
        return 0.0

    N = 32
    m = np.arange(1, N)
    lam = 2.0 - 2.0 * np.cos(2.0 * np.pi * m / N)
    j = np.arange(N)
    G = np.zeros((N, N))
    for d in range(N):
        G[j, (j + d) % N] = np.sum(np.cos(2.0 * np.pi * m * d / N) / lam) / N

    def U(x):
        rho = np.array([bspline2(((k - x + N / 2) % N) - N / 2) for k in range(N)])
        rho -= rho.sum() / N
        return 0.5 * rho @ G @ rho

    vals = [U(x) for x in (0.0, 0.25, 0.5)]
    corr = max(vals) - min(vals)
    ok = corr > 1e-3 and abs(corr - 1.0 / 64.0) < 1e-10
    print(f"    position-fiber corrugation = {corr:.10f} (1/64 = {1/64:.10f})")
    check("K3 position fiber split: U non-constant, corrugation = 1/64 to 1e-10 "
          "(variational selector exists; contrast with K1)", ok)


def main():
    print("FTD-0566 degeneracy-criterion verification")
    k1_no_selector()
    k2_schedule_split()
    k3_position_split()
    n_ok = sum(1 for _, ok in PASS if ok)
    print(f"\n{n_ok}/{len(PASS)} PASS")
    sys.exit(0 if n_ok == len(PASS) else 1)


if __name__ == "__main__":
    main()
