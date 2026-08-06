# FTD-0719 — Polarity-snapshot current non-uniqueness v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parent:** FTD-0718

## Question

Does an unordered signed snapshot of manifested subcell positions determine
the exact oriented face current connecting it to the next snapshot, or can two
causal charge-preserving correspondences have identical endpoint densities and
different physical currents?

## Frozen witness

Use `L=9` and four constituents with charges `(+1,+1,-1,-1)`.  Relative to an
interior origin, place the positive pair about a cluster at `-3/4 e_z` and the
negative pair about a cluster at `+3/4 e_z`.

For each cluster use start offsets `(+1/4 e_x,-1/4 e_x)` and endpoint offsets
`(+1/4 e_y,-1/4 e_y)`.  The negative pair uses the direct correspondence in
both histories.  Compare two positive correspondences:

1. direct: `+x -> +y`, `-x -> -y`;
2. crossed: `+x -> -y`, `-x -> +y`.

Deposit every segment with the registered quadratic polarity coat and exact
straight oriented-face current.  All segment lengths are `sqrt(1/8)`, below
`C_SPEED=1/sqrt(3)`.

## Locked measurements

For the aggregate direct current `J_d`, crossed current `J_x`, and difference
`Delta J=J_d-J_x`, measure:

1. equality of aggregate start and endpoint densities;
2. exact continuity for both histories;
3. `div(Delta J)`;
4. coefficient `L2` and maximum norms of `Delta J`;
5. `curl^T(Delta J)` norms;
6. the total-current-moment difference;
7. the connection witness `<Delta J,Delta J>`;
8. reversal residual under swapping every segment endpoint;
9. covariance of the nonzero norms under all 24 proper cubic rotations and
   one nonzero integer translation.

Use `1e-12` for equality, continuity, divergence, reversal, moment, and
covariance gates.  Require the current-difference, curl-difference, and
connection witness to exceed `1e-6`.

## Algebraic statement

For any two charge-preserving matchings with the same endpoint densities,
exact continuity gives

\[
\nabla\!\cdot(J_1-J_2)=0.
\]

The witness determines whether this divergence-free difference can be
nonzero in the registered face-current construction.  If it can, snapshots
determine only the longitudinal current constraint; the transverse/cycle
current remains unspecified.

## Verdicts

- `POLARITY_SNAPSHOT_CURRENT_NONUNIQUENESS_THEOREM_WITNESSED` requires every
  exact and covariance gate plus a nonzero transverse witness.  The conclusion
  is that an unordered polarity snapshot is not a dynamically complete Markov
  state by itself.
- `POLARITY_SNAPSHOT_DETERMINES_REGISTERED_CURRENT` applies if endpoint-density
  equality and continuity pass but the current difference vanishes.
- `POLARITY_SNAPSHOT_CURRENT_WITNESS_EXECUTION_INVALID` applies to invalid
  segments, causal failure, density mismatch, continuity failure, broken
  reversal, or broken cubic/translation covariance.

A constructive witness does not force persistent constituent labels.  It
forces one of three structures: a deterministic correspondence rule derived
from the common action, an explicit current/history state, or an equivalent
connection variable.  Selecting among those remains open.
