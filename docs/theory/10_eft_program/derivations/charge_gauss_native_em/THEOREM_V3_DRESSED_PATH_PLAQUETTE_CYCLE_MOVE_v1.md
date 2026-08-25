# V3 dressed-path plaquette cycle move v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT LOCAL GAUSS-STRING DEFORMATION]** +
**[SELECTION — CANDIDATE PHI EXTENSION]** + **[OPEN — SCHEDULE, WEIGHTS,
CHARGED POLE, ENDPOINT MOTION, AND WORK]**  
**Carrier price:** none  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[`proof_v3_dressed_path_plaquette_cycle_move.py`](../../../../../scripts/proofs/proof_v3_dressed_path_plaquette_cycle_move.py)
passes 11/11 gates and 17,280 exact local identities. It checks every ordered
perpendicular SC pair, polarity, C4 phase, C3 layer, three translated fixtures,
and all 48 signed-cubic transformations.

---

## 1. Two paths, one boundary

For perpendicular SC steps `a` and `b`, the two paths across one plaquette are

\[
 P_{ab}=[x,x+a]+[x+a,x+a+b],
\]

\[
 P_{ba}=[x,x+b]+[x+b,x+a+b].
\]

Their boundaries are identical:

\[
 \partial P_{ab}=\partial P_{ba}
 =-\delta_x+\delta_{x+a+b}.
\]

Therefore their difference is the oriented plaquette cycle

\[
 \partial(P_{ba}-P_{ab})=0.
\]

Replace every active edge by the complete dressed SC macrostate of the
preceding theorem. The flip exchanges the two active primary tokens and their
sixteen bound field channels with the two alternate reserve tokens and their
new sixteen bound channels. Four A9 tokens and sixteen field bits exist on
both sides.

---

## 2. Exact source/current/Gauss identities

For polarity `epsilon`, the path charge and electric cochain are

\[
 Q(P)=-\epsilon\,\partial P,
 \qquad E(P)=-\epsilon P.
\]

Thus both paths obey

\[
 \operatorname{div}E(P)=Q(P).
\]

The ownership current of the flip is

\[
 j=\epsilon(P_{ba}-P_{ab}),
\]

and consequently

\[
 \boxed{\operatorname{div}j=0},
 \qquad
 \boxed{\Delta E=-j},
 \qquad
 \boxed{\Delta Q=0}.
\]

The endpoints and their charge do not move. What moves is the intervening
Gauss dressing. The flip is local, finite, reversible on the declared
macrostate, and signed-cubic covariant.

---

## 3. Connected string sector

Every adjacent transposition of two distinct step directions is one plaquette
flip. The ordinary inversion-reducing adjacent-swap argument therefore
connects all monotone paths with the same step multiplicities. The certificate
also enumerates the `(2,2,1)` class exactly: all 30 paths lie in one connected
component.

This is the first v3-specific construction showing that exact Gauss dressing
need not be a rigid frozen string. The common finite carrier contains local
cycle moves capable of exploring a connected dressing class.

---

## 4. Why this is not yet a Coulomb pole

Connectivity does not select a stationary measure. Equal-length plaquette
flips carry no derived preference among paths, and uniform counting over
shortest paths is not automatically the quadratic Maxwell/Gauss measure.
Therefore the result does not establish:

- an ergodic conflict-free global schedule;
- physical weights on string configurations;
- relaxation to the minimum-norm Gauss field;
- a `1/Lambda(k)` static kernel;
- endpoint motion or reciprocal force; or
- the blocked action curvature `chi_EM`.

The next gate is to obtain the schedule and weights from the same finite
history object used for Born statistics. In this precise sense the charged
pole, coupling normalization, and Born preparation problems now meet at one
shared action/measure boundary.

