# 0005 — Live-reference factory pattern (cache-coherent extraction)

**Status:** Accepted (load-bearing for Phase 2)

**Current scope (2026-08-28):** Scale-0 `MockBridge` and its diagnostics,
particle, and lattice factories were retired when the web engine became
WASM-only. The decision remains active for `createAtomEngine(state)`, the
Scale-2 JS helper hosted by `WasmBridge`.

## Context

When extracting subsystems from `MockBridge` (diagnostics, particle engine,
lattice samplers, atom engine), the extracted modules need access to live
state (particles, flux arrays, cache fields). Two designs were considered:
destructure state into the factory (snapshot at construction) vs hold a
live reference. The destructure approach was used in early extractions and
caused subtle cache-invalidation bugs: the bridge would reset
`_energyCacheTick = -1`, but extracted closures held a stale `_tick`
value and never recomputed.

## Decision

Every extracted subsystem follows the **live-reference factory pattern**:

```js
export function createXxxProvider(state) {  // state is the live MockBridge instance
    return {
        getX() { return state._cachedX; },  // re-reads each call
    };
}
```

Rules (codified in CONTRACTS.md §1):
- Factories MUST hold the `state` reference verbatim (never destructure).
- Methods are stateless; every call re-reads `state.<field>`.
- Cache invalidation written from the bridge side (e.g., `_energyCacheTick = -1`)
  is immediately visible to the factory's methods.
- Each module's STATE CONTRACT block enumerates ownership and invariants.

Current exemplar: `engine/web/js/bridge/mock-atom-engine.js` lines 14–28.

## Consequences

- (+) Cache invalidation works correctly across extractions
- (+) Subsystems can be tested with a mock state object
- (+) Module ownership is explicit and enforced via STATE CONTRACT
- (−) Discipline burden: easy to accidentally destructure during refactors

## Alternatives considered

- Destructure state at construction — rejected: causes cache bugs.
- Pass state to every method as a parameter — rejected: pollutes consumer
  signatures.

## References

- Current file: `engine/web/js/bridge/mock-atom-engine.js`
- Retired Scale-0 factories remain available in Git history before the
  August 2026 product consolidation.
- Cross-refs: CONTRACTS.md §1, ADR-0002 (capability factories)
