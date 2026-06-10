# engine/web/js/scales/scale0 — Scale-0 controller (lattice / substrate)

**Purpose.** Top-level orchestrator for the Scale-0 (lattice/substrate)
view of the dashboard. Owns the tick loop, scenario lifecycle, UI state,
panel bindings, and viewport adapter for everything happening below
the particle scale.

## Public API

The controller exports lifecycle hooks consumed by `app.js` (the
top-level dashboard orchestrator):

```js
import * as Scale0 from './scales/scale0/controller.js';

Scale0.bindUI(ctx);                  // wire DOM
Scale0.loadScenario(ctx, name);      // load + reset
Scale0.animateLattice(ctx, t);       // per-frame
Scale0.resizeLattice(ctx);           // on window resize
Scale0.exitScale0();                 // tear down
```

Plus: `getFluxMock`, `setLatticeNeedsUpload`, `setForceStyle`,
`setFieldToggle`, `getCurrentScenarioId`, `handleShortcutKey`,
`shouldUseFluxMock`.

## Internal structure (3-folder package convention; see ADR-0004)

| Folder | Role |
|---|---|
| `controller.js` | Top-level orchestrator (entry point) |
| `runtime/` | Tick loop (`tick.js`, `frame-sync.js`), scenario loading (`scenario-loader.js`), diagnostics (`diagnostics.js`), field overlays (`field-overlays.js`) |
| `ui/` | DOM bindings (`bindings.js`), controls (`controls/`), overlays (`overlays/`), panels (`panels/`) |
| `state/` | Reactive state store (`store.js`) |
| `viewport-adapter.js` | Bridge to the Three.js renderer |
| `scenario-registry.js` | Catalog + dropdown population |
| `pedagogy.js` | Educational walkthroughs |

## Dependencies

- **Imports from**: `../../constants.js`, `../../config/toggles.js`, `../../bridge/scenarios/`, `../../bridge-init.js` (re-export shim; underlying classes live in `../../bridge/`), `../../viewport.js` (1256-LOC orchestrator that composes 4 sub-renderers — see below)
- **Imported by**: `../../app.js` (dashboard root)
- **No cross-scale imports** (Scale 1, 2, etc. are independent)

The viewport adapter ultimately drives 4 cascading sub-renderers under
`engine/web/js/viewport/`: `scene-core.js` (camera/boundary/render loop),
`flux-renderer.js` (volume + slice + streamlines), `particle-renderer.js`
(particles + trails + velocity vectors), `field-renderer.js` (27+ field
overlays + canonical mesh-factory home). Lifecycle and cross-renderer
helpers follow the cascade-callback pattern (ADR-0010) and the
mesh-factory callback pattern (ADR-0011).

## The `ctx` object

The controller passes a shared `ctx` object to every submodule. See
[CONTRACTS.md §3](../../../../CONTRACTS.md#3--scale-controller-ctx-contract)
for the canonical shape and the rules that govern who reads/writes what.

## Scenario load flow

1. User changes the scenario dropdown
2. `bindings.js` calls `loadScenario(ctx, name)`
3. `runtime/scenario-loader.js`:
   a. Checks `shouldUseFluxMock` (`flux-*`, explicit mock-owned demos such as `s0-field-spacetime-forcing-boundary`, or bridge-without-flux-volume → MockBridge)
   b. Allocates fluxMock if needed
   c. Calls `bridge.setupScenario(name)` → routes to `bridge/scenarios/index.js` dispatcher
   d. `applyToggleDefaults` resets to `SCALE0_TOGGLES` then applies `SCALE0_SCENARIO_OVERRIDES[name]` (prerequisite-first sort)
   e. Resets viewport visual state, charts, memory budget
4. Tick loop resumes via `runtime/tick.js`

## How to extend

- **Add a new panel** → drop into `ui/panels/` or `ui/overlays/`; register in `bindings.js`
- **Add a new control** → drop into `ui/controls/`; wire in `wire.js`
- **Add a new tick phase** → extend `runtime/tick.js`; document the phase order
- **Change scenario behavior** → edit `runtime/scenario-loader.js` (loader logic) or `bridge/scenarios/<group>-scenarios.js` (per-scenario setup)

## Invariants

- `ctx.bridge` MAY change between scenario loads; code MUST re-read it each frame
- `ctx.bridge.capabilities.scale0` is the sole interface for physics-state reads/writes — never poke `bridge._toggles` etc. directly
- Per-tick caches on the bridge (e.g. `_energyCacheTick`) are owned by the bridge; UI/panels MUST NOT reset them

## Related docs

- [CONTRACTS.md §3](../../../../CONTRACTS.md#3--scale-controller-ctx-contract), [§4](../../../../CONTRACTS.md#4--scenario-dispatch-contract)
- [docs/adr/0004-scale-controllers.md](../../../../docs/adr/0004-scale-controllers.md)
- [docs/adr/0010-cascade-callback-pattern.md](../../../../docs/adr/0010-cascade-callback-pattern.md) — viewport sub-renderer lifecycle hooks
- [docs/adr/0011-mesh-factory-callback.md](../../../../docs/adr/0011-mesh-factory-callback.md) — cross-sub-renderer mesh helpers
- [engine/web/docs/USER_GUIDE.md](../../../docs/USER_GUIDE.md)
- [META_PROJECT_ATLAS.md](../../../../META_PROJECT_ATLAS.md) §1, §2
