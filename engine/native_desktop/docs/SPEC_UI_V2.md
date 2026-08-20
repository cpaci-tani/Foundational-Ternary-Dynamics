# SPEC — Native Desktop UI v2.0

**Status:** `[DESIGN — APPROVED, revision 3 (implementation boundary corrected)]` · **Scope:** `engine/native_desktop` · **Created:** 2026-08-18 · **Revised:** 2026-08-19
**Supersedes:** the raw Win32 control panel in `engine/native_desktop/src/main.cpp`
**Companion:** [`README.md`](../README.md) (subsystem overview), [`SPEC_NATIVE_GPU_TELEMETRY.md`](../../SPEC_NATIVE_GPU_TELEMETRY.md) (telemetry producer)

> **Revision 3 note.** A second source and architecture audit found four implementation blockers in revision 2: Phase 0 tested a command queue not introduced until Phase 2; the custom triple buffer admitted a reader/writer data race; session code depended on transport types owned by a UI target it was forbidden to link; and the proposed presenter callback could neither receive the active D3D12 command list nor record a legal readback after submission. Revision 3 replaces the foundation with independently shippable Phases 0A, 0B and 1, adopts a mutex-protected immutable snapshot publication model, introduces a UI-independent model target, and pins separate record/submit/capture seams. It also resolves the interactive-GPU EnergyLedger decision in favour of explicit, demand-gated host synchronization.
>
> **Approval record.** Source/architecture audit baseline: `d468585510b7`. Owner decisions on 2026-08-19: adopt the 0A→0B→1 split; use mutex-protected immutable snapshots; move the swapchain in Phase 1; provide demand-gated synchronized GPU ledger data. Revision 3 is the implementation-governing text; revision history remains in git.

---

## 0. Decisions of record

Settled with the owner during design. Do not relitigate these while implementing; raise a new decision instead.

| # | Decision | Rationale |
|---|----------|-----------|
| D1 | **Dear ImGui (docking branch) + ImPlot**, vendored as source | Renders inside the existing D3D12 command list — zero compositing friction with the CUDA interop path. Immediate mode suits a simulation instrument: no UI-state duplication. |
| D2 | **Flat, monochrome chrome.** Default theme **Graphite** (`#17181b` panel, `#5b8db8` muted steel accent) | Saturated colour is reserved for physics data. A cyan-accented shell competes with field ramps for attention. |
| D3 | **Scale 0 only** for v2.0, architected so further scales are additive | Keeps scope tractable without an architectural dead end. Realised, not merely asserted: `ScenarioMeta` carries a `scale` column (§5.1) and `PanelRegistry` is scale-agnostic, so Scale 1 is rows plus panels, never a schema migration. |
| D4 | All four capability clusters in scope | Physics control · field visualisation · instrumentation · workflow/output. |
| D5 | **Docking ON, multi-viewport OFF** in v2.0 (flag, default off) | Docking delivers the full shell IA. `ViewportsEnable` adds a swapchain per torn-out window, interacting with the interop path, for no IA gain. Revisit v2.1. |
| D6 | Panels never touch `RenderBridge`. Snapshot in, typed commands out | Enforced mechanically by a lint (§9.3), not by convention. |
| D7 | **Physics stays bit-accurate.** No UI feature may alter simulation results | §2 is the binding contract. |
| D8 | UI settings panel deferred to v2.1 | Owner decision. Theme/workspace switching still ships, via the menu bar and the palette (§4.1, §4.3) rather than a settings panel. |
| D9 | The foundation lands as **0A characterization → 0B transport/session spine → 1 renderer host** | A queue cannot be tested before it exists, and renderer migration is not a characterization task. Every increment remains launchable. |
| D10 | Snapshots publish as a **mutex-protected `shared_ptr<const UiSnapshot>`** | Correct ownership first. The measured rate is at most one publication per tick; a custom lock-free protocol is not justified until profiling proves contention. |
| D11 | Interactive-GPU EnergyLedger support uses **explicit demand-gated host synchronization** | The panel shows real bookkeeping rather than a frozen line. The synchronization is active only while requested, is surfaced as a performance mode, and produces gaps when disabled. |
| D12 | The swapchain moves to the top-level HWND in **Phase 1**, after an explicit viewport/scissor/input contract exists | Moving it in a no-UI gate phase left scene placement, camera aspect, control-strip occlusion and pointer transforms unspecified. |

---

## 1. Goals and non-goals

### 1.1 What v2.0 must achieve

The current app is a 1,176-line raw Win32 shell around a genuinely good D3D12 renderer. It exposes **0 of the 43** `TOGGLE_SPECS` rows, **1 of the 10** non-bool `TermToggles` config fields (`flux_boundary`, via the Boundary combobox at `main.cpp:934` → `NativeEngineSession::set_flux_boundary`), **1 of 18** field kinds (`FluxVector`, hardcoded in `append_flux`), and **4 numbers plus 3 strings** through a single 90px `STATIC` control (`main.cpp:1100-1111`) rewritten 60×/second. `RenderBridge` offers `diagnostics()`, `energy_audit()`, `poll_telemetry_snapshot()`, `copy_compact_lagrangian()`, `gravity_metric_agg()`, `energy_ledger()`, `continuity_step()`, `inspect_voxel()`, `inspect_force()` — **none of it is wired in.** The scientific value of a native app is currently unreachable from the native app.

v2.0 makes the instrument usable:

- **G1** Every one of the 43 `TOGGLE_SPECS` rows, the 10 non-bool `TermToggles` config fields, and the 6 `RenderBridge`-level knobs are reachable and editable while the sim runs. The six are, explicitly: `lattice_size`, `dt`, `sor_iterations` (`render_bridge.h:210-211`), `genesis_threshold_override`, `manifest_scale_override`, `manifest_use_temperature` (`render_bridge.h:354-356`). 43 + 10 + 6 = **59 knobs**; the arithmetic in §4.3 refers to exactly this set, and no field appears in two panels (§5.2/§5.3).
- **G2** All 18 `VisualFieldKind` values selectable, with honest legends and disclosed decimation.
- **G3** Live instrumentation: diagnostics, energy audit, Lagrangian, gravity, energy ledger as ImPlot series with rolling history; click-to-inspect a voxel.
- **G4** Scenario discovery across all 130 Scale-0 ids with search, categories and metadata; session persistence; export.
- **G5** Per-monitor-V2 DPI correctness. The app is currently DPI-unaware and bitmap-stretched on any display above 100%.
- **G6** A themeable, flat, professional shell that reads as a product rather than an unfinished dialog.

### 1.2 Explicit non-goals for v2.0

- Multi-viewport tear-out into OS windows (D5).
- Scales 1–6 (D3).
- A UI settings panel (D8).
- Replacing `engine/web` — the web dashboard remains the multi-scale surface.
- General screenshot-diff testing (§9.6 explains why this is a deliberate refusal, not an omission).

---

## 2. The physics-accuracy contract

**Binding on every task.** Where this section conflicts with any other, this section wins.

Every clause in this section names the test that enforces it. A clause with no test is not a contract, it is a wish; §9.5 and §9.1 carry the enforcement, and §2.6 is the index from clause to test.

### 2.1 Simulation invariance

- **C1** The UI must not change simulation results. The merge gate (§9.4) runs on *every* UI task, not once at the end.
- **C2** ⚠ **"Golden green" is not "physics verified."** Per `docs/adr/0012-golden-tick-regression-gate.md`, the frozen golden profile runs ~14 subsystems toggled **off**. Only `render_bridge_golden_default` (zero toggle writes) pins the shipping `TermToggles{}` defaults. A task that changes shipping defaults or the boot scenario changes what physics runs while the minimal golden stays green — such tasks **must** state in the commit message which defaults moved.
- **C3** Commands mutating engine state apply **only at tick boundaries**, never mid-tick — and this is a rule the UI must *enforce*, not a property it inherits. ⚠ The GPU backend does snapshot state: `GpuBackend::tick()` copies `bridge_.toggles → engine_->toggles` at `backend.cpp:250`, near its top. **The CPU backend has no such snapshot** — `RenderBridge::toggles` is a public member that every phase reads live throughout `tick()` (`render_bridge.cpp:800+`, `render_bridge_phases/phase_write.cpp:152-157`). On CPU a mid-tick write is a data race that genuinely does apply mid-tick. The drain seam is defined precisely in §3.4 and asserted by §9.5 N4.
- **C4** The UI never touches any RNG stream. There are **three**, and the spec names all three so no future reader believes there is one:
  1. **Tick-time noise** — `voxel_rng.h`'s stateless index-keyed `voxel_uniform/voxel_normal(seed, voxel_idx, tick, salt)`, seeded from `toggles.langevin_seed`. This is what the Langevin thermostat actually draws (`render_bridge_phases/phase_write.cpp:258-262`), and it is thread-count-independent by construction.
  2. **`BridgeRng`** — a stateful mt19937 owned by `RenderBridge` (`bridge_rng.h`; member at `render_bridge.h:678`; constructed with seed 42 at `render_bridge.cpp:97`). `phase_write` calls `rb.rng_state_->reseed_thread_pool(...)` **unconditionally every tick** (`phase_write.cpp:174`) with a pool sized by `omp_get_max_threads()` (`:163`), so the parent generator advances once per thread per tick. ⚠ **No sampling method of `BridgeRng` has a call site in `engine/src/` today** (`sample_uniform`/`thread_uniform`/`thread_normal` are defined in `bridge_rng.cpp` and called nowhere else; the `BridgeRng& rng` binding at `phase_write.cpp:186` is unused), so the *trajectory* does not depend on thread count — but `rng_state_hash()` (`render_bridge.h:235`) does, and four `src/eft/` bounds compare it. Any future consumer of `BridgeRng` makes the trajectory thread-count-dependent silently.
  3. **Scenario initial conditions** — a separate `thread_local` mt19937 seeded from `SCN_RNG_SEED = 0xC0DEFACE` (`scenarios.cpp:44`), reset unconditionally at the top of every `dispatch_scenario` (`scenarios.cpp:206-211`). See §2.3.
  **Standing rule:** the UI thread must not alter the OpenMP runtime — no `omp_set_num_threads`, no thread-affinity changes. Phase 1 snapshots `omp_get_max_threads()` around headless ImGui initialization and around interactive D3D12 backend initialization; both must remain unchanged (§9.1).

### 2.2 Demand-gating safety

Revision 1 split observers two ways. That split was wrong: it treated six methods as pure reads when they perform a host→device write on the GPU backend, and it treated `begin_telemetry_snapshot()` as an observer when it is a commit point. The correct split is **four categories**.

**Category 1 — pure reads.** Free to call, free to skip, free to reorder. `energy_ledger()` (returns the stored struct), `current_tick()`, `dt()`, `sor_iterations()`, `physical_time()`, `lattice()`, `interactive_gpu_mode()`, `backend().kind()`, and the `TermToggles` copy. These are the only methods the snapshot builder may call unconditionally.

**Category 2 — scheduled flushing observers.** Each calls `backend_->flush_host_mutations()` and may be selected by telemetry demand:

| Observer | Call site |
|----------|-----------|
| `gravity_metric_agg()` | `render_bridge.cpp:469` |
| `diagnostics()` | `render_bridge.cpp:1193` |
| `energy_audit()` | `render_bridge.cpp:1202` |
| `copy_compact_lagrangian()` | `render_bridge.cpp:1228` |

On the GPU backend that flush reaches `GpuEngine::upload_from_host()` (`cuda/gpu_engine.cu:1817`), which **invalidates the continuity ledger** (`:1854`), **recomputes `weak_field_active_`** — a latch mixed into `graph_key()` at `:1011` — via `refresh_weak_field_active_from_host()` (`:1852`), and **bumps `state_version_`** through `mark_device_state_changed()` (`:1855`), which stamps telemetry and visual snapshot provenance. `host_mutated_` is set by *merely calling* the non-const `voxels()` or `voxel_at()` (`render_bridge.h:137-158` — `mark_host_dirty()` fires on handout, no mutation required).

The failure this creates is a draw-order dependency, not a crash: with one pending host edit, if the Inspector draws before a Continuity readout, `inspect_voxel()` flushes and `continuity_step()` then returns an all-zero `DualCellContinuity{}` (`gpu_engine.cu:1805`, guarded by `continuity_ledger_valid_`). Dragging a tab in the dockspace — a pure layout action, persisted per workspace — would change whether a diagnostic has content.

**Therefore, mandatory:** the sim thread performs **one explicit `flush_host_mutations()` at a fixed point** in the tick-boundary sequence (§3.4), *after mutation commands and before any observer request runs*. Every Category-2 observer's internal flush is then a guaranteed no-op, and call order stops mattering. Enforced by §9.5 N5.

**Category 3 — one-shot observers, requested by commands.** `inspect_voxel()` and `inspect_force()` also call `flush_host_mutations()`; `charge_sum()` (`render_bridge.cpp:339`) and `continuity_step()` (`render_bridge.cpp:1159`) have their own synchronization/provenance rules. None is a telemetry group or polled per frame. Their commands are classified as observation requests during the drain, retained until after the fixed-point flush, and only then executed. Results ride a later `UiSnapshot`. `RequestChargeSum` on interactive GPU explicitly synchronizes the ternary state before reducing it and carries a synchronization-cost flag in the result; it must never report a stale host cache. If the fixed flush invalidates continuity provenance, `RequestContinuity` remains pending and executes after the next completed tick; the intervening snapshot says `PendingAfterHostUpload`, never a fabricated zero result. ⚠ `continuity_step()` allocates six L³ vectors per call (W13) — one-shot only, never a series.

**Category 4 — engine-maintained producers with explicit cadence rules.**

- **`update_energy_ledger()`** — non-const per-tick conservation bookkeeping with **two current call sites and two reachability conditions**: `render_bridge.cpp:906` on the GPU path, and `render_bridge.cpp:1141` on the CPU path (unconditional, at the tick tail). The CPU producer is never gated. Interactive GPU currently skips it at `render_bridge.cpp:899-904`; Phase 6 adds the explicit `gpu_energy_ledger` demand state. On the off→on boundary, the session increments `ledger_epoch`, clears cumulative/derivative state, synchronizes one seeding sample, and sets `tick_prev`, `E_prev` and `E_curr` without emitting a derivative sample. Each subsequent demanded GPU tick synchronizes and updates exactly once, reporting duration and freshness. On→off performs no sync or update and charts append a gap. No derivative, residual or cumulative quantity bridges epochs.
- **The `poll_telemetry_snapshot()` drain** — ⚠ **there is no queue.** `CpuBackend::begin_telemetry_snapshot()` is a single-slot latch: `if (telemetry_snapshot_pending_) return false;` … `telemetry_snapshot_ = snapshot; telemetry_snapshot_pending_ = true;` (`backend.cpp:134-181`), and `poll_telemetry_snapshot()` copies the one slot and clears the flag (`backend.cpp:188-193`). The GPU path is the same single-slot pattern (`cuda/gpu_engine.cu:1428-1467`). Gating the poll does not deepen a queue — **it wedges the producer permanently**, because every subsequent `begin` returns `false` forever with no error. Gate the upstream request, never the drain.

**The gating primitive, and the existing implementation of it.** `TelemetrySnapshotRequest.groups` is a bitmask over `TELEMETRY_DIAGNOSTICS | AUDIT | GRAVITY | LAGRANGIAN`, and **a group's fields in `TelemetrySnapshot` are valid only if its bit is set**. ⚠ **`begin_telemetry_snapshot()` is not itself an observer on either backend:**

- **GPU** — `GpuBackend::begin_telemetry_snapshot()` (`backend.cpp:528-545`) assigns `engine_->toggles = bridge_.toggles`, copies the three override scalars, and calls `flush_host_mutations()` before stamping the request. It is a *commit point*, so it must be issued from the sim thread at the same tick boundary as the command drain, never opportunistically from the GUI thread.
- **CPU** — `CpuBackend::begin_telemetry_snapshot()` computes every requested group synchronously by calling `bridge_.diagnostics()`, `bridge_.energy_audit()` and `bridge_.gravity_metric_agg()` (`backend.cpp:162-178`) — three Category-2 observers. The want-mask therefore transitively selects which flushing observers run. The fixed-point flush above removes that as a timing variable on both backends.

**The engine already ships the scheduler that wraps this correctly.** `engine/include/ftd/native_telemetry_scheduler.h` is a header-only, `ws_server`-consumed scheduler with per-group demand masks, per-group tick cadences (`Demand::every_ticks{1, 8, 4, 12}`, `:52`), min-interval throttling, epoch/source-epoch coalescing, direct-mutation debouncing, retained-group provenance with its own per-group metadata (exactly the P7 requirement), `Invalidation` deltas, and a suspend/deadline path. **Decision of this spec:** the native shell owns its own `NativeTelemetryScheduler` instance on the sim thread, and the UI want-mask drives its `Demand` struct; the shell consumes `CachedView`/`Publication`. The UI does **not** call `begin_telemetry_snapshot()` directly, and does not build a second gating path. §12 Phase 6 is therefore "wire `DataNeeds` into the existing scheduler's `Demand`", not "introduce gating".

⚠ The scheduler's deadline throw (`native_telemetry_scheduler.h:295-302`) is a **wall-clock** timeout of 2/6/15 s by lattice size (`snapshot_timeout`, `:605-613`). It is a real failure surface (§2.5) but it is timing-dependent and therefore belongs to the `interactive` label, never to `merge_gate` (§9.5 N3).

**Field-sampler cadence.** `copy_visual_field_sample()` is the most expensive observer in the design (W12) and is **non-const and sim-thread only**. Its cadence is: **on explicit `RequestField` command and on any change to kind or stride — never per frame.** The engine API has no threshold parameter and applies kind-specific internal floors. The UI threshold is therefore a display-only post-filter constrained to values at or above the sampler floor; changing it does not resample and cannot recover samples omitted by the engine. The legend discloses both floors. The two prepass kinds (`Kretschmann`, `Latency`) are additionally rate-limited to at most one sample per 8 ticks, surfaced in the Fields panel as a cost badge.

This section is enforced by executable tests, not by convention — see §9.5 and the index in §2.6.

### 2.3 Reproducibility

Live parameter editing is a headline feature and the thing most likely to quietly destroy auditability. A run is fully determined by the following **de-duplicated** set (each field appears exactly once, and each has exactly one journal key and one owning panel):

```
── environment (journal header, captured once at boot) ──
backend().kind()  ·  interactive_gpu_mode()  ·  omp_get_max_threads()
·  FTD_FORCE_CPU  ·  FTD_FORCE_GPU  ·  engine version

── run identity ──
scenario id  ·  lattice_size

── the 43 bool TOGGLE_SPECS rows ──

── the 10 non-bool TermToggles config fields (§5.3 owns all ten) ──
bcc_stencil · langevin_site_filter · langevin_T · langevin_gamma
· langevin_seed · coulomb_charge_coupling · coulomb_source_scale
· omega0 · kinetic_drain · flux_boundary

── the 6 RenderBridge-level knobs (§5.2 owns all six) ──
lattice_size · dt · sor_iterations
· genesis_threshold_override · manifest_scale_override · manifest_use_temperature
```

⚠ `lattice_size` is listed under both run identity and the six knobs deliberately — it is one field with one key (`bridge.lattice_size`), named twice for reading convenience. `langevin_seed` and `flux_boundary` are `TermToggles` members (`term_toggles.h:133`, `:144`) and belong **only** to the ten; revision 1 double-listed them and gave them two owning panels.

**Environment fields matter.** `strong_force` and `exchange_force` are no-ops on CPU builds (W10), so an identical toggle profile is different physics on CPU and GPU. `interactive_gpu_mode()` determines whether the energy ledger accumulates at all (§2.2). `FTD_FORCE_CPU`/`FTD_FORCE_GPU` are read at runtime (`backend.cpp:610`, `render_bridge.h:293-297`) and silently change the backend — `force_cpu()` is a **no-op** when `FTD_FORCE_GPU` is set.

**Journal record schema.** Every change is appended to an in-memory **parameter journal**, exported alongside CSV telemetry. A run remains reproducible from *initial state + journal*.

```cpp
enum class JKind { Bool, Double, UInt, Enum, Boundary, ScenarioId };
struct JValue { JKind kind; bool b; double d; unsigned u; int e; std::string s; };
struct JournalEntry {
  int          tick_applied;   // the tick boundary at which it was applied
  std::string  key;            // namespaced: "toggles.*" | "bridge.*" | "run.*"
  JValue       old_value;
  JValue       requested;      // what the UI asked for
  JValue       applied;        // what the engine actually holds afterwards
};
```

Three rules make the journal replay-safe:

1. **One entry per changed key.** Never a batch entry standing in for several fields.
2. **`applied` is read back from the engine after the command lands**, not copied from the request. ⚠ This is load-bearing: `set_dt()` silently clamps (W4) and `tick()` re-clamps to 1.0 under either Floquet toggle (`render_bridge.cpp:784-791`), so a journal of *requested* values does not reproduce the run it claims to. When `requested != applied`, the UI badges the control.
3. **A scenario load emits a `run.scenario` entry followed by the complete resulting diff** against the prior profile — because `dispatch_scenario()` mutates `rb.toggles` as a side effect and can zero the whole registry (`scenarios/s0_seed.cpp:969`), so the id alone loses the profile. A profile restored from persisted settings (§8) is journalled the same way, at tick 0.

An L0 test asserts that replaying the journal from initial state reproduces the final `TermToggles` and the six knobs bit-exactly (§9.1).

⚠ **Do not claim the seed control randomises initial conditions.** Scenario ICs use the independent `SCN_RNG_SEED = 0xC0DEFACE` stream (`scenarios.cpp:44`), completely independent of `toggles.langevin_seed`. The seed control's tooltip must say so. ⚠ That stream's generator is `thread_local` (`scenarios.cpp:45`) and the app dispatches scenarios from **two different threads** across a session — the main thread inside the initial `boot()` (`engine_session.cpp:116`, before the sim thread exists) and the sim thread on every reload. The two are distinct generator instances; results match **only** because `detail::reset_scenario_rng()` runs unconditionally at the top of every dispatch (`scenarios.cpp:206-211`). v2.0 routes `LoadScenario` through the tick-boundary drain, making that split permanent. Any future caller of `detail::urand()` outside `dispatch_scenario` breaks it. An L0 test dispatches the same stochastic scenario from two threads and asserts identical lattices.

### 2.4 Presentation honesty

The visualisation must not lie:

- **P1** Fix the unsorted-alpha-with-depth-writes bug. Translucent sprite edges currently occlude particles behind them in a draw-order-dependent way, worsening with particle count. **Owned by Phase 5** (§12), verified by an L2 offscreen readback of two overlapping translucent sprites in both draw orders asserting identical output.
- **P2** **Disclose decimation, unconditionally.** The legend **always** shows requested stride, effective stride, engine sampling floor, UI display threshold, sample count, and `origin`; a mismatch between requested and effective is *additionally* badged. `VisualFieldSample::effective_stride` may be *raised* by the sampler (cap 262,144 samples) and `count()` is data-dependent because sub-floor samples are omitted; six kinds are interior-only (`origin=1`). A legend that only sometimes shows these provenance fields trains the reader not to look for them.
- **P3** Legends carry real numeric bounds, not a bare gradient. Verified by a headless assertion over the legend strings produced from a synthetic snapshot (§9.1 L1).
- **P4** Charts plot raw samples. No smoothing that hides values; **gaps render as gaps**. A demand-gated group appends nothing to its series (§3.3b `History`), so a gated interval is a real gap in the tick axis, never a flatline or a held last value.
- **P5** A colourblind-safe ternary ramp option, shipped in every built-in theme. The current red/green ±1 pair is the worst possible choice for deuteranopia, and today there is no way to distinguish them at all. **Concrete bar:** under the Brettel–Viénot–Mollon deuteranopia and protanopia simulations, the three ternary swatches (−1, 0, +1) must pairwise exceed **ΔE₀₀ ≥ 20**. **Owned by Phase 5**, asserted numerically over the ramp LUT by an L0 test.
- **P6** ⚠ `TelemetryLagrangian::total_hamiltonian` / `LagrangianDiag::total_hamiltonian` is a documented legacy misnomer excluding field kinetic/gradient/cross terms and "must not be used as a total wave-energy conservation observable" (`lagrangian.h:215-218`). Never label that series as total energy. Verified by a one-line string lint over the series labels (§9.1 L1).
- **P7** Each telemetry group carries its own `*_meta.tick`. Charting all four groups against the top-level `tick` silently mis-aligns series when a slow group is retained — and the scheduler retains slow groups by design (`every_ticks{1, 8, 4, 12}`). Plot each series against its own group meta tick. Verified by an L1 test with deliberately mismatched group metas asserting each series' x-vector equals its own group tick.
- **P8** ⚠ **`residual` is not a conservation violation on the shipping profile.** `energy_ledger_compute.cpp` documents in its own body that `selective_damping` — **ON by default** in shipping `TermToggles{}` (`term_toggles.h:56`) — damps only manifested sites plus their six face neighbours, so "no single global scalar can express the expected rate for that regime … `expected_rate` remains an approximation and `residual` should not be read as a conservation violation." Whenever `selective_damping` is true, the EnergyLedger panel badges `expected_rate`, `residual` and `max_residual_seen` as approximate and must not present `residual` as a violation. The badge is driven off the snapshot's `term_toggles`, never off a constant.

### 2.5 Failure visibility

Interop degrading to the CPU path mid-session becomes a **status-bar state change**, not a line printed to a console window nobody reads. Same for backend selection, device-removed recovery, scenario rejection, the six CPU-forcing toggles (W5), the two `tick()` throws (W18), and the telemetry scheduler's deadline throw (§2.2).

⚠ The status bar is **shell chrome, not a dock** (§4.1) and is never hidden by any workspace, including Presentation. Failure visibility must not depend on a layout choice.

### 2.6 Clause → test index

Every binding clause has a named enforcer. A clause added to §2 without a row here is not landed.

| Clause | Enforced by |
|--------|-------------|
| C1 | §9.4 merge gate, every task |
| C2 | Commit-message rule + `render_bridge_golden_default` |
| C3 | §9.5 **N4** (command-apply timing equivalence + negative control) |
| C4 | §9.5 **N2** (extended hash incl. `rng_state_hash()`) + §9.1 Phase-1 OpenMP-drift tests |
| §2.2 cat. 2 fixed-point flush | §9.5 **N5** (`gpu interactive`) |
| §2.2 cat. 4 ledger | §9.5 **N1** (CPU) + **N1-gpu** (`interactive`) |
| §2.2 cat. 4 drain | §9.5 **N3** (single-slot wedge) |
| §2.2 want-mask neutrality | §9.5 **N6** (`gpu interactive`) |
| §2.3 journal | §9.1 L0 journal-replay test |
| §2.3 scenario RNG thread split | §9.1 L0 two-thread dispatch test |
| P1 | §9.1 L2 two-draw-order readback |
| P2, P3 | §9.1 L1 legend-string assertions |
| P4, P7 | §9.1 L1 mismatched-meta series test |
| P5 | §9.1 L0 ΔE₀₀ assertion over the ramp LUT |
| P6 | §9.1 L1 series-label lint |
| P8 | §9.1 L1 badge assertion from a `selective_damping=true` snapshot |
| §2.5 | §9.1 L1 status-bar state assertions from synthetic failure snapshots |

---

## 3. Architecture

### 3.1 Library split

The single decision that makes everything else testable. Five targets:

| Target | Contents | Links |
|--------|----------|-------|
| `ftd_imgui` (STATIC) | `imgui.cpp`, `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`, `implot.cpp`, `implot_items.cpp`. **No backend.** Configured via `IMGUI_USER_CONFIG` (§10), so the upstream tree stays unmodified. | — |
| `ftd_imgui_win32_dx12` (STATIC) | `imgui_impl_win32.cpp`, `imgui_impl_dx12.cpp` | `ftd_imgui`, `d3d12`, `dxgi` |
| `ftd_native_ui_model` (STATIC) | `UiCommand`, `UiSnapshot`, queue, immutable snapshot publisher, journal value types, result/status vocabulary. **No ImGui, D3D12, CUDA or `RenderBridge`.** | `ftd_core` for allowlisted value types |
| `ftd_native_ui` (STATIC) | Every panel, state machine, theme and workspace TU | `ftd_imgui` + `ftd_native_ui_model` — **zero D3D12, zero `RenderBridge` use** |
| `ftd_native_session` (existing, extended §3.4b) | Session/bridge ownership, snapshot builder, command applier, reload/tick result channels | `ftd_native_ui_model`, `ftd_core`, conditionally `ftd_cuda` |

**The dependency rule, stated once.** Revision 1 gave three mutually incompatible versions of it ("links `ftd_imgui` only", "no engine headers", "no `render_bridge.h`") and all three were contradicted by the panel catalogue. The rule is:

> `ftd_native_ui_model` may include exactly these engine headers for **data types and pure lookup symbols only**; `ftd_native_ui` consumes those values through the model target:
>
> `ftd/term_toggles.h` · `ftd/telemetry_snapshot.h` · `ftd/visual_field_sample.h` · `ftd/render_bridge_diagnostics.h` · `ftd/scenario_meta.h` · `ftd/lattice.h` · `ftd/constants.h`
>
> Every other engine header is forbidden, `ftd/render_bridge.h` above all. The architectural guarantee is the **lint's** job (§9.3), not the link line's.

Engine-only results not defined by those headers are copied into model DTOs. In particular, `ContinuitySnapshot` is the UI-model projection of `DualCellContinuity`; the model does not include `ftd/eft/dual_cell_continuity.h`.

Linking `ftd_core` is unavoidable and was always going to happen: `scale0_scenario_ids()` is a link-time symbol defined at `engine/src/scenarios.cpp:64`, and `visual_field_kind_name()` / `visual_field_components()` are declared at `visual_field_sample.h:47-49` but defined in `src/visual_field_sample.cpp`. Hand-copying an 18-entry name table into the UI would guarantee drift from the wire ids the header warns must stay synchronised.

⚠ Revision 1's "lets panel tests register `NO_CORE` and run in milliseconds" is **withdrawn as false**: `ftd_add_test` links every target against `ftd_test_support` (`cmake/FtdAddTest.cmake:111-112`), which is `target_link_libraries(ftd_test_support PUBLIC ftd_core)` (`engine/CMakeLists.txt:502`), so `NO_CORE` tests get `ftd_core` transitively regardless. The true payoff stands and is what matters: **panel tests need no device, no window and no `RenderBridge` construction**, hence milliseconds.

`UiSnapshot` lives in `native_desktop/include/native_desktop/ui_snapshot.h`. ⚠ It is **not** a POD — `VisualFieldSample` holds two `std::vector<float>` (`visual_field_sample.h:39-40`), so the snapshot is a heap-owning aggregate. It is **value-semantic, shared copy-free via `shared_ptr<const>`, and includes no engine *behaviour* headers**.

⚠ `src/d3d12_presenter.cpp` is compiled into **9 targets** (the app plus 8 test binaries — verified: nine occurrences in `engine/native_desktop/CMakeLists.txt`). Adding `#include <imgui.h>` there forces all 8 to link ImGui. Keep the presenter ImGui-free: it takes an opaque overlay recorder receiving the active `ID3D12GraphicsCommandList*` and render-target metadata (§3.5).

### 3.2 File layout

```
engine/native_desktop/
  include/native_desktop/
    ui_snapshot.h          ftd_native_ui_model — immutable published value
    ui_command.h           ftd_native_ui_model — typed command variant
    ui_demand.h            ftd_native_ui_model — DataNeeds and demand vocabulary
    command_queue.h        ftd_native_ui_model — owning MPSC transport
    snapshot_publisher.h   ftd_native_ui_model — mutex-protected publication
    ui_snapshot_builder.h  sim-thread builder (§3.4b)
    command_applier.h      sim-thread applier (§3.4b)
  src/
    command_queue.cpp          ftd_native_ui_model
    snapshot_publisher.cpp     ftd_native_ui_model
    ui_snapshot_builder.cpp    ftd_native_session — sim thread, touches RenderBridge
    command_applier.cpp        ftd_native_session — sim thread, touches RenderBridge
    parameter_journal.{h,cpp}  ftd_native_session — §2.3
    ui/                        ftd_native_ui — LINTED (§9.3); never touches RenderBridge
      ui_shell.{h,cpp}         dockspace, menu bar, status bar, workspace switching
      panel.h                  Panel interface + PanelContext + DockSlot
      panel_registry.{h,cpp}   registration + iteration
      command_palette.{h,cpp}  Ctrl+K
      theme.{h,cpp}            Theme struct, parse, apply, built-ins
      workspace.{h,cpp}        layout persistence + migration + DockBuilder recipe
      history.{h,cpp}          per-series ring buffers for charts
      widgets/                 search_box, toggle_table, kv_table, ramp_legend
      panels/                  one file per panel
  themes/*.theme           external theme files (§7)
  assets/font_*.inl        generated compressed font (§9.2, §10)
  docs/SPEC_UI_V2.md       this document
```

⚠ There is no `src/panels/` directory. Panels live at `src/ui/panels/`, and §9.3's lint glob must say so.

### 3.3 Panel interface

```cpp
struct Panel {
  virtual ~Panel() = default;
  virtual const char* id()    const = 0;   // "telemetry" — stable, never displayed
  virtual const char* title() const = 0;   // "Telemetry" — display, may change
  virtual DockSlot default_slot() const = 0;
  virtual DataNeeds needs()   const { return {}; }   // drives next-frame demand
  virtual ImGuiWindowFlags flags() const { return 0; }
  virtual void draw_contents(PanelContext&) = 0;
};
```

⚠ **The window-name rule, stated once.** ImGui's `.ini` keys settings on the string passed to `ImGui::Begin()`, and `DockBuilderDockWindow` keys on `ImHashStr` of that same string — **not on any app-side id**. Revision 1's comment "`id()` — stable, used in .ini" was therefore false and would have silently orphaned every saved layout on the first title change. The rule:

> Every panel is opened as `ImGui::Begin(ui::window_name(*this), ctx.open, p.flags())`, where `window_name` returns `"<title()>###<id()>"`. `DockBuilderDockWindow`, `SetWindowFocus` and every other name-keyed ImGui call use the **identical** composed string, obtained from the same helper. An L1 assertion checks the composed name is stable across two draws and contains `###`.

`PanelContext` is defined in §3.3b. Adding a panel is one new file plus one registration line. **Open/closed/hidden/collapsed is shell-owned:** the shell calls `ImGui::Begin()`/`End()`, records whether `Begin()` returned visible contents, and calls `draw_contents()` only when appropriate. The resulting visibility drives the **next frame's** aggregated demand, avoiding a circular attempt to know collapse state before `Begin()`. The shell holds a `bool` per panel keyed by `id()`, persists the open-set in the workspace, and skips the panel entirely when hidden.

> **Ported from the web dashboard, improved.** `engine/web/js` has three incompatible mounting patterns — **22** panels via `init*Panel()`, **12** via hand-rolled `mount*Panel(host)` factories that stash themselves on `window.__ftd*Panel`, and **2** exports with no call site anywhere in `js/` (`initOverlayPanel`, `mountFluxSlicePanel`). One pattern here, no exceptions, enforced by §9.3.

### 3.3b Type declarations

The design rests on nine types. Revision 1 declared one. These are the actual contract; `DockSlot` and `DataNeeds` sit on `Panel`'s vtable, so nothing can be written before they are pinned.

```cpp
// ── Dock placement ──────────────────────────────────────────────────────
// The Viewport is the dockspace CENTRAL NODE, not a slot (§4.1).
enum class DockSlot { Setup, Instruments, Physics };

// ── Data demand (ui_demand.h; owned by ftd_native_ui_model) ─────────────
struct DataNeeds {
  std::uint32_t telemetry_groups = 0;  // TELEMETRY_DIAGNOSTICS|AUDIT|GRAVITY|LAGRANGIAN
  bool  energy_ledger   = false;       // CPU read; enables explicit GPU sync mode
  bool  field_sample    = false;       // drives RequestField cadence (§2.2)
  int   history_depth   = 0;           // 0 ⇒ this panel needs no rolling history
};
DataNeeds operator|(const DataNeeds&, const DataNeeds&);   // group-wise OR / max

// The shell ORs the needs of panels whose contents were visible in the previous
// frame. One-frame demand latency is intentional and deterministic. The result
// becomes NativeTelemetryScheduler::Demand plus gpu_energy_ledger demand.

// ── Command sink ────────────────────────────────────────────────────────
class CommandSink {
 public:
  virtual ~CommandSink() = default;
  /// Enqueue one command; returns its monotone sequence number. A panel may
  /// compare it against UiSnapshot::last_applied_seq to render a pending state.
  virtual std::uint64_t push(UiCommand) = 0;
};

// ── Per-frame panel context ─────────────────────────────────────────────
struct PanelContext {
  const UiSnapshot& snapshot;
  CommandSink&      commands;
  const Theme&      theme;
  const History&    history;
  bool*             open;        // shell-owned; nullptr ⇒ panel is not closable
  float             dpi_scale;   // 1.0 at 100%; all DIP figures multiply by this
};

// ── Chart history ───────────────────────────────────────────────────────
// P4 and P7 make the naive ring<double> wrong: each series carries its OWN
// x-axis from its group meta tick, and a gated interval must produce a GAP.
struct SeriesKey {                      // ordered, usable as a map key
  std::uint8_t  group;                  // 0=Diagnostics 1=Audit 2=Gravity
                                        // 3=Lagrangian 4=EnergyLedger
  std::uint16_t field;                  // index into that group's field list
};
struct Series {
  std::vector<int>    tick;             // this group's meta tick per sample
  std::vector<double> value;            // tick.size() == value.size()
};
class History {
 public:
  static constexpr std::size_t kCapacity = 4096;   // samples per series
  const Series* find(SeriesKey) const;             // nullptr ⇒ never sampled
 private:
  friend class UiShell;   // written ONLY by UiShell::on_snapshot(), post-acquire
};
// Append rule: for each series, if its group's bit is absent from
// snapshot.groups, append NOTHING. Never a repeated value, never an
// interpolated one. ImPlot is called with the series' own `tick` vector as x.

// ── Theme ───────────────────────────────────────────────────────────────
struct Rgba { float r, g, b, a; };            // sRGB floats in [0,1]
struct Ramp { std::string name; std::vector<Rgba> stops; bool cvd_safe; };
struct Theme {
  std::string name;
  Rgba surface_0, surface_1, surface_2, surface_3;
  Rgba text_primary, text_secondary, text_muted, text_dim;
  Rgba border, accent;
  Rgba status_ok, status_warn, status_error;
  struct Data {
    Ramp                 ternary;        // −1 / 0 / +1; MUST include a cvd_safe variant
    std::array<Ramp, 3>  field_ramps;    // [0]=scalar [1]=signed-scalar [2]=vector-magnitude
    std::array<Rgba, 8>  chart_series;
  } data;
  struct Metrics {
    float rounding = 2.0f, padding = 6.0f, spacing = 6.0f,
          border_size = 1.0f, font_size = 15.0f;   // DIPs; see §7 for expansion
  } metrics;
};
```

### 3.4 Snapshot and command flow

```
  SIM THREAD (owns RenderBridge)                     GUI THREAD
  ──────────────────────────────                     ──────────
  for (;;) {
    bridge.tick()                                    acquire() ─► shared_ptr<const UiSnapshot>
      ├─ (CPU) update_energy_ledger()  [never gated]        │
      └─ (GPU) ledger sync/update iff demanded — §2.2       ├─ panels draw from it
                                                            │
    ── TICK BOUNDARY ──────────────────────────────         ├─ shell appends to History
    1. drain queue; apply MUTATIONS  ◄───────────────────────┤ panels push UiCommand
    2. flush_host_mutations()      [fixed point]            │
    3. execute retained OBSERVATION requests                │
    4. scheduler.on_tick_complete(bridge); pump(bridge)     │
    5. build immutable UiSnapshot                           │
    6. publish(shared_ptr<const UiSnapshot>) ───────────────┘
  }
```

⚠ **The drain sits in the sim loop, outside `bridge.tick()`.** Revision 1's diagram nested it under `tick()`, between the ledger and the publish — an implementer following that literally could drain from inside `RenderBridge` and land a toggle one phase late, which is exactly the failure §11 calls invisible to goldens. The rule in prose: **commands are drained by the sim loop after `bridge.tick()` returns and before the next call; no drain may occur inside `RenderBridge` or any phase.** §9.5 N4 asserts it, with a negative control that fails when the drain is moved inside the tick.

Steps 1–4 are ordered, not interchangeable. Draining first partitions commands into mutation and observation requests while preserving FIFO sequence numbers. Mutations apply before the fixed flush; observation requests execute only after it. The flush precedes `on_tick_complete()` so the want-mask cannot change flush timing on either backend (§2.2).

`Pause`, `Run` and `Step` are loop-control commands, not bridge observations. They apply in FIFO order with mutations during step 1, but `Step{n}` only increments a pending-step counter; it never calls `bridge.tick()` from inside the drain. The sim loop consumes pending steps on subsequent iterations. Consequently every mutation in the same drain, including one sequenced after `Step`, applies before the scheduled tick. A caller that needs "step, then mutate" must enqueue the mutation after observing the stepped snapshot in a later drain. Tests pin this rule.

**Immutable snapshot publication — correctness before lock-free cleverness.** `SnapshotPublisher` owns a `std::mutex` and the latest `std::shared_ptr<const UiSnapshot>`. `publish(UiSnapshot)` constructs the next immutable snapshot off-lock, takes the mutex only to replace the shared pointer, and releases it. `acquire()` copies the shared pointer under the same mutex. Readers can retain snapshots for any number of frames; writers never mutate a published object. The Phase-0B stress test asserts checksum integrity and monotone sequence under concurrent publish/acquire. Allocation count and lock-free replacement are profiling questions, not v2.0 contracts.

`UiSnapshot` extends today's `NativeFrame` with: `TelemetrySnapshot` (all four groups **and** their per-group metas), `EnergyLedger`, `VoxelInspection` + `ForceDiag` for the selected voxel, model-owned `ContinuitySnapshot` and `charge_sum` results when requested, `VisualFieldSample` for the active field, the current `TermToggles` **named `term_toggles`** (§9.3 — the field name must not collide with the lint's banned token), the six `RenderBridge`-level knobs, an `EnvInfo` block (backend kind, `interactive_gpu_mode`, thread count), `last_applied_seq`, and a monotone `seq`.

**Commands — the complete variant.** One row per journal key, with the exact target of each alternative. Key namespaces are `enum class`, so a bad key is a compile error rather than a runtime string miss.

```cpp
enum class DoubleKey  { langevin_T, langevin_gamma, coulomb_charge_coupling,
                        coulomb_source_scale, omega0, kinetic_drain,      // toggles.*
                        genesis_threshold_override, manifest_scale_override }; // bridge.*
enum class EnumKey    { bcc_stencil, langevin_site_filter };              // toggles.*
enum class UIntKey    { langevin_seed };                                  // toggles.*
enum class BoolCfgKey { manifest_use_temperature };                       // bridge.*

struct SetToggle        { std::string name; bool value; };  // owning TOGGLE_SPECS name
struct SetToggleProfile { TermToggles profile; };           // atomic, pre-validated
struct SetDouble        { DoubleKey key; double value; };
struct SetEnum          { EnumKey key; int value; };
struct SetUInt          { UIntKey key; unsigned value; };   // Langevin seed only
struct SetBoolConfig    { BoolCfgKey key; bool value; };
struct SetBoundary      { FluxBoundaryMode mode; };         // toggles.flux_boundary
struct SetDt            { double dt; };
struct SetSorIterations { int n; };
struct LoadScenario     { std::string id; };                // immediate structured reload
struct SetLatticeSize   { int n; };                         // stage pending reboot value
struct ApplyReboot      { };                                // apply staged lattice reload
struct ResetToDefaults  { };                                // shipping TermToggles{}
struct InspectVoxel     { int x, y, z; };
struct InspectForce     { int x, y, z; };
struct RequestField     { VisualFieldKind kind; int stride; };
struct RequestContinuity{ };
struct RequestChargeSum { };
struct SetTelemetryDemand { DataNeeds needs; };             // → scheduler Demand
struct Pause { }; struct Step { int ticks; }; struct Run { };

using UiCommand = std::variant< /* alternatives above */ >;
```

Rules that make the variant safe:

- **`SetToggle::name` must resolve through `find_spec()`** (`term_toggles.h:263`) in the applier before apply; an unresolved name is a hard error, journalled and surfaced, never silently ignored. `manifest_use_temperature` is a bool but **not** a `TOGGLE_SPECS` row, which is why `SetBoolConfig` exists.
- **`SetToggleProfile` is the only way to reach a profile that single-toggle steps cannot** (W18). The Physics panel computes the target profile, runs `validate()` on it **before** enqueueing, and refuses to enqueue an invalid one. `SetToggle` is reserved for edits that `validate()` clean in one step.
- **Reload semantics are unambiguous.** `SetLatticeSize` changes only a staged UI-model value. `ApplyReboot` performs one structured reload using that value. `LoadScenario` performs an immediate structured reload at the boundary. Neither calls `dispatch_scenario()` after `boot()` if `boot()` already dispatched it. The session API returns distinct outcomes for unknown id, validation rejection, device/interop failure and success.
- **The UI does not call `RenderBridge::seed_rng()`.** `SetUInt{langevin_seed}` changes the `TermToggles` seed. When Langevin is active, existing `phase_write` logic observes that change and reseeds `BridgeRng` at the next tick; journal replay reproduces that engine-owned consequence. Scenario initial conditions retain their separate fixed reset protocol (§2.1 C4).
- **Ordering and coalescing, stated so they do not contradict.** The queue is FIFO. **Physics-parameter commands are excluded from coalescing entirely**: `SetToggle`, `SetToggleProfile`, `SetDouble`, `SetEnum`, `SetUInt`, `SetBoolConfig`, `SetBoundary`, `SetDt`, `SetSorIterations`. Only idempotent view/request commands coalesce (`RequestField`, `SetTelemetryDemand`, `InspectVoxel`, `InspectForce`), and there coalescing keeps the **last write at the last occurrence's position**. ⚠ This matters exactly where W4 bites: a drain containing `SetToggle{symplectic_leapfrog,true}`, `SetDt{0.5}`, `SetToggle{symplectic_leapfrog,false}`, `SetDt{0.7}` must apply all four in order, because `set_dt()`'s clamp depends on which integrator toggles have already been applied. This case is named in the §9.1 L0 command-ordering test.

### 3.4b Snapshot builder and command applier

Revision 1's diagram showed a sim thread publishing snapshots and draining commands, and never said which class did either — while `NativeEngineSession` (`engine_session.h`) exposes no `RenderBridge` accessor, no toggles, no telemetry, no `energy_ledger()`, no `inspect_voxel()`. This is the load-bearing half of the phase the risk register calls Critical, so it is specified here.

| Component | Target | File | Signature |
|-----------|--------|------|-----------|
| Snapshot builder | `ftd_native_session` | `src/ui_snapshot_builder.{h,cpp}` | `void build_snapshot(RenderBridge&, const NativeTelemetryScheduler::CachedView&, const DataNeeds&, UiSnapshot& out);` |
| Command applier | `ftd_native_session` | `src/command_applier.{h,cpp}` | `ApplyResult apply_mutation(NativeEngineSession&, const UiCommand&, ParameterJournal&);` |
| Observation service | `ftd_native_session` | `src/command_applier.{h,cpp}` | `ObservationResult observe(NativeEngineSession&, const UiCommand&);` |
| Journal | `ftd_native_session` | `src/parameter_journal.{h,cpp}` | append / export / replay (§2.3) |

Binding rules:

- Both are **sim-thread-only**. Both link `ftd_native_ui_model` and `ftd_core`. Neither links `ftd_native_ui`; transport ownership therefore follows the dependency graph rather than contradicting it.
- **`NativeEngineSession` never exposes `RenderBridge`.** It grows boundary operations for partitioned drain/apply/flush/observe/publish, all called from inside its own tick loop. No `RenderBridge& bridge()` accessor is added.
- `ApplyResult` carries `{sequence, ok, error_code, message}` for command application. `TickResult` is a separate channel because `tick()` runs outside any individual command application. A caught W18 `std::logic_error`, scheduler timeout or device failure becomes a `TickResult` status-bar error, never a fabricated result attached to the last command.
- Reload is session-owned. `ReloadResult` distinguishes unknown scenario, validation-rejected scenario, backend recreation failure, interop re-import requirement and success. The GUI thread receives a presenter action (`RebindInteropSrv` or `DisableInterop`) rather than allowing `command_applier.cpp` to manipulate D3D12 descriptors or NT/fence handles it does not own.

### 3.5 Presenter integration

Verified against the current `render()`:

- **Bootstrap context:** after presenter initialization and before the first frame, `ui_backend_context()` returns a non-owning `PresenterUiContext` containing the live `ID3D12Device*`, `ID3D12CommandQueue*`, shader-visible SRV heap, RTV/DSV formats, `kFrameCount`, and descriptor allocate/free callbacks. It is valid until presenter shutdown or device recreation. Device recreation emits `UiBackendInvalidated` before releasing those objects; ImGui tears down and reinitializes from the replacement context before another overlay record.
- **Debug observability is configuration-driven, not `_DEBUG`-driven.** `PresenterOptions::enable_debug_layer` may enable the D3D12 debug layer in a Release test binary before device creation. When enabled, `debug_messages()` exposes filtered `ID3D12InfoQueue` errors/warnings after a fence retirement. Shipping defaults it off.
- **Insertion point:** invoke `OverlayRecorder::record(ID3D12GraphicsCommandList*, const RenderTargetInfo&)` between the interop `DrawInstanced` and the final `RENDER_TARGET → PRESENT` barrier. The application-owned recorder calls `ImGui_ImplDX12_RenderDrawData()`; the presenter never includes ImGui.
- **Rebind RTV with a NULL DSV** at that seam. `render()` binds a `D32_FLOAT` DSV, while ImGui's backend PSO is built with `DSVFormat = DXGI_FORMAT_UNKNOWN`; the debug layer flags the mismatch.
- **ImGui draws last, always.** Its backend clobbers root signature, PSO, descriptor heaps, IA buffers, topology, viewport, scissor and blend factor. Harmless only because nothing follows it — no presenter draw may ever be added after it without re-establishing state.
- **Frame-in-flight bound must agree, indices need not.** `impl_->frame` is the swapchain back-buffer index; ImGui's backend keeps its own monotonic counter and does **no fencing of its own**, assuming the app guarantees the submission from `num_frames_in_flight` calls ago has retired. The presenter's per-slot wait guarantees exactly that for 2. Therefore: pass `num_frames_in_flight = kFrameCount = 2`, and call `RenderDrawData` **exactly once per `render()`**. Calling it twice in one frame reuses a buffer inside the same in-flight window and corrupts geometry.
- **Three presenter seams are distinct.** (1) `set_overlay_recorder(OverlayRecorder*)` installs a non-owning recorder whose lifetime is removed before UI shutdown; recording occurs on the open graphics list before its final PRESENT barrier. (2) `request_capture(CaptureRegion)` returns a monotone `CaptureToken`; the next render records `RENDER_TARGET → COPY_SOURCE → RENDER_TARGET` plus the readback copy in that same list before the final PRESENT transition. Only one request per token is recorded. (3) `poll_capture(CaptureToken)` returns `Pending`, `Ready{width,height,row_pitch,bytes}`, or `Failed{reason}` after checking the submission fence. No callback records commands after `ExecuteCommandLists`.
- **SRV heap must grow.** It is currently sized `NumDescriptors = 1`, and the interop draw binds it. ImGui needs at least a font-atlas SRV, and post-1.91.5 requires app-supplied alloc/free callbacks. Grow to 256 with a small free-list, with **named reserved slots: index 0 = the existing interop `StructuredBuffer` SRV; index 1 = the ImGui font atlas.** Slots 2+ are the free-list.
- **The swapchain moves to the top-level HWND in Phase 1**, after the presenter accepts an explicit scene rectangle. The contract covers viewport and scissor origin/extent, depth-target extent, clear policy outside the scene, camera aspect from the scene extent, client-to-scene pointer transformation, and rejection of input in the Win32 control strip. Until that slice lands, the child HWND remains the swapchain owner.

### 3.5b DPI: the mechanics

⚠ **`io.DisplayFramebufferScale` does not scale ImGui layout.** It scales the framebuffer relative to `DisplaySize` for the backend's clip rects. Revision 1 proposed an L1 matrix over it as the DPI mitigation; that matrix produces identical widget geometry and asserts nothing about DPI. ImGui DPI is done by rebuilding the font atlas at `font_size × dpi_scale` and calling `ImGuiStyle::ScaleAllSizes(dpi_scale)`.

- **Awareness mechanism:** `SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)` called before any window creation. No manifest (the app has neither today, verified).
- **`WM_DPICHANGED` sequence**, in this order, at a frame boundary **after** the presenter's per-slot fence wait — never mid-frame, and never inside the 2-frame in-flight window:
  1. resize/reposition the window from the suggested rect;
  2. `ImGui_ImplDX12_InvalidateDeviceObjects()`;
  3. `io.Fonts->Clear()` → `AddFontFromMemoryCompressedTTF(..., font_size × dpi_scale)` → `Build()`;
  4. `ImGui_ImplDX12_CreateDeviceObjects()`, re-binding the atlas SRV to **reserved slot 1**; the retired font texture goes through the same deferred-release path as other resources;
  5. **re-apply the `Theme` from scratch** (`apply(theme)`), *then* `ImGuiStyle::ScaleAllSizes(dpi_scale)`.
- ⚠ Step 5's order is not optional: **`ScaleAllSizes` is not idempotent**. Scaling a style that was already scaled compounds sizes on every monitor change.
- **The DPI test matrix (L1)** is over `{font_size × scale, ScaleAllSizes(scale)}` for `scale ∈ {1.0, 1.5, 2.0}`, asserting that panel content bounding boxes scale proportionally and that no window's content exceeds `DisplaySize`.

### 3.5c Win32 coexistence, Phase 1 through Phase 3b

After the Phase-1 swapchain move, the app is one top-level HWND hosting the D3D12 viewport plus the existing Win32 control strip as sibling children. Until Phase 4 retires them:

- `ImGui_ImplWin32_WndProcHandler` is installed **first** in `wnd_proc`; unhandled messages fall through to the existing handling.
- ImGui is suppressed from consuming keyboard input while `is_edit_focus()` is true (`main.cpp`'s existing focus hack), so the `EDIT` and `LISTBOX` controls keep working.
- The explicit scene rectangle is the same client rect the old child view occupied; viewport, scissor, camera aspect and pointer coordinates all use it. The control strip keeps its rect and cannot receive scene input. Phase 4's last task deletes the strip and expands the scene/dockspace to the full client area — a rect change, not a window change.
- Camera orbit/zoom/pick handlers run only for coordinates inside the scene rectangle and only when `io.WantCaptureMouse == false`; keyboard shortcuts run only when `io.WantCaptureKeyboard == false` and no Win32 edit has focus. Mouse capture is released on focus loss, cancellation, or when ImGui begins capturing during a drag. L1 message-sequence tests assert that dragging a panel, scrolling a table, editing text and opening a popup cannot orbit, zoom, pick, pause or step the simulation.

### 3.6 Engine hazards implementers must respect

Discovered during design research and re-verified for revision 2. Each has bitten or would bite.

- **W1** ⚠ **`const` on `RenderBridge` means nothing about mutation.** `voxels() const` triggers a full device→host mirror; `su2_links_x() const` allocates ~132 MiB at L=64; `strong_stress_cells() const` recomputes everything; `phi_latency() const` does a `const_cast` and a device mirror. "It's const, so a panel can call it" is **false everywhere**. This is precisely why D6 exists.
- **W2** **`TOGGLE_SPECS` has no group/category column.** `ToggleSpec` is `{name, field, default_value, bulk_managed, requires_, conflicts, gpu_only_warning, backends, description}` — grouping is UI-side (`name → group` map, decided in §13 D-Q2). Do not write code assuming grouping metadata exists.
- **W3** **`TermToggles::validate()` Pass 2 encodes cross-cutting guards emitting 13 distinct error strings**, none of them derivable from the `requires_`/`conflicts` columns (`term_toggles.h:329-403`). A UI that greys checkboxes purely from the table will permit invalid profiles. Always call `validate()` on the *target profile* and surface `err`.
- **W4** **`set_dt()` clamps `dt < 1.0` to 1.0** unless `symplectic_leapfrog` or `verlet_wave_integrator` is on (`render_bridge.cpp:511-531`); `dt > 1.0` always applies. Either Floquet toggle forces `dt_ = 1.0` unconditionally, both in `set_dt()` and again inside `tick()` (`render_bridge.cpp:784-791`). Command ordering within one drain is therefore semantically significant, and the journal must record the **applied** value (§2.3).
- **W5** ⚠ **No Scale-0 physics toggle still forces a mid-tick CPU backend switch.** `matched_gauss_dynamics`, `strong_stress_energy`, Verlet, both Floquet prototypes, `symmetric_movement_order`, and `cluster_inertia` are native CUDA. A lone `SetToggle{"matched_gauss_dynamics", true}` still throws from the next `tick()` because isolation `validate()` fails against shipping defaults — that is a profile error, not a backend fallback. Under `FTD_FORCE_GPU` there is no CPU escape hatch (W24).
- **W6** **`knot_tracking` throws `std::logic_error` out of `GpuBackend::tick()`** when `interactive_gpu_mode && L > 64`. `cluster_inertia` is native CUDA. Pre-check with `validate_backend(GPU, require_device_resident=true)`; do not discover this via an exception landing in `frame.status`.
- **W7** **`su2_gauge`/`su3_gauge` allocate 528 B/site lazily** on first enabled tick (~132 MiB at L=64, larger than the voxel array). A checkbox click is a large allocation — show it.
- **W8** **`dual_substrate` defaults true**, making `langevin`, `db_clock_coulomb` and non-`FULL` `bcc_stencil` unreachable from the default profile. Dependency arrows must express the *mutual exclusion direction* or they look broken.
- **W9** ⚠ **`dispatch_scenario()` validates LAST, and its `false` return has two very different meanings.** Revision 1 stated this backwards. The unknown-id check returns *before any mutation* (`scenarios.cpp:213-216`) — a clean refusal. But for a known id the scenario body runs first (writing toggles, injecting voxels; `scenarios/s0_seed.cpp:969` zeroes the entire toggle registry), and only then does `accept_profile` call `rb.toggles.validate()` and return `false` (`scenarios.cpp:218-228`). **A validation `false` therefore means the live simulation has already been half-mutated into a profile the engine itself rejects.** `NativeEngineSession::boot()` compounds it: on `false` it falls back to `"demo-pair"` and calls `seed_visible_pair()` (`engine_session.cpp:116-119`) — seeding a pair *on top of* the rejected scenario's toggles and injections — and it uses the same branch for both failure modes. **v2.0 requirement:** treat validation-rejection as fatal to current state and re-boot to a known-good scenario; and any UI caching toggle widget state across a scenario load shows stale lies.
- **W10** **`strong_force` and `exchange_force` are native on CPU and CUDA** (shared pairwise helpers). `cpu_runtime_warnings()` is empty for those rows. Backend kind remains part of run identity (§2.3) for the FFT-vs-SOR Poisson contract.
- **W11** **On the CPU backend, `begin_telemetry_snapshot()` computes all requested groups synchronously** — and it does so by calling `bridge_.diagnostics()`, `bridge_.energy_audit()` and `bridge_.gravity_metric_agg()` (`backend.cpp:162-178`), three Category-2 flushing observers. The begin/poll API looks async but is not on CPU; requesting `TELEMETRY_ALL` per frame is four O(N) sweeps per frame there, *and* it makes the want-mask control flush timing. §2.2's fixed-point flush neutralises the second half; the scheduler's per-group cadence neutralises the first.
- **W12** **Field kinds are not uniformly costly.** `Kretschmann` and `Latency` run an un-strided full-lattice max-ρ prepass, and `Kretschmann` additionally allocates an N³ float grid plus an 18-point stencil per sample. `FluxVector` scans stride³ voxels per output cell — small output, ~O(N³) read cost. These drive the §2.2 cadence rule and the §5.4 cost badges.
- **W13** **`continuity_step()` allocates six L³ vectors per call.** One-shot diagnostic (Category 3), never a per-frame series.
- **W14** **`capture()` currently blocks up to 1 second** in a 5000 × 200 µs poll loop (`engine_session.cpp:37-43`) and unconditionally runs an ~O(N³) `append_flux` block-max scan every frame (`:208`). ⚠ It runs **exclusively on the sim thread** (`main.cpp:1001`); the GUI thread renders from the mutex-protected `latest` snapshot and never calls it. So left as-is it caps the **sim thread's tick rate**, not the UI frame rate — a 1-second stall there freezes tick advance, telemetry freshness, and every queued command drain.
- **W15** **`boot()` clears `interop_enabled_` on every reload.** Every scenario load, lattice change and reset must be followed by `reimport_interop_after_reload()` on the sim thread, plus the GUI-thread `bind_interop_particle_srv()` catch-up. §3.4's `LoadScenario`/`SetLatticeSize` are *defined* to include that sequence so no new reload trigger can bypass it.
- **W16** **Cross-thread CUDA works here only because** the Runtime API shares the device-0 primary context and the codebase never calls `cudaSetDevice()`. Any new worker thread touching CUDA inherits that undocumented assumption and breaks on multi-GPU.
- **W17** **`bridge_` is set to null mid-`boot()` with zero locking.** Reload paths must not race a reader.
- **W18** ⚠ **Two unconditional throws out of `tick()`, both reachable from a single checkbox click at shipping defaults.**
  1. `render_bridge.cpp:801` — `if (toggles.strict_validation || toggles.matched_gauss_dynamics)` throws `std::logic_error` on **any** `validate()` failure. `matched_gauss_dynamics` requires ~37 named toggles simultaneously false plus `flux_boundary == Periodic` (`term_toggles.h:371-388`), and shipping `TermToggles{}` has **12** of them true. A lone `SetToggle{"matched_gauss_dynamics", true}` therefore throws out of the very next `tick()`, guaranteed, from defaults.
  2. `render_bridge.cpp:822-830` — `matched_gauss_dynamics` with `dt_ != 1.0` throws unconditionally.
  **Consequences for the UI:** `matched_gauss_dynamics` is routed **exclusively** through `SetToggleProfile` carrying the full validated target profile plus `SetDt{1.0}` in the same atomic drain, behind the W5 confirmation. `strict_validation` is a **developer flag, presented outside the ordinary 43-row table**, because ticking it converts every subsequent transient invalid combination into a sim-thread exception. And the general rule: profiles like `db_clock_coulomb` (requires three toggles on, `dual_substrate` off, `forces` off — `term_toggles.h:250`, `:353`) are unreachable by any sequence of single-toggle FIFO commands without passing through invalid intermediate states, which is the entire reason `SetToggleProfile` exists.
- **W19** ⚠ **Six "observers" perform a host→device write on the GPU backend.** Enumerated with call sites in §2.2, Category 2. `const`-ness and the word "observer" both lie here.
- **W20** ⚠ **`mutable`-state sync is OpenMP-critical-guarded only, and is NOT `std::thread`-safe.** `sync_ternary_from_voxels_if_needed()` (`render_bridge.cpp:246-267`) guards its `mutable` dirty flag with `#pragma omp critical` **only when `omp_in_parallel()`**; called from a plain non-OpenMP thread it takes no lock at all. It is reached from `active_indices()`, `ordered_active_indices()`, `ternary_field()`, `engine_state()`, `fields()` and `curl_state_velocity()` — all `const`. Any GUI-thread call to one of these races the sim thread's `tick()`, which calls the same function at `render_bridge.cpp:851` and `:1135`. Compounding it, the **non-const** `voxels()`/`voxel_at()` (`render_bridge.h:137-158`) set both dirty flags *on handout*, before any mutation occurs. The §11 `FTD_UI_DEBUG_THREAD_GUARD` therefore covers **const observers too**, not only mutating methods — here the const ones are the dangerous ones.
- **W21** ⚠ **`update_energy_ledger()` is already skipped in interactive GPU mode.** `render_bridge.cpp:899-904` returns before the GPU-path call at `:906`; `engine_session.cpp:112` selects that mode. See §2.2 and §5.5.
- **W22** ⚠ **`begin_telemetry_snapshot()` is a commit point, not an observer.** GPU: assigns `engine_->toggles = bridge_.toggles` + three overrides + `flush_host_mutations()` (`backend.cpp:528-545`). CPU: calls three Category-2 observers synchronously (`backend.cpp:162-178`). Sim thread only, at the tick boundary.
- **W23** **`BridgeRng` is stateful and thread-count-sensitive.** See §2.1 C4(2). It has no tick-path consumer today; `rng_state_hash()` is nevertheless thread-count-dependent, and four `src/eft/` bounds compare it.
- **W24** **`force_cpu()` is a no-op when `FTD_FORCE_GPU` is set** (`render_bridge.h:293-297`), and `make_default_backend` honours `FTD_FORCE_CPU` symmetrically (`backend.cpp:610`). A test that calls `force_cpu()` and asserts CPU behaviour is running an unrelated third configuration under an ambient env var. §9.5 asserts the backend kind after forcing it, and §9.4 pins both variables unset.

---

## 4. Shell and information architecture

### 4.1 Regions

One dockspace filling the top-level window. ⚠ **The Viewport is the dockspace central node — never closable, never floating, never hidden.** Setup, Instruments and Physics are the **three dockable regions**, which is why §12 Phase 3a says "three docks" and §4.2's Presentation workspace can hide "all docks" while a full-bleed viewport remains.

| Region | Kind | Default contents |
|--------|------|------------------|
| **Menu bar** | shell chrome | File · View (workspaces) · Theme · Help. Carries theme and workspace switching, since D8 defers the settings panel. |
| **Setup** (left, ~236 DIP) | dock | Scenario browser · Run config |
| **Viewport** (centre) | dockspace central node | 3D scene + HUD: axis gizmo, field legend, hover/selection readout |
| **Instruments** (right, ~268 DIP) | dock | Telemetry · Audit · Lagrangian · Inspector |
| **Physics** (bottom, ~148 DIP) | dock | Term toggles · Fields · Log |
| **Status bar** | shell chrome | Backend · interop health · graph capture · tick · L · particle count · **profile-differs-from-defaults indicator** (§8) · **frame time and worst-panel cost** (debug-only, `FTD_UI_SHOW_FRAME_COST`) |

The three docks are movable, resizable, closable, and restored from the active workspace. The menu bar and status bar are **shell code**, not `Panel`s — `ImGuiWindowFlags_MenuBar` on the dockspace host and `BeginViewportSideBar` respectively — and **no workspace may hide the status bar** (§2.5).

All region dimensions are **DIPs at 100% scaling** and are multiplied by `PanelContext::dpi_scale`; 236 DIP is 472 physical pixels at 200%.

### 4.2 Workspaces

Named dock layouts, each persisted to its own `.ini`:

- **Experiment** — physics terms and run config forward.
- **Analysis** — charts and inspector maximised.
- **Presentation** — all three docks hidden; full-bleed viewport with legend, menu bar and **status bar** only. This is how "demoable second" is satisfied without compromising the dense default, and without breaking §2.5.

Workspace files are versioned with a migration path and must recover from a corrupt file by falling back to the built-in default rather than failing to start. A workspace persists the **open-set** (which panels are shown) alongside the ImGui `.ini`.

**DockBuilder recipe** — run **only** when no `.ini` exists or after an explicit workspace reset, guarded by `ImGui::DockBuilderGetNode(dockspace_id) == nullptr`. Ratios are fractions of the parent, computed against the §4.4 reference `DisplaySize` of 1920×1080 DIP:

```
DockBuilderRemoveNode(root); DockBuilderAddNode(root, Dockspace); DockBuilderSetNodeSize(root, {1920,1080})
  split root Left   0.123  → Setup        (236/1920)
  split rest Right  0.159  → Instruments  (268/1685)
  split rest Down   0.137  → Physics      (148/1080)
  remainder                → central node (Viewport, no window docked)
Setup:        Scenario browser and Run config are a VERTICAL SPLIT at 0.62 / 0.38
Instruments:  Telemetry · Audit · Lagrangian · Inspector are TABBED in one node
Physics:      Term toggles · Fields · Log are TABBED in one node
```

`DockBuilderDockWindow` is passed the composed `"Title###id"` string (§3.3), never `title()` alone.

### 4.3 Command palette (Ctrl+K)

One fuzzy search over **scenarios, toggles, fields, panels, and actions**, each result badged with its kind and its current state. With 59 knobs, 130 scenarios and 18 fields, this is the difference between "powerful" and "where is that toggle."

| Result kind | State string |
|-------------|--------------|
| Scenario | `loaded` · `available` · `min-lattice unmet (needs L ≥ n)` |
| Toggle | `enabled` · `disabled` · `unavailable on this backend` · `blocked by validate(): <first error>` · `mode switch — confirm` (the six W5 rows) |
| Field | `active` · `inactive`, plus a cost badge (`cheap` · `strided O(N³) read` · `prepass + N³ alloc`) |
| Panel | `visible` · `hidden` · `floating` |
| Action | `enabled` · `disabled: <reason>` |

**Ranking between kinds**, since one box over 59 knobs + 130 scenarios + 18 fields collides constantly: exact-prefix matches first, then by kind in the order Action → Panel → Toggle → Field → Scenario, then by fuzzy score, then alphabetically. Ties never reorder between frames.

The palette's index is **extended by each later phase**, and that extension is an explicit exit criterion of Phases 4, 5 and 7a (§12): Phase 3b ships panels + actions, Phase 4 adds toggles, Phase 5 adds fields, Phase 7a adds scenarios.

> **Ported from the web dashboard, improved.** Over there, controls are scattered across five separate chrome surfaces — toolbar, Controls panel, in-viewport Visualization grille, status-bar `<details>` menus, and a floating play-bar popover — with no unified search. One box here reaches all of them.

### 4.4 Density

Default layout targets a **1920×1080 DIP** window at 100% scaling with every region populated. The checkable density claim: **Run config, the toggle table's group headers, and the instrument panel headers require no scrolling at that reference size**; list panels (scenario browser, log, and the expanded toggle table) scroll by design — a category-grouped list of 130 scenario ids in a 236 DIP column scrolls by construction, and pretending otherwise would be a claim no test could hold. `ImGuiStyle` spacing is theme-driven so a future compact mode is a theme edit, not a layout rewrite.

---

## 5. Panel catalogue

Field lists are taken from the actual headers so panels can be built without re-deriving them. ⚠ **Where a list is long, the header is the normative source and the coverage test reads it** — see §9.8. A phrase like "the dual-substrate block" is not a specification and does not appear below.

### 5.1 Scenario browser

- Live substring + fuzzy search over `scale0_scenario_ids()` (**130** ids, `scenarios.cpp:64-201`).
- Grouped by category; selected scenario shows description, min lattice, admission status.
- ⚠ **The C++ side has ids only** — no title, category, tags, or description. Rich metadata is JS-only (`engine/web/js/scales/scale0/scenario-registry.js`). ⚠ **Correction to revision 1:** the two lists are *not* verified set-identical by any test. `engine/web/tests/scenario-parity.spec.js` compares the JS catalog against `case '…'` labels and `name == "…"` branches in `src/scenarios.cpp` + `src/scenarios/*.cpp` (`extractCppScenarios`, lines 92-115) — **it never reads `ftd::scale0_scenario_ids()`**. That vector has exactly three consumers today: the dispatcher's own id check (`scenarios.cpp:213`), the current Win32 scenario listbox (`native_desktop/src/main.cpp:684`), and one test that iterates it for a different purpose (`engine/tests/test_toggle_matrix.cpp:257`). **No test compares it to the JS registry.** Both hold 130 entries today by maintenance discipline, not by a gate.
  **Therefore v2.0 introduces `engine/include/ftd/scenario_meta.h`** as the **single source of truth**, with a parity test that covers `scale0_scenario_ids()` explicitly (a §9.8 deliverable, not an existing property). This directly addresses the open question in `engine/web/docs/adr/0002-scenario-architecture.md` (Direction B: unify to one descriptor) and lets the JS registry become a generated artifact later.

```cpp
// engine/include/ftd/scenario_meta.h — mirrors the ToggleSpec precedent:
// const char* throughout, comma-separated lists, constexpr table.
struct ScenarioMeta {
  const char* id;                 // must equal an entry of scale0_scenario_ids()
  const char* title;              // from the registry's `sourceTitle`, UNdecorated
  const char* category;
  const char* description;        // "" ⇒ unauthored; rendered as an honest placeholder
  const char* tags;               // comma-separated; iterate with for_each_csv()
  const char* epistemic_status;
  const char* admission_status;
  int         scale;              // 0 for every v2.0 row (D3 forward-compatibility)
  int         min_lattice;        // 0 ⇒ unconstrained
};
inline constexpr ScenarioMeta SCENARIO_META[] = { /* 130 rows */ };
```

Sourcing rules, because two of the eight fields **do not exist to be generated**:
- `title` comes from the registry's `sourceTitle` (`scenario-registry.js:16`), not its decorated `title` (`"… — Research Setup (Behavior Unvalidated)"`). The admission decoration is applied **at render time** from `admission_status`.
- `description` has no registry field — the prose lives in freeform JS block comments above each `makeScenario()` call. Initial rows ship with `description = ""`, which the UI renders as *"No description authored yet"*, never as fabricated prose.
- `min_lattice` exists nowhere, in the registry or in C++. Initial rows ship `min_lattice = 0` (unconstrained) and are owner-authored incrementally.

- ⚠ Handle `dispatch_scenario()` returning `false` (W9) with a visible error and a re-boot to a known-good scenario on the validation-rejection path — never a blank lattice, never a half-mutated bridge.

### 5.2 Run config

Owns **exactly the 6 `RenderBridge`-level knobs** and nothing else:

`lattice_size` (free integer 4–256, not the current 6 presets) · `dt` · `sor_iterations` · `genesis_threshold_override` · `manifest_scale_override` · `manifest_use_temperature`.

⚠ `langevin_seed` and `flux_boundary` are **`TermToggles` members** (`term_toggles.h:133`, `:144`) and belong to §5.3, which owns all ten non-bool config fields. Revision 1 listed them here as well, giving one engine field two widgets, two command paths and two journal keys. Run config shows them as **read-only mirrors with a "go to Physics terms" link**, so the ergonomics survive without the duplication.

Changes requiring a reboot (`lattice_size`) are marked and batched behind an explicit **Apply** (`ApplyReboot`), which is defined to include the W15 reimport sequence.

### 5.3 Physics terms

All **43** `TOGGLE_SPECS` rows in a searchable, grouped table. Each row shows `description`, and derives state from `requires_`, `conflicts`, `backends` (CPU/GPU/JS bitmask) and `gpu_only_warning`. Plus **all 10** non-bool `TermToggles` config fields: `bcc_stencil`, `langevin_site_filter`, `langevin_T`, `langevin_gamma`, `langevin_seed`, `coulomb_charge_coupling`, `coulomb_source_scale`, `omega0`, `kinetic_drain`, `flux_boundary`.

**Edit protocol**, binding:

1. The panel computes the **target profile** as a whole `TermToggles` value.
2. It calls `validate()` on that target and **refuses to enqueue** if it fails, showing `err` (W3).
3. If the target is one atomic step from the current profile, it enqueues `SetToggle`/`SetEnum`/`SetDouble`/`SetUInt`/`SetBoundary`. Otherwise it enqueues `SetToggleProfile` (W18).
4. Rows carrying W5–W8 hazards are annotated inline; the six W5 rows are presented as **mode switches behind a confirmation**, with the interop consequence spelled out.
5. `matched_gauss_dynamics` is only ever reachable via `SetToggleProfile` + `SetDt{1.0}` in one drain (W18).
6. `strict_validation` is **not a row in this table** — it is a developer flag in a separate "Diagnostics" group with an explicit warning, because it converts every transient invalid combination into a sim-thread exception (W18).

### 5.4 Fields

All 18 `VisualFieldKind` values: `Electric, Magnetic, Poynting, Divergence, FluxVector, Vorticity, Helicity, Kretschmann, Latency, Fisher, Coherence, Curl, State, GaussResidual, EmForce, GravityForce, StrongForce, PoissonLatency`. Stride control, display-only threshold, ramp choice, and a legend showing **requested stride, effective stride, engine sampling floor, UI display threshold, sample count and `origin`, always** (P2). The threshold can only remove already-returned samples; it is not passed to the engine sampler. Expensive kinds (W12) are badged with their cost, and the two prepass kinds carry the §2.2 rate limit.

**Ramp ownership and precedence**, since three sections touch it:

> The **theme** supplies the available ramp *set* — `data.ternary` (which must include a `cvd_safe` variant, P5) and `data.field_ramps[3]`, keyed by **component class** (`0` scalar, `1` signed-scalar, `2` vector-magnitude), not by all 18 kinds. The **Fields panel** selects within that set. The selection is per-session state persisted in §8, and is reset only if a newly-activated theme does not offer the selected ramp.

### 5.5 Telemetry / Audit / Lagrangian / Ledger / Gravity

ImPlot series with rolling history from the GUI-side `History` (§3.3b), each plotted against **its own group meta tick** (P7), each gated interval rendered as a **gap** (P4).

- **`Diagnostics`** — `tick, total_flux, total_energy, avg_drag, max_bandwidth, max_causal_budget, causal_projection_events, manifested_count, positive_count, negative_count, total_entropy, spin_up_count, spin_down_count, color_count[4], total_angular_momentum`.
- **`EnergyAudit`** — ⚠ **34 members**, not the 40 revision 1 claimed (`render_bridge_diagnostics.h:84-137`). Rather than a prose list that goes stale, the normative rule is: **the panel charts every scalar member of `EnergyAudit` as declared in the header, and §9.8's coverage test asserts one series per member.** The blocks, for orientation: base energy/Gauss (`field_energy, wave_energy, particle_ke, total_energy, gauss_violation, max_gauss_error, self_field_injection, coulomb_pe, E_field_energy, B_field_energy, charge_total, manifested_count, total_poynting`), dual-substrate (`E_L_total, E_R_total, wv_L_total, wv_R_total, chirality_total`), strong/weak (`strong_energy, weak_energy`), FTD-0402 mass-role (`particle_rest_energy, particle_energy, particle_momentum, dynamic_energy`), FTD-0404 density metadata (`cell_volume, field_energy_density_sum, wave_energy_density_sum`), FTD-0406 strong-projection (`strong_potential_energy, strong_gravitational_mass, strong_projection_residual, strong_projection_lambda, strong_projection_events, strong_projection_failures, strong_topology_failures`).
- **`EnergyLedger`** — all 11 members: **`updates`, `tick_prev`**, `E_prev, E_curr, dE_dt, drift_frac, expected_rate, residual, cumulative_injection, cumulative_dissipation, max_residual_seen`. `tick_prev` and `updates` are freshness detectors, not decoration. Two mandatory states:
  - **Interactive GPU mode:** opening or expanding the panel requests the D11 synchronization mode. The status bar and panel show **"GPU ledger synchronization active"**, the most recent synchronization duration, ledger epoch and source tick. Collapsing, hiding or closing the panel disables the mode after the next boundary; history records a gap. Re-enabling resets and seeds a new epoch; the seed snapshot is visibly marked and is not charted as a derivative/residual sample.
  - ⚠ **`selective_damping` true (the shipping default):** `expected_rate`, `residual` and `max_residual_seen` are badged approximate and `residual` is not presented as a conservation violation (P8).
- **`TelemetryLagrangian`** — all 17 members (`telemetry_snapshot.h:48-66`): the seven term sums (`field_kinetic_sum, field_gradient_sum, born_infeld_sum, coupling_sum, velocity_coupling_sum, gauss_sum, dissipation_sum`), plus `total_lagrangian, total_hamiltonian, total_action, gauss_violation, max_gauss_error, total_flux_mag, total_wave_energy, manifested_count, locked_count, cell_volume`. ⚠ `total_hamiltonian` is charted but is **excluded from any "total energy" framing** (P6) and carries the `lagrangian.h:215-218` caveat as its tooltip.
- **`GravityMetricAgg`** — `latency_max/mean, f_min, gamma_max, dilation_max_pct, voxel_count`, with `requested`/`active` driving an explicit "not enabled" empty state.

### 5.6 Inspector

Driven by viewport picking, via the Category-3 `InspectVoxel`/`InspectForce` commands. **Normative rule:** the panel renders every member of `VoxelInspection` and `ForceDiag` as declared in `render_bridge_diagnostics.h`, and §9.8's coverage test asserts one row per member. For orientation that is the embedded `Voxel` (state, flux, wave_vel, dual-substrate L/R, velocity, remainder, latency, tau, phase, locked, particle_id, pair_id, spin, color, flavor, accel_mag, and the strong/weak channels as declared), plus `divergence`, `curl`, the `EMFieldDiag`, and `ForceDiag`'s `f_coulomb, f_strong, f_magnetic, f_gravity, f_exchange`.

### 5.7 Log

Structured, severity-tagged, filterable, copyable. Replaces the console window and the single `status` string rendered in grey text identical to normal status. Carries every §2.5 state change, every rejected command, every `validate()` error, and both W18 throws.

---

## 6. Viewport and renderer

Second implementation plan. ⚠ **Revision 1 left this as a coherence note while two phases depended on it and two binding §2 clauses had no owner.** The cross-plan dependency is now explicit and per-phase:

| §12 phase | §6 deliverable it requires | Consequence if the renderer plan slips |
|-----------|---------------------------|----------------------------------------|
| **Phase 5** | Field render modes (point / arrow-glyph / slice-plane), HUD field legend with numeric bounds, **P1** alpha-sort/depth-write fix, **P5** colourblind-safe ternary ramp | Phase 5 cannot start; its exit criterion is unreachable |
| **Phase 6** | Click-to-pick → voxel coordinate, hover coordinate readout | Phase 6 ships charts only; Inspector stays keyboard/coordinate-entry driven |

P1 and P5 are binding §2.4 clauses and are hereby **assigned to Phase 5**; they are not optional renderer polish.

- **Camera:** add **pan** (currently impossible — orbit-only, fatal once zoomed into a 49³ lattice), adjustable sensitivity, orthographic projection, axis-aligned preset views, and a distance clamp that does not snap on the first wheel event at large L.
- **Picking:** click → voxel coordinate → `InspectVoxel` command → Inspector. Hover shows a coordinate readout.
- **HUD:** axis gizmo, field legend with numeric bounds (P3), selection highlight.
- **Correctness:** fix P1 (alpha sort / depth writes). Add MSAA.
- **Field rendering:** point, arrow-glyph and slice-plane modes. ⚠ **Correction to revision 1:** `copy_visual_flux_magnitude_plane` is *not* unused — it has two live consumers, `src/ws_server.cpp:1353` (the native WebSocket path serving the web dashboard) and `engine/tests/test_gpu_compact_diagnostics.cpp:153-154` (a CPU/GPU parity assertion). It is unreached from the native app only, and it already carries parity coverage this plan can lean on.
- ⚠ `copy_visual_field_sample()` is **non-const and sim-thread only** (`render_bridge.h:330`; it calls `flush_host_mutations`). Field sampling goes through the command/snapshot path like everything else, at the §2.2 cadence.

---

## 7. Theming

A `Theme` is **semantic roles**, never raw ImGui enums. The literal declaration is in §3.3b. This section specifies the two things that decide whether two implementers ship the same Graphite: the **role → ImGui mapping** and the **file format**.

### 7.1 Role → ImGui mapping

`apply(const Theme&)` is the only function in the codebase permitted to write `ImGui::GetStyle()` or an `ImGuiCol_`/`ImPlotCol_` value (enforced by the §9.3 theme-token lint). It maps ~14 roles onto the ~55 `ImGuiCol_` entries of the pinned ImGui version (§10 pins the SHA, so the enumeration is exact) by this table plus one derivation rule.

**Derivation rule for interaction states**, applied to every `*Hovered` / `*Active` / `*Selected` entry:

```
Hovered  = lighten(base, 0.08)      // per-channel, toward white in sRGB
Active   = lighten(base, 0.16)
Disabled = mix(base, surface_1, 0.55)
```

| Role | `ImGuiCol_` entries fed |
|------|-------------------------|
| `surface_0` | `WindowBg`, `DockingEmptyBg` |
| `surface_1` | `ChildBg`, `PopupBg`, `MenuBarBg`, `ScrollbarBg`, `TitleBg`, `TitleBgCollapsed`, `TabUnfocused`, `TableRowBg` |
| `surface_2` | `FrameBg`(+Hovered/Active), `Button`(+Hovered/Active), `Header`(+Hovered/Active), `TitleBgActive`, `Tab`, `TabUnfocusedActive`, `TableHeaderBg`, `TableRowBgAlt`, `ScrollbarGrab`(+Hovered/Active), `ResizeGrip`(+Hovered/Active) |
| `surface_3` | `TabActive` (`TabSelected` on ≥1.91.5), `NavWindowingDimBg`, `ModalWindowDimBg` (at α 0.55) |
| `text_primary` | `Text` |
| `text_secondary` | `PlotLines` label tint (via ImPlot `AxisText`) |
| `text_muted` | `TextDisabled` |
| `text_dim` | `BorderShadow` (α 0) |
| `border` | `Border`, `Separator`(+Hovered/Active), `TableBorderStrong`, `TableBorderLight` |
| `accent` | `CheckMark`, `SliderGrab`(+Active), `TabHovered`, `DockingPreview`, `TextSelectedBg` (α 0.35), `DragDropTarget`, `NavHighlight`, `NavWindowingHighlight` |
| `status_ok` / `status_warn` / `status_error` | status-bar and badge drawing only; no `ImGuiCol_` entry |
| `data.chart_series[8]` | `ImPlotCol_Line` cycle (`ImPlot::SetupAxis` + per-series `SetNextLineStyle`), `PlotLines`, `PlotLinesHovered`, `PlotHistogram`, `PlotHistogramHovered` |

**Metrics expansion.** The five `Metrics` scalars fan out to ~20 `ImGuiStyle` fields, so the fan-out is specified rather than invented:

| Metric | `ImGuiStyle` fields |
|--------|--------------------|
| `padding` | `WindowPadding = {p, p}`, `FramePadding = {p, p·0.55}`, `CellPadding = {p·0.65, p·0.35}` |
| `spacing` | `ItemSpacing = {s, s·0.75}`, `ItemInnerSpacing = {s·0.65, s·0.65}`, `IndentSpacing = s·2`, `ScrollbarSize = s·2`, `GrabMinSize = s·1.6` |
| `rounding` | `WindowRounding`, `ChildRounding`, `FrameRounding`, `PopupRounding`, `ScrollbarRounding`, `GrabRounding`, `TabRounding` — all `= r` |
| `border_size` | `WindowBorderSize`, `ChildBorderSize`, `PopupBorderSize`, `FrameBorderSize`, `TabBorderSize` — all `= b` |
| `font_size` | atlas build size (×`dpi_scale`, §3.5b) |

### 7.2 File format

⚠ **Revision 1 mandated TOML.** The repository contains no TOML parser, and §10 authorises vendoring exactly two libraries with a hash-manifest lint — so "themes are TOML" silently required either a third vendored dependency (unbudgeted, unlicensed, uncovered by the manifest) or a hand-rolled parser. **Decision: no third dependency.** External themes use a repo-owned flat format, `engine/native_desktop/themes/*.theme`, whose entire grammar is:

```
# comment to end of line
name = Graphite
surface_0 = #17181b
accent    = #5b8db8
metrics.rounding = 2
data.ternary = cvd_safe: #3b6ea5, #4a4d55, #d4884a
data.field_ramps.1 = #2b3f5c, #4a4d55, #b8752f
data.chart_series = #5b8db8, #c08a4a, #7ba05b, #a5688f, #4f9ba3, #b06a5c, #8a7fb0, #999da6
```

Grammar: one `key = value` per line; keys are dotted paths into `Theme`; values are `#rrggbb` / `#rrggbbaa`, a float, a bare string, or a comma-separated list optionally prefixed `cvd_safe:`. Unknown keys are a **parse error**, not a silent skip. Empty input is rejected. **Graphite** (default), **Contrast**, **Slate** and **Carbon** ship built-in and **compiled-in**, so a missing or corrupt file can never prevent startup.

**Hot reload** (v2.0, §12 Phase 7b) must respect: debounce; parse **off-thread into a `Theme` value**; apply at the top of a frame **before `NewFrame()`**; reject empty or unparseable input while preserving the previous theme. Editors commonly write truncate-then-write, so a naive watcher reads an empty file and a naive parser applies an empty theme. Mutating `ImGui::GetStyle()` from a watcher thread mid-frame is a data race.

> **Ported from the web dashboard, improved.** `engine/web/css/tokens.css` is a genuine **518-line** token system with a written spec — but discipline decayed to **392 hardcoded hex colour literals (220 distinct)** and **80 `!important`** across `engine/web/css/**/*.css` excluding `tokens.css` itself (measured over the whole subtree, including `themes/` and `ui/`, which hold most of the files), against a spec forbidding both; the proposed `theme_check.py` lint was never built, and the theme doc contains invalid token names from a botched find/replace. Here the token layer is a struct the compiler checks — and, because "the compiler checks it" does not stop a panel writing a raw colour, §9.3 ships an actual **theme-token lint** in Phase 3a, not a proposal for later.

---

## 8. Persistence and output

Single settings file, versioned, with migration and corrupt-file fallback: window geometry, active workspace + per-workspace `.ini` + per-workspace open-set, active theme, camera, last scenario, toggle profile, chart selections, **field ramp selection** (§5.4).

⚠ **Boot order is specified, because §8 and W9 interact badly.** Persisting a toggle profile means the app can boot running non-shipping-default physics — precisely the failure C2 warns about — and `dispatch_scenario()` can zero the whole registry, so restore-then-load and load-then-restore give different physics. The order is:

1. shipping `TermToggles{}` defaults;
2. scenario load (`dispatch_scenario`), which may rewrite the registry;
3. restore the persisted profile as a diff;
4. `validate()`;
5. on failure, fall back to shipping defaults with a visible log entry and a status-bar warning.

The restored profile is **journalled at tick 0 as an explicit diff** (§2.3). Whenever the live profile differs from `TermToggles{}`, a **persistent status-bar indicator** says so, and a **"reset to shipping defaults"** action (`ResetToDefaults`) is registered in the palette.

⚠ **`io.IniFilename` must be `nullptr`** — the app owns persistence explicitly. ImGui's default writes `imgui.ini` to the process CWD, which under CTest is a build dir shared by every test, making parallel runs order-dependent.

**Export:**

- **CSV telemetry** — the charted series plus the parameter journal (§2.3).
- **Reproduction bundle** — the journal header (environment), the initial state and the full journal, sufficient to reproduce a run.
- **PNG screenshot** — specified rather than left to interpretation, because §3.5 puts ImGui last into the backbuffer so a naive readback captures all chrome while a viewport-only capture drops the mandatory legend. **The capture copy is recorded on the still-open graphics list after the HUD legend and before the final PRESENT barrier; bytes become available only after that submission's fence retires.** A full-window capture is offered as a separate action. ⚠ Any exported PNG carries the same stride/count/`origin` disclosure the on-screen legend does (P2) — burned into the legend region, never dropped.

Nothing produced by the app can leave it today.

---

## 9. Testing and quality

### 9.1 Test pyramid

| Tier | Share | Runs | Content |
|------|-------|------|---------|
| **L0** logic | ~70% | everywhere, <2s | Command queue **ordering (incl. the named W4 four-command case, §3.4) and coalescing**; immutable publisher concurrent publish/acquire stress with per-snapshot checksum and monotone seq; scenario search/rank; theme parse incl. malformed **and empty**; workspace round-trip + migration + corrupt recovery; **journal replay reproduces the final profile bit-exactly** (§2.3); **two-thread `dispatch_scenario` produces identical lattices** (§2.3); **P5 ΔE₀₀ ≥ 20 over the ternary ramp LUT under deuteranopia and protanopia**; toggle dependency resolution — see the oracle rule below |
| **L1** headless ImGui | ~20% | from Phase 1, <5s | `omp_get_max_threads()` unchanged across ImGui init (C4); every panel draws assert-free from a synthetic `UiSnapshot` (catches unbalanced `Begin/End`, `PushID/PopID`, `PushStyleColor/Pop`, `BeginTable/EndTable` — the dominant ImGui bug class); draw-data invariants (no NaN in `ImDrawVert.pos`, clip rects inside `DisplaySize`); deterministic re-draw; **composed window name contains `###` and is stable across two draws**; **DPI matrix over `{font_size × scale, ScaleAllSizes(scale)}` for scale ∈ {1.0, 1.5, 2.0}** asserting proportional content bounds and no overflow of `DisplaySize`; **P2/P3 legend strings**; **P4/P7 mismatched-group-meta series test**; **P6 series-label lint**; **P8 badge from a `selective_damping=true` snapshot**; **§2.5 status-bar states from synthetic failure snapshots** |
| **L2** device | ~8% | `interactive`, owner machine | `native_desktop_d3d12_init_neutrality` asserts `omp_get_max_threads()` unchanged across D3D12/ImGui backend init; D3D12 `render()` smoke test with test-mode debug layer and inspectable `ID3D12InfoQueue`; ImGui-in-D3D12 draw; **P1 two-draw-order translucent-sprite readback asserting identical output on both CPU-expanded and interop particle paths** |
| **L3** image | ~2% | `interactive` | Theme-token solid-colour offscreen readback only |

⚠ **The toggle-dependency L0 test needs an oracle, not a mirror.** W3 says `validate()` Pass 2 encodes guards not derivable from `requires_`/`conflicts`, so a test deriving its expectations *from* `TOGGLE_SPECS` tests the derivable subset against itself and stays green while the panel shows exactly the wrong enable/disable state. Two tests, therefore:

- **`ui_toggle_widget_state_oracle`** — **the merge-gate one.** Over an exhaustive enumeration of a chosen 12-toggle subset plus 10,000 seeded random full profiles, assert the panel's derived widget state **never marks permissible a profile that `validate()` rejects**. One-sided by design: over-restriction is a usability bug, under-restriction is a sim-thread exception (W18).
- **`ui_toggle_table_derivation`** — the weaker table-derived case, kept for fast feedback, **not** in the merge gate.

### 9.2 Test-harness requirements

- **`IM_ASSERT` must be hookable without making shipping ImGui depend on test support.** `ftd_imconfig.h` routes assertions through a tiny always-available dispatch function whose default handler aborts; a test installs a scoped failure callback. `ftd_imgui` is compiled once. No consumer-specific macro changes its object code.
- **Embed the font**, with its provenance nailed down, because font provenance changes draw-data vertex counts and a disk-loaded fallback makes any pinned count flaky across machines:
  - **Font:** *Inter* (SIL Open Font License 1.1) at `engine/native_desktop/assets/` — chosen over ImGui's built-in ProggyClean for legibility at the §7 metrics.
  - **Licence:** the full OFL text ships beside the asset; §10's manifest covers it as a third-party asset row.
  - **Generated artifact:** `assets/font_inter_regular.inl`, produced by `imgui/misc/fonts/binary_to_compressed_c`; the exact regeneration command is recorded in `assets/README.md`. Loaded via `AddFontFromMemoryCompressedTTF`.
- **Pin `io.DeltaTime`; route all time through `UiSnapshot`.** Panels reading `ImGui::GetTime()` or a wall clock make deterministic re-draw tests impossible. This is a lint rule (§9.3), not a convention.
- **Where UI tests are registered.** ⚠ See §9.4 — never in `native_desktop/CMakeLists.txt`.

### 9.3 Lints

Two lint blocks appended to `engine/cmake/FtdSourceLint.cmake` (already registered as CTest `source_lint`), following the ADR-0016 quarantine block's pattern. ⚠ **Both must state absolute globs.** The existing blocks glob `${ENGINE_DIR}/src/*.cpp` where `ENGINE_DIR = engine/`; an implementer copying that literally would produce `engine/src/ui/**`, which matches **zero files**, and D6 would go unenforced while everyone believed it was enforced.

**9.3a Panels lint (D6).**

- **Glob:** `${ENGINE_DIR}/native_desktop/src/ui/**/*.h` and `**/*.cpp`. There is no `src/panels/`; revision 1's second glob matched nothing.
- **Non-vacuity self-check:** `FATAL_ERROR` if the glob matched **0 files**. A lint that passes because it sees nothing is worse than no lint.
- **Banned tokens** — anchored to the actual violation shape, not to a substring every legitimate panel contains:
  `RenderBridge` · `NativeEngineSession` · `#include "ftd/render_bridge.h"` · `->tick(` · `bridge_.toggles` · `bridge->toggles` · `rb.toggles` · `ImGui::GetTime(` · `std::chrono::` · `GetTickCount`
- ⚠ **`.toggles` is NOT banned.** §3.4 puts the current `TermToggles` in the snapshot and §5.3 requires the Physics panel to render all 53 rows from it, so every legitimate panel line reads `ctx.snapshot.term_toggles.<field>`. Banning the bare token would fail the first correct panel and force the pattern to be weakened — the exact erosion this lint exists to prevent. The snapshot field is named **`term_toggles`** (§3.4) so no anchored pattern can collide with it.
- ⚠ **Comment text counts.** The existing `FtdSourceLint.cmake` blocks match raw file text, so a comment mentioning `RenderBridge` fires the lint. That is accepted and stated here so it is not discovered as a surprise; panels refer to it as "the bridge" in prose.
- **Escape hatch, stated so nobody invents one:** a panel that legitimately needs a toggle *name string* uses `ftd::find_spec(name)->description` from the allowlisted `term_toggles.h` — it never needs the type name `RenderBridge`.

This lint lands with the first panel files in Phase 3a; its non-vacuity check makes an earlier empty-tree registration invalid.

**9.3b Boundary lint (main.cpp / presenter).** D6's protection stops at the panel boundary, but the GUI/message-loop thread already reaches the session today. A narrower rule over `${ENGINE_DIR}/native_desktop/src/main.cpp` and `src/d3d12_presenter.cpp` forbids any `RenderBridge` observer call outside the sim-thread lambda, and forbids `#include <imgui.h>` in the presenter (§3.1).

**9.3c Theme-token lint.** Glob `${ENGINE_DIR}/native_desktop/src/ui/**` **excluding `theme.cpp`**; `FATAL_ERROR` on `ImGui::GetStyle()`, `ImGuiCol_` used as an assignment target, `ImPlotCol_` used as an assignment target, and hex-literal colour constants (`0xFF......`, `IM_COL32(`). This is what actually prevents the `tokens.css` decay §7 describes; the panels lint has nothing to do with theme tokens. Lands in **Phase 3a** with the theme.

**9.3d UI-model include lint.** From Phase 0B, non-vacuously glob the `ftd_native_ui_model` headers/sources listed in §3.2. Reject every `#include "ftd/...` not in §3.1's explicit allowlist, and reject ImGui, D3D12, CUDA and `RenderBridge` tokens. The lint also asserts that each expected model source exists, so a path move cannot turn enforcement vacuous.

### 9.4 The UI task merge gate

One copy-pasteable block, one working directory, `-C Release` on every `ctest` line (the build is Ninja **Multi-Config** — without `-C`, ctest matches no configuration and reports **no tests**, which reads as a pass). Run with `FTD_FORCE_CPU` and `FTD_FORCE_GPU` **both unset** (W24).

```bash
# from the repo root
engine\build_native.bat golden          # 7 tests, serial (golden|gauge_links)
cd engine/build
ctest -L merge_gate      -j 32 -C Release --output-on-failure   # grows in 0A and 0B
ctest -R source_lint           -C Release --output-on-failure   # 9.3a + 9.3b + 9.3c
ctest -L "^ui$" -LE interactive -j 32 -C Release --output-on-failure # L0 + L1
ctest -R ui_test_inventory     -C Release --output-on-failure   # non-vacuity floor
```

- The CPU neutrality registration joins `merge_gate` in Phase 0A. The command-boundary registration joins it in Phase 0B, when the real queue and applier exist. GPU cases are separate CUDA-conditional registrations carrying `gpu;interactive`, never subcases pretending to have CTest labels.
- ⚠ **All UI test registrations go in the parent `engine/CMakeLists.txt`**, following the existing `test_native_desktop_session` / `test_interop_reload_orchestration` precedent at lines 505-527. **Never in `engine/native_desktop/CMakeLists.txt`** — that directory's `add_subdirectory()` runs before `enable_testing()` in the parent, so no `CTestTestfile.cmake` is generated there and `add_test()` calls placed there are **silently dropped** (documented from hard experience at `native_desktop/CMakeLists.txt:54-60`). The first implementer to register a panel test next to the target would get a green build, a binary that exists, and `ctest -L ui` reporting nothing.
- **`ui_test_inventory`** is the floor assertion that makes a vacuous label impossible. It is created and seeded in Phase 0A with the first CPU UI-labelled registration, compares `ctest -L "^ui$" -N` against an expected minimum count (CTest `-L` is a regex, so the unanchored `ui` pattern would also match the `unit` label), and is bumped by every later phase that adds tests.
- Tasks touching `d3d12_presenter.cpp` additionally run **`ctest -L native_desktop -C Release`**: **10 tests** carry that label (`test_native_desktop_session`, `test_interop_reload_orchestration`, `d3d12_adapter_selection`, `d3d12_shared_buffer`, `cuda_d3d12_adapter_match`, `cuda_import_shared_buffer`, `interop_gather`, `interop_fence_roundtrip`, `interop_reload_reset`, `interop_visual_parity` — `engine/CMakeLists.txt:505-615`), **8 of them `interactive`**, **6 additionally `gpu`**. ⚠ On a CUDA-off configure only 3 targets compile the presenter and only 2 presenter tests exist, so **presenter-touching tasks must be verified on the owner rig with `FTD_ENABLE_CUDA=ON`** before merge. (Revision 1 gave this count three incompatible ways — 8/5, 5, 8 — none matching the tree; the "9" in §3.1 is the count of *targets compiling the file*, a different and correct number.)
- Tasks touching shipping toggle defaults or the boot scenario additionally state which defaults moved (C2).
- Any task that changes gate composition also updates `engine/docs/CI_GATE.md` (§9.7), and that update is an exit criterion of the phase (§12).

### 9.5 `ui_observer_neutrality` — the demand-gating catcher

The neutrality suite has separate registrations because CTest labels apply to a registration, not to individual cases:

- `ui_observer_neutrality_cpu`: `unit;native_desktop;ui;merge_gate`, available on every build.
- `ui_observer_neutrality_gpu`: `native_desktop;ui;gpu;interactive`, registered only when CUDA is enabled.
- `ui_command_boundary`: `unit;native_desktop;ui;merge_gate`, introduced in Phase 0B with the real transport.

Every CPU case begins with `force_cpu()` and asserts `backend().kind() == Cpu`. Every GPU case asserts `backend().kind() == Gpu` and `interactive_gpu_mode()`, otherwise it skips with an explicit unsupported-environment result rather than testing CPU under a GPU name.

**N1 — ledger continuity (CPU).** `RenderBridge rb(9)`, `force_cpu()`, `seed_rng(42)`, shipping defaults, 200 ticks.

⚠ Revision 1's three assertions were **all satisfied by a frozen ledger** and detected nothing. `update_energy_ledger_cpu()` assigns `L.tick_prev = rb.tick_ - 1` **unconditionally, re-derived from the current tick** (`energy_ledger_compute.cpp:62`) — skip ticks 50–59 and the update at tick 60 still writes `tick_prev = 59`. The `E_prev == E_curr(t−1)` chain is self-referential (`const double E_prev = L.E_curr;`, `:61`), so a frozen ledger satisfies it trivially. And any `K` below the observed-tick count passes the third. A concrete bug that passed all three: gate the ledger on "at least one instrument panel is open" — the Experiment workspace docks Telemetry/Audit/Lagrangian, so the test-shaped configuration observes every tick while a user in Presentation silently loses `drift_frac`, `residual` and `max_residual_seen` for the whole session.

The replacement assertions cannot be satisfied by a frozen accumulator:

- **(a) Update count.** Add `std::uint64_t updates = 0;` to `EnergyLedger` and increment it as the **first statement** of `update_energy_ledger_cpu()`, *before* the seeding branch. Assert `energy_ledger().updates == static_cast<std::uint64_t>(current_tick())` after every tick. This is an append-only observation field, not folded into `compute_state_hash`, therefore golden-neutral by construction.
- **(b) Seeding shape, named so it is not "fixed" back later.** ⚠ `update_energy_ledger_cpu()` has a seeding branch: when `L.tick_prev < 0` it sets `L.tick_prev = rb.tick_` — **not `tick_ - 1`** — and returns (`energy_ledger_compute.cpp:49-59`); `EnergyLedger::tick_prev` defaults to `-1` (`render_bridge_diagnostics.h:156`) and nothing calls the update before the first `tick()`. So after tick 1 the ledger holds `tick_prev == 1` while `current_tick() == 1`. Assert **after tick 1**: `tick_prev == current_tick()` and `E_prev == E_curr`. Assert **from tick 2**: `tick_prev == current_tick() - 1`.
- **(c) Accumulators advance.** `cumulative_injection + cumulative_dissipation` is non-decreasing, and **strictly increases on exactly the ticks where the ungated reference run's value changes** — compared tick-for-tick against a reference vector captured in the same test, so no magic constant `K` is needed. (Revision 1 left `K` unbound in a merge-gate test.)

**N1-gpu — characterize the pre-Phase-6 baseline (`gpu;interactive`).** Same fixture without `force_cpu()`, with `set_interactive_gpu_mode(true)`. In Phase 0A, assert `energy_ledger().updates == 0` after 200 ticks, documenting W21 rather than presenting stale values. Phase 6 replaces this with two cases: demand off preserves zero synchronization and appends gaps; demand on increments once per tick within a fresh epoch and reports synchronization cost.

**N2 — observer idempotence.** Two identically-seeded bridges: A runs 100 bare ticks; B runs 100 ticks each followed by the full Category-2 + Category-3 observer set the UI may demand.

⚠ Revision 1 used `compute_state_hash` as the instrument, which folds `mix_audit` → `rb.energy_audit()` (`tests/support/golden_hash.h:82-84`) — running an observer on the supposedly-unobserved control, so any side effect idempotent after the first call cancels between A and B. Its field set was also far narrower than what an observer can perturb.

- Use **`compute_state_only_hash`** (`golden_hash.h:225-230`) — the trajectory-only fold (per-voxel fields + manifested list), with **no `energy_audit()` call**.
- Assert equality **per tick**, not only at the end.
- Extend the comparison to `rng_state_hash()` (C4), **every** `EnergyLedger` field including `updates`, `current_tick()`, `physical_time()`, `dt()`, `sor_iterations()`, and a field-wise canonical serialization of `TermToggles`. Never `memcmp` the struct: padding bytes are not semantic state.
- **N2-gpu** (label `native_desktop ui gpu interactive`): the identical comparison **without** `force_cpu()`, since that is the configuration the claim needs to hold in. On CPU, `flush_host_mutations`/`sync_to_host` are no-ops and every dangerous side effect enumerated in W19 is GPU-only — so the CPU form alone is close to a tautology and proves nothing about the shipping backend.

**N3 — the request slot is single-occupancy.** Both `CpuBackend::begin_telemetry_snapshot()` (`backend.cpp:136`) and `GpuEngine::begin_telemetry_snapshot()` (`cuda/gpu_engine.cu:1430`) return `false` while one is pending. This is backpressure at the backend layer, not by itself a shell hard error.

Rewritten against the real contract:

- `begin_telemetry_snapshot()` on tick 1 returns `true`.
- Without polling, `begin_telemetry_snapshot()` on ticks 2…N returns `false`, and the pending snapshot's contents are **unchanged** by every rejected call.
- One `poll_telemetry_snapshot()` re-opens the slot; the next `begin` returns `true`.
- The scheduler must keep draining/pumping and may retry according to its existing state machine. Only its deadline or an explicit backend failure becomes a shell error.
- The wall-clock deadline is exercised by a separate `interactive`-labelled test driving `NativeTelemetryScheduler` after the status bar exists, not by Phase 0A or `merge_gate`.

**N4 — commands apply at the tick boundary (C3), introduced in Phase 0B with the real queue.**

- Two identically-seeded CPU bridges, 200 ticks. A has `larmor_radiation` written **directly between `tick()` calls** at tick 50. B has the same change delivered through the real `CommandQueue` drain at the sim-loop seam.
- Assert `compute_state_only_hash(A) == compute_state_only_hash(B)` at every tick, and that `energy_ledger().tick_prev` and `updates` agree.
- **Negative-control proof:** before the production applier exists, the test is run against a deliberately wrong test-local seam and observed to fail for the expected trajectory difference. The committed test exercises only public sim-loop seams; it does not require private tick-phase access.
- Also asserts the §3.4 ordering rule directly: a tick counter recorded at apply time equals the boundary's pre-next-tick value for every mutation in a drain.

**N5 — the fixed-point flush makes observer order irrelevant (`gpu;interactive`).** Exercise the production partition/apply/flush/observe path with one pending host mutation (obtained by calling the non-const `voxels()` — W19) and six observer request orders. The immediate snapshot must report continuity `PendingAfterHostUpload` in every order, because `upload_from_host()` intentionally invalidates the ledger. After the next completed tick, the retained request executes and all orders must return equal availability/provenance and equal state-only hashes. This tests both the actual immediate ordering and the specified deferral.

**N6 — the want-mask is physics-neutral (label `native_desktop ui gpu interactive`).** Two identically-seeded GPU sessions, 100 ticks: one requesting `TELEMETRY_ALL` every tick, one requesting nothing. Assert equal `compute_state_only_hash` and equal `rng_state_hash()`. This is the assertion that `begin_telemetry_snapshot()`'s commit behaviour (W22) does not leak into the trajectory.

### 9.6 What we deliberately do not build

A general screenshot-diff suite. It goes red from driver updates alone, can only run on one machine, and the repo has no binary-asset store. Confining image testing to solid-colour theme-token readback (L3) and the single deterministic P1 two-draw-order readback (L2) is the difference between a gate people trust and a gate people disable.

### 9.7 CI

⚠ Adding `-LE interactive` to `.github/workflows/ci.yml` **removes coverage that runs today**, and revision 1 presented it as purely additive. The C++ job is `runs-on: windows-latest` with `-DFTD_ENABLE_CUDA=OFF` and runs `ctest -L unit -E "^helium_scale1$"` (`ci.yml:41`); `d3d12_adapter_selection` and `d3d12_shared_buffer` carry `LABELS "unit;native_desktop;interactive"` (`engine/CMakeLists.txt:541-554`), so that invocation executes them now. **Compensation is mandatory in the same task:** re-add them by name in the same step (`ctest -R "d3d12_adapter_selection|d3d12_shared_buffer" -C Release`), or split those two off the `interactive` label first.

`-LE interactive` lands in **Phase 0A** with the baseline device test and the required compensation. `engine/docs/CI_GATE.md` is updated in every increment that changes gate composition: 0A adds CPU neutrality and interactive exclusion; 0B adds command-boundary and publisher tests; Phase 1 adds ImGui/device coverage; Phase 3a adds panel inventory and lints.

### 9.8 Scope-creep control as an executable bar

Each of D4's **four** clusters gets a sentence with a number, backed by a coverage test. Revision 1 supplied two, leaving the two clusters most likely to creep unguarded.

**Descriptor oracle.** C++17 has no reflection, so "header-derived" is not shorthand for an impossible compile-time enumeration. Instrumentation panels use explicit typed descriptor arrays (name, component, pointer/getter, group). `scripts/tests/test_native_ui_descriptor_coverage.py` parses the named aggregate declarations in `render_bridge_diagnostics.h`, `telemetry_snapshot.h` **and `voxel.h`**, compares declaration names and extents to the descriptor manifest, and fails on an unrepresented addition or stale descriptor. Flattening is fixed: arithmetic members produce one scalar; `Vec3` produces `.x/.y/.z`; fixed arrays produce `[0..N-1]`; nested `Voxel`, `EMFieldDiag` and force structures recurse through their own manifests. Parser fixtures include comments, default initializers and arrays so the gate fails closed rather than silently skipping unfamiliar syntax.

| Cluster | Bar | Test |
|---------|-----|------|
| 1 · Physics control | "The Physics dock exposes exactly the 43 `TOGGLE_SPECS` rows + 10 non-bool config fields; Run config exposes exactly the 6 `RenderBridge` knobs; no field appears twice" | Iterate `TOGGLE_SPECS` + a pinned 10-name and 6-name list; assert bidirectional coverage against the two panel descriptor lists **and empty intersection** |
| 2 · Field visualisation | "The Fields panel exposes exactly the 18 `VisualFieldKind` values" | Enum-iteration set-equality against the panel's kind list |
| 3 · Instrumentation | "The instrument panels chart exactly one series per flattened scalar component of `Diagnostics`, `EnergyAudit` (34 direct members), `EnergyLedger` (11), `TelemetryLagrangian` (17), `GravityMetricAgg`, and render one row per flattened member of `VoxelInspection` and `ForceDiag`" | Typed descriptor arrays plus the fail-closed source parser above |
| 4 · Workflow/output | "The Scenario browser covers `scale0_scenario_ids()` in full and nothing else, and `SCENARIO_META` is set-equal to it" | Set-equality test against `scale0_scenario_ids()` **explicitly** (precedent: `engine/web/tests/scenario-parity.spec.js`, which does *not* itself do this — §5.1) |

"Done" becomes machine-checkable, and "just one more panel" becomes a failing test rather than a conversation.

---

## 10. Vendoring

`engine/thirdparty/imgui/` and `engine/thirdparty/implot/`, each with `VERSION.txt` pinning **the upstream commit SHA, a minimum version, the upstream URL, and the MIT licence text**. ⚠ The minimum is **ImGui ≥ 1.91.5** (docking branch), because §3.5 depends on the post-1.91.5 app-supplied descriptor alloc/free callbacks; an older docking SHA gives a different `ImGui_ImplDX12_Init` signature. `VERSION.txt` also records the ImPlot compatibility range for that ImGui version.

⚠ **`imconfig.h` is not edited.** Revision 1 listed a "repo-owned `imconfig.h`" under `ftd_imgui`'s contents without saying whether that meant replacing `thirdparty/imgui/imconfig.h` — which would break the hash manifest this section mandates. The correct mechanism: define `IMGUI_USER_CONFIG="ftd_imconfig.h"` and keep `ftd_imconfig.h` **outside `thirdparty/`**, at `engine/native_desktop/include/native_desktop/ftd_imconfig.h`. The vendored tree stays byte-identical to upstream, so the manifest covers it cleanly.

⚠ **`.gitignore` currently ignores `engine/thirdparty/` wholesale** (`.gitignore:179`) — the existing `glad` loader is untracked. Vendoring there as-is would make the dependency invisible to git and break every fresh clone. Add explicit re-inclusion exceptions, following the existing precedent for `!engine/web/js/vendor/three/build/` (`.gitignore:13`).

Follow the `engine/web/js/vendor/three/README.md` pattern: upstream URL, licence, exact version, per-file purpose, upgrade command, SHA-256 per file. Ship the upstream `LICENSE` text — the existing vendor dirs omit it, which is a gap to fix rather than a precedent to copy.

**Third-party assets** get the same treatment: `engine/native_desktop/assets/` carries the Inter TTF's OFL 1.1 text, the source URL and version, the generated `.inl`, and the `binary_to_compressed_c` regeneration command (§9.2).

**No third library is vendored.** §7.2 chose a repo-owned theme format specifically so this section stays at two libraries plus one font asset.

Forbid local edits to the vendored trees, enforced by a **hash-manifest lint block in `FtdSourceLint.cmake`** so `ctest -R source_lint` already covers it. It lands in **Phase 1** with the vendoring.

---

## 11. Risk register

| Risk | Severity | Mitigation |
|------|----------|------------|
| **Tick-boundary command queue silently breaks bit-accuracy** | **Critical** | The queue, applier and N4 land together in Phase 0B. `RenderBridge::toggles` is public and the CPU path reads it live (C3); applying a toggle one phase late changes only that tick's trajectory — invisible to goldens that never toggle mid-run. |
| **Panel draw order changes diagnostic content (W19)** | **Critical** | The Category-2 fixed-point flush (§2.2) plus §9.5 **N5** on the owner rig. Untreated, dragging a dock tab silently empties `continuity_step()`. |
| **Multi-toggle profiles throw out of `tick()` (W18)** | High | `SetToggleProfile` + pre-enqueue `validate()` (§3.4, §5.3); `strict_validation` removed from the ordinary table; tick-time throws surface via `TickResult` (§3.4b). |
| Scope creep across 4 clusters | High | §9.8 executable bars for all four clusters; ten independently shippable increments. |
| ImGui multi-viewport instability | High | **Not shipped in v2.0** (D5). |
| Interop path regression | High | Fails loudly (black screen / device removed / a red interop test). Full `native_desktop` label on any presenter-touching task, on the owner rig with CUDA on (§9.4). |
| DPI regression | Medium | Phase 0A establishes process awareness and top-level `WM_DPICHANGED` handling without moving the swapchain. Phase 1 couples scene-rectangle/backbuffer migration with the font-atlas × `ScaleAllSizes` matrix (§3.5b). |
| Snapshot tearing | Medium | Mutex-protected immutable `shared_ptr<const UiSnapshot>` publication (§3.4); Phase-0B writer/reader stress with checksum and monotone seq. |
| Theme hot-reload race | Medium | Off-thread parse into a `Theme` value, apply before `NewFrame`, debounce, reject empty/unparseable (§7.2). |
| Panel draw cost at 60 Hz | Medium | Demand-gating via the existing scheduler (§2.2) + the **debug-only frame-time and worst-panel-cost element in the status bar** (§4.1 — revision 1's mitigation named a surface whose content list did not contain it). **Budget: 4 ms of a 16.7 ms frame for all panel drawing combined**, measured at the reference layout; exceeding it is a bug, not a preference. |
| Thread-ownership violation | Medium | Debug-only `FTD_UI_DEBUG_THREAD_GUARD` asserting the calling thread on public `RenderBridge` methods — **including the `const` observers**, which are the dangerous ones here (W20). Debug-only, therefore golden-neutral by construction. MSVC has no ThreadSanitizer. Lands in **Phase 0B**. |
| Regression bisection difficulty | Low | `--no-ui` runtime flag (**Phase 1**, when UI first exists): skips all ImGui init and draw; renderer, sim loop and status logging unchanged. ⚠ After Phase 4 it yields an app with no controls; it is a bisection tool, not a supported mode. |

---

## 12. Phasing

The corrected foundation is three independently shippable increments: 0A characterization, 0B transport/session spine, and 1 renderer host. Later feature phases retain the prior cluster decomposition.

**Every increment ends in a launchable, demoable, golden-green app.** ImGui goes in alongside the existing Win32 controls under §3.5c; old controls are deleted only after their replacements are demoed. The swapchain remains on the child HWND through 0B and moves only in Phase 1 with the complete scene-rectangle contract.

| Phase | Content | Exit criterion |
|-------|---------|----------------|
| **0A · Characterization gates** | No UI model and no queue. Add `EnergyLedger::updates`; register separate CPU/GPU neutrality binaries; create/seed `ui_test_inventory`; land N1, N1-gpu, N2, N2-gpu, N3, N6; establish a Release-capable D3D12 test-debug switch plus inspectable info-queue baseline; enable per-monitor-V2 awareness and top-level `WM_DPICHANGED` handling **without moving the child swapchain**; add `native_desktop_dpi_awareness`, a hidden-window test that verifies awareness is set before window creation and that a synthetic `WM_DPICHANGED` applies the suggested top-level rectangle; apply the §9.7 CI compensation; update `CI_GATE.md` | Current app launches with unchanged controls and child renderer; CPU merge-gate and inventory cases pass; CUDA cases pass on the owner rig; debug-layer baseline is inspectable and clean; `native_desktop_dpi_awareness` passes without claiming scene/backbuffer migration |
| **0B · Transport/session spine** | Add `ftd_native_ui_model`, owning commands, queue, mutex-protected immutable publisher, result/status vocabulary and journal; extend `ftd_native_session` with partitioned mutation/flush/observation ordering and structured reload/tick outcomes; route pause/step first; land N4/N5 and concurrent publisher stress; add thread guard, boundary/model-include lints and raise the inventory floor | Current Win32 app remains fully usable; `ui_command_boundary` (N4), N5, publisher stress, command ordering/control scheduling, journal replay, structured reload-result, tick-result, thread-guard, model-include-lint and inventory tests pass; pause/step use the queue; goldens remain green |
| **1 · Renderer host + first pixel** | Pin/vendor dependencies; add assertion dispatch and embedded font; define bootstrap, overlay-record, capture-record and post-fence completion seams; add explicit scene rectangle; move swapchain to top-level HWND; grow SRV heap; wire ImGui Win32/DX12 and DPI atlas rebuild; add `--no-ui`; render one debug window while preserving Win32 controls | ImGui renders over the lattice inside the explicit scene rect; viewport/scissor/aspect/input-arbitration, DPI/font rebuild, `--no-ui`, OpenMP-neutral initialization, one-render-call-per-frame/frame-in-flight and capture lifecycle tests pass; both CPU-expanded and interop particle paths remain visible; info queue is clean on the owner rig |
| **3a · Shell IA** | Dockspace with the viewport as central node, three docks, menu bar, status bar, compiled-in Graphite + theme-token lint (9.3c), owned workspace persistence with the DockBuilder recipe, migration and corrupt recovery | The shell of §4 is navigable; workspace round-trip and corrupt-file recovery pass |
| **3b · Palette** | Ctrl+K over **panels and actions only**, with the §4.3 state strings and ranking | Palette reaches every panel and action; its index is extended by 4, 5 and 7a as an exit criterion of each |
| **4 · Cluster 1** | Physics experiment control: the 43-row table, all 10 config fields, `SetToggleProfile`, the W5 confirmations, the W18 handling, the toggle-state oracle test. **Last task retires the old Win32 control strip** and expands the dockspace to the full client area | All 43 toggles + 10 config fields + 6 knobs editable live, no field owned twice; palette indexes toggles |
| **5 · Cluster 2** | Field visualisation: all 18 kinds, honest legends (P2, P3), the §2.2 cadence rule, cost badges — **plus the §6 deliverables this phase owns: field render modes, HUD legend, P1 alpha/depth fix, P5 colourblind-safe ternary ramp**. Highest interop exposure — runs the full `native_desktop` label | All 18 field kinds selectable with honest legends; P1 readback test green; P5 ΔE₀₀ assertion green; palette indexes fields |
| **6 · Cluster 3** | Instrumentation: `History`, five chart panels, P8 badge, `DataNeeds` wired into `NativeTelemetryScheduler::Demand`, D11 GPU-ledger synchronization/epoch/cost telemetry, Category-3 one-shot commands — **plus §6's picking**. This phase activates telemetry demand for the first time | Charts live with real gaps at gated intervals; GPU-ledger mode reports synchronization cost and never bridges epochs; voxel picking drives the Inspector; N1-gpu/N5/N6 green on the owner rig |
| **7a · Workflow** | Scenario browser + `scenario_meta.h` (machine-generated rows, §13 Q1), settings persistence with the §8 boot order, PNG/CSV/reproduction-bundle export | Scenario browser covers all 130 ids; parity test green from day one; export produces a replayable bundle |
| **7b · Theme authoring** | External `.theme` files + hot reload; Contrast/Slate/Carbon as external files verifying the format; the owner-review pass over the 130 descriptions | Hot reload survives truncate-then-write; no theme file can prevent startup |

**Interim telemetry policy, Phases 0B–5.** `NativeTelemetryScheduler::Demand::enabled_mask` remains zero. Shell status uses Category-1 snapshot fields and does not compute chart groups before chart panels exist. Phase 6 introduces `DataNeeds` aggregation and D11's GPU-ledger mode without changing the shared scheduler default used by `ws_server`.

---

## 13. Open questions

Revision 1 listed four; three were not open (one was already decided in §2.3, one restated a decision of record, one blocked Phase 4). Those are resolved below as decisions. What remains, plus what revision 2's verification newly opened, is listed as **OQ-n**.

### 13.1 Resolved into decisions

- **D-Q1 · Parameter journal persistence.** ~~Does the journal need to persist across a session restart?~~ **Decided: in-memory + export, v2.0** — §2.3 already committed to this; the question re-asked what the spec had decided. Cross-session journal chaining is out of scope.
- **D-Q2 · Toggle grouping taxonomy (W2).** This blocked Phase 4 (§5.3 requires a *grouped* table and no grouping metadata exists), so it ships as a decision with a stated default rather than a question. **Decided: the `name → group` map is derived from the tick phase each term participates in**, with eight groups — `Field evolution` (phase_read), `Manifestation` (phase_write), `Constraint` (gauss_project), `Latency` (latency_solve), `Forces` (phase_forces), `Movement` (phase_movement), `Boundary` (boundary), `Transmutation` (weak/triad) — plus `Diagnostics` for the developer flags (§5.3 item 6). The map lives in `src/ui/panels/toggle_groups.inl`, is explicitly **non-load-bearing**, and may be revised without a spec change.
- **D-Q3 · Scale filter (D3).** ~~Confirm no scale filter in v2.0 is acceptable.~~ **Closed: D3 already decided it**, and §0 forbids re-opening a decision of record. The forward-compatibility mechanism that D3 promised is now realised rather than asserted: `ScenarioMeta::scale` exists (§5.1) and `PanelRegistry` is scale-agnostic. No filter ships in v2.0.
- **D-Q4 · Interactive-GPU EnergyLedger.** **Decided 2026-08-19: synchronize on explicit demand** (D11). Phase 6 adds a ledger epoch, exact once-per-tick update while demanded, gap semantics while disabled, and visible synchronization-cost telemetry. It does not silently impose the host mirror on all GPU sessions.

### 13.2 Genuinely open

- **OQ-2 · `ScenarioMeta` authorship** (§5.1). 130 titles and descriptions must eventually be written; `description` and `min_lattice` do not exist anywhere to be generated from. The plan de-risks this: Phase 7a lands machine-generated rows (`title` from `sourceTitle`, `category`/`tags`/statuses from the registry, `description = ""` rendered as an honest placeholder, `min_lattice = 0`) so the parity test is green from day one, and Phase 7b carries a **non-blocking** owner-review pass. What remains open is only *who* authors the 130 descriptions and on what cadence.
- **OQ-3 · Whether `NativeTelemetryScheduler` should be shared with `ws_server` or instantiated per-consumer.** §2.2 decides the native shell owns its own instance, which is correct for v2.0 (the header is header-only and `ws_server` is not linked into the desktop app). If a future build runs both surfaces against one engine in one process, epoch semantics across two scheduler instances need a decision. Not blocking v2.0.
- **OQ-4 · Whether the six W5 CPU-forcing toggles should be blocked outright on a GPU session** rather than offered behind a confirmation. Blocking is safer and loses a real capability; confirming preserves it and lets a user destroy their own interop session mid-run. v2.0 ships the confirmation. Revisit after the first GPU session where someone does it.
- **OQ-5 · Frame budget under demand-gating on the CPU backend at large L.** §11 sets 4 ms for all panel drawing, but W11's synchronous group computation is charged to the *sim* thread and its cost at L=128+ with `TELEMETRY_ALL` has not been measured. The Phase-3a interim cadence is a guess informed by `ws_server`'s defaults, not a measurement. Measure in Phase 6 and pin the numbers.

---

## 14. References

- `docs/adr/0012-golden-tick-regression-gate.md` — the golden gate's own scoping caveat (C2)
- `docs/adr/0016` quarantine block in `engine/cmake/FtdSourceLint.cmake` — the lint pattern §9.3 extends, and the source of its `${ENGINE_DIR}` glob convention
- `engine/native_desktop/CMakeLists.txt:54-60` — the documented reason UI tests must be registered from the parent (§9.4)
- `engine/include/ftd/native_telemetry_scheduler.h` — the existing demand/cadence/retention scheduler §2.2 adopts
- `engine/tests/support/golden_hash.h` — `compute_state_only_hash` / `compute_audit_only_hash`, the split folds §9.5 N2 uses
- `engine/src/energy_ledger_compute.cpp` — the ledger seeding branch (N1b) and the `selective_damping` caveat (P8)
- `engine/web/docs/adr/0002-scenario-architecture.md` — the undecided scenario-layer question §5.1 resolves
- `engine/web/css/tokens.css`, `engine/web/css/THEMING.md` — the token system §7 improves on
- `engine/web/tests/scenario-parity.spec.js` — the set-equality test pattern §9.8 copies, and the test §5.1 corrects the record about
- `engine/cmake/FtdAddTest.cmake` — `ftd_test_support PUBLIC ftd_core`, the reason the `NO_CORE` speed claim was withdrawn (§3.1)
- `engine/include/ftd/term_toggles.h` · `visual_field_sample.h` · `telemetry_snapshot.h` · `render_bridge_diagnostics.h` · `voxel_rng.h` · `bridge_rng.h` · `scenarios.h` — the authoritative field lists behind §5 and the RNG inventory behind C4
