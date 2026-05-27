# ADR 0001 — Decompose `viewport.js`

**Status:** Accepted (deferred execution)
**Date:** 2026-04-14
**Decider:** Engine team
**Context:** Phase D.4 of the April 2026 web-engine refactor

## Context

`engine/web/js/viewport.js` is currently **~3,400 LOC** containing a
single `Viewport` class with ~70 methods that cover the entire scene
graph and per-frame rendering for Scales 0-3 (lattice, particles,
atoms, molecules). It is the largest single live module in the web
engine after `bridge-init.js`, and every scale controller
depends on it.

The file mixes eight distinct concerns (see the categorized header at
the top of `viewport.js`):

1. Scene lifecycle (init, dispose, render, post-processing)
2. Camera & input (OrbitControls, picking)
3. Particle rendering (points + trails + velocity vectors + forces)
4. Boundary rendering (cube/sphere/platonic/cylinder/torus)
5. Molecular rendering (bond lines)
6. Field visualization (E/B/Poynting/divergence/flux/force/gravity/
   halo/damping/genesis/confinement)
7. Volumetric rendering (flux volume + slice)
8. Scenario chrome (event horizon, axes, grid, ontic cube)

At this size the class violates single-responsibility by a wide
margin, and adding any new rendering concern forces developers to
navigate the entire file to understand surrounding state.

## Decision

**Decompose `viewport.js` into a small `Viewport` orchestrator plus
~6 focused renderer modules in a future dedicated initiative.
Do NOT decompose it as part of the April 2026 refactor pass.**

The April 2026 pass (this one) limits itself to:

- Updating the file header to enumerate the eight concern groups.
- Grouping related methods visually in the file (sections 1-8 above).
- Recording this ADR so the eventual split has a target.

## Rationale for deferring execution

1. **High blast radius, no test coverage.** Before this refactor pass
   there was zero automated coverage of the web engine. Phase E.1
   added a 12-test Playwright smoke suite; that is enough to catch
   catastrophic breakage but not enough to catch subtle visual
   regressions during a 3,400-LOC rewrite. A viewport split deserves
   at least the following prerequisites that Phase E.1 explicitly
   deferred:
   - A visual regression harness of some kind (golden-image or
     property-based).
   - Enough manual tests in the Playwright suite to cover every field
     overlay toggle combination.

2. **Every scale controller depends on it.** `scale{0,1,2,3}/
   controller.js` all call into `viewport.*`. Scale 0 alone uses
   ~20 different methods. Any decomposition that preserves the
   public surface area has to route through a facade, and the
   transition period is error-prone.

3. **The consumers of a split are different tools than the consumers
   of the current code.** The current `Viewport` class is a
   legitimate god object for a dashboard prototype. Splitting it only
   pays off if additional consumers appear (a Playwright-driven
   visual-regression harness, a standalone WebXR viewer, a
   decoupled scale-5 renderer, ...) or if the file grows to the
   point where IDE responsiveness or build times suffer. Neither is
   currently true.

4. **The recent revert history demands small diffs.** Commit
   `cccb38f` reverted the entire `engine/web/` tree to a pre-session
   state, which sets the safety posture for this engine: small,
   atomic, individually revertable commits. A 3,400-LOC restructure
   would violate that posture by an order of magnitude and would be
   difficult to revert piecemeal if something regressed.

## Proposed target structure (for the future split)

```
engine/web/js/viewport/
├── viewport.js              (~400 LOC) — orchestrator, public API
├── scene-lifecycle.js       (~300 LOC) — renderer, composer, resize, dispose
├── camera-input.js          (~200 LOC) — camera, OrbitControls, picking
├── particle-renderer.js     (~500 LOC) — points + trails + velocities + forces
├── boundary-renderer.js     (~450 LOC) — cube/sphere/platonic/cylinder/torus
├── field-renderer.js        (~900 LOC) — all Scale 0 field overlays
├── volumetric-renderer.js   (~400 LOC) — flux volume + slice
└── scenario-chrome.js       (~250 LOC) — axes, grid, event horizon, ontic cube
```

The orchestrator holds the public `Viewport` class and delegates to
the sub-renderers. Sub-renderers each own their Three.js objects
(mesh, material, geometry) and expose a minimal `update()` + `dispose()`
contract. The shared scene graph is passed into each sub-renderer's
constructor so nothing reaches into `viewport.scene` directly.

## Prerequisites for execution

Before a follow-up branch executes this split, the following must
land:

1. **Visual regression coverage.** At minimum, the Playwright suite
   should capture a baseline screenshot of each scale (0-3) with each
   major overlay combination toggled on/off. Any alternative approach
   (e.g., a compositor harness that renders to an offscreen canvas
   and diffs pixel-for-pixel against a baseline) must provide
   comparable coverage. GPU nondeterminism makes this non-trivial —
   the harness likely needs a software renderer or large tolerance
   thresholds.

2. **Field-overlay toggle matrix.** Scale 0 has 14 field overlay
   toggles that interact nontrivially (see
   `scale0/controller.js:50-65`). A decomposition has to preserve
   the current matrix semantics. The test harness must exercise at
   least the diagonal (each toggle alone) + a small set of known
   interesting combinations before the split lands.

3. **Frozen Scale 0 behavior.** The Phase D.3 "state ownership
   cleanup" shifted all Scale 0 field flags into
   `scale0/controller.js`. Any ongoing work that moves more state
   around must land before the viewport split begins, or the
   two refactors will conflict on the same hot paths.

## Consequences

### Positive

- Future file sizes stay under 1,000 LOC each, matching the rest of
  the codebase.
- New renderers (a stand-alone WebXR viewer, a decoupled Scale 5
  renderer extraction, etc.) can import the specific sub-renderer
  they need.
- The boundary between scene graph ownership (renderer) and
  application state (controller) becomes explicit.
- Tests can mock individual sub-renderers without mocking the entire
  Three.js scene.

### Negative

- Execution requires substantial time and good test coverage —
  this ADR's prerequisites aren't free.
- The public `Viewport` API has ~40 methods today; preserving binary
  compatibility with existing scale controllers during the split
  requires either a facade or a breaking change to every controller.
  The facade is the safer path but adds indirection.
- Until execution lands, readers of `viewport.js` still have to
  navigate ~3,400 LOC to find a given concern.

## Alternatives considered

1. **Do the split now as part of the April 2026 refactor pass.**
   Rejected — blast radius, no visual regression coverage, violates
   the "small atomic commits" safety posture established after the
   `cccb38f` revert.

2. **Leave `viewport.js` permanently monolithic.** Rejected —
   the file is already the second-largest live module. It will
   keep growing as new field visualizations and scales are added,
   and the cost of eventual decomposition grows with it.

3. **Split only the field-visualization cluster (concern 6).**
   Tempting because it's the biggest section, but partial splits
   leave the orchestrator with half its methods delegated and half
   inline, which is harder to reason about than either extreme.
   Rejected in favor of either all-or-nothing.

## References

- Phase D.4 of `dazzling-tumbling-moth.md` (April 2026 refactor plan)
- `engine/web/js/viewport.js` header comment (categorized concerns)
- Commit `cccb38f` — "revert engine/web/ + engine/wasm/ to pre-session
  state" (precedent for cautious diffs)
