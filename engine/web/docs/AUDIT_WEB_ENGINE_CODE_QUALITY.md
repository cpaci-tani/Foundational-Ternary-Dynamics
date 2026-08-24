# FTD Web Engine — Code-Quality Audit

**Date:** 2026-08-23 · **Scope:** all of `engine/web/js/` (341 files, ~134K LOC) · **Method:** six parallel read-only audit agents, one per subsystem (viewport renderers · bridges/transport · Scale-0 runtime · Scale-0 state/UI · Scales 1–6 · app/libs/config), each hunting DRY, lifecycle, and physics-engine-specific hazards. Report-only; no code was changed.

---

## Executive summary

The web engine is **in good shape for its size and shows extensive prior optimization** (scratch-buffer reuse in hot loops, clip-hoisting, a shared lifecycle base class, single-sourced JS constants, P0/P1/F-* audit markers throughout). The **scale-switch lifecycle is genuinely solid** — a shared `BaseLifecycleController` auto-reclaims listeners/timers/Three.js objects and `switchEngineMode` disposes the previous controller before mounting the next, so there are **no runaway RAF loops or leaked WASM instances** on scale changes.

The residual risk is not catastrophic breakage; it is **five recurring patterns** that will keep generating bugs until addressed:

| # | Theme | Severity | One-line |
|---|-------|----------|----------|
| A | **Parallel-source drift** | HIGH | The same data (sampler registries, `getXSampled` methods, toggle defaults, overlay lists) is written N times and has already silently diverged — one instance is a live cross-backend bug. |
| B | **Incomplete scenario-reset** | HIGH/MED | Module-singleton state (peak-hold normalizers, knot trackers, a leaked panel + timer) is not cleared on scenario/scale change, bleeding across sessions. |
| C | **Per-frame allocation** | MED | A handful of hot paths reintroduced GC churn the codebase elsewhere already eliminated. |
| D | **Zero-copy / concurrency hazards** | MED | Raw heap views handed out on one backend, un-double-buffered flux slices, an async promise that never settles. |
| E | **Config / science-claim drift** | MED | A JS↔C++ default mismatch and a shipped FAQ claim that contradicts the LEDGER. |

**The single highest-leverage cleanup** is a small set of factories/registries that collapse the duplication in Theme A + the panel layer of Theme B.

---

## Critical findings (fix first)

1. **[HIGH · live bug · bridges] Drifted sampler registries break Poisson-latency on every backend except native GPU.** Three parallel `kind→method` maps (`bridge/bridge-contract.js:105`, `bridge/wasm-bridge.worker.js:71`, `ws-bridge.js:55`) have diverged; `getPoissonLatencySampled` exists only on `ws-bridge.js:2675`, but `field-sample-cache.js:78` dispatches `poissonLatency` through `getSamplerOr` on all backends → empty result + the contract's own anti-drift warning fires in production. **Fix:** one exported registry as the single source of truth with per-backend "supported" flags; generate the per-kind methods from it.

2. **[HIGH · leak · Scale-0 UI] The genesis-burst panel leaks across a scale switch.** `genesis-burst-panel.js:182` installs a 500 ms `setInterval` self-dispose poll keyed on the Scale-0 dropdown value, which never changes on a *scale* switch, and `controller.js:333` omits it from the 11-panel disposal list. Result: a floating DOM panel + a 2 Hz timer run forever over unrelated scales. **Fix:** add its dispose to `Scale0LifecycleController.destroy()`.

3. **[HIGH · latent correctness · bridges] In-thread `getParticleData()` returns raw embind heap views** (`wasm-bridge.js:384`) with no copy — the next `tick()`/`inject*` clobbers them in place. The worker copies and WS returns non-aliased views, so code correct on the default path is silently wrong in the in-thread fallback. **Fix:** copy on return (as the worker does), or document + enforce a consume-before-next-call contract.

4. **[MED→HIGH · leaks · viewport] Two teardown paths drop live resources.** `particle-renderer.dispose()` (`:951`) disposes only 1 of its 4 force-arrow layers (`_peForceCoulomb/Gravity/Strong` leak geometry+material and orphan meshes on every scale switch); `scene-core.dispose()` misses `_areaHighlight`. **Fix:** dispose all four layers + the area highlight.

---

## Theme A — DRY / parallel-source drift (dominant)

The engine repeatedly encodes one fact in several hand-maintained places. Every instance below has either already drifted or is one edit away from it.

- **Sampler surface written 3–4×.** Beyond the registries (critical #1): the ~18 `getXSampled` methods are hand-written once per bridge (`wasm-bridge.js:703`, `wasm-bridge-proxy.js:519`, `ws-bridge.js:2660`) so adding one sampler is a 4–6-file change; the empty `EMPTY_FIELD_SAMPLE`/`EMPTY_SCALAR_SAMPLE`/`EMPTY_KNOT_*` singletons are defined 4× and inconsistently (`bridge-contract.js:82` vs each bridge). **Fix:** generate per-kind methods from the unified registry via a shared mixin; export the empty singletons once.
- **`TOGGLE_REQUIRES` duplicated verbatim** between `wasm-bridge.js:652` and `wasm-bridge.worker.js:138`, both shadowing C++ `TermToggles::validate()`. **Fix:** hoist to `bridge-contract.js`.
- **Point-cloud builder copy-pasted 7×** across the field-renderer mixins (`field-em-renderer.js:15/69/662`, `field-topology-renderer.js:30/285`, `field-quantum-renderer.js:11/96`) — with visible drift (`field-em-renderer.js:669` sets a dead duplicate `color` attribute). An existing `mesh-factory.buildArrowFieldMesh` is **bypassed by 11 would-be callers**, and the canonical `_writeArrowFieldIntoMesh` is re-implemented by two of its siblings (`field-em-renderer.js:603`, `field-force-renderer.js:43`). **Fix:** one `_buildParticlePointsCloud(...)` in `field-renderer-shared.js`; route line/arrow builders through `mesh-factory`.
- **12 overlay panels hand-roll identical mount/dispose/singleton/rAF scaffolding** (~200–250 dup LOC across dispersion/gravity/time/thermo/spectrum/scale-context/… panels), plus `ensureCss()` copied 6× and the arm→live rAF-subscribe idiom copied 5×. This duplication is *why* the double-mount (below) and knots-dispose bugs exist — each copy drifts. **Fix:** a `createSingletonPanel({panelId, hostId, build, wire, hz, update})` factory + a `rafCoordinator.subscribeWhenLive(...)` helper resolves five findings at once.
- **Camera save/restore triplicated + already drifted** across the self-bridged scales (`scale4/controller.js:72`, `scale5:120`, `scale6:72`) — scale5 omits the `controls.update()` the other two call. **Fix:** hoist `saveCameraState`/`restoreCameraState` into `scale-utils.js`.
- **Overlay set enumerated a 4th time** in `viewport-adapter.js:182` (`clearScaleVisuals` hand-lists ~35 toggles that duplicate its own three maps). **Fix:** iterate the maps.
- **Dead, drifted legacy overlay path.** `field-overlays.js:59/222/468` (`buildQuantumOverlayData`/`applyOverlayFrame`/`sampleFieldState`) has no callers and already lags the live `SCALAR_JOBS` table by 3 overlays. **Fix:** delete it.

## Theme B — Lifecycle: scenario-reset gaps + panel layer

Scale-*switch* teardown is solid; scenario-*load* reset and the panel layer are not.

- **Peak-hold normalizers never reset** (`overlay-frames.js:39`): `state.decayingMax` survives scenario load, so a strong→weak switch renders EM-energy/pressure/vorticity clouds washed-out and the horizon overlay *blank* for ~7 s. **Fix:** clear it in `resetFrameState`.
- **Knot trackers never reset** on scenario load (`field-line-knots.js:674`) — if tracking is on across a switch, the new field is matched against the old scenario's knot IDs, emitting phantom birth/death/fission events. **Fix:** `forEachKnotTracker(t => t.reset())` in the scenario-load reset.
- **Unbounded WS caches** (`ws-bridge.js:238`): `_voxelCache`/`_forceAtCache` are string-keyed maps with no LRU, cleared only on scenario change — every hovered/probed voxel accumulates permanently within a long scenario. **Fix:** small LRU or evict on the per-tick epoch bump.
- **7 panels double-mount at boot** (`app.js:490` then `controller.js:300`); only `rafCoordinator`'s keyed-replace averts a hard rAF leak (and it `console.warn`s per panel per load). The unguarded `initXPanel`s orphan their first `api` without `dispose()`. **Fix:** the singleton guard the other three already use, or the panel factory.
- **`knots-panel.dispose()` never nulls its singleton** (`:515`), breaking the contract the 11 siblings rely on. **Fix:** null with the `=== api` guard.
- **Narrower retention:** `scale4/5` in-place reloads accumulate disposed-renderer refs in `_threeObjects` (no `untrack` in `lifecycle.js`); `ws-bridge.dispose()` doesn't set `ready=false`; `getParticleDataAsync()` (`ws-bridge.js:2258`) is never settled on `onclose`/`onerror`/`dispose` → an awaiting caller hangs forever after a scale switch. **Fix:** add `untrack`; set `ready=false`; settle the binary-resolve slot in all teardown paths.

## Theme C — Per-frame allocation (hot path)

The steady-state hot path is mostly allocation-free; these are the leaks *out* of that discipline.

- **`lerpPalette` returns a fresh `[r,g,b]` per voxel** (`color-ramps.js:204`) in three force-overlay loops (~8000 allocs/overlay/frame). **Fix:** an in-place `lerpPaletteInto(pal,t,out,i)`.
- **Flux-line coloring reintroduced the old allocating index** (`field-overlays.js:203`): `buildFluxStreamlines` re-runs `buildFieldIndex` (Array-of-Arrays rebuild) + per-vertex `lookupField` (fresh tuple) — ~30k throwaway arrays/frame, the exact pattern the CSR rewrite removed. **Fix:** allocation-free `lookupFieldInto`; retain the persistent index from `computeStreamlines`.
- **Per-frame string-keyed Map/Set joins** in `computeEmEnergyFrame`/`computeLagrangianDensityFrame` (`overlay-frames.js:270/121`) — the repo already solved this with bit-packed integer keys in `manifestation-flash.js:30`. **Fix:** bit-pack voxel coords, reuse persistent Maps.
- **Atom-engine MD hot loop** (`mock-atom-engine.js`): allocates a fresh force array + N force objects twice per tick **and double-counts every pair** (`j=0…N` instead of `j=i+1` with Newton's-3rd-law accumulation) — 2× the O(N²) work. The cosmic engine already fixed this exact pattern (`_dataBuf`). **Fix:** persistent buffers + single `i<j` loop.
- **Smaller:** `manifestation-flash` allocates 2 arrays + a Set per frame; `insideBoundary` rebuilds face-normal arrays per voxel for platonic boundaries; `updatePhaseField` allocates `new Float32Array(3)` per frame.

## Theme D — Zero-copy / concurrency hazards

- **Flux path has no double-buffering** (`wasm-bridge.worker.js:244` + `wasm-bridge-proxy.js:443`): the worker memcpys the live field while the main thread reads `_fluxView` unsynchronized → torn (half-old/half-new) slices during playback; and `_fluxView` is captured once, so a mid-scenario SAB growth detaches it and silently **blanks the flux overlay until reload**. **Fix:** double-buffer with an atomic swap index; re-publish the heap/ptr/len on heap growth.
- **Sticky sampler demand** (`wasm-bridge-proxy.js:494`): `_wantSampler` pins a per-frame worker computation forever unless the consumer calls `unwantSampler`; hiding a row without it leaks compute for the session. **Fix:** refcount wants to overlay lifecycle, or a TTL like the WS telemetry demand.
- **Unknown binary frames treated as legacy particle data** (`ws-bridge.js:2077`) — a stray/misversioned frame clears the in-flight latch and can publish nonsense. **Fix:** gate the legacy path on an explicit flag; drop unknown magics.

## Theme E — Config & science-claim drift

- **JS↔C++ toggle default mismatch:** `gravity` is `false` in `config/toggles.js:155` but `true` in `term_toggles.h:52`. Masked because Scale-0 always pushes the JS table at boot, but a fresh `ws_server.exe` session or non-Scale-0 consumer sees `true`. Every other overlapping default matches — which makes this drift the insidious kind. **Fix:** generate the JS table from the C++ struct (or a shared JSON), or add a CI diff.
- **FAQ overclaims withdrawn evidence:** `ui/components/faq/data.js:185` still states `x₊=1/α` is "supported by … structural-uniqueness scans (~4×10⁵:1)" — formally **withdrawn** per LEDGER FTD-0791/0802 ("numerical-uniqueness support ZERO"). Shipped UI content contradicting the project's top epistemic invariant. **Fix:** strike the clause; keep the accurate "1.26 ppm at tree level, no derivation chain" framing (`app-ontic.js:92` already models the right pattern).
- **Minor:** the main animation loop (`app.js:662`) is a raw self-rescheduling rAF whose id is discarded — it bypasses the `rafCoordinator` (so no pause/cancel/teardown for the *primary* loop) and the six scales use three different frame-loop driver patterns. `scale1/scenario-registry.js:188` hardcodes `1836` instead of importing `PROTON_RATIO`. `lib/` vs `core/` has no stated boundary. The `t-evaporation` binding and (from the recent heat-map work) `scalarRenderMode` are not reset symmetrically with their siblings.

---

## What's already strong

- **Scale-switch teardown** — shared `BaseLifecycleController`, `destroy()`→`mount()` pairing, self-cleaning rafCoordinator subs, WASM engine held as a reused singleton. No leaked loops/instances.
- **Hot-path discipline** — the streamline integrator, persistent CSR spatial index, pooled result ring, and closure-free job dispatcher are allocation-free in steady state; the `fieldDataVersion` gate is correct.
- **Renderer tuning** — flux/topology renderers use peak-hold decay, bilinear splat + box blur, and build-cache guards; hot loops reuse `_ensureActiveIdx`/`_magCache` scratch.
- **Constants** — `constants.js` is a genuine single source on the JS side (no drift found on G\*, α, C_SPEED, DM fraction).
- **Both N-body Verlet integrators + the atomic MD integrator** are correctly-structured symplectic kick-drift-kick with sensible fixed timesteps; no numerical-correctness defects found.

---

## Prioritized recommendations

**Tier 1 — correctness / live bugs (small, high-value):**
1. Fix the sampler-registry drift → restores Poisson-latency on WASM (critical #1).
2. Dispose the genesis-burst panel on scale switch (critical #2).
3. Copy or contract-document the in-thread `getParticleData` views (critical #3).
4. Dispose all 4 particle force-layers + `_areaHighlight` (critical #4).
5. Reset `decayingMax` + knot trackers on scenario load (Theme B).
6. Settle `getParticleDataAsync` on teardown; set `ws-bridge ready=false` (Theme B).

**Tier 2 — structural DRY (biggest long-term leverage):**
7. One sampler registry + generated per-bridge methods + shared empty singletons (kills Theme A's bridge half and prevents future drift).
8. A `createSingletonPanel` factory + `subscribeWhenLive` (kills the 12-panel duplication and its double-mount/dispose bugs).
9. A shared `_buildParticlePointsCloud` + route builders through `mesh-factory` (kills the 7×/11× renderer duplication).
10. Generate the physics-toggles card + JS toggle table from a single source; add a CI diff against `term_toggles.h`.

**Tier 3 — hot-path perf (do if profiling shows GC hitches):**
11. `lerpPaletteInto`, allocation-free flux-line coloring, bit-packed frame-join keys, atom-engine buffer reuse + single-pass pairs.

**Tier 4 — housekeeping:** delete the dead legacy overlay path; migrate the main loop into `rafCoordinator`; fix the FAQ science claim; the assorted LOW items.

---

*Per-subsystem raw findings (with every file:line and fix) are preserved in the six agent reports that produced this synthesis.*

---

## Resolution (2026-08-24)

The findings were worked in seven verified, per-batch commits (staged by path; no
engine/golden impact — all changes are JS/HTML). Each item below is marked
**Fixed** (with commit), **Not-a-bug** (with evidence), or **Deferred** (with the
reason it was not safe/valuable to fix in this pass).

### Fixed

| Finding | Commit |
|---------|--------|
| Crit #2 — genesis-burst panel leak on scale switch (dispose added to `Scale0LifecycleController.destroy`) | `c73d15b2` |
| Crit #3 — in-thread `getParticleData()` raw embind views (now copied into grow-in-place scratch, length-exact) | `c73d15b2` |
| Crit #4 — `particle-renderer.dispose` disposes all 4 force layers; `scene-core.dispose` disposes `_areaHighlight` | `c73d15b2` |
| Theme B — `decayingMax` peak-hold cleared in `resetFrameState`; knot trackers reset on scenario load | `c73d15b2` |
| Theme B — `getParticleDataAsync` binary slot settled on all teardown paths; `ws-bridge.dispose` sets `ready=false` | `c73d15b2` |
| Theme A — dead/drifted legacy overlay path deleted (`buildQuantumOverlayData`/`applyOverlayFrame`/`sampleFieldState`/`buildElectromagneticOverlayData`, 143 LOC) | `2c2cf382` |
| Theme C — flux-line coloring allocation removed (`buildPersistentIndex` + `sampleFieldMagInto`, no per-vertex tuples) | `2c2cf382` |
| Theme C — per-frame string-keyed frame-joins → bit-packed `posKey` in `overlay-frames.js` | `2c2cf382` |
| Theme B — unbounded WS `_voxelCache`/`_forceAtCache` given a 4096-entry FIFO cap | `223d9e26` |
| Theme C — `lerpPaletteInto` allocation-free writer; 3 force-overlay loops routed through it | `92a09f2c` |
| Theme A — dead duplicate `color` BufferAttribute in the divergence field removed | `92a09f2c` |
| Theme C — `insideBoundary` platonic face-normals hoisted to pre-normalized module tables | `92a09f2c` |
| Theme C — confinement spatial hash: `"x,y,z"` strings → exact base-4096 integer key | `92a09f2c` |
| Theme C — `molecular-renderer._updateBonds` per-bond THREE allocations reuse hoisted scratch | `92a09f2c` |
| Theme B — `knots-panel.dispose` nulls its `window.__ftdKnotsPanel` singleton with the `=== api` guard | `70724d35` |
| Theme E — `scalarRenderMode` (heat-map meta-toggle) now captured/reset/restored symmetric to `forceStyle` | `70724d35` |
| Theme A — camera save/restore de-triplicated into `scale-utils.saveScaleCameraState`/`restoreScaleCameraState` (scales 4/5/6) | `e77da7c9` |
| Theme E — FAQ α card: withdrawn structural-uniqueness support struck, honest [SMC]/CODATA framing stated | `d1300d85` |
| Theme E — `gravity` JS↔C++ default divergence documented as intentional (JS whitelist wins at runtime) | `d1300d85` |

### Not-a-bug (reassessed on closer trace)

- **Crit #1 — Poisson-latency registry drift is *not* a live bug.** The only site
  that requests the `poissonLatency` kind is `scale0FieldKindOverrides`
  (`field-overlays.js:47`), which returns the override **only when
  `active.isNativeGPU`** — i.e. exactly the backend (`ws-bridge`) that implements
  `getPoissonLatencySampled`. On WASM the `latency` kind never becomes
  `poissonLatency`, so no empty result is produced. The registries *have* drifted
  (a real maintainability smell) but the drift does not surface as a runtime
  fault. Unification is retained below as a Deferred DRY item.
- **`scale1` `resetScale1`/`destroy`** are intentional aliases onto one lifecycle
  destroy; `loadPEScenario`'s `resetAllVisualState()` is a distinct master reset —
  no double-call defect.
- **`scale3` destroy AE reset** — the shared atom-engine is re-initialised by the
  next scale's `initAE()` on mount, so molecule state does not leak across a switch.
- **`mock-scale4` SUBSTEPS** — already a named constant.
- **`updatePhaseField` scratch** — the `Float32Array(3)` is already hoisted out of
  the per-voxel loop (one alloc/frame); negligible.

### Deferred (with reason)

- **Sampler-registry unification + `TOGGLE_REQUIRES` hoist + generated `getXSampled`
  mixin + shared empty singletons (Theme A / rec 7).** `wasm-bridge.worker.js` is a
  **classic** worker (Emscripten `importScripts`, instantiated with no
  `{type:'module'}`) and cannot ES-import a shared registry module. The duplication
  is architectural; collapsing it needs a module-worker conversion, out of scope for
  a drive-by pass. (And per Not-a-bug above, no correctness fault is pending on it.)
- **`createSingletonPanel` factory + `subscribeWhenLive` (rec 8), `_buildParticlePointsCloud`
  + `mesh-factory` routing (rec 9), physics-toggles/JS-table generation + CI diff
  (rec 10).** Large DOM/renderer-generation refactors whose behavior cannot be
  verified in the frozen-rAF preview pane; each warrants its own tested change.
- **`clearScaleVisuals` iterate-maps (Theme A).** The explicit `viewport.toggleX?.(false)`
  list is a *completeness-critical* teardown and refactor-safe (renaming a method
  breaks the call visibly); a string-keyed loop would trade that for silent-drift risk.
- **`BaseLifecycleController.untrack` + scale4/5 `_threeObjects` (Theme B).**
  `trackThreeObject` fires exactly once per controller (at mount, tracking the
  renderer/root) — no path rebuilds a tracked object — so `untrack` would be dead API.
- **Atom-engine half-pairs + buffer reuse (Theme C).** The pair loop documents a
  bit-identity requirement on its f64/f32 summation order; half-pairs would change it.
  N is tiny (dozens of atoms) and the force buffers are returned to the caller, so
  scratch reuse would alias. Real risk, no measurable payoff.
- **Flux double-buffering + sticky-sampler TTL + unknown-magic gate (Theme D).** The
  flux SAB double-buffer with atomic swap is a genuine concurrency change needing a
  dedicated, tested pass; grouped with the sampler-demand refcount and the binary-
  frame magic gate.
- **Main-loop → `rafCoordinator` / frame-loop unification (Theme E).** `animate()` is a
  deliberately plain, throw-isolated unconditional rAF, and scale4 already drives its
  own coordinator subscription — unifying risks double-driving and changing error
  isolation, and is unverifiable in the frozen preview.
- **`scale1/scenario-registry.js:188` `1836` → `PROTON_RATIO`.** That file is owned by a
  concurrent session (uncommitted `M`) and must not be staged from web work.
- **7-panel double-mount guard, `lib/`-vs-`core/` doc.** The double-mount is already
  averted by `rafCoordinator`'s keyed-replace (its full fix is the deferred panel
  factory); the directory-boundary note is low-value and staleness-prone.
