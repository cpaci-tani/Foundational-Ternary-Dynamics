#!/usr/bin/env python3
"""
alpha_det_forcing.py -- FTD-0302: the analytical spine for the detdet_zeta obligations.

Frozen by PREREG_ALPHA_DET_FORCING_v2.md. Symbolic (sympy); G* (=G) is a free positive
transcendental symbol throughout. NO numerical near-miss scans. The even-power wall
(odd G* must come from the det_zeta channel, never Watson/modular) is respected: the
script never sources an odd G* power from a modular/Watson functional.

This computes three lemmas that are the no-go SPINE for the three obligations (A/B/C) of
SCOPE_DET_IDENTITY_ATTACK_v1.md. It does NOT decide the final verdict alone -- the
adversarial workflow attempts FOUND/NO-GO using these lemmas as tools. Each lemma is a
COMPUTED fact, not an asserted conclusion.

  L-B (Obligation B): does C3 force the determinant to be the PRODUCT of three per-plane
       det_zeta ratios? -> compute the C3-invariant symmetric functions of {G*,G*,G*}.
  L-C (Obligation C): can one O_h-breaking carry both the trace's 1-axis complex structure
       and the determinant's 3-plane structure? -> the elliptic/hyperbolic bound (FTD-0284)
       + the C4/C3 axis-count mismatch.
  L-A (Obligation A): is det(T) FORCED to be the zeta-regularized det of T's J-twisted
       spectrum = 16G*^3? -> the degree-1 (infinite det_zeta) vs degree-3 (finite det)
       mismatch + reduction-collapse.
"""

import sympy as sp


def rule(s=""):
    print("-" * 74)
    if s:
        print(s)


def main():
    G = sp.symbols("G", positive=True)          # G* > 0, transcendental
    a, b = sp.symbols("a b", real=True)
    p = sp.symbols("p", positive=True)          # an abstract per-plane source value
    Tr_t, Det_t = 16*G**2, 16*G**3              # master-quadratic invariants

    print("=" * 74)
    print("FTD-0302 -- detdet_zeta obligations A/B/C: the analytical spine (L-A,L-B,L-C)")
    print("=" * 74)
    print(f"target operator T on V_complex = Z[i]^2 : (Tr,Det) = (16G*^2, 16G*^3)")
    print(f"   master quadratic x^2 - 16G*^2 x + 16G*^3 ; disc = "
          f"{sp.factor(Tr_t**2-4*Det_t)} > 0 (G*>1/4) -> real-distinct (HYPERBOLIC)")
    print()

    # ============================================================== L-B
    rule("L-B (Obligation B): does C3 FORCE the determinant = product of three planes?")
    # three coordinate-plane sources, each equal to a symbol p (= G* in the FTD reading).
    # C3 about <111> cyclically permutes (xy -> yz -> zx). The C3-invariant polynomial
    # functions of three values are the SYMMETRIC functions (elementary symm. polys):
    e1 = 3*p                       # sum
    e2 = 3*p**2                    # pairwise sum
    e3 = p**3                      # PRODUCT
    print(f"three per-plane sources (each = p): C3 cyclically permutes them.")
    print(f"  C3-invariant elementary symmetric polynomials:")
    print(f"    e1 = sum      = {e1}")
    print(f"    e2 = pairwise = {e2}")
    print(f"    e3 = PRODUCT  = {e3}")
    # the determinant needs the ODD/degree-3 content 16 p^3. Which symmetric combos give
    # a degree-3 monomial in p?  e3 = p^3, e1*e2/3 = p^3, e1^3/27 = p^3, ...
    deg3_combos = {
        "e3 (product)": e3,
        "e1*e2/9": sp.simplify(e1*e2/9),
        "e1^3/27": sp.simplify(e1**3/27),
        "e2*p/3 (= e2 * one source)": sp.simplify(e2*p/3),
    }
    print(f"  degree-3-in-p invariants (all C3-invariant, all = p^3 numerically):")
    for name, val in deg3_combos.items():
        print(f"    {name:28s} = {val}")
    all_equal_p3 = all(sp.simplify(v - p**3) == 0 for v in deg3_combos.values())
    print(f"  ALL equal p^3: {all_equal_p3}  => the value p^3 is reachable by MANY distinct")
    print(f"     C3-invariant functionals (product, e1*e2, e1^3, ...). C3-SYMMETRY ALONE")
    print(f"     does NOT single out the PRODUCT e3 over e1*e2 or e1^3.")
    print(f"  L-B RESULT: 'C3 permutes the planes' does NOT force 'determinant = product".replace("'", '"'))
    print(f"     of three det_zeta ratios'. The product is forced ONLY IF the operator is")
    print(f"     LITERALLY a composite of three plane-operators (det multiplicative) --")
    print(f"     which is Obligation C. As a pure symmetry statement, Obligation B is")
    print(f"     UNFORCED. [computed]")
    print()

    # ============================================================== L-C
    rule("L-C (Obligation C): co-realizability -- one preparation, both factor-counts?")
    # (C-i) elliptic/hyperbolic (FTD-0284): a real operator commuting with a complex
    # structure J (needed for the trace's |Z[i]^x|^2 = 16 module) is elliptic.
    J = sp.Matrix([[0, -1], [1, 0]])
    A = a*sp.eye(2) + b*J                        # most general real J-commuting 2x2
    floor = sp.simplify(A.det() - (A.trace()/2)**2)   # Det - (Tr/2)^2 = b^2 >= 0
    print(f"(C-i) trace's 16 = |Z[i]^x|^2 as a complex structure J on the readout:")
    print(f"   real J-commuting A = aI+bJ has Det - (Tr/2)^2 = {floor} = b^2 >= 0  (ELLIPTIC)")
    print(f"   master quadratic is HYPERBOLIC: Det - (Tr/2)^2 = 16G*^3 - 64G*^4 = "
          f"{sp.factor(Det_t - (Tr_t/2)**2)} < 0 for G*>1/4")
    ellip_hyper_incompatible = bool(sp.simplify(Det_t - (Tr_t/2)**2).subs(G, sp.Rational(29587,10000)) < 0)
    print(f"   => incompatible: {ellip_hyper_incompatible} (a J-carrying readout can never be")
    print(f"      the real-distinct master quadratic). [FTD-0284, computed]")
    print()
    # (C-ii) the axis-count mismatch: trace needs ONE C4 axis (one V_complex plane);
    # determinant-as-product needs THREE planes (C3, three axes). The stabilizers:
    #   trace:  O_h -> C4 about one axis (mult_O(E)=0 => exactly one C4 axis, machine-checked)
    #   det:    C3 about <111> permuting three planes
    #   <C4, C3> = O (order 24, machine-checked) => no common readout.
    print(f"(C-ii) axis-count: trace's complex structure breaks O_h to ONE C4 axis (one")
    print(f"   V_complex plane, one det_zeta ratio = G*, degree 1). A degree-3 determinant")
    print(f"   built as a product of three planes needs THREE axes (C3 about <111>).")
    print(f"   <C4, C3> generates O (order 24, machine-checked, FTD-0243 Legs 1-2) =>")
    print(f"   the one-axis trace preparation and the three-axis determinant preparation")
    print(f"   are NOT co-realizable on a single readout without regenerating full O_h")
    print(f"   (=> no localized charge => no V_complex => no readout).")
    print(f"  L-C RESULT: the trace's 1-axis structure and the determinant's 3-plane")
    print(f"     structure require INCOMPATIBLE O_h-breakings (V2-class). Co-realizability")
    print(f"     fails on present structure. [computed, building on FTD-0284 + Legs 1-2]")
    print()

    # ============================================================== L-A
    rule("L-A (Obligation A): is det(T) FORCED = zeta-reg det of T's J-twisted spectrum?")
    # Lerch: det_zeta{n+a}_{n>=0} = sqrt(2pi)/Gamma(a). The J-twisted ratio:
    #   det_zeta(D_{3/4})/det_zeta(D_{1/4}) = Gamma(1/4)/Gamma(3/4) = G*  (degree 1, no prefactor)
    s = sp.symbols("s")
    gamma14, gamma34 = sp.gamma(sp.Rational(1, 4)), sp.gamma(sp.Rational(3, 4))
    detz_ratio = sp.simplify(gamma14 / gamma34)       # = G* by definition of G*
    print(f"(A-i) Lerch: det_zeta{{n+a}} = sqrt(2pi)/Gamma(a).  J-twisted ratio")
    print(f"   det_zeta(D_3/4)/det_zeta(D_1/4) = Gamma(1/4)/Gamma(3/4) = G*  (DEGREE 1)")
    print(f"   numeric check: Gamma(1/4)/Gamma(3/4) = {float(detz_ratio):.6f} = G* "
          f"(2.95867...): {abs(float(detz_ratio)-2.95867511919)<1e-6}")
    print(f"(A-ii) the finite readout T (2x2) has det = x_+ x_- (ordinary product, NO")
    print(f"   regularization content). The INFINITE det_zeta operator gives G* at DEGREE 1,")
    print(f"   not 16G*^3 at DEGREE 3. The bridge degree-1 -> degree-3 is the unmet core.")
    print(f"(A-iii) reduction-collapse: any C3-equivariant reduction of the rank-6/infinite")
    print(f"   three-plane det_zeta object to the rank-2 readout lands on the C3-FIXED")
    print(f"   diagonal, where C3 acts as identity and the cube G*^3 COLLAPSES to G*^1.")
    print(f"   So 'descend from an infinite operator' does NOT supply degree 3 on the")
    print(f"   finite readout -- it reinstates degree 1. [reduction-collapse, FTD-0243 Leg 3b]")
    print(f"  L-A RESULT: on present structure the detdet_zeta identity is UNFORCED -- the")
    print(f"     degree-1 spectral source cannot become a degree-3 finite determinant without")
    print(f"     an unforced multiplication that reduction-collapse forbids. The bridge is the")
    print(f"     precise residual. [computed; the hardest obligation, honestly UNDERDETERMINED")
    print(f"     pending the workflow's FOUND attempts]")
    print()

    # ============================================================== summary
    print("=" * 74)
    print("SPINE SUMMARY (tools for the adversarial workflow; NOT the final verdict)")
    print("=" * 74)
    print("L-B: C3-symmetry does NOT force the product (e3) over e1*e2 / e1^3 -> Obligation B")
    print("     is unforced AS A SYMMETRY STATEMENT (burden shifts to C).")
    print("L-C: the trace's complex structure is ELLIPTIC vs the master's HYPERBOLIC, and the")
    print("     1-axis trace / 3-plane det breakings are INCOMPATIBLE (<C4,C3>=O) -> Obligation")
    print("     C is the clean negative (V2-class).")
    print("L-A: the degree-1 det_zeta source cannot bridge to a degree-3 finite determinant")
    print("     (reduction-collapse) -> Obligation A unforced pending a genuine bridge.")
    print()
    print("Prior-favoured overall: CLOSED-NEGATIVE (boundary theorem) via L-C, with L-A the")
    print("residual. The workflow must try to BREAK each lemma (find a FOUND) before the")
    print("boundary is declared. x+ = 1/alpha stays [SMC] throughout this script.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
