# CONTRACTS — Formal interface specifications

**Audience: LLM agents and humans extending or refactoring shared interfaces.**
**Update trigger: any change to a documented interface; new contract; pattern emergence.**
**Last refreshed: 2026-04-27 (post-refactor: 8-phase sweep complete, 17 commits).**

This file is the canonical reference for **cross-module interfaces** in the
FTD codebase. When adding a new public-API consumer, refactoring a module
boundary, or reviewing a cross-cutting change, cite the relevant contract here.

A **contract** is a behavior promise between modules. Violating it produces
silent bugs (cache invalidation broken, polymorphism violated, state drift),
not crashes — which is why each must be explicit.

---

## §1 — Bridge State Contract (live-reference pattern)

### Purpose

The MockBridge in [`engine/web/js/bridge-init.js`](engine/web/js/bridge-init.js)
is a JS-only physics engine. To keep its diagnostics, force computations,
field samplers, and atom engine independently testable and incrementally
extractable, each subsystem is a factory function that takes a **live reference**
to the MockBridge instance and returns a methods object.

```js
import { createDiagnosticsProvider } from './bridge/mock-diagnostics.js';

class MockBridge {
    constructor(N) {
        this._tick = 0;
        this._fluxJ = null;
        this._fluxWV = null;
        // ... other state ...
        this._diag = createDiagnosticsProvider(this);  // ← live reference
    }
    getDiagnostics() { return this._diag.getDiagnostics(); }
}
```

### The contract

A factory function `createXxxProvider(state)`:

1. **MUST hold the `state` reference verbatim** (as a closure). The factory
   MUST NOT destructure `state` into local variables that would shadow
   subsequent mutations.
2. **MUST treat its returned methods as stateless** — every call reads
   the current `state.<field>` value, not a captured-at-construction one.
3. **MAY mutate `state.<field>` cache fields it owns** (e.g.
   `state._cachedFieldEnergy`, `state._energyCacheTick`).
4. **MUST NOT mutate `state.<field>` fields it does not own** (the
   STATE CONTRACT block at top of file enumerates ownership).
5. **MUST honor cache invalidation written from the bridge side.**
   The bridge writes `state._energyCacheTick = -1` on `reset()`,
   `setScale0FluxBuffer()`, etc., and the factory MUST treat that
   sentinel as "next call recomputes from scratch."

### STATE CONTRACT block (mandatory at top of file)

```js
/* ============ STATE CONTRACT ============
 * Reads:    state._tick           // for cache validity check
 *           state._fluxJ          // primary input, may be null
 *           state._fluxWV         // primary input, may be null
 *           state._particles      // particle list
 * Writes:   state._cachedFieldEnergy  // owned cache field
 *           state._cachedWaveEnergy   // owned cache field
 *           state._energyCacheTick    // cache validity sentinel
 *           state._fluxMag            // owned magnitude cache
 *           state._fluxDirty          // cleared after fill
 * Invariants:
 *   - After ensureEnergyCache(), _cachedFieldEnergy == sum(|J[i]|²)/2
 *   - After ensureEnergyCache(), _energyCacheTick == _tick
 *   - state must be the live MockBridge instance (not a copy)
 * ======================================== */
```

### Reference exemplar

[`engine/web/js/bridge/mock-diagnostics.js`](engine/web/js/bridge/mock-diagnostics.js)
lines 26–50 carry the canonical block. New extractions MUST mirror this style.
The Phase 2 bridge split (`bridge/mock-bridge.js`, `bridge/wasm-bridge.js`,
`bridge/capabilities/*.js`) and Phase 3 viewport split (`viewport/scene-core.js`,
`viewport/flux-renderer.js`, `viewport/particle-renderer.js`,
`viewport/field-renderer.js`) are working examples of this pattern at scale.

### Anti-pattern (do not do this)

```js
// WRONG: destructures state at factory time
export function createXxx({ _fluxJ, _tick }) {
    return {
        getX() { return _fluxJ.length; }  // captures stale reference
    };
}
```

```js
// CORRECT: keeps state reference live
export function createXxx(state) {
    return {
        getX() { return state._fluxJ.length; }  // re-reads each call
    };
}
```

---

## §2 — Capability Factory Contract (symmetric polymorphism)

### Purpose

Both `MockBridge` and `WasmBridge` expose the same external surface to
consumers (scale controllers, scenarios, viewport). Rather than inheritance,
each bridge has a `capabilities` object keyed by scale, populated by
factory functions.

```js
bridge.capabilities = {
    scale0: { setupScenario, tickScale0, getScale0Diagnostics, getScale0EnergyAudit, ... },
    scale1: { ... },
    scale2: { ... },
};
```

### The contract

A capability factory `createScaleXCapabilities(bridge)`:

1. **MUST return an object with a fixed set of method names** — the
   "ScaleX surface." Both MockBridge and WasmBridge implementations of
   the same scale MUST expose identical method names with compatible
   signatures.
2. **MAY return `null` or `undefined` from optional methods** — consumers
   guard with `?.` or check before calling.
3. **MUST preserve method binding** — methods called as
   `bridge.capabilities.scale0.setupScenario(...)` must work without
   `.bind(this)`. Use arrow functions or factory closures.
4. **MUST be idempotent on initialization** — calling the factory twice
   on the same bridge instance MUST produce equivalent objects.

### Required Scale 0 surface (current as of 2026-04-27)

```js
{
    // Lifecycle
    setupScenario(name),
    setBoundaryShape(shape),
    setReflectiveBoundary(on),

    // Tick
    tickScale0(),

    // Diagnostics (factual snapshots)
    getScale0Diagnostics() → { tick, manifested, positive, negative, totalEnergy, ... },
    getScale0EnergyAudit() → { fieldEnergy, waveEnergy, particleKE, coulombPE, ... },
    getScale0Lagrangian() → { fieldKinetic, fieldGradient, total, ... },

    // Field samplers
    getScale0FluxVolume(),
    getScale0FluxSlice(axis, index),
    getScale0FieldSamples(),
    getScale0ForceField(),

    // Particles
    getScale0ParticleFrame(),
    getScale0ParticleList(),

    // Toggles
    setToggle(key, value),

    // Snapshot/load (timeline)
    getScale0Snapshot(),
    loadScale0Snapshot(snap),
    setScale0Tick(t),

    // Lattice metadata
    latticeSize,
}
```

Any new method added to one bridge implementation MUST also be added
to the other (or stubbed with a `[Mock-only]` / `[Wasm-only]` comment
and consumer-side guard).

### Reference exemplar

`createScale0Capabilities(bridge)` in
[`engine/web/js/bridge-init.js`](engine/web/js/bridge-init.js).

---

## §3 — Scale Controller `ctx` Contract

### Purpose

Each scale (0–11) has a controller (e.g.
[`engine/web/js/scales/scale0/controller.js`](engine/web/js/scales/scale0/controller.js))
that orchestrates the scale's lifecycle. The controller passes a `ctx` object
to its runtime/UI/state submodules.

### Required `ctx` shape (Scale 0)

```js
ctx = {
    bridge,                         // active simulation backend (MockBridge or WasmBridge)
    viewport,                       // Three.js renderer
    appShell,                       // top-level dashboard shell
    inspector,                      // per-voxel inspector
    diagnostics,                    // diagnostics state
    diagnosticsPanel,               // diagnostics UI
    chartsPanel,                    // charts UI
    lagrangianPanel,                // Lagrangian readout
    fluxEnergyChart,                // chart instance
    particleChart,                  // chart instance
    peTelemetry,                    // PE telemetry hub
    telemetryHub,                   // global telemetry
    running: boolean,
    scenarioRunning: boolean,
    globalTick: number,
    ticksPerFrame: number,
    engineMode: 'wasm' | 'mock' | 'auto',
    activeTab: string,
    frameCount: number,
    dom: { ... },                   // DOM refs

    // Methods
    updateOnticPanel(),
    updateHierarchyPanel(),
    resetAllVisualState(),
    pauseSimulation(),
    applyTicksPerFrameFromSlider(),
    applyBoundaryShape(shape),
    applyReflectiveBoundary(on),
    clearCharts(),
    // ...
}
```

### The contract

1. The runtime (`scales/scale0/runtime/`) MAY read any field but MUST NOT
   replace `ctx.bridge` mid-frame.
2. UI modules (`scales/scale0/ui/`) read `ctx.bridge.capabilities.scale0`
   and call its methods; they MUST NOT poke `ctx.bridge` internals directly.
3. The state store (`scales/scale0/state/`) is the sole owner of `ctx.state`
   (when present); module-private state lives there.
4. `ctx.bridge` MAY change between scenario loads (e.g., MockBridge for
   flux-* scenarios, WasmBridge for others) — code MUST re-read it
   each frame, never cache it.

---

## §4 — Scenario Dispatch Contract

### Purpose

Scenarios are partitioned into 5 prefix-named groups in
[`engine/web/js/bridge/scenarios/`](engine/web/js/bridge/scenarios/):
`flux-`, `light-`, `quantum-`, `s0-seed-`, `s0-field-`. The dispatcher
in `index.js` chains them with prefix matching.

### The contract

A scenario group file exports `setupXxxScenario(name, ctx)`:

1. **MUST return `true`** if it handled the scenario (matched its prefix
   AND completed setup); **MUST return `false`** if the prefix did not
   match (allowing the next group in the chain to try).
2. **MUST throw** on a known prefix with malformed scenario name (do
   not silently fall through).
3. **MUST be called with `.call(this, ...)`** so the scenario body has
   access to the bridge's mutation methods (`this.injectParticle`,
   `this._injectFlux`, `this.injectWavepacket`).
4. **MUST use shared helpers** from `_helpers.js`
   (`injectRadialEnvelope`, `injectParticleFull`, `injectDressedParticle`,
   `injectTriad`) rather than open-coding particle/flux placement.
5. **MUST mirror the C++ scenario** in `engine/src/scenarios/<group>.cpp`
   (same particle counts, positions, charges, signs, locked-flags).
   When the JS and C++ sides drift, the JS side wins for dashboard
   behavior; the C++ side wins for benchmark/campaign runs.

### Adding a new scenario

1. Add a `case 'new-scenario-id':` to the relevant group file.
2. Mirror the C++ in `engine/src/scenarios/<group>.cpp`.
3. Register in [`engine/web/js/scales/scale0/scenario-registry.js`](engine/web/js/scales/scale0/scenario-registry.js).
4. If the scenario needs non-default toggles, add an entry to
   `SCALE0_SCENARIO_OVERRIDES` in
   [`engine/web/js/config/toggles.js`](engine/web/js/config/toggles.js).
5. **Do NOT** mutate `this._toggles` directly inside the scenario body —
   `applyToggleDefaults` runs AFTER `setupScenario` and resets to the
   `SCALE0_TOGGLES` defaults. Use `SCALE0_SCENARIO_OVERRIDES` instead.

---

## §5 — Toggle Contract

### Purpose

[`engine/include/ftd/term_toggles.h`](engine/include/ftd/term_toggles.h)
defines the `TermToggles` struct: 20 boolean fields + a few enums controlling
which physics terms are active in the C++ tick. Mirrored in JS via
[`engine/web/js/config/toggles.js`](engine/web/js/config/toggles.js).

### The contract

1. **Adding a new toggle** is a 2-place edit since Phase 6 (commit 2aa2df9):
   - The boolean field declaration on the `TermToggles` struct (unavoidable — actual storage)
   - One row in `static constexpr ToggleSpec TOGGLE_SPECS[]`
   `validate()`, `enable_all()`, `disable_all()`, `cpu_runtime_warnings()`,
   AND the WASM `rb_toggle_map` all auto-derive from the table row.
   See [ADR-0013](docs/adr/0013-toggle-table-driven.md).
2. **Toggle dependencies are enforced by `validate()`**. Currently checks 13
   relationships (e.g., `weak_transmutation` requires `dual_substrate`). When
   adding a toggle with a dependency, extend `validate()`.
3. **JS↔C++ default values MUST match.** `MockBridge._toggles` defaults
   (in `bridge-init.js` constructor) MUST equal `SCALE0_TOGGLES` defaults
   (in `config/toggles.js`) MUST equal `TermToggles{}` defaults (in
   `term_toggles.h`). When they drift, the validator fires console errors.
4. **Toggles set via `setToggle(key, value)`** propagate through the
   capability factory; direct mutation of `bridge._toggles[key]` works but
   bypasses validation.
5. **Scenario toggle overrides** apply via `SCALE0_SCENARIO_OVERRIDES` entries.
   Apply order: bridge constructor defaults → `SCALE0_TOGGLES` defaults
   reset by `applyToggleDefaults` → per-scenario overrides → user
   click-toggles after load.
6. **Prerequisite-first ordering**: when overrides include both a
   prerequisite and its dependent (e.g., `dual_substrate=true` + `weak_transmutation=true`),
   the loader applies prerequisites first to satisfy the validator. See
   [`scenario-loader.js`](engine/web/js/scales/scale0/runtime/scenario-loader.js)
   for the prerequisite list.

---

## §6 — Energy Convention Contract

### Purpose

Field, wave, particle KE, and Coulomb PE diagnostics MUST agree across
WasmBridge (C++) and MockBridge (JS). The pre-2026-04-27 audit found 2×
drift on multiple energy quantities; convention is now uniform.

### The contract

| Quantity | Convention | Site |
|---|---|---|
| `field_energy` | ½·Σᵢ \|Jᵢ\|² | `compute_energy_audit` (C++), `mock-diagnostics.js` (JS) |
| `wave_energy` | ½·Σᵢ \|wave_velᵢ\|² | same |
| `E_field_energy` | ½·Σᵢ \|Eᵢ\|² where E = -wave_vel | same |
| `B_field_energy` | ½·Σᵢ \|Bᵢ\|² where B = curl(J) | same |
| `E_L_total / E_R_total` | ½·Σᵢ \|flux_{L,R}\|² (dual substrate only) | same |
| `wv_L_total / wv_R_total` | ½·Σᵢ \|wave_vel_{L,R}\|² | same |
| `coulomb_pe` | ½·Σᵢ α·qᵢ·φᵢ ≡ Σᵢ<ⱼ α·qᵢ·qⱼ/rᵢⱼ | same |
| `particle_ke` | ½·Σᵢ \|vᵢ\|² (per manifested particle) | same |

The ½ factor is **mandatory** on every site listed. Pre-2026-04-27 the C++
side dropped the ½ on field/wave/dual energies and on coulomb_pe. Fix
landed in commit aa83cd8.

---

## §7 — Constants Chain Contract

### Purpose

Physics constants flow through 5 representations: Python (canonical), C++
(`ontic.h` derivation chain), JS (`constants.js` mirror), WASM (re-exports),
CUDA (device-side `__constant__` mirrors). All must agree to ≥10 digits.

### The contract

1. [`scripts/constants.py`](scripts/constants.py) is the canonical Python
   source. All Python tooling imports from here.
2. [`engine/include/ftd/ontic.h`](engine/include/ftd/ontic.h) is the
   canonical C++ derivation chain. The 9 layers derive everything from D=3
   and varpi.
3. [`engine/include/ftd/constants.h`](engine/include/ftd/constants.h)
   re-exports `ontic::*` into `ftd::*` namespace; adds `static_assert`
   guards (`G_C * G_C ≈ ALPHA`, etc.).
4. [`engine/include/ftd/constants_gpu.cuh`](engine/include/ftd/constants_gpu.cuh)
   is the device-side mirror. Values MUST match host-side to bit precision.
5. [`engine/web/js/constants.js`](engine/web/js/constants.js) is the
   canonical JS mirror. Layered by category (Layer 0–8). All JS files
   import from here; literal physics values in other JS files are a bug.
6. WASM `get_constants()` is for **observatory display only**; it MUST NOT
   mutate any of the above.
7. Any new physics constant added at any layer MUST be added at all
   downstream layers in the same commit.

### Anti-drift guard

[`engine/include/ftd/constants.h`](engine/include/ftd/constants.h) lines
128–132 carry compile-time `static_assert`s that fire if `G_C² ≠ ALPHA` or
the master quadratic identities break. Extend this pattern when adding
new derived constants.

---

## §8 — Test Telemetry Contract

### Purpose

[`engine/include/ftd/test_telemetry.h`](engine/include/ftd/test_telemetry.h)
provides the test-side assertion API: `section(name)`, `check(name, condition, detail)`,
`check_close(name, computed, expected, tol)`, `metric(name, value, tick)`,
`tick(t, dt, extras)`, `snapshot(t, L, stride, voxels, count)`, `finalize()`.

### The contract

1. Every test main calls `init(name)` first, `finalize()` last. `finalize()`
   returns the failure count (use as exit code).
2. `check*` macros: condition failures increment the failure counter,
   pass-through return value to allow chaining.
3. Output mode is env-gated: with `FTD_TEST_TELEMETRY=1`, tests emit NDJSON
   for telemetry pipelines; without, they emit human-readable text.
4. Tests MUST be runnable standalone (no shared state across tests).
5. New test files MUST be registered in
   [`engine/CMakeLists.txt`](engine/CMakeLists.txt) via the `ftd_add_test`
   macro (template: `engine/tests/test_audit_regression.cpp`). The macro
   auto-links `ftd_test_support` (Phase 7).
6. Shared test helpers landed in Phase 7 (commit 87158ae) at
   [`engine/tests/support/bridge_fixtures.h`](engine/tests/support/bridge_fixtures.h):
   - `make_bridge(L, ToggleProfile, seed=42, force_cpu=true)` — returns
     `std::unique_ptr<RenderBridge>` (RenderBridge has user-declared dtor
     so the implicit move ctor is suppressed)
   - `run_for(rb, n)`
   - `inject_particle_at_center(rb, state, v={})`
   - `assert_energy_conserved(rb, n_ticks, eps_rel=1e-6)`
   - `enum class ToggleProfile { Logic6, LogicOnly, FullEM, FullSM, Custom }`
7. CTest LABELS (Phase 7): every registered test carries one or more of
   `unit` / `physics` / `golden` / `slow` / `gpu`. Run a focused subset
   via `ctest -L <label>` — e.g., `ctest -L golden` runs only
   `test_render_bridge_golden`, the bit-exact regression gate established
   in Phase 4 pre-flight (hash `0xcd957b601d47868a`).

---

## §9 — Refactor Companion Contract

### Purpose

Every refactor session produces three artifacts so future sessions can
reproduce the rationale.

### The contract

1. **SPEC_REFACTOR_<name>.md**: opens the session. Lists scope, files,
   success criteria. Status starts "Open" → "In progress" → "Implemented".
2. **AUDIT_<name>.md** (or section in `AUDIT_LEDGER.md`): live tracker
   with `[x]/[~]/[d]/[n]` legend during work; archived to
   `docs/audits/AUDIT_<YYYY-MM>_<slug>.md` on close.
3. **ADR(s) in `docs/adr/NNNN-*.md`**: emitted for any pattern decisions
   taken during the refactor (≤200 words each, Status / Context / Decision /
   Consequences).
4. **README + ATLAS update**: every directory whose source changed must
   have its README diff in the same PR. META_PROJECT_ATLAS.md updated to
   reflect new structure.

PR-merge gate (target): SPEC.Status = Implemented AND ATLAS diff exists
AND ADR(s) for any new pattern AND audit archived.

---

## §10 — Cascade Callback Contract (Phase 3 viewport pattern)

### Purpose

Sub-renderers extracted from a parent class need to react to lifecycle
events the parent owns (lattice resize, boundary shape change, dispose).
Phase 3 of the refactor sweep established the pattern across 4 viewport
sub-renderers (SceneCore, FluxRenderer, ParticleRenderer, FieldRenderer).

### The contract

Each sub-renderer exposes lifecycle methods the orchestrator calls
**unconditionally** when an event fires. Sub-renderers implement no-op
bodies if the event doesn't apply to them.

```js
class ViewportXxxRenderer {
    onLatticeSizeChanged(size, halfN) { /* rebuild meshes */ }
    setBoundaryShape(shape)          { /* update cached shape */ }
    setEngineMode(mode)              { /* engine-mode-specific behavior */ }
    setAnimationClock(ms)            { /* animation-driven uniforms */ }
    dispose()                        { /* remove from scene + dispose geom/mat */ }
}
```

Orchestrator-side cascade dispatcher (e.g., Viewport.setLatticeSize):

```js
setLatticeSize(size) {
    this.latticeSize = size;
    this._halfN = size * 0.5;
    this._sceneCore?.onLatticeSizeChanged(size, this._halfN);
    this._fluxRenderer?.onLatticeSizeChanged(size, this._halfN);
    this._particleRenderer?.onLatticeSizeChanged(size, this._halfN);
    this._fieldRenderer?.onLatticeSizeChanged(size, this._halfN);
    // ... already-extracted modules ...
    this._molRenderer?.onLatticeSizeChanged?.(size);
    this._topoRenderer?.onLatticeSizeChanged?.(size);
    this.spinArrowManager?.dispose();
    this._applyScenarioScale();
}
```

### Rules

1. **Orchestrator dispatches unconditionally** — the orchestrator MUST
   call every sub-renderer's lifecycle method, even if the event doesn't
   appear to apply. Missing the call = silent stale geometry.
2. **Sub-renderer bodies may be no-op** — if the sub-renderer doesn't
   care about the event, the method body can be empty (with a comment
   explaining why) or simply update internal cached state.
3. **Constructor ordering matters** for callbacks captured at ctor time.
   Phase 3 established: SceneCore is constructed FIRST, FieldRenderer
   SECOND (it owns mesh-factory helpers), FluxRenderer + ParticleRenderer
   THIRD (their ctors capture callbacks bound to `this._fieldRenderer.<method>`).
4. **Disposal ordering**: dispose sub-renderers BEFORE the parent's
   shared resources (renderer, scene). Phase 3a's invariant: SceneCore
   disposes after its child sub-renderers but before
   `this.renderer.dispose()` (the composer is in SceneCore and depends
   on the WebGLRenderer).

### Reference exemplars

- `engine/web/js/viewport.js` `setLatticeSize` and `dispose` (orchestrator)
- `engine/web/js/viewport/scene-core.js`, `flux-renderer.js`,
  `particle-renderer.js`, `field-renderer.js`

See also: ADR-0010 (Cascade callback pattern).

---

## §11 — Mesh-Factory Callback Contract

### Purpose

Cross-cutting mesh factories (e.g., 18-pt streamline mesh, arrow-field
mesh, write-data-into-mesh helpers) often span multiple sub-renderers.
Phase 3c put the canonical helpers on `FieldRenderer`; FluxRenderer and
ParticleRenderer receive bound callbacks at ctor time.

### The contract

```js
this._fluxRenderer = new ViewportFluxRenderer({
    // ... primary args ...
    buildStreamlineMesh:      (m, o) => this._fieldRenderer.buildStreamlineMesh(m, o),
    writeStreamlinesIntoMesh: (m, s, c) => this._fieldRenderer.writeStreamlinesIntoMesh(m, s, c),
});
```

### Rules

1. **Single canonical home.** Each factory lives in exactly ONE
   sub-renderer (the one most semantically aligned). Other sub-renderers
   call it via injected callback.
2. **Public method names** (no underscore prefix) for callback-bound
   methods, since they cross sub-renderer boundaries.
3. **Constructor capture** — callbacks are captured at ctor time.
   Constructor ordering must place the canonical-home sub-renderer
   BEFORE its callback consumers.
4. **No state ownership transfer** through the callback. The callback
   is a function call, not a reference to mutable state.

See also: ADR-0011 (Mesh-factory callback pattern).

---

## §12 — Golden-Tick Regression Gate

### Purpose

Physics-touching extractions (Phase 4 phase_write/forces/read/movement
decomposition) carry high silent-drift risk. The golden-tick gate is a
deterministic byte-hash of a 100-tick scenario; any extraction that
changes the hash has changed physics and must be reverted.

### The contract

The test [`engine/tests/test_render_bridge_golden.cpp`](engine/tests/test_render_bridge_golden.cpp)
(commit 8afc8be):

1. Constructs `RenderBridge(L=16)`, forces CPU backend, seeds RNG with 42
2. Applies a fixed toggle profile (Logic6-like)
3. Injects deterministic initial state (3 manifested particles + 1 flux pulse)
4. Runs exactly 100 ticks
5. Computes a 64-bit FNV-1a hash over voxels (state, flux, wave_vel,
   velocity), every EnergyAudit field, and per-manifested-site state
6. Asserts hash equals frozen baseline `0xcd957b601d47868aULL`

### Rules

1. **Hash is bit-exact.** Tolerance is zero. Any change means physics
   has drifted.
2. **CPU backend forced** so cuRAND non-determinism doesn't leak in.
3. **All Phase 4 + 5 + 6 + 7 commits MUST preserve the hash.** Phase 4a/4b/4c
   each verified bit-exact against this gate; Phase 5/6/7 also held it.
4. **Adding new physics** (intentional behavior change) means: write
   the new feature, run the test, capture the new hash, freeze it as
   the new baseline in a separate commit BEFORE any extraction work.
5. **CTest label `golden`** — `ctest -L golden -C Release` runs only
   this test (0.20 sec).

See also: ADR-0012 (Golden-tick regression gate).

---

## §13 — Scale-engine lifecycle (`ScaleEngine::clear()` + RAII)

### Purpose

The abstract base [`engine/include/ftd/scale_engine.h`](engine/include/ftd/scale_engine.h)
is the polymorphic interface the bridge holds (`ScaleEngine*`) while the web
dashboard switches scales at runtime. Two members govern an engine's
construct → use → reset → reuse → destruct lifecycle:

```cpp
virtual void clear()       = 0;          // reset to a reusable-empty state
virtual ~ScaleEngine()     = default;    // RAII: member destructors free all
```

Concrete subclasses: `ParticleEngine` (Scale 1), `CosmicEngine` (Scale 5),
`DagEngine` (Scale 0, EXPERIMENTAL). Each owns entity vectors and/or a
unique_ptr resource; the lifecycle contract is what lets the bridge re-seed an
engine between scenarios without leaking or carrying stale state.

### The contract

A concrete `ScaleEngine`:

1. **`clear()` returns the engine to reusable-empty.** After `clear()`,
   `entity_count() == 0`, the entity container is empty, and the tick counter
   is back to 0. The engine MUST be immediately reusable — a subsequent
   inject + `tick()` works exactly as on a freshly-constructed instance.
2. **`clear()` also resets per-engine integrator/world state**, not just the
   entity list. `CosmicEngine::clear()` resets the Friedmann cosmology
   (`scale_factor() → 1.0`, `hubble_parameter() → 0`, cosmic time → 0);
   `ParticleEngine`/`CosmicEngine` reset their id counter (`next_id_ → 0`, so
   ids restart); `DagEngine::clear()` rebuilds the DAG (all flux wiped) and
   resets the tick.
3. **`clear()` does NOT reset configuration.** Time step (`dt_`), softening,
   box size, lattice/DAG size, and the toggle struct survive `clear()` — they
   are configuration, not state. (`DagEngine::clear()` preserves the DAG's
   power-of-two `size()`.) Callers that need a config reset construct a new
   engine.
4. **`clear()` is idempotent.** Calling it on an already-empty engine neither
   crashes nor resurrects state; the engine stays empty with tick 0.
5. **The destructor is virtual and defaulted; teardown is member-RAII.**
   Deleting any subclass through a `ScaleEngine*` MUST dispatch the correct
   derived destructor and free every owned resource (`DagEngine`'s
   `unique_ptr<SparseVoxelDAG>`, the Barnes-Hut octrees, the entity vectors).
   Construct + populate + destruct in a tight loop MUST not crash or grow
   unboundedly.

### Known quirk (recorded, not a contract waiver)

`DagEngine::entity_count()` reads `active_indices_`, a member that is declared
but never written anywhere in the codebase, so it is **permanently 0**. The
DAG carries injected flux correctly (`dag().get_voxel(...)`), but the
polymorphic `entity_count()` contract (clause 1) is unmet for `DagEngine`.
The lifecycle test asserts `DagEngine` population/reset against the DAG voxel
state directly and records the `entity_count() == 0` quirk explicitly rather
than skipping it. Closing this means populating `active_indices_` during
`phase_write`/`phase_read` (tracked alongside the engine's other `[OPEN]`
stub phases).

### Enforcement

[`engine/tests/test_engine_lifecycle.cpp`](engine/tests/test_engine_lifecycle.cpp)
enforces clauses 1–5 for all three concrete subclasses (labelled L1–L6 per
engine) plus a delete-through-`ScaleEngine*` polymorphic-teardown section. It
is a deterministic CPU test built against `ftd_core` + `ftd_test_support` and
uses the §8 test-telemetry API. New `ScaleEngine` subclasses that are
practical to instantiate standalone MUST be added to it; subclasses that
require a full `RenderBridge`/scene MAY be recorded as an explained `check`
skip rather than faked.

---

## Cross-references

- [META_PROJECT_ATLAS.md](META_PROJECT_ATLAS.md) — entry-point navigation
- [docs/adr/INDEX.md](docs/adr/INDEX.md) — architectural decisions
- [docs/audits/INDEX.md](docs/audits/INDEX.md) — historical audits
- [engine/web/js/bridge/mock-diagnostics.js](engine/web/js/bridge/mock-diagnostics.js) — STATE CONTRACT exemplar
- [engine/web/js/bridge/scenarios/_helpers.js](engine/web/js/bridge/scenarios/_helpers.js) — scenario primitives
- [engine/include/ftd/term_toggles.h](engine/include/ftd/term_toggles.h) — toggle struct
- [engine/include/ftd/test_telemetry.h](engine/include/ftd/test_telemetry.h) — test API
- [engine/include/ftd/scale_engine.h](engine/include/ftd/scale_engine.h) — ScaleEngine lifecycle interface (§13)
- [engine/tests/test_engine_lifecycle.cpp](engine/tests/test_engine_lifecycle.cpp) — ScaleEngine lifecycle enforcement (§13)
- [scripts/constants.py](scripts/constants.py) — canonical Python constants
- [engine/include/ftd/ontic.h](engine/include/ftd/ontic.h) — canonical C++ derivation chain
- [engine/web/js/constants.js](engine/web/js/constants.js) — canonical JS mirror

---

*Last refreshed: 2026-04-27 | post-refactor sweep complete (17 commits)*
