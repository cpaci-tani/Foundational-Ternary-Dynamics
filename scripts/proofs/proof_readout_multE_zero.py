#!/usr/bin/env python3
"""
proof_readout_multE_zero.py
===========================

Trace-leg of the Readout-Structure Independence theorem (MC-T4.3 boundary).

CLAIM (machine-checked here, exact integer/character arithmetic):

  The permutation module of the cubic rotation group O (= the 8 BCC/cube-corner
  sites of the Moore neighbourhood) decomposes as

        Q[8 corners]  ≅  A1 ⊕ A2 ⊕ T1 ⊕ T2          (dims 1+1+3+3 = 8)

  and the 2-dimensional irrep E has  mult_O(E) = 0.

CONSEQUENCE (the reason this matters; see
docs/theory/10_eft_program/preregistrations/PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md):

  A complex structure J (J^2 = -I) needs an even-dimensional invariant block.
  Since mult_O(E) = 0, there is NO O-symmetric 2-dimensional subspace on the
  corner module, so no O-invariant complex structure exists.  A definite "i" for
  the readout operator on V_complex ≅ Z[i]^2 therefore REQUIRES breaking O to a
  single C4 axis (FTD-0231); that selection removes the C3(<111>) rotation from
  the preparation's stabilizer.  Combined with the group core
  <C4(<001>), C3(<111>)> = O (also checked here), this is the trace-side leg of
  the boundary theorem: "single definite i" (C3 not in Stab) and "three symmetric
  planes" (C3 in Stab) cannot be co-realized from one preparation.

This script proves the ALGEBRA only (a character computation). It does not by
itself prove MC-T4.3 is unforced; see the pre-registration for the full obligation
(the determinant-side Leg 3 + the independence half remain open).

No external dependencies (pure-Python integer arithmetic).
Run:  python scripts/proofs/proof_readout_multE_zero.py
"""

import itertools
import sys


def mul(A, B):
    """3x3 integer matrix product."""
    return tuple(
        tuple(sum(A[r][k] * B[k][c] for k in range(3)) for c in range(3))
        for r in range(3)
    )


def trace(g):
    return g[0][0] + g[1][1] + g[2][2]


def applyv(g, v):
    return tuple(sum(g[r][c] * v[c] for c in range(3)) for r in range(3))


def col(g, i):
    return (g[0][i], g[1][i], g[2][i])


def neg(v):
    return tuple(-x for x in v)


def perm_sign(p):
    s = 1
    for i in range(len(p)):
        for j in range(i + 1, len(p)):
            if p[i] > p[j]:
                s = -s
    return s


def generate_group(gens):
    """Close a set of 3x3 integer matrices under multiplication (BFS)."""
    I = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
    G = {I}
    frontier = [I]
    while frontier:
        g = frontier.pop()
        for s in gens:
            h = mul(g, s)
            if h not in G:
                G.add(h)
                frontier.append(h)
    return list(G)


def main():
    print("=== proof_readout_multE_zero.py : trace-leg of MC-T4.3 boundary ===")
    checks = []

    # Generators: C4 about z (90 deg), C3 about the body diagonal (1,1,1).
    C4z = ((0, -1, 0), (1, 0, 0), (0, 0, 1))      # ex->ey, ey->-ex, ez->ez
    C3 = ((0, 0, 1), (1, 0, 0), (0, 1, 0))        # ex->ey->ez->ex (rotation about <111>)

    # --- Leg 2: the group core  <C4(<001>), C3(<111>)> = O  (order 24) ---
    G = generate_group([C4z, C3])
    order = len(G)
    print(f"\n[Leg 2] |<C4(z), C3(111)>| = {order}  (expected 24 = |O| = |S4|)")
    checks.append(("group core <C4,C3> = O (order 24)", order == 24))

    # --- Build the needed class functions on G, by elementary geometry ---
    verts = list(itertools.product((1, -1), repeat=3))   # 8 cube corners
    e = [(1, 0, 0), (0, 1, 0), (0, 0, 1)]
    D = [(1, 1, 1), (1, 1, -1), (1, -1, 1), (-1, 1, 1)]   # 4 body diagonals (as lines)

    chi8 = {}   # permutation character on the 8 corners  = # fixed vertices
    chiE = {}   # 2-dim irrep E:  (# fixed coordinate axes) - 1   [E = perm-on-3-axes - triv]
    chiT1 = {}  # standard 3-dim rep T1 = trace(g)
    chiA2 = {}  # sign rep A2 = sign of g's permutation on the 4 body diagonals
    for g in G:
        chi8[g] = sum(1 for v in verts if applyv(g, v) == v)
        fixed_axes = sum(1 for i in range(3) if col(g, i) == e[i] or col(g, i) == neg(e[i]))
        chiE[g] = fixed_axes - 1
        chiT1[g] = trace(g)
        p = []
        for d in D:
            gd = applyv(g, d)
            for j in range(4):
                if gd == D[j] or gd == neg(D[j]):
                    p.append(j)
                    break
        chiA2[g] = perm_sign(p)

    chiA1 = {g: 1 for g in G}
    chiT2 = {g: chiT1[g] * chiA2[g] for g in G}   # T2 = T1 (x) A2

    def mult(chi):
        # <chi8, chi> = (1/|G|) sum_g chi8(g) chi(g)   (real characters)
        total = sum(chi8[g] * chi[g] for g in G)
        assert total % order == 0, "non-integer multiplicity -> a character is wrong"
        return total // order

    mA1, mA2, mE, mT1, mT2 = (mult(chiA1), mult(chiA2), mult(chiE),
                              mult(chiT1), mult(chiT2))
    n_constituents = sum(chi8[g] ** 2 for g in G) // order

    print("\n[Leg 1] decomposition of Q[8 corners] under O:")
    print(f"        mult A1 = {mA1}")
    print(f"        mult A2 = {mA2}")
    print(f"        mult E  = {mE}    <-- the load-bearing claim (expected 0)")
    print(f"        mult T1 = {mT1}")
    print(f"        mult T2 = {mT2}")
    print(f"        #irreducible constituents (sum chi8^2 / |G|) = {n_constituents}")
    print(f"        dim check 1*{mA1}+1*{mA2}+2*{mE}+3*{mT1}+3*{mT2} = "
          f"{mA1 + mA2 + 2 * mE + 3 * mT1 + 3 * mT2}  (module is 8-dim)")

    checks.append(("mult_O(E) = 0 (no O-symmetric 2-dim subspace)", mE == 0))
    checks.append(("decomposition A1+A2+T1+T2", (mA1, mA2, mT1, mT2) == (1, 1, 1, 1)))
    checks.append(("dim sums to 8", mA1 + mA2 + 2 * mE + 3 * mT1 + 3 * mT2 == 8))
    checks.append(("4 irreducible constituents", n_constituents == 4))

    # --- The symmetric average of the three plane-i's is NOT a complex structure ---
    # J_a = (P_a - P_a^3)/2 is the imaginary-unit generator about axis a; on its
    # support J_sym = (J_x+J_y+J_z)/3 squares to -I/3, not -I.  We verify the scalar
    # factor 1/3 structurally via the rep: a genuine J needs J^2 = -I (order 4),
    # available only after breaking to one axis.  (Numeric check kept simple.)
    # C4 about x and about y, to form J_x, J_y, J_z:
    C4x = ((1, 0, 0), (0, 0, -1), (0, 1, 0))
    C4y = ((0, 0, 1), (0, 1, 0), (-1, 0, 0))
    def Jmat(C4):  # (C4 - C4^3)/2  -> the rotation generator (integer*2 then /2)
        C4_3 = mul(mul(C4, C4), C4)
        return tuple(tuple((C4[r][c] - C4_3[r][c]) / 2 for c in range(3)) for r in range(3))
    Jx, Jy, Jz = Jmat(C4x), Jmat(C4y), Jmat(C4z)
    Jsym = tuple(tuple((Jx[r][c] + Jy[r][c] + Jz[r][c]) / 3 for c in range(3)) for r in range(3))
    Jsym2 = mul(Jsym, Jsym)
    # The C3-symmetric average IS the generator of rotation about <111>: it
    # ANNIHILATES the <111> axis (J_sym . (1,1,1) = 0), hence is singular and
    # cannot satisfy J^2 = -I.  (On the plane perpendicular to <111> it does act
    # as -I/3, i.e. J_sym^2 = -I/3 + Ones/9.)  Either way: not a complex structure.
    axis = (1.0, 1.0, 1.0)
    J_axis = applyv(Jsym, axis)
    singular_on_axis = all(abs(c) < 1e-12 for c in J_axis)
    perp = (1.0, -1.0, 0.0)                       # a vector perpendicular to <111>
    JJ_perp = applyv(Jsym2, perp)
    minus_third_on_perp = all(abs(JJ_perp[i] - (-1.0 / 3.0) * perp[i]) < 1e-12 for i in range(3))
    not_complex_structure = singular_on_axis and minus_third_on_perp
    print(f"\n[Leg 1 cor.] symmetric (J_x+J_y+J_z)/3 annihilates the <111> axis "
          f"(singular): {singular_on_axis}")
    print(f"            and acts as -I/3 on the perpendicular plane: {minus_third_on_perp}")
    print("            => NOT a complex structure (J^2 != -I); a definite i needs one broken axis.")
    checks.append(("symmetric average is singular on <111> -> not a complex structure",
                   not_complex_structure))

    # --- report ---
    print("\n=== RESULTS ===")
    all_pass = True
    for name, ok in checks:
        print(f"  [{'PASS' if ok else 'FAIL'}] {name}")
        all_pass = all_pass and ok
    print("=" * 60)
    if all_pass:
        print("ALL CHECKS PASS — trace-leg established: mult_O(E)=0, so the readout's")
        print("definite complex structure forces a single C4 axis (C3 not in Stab).")
        return 0
    print("FAILURE — a check did not pass; do NOT cite this leg as established.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
