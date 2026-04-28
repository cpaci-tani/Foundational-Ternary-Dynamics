# 0011 — Mesh-factory callback pattern (cross-sub-renderer helpers)

**Status:** Accepted
**Date:** 2026-04-27
**Author:** refactor sweep Phase 3c (commit 506805b)

## Context

Phase 3c extracted FieldRenderer (~1800 LOC, 66 methods), the largest
viewport sub-renderer. FieldRenderer naturally owns the canonical 18-pt
streamline-mesh factory and the arrow-field write helpers — but FluxRenderer
(Phase 3b) and ParticleRenderer (Phase 3d) also need them. The choice was
between (a) duplicating helpers per sub-renderer (drift risk), (b) extracting
to a shared `viewport/_mesh-factories.js` module, or (c) keeping a single
canonical home and passing bound methods as ctor callbacks.

## Decision

Each shared helper has exactly **one canonical home** (the sub-renderer
most semantically aligned). Other sub-renderers call it via a callback
captured at constructor time:

```js
this._fluxRenderer = new ViewportFluxRenderer({
    // ... primary args ...
    buildStreamlineMesh:      (m, o) => this._fieldRenderer.buildStreamlineMesh(m, o),
    writeStreamlinesIntoMesh: (m, s, c) => this._fieldRenderer.writeStreamlinesIntoMesh(m, s, c),
});
```

This forces a **construction-order invariant**: the canonical-home sub-renderer
must be built before its callback consumers. Phase 3 ordering:
SceneCore → FieldRenderer → FluxRenderer → ParticleRenderer.

## Consequences

- (+) Single source of truth per helper; no drift between copies
- (+) No new shared-module dependency; the pattern reuses the live-reference
  factory contract (CONTRACTS.md §1)
- (+) Helpers are publicly named on the canonical sub-renderer
  (`buildStreamlineMesh`, not `_buildStreamlineMesh`), making the API surface
  intentional rather than incidental
- (−) Constructor ordering is now load-bearing; documented in CONTRACTS.md §10

## Alternatives considered

- Per-sub-renderer duplicate copies — rejected: drift risk identical to
  the CUDA index-helper drift that motivated ADR-0007
- Extract to a stateless shared module — viable but adds another file;
  rejected because the canonical helpers depend on `_insideBoundary`
  (orchestrator-owned callback) and would need callback plumbing anyway

## References

- Files: `engine/web/js/viewport/field-renderer.js` (canonical home),
  `engine/web/js/viewport/flux-renderer.js` (consumer),
  `engine/web/js/viewport/particle-renderer.js` (consumer),
  `engine/web/js/viewport.js` (constructor wiring)
- Cross-refs: CONTRACTS.md §11, ADR-0010 (cascade callback), ADR-0007 (CUDA helper consolidation — same family)
