# CONTRACTS — Cross-Module Interface Summary

**Audience:** humans and agents changing shared module boundaries.
**Status:** active summary, refreshed after the June 2026 web cleanup pass.

This file is intentionally a summary. Detailed web contracts live beside the
web engine:

- `engine/web/ARCHITECTURE.md`
- `engine/web/docs/INDEX.md`
- `engine/web/docs/SPEC_SCALE0_BRIDGE_ARCHITECTURE.md`
- `engine/web/docs/SPEC_SCALE0_RUNTIME_PIPELINE.md`
- `engine/web/docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`
- `engine/web/js/bridge/README.md`
- `engine/web/js/bridge/scenarios/README.md`
- `engine/web/js/scales/scale0/README.md`

If a detailed Scale-0 rule here conflicts with those web specs, the web spec
and current source code win. Update this summary when changing a public
interface or adding a new cross-module pattern.

---

## 1. Bridge State Contract

Mock bridge subsystems use the live-reference factory pattern. A provider
factory receives the bridge instance and must keep that live reference, not a
destructured snapshot.

Reference examples:

- `engine/web/js/bridge/mock-diagnostics.js`
- `engine/web/js/bridge/mock-lattice-samplers.js`
- `engine/web/js/bridge/mock-particle-engine.js`
- `engine/web/js/bridge/mock-atom-engine.js`

Rules:

1. Provider factories keep the bridge/state object by reference.
2. Returned methods re-read state on every call.
3. Cache fields are owned and documented by the provider that writes them.
4. Providers must honor bridge-side cache invalidation sentinels.
5. Consumers must not poke private bridge fields such as `_fluxJ`, `_toggles`,
   or `_particles` directly.

---

## 2. Capability Factory Contract

Bridge consumers should prefer `bridge.capabilities.scaleN.*` over raw bridge
methods. The capability getter is installed by
`engine/web/js/bridge-init.js` through
`engine/web/js/bridge/capabilities/install.js`.

Reference factories:

- `engine/web/js/bridge/capabilities/scale0.js`
- `engine/web/js/bridge/capabilities/scale1.js`
- `engine/web/js/bridge/capabilities/scale2.js`

Rules:

1. A capability factory returns a stable method surface for a scale.
2. Methods must be safe to call as object methods without manual binding.
3. Optional capabilities may return `null`/empty samples, but the absence must
   be guarded by consumers.
4. Mock/WASM/native/worker bridges must satisfy the same consumer-facing
   surface for the scale they claim to support.
5. Adding a new public sampler or diagnostic requires updates to the bridge
   implementation, capability factory, docs, and regression tests.

Scale 0 is forward-only. Snapshot/timeline methods formerly documented here
are no longer part of the active surface.

---

## 3. Scale Controller Context Contract

`app.js` builds a live context object and passes it into scale controllers.
Controllers should read from `ctx` at use time rather than closing over stale
copies.

Common fields:

```js
ctx = {
  bridge,             // active app bridge
  viewport,           // shared viewport facade
  appShell,           // UI shell facade
  inspector,          // inspector runtime
  telemetryHub,       // shared telemetry buffers
  running,            // global play/pause state
  scenarioRunning,    // scenario-level run state where applicable
  ticksPerFrame,      // speed control
  engineMode,         // dashboard mode: lattice, particles, atoms, ...
  activeTab,
  frameCount,
  dom,

  pauseSimulation(),
  resetAllVisualState(),
  applyTicksPerFrameFromSlider(value),
  applyBoundaryShape(shape),
  applyReflectiveBoundary(on),
  clearCharts(),
}
```

Rules:

1. Controllers are leaves; they do not import `app.js`.
2. `ctx.bridge` may change across mode switches and scenario ownership changes.
3. Code that needs the active Scale-0 physics owner must account for
   `state.useFluxMock && state.fluxMock`.
4. UI code should prefer registry/shell services over direct global DOM lookups
   when a service exists.

---

## 4. Scale-0 Scenario Contract

The current Scale-0 scenario system has multiple coordinated layers:

- UI registry: `engine/web/js/scales/scale0/scenario-registry.js`
- JS seeds: `engine/web/js/bridge/scenarios/*.js`
- C++ seeds: `engine/src/scenarios/*.cpp`
- Metadata: `engine/web/js/config/scenarios.js`
- Toggle profiles: `engine/web/js/config/toggles.js`

The authoritative scenario architecture is
`engine/web/docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`.

Rules:

1. Scenario ids must stay consistent across registry, JS dispatcher, C++
   dispatcher, metadata, and tests.
2. A JS scenario handler returns `true` when it handles an id and `false` when
   the prefix does not apply.
3. Scenario handlers receive `(name, harness, ctx)` and should use shared
   helpers from `engine/web/js/bridge/scenarios/_helpers.js`.
4. Prefer declarative toggle profiles in `SCALE0_SCENARIO_OVERRIDES` over
   imperative scenario-body toggle mutation. Bridge reset currently prevents
   known leaks, but imperative mutations remain harder to audit and document.
5. Update `engine/web/tests/scenario-parity.spec.js` and related health tests
   when scenario structure changes.

---

## 5. Telemetry Contract

`engine/web/js/telemetry-hub.js` is the shared browser telemetry hub. Panels and
charts should read from hub buffers or descriptor tables unless they are
intentionally sampling a live active bridge owner.

Rules:

1. Scale-0 diagnostics/audit/Lagrangian collection must use the active owner:
   the app bridge for WASM/native scenarios, or `state.fluxMock` for mock/worker
   owned scenarios.
2. WASM `diagnostics.totalEnergy` follows the dashboard physical energy channel
   (`getEnergyAudit().totalEnergy`); the native baseline is preserved separately
   as `vacuumBaselineEnergy` when available.
3. Demand-gated telemetry must be paired with visible-consumer checks and
   regression coverage.
4. New telemetry rows need descriptor, hub, bridge/capability, and tests.

Reference tests:

- `engine/web/tests/scale0-panel-wiring.spec.js`
- `engine/web/tests/scale0-scenario-telemetry-contract.spec.js`
- `engine/web/tests/scale0-telemetry-gating.spec.js`

---

## 6. Documentation Cleanup Contract

For cleanup work:

1. Preserve provenance for meaningful historical docs by moving them to an
   explicit historical/provenance location.
2. Generated artifacts may be removed when their tracked source exists and no
   active link depends on the render.
3. Update navigation in the same change: indexes, READMEs, and active specs.
4. Do not promote epistemic tags or scientific claims during cleanup.
5. Verify with path/link searches and `git diff --check`.

For web docs, use `engine/web/docs/historical/` for tracked historical
provenance. Avoid `archive/` under this repository unless `.gitignore`
exceptions are added deliberately.
