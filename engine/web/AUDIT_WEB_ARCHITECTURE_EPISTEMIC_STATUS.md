# FTD Web Engine — Epistemic Status Checklist

**Scope:** `engine/web/ARCHITECTURE.md`
**Purpose:** Explicitly classify which architecture claims are directly
verified from source, which are documentation-level abstractions, and
which still require runtime verification.

---

## 1. Tagging policy for this audit

This checklist applies the project epistemic labels conservatively to
software-architecture claims:

- **[THEOREM]**: directly source-verified in the current checked files.
  For software, read this as "traceable in code," not "proven from FTD
  axioms."
- **[IMPOSED]**: explicit implementation choice, constant, URL, or
  lifecycle rule encoded in code.
- **[SELECTION]**: documentation compression or explanatory framing.
  Useful, but not uniquely forced by the code.
- **[EMERGENT]**: runtime behavior expected from the code path, but best
  verified in a live browser session.
- **[CONJECTURE]**: plausible interpretation not traced tightly enough
  to promote to source-verified status.
- **[OPEN]**: not fully audited or still carrying meaningful uncertainty.

When in doubt, this audit uses the weaker label.

---

## 2. High-level verdict

- **[x] [THEOREM]** The main control-flow claims in
  `ARCHITECTURE.md` are source-grounded.
  Evidence: `engine/web/js/app_dag.js`, `engine/web/js/wasm-bridge-dag.js`,
  `engine/web/js/scales/scale*/controller.js`, `engine/wasm/ftd_wasm.cpp`,
  `engine/src/render_bridge.cpp`.

- **[x] [SELECTION]** The document's "three-layer" picture is a useful
  explanatory compression, not a literal formal decomposition in code.
  Evidence: the runtime really does have app shell, controllers, and
  backends, but also has cross-cutting helpers such as `viewport.js`,
  `inspector.js`, charts, and DOM wiring.

- **[x] [OPEN]** Some statements remain only partially audited because
  they are runtime-sensitive rather than purely static.
  Evidence: native-bridge reconnection behavior, live inspector mapping,
  disposal cleanliness, and framerate-dependent claims were not executed
  exhaustively in a browser session during this pass.

---

## 3. Checklist

### 3.1 Entry point and boot

- **[x] [THEOREM]** `index.html` loads exactly one module entry:
  `js/app_dag.js`.
  Evidence: `engine/web/index.html`.

- **[x] [THEOREM]** `init()` is the real browser boot entry point.
  Evidence: `engine/web/js/app_dag.js`.

- **[x] [THEOREM]** The app tries the native WebSocket bridge first,
  then falls back to `createBridge()`.
  Evidence: `app_dag.js` `init()`, `ws-bridge.js`, `bridge-factory-dag.js`.

- **[x] [THEOREM]** `createBridge()` itself only chooses between WASM
  and `MockBridge`; it does not probe the native server.
  Evidence: `engine/web/js/bridge/bridge-factory-dag.js`.

- **[x] [THEOREM]** The default boot scenario is `flux-pulse`.
  Evidence: `app_dag.js` `Scale0Controller.loadScenario(_makeCtx(), 'flux-pulse')`.

- **[x] [THEOREM]** Scale 0 UI binding now happens during boot through
  `Scale0Controller.bindUI(_makeCtx())`.
  Evidence: `engine/web/js/app_dag.js`.

- **[x] [EMERGENT]** The loading overlay should disappear after startup
  completes, with an 8-second safety timeout if init hangs.
  Evidence: `app_dag.js`.
  Note: source-verified statically, but user-visible timing behavior was
  not exercised live in this audit.

### 3.2 Bridge selection and backend reality

- **[x] [THEOREM]** `WasmBridge.init()` injects `wasm/ftd_core.js` on
  demand and then calls `createFTDModule({ locateFile })`.
  Evidence: `engine/web/js/wasm-bridge-dag.js`.

- **[x] [THEOREM]** The browser substrate bridge is instantiated as
  `module.RenderBridge`, not `module.DagEngine`.
  Evidence: `wasm-bridge-dag.js`, `engine/wasm/ftd_wasm.cpp`.

- **[x] [THEOREM]** The native server URL is `ws://localhost:9100`.
  Evidence: `engine/web/js/ws-bridge.js`.

- **[x] [IMPOSED]** The native-bridge endpoint and retry strategy are
  implementation choices, not emergent properties.
  Evidence: `ws-bridge.js`.

- **[x] [THEOREM]** The native bridge retries forever with exponential
  backoff.
  Evidence: `ws-bridge.js` `_scheduleReconnect()`.

- **[x] [THEOREM]** Scale 0 and Scale 1 can use real C++ browser paths
  through WASM today.
  Evidence: `wasm-bridge-dag.js`, `ftd_wasm.cpp`.

- **[x] [THEOREM]** `WasmBridge` and `MockBridge` now expose
  lazily-built capability surfaces including `scale0`, `scale1`, and
  `scale2`.
  Evidence: `engine/web/js/wasm-bridge-dag.js`.

- **[x] [THEOREM]** The web AE path is forced onto JavaScript fallback
  because `_aeHasWasm` returns `false`.
  Evidence: `wasm-bridge-dag.js`.

- **[x] [IMPOSED]** The reason given for disabling AE WASM is the unit
  mismatch between Planck-scaled engine units and Bohr-scaled web UI
  units.
  Evidence: inline comment in `wasm-bridge-dag.js`.

- **[ ] [OPEN]** Native WebSocket feature parity with the WASM bridge
  was not exhaustively audited in this pass.
  Evidence gap: no end-to-end live native session executed.

### 3.3 Main scheduler and mode dispatch

- **[x] [THEOREM]** `app_dag.js` owns a single unconditional rAF loop
  for every mode except planetary physics.
  Evidence: `engine/web/js/app_dag.js` `animate(now)`.

- **[x] [THEOREM]** The next rAF is scheduled before mode dispatch.
  Evidence: first line of `animate(now)`.

- **[x] [THEOREM]** Dispatch by `engineMode` matches the architecture
  document's table.
  Evidence: `app_dag.js` `animate(now)`.

- **[x] [THEOREM]** Scale 4 planetary physics runs in its own
  `setInterval(..., 16)` loop.
  Evidence: `engine/web/js/scales/scale4/controller.js`.

- **[x] [THEOREM]** Scale 5 cosmic runs from rAF and only advances
  physics on every other frame.
  Evidence: `engine/web/js/scales/scale5/controller.js`.

- **[x] [EMERGENT]** The "roughly 30 Hz" cosmic physics cadence is only
  true when the browser is presenting at roughly 60 Hz.
  Evidence: every-other-frame logic in `scale5/controller.js`.

- **[x] [SELECTION]** The scheduler chapter in `ARCHITECTURE.md`
  intentionally compresses background updates, FPS bookkeeping, and
  floating-panel tracking into one control-flow narrative.
  Evidence: `app_dag.js` contains more detail than the summary text.

### 3.4 Play, step, reset, mode switch

- **[x] [THEOREM]** Play toggles `running`; it does not directly tick
  the simulation.
  Evidence: `app_dag.js` `togglePlay()`.

- **[x] [THEOREM]** Step dispatches directly to per-mode tick or step
  entry points.
  Evidence: `app_dag.js` Step button handler.

- **[x] [THEOREM]** Lattice step now dispatches through
  `Scale0Controller.step(_makeCtx())`.
  Evidence: `engine/web/js/app_dag.js`.

- **[x] [THEOREM]** Reset reloads the current mode's scenario instead of
  performing a universal engine reset.
  Evidence: `app_dag.js` Reset button handler.

- **[x] [THEOREM]** Lattice reset now dispatches through
  `Scale0Controller.reset(_makeCtx())`.
  Evidence: `engine/web/js/app_dag.js`.

- **[x] [THEOREM]** `switchEngineMode()` is the sole mode-switch entry
  point.
  Evidence: comment and dispatch structure in `app_dag.js`.

- **[x] [THEOREM]** Mode switching pauses simulation, updates CSS/tab
  visibility, performs scale cleanup, then loads the target scenario.
  Evidence: `app_dag.js` `switchEngineMode(mode)`.

- **[x] [THEOREM]** Leaving lattice now routes cleanup through
  `Scale0Controller.exit(_makeCtx())`.
  Evidence: `engine/web/js/app_dag.js`.

- **[ ] [OPEN]** Full cleanup correctness across long repeated mode
  switches was not re-run manually in this audit.
  Evidence gap: static review plus smoke-test reading only.

### 3.5 JS <-> WASM <-> C++ handoff

- **[x] [THEOREM]** `ftd_wasm.cpp` is the web binding layer that exposes
  `RenderBridge`, `ParticleEngine`, `AtomEngine`, and free helper
  functions.
  Evidence: `engine/wasm/ftd_wasm.cpp`.

- **[x] [THEOREM]** The substrate tick path documented in
  `ARCHITECTURE.md` matches the code:
  `WasmBridge.tick()` -> `RenderBridge::tick()`.
  Evidence: `wasm-bridge-dag.js`, `ftd_wasm.cpp`, `render_bridge.cpp`.

- **[x] [THEOREM]** The CPU `RenderBridge::tick()` order documented in
  `ARCHITECTURE.md` matches the real call sequence.
  Evidence: `engine/src/render_bridge.cpp`.

- **[x] [THEOREM]** The PE path documented in `ARCHITECTURE.md` matches
  the real call sequence:
  `bridge.peTick()` -> `this._pe.tick()`.
  Evidence: `wasm-bridge-dag.js`, `ftd_wasm.cpp`.

- **[ ] [OPEN]** The GPU/native `RenderBridge::tick()` path was only
  read statically, not exercised live.
  Evidence gap: no CUDA/native runtime session in this pass.

### 3.6 Scale 0 `lattice`

- **[x] [THEOREM]** Scale 0 owns the most complex controller-level frame
  path in the current web engine.
  Evidence: `engine/web/js/scales/scale0/controller.js`.
  Note: "most complex" is partly a documentation judgment; the concrete
  multi-bridge, multi-overlay behavior is source-verified.

- **[x] [THEOREM]** Scale 0 now follows a package-style structure with
  dedicated runtime, state, UI, scenario-registry, and viewport-adapter
  modules.
  Evidence: `engine/web/js/scales/scale0/`.

- **[x] [THEOREM]** `_useFluxMock` is forced for `flux-*`,
  `s0-seed-*`, and `s0-field-*` scenarios, and otherwise decided by a
  flux-volume capability probe.
  Evidence: `shouldUseFluxMock()` in
  `engine/web/js/scales/scale0/runtime/scenario-loader.js`.

- **[x] [THEOREM]** Scale 0 scenario load creates a fresh `_fluxMock`,
  resets toggles to defaults, applies overrides, and marks lattice data
  dirty.
  Evidence: `Scale0Controller.loadScenario()`,
  `loadScale0Scenario()` in `scale0/runtime/scenario-loader.js`.

- **[x] [THEOREM]** Scale 0's frame path is explicitly decomposed into
  simulation advance, renderable-data sync, overlay update, viewport
  render, and diagnostics/panel refresh phases.
  Evidence: `Scale0Controller.animate()`,
  `scale0/runtime/tick.js`,
  `scale0/runtime/frame-sync.js`,
  `scale0/runtime/field-overlays.js`,
  `scale0/runtime/diagnostics.js`.

- **[x] [THEOREM]** Scale 0 UI ownership for field toggles, force-style
  controls, boundary controls, flux volume/slice toggles, keyboard
  shortcuts, and scenario selection now lives inside the Scale 0
  package.
  Evidence: `scale0/ui/bindings.js`, `app_dag.js`.

- **[x] [THEOREM]** Scale 0 now consumes backend data through
  `bridge.capabilities.scale0` rather than reading `MockBridge` private
  fields directly.
  Evidence: `scale0/runtime/*.js`, `wasm-bridge-dag.js`.

- **[x] [THEOREM]** Scale 0 scenario selection is registry-driven and
  can be validated through `validateScale0ScenarioRegistry()`.
  Evidence: `scale0/scenario-registry.js`,
  `scale0/ui/bindings.js`,
  `engine/web/tests/scales.spec.js`.

- **[ ] [OPEN]** Overlay correctness for every optional field layer
  was not audited live in-browser.

### 3.7 Scale 1 `particles`

- **[x] [THEOREM]** Scale 1 ticks via `bridge.peTick()` and reads back
  PE particle/diagnostic data through the bridge facade.
  Evidence: `engine/web/js/scales/scale1/controller.js`.

- **[x] [THEOREM]** Scale 1 uses a cloud-expansion render path rather
  than rendering raw PE particle data directly.
  Evidence: `scale1/controller.js`.

- **[x] [SELECTION]** The architecture doc's phrase "cleanest real C++
  browser path after lattice" is explanatory, not a formal property.

### 3.8 Scale 2 `atoms` and Scale 3 `molecules`

- **[x] [THEOREM]** Scale 2 and Scale 3 share the same steady-state
  animation loop through `animateAE()`.
  Evidence: `engine/web/js/scales/scale3/controller.js`.

- **[x] [THEOREM]** Scale 2 ticks through the AE facade, which resolves
  to JS fallback today.
  Evidence: `scale2/controller.js`, `wasm-bridge-dag.js`.

- **[x] [THEOREM]** Scale 3's distinctive behavior is in scenario load,
  not in a separate frame scheduler.
  Evidence: `scale3/controller.js`.

- **[x] [THEOREM]** Molecule loading includes a one-tick stability
  dry-run and a reset before the actual run.
  Evidence: `Scale3Controller.loadMoleculeScenario()`.

- **[ ] [OPEN]** Numerical stability claims for molecule scenarios were
  not independently verified in this documentation pass.

### 3.9 Scale 4 `planetary`

- **[x] [THEOREM]** Scale 4 owns a private `PlanetaryMockBridge` and
  `PlanetaryRenderer`.
  Evidence: `engine/web/js/scales/scale4/controller.js`.

- **[x] [THEOREM]** Scale 4 hides shared lattice visuals on entry and
  restores shared particle visibility on dispose.
  Evidence: `scale4/controller.js`.

- **[x] [THEOREM]** Step and dispose behavior described in
  `ARCHITECTURE.md` matches the controller.
  Evidence: `scale4/controller.js`.

### 3.10 Scale 5 `cosmic`

- **[x] [THEOREM]** Scale 5 owns a private `CosmicMockBridge` and
  `CosmicRenderer`.
  Evidence: `engine/web/js/scales/scale5/controller.js`.

- **[x] [THEOREM]** Cosmic no longer uses a dedicated interval and
  instead advances on every other rAF frame.
  Evidence: `scale5/controller.js`, `engine/web/tests/scales.spec.js`.

- **[x] [EMERGENT]** Smooth OrbitControls behavior is expected because
  rendering still happens every rAF frame.
  Evidence: controller code and inline comments.
  Note: not re-observed live in this audit.

### 3.11 `meta`

- **[x] [THEOREM]** `meta` uses `Scale6Controller.updateMeta(ctx, 1/60)`
  from the main rAF loop.
  Evidence: `app_dag.js`, `scale6/controller.js`.

- **[x] [THEOREM]** The controller file is `scale6`, while the UI scale
  index is `12`.
  Evidence: `app_dag.js`.

- **[x] [SELECTION]** Calling this a "historical numbering mismatch" is
  a documentation interpretation of what the code shows.

### 3.12 Scale 11 `consciousness`

- **[x] [THEOREM]** Scale 11 swaps the active bridge to a flux-only
  `MockBridge(32)` and stores the original in `_savedBridge`.
  Evidence: `engine/web/js/scales/scale11/controller.js`.

- **[x] [THEOREM]** `resetScale11()` restores the saved bridge.
  Evidence: `scale11/controller.js`.

- **[x] [THEOREM]** The frame path documented in `ARCHITECTURE.md`
  matches the controller:
  per-tick injections, `bridge.tick()`, energy-audit readout,
  `_csEngine.update(...)`, DOM updates, `viewport.render()`.
  Evidence: `scale11/controller.js`.

- **[x] [SELECTION]** Describing Scale 11 as "the strangest lifecycle"
  is explanatory language, not a formal property.

- **[ ] [OPEN]** Long-session listener cleanliness beyond the existing
  regression test was not manually re-exercised.

### 3.13 Rendering ownership

- **[x] [THEOREM]** `viewport.js` owns the shared scene, camera,
  renderer, controls, particle cloud, and many overlays.
  Evidence: `engine/web/js/viewport.js`.

- **[x] [THEOREM]** `PlanetaryRenderer`, `CosmicRenderer`, `MetaUnit`,
  and `ConsciousnessEngine` add specialized content into the shared
  scene.
  Evidence: their constructors and load paths.

- **[x] [SELECTION]** The "god object" description of `viewport.js` is a
  documentation judgment, though a well-supported one.

### 3.14 Inspector and read-side queries

- **[x] [THEOREM]** The inspector is mode-aware and receives distinct
  contexts for lattice, particles, atoms/molecules, and planetary/cosmic.
  Evidence: `engine/web/js/inspector.js` plus controller wiring.

- **[x] [SELECTION]** The architecture doc's phrase "main read-side
  query path" is an explanatory framing of how the inspector is used.

- **[ ] [OPEN]** Exact click-to-entity mapping behavior for every mode
  was not revalidated interactively in this pass.

### 3.15 Testing claims

- **[x] [THEOREM]** The smoke suite documentation claims in
  `ARCHITECTURE.md` are supported by `engine/web/tests/README.md` and
  `engine/web/tests/scales.spec.js`.
  Evidence: both files inspected directly.

- **[x] [THEOREM]** The suite covers scale sweep, bridge init, cosmic
  interval regression, consciousness listener regression, and constants.
  Evidence: `tests/README.md`, `tests/scales.spec.js`.

- **[x] [THEOREM]** The suite now also covers the Scale 0 module
  contract surface and the validity of the Scale 0 scenario registry.
  Evidence: `tests/scales.spec.js`.

- **[x] [THEOREM]** The suite does not claim physics correctness.
  Evidence: `tests/README.md`, `scales.spec.js`.

- **[x] [THEOREM]** This documentation pass did run the Playwright
  suite successfully after the Scale 0 refactor.
  Evidence: local `npm test` result in `engine/web/tests`.

---

## 4. Residual uncertainty ledger

- **[OPEN]** Live browser timing and perceived smoothness were not
  measured.
- **[OPEN]** Native WebSocket bridge behavior was not exercised against a
  real server during this pass.
- **[OPEN]** GPU/native cleanup and memory behavior were not profiled.
- **[OPEN]** Field overlays and inspector mappings were not
  interactively spot-checked across every mode.
- **[OPEN]** The audit is static-source-heavy by design; runtime traces
  would strengthen the remaining [EMERGENT] and [OPEN] items.

---

## 5. Bottom line

- **[x] [THEOREM]** The architecture document is now mostly a
  source-verified runtime map rather than a speculative overview.
- **[x] [SELECTION]** Some narrative simplifications remain, but they are
  called out here rather than being presented as forced truths.
- **[x] [OPEN]** The remaining uncertainty is concentrated in runtime
  behavior, not in the main static call-stack description.
