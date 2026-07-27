# PRE-REGISTRATION — Poisson reciprocity convergence v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0440`  
**Status:** `[PRE-REGISTRATION — INVALID IMPLEMENTATION, NOT EXECUTED]`  
**Parent:** `FTD-0439` existing-force-branch reciprocity  
**Engine artifact:** `engine/tests/campaign_poisson_reciprocity_convergence.cpp`  
**Artifact SHA256:** `96dba0c60804220226c846ee15dde5d6698675475e4f4d2e2d1947982249060d`

**Invalidation note:** the locked source attempted to call the private
`RenderBridge::solve_coulomb_poisson()` and `RenderBridge::phase_forces()`
subphases. Compilation failed before any campaign execution. Revision 2 uses
only the public production tick and preserves this file as provenance.

## 1. Question

FTD-0439 found a `8.11e-9` total-momentum residual in the default Poisson
branch. The production default performs six warm-started SOR sweeps per tick,
while the public API recommends `20–30` iterations for scientific Coulomb
benchmarks. FTD-0440 asks:

> Is the Poisson pair-force imbalance a cold-start SOR transient, or does it
> converge to a nonzero reciprocity floor?

## 2. Frozen operator protocol

The pair is fixed at separation `8` on periodic `L=33`. Only `forces`,
`poisson_coulomb`, and `strict_validation` are enabled. Wave propagation,
coupling, and movement are disabled so the measured quantity is the production
Poisson solve plus production force evaluation, uncontaminated by trajectory
history.

The cold `x`-axis arms start from the constructor's zero potential and use SOR
counts

$$
\{1,2,4,6,12,24,48,96\}.
$$

Matched pre-relaxed arms first call the production Poisson solver for exactly
`96` sweeps, then execute the registered count inside `phase_forces`. Additional
pre-relaxed controls use the default `6`-sweep force phase for pair axes `y,z`.
FTD-0439 already established exact simultaneous-polarity-reversal equality for
the Poisson branch, so only low-coordinate `+1`, high-coordinate `-1` is used.

## 3. Primary observable

At the two fixed source sites, read the production Coulomb force diagnostics
after one force phase and compute

$$
F_{net}=F_++F_-.
$$

The production particle momentum after the force phase is recorded as a check,
but `|F_net|` is primary. No force or potential rescaling is allowed.

## 4. Locked gates

- cold default is resolved when `|F_net(6)|>1e-12`;
- cold values must be nonincreasing with at most `1%` local slack;
- high-iteration suppression requires
  `|F_net(96)|/|F_net(6)| <= 0.01`;
- every pre-relaxed arm must satisfy `|F_net|<=1e-12`.

## 5. Locked outcomes

- `SOR_TRANSIENT_EXPLAINS_POISSON_LEAK`: all four gates pass.
- `CONVERGED_POISSON_RECIPROCITY_FLOOR`: convergence dependence exists but the
  pre-relaxed maximum remains above `1e-12`, or another valid mixed pattern
  fails the full transient verdict.
- `NO_SOR_DEPENDENCE`: the high/default suppression ratio exceeds `0.01`.
- `INVALID_PROTOCOL`: nonfinite output, toggle/backend mismatch, or missing arm.

## 6. Interpretation boundary

The transient verdict would reclassify only the small FTD-0439 Poisson miss as
numerical initialization error. It would not rescue the selected magnitude-
gradient force and would not promote an instantaneous imported Poisson
potential to native electromagnetism. A converged floor would close the current
Poisson force/stencil pair negative for reciprocal mechanics at the registered
tolerance.

## 7. Banned moves

- No iteration counts, pre-relaxation count, axes, thresholds, or solver
  parameters may change after first execution.
- No extrapolated zero may replace the explicit pre-relaxed gate.
- No coefficient fitting or net-force subtraction.
