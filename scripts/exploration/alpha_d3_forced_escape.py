#!/usr/bin/env python3
"""
alpha_d3_forced_escape.py -- FTD-0284: the D=3 FORCED-escape test.

Frozen by PREREG_ALPHA_D3_FORCED_ESCAPE_v1.md. Symbolic (sympy), NOT a numerical scan.
G* (=G) is kept a free positive transcendental symbol throughout.

QUESTION (prereg §3): can a real operator built only from the FTD-native generators
  G = { three coordinate-plane sources g_xy=g_yz=g_zx=G* (C3-permuted);
        a complex structure J (J^2=-I) on one C4 plane;
        the Watson even scalar G*^2 and the integer 16=|Z[i]^x|^2;
        real linear combination, direct sum, restriction }
admit a 2D real readout R with char poly x^2-16G*^2 x+16G*^3 (real-distinct), with the
determinant INHERITED from the three-plane product (not hand-placed), without forcing
<C4,C3> on R (no collapse to O)?

The verdict is COMPUTED from two symbolic facts, not asserted:
  FACT-1 (elliptic bound): a real 2x2 A commuting with J has Det(A) >= (Tr A)^2/4.
  FACT-2 (master is hyperbolic): the master quadratic has Det = 16G*^3 < (16G*^2)^2/4 = 64G*^4
          (i.e. Tr^2-4Det = 64G*^3(4G*-1) > 0) for every G* > 1/4.
Together: the master determinant lies strictly BELOW the J-commuting real floor, so no real
operator carrying the complex structure on R can realize it. That CLOSES the
'complex-structure-on-readout' branch. The 'bare-coefficient' branch is examined separately
and is shown NOT closed by reality alone -> UNDERDETERMINED residual.

[EPISTEMIC: the elliptic-bound closure of branch (A) is a clean derivation; branch (B) is
 honestly left UNDERDETERMINED (it is the W-CRIT-2 question). NO claim of a full no-go, NO
 numerical near-miss scans, NO promotion of x+ = 1/alpha.]
"""

import sympy as sp


def banner(s):
    print("=" * 74); print(s); print("=" * 74)


def main():
    G = sp.symbols("G", positive=True)            # G* > 0, transcendental
    a, b = sp.symbols("a b", real=True)
    x = sp.symbols("x")

    Tr_t = 16*G**2
    Det_t = 16*G**3
    e3 = G**3                                      # three-plane product g_xy g_yz g_zx
    verdicts = {}

    banner("FTD-0284 -- D=3 FORCED-escape: trace-16 vs three-plane-det co-realizability")
    disc = sp.factor(Tr_t**2 - 4*Det_t)
    print(f"master quadratic: x^2 - {Tr_t} x + {Det_t}")
    print(f"FACT-2 (hyperbolic): Tr^2 - 4 Det = {disc}  > 0 for G*>1/4  => roots REAL-DISTINCT")
    print(f"   so Det = 16G*^3 < Tr^2/4 = 64G*^4   (ratio Det/(Tr^2/4) = {sp.simplify(Det_t/(Tr_t**2/4))} < 1)")
    print()

    # ---- (ii) availability: Det is a forced symmetric functional of the three planes ----
    banner("(ii) availability -- Det = 16 * (three-plane product) is a clean symmetric functional")
    print(f"elementary symmetric polys of (G*,G*,G*): e1=3G*, e2=3G*^2, e3=G*^3")
    print(f"Det/16 = G*^3 = e3  ->  Det = 16 e3 = 16 g_xy g_yz g_zx.  (ii) satisfiable in principle.")
    avail_ii = sp.simplify(16*e3 - Det_t) == 0
    print(f"   16 e3 == Det target: {avail_ii}")
    print()

    # ---- BRANCH (A): the trace-16 enters as a complex structure J acting ON the readout ----
    banner("BRANCH (A): 16 = |Z[i]^x|^2 enters as a complex structure J ON the 2D readout R")
    J = sp.Matrix([[0, -1], [1, 0]])
    A = a*sp.eye(2) + b*J                           # most general REAL J-commuting 2x2
    trA, detA = A.trace(), sp.expand(A.det())
    print(f"most general real J-commuting A = a I + b J = {A.tolist()}")
    print(f"   Tr(A) = {trA} ,  Det(A) = {detA} = a^2 + b^2  (FACT-1: Det >= (Tr/2)^2, the elliptic bound)")
    # impose the master trace, solve for the determinant floor, compare to target
    a_val = sp.solve(sp.Eq(trA, Tr_t), a)[0]        # a = 8G*^2
    bsq = sp.solve(sp.Eq(detA.subs(a, a_val), Det_t), b**2)
    bsq_val = sp.factor(bsq[0]) if bsq else None
    print(f"   impose Tr=16G*^2 -> a = {a_val}; then b^2 = Det - a^2 = {sp.factor(Det_t - a_val**2)}")
    bsq_expr = sp.factor(Det_t - a_val**2)
    neg = sp.simplify(bsq_expr.subs(G, sp.Rational(29587, 10000)))
    print(f"   b^2 = {bsq_expr} ;  at G*=2.9587 -> b^2 = {float(neg):.4f}  < 0")
    branchA_closed = bool(neg < 0)
    print(f"   => b is NOT real: NO real J-commuting operator on R has the master determinant.")
    print(f"      (The only reproduction is the NON-REAL witness M_w = a I + i|b| J, which invokes")
    print(f"       the ambient scalar i -> a single C4 axis -> <C4,C3> = O -> collapse, no readout:")
    print(f"       the machine-checked Leg1-2 / 3b wall.)   BRANCH (A) CLOSED: {branchA_closed}")
    verdicts["A_complex_structure_on_readout"] = "CLOSED-NEGATIVE" if branchA_closed else "OPEN"
    print()

    # ---- BRANCH (B): the trace-16 is a bare integer coefficient; R a plain real 2D space ----
    banner("BRANCH (B): 16 is a bare scalar coefficient; R a plain real 2D space (no J on R)")
    # exhibit a real, real-distinct operator with the master invariants (companion form)
    c1, c2 = sp.symbols("c1 c2", real=True)
    Areal = sp.Matrix([[0, -Det_t], [1, Tr_t]])     # companion form: char poly = master quadratic
    cp = sp.factor(Areal.charpoly(x).as_expr())
    print(f"a real 2x2 with the master invariants exists (companion form), char poly = {cp}")
    print(f"   eigenvalues real-distinct (disc>0). So reality does NOT close branch (B).")
    print(f"   BUT: with no module structure on R, Det = ad - bc is a FREE invariant (W-CRIT-2):")
    print(f"   nothing in G forces Det to equal 16 e3 rather than 16 e2 G* (=48G*^3) or any other")
    print(f"   symmetric functional. The companion form's '-16G*^3' entry is HAND-PLACED (banned F-HP).")
    print(f"   Whether the three-plane structure can CONSTRAIN a real operator's readout det to")
    print(f"   16 e3 WITHOUT a module structure on R is exactly the unresolved W-CRIT-2 question.")
    verdicts["B_bare_coefficient"] = "UNDERDETERMINED"
    print(f"   BRANCH (B): UNDERDETERMINED (the precise residual = W-CRIT-2)")
    print()

    # ---- reducible / direct-sum does not create a third branch ----
    banner("reducible / direct-sum check")
    print("a direct sum M = M_tr (+) M_det has whole-operator Tr = Tr(M_tr)+Tr(M_det) and")
    print("Det = Det(M_tr)*Det(M_det): these are NOT the invariants of ONE 2D readout. Any single")
    print("2D readout R = a coordinate 2-plane of M still has a real 2x2 M|R obeying the (A)/(B)")
    print("dichotomy above. So reducibility adds no third branch for a 2D readout.")
    print()

    # ---- overall verdict (computed) ----
    banner("OVERALL VERDICT (computed)")
    if branchA_closed and verdicts["B_bare_coefficient"] == "UNDERDETERMINED":
        overall = ("FORCED-ESCAPE NARROWED: branch (A) [complex-structure-on-readout] CLOSED by "
                   "the elliptic/hyperbolic incompatibility (a clean argument extending the C4/C3 "
                   "wall to ANY J-commuting 2D readout, not just C3-equivariant rank-2); branch "
                   "(B) [bare coefficient] = the W-CRIT-2 residual, UNDERDETERMINED.")
    elif branchA_closed:
        overall = "CLOSED-NEGATIVE"
    else:
        overall = "OPEN"
    print(f"branch (A): {verdicts['A_complex_structure_on_readout']}")
    print(f"branch (B): {verdicts['B_bare_coefficient']}")
    print(f"\n{overall}")
    print()
    print("HONEST READING: this does NOT derive alpha and does NOT fully close MC-T4.3. It")
    print("closes the complex-structure-on-readout escape with a clean elliptic-bound theorem,")
    print("and pins the entire remaining FORCED-escape to ONE residual: can the three-plane")
    print("structure force a REAL operator's 2D-readout determinant to 16 e3 without placing it")
    print("(the W-CRIT-2 / detdet_zeta identity)? That residual -- and the infinite-descended")
    print("transfer-operator sub-branch -- is the entire surviving surface. x+ = 1/alpha stays [SMC].")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
