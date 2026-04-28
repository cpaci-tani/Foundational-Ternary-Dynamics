# 0004 — Scale controllers + 3-folder package structure

**Status:** Accepted
**Date:** 2026-04 (retroactive)
**Author:** codified 2026-04-27

## Context

The dashboard hosts 12+ scales (lattice / particle / atom / molecular /
planetary / cosmic / meta / consciousness / etc.). Each scale needs its own
tick loop, scenario loading, UI panels, and reactive state. Putting all of
this in one entry point (`app_dag.js`) made cross-scale concerns implicit
and prevented isolated testing.

## Decision

Each scale lives in `engine/web/js/scales/scale<N>/` with three subfolders:

- `runtime/` — tick loop, frame sync, scenario loading, diagnostics updates
- `ui/` — bindings, controls, overlays, panels
- `state/` — reactive store

Plus a top-level `controller.js` orchestrating them and a `viewport-adapter.js`
for the rendering bridge. `app_dag.js` plugs scales together via stateless
exports; no cross-scale imports.

The shared `ctx` object (see CONTRACTS.md §3) is the only inter-scale
communication channel.

## Consequences

- (+) Each scale is independently testable, navigable, and replaceable
- (+) New scales are added by copying the directory template
- (+) Cross-scale dependencies are forced through `ctx`, making them visible
- (−) Some boilerplate per scale (controller, adapter)

## Alternatives considered

- Single monolithic dashboard — rejected: 12 scales × N panels = combinatorial
  growth with no boundaries.
- Scale-as-class with inheritance — rejected: see ADR-0002 rationale.

## References

- Files: `engine/web/js/scales/scale0/`, `scales/scale1/`, ..., `scales/scale11/`
- Cross-refs: CONTRACTS.md §3, ADR-0002 (capability factories)
