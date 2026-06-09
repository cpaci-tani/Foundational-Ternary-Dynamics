# FTD Scenario Architecture

This document maps how the engine turns a named scenario into an initial
condition, a bridge owner, a toggle profile, and then ordinary tick dynamics.
It is the cross-scale companion to:

- [VISUAL_GUIDE.md](VISUAL_GUIDE.md) - learner-facing view of what the simulator teaches.
- [CALLSTACKS.md](CALLSTACKS.md) - call graphs for primary runtime features.
- [SPEC_ENGINE.md](SPEC_ENGINE.md) - detailed engine reference.
- [web/docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md](web/docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md) - deep Scale 0 scenario subsystem audit.

## 1. What A Scenario Is

A scenario is a one-shot seed recipe. It is not a saved simulation state and it
is not a per-frame script.

At load time a scenario usually does four things:

1. Resolve a string id from a UI registry or caller argument.
2. Reset or initialize the bridge that will own the run.
3. Apply scale-local default toggles plus scenario-specific overrides.
4. Inject particles, flux, atoms, molecules, bodies, or camera/visual context.

After seeding, the normal engine tick owns the evolution. That distinction
matters: scenario code creates starting conditions; runtime phase loops create
the dynamics.

## 2. Architectural Layers

The scenario system is intentionally layered. Different scales use different
engines, but the same ownership pattern recurs.

| Layer | Role | Typical files |
|---|---|---|
| Catalog / registry | User-facing ids, titles, categories, epistemic labels | `web/js/scales/scale0/scenario-registry.js`, `web/js/config/scenarios.js` |
| Loader / lifecycle | Reset, toggle defaults, bridge choice, UI sync, resize/reload | `web/js/scales/*/controller.js`, `web/js/scales/scale0/runtime/scenario-loader.js` |
| Bridge capability | Common method such as `setupScenario`, plus injection helpers | `web/js/physics/physics-harness.js`, `web/js/bridge/*`, `wasm/ftd_wasm.cpp` |
| Seed bodies | Imperative setup switch/cases that place initial objects/fields | `web/js/bridge/scenarios/*.js`, `src/scenarios/*.cpp`, `web/js/scales/*/scenarios.js` |
| Toggle profiles | Scenario-owned toggle defaults and boundary profiles | `web/js/config/toggles.js` |
| Static manifests | Data catalogs for docs/configuration surfaces; not always the live runtime source | `config/scenarios/*.json` |

The live runtime key is the scenario id string. It crosses the UI, loader,
bridge, and seed layers. If an id exists in only one layer, the scenario is
partially wired.

## 3. Scale 0 Lattice Scenarios

Scale 0 is the most developed scenario stack because it has two execution
implementations: JS MockBridge and C++ RenderBridge through WASM/native paths.
The live dashboard path is spread across several definition layers:

| Concern | File |
|---|---|
| UI descriptors and optgroups | [web/js/scales/scale0/scenario-registry.js](web/js/scales/scale0/scenario-registry.js) |
| Runtime load/resize/tick owner | [web/js/scales/scale0/runtime/scenario-loader.js](web/js/scales/scale0/runtime/scenario-loader.js) |
| JS dispatcher and seed bodies | [web/js/bridge/scenarios/index.js](web/js/bridge/scenarios/index.js), [web/js/bridge/scenarios/](web/js/bridge/scenarios/) |
| C++ dispatcher and seed bodies | [include/ftd/scenarios.h](include/ftd/scenarios.h), [src/scenarios.cpp](src/scenarios.cpp), [src/scenarios/](src/scenarios/) |
| WASM binding | [wasm/ftd_wasm.cpp](wasm/ftd_wasm.cpp) |
| Toggle/boundary overrides | [web/js/config/toggles.js](web/js/config/toggles.js) |
| Metadata and explanatory text | [web/js/config/scenarios.js](web/js/config/scenarios.js) |
| Deep subsystem spec | [web/docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md](web/docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md) |

### 3.1 Descriptor Contract

Most Scale 0 entries are created by `makeScenario(...)` and have this shape:

```js
{
  id,
  scale: 'lattice',
  title,
  category,
  tags,
  defaultParams: {},
  requiredCapabilities: ['scale0'],
  epistemicStatus,
  load(harness, params) {
    harness.setupScenario(params.id || id);
  }
}
```

Some entries use custom `load(...)` functions when they need to stage special
toggle or bridge work before calling `setupScenario`.

### 3.2 Runtime Load Callstack

```text
UI select / app caller
  -> scale0/controller.js loadScenario(ctx, id)
  -> runtime/scenario-loader.js loadScale0Scenario(ctx, state, viewportAdapter, id)
     -> getScale0Scenario(id)
     -> shouldUseFluxMock(ctx.bridge, id)
     -> maybe create MockBridge / MockBridgeProxy
     -> applyToggleDefaults(mainScale0, mockScale0, id)
     -> reset visual state and auxiliary defaults
     -> getPhysicsHarness(active bridge)
     -> scenario.load(harness, params)
        -> harness.setupScenario(id)
        -> bridge.setupScenario(id)
        -> JS runSetupScenario(...) or C++ dispatch_scenario(...)
     -> apply late boundary/gravity/wave flags
     -> sync UI selection, sliders, override markers, and overlay prefs
```

`stepScale0(...)` then ticks the owner chosen during load. If the scenario is
mock-owned, the flux mock is ticked. Otherwise the main bridge is ticked.

### 3.3 Bridge Ownership

`shouldUseFluxMock(...)` decides whether the scenario runs on the JS mock path
or the primary bridge:

1. Native GPU and WebSocket bridges do not use the flux mock.
2. `flux-*` scenarios are mock-owned by default.
3. Other scenarios use the mock only if the active bridge cannot expose a flux
   volume.

When worker conditions are available, `MockBridgeProxy` can put that mock in a
Web Worker backed by `SharedArrayBuffer`. This creates a two-bridge setup:
`ctx.bridge` remains the primary engine, while `state.fluxMock` owns the actual
Scale 0 ticking for that scenario.

### 3.4 Dispatch And Parity

JS and C++ use the same prefix-dispatch architecture:

```text
flux-*       -> flux scenario group
light-*      -> light scenario group
quantum-*    -> quantum scenario group
s0-vacuum-*  -> vacuum particle group
s0-seed-*    -> particle/aggregate seed group
s0-field-*   -> field configuration group
```

JS dispatch lives in [web/js/bridge/scenarios/index.js](web/js/bridge/scenarios/index.js).
C++ dispatch lives in [src/scenarios.cpp](src/scenarios.cpp) and is declared in
[include/ftd/scenarios.h](include/ftd/scenarios.h). The C++ path resets a
thread-local scenario RNG at the start of each `dispatch_scenario(...)` call so
stochastic C++ seeds are reproducible per setup call. JS `Math.random()` is not
bit-exact with that stream, so stochastic parity is structural/statistical, not
sample-for-sample identical.

### 3.5 Toggle Ownership

Scale 0 scenario loading resets only the dashboard scenario whitelist in
`SCALE0_TOGGLES`. Keys outside that list are long-lived user controls unless a
scenario explicitly owns and restores them.

Preferred pattern:

1. Put scenario default toggle changes in `SCALE0_SCENARIO_OVERRIDES`.
2. Use `LIGHT_SCENARIO_OVERRIDES`, boundary maps, absorbing-boundary maps, or
   mass-gravity maps for special profiles.
3. Avoid mutating non-whitelisted toggles inside seed bodies.

This contract is mirrored in comments in both
[web/js/scales/scale0/runtime/scenario-loader.js](web/js/scales/scale0/runtime/scenario-loader.js)
and [include/ftd/scenarios.h](include/ftd/scenarios.h).

### 3.6 Adding A Scale 0 Scenario

To add a fully wired Scale 0 scenario:

1. Add the JS seed case in the correct group file under
   [web/js/bridge/scenarios/](web/js/bridge/scenarios/).
2. Mirror the seed in the matching C++ group under [src/scenarios/](src/scenarios/).
3. Register the id in [web/js/scales/scale0/scenario-registry.js](web/js/scales/scale0/scenario-registry.js).
4. Add metadata/explanatory text in [web/js/config/scenarios.js](web/js/config/scenarios.js) if the UI or knowledge base needs it.
5. Add toggle/boundary overrides in [web/js/config/toggles.js](web/js/config/toggles.js) when non-default behavior is required.
6. Run the web parity/health specs that cover scenario inventory and loading.

The root rule is simple: scenario id, JS seed, C++ seed, registry entry, and
toggle profile must agree.

## 4. Macro-Scale Scenario Patterns

The higher scales do not mirror JS and C++ seed bodies one-for-one the way Scale
0 does. Their controllers own more of the scenario lifecycle directly.

| Scale | Loader | Seed implementation | Runtime owner |
|---|---|---|---|
| Scale 1 particles | `web/js/scales/scale1/controller.js::loadPEScenario` | `web/js/scales/scale1/scenarios.js::setupPEScenario` | `ParticleEngine` through bridge capability methods |
| Scale 2 atoms | `web/js/scales/scale2/controller.js::loadAEScenario` | `web/js/scales/scale2/scenarios.js::setupAEScenario` | `AtomEngine` |
| Scale 3 molecules | `web/js/scales/scale3/controller.js::loadMoleculeScenario` | molecule loader plus special-case crystal/custom paths | `AtomEngine` reused as the molecular substrate |
| Scale 4 planetary/reference context | `web/js/scales/scale4/controller.js::loadScenario` | `PlanetaryMockBridge.setupScenario` | JS planetary mock bridge and renderer |
| Scale 5 cosmic | `web/js/scales/scale5/controller.js::loadCosmicScenario` | `web/js/bridge/cosmic-scenarios/index.js::runCosmicScenario` | `CosmicMockBridge` / cosmic renderer in dashboard; C++ `CosmicEngine` for native scale engine work |

### 4.1 Scale 1 Particles

`loadPEScenario(...)` resets visual state, initializes the particle engine,
restores particle-engine defaults, syncs UI controls, then calls
`setupPEScenario(...)`. Scenario cases add particles through bridge methods
such as `peAddParticle` and `peAddLockedParticle`. A scenario may return state
hints, for example black-hole overlay activation for `pe-micro-bh`.

### 4.2 Scale 2 Atoms

`loadAEScenario(...)` initializes the atom engine, resets atom toggles, syncs
parameters from the UI, clears molecule inspection state, and calls
`setupAEScenario(...)`. Scenario cases create atoms and bonds through AtomEngine
bridge methods. Individual atom scenarios disable auto-bonding so a single atom
does not immediately become a molecule.

### 4.3 Scale 3 Molecules

Scale 3 reuses AtomEngine. `loadMoleculeScenario(...)` resets atom state, loads
a molecule by id when the scenario name is `mol-<id>`, pre-bonds it, runs a
short stability dry run, then reloads and pre-bonds again for a clean starting
state. Special paths cover crystal and custom molecule starts.

### 4.4 Scale 4 Planetary / Reference Context

Scale 4 creates a `PlanetaryMockBridge`, reapplies remembered gravity mode
before setup, runs `bridge.setupScenario(name)`, creates the renderer and
inspector context, configures camera state, performs an initial render, and
starts the RAF-driven controller loop.

### 4.5 Scale 5 Cosmic

Scale 5 pauses the active loop, creates a `CosmicMockBridge`, calls
`bridge.setupScenario(scenarioName)`, attaches an inspector context, creates a
`CosmicRenderer`, configures camera and presets, renders once, then usually
auto-plays. Dashboard cosmic seed dispatch lives in
[web/js/bridge/cosmic-scenarios/index.js](web/js/bridge/cosmic-scenarios/index.js),
with galaxy and exotic setup bodies split into neighboring modules.

## 5. Static JSON Scenario Manifests

The files under [config/scenarios/](config/scenarios/) are useful static
manifests for scale catalogs, documentation, and data-driven experiments. They
should not be assumed to be the authoritative live dashboard registry unless a
specific controller imports them.

Current live dashboard Scale 0 dispatch is JS-registry plus JS/C++ seed bodies.
If a scenario id is added only to a JSON manifest, it may be visible to a config
reader but still absent from the browser dropdown, the C++ dispatcher, or the
WASM path.

## 6. CLI Demo Scenarios

The native CLI accepts lettered demo scenarios through `main.cpp` and the
`cli_demos` helper layer. These are standalone demonstration presets for
terminal runs. They are separate from the browser Scale 0 scenario registry and
from the JS/C++ prefix-dispatch scenario library.

Treat CLI demos as smoke-test and pedagogy entrypoints unless a test or spec
explicitly promotes one to a formal benchmark.

## 7. Scenario Safety Rules

- Scenario code seeds initial conditions; tick phases own the physics after
  load.
- Scale 0 seed bodies must preserve JS/C++ parity unless drift is documented
  inline and covered by tests.
- Scenario loaders should reset their scale-local bridge before seeding.
- Toggle changes should live in override maps when they are part of a scenario
  profile.
- Non-whitelisted toggles are user-owned state across Scale 0 scenario loads.
- GPU/WebSocket ownership decisions must not silently route canonical runs
  through a mock-only path.
- Static manifests and live registries must be reconciled deliberately; do not
  assume one updates the other.

## 8. Debugging Map

| Symptom | Start here |
|---|---|
| Scenario absent from dropdown | `web/js/scales/scale0/scenario-registry.js` or scale controller select population |
| Scenario dropdown exists but seeds nothing on WASM | `include/ftd/scenarios.h`, `src/scenarios.cpp`, matching `src/scenarios/*.cpp` group |
| JS mock and WASM differ | Compare `web/js/bridge/scenarios/<group>.js` to `src/scenarios/<group>.cpp`; run parity specs |
| Toggle appears stuck after switching scenarios | `web/js/config/toggles.js` whitelist and `runtime/scenario-loader.js::applyToggleDefaults` |
| Boundary or absorbing behavior changes unexpectedly | `SCALE0_SCENARIO_BOUNDARY`, `SCALE0_ABSORBING_SCENARIOS`, late boundary flags in the loader |
| Resize loses scenario state | `resizeScale0Lattice(...)` and fluxMock rebuild logic |
| Macro scenario loads but renders stale objects | Scale-specific controller reset/dispose path before seed |

For the per-feature runtime path after a scenario has loaded, use
[CALLSTACKS.md](CALLSTACKS.md).
