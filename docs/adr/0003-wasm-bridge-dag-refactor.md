# 0003 — WASM bridge DAG refactor (DAG scheduler over linear pipeline)

**Status:** Accepted
**Date:** 2026-04 (retroactive)
**Author:** codified 2026-04-27

## Context

The original WASM bridge ran phases as a linear pipeline:
read → write → gauss → forces → movement. As the engine added gravity,
Lorentz, color forces, dual-substrate, and EFT modes, dependencies between
phases became less linear. A rigid sequence forced toggle gates inside
each phase to skip work, polluting hot paths and obscuring the actual
data-flow graph.

## Decision

Replace the linear pipeline with a DAG scheduler that resolves phase
dependencies declaratively. Each phase declares its inputs (state fields,
toggles required) and outputs; the scheduler topologically sorts and
executes only enabled nodes per tick.

The current implementation lives in `engine/web/js/bridge-init.js`
(the `-dag` suffix is the historical marker; the file holds both
MockBridge and WasmBridge, which Phase 2 will split).

## Consequences

- (+) Adding a new phase is a localized change (declare inputs/outputs,
  register with the scheduler)
- (+) Disabled phases produce zero overhead (not just early-return inside)
- (+) Data-flow graph is inspectable
- (−) Initial cost was a substantial rewrite (~5736 → ~2132 LOC during the
  refactor)

## Alternatives considered

- Keep linear pipeline with toggle-gated bodies — rejected: degraded
  readability and made dependency reasoning implicit.
- Reactive (event-bus) approach — rejected: harder to reason about
  per-tick determinism.

## References

- Files: `engine/web/js/bridge-init.js`
- Cross-refs: ADR-0008 (R1-R5 phase extraction in C++), CONTRACTS.md §2
