#!/usr/bin/env python3
"""
audit_colour_singlet_rank.py -- computational backbone for the Q11 audit.

Executes the catalog enumeration and rank tally behind
``docs/theory/08_structural/AUDIT_COLOUR_SINGLET_RANK.md``, the result of
the pre-registered colour-singlet rank audit:

    pre-reg : docs/theory/08_structural/PREREG_COLOUR_SINGLET_RANK_v1.md
    LEDGER  : FTD-0191  [PRE-REGISTRATION]
    git tag : preregister-colour-singlet-rank-v1
    SHA256  : 08c55b8e060332a2311be7ae6dedf5d48cbf1af861db627195d1dd2f8a886dbe

This is NOT a numerical search and contains NO near-miss / coincidence
scan.  It enumerates the FROZEN catalog of pre-reg section 4 (identical to
the Q10 section-4 catalog), classifies every structure (colour-singlet?
internal? cyclic order?), and *computes* -- does not assert -- the
colour-singlet internal abelian rank under each admissible reading
(pre-reg section 9 method, steps 1-6).

The dual-substrate decomposition is read from the catalog-cited document
``docs/theory/02_foundations/FOUND_FORCE_STRUCTURE.md`` (Parts III, VII):
a dual-substrate vector field (J_L, J_R) in D=3 has four mode types --
sum J (EM), difference phi = J_L - J_R (weak/chirality), magnitude |J|
(gravity), and internal orientation of J_R (colour / SU(3)).

No new free integer, exponent, group, or scale is introduced.
Run: ``python audit_colour_singlet_rank.py``
"""

import sys


def banner(text):
    print()
    print("=" * 72)
    print(text)
    print("=" * 72)


# ----------------------------------------------------------------------
# pre-reg D1 -- the rank-1 U(1)-shadow test (computed, not asserted)
# ----------------------------------------------------------------------
def is_rank1_u1_shadow(name, cyclic_order):
    """D1: a clean rank-1 U(1)-shadow is a cyclic group of order >= 3.
    An order-2 group (Z2) is a mu_2 / Weyl element, explicitly excluded."""
    if cyclic_order is None:
        return False, "non-cyclic / not abelian"
    if cyclic_order >= 3:
        return True, f"Z{cyclic_order}: cyclic, order {cyclic_order} >= 3 -> U(1)-shadow"
    return False, f"Z{cyclic_order}: order {cyclic_order} < 3 -> NOT a U(1)-shadow (D1)"


def main():
    banner("Q11 colour-singlet rank audit -- computational backbone")
    print("pre-registration : PREREG_COLOUR_SINGLET_RANK_v1.md  (FTD-0191)")
    print("catalog FROZEN (pre-reg section 4 = Q10 section 4).  Enumeration only.")

    # ------------------------------------------------------------------
    # section 9 step 1 -- catalog, marked colour-singlet / internal
    # ------------------------------------------------------------------
    banner("9.1  catalog -- colour-singlet (D2) and internal (D3) marks")
    # (item, structure, colour_singlet, internal, carried_forward, reason)
    catalog = [
        ("1", "ternary alphabet {-1,0,+1}, Aut = Z2", True, True, True,
         "per-site, no triality; acts on state"),
        ("2", "O_h spatial point group (order 48)", True, False, False,
         "SPACETIME symmetry (permutes lattice position) -- D3 excludes"),
        ("2", "BCC triality Z3 -> SU(3) colour", False, True, False,
         "this IS the colour structure -- D2 excludes (it defines colour)"),
        ("3", "Z[i]^x = Z4 (Gaussian units)", True, True, True,
         "per-site arithmetic; acts on flux/state"),
        ("4", "27-block O_h irreps", True, False, False,
         "the O_h here is the spatial point group -- D3 excludes"),
        ("5", "dual substrate (J_L, J_R)", True, True, True,
         "per-site L/R flux; acts on flux structure"),
        ("6", "framework integers {3,4,7,13}", True, True, False,
         "invariants, not symmetry groups"),
    ]
    for item, struct, cs, intl, fwd, reason in catalog:
        mark = "CARRY" if fwd else "drop "
        print(f"  [{mark}] item {item}: {struct}")
        print(f"          colour-singlet={cs!s:5s} internal={intl!s:5s} -- {reason}")
    carried = [c for c in catalog if c[4]]
    print(f"\n  carried forward (colour-singlet + internal): {len(carried)} structures")

    # ------------------------------------------------------------------
    # section 9 step 1 (cont.) -- dual-substrate four-mode decomposition
    # ------------------------------------------------------------------
    banner("9.1  dual substrate -- four-mode decomposition (FOUND_FORCE_STRUCTURE.md)")
    # (mode, description, colour-singlet?, forced internal symmetry)
    ds_modes = [
        ("J = J_L + J_R", "EM transverse vector", True,
         "U(1)_EM -- the residual readout (lock OUTPUT, not an input rank-1)"),
        ("phi = J_L - J_R", "weak / chirality pseudovector", True,
         "parity Z2 (J_L<->J_R) only -- order 2"),
        ("|J|", "gravity scalar", True,
         "none -- a scalar carries no internal U(1)"),
        ("orientation of J_R", "colour / SU(3)", False,
         "BCC triality -- COLOURED, excluded by D2"),
    ]
    for mode, desc, cs, sym in ds_modes:
        keep = "colour-singlet" if cs else "COLOURED (excluded)"
        print(f"  {mode:18s} {desc:32s} [{keep}]")
        print(f"  {'':18s} forced internal symmetry: {sym}")

    # ------------------------------------------------------------------
    # section 9 steps 2-3 -- abelian shadows + rank-1 classification
    # ------------------------------------------------------------------
    banner("9.2-3  abelian shadows of the colour-singlet internal sector")
    # (label, cyclic_order or None, grade)  grade in {forced, selection, residual}
    shadows = [
        ("ternary alphabet: Aut = Z2",                    2,    "forced"),
        ("Z[i]^x = Z4",                                   4,    "forced"),
        ("dual substrate |J| (scalar)",                   None, "forced"),
        ("dual substrate J=J_L+J_R: U(1)_EM (residual)",  None, "residual"),
        ("dual substrate phi: parity Z2",                 2,    "forced"),
        ("dual substrate phi: weak SU(2) Cartan",         "su2", "selection"),
    ]
    forced_rank1 = []
    selection_rank1 = []
    for label, order, grade in shadows:
        if order == "su2":
            # SU(2) is non-abelian but contributes exactly one rank-1 Cartan
            ok, why = True, "SU(2): non-abelian, contributes one rank-1 Cartan"
        else:
            ok, why = is_rank1_u1_shadow(label, order)
        tag = ""
        if ok and grade == "forced":
            forced_rank1.append(label)
            tag = "  -> FORCED rank-1"
        elif ok and grade == "selection":
            selection_rank1.append(label)
            tag = "  -> rank-1 only under a [SELECTION]-grade reading"
        elif grade == "residual":
            tag = "  -> residual readout (lock output; not a pre-breaking input)"
        print(f"  {label:46s} {why}{tag}")

    # ------------------------------------------------------------------
    # section 9 steps 3-5 -- rank tally under each admissible reading
    # ------------------------------------------------------------------
    banner("9.3-5  colour-singlet internal abelian rank -- by reading")
    rank_forced = len(forced_rank1)
    rank_selection = len(forced_rank1) + len(selection_rank1)
    print(f"  FORCED reading (count only catalog-FORCED rank-1 U(1)-shadows):")
    for s in forced_rank1:
        print(f"    + {s}")
    print(f"    rank_forced = {rank_forced}")
    print()
    print(f"  [SELECTION] reading (also count the weak SU(2) Cartan on phi,")
    print(f"  per FOUND_FORCE_STRUCTURE.md FST-1 [SELECTION] / FST-6 [CONJECTURE]):")
    for s in forced_rank1 + selection_rank1:
        print(f"    + {s}")
    print(f"    rank_selection = {rank_selection}")
    print()
    forced = (rank_forced == rank_selection)
    print(f"  rank reading-invariant (D6 'forced')?  {forced}")
    assert not forced, "expected the rank to be reading-dependent"
    assert rank_forced == 1, "expected forced rank = 1"
    assert rank_selection == 2, "expected selection rank = 2"
    print(f"  => rank in {{{rank_forced}, {rank_selection}}} -- NOT forced (D6 fails)")

    # ------------------------------------------------------------------
    # section 9 step 5-6 -- compare to benchmark; falsifier; Q11c
    # ------------------------------------------------------------------
    banner("9.5-6  benchmark (rank 2, SU(2)xU(1)) + falsifier + Q11c")
    print("  F-a (rank forced != 2):     does NOT fire -- rank is not forced at all")
    print("  F-b (rank 2 needs a new postulate): does NOT fire -- the weak-SU(2)")
    print("       reading is in the catalog-cited FOUND_FORCE_STRUCTURE.md, as a")
    print("       [SELECTION], not a new structure")
    print("  F-d (a Z2 counted as a U(1)): does NOT fire -- both Z2's (ternary,")
    print("       parity) were correctly rejected by D1")
    print("  F-e (spacetime as internal):  does NOT fire -- O_h spatial group was")
    print("       excluded by D3")
    print("  Q11c: under the rank-2 [SELECTION] reading the second factor IS the")
    print("        weak SU(2) (one non-abelian factor) -> structure SU(2)xU(1),")
    print("        so F-f would not fire either -- but the rank is not forced, so")
    print("        Q11c is moot for the verdict.")

    # ------------------------------------------------------------------
    # section 9 step 7 -- verdict
    # ------------------------------------------------------------------
    banner("9.7  VERDICT INPUT  ->  UNDERDETERMINED  (pre-reg section 6)")
    print("  The catalog FORCES exactly one colour-singlet internal rank-1")
    print("  U(1)-shadow: Z[i]^x = Z4.  A SECOND rank-1 -- the weak SU(2)")
    print("  Cartan -- exists only under FTD's [SELECTION]/[CONJECTURE]-grade")
    print("  assignment of the weak force to the chirality mode phi = J_L - J_R")
    print("  (FOUND_FORCE_STRUCTURE.md FST-1, FST-6).  phi's only FORCED internal")
    print("  symmetry is the parity Z2, which D1 excludes.  Hence")
    print("  rank in {1, 2}, not reading-invariant => NOT forced => UNDERDETERMINED.")
    print()
    print("  Consequence for Q10 (FTD-0190): stays UNDERDETERMINED.")
    print("  see AUDIT_COLOUR_SINGLET_RANK.md")
    print()
    print("audit backbone complete; all assertions passed.")


if __name__ == "__main__":
    main()
