# 0002 — Capability factories (symmetric polymorphism without inheritance)

**Status:** Accepted
**Date:** 2026-04 (retroactive)
**Author:** codified 2026-04-27

## Context

`MockBridge` (JS-only physics) and `WasmBridge` (C++/WASM bindings) need to
expose the same external surface to scale controllers, scenarios, and the
viewport. JavaScript class inheritance was rejected because: (a) the
implementations diverge significantly (MockBridge is ~1500 LOC of pure JS
physics; WasmBridge is a ~670 LOC wrapper); (b) inheritance ties extension
to a class hierarchy that doesn't fit the per-scale partitioning.

## Decision

Each bridge populates a `bridge.capabilities` object keyed by scale:

```js
bridge.capabilities = {
    scale0: { setupScenario, tickScale0, getScale0Diagnostics, ... },
    scale1: { ... },
    scale2: { ... },
};
```

Factories `createScale0Capabilities(bridge)` etc. produce these objects.
Both bridges expose identical method names with compatible signatures.
Consumers call `ctx.bridge.capabilities.scale0.tickScale0()` without ever
checking `bridge.isWasm`.

## Consequences

- (+) Extension by adding a new scale = add one factory, no class hierarchy
- (+) Consumers stay backend-agnostic
- (+) Easier to test (each capability is a plain object of functions)
- (−) Method discovery requires knowing the factory location

## Alternatives considered

- Class inheritance (`MockBridge extends Bridge`) — rejected, see Context.
- Duck typing without a registry — rejected: no discoverability for LLMs.

## References

- Files: `createScale0/1/2Capabilities` in `engine/web/js/wasm-bridge-dag.js`
- Cross-refs: CONTRACTS.md §2, ADR-0004 (scale controllers)
