# 0006 — Prefix-dispatch scenario registry

**Status:** Accepted
**Date:** 2026-04 (retroactive)
**Author:** codified 2026-04-27

## Context

The dashboard hosts 84+ Scale-0 scenarios across 5 distinct domains
(flux dynamics, light/EM, quantum, SM seeds, field configurations). A
single switch statement would grow unboundedly; a class-per-scenario
pattern would be over-engineered for what are mostly 10-50 line setup
bodies. We needed a partition that lets each domain evolve independently
while keeping dispatch O(1) and discoverable.

## Decision

Scenarios are partitioned by filename prefix into 5 group files in
`engine/web/js/bridge/scenarios/`: `flux-scenarios.js`, `light-scenarios.js`,
`quantum-scenarios.js`, `s0-seed-scenarios.js`, `s0-field-scenarios.js`.
Each group exports `setupXxxScenario(name, ctx)` that returns `true` if it
handled the scenario (matched its prefix AND completed setup) or `false`
if the prefix didn't match.

The dispatcher `runSetupScenario` in `index.js` chains them via
`.call(this, name, ctx)` so the scenario body has access to the bridge's
mutation methods.

## Consequences

- (+) New scenarios drop into the appropriate group file
- (+) New domains add a new group file + one line in the dispatcher chain
- (+) Each scenario file is small and focused
- (+) C++ side mirrors this partitioning (`engine/src/scenarios/<group>.cpp`)
- (−) Slight indirection if you don't know the prefix convention

## Alternatives considered

- Class-per-scenario — rejected, see Context.
- Single switch statement — rejected: scaling problem.
- Map-based registry with explicit register() calls — rejected: cuts
  against ESM module loading order; less discoverable.

## References

- Files: `engine/web/js/bridge/scenarios/`, `engine/src/scenarios/`
- Cross-refs: CONTRACTS.md §4
