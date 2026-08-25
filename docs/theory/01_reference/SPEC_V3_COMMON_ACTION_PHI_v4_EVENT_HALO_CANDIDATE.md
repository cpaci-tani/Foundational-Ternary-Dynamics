# SPEC — V3 common-action Phi-v4 event-halo candidate

**Date:** 2026-08-24  
**Status:** **[SELECTION — CANDIDATE EXTENSION, NOT CANONICAL PHI]** +
**[THEOREM — HOMOGENEOUS RADIUS-ONE ADMISSION AND CONFLICT-FREE SCHEDULE ON
THE PREPARED EVENT-HALO SECTOR]** +
**[THEOREM — EXACT STAGED POINTER/HERALD PERMUTATION]** +
**[THEOREM — EXACT GAUSS, CONTINUITY, STRESS, AND NEUTRAL GRAVITY-SOURCE
HANDOFF]** + **[THEOREM, CONDITIONAL — PREPARED `|Z|^2` EVENT COUNTS]** +
**[OPEN — FORMATION, PERSISTENT RELEASE, STABILITY, POLES, ABSOLUTE
NORMALIZATION, NO-SIGNALLING, AND LENSING]**  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Base candidate:**
[`SPEC_V3_CHARGED_COMMON_ACTION_PHI_v3_CANDIDATE.md`](SPEC_V3_CHARGED_COMMON_ACTION_PHI_v3_CANDIDATE.md)  
**Machine manifest:**
[`strict_discrete_common_action_phi_v4_event_halo_candidate.json`](strict_discrete_common_action_phi_v4_event_halo_candidate.json)  
**Exact certificate:**
[`proof_v3_homogeneous_event_halo_phi_v4_candidate.py`](../../../scripts/proofs/proof_v3_homogeneous_event_halo_phi_v4_candidate.py)

---

## 1. Verdict

The prepared five-sector event seam can be scheduled by one homogeneous,
state-only, radius-one candidate rule without adding a carrier, coordinate
coloring, random priority, or same-tick broadcast.

The construction uses one complete Moore cube. Seventeen sites carry exact
marker-only zero-`E/B` signatures; ten sites carry the chart, bank, retained
pointers, herald, charged source, and neutral source packets. A globally unique
center signature identifies the chart. Every role site lies within one Moore
step of that center. The center first writes a retained herald; remote writers
act only on the following tick.

This closes homogeneous event admission and writer arbitration on a prepared
sector. It does not form the apparatus, produce a permanent detector record,
stabilize matter, create a protected charged/tensor pole, or select an absolute
coupling.

---

## 2. Exact halo and exclusion

In chart-relative coordinates, all 27 positions of
`{-1,0,+1}^3` are used. The ten writer-role positions are

```text
(-1,+1, 0)  (0,-1, 0)  (0,0,-1)  (0,0,0)  (0,0,+1)
( 0,+1, 0)  (+1,0,-1)  (+1,0,0)  (+1,0,+1)  (+1,+1,0)
```

and the complement is marker-only. The exact charged frame is the square
`z=0`, `x,y in {0,1}` inside those writer positions.

The certificate exhausts all 48 relative signed-cubic orientations and every
displacement for which two radius-one cubes overlap. In all 6,000 cases the
overlap contains a site that is marker-only in at least one proposed halo.
The 19,584 marker signatures are globally unique. The center chart signature
is also globally unique and disjoint from every marker; noncentral role pads
have a different exact record weight. Therefore two complete writer halos
cannot overlap.

This exclusion is physical state, not a coordinate coloring. A malformed or
partial halo is not admitted.

---

## 3. Radius-one local recognition

The center can read the entire prepared cube in one P4 neighborhood. Its
unique zero-`E/B` chart code identifies the complete chart and distinguishes
the center from every marker. Every remote writer is itself within one Moore
step of the center. It recognizes:

1. the center code and retained herald;
2. its chart-relative displacement from that center; and
3. its own writer-clear local role pad.

The center does not command remote sites during the same tick. It first
replaces a retained dark neutral pair by the addressed dark or event herald.
Only the next tick may consume that physical record. Thus every dependency is
radius one in the preceding state.

The center/role signatures and the complete active packing are covariant under
the signed-cubic generators. The construction selects no absolute spatial
direction beyond the prepared oriented chart.

---

## 4. Staged apparatus permutation

Let `t` be the joint address of physical pointer cycles of lengths 384 and 385.
The second cycle has one blank delay state. The coprime reconstruction is

\[
 t=i+384(i-j\bmod385)\pmod{147{,}840}.
\]

The pointer records remain physical neutral field pairs with existing A2
polarity tokens. They are not replaced by an abstract integer or an unbounded
counter. All 147,840 physical pointer pairs admit a collision-free placement
inside the prepared halo.

For the addressed label `o(t)`, define

```text
READY(t)                  -> HERALD(t,o(t))
HERALD(t,dark)            -> READY(t+1)
HERALD(t,bright outcome)  -> RECOVERY(t,outcome)
RECOVERY(t,outcome)       -> READY(t+1)
```

The complete registered fixture contains 295,739 staged states. The forward
map is a bijection and the explicit inverse recovers every state. Replacing
one neutral herald pair by another has occupancy delta zero.

---

## 5. Active common event

On a bright herald, a finite prepared A2 work reserve is converted into:

- eight charged field records forming one dressed Gauss packet;
- one SC-A1 source token; and
- 24 neutral records forming two scalar/vector/STF source packets.

In role order `(F,A1_SC,A1_FCC,A2)`, the active transition is

\[
 \boxed{\Delta N=(32,1,0,-33)}.
\]

Hence the conditionally selected all-equal relative occupancy ray is exactly
conserved. Recovery applies the negative delta. The certificate packs every
combination of six source directions, two charge signs, and three C3 layers
without slot collision or capacity overflow.

The charged part obeys

\[
 \nabla\!\cdot E=Q,
 \qquad \Delta Q+\nabla\!\cdot j=0.
\]

Its charge-even stress is handed to two neutral packets satisfying the parent
tensor/vector source-coordinate identities. These are exact local source
records, not yet autonomous response poles.

---

## 6. Prepared Born count and resource boundary

The stage insertion changes timing but not address compatibility. One
RECOVERY state remains for every bright compatible pair. Therefore the
prepared event count remains

\[
 M_o=|Z_o|^2.
\]

No probability or target weight is read by the rule. The statement remains
conditional on a prepared chart, bank, pointers, herald register, and work
reserve.

Exact recovery permits repeated trials without leaving an outgoing record. If
the 33 converted work occupancies are instead released and persist, a finite
reserve `R` supports at most

\[
 \left\lfloor {R\over33}\right\rfloor
\]

events unless a causal refill current enters. Persistent amplification and
indefinite renewal are therefore a physical resource problem, not free
consequences of deterministic counting.

---

## 7. Candidate schedule

The synchronous candidate order is:

1. recognize every complete center chart code and event halo;
2. write or retain the center herald;
3. hold prepared halo records during READY/dark stages;
4. on a prior-tick bright herald, write the disjoint charged and neutral source
   records;
5. on RECOVERY, apply the exact inverse and advance both local pointer cycles;
6. apply the charged Phi-v3 candidate to unclaimed charged coordinates; and
7. apply frozen canonical Phi-v2 to every remaining unclaimed coordinate.

Complete event halos cannot overlap. Their claimed coordinates therefore need
no tie breaker. The event extension is target-blind and inert outside the
exact prepared sector.

---

## 8. Epistemic boundary

Closed for this candidate sector:

- homogeneous state-only halo admission;
- exact overlap exclusion and writer arbitration;
- radius-one causal heralding;
- finite pointer/stage permutation with inverse;
- exact event carrier packing and relative occupancy conservation;
- Gauss/continuity plus stress/tensor/vector source handoff;
- signed-cubic covariance; and
- unchanged prepared `|Z|^2` event counts.

The exact formation census in
[`THEOREM_V3_EVENT_HALO_FORMATION_SEED_AND_RESOURCE_BOUNDARY_v1.md`](../10_eft_program/derivations/constituent_complete_matter/THEOREM_V3_EVENT_HALO_FORMATION_SEED_AND_RESOURCE_BOUNDARY_v1.md)
counts 567 occupied field records in the READY halo and 599 in the active
halo. Conditional on the selected nonnegative all-equal occupancy invariant,
neither can descend from a completely blank causal past. Formation must
consume a finite nonblank genesis seed, receive causal boundary inflow, or
explicitly replace that invariant. This is a resource obstruction, not a
formation law.

A conditional companion theorem places three disjoint orthogonal A9 clocks
and three copied neutral herald registers in the prepared chart without using
the charged-frame or source relations. Its period-16 READ/COMMIT code repairs
all 1,488 one-register valid-symbol substitutions within two ticks and has
isotropic mean capacity deficit `-I/36`:
[`THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md`](../10_eft_program/derivations/constituent_complete_matter/THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md).
That theorem is not yet composed with active event traffic and supplies no
assembly, binding energy, translation, or collision law.

A further Phi-v5 prepared-sector candidate enlarges the copied herald to carry
one syndrome bit and changes one existing fixed-occupancy A2 token READY to
EXCITED when repair occurs. It closes all 2,256 enlarged-register error rows,
the relative ledger `h+w=1`, and busy-port backpressure while leaving the
absolute multiplier and work export/reset open:
[`THEOREM_V3_TRIPLET_RELATIONAL_REPAIR_WORK_PORT_PHI_v5_CANDIDATE_v1.md`](../10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_V3_TRIPLET_RELATIONAL_REPAIR_WORK_PORT_PHI_v5_CANDIDATE_v1.md).

Still open:

- target-blind causal assembly of the halo, frame, bank, pointers, herald, and
  work reserve from an admissible seed or inflow;
- persistent amplified records or a causal reserve-refill law;
- perturbatively stable translating matter;
- autonomous charged and protected tensor poles;
- one interacting action and absolute physical coupling normalization;
- multipartite no-signalling;
- universal clock/trajectory/light response, common cone, and lensing; and
- nonlinear gravity.

The canonical law remains Phi-v2. This document records a selected candidate
extension and does not move any five-sector item from `[OPEN]` to closed.

---

## 9. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_homogeneous_event_halo_phi_v4_candidate.py
```

Expected result: `17/17` checks pass, including 6,000 overlap rows, 19,584
marker signatures, 58,752 marker-generator rows, 147,840 physical pointer
rows, 295,739 staged states, and 36 active direction/sign/layer packings.
