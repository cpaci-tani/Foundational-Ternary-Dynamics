# 0006 — Prefix-dispatch scenario registry

**Status:** Accepted

## Context

The dashboard hosts 84+ Scale-0 scenarios across 5 distinct domains
(flux dynamics, light/EM, quantum, SM seeds, field configurations). A
single switch statement would grow unboundedly; a class-per-scenario
pattern would be over-engineered for what are mostly 10-50 line setup
bodies. We needed a partition that lets each domain evolve independently
while keeping dispatch O(1) and discoverable.

## Decision

Scenarios are partitioned by filename prefix into group translation units
under `engine/src/scenarios/`: `flux.cpp`, `light.cpp`, `quantum.cpp`,
`vacuum.cpp`, `s0_seed.cpp`, and `s0_field.cpp`. Each group returns `true` if
it handled the scenario or `false` if the prefix did not match. The thin
router in `engine/src/scenarios.cpp` owns ordering and deterministic RNG reset.

**Amendment 2026-08-27:** the former JS mirror under
`engine/web/js/bridge/scenarios/` was archived after Scale-0 became C++-only.
The prefix-dispatch decision remains; the duplicate implementation does not.

## Consequences

- (+) New scenarios drop into the appropriate C++ group file
- (+) New domains add a new group file + one line in the dispatcher chain
- (+) Each scenario file is small and focused
- (+) Every host reaches the same seed body through `ftd::dispatch_scenario`
- (−) Slight indirection if you don't know the prefix convention

## Alternatives considered

- Class-per-scenario — rejected, see Context.
- Single switch statement — rejected: scaling problem.
- Map-based registry with explicit register() calls — rejected: cuts
  against ESM module loading order; less discoverable.

## References

- Files: `engine/src/scenarios.cpp`, `engine/src/scenarios/`
- Cross-refs: CONTRACTS.md §4
