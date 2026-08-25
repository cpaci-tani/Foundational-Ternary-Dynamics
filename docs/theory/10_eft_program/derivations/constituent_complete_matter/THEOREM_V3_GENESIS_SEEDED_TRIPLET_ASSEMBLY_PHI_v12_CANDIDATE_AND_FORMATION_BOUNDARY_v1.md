# V3 genesis-seeded triplet assembly Phi-v12 candidate and formation boundary v1

**Date:** 2026-08-24  
**Status:** **[SELECTION — ORIENTED GENESIS-SEED ASSEMBLY BRANCH]** +
**[THEOREM, CONDITIONAL — EXACT ONE-TICK RESOURCE-CONSERVING TRIPLET
FORMATION]** + **[THEOREM — NONZERO SEED-ERROR BASIN WITH RETAINED WORK]** +
**[THEOREM — SIGNED-CUBIC COVARIANCE AND BLANK-PAST OBSTRUCTION]** +
**[BOUNDARY — GENESIS-CHART ORIGIN, BINDING, GENERAL STABILITY, AND CANONICAL
PHI OPEN]**  
**Additional carrier price:** none beyond a finite nonblank oriented seed made
from three existing SC A9 owners, three existing neutral field-pair markers,
and one existing A2 A9 work/phase owner  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Prepared matter parent:**
[`THEOREM_V3_TRIPLET_RELATIONAL_REPAIR_WORK_PORT_PHI_v5_CANDIDATE_v1.md`](../common_action_mechanics_reciprocity/THEOREM_V3_TRIPLET_RELATIONAL_REPAIR_WORK_PORT_PHI_v5_CANDIDATE_v1.md)  
**Formation obstruction parent:**
[`THEOREM_V3_EVENT_HALO_FORMATION_SEED_AND_RESOURCE_BOUNDARY_v1.md`](THEOREM_V3_EVENT_HALO_FORMATION_SEED_AND_RESOURCE_BOUNDARY_v1.md)  
**Certificate:**
[`proof_v3_genesis_seeded_triplet_assembly_phi_v12_candidate.py`](../../../../../scripts/proofs/proof_v3_genesis_seeded_triplet_assembly_phi_v12_candidate.py)

---

## 1. Why formation must start nonblank

The v3 common transaction census admits the nonnegative relative occupancy
invariant

\[
 H_{\rm occ}=N_F+N_{A1,SC}+N_{A1,FCC}+N_{A2}.          \tag{1}
\]

The prepared Phi-v5 triplet has role count

\[
 (N_F,N_{A1,SC},N_{A1,FCC},N_{A2})=(6,3,0,1).         \tag{2}
\]

Consequently a conserving rule cannot create this body from a completely
blank causal past. This is not a defect repaired by saying “emergence.” It is
an exact provenance requirement. Under P1--P5 the admissible alternatives are
a finite nonblank genesis seed, a causal boundary current, or abandonment of
equation (1).

Phi-v12 selects the first branch. The seed is an initial physical record, not
a sixth postulate and not an external time-dependent instruction.

---

## 2. Finite oriented genesis seed

Condition on one already physical oriented chart from the registered
1,152-state chart orbit. Relative to its ordered axes
`(first,second,repair_normal)`, the seed owns three SC A9 relations with tails

\[
 (-1,-1,0),\qquad(1,-1,-1),\qquad(-1,1,-1),           \tag{3}
\]

and directions `first`, `second`, and `repair_normal`, respectively. Their
destinations are the three disjoint triplet arms already used by Phi-v5. Each
relation midpoint moves by at most one Moore step; all endpoints remain in the
closed radius-one chart cube.

The three seed relations are disjoint from:

1. the retained oriented chart relations;
2. all registered event-source relations; and
3. all three destination triplet relations.

At the chart center, three neutral opposite-polarity field pairs carry a
finite seed marker. The exact clear controller pool has 71 states left after
the 99 Phi-v5 pending-register states are reserved. Phi-v12 uses 48 of those
states as three disjoint 16-symbol banks. Every selected symbol has zero
electric and magnetic readout on every layer.

One existing A2 A9 owner carries the phase immediately preceding the Phi-v5
READY phase:

\[
 a_{\rm seed}=({\tt offset}-1\bmod4,{\tt polarity}).   \tag{4}
\]

Thus the assembly trigger is contained in the seed state itself. It is not a
reference to an external global tick number.

---

## 3. One homogeneous local formation branch

The three A9 owners carry three copies of one logical clock symbol `q`. The
three neutral markers carry three copies of READY. The state-complete local
branch is:

```text
if strict-majority(A9 owners) = q
and strict-majority(seed markers) = READY
and A2 phase = ASSEMBLY:
    move the three A9 owners to the triplet arms
    rewrite the three neutral markers to the Phi-v5 DARK herald symbols
    write A2 READY if no mismatch, otherwise A2 EXCITED
else:
    leave the seed unchanged
```

The chart is a surviving physical context record. Applying the same predicate
at every translated voxel is spatially homogeneous. Transforming chart,
relations, field owners, and A9 states together by any signed-cubic generator
commutes with the transaction.

For a clean seed the exact one-tick map is

\[
 (6,3,0,1)_{\rm seed}
 \longmapsto
 (6,3,0,1)_{\rm triplet},                              \tag{5}
\]

so

\[
 \Delta(N_F,N_{A1,SC},N_{A1,FCC},N_{A2})=(0,0,0,0).   \tag{6}
\]

The formed body is exactly the READY Phi-v5 state and enters its exact
period-16 self-correcting clock orbit on the next tick. No interface adapter
or new carrier is required.

---

## 4. A genuine seed-admission basin

The construction does not certify only one hand-picked trajectory. For every
one of the sixteen valid A9 logical states, the certificate replaces in turn:

- one of three seed A9 copies by any of the other fifteen valid states; or
- one of three seed-marker copies by any of the other fifteen admitted
  marker symbols.

This gives exactly

\[
 16\,[3(15)+3(15)]=1,440                              \tag{7}
\]

registered one-symbol seed perturbations. Every row forms the same decoded
triplet in one tick and changes the A2 owner to EXCITED. The exact relative
ledger is the already registered Phi-v5 relation

\[
 h_{\rm seed}+w_{A2}:\qquad 1+0\longmapsto0+1.         \tag{8}
\]

Different erroneous seed identities map to the same formed body and generic
work record. The detailed label therefore genuinely expires, while its
physical consequence survives. This is an explicit use of P5's named
many-to-one expiry branch, not hidden irreversible bookkeeping.

Malformed triples with no strict majority and seeds whose A2 port is not in
the ASSEMBLY phase fail closed.

---

## 5. Exact finite certificate

The proof establishes:

1. 1,152 finite oriented chart states;
2. seed/final relation disjointness, chart/source clearance, and one-hop
   Moore locality on all charts;
3. 55,296 existing neutral seed-register rows;
4. 165,888 signed-cubic register-covariance rows;
5. all sixteen clean logical formation rows;
6. all 1,440 one-symbol seed-error rows;
7. noninjective error-identity expiry with exactly sixteen decoded outputs;
8. fail-closed malformed and busy-port behavior;
9. exact entry into the Phi-v5 period-16 orbit;
10. zero role-count debit and all-equal-ray conservation;
11. impossibility of this branch acting on a blank state; and
12. absence of an empirical target, probability primitive, continuum field,
    or fitted action scale.

Reproduce with:

```powershell
python scripts/proofs/proof_v3_genesis_seeded_triplet_assembly_phi_v12_candidate.py
```

Expected result: `14/14` exact checks pass.

---

## 6. What “native formation” now means—and does not mean

This theorem closes the first exact **seed-to-body assembly** subgate:

```text
finite oriented genesis seed
  -> one homogeneous Moore-local transaction
  -> resource-identical self-correcting triplet
  -> autonomous period-16 material clock
```

It does not derive the genesis seed from blankness. Equation (1) proves that
such a claim would contradict the selected conserved resource. Nor does this
theorem yet establish:

1. formation of the oriented chart itself from a more primitive seed or
   boundary current;
2. uniqueness or canonical provenance of the Phi-v12 branch;
3. overlapping-seed and event-halo traffic arbitration;
4. positive binding or the absolute relational action multiplier;
5. occupancy faults, multiple simultaneous errors, or indefinite
   environmental survival;
6. general collisions, scattering, source emission, or absorption;
7. mass, dispersion, or relativistic saturation; or
8. protected charged/tensor poles and universal gravity.

The correct status is therefore not “stable matter derived.” It is:

> **A finite genesis seed can causally assemble the exact self-correcting
> triplet under one selected, target-blind, resource-conserving local branch;
> the origin and physical stabilization of that seed/body remain open.**
