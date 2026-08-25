# V3 homogeneous event halo and renewal-resource boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON PREPARED RESOURCES — HOMOGENEOUS
RADIUS-ONE FIVE-SECTOR EVENT SCHEDULE]** +
**[THEOREM — EXACT OVERLAP EXCLUSION, STAGED INVERSE, AND SIGNED-CUBIC
COVARIANCE]** + **[THEOREM — FINITE PERSISTENT-RELEASE RESOURCE BOUNDARY]** +
**[OPEN — FORMATION, STABILITY, POLES, ABSOLUTE SCALE, NO-SIGNALLING, AND
LENSING]**  
**Candidate rule:**
[`SPEC_V3_COMMON_ACTION_PHI_v4_EVENT_HALO_CANDIDATE.md`](../../../01_reference/SPEC_V3_COMMON_ACTION_PHI_v4_EVENT_HALO_CANDIDATE.md)  
**Parent event seam:**
[`THEOREM_V3_MATTER_ANCHORED_BORN_GAUSS_GRAVITY_EVENT_SEAM_v1.md`](THEOREM_V3_MATTER_ANCHORED_BORN_GAUSS_GRAVITY_EVENT_SEAM_v1.md)  
**Exact certificate:**
[`proof_v3_homogeneous_event_halo_phi_v4_candidate.py`](../../../../../scripts/proofs/proof_v3_homogeneous_event_halo_phi_v4_candidate.py)

---

## Theorem

On the selected v3 carrier, conditional on one prepared exact event halo, there
exists a homogeneous state-only schedule with the following properties:

1. every dependency is contained in a preceding-tick radius-one Moore
   neighborhood;
2. two complete event halos cannot overlap;
3. every remote writer identifies its chart/role from a globally unique center
   code, its displacement from the center, its local role pad, and a retained
   prior-tick herald;
4. READY, HERALD, and RECOVERY form one exact finite permutation;
5. every physical `384 x 385` pointer address admits a collision-free halo
   placement;
6. a bright event has exact role delta `(32,1,0,-33)`, preserves the all-equal
   relative occupancy ray, and realizes the parent Gauss/continuity and
   stress/tensor/vector source identities;
7. the construction is signed-cubic covariant; and
8. the prepared manifested-event multiplicities remain `M_o=|Z_o|^2`.

If the active source records are exactly recovered, no persistent output is
left. If instead all 33 converted work occupancies are released as persistent
records, a finite prepared reserve `R` can support at most `floor(R/33)` such
events without a causal refill current.

---

## Proof

Choose the ten chart-relative writer positions and seventeen-site complement
specified by the candidate rule. Exhaustion of all 48 relative signed-cubic
orientations and all intersecting displacements gives 6,000 overlap cases.
Every overlap contains a marker-only witness. Marker signatures are globally
unique; the center chart signature is globally unique and is not a marker;
other role pads have a different constant record weight. Hence two complete
halos cannot occupy the same overlap.

The center's Moore neighborhood is the full prepared cube. Every writer site
is within one Moore step of the center. The center changes only its retained
neutral herald on the first stage. Remote writes depend on that physical
herald in the following state, so no same-tick broadcast occurs.

The two finite physical pointer cycles are coprime. Their CRT address map is a
bijection on 147,840 states. Direct enumeration of every staged state proves
the forward map and explicit inverse. Direct carrier packing covers all six
directions, two polarities, and three C3 layers. The parent algebra then gives
Gauss, continuity, charge-even stress, and the neutral source-coordinate
handoff. Generator exhaustion proves covariance. Adding the herald stage does
not change which pointer pairs are compatible, so the parent `|Z|^2` count is
unchanged.

Finally, persistent release consumes 33 prepared work occupancies per event.
After `k` releases the debit is `33k`; `33k <= R` is equivalent to
`k <= floor(R/33)`. Exact recovery avoids that debit only by removing the
outgoing records. This proves the renewal-resource alternative. QED.

---

## Scope boundary

This theorem closes the earlier event seam's homogeneous admission,
overlap-arbitration, and prior-tick heralding debts on the prepared sector. It
does not close native resource formation, persistent amplification, causal
refill, stable matter, dynamical response poles, absolute normalization,
multipartite no-signalling, lensing, or nonlinear gravity. Canonical Phi-v2
and all five `[OPEN]` recovery gates retain their status.
