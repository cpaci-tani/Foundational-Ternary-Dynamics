# SPEC — Scale-0 Performance: Telemetry Demand-Gating, Panel Rendering, and Config Hardening

**Status:** `[PARTIALLY IMPLEMENTED — 2026-06-05]` — Phase 1 + Phase 2 shipped, verified, and **on by
default**; Phase 3 partial (G done); Phase 3-F, the §6.2/§6.3 render items, and Phase 4-N deferred (see
§9 status).  ·  **Date:** 2026-06-05  ·  **Scope:** `engine/web` Scale-0
consumption-side per-frame cost (telemetry collection, sidepanel rendering, overlay sampling, GC
hygiene, worker-path config). The **producer** side (the wave tick + worker offload) is covered by
[`SPEC_SCALE0_LATTICE_PERF.md`](SPEC_SCALE0_LATTICE_PERF.md) and is already shipped — this spec is the
*next* round: the cost the new live sidepanels (energy-audit + Lagrangian, 2026-06-05) added on the
**consumption** side.

**Companions:** [`SPEC_SCALE0_RUNTIME_PIPELINE.md`](SPEC_SCALE0_RUNTIME_PIPELINE.md) (the per-frame
`animate()` pipeline), [`SPEC_SCALE0_BRIDGE_ARCHITECTURE.md`](SPEC_SCALE0_BRIDGE_ARCHITECTURE.md)
(worker/shadow), [`SPEC_SCALE0_LATTICE_PERF.md`](SPEC_SCALE0_LATTICE_PERF.md) (tick/worker offload).

**Provenance:** grounded in a 5-engineer source-verified redteam (2026-06-05). All `file:line` are as of
2026-06-05 source; re-derive before relying on line numbers.

---

## 1. Problem & evidence

**Symptom:** choppy sim *playback* — low/falling FPS while the simulation is running, worse at large
lattice L (L≈97–129). It appeared after the 2026-06-05 work that wired energy-audit + Lagrangian
telemetry and made "all sidepanels live" (commits `9cfd63f0`, `b319fd90`, `a3a79c2d`).

**Root cause (one sentence):** telemetry collection is **decoupled from consumption** — the expensive
audit/Lagrangian streams are computed unconditionally (every worker tick, and every 3rd render frame)
regardless of whether any panel consumes them, and a floated Telemetry Grid re-renders at 60 Hz even
while collapsed. The heavy *producer* offload is already correct; the regression is on the consumption side.

### 1.1 The three real causes (configuration-dependent)

| # | Cause | Bites on | Evidence (`file:line`) |
|---|-------|----------|------------------------|
| **C1** | The **worker** recomputes `getScale0EnergyAudit()` (a full O(N³) curl/Poynting/div pass) + `getScale0Lagrangian()` **every tick, unconditionally**, stretching its `setTimeout` budget so the sim-advance rate (`FRAME` counter) drops at L≥97. Panels need ≤4 Hz; the worker computes at ~60 Hz. | **Default (worker) path** (`flux-*` scenarios, COOP/COEP on) | `mock-bridge.worker.js:39,48,49`; cadence `:78`; audit pass `mock-diagnostics.js:110-188` |
| **C2** | `collectScale0Audit` + `collectScale0Lagrangian` fire **every 3rd render frame even when `activeTab='controls'`** (the default tab) and no panel consumes them. On the in-thread/WASM paths this is a main-thread O(N³) (in-thread `MockBridge`) or an **uncached** native call (WASM). | **In-thread path** (Safari/no-COI) and **WASM-scenario path** (`s0-*`/`light-*`/`quantum-*`) | `diagnostics.js:11-14`; default `activeTab` `app.js:86`; the consumer-gate existed through 2026-05-26 and was lifted by commit `9cfd63f0` |
| **C3** | A **floated** Telemetry Grid renders 23 sparklines at **60 Hz** (no `%3` skip) and **keeps rendering while collapsed** — ~1,380 uPlot full-repaints/sec + 46 fresh typed-array allocs/update + 23 `querySelector`s/update, all invisible to the user. | **Any path, when the grid is floated** | `app.js:722` + `_shouldAppUpdatePanel` `app.js:745-752`; gate `telemetry-grid/component.js:185` misses `is-collapsed`; collapse is CSS-only `floating-window/component.js:101-110,222-224`; allocs `:212-213`; querySelector `:225` |

### 1.2 The silent-fallback trap (C4)

If the **deploy host** omits COOP/COEP, `crossOriginIsolated` is false → `workerEligible` fails
(`scenario-loader.js:90-94`) → `makeFluxMock` returns an **in-thread `MockBridge`** (`:99`) and the entire
O(N³) tick runs on the render thread — the worst-case choppy path, **invisible** to anyone who doesn't
check. `serve.py:57-59` sends the headers in dev; production may not.

### 1.3 Corrections to the existing SPECs (the redteam found two stale claims)

- `SPEC_SCALE0_BRIDGE_ARCHITECTURE.md §5` — "the field terms of audit/Lagrangian read live off the
  shadow" is **stale**. Commit `b319fd90` moved them into the worker; the proxy serves worker-computed
  scalars (`mock-bridge-proxy.js:121-122`), main-thread cost ~0. (So the audit/Lagrangian are *not*
  computed on the render thread on the default path — C1, not a main-thread sweep, is the default-path cause.)
- `SPEC_SCALE0_LATTICE_PERF.md §1` — the profile table (tick 12/38/89 ms; diagnostics 11/33 ms) was
  measured **2026-06-03, in-thread, 14 overlays**, *before* this regression and *before* the worker became
  the default path. It does not bound the worker-mode, all-panels-live, 19-scalar case users actually run.
  **→ Phase 0 re-measures it.**

### 1.4 What is already good (do NOT touch — confirmed solid by the redteam)

The streamline integrator (`fieldlines.js` — allocation-free RK4 + CSR spatial index + pooled results),
the amortized overlay **job-drain** scheduler (`field-overlays.js:533-1049` — allocation-free job pool,
integer-`kind` dispatch, one-streamline-per-frame budget, `fieldDataVersion` skip-unchanged), the
flux-volume upload (`flux-renderer.js` — cap-bounded fractional-stride decimation, in-place colormap),
the per-tick energy cache (`mock-diagnostics.js:69-95`), the SAB shadow design, the rAF coordinator, and
the memoized viewport adapter are all genuinely well-optimized. This spec changes **none** of them.

---

## 2. Goals / non-goals

**Goals**
- G1. Restore interactive playback FPS (>30) at L≥97 on the default worker path while a sim runs.
- G2. Eliminate per-frame work for telemetry/panels that **no visible consumer** needs.
- G3. Keep every panel, overlay, scenario, and telemetry value — and make several **more** correct
  (the wrong-bridge panels, the freeze-when-floated charts).
- G4. Zero change to any displayed value when a consumer is open (parity-verified).
- G5. Make the silent in-thread fallback (C4) detectable.

**Non-goals**
- N1. Re-doing the producer-side optimizations (sparse tick, worker offload) — done, see LATTICE_PERF.
- N2. Speeding up the WASM/C++ engine compute. WebGPU compute. Native `ws_server`.
- N3. Removing or restructuring any panel/feature. This is gate / cache / relocate / amortize only.

---

## 3. The unifying principle

**Make the *expensive* telemetry streams demand-driven and version-gated; keep the *cheap* primary
telemetry always-on.**

- **Demand-driven:** compute `audit`/`lagrangian` only when a consumer of them is **mounted and visible**
  (`{diagnostics, charts, lagrangian, telemetry-grid}` for audit; `{charts, lagrangian, telemetry-grid}`
  for Lagrangian). On the worker path, forward that want-mask so the worker stops computing them 60×/s.
- **Version-gated:** skip all collection when `fieldDataVersion` has not advanced since the last collect
  (the telemetry analogue of the overlay scheduler's existing skip-unchanged latch). Zero telemetry CPU on
  paused/static frames.
- **History continuity (the load-bearing caveat):** `collectScale0` (flux/energy/charge/particles — cheap)
  stays **unconditional** so the primary sparkline history is never interrupted. Only the two expensive
  streams gate. On panel-open, a **single synchronous catch-up collect** seeds the buffer so the user never
  sees an empty chart. This bounds the worst case to "the *audit* sparkline starts at panel-open," never an
  empty *primary* chart.

This collapses **four independent triggers** of the O(N³) audit pass (the diagnostics collector, the
conservation panel, and — when open — the charts and Lagrangian panels) into **at most one per changed
tick, and zero when nothing visible consumes it.**

---

## 4. Phase 0 — Baseline harness (measure first)

Before any change: a repeatable measurement so every fix has a before/after number and the stale §1 table
is replaced.

- A `preview_eval` timing harness capturing, at **L=65/97/129**, in **worker mode** and **in-thread mode**,
  with (a) default overlays + controls tab, (b) all telemetry panels open, (c) a floated+collapsed
  Telemetry Grid: worker tick period, main-thread frame time, and per-stage breakdown
  (tick / upload / overlay sweep-start vs drain / diagnostics collect / panel update).
- Record the numbers in a new profile table; annotate `SPEC_SCALE0_LATTICE_PERF.md §1` as superseded for
  the worker-mode case.
- **Deliverable:** measured baseline + a re-runnable harness (`tests/` spec or a documented `preview_eval`
  snippet) used to validate Phases 1–4.

*Files:* new `engine/web/tests/scale0-perf-baseline.*` (or a documented harness); no source changes.
*Effort:* S. *Risk:* none (measurement only).

---

## 5. Phase 1 — Demand-gate the expensive telemetry  ·  flag `FTD_TELEMETRY_ONDEMAND`

The core regression fix. Behind a feature flag (default on after §9 verification; off = today's
unconditional behavior, instant rollback).

### 5.1 Consumer-visibility gate (main thread)
`diagnostics.js:11-14`: collect `audit`/`lagrangian` only when a consumer is visible. Reuse the canonical
predicate at `app.js:745-751` (`activeTab === id || floatingWindowManager.has(id)`), extended with the
`is-collapsed` check (Phase 2). Keep `collectScale0` (`:9`) unconditional (status bar + primary history).
On a consumer mounting, fire one synchronous catch-up collect (§3 history continuity).

### 5.2 Worker want-mask
A new `setTelemetryMask({audit, lagrangian})` command (proxy → worker), driven by the same visibility
signal. `mock-bridge.worker.js:48-49` computes each stream only when its mask bit is set, **and** at a
**decoupled cadence** (~10–15 Hz, not every tick) — ship the last-computed value on intervening frames
(the proxy already serves "last" values, so no consumer notices; mirrors the `PLIST_EVERY` pattern, `:55`).
`_updateFluxMag()` (`:39`) stays **every tick** (the SAB `_fluxMag` must stay fresh for the volume upload).
This is the **#1 default-path lever** — it returns worker frame-budget to the tick at large L.

### 5.3 `fieldDataVersion` gate (all paths)
The hub records `lastCollectedVersion`; skip all collection when `state.fieldDataVersion` is unchanged
(`tick.js:56`; mirror `field-overlays.js:998`). Eliminates redundant recompute on paused/static frames —
especially the **uncached WASM path** (`wasm-bridge.js:344` has no tick-cache).

### 5.4 Prerequisite: bare proxy forwarders
`mock-bridge-proxy.js`: add bare `getEnergyAudit() { return this._lastAudit ?? null; }` and
`getLagrangian() { return this._lastLagrangian ?? null; }` — `physics-harness.js:118-120` calls them
**directly on the bridge** (not via `capabilities.scale0`), and they are currently absent on the proxy, so
the conservation panel's momentum terms silently read 0 under the worker. (Load-bearing for Phase 3 F.)

*Files:* `scales/scale0/runtime/diagnostics.js`, `telemetry-hub.js`, `bridge/mock-bridge.worker.js`,
`bridge/mock-bridge-proxy.js`, `bridge/capabilities/scale0.js` (mask plumbing), `config/toggles.js` (flag).
*Effort:* M. *Risk:* Low–Med (visibility wiring; default-safe so panels populate on open).

---

## 6. Phase 2 — Panel render + overlay-sampling taxes  ·  flag `FTD_PANEL_RENDER_V2`

### 6.1 Telemetry-grid (C3 — the floated/collapsed 60 Hz cost)
`telemetry-grid/component.js`: (a) early-return `update()` when the host floating window is collapsed
(`this.el.closest('.floating-window.is-collapsed')`); (b) throttle to ≤30 Hz (internal `nextDueAt`, or
route through the `%3` path / rAF coordinator at a declared Hz instead of raw `app.js:722`);
(c) preallocate the per-channel `xs`/`ys` `Float64Array` once at `rebuildGrid` and `subarray(0,n)` each
update (removes 46 allocs/update); (d) cache the value `<span>` at build time (removes 23
`querySelector`s/update, `:225`).

### 6.2 uPlot rescale tax (E)
`uplot-chart.js:130`, `sparkline.js:56`, `telemetry-grid:221`: stop `setData(…, true)` from forcing a
full scale-recompute + repaint every frame on every chart. Track last min/max per buffer; pass `false`
unless the new sample exceeds the current range, then rescale. Or skip `setData` entirely when the buffer
didn't advance since last draw. (Keep auto-fit — the `:127-129` comment's legibility constraint holds.)

### 6.3 Overlay `sampleFieldState` amortization (D)
`field-overlays.js:1010` samples **all enabled field kinds in one unbudgeted sweep-start frame**
(`:35-89`). Move per-kind sampling into the consuming `runJob`, cached once per sweep on `sched.sampled`
(first job to need a kind samples it; later jobs read the cache — preserving the one-snapshot-per-sweep
coherence), and add a `COST_SAMPLE` charge so the budget spreads it. Optionally raise `fieldThrottle` at
L>96 (12→16) — overlays already lag a few hundred ms, so it's imperceptible.

### 6.4 Visibility-predicate unification (I, J — also fixes a real bug)
charts (`:89`), lagrangian (`:133`), and grid (`:185`) use **three different** visibility predicates;
charts/lagrangian gate on `.active`, which is false when floated → **floated charts/Lagrangian panels
freeze**. Unify on one helper:
`isPanelLive(el) = el.classList.contains('active') || (el.closest('.floating-window') && !el.closest('.floating-window.is-collapsed'))`
and use it in all four panels' `update()`. Add the missing self-guard to `diagnostics-panel/component.js:38-40`.

*Files:* `ui/panels/telemetry-grid/component.js`, `ui/panels/charts-panel/*`, `ui/panels/lagrangian-panel/*`,
`ui/panels/diagnostics-panel/*`, `ui/components/uplot-chart.js`, `ui/components/sparkline.js`,
`scales/scale0/runtime/field-overlays.js`, a shared `isPanelLive` helper.
*Effort:* M. *Risk:* Low–Med (sampling amortization must never re-sample mid-sweep).

---

## 7. Phase 3 — GC hygiene + correctness  ·  ship direct (verified, no flag)

- **F — wrong-bridge panels.** `conservation-micropanel.js:261-263` (and the same pattern in
  `spectrum-panel.js`/`p1-observables-panel.js`) resolve `getBridge() = ctx.bridge` = the **idle WASM
  bridge** for `flux-*` scenarios → frozen/wrong conservation data, plus a redundant main-thread audit at
  4 Hz. Route these through `telemetryHub.s0.*` / the active-owner selector
  (`state.useFluxMock ? state.fluxMock : ctx.bridge`) — the hub already picks the right bridge
  (`telemetry-hub.js:159-170`). Depends on Phase 1 §5.4 forwarders. **Correctness + perf.**
- **G — `Int8Array(N³)` per audit.** `mock-diagnostics.js:127-136` allocates a fresh state map per call
  when particles exist (2.1 MB/tick at L=129). Hoist to a persistent grow-in-place scratch; clear only
  touched cells. (Flux-only scenarios already skip it.)
- **H — `getFluxSlice` allocation.** `mock-bridge.js:1508-1523` returns a fresh `Float64Array(N²)` per axis
  per upload (3×). Use two rotating persistent per-axis scratch buffers (preserves the deliberate no-alias
  contract from the 2026-04-26 fix without per-call allocation).
- **L — empty particle upload.** `frame-sync.js:18-19` uploads the particle frame unconditionally; skip
  when `count === 0` and was 0 last frame.
- **M — dead-code sweep.** Confirm the removed timeline/`MemoryRecorder`/`globalTick`/`setScale0*Buffer`
  scaffolding has no per-frame stragglers (grep `Scale0.*Buffer`, `MemoryRecorder`, `globalTick`).

*Files:* `scales/scale0/ui/overlays/{conservation-micropanel,p1-observables-panel,spectrum-panel}.js`,
`bridge/mock-diagnostics.js`, `bridge/mock-bridge.js`, `scales/scale0/runtime/frame-sync.js`.
*Effort:* M. *Risk:* Low (G/H bit-identical output; F is strictly more correct).

---

## 8. Phase 4 — Config hardening + doc reconciliation  ·  ship direct

- **N — COOP/COEP + visible indicator.** Verify the production host sends
  `Cross-Origin-Opener-Policy: same-origin` + `Cross-Origin-Embedder-Policy: require-corp`
  (`serve.py:57-59` is dev-only). Add a status indicator (extend the existing Native/WASM/Mock · GPU/CPU
  chip) that shows when the **worker path is NOT taken** so the silent in-thread fallback (C4) is detectable.
- **O — doc reconciliation.** Fix the two stale claims (§1.3): `SPEC_SCALE0_BRIDGE_ARCHITECTURE.md §5`
  shadow note; `SPEC_SCALE0_LATTICE_PERF.md §1` profile table (annotate + link the Phase-0 worker-mode
  numbers).

*Files:* `serve.py`, the status-chip component, `docs/SPEC_SCALE0_BRIDGE_ARCHITECTURE.md`,
`docs/SPEC_SCALE0_LATTICE_PERF.md`. *Effort:* S. *Risk:* Low (config + docs).

---

## 9. Build sequence

1. **Phase 0** — baseline harness + measured numbers (worker + in-thread, L=65/97/129). Record.
2. **Phase 1** — demand-gate telemetry behind `FTD_TELEMETRY_ONDEMAND` (5.4 forwarders → 5.1 main-thread
   gate → 5.3 version gate → 5.2 worker want-mask). Re-measure; flip flag on when verified.
3. **Phase 2** — panel render + overlay sampling behind `FTD_PANEL_RENDER_V2` (6.1 grid → 6.2 uPlot →
   6.4 predicate unification → 6.3 overlay sampling). Re-measure; flip on when verified.
4. **Phase 3** — GC/correctness fixes, direct, each verified individually.
5. **Phase 4** — config hardening + doc reconciliation.

After each phase: re-run the Phase-0 harness (gains confirmed, no regression) **and** the regression suite
(§11).

### Implementation status (2026-06-05)

| Item | Status | Notes |
|---|---|---|
| **Phase 1** (demand-gate telemetry, `FTD_TELEMETRY_ONDEMAND`) | ✅ shipped, default ON | Measured ~14.3 ms/tick audit pass reclaimed at L=97. `scale0-telemetry-gating.spec.js`; flag-off verified across all ~95 scenarios + smoke. |
| **Phase 2** (`isPanelLive` predicate + telemetry-grid caching/gate, `FTD_PANEL_RENDER_V2`) | ✅ shipped, default ON | Fixes floated charts/Lagrangian freeze + collapsed-grid 60 Hz. `scale0-panel-render.spec.js`. |
| **§6.2** uPlot no-rescale | ⏳ deferred | Needs per-chart range tracking; risk of visual clipping. The 30 Hz cap already halves the floated-grid repaint rate. |
| **§6.3** overlay `sampleFieldState` amortization | ⏳ deferred | Must preserve the per-sweep coherence snapshot — careful change. |
| **Phase 3-G** `Int8Array(N³)` audit scratch hoist | ✅ shipped | Persistent grow-in-place; bit-identical. |
| **Phase 3-M** dead-code sweep | ✅ done | `MemoryRecorder`/`globalTick`/`timeline/`/`*Scale0*Buffer` confirmed gone (one stale worker comment fixed). |
| **Phase 3-F** conservation/spectrum/p1 wrong-bridge | ⏳ deferred | Routing to the worker entangles with the Phase-1 want-set (an always-on consumer would defeat gating); needs a visibility gate + conditional want-mask. Tracked. |
| **Phase 3-H/L** `getFluxSlice` double-buffer / empty-particle skip | ⏳ deferred | Marginal GC (off-by-default overlays); Phase 1 already reduced audit frequency. |
| **Phase 4-O** doc reconciliation (stale §5 shadow note + §1 table) | ✅ done | This commit. |
| **Phase 4-N** COOP/COEP indicator | ⏳ deferred | Verify prod headers; add a "worker path active?" status chip. |

---

## 10. Risks, flags, rollback

- **Feature flags:** `FTD_TELEMETRY_ONDEMAND` (Phase 1) and `FTD_PANEL_RENDER_V2` (Phase 2), both default
  off until their phase passes §9/§11, then default on — instant revert without a code change. Phase 3/4
  ship direct (correctness/hygiene, individually verified).
- **History-continuity risk** (the steelman against demand-gating): mitigated by keeping `collectScale0`
  unconditional + the synchronous catch-up collect on panel-open (§3). Only the two expensive streams gate.
- **Visibility-detection leakiness:** browser visibility is imperfect (`offsetParent` null for
  `position:fixed`, occlusion undetectable). Mitigation: gate on the *explicit* app signals we control
  (`activeTab`, `floatingWindowManager.has`, `is-collapsed`) — not heuristic occlusion — so it is
  deterministic, and bias toward *over*-collecting (a closed panel that reads as visible costs CPU but is
  never wrong).
- **Worker cadence decoupling:** audit at ~10–15 Hz vs sparkline at 4 Hz is imperceptible; the field-advance
  rate *increases*. If a future feature needs a self-consistent whole-field main-thread snapshot, add a
  seqlock to `shared-field.js` CTRL first (the current single-buffer SAB has no tear guard — fine for
  strided visual reads, unsafe to extend).

---

## 11. Verification

- **Parity:** every panel shows identical values when open, flag-on vs flag-off (a Playwright spec drives
  open-each-panel + compare). The existing `scale0-panel-wiring.spec.js` is the template.
- **Perf:** re-run the Phase-0 harness; record worker tick period + main-thread frame time per phase.
  Target: >30 FPS playback at L=97 worker mode with all panels closed; no multi-frame stall with a floated
  collapsed grid.
- **Regression suite stays green:** `scenario-parity`, `toggle-coverage`, `wasm-scenario-coverage`,
  `scale0-worker-teardown`, `scale0-scenario-health`, `lifecycle-harness`, `overlay-scheduler`,
  `scale0-panel-wiring`.
- **Correctness spot-checks:** floated charts/Lagrangian panels stay live (Phase 2 fix); conservation panel
  shows live (non-frozen) data on a `flux-*` scenario (Phase 3 F).

---

## 12. Finding → fix traceability (nothing from the redteam is dropped)

| Redteam finding | Phase | Fix |
|---|---|---|
| Eng1-F1 / Eng5-RF-1 — unconditional audit/Lagrangian collection (`diagnostics.js:11-14`) | 1 | §5.1 consumer gate |
| Eng2-F1 / Eng5-RF-3 — worker recomputes audit/Lagrangian every tick | 1 | §5.2 want-mask + decoupled cadence |
| Eng5-RF-6 — telemetry has no `fieldDataVersion` gate | 1 | §5.3 version gate |
| Eng2-B3a — bare `getEnergyAudit`/`getLagrangian` absent on proxy | 1 | §5.4 forwarders |
| Eng4-F1/F2/F3 — floated grid 60 Hz + collapsed + allocs + querySelector | 2 | §6.1 |
| Eng4-F4 — `setData(…,true)` full repaint every frame | 2 | §6.2 |
| Eng3-F1/F2 — `sampleFieldState` unbudgeted sweep-start spike | 2 | §6.3 amortize |
| Eng4-F5/F6 — inconsistent predicates; floated charts freeze; diagnostics no guard | 2 | §6.4 unify |
| Eng2-F2 / Eng5-RF-2/RF-4/RF-9 — wrong-bridge panels + redundant 4 Hz audit | 3 | §7 F |
| Eng1-F5 / Eng5-RF-5 — `Int8Array(N³)` per audit | 3 | §7 G |
| Eng3-F4 / Eng5-RF-8 — `getFluxSlice` per-axis alloc | 3 | §7 H |
| Eng3-F5 — empty particle upload | 3 | §7 L |
| Eng5-RF-10 — timeline/MemoryRecorder/globalTick stragglers | 3 | §7 M |
| Eng2-F5 — silent in-thread fallback if no COOP/COEP | 4 | §8 N |
| Eng3-F6 / Eng2/Eng3 corrections — stale §1 table + §5 shadow note | 0, 4 | §4, §8 O |
| Eng4-F7 — `triggerChartResize` walks `querySelectorAll('*')` | 2/3 | scope the walk (minor, fold into §6.1/§7) |
| Eng5-RF-7 — sparse tick reverts to dense for flux-pulse seed | — | investigation-only; ε-prune is a fidelity tradeoff, out of scope (measure then decide) |

---

## 13. Open questions

- O1. Worker-mode measured ms for the `sampleFieldState` sweep-start frame with all 19 scalars at
  L=97/129 (Phase 0 fills this in — the redteam reasoned the *shape* but not the absolute constant).
- O2. Does production send COOP/COEP? (Phase 4 N — determines whether C4 is live in prod.)
- O3. After Phases 1–2, is the broad-seed default `flux-pulse` tick itself (sparse reverts to dense,
  Eng5-RF-7) the next floor? If so, an opt-in ε-prune is the only remaining lever — separate spec.
