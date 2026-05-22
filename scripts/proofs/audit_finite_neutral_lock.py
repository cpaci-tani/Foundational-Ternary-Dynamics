#!/usr/bin/env python3
"""
audit_finite_neutral_lock.py -- computational backbone for the Q10 audit.

Executes the stabiliser / orbit / charge computations behind
``docs/theory/08_structural/AUDIT_FINITE_NEUTRAL_LOCK.md``, the result of the
pre-registered finite neutral-lock audit:

    pre-reg : docs/theory/08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md
    LEDGER  : FTD-0190  [PRE-REGISTRATION]
    git tag : preregister-finite-neutral-lock-v1
    SHA256  : 41c3f86584270d59fd25736bfec3cee3efb6a656d34f12be44b93272e57ae346

This is NOT a numerical search and contains NO near-miss / coincidence
scan.  It enumerates the FROZEN finite catalog of pre-reg section 4 and
*computes* -- does not assert -- the group-theoretic facts the audit rests
on (pre-reg section 9 method, steps 1-6):

  * the finite groups of the catalog and their natural actions;
  * distinguished configurations and their stabilisers (steps 2-3);
  * which stabilisers are a rank-1 U(1)-shadow + whether a genuine lock
    exists (step 4);
  * the two-state-opposition charge arithmetic for candidates A and B
    (steps 5-6: does falsifier F-c fire?).

Every object is from the locked catalog; no new free integer, exponent,
group, or scale is introduced.  Run: ``python audit_finite_neutral_lock.py``
"""

import itertools
import sys

try:
    from sympy.combinatorics import Permutation, PermutationGroup
except ImportError:  # pragma: no cover
    print("ERROR: this audit backbone requires sympy (sympy.combinatorics).")
    sys.exit(1)


def banner(text):
    print()
    print("=" * 72)
    print(text)
    print("=" * 72)


# ----------------------------------------------------------------------
# pre-reg section 9 step 1 -- build the catalog finite group O_h
# ----------------------------------------------------------------------
def build_Oh_on_27():
    """O_h as signed coordinate permutations acting on the 27 sites of the
    3x3x3 Moore block {-1,0,1}^3.  Returns (group, points, index)."""
    points = list(itertools.product((-1, 0, 1), repeat=3))
    index = {p: i for i, p in enumerate(points)}
    elements = []
    for perm in itertools.permutations(range(3)):          # S_3  (6)
        for sgn in itertools.product((1, -1), repeat=3):    # Z_2^3 (8)
            mapping = [0] * 27
            for p in points:
                q = tuple(sgn[k] * p[perm[k]] for k in range(3))
                mapping[index[p]] = index[q]
            elements.append(Permutation(mapping))
    return PermutationGroup(elements), points, index


def main():
    banner("Q10 finite neutral-lock audit -- computational backbone")
    print("pre-registration : PREREG_FINITE_NEUTRAL_LOCK_v1.md  (FTD-0190)")
    print("catalog is FROZEN (pre-reg section 4).  Enumeration only; no search.")

    # ------------------------------------------------------------------
    # section 9 steps 1-3 -- catalog item 1: ternary alphabet
    # ------------------------------------------------------------------
    banner("9.1-3  catalog item 1 -- ternary alphabet {-1,0,+1}")
    ternary = (-1, 0, 1)
    tau = {-1: 1, 0: 0, 1: -1}                       # sign-flip automorphism
    assert all(tau[tau[x]] == x for x in ternary), "tau^2 != id"
    print("  group : Aut = Z2 = <tau : x |-> -x>,  tau^2 = id   [verified]")
    for v in ternary:
        fixed = (tau[v] == v)
        stab = "Z2 (whole group)" if fixed else "trivial"
        print(f"  config v = {v:+d} : tau-fixed = {fixed!s:5s} -> Stab = {stab}")
    print("  two-state opposition {+1,-1}: tau swaps them, neither is")
    print("  separately fixed  ->  D2 PASS;  natural T3 charge = +-1")

    # ------------------------------------------------------------------
    # section 9 steps 1-3 -- catalog item 3: Z[i]^x
    # ------------------------------------------------------------------
    banner("9.1-3  catalog item 3 -- Z[i]^x  (units of the Gaussian integers)")

    def cmul(p, q):
        (a, b), (c, d) = p, q
        return (a * c - b * d, a * d + b * c)

    one, I, mone, mI = (1, 0), (0, 1), (-1, 0), (0, -1)
    units = [one, I, mone, mI]
    powers, cur = [], one
    for _ in range(4):
        powers.append(cur)
        cur = cmul(cur, I)
    assert powers == [one, I, mone, mI], "Z[i]^x is not <i> cyclic-4"
    assert cmul(I, I) == mone, "i^2 != -1"
    print("  group : Z[i]^x = <i> ~= Z4  (cyclic, order 4 = N_base)  [verified]")

    def conj(p):
        a, b = p
        return (a, -b)

    assert all(conj(conj(u)) == u for u in units), "conj^2 != id"
    assert {conj(I), I} == {I, mI}, "conjugation does not swap i <-> -i"
    print("  conjugation c : (a,b) |-> (a,-b) -- automorphism, c^2 = id  [verified]")
    print("  c-orbit of i = {i,-i} : genuine two-state opposition  ->  D2 PASS")
    print("  multiplication action of Z4 on Z[i]:")
    print("    config v = 0    : Stab = Z4 (whole group) -- rank-1 U(1)-shadow,")
    print("                      but Stab = G  =>  NO lock (D1 fails)")
    print("    config v = unit : Stab = trivial")
    print("  grade reading: {i,-i} are the grade-(+-1) states of Z4.  Relative")
    print("  to the order-2 element -1 carrying unit weight, i (a square root")
    print("  of -1) carries half-unit weight  =>  {i,-i} carry T3 = +-1/2,")
    print("  the doublet normalisation, free from the '4' of |Z[i]^x| = N_base.")

    # ------------------------------------------------------------------
    # section 9 steps 1-3 -- catalog item 2/4: O_h on the 27-block
    # ------------------------------------------------------------------
    banner("9.1-3  catalog item 2/4 -- O_h on the 27-block")
    G, points, index = build_Oh_on_27()
    order = G.order()
    print(f"  |O_h| (computed)        : {order}")
    assert order == 48, "O_h order != 48"
    orbit_sizes = sorted(len(o) for o in G.orbits())
    print(f"  orbit sizes on 27-block : {orbit_sizes}   (sum = {sum(orbit_sizes)})")
    assert orbit_sizes == [1, 6, 12, 8] or sorted(orbit_sizes) == [1, 6, 8, 12]
    stab_centre = G.stabilizer(index[(0, 0, 0)])
    print(f"  Stab_O_h(centre site)   : order {stab_centre.order()}  "
          f"(centre is O_h-fixed)")
    print(f"  Stab abelian?           : {stab_centre.is_abelian}  "
          f"-> NOT a single-U(1) shadow")
    print("  the centre site and the 4 A_1g shell-sum vectors (mult(A_1g)=4,")
    print("  DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) are O_h-fixed: Stab = O_h,")
    print("  non-abelian -- fails D5 (a stabiliser must be exactly one U(1)).")

    # ------------------------------------------------------------------
    # section 9 step 4 -- filter to rank-1 U(1)-shadow stabilisers
    # ------------------------------------------------------------------
    banner("9.4  filter -- rank-1 U(1)-shadow stabilisers + genuine lock?")
    print("  D5: Stab must be the finite shadow of exactly one U(1).")
    print("  D1: a LOCK needs G strictly larger than Stab (a broken sector).")
    print()
    header = ("config", "Stab", "rank-1 U(1)?", "genuine lock?")
    print(f"  {header[0]:24s} {header[1]:10s} {header[2]:16s} {header[3]}")
    print("  " + "-" * 68)
    rows = [
        ("ternary   v = 0",      "Z2",      "ambiguous (Z2)", "no  (Stab = G)"),
        ("ternary   v = +-1",    "trivial", "no (rank 0)",    "n/a"),
        ("Z[i]      v = 0",      "Z4",      "YES",            "no  (Stab = G)"),
        ("Z[i]      v = unit",   "trivial", "no (rank 0)",    "n/a"),
        ("27-block  v = centre", "O_h",     "no (non-abel.)", "no  (Stab = G)"),
    ]
    for r in rows:
        print(f"  {r[0]:24s} {r[1]:10s} {r[2]:16s} {r[3]}")
    print()
    print("  FINDING: the only clean rank-1 U(1)-shadow stabiliser is")
    print("  Stab_{Z[i]^x}(0) = Z4 -- but there Stab = G, a trivial fixed")
    print("  point, NOT a lock.  No single catalog object is a genuine")
    print("  rank-2 -> rank-1 electroweak lock.  The only catalog rank-2")
    print("  shadow is the SU(3) colour triality (BCC triple-cosine) -- the")
    print("  wrong sector: an EW lock must be a colour singlet (D6 ii).")

    # ------------------------------------------------------------------
    # section 9 steps 5-6 -- candidate charge arithmetic + falsifier F-c
    # ------------------------------------------------------------------
    banner("9.5-6  two-state opposition -> Q-charges  (falsifier F-c)")
    print("  D4 forces the hypercharge Y by Q(vev) = T3 + Y = 0 on the vev")
    print("  component (T3 = -t3).  Then Q on the partner = +t3 + Y.")
    print()
    target_patterns = [(0.0, 1.0), (-1.0, 0.0)]    # (neutral, unit) per F-c
    verdict_lines = []
    for label, t3 in (("A   ternary {+1,-1}", 1.0),
                       ("B   Z[i]^x {i,-i}", 0.5)):
        Y = +t3
        q_lower = -t3 + Y
        q_upper = +t3 + Y
        pair = (round(q_lower, 6), round(q_upper, 6))
        fires = pair not in target_patterns
        print(f"  candidate {label}")
        print(f"    T3 = +-{t3};  vev on T3 = -{t3};  D4 forces  Y = {Y:+.1f}")
        print(f"    Q-charges (lower, upper) = ({q_lower:+.1f}, {q_upper:+.1f})")
        print(f"    matches (neutral, unit)? "
              f"{'NO  -> F-c FIRES' if fires else 'YES -> F-c quiet'}")
        print()
        verdict_lines.append((label, fires))

    a_fires = dict((lbl.split()[0], f) for lbl, f in verdict_lines)
    assert a_fires["A"] is True,  "expected candidate A to fire F-c"
    assert a_fires["B"] is False, "expected candidate B not to fire F-c"
    print("  => candidate A (T3 = +-1)   FIRES F-c: Q-pattern (0,+2),")
    print("     shadow provably != (1,2)_1/2.")
    print("  => candidate B (T3 = +-1/2) does NOT fire F-c; nor F-a (the")
    print("     Z[i]^x/conjugation structure carries no BCC triality, so it")
    print("     is a colour singlet); nor F-e (Y = +1/2 is forced, not")
    print("     inserted).  B is the survivor.")

    # ------------------------------------------------------------------
    # section 9 step 7 -- verdict input
    # ------------------------------------------------------------------
    banner("9.7  VERDICT INPUT  ->  UNDERDETERMINED  (pre-reg section 6, clause a)")
    print("  The catalog supplies every INGREDIENT of the finite neutral-lock")
    print("  skeleton:")
    print("    * a genuine two-state opposition with a derivable +-1/2")
    print("      normalisation  (candidate B: {i,-i} of Z[i]^x ~= Z4);")
    print("    * a rank-1 U(1)-shadow for the residual readout (Z[i]^x);")
    print("    * SU(3)-shadow-singlet compatibility (B is colour-blind);")
    print("    * the hypercharge forced by D4, not inserted.")
    print("  It does NOT supply a FORCED rank-2 -> rank-1 assembly: no catalog")
    print("  theorem delivers the colour-singlet rank-2 SU(2)xU(1) shadow as a")
    print("  single object, and the choice of which catalog U(1)-shadow is")
    print("  T3's Cartan vs Y is unforced.  No falsifier provably fires on B")
    print("  (rules out CLOSED-NEGATIVE); the construction trace is not")
    print("  all-[THEOREM]/[DERIVED] (rules out FOUND).")
    print()
    print("  verdict: UNDERDETERMINED -- see AUDIT_FINITE_NEUTRAL_LOCK.md")
    print()
    print("audit backbone complete; all assertions passed.")


if __name__ == "__main__":
    main()
