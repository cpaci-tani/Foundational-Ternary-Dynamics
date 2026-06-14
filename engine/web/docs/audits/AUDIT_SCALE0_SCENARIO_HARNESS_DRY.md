# Audit: Scale-0 scenario physics + telemetry harness DRY (2026-06-13)

**Scope:** All 111 registered Scale-0 scenarios, their load path, shared helpers,
and telemetry/active-bridge resolution across runtime + panels.

**Goal:** One physics write surface (`PhysicsHarness`), one active-owner resolver
(`store.js`), one telemetry collector (`telemetryHub`), shared scenario primitives
(`_helpers.js`).

**Companion:** `AUDIT_SCALE0_CALLSTACK.md` (tick/load/resize active-owner integrity, 2026-06-13).

---

## Load path (canonical)

```
bindings.js (scenario-select)
  → loadScale0Scenario
      → shouldUseFluxMock → makeFluxMock when needed
      → setFluxMock(fluxMock, useFluxMock)   // requires store.js import
      → getPhysicsHarness(activeBridge).load(harness)
      → applyGravityAbsorbingToggles
      → viewport.setLatticeSize(activeN) when N differs
```

**Fixed:** `scenario-loader.js` no longer calls `fluxMock.setupScenario()` before
`scenario.load()` (double-seed bug on mock path).

---

## Physics harness consolidation

| Before | After |
|---|---|
| Inline harness in `index.js` + `makeBridgeHarness` in s0-seed | `createScenarioHarness(bridge)` in `_helpers.js` |
| Direct `harness.bridge._particles[last]` mutations | `harness.injectParticle(..., { vx, spin, color, … })` |
| Duplicate slit/barrier geometry in light + quantum | `injectCoherentSlitPair`, `injectLockedYZPlane`, `injectLockedBarrierWall` |
| 17× repeated emergent-ic toggle blocks in registry | `setupEmergentSpectrumScenario()` + `EMERGENT_IC_TOGGLES` |

**Remaining intentional bridge access:**

- `harness.bridge._quantum*` experiment flags in quantum scenarios (overlay/panel hooks)

**Resolved (2026-06-13 pass 2):**

- Registry custom loads use `harness.*` only — zero `harness.bridge` in `scenario-registry.js`
- `PhysicsHarness` delegates: `setLangevinParams`, `setLangevinTemp`, `setOmega0`, `injectUniformFluxAdd`, `initFluxGrid`
- Named toggle bundles: `DE_BROGLIE_CLOCK_TOGGLES`, `THERMAL_IGNITION_TOGGLES`, `QGP_TOGGLES`, `EW_PHASE_TOGGLES`
- Shared `activateStateFieldOverlay()` for interactive scenario loads
- Vacuum w/z/higgs/neutrino cases use `vox()` / `sig()` (fixed neutrino `const sig = 2` shadowing bug)

**Resolved (2026-06-13 pass 3 — callstack / controls):**

- `PhysicsHarness`: `getParam`, `setParam`, `clearField`, `seedRandomFlux`, `tickScale0`
- `wire.js`: `dualHarness`, `getActiveScale0Bridge` for K_B; harness clear/seed (no raw `.bridge`)
- `genesis-burst-panel.js`: harness-only fire path (`tickScale0`, not `bridge.tick()`)
- `scenario-loader.js`: `runScale0PhysicsTicks` shared with `tick.js`; required `setFluxMock` / `setForceStyle` imports

---

## Active physics owner (telemetry reads)

When `state.useFluxMock === true`, the mock/worker owns live physics. Sampling
`ctx.bridge` shows stale data.

**Canonical API** (`state/store.js`):

- `getActiveScale0Bridge(ctx, state)`
- `getActiveScale0Capability(ctx, state)`
- `resolveActiveScale0BridgeFromWindow()` — panels + lazy init

**Wired (this pass):**

- `runtime/frame-sync.js`
- `runtime/field-overlays.js` (`emActiveScale0`)
- Panels: conservation, p1-observables, time, thermo, spectrum, gravity, flux-slice

**Telemetry collection** stays on `telemetryHub.collectScale0*(ctx.bridge, fluxMock, useFluxMock)` —
the hub already encapsulates mock vs main; no change needed in `diagnostics.js`.

---

## Toggle policy DRY

**Declarative (preferred):** `SCALE0_SCENARIO_OVERRIDES` in `config/toggles.js`

**Registry custom loads:** use `harness.setToggle` / `harness.setupScenario`, not
`const bridge = harness.bridge || harness`. Emergent/cluster-law family uses
`setupEmergentSpectrumScenario`.

**Still duplicated (low priority):**

- Per-scenario `genesis: false` in light/quantum bodies where overrides also exist
- de-broglie-clock / thermal-ignition bespoke toggle maps (unique, keep local)

---

## Fixed-voxel physics

Localized geometry uses `ctx.vox()` / `ctx.sigma()` / `ctx.band()` from
`physics-lattice.js` — **fixed voxel counts**, not fractional box fill.
See scenarios README §Fixed-voxel physics vs visual scaling.

---

## Verification

Run after harness changes:

```bash
cd engine/web && npx playwright test tests/scenario-parity.spec.js
cd engine/web && npx playwright test tests/scale0-scenario-telemetry-contract.spec.js
```

---

## Open follow-ups

1. Move duplicated `genesis: false` in light/quantum bodies to `SCALE0_SCENARIO_OVERRIDES` where not already declared
2. Quantum experiment flags (`_quantumBarrierWidth`, etc.) — optional harness setters
3. C++ parity check for `s0-seed-thermal-ignition` JS stub vs `s0_seed.cpp`
