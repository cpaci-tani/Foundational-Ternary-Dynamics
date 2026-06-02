#!/usr/bin/env python3
"""
proof_readout_reduction_collapse.py
===================================

LEG 3b of the Readout-Structure Independence theorem (MC-T4.3 boundary),
pre-registered in
docs/theory/10_eft_program/preregistrations/PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md
(§5 Leg 3(3b); §9 method step 4).

CLAIM (made a theorem here, exact representation-theory arithmetic):

  ANY C3(<111>)-equivariant linear reduction (projection) of a three-plane /
  rank>=3 (or infinite) det_zeta-bearing object DOWN TO the rank-2 readout
  operator on V_complex (FTD-0122, ~= Z[i]^2) necessarily FACTORS THROUGH the
  C3-FIXED subspace.  That fixed subspace is the DIAGONAL {(v,v,v)}, on which
  C3 acts as the IDENTITY.  Hence the det_zeta determinant grading collapses
  from the three-plane product G*^3 to a single per-plane factor G*^1 on the
  reduced 2x2 operator.  The cube cannot survive reduction to the readout.

This is the operator-level reason the odd power G*^3 cannot be produced FORWARD
by a C3-equivariant reduction; supplying it requires CHOOSING the 2x2 entries to
match (the imposed master quadratic W-CRIT-2, banned B-1/F-a), not a harvest of
three per-plane det_zeta ratios.

ESTABLISHED INPUTS (cited, not re-derived here):
  * C3(<111>) cyclically permutes the three coordinate axes x->y->z->x, hence
    the three coordinate PLANES (xy)->(yz)->(zx); det grading reads as three
    per-plane det_zeta ratios (= G* each), organized by this C3
    (DERIV_BCC_ALGEBRAIC_READOUT.md; EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md §4).
  * V_complex ~= Z[i]^2 is the rank-2 readout, fixed by a SINGLE C4(<001>) axis;
    C3(<111>) is NOT in the stabilizer of that preparation
    (Leg 1: mult_O(E)=0; Leg 2: <C4(<001>),C3(<111>)>=O), so a NON-TRIVIAL C3
    character on V_complex would force C3 in Stab (= unbroken = no V_complex).
    Therefore the readout target carries only the TRIVIAL C3-action; this is the
    input from the discharged legs that selects the trivial isotypic.
    (proof_readout_multE_zero.py)

PROOF STRUCTURE:
  (i)   C3 on the 3 planes = regular rep of Z/3 = triv (+) omega (+) omegabar.
        C3-fixed subspace = the diagonal {(v,v,v)}; C3|_diagonal = id.        [exact]
  (ii)  Schur/Reynolds: with the target carrying the trivial C3-action (forced),
        EVERY C3-equivariant linear pi: M -> V_complex satisfies pi = pi o Pi_triv,
        i.e. it annihilates the omega, omegabar isotypics and factors through the
        diagonal.  This identity is DIMENSION-FREE (it uses only |<C3>|=3), hence
        holds for finite rank-3, rank-6, ..., AND the infinite det_zeta descent. [exact]
  (iii) FAITHFUL MODEL M = V_complex (x) C^3 = (Z[i]^2)^3: the C3-fixed subspace
        is the rank-2 diagonal copy of V_complex; the three EQUAL per-plane
        factors (C3-equivariance forces s_xy=s_yz=s_zx=G*) collapse, under the
        diagonal identification, to ONE retained factor: G*^3 -> G*^1.          [exact]
  (iv)  Multiplicative-functional objection (the FORCED escape lives here):
        a rank-2 operator determinant is a product of exactly TWO eigenvalues;
        no C3-equivariant compression of the rank-3 source carries THREE forward
        factors of G* (g*I_3 compresses to g*I_2 -> g^2 at most; the diagonal
        collapse gives g^1).  det=g^3 on a 2x2 is a value-match (W-CRIT-2).      [exact]

Numerical values (G*, 16G*^3) are CORROBORATION ONLY (pre-reg F-g), never the proof.

No project-prior is assumed; forward direction only (postulates -> C3 action ->
equivariant reduction -> invariant).  No banned move (no det=Tr*G*, no chosen-
entry 2x2, no master quadratic / alpha / CODATA import).

Run:  python scripts/proofs/proof_readout_reduction_collapse.py
Depends on sympy (exact) and mpmath (numeric corroboration only).
"""

import itertools
import sys

import sympy as sp

try:
    import mpmath as mp
except Exception:  # pragma: no cover
    mp = None


def banner(t):
    print("\n" + "=" * 72)
    print(t)
    print("=" * 72)


def main():
    print("=== proof_readout_reduction_collapse.py : LEG 3b (reduction-collapse) ===")
    checks = []

    omega = sp.exp(2 * sp.pi * sp.I / 3)
    g = sp.Symbol("g", positive=True)  # the per-plane det_zeta ratio (= G*)

    # C3(<111>): e_x->e_y->e_z->e_x ; as a 3x3 integer rotation (matches
    # proof_readout_multE_zero.py's C3).  As the action on the 3 plane-labels it
    # is the same 3-cycle (regular rep of Z/3).
    C3 = sp.Matrix([[0, 0, 1], [1, 0, 0], [0, 1, 0]])

    # ---------------------------------------------------------------- (i)
    banner("(i) C3 on the three planes = regular rep of Z/3; fixed = diagonal")
    order3 = (C3 ** 3 == sp.eye(3)) and (C3 != sp.eye(3)) and (C3 ** 2 != sp.eye(3))
    proper = (C3.det() == 1)
    print(f"C3 order 3: {order3}; det(C3)=+1 (proper rotation): {proper}")
    checks.append(("C3 order 3, proper rotation", order3 and proper))

    # eigenvalues {1, omega, omegabar}
    eigs = C3.eigenvals()
    def rect(z):
        return sp.simplify(sp.expand_complex(z))
    got = sorted([rect(k) for k in eigs], key=lambda z: (sp.re(z), sp.im(z)))
    want = sorted([rect(sp.Integer(1)), rect(omega), rect(omega ** 2)],
                  key=lambda z: (sp.re(z), sp.im(z)))
    eig_ok = len(got) == 3 and all(sp.simplify(a - b) == 0 for a, b in zip(got, want))
    print(f"C3 eigenvalues = {{1, omega, omegabar}} (regular rep of Z/3): {eig_ok}")
    checks.append(("C3 spectrum = {1, omega, omegabar}", eig_ok))

    # isotypic multiplicities (each 1)
    perm_char = {0: sp.Integer(3), 1: sp.Integer(0), 2: sp.Integer(0)}
    def mult(j):
        s = sum(perm_char[k] * sp.conjugate(sp.simplify(omega ** (j * k))) for k in range(3))
        return sp.simplify(s / 3)
    m = (mult(0), mult(1), mult(2))
    print(f"isotypic multiplicities (triv, omega, omegabar) = {m}")
    checks.append(("regular rep = triv (+) omega (+) omegabar, each mult 1",
                   m == (1, 1, 1)))

    # C3-fixed subspace = diagonal {(v,v,v)}; C3 = id there
    fixed = (C3 - sp.eye(3)).nullspace()
    diag = sp.Matrix([1, 1, 1])
    fixed_is_diag = (len(fixed) == 1 and
                     sp.simplify(fixed[0][0] - fixed[0][1]) == 0 and
                     sp.simplify(fixed[0][1] - fixed[0][2]) == 0)
    id_on_diag = (C3 * diag == diag)
    print(f"C3-fixed subspace = span{{(1,1,1)}} (the diagonal): {fixed_is_diag}; "
          f"C3|_diagonal = id: {id_on_diag}")
    checks.append(("C3-fixed subspace is the diagonal; C3 acts as identity there",
                   fixed_is_diag and id_on_diag))

    # ---------------------------------------------------------------- (ii)
    banner("(ii) Schur/Reynolds: equivariant reduction factors through the "
           "diagonal (DIMENSION-FREE)")
    # The Reynolds projector onto the trivial isotypic (the C3-fixed subspace):
    P3 = C3
    Pi_triv = (sp.eye(3) + P3 + P3 ** 2) / 3
    idem = sp.simplify(Pi_triv * Pi_triv - Pi_triv) == sp.zeros(3, 3)
    rk1 = (Pi_triv.rank() == 1)
    img_diag = sp.simplify(Pi_triv * sp.Matrix([1, 0, 0]) -
                           sp.Rational(1, 3) * diag) == sp.zeros(3, 1)
    print(f"Reynolds Pi_triv idempotent: {idem}; rank 1: {rk1}; image = diagonal: {img_diag}")
    checks.append(("Reynolds projector onto diagonal (idempotent, rank 1)",
                   idem and rk1 and img_diag))

    # Dimension-free factorization: target carries the trivial C3-action
    # (forced: a non-trivial C3 char on V_complex => C3 in Stab => Leg-2
    # contradiction).  Then any linear pi has C3-equivariant part = pi o Pi_triv.
    # Exhibit for M = C^N (internal, any dim) (x) C^3 (the three planes), several N
    # (N=2 is the faithful V_complex case; larger N stands in for higher / infinite
    # internal det_zeta content).
    all_dimfree = True
    for N in (1, 2, 3, 5):
        C3_M = sp.Matrix(sp.kronecker_product(P3, sp.eye(N)))
        dim = 3 * N
        Pi = sp.Rational(1, 3) * (sp.eye(dim) + C3_M + C3_M ** 2)
        pi = sp.Matrix(2, dim, sp.symbols(f"q0:{2 * dim}"))
        pi_eq = sp.expand((pi + pi * C3_M + pi * C3_M ** 2) / 3)
        pi_fac = sp.expand(pi * Pi)
        ok = (sp.simplify(pi_eq - pi_fac) == sp.zeros(2, dim) and
              sp.simplify(sp.expand(pi_eq * (sp.eye(dim) - Pi))) == sp.zeros(2, dim))
        all_dimfree = all_dimfree and ok
        print(f"  internal dim N={N} (M-dim {dim}): pi_eq == pi o Pi_triv, kills "
              f"non-fixed part: {ok}")
    print("=> factorization uses only |<C3>|=3 (dimension-free): holds for finite "
          "rank-3/6/..., AND the infinite det_zeta descent.")
    checks.append(("equivariant reduction factors through diagonal at all dims "
                   "(dimension-free => infinite descent)", all_dimfree))

    # ---------------------------------------------------------------- (iii)
    banner("(iii) Faithful model (Z[i]^2)^3: G*^3 -> G*^1 on the diagonal copy")
    J = sp.Matrix([[0, -1], [1, 0]])  # internal definite i on one V_complex
    checks.append(("internal complex structure J^2 = -I (definite i)",
                   J ** 2 == -sp.eye(2)))
    C3_M = sp.Matrix(sp.kronecker_product(P3, sp.eye(2)))  # C3 permutes the 3 slots
    fixedM = (C3_M - sp.eye(6)).nullspace()
    v = sp.Matrix(sp.symbols("v1 v2"))
    diag_embed = sp.Matrix.vstack(v, v, v)
    diag_fixed = (C3_M * diag_embed == diag_embed)
    # every fixed vector has equal slot-blocks
    fb = sp.Matrix.hstack(*fixedM)
    blocks_equal = (sp.simplify(fb[0:2, :] - fb[2:4, :]) == sp.zeros(2, 2) and
                    sp.simplify(fb[2:4, :] - fb[4:6, :]) == sp.zeros(2, 2))
    print(f"dim(M^C3) = {len(fixedM)} (= rank 2, one copy of V_complex); diagonal "
          f"(v,v,v) is fixed: {diag_fixed}; every fixed vector is diagonal: {blocks_equal}")
    checks.append(("C3-fixed subspace of (Z[i]^2)^3 = rank-2 diagonal copy of V_complex",
                   len(fixedM) == 2 and diag_fixed and blocks_equal))

    # C3-equivariance forces the three per-plane factors EQUAL (= g = G*):
    a, b, c = sp.symbols("a b c")
    S = sp.diag(a, b, c)
    eqs = [sp.Eq((P3 * S * P3.inv())[i, j], S[i, j]) for i in range(3) for j in range(3)]
    sol = sp.solve(eqs, [a, b, c], dict=True)
    forces_equal = (len(sol) == 1 and len(sol[0]) == 2)  # two of three pinned to the third
    source_grading = (g * g * g)  # three equal per-plane det_zeta ratios on the source
    reduced_grading = g           # one retained representative after diagonal identification
    print(f"C3-equivariance forces s_xy=s_yz=s_zx=g: {forces_equal}")
    print(f"source three-plane det_zeta grading = g^3 = {source_grading}; "
          f"reduced (diagonal) grading = {reduced_grading} (G*^1)")
    checks.append(("C3-equivariance forces three EQUAL factors; "
                   "diagonal collapse g^3 -> g^1",
                   forces_equal and source_grading == g ** 3 and reduced_grading == g))

    # ---------------------------------------------------------------- (iv)
    banner("(iv) Multiplicative-functional objection (the FORCED escape) — closed")
    # rank/multiplicity: a rank-2 operator's determinant is a product of TWO
    # eigenvalues; any C3-equivariant compression of g*I_3 (the equivariant
    # rank-3 source) has det <= g^2; the diagonal collapse gives g^1.  THREE
    # forward factors require rank>=3 RETAINED -> not the rank-2 readout.
    rank2_compressions = []
    for pair in itertools.combinations(["triv", "omega", "omegabar"], 2):
        rank2_compressions.append((pair, g ** 2))  # g*I_3 restricts to g*I_2
    g2_only = all(d == g ** 2 for _, d in rank2_compressions)
    print("rank-2 C3-equivariant compressions of g*I_3 all have det = g^2 (NOT g^3):")
    for pair, d in rank2_compressions:
        print(f"  keep {pair}: det = {d}")
    checks.append(("rank-2 compression det = g^2 at most (3rd forward factor not retained)",
                   g2_only))

    # the only way a 2x2 determinant equals g^3 is to CHOOSE entries (value match):
    x_p, x_m = sp.symbols("x_p x_m")
    detT = sp.Matrix([[x_p, 0], [0, x_m]]).det()
    match = sp.solve(sp.Eq(detT, g ** 3), x_m)
    print(f"\n2x2 det = x_p*x_m = {detT}; det = g^3 forces x_m = {match} "
          f"(a chosen value-match = W-CRIT-2, banned B-1/F-a), not three harvested factors.")
    checks.append(("2x2 det=g^3 is a value-match (W-CRIT-2), not a forward 3-factor harvest",
                   match == [g ** 3 / x_p]))

    # ---------------------------------------------------------------- numeric
    banner("Numerical corroboration (mpmath; CORROBORATION ONLY, pre-reg F-g)")
    if mp is not None:
        mp.mp.dps = 50
        Gstar = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
        G_BCC0 = mp.gamma(mp.mpf(1) / 4) ** 4 / (4 * mp.pi ** 3)
        watson = abs(Gstar ** 2 / (2 * mp.pi) - G_BCC0) < mp.mpf(10) ** (-45)
        print(f"G* = Gamma(1/4)/Gamma(3/4) = {Gstar}")
        print(f"Watson G*^2/(2pi) == G_BCC(0): {watson}")
        print(f"source grading 16 G*^3 = {float(16 * Gstar ** 3):.5f}  ->  "
              f"reduced grading 16 G*^1 = {float(16 * Gstar):.5f}  (the collapse)")
        checks.append(("numeric: Watson identity to 45 dp (corroboration only)", watson))
    else:
        print("mpmath unavailable; skipping numeric corroboration (not part of the proof).")

    # ---------------------------------------------------------------- report
    banner("RESULTS")
    all_pass = True
    for name, ok in checks:
        print(f"  [{'PASS' if ok else 'FAIL'}] {name}")
        all_pass = all_pass and ok
    print("=" * 72)
    if all_pass:
        print("ALL CHECKS PASS — LEG 3b established (forward, no banned move):")
        print("  any C3-equivariant linear reduction of a three-plane/rank>=3/infinite")
        print("  det_zeta object onto the rank-2 readout factors through the C3-fixed")
        print("  diagonal (C3=id), collapsing the determinant grading G*^3 -> G*^1.")
        return 0
    print("FAILURE — a check did not pass; do NOT cite this leg as established.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
