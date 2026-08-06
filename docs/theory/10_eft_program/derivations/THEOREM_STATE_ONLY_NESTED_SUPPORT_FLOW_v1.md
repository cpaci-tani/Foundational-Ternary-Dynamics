# State-only nested-support projection and dressing-energy flow

**Status:** `[THEOREM — CONDITIONAL ON THE SELECTED FINITE-SUPPORT GAUSS
PROBLEMS] + [CONSTRUCTIVE NUMERICAL FACT — EXISTING DISCOVERY CORPUS ONLY] +
[ONTOLOGICAL CORRECTION — SUPPORT IS A RESOLUTION SCALE]`  
**Date:** 2026-07-30  
**Bookkeeping:** FTD-0754 analytic addendum “FTD-0754C”; no new ledger
identifier  
**Protocol:** `PREREG_STATE_ONLY_SUPPORT_LADDER_v1.md`  
**Certificate:** `scripts/proofs/proof_state_only_support_ladder.py` —
1205/1205 checks

## 1. Verdict

The compact Gauss support used by the FTD-0754 observer is not a uniquely
selected material boundary. The family of minimum-energy dressings on nested
supports is instead an exact orthogonal-projection ladder.

For support half-widths (R<S), let (b_R) and (b_S) be the selected
minimum-energy primitive face fields. Then

\[
\boxed{
U_R=U_S+\frac12\lVert b_R-b_S\rVert^2,
\qquad U_R=\frac12\lVert b_R\rVert^2.
}
\]

Thus dressing energy decreases monotonically as the bookkeeping support is
enlarged. The decrement is not lost: it is exactly the squared norm of the
field component relaxed by the larger support.

For any actual primitive field (E), every support also gives the exact
accounting identity

\[
\frac12\lVert E\rVert^2
=U_R+\frac12\lVert E-b_R\rVert^2
 +\langle b_R,E-b_R\rangle.
\]

The actual state and its total field energy are support-independent; the
assignment among dressing, residual, and interaction runs with (R).

## 2. Proof

Let (K_R\subset K_S) be the nested induced cubic support graphs, with
internally oriented face spaces (F_R\subset F_S). Let

\[
\mathcal A_R=\{e\in F_R:D_Re=\rho\}
\]

denote the nonempty affine space satisfying the same neutral compact source
and zero boundary-crossing condition. Extend every field in (F_R) by zero
on (F_S\setminus F_R). The zero-crossing condition makes this extension
divergence-free at the old boundary, so

\[
\mathcal A_R\subset\mathcal A_S.
\]

The strictly convex primitive energy has one minimizer

\[
b_R=\operatorname*{argmin}_{e\in\mathcal A_R}
       \frac12\lVert e\rVert^2,
\qquad
b_S=\operatorname*{argmin}_{e\in\mathcal A_S}
       \frac12\lVert e\rVert^2.
\]

Because (b_R,b_S\in\mathcal A_S), their difference

\[
d=b_R-b_S
\]

lies in the homogeneous tangent space (ker D_S). First-order optimality of
the minimum-norm point in (mathcal A_S) gives

\[
\langle b_S,d\rangle=0.
\]

Therefore

\[
\lVert b_R\rVert^2
=\lVert b_S+d\rVert^2
=\lVert b_S\rVert^2+\lVert d\rVert^2,
\]

which proves the boxed projection identity and (U_R\ge U_S\).

The actual-field decomposition follows by setting (r_R=E-b_R) and
polarizing the quadratic norm:

\[
\tfrac12\lVert b_R+r_R\rVert^2
=\tfrac12\lVert b_R\rVert^2
 +\tfrac12\lVert r_R\rVert^2
 +\langle b_R,r_R\rangle.
\]

No field evolution equation, trajectory label, future state, or physical
constant is used in either proof. The result is conditional on the selected
finite-support Gauss spaces; it does not prove that this projector is forced
by the five postulates.

## 3. Registered existing-corpus result

The locked (R=\{4,6,8\}) observer replayed the exact FTD-0753/0754
`L=321` face, edge, and body histories at the eight existing observer ticks.
No new perturbation, volume, direction, or state was generated.

Results:

- old scalar history replay: 939/939 exact;
- support-scale rows valid: 72/72;
- adjacent nested transitions valid: 48/48;
- maximum actual-energy reconstruction residual: `2.158e-17`;
- maximum projection inner product: `4.440e-16`;
- maximum Pythagorean residual: `4.405e-16`;
- minimum positive adjacent energy decrement: `7.557e-5`;
- radius-4 to radius-8 dressing-energy flow: `1.811%` to `2.568%` of
  radius-4 dressing energy over the 24 snapshots;
- independent certificate: 1205/1205.

The separately computed radius-four primitive cross term agrees with the
FTD-0754B observer to maximum absolute difference `7.589e-19`. The values are
not byte-identical because the two deterministic observers use different face
summation orders. This bounded arithmetic difference is far below the locked
`1e-12` gate and changes no state or verdict.

The percentage interval is a descriptive property of this discovery corpus,
not a constant, tolerance, or FTD-0755 success threshold.

## 4. Corrected matter interpretation

The result rules out the following literal picture:

> matter owns one sharply bounded flux cloud whose energy is fixed by the
> radius-four observer.

The narrower supported picture is:

1. the relational constituent core is the candidate scale-independent object
   feature;
2. the Gauss dressing is a deterministic conditional field assignment at a
   declared resolution scale;
3. dressing, residual, and primitive boundary interaction energies run with
   that scale while their exact sum remains fixed;
4. centered electric/magnetic readout corrections remain a separate observer
   layer under FTD-0754B;
5. no tested support surface is a physical membrane unless an independent
   state variable, stress law, and dynamics for such a membrane are supplied.

This resembles a hierarchy of open-system descriptions, not a collection of
literal nested shells. A larger support assigns more of the constraint field
to the dressing. The orthogonal decrement guarantees that changing the
description does not manufacture energy.

## 5. Consequence for the matter predicate

FTD-0755 must not define membership by a fixed dressing-energy value. It must
separate:

\[
P_{\rm matter}(X),
\qquad
\mathcal C_{\rm env}(X;R),
\qquad
\mathcal L_{\rm energy}(X;R).
\]

- (P_{\rm matter}) is the support-independent core/family predicate.
- (mathcal C_{\rm env}) classifies quiet, outgoing, incoming, throughput,
  or constraint-maintained surroundings.
- (mathcal L_{\rm energy}) is the exact scale-indexed internal, boundary,
  readout, and environmental ledger.

The same complete state must receive the same matter classification for
(R=4,6,8). Energy components are allowed—and required—to change only by the
registered projection and readout identities. A remote divergence-free field
outside the causal buffer may change the environmental class but must not
change pre-contact matter membership.

This does not yet identify a support-independent mass. A physical mass claim
requires an action-derived invariant or a stable positive-residue pole of the
complete response, after M3 and mobility close.

## 6. Recursive questions

1. Which instantaneous core margins remain invariant under all three support
   choices?
2. Does a remote divergence-free field change only the environmental fibre
   before causal contact?
3. Does the projection flow remain valid when the core moves and the support
   centre crosses an integer chart?
4. What discrete Reynolds transport term accompanies a moving bookkeeping
   surface?
5. Does formation coincide with entry into a support-independent core family
   rather than first appearance of an outgoing wake?
6. Does decay coincide with exit from that family plus closed transfer into
   boundary and outgoing ledgers?
7. For two cores, when do two support ladders become one composite ladder?
8. Is composition controlled by a persistent interaction margin rather than
   a distance cutoff?
9. Is there a scale-invariant pole or action quantity that supplies mass?
10. Does any exact reaction-complete invariant constrain the scale flow and
    thereby deserve the name charge?

## 7. Boundary of the result

FTD-0754C establishes a coherent selected observer scale flow on an existing
discovery corpus. It does not establish M3, an invariant basin, a particle,
autonomous motion, charge, mass, radiation ontology, unitarity, or Lorentz
recovery. Every new FTD-0755 perturbation and volume remains unseen.

Production defaults, established CUDA, scenarios, and ontology are unchanged.
