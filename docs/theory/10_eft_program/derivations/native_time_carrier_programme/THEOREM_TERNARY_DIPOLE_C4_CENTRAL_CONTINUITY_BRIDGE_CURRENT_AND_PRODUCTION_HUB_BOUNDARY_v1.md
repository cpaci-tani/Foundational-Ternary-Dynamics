# Theorem — Ternary-dipole `C4` central-continuity bridge current and production-hub boundary v1

**Identifier:** `FTD-0924`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT COMPACT CENTRAL-CONTINUITY CURRENT]` +
`[REFERENCE CONSTRUCTION — CONTINUITY-COMPATIBLE EVANESCENT C4 FIELD ORBIT]` +
`[THEOREM — ZERO IDEAL FIELD/INTERACTION/MATTER WORK]` +
`[SCOPED NO-GO — ENDPOINT-SUPPORTED LIVE j=s v]` +
`[CLOSED NEGATIVE — ONE-SITE UNIT-TICK TERNARY HUB UNDER PRODUCTION BANDWIDTH]` +
`[OPEN — CAUSAL DISTRIBUTED BRIDGE OR INDEPENDENT CURRENT TYPE]`

## 1. Result

The four FTD-0923 ternary-dipole snapshots possess an exact compact current
under the unchanged central divergence. The paired transition is special:
the two counter-moving polarities cancel every checkerboard/parity
obstruction that forbids a single cardinal hop.

For

\[
 s_0=\delta_{e_x}-\delta_{-e_x},
 \qquad s_n=S^ns_0,
\]

the exact integrated current is

\[
 \boxed{
 Q_0=2(e_y-e_x)\delta_0,
 \qquad Q_n=S^nQ_0,}
\]

and it satisfies

\[
 \boxed{s_{n+1}-s_n+\operatorname{div}_cQ_n=0}
\]

on all four arms.

This advances FTD-0923 substantially: the localized field clock no longer
requires continuity-violating source snapshots at the reference level. Its
current curl can be included, the exact evanescent `C4` recurrence survives,
and the FTD-0576 conditional Hodge ledger assigns zero field, interaction,
and matter-reaction work to every ideal arm.

The result does **not** yet produce a live clock. The successful current is
concentrated at the rotation center, where every two-site dipole snapshot is
void. No current supported on the manifested endpoints, even allowing
arbitrary real vectors, solves the central continuity equation. A diagnostic
ternary center hub can carry the current algebraically, but its unit-tick
speed is `2 sqrt(2)`, exceeding the selected production speed ceiling
`1/sqrt(3)`, and its fixed manifestation adds a static electric source.

The exact missing dynamics is therefore a causal distributed bridge
scaffold, or an explicitly priced independent current/connection type. It is
not an unspecified maintenance-energy term.

## 2. Why the paired transition is locally soluble

Use the production central derivative

\[
 D_if(x)={f(x+e_i)-f(x-e_i)\over2}.
\]

The first transition is

\[
 \delta s_0=s_1-s_0
 =\delta_{e_y}-\delta_{-e_y}
  -\delta_{e_x}+\delta_{-e_x}.
\]

Both `+e_x` and `-e_x` occupy the same site-parity class, and their
coefficients in `delta s_0` sum to zero. The same is true of the two `y`
sites. Consequently all eight parity characters vanish:

\[
 \sum_x(-1)^{\epsilon\cdot x}\delta s_0(x)=0,
 \qquad \epsilon\in\{0,1\}^3.
\]

The certificate verifies the same statement after every quarter-turn. This
is exactly what a one-site cardinal hop fails in FTD-0576.

The central divergence of a point vector is

\[
 \operatorname{div}_c\!\left[(a,b,0)\delta_0\right]
 =-{a\over2}s_0-{b\over2}s_1.
\]

Setting `(a,b)=(-2,2)` gives

\[
 \operatorname{div}_cQ_0=s_0-s_1=-\delta s_0.
\]

Rotation covariance gives the remaining three arms. Explicitly,

\[
 Q_1=-2(e_x+e_y)\delta_0,
 \qquad Q_2=-Q_0,
 \qquad Q_3=-Q_1.
\]

Thus the current itself carries a handed order-four phase:

\[
 Q_{n+1}=SQ_n,
 \qquad Q_{n+2}=-Q_n.
\]

## 3. Exact current curl

For `Q_n=(a_n,b_n,0)delta_0`, direct central differentiation gives

\[
 \boxed{
 \operatorname{curl}_cQ_n=
 \left(-b_nD_z\delta_0,
        a_nD_z\delta_0,
        b_nD_x\delta_0-a_nD_y\delta_0\right).}
\]

It is supported on six sites: the two sites on each of the `x`, `y`, and `z`
axes adjacent to the origin. Exact finite sums give

\[
 \boxed{
 \|Q_n\|_2^2=8,
 \qquad
 \|\operatorname{curl}_cQ_n\|_2^2=8.}
\]

It obeys

\[
 \operatorname{div}_c\operatorname{curl}_cQ_n=0
\]

and rotates covariantly. The curl therefore adds the required transverse
production-shaped source without disturbing continuity.

## 4. Continuity-compatible Hodge source

FTD-0576's exact finite-step energy theorem uses the endpoint midpoint

\[
 \bar s_n={s_n+s_{n+1}\over2}
\]

and the integrated current `Q_n`. Define

\[
 q_n=\nabla_c\bar s_n-\operatorname{curl}_cQ_n,
 \qquad
 U_n=-q_n.
\]

The midpoint is an arithmetic ledger coordinate, not a fourth actual state
value. Since central gradients are orthogonal to central curls on finite
support,

\[
 \left\langle\nabla_c\bar s_n,
 \operatorname{curl}_cQ_n\right\rangle=0.
\]

The exact source norms are

\[
 \|\nabla_c\bar s_n\|_2^2={7\over4},
 \qquad
 \boxed{\|q_n\|_2^2={39\over4}}.
\]

These are uncontained finite-support identities. The separate `L=4`
periodic witness is used only for the exact rational field recurrence.

## 5. The evanescent `C4` orbit survives the current curl

At the exact order-four stiffness `kappa=2`, define

\[
 F_n=(2I-K)^{-1}q_n.
\]

FTD-0923 already proved that `2` lies above the C18 free band `[0,16/9]`
and that every compact source obeys

\[
 \|P_{\ge r}F_n\|_2
 \le {9\over2}\left({8\over9}\right)^r\|q_n\|_2.
\]

The new source satisfies

\[
 q_{n+1}=Sq_n,
 \qquad q_{n+2}=-q_n.
\]

Because the resolvent commutes with the cubic rotation,

\[
 F_{n+1}=SF_n,
 \qquad F_{n+2}=-F_n.
\]

Moreover,

\[
 U_n=(K-2I)F_n.
\]

With

\[
 P_n=F_n+F_{n+1},
\]

the same exact kick--drift calculation as FTD-0923 gives

\[
 P_{n+1}=P_n-KF_n+U_n,
 \qquad
 F_{n+1}=F_n+P_{n+1}.
\]

The current-corrected source therefore retains the exact localized
source-locked `C4` field body.

## 6. Exact ideal reaction and work

Use the unique FTD-0576 work coordinate

\[
 R_n=F_n-{1\over2}P_n.
\]

On this orbit,

\[
 R_{n+1}=SR_n,
 \qquad
 \delta R_n=F_{n+1},
 \qquad
 \bar R_n={1\over2}F_n.
\]

The exact field work is

\[
 \Delta H_f=\langle U_n,\delta R_n\rangle=0,
\]

because the self-adjoint radial return is orthogonal to the rotated field
increment.

The endpoint interaction energy is

\[
 U_{\rm int}(s_n,R_n)=-\langle s_n,DR_n\rangle.
\]

Both arguments advance by the same orthogonal rotation and `D` is covariant,
so `U_int` is exactly constant. Therefore

\[
 \Delta U_{\rm int}=0.
\]

Finally the FTD-0576 matter-reaction target is

\[
 \Delta H_m=
 \left\langle Q_n,
 GD\bar R_n-C\delta R_n\right\rangle.
\]

The conditional total-energy identity gives, and the exact rational witness
checks independently,

\[
 \boxed{
 \Delta H_f=\Delta U_{\rm int}=\Delta H_m=0}
\]

on each of the four arms.

This is a lossless ideal rotation, not a free creation mechanism. It says the
registered path need not consume maintenance work once present. It does not
supply a positive source Hamiltonian, choose the state transition, form the
clock, or restore it after perturbation.

## 7. Why the current is not existing two-site matter motion

The nonzero sites of `s_n` occupy one parity class; the union of the nonzero
sites of `s_n` and `s_{n+1}` occupies the two axial parity classes in the
orbit plane. Central differentiation flips exactly one parity bit. Therefore
the divergence of any current supported on either endpoint set lies in
parity classes disjoint from the support classes of `delta s_n`.

The exact coefficient systems confirm this for all eight cases:

- current on the two currently occupied endpoints; and
- current on the four-site union of current and next endpoints,

for each of the four transitions.

Hence

\[
 \boxed{
 \nexists v_n:\quad
 s_{n+1}-s_n+\operatorname{div}_c(s_nv_n)=0}
\]

when `s_n v_n` is restricted to the two actual sites. Allowing arbitrary real
endpoint velocities does not help.

The successful `Q_n` lives at the origin, while

\[
 s_n(0)=0
\]

for every snapshot. Thus the unchanged production product `s_n v_n`
vanishes exactly where the required bridge current lives.

This is the concrete content of “something more than matter” in the present
minimal model: a central relation/current between the two manifested arms is
required. The theorem exposes that role; it does not yet adopt a new type for
it.

## 8. The minimal ternary-hub control fails production admissibility

Add a fixed central manifested site

\[
 h=\eta\delta_0,
 \qquad \eta\in\{-1,+1\}.
\]

Then the three-site ternary state `s_n+h` can reproduce the point current by
setting

\[
 v_n(0)={Q_n(0)\over\eta}.
\]

This is an exact algebraic witness that a manifested bridge site is sufficient
to compile the current through `j=s v`. But in one production tick,

\[
 |v_n(0)|^2=8,
 \qquad
 |v_n(0)|=2\sqrt2.
\]

The selected flat production bandwidth permits only

\[
 |v|<{1\over\sqrt3},
\]

with an even smaller bound when latency is nonzero. The movement entry
projects an over-budget velocity back inside that open cone, changing `Q_n`
and breaking exact continuity. Therefore the one-site unit-tick hub is closed
negative under the unchanged production bandwidth.

It also changes the source independently of speed:

\[
 \|\nabla_ch\|_2^2={3\over2}.
\]

This gradient is rotation invariant, not antipodal. It adds a static electric
source to the `C4` doublet and cannot be inserted into the FTD-0923 orbit
without solving a separate static response.

This does not rule out a larger manifested scaffold, a multi-tick transition,
or an independent link/current variable.

## 9. What the result means

The source problem has split into a clean three-level hierarchy:

1. **Continuity:** solved exactly by the compact point bridge current.
2. **Ideal field/reaction energy:** solved conditionally and losslessly by the
   midpoint Hodge source.
3. **Production realization:** still open because the bridge current is
   void-centered, endpoint matter cannot carry it, and the one-site manifested
   hub violates bandwidth and changes the electric source.

The most economical existing-ontology successor is now a finite manifested
scaffold on the allowed parity classes whose bounded site currents sum to the
same divergence while keeping the full source in an order-four doublet. The
next exact test should minimize its radius, manifested-site count, peak
speed, and static-source residue. If no finite causal scaffold exists under a
locked radius ladder, the independent current/connection branch becomes the
honest type-priority candidate and must be priced before adoption.

`G*` remains downstream. No lemniscatic constant enters the continuity
current, the curl source, the resolvent, or the work cancellation.

## 10. Epistemic boundary

This theorem does not derive autonomous source dynamics, a source Hamiltonian,
positive storage, switching logic, formation, reset, perturbation recovery,
mobility, physical scale, a `G*` gearbox, gamma, Born frequencies, Bell
correlations, measurement context, or preferred-tick hiding. It adopts no
independent current type and changes no engine source, CMake target, toggle,
default, import, or selected type.

The midpoint source is the exact FTD-0576 energy-centering reference. The
unchanged production source reads the same-tick `s` and `s v`; their
identification is not silently assumed.

## 11. Verification

The locked preregistration is
`PREREG_TERNARY_DIPOLE_C4_CENTRAL_CONTINUITY_CURRENT_AND_HUB_BOUNDARY_v1.md`,
SHA-256
`9D46FD21080BFF3218E690CAED22A04B8555D5FF1EE1A95DD199C90E8B7A6425`.

The exact certificate is
`scripts/proofs/proof_ternary_dipole_c4_central_continuity_current_hub_boundary.py`,
SHA-256
`872EF5FAD66E3020A1586F7C0BD66E175ED2B3A38AE5BFB2D420443402FC40E2`.

It passes `138/138` gates and reports

```text
OUTCOME=A_COMPACT_BRIDGE_CURRENT_WITH_PRODUCTION_CARRIER_BOUNDARY
CENTRAL_CONTINUITY=EXACT_ALL_FOUR_ARMS
CURRENT_SUPPORT=VOID_ROTATION_CENTER
CURRENT_NORM_SQUARED=8
CURRENT_CURL_SUPPORT=6
CURRENT_CURL_NORM_SQUARED=8
MIDPOINT_HODGE_SEED_NORM_SQUARED=39/4
CONTINUITY_COMPATIBLE_C4_FIELD_ORBIT=EXACT
FIELD_INTERACTION_MATTER_WORK=ZERO_EACH_TICK
ENDPOINT_SUPPORTED_SV_CURRENT=IMPOSSIBLE
UNIT_TICK_TERNARY_HUB_SPEED_SQUARED=8_GT_1/3
INDEPENDENT_CURRENT_TYPE_ADOPTED=FALSE
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```
