# V3 translating self-correcting triplet clock Phi-v6 candidate and inertia boundary v1

**Date:** 2026-08-24  
**Status:** **[SELECTION — PREPARED PHI-v6 TRANSLATION CANDIDATE]** +
**[THEOREM, CONDITIONAL — HOMOGENEOUS RADIUS-ONE ONE-HOP TRANSLATION]** +
**[THEOREM — EXACT TRANSLATING CLEAN-SECTOR BIJECTION]** +
**[THEOREM — COMPLETE MOVING PHI-v5 SUBSTITUTION REPAIR/WORK BASIN]** +
**[BOUNDARY — ADDITIVE TRANSLATION WORK ZERO; INERTIA AND DISPERSION OPEN]** +
**[OPEN — NATIVE FORMATION, COLLISION ARBITRATION, CANONICAL PHI,
PACKET EMISSION, PHYSICAL MASS, AND GENERAL STABILITY]**  
**Carrier price:** existing cubic triplet, three copied 33-symbol Phi-v5
pending registers, one existing A2 READY/EXCITED work token, and the existing
polar repair header; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Repair/work parent:**
[`THEOREM_V3_TRIPLET_RELATIONAL_REPAIR_WORK_PORT_PHI_v5_CANDIDATE_v1.md`](../common_action_mechanics_reciprocity/THEOREM_V3_TRIPLET_RELATIONAL_REPAIR_WORK_PORT_PHI_v5_CANDIDATE_v1.md)  
**Motion-source successor:**
[`THEOREM_V3_TRIPLET_DISCRETE_MOTION_MOMENT_GRAVITY_LIFT_AND_RELATIVE_NORMALIZATION_BOUNDARY_v1.md`](../gravity_cosmology/THEOREM_V3_TRIPLET_DISCRETE_MOTION_MOMENT_GRAVITY_LIFT_AND_RELATIVE_NORMALIZATION_BOUNDARY_v1.md)  
**Reciprocal motion/work successor:**
[`THEOREM_V3_TRIPLET_MOTION_PACKET_WORK_RECIPROCAL_PHI_v7_CANDIDATE_AND_RELEASE_BOUNDARY_v1.md`](../common_action_mechanics_reciprocity/THEOREM_V3_TRIPLET_MOTION_PACKET_WORK_RECIPROCAL_PHI_v7_CANDIDATE_AND_RELEASE_BOUNDARY_v1.md)  
**Exact certificate:**
[`proof_v3_triplet_translating_self_correcting_clock_phi_v6_candidate.py`](../../../../../scripts/proofs/proof_v3_triplet_translating_self_correcting_clock_phi_v6_candidate.py)

---

## 1. Why translation was not automatic

The cubic triplet parent proves a localized period-16 clock and a complete
one-substitution correction basin. It does not follow that the complete
object can move under P4. A valid translating successor must establish:

1. a direction stored in existing physical state;
2. one-tick output support inside the Moore causal neighborhood;
3. no source/output writer collision;
4. homogeneous and signed-cubic-covariant admission;
5. compatibility with the Phi-v5 pending-register and A2 work mechanism; and
6. correction while the object's center changes.

The existing oriented repair chart already contains a polar axis `n_C`. No
new direction bit is introduced. This candidate selects forward motion along
that carried axis.

---

## 2. The one-sided arm geometry

In chart coordinates `(e_1,e_2,n_C)`, the three triplet arm tails are

\[
 (-1,-1,-1),\qquad(1,-1,0),\qquad(0,1,-1),           \tag{1}
\]

and their edge directions are respectively `e_1,e_2,n_C`. Every source
endpoint lies in the center's closed Moore cube.

Translate the complete chart by

\[
 x_C\longmapsto x_C+n_C.                             \tag{2}
\]

Because equation (1) occupies only normal layers `-1` and `0`, every output
endpoint after equation (2) lies in normal layers `0` or `1`. Therefore every
output relation owner and the new center are still within Chebyshev distance
one of the prior center:

\[
 \boxed{
 \|x_{\rm output}-x_C\|_\infty\le1.}                \tag{3}
\]

The three translated relation keys remain distinct and are disjoint from all
three source relation keys. The certificate verifies equations (2)--(3) on
all 1,152 oriented charts.

Conditional on the parent's globally unique prepared center/header code, a
radius-one rule can therefore distinguish every relative output site and read
the prior chart state. This is the precise locality reason the selected
translation works; compactness alone would not have sufficed.

---

## 3. Phi-v6 prepared-sector map

Let `C` be an admitted oriented chart and `X` its Phi-v5 work-clock state:

\[
 X=(a_1,a_2,a_3;h_1,h_2,h_3;w),                     \tag{4}
\]

where the `a_i` are existing A9 arm states, the `h_i` are copied symbols from
the 33-state alphabet

\[
 \{\mathrm{DARK}\}\cup
 \{(q,\sigma):q\in A9_{\rm valid},\ \sigma\in\{0,1\}\},
\]

and `w` is one A2 READY/EXCITED token.

Define the candidate map

\[
 \boxed{
 \Phi_6(C,X)=
 \bigl(C+n_C,\ \Phi_{5,\rm work}(C,X)\bigr).}        \tag{5}
\]

Equation (5) shifts the owners of all three arms, all six neutral field
records in the copied pending registers, and the A2 work owner by one SC hop.
It simultaneously applies the exact Phi-v5 READ/COMMIT/work transaction.

The relative rule is independent of `x_C`. Translating a chart by an arbitrary
integer vector before equation (5) only translates the output by the same
vector. Applying any `Q in O_h` before equation (5) gives the transformed
chart, transformed register code, transformed arm support, and displacement
`Qn_C`. The certificate checks all 48 signed-cubic matrices and every clean
clock phase.

This is a prepared-sector extension and is not promoted into canonical
state-complete `Phi`. Blank, malformed, overlapping, or contested
neighborhoods still require an integrated fallback/arbitration proof.

---

## 4. Clean translating clock

The internal Phi-v5 clean state has period sixteen. Iterating equation (5)
gives

\[
 x_C(n)=x_C(0)+n n_C,
 \qquad
 X(n+16)=X(n).                                       \tag{6}
\]

Thus the local internal clock recurs while the object's absolute lattice
position does not. The center advances exactly one SC hop per global tick:

\[
 \boxed{
 \|x_C(n+1)-x_C(n)\|_\infty=1.}                    \tag{7}
\]

Equation (7) saturates P4's causal ceiling and never exceeds it.

On the clean sector, the predecessor is explicit: move the output chart one
hop along `-n_C` and replace its internal state by the preceding point of the
finite period-16 orbit. The certificate verifies this inverse on

\[
 1,152\times16=18,432
\]

chart-phase states. Clean translation is therefore a finite bijection even
though error repair remains noninjective.

---

## 5. Correction and work while moving

The Phi-v5 registered perturbation class contains every one-register valid
substitution in any of the three arms or three 33-symbol pending registers at
every clean phase. There are 2,256 exact cases.

For each perturbed state, compare equation (5) with the translating clean
trajectory. The certificate proves

\[
 \boxed{
 \text{every registered body rejoins within at most two global ticks}.} \tag{8}
\]

Every repair also retains the Phi-v5 relational consequence

\[
 \boxed{
 h_{\rm syndrome}+w:\quad1+0\longmapsto0+1,}         \tag{9}
\]

so the A2 port changes READY to EXCITED. Two distinct error identities still
map to the same corrected moving body and same generic work record. Detailed
error identity expires; its finite work consequence survives.

This corrects an important composition hazard: moving the older 17-symbol
triplet without the Phi-v5 work port would have silently discarded equation
(9). Phi-v6 is defined on the actual 33-symbol/A2 state instead.

---

## 6. Exact occupancy and the inertia boundary

Every clean moving state has role occupancy

\[
 \boxed{
 (N_F,N_{A1,SC},N_{A1,FCC},N_{A2})=(6,3,0,1).}       \tag{10}
\]

The six field records are three opposite-polarity pairs and have exact zero
additive `E/B` on every C3 layer. Every A9 arm has one token and zero net
ternary charge. Translation preserves equation (10) exactly.

Consequently every homogeneous phase-blind additive occupancy action assigns

\[
 \boxed{\Delta H_{\rm occ}=0}                        \tag{11}
\]

to a clean move. Equation (11) proves that the candidate has kinematic motion
but does not derive positive translation work, inertia, rest mass, or a
dispersion relation. Those require relational action curvature or a clock
debit that changes under motion. Counting an unchanged object at two positions
cannot supply that physics.

---

## 7. Gravity connection

Equation (7) supplies exactly the finite chord required by the motion-source
theorem. Conditional on the selected minimal chord moment, every translating
chart therefore has source-coordinate shape

\[
 {\cal S}_{\rm move}(n_C)
 ={1\over12}(1,3n_Cn_C^{\mathsf T}-I,n_C).           \tag{12}
\]

This closes the former purely hypothetical displacement premise at the
prepared local-candidate level. The Phi-v7 successor now composes the clean
hop with a physical neutral request, an explicit 24-occupancy A2 reserve, the
theorem-minimum two-packet motion source, READY-to-EXCITED work, and exact
reciprocal absorption. That successor is a lean prepared vertex; persistent
refill/reset, full event-halo integration, and protected field response remain
open.

---

## 8. Exact boundary after Phi-v6

```text
prepared one-hop triplet translation:          exact candidate
clean translating inverse:                     exact
moving one-substitution correction:            exact (2,256 cases)
moving repair work consequence:                exact
spatial homogeneity and O_h covariance:         exact
native chart/header formation:                  open
multi-object destination/collision arbitration: open
positive binding and translation work:          open
inertia, mass, and dispersion:                   open
motion-source packet emission/absorption:        exact Phi-v7 lean successor
canonical state-complete Phi integration:        open
protected gravity and universal response:        open
```

The construction advances stable matter from “stationary correcting clock” to
“prepared translating correcting clock.” It does not yet establish a freely
propagating particle or a long-lived object in unprepared environmental
traffic.

---

## 9. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_triplet_translating_self_correcting_clock_phi_v6_candidate.py
```

Expected result: `15/15` exact checks pass, with 1,152 local geometry rows,
18,432 clean inverse rows, all 2,256 Phi-v5 substitutions corrected within two
ticks while translating, exact READY-to-EXCITED work retention, occupancy
`(6,3,0,1)`, and zero clean additive translation work.
