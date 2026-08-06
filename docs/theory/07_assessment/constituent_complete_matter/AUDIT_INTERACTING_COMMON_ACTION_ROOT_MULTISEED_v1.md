# Audit — FTD-0720 interacting common-action root multiseed

**Status:** `[AUDIT — ONE LOCAL ROOT BASIN WITNESSED / GLOBAL UNIQUENESS OPEN]`  
**Date:** 2026-07-28

## Findings

1. **The registered root is seed-robust.** All 39 deterministic initial
   guesses converge and pass the pre-existing common-action gates. Complete
   state and deposited current agree below `3.31e-14` and `1.58e-14`.

2. **The inverse is not a post-hoc reconstruction.** Every accepted later
   state returns through the unchanged reverse action, with worst recovery
   `3.31e-14`.

3. **Persistent labels do not drive the result.** Swapping the complete
   constituent bookkeeping in each arm and undoing that permutation reproduces
   the same physical state and current.

4. **Global uniqueness is not proved.** The campaign samples one finite root
   basin for a width-one neutral composite. It computes neither a global
   contraction bound nor the degree of the full nonlinear map.

5. **The result does not contradict snapshot non-uniqueness.** FTD-0719
   removes momentum, field, graph, and action data. FTD-0720 supplies those
   data and observes that the action selects one current in its registered
   sector.

6. **Collision remains outside the state chart.** FTD-0503's coincident-target
   control still returns `COLLISION_RULE_REQUIRED`; FTD-0720 contains no
   coincident anchor, graph change, or reaction channel.

## Correct statement

The selected interacting common action has one numerically witnessed root
basin for the three registered smooth width-one neutral states, independent of
the 13 locked momentum seeds and constituent record ordering. This supports,
but does not prove, deriving current from the complete transaction without a
new primitive. Global root uniqueness and collision semantics remain open.

## Verification

- focused CTest: `1/1 PASS`;
- independent certificate: `30/30 PASS`;
- production defaults and tick: unchanged;
- repository-wide build: stopped by the pre-existing Windows CUDA device-link
  target `benchmark_invariant_matrix_constant_memory`; the focused CPU target
  compiled and linked successfully under the pinned toolchain.

