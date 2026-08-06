# FTD-0720 — Interacting common-action root multiseed v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Date locked:** 2026-07-28  
**Parents:** FTD-0503, FTD-0622, FTD-0719  
**Scope:** observer-only nonlinear-root discriminator; production unchanged

## Question

Does the already selected connected matter--field action return the same
accepted interacting transaction from a fixed family of widely separated
momentum seeds, or can the present complete state admit more than one physical
next state?

FTD-0503 already proves unique current matching in the free distinct-endpoint
sector. FTD-0719 proves that unordered polarity snapshots alone do not select
the cycle-current component. This campaign tests the remaining middle case:
whether the interacting common action numerically selects one root for the
smallest connected neutral composite.

## Frozen system

Use the unchanged FTD-0622 action and initialization:

- periodic `L=17`;
- width-one connected bipole: two constituents with charges `+1,-1` and one
  Moore-local quartic bond;
- production dispersion, quadratic coat, matched face/edge field update,
  `dt=1`, `C_SPEED`, unit binding stiffness, and the measured FTD-0468
  normalization;
- central-difference Newton solver, `solve_tolerance=2e-11`,
  `gate_tolerance=1e-10`, `max_iterations=64`;
- no legacy force, damping, source repair, post-hoc energy correction, random
  start, or altered field equation.

The three registered states are:

1. integer-phase axial rest: orientation `x`, phase axis `x`, phase `0`;
2. fractional parallel response: orientation `x`, phase axis `x`, phase `1/4`;
3. fractional transverse response: orientation `x`, phase axis `y`, phase
   `1/4`.

## Frozen seed family

The nonlinear unknown is the six-component later-momentum vector. For each
registered state use exactly 13 deterministic initial guesses:

1. the state's incoming momentum vector;
2. common boosts `+/-0.2` along each of `x,y,z` (six seeds);
3. charge-odd boosts of magnitude `0.2` along each of `x,y,z`, with both
   orientations (six seeds).

For a common boost both constituents receive the same vector. For a charge-odd
boost the positive constituent receives the named vector and the negative
constituent its negative. Seed magnitude and membership may not be changed
after any result is inspected.

## Frozen comparison and covariance

An accepted root must pass every existing FTD-0622 common-action gate. Compare
complete later states with `connected_moore_block_state_max_difference` and
compare the deposited face currents componentwise. Root agreement is
`<=1e-9`; current agreement is `<=1e-9`.

Repeat the canonical incoming-momentum seed after swapping the complete two
constituent records, charges, and the bond endpoints. Undo that record
permutation before comparison. This tests relabeling covariance of the action;
it is not a persistent-label input.

For every accepted root, run the unchanged state-only reverse solve and require
recovery of its own earlier state to `<=1e-8`.

## Frozen verdicts

- `INTERACTING_COMMON_ACTION_ONE_BASIN_WITNESSED`: all 39 registered seeds and
  all three relabeling arms converge, pass the action and inverse gates, and
  agree within the locked state/current tolerances.
- `MULTIPLE_INTERACTING_ROOTS_WITNESSED`: at least two accepted roots for the
  same registered state differ beyond either locked comparison tolerance.
- `INTERACTING_ROOT_GLOBAL_UNIQUENESS_UNRESOLVED`: no distinct accepted roots
  are found, but at least one registered seed, inverse, or relabeling arm does
  not pass.
- `INTERACTING_COMMON_ACTION_INVALID`: the canonical seed fails any existing
  FTD-0622 algebraic common-action gate.

The first verdict is finite numerical evidence for one attraction basin, not a
global uniqueness theorem. Only the second verdict proves non-uniqueness. A
failed seed is not counted as a second root and is not converted into positive
evidence.

## Ontological consequence lock

A one-basin result keeps the derived-transaction route viable only for the
registered smooth, distinct-endpoint sector. It does not solve collision,
formation, graph selection, or global root uniqueness. A multiple-root result
means the present complete state is not Markov-complete without a branch,
history, or connection variable. Any coincident-target case remains explicitly
outside this campaign and must enter a separately registered collision law.

