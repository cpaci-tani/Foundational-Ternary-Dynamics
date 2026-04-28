# 0010 — Cascade callback pattern (sub-renderer lifecycle)

**Status:** Accepted
**Date:** 2026-04-27
**Author:** refactor sweep Phase 3 (commits 8b4732d, 1506079, 1499a11, 506805b)

## Context

Phase 3 of the refactor sweep extracted 4 sub-renderers from the monolithic
3953-LOC `Viewport` class. Each sub-renderer needs to react to lifecycle
events the orchestrator owns: lattice resize, boundary shape change,
engine mode change, animation tick, dispose. The `setLatticeSize` cascade
in particular touches every sub-renderer (rebuilds boundary, axes, flux
volume, particle meshes, field overlays — 27+ meshes total).

## Decision

Each sub-renderer exposes a fixed set of lifecycle methods
(`onLatticeSizeChanged`, `setBoundaryShape`, `setEngineMode`,
`setAnimationClock`, `dispose`). The orchestrator dispatches
**unconditionally** to every sub-renderer when an event fires; sub-renderers
implement no-op bodies if the event doesn't apply. Sub-renderers do NOT
subscribe to events — the orchestrator pushes; this avoids hidden
event-bus state and keeps the data flow inspectable.

## Consequences

- (+) Missing a sub-renderer in a cascade is a structural mistake (visible
  in the `setLatticeSize` body), not a silent runtime bug
- (+) Each sub-renderer's lifecycle is explicit and testable
- (+) Constructor ordering becomes documented (SceneCore → FieldRenderer →
  Flux/ParticleRenderer because the latter capture FieldRenderer's
  mesh-factory callbacks at ctor time)
- (−) Discipline burden: every new event type requires the orchestrator
  to add the dispatcher branch + every sub-renderer to add the method

## Alternatives considered

- Event bus / observer pattern — rejected: opaque registration order,
  silent dropouts when sub-renderers forget to subscribe
- Inheritance with template methods — rejected: see ADR-0002 rationale
  (composition over inheritance for scale capabilities)

## References

- Files: `engine/web/js/viewport.js` (orchestrator), `engine/web/js/viewport/{scene-core,flux-renderer,particle-renderer,field-renderer}.js`
- Cross-refs: CONTRACTS.md §10, ADR-0001 (viewport-decomposition), ADR-0011 (mesh-factory callback)
