# Preregistration — Ignition-Cut Support Ablation (FTD-0587)

**Status:** `[LOCKED — RUN OF RECORD NOT YET EXECUTED]`  
**Date:** 2026-07-26  
**Production effect:** none.

## 1. Question

FTD-0474 reported finite-support reaction fronts after a one-time local flux
injection, but its reaction arms disabled native state--flux coupling and
enabled the selected Gauss projector. FTD-0586 then closed self-ignition by
one, two, or three sanitized stationary sources in the isolated causal
coupling sector. The remaining claim is therefore narrower:

> After the registered FTD-0474 external ignition has formed a manifested
> support, what maintains that support: retained injected field, the causal
> state-gradient source, or repeated Gauss projection?

This is a mechanism ablation, not a search for a favorable seed, amplitude,
cut time, or survival tolerance. It cannot establish particle identity,
transport, reciprocity, a membrane, or a common action.

## 2. Frozen source contract

| source | SHA-256 |
|---|---|
| `engine/src/render_bridge.cpp` | `A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/poisson_solvers.cpp` | `AF43DC1DDE2DDF4A47C87B6D552DB053D7D25038FF801D5CB929401E681B4264` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/tests/campaign_emergent_boundary_mechanism.cpp` | `04F4D0D72879427EFC6BB1354B3D904C8F2214BE4B9F70912E5362F22F66135F` |
| `engine/include/ftd/eft/emergent_boundary_observer.h` | `913789453A934EF8765414B9F523078E1BFA6E542BB67C0CA8D608EA7B651FC2` |

The production tick, default toggles, constants, scenarios, and FTD-0474
observer are frozen. CPU execution is mandatory because the history journal
is immutable and CPU-only.

## 3. Locked ignition prefix

The initial-condition cells are copied exactly from the registered
FTD-0474 reaction-dispersal arm:

- `L={24,32}`;
- injection amplitudes `A={12,20,40}` in units of `K_GENESIS`;
- seeds `{0xE0102000,0xE0102001,0xE0102002,0xE0102003}`;
- one axial center injection `(A K_GENESIS,0,0)`;
- `wave_propagation=true`, `gauss_projection=true`, `genesis=true`;
- `coupling=false`, `dual_substrate=false`, and dispersal field boundary;
- all force, movement, damping, bath, pair, weak, and clock branches off.

Each prefix runs for exactly 150 ticks. This cut is not inferred from the new
data: tick 150 was the first locked FTD-0474 tail sample. Every continuation
is produced by deterministic replay of the same prefix. Before any ablation,
the full selected state hash, RNG hash, ternary support hash, occupancy,
polarity counts, and quadratic field-amplitude norm are recorded. All six
replays of a cell must agree bit-for-bit at this point.

All voxel `velocity` and `remainder` values must already be zero. They are
explicitly rebased to zero at the cut in every arm, and any nonzero pre-rebase
component invalidates the campaign rather than being silently treated as a
source.

## 4. Six registered continuations

Every arm retains `wave_propagation`, `genesis`, single substrate, dispersal
boundary, and the exact prefix ternary state. It runs from absolute tick 150
through tick 300.

| arm | cut field | causal coupling | Gauss projection | purpose |
|---|---|---:|---:|---|
| `intact_reservoir` | retained | off | off | retained external field alone |
| `intact_causal` | retained | on | off | retained field plus causal state source |
| `intact_projected` | retained | off | on | exact FTD-0474 continuation |
| `cleared_control` | `J=W=0` | off | off | state-only negative control |
| `cleared_causal` | `J=W=0` | on | off | causal regeneration from `s_cut` |
| `cleared_projected` | `J=W=0`, then one registered Gauss solve | off | on | selected constraint regeneration from `s_cut` |

The field clear also zeros the unused single-substrate split fields, but never
changes ternary state, particle labels, RNG state, or tick. The one explicit
Gauss solve in `cleared_projected` is part of that arm's intervention: it asks
whether the selected constraint map can reconstruct support from `s_cut`
without the inherited injection reservoir. No analogous solve is inserted in
the causal arm.

## 5. Locked estimators

Samples are taken at absolute ticks `{150,180,210,240,270,300}` using the
unchanged FTD-0474 morphology observer. For each run record:

- occupancy, boundary/interior counts, centroid, RMS radius, and largest
  connected component;
- positive and negative support counts;
- genesis, evaporation, movement, and annihilation event counts after the cut;
- `Q=1/2 sum_x (|J_x|^2+|W_x|^2)`, explicitly labelled a quadratic field
  amplitude norm rather than a conserved Hamiltonian;
- maximum `|J|`, maximum `|W|`, the production vacuum-site Gauss residual,
  and the maximum velocity/remainder component;
- state, RNG, and observer-neutrality hashes.

The unchanged FTD-0474 stability predicate is reused verbatim:

1. all six samples are valid;
2. `4 <= occupancy <= 0.01 L^3` at every sample;
3. occupancy coefficient of variation is at most `0.20`;
4. RMS-radius coefficient of variation is at most `0.15`.

A `(L,A)` cell passes for an arm when at least three of four seeds are stable.
An arm is **support-qualified** when at least five of its six cells pass, the
same cell count achieved by the registered reaction-dispersal baseline. Event
counts and field norms are reported independently; they are not folded into
the support predicate.

The following mechanism predicates are fixed before execution:

- `RESERVOIR_SUFFICIENT` iff `intact_reservoir` is support-qualified;
- `CAUSAL_STATE_SOURCE_SUFFICIENT` iff `cleared_causal` is support-qualified
  and `cleared_control` is not;
- `GAUSS_CONSTRAINT_SUFFICIENT` iff `cleared_projected` is support-qualified
  and `cleared_control` is not;
- `STATE_ONLY_PERSISTENCE` iff `cleared_control` is support-qualified;
- `MIXED_OR_UNRESOLVED` iff more than one sufficient-mechanism predicate is
  true, or intact and cleared comparisons do not isolate a unique mechanism;
- `NO_REGISTERED_SUPPORT_MECHANISM` iff none of the first four predicates is
  true.

`intact_projected` must reproduce the FTD-0474 reaction-dispersal cell verdicts
and aggregate `20/24` stable runs. Failure invalidates the ablation.

## 6. Algebraic scope

Before the first post-cut reaction, the field response can be separated at the
level of interventions into:

\[
 z_n=T^{n-n_c}z_{n_c}
     +\sum_{m=n_c}^{n-1}T^{n-1-m}K s_m
     +\mathcal P_G[s_m],
\]

where the first term is retained reservoir evolution, `K s` is the coded
causal state-gradient source, and `P_G` denotes the selected Gauss correction.
Genesis and evaporation make the later histories nonlinear, so differences
between arms are causal ablations, not an additive superposition theorem.
The cleared arms decide whether the cut state alone can regenerate the field
needed for persistence under the registered continuation map.

## 7. Validity and interpretation

Required structural gates:

- all 144 continuations execute and remain finite;
- all six prefix replays per cell have identical selected-state and RNG hashes;
- ternary support and labels are unchanged by field clearing;
- no movement or annihilation event occurs;
- velocity and remainder stay exactly zero;
- the history journal is state- and RNG-neutral;
- the intact projected control reproduces FTD-0474 exactly.

No result promotes a particle, membrane, pilot wave, wake, aura, conserved
energy, reciprocal action, scenario, or production toggle. A positive Gauss
predicate means only that repeated application of the selected nonlocal
constraint map supports the thresholded pattern. A positive reservoir
predicate means the original one-time injection still powers the registered
finite observation window. A positive causal predicate is conditional on the
externally prepared `s_cut` geometry and does not contradict the FTD-0586
small-source bound.

The campaign is `INVALID` if a hash changes, an amplitude/cut/tolerance is
altered after execution, a branch is added post hoc, or a production source is
modified.
