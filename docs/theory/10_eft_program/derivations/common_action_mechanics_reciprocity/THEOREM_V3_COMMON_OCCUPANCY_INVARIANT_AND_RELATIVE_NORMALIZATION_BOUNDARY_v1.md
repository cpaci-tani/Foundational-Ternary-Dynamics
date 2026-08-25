# V3 common occupancy invariant and relative-normalization boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON THE SELECTED TRANSACTION SET AND
ADDITIVE PHASE-BLIND ANSATZ — EXACT TWO-PARAMETER INVARIANT FAMILY]** +
**[THEOREM — FIELD/SC-A1/A2 RELATIVE WEIGHTS EQUAL]** +
**[BOUNDARY — FCC-A1 RELATIVE WEIGHT AND ABSOLUTE SCALE FREE]** +
**[NOT A VARIATIONAL GENERATOR OR PHYSICAL COUPLING DERIVATION]**  
**Scope:** homogeneous additive occupancy energies on the selected v3 carrier
roles and registered transactions  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Matter parent:**
[`THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md`](../constituent_complete_matter/THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md)  
**Successor candidate:**
[`THEOREM_V3_FCC_A1_A2_INCIDENCE_BRIDGE_AND_FULL_OCCUPANCY_EQUALITY_v1.md`](THEOREM_V3_FCC_A1_A2_INCIDENCE_BRIDGE_AND_FULL_OCCUPANCY_EQUALITY_v1.md)  
**Exact certificate:**
[`proof_v3_common_occupancy_invariant_relative_normalization_boundary.py`](../../../../../scripts/proofs/proof_v3_common_occupancy_invariant_relative_normalization_boundary.py)

---

## 1. Question

The payload-complete repair conserved one selected equal-occupancy count, but
did not establish whether equality was forced by the registered microscopic
transactions. Classify every homogeneous, phase-blind additive occupancy
energy on the four existing carrier roles

\[
 (F,A_{1,\mathrm{SC}},A_{1,\mathrm{FCC}},A_2)
\]

without using a target coupling, empirical constant, mass, or Born weight.

This is a classification inside the additive ansatz. It does not assert that
physical action must be additive or phase blind.

---

## 2. Exact role exchanges

The selected v3 transactions give the following role-count changes:

1. canonical absorption/expiry transfers one field record into one SC A1
   relation record,
   \[
   (-1,+1,0,0);
   \]
2. the complete 37,632-row repair shell supplies the exchange vectors
   \[
   (0,+1,0,-1),\quad(+1,0,0,-1),\quad(-1,0,0,+1),
   \]
   together with the zero vector; and
3. field collisions, free streaming, SC/FCC relation recurrence, charged
   frame evolution, and neutral syndrome/vector/tensor transport preserve
   their respective role counts.

The exact repair census is

| role-count change `(F,SC,FCC,A2)` | rows |
|---|---:|
| `(0,+1,0,-1)` | 96 |
| `(+1,0,0,-1)` | 384 |
| `(0,0,0,0)` | 672 |
| `(-1,0,0,+1)` | 36,480 |

For role weights

\[
 e=(e_F,e_{\mathrm{SC}},e_{\mathrm{FCC}},e_{A_2}),
\]

exact conservation requires every exchange row to annihilate `e`. The
exchange matrix has rank two and a two-dimensional nullspace.

---

## 3. Complete invariant family

The nullspace is spanned by

\[
 (1,1,0,1),\qquad(0,0,1,0).
\]

Therefore every invariant in the declared class, and only such an invariant,
has the form

\[
 \boxed{
 H_{\mathrm{occ}}
 =\Gamma\bigl(N_F+N_{A_{1,\mathrm{SC}}}+N_{A_2}\bigr)
  +\Eta N_{A_{1,\mathrm{FCC}}}.
 }                                                        \tag{1}
\]

Consequently,

\[
 \boxed{e_F=e_{A_{1,\mathrm{SC}}}=e_{A_2}}              \tag{2}
\]

is forced by the selected absorption and repair transactions within the
ansatz. The equal weighting used by the payload-complete repair was therefore
not arbitrary on this connected three-role component.

---

## 4. Exact FCC boundary

No selected transaction changes FCC A1 occupancy. Signed-cubic symmetry also
cannot supply the missing equality: SC and FCC bonds are distinct orbits with
squared lengths one and two. Thus neither the transaction graph nor cubic
covariance fixes

\[
 \Eta/\Gamma.                                           \tag{3}
\]

This is a precise missing-edge result. It is not evidence that the two roles
must have unequal physical energies; it says their equality is not implied by
the transaction set audited here.

The incidence-bridge successor tests the minimal existing-carrier exchange
that can connect this isolated FCC role to A2.

---

## 5. What remains physically open

Even equation (1) is a conserved scalar, not yet the generator of `Phi`.
Neither exact conservation nor positivity fixes:

1. the absolute connected multiplier `Gamma` relative to global clock action;
2. the FCC ratio `Eta/Gamma` without an additional transaction;
3. a variational or permutation-generator principle;
4. site-actuality and C3-layer action terms;
5. native formation, writer arbitration, and integration into homogeneous
   `Phi`; or
6. the block-stable interacting curvature and charged pole residue.

Accordingly, the theorem closes a relative-normalization subproblem. It does
not derive `chi_EM`, the fine-structure constant, a mass, or a physical unit
of energy.

The
[`matter-anchored event-seam successor`](THEOREM_V3_MATTER_ANCHORED_BORN_GAUSS_GRAVITY_EVENT_SEAM_v1.md)
now supplies one prepared transaction with role delta
`(F,A1_SC,A1_FCC,A2)=(8,1,0,-9)`. It therefore conserves the connected
all-equal ray exactly while launching charged and neutral source records. The
event confirms that the relative invariant composes across the five sectors;
it remains insensitive to the absolute multiplier and hence does not close
this normalization boundary.

---

## 6. Reproduction

```bash
python scripts/proofs/proof_v3_common_occupancy_invariant_relative_normalization_boundary.py
```

Expected result: `14/14` checks pass, with exchange rank two and invariant
family `Gamma*(N_F+N_A1_SC+N_A2)+Eta*N_A1_FCC`.
