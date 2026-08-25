# V3 triplet relational-repair work-port Phi-v5 candidate v1

**Date:** 2026-08-24  
**Status:** **[SELECTION — PREPARED-SECTOR PHI-v5 CANDIDATE, NOT CANONICAL
PHI]** + **[THEOREM, CONDITIONAL — EXACT RELATIONAL-SYNDROME/A2-WORK
TRANSACTION]** + **[THEOREM — NONINJECTIVE IDENTITY EXPIRY WITH SURVIVING
CONSEQUENCE]** + **[OPEN — ACTION COEFFICIENT, WORK EXPORT/RESET, FORMATION,
GENERAL STABILITY, AND PHYSICAL SECTORS]**  
**Stability parent:**
[`THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md`](../constituent_complete_matter/THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md)  
**Energy boundary:**
[`THEOREM_V3_TRIPLET_SYNDROME_ENERGY_AND_ABSOLUTE_NORMALIZATION_BOUNDARY_v1.md`](../constituent_complete_matter/THEOREM_V3_TRIPLET_SYNDROME_ENERGY_AND_ABSOLUTE_NORMALIZATION_BOUNDARY_v1.md)  
**Exact certificate:**
[`proof_v3_triplet_relational_work_phi_v5_candidate.py`](../../../../../scripts/proofs/proof_v3_triplet_relational_work_phi_v5_candidate.py)

---

## 1. Why Phi-v4 is insufficient

The Phi-v4 event halo and the prepared cubic-triplet clock can detect and
correct one valid-symbol substitution, but their additive occupancy action is
flat on the error basin. If a positive relational syndrome energy is assigned,
repair must transfer that energy into a surviving physical record.

This candidate supplies exactly that finite transaction without adding a
carrier type or target value.

---

## 2. Pending herald and work ownership

Enlarge each of the three copied center herald registers from

\[
 \{D\}\sqcup\{q\}
\quad\hbox{to}\quad
 \{D\}\sqcup\{(q,e):q\in Q_{16},\ e\in\{0,1\}\}.  \tag{1}
\]

Each register therefore has 33 symbols. Three copies require 99 distinct
neutral controller pairs. After excluding the event center pad, charged-frame
fields, and event-herald namespace, every one of the 1,152 oriented charts has
exactly 170 available controllers in the selected intrinsic pool. The code
fits without a new type and closes 342,144 generator-covariance rows.

One existing A2 A9 slot carries one token at fixed occupancy. Relative to the
chart its states are

\[
 W_R=(k,\epsilon),\qquad
 W_E=(k+1\bmod4,\epsilon),                    \tag{2}
\]

called READY and EXCITED. Spatial signed-cubic transformations leave the
internal phase/polarity payload rule covariant.

---

## 3. Exact READ/COMMIT rule

On READ, the center majority-decodes the three arms. It sets `e=1` if any arm
differs from that majority or any dark herald copy is malformed. If `e=1` and
the work port is not READY, the transaction fails closed. Otherwise all three
herald copies retain `(q,e)`.

On COMMIT, every arm reads the retained majority herald. The transaction also
sets `e=1` if an arm or herald copy was perturbed after READ. It then performs

\[
 (q_1,q_2,q_3;(q,e)^3;W_R)
 \longmapsto
 (Uq,Uq,Uq;D^3;W_{R+e}),                     \tag{3}
\]

where `W_(R+0)=W_R` and `W_(R+1)=W_E`.

All reads and writes lie in the same radius-one oriented chart. The retained
herald prevents same-tick broadcast, and disjoint arms prevent endpoint write
conflicts.

---

## 4. Exhaustive recovery and work ledger

The larger herald alphabet gives the exact one-register mutation census

\[
 16\,[3(16-1)+3(33-1)]=2{,}256.              \tag{4}
\]

Every case rejoins the clean **body** orbit within at most two ticks and leaves
the work port EXCITED. The clean clock continues to recur while that record is
outstanding, but a second error cannot spend the same port: it fails closed.

Let `h` be the registered relational-syndrome quantum and `w` the work-port
excitation. Each repair obeys

\[
 \boxed{h_{\rm before}+w_{\rm before}
       =1+0=0+1
       =h_{\rm after}+w_{\rm after}.}         \tag{5}
\]

A common multiplier `epsilon_syn` may price both sides. Equation (5) fixes the
**relative** equality of syndrome and work quanta, but does not determine the
absolute multiplier.

The map is intentionally noninjective on error identity: different erroneous
phase/arm presentations can produce the same corrected body and generic
EXCITED record. The expired distinctions are not stored in an inverse tape.
Their physical consequence survives as the work excitation. This is the
finite ontological distinction between memory and consequence required by P5.

---

## 5. What is and is not closed

Closed for the selected prepared sector:

- one radius-one state-only relational repair transaction;
- exact recovery of all 2,256 one-register valid-symbol errors;
- constant A2 occupancy and exact dimensionless syndrome/work conservation;
- atomic backpressure against double spending;
- signed-cubic covariance; and
- physical work consequence without retaining error identity.

Still open:

- derivation of `epsilon_syn` from the common action/Hessian;
- causal propagation, absorption, or reset of the EXCITED work record;
- native formation of the triplet, 99-symbol herald namespace, and READY port;
- occupancy loss/gain, multiple errors, traffic, body overlap, and collisions;
- translation, recoil, binding energy, mass, and dispersion;
- integration with charged relaxation/static poles and Born apparatus traffic;
- universal tensor response, absolute coupling, common cone, and lensing; and
- promotion of any successor beyond canonical Phi-v2.

Thus the candidate closes a relative repair/work ledger, not physical coupling
normalization or stable matter as a whole.

---

## 6. Reproduction

```bash
python scripts/proofs/proof_v3_triplet_relational_work_phi_v5_candidate.py
```

Expected result: `12/12` checks pass.
