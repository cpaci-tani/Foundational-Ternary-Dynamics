# Hodge-flag O_h-equivariant seven-invariant collision and full-tick boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT FIXED-POINT-FREE CUBIC-EQUIVARIANT
COLLISION]** + **[THEOREM — EXACT SEVEN-DIMENSIONAL ADDITIVE-INVARIANT
SPACE]** + **[CLOSED NEGATIVE, SCOPED — THIS COLLISION WITH THE
SHARED-EDGE C3×C4 TICK]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_hodge_flag_equivariant_pair_matching.py](../../../../../scripts/proofs/proof_hodge_flag_equivariant_pair_matching.py)
performs 880,560 exact checks on all 18,336 unordered pairs of the 192-state
Hodge-flag alphabet. No measured target or fitted coefficient enters the
construction.

---

## 1. Collision problem

For a flag-phase state $z=(d,n,h,p)$ use the parity-correct field readout

\[
 E(z)=\Re(i^p)d,
 \qquad
 B(z)=\Im(i^p)n .                                  \tag{1}
\]

The pair relation classified previously proves that record number and the six
components of $(E,B)$ are the only additive quantities that must be
conserved. That relation did not yet exhibit one deterministic collision
commuting with the full signed cubic group $O_h$.

## 2. Exact orbit matching

The 18,336 pair states decompose into 420 $O_h$ orbits,

\[
 76\text{ of size }24,
 \qquad
 344\text{ of size }48.                            \tag{2}
\]

Every orbit admits at least one field-preserving fixed-point-free
self-involution. There are also 13,694 compatible edges between distinct
orbits.

The union of every self-orbit option has transition rank only

\[
 \operatorname{rank}T_{\rm self}=182,              \tag{3}
\]

so self-orbit collisions inevitably preserve three surplus one-particle
quantities. Each distinct-orbit exchange contributes at most one missing
quotient direction: 9,796 compatible edges contribute one and 3,898
contribute zero. Therefore at least three distinct-orbit exchanges are needed.

A deterministic disjoint choice

\[
 (0,2),\qquad(1,50),\qquad(5,178)                  \tag{4}
\]

spans all three missing directions. Rank-greedy selection among the remaining
exact self-involutions then produces a single global collision $C$ satisfying

\[
 C^2=1,\qquad C(z)\ne z,\qquad
 C(gz)=gC(z),\qquad
 (E,B)(Cz)=(E,B)(z)                                \tag{5}
\]

for every pair state $z$ and every $g\in O_h$.

The labels in equation (4) are canonical orbit identifiers in the exact
certificate, not physical parameters.

## 3. Complete invariant result

The selected collision has exact rational transition rank

\[
 \operatorname{rank}T_C=185,
 \qquad
 \dim\ker T_C=192-185=7.                           \tag{6}
\]

The seven independent additive invariants are precisely

\[
 \boxed{1,E_x,E_y,E_z,B_x,B_y,B_z}.                \tag{7}
\]

Thus a single finite, reversible, fixed-point-free, spatially cubic-equivariant
pair collision can mix away all ray, phase, handedness, and controller labels
not required by the electromagnetic field totals. Inter-orbit exchange is
structurally necessary for this minimal invariant count.

## 4. What this does not prove

This remains a valid collision theorem, but its subsequent
[full-tick certificate](THEOREM_HODGE_FLAG_FIXED_FRAME_FULL_TICK_ZERO_MODE_BOUNDARY_v1.md)
closes the selected composition negative: the twelve clock images of its
field kernel intersect only in the uniform number mode. Consequently it does
not prove:

- preservation of the two divergence constraints by one finite full tick;
- a four-dimensional transverse slow sector with $\omega\sim |k|$;
- a conservative capacity/work exchange with actualization sources;
- autonomous matter, detector, or clock formation;
- tensor propagation, universal gravity, or lensing; or
- a native dimensionless electromagnetic coupling or fine-structure readout.

## 5. Next locked gate

Compose the explicit collision with the shared-edge flag transport and C4
advance as one reversible tick. The next certificate must compute its exact
Bloch kernel and either recover the centered edge--face Hodge generator in the
long-wavelength sector or close this particular lift negative. Only after that
pass can source work and a native coupling observable be attached without
assuming Maxwell dynamics.
