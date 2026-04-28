# 0001 — Viewport decomposition

**Status:** Accepted (Phase 3 of refactor sweep will extend this)
**Date:** 2026-04 (retroactive)
**Author:** refactor sweep, codified 2026-04-27

## Context

`engine/web/js/viewport.js` accumulated 3953 LOC and 169 methods on a single
`Viewport` class, mixing camera/lights, particle rendering, field rendering,
flux volume/slice, atom rendering pass-throughs, and viz settings. Every
new visualization touched the same file; cognitive load on physics fixes
that needed to scroll past unrelated rendering code grew superlinearly.

## Decision

Extract single-responsibility sub-renderers under `engine/web/js/viewport/`,
each owning a narrow concern with explicit `{scene, camera, settings}`
constructor parameters. Initial extractions: `molecular-renderer.js`,
`boundary-geometry.js`, `topology-sheet-renderer.js`, `color-ramps.js`.
The `Viewport` class becomes an orchestrator that composes sub-renderers.

## Consequences

- (+) Each sub-renderer is independently testable and editable
- (+) New visualization types extend the pattern without touching core
- (+) Fewer merge conflicts during parallel work
- (−) Slight indirection cost when reading composite behaviors
- Phase 3 will extract 5 more sub-renderers (scene-core, flux-renderer,
  field-renderer, particle-renderer, viz-settings) reducing viewport.js
  to ~300 LOC.

## Alternatives considered

- Keep monolithic `Viewport` class — rejected: scaling problem confirmed
  (3953 LOC, slow to navigate).
- Use Three.js scene graph nodes as the boundary — rejected: bookkeeping
  remains centralized in Viewport regardless.

## References

- Files: `engine/web/js/viewport.js`, `engine/web/js/viewport/*.js`
- Cross-refs: ADR-0005 (live-reference pattern), `META_PROJECT_ATLAS.md` §2
