# V3 FCC A1/A2 incidence bridge and full occupancy equality v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON THE SELECTED BRIDGE — EXACT
INCIDENCE, LOCAL PERMUTATION, AND COVARIANCE]** +
**[THEOREM, CONDITIONAL ON THE SELECTED BRIDGE AND ADDITIVE PHASE-BLIND
ANSATZ — UNIQUE ALL-EQUAL RELATIVE OCCUPANCY RAY]** +
**[SELECTION — ONE C3 CLOCK SECTION AND FCC-RESERVE/A2-DIAGONAL CROSSING]** +
**[OPEN — COMMON-ACTION PROVENANCE, PHI INTEGRATION, AND ABSOLUTE SCALE]**  
**Scope:** one selected existing-carrier FCC-A1/A2 exchange on the v3 cubic
record complex  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Parent boundary:**
[`THEOREM_V3_COMMON_OCCUPANCY_INVARIANT_AND_RELATIVE_NORMALIZATION_BOUNDARY_v1.md`](THEOREM_V3_COMMON_OCCUPANCY_INVARIANT_AND_RELATIVE_NORMALIZATION_BOUNDARY_v1.md)  
**Exact certificate:**
[`proof_v3_fcc_a1_a2_incidence_bridge_full_occupancy_equality.py`](../../../../../scripts/proofs/proof_v3_fcc_a1_a2_incidence_bridge_full_occupancy_equality.py)

---

## 1. Minimal geometric connection

Every unoriented FCC bond of displacement `+/-e_i +/- e_j`, `i != j`, is the
unique diagonal of one elementary SC plaquette. Conversely, each elementary
plaquette has two FCC diagonals. On a periodic cubic lattice of side at least
three this gives a bijection

\[
 \{\text{FCC A1 bonds}\}
 \longleftrightarrow
 \{\text{labeled A2 plaquette diagonals}\}.             \tag{1}
\]

The exact `L=5` certificate contains 750 incidences and verifies all 36,000
incidence rows under the 48 signed-cubic transformations. The correspondence
therefore uses no preferred spatial direction.

Both incident cells already carry an A9 pair in the selected v3 inventory:
the FCC A1 bond has primary/reserve slots and the A2 plaquette has diagonal
slots. No new primitive type, continuous coordinate, or counter is required.

---

## 2. Selected local clock

Select one existing C3 layer as a bridge section. On that section:

1. the FCC primary advances one C4 phase in place;
2. the FCC reserve and its unique A2 diagonal slot execute the already
   certified A9 crossing clock; and
3. the primary source occupancy and polarity remain unchanged.

On the other two C3 sections, the FCC primary/reserve pair executes its
existing A1 recurrence while the A2 diagonal token advances one C4 phase in
place.

For either parity gate, exhaustive enumeration of all

\[
 2\cdot3\cdot9^3=4,374                                \tag{2}
\]

local rows proves that the complete layer-clock map is a finite permutation.
Every occupied token advances exactly one C4 phase per tick and retains its
polarity. The bridge section is source neutral; the two recurrence sections
retain their exact bond-current continuity identities. Unique incidence also
gives one writer per FCC reserve and labeled A2 diagonal under simultaneous
application.

These facts establish that the bridge is internally admissible. They do not
derive why canonical `Phi` must choose it or this C3 section.

---

## 3. Full relative occupancy equality

The bridge contributes the role-count exchange

\[
 (0,0,-1,+1)                                          \tag{3}
\]

to the parent exchange matrix on
`(F,A1_SC,A1_FCC,A2)`. The augmented matrix has rank three and a
one-dimensional nullspace spanned by

\[
 \boxed{(1,1,1,1)}.                                   \tag{4}
\]

Hence, conditional on the selected bridge and the same homogeneous additive
phase-blind ansatz, every conserved occupancy energy is

\[
 \boxed{
 H_{\mathrm{occ}}
  =\Gamma\bigl(N_F+N_{A_{1,\mathrm{SC}}}
                    +N_{A_{1,\mathrm{FCC}}}+N_{A_2}\bigr).
 }                                                     \tag{5}
\]

The result closes the complete **relative** role-weight ray. It provides no
value for `Gamma`.

---

## 4. Epistemic boundary

The exact claims are conditional on a plainly named selection. The bridge is
target blind and finite, but it has not been obtained as the stationary rule
of one common history action and has not been composed with charged dressing,
writer priority, repair traffic, or canonical `Phi-v2`.

Still open are:

1. common-action or other state-complete provenance for the bridge;
2. integration with canonical `Phi` and its writer priorities;
3. the absolute multiplier `Gamma` relative to clock action;
4. bundle clock-debit and formation work;
5. block-stable interacting curvature and charged static-pole residue; and
6. multi-event arbitration and stable matter formation.

Thus equation (5) is not a fine-structure result. It removes one structural
normalization freedom while leaving the physical coupling normalization
open.

The
[`matter-anchored event-seam successor`](THEOREM_V3_MATTER_ANCHORED_BORN_GAUSS_GRAVITY_EVENT_SEAM_v1.md)
uses this conditional all-equal ray in an exact prepared bright/recovery
transaction. Its delta `(8,1,0,-9)` conserves equation (5), joining the
apparatus, Gauss packet, and gravity-source handoff without fixing `Gamma` or
integrating this bridge into homogeneous `Phi`.

---

## 5. Reproduction

```bash
python scripts/proofs/proof_v3_fcc_a1_a2_incidence_bridge_full_occupancy_equality.py
```

Expected result: `12/12` checks pass, with 750 incidences, 4,374 local clock
rows, exchange rank three, and
`e_F=e_A1_SC=e_A1_FCC=e_A2`.
