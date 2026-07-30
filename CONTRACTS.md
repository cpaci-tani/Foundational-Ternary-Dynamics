# CONTRACTS — Cross-Module Interface Summary

**Audience:** humans and agents changing shared module boundaries.

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

Reference examples (the Scale-1/2 engine helpers; Scale-0 is WASM-only and no
longer has JS live-ref factories):

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
4. All live bridges — `WasmBridge`, the `WasmBridgeProxy` worker path, and
   `WebSocketBridge` (native server) — must satisfy the same consumer-facing
   surface for the scale they claim to support.
5. Adding a new public sampler or diagnostic requires updates to the bridge
   implementation, capability factory, docs, and regression tests.

Scale 0 is forward-only. Snapshot/timeline methods formerly documented here
are no longer part of the active surface.

Scale-1 rule-4 clarification (2026-07-29 revision): the Scale-1 particle
engine always runs on the **main-thread `WasmBridge`** (native C++/WASM
`ParticleEngine` via `bridge/native-particle-engine.js`). `WasmBridgeProxy`
claims **scale0 only** and is exempt from carrying a `scale1` surface; the
promotion pipeline (`scales/scale1/promotion.js`) reads Scale-0 cluster data
from the ACTIVE Scale-0 owner via `capabilities.scale0` — including the
worker-path additions `stepScale0` (real single-step; `tickScale0` on the
proxy stays a no-op because the worker self-ticks) and the bridge-level
one-shot `coarsenToParticles()` (synchronous data on `WasmBridge`, a Promise
on the proxy). `WebSocketBridge` satisfies the Scale-1 surface by delegating
to an in-page `WasmBridge` fallback whose module load is kicked off on first
use; until (or unless) it loads, every `pe*` read returns a contract-empty
shape and `peGetBackendCapabilities().backend === 'unavailable'`.

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
5. Scale-1 ctx consolidation (2026-07-29): the per-frame `_scale1Ctx` carries
   the same load-bearing members as the full `_makeCtx()` shape —
   `telemetryHub`, `engineMode`, `isPanelVisible`, `resetAllVisualState` —
   and the former stub call sites (`resetScale1`, scenario-load shim) pass
   `_makeCtx()`. Scale-1 controllers therefore see ONE ctx shape everywhere;
   `resetScale1` additionally tolerates a minimal `{ viewport }` object for
   defensive robustness.

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
   owned scenarios. Prefer the canonical selectors in `scale0/state/store.js`:
   `getActiveScale0Bridge`, `getActiveScale0Capability`, `getActiveLatticeSize`,
   `resolveActiveScale0BridgeFromWindow`. The telemetry hub's `collectScale0*`
   helpers already encapsulate mock-vs-main when passed `(ctx.bridge, fluxMock, useFluxMock)`.
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

### 5.1 Scale-Context Readout Gate Contract

`engine/src/scale_context.cpp` (+ `scale_context.h`, results in
`render_bridge_diagnostics.h`) implements the read-only readout admissibility
gate $\mathcal{C}_{\rm scale}$ (`docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md`).

Rules:

1. **Read-only:** `measure_scale_context` / `ScaleContextTracker` take
   `const RenderBridge&` and use only its const accessors; never called from
   `tick()`. The golden hash (`test_render_bridge_golden`, L=17,
   `0xb604d81a3d79366e`) is preserved by construction — scoped to the frozen
   L=17/100-tick/seed-42 configuration with ~14 subsystems toggled off by
   default (see `docs/adr/0012-golden-tick-regression-gate.md`, scoping
   caveat added 2026-07-01; the caveat is now PARTIALLY CLOSED by the
   2026-07-02 multi-profile amendment — nine pinned profiles including
   shipping-defaults, boundary modes, L=9, GPU, and gauge links; canonical
   inventory table in `engine/CHECKLIST_ENGINE.md` §"Pinned golden-profile
   inventory"); it does not certify unqualified "physics."
2. **$\alpha$-blind by contract:** the module must NEVER reference `ALPHA`,
   `ALPHA_EFT`, the Koopman eigenvalue, or `137.036`. Its inputs are lattice
   geometry, $|J|^2$, and the observation-only genesis/evaporation counters only.
3. **Observe-only default:** `ScaleContextConfig::gate_active` defaults to
   `false` (status forced to `DiagnosticOnly`); arming the gate is opt-in so no
   existing campaign is silently blocked. The hard refusal lives downstream in
   `scripts/proofs/proof_alpha_stochastic_koopman.py`.
4. **Thresholds are `[IMPOSED engineering defaults]`**, never theorem values, and
   must stay self-consistent (see `SPEC_SCALE_CONTEXT_READOUT §5.2`).

Reference test: `engine/tests/test_scale_context.cpp` (ctest `scale_context`).

### 5.2 Scale-2 Atomic Closure-Context Contract

Scale-2 now carries two distinct atomic scale channels:

- `Atom.radius` / `vdw_sigma`: simulation interaction scales used by
  Lennard-Jones, bonding, CUDA pair-force buffers, and scale bridges.
- `AtomicClosureContext::r_cloud`: physics-facing shell-context readout used
  for scale interpretation and diagnostics.

Rules:

1. Do not substitute `r_cloud` into `Atom.radius`, `vdw_sigma`, bond capture
   ranges, or CUDA atom buffers without a separate MD retuning and audit.
2. `AtomicClosureContext` is computed from shell bookkeeping and Slater
   shielding; Slater constants are `[IMPOSED]`, and the hydrogenic
   `R_BOHR*n_shell^2/Z_eff` scale is a parametric reference estimate, not an
   FTD derivation of empirical atomic radii.
3. C++ and JS mirrors must stay vocabulary-compatible:
   `compute_atomic_closure_context(...)` / `AtomicProperties::closure_context`
   / `AtomEngine::closure_context_for(...)` correspond to
   `computeAtomicClosureContext(...)` / `computeAtomicProps(...).closure_context`.
4. The expected periodic pattern is structural: within a shell,
   increasing screened return force contracts `r_cloud`; opening a new shell
   resets `r_cloud` outward. Tests pin this behavior without comparing to
   experimental radius tables.

Reference doc: `docs/theory/05_particles/SPEC_ATOMIC_PROPERTY_LEDGER.md`.
Reference test: `engine/tests/test_atom_engine.cpp`.

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
