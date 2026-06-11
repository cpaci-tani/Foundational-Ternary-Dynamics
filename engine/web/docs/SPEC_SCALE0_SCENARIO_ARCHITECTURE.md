# SPEC — Scale 0 Scenario Subsystem Architecture

**Status:** foundation reference (descriptive — documents the system as built).
**Baseline:** the working tree as of 2026-06-05, reflecting this session's committed scenario
work — forward-only time model + play-bar (`9cc1f38b`), `flux-zero-point` + the per-scenario
boundary mechanism (`34c19160`), and the all-scenario health sweep + worker fix (`7fc4296b`).
**Companion docs:** the gap/drift findings live in
[`audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md`](audits/AUDIT_SCALE0_SCENARIO_LIFECYCLE_2026-06-05.md);
the per-scenario mount/telemetry + physics-sense audit in
[`audits/AUDIT_SCALE0_SCENARIO_HEALTH_2026-06-05.md`](audits/AUDIT_SCALE0_SCENARIO_HEALTH_2026-06-05.md);
the remediation roadmap in [`PLAN_SCALE0_SCENARIO_MODULARIZATION.md`](PLAN_SCALE0_SCENARIO_MODULARIZATION.md).
Foundation companions: [`SPEC_SCALE0_BRIDGE_ARCHITECTURE.md`](SPEC_SCALE0_BRIDGE_ARCHITECTURE.md) (the
bridges scenarios run on) and [`SPEC_SCALE0_RUNTIME_PIPELINE.md`](SPEC_SCALE0_RUNTIME_PIPELINE.md) (the
per-frame loop that drives them).
Cross-scale companion: [`engine/SCENARIO_ARCHITECTURE.md`](../../SCENARIO_ARCHITECTURE.md)
(scenario lifecycle and seed architecture across all dashboard scales).
Adjacent surfaces already specced: [`audits/AUDIT_BRIDGE_WIRING_2026-06-03.md`](audits/AUDIT_BRIDGE_WIRING_2026-06-03.md)
(bridge read-surface), [`AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md`](AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md)
(lifecycle controller / worker teardown / perf), [`TOGGLE_REGISTRY.md`](TOGGLE_REGISTRY.md),
[`SPEC_VACUUM_PARTICLE_SCENARIOS.md`](SPEC_VACUUM_PARTICLE_SCENARIOS.md).

**Path convention:** JS paths are relative to `engine/web/js/` (e.g.
`scales/scale0/runtime/scenario-loader.js`); docs relative to `engine/web/docs/`;
C++ paths from the repo root (`engine/src/...`, `engine/include/...`). Every claim below
carries a `file:line` so it can be re-verified against source — and should be, per the
project's verify-before-claiming discipline (the 2026-05-27 web audit over-counted dead
code by ~10 items by inferring rather than reading).

---

## 0. What "a scenario" is, in one paragraph

A **scenario** is a named initial condition for the Scale-0 lattice (e.g. `flux-pulse`,
`s0-seed-hydrogen`, `quantum-tunnel`). Selecting one resets the active bridge, configures
the physics toggles, and runs a small imperative "seed" routine that injects flux and/or
particles into the lattice. From that seed the engine evolves under the standard tick
cycle. A scenario is **not** a saved state or a script — it is a one-shot seeding function
plus the toggle profile it needs, identified by a string id and surfaced in a dropdown.

The subsystem that makes this work is spread across **four parallel definition layers**
(§3) wired together by a **runtime lifecycle** (§5) over a **two-bridge execution model**
(§2), held together by a set of **contracts** (§6). This document is the map.

---

## 1. The 30-second model

```
            ┌──────────────── DEFINITION (static) ────────────────┐
  UI registry        JS seed impl         C++ seed impl       Metadata
  scenario-registry  bridge/scenarios/*   engine/src/         config/
  .js (96 entries)   .js (96 scenarios)   scenarios/ (95)     scenarios.js
       │                   │                    │                 │
       └─── id ────────────┴──── id ────────────┴──── id ─────────┘   (string id = the only key)
                                   │
            ┌──────────────── LIFECYCLE (runtime) ─────────────────┐
  select → loadScale0Scenario → pick bridge → reset toggles →
  getPhysicsHarness → scenario.load(harness) → harness.setupScenario →
  bridge.setupScenario → [JS runSetupScenario | C++ dispatch_scenario] →
  inject flux/particles → tick loop → (switch/resize/exit) teardown
            └──────────────────────────────────────────────────────┘
```

The string **id** is the *only* thing that links the four definition layers. There is no
shared schema, no generated table, and only one of the three impl/registry edges
(JSC++) has an automated drift guard (§6.3). This is the central architectural fact
the modularization roadmap addresses.

---

## 2. Two-bridge execution model & ownership

Scale-0 physics can run on one of three bridge implementations. Which one owns a given
scenario is decided per-load by `shouldUseFluxMock()` and `workerEligible()`.

| Bridge | File | Role |
|---|---|---|
| **WasmBridge** | `bridge/wasm-bridge.js` | Emscripten-compiled C++ engine — the canonical fast path. `setupScenario` (`:373-376`) → `this._module.setupScenario(this._bridge, name)` → C++ `dispatch_scenario`. |
| **MockBridge** | `bridge-init.js` / `bridge/mock-bridge.js` | Pure-JS lattice. `setupScenario(name, harness)` (`mock-bridge.js:1651`) → `runSetupScenario` (the JS dispatcher). In-thread. |
| **MockBridgeProxy** | `bridge/mock-bridge-proxy.js` | Worker wrapper around a MockBridge running in a Web Worker; a main-thread "shadow" reads the worker's `SharedArrayBuffer`s zero-copy. `setupScenario` (`:121`). **The default deployed path for `flux-*` scenarios** since 2026-06-03 (see `AUDIT_BRIDGE_WIRING_2026-06-03.md`). |

**Ownership decision** — `scales/scale0/runtime/scenario-loader.js`:

- `shouldUseFluxMock(bridge, scenarioName)` (`:70-78`):
  1. native GPU or `WebSocketBridge` → `false` (run on the canonical engine);
  2. `scenarioName.startsWith('flux-')` → `true` (JS mock owns it);
  3. otherwise probe `bridge.getFluxVolume()`; empty/throw → `true` (mock), else `false`.
- `workerEligible(scenarioId, bridge)` (`:89-94`): `FTD_PHYSICS_WORKER && SharedArrayBuffer
  && crossOriginIsolated && shouldUseFluxMock(...)` → use `MockBridgeProxy`, else in-thread
  `MockBridge`. `makeFluxMock()` (`:95-99`) builds the chosen instance.

> **Resolved ownership (verified 2026-06-05 via the all-scenario health sweep).** Only
> `flux-*` is unconditionally mock-owned (rule 2). **Every other scenario** — `empty`,
> `light-*`, `quantum-*`, `s0-seed-*`, `s0-field-*`, `s0-vacuum-*` — runs on the **WASM
> engine** whenever the WASM bridge exposes a flux volume (rule 3 returns `false`), and
> falls back to the mock only when it does not (Safari/iOS, no COOP/COEP). The sweep
> confirmed this empirically (`flux-*` → `owner=mock`, all others → `owner=wasm`). See
> [`audits/AUDIT_SCALE0_SCENARIO_HEALTH_2026-06-05.md`](audits/AUDIT_SCALE0_SCENARIO_HEALTH_2026-06-05.md) §A.
> *Consequence:* a **JS-only** scenario bug is latent (mock-fallback path only) — e.g. the
> `vacuum-scenarios.js` `harness` ReferenceError that affected every `s0-vacuum-*` on the mock
> path but never the deployed WASM path (health audit §A.4; **fixed** 2026-06-05, guarded by a
> `?engine=mock` test in `tests/scale0-scenario-health.spec.js`).

When a fluxMock owns the scenario, **two bridges coexist**: `ctx.bridge` (the primary
engine, idle for Scale 0 this scenario) and `state.fluxMock` (the JS mock that actually
ticks). `stepScale0()` (`scenario-loader.js:435-442`) ticks `state.fluxMock` when
`state.useFluxMock`, else the primary `ctx.bridge`.

---

## 3. The four definition layers

A scenario is defined — partially — in up to four places. Adding a fully-wired,
documented scenario touches all four.

### 3.1 UI registry — `scales/scale0/scenario-registry.js`

The canonical catalogue the dropdown is built from. `SCALE0_SCENARIOS` (`:17-398`) is an
array of **96 descriptors** (as extracted 2026-06-05: **86** built by the `makeScenario`
factory `:1-15` + **10** hand-written object literals — `s0-seed-quark-gluon-plasma`
`:74-97` and nine `s0-seed-emergent-ic*` `:133-375`). Helpers:

- `SCALE0_SCENARIO_MAP` (`:400`) — `id → descriptor`.
- `getScale0Scenario(id)` (`:402-404`) — map lookup, **defaults to `flux-pulse`** on miss
  (so an unknown id silently loads the default rather than erroring).
- `populateScale0ScenarioSelect(select, selectedId)` (`:406-427`) — builds `<optgroup>`s
  keyed by `descriptor.category`.
- `validateScale0ScenarioRegistry()` (`:429-440`) — checks duplicate ids, `scale==='lattice'`,
  non-empty category, array `requiredCapabilities`. Returns `{ok, errors, count}`. **This is
  the only registry self-check, and it is not invoked anywhere in the runtime** (see audit).

### 3.2 JS seed implementation — `bridge/scenarios/*.js`

Six prefix group files, each exporting `setupXxxScenario(name, harness, ctx)` that returns
`true` iff it handled `name`. Dispatched by `runSetupScenario` in `bridge/scenarios/index.js`
(`:51-91`). Case counts (extracted 2026-06-05):

| Group file | prefix | `case` count |
|---|---|---|
| `flux-scenarios.js` | `flux-` | 21 |
| `light-scenarios.js` | `light-` | 4 |
| `quantum-scenarios.js` | `quantum-` | 9 |
| `vacuum-scenarios.js` | `s0-vacuum-` | 15 |
| `s0-seed-scenarios.js` | `s0-seed-` | 44 |
| `s0-field-scenarios.js` | `s0-field-` | 8 |
| **total** | | **101** case-label occurrences = **95** unique scenarios (+ `empty`); the surplus is fall-through/duplicate `case` labels (verified via the parity inventory, §6.3) |

Shared seed primitives live in `bridge/scenarios/_helpers.js`: `injectRadialEnvelope`,
`injectParticleFull`, `injectDressedParticle`, `injectTriad`, `applyVacuumEnvironment`,
and the `TRIAD_ANGLES` constant — the JS mirror of the C++ `_helpers.h` macros (§7).

### 3.3 C++ seed implementation — `engine/src/scenarios/*.cpp`

The canonical engine path. `dispatch_scenario(rb, name)` (`engine/src/scenarios.cpp:63-78`)
resets the shared RNG then walks six `setup_*_scenario(rb, name)` group functions in prefix
order (contract in `engine/include/ftd/scenarios.h:58-68`). `name ==` branch counts:

| Group file | `name ==` count |
|---|---|
| `flux.cpp` | 21 |
| `light.cpp` | 4 |
| `quantum.cpp` | 9 |
| `vacuum.cpp` | **17** |
| `s0_seed.cpp` | **40** |
| `s0_field.cpp` | 8 |
| **total** | **99** occurrences = **95** unique |

> **Raw counts ≠ unique scenarios.** These are `name ==` *occurrences*; some scenarios are
> tested in more than one branch (e.g. `name == "a" || name == "b"`), so the deduplicated
> set is **95 unique C++ scenarios** — exactly matching the 95 unique JS scenarios. The
> JSC++ parity guard (§6.3) is **GREEN** (6/6, verified 2026-06-05 after adding
> `flux-zero-point` and removing `frw-patch`: inventory `UI 96 / JS 96 / C++ 95 / shared 95`,
> +1 C++ legacy). Do **not** read the per-file occurrence counts as drift; the unique scenario
> sets are in parity.

### 3.4 Metadata — `config/scenarios.js`

Human-readable descriptions + epistemic tags, surfaced in the info panel via
`formatS0SeedMetadata(id)` (`:334-343`) and rendered by `bindings.js:20-22`. Two tables:

- `S0_SEED_SCENARIO_METADATA` (`:57-327`) — covers **only** `s0-seed-*` (and only ~22 live
  entries; many are removed/commented for provenance). Each entry: `{title, desc,
  epistemic: [[field, tag, note], …]}`. The header (`:24-56`) is a load-bearing epistemic
  contract: tags must match the canonical `LEDGER.md`; do not invent or upgrade tags.
- `QUANTUM_SCENARIO_DESCRIPTIONS` (`:9-19`) — 9 `quantum-*` blurbs.

**No metadata exists for `flux-*`, `light-*`, `s0-field-*`, or `s0-vacuum-*`** (the last
points readers to `SPEC_VACUUM_PARTICLE_SCENARIOS.md` instead).

### 3.5 Adjacent — the toggle profile (`config/toggles.js`)

Not a fifth "definition" of the scenario, but a fifth file most non-trivial scenarios must
touch: `SCALE0_SCENARIO_OVERRIDES` (`:82-198`) maps `id → [[toggleKey, value, domId], …]`
applied declaratively at load (§6.1). `LIGHT_SCENARIO_OVERRIDES` (`:201-210`) applies to
every `light-*`.

---

## 4. Scenario descriptor schema

The shape every registry entry has (factory form, `scenario-registry.js:1-15`):

```js
{
  id:                   string,        // the cross-layer key, e.g. 's0-seed-hydrogen'
  scale:                'lattice',     // always; validated
  title:                string,        // dropdown label
  category:             string,        // <optgroup> grouping
  tags:                 string[],      // free-form, advisory
  defaultParams:        {},            // always empty today (params plumbed but unused)
  requiredCapabilities: ['scale0'],    // validated; never enforced at load (see audit)
  epistemicStatus:      string,        // e.g. '[CONJECTURE]'; advisory, not synced to metadata
  load(harness, params) { … },         // the seed entry point
}
```

`load` has **two realized forms**:

1. **Factory (delegating)** — 86 of 96 entries. `load(harness, params){ harness.setupScenario(params.id || id); }`
   (`:11-13`). All seeding logic lives in the JS/C++ impl layer; the registry entry is a
   pure pointer.
2. **Custom (imperative)** — 10 entries (`s0-seed-quark-gluon-plasma` `:74-97`, nine
   `emergent-ic*` `:133-375`). These run `bridge.setToggle(...)` / `bridge.setLangevinParams(...)`
   *before* delegating to `bridge.setupScenario(...)`, and defensively unwrap the bridge with
   `const bridge = harness.bridge || harness;`.

The two forms are the root of two findings: the custom form (a) duplicates the job of
`SCALE0_SCENARIO_OVERRIDES` imperatively, (b) sets the **non-whitelisted** `langevin` toggle
and never restores it (§6.1 leak), and (c) is invisible to the parity guard's registry
extractor (§6.3). See the audit.

---

## 5. The runtime lifecycle

### 5.1 Canonical call path (load)

```
USER picks a scenario in #scenario-select
  │  scales/scale0/ui/bindings.js:48-54  ('change' handler)
  ├─ ctx.pauseSimulation()
  ├─ id = getScale0Scenario(select.value).id          // registry lookup, default-safe
  ├─ api.loadScenario(ctx, id)                          // → controller.loadScenario
  └─ updateScenarioMetadata(id)                         // info panel text

controller.loadScenario(ctx, id, params)   scales/scale0/controller.js:215-217
  └─ loadScale0Scenario(ctx, state, viewportAdapter(ctx), id, params)
        scales/scale0/runtime/scenario-loader.js:256-301
     1. scenario = getScale0Scenario(id)                         :257
     2. overlayPrefs = captureOverlayPreferences(state)          :263   (save user overlays)
     3. useFluxMock = shouldUseFluxMock(ctx.bridge, id)          :265   (§2 ownership)
     4. if useFluxMock: fluxMock = makeFluxMock(...);            :267-272
          setBoundaryShape(boundaryShapeFor(id))                         (§6.6 per-scenario boundary:
          setReflectiveBoundary(reflectiveFor(id))                        SCALE0_SCENARIO_BOUNDARY ?? DOM)
          fluxMock…setupScenario(id)
     5. applyToggleDefaults(ctx.bridge…scale0, fluxMock…?, id)   :275   (§6.1 reset+overrides)
     6. if fluxMock: copy each DEFAULT_TOGGLES state from DOM    :277-280
     7. setFluxMock(fluxMock, useFluxMock)                       :282   (disposes prior mock)
     8. ctx.resetAllVisualState() → resetScale0VisualState       :286
        applyAuxiliaryDefaults(ctx, viewportAdapter, id)         :287   (tpf=50; boundary via §6.6 —
                                                                          honors SCALE0_SCENARIO_BOUNDARY,
                                                                          else resets cube + non-reflective)
     9. harness = getPhysicsHarness(ctx.bridge)                  :289   (§5.3)
        scenario.load(harness, params)                           :290   → harness.setupScenario(id)
                                                                          → bridge.setupScenario(id, harness)
                                                                          → JS runSetupScenario | C++ dispatch_scenario
                                                                          → inject flux / particles
    10. setCurrentScenarioId(id)                                 :292
        markScenarioOverrideRows(DEFAULT_TOGGLES)                :294
        syncComboSliders(ctx.bridge)                             :295
    11. restoreOverlayPreferences(overlayPrefs, …)               :300   (re-apply saved overlays)
```

### 5.2 Stages, named

| Stage | Owner | Entry point |
|---|---|---|
| **Select** | UI | `bindings.js:48` change handler |
| **Load** | controller → loader | `controller.js:215` → `scenario-loader.js:256` |
| **Bridge-ownership** | loader | `shouldUseFluxMock` / `workerEligible` `:70-94` |
| **Toggle reset** | loader | `applyToggleDefaults` `:136-190` |
| **Boundary** | loader | `boundaryShapeFor` / `reflectiveFor` `:33-44` (§6.6) |
| **Init / seed** | harness → bridge → impl | `scenario.load` `:290` → `runSetupScenario` / `dispatch_scenario` |
| **Tick** | controller | `animate` `controller.js:219-229` → `advanceSimulation` → `stepScale0` |
| **Teardown / switch** | implicit | next load resets; `setFluxMock` disposes prior mock `store.js:137-148` |
| **Exit (scale switch)** | lifecycle controller | `Scale0LifecycleController.destroy` `controller.js:178-194` → `exitScale0` |
| **Resize** | loader | `resizeScale0Lattice` `:346-433` |

### 5.3 The PhysicsHarness (init surface)

`getPhysicsHarness(bridge)` (`physics/index.js:25-31`) lazily attaches **one** `PhysicsHarness`
per bridge, stored at `bridge.__ftdPhysicsHarness__`. The harness (`physics/physics-harness.js:82-436`)
**wraps, does not replace** the bridge (`harness.bridge` is the raw instance). It exposes:

- the **injection surface** seed routines use — `injectParticle` (`:338`), `injectFlux`
  (`:358`), `injectWaveVel` (`:370`), `createEntangledPair` (`:379`), `injectWavepacket`
  (`:384`) — papering over MockBridge-vs-WASM duck-typing;
- `setupScenario(name)` (`:402-407`) → `bridge.setupScenario(name, this)`;
- read getters (`getParticleList`, samplers, `getEnergyAudit`, `getDiagnostics`) and toggle
  `get/setToggle` (`:306-328`).

Per its own docstring (`:28-36`), there is **no intermediate JS scenario registry** at the
harness layer — `setupScenario` defers straight to whichever bridge owns the run.

> **Inconsistency to note:** `runSetupScenario` (`index.js:51-91`) accepts *either* a real
> harness *or* falls back to building an ad-hoc `scenarioHarness` literal (`:70-84`) when
> called with `this`-binding and no harness. So the injection surface is defined in **two**
> places (the `PhysicsHarness` class and this inline literal). See the audit.

### 5.4 Tick, teardown, switch, resize

- **Tick:** `controller.animate` (`:219-229`) runs `advanceSimulation → syncRenderableData →
  updateFieldOverlays → renderFrame → updateDiagnosticsAndPanels`. `stepScale0`
  ticks fluxMock or primary per `state.useFluxMock`. Physics advances only while `ctx.running`;
  pause is the only freeze — the simulation is forward-only (see
  `SPEC_SCALE0_RUNTIME_PIPELINE.md` §8).
- **Switch (scenario→scenario):** there is **no `onExit`/`dispose` hook on a scenario**.
  Teardown is implicit: the next `loadScale0Scenario` calls `bridge.reset()` (via the seed
  dispatcher, `index.js:59-60`) and `setFluxMock` disposes the previous mock
  (`store.js:137-148`, idempotent, no-op for non-MockBridge).
- **Exit (Scale 0 → another scale):** `Scale0LifecycleController.destroy` (`controller.js:282-299`)
  disposes the four overlay panels and calls `exitScale0()` → `clearFluxMock()` (disposes +
  nulls the mock). `mount` (`:264-280`) re-creates panels idempotently on re-entry.
  (Lifecycle validity verified end-to-end in `AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md`.)
- **Resize:** `resizeScale0Lattice` (`:356-433`) heap-guards the new size, resizes the
  bridge, then **re-loads the scenario** via `getScale0Scenario(id).load(getPhysicsHarness(bridge), { id })`
  (`:400`) — fixed 2026-06-05 (A1). Pre-fix this passed a bare `{ bridge }`, which the refactored
  factory `load` could not call `setupScenario` on, silently breaking resize on every factory
  scenario (the fluxMock was never rebuilt).

### 5.5 Overlay-preference round-trip

Because `resetScale0VisualState` (`:288-299`) clears every field overlay, the loader snapshots
the user's overlay buttons before the reset (`captureOverlayPreferences` `:237-248`) and
re-applies them after (`restoreOverlayPreferences` `:257-286`). Both walk `FIELD_BUTTON_IDS`
and `FIELD_BUTTON_TO_FLAG`, which — since the B1 fix (2026-06-05) — are **derived** from
`ui/dom.js` `FIELD_TOGGLE_BINDINGS`, the canonical **36-entry** buttonflag map shared with
`bindings.js` and kept in lockstep with `store.js` `FIELD_TOGGLE_KEYS` (also 36). Before the fix
they were a hand-maintained 32-entry mirror that had fallen behind the four 2026-06-03 substrate
overlays (`showStateField`/`showLatency`/`showGaussResidual`/`showMooreDecomp`).

---

## 6. Contracts (the modular boundaries)

These are the invariants that keep the four layers coherent. Each is a candidate for an
explicit, testable boundary in the modularization roadmap.

### 6.1 Toggle whitelist + reset-between-loads + prerequisite ordering

- **Source of truth:** `config/toggles.js` `SCALE0_TOGGLES` (`:14-33`, 18 entries
  `[key, default, domId]`), mirrored as a C++ comment in `engine/include/ftd/scenarios.h:49-57`.
- **Reset:** `applyToggleDefaults` (`scenario-loader.js:136-190`) resets all 18 to default,
  applies `SCALE0_SCENARIO_OVERRIDES[id]` (prerequisite-sorted so e.g. `dual_substrate`
  precedes `weak_transmutation` — the C++ `TermToggles::validate` dependency, documented
  `:190-204`), then `LIGHT_SCENARIO_OVERRIDES` for `light-*`.
- **The contract** (`scenario-loader.js:31-45`, `scenarios.h:49-57`): a scenario body may
  mutate **only** whitelisted toggles; non-whitelisted research toggles (`pair_production`,
  `langevin`, `latency_field`, `emergent_forces`, … — enumerated `toggles.js:35-61`) are
  user-owned and **must not** be touched, or must be restored at scenario-end. Violations
  are the "toggle-leak vector ARC-1 audited."
- **Investigated → not a leak (audit B3, closed):** the custom-`load()` scenarios
  (`scenario-registry.js` qgp `:86-91`, emergent `:151-157` etc.) call `bridge.setToggle('langevin',
  true)` and never restore it — but runtime shows **no leak** (the bridge reset in `setupScenario`
  clears it on the next load; guard: `tests/scale0-toggle-leak.spec.js`). What remains is the
  *duplication* of two toggle mechanisms (declarative overrides vs imperative custom-loads) — a
  quality item (B2), not a bug.

### 6.2 Reset-before-dispatch

The caller must `reset()` the lattice before seeding. Enforced inside the dispatchers:
`runSetupScenario` (`index.js:59-60`) calls `harness.reset()`/`bridge.reset()` first; the C++
contract states the same (`scenarios.h:46-47`). Seed bodies therefore assume a flux-zero,
particle-empty lattice.

### 6.3 JS  C++ parity

`tests/scenario-parity.spec.js` is a source-text lint (no WASM load) with four assertions
(`:128-188`):

1. every JS `case` has a C++ `name ==` branch (`:128-141`);
2. every C++ branch has a JS `case`, minus `KNOWN_LEGACY_ONLY` (`:37-45`, `:143-155`);
3. every `ftd_wasm.cpp`/`bindings_render_bridge.cpp` legacy branch is shared or allowlisted
   (`:157-174`);
4. every **UI-registry** scenario has a JS impl (`:176-188`).

The guard is **GREEN** (6/6, 2026-06-05). It was hardened (B5) so the registry extractor
(`:112-123`) now matches **both** the `makeScenario(...)` factory form **and** the custom
object-literal `id:` form — the 10 custom-literal scenarios are no longer invisible (UI inventory
86 → 96) — plus a new **orphan-metadata** assertion (every `S0_SEED_SCENARIO_METADATA` key must map
to a real scenario). Before the fix the extractor matched only the factory form, and there was no
registry  metadata check.

### 6.4 Bridge direct-read surface

`bridge/bridge-contract.js` `SCALE0_DIRECT_READS` (`:80-110`, 20 entries) is the single source
of truth for read methods consumers call *directly* on a bridge (not via
`capabilities.scale0.*`). The worker proxy forwards each to its shadow; `scale0-worker.spec.js`
asserts coverage. This is the **reference example of a contract done right** — a named export
consumed by both the implementation and its regression test. (Full treatment:
`AUDIT_BRIDGE_WIRING_2026-06-03.md`.)

### 6.5 Overlay-preference round-trip

Covered in §5.5: the loader's `FIELD_BUTTON_IDS` / `FIELD_BUTTON_TO_FLAG` must stay in lockstep
with `store.js`'s `FIELD_TOGGLE_KEYS`. There is no programmatic link today (cf. the
`createFieldFlags()` precedent, §8).

### 6.6 Per-scenario boundary

A scenario may **pin its lattice boundary** instead of inheriting the live DOM controls.
`config/toggles.js` `SCALE0_SCENARIO_BOUNDARY` — a `{ reflective?, shape? }` map mirroring the
`SCALE0_SCENARIO_OVERRIDES` toggle-default pattern — is consulted by the loader resolvers
`boundaryShapeFor(id)` / `reflectiveFor(id)` (`scenario-loader.js:33-44`) at **every**
boundary-application site: the flux-mock create (`:270-271`), `applyAuxiliaryDefaults`
(`:120-128`), and the resize path (`:351-364`). Scenarios without an entry fall back to
`#boundary-select` / `#toggle-reflective` (unchanged behavior).

Added 2026-06-05 with `flux-zero-point`, which declares `reflective: true` so its irreducible
energy floor is trapped rather than absorbed at the lattice edges (without it the floor bleeds
away — not "zero-point"). This **removes a real coupling**: before, the loader read the boundary
**only** from the DOM controls, so a scenario could not declare its own boundary need (the
UIbridge coupling noted as a modular-boundary gap in §6's preamble). The same change also fixed
a latent clobber where `applyAuxiliaryDefaults` forced `reflective=false` *after* the scenario's
boundary had been set, so a scenario-set reflective boundary never stuck. Verified by
`tests/scale0-zero-point.spec.js` (persistent floor) and the all-scenario sweep (other 95
scenarios unaffected).

---

## 7. C++ scenario model & parity

Structurally 1-for-1 with the JS layer (by design — `scenarios.h:17-27`):

| Concern | JS | C++ |
|---|---|---|
| Dispatcher | `runSetupScenario` `index.js:51-91` | `dispatch_scenario` `scenarios.cpp:63-78` |
| Group fn | `setupXxxScenario(name, harness, ctx)` → bool | `setup_xxx_scenario(rb, name)` → bool `scenarios.h:63-68` |
| Seed primitives | `_helpers.js` (`injectRadialEnvelope`, …) | `_helpers.h` macros `IF/IW/IP/IPF/SET_VEL/LOCK/SET_SPIN` `:40-63` |
| RNG | `Math.random()` (unseeded) | `thread_local mt19937(0xC0DEFACE)`, reset per dispatch `scenarios.cpp:40-68` |
| Toggle mutation | `harness.setToggle` / bridge | `rb.toggles.<field> = …` |

**Stochastic parity is statistical, not bit-exact** (`scenarios.cpp:29-43`): JS `Math.random()`
is unseedable, so the six stochastic scenarios (`flux-random-genesis`, `flux-thermalization`,
`flux-vacuum-foam`, `flux-zero-point`, `quantum-born-rule`, `quantum-casimir`) match in
distribution only. The C++ seed gives repeatability *within* a process run.

`ftd_wasm.cpp` is now a thin delegator to `dispatch_scenario`; pre-port it knew only 35 of the
83 UI scenarios and the rest silently no-op'd on WASM (`scenarios.h:8-15`) — the historical
motivation for the parity guard.

---

## 8. The modular target — two documented directions

The audit decides which to pursue; both are recorded here so the SPEC is the single
reference. The relevant precedent already in-tree: `state/store.js:56-64` `createFieldFlags()`
derives the all-off field-flags bag **programmatically** from `FIELD_TOGGLE_KEYS` precisely
because the old hand-maintained mirror drifted ("Auditors caught multiple drift incidents;
keeping the two in lockstep programmatically removes the hazard"). The question is how far to
generalize that principle to the scenario definition.

### Direction A — keep four layers, add an explicit sync contract + CI guard

Leave the registry / JS impl / C++ impl / metadata layers where they are, but:

- extend the parity guard (§6.3) to cover **custom-literal** registry entries and add a
  **registry  metadata** assertion;
- add a runtime call (or test) to `validateScale0ScenarioRegistry()`;
- document the toggle whitelist + overlay round-trip as named, tested boundaries.

Lower risk, incremental, no churn to the 96/101/99 entries. The four layers remain four;
drift just fails loudly instead of silently.

### Direction B — unify to one scenario descriptor source of truth

A single per-scenario record (id, title, category, tags, epistemic, toggle profile,
seed-fn reference, capabilities) from which the registry, the metadata, the toggle-override
table, and the parity manifest are **generated or validated**. Collapses the custom-`load()`
duality (§4) into declarative toggle profiles, brings `flux/light/s0-field/s0-vacuum` under
one metadata umbrella, and makes the seed impl the only hand-written per-scenario code.
Higher payoff, larger one-time churn; the C++ seed bodies stay hand-written but become the
*only* place a new scenario needs C++.

---

## 9. Quick reference — files

| Layer | File |
|---|---|
| Registry | `scales/scale0/scenario-registry.js` |
| JS dispatch | `bridge/scenarios/index.js` + `{flux,light,quantum,vacuum,s0-seed,s0-field}-scenarios.js` + `_helpers.js` |
| C++ dispatch | `engine/src/scenarios.cpp` + `engine/src/scenarios/*.cpp` + `_helpers.h`; contract `engine/include/ftd/scenarios.h` |
| Metadata | `config/scenarios.js` |
| Toggles | `config/toggles.js` |
| Lifecycle | `scales/scale0/runtime/scenario-loader.js`, `scales/scale0/controller.js`, `scales/scale0/ui/bindings.js`, `scales/scale0/state/store.js` |
| Harness | `physics/index.js`, `physics/physics-harness.js` |
| Bridges | `bridge/wasm-bridge.js`, `bridge/mock-bridge.js`, `bridge/mock-bridge-proxy.js`, `bridge/bridge-contract.js` |
| Parity | `tests/scenario-parity.spec.js` |

*Counts — 96 registry entries (86 factory + 10 custom literals), 95 unique JS scenarios,
95 unique C++ scenarios (+1 C++ legacy; JSC++ parity verified **green** 6/6 2026-06-05); raw
`case` / `name ==` occurrences are 101 / 99. All `file:line` references are as of the 2026-06-05
working tree; re-derive from source before relying on them.*
