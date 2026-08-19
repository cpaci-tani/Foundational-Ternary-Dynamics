# SPEC — Scale 0 Runtime Pipeline Architecture

**Status:** foundation reference (descriptive — documents the system as built).
**Scope:** the **per-frame runtime** of Scale 0 — the `animate()` pipeline (tick → upload → overlays
→ render → diagnostics), the tick model, the render-upload cadence, the amortized field-overlay
scheduler, the diagnostics/telemetry path, the shared rAF coordinator, and the forward-only time model.
**Companions:** [`SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](SPEC_SCALE0_SCENARIO_ARCHITECTURE.md)
(what seeds the lattice), [`SPEC_SCALE0_BRIDGE_ARCHITECTURE.md`](SPEC_SCALE0_BRIDGE_ARCHITECTURE.md)
(what the loop ticks/reads). Perf history: [`AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md`](audits/AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md),
[`AUDIT_SCALE0_CALLSTACK.md`](audits/AUDIT_SCALE0_CALLSTACK.md) (active-owner routing).

**Path convention:** JS paths relative to `engine/web/js/`; `scale0/` = `scales/scale0/`. Every claim
carries a `file:line`; re-derive against source before relying on line numbers.

---

## 1. The 30-second model — two loops

Scale 0 runs on **two independent rAF loops**, both reading live state so either sees the other's
mutations immediately:

1. **The main `animate()` loop** (`app.js`) — one `requestAnimationFrame` drives physics + render for
   the active scale. For Scale 0 it calls `Scale0Controller.animate(ctx)`
   (`scale0/controller.js:324-334`), a fixed 5-stage pipeline (§2).
2. **The `rafCoordinator`** (`lib/raf-coordinator.js`) — one shared rAF that drives the floating
   **overlay panels** (flux-slice, P1-observables, conservation, spectrum) at their own low Hz, with
   tab-hidden pause and self-healing (§7). It exists so 5+ panels don't each spin their own rAF.

The pipeline is **dirty-flag + throttle driven**: nothing recomputes unless a tick advanced or the
user changed something, and each stage has its own cadence so large lattices stay interactive.

**Time model (§8):** the engine tick is the *only* sim clock; the `animate()` rAF is the observer's
external, uncapped (native-refresh) render loop. The two are independent — pausing freezes recorded
sim time while the render loop keeps flowing. The simulation is **forward-only** (no reverse/scrub).

---

## 2. The per-frame pipeline

`Scale0Controller.animate(ctx)` (`controller.js:219-229`) runs five stages in strict order, then
refreshes the play bar:

```
animate(ctx)                                   controller.js:219
 ├─ advanceSimulation(ctx, state)              runtime/tick.js:1        — tick physics (or read worker)
 ├─ syncRenderableData(ctx, state, va)         runtime/frame-sync.js:3  — upload lattice → GPU
 ├─ updateFieldOverlays(ctx, state, va)        runtime/field-overlays.js — amortized overlay sweep
 ├─ renderFrame(ctx)                           controller.js:71        — advance clock + viewport.render()
 ├─ updateDiagnosticsAndPanels(ctx, state)     runtime/diagnostics.js:5 — telemetry (every 3rd frame)
 └─ _playBar?.refresh()                         controller.js:225       — forward "T N" tick readout
```

`va` is the **memoized viewport adapter** (`controller.js:60-69`): `animate()` calls
`viewportAdapter(ctx)` three times a frame, and the adapter is a ~50-closure object, so it is rebuilt
only when `ctx.viewport` identity changes (a scale switch), not per call.

`renderFrame` (`controller.js:71-82`) advances the viewport's wall-clock animation timer **only when
`ctx.running`** — so slow-pulse visuals (|ψ|² breathing) stay static during pause even when an overlay
toggle forces a repaint — then calls `viewportAdapter.render()`.

---

## 3. The tick model (`runtime/tick.js`)

`advanceSimulation(ctx, state)` decides how much physics to run this frame.

- **Worker path first** (`:4-18`): if `state.fluxMock` is a worker proxy and owns the scenario, the
  worker self-ticks on its own clock. The main thread only forwards run-state
  (`fm.setRunning(ctx.running)`), and when the worker's `frameCounter` advances it marks
  `latticeNeedsUpload` and bumps `fieldDataVersion` — then returns.
- **Gate** (`:21`): `!ctx.running` (global pause) returns immediately — no tick, no upload, no mock
  tick. Pause is the *only* thing that stops the sim clock; there is no scrub/reverse gate (§8).
- **Accumulator** (`:22`): `ctx.ticksPerFrame` is a *fractional* slider value; `state.tickAccumulator`
  integrates it into `wholeTicks` across frames. (`ticksPerFrame` is the speed control — the *rate* of
  recording, not a second clock — and is Scale 0 only; scales 1/2/4/5 keep the global slider.)
- **Throttle** (`:23-24`): `maxTicksPerFrame` is `1` for L>48, `2` for L>32, else `wholeTicks` — bigger
  lattices run fewer ticks per frame to hold the frame budget. `ticksToRun = min(wholeTicks, max)`.
- **Dual-bridge tick** (`tick.js` `runScale0PhysicsTicks`): tick the WASM/main bridge **unless**
  `state.useFluxMock`; tick the mock **only when it owns physics**. Worker-backed mocks self-tick;
  the main thread forwards `setRunning` and bumps `fieldDataVersion` from `frameCounter`. Shared by
  `advanceSimulation` and `stepScale0`.
  (Enabling a *derived overlay* no longer starts the mock ticking — overlays sample whatever state the
  mock is in.)
- **Dirty signals** (`:54-57`): `latticeNeedsUpload = true` every frame past the gate; `fieldDataVersion`
  (monotonic, never cleared) increments only when `ticksToRun > 0`. The two are deliberately different:
  `latticeNeedsUpload` is a one-shot consumed by frame-sync on a finer cadence, so the overlay scheduler
  can't use it as a "did the field change?" signal — it latches `fieldDataVersion` instead (§5).

---

## 4. The render-upload path (`runtime/frame-sync.js`)

`syncRenderableData` uploads lattice/particle/flux data to the GPU, throttled:

- **Cadence** (`:5-6`): `volUpdateInterval` = 6 (L>96) / 4 (L>64) / 3 (L>48) / 1 — and the whole
  function early-returns unless `state.latticeNeedsUpload && ctx.frameCount % volUpdateInterval === 0`.
  So at L=32 it uploads every frame; at L=97 every 6th.
- **Active-bridge selector** (`frame-sync.js`, `field-overlays.js`, panels): use
  `getActiveScale0Bridge` / `getActiveScale0Capability` / `getActiveLatticeSize` from
  `scale0/state/store.js` — not ad-hoc `(useFluxMock ? fluxMock : bridge)` ternaries. Sampling the
  wrong owner shows stale/frozen data (the same bug class the flux-slice panel once hit).
- **Uploads** (`:18-41`): the particle frame always; confinement strings, the flux **volume**, and the
  flux **slice** (every enabled axis of yz/xz/xy, packed into one update) when their overlays are on.
- **Clear** (`:43`): `latticeNeedsUpload = false` after a successful upload (one-shot).

This stage is also where the large-L upload optimizations live (the flux-volume scan
decimated to the drawn-point step; the adaptive flux-dot budget) — see
[`AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md`](audits/AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md) §4.

---

## 5. The amortized field-overlay scheduler (`runtime/field-overlays.js`)

The most sophisticated stage. Computing all enabled overlays in one frame would spike; instead the
scheduler **spreads the work across frames under a fixed budget**, allocation-free.

- **Budget** (`:538`): `OVERLAY_FRAME_BUDGET = 100` cost units/frame. Weights (`:550-554`):
  `COST_STREAMLINE = 50` (E/B/flux/each force-flow — the dominant cost: spatial index + bidirectional
  RK4 over ~300 lines), `COST_FORCE_FIELD = 25`, `COST_DERIVED = 20`, `COST_SCALAR = 12`,
  `COST_PASSTHROUGH = 4`. The budget is sized so **one** streamline fits per frame but a second defers —
  that is the whole point. (`COST_STREAMLINE` was lowered 100→50 so E and B land on the *same* frame and
  read the *same* snapshotted particle positions, fixing the "B field shifts/translates" offset bug.)
- **First-job guarantee** + **lag ceiling** (`:555-565`): the loop always runs at least the first
  remaining job each frame (so an N-job sweep finishes in ≤ N frames on its own); `OVERLAY_SWEEP_MAX_FRAMES
  = 30` is a safety-valve drain, not the primary spread.
- **Skip-unchanged** (`:528-531`): a fresh sweep starts only when the data changed since the last sweep
  finished — the scheduler latches `fieldDataVersion` (`sched.lastVersion`) and re-sweeps only when it
  moved (or an explicit dirty arrived). Static field between throttle boundaries → zero overlay CPU.
- **Allocation-free job pool** (`:567-676`): the persistent `sched` (`ensureOverlaySched`, `:631-662`)
  holds a reused `jobs[]` pool (`jobSlot` grows it once to a high-water mark, `:669-676`), an integer
  `kind` per slot dispatched by a single module-level `runJob` (no per-job closures), and the per-sweep
  context (ctx/state/adapter/latticeSize/params/capabilities) stashed once. Job kinds: `JOB_EFIELD`,
  `JOB_BFIELD`, `JOB_FLUX`, `JOB_PASS`, `JOB_FORCE_FIELDS`, `JOB_FORCE_FLOW`, `JOB_DERIVED`, `JOB_SCALAR`
  (`:586-593`).
- **Coherence snapshot** (`sched.sampled`): the field is sampled **once** at sweep start so every job in
  the sweep sees the same tick's data (and the same particle positions) — preventing inter-overlay drift.
- **The scalar dispatch table** (`SCALAR_JOBS`, `:602-622`): the static, allocated-once `[flag, computeFn,
  applyFn]` table for the **14 Tier-1/2/3 scalar topology overlays** (`showPsiSquared`, `showPhase`,
  `showLagrangianDensity`, `showEntropyDensity`, `showGravPotential`, `showEmEnergy`, `showChargeDensity`,
  `showVorticity`, `showHorizon`, `showEPressure`, `showBPressure`,
  `showStateField`, `showLatency`, `showGaussResidual`).
  Splitting compute/apply lets a scalar job run with zero per-run allocation. The 4 force-flow types live
  in `FLOW_TYPES` (`:626-629`).

The canonical list of all overlay flags is `state/store.js` `FIELD_TOGGLE_KEYS` (36) — streamlines (E/B/
flux), the 4 forces, the passthroughs, the derived group, and the 19 scalars above partition it.

### 5.1 Overlay line appearance (visual, not physics)

Streamlines and force-flow lines use WebGL `LineBasicMaterial` (~1 CSS pixel thick; `linewidth` is
ignored in browsers). Integrator **stride** grows with lattice N (`streamline-integrator.js`) so large
L trades curve fidelity for CPU budget. The amortized scheduler spreads heavy streamline jobs across
frames (`OVERLAY_FRAME_BUDGET = 100`, `COST_STREAMLINE = 50`). Paused sims freeze streamline seeds
(skip-unchanged gate). Viewport **must** stay lattice-aligned with the active bridge
(`scenario-loader.js` calls `viewport.setLatticeSize` on load/resize); `_onResize` refreshes
`devicePixelRatio` to avoid HiDPI blur. See
[`audits/AUDIT_SCALE0_CALLSTACK.md`](audits/AUDIT_SCALE0_CALLSTACK.md) §Overlay line visual quality.

---

## 6. Diagnostics & telemetry (`runtime/diagnostics.js`)

`updateDiagnosticsAndPanels` runs every **3rd** frame (`:6`, ~30 Hz at 60 fps). It routes **all** bridge
reads through the **telemetry hub** — the single source of truth (`telemetry-hub.js`): `collectScale0`
(`:9`) plus `collectScale0Audit` / `collectScale0Lagrangian` (`:13-14`) push into 500-sample ring
buffers that back the panel sparklines. The hub picks the right bridge (prefers the mock when WASM has
no particles but the mock has flux). It then updates the status bar (physical time, particle count,
energy, Running/Idle, `:30-40`) and the active tab's panel (`diagnostics`/`charts`/`lagrangian`/
`inspector`/`hierarchy`, `:19-57`) — only the visible one.

---

## 7. The rAF coordinator (`lib/raf-coordinator.js`)

A single shared rAF for the overlay **panels**, separate from the main `animate()` loop. Panels
`subscribe(id, {hz, cb})` (`:59-84`) at their own rate (2–4 Hz typically); the coordinator schedules
each by `nextDueAt` (additive, clamped to `now` so a slow callback can't permanently lag, `:115`). Key
behaviors:

- **Tab-hidden pause** (`:109`): when `document.hidden`, subscribers slower than `VISIBILITY_PAUSE_THRESHOLD_HZ`
  (30) pause; ≥30 Hz keep ticking (so the 3D viewport doesn't freeze). Due-times reset on resume so high-Hz
  subscribers don't burst (`:41-46`).
- **Self-healing** (`:119-131`): a callback that throws is caught + warned; after `ERROR_BUDGET` (10)
  consecutive throws the subscriber is auto-unsubscribed so one bad panel can't pin the loop.
- **Introspection**: `.size()` (`:138`, the lifecycle-harness leak proxy via `window.__ftdRAF`) and
  `.clear()` (`:147`, HMR/test teardown).

The two loops relate cleanly: the main loop ticks physics and refreshes telemetry every 3rd frame; the
panels poll the *same* live hub/state at their own low Hz via the coordinator — no duplicated 60 fps work.

---

## 8. Time model — forward-only, single source

The **engine tick is the only clock the simulation has.** Sim state advances *only* when
`advanceSimulation` runs a tick (§3); nothing in the lattice moves between ticks. There is exactly
**one** tick source — `bridge._tick`, surfaced through `getScale0Diagnostics().tick`. (There is no
`globalTick` render-frame counter; the render loop carries no second
clock, and there is no dual `t=… (g=…)` readout.)

The **render loop is the observer's external time** — an uncapped `requestAnimationFrame`
(`app.js:695-696`, scheduled unconditionally) that runs at the monitor's native refresh, independent
of the sim clock. This is the ontological split the model makes explicit: time never stops in the
world, it is only *recorded*. Pausing the sim (`ctx.running = false`, §3) freezes recorded sim time
while the observer keeps experiencing always-flowing real time — the rAF still fires, the camera still
moves, the scene still repaints. Wall-clock visuals that should look "frozen while paused" (the
slow-pulse |ψ|² breathing) are deliberately gated to `ctx.running` inside `renderFrame` (§2) so they
hold, even though the loop around them keeps running.

**Speed is the rate of recording, not a second clock.** `ticksPerFrame` (§3) sets how many engine
ticks the observer records per rendered frame (fractional, accumulated across frames). It changes how
fast recorded time advances relative to observer time; it does not introduce a second timeline. Speed
control is **Scale 0 only** — scales 1/2/4/5 keep the global toolbar slider.

The simulation is **forward-only**: there is no reverse, rewind, or scrub-back timeline. The play bar
(`ui/components/play-bar/`) exposes play/pause, single-step, reset, speed, and the forward "T N" tick
readout — nothing that moves the clock backward. (There is no snapshot/scrub-back recorder —
`MemoryRecorder` + `timeline/` + the `setScale0*Buffer`/`getScale0*Buffer` bridge hooks are absent:
recording every tick to a memory buffer does not fit the perf budget at large L, and a forward-only
substrate is the cleaner ontological match to "time is only recorded.")

---

## 9. State & dirty flags (`scale0/state/store.js`)

The Scale-0 runtime is coordinated by one state object (`store.js:66-87`) and its dirty flags:

| Flag | Set by | Cleared by | Meaning |
|---|---|---|---|
| `latticeNeedsUpload` | every tick (`tick.js:54`) | frame-sync after upload (`:43`) | GPU upload pending |
| `fieldDataVersion` | every actual tick (`tick.js:56`) | never (monotonic) | overlay scheduler's "did the field change?" latch |
| `fieldNeedsUpdate` | toggle / force-style / scenario load (`store.js` setters) | overlay sweep | explicit overlay dirty (preempts a half-finished sweep) |
| `anyFieldActive` | recomputed on any field toggle | — | short-circuits the overlay stage when nothing's on |
| `useFluxMock` / `fluxMock` | scenario load (`scenario-loader.js`) | scenario switch / exit | which bridge the loop ticks + reads |

**Active-owner helpers** (export from `store.js`; use instead of manual mock ternaries):

- `getActiveScale0Bridge(ctx, state)`
- `getActiveScale0Capability(ctx, state)`
- `getActiveLatticeSize(ctx, state)`
- `resolveActiveScale0BridgeFromWindow()` — panels without a `ctx` closure

---

## 10. Quick reference — files

| Stage | File |
|---|---|
| Orchestrator | `scale0/controller.js` (`animate`, `renderFrame`, memoized `viewportAdapter`) |
| Tick | `scale0/runtime/tick.js` |
| Upload | `scale0/runtime/frame-sync.js` |
| Overlays | `scale0/runtime/field-overlays.js` (scheduler `:533-676`, `SCALAR_JOBS` `:602-622`) |
| Diagnostics | `scale0/runtime/diagnostics.js` + `telemetry-hub.js` |
| Shared panel loop | `lib/raf-coordinator.js` |
| Play bar | `ui/components/play-bar/` (transport + speed + step + forward "T N" readout) |
| State | `scale0/state/store.js` |
| Main loop dispatch | `app.js` (`animate` → per-scale controller) |

*Re-derive all `file:line` references against source before relying on line numbers.*
