# PRE-REGISTRATION — Poisson cold-start memory v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0441`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0440` Poisson reciprocity convergence  
**Engine artifact:** `engine/tests/campaign_poisson_cold_start_memory.cpp`  
**Artifact SHA256:** `2d0ddcf0a87ae895241573711e91dc21c8df6b7c4d8187fe244c797dfc62cd1b`

## 1. Question

FTD-0439 accumulates `8.11e-9` particle momentum during a 200-tick Poisson
trajectory. FTD-0440 shows that the default six-sweep static force is initially
balanced, later cold partial solves can be imbalanced, and a 96-sweep prepared
potential is reciprocal below `1.43e-13`. FTD-0441 asks:

> Is the full Poisson trajectory leak stored memory of the cold solver's
> transient imbalance, even though neither source voxel hops?

## 2. Frozen matched trajectories

Both arms use periodic `L=33`, pair separation `8`, 200 measured ticks, six SOR
sweeps per tick, and the exact FTD-0439 Poisson configuration:
`wave_propagation`, `coupling`, `forces`, `movement`, `poisson_coulomb`, and
`strict_validation` enabled; every other Boolean extension disabled.

- cold arm: movement is enabled from the first tick;
- prepared arm: movement is disabled for 16 warmup ticks (`96` total SOR
  sweeps), the two accumulated particle velocities are reset to zero, movement
  is enabled, and 200 measured ticks follow.

Axes `x,y,z` are tested. FTD-0439 established exact simultaneous-polarity-
reversal equality for Poisson, so only low-coordinate `+1`, high-coordinate
`-1` is used.

## 3. Frozen observables and gates

Record center-of-mass displacement, maximum/final production particle momentum,
minimum separation, source-voxel hop count, survival, and configuration state.

- each cold arm must exceed either `1e-10` momentum or `1e-8` common motion;
- every prepared arm must stay below both gates;
- maximum prepared/cold ratios for momentum and common motion must each be at
  most `0.01`;
- neither particle may execute a voxel hop in any arm.

## 4. Locked outcomes

- `COLD_START_TRANSIENT_EXPLAINS_POISSON_LEAK`: cold defect resolves, every
  prepared arm passes, both suppression ratios pass, and no voxel hops occur.
- `PERSISTENT_MOVEMENT_PHASE_DEFECT`: valid execution but the complete transient
  outcome fails.
- `NO_COLD_DEFECT_REPRODUCED`: every cold arm fails to resolve the prior defect.
- `INVALID_PROTOCOL`: nonfinite output, missing particle, toggle/backend
  mismatch, or incomplete execution.

## 5. Interpretation boundary

The transient verdict would clear the Poisson movement integrator of a residual
defect only for no-hop sub-voxel trajectories after explicit solver
preparation. It would classify the default cold scenario initialization as
numerically under-prepared. It would not make the instantaneous Poisson branch
native electromagnetism and would not affect the closed-negative selected
`G_C s grad|J|` force.

## 6. Banned moves

- No warmup length, SOR count, trajectory duration, axes, gates, or toggles may
  change after first execution.
- Only particle velocities may be reset after warmup.
- No baseline subtraction or fitted correction.
