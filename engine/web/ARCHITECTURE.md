# FTD Web Engine — Architecture

This document explains how the browser dashboard under `engine/web/`
actually runs today: startup, control flow, frame scheduling, bridge
selection, rendering ownership, and the exact call stacks that matter
when debugging.

The C++ engine itself is documented in `engine/SPEC_ENGINE.md`. This
document is about the web runtime that sits on top of it.

Epistemic companion: see `engine/web/AUDIT_WEB_ARCHITECTURE_EPISTEMIC_STATUS.md`
for a checklist of which claims here are source-verified, which are
documentation compressions, and which still need live-runtime
verification.

If this file disagrees with the code, the code wins. Update the doc in
the same change.

---

## 1. Scope and layout

The web engine is not a single renderer. It is a browser shell that
hosts several distinct simulation modes:

- Scale 0 `lattice`: substrate / flux / manifested particles
- Scale 1 `particles`: ParticleEngine (PE)
- Scale 2 `atoms`: AtomEngine-style atomic UI
- Scale 3 `molecules`: same AE runtime as Scale 2, different loaders
- Scale 4 `planetary`: standalone N-body mock
- Scale 5 `cosmic`: standalone cosmic mock
- Scale 11 `consciousness`: flux-only pedagogical mode
- `meta`: existential-unit mode

Important directory landmarks:

```text
engine/web/
├── index.html
├── ARCHITECTURE.md
├── css/
├── wasm/
│   ├── ftd_core.js
│   └── ftd_core.wasm
├── js/
│   ├── app_dag.js
│   ├── viewport.js
│   ├── inspector.js
│   ├── wasm-bridge-dag.js
│   ├── ws-bridge.js
│   ├── bridge/
│   │   ├── bridge-factory-dag.js
│   │   ├── mock-scale4.js
│   │   └── mock-scale5.js
│   └── scales/
│       ├── scale0/
│       │   ├── controller.js
│       │   ├── scenario-registry.js
│       │   ├── viewport-adapter.js
│       │   ├── runtime/
│       │   ├── state/
│       │   └── ui/
│       ├── scale1/controller.js
│       ├── scale2/controller.js
│       ├── scale3/controller.js
│       ├── scale4/controller.js
│       ├── scale5/controller.js
│       ├── scale6/controller.js
│       └── scale11/controller.js
└── tests/
```

`index.html` loads exactly one module entry:

```html
<script type="module" src="js/app_dag.js?v=..."></script>
```

There is no bundler. Everything else is pulled in by native ES-module
imports. Three.js is provided via import map.

---

## 2. Architectural picture

At the highest level the runtime looks like this:

```text
Browser DOM
  -> index.html
  -> js/app_dag.js
     -> bridge selection
        -> WebSocketBridge (optional native server)
        -> WasmBridge (default real browser engine)
        -> MockBridge (fallback)
     -> shared Viewport / Inspector / panels
     -> per-scale controller
        -> bridge tick/query APIs
        -> scene updates
        -> viewport.render()
```

There are three main layers:

1. Application shell
   `app_dag.js` owns startup, global state, shared context
   construction, mode switching, and the main
   `requestAnimationFrame` loop. After the Scale 0 refactor it is still
   the composition root, but it no longer owns Scale 0 field-toggle,
   force-style, boundary, or scenario-selector wiring.

2. Scale controllers
   Each controller owns scenario loading plus the per-frame logic for a
   mode. Scale 0 now also owns its own UI binding, internal runtime
   phases, state store, and scenario registry. Controllers are leaves:
   they do not import `app_dag.js`.

3. Simulation backends
   The active controller talks to a bridge. That bridge may be a native
   WebSocket server, a browser WASM `RenderBridge` / `ParticleEngine`,
   or one of several JavaScript mocks.

One more boundary now matters in practice:

4. Capability and presentation adapters
   Scale 0 talks to `bridge.capabilities.scale0` rather than poking at
   bridge internals, and talks to `viewport.js` through a dedicated
   `viewport-adapter.js` facade rather than treating `viewport` as one
   undifferentiated API surface.

---

## 3. Entry point and boot sequence

The browser boot path is:

```text
index.html
  -> load js/app_dag.js
  -> init()
     -> _cacheDOM()
     -> tryNativeBridge(latticeSize)
        -> WebSocketBridge.connect()
     -> if native unavailable: createBridge(latticeSize)
        -> new WasmBridge()
        -> WasmBridge.init(latticeSize)
           -> inject wasm/ftd_core.js if needed
           -> createFTDModule({ locateFile })
           -> new module.RenderBridge(latticeSize)
        -> if WASM init fails: new MockBridge(latticeSize)
     -> new Viewport(...)
     -> new DiagnosticsPanel / charts / Inspector / PETelemetryPanel
     -> initOnticPhysicsHierarchy()
     -> wireToolbar() / wireTabs() / wireControls() / wireViewportToggles()
     -> initZoo(bridge)
     -> Scale0Controller.bindUI(_makeCtx())
     -> new BackgroundManager(viewport.scene)
     -> Scale0Controller.loadScenario(_makeCtx(), 'flux-pulse')
     -> requestAnimationFrame(animate)
```

Key facts:

- The app always tries the native WebSocket bridge first in `init()`.
- `createBridge()` does not probe WebSocket. It only tries WASM, then
  falls back to `MockBridge`.
- The default boot scenario is `flux-pulse`.
- A loading overlay is shown during startup and hidden after the default
  scenario is loaded.

### Why `RenderBridge`, not `DagEngine`

`WasmBridge.init()` must instantiate `module.RenderBridge`, not
`module.DagEngine`. The exported helper functions in
`engine/wasm/ftd_wasm.cpp` such as `setupScenario`, `injectFlux`,
`getParticleData`, `getDiagnostics`, and the sampled-field accessors all
accept `ftd::RenderBridge&`. `DagEngine` is bound, but it is not the
type the web helper functions operate on.

---

## 4. Global ownership model

`app_dag.js` is the application root and owns the following long-lived
objects:

- `bridge`
- `viewport`
- `inspector`
- diagnostics panels and charts
- global play state: `running`
- speed state: `ticksPerFrame`
- active mode: `engineMode`
- active UI tab: `activeTab`
- shared frame counter: `frameCount`
- background manager and top-level UI wiring

Controllers do not receive snapshots. `_makeCtx()` builds a context
object with getters and setters, so controllers read and write the live
module-level state.

The shared app context also now exposes a few app-shell services that
Scale 0 uses instead of reaching into the shell indirectly:

- `pauseSimulation()`
- `applyTicksPerFrameFromSlider(...)`
- `applyBoundaryShape(...)`
- `applyReflectiveBoundary(...)`
- `switchToQuantumLabTab()`

Example: a controller reading `ctx.running` is reading the real app
state, not a copied boolean.

---

## 5. Schedulers and the main loop

### 5.1 Main scheduler

For every mode except planetary physics, `app_dag.js` owns a single
unconditional `requestAnimationFrame` loop:

```text
function animate(now) {
  requestAnimationFrame(animate);   // schedule first
  dispatch by engineMode;
  bgManager.update(...);
  floating UI tracking;
  FPS bookkeeping;
}
```

Dispatch table:

- `lattice` -> `Scale0Controller.animateLattice(_makeCtx())`
- `particles` -> `animatePE(now)` -> `Scale1Controller.animatePE(...)`
- `atoms` -> `animateAE(now)` -> `Scale2Controller.animateAE(...)`
- `molecules` -> `animateAE(now)` -> `Scale2Controller.animateAE(...)`
- `cosmic` -> `Scale5Controller.animateCosmic(_makeCtx())`
- `meta` -> `Scale6Controller.updateMeta(_makeCtx(), 1 / 60)`
- `consciousness` -> `Scale11Controller.animateConsciousness(...)`
- `planetary` -> no physics work here; Scale 4 runs its own interval

The next rAF is scheduled first so the loop survives a later exception
better than a tail-scheduled loop.

### 5.2 Planetary special case

Scale 4 is intentionally different. `Scale4Controller.loadScenario()`
starts a `setInterval(..., 16)` loop that owns planetary stepping and
render refresh:

```text
Scale4Controller.loadScenario(...)
  -> _startPlanetaryLoop(ctx)
     -> setInterval(...)
        -> if running: _planetaryBridge.run(f)
        -> _planetaryRenderer.update(...)
        -> ctx.inspector.update()
        -> ctx.viewport.render()
```

The main rAF continues to exist, but does no planetary physics.

### 5.3 Cosmic special case

Scale 5 used to have its own interval. It no longer does. Cosmic now
runs on the main rAF loop, but physics only advances on every other
frame to preserve a roughly 30 Hz simulation cadence while rendering
camera motion smoothly at rAF frequency.

---

## 6. Control-flow call stacks

### 6.1 Play

Play does not tick the engine directly. It only flips state:

```text
Play button / keyboard
  -> togglePlay()
     -> running = !running
     -> updatePlayButton()
```

Actual ticking happens later inside the active controller's frame loop.

### 6.2 Step

Step is direct and mode-specific:

```text
Step button
  -> running = false
  -> updatePlayButton()
  -> switch by engineMode
     -> consciousness: Scale11Controller.step(...)
     -> atoms/molecules: bridge.aeTick()
     -> particles: bridge.peTick()
     -> cosmic: Scale5Controller.step(...)
     -> planetary: Scale4Controller.step()
     -> meta: Scale6Controller.step(...)
     -> lattice: Scale0Controller.step(_makeCtx())
```

### 6.3 Reset

Reset is also mode-specific and reloads the active scenario:

```text
Reset button
  -> running = false
  -> updatePlayButton()
  -> switch by engineMode
     -> cosmic: Scale5Controller.loadCosmicScenario(...)
     -> meta: Scale6Controller.loadMetaScenario(...)
     -> consciousness: loadConsciousnessScenario(...)
     -> molecules: loadMoleculeScenario(...)
     -> atoms: loadAEScenario(...)
     -> particles: loadPEScenario(...)
     -> lattice: Scale0Controller.reset(_makeCtx())
```

### 6.4 Mode switch

All mode changes are routed through one function:

```text
engine-mode <select>
  -> change listener in wireControls()
  -> switchEngineMode(mode)
     -> engineMode = mode
     -> running = false
     -> updatePlayButton()
     -> toggle root CSS mode classes
     -> set #app[data-active-scale]
     -> hide/show tabs by data-scales
     -> if leaving lattice: Scale0Controller.exit(_makeCtx())
     -> inspector.setEngineMode(mode)
     -> viewport.setEngineMode(mode)
     -> setZooMode(mode)
     -> applyTicksPerFrameFromSlider(current slider)
     -> cleanup old scale:
        -> Scale11Controller.resetScale11(...)
        -> Scale5Controller.resetScale5(...)
        -> Scale4Controller.dispose(...)
        -> Scale6Controller.resetScale6(...)
     -> load target scenario:
        -> lattice enter hook is currently a no-op
        -> Scale0Controller.loadScenario(...)
        -> loadPEScenario(...)
        -> loadAEScenario(...)
        -> loadMoleculeScenario(...)
        -> Scale4Controller.loadScenario(...)
        -> Scale5Controller.loadCosmicScenario(...)
        -> Scale6Controller.loadMetaScenario(...)
        -> loadConsciousnessScenario(...)
     -> Scale0Controller.setLatticeNeedsUpload()
     -> frameCount = 0
```

This is the most important lifecycle transition in the app. When
debugging stale state, start here.

---

## 7. Bridge hierarchy

### 7.1 Bridge families

There is not one universal backend:

```text
app_dag.js bridge variable
  -> WebSocketBridge      optional native server bridge
  -> WasmBridge           browser default real engine
     -> RenderBridge      Scale 0 substrate
     -> ParticleEngine    Scale 1
     -> AtomEngine        compiled, but not currently used by web UI
  -> MockBridge           JS fallback for Scale 0 and AE fallback logic

Scale 4 -> PlanetaryMockBridge
Scale 5 -> CosmicMockBridge
Scale 11 -> temporary flux-only MockBridge swap
```

### 7.2 Selection policy

Real startup order is:

1. `tryNativeBridge(latticeSize)` in `app_dag.js`
2. if native unavailable: `createBridge(latticeSize)`
3. inside `createBridge()`:
   `WasmBridge.init()` -> else `new MockBridge()`

So the selection decision is split across two files:

- `app_dag.js` chooses native vs browser-local
- `bridge-factory-dag.js` chooses WASM vs mock

### 7.3 Native bridge behavior

`ws-bridge.js` talks to `ws://localhost:9100`. If the server is absent,
the startup attempt returns `null`, and the bridge then keeps an
exponential reconnect loop alive. The console noise is expected in a
browser-only session.

### 7.4 WASM bridge behavior

`WasmBridge` is a facade over Emscripten module functions. It owns:

- `this._module` from `createFTDModule(...)`
- `this._bridge` as `new module.RenderBridge(...)`
- optional PE object `this._pe`
- optional AE object `this._ae`

Scale 0 and Scale 1 are truly backed by C++ in the browser today.

As of the Scale 0 modularity pass, both `WasmBridge` and `MockBridge`
also expose lazily-built capability surfaces:

- `bridge.capabilities.scale0`
- `bridge.capabilities.scale1`
- `bridge.capabilities.scale2`

Scale 0 uses these capability objects as its backend contract.

### 7.5 AtomEngine reality

`AtomEngine` is compiled and bound in `ftd_wasm.cpp`, but the web UI
does not currently use the WASM AE runtime. In `wasm-bridge-dag.js`,
`_aeHasWasm` is hardcoded to `false` because the WASM engine uses
Planck-scaled units while the web atom/molecule UI uses Bohr-scaled
simulation units.

That means:

- `bridge.initAE()`
- `bridge.aeTick()`
- `bridge.aeGetAtomData()`
- `bridge.aeGetDiagnostics()`

all route to a JavaScript fallback implementation today.

---

## 8. JS <-> WASM <-> C++ boundary

`engine/wasm/ftd_wasm.cpp` registers the browser-facing binding layer:

- `class_<ftd::RenderBridge>("RenderBridge")`
- `class_<ftd::ParticleEngine>("ParticleEngine")`
- `class_<ftd::AtomEngine>("AtomEngine")`
- free functions such as:
  `getParticleData`, `getDiagnostics`, `getFluxVolume`,
  `getFluxSlice`, `setupScenario`, `getPEParticleData`,
  `getPEDiagnostics`, `getAEAtomData`, `getAEDiagnostics`, and many
  field / toggle helpers

This is the boundary between JavaScript orchestration and C++ physics.

The most important substrate path is:

```text
JS controller
  -> WasmBridge.tick()
  -> embind RenderBridge.tick()
  -> C++ RenderBridge::tick()
```

Inside `RenderBridge::tick()` the CPU path is:

```text
RenderBridge::tick()
  -> validate toggle dependencies
  -> if wave_propagation || coupling: phase_read()
  -> phase_write()
  -> if gauss_projection: gauss_project()
  -> if latency_field: solve_latency_poisson()
  -> if forces: phase_forces()
  -> if movement: phase_movement()
  -> if weak_transmutation: weak-transmutation block
  -> update physical_time_
  -> ++tick_
```

If CUDA is enabled and active in a native build, `RenderBridge::tick()`
can instead delegate to the GPU backend before updating `physical_time_`
and `tick_`.

The Scale 1 path is analogous:

```text
Scale1Controller.animatePE(...)
  -> bridge.peTick()
  -> WasmBridge.peTick()
  -> this._pe.tick()
  -> C++ ParticleEngine::tick()
```

---

## 9. Per-scale runtime stacks

This section is the practical debugging map.

### 9.1 Scale 0 `lattice`

Scale 0 is the most complex mode because it mixes:

- real substrate stepping through `RenderBridge`
- optional JS `MockBridge` flux ownership for some scenarios
- heavy field / volume / overlay rendering through `viewport.js`
- its own package-local state store, runtime phase modules, scenario
  registry, and UI bindings

Scale 0 now follows the target module contract more closely than the
other scales. Its public controller surface is:

- `bindUI(ctx)`
- `enter(ctx, options)` (currently a no-op lifecycle placeholder)
- `exit(ctx)`
- `loadScenario(ctx, scenarioId, params?)`
- `animate(ctx)`
- `step(ctx)`
- `reset(ctx)`
- `resize(ctx, newSize)`

Frame stack:

```text
requestAnimationFrame
  -> app_dag.animate(now)
  -> Scale0Controller.animate(ctx)
     -> advanceSimulation(ctx, state)
        -> bridge.capabilities.scale0.tickScale0()
        -> optional state.fluxMock.capabilities.scale0.tickScale0()
     -> syncRenderableData(ctx, state, viewportAdapter)
        -> getScale0ParticleFrame()
        -> getScale0FluxVolume() / getScale0FluxSlice()
        -> viewportAdapter.applyParticleFrame(...)
     -> updateFieldOverlays(ctx, state, viewportAdapter)
        -> sampleFieldState(...)
        -> buildElectromagneticOverlayData(...)
        -> buildForceOverlayData(...)
        -> buildDerivedSubstrateData(...)
        -> applyOverlayFrame(...)
     -> viewportAdapter.render()
     -> updateDiagnosticsAndPanels(ctx, state)
        -> getScale0Diagnostics()
        -> getScale0EnergyAudit()
        -> getScale0Lagrangian()
        -> update charts / inspector / panels / ontic / hierarchy
```

Scenario load stack:

```text
Scale0Controller.loadScenario(ctx, scenarioId)
  -> ctx.resetAllVisualState()
  -> applyAuxiliaryDefaults(...)
  -> getScale0Scenario(scenarioId)
  -> scenario.load({ bridge, capabilities }, params)
  -> new MockBridge(L)
  -> fluxMock.capabilities.scale0.setupScenario(scenarioId)
  -> apply toggle defaults and scenario overrides
  -> setFluxMock(fluxMock, shouldUseFluxMock(...))
  -> mark overrides and resync sliders
  -> state.latticeNeedsUpload = true
```

Important nuance:

- `_useFluxMock` means the JS mock owns the physics for that scenario.
- This is forced for `flux-*`, `s0-seed-*`, and `s0-field-*` scenarios,
  and also when real flux-volume export is unavailable.
- The `scenario-select` dropdown is now populated from
  `scale0/scenario-registry.js` rather than being treated as the sole
  source of truth.

### 9.2 Scale 1 `particles`

Scale 1 is the cleanest real C++ browser path after lattice.

Frame stack:

```text
requestAnimationFrame
  -> app_dag.animate(now)
  -> animatePE(now)
  -> Scale1Controller.animatePE(ctx)
     -> if running:
        -> wholeTicks = tickAccumulator.accumulate(ticksPerFrame)
        -> repeat bridge.peTick()
     -> pData = bridge.peGetParticleData()
     -> particle types / force sources / force overlays
     -> expand to particle cloud representation
     -> viewport.updateParticles(cloud)
     -> optional trails / velocity vectors / field heatmaps
     -> viewport.render()
     -> diag = bridge.peGetDiagnostics()
     -> extra = bridge.peGetExtendedData()
     -> update inspector / telemetry / panels
```

Scenario load stack:

```text
loadPEScenario(name)
  -> Scale1Controller.loadPEScenario(...)
     -> ctx.resetAllVisualState()
     -> bridge.initPE()
     -> reset PE-specific state
     -> seed scenario particles and toggles
```

### 9.3 Scale 2 `atoms`

Scale 2 uses the AE-style API, but today that API resolves to the JS
fallback instead of the WASM `AtomEngine`.

Frame stack:

```text
requestAnimationFrame
  -> app_dag.animate(now)
  -> animateAE(now)
  -> Scale2Controller.animateAE(ctx)
     -> if running:
        -> wholeTicks = tickAccumulator.accumulate(ticksPerFrame)
        -> repeat bridge.aeTick()
           -> WasmBridge.aeTick()
           -> _ensureAEFallback().aeTick()
     -> atomData = bridge.aeGetAtomData()
     -> viewport.updateParticles(...)
     -> update bonds / shells / lobes / labels / force arrows / field overlay
     -> viewport.render()
     -> diag = bridge.aeGetDiagnostics()
     -> inspector / drift / panels
```

Scenario load stack:

```text
loadAEScenario(name)
  -> Scale2Controller.loadAEScenario(...)
     -> ctx.resetAllVisualState()
     -> bridge.initAE()
     -> reset AE toggles
     -> sync AE params from UI
     -> seed atoms / clusters / special scenarios
```

### 9.4 Scale 3 `molecules`

Scale 3 reuses Scale 2's animation loop exactly. The difference is the
loader:

```text
loadMoleculeScenario(name)
  -> Scale3Controller.loadMoleculeScenario(...)
     -> ctx.resetAllVisualState()
     -> bridge.initAE()
     -> reset AE toggles and sync params
     -> load molecule from molecules.js
     -> if available: bridge.aePreBond()
     -> one-tick stability dry-run:
        -> preData = bridge.aeGetAtomData()
        -> bridge.aeTick()
        -> postData = bridge.aeGetAtomData()
     -> reset AE again
     -> reload molecule for actual run
```

So the Scale 3 steady-state frame stack is the Scale 2 stack.

### 9.5 Scale 4 `planetary`

Scale 4 is fully standalone relative to the main bridge.

Load stack:

```text
switchEngineMode('planetary')
  -> Scale4Controller.loadScenario(ctx, name)
     -> hide lattice-specific visuals
     -> _planetaryBridge = new PlanetaryMockBridge()
     -> _planetaryBridge.setupScenario(name)
     -> _planetaryRenderer = new PlanetaryRenderer(...)
     -> inspector.setPlanetaryContext(...)
     -> _startPlanetaryLoop(ctx)
```

Frame stack:

```text
setInterval(..., 16)
  -> if running:
     -> accumulate fractional ticks
     -> _planetaryBridge.run(f)
  -> currentData = _planetaryBridge.getPlanetaryData()
  -> _planetaryRenderer.update(currentData)
  -> ctx.inspector.update()
  -> ctx.viewport.render()
```

Step path:

```text
Scale4Controller.step()
  -> _planetaryBridge.run(1)
  -> _planetaryRenderer.update(...)
```

### 9.6 Scale 5 `cosmic`

Scale 5 has its own bridge and renderer, but uses the app's rAF loop.

Load stack:

```text
Scale5Controller.loadCosmicScenario(ctx, name)
  -> ctx.resetAllVisualState()
  -> _cosmicBridge = new CosmicMockBridge()
  -> _cosmicBridge.setupScenario(name)
  -> _cosmicRenderer = new CosmicRenderer(...)
  -> camera configuration
  -> initial renderer update
  -> auto-play behavior
```

Frame stack:

```text
requestAnimationFrame
  -> app_dag.animate(now)
  -> Scale5Controller.animateCosmic(ctx)
     -> isPhysicsFrame = (ctx.frameCount & 1) === 0
     -> if physics frame and running:
        -> _cosmicBridge.run(round(ticksPerFrame))
     -> if physics frame:
        -> data = _cosmicBridge.getCosmicData()
        -> diag = _cosmicBridge.getDiagnostics()
        -> _cosmicRenderer.update(data, diag)
     -> viewport.render()
```

### 9.7 `meta`

The meta controller is mostly a scene-object animator:

```text
requestAnimationFrame
  -> app_dag.animate(now)
  -> Scale6Controller.updateMeta(ctx, 1 / 60)
     -> metaUnit.update(dt)
     -> viewport.render()
```

Load stack:

```text
Scale6Controller.loadMetaScenario(ctx)
  -> ctx.resetAllVisualState()
  -> hide lattice visuals
  -> metaUnit = new MetaUnit(...)
  -> build pedagogy/info panel
  -> wire geometry toggle controls
```

Important naming note:

- the controller lives in `scales/scale6/`
- but `app_dag.js` maps `meta` to UI scale index `12`

That mismatch is intentional historical numbering, not a typo.

### 9.8 Scale 11 `consciousness`

Scale 11 is the strangest lifecycle because it swaps out the active
bridge.

Load stack:

```text
loadConsciousnessScenario(name)
  -> Scale11Controller.loadConsciousnessScenario(ctx, name)
     -> ctx._resetAllVisualState()
     -> if !_csEngine: _csEngine = new ConsciousnessEngine(viewport.scene)
     -> if !_csPedagogy: create pedagogy helpers and wire subtabs
     -> if !_savedBridge:
        -> _savedBridge = ctx.bridge
        -> ctx.bridge = new MockBridge(32)
     -> configure flux-only toggles on swapped bridge
     -> seed scenario flux and metadata
```

Frame stack:

```text
requestAnimationFrame
  -> app_dag.animate(now)
  -> Scale11Controller.animateConsciousness(ctx, now)
     -> if running:
        -> wholeTicks = tickAccumulator.accumulate(ticksPerFrame)
        -> scenario-specific injections
        -> repeat ctx.bridge.tick()
     -> extract flux / energy diagnostics from swapped bridge
     -> _csEngine.update(...)
     -> update consciousness DOM panels
     -> viewport.render()
```

Exit stack:

```text
switch away from consciousness
  -> Scale11Controller.resetScale11(ctx)
     -> _csEngine.dispose()
     -> if _savedBridge:
        -> ctx.bridge = _savedBridge
        -> _savedBridge = null
     -> reset iteration state
     -> restore lattice particle visibility
```

This is the only place where the active bridge is intentionally mutated
mid-session.

---

## 10. Rendering ownership

### 10.1 Shared viewport

`viewport.js` is the shared rendering host for almost every mode. It
owns:

- `scene`
- `camera`
- `renderer`
- `OrbitControls`
- shared particle cloud
- many overlay meshes and helper layers
- `render()`

It is effectively a scene-and-overlay god object. See
`docs/adr/0001-viewport-decomposition.md`.

### 10.2 Specialized scene owners

Several modes add their own content into the shared scene:

- `PlanetaryRenderer`
- `CosmicRenderer`
- `MetaUnit`
- `ConsciousnessEngine`

These specialized renderers usually receive:

- `viewport.scene`
- `viewport.camera`
- `viewport.renderer`

So the scene is shared, but content ownership is mode-specific.

### 10.3 Inspector

`inspector.js` is the main read-side query path from the view back into
the simulation:

- lattice mode: raycast -> voxel query / bridge inspection
- particles mode: clicked cloud point -> PE particle mapping
- atoms/molecules mode: clicked cloud point -> atom mapping
- planetary/cosmic: controller-specific context objects

When debugging "I clicked something and the panel is wrong", inspect the
controller-specific inspector context first.

---

## 11. Scenario loading rules

Across the app, scenario loading generally means:

1. clear visual leakage from the previous scenario
2. reset or initialize the relevant backend
3. seed entities / flux / bodies
4. sync UI controls back into the bridge
5. mark render buffers dirty

Two important exceptions:

- Scale 0 resize is not the same as scenario load. Resize preserves the
  user's existing toggles and re-creates the bridge.
- Scale 11 intentionally preserves pedagogy helpers across re-entry to
  avoid listener leaks and redundant setup.

---

## 12. Current realities and caveats

These are not theoretical concerns. They are true of the current code.

- `viewport.js` is large and central. Many rendering behaviors still
  converge there.
- `wasm-bridge-dag.js` is also large because it combines real bridge
  code, fallback code, and many scenario helpers.
- The native WebSocket bridge will keep trying to reconnect forever.
  Console reconnect logs are normal if no native server is running.
- The web AE path is still JS fallback even though `AtomEngine` is bound
  in WASM.
- Scale numbering is mixed:
  folder names follow refactor extraction history,
  while UI `data-active-scale` follows the broader ontological numbering.
- Scale 0 has dual data ownership in some scenarios:
  real bridge plus `_fluxMock`. That is intentional.

---

## 13. Testing

Web-level regression coverage lives under `engine/web/tests/` and is
primarily Playwright smoke coverage:

- modes load without console errors or missing assets
- bridge initializes
- scale transitions work
- specific regression guards such as cosmic interval cleanup are covered
- Scale 0 module-contract and scenario-registry wiring are covered

This is not physics-validation coverage. Physics correctness still lives
in:

- `engine/tests/` for C++ CTest coverage
- `scripts/tests/` and related Python verification code

---

## 14. Debugging guide

When tracing runtime behavior, start at these files:

- boot problems:
  `engine/web/index.html`, `engine/web/js/app_dag.js`,
  `engine/web/js/bridge/bridge-factory-dag.js`,
  `engine/web/js/ws-bridge.js`,
  `engine/web/js/wasm-bridge-dag.js`
- substrate frame behavior:
  `engine/web/js/scales/scale0/controller.js`,
  `engine/src/render_bridge.cpp`,
  `engine/wasm/ftd_wasm.cpp`
- particle mode:
  `engine/web/js/scales/scale1/controller.js`,
  `engine/include/ftd/particle_engine.h`,
  `engine/wasm/ftd_wasm.cpp`
- atom/molecule mode:
  `engine/web/js/scales/scale2/controller.js`,
  `engine/web/js/scales/scale3/controller.js`,
  `engine/web/js/wasm-bridge-dag.js`
- rendering bugs:
  `engine/web/js/viewport.js`,
  `engine/web/js/inspector.js`
- mode-switch bugs and leaked state:
  `engine/web/js/app_dag.js` `switchEngineMode()`

If you only remember one call stack, remember this one:

```text
UI event
  -> app_dag.js
  -> scale controller
  -> bridge facade
  -> optional WASM/native boundary
  -> simulation tick/query
  -> viewport update
  -> viewport.render()
```

That is the web engine in one line.

---

## 15. Adding a new scale

To add a scale cleanly:

1. Create `js/scales/scale{N}/controller.js`.
2. Export a loader plus the runtime functions the mode needs, usually
   `animate...`, `load...Scenario`, and `reset...`.
3. Wire the controller into `app_dag.js`:
   imports, `animate()` dispatch, and `switchEngineMode()`.
4. Add the mode to the UI in `index.html`.
5. Reuse shared helpers from `scales/scale-utils.js`.
6. Decide explicitly whether the mode:
   uses the shared `bridge`,
   owns a private bridge,
   uses the shared rAF loop,
   or needs its own scheduler.
7. Update this document with the new mode's call stack.

Keep the controller's ownership boundaries obvious. The easiest way to
create bugs in this codebase is to make it unclear which layer owns
state, scheduling, or scene cleanup.
