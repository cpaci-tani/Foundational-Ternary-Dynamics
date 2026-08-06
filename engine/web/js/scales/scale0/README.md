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

Plus: `getFluxMock` (legacy name for the off-thread `WasmBridgeProxy`),
`setLatticeNeedsUpload`, `setForceStyle`, `setFieldToggle`,
`getCurrentScenarioId`, `handleShortcutKey`.

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
   a. If `wasmWorkerEligible` (COI + SAB + primary `isWasm` + worker not disabled), construct
      `WasmBridgeProxy` and store it in legacy `fluxMock` / `useFluxMock` slots
   b. Otherwise keep physics on `ctx.bridge` (`WasmBridge` or WebSocketBridge)
   c. Active bridge runs `setupScenario` → WASM `ftd::dispatch_scenario` (JS `bridge/scenarios/`
      is parity-only; registry `load(harness)` always uses the catalog id)
   d. `applyToggleDefaults` + post-load profile / research terms on the active owner
   e. `viewport.setLatticeSize(activeN)` when viewport N differs from active bridge
   f. Resets visual state, restores overlay prefs, marks `fieldNeedsUpdate` + `latticeNeedsUpload`
4. Tick loop resumes via `runtime/tick.js` (worker self-ticks when `useFluxMock`; else main bridge)

Worker init failure → `onInitFailure` falls back to in-thread WASM and latches
`ctx._wasmWorkerDisabled`. Setup failure (`setupScenario` false) → toast via `onSetupFailure`.

## Invariants

- `ctx.bridge` MAY change between scenario loads; code MUST re-read it each frame
- **Active physics owner:** when `state.useFluxMock`, reads/ticks use `getActiveScale0Bridge(ctx, state)` /
  `getActiveScale0Capability` / `getActiveLatticeSize` — not raw `ctx.bridge` alone
- **Physics writes:** scenarios and controls go through `getPhysicsHarness(activeBridge)` — not
  `bridge.tick()`, `bridge.setupScenario()`, or `harness.bridge.*` except documented experiment flags
- `ctx.bridge.capabilities.scale0` is the capability surface on whichever bridge owns physics
- Per-tick caches on the bridge (e.g. `_energyCacheTick`) are owned by the bridge; UI/panels MUST NOT reset them
- Overlay toggles (`FIELD_TOGGLE_KEYS`) are visual-only — they never flip physics toggles or tick cadence

## Related docs

- [CONTRACTS.md §3](../../../../CONTRACTS.md#3--scale-controller-ctx-contract), [§4](../../../../CONTRACTS.md#4--scenario-dispatch-contract), [§5](../../../../CONTRACTS.md#5--telemetry-contract)
- [docs/adr/0004-scale-controllers.md](../../../../docs/adr/0004-scale-controllers.md)
- [docs/adr/0010-cascade-callback-pattern.md](../../../../docs/adr/0010-cascade-callback-pattern.md) — viewport sub-renderer lifecycle hooks
- [docs/adr/0011-mesh-factory-callback.md](../../../../docs/adr/0011-mesh-factory-callback.md) — cross-sub-renderer mesh helpers
- [engine/web/docs/SPEC_SCALE0_RUNTIME_PIPELINE.md](../../../docs/SPEC_SCALE0_RUNTIME_PIPELINE.md)
- [engine/web/docs/audits/AUDIT_SCALE0_CALLSTACK.md](../../../docs/audits/AUDIT_SCALE0_CALLSTACK.md)
- [engine/web/docs/audits/AUDIT_SCALE0_SCENARIO_HARNESS_DRY.md](../../../docs/audits/AUDIT_SCALE0_SCENARIO_HARNESS_DRY.md)
- [engine/web/docs/USER_GUIDE.md](../../../docs/USER_GUIDE.md)
- [META_PROJECT_ATLAS.md](../../../../META_PROJECT_ATLAS.md) §1, §2

## How to extend

- **Add a new panel** → drop into `ui/panels/` or `ui/overlays/`; register in `bindings.js`
- **Add a new control** → drop into `ui/controls/`; wire in `wire.js` (use `dualHarness` / active-owner helpers)
- **Add a new tick phase** → extend `runtime/tick.js`; document the phase order
- **Change scenario behavior** → edit `runtime/scenario-loader.js` (loader logic) or `bridge/scenarios/<group>-scenarios.js` (per-scenario setup)
