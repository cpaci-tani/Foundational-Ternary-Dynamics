# V3 triplet isotropic scalar-gravity Green bridge and tensor silence v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT TRIPLET REST-SOURCE DECOMPOSITION]** +
**[THEOREM — EXISTING-PACKET PURE-SCALAR CUBIC CYCLE]** +
**[THEOREM, CONDITIONAL — PREPARED SCALAR DIRICHLET GREEN SEAM]** +
**[BOUNDARY — STATIC VECTOR/STF SILENCE AND FREE RESPONSE RESIDUE]** +
**[OPEN — HOMOGENEOUS PHI, PROTECTED POLES, UNIVERSAL COUPLING, COMMON CONE,
LENSING, AND NONLINEAR GRAVITY]**  
**Carrier price:** six signed-cubic images of one existing neutral
scalar/vector/STF packet; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Matter parent:**
[`THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md`](../constituent_complete_matter/THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md)  
**Bundle parent:**
[`THEOREM_V3_NEUTRAL_SCALAR_VECTOR_STF_BUNDLE_AND_COMMON_GREEN_SEAM_v1.md`](THEOREM_V3_NEUTRAL_SCALAR_VECTOR_STF_BUNDLE_AND_COMMON_GREEN_SEAM_v1.md)  
**Exact certificate:**
[`proof_v3_triplet_isotropic_scalar_gravity_green_bridge.py`](../../../../../scripts/proofs/proof_v3_triplet_isotropic_scalar_gravity_green_bridge.py)

---

## 1. What is proved

The cubic triplet theorem gives the clean-orbit mean capacity deficit

\[
 D_{\rm cap}=-{I_3\over36}.                           \tag{1}
\]

This document defines the corresponding **positive capacity-rest readout** as

\[
 R_{\rm rest}:=-D_{\rm cap}={I_3\over36}.             \tag{2}
\]

Equation (2) is a finite-state readout selected from an already proved tensor.
It is not yet an identification with measured gravitational mass. Its exact
scalar/vector/STF decomposition is

\[
 \rho_{\rm rest}=\operatorname{tr}R_{\rm rest}={1\over12},
 \qquad
 R_{\rm rest}^{\rm STF}=0,
 \qquad
 j_{\rm rest}=0.                                     \tag{3}
\]

For every signed-cubic matrix `Q in O_h`,

\[
 Q R_{\rm rest}Q^T=R_{\rm rest}.                     \tag{4}
\]

Thus the prepared triplet's static isotropic readout is an exact scalar
source. It cannot carry a preferred vector direction or a static
trace-free tensor source.

---

## 2. A pure scalar source cycle from existing packets

The joint bundle parent reads each neutral packet as

\[
 {\cal P}=(1,\operatorname{STF}_5,v_3)\in\mathbb Z^9. \tag{5}
\]

Choose the certificate's first admissible pair of distinct internal clock
orbits and transform it under all 48 signed-cubic matrices. The orbit contains
six distinct physical payload rows:

\[
\begin{aligned}
 &(1,-4, 2,0,0,0,-1,0,0),\\
 &(1,-4, 2,0,0,0, 1,0,0),\\
 &(1, 2,-4,0,0,0,0,-1,0),\\
 &(1, 2,-4,0,0,0,0, 1,0),\\
 &(1, 2, 2,0,0,0,0,0,-1),\\
 &(1, 2, 2,0,0,0,0,0, 1).
\end{aligned}                                        \tag{6}
\]

They obey the exact cancellation identity

\[
 \boxed{\sum_{a=1}^6{\cal P}_a=(6,0,0,0,0,0,0,0,0).} \tag{7}
\]

No desired field value was inserted to choose these rows: they are the six
distinct images of one registered packet. Averaging equation (7) and applying
the triplet source factor gives

\[
 {1\over12}{1\over6}\sum_{a=1}^6{\cal P}_a
 =({1\over12},0,0,0,0,0,0,0,0).                     \tag{8}
\]

This is an explicit finite-carrier realization of equation (3). It closes
the representation and cancellation seam, not the native formation,
injection, or renewal of the six-packet cycle.

---

## 3. Exact finite-domain scalar response

Let `L_D` be the positive Dirichlet cubic Laplacian on a finite box and
`delta_s` a unit source at the central site. With

\[
 G_D=L_D^{-1}\delta_s,                                \tag{9}
\]

the triplet response is exactly

\[
 {\cal H}_D
 =G_D({1\over12},0,\ldots,0),
 \qquad
 L_D{\cal H}_D
 =\delta_s({1\over12},0,\ldots,0).                  \tag{10}
\]

Every vector and STF entry in equation (10) vanishes identically. Applying
the common bundle kernel packetwise to equation (7) gives the same response,
so the cancellation is preserved by the finite Green operator rather than
being only a source-level identity.

The parent deterministic rotor histories give

\[
 \left\|L_DG_N-\delta_s\right\|_\infty\le {8\over N}.
\]

Multiplication by the triplet scalar `1/12` therefore yields the exact bound

\[
 \boxed{
 \left\|L_DH_N-{1\over12}\delta_s\right\|_\infty
 \le {2\over3N}.}                                   \tag{11}
\]

In the controlled large-domain readout, with

\[
 \Lambda(k)=6-2(\cos k_x+\cos k_y+\cos k_z),         \tag{12}
\]

the prepared static response conditionally has the scalar pole

\[
 \boxed{H(k)={{1/12}\over\Lambda(k)}.}               \tag{13}
\]

Equation (13) is a blocked-history Green seam. It is not an autonomous
dynamical field mode and is not yet a physical Newtonian limit.

---

## 4. Tensor silence is a result, not a failure

At rest, the triplet source is exactly isotropic. Representation theory then
forces the vector and rank-five STF coordinates to vanish. This establishes a
useful separation:

```text
static triplet capacity-rest scalar:       present exactly
existing pure-scalar packet cycle:         present exactly
prepared scalar Green history:             present conditionally
static vector source:                       absent exactly
static STF source:                          absent exactly
motion-generated vector/STF stress:         open
protected tensor radiation:                 open
```

A spin-2 claim cannot be extracted from the static triplet alone. Nonzero
vector and STF gravity sources must arise from relational motion, stress,
anisotropy, or composite interactions and then survive the constraint and
protection gates. This is the discrete analogue of not confusing rest density
with the full stress tensor.

The
[`discrete-motion successor`](THEOREM_V3_TRIPLET_DISCRETE_MOTION_MOMENT_GRAVITY_LIFT_AND_RELATIVE_NORMALIZATION_BOUNDARY_v1.md)
now closes the prepared representation part of that statement. Conditional on
an admitted SC chord `u`, exact cubic symmetry leaves the shapes `1`, `u`, and
`3uu^T-I`; the selected minimal chord moment is realized by exactly two
existing joint packets and inherits the common history kernel. It does not
construct the translating triplet transaction, derive physical stress, or
protect the response.

The
[`finite A2-memory successor`](THEOREM_V3_ROTOR_GREEN_A2_PHYSICAL_MEMORY_AND_PHASE_PROTECTION_BOUNDARY_v1.md)
now closes a narrower protection statement. It retains the rotor current and
source count in fixed-occupancy existing A2 states and proves, for edge
transfer norm `K_e`,

\[
 |J_N-\nabla G_D|\le {8\over3N}+{8K_e\over N}.
\]

All 192 native uniform initial phases satisfy the exact bound at the certified
radius-one apparatus. The triplet `1/12` source inherits it. This is physical
finite memory plus initial-phase protection, not traffic protection,
mechanical backreaction, or an action-fixed pole.

---

## 5. Why the gravitational normalization remains free

The triplet fixes the dimensionless source shape and the bundle fixes the
relative component coordinates. Neither fixes the coefficient multiplying
the response action. For any positive `g_R`,

\[
 H_{g_R}(k)=g_R{{1/12}\over\Lambda(k)}                \tag{14}
\]

uses the same packets, local transitions, cubic covariance, and finite-history
bound. The certificate checks distinct examples `g_R=1,2,7`; nothing in the
construction selects among them.

Consequently, this theorem does **not** derive Newton's constant, universal
free fall, a shared electromagnetic/gravitational normalization, or the
coupling of radiation and matter to one metric response. Calling equation
(2) “mass” before that reciprocal action is proved would promote a selected
readout into a physical identification.

---

## 6. Remaining gravity closure chain

The result narrows the gravity programme to six concrete debts:

1. integrate source formation, six-packet injection, absorption, and renewal
   into one state-complete homogeneous `Phi`;
2. derive a positive dynamical Hessian and protect the static constraint pole;
3. generate vector and STF stress from moving matter under the same action;
4. derive one universal reciprocal source-response coefficient and absolute
   normalization for matter and radiation;
5. recover a common causal cone, local clock response, Shapiro delay, and
   lensing; and
6. obtain nonlinear self-coupling only after the microscopic spin-2 input is
   established.

The Deser bootstrap can complete an already present massless spin-2 field; it
cannot supply steps 1--4.

---

## 7. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_triplet_isotropic_scalar_gravity_green_bridge.py
```

Expected result: `13/13` exact checks pass, with source tensor `I/36`, scalar
coordinate `1/12`, zero vector/STF source, six distinct cubic packet rows,
finite-history residual bound `2/(3N)`, conditional pole
`(1/12)/Lambda`, and free absolute gravity residue.
