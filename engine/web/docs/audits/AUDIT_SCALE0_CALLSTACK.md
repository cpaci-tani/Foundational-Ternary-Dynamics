# Scale-0 Callstack Audit — Physics Owner Integrity (2026-06-13)

**Scope:** End-to-end Scale-0 runtime path — scenario load, lattice resize, tick
advance, render sync, overlay sampling, telemetry, and user controls — verified
against the physics contract: **one active owner per frame**, harness-only writes,
mock-preferred reads when `useFluxMock`.

**Companion:** `AUDIT_SCALE0_SCENARIO_HARNESS_DRY.md` (scenario DRY + harness consolidation).

---

## Physics contract (must hold)

| Rule | Meaning |
|------|---------|
| **P1 — Single owner** | When `state.useFluxMock && state.fluxMock`, only the mock/worker advances physics; WASM main thread is not ticked. |
| **P2 — Active N** | Lattice dimension for clamping, sampling, and upload comes from `getActiveLatticeSize(ctx, state)`, not `ctx.bridge.latticeSize` when mock owns physics. |
| **P3 — Harness writes** | Scenario load, injection, toggles (post-load), genesis burst, and substrate controls go through `getPhysicsHarness(activeBridge)` — not raw `bridge.tick()` / `bridge.setupScenario()`. |
| **P4 — Dual mirror** | User toggles/sliders after load mirror to **both** WASM + mock (`dualHarness` / checkbox handlers) so load-time parity is preserved at runtime. |
| **P5 — Visual-only overlays** | `FIELD_TOGGLE_KEYS` in `store.js` never change physics toggles; overlays read sampled state from the active owner. |
| **P6 — Telemetry hub** | `telemetryHub.collectScale0*(ctx.bridge, fluxMock, useFluxMock)` — hub already prefers mock when active; callers pass all three. |

---

## Canonical callstack

### Load scenario

```
bindings.js (scenario-select change)
  → loadScale0Scenario(ctx, state, viewportAdapter, scenarioId)
      → shouldUseFluxMock(ctx.bridge, scenarioId)
      → makeFluxMock(N, …) when mock path
      → setFluxMock(fluxMock, useFluxMock); sync ctx.useFluxMock / ctx.fluxMock
      → getPhysicsHarness(activeBridge).load(harness)   // scenario-registry
      → applyGravityAbsorbingToggles (WASM + mock caps)
      → syncComboSliders reads active bridge params
      → viewport.setLatticeSize(activeN) when N differs from viewport
```

**Fixed (2026-06-13):** removed pre-load `fluxMock.setupScenario()` double-seed.

**Fixed (2026-06-13 pass 2):** `setFluxMock` / `setForceStyle` imports restored in
`scenario-loader.js` (init crash). Viewport lattice synced on load so streamlines,
wireframe, and clip bounds match the active bridge N.

### Lattice resize

```
wire.js lattice-size change
  → resizeScale0Lattice(ctx, state, viewportAdapter, newSize)
      → skip bridge.resize() when ownerIsMock
      → rebuild fluxMock at newSize BEFORE scenario.load (not empty mock)
      → reload scenario on activeBridge harness (same as load path)
      → viewport.setLatticeSize(newSize)
```

**Fixed (2026-06-13):** resize no longer leaves mock-owned scenarios ticking an empty unseeded mock.

### Per-frame tick (Scale-0 controller)

```
Scale0Controller.animateFrame
  → advanceSimulation(ctx, state)          // tick.js
      → worker path: fm.setRunning(ctx.running); bump fieldDataVersion from frameCounter
      → in-thread: runScale0PhysicsTicks(ctx, state, ticksToRun)
          → exclusive: !useFluxMock → mainScale0.tickScale0()
                     useFluxMock → mockScale0.tickScale0()
  → syncRenderableData(ctx, state)          // frame-sync.js — active capability
  → updateFieldOverlays(ctx, state)          // field-overlays.js — active capability + N
  → updateDiagnosticsAndPanels(ctx, state)   // telemetryHub
```

**Fixed (2026-06-13):** `stepScale0` and `advanceSimulation` share `runScale0PhysicsTicks`; mock guard no longer skips `fieldDataVersion` bump.

### Active-owner API (`state/store.js`)

```javascript
getActiveScale0Bridge(ctx, state)      // mock if useFluxMock else ctx.bridge
getActiveScale0Capability(ctx, state)  // .capabilities.scale0 on active owner
getActiveLatticeSize(ctx, state)       // activeBridge.latticeSize
resolveActiveScale0BridgeFromWindow()  // panels without ctx closure
```

---

## Violation checklist (post-fix)

| ID | Issue | Status |
|----|-------|--------|
| H1 | Resize builds empty mock, physics stalls | **FIXED** — `resizeScale0Lattice` mirrors load |
| H2 | `genesis-burst-panel` calls `bridge.tick()` / bypasses harness | **FIXED** — `harness.tickScale0()`, `harness.setupScenario('empty')` |
| H3 | Manual `(useFluxMock ? fluxMock : bridge)` in overlays | **FIXED** — `field-overlays.js`, `overlay-frames.js`, `controller.js` playbar |
| H4 | `stepScale0` tick path diverged from `advanceSimulation` | **FIXED** — shared `runScale0PhysicsTicks` |
| H5 | `syncComboSliders` read WASM params while mock active | **FIXED** — active bridge in `scenario-loader.js` |
| H6 | Injection K_B from `getFluxMock() \|\| bridge` (wrong when mock inactive but exists) | **FIXED** — `getActiveScale0Bridge` in `wire.js` |
| H7 | `wire.js` clear/seed hit `h.bridge` directly | **FIXED** — `PhysicsHarness.clearField` / `seedRandomFlux` |
| H8 | `frame-sync` / panels used stale WASM lattice N | **FIXED** — `getActiveLatticeSize` |
| H9 | Scenario load left viewport N stale vs active mock | **FIXED** — `viewport.setLatticeSize(activeN)` at end of load |
| H10 | HiDPI canvas blur after panel/window resize | **FIXED** — `viewport._onResize` re-applies `setPixelRatio` |

### Init regressions caught in QA (same arc)

| Symptom | Cause | Fix |
|---------|-------|-----|
| `setFluxMock is not defined` | Missing import from `store.js` | Added to `scenario-loader.js` imports |
| `setForceStyle is not defined` | Missing import from `store.js` | Added to `scenario-loader.js` imports |
| `Unexpected token 'else'` in `wire.js` | Invalid `h.clearField?.(); else` | Proper `if/else` with harness methods |

### Intentional dual-bridge writes (not violations)

- **Physics toggle checkboxes** (`wire.js`): update WASM + mock caps — required by P4.
- **`applyToggleDefaults` / gravity toggles at load**: write both owners once at scenario boundary.
- **`dualHarness` / `dualInject`**: mirror user actions to both bridges after load.
- **`loadScale0Scenario` line `ctx.bridge.latticeSize`**: seeds mock at UI-configured N before mock exists — correct for construction.

### Remaining low-priority notes

- **`shouldUseFluxMock`**: heuristic from scenario id prefix + WASM volume probe; an explicit allowlist would be clearer but is not a physics bug.
- **`harness.bridge._quantum*` flags** in quantum scenarios: overlay experiment hooks — documented in harness DRY audit.
- **Worker telemetry mask** (`diagnostics.js` `fm.setTelemetryMask`): only applies when mock is worker-backed; OK.

---

## PhysicsHarness surface (canonical writes)

Delegated on `physics-harness.js` (scenario + controls should use these, not `.bridge`):

| Method | Purpose |
|--------|---------|
| `setupScenario`, `setToggle`, `injectFlux`, `injectParticle`, … | Scenario orchestration |
| `setLangevinParams`, `setLangevinTemp`, `setOmega0` | Thermal / clock params |
| `getParam`, `setParam` | Combo sliders + injection calibration |
| `clearField`, `seedRandomFlux` | Substrate actions |
| `tickScale0` | Deterministic burst panels (genesis N(A)) |
| `getLatticeSize`, `getDiagnostics` | Readbacks |

---

## Files wired to active owner (this pass)

| Module | Change |
|--------|--------|
| `runtime/tick.js` | `runScale0PhysicsTicks` — single tick entry |
| `runtime/scenario-loader.js` | resize fix, `syncComboSliders`, `applyGravityAbsorbingToggles`, `stepScale0`, load viewport sync |
| `viewport.js` | `_onResize` refreshes `devicePixelRatio` before `setSize` |
| `runtime/frame-sync.js` | `getActiveScale0Capability`, `getActiveLatticeSize` |
| `runtime/field-overlays.js` | `getActiveScale0Bridge` for sampling |
| `runtime/overlay-frames.js` | grav potential + scalar topology frames |
| `ui/controls/wire.js` | `latticeN`, `dualHarness`, active K_B, harness clear/seed |
| `ui/overlays/genesis-burst-panel.js` | harness-only fire path |
| `controller.js` | playbar `getNowTick` via active capability |
| `physics/physics-harness.js` | `getParam`, `setParam`, `clearField`, `seedRandomFlux`, `tickScale0` |

Panels already on `resolveActiveScale0BridgeFromWindow` / active helpers (prior pass): conservation, p1-observables, time, thermo, spectrum, gravity, flux-slice.

---

## Verification

Manual smoke:

1. Load `flux-pulse` → switch lattice 33 → 65 → 33 — field should persist/reload, not go blank.
2. Load `s0-seed-cluster-law` → Fire on genesis panel — N(A) points should update (mock ticks).
3. Toggle physics term while mock scenario running — both DOM and mock state should change.

Automated:

```bash
cd engine/web/tests && npx playwright test scale0-resize-guard.spec.js
```

(Allow ≥60 s timeout if WASM init is slow on the host.)

---

## Overlay line visual quality (not a physics regression)

Users may report streamlines / wireframe / force lines looking “lower quality” after
the active-owner refactor. No line materials or shaders were changed in this arc.
Common causes:

| Factor | Effect |
|--------|--------|
| **WebGL `LineBasicMaterial`** | Browser renders ~1 CSS px lines regardless of `linewidth`; zoom-in looks aliased |
| **Streamline stride** (`streamline-integrator.js`) | `stride ≈ round(N/16)` (clamped 2–8) — larger L → coarser field sampling → chunkier curves |
| **Amortized overlay sweep** | E/B/flux/force streamlines rebuild across multiple frames under `OVERLAY_FRAME_BUDGET`; mid-sweep can look sparse |
| **Skip-unchanged gate** | Paused sim freezes streamline seeds (no re-randomization jitter) — static, not blurry |
| **Viewport N mismatch** | Fixed by H9 — misaligned clip/center made lines look cut off or sparse |
| **Canvas DPR on resize** | Fixed by H10 — stale backing store blurred the whole viewport including lines |

**Native WS probe:** `ws://127.0.0.1:9100` connection failure on boot is expected when
`ws_server.exe` is not running; the dashboard falls back to WASM/Mock and is not a line-quality issue.

---

## Verdict

**Scale-0 callstack follows physics** after this pass: exclusive mock/main ticking, consistent active-owner reads for N and sampling, harness-only scenario/control writes, and resize/load parity on the mock path. No physics toggles were changed; routing and delegation fixes only.
