# Refactor Spec: Split the Three Large FTD Web-Engine Files

**Status:** `[REFERENCE]` — active refactoring specification
**Version:** 1.0 (2026-04-18)
**Scope:** `engine/web/js/viewport.js`, `engine/web/js/wasm-bridge-dag.js`, `engine/web/js/app_dag.js`
**Audience:** Contributors executing or reviewing the extraction work; agents resuming this project across sessions.

---

## 1. Why This Refactor

Three files have grown past the comfortable-maintainability ceiling:

| File | Current LOC | Primary concerns packed in |
|---|---|---|
| `js/viewport.js` | 5,325 | 8 Three.js concerns (scene, camera, particles, boundary, molecular, fields, flux volume, quantum/topology) |
| `js/wasm-bridge-dag.js` | 5,736 | MockBridge (JS fallback physics ~4.9K LOC) + WasmBridge forwarder + 89-case `setupScenario` + 14 lattice samplers + PE + AE engines |
| `js/app_dag.js` | 1,898 | App lifecycle + 8-scale dispatch + ~54 listener wirings + Ontic Observatory glue |

**Symptoms:** Every session that reads these files consumes a large share of agent context. Merge conflicts grow; refactoring in one section risks touching unrelated sections. Architecture, tech-debt, and documentation audits independently converged on "split these files" as the top priority.

**Goals of this refactor:**
1. Drop each file below 3,000 LOC via targeted extractions
2. **Zero** runtime-behavior change (every recent stability fix must keep working)
3. **Zero** hot-path regressions (FPS, GC, frame time unchanged)
4. Establish module boundaries that future work can extend cleanly

**Non-goals:**
- Rewriting any module — this is a pure extraction, not a redesign
- Imposing a common `ScaleController` interface (deferred)
- Changing the capability-getter pattern, the three-level pause model, or any external API

---

## 2. Target Module Structure

Directory additions only; existing files unchanged unless listed:

```
engine/web/js/
├── viewport/
│   ├── color-ramps.js          [NEW] 16 rampXxx fns + FORCE_PALETTES + lerpPalette
│   ├── molecular-renderer.js   [NEW] bonds, nucleus shells, orbital lobes, AE force arrows
│   └── quantum-renderer.js     [NEW] quantumField cloud, topology rubber-sheets, horizon,
│                                      advanceAnimationClock accumulator, bloom composer
├── bridge/
│   ├── mock-diagnostics.js     [NEW] getDiagnostics, getEnergyAudit, getLagrangian, ensureEnergyCache
│   ├── mock-lattice-samplers.js [NEW] 14 getXxxSampled + 3 force-field samplers
│   ├── mock-particle-engine.js [NEW] initPE, peTick, peGetParticleData, _peComputeForces, ...
│   ├── mock-atom-engine.js     [NEW] initAE, aeTick, aeAddAtom, _aeComputeAllForces, ...
│   └── scenarios/
│       ├── index.js                [NEW] setupScenario dispatcher
│       ├── flux-scenarios.js       [NEW] 'flux-*' + QCD/quark/gluon/hadron cases
│       ├── light-scenarios.js      [NEW] 'light-*' cases
│       ├── quantum-scenarios.js    [NEW] 'quantum-*' cases
│       ├── s0-seed-scenarios.js    [NEW] 's0-seed-*' + 'lhc-*' + 'ae-seed-*' cases
│       └── s0-field-scenarios.js   [NEW] 's0-field-*', gravity/EM dipoles
└── ui/
    └── app-ontic.js            [NEW] populateConstants, getOnticDiagnostics,
                                       updateOnticPanel, updateHierarchyPanel,
                                       initOnticPhysicsHierarchy, renderOnticChainSummary

engine/web/tests/
├── color-ramps.spec.js         [NEW] Node-level unit tests for pure ramp functions
└── perf-baseline.spec.js       [NEW] Playwright perf regression: FPS, frame time, allocs
```

**Post-refactor LOC targets:**
| File | Before | After | Change |
|---|---|---|---|
| `viewport.js` | 5,325 | ~3,500 | −1,825 |
| `wasm-bridge-dag.js` | 5,736 | ~2,400 | −3,336 |
| `app_dag.js` | 1,898 | ~1,350 | −548 |
| New module files | 0 | ~5,700 | +5,700 |
| **Net total** | **12,959** | **13,150** | **+191 (JSDoc overhead)** |

---

## 3. Wave 0 — Pre-flight

**Ticket 0a.** This document (committed).

**Ticket 0b.** `engine/web/tests/perf-baseline.spec.js` — Playwright regression gate. Measures at steady state (tick > 200), `flux-pulse` at N=32, preset "Full physics":
- `updateFieldOverlays` call time (mean + p95)
- Rendered FPS over 3 seconds
- JS heap via `performance.measureUserAgentSpecificMemory()` or fallback
- GC pause rate

**Baseline recorded BEFORE any extraction merges.** Regression gates:
- FPS down > 5 % → FAIL
- `updateFieldOverlays` slower > 2 ms absolute → FAIL
- JS heap up > 10 % → FAIL
- GC pause rate up > 20 % → FAIL

Results serialized to `tests/perf-baseline-results.json` for diff between waves.

---

## 4. Wave 1 — Pure Extractions (LOW risk)

Each ticket is independent and can merge separately. Order within wave doesn't matter.

| # | Ticket | Target path | Source lines | LOC delta | Effort | Risk |
|---|---|---|---|---|---|---|
| 1 | Extract color ramps | `viewport/color-ramps.js` | viewport.js 3418–4223 | −810 / +825 | 2h | LOW |
| 2 | Extract lattice samplers | `bridge/mock-lattice-samplers.js` | wasm-bridge-dag.js 1354–1985 | −630 / +680 | 3h | LOW |
| 3 | Extract diagnostics | `bridge/mock-diagnostics.js` | wasm-bridge-dag.js 705–855 | −150 / +175 | 2h | LOW |

### Ticket 1 — Color Ramps

**Source:** viewport.js lines 3418–4223 (16 `_rampXxx` static methods + `FORCE_PALETTES` + `_lerpPalette`).

**Exports:** `rampViridis, rampCyclicHSL, rampDivergingRdBu, rampGrayscale, rampGravWell, rampHeatRed, rampPlasma, rampTurbo, rampMagneticPressure, rampKineticEnergy, rampFisher, rampCoherence, rampHelicity, rampChirality, rampEntropy, rampLagrangian, FORCE_PALETTES, lerpPalette`.

**Integration:** Import at top of viewport.js, rewrite call sites from `this._rampViridis(...)` to `rampViridis(...)`.

**Verify:** `node --check`. Enable Viridis + Helicity overlays on `flux-pulse` — colors render identically.

### Ticket 2 — Lattice Samplers

**Source:** wasm-bridge-dag.js lines 1354–1985 (14 `getXxxSampled` + 3 force-field samplers).

**Exports:** `createLatticeSamplers(state)` factory. State contract in Section 7.

**Integration:** MockBridge constructor creates `this._samplers = createLatticeSamplers(this)`. Each method becomes a one-line delegator. WasmBridge unchanged (it forwards to C++ module).

**Critical detail:** `getBFieldSampled` and `getPoyntingSampled` recompute curl on-the-fly from `_fluxJ`. Preserve this — do NOT optimize by caching B-field.

**Verify:** Enable B-field, Poynting, DivJ, Kretschmann, Latency overlays — all render.

### Ticket 3 — Diagnostics

**Source:** wasm-bridge-dag.js lines 705–855 (`getDiagnostics`, `getEnergyAudit`, `getLagrangian`, `_ensureEnergyCache`).

**Exports:** `createDiagnosticsProvider(state)` factory. Must receive MockBridge instance itself (not destructured copy). State contract in Section 7.

**Integration:** `this._diagnostics = createDiagnosticsProvider(this)`. Methods delegate.

**Critical:** Cache invalidation (`this._energyCacheTick = -1`) stays in MockBridge — four sites: `reset()`, `setScale0Tick()`, `setScale0FluxBuffer()`, `setScale0WaveBuffer()`.

**Verify:** Diagnostics panel populates across all scales. Scrub test: rewind 20 ticks → unpause → energy values update to current tick.

---

## 5. Wave 2 — Shared-State Extractions (LOW–MEDIUM risk)

Must land **after** Wave 1.

| # | Ticket | Target path | Source lines | LOC delta | Effort | Risk |
|---|---|---|---|---|---|---|
| 4 | Extract molecular renderer | `viewport/molecular-renderer.js` | viewport.js 940–1011, 4399–4540, 5023–5113 | −210 / +260 | 3h | LOW |
| 5 | Extract particle engine | `bridge/mock-particle-engine.js` | wasm-bridge-dag.js 2088–2380 | −293 / +330 | 3h | LOW |
| 6 | Extract atom engine | `bridge/mock-atom-engine.js` | wasm-bridge-dag.js 2380–2947, 3050–3388 | −570 / +630 | 4h | MEDIUM |
| 7 | Extract ontic panel | `ui/app-ontic.js` | app_dag.js 1635–1881 | −248 / +280 | 2h | MEDIUM |

### Ticket 4 — Molecular Renderer

**Exports:** Class `MolecularRenderer`. Owns bonds, nucleus shells, orbital lobes, AE force arrows, element labels. Constructor takes `scene`. Has `dispose(scene)` method.

**Integration:** Viewport constructor: `this._molRenderer = new MolecularRenderer(this.scene)`. Delegate methods. Viewport's main `dispose()` calls `this._molRenderer.dispose(this.scene)`.

**Verify:** Scale 2 Carbon + Scale 3 Water render with bonds, shells, orbital lobes, element labels.

### Ticket 5 — Particle Engine (PE)

**Exports:** `createParticleEngine({ ALPHA, G_N, C_SPEED, insideBoundary, reflectIntoBoundary })` factory. Returns object with `initPE`, `peTick`, `peGetParticleData`, `peGetFieldSources`, `peGetForces`, `_peComputeForces`, `peAddParticle`, `peAddLockedParticle`, `resetPE`, `peGetDiagnostics`, `getState()`.

**Integration:** MockBridge delegates each PE method. WasmBridge unchanged.

**Verify:** Scale 1 hydrogen scenario — orbital motion, force arrows, PE telemetry panel all work.

### Ticket 6 — Atom Engine (AE)

**Exports:** `createAtomEngine({ AE_EPS_BASE, AE_K_COULOMB, AE_K_BOND, AE_SPEED_MAX, AE_H_BOND_EPS, AE_K_ANGLE, AE_THERMOSTAT_TAU, cpkColor, insideBoundary, reflectIntoBoundary, getBoundaryShape })` factory.

**Integration:** Receives `getBoundaryShape: () => this._boundaryShape` so AE can read the current shape without holding a stale reference.

**Also moves:** Module-level functions `_valenceElectrons(Z)` and `computeAtomicProps(Z, N)` (wasm-bridge-dag.js lines 25–60) → `bridge/atomic-props-helpers.js` (or inline into mock-atom-engine.js).

**Verify:** Carbon, Oxygen, Water scenarios. Bonds, VSEPR geometry, AE diagnostics panel.

### Ticket 7 — Ontic Panel

**Exports:** `createOnticController(stateRef)` factory receiving getters for `{ bridge, engineMode, observatory, aggregateDetector, emergenceMonitor, scaleBridgeViz }`.

**Integration:** app_dag.js holds `_ontic = createOnticController(_makeOnticStateRef())`. All legacy ontic functions become one-line delegators.

**Verify:** Switch between all 8 scales. Ontic Observatory populates. Hierarchy tower updates as sim runs.

---

## 6. Wave 3 — Scenarios + Quantum Renderer (MEDIUM–HIGH risk)

Tickets 8–13 land **in order** — each grows `scenarios/index.js` dispatcher + smoke-tests one group. Original plan assumed 6 group files matching JS prefixes like `qcd-*` and `sm-seed-*`; the actual shipped split collapsed these into the 5 real prefixes the code uses (`flux-*`, `light-*`, `quantum-*`, `s0-seed-*`, `s0-field-*`) plus `index.js`.

| # | Ticket | Target path | Source lines | LOC shipped | Effort | Risk | Status |
|---|---|---|---|---|---|---|---|
| 8 | Flux scenarios + dispatcher | `bridge/scenarios/{flux-scenarios,index}.js` | flux-* + QCD/quark/gluon/hadron cases | flux=448, index=70 | 2h | MEDIUM | LANDED |
| 9 | Light scenarios | `bridge/scenarios/light-scenarios.js` | light-* cases | 116 | 2h | MEDIUM | LANDED |
| 10 | Quantum scenarios | `bridge/scenarios/quantum-scenarios.js` | quantum-* cases | 229 | 2h | MEDIUM | LANDED |
| 11 | S0-seed + LHC + AE-seed scenarios | `bridge/scenarios/s0-seed-scenarios.js` | s0-seed-* + lhc-* + ae-seed-* cases | 679 | 2h | MEDIUM | LANDED |
| 12 | S0-field scenarios | `bridge/scenarios/s0-field-scenarios.js` | s0-field-* + gravity/EM dipoles | 193 | 1.5h | MEDIUM | LANDED |
| 13 | (collapsed into #11) | — | — | — | — | — | MERGED INTO #11 |
| 14 | Quantum renderer | `viewport/quantum-renderer.js` | viewport.js 2817–3690, 3393–3415, 3489–3640, 4267–4398, 5040–5145 | −1015 / +1100 | 5h | **HIGH** | **DEFERRED** (guard test delivered) |

### Tickets 8–13 — Scenario Modules

**Exports per file:** `setupXScenario(name, helpers)` returning `true` if handled, `false` otherwise. `scenarios/index.js` exports `setupScenario(name, helpers)` dispatcher that tries each module in order.

**Scenario helpers contract (Section 7).**

**Integration:** MockBridge.setupScenario becomes:
```
setupScenario(name) {
  this.reset();
  dispatchScenario(name, this._makeScenarioHelpers());
}
```

**Critical:** `_makeScenarioHelpers()` must include `initFluxGrid` — flux-only scenarios call it as their first line.

**Per-ticket verification:** Load 3 scenarios from each newly-extracted group. No console errors. Visuals unchanged.

### Ticket 14 — Quantum Renderer (HIGH RISK — DEFERRED, GUARD IN PLACE)

**Status (post-Wave-3):** Deferred to a dedicated future ticket. The extraction itself would touch 1000+ LOC of tightly interlocked Three.js state across 7 concerns (quantum field cloud, phase needles, 4 topology rubber-sheets, horizon field, gravitational potential surface, bloom composer, animation-clock accumulator). The plan's Risk 1 (animation-clock freeze) and "Do NOT Touch" item #2 (advanceAnimationClock contract) together make this the highest-risk ticket in the refactor.

**Guard written (this session):** `engine/web/tests/animation-clock-freeze.spec.js` — Playwright test that:
1. Loads the dashboard, runs flux-pulse, enables ψ² overlay
2. Pauses the sim
3. Waits 1 second of wall-clock time (many `requestAnimationFrame` repaints)
4. Toggles the overlay 5 times while paused (forcing repaints)
5. Asserts the quantum-field material opacity never drifts from the value captured just after pause

This test is the regression guard required by Risk 1 ("Re-attempt only after writing a Playwright test that explicitly exercises pause+toggle+capture flow"). Any future attempt at Ticket 14 can now proceed by:
1. Running `npx playwright test animation-clock-freeze.spec.js` on current main (to record the baseline pass)
2. Performing the QuantumRenderer extraction
3. Re-running the same test to confirm the freeze contract survived

**Current viewport.js state:** 4,611 LOC (down from 5,325 via tickets 1 + 4). Above the plan's ~3,500 target but the remaining bulk is stable, interlocked Three.js state that was explicitly protected in the "Do NOT Touch" list. The guard test is the high-value deliverable from this session's Ticket-14 slot.

### Ticket 14 — Quantum Renderer (original plan, for reference)

**Exports:** Class `QuantumRenderer`. Owns `_quantumField`, `_topoSheets`, `_horizonField`, `_composer`, `_bloomPass`, `_animClock`. Constructor takes `scene, renderer, camera`. Exposes:
- `advanceAnimationClock(dt)` — accumulates; called externally
- `animateFrame()` — reads clock without advancing
- `renderFrame()` — compositor render
- Toggle / update methods for every rubber-sheet overlay
- `dispose()`

**Integration pattern (critical to get right):**
```
render() {
  this.controls.update();
  this._quantumRenderer.animateFrame();          // reads clock, does NOT advance
  if (this._quantumRenderer.usePostProcessing) {
    this._quantumRenderer.renderFrame();
  } else {
    this.renderer.render(this.scene, this.camera);
  }
}
advanceAnimationClock(dt) {
  this._quantumRenderer.advanceAnimationClock(dt);
}
```

**Verify:** Pause sim → toggle ψ² overlay → opacity must NOT pulse. Enable bloom post-processing. All 7 rubber-sheet topology overlays render.

**Total effort:** ~29 h across 13 landed tickets + Ticket 14 guard-test deliverable + Wave 0 setup.

---

## 7. Interface Contracts

### Scenario Helpers (used by all 6 scenario modules)
```
{
  latticeSize: number,
  midF: number,               // (latticeSize - 1) / 2, precomputed
  mid: number,                // Math.floor(latticeSize / 2), precomputed
  constants: { K_B, K_GENESIS, ALPHA_EFT, N_BASE, G_STAR, VARPI },
  initFluxGrid(): void,
  injectParticle(x, y, z, state): void,
  injectParticleFull(x, y, z, state, spin, color): void,
  injectFlux(x, y, z, fx, fy, fz): void,
  injectWaveVel(x, y, z, wx, wy, wz): void,
  peAddParticle(...): number,   // ae-seed group only
  aeAddAtom(...): number        // ae-seed group only
}
```

### Lattice Sampler State Reference
```
{
  latticeSize: number,
  _fluxJ, _fluxWV, _fluxMag: Float64Array | null,
  _forceEM, _forceGravity, _forceStrong: Array,
  _latencyProxy: Float64Array | null,
  fluxIdx(x, y, z): number     // exposed as method; do NOT re-implement
}
```

### Diagnostics State Reference
```
{
  _tick, _dt, _physicalTime,
  _particles, _fluxJ, _fluxWV, _fluxMag,
  _energyCacheTick, _cachedFieldEnergy, _cachedWaveEnergy, _cachedFluxMag,
  _fluxDirty   // read + cleared by ensureEnergyCache
}
```
Cache invalidation stays in MockBridge. Diagnostics module only reads, plus writes cache fields.

### MolecularRenderer
Class exposes `.dispose(scene)`. Viewport's main `dispose()` delegates.

### QuantumRenderer
See Section 6 ticket 14 for the exact `render()` call order. `animateFrame()` is a separate entry point from `renderFrame()`.

### No ScaleController interface
Deliberately deferred. Scale controllers keep their current heterogeneous surface.

---

## 8. Verification Strategy

### Per-ticket checks (every extraction)
1. `node --check <new-file>`
2. `node --check <modified-file>`
3. Dashboard loads with no console errors: `python -m http.server 8080 -d engine/web`
4. Playwright smoke: `cd engine/web && npx playwright test tests/scales.spec.js`
5. `perf-baseline.spec.js` passes regression gates

### Per-wave checks

**After Wave 1:**
- 5 flux scenarios load; visuals unchanged
- All 14 samplers return non-empty `{ positions, values, count }` on `flux-pulse` at tick 100
- Diagnostics panel populates across all scales

**After Wave 2:**
- Scale 2 Carbon + Oxygen + Water render with bonds, shells, orbital lobes
- Scale 1 hydrogen orbit preserved
- Ontic Observatory populates across all 8 scales

**After Wave 3:**
- **Every scenario in the registry loads without error** (~60+ scenarios)
- Pause sim → toggle ψ² overlay → opacity stays pinned (animation clock freeze test)
- Enable bloom post-processing → compositor activates cleanly
- All 7 rubber-sheet topology overlays render

### Full-project regression
```
cd engine/web && npx playwright test
```
All 7 existing spec files must pass: `scales.spec.js`, `playback-smoke.spec.js`, `timeline-buffer.spec.js`, `panel-mount.spec.js`, `panel-mount-integration.spec.js`, `verify-panel.spec.js`, `panels-redesign.spec.js`.

### Performance regression (`perf-baseline.spec.js`)
Gates on `flux-pulse` at N=32, preset "Full physics", steady-state tick > 200:
- FPS down > 5 % → FAIL
- `updateFieldOverlays` mean time up > 2 ms → FAIL
- JS heap up > 10 % → FAIL
- GC pause rate up > 20 % → FAIL

---

## 9. Risk & Rollback

### Risk 1 — Animation-clock freeze broken (ticket 14)

**Description:** `advanceAnimationClock(dt)` accumulates wall-clock; `_animateQuantumField()` reads without advancing. If extraction changes call order (`renderFrame` calls `_animateQuantumField` before the clock advances, or the accumulator shifts inside the renderer), opacity advances on overlay toggles while paused.

**Signal:** Pause sim → toggle ψ² overlay → opacity visibly pulses. Playwright-recordable.

**Rollback:** Revert ticket 14. Re-attempt only after writing an explicit Playwright test for pause+toggle+opacity-capture flow.

### Risk 2 — Scenario helper missing `initFluxGrid` (tickets 8–13)

**Description:** Flux-only scenarios call `this._initFluxGrid()` first. If omitted from helper bag, scenarios load silently with empty flux (no console error; the guard in `_injectFlux` swallows the null).

**Signal:** `flux-pulse` loads but particles / flux volume render empty.

**Rollback:** Add `initFluxGrid` to helper bag, re-run. Smoke-test after ticket 8 before proceeding.

### Risk 3 — Energy-cache invalidation broken (ticket 3)

**Description:** If `createDiagnosticsProvider` receives destructured field copies (value) instead of MockBridge instance (reference), cache invalidation in MockBridge will not propagate — stale energies served after scrub.

**Signal:** Scrub 20 ticks backward → unpause → diagnostics show pre-scrub energy values.

**Rollback:** Revert ticket 3. Re-attempt passing MockBridge instance (`this`) directly.

---

## 10. What to NOT Touch

These code paths were stabilized in the last two weeks. Leave them verbatim even when surrounding code is moved:

1. **`setFieldToggle` dirty-on-both-transitions** (`scales/scale0/state/store.js`) — Do NOT "simplify" to dirty-on-enable only.
2. **`advanceAnimationClock` accumulator** (`viewport.js`) — clock advances ONLY when external controller calls it. `_animateQuantumField()` reads without advancing. Do not combine them.
3. **`_forceGlyphMeshes` per-type bag** (`viewport.js`) — one InstancedMesh per force type is deliberate.
4. **`forceStride = 1` for N ≤ 32** (`scales/scale0/runtime/field-overlays.js`) — prevents Moore-neighbor aliasing.
5. **Cache-invalidation chain in MockBridge** — `setScale0FluxBuffer`, `setScale0WaveBuffer`, `setScale0Tick`, `reset` all invalidate `_latencyProxyTick` and `_energyCacheTick`. Extracted modules must NEVER write to cache tick fields.
6. **`_buildLatencyProxy` size-check** — rebuilds when tick is −1 OR size doesn't match. Do not simplify.
7. **Coordinate-convention unification** — every overlay renders at voxel-center (`x + 0.5`). Wireframe subdivision at `raw + 0.5`. Click-to-inspect uses `Math.floor`.

---

## 11. Critical Files

**Existing files being modified:**
- `engine/web/js/viewport.js`
- `engine/web/js/wasm-bridge-dag.js`
- `engine/web/js/app_dag.js`

**Templates to emulate (already established patterns):**
- `engine/web/js/bridge/boundary.js` — factored bridge submodule
- `engine/web/js/bridge/mock-scale4.js`, `mock-scale5.js` — mock-engine factoring pattern
- `engine/web/js/bridge/bridge-factory-dag.js` — factory function style
- `engine/web/js/scales/scale0/runtime/tick.js` — small module with clear state references
- `engine/web/js/scales/scale0/ui/overlays/presets.js` — named-export module with file-level JSDoc

**Utilities to reuse:**
- `bridge/boundary.js::insideBoundary, reflectIntoBoundary`
- `constants.js` (all physics constants — SSoT)
- `scales/scale0/runtime/tick.js::createTickAccumulator`

---

## 12. End-to-End Verification Procedure

After all 14 tickets land:

1. `cd engine/web`
2. `node --check js/viewport.js js/wasm-bridge-dag.js js/app_dag.js`
3. `node --check js/viewport/*.js js/bridge/mock-*.js js/bridge/scenarios/*.js js/ui/app-ontic.js`
4. `python -m http.server 8080 -d .` (separate terminal)
5. `npx playwright test` — full suite, zero failures
6. Inspect `tests/perf-baseline-results.json` — before/after within regression gates
7. Manual smoke: cycle all 8 scales via dropdown; open each panel tab; load 3 scenarios per scale; enable all overlays per scenario; no console errors
8. Manual pause test: pause global playback; toggle ψ² and helicity overlays repeatedly; visual state unchanged between toggles
9. Line-count confirmation: `wc -l js/viewport.js js/wasm-bridge-dag.js js/app_dag.js` — all three under 3,600

If every step passes: merge to main; update this spec with final LOC numbers; close out.

---

## 13. Post-completion addendum: WASM scenario port

After the JS-side refactor above landed, the same session also closed the long-standing JS↔WASM scenario gap: all 83 browser scenarios were ported from the MockBridge `setupScenario` dispatcher into the C++ engine at `engine/src/scenarios.cpp`, with the public entry point declared in `engine/include/ftd/scenarios.h`.

This means the WasmBridge path now handles the full scenario registry natively — no more MockBridge-only scenarios. The five-file scenario split on the JS side (flux/light/quantum/s0-seed/s0-field) mirrors the C++ groupings in `scenarios.cpp`, keeping the two sides structurally parallel.

**Coverage:** `engine/web/tests/scenarios-wasm.spec.js` — 44/44 Playwright cases exercising every scenario group through the WASM path, all green.

Readers picking up future work on scenarios should treat `scenarios.h` / `scenarios.cpp` as the canonical C++ side and the `bridge/scenarios/*.js` modules as the canonical JS side; the two are expected to stay in lock-step.
