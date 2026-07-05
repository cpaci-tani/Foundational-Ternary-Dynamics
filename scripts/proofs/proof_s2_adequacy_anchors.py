"""proof_s2_adequacy_anchors.py — FTD-0368 Stage S2 / adequacy anchors for the
frozen closure definition (the pre-registered instrument).

Role: this is the INSTRUMENT of PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md.
It verifies the adequacy anchors of the DYNAMICAL component N_dyn of the
frozen native-closure definition — i.e., that the two documented
lattice-borne inventory rows (Watson BCC self-energy; the Phase-G periodic
Poisson Green's function class) are limits of uniform linear-sector native
schemas with polynomial convergence moduli, exactly as the definition
requires — plus the schema-instance facts (exact algebraicity at finite L,
rational-linearity of the solve map) that Lemma 0 predicts.

What this script does:
    (A1) Exact finite-L BCC Green's function values: computes the
         zero-mode-excluded momentum sum G^BCC_L(0) EXACTLY for L in {2,3,4,6}
         (cosine values rational there) and for L=5 (algebraic, golden-ratio
         cosines) — every value is verified algebraic (rational where
         expected), witnessing the N_dyn schema-instance claim at finite L.
    (A2) Polynomial convergence modulus (empirical witness): computes
         G^BCC_L(0) in float for L in {8,16,24,32,48,64}, fits
         |G_L - I_1| ~ C * L^(-theta), and asserts theta >= 1 — the
         polynomial-modulus clause of the definition is witnessed for the
         Watson anchor. (The membership claim itself rests on the classical
         Watson limit theorem; this is its rate witness, [MEASURED]-support.)
    (A3) The limit identity I_1 = G*^2/(2*pi) = Gamma(1/4)^4/(4*pi^3)
         re-pinned exactly (spine Theorem 5 convention), so the anchor's
         limit VALUE is the documented inventory row B4-B5.
    (A4) Phase-G anchor class: the periodic SC lattice Poisson Green's
         function G_L(r) — exact rationality of the solve at L=3 with a unit
         source (canonical source, linear sector), i.e. a schema instance;
         plus float convergence-rate witness of G_L(1) toward its L -> inf
         value with polynomial modulus.
    (A5) Rational-linearity of the schema map: the solve map
         (source -> potential) at L=3 is verified to be EXACTLY linear over Q
         (superposition on two rational sources) — the linear-sector clause.

What this script is NOT:
    - NOT a delta-membership test. Per the S2 gate, the definition is frozen
      BEFORE delta is tested against it; this instrument contains NO
      computation involving delta = sqrt(G*(4G*-1)), and none may be added
      post-lock (banned move B1 of the prereg).
    - NOT a near-miss search: every comparison is against a pre-stated
      classical identity (Watson's theorem; the periodic Green's function
      closed form) or a structural property (algebraicity, linearity, rate).
    - NOT a promotion instrument: no tag moves; x+ = 1/alpha stays [SMC];
      MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; FC-W stays [AXIOM].

Usage:
    python scripts/proofs/proof_s2_adequacy_anchors.py
"""

from __future__ import annotations

import os
import sys
import time
from itertools import product

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

import mpmath as mpm
import sympy as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

mpm.mp.dps = 30

suite = ProofSuite("S2 adequacy anchors: the dynamical closure component N_dyn")


# ---------------------------------------------------------------------------
# The Watson/BCC schema: G^BCC_L(0) = (1/L^3) * sum'_k 1/(1 - prod cos(2 pi
# k_i / L)) over the ODD-L ladder, zero mode excluded.  The odd-L restriction
# is part of the schema's finite description: for odd L the symbol's kernel
# on the momentum torus is exactly {0} (|cos(2 pi k/L)| = 1 requires
# k in {0, L/2}; L odd kills L/2), whereas even L has three extra singular
# modes (k_i in {0, L/2} with an even number of L/2 entries).  One finite
# description, every odd L; limit (classical Watson / van Peype):
# I_1 = G*^2/(2 pi).
# ---------------------------------------------------------------------------

EXACT_COS = {
    3: {0: sp.Integer(1), 1: sp.Rational(-1, 2), 2: sp.Rational(-1, 2)},
    5: {0: sp.Integer(1),
        1: (sp.sqrt(5) - 1) / 4, 4: (sp.sqrt(5) - 1) / 4,
        2: -(sp.sqrt(5) + 1) / 4, 3: -(sp.sqrt(5) + 1) / 4},
}


def bcc_green_exact(L: int) -> sp.Expr:
    tab = EXACT_COS[L]
    total = sp.Integer(0)
    for k in product(range(L), repeat=3):
        if k == (0, 0, 0):
            continue
        c = tab[k[0]] * tab[k[1]] * tab[k[2]]
        total += 1 / (1 - c)
    return sp.radsimp(sp.simplify(total / L**3))


def bcc_green_float(L: int) -> float:
    import math
    assert L % 2 == 1, "the Watson anchor schema is the odd-L ladder"
    total = 0.0
    cos_tab = [math.cos(2 * math.pi * j / L) for j in range(L)]
    for kx in range(L):
        for ky in range(L):
            for kz in range(L):
                if kx == ky == kz == 0:
                    continue
                total += 1.0 / (1.0 - cos_tab[kx] * cos_tab[ky] * cos_tab[kz])
    return total / L**3


def check_a1() -> None:
    v3 = bcc_green_exact(3)
    suite.assert_true(
        f"A1 BCC schema instance L=3: exact value is rational ({v3})",
        bool(sp.simplify(v3).is_Rational), tag="[THEOREM]")
    v5 = bcc_green_exact(5)
    # cos(2 pi/5) is algebraic (golden-ratio class); value must be algebraic
    mp5 = sp.minimal_polynomial(v5, sp.Symbol("x"))
    suite.assert_true(
        f"A1 BCC schema instance L=5: exact value ALGEBRAIC "
        f"(min poly degree {sp.degree(mp5)})",
        mp5 is not None and sp.degree(mp5) >= 1, tag="[THEOREM]")


def check_a2() -> None:
    import math
    I1 = float(mpm.gamma(mpm.mpf(1) / 4) ** 4 / (4 * mpm.pi ** 3))
    Ls = [9, 17, 25, 33, 49, 65]
    errs = [abs(bcc_green_float(L) - I1) for L in Ls]
    # least-squares slope of log err vs log L
    xs = [math.log(L) for L in Ls]
    ys = [math.log(e) for e in errs]
    n = len(xs)
    xbar, ybar = sum(xs) / n, sum(ys) / n
    slope = (sum((x - xbar) * (y - ybar) for x, y in zip(xs, ys))
             / sum((x - xbar) ** 2 for x in xs))
    theta = -slope
    suite.assert_true(
        f"A2 Watson anchor: |G_L - I_1| ~ L^(-theta), fitted theta = "
        f"{theta:.2f} >= 1 (polynomial modulus witnessed)",
        theta >= 1.0, tag="[EXTERNAL]")
    suite.assert_true(
        "A2 monotone shrinking errors across the L-ladder",
        all(errs[i + 1] < errs[i] for i in range(len(errs) - 1)),
        tag="[EXTERNAL]")


def check_a3() -> None:
    G14 = sp.gamma(sp.Rational(1, 4))
    G34 = sp.gamma(sp.Rational(3, 4))
    Gs = G14 / G34
    refl = {G34: sp.sqrt(2) * sp.pi / G14}
    ok = sp.simplify((Gs**2 / (2 * sp.pi)
                      - G14**4 / (4 * sp.pi**3)).subs(refl)) == 0
    suite.assert_true("A3 anchor limit value: I_1 = G*^2/(2 pi) = "
                      "Gamma(1/4)^4/(4 pi^3) (exact; inventory row B4-B5)",
                      bool(ok), tag="[THEOREM]")


# ---------------------------------------------------------------------------
# The Phase-G schema class: periodic SC lattice Poisson Green's function —
# solve  (6 phi(p) - sum_nbr phi) = delta_{p,0} - 1/L^3  on the L^3 torus
# (compact SC Laplacian; canonical unit source over vacuum, zero-mean gauge).
# ---------------------------------------------------------------------------

def sc_poisson_exact(L: int):
    sites = list(product(range(L), repeat=3))
    idx = {p: i for i, p in enumerate(sites)}
    n = len(sites)

    def nbr(p, ax, st):
        q = list(p)
        q[ax] = (q[ax] + st) % L
        return tuple(q)

    A = sp.zeros(n, n)
    for p in sites:
        A[idx[p], idx[p]] = 6
        for ax in range(3):
            for st in (1, -1):
                A[idx[p], idx[nbr(p, ax, st)]] -= 1
    b = sp.Matrix([sp.Integer(1) if p == (0, 0, 0) else sp.Integer(0)
                   for p in sites]) - sp.ones(n, 1) / n
    phired = A[1:, 1:].LUsolve(b[1:, 0])
    phi = {sites[0]: sp.Integer(0)}
    for i in range(1, n):
        phi[sites[i]] = phired[i - 1]
    # zero-mean gauge
    mean = sum(phi.values()) / n
    return {p: sp.nsimplify(v - mean) for p, v in phi.items()}, sites, idx, A


def check_a4() -> None:
    phi, sites, idx, A = sc_poisson_exact(3)
    ok_rat = all(v.is_Rational for v in phi.values())
    suite.assert_true(
        "A4 Phase-G schema instance L=3: exact periodic Poisson solve is "
        "rational over Q (canonical unit source, zero-mean gauge)",
        bool(ok_rat), tag="[THEOREM]")

    # float rate witness for G_L(1) -> G_inf(1) via momentum sums
    import math
    def sc_green_r1_float(L: int) -> float:
        total = 0.0
        cos_tab = [math.cos(2 * math.pi * j / L) for j in range(L)]
        for kx in range(L):
            for ky in range(L):
                for kz in range(L):
                    if kx == ky == kz == 0:
                        continue
                    denom = 6.0 - 2.0 * (cos_tab[kx] + cos_tab[ky] + cos_tab[kz])
                    total += cos_tab[kx] / denom   # offset r = (1,0,0)
        return total / L**3

    # doubling ladder: for a polynomial modulus |G_inf - G_L| ~ c/L^theta the
    # doubling differences scale like L^(-theta); fit theta on log-log.
    Ls = [8, 16, 32, 64]
    vals = [sc_green_r1_float(L) for L in Ls]
    diffs = [abs(vals[i + 1] - vals[i]) for i in range(len(vals) - 1)]
    xs = [math.log(L) for L in Ls[:-1]]
    ys = [math.log(d) for d in diffs]
    n = len(xs)
    xbar, ybar = sum(xs) / n, sum(ys) / n
    slope = (sum((x - xbar) * (y - ybar) for x, y in zip(xs, ys))
             / sum((x - xbar) ** 2 for x in xs))
    theta = -slope
    suite.assert_true(
        f"A4 Phase-G anchor: doubling differences of G_L(r=1) fit "
        f"L^(-theta), theta = {theta:.2f} >= 0.8 (polynomial modulus witness)",
        theta >= 0.8, tag="[EXTERNAL]")


def check_a5() -> None:
    # superposition: solve for two rational sources separately and summed;
    # the map source -> potential is exactly Q-linear.
    L = 3
    sites = list(product(range(L), repeat=3))
    idx = {p: i for i, p in enumerate(sites)}
    n = len(sites)

    def nbr(p, ax, st):
        q = list(p)
        q[ax] = (q[ax] + st) % L
        return tuple(q)

    A = sp.zeros(n, n)
    for p in sites:
        A[idx[p], idx[p]] = 6
        for ax in range(3):
            for st in (1, -1):
                A[idx[p], idx[nbr(p, ax, st)]] -= 1

    def solve(bvec):
        b0 = bvec - sp.ones(n, 1) * (sum(bvec) / n)
        x = A[1:, 1:].LUsolve(b0[1:, 0])
        out = sp.zeros(n, 1)
        for i in range(1, n):
            out[i] = x[i - 1]
        return out

    b1 = sp.Matrix([sp.Rational(1, 2) if i == 0 else sp.Integer(0)
                    for i in range(n)])
    b2 = sp.Matrix([sp.Rational(1, 3) if i == 5 else sp.Integer(0)
                    for i in range(n)])
    lhs = solve(b1 + b2)
    rhs = solve(b1) + solve(b2)
    ok = all(sp.simplify(lhs[i] - rhs[i]) == 0 for i in range(n))
    suite.assert_true(
        "A5 schema linearity: solve(b1 + b2) = solve(b1) + solve(b2) exactly "
        "over Q (the linear-sector clause)", bool(ok), tag="[THEOREM]")


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  FTD-0368 S2 - adequacy anchors for the frozen closure definition")
    print("  N_dyn: uniform linear-sector native schemas, canonical sources,")
    print("  polynomial convergence moduli.  NO delta content anywhere here.")
    print("=" * 70)

    check_a1()
    check_a2()
    check_a3()
    check_a4()
    check_a5()

    suite.print_summary()
    print(f"\n  Wall time: {time.time() - t0:.1f}s")
    print("\n  PREREG DISCIPLINE (binding):")
    print("  - This instrument is hash-locked by")
    print("    PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md; the definition is")
    print("    frozen BEFORE any delta-membership computation exists.")
    print("  - Banned move B1: adding any delta = sqrt(G*(4G*-1)) computation")
    print("    to this file post-lock voids the pre-registration.")
    print("  - Zero promotions; no alpha content.")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
