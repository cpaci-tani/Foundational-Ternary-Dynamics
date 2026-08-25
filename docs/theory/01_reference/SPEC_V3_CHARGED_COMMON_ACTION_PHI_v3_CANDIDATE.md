# SPEC — V3 charged common-action Phi candidate v3

**Date:** 2026-08-24  
**Status:** **[SELECTION — CANDIDATE EXTENSION, NOT CANONICAL PHI]** +
**[THEOREM — CONFLICT-FREE FULL-TICK SCHEDULE ON THE PURE-BOUND CHARGED
SECTOR]** + **[THEOREM — EXACT PARALLEL GAUSS/CONTINUITY AND ONE-HOP ENDPOINT
TRANSPORT]** + **[THEOREM — EXACT CIRCULATION-FRAMED PLAQUETTE ORBIT]** +
**[OPEN — CHARGED POLE, HISTORY MEASURE, WORK NORMALIZATION, AND PHYSICAL
COUPLING]**  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Base law:**
[`SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md`](SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md)  
**Machine manifest:**
[`strict_discrete_common_action_phi_v3_charged_candidate.json`](strict_discrete_common_action_phi_v3_charged_candidate.json)  
**Exact certificate:**
[`proof_v3_charged_common_action_phi_v3_candidate.py`](../../../scripts/proofs/proof_v3_charged_common_action_phi_v3_candidate.py)

---

## 0. Verdict

The dressed-SC source and plaquette-cycle theorems can be composed into one
explicit radius-one candidate tick without adding a carrier, coordinate
coloring, random scheduler, or tie breaker.

The missing scheduling datum is supplied by records already present in the
v3 carrier. Each intrinsically oriented square plaquette uses its four boundary
A9 phases as a circulation frame. The parity of the common C4 offset selects
which of the two equal-boundary paths is primary-owned. One tick swaps the
paths and advances the phases. The output is another valid frame.

The resulting candidate closes two previously open parts of the charged gate:

- simultaneous composition of clean dressed sources, including one-hop
  endpoint transport by path extension/withdrawal; and
- an exact conflict-free schedule for repeated local Gauss-string
  deformation.

It does **not** close the charged static pole or the physical measure. The
selected canonical law remains Phi-v2.

---

## 1. Admission sector

### 1.1 Pure-bound site condition

For each site and polarity, collect every complete eight-channel packet owned
there by an incident primary dressed-SC relation. A site/polarity bank is
**pure bound** when it is exactly the disjoint union of those packets and
contains no free channel of that polarity.

Distinct incident directions have disjoint packet slots for every C4 phase,
C3 layer, and polarity. Hence this ownership predicate is state-only and
contains no packet identity, route label, or replay history.

The charged invariant sector consists of finite pure-bound SC networks and the
circulation frames defined below. Outside that sector, unclaimed coordinates
retain the frozen Phi-v2 rule. The exact Gauss theorem in this document is
scoped to the admitted charged sector; it is not a claim about arbitrary
malformed mixtures of free and bound packets.

### 1.2 Generalized source retiming

At an ordinary site the C3 layer advances as in Phi-v2,

\[
 \ell' = \ell-1\pmod 3.
\]

At a site claimed by an admitted plaquette transaction, the local layer is
stalled for that tick. Every nonplaquette dressed packet owned at that site is
then retimed to the unique complete frame selected by its advanced A9 phase
and the stalled output layer. This is a finite permutation/replacement inside
the existing 384-channel bank.

For either output-layer choice, every dressed edge retains

\[
 \operatorname{div}E=Q,
 \qquad
 \Delta Q+\operatorname{div}j=0,
 \qquad
 \Delta E=-j.
\]

The layer stall is a selected local clock debit. It is not yet a derived work
or gravity law.

---

## 2. Circulation-framed plaquette transaction

### 2.1 Intrinsic boundary roles

For plane family

\[
 (a,b)\in\{(e_x,e_y),(e_y,e_z),(e_z,e_x)\},
\]

write the four positive-presentation boundary relations as

\[
 e_0=[x,x+a],\quad
 e_1=[x+a,x+a+b],\quad
 e_2=[x+b,x+a+b],\quad
 e_3=[x,x+b].
\]

The two equal-boundary paths are

\[
 P_A=(e_0,e_1),
 \qquad
 P_B=(e_3,e_2).
\]

An offset `r in Z4` defines the boundary payloads

\[
 z_i=z(r+i,\epsilon).
\]

If `r` is even, `P_A` is primary-owned and `P_B` reserve-owned. If `r` is
odd, `P_B` is primary-owned and `P_A` reserve-owned. The four vertex C3
layers all equal the plane-family label `0`, `1`, or `2`.

### 2.2 Tick

The plaquette transaction performs simultaneously:

1. `r -> r+1 mod 4` on all four boundary payloads;
2. primary/reserve exchange of the two paths;
3. replacement of the sixteen input bound-channel bits by the sixteen output
   bound-channel bits; and
4. one-tick stall of the four vertex C3 layers.

Because parity changes when `r` advances, the output ownership is exactly the
ownership required by the next circulation frame. The local orbit has period
four.

The path boundaries agree, so for the flip current

\[
 j=\epsilon(P_{\rm after}-P_{\rm before})
\]

one has exactly

\[
 \Delta Q=0,
 \qquad
 \operatorname{div}j=0,
 \qquad
 \Delta E=-j.
\]

Four A9 tokens and sixteen field bits exist before and after.

---

## 3. Why the schedule is conflict-free

No external plaquette coloring is used.

### 3.1 Same-plane neighbors

Two adjacent squares in the same plane assign boundary roles differing by two
to their shared edge. Equality of the shared A9 phase therefore forces their
common offsets to have the same parity. But at equal parity, one frame demands
primary ownership of that role and the other demands reserve ownership. The
two pre-states cannot coexist.

### 3.2 Different planes

Different plane families require different C3 layer values at every vertex.
Two frames from different families that share a vertex therefore cannot both
be admitted.

### 3.3 Vertex-only contact

Compatible same-plane frames may touch only at a vertex. Their incident edge
directions are distinct, and the complete bound packets for distinct
directions occupy disjoint field-channel slots. Multiple layer-stall proposals
at the shared site request the same unchanged layer value.

The exact certificate compares every anchor frame with all frames whose origins
lie in its Moore neighborhood: 9,984 compatible pairs remain, and none shares
a relation writer, input field writer, output field writer, or input/output
field slot.

---

## 4. Complete candidate tick

All outputs are coordinates of one synchronous tick:

1. recognize every complete circulation frame;
2. form the union of its vertex-layer stalls;
3. write all disjoint plaquette transactions;
4. apply the generalized dressed-source tick to every nonplaquette pure-bound
   SC relation, using the same site-layer stall mask;
5. reduce all output primary incidence to the unique site actuality writer;
6. apply the frozen Phi-v2 cases to every unclaimed coordinate.

The plaquette phase/parity theorem removes shared relation claims before the
tick is applied. Packet disjointness removes shared field claims. Every
dependency lies inside one Moore cube. The schedule therefore requires no
proposal arbitration or coordinate-dependent priority.

On the registered R5 vacuum both slots of every relation background are
occupied. The charged source and plaquette recognizers require one-owned
relations, so this extension is exactly inert there. The frozen transverse
vacuum collision and speed `1/6` are unchanged.

---

## 5. Moving-source composition

Let a clean path end at `y`, and let the adjacent relation `[y,y+d]` be
reserve-owned at phase zero. Its ordinary source activation appends one
dressed edge. The new dipole contribution at `y` cancels the old endpoint
charge and recreates it at `y+d`:

\[
 \epsilon\delta_y-\epsilon\delta_{y+d}.
\]

Thus the endpoint has moved one SC hop, and the same activation current obeys

\[
 \Delta Q+\operatorname{div}j=0.
\]

Withdrawing the terminal edge produces the reverse move. The certificate checks
all 144 perpendicular direction/polarity/layer fixtures. This closes moving
endpoint **composition** on clean dressed paths. It does not yet produce a
stable translating particle or a reciprocal force law.

---

## 6. Epistemic boundary

Closed for the candidate extension:

- no new microscopic type;
- a state-only pure-bound admission predicate;
- parallel dressed-source Gauss and continuity;
- one-hop endpoint transport by extension/withdrawal;
- repeated finite plaquette deformation;
- a conflict-free, translation-homogeneous, radius-one schedule; and
- exact inactivity on the canonical transverse-vacuum preparation.

Still open:

- a physically selected history measure or mixing theorem;
- relaxation to the minimum-norm Gauss representative;
- the charged static pole `1/Lambda(k)`;
- reciprocal work and action normalization;
- `chi_EM` and the physical fine-structure coupling;
- formed perturbatively stable matter;
- native Born preparation and trials; and
- the complete protected tensor/gravity sector.

The one-tick C3 layer stall is a real candidate dynamics price. It is not
presented as an emergent consequence, and no physical target chose it. The
next decisive test is whether the deterministic framed-path dynamics has a
derived invariant/mixing object whose charged two-point function contains the
massless static pole. If it does not, the schedule remains an exact kinematic
construction rather than electromagnetism.

The subsequent
[`matter-anchored event seam`](../10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_V3_MATTER_ANCHORED_BORN_GAUSS_GRAVITY_EVENT_SEAM_v1.md)
adds an exact prepared provenance transaction for one dressed source: nine A2
occupancies become eight field records plus one SC-A1 source record, with
exact Gauss continuity, recovery, relative-occupancy conservation, and a
neutral gravity-source handoff. It does not amend this candidate schedule or
close its homogeneous-`Phi`, pole, work, stability, or absolute-scale gates.
