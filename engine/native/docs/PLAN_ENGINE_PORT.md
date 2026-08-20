# PLAN — Native Desktop Engine Port

**Status:** `[PLAN — DRAFT for owner review]` · **Created:** 2026-08-19 · **Scope:** extend `engine/native_desktop` from a Scale-0 shell into the full multi-scale FTD engine, natively (in-process C++/CUDA physics, D3D12 rendering, Dear ImGui UI).

**Companions:** [`REF_WEB_ENGINE_ARCHITECTURE.md`](REF_WEB_ENGINE_ARCHITECTURE.md) (the as-is map of what is being ported) · [`SPEC_UI_V2.md`](SPEC_UI_V2.md) (the approved Scale-0 native shell design, substantially implemented) · [`../README.md`](../README.md).

This plan has two parts: **Part 1** is an audit of the native shell as it actually stands (evidence-grounded, working-tree state as of 2026-08-19); **Part 2** is the phased game plan built on that audit. Every phase names its goal, its dependencies, its exit criterion, and its risk. Effort is given as relative T-shirt weight (S/M/L/XL), not calendar time.

---

# Part 1 — Audit of the current native shell

## 1.1 Headline

The **hard, architecture-defining half of SPEC_UI_V2 is genuinely built, to a high standard** — the two-thread model, the transport/session/command spine, the CUDA↔D3D12 interop path, the ImGui-in-D3D12 overlay, the vendoring, DPI, and an unusually faithful test + lint harness. The **user-facing scientific panels are almost entirely placeholder stubs.** By scope roughly **45–50% of the spec is delivered, and that slice carries ~90% of the engineering risk.** The critical caveat: **all of it is uncommitted working-tree state**, so nothing has passed a commit or CI.

## 1.2 Phase-by-phase status (SPEC_UI_V2 §12)

| Phase | Status | Evidence |
|---|---|---|
| **0A** Characterization gates | **DONE** | `EnergyLedger::updates`; CPU/GPU neutrality binaries (N1/N2/N3, N1-gpu/N2-gpu/N6); `ui_test_inventory` floor; D3D12 debug-layer switch + info queue; per-monitor-V2 + `WM_DPICHANGED` without moving the swapchain; `CI_GATE.md` updated. |
| **0B** Transport/session spine | **DONE** | `ftd_native_ui_model`; full 27-alternative `UiCommand` variant; FIFO+coalesce `CommandQueue`; mutex `SnapshotPublisher`; `ParameterJournal` (+ working `replay_requests`); partitioned drain/flush/observe (`command_applier.cpp:490-552`); N4/N5 tests; thread-guard + all four lints. |
| **1a** Vendor + headless ImGui | **DONE** | ImGui 1.92.9b-docking + ImPlot vendored w/ `MANIFEST.sha256`; `ftd_imconfig.h` assert dispatch; embedded Inter font; hash-manifest lint; L1 headless tests; `--no-ui`. |
| **1b** Renderer host + first pixel | **DONE (ahead of plan)** | Single top-level HWND, swapchain moved; overlay-record + capture seams; SRV heap 256 (slot 0 = interop, 1 = font); DPI atlas rebuild. The old Win32 control strip is **already deleted** — Phase 4's "retire the strip" task is effectively pre-done. |
| **3a** Shell IA | **DONE** | Dockspace (viewport central node + three docks), menu bar, status bar; Graphite + 4 built-in themes; theme-token lint; workspace persistence + DockBuilder recipe + migration + corrupt recovery. |
| **3b** Command palette (Ctrl+K) | **DONE** | `command_palette.{h,cpp}`; ranking + Phase 3b catalog (panels + actions); Ctrl+K modal; `ui_command_palette` + shell-draw popup coverage. Toggles/fields/scenarios extend the same index in Phases 4/5/7a. |
| **4** Physics control (43/44 toggles) | **NOT STARTED** | `physics_terms.cpp:26` = *"Full 43-row table ships in Phase 4."* — exposes 0 toggles beyond a flux-boundary combo. |
| **5** Field visualisation (18 kinds) | **NOT STARTED** | `fields.cpp`, `inspector.cpp` stubs; 0 of 18 field kinds; no P1/P5 renderer work. |
| **6** Instrumentation (charts) | **NOT STARTED** | `telemetry/audit/lagrangian.cpp` stubs; `History::find()` hardcodes `return nullptr`; scheduler `enabled_mask` pinned 0. |
| **7a** Workflow | **PARTIAL (~70%)** | `scenario_meta.h` (130 rows) + grouped native browser (title, category, epistemic tooltip). Missing: authored descriptions, min_lattice, persistence/export. |
| **7b** Theme authoring | **NOT STARTED** | `parse_theme()` grammar exists but no `themes/` dir, no external `.theme` loading, no hot-reload. |

**Working panels at runtime:** Scenarios (grouped titles from `SCENARIO_META`, load), Run config (lattice reboot via presets), Play bar (viewport chrome), Substrate (inject harness), Physics terms (flux boundary only), command palette (Ctrl+K over panels + actions), plus menu/workspaces/theme/status/dockspace. **Placeholder panels (one line of text each):** Telemetry, Audit, Lagrangian, Inspector, Fields, Log.

## 1.3 What is reusable vs Scale-0-specific (the multi-scale readiness map)

**Reusable as-is for any scale** (carries no Scale-0 assumptions): the transport (`CommandQueue`, `SnapshotPublisher`, `ParameterJournal`); the shell (`UiShell`, dockspace, `PanelRegistry`, `Workspace`, `Theme`/`apply_theme`, the DockBuilder recipe); the presenter + CUDA↔D3D12 interop + overlay + capture seams; and the whole lint/vendoring/test harness. `DockSlot`, `PanelContext`, `Panel`, and `PanelRegistry` are scale-agnostic by design. `NativeFrame` (`{x,y,z,r,g,b,size}` colored points) is ~60% scale-generic — the point path fits Scale-1 particles, Scale-5 bodies, Scale-4 planets, Scale-2 atoms.

**Hardcoded to Scale 0 / `RenderBridge` — must be generalized:**
- **No `ScaleEngine` seam in the shell.** `NativeEngineSession` owns a `std::unique_ptr<RenderBridge>` directly (`engine_session.h:157`); every accessor (`tick`/`capture`/`current_tick`/`backend_name`) reaches into `RenderBridge`.
- `command_applier.cpp` is entirely `RenderBridge`-shaped (`bridge.toggles`, `TOGGLE_SPECS`, the six knobs, `inspect_voxel`, `continuity_step`).
- `UiSnapshot` embeds Scale-0 types (`TermToggles`, `EnergyLedger`, `VoxelInspection`, `TelemetrySnapshot`); the `UiCommand` variant is Scale-0 vocabulary (`SetToggle`, `SetBoundary`, …).
- **The D3 forward-compat mechanism is asserted but not realized.** `ScenarioMeta::scale` — the column SPEC_UI_V2 §5.1 promised as the "Scale 1 is rows, not a schema migration" guarantee — does not exist, because `scenario_meta.h` was never built.

**Crucially, the per-scale engines are already available.** `particle_engine.cpp`, `atom_engine.cpp`, `cosmic_engine.cpp` are compiled into `ftd_core` (`engine/CMakeLists.txt:340-342`), all derive from `ScaleEngine` (`scale_engine.h`), and `ftd_native_session` already links `ftd_core`. Wiring another engine is a **code** change (generalize the session), not a build change.

## 1.4 Defects and risks found in the current code

- **R-NOW-1 (Critical): the entire delivery is uncommitted**, including the untracked vendored `thirdparty/imgui` / `thirdparty/implot`. A fresh checkout does not configure or build; the golden-green status is a local claim that has never survived a commit or CI run. 26 untracked + 12 modified files under `native_desktop`.
- **R-NOW-2 (High): the W9 scenario-validation-rejection path silently corrupts state, reachable today.** `boot()` (`engine_session.cpp:126-132`) treats both meanings of `dispatch_scenario()==false` identically: it seeds `demo-pair` on top of a possibly half-mutated toggle registry. Because that path never throws, `command_applier`'s `LoadScenario` try/catch never fires and `ReloadStatus::ValidationRejected` is unreachable — a known-but-invalid scenario is silently mishandled from the scenario browser.
- **R-NOW-3 (Low): run-identity fidelity gaps.** `ui_snapshot_builder.cpp:33` hardcodes `env.thread_count = 1` instead of `omp_get_max_threads()` (§2.3); run config uses 10 lattice presets, not the free integer range 4–256 (§5.2).
- **R-NOW-4 (Note): `NativeEngineSession::debug_bridge()` exposes `RenderBridge&`** (`engine_session.h:148`) — SPEC_UI_V2 §3.4b said never to add this. It is marked TEST-ONLY and confined to the sim-thread target, so D6 holds where it matters, but it is a door the multi-scale refactor should close deliberately.

## 1.5 Verdict

The foundation is a strong, high-quality base to build on — the risky architecture is done. The remaining work is two large, mostly-additive fronts: **(A) finish the Scale-0 instrument** (the specced-but-unbuilt Phases 3b–7), and **(B) generalize the session to host the other engines** (the multi-scale pivot the shell was designed for but has not yet exercised). Before either, the working tree must be committed and CI-green, and the live W9 defect fixed.

---

# Part 2 — The port game plan

## 2.1 Target end state

One native Windows application that hosts **all seven scales in-process** through a common `ScaleEngine` seam, running each scale's real C++ physics on **native CUDA** where a GPU backend exists, rendering with **D3D12**, and instrumented with the full ImGui/ImPlot panel surface — at parity with (and beyond) the web dashboard's scientific reach, without any browser, WASM, WebSocket, WSL2, or WebView2 boundary. The physics-accuracy contract (SPEC_UI_V2 §2, the golden gate) holds unbroken throughout.

## 2.2 Guiding principles

1. **Reuse the foundation; generalize, don't rewrite.** The transport, shell, presenter, interop, and test harness carry over untouched. The port adds a `ScaleEngine` seam beneath the session and per-scale panel/command/render vocabulary above it.
2. **Wire the dark engines before writing new physics.** `AtomEngine` (+CUDA, +Barnes-Hut) and `CosmicEngine` (18-phase, true SPH) already exist and are tested; the CUDA `ParticleEngine` backend exists. The accuracy/performance win is mostly a wiring and unit-reconciliation job (REF §11.1).
3. **One source of truth.** Consume `ontic.h` (constants), `TOGGLE_SPECS[]` (toggles), and `dispatch_scenario` (scenarios) directly; do not port the web's JS mirror layers or their parity guards (REF §8).
4. **One panel discipline.** Every panel is one file + one `PanelRegistry` line behind the `Panel` vtable — no `window.__ftd*`-style globals, no per-panel frame loops (REF §9.3). Panels are keyed to scales by `ScenarioMeta::scale`.
5. **The golden gate is inviolable.** Every phase runs the merge gate (SPEC_UI_V2 §2, §9.4). Physics results never change; the UI only observes and drives at tick boundaries.

## 2.3 Architecture decision for the pivot (recommended)

Generalize by **composition, not a mega-variant**:
- `NativeEngineSession` holds a `std::unique_ptr<ScaleEngine>` (the base already provides `tick`/`run`/`dt`/`get_toggle`/`set_toggle`/`entity_count`/`base_diagnostics`/`scale_level`/`scale_name`) plus a per-scale **adapter** that owns the scale-specific pieces the base does not cover: the visual-capture producer (→ `NativeFrame`), the command applier, the snapshot builder, and the panel set.
- The `UiCommand`/`UiSnapshot` split into a **scale-common core** (loop control, scenario load, lattice/reboot where meaningful, capture/inspect requests) plus a **scale-namespaced payload** (Scale-0 keeps its `SetToggle`/`SetBoundary`/field vocabulary; Scale-1 gets `pe*` commands; etc.). This avoids a combinatorial variant explosion while keeping each scale's applier statically typed.
- `ScenarioMeta` gains its promised `scale` column, and `PanelRegistry` filters panels by the active scale — realizing the D3 mechanism the spec asserted but never built.

This keeps the Scale-0 code essentially as-is (its adapter is the current session logic), and each new scale is an adapter + panels, never a schema migration.

## 2.4 The phases

Dependencies are explicit. Phases P0 and the *infrastructure* deliverables of P1 gate P2; P3/P4 build on P2; P5 is cross-cutting polish. Within a phase, items may parallelize.

### Phase P0 — Commit and stabilize the foundation `[S–M, blocking]`

The foundation is done but unproven-in-CI and carries one live defect. Nothing else should start until this is closed.

- **P0.1** Commit the working tree so a fresh clone builds: `git add` the untracked `thirdparty/imgui`, `thirdparty/implot`, `native_desktop/src/ui/**`, panels, assets, the new tests, and the modified CMake/lint files. Verify a **clean checkout configures and builds** on the 14.44/CUDA-13 toolchain.
- **P0.2** Fix **R-NOW-2 (W9)**: make `dispatch_scenario`'s two `false` meanings distinct in `boot()`; on validation-rejection surface `ReloadStatus::ValidationRejected` and re-boot to a known-good scenario rather than seeding `demo-pair` over a half-mutated registry. Add the missing regression test.
- **P0.3** Fix **R-NOW-3**: `thread_count = omp_get_max_threads()` in the snapshot builder; free-integer lattice 4–256 in run config.
- **P0.4** Run the golden/merge gate and the full `native_desktop` CTest label through an actual commit + CI, so "golden-green" is a gate result, not a local claim.
- **Exit:** fresh clone builds; golden + `native_desktop` labels green in CI; W9 path reaches `ValidationRejected` and recovers. **Risk:** low. **Effort:** S–M.

### Phase P1 — Finish the Scale-0 instrument `[XL]`

Deliver SPEC_UI_V2 Phases 3b → 6 plus 7a's `scenario_meta.h`. This is the largest single body of work, it is **fully specced already**, it delivers the highest-value/lowest-risk outcome (a genuinely useful native Scale-0 instrument at CUDA speed — the biggest performance + accuracy win), and it forces the reusable panel/field/telemetry/inspector/scenario infrastructure to maturity so the pivot has something real to generalize.

- **P1.1 (Phase 4)** The 44-toggle `TOGGLE_SPECS` table + 10 config fields, `SetToggleProfile`, the W5 mode-switch confirmations, the W18 `matched_gauss_dynamics`/`strict_validation` handling, and the toggle-state oracle test. Build the `widgets/toggle_table` widget. `[L]`
- **P1.2 (Phase 5)** All 18 `VisualFieldKind` values with honest legends (P2/P3), the §2.2 field-sampler cadence + cost badges, field render modes (point/arrow/slice), **the P1 alpha-sort/depth-write fix and P5 colourblind-safe ternary ramp** (both binding §2.4 clauses), and command-driven field selection (retire the `append_flux` `FluxVector` hardcode). `[L]`
- **P1.3 (Phase 6)** Implement `History` (currently returns `nullptr`), the five ImPlot chart panels (Diagnostics/Audit/Lagrangian/EnergyLedger/Gravity) with per-group-tick charting (P7) and real gaps (P4), wire `DataNeeds` into the existing `NativeTelemetryScheduler::Demand`, the D11 GPU-ledger sync mode, and voxel picking → Inspector. `[L]`
- **P1.4 (Phase 3b)** The Ctrl+K command palette over panels + actions (extended by later phases). **DONE.** `[M]`
- **P1.5 (Phase 7a-meta)** Build `engine/include/ftd/scenario_meta.h` **with the `scale` column** (the P2 dependency), machine-generated rows + parity test, grouped scenario browser. `[M]`
- **Exit:** SPEC_UI_V2 Scale-0 parity; all 44 toggles + 18 fields + 5 chart panels + inspector live; the P1/P5 readback and ΔE₀₀ tests green; golden green. **Risk:** medium (fully specced, but large). **Effort:** XL.

> **Gate to P2:** P2 depends on P1's *infrastructure* — the `Panel`/`History`/field/inspector/telemetry patterns and `scenario_meta.h` — not on every last polish item. P1.2's renderer depth and Phase-7b theming may run in parallel with or after the pivot.

### Phase P2 — The multi-scale pivot `[L]`

The architectural generalization that unlocks every other scale. Localized to the session and the command/snapshot vocabulary (Part 1.3); the shell/transport/presenter carry over.

- **P2.1** Introduce the `ScaleEngine` seam under `NativeEngineSession` (§2.3): hold a `ScaleEngine*` + a per-scale adapter interface (capture producer, command applier, snapshot builder, panel set). Refactor the current Scale-0 logic to *be* the Scale-0 adapter, unchanged in behavior.
- **P2.2** Split `UiCommand`/`UiSnapshot` into a scale-common core + scale-namespaced payload; make `command_applier`/`ui_snapshot_builder` dispatch through the adapter.
- **P2.3** Add the scale switcher to the shell (menu/mode select → swap engine + adapter + panel set, filtered by `ScenarioMeta::scale`); handle interop/backend teardown on switch the way `boot()` handles reload.
- **P2.4** Close R-NOW-4: retire or fully quarantine `debug_bridge()` under the adapter boundary.
- **P2.5** Prove the seam with a **minimal second scale** — host `ParticleEngine`: tick + particle capture into `NativeFrame` + one or two panels (run config + a diagnostics readout). No rendering depth yet.
- **Exit:** two scales hosted in one native app via the `ScaleEngine` seam; scale switching works; the golden gate (Scale 0) still green; the Scale-0 adapter is behaviorally identical to pre-pivot. **Risk:** medium-high (the variant/adapter design is the crux — mitigate with a spike on P2.2 first). **Effort:** L.

### Phase P3 — Wire the real, CUDA-capable engines `[L–XL]`

The performance + accuracy payoff: turn three JS mocks (and one CPU-only real engine) into native, CUDA-backed, in-process engines.

- **P3a Scale 1 — `ParticleEngine` `[M–L]`.** Full adapter: catalog/Zoo injection, the ⤴ promotion pipeline (coarse-grain the live Scale-0 lattice → particles), particle-cloud rendering, PE telemetry. **Enable the CUDA `gpu_particle_engine` backend in-process** (verify its kernels are compiled into `ftd_cuda`; enable if not — REF §5.1). Expose the engine's own `force_diag_` rather than re-deriving forces (REF §5.1 debt).
- **P3b Scale 2/3 — `AtomEngine` `[L]`.** **Build the Planck↔Bohr unit shim** — the specific blocker that keeps the compiled engine dark in the browser (REF §5.2, §8.1). Then wire the engine (+ Barnes-Hut + CUDA backend), molecular rendering (bonds, orbital shells/lobes — the `InstancedMesh` set from REF §6.2), and atom/molecule scenarios. Collapse Scales 2 and 3 into one atoms/molecules adapter with a scene loader.
- **P3c Scale 5 — `CosmicEngine` `[L]`.** Wire the existing 18-phase engine (true SPH, real `f_hubble`, GW), cosmic-body rendering (the multi-layer black-hole + Doppler-disk + jet shaders from REF §6.3), and cosmology telemetry. Delete the JS-mock physics conceptually.
- **Exit:** all real-physics scales (0, 1, 2/3, 5) run in-process on native CUDA; the mock physics is retired for them; per-scale golden/parity checks green. **Risk:** medium (P3b's unit shim is a genuine design problem — spike it early). **Effort:** L–XL.

### Phase P4 — Complete coverage and rendering depth `[L]`

The scales with no native engine and the remaining renderer surface.

- **P4a Scale 4 — Planetary `[M]`.** A new C++ N-body `ScaleEngine` (reuse the cosmic Barnes-Hut/Verlet), exoplanet seeds, procedural-terrain HLSL shaders (fbm displacement), planetary telemetry. Add merger/collision handling and an energy audit the web mock lacks (REF §5.4).
- **P4b Meta (Scale 6) `[S–M]`.** A geometry/label adapter (no physics tick): the 27-site Moore decomposition, polyhedra, symmetry elements, framework-integer labels — a D3D12+ImGui geometry port (REF §5.6).
- **P4c Renderer depth `[L]`.** Port the remaining GLSL→HLSL inventory (REF §6.3): the point-sprite billboard system (the one shader that covers most point clouds), field overlays, the topology rubber-sheets, and the OIT/depth decision for translucency (REF §6.5). Backgrounds are optional.
- **P4d Content `[S]`.** Knowledge base / FAQ / tooltips as embedded C++ string tables (REF §9.4).
- **Exit:** all seven scales native at parity. **Risk:** medium (P4c is the largest greenfield; REF §6 is the inventory). **Effort:** L.

### Phase P5 — Workflow, persistence, and polish `[M]`

SPEC_UI_V2 Phase 7 completion, extended cross-scale.

- Cross-scale scenario metadata + admission; PNG/CSV/reproduction-bundle export; the §8 settings-persistence boot order; external `.theme` files + hot reload (Phase 7b); the command palette extended across scales and scenarios; session persistence.
- Decide the disposition of the web dashboard (retire as the multi-scale surface, or keep as a portable/reference surface — see D2).
- **Exit:** production-quality native multi-scale instrument; SPEC_UI_V2 fully delivered and extended to all scales. **Risk:** low. **Effort:** M.

## 2.5 Sequencing rationale, and the alternative

**Recommended order: P0 → P1 → P2 → P3 → P4 → P5.** The logic is both value- and dependency-driven:
- P0 is a hard prerequisite (uncommitted, un-CI'd foundation with a live defect).
- P1 before P2 because the pivot *generalizes* the panel/field/telemetry/inspector/scenario infrastructure — which does not exist yet. Building it once for Scale 0 and then generalizing is far lower-risk than building and generalizing simultaneously, and `scenario_meta.h`'s `scale` column (a P1 item) is a direct P2 dependency. P1 also front-loads the single largest performance + accuracy win (a full native Scale-0 instrument at CUDA speed).
- P3 before P4 because P3 is where the CUDA performance and physics-accuracy payoff lives (the user's stated priorities); the mock-only scales (P4) have no performance story.

**Alternative — breadth-first:** run P2 immediately after P0, host Scale 0 + Scale 1 at a minimal level, then deepen each scale in later passes. This reaches "the entire engine is native" sooner but delivers less depth per scale early and is riskier (it generalizes infrastructure before it is mature). Choose this only if a broad, shallow multi-scale demo outranks a deep, finished real-physics core. **(Decision D1.)**

## 2.6 Risk register

| ID | Risk | Sev | Mitigation |
|---|---|---|---|
| R1 | Foundation uncommitted / never CI'd; fresh clone won't build | **Critical** | P0.1 — commit + verify clean-checkout build + CI before anything else |
| R2 | W9 scenario-rejection corrupts state, reachable now | **High** | P0.2 — distinct handling + re-boot + regression test |
| R3 | Command/snapshot generalization becomes a combinatorial variant explosion | **High** | P2.2 spike; scale-namespaced payload + adapter dispatch (§2.3), not a mega-variant |
| R4 | AtomEngine Planck↔Bohr unit reconciliation is an unsolved design problem | **Med** | P3b — spike the unit shim early; decide native-units-with-display-conversion vs a scene-unit shim (D4) |
| R5 | CUDA backends for particle/atom may not be compiled into `ftd_cuda` | **Med** | Verify in P3a/P3b; enable the kernels if absent |
| R6 | Renderer (Three.js → D3D12) is the largest greenfield | **Med** | REF §6 is the full primitive/shader inventory; do the point-sprite billboard system first (covers most clouds) |
| R7 | Windows-native CUDA is far slower than WSL2 for campaigns | **Med** | Native app targets *interactive* use, where Windows-native CUDA is acceptable; keep WSL2/`ws_server` for measurement campaigns |
| R8 | A UI change silently perturbs physics | **Med** | The golden/merge gate runs every task (SPEC_UI_V2 §2, §9.4); commands drain only at tick boundaries |
| R9 | Scope of the mock-only scales (4, meta) inflates | **Low** | Treat P4a/P4b as parity-not-performance; keep them light unless the owner scopes otherwise (D5) |

## 2.7 Decisions needed from the owner

- **D1 — Sequencing.** Depth-first (recommended: finish Scale 0, then pivot) vs breadth-first (pivot early, all scales shallow first). §2.5.
- **D2 — Web dashboard disposition.** Does the native app **replace** the web dashboard as the multi-scale surface, or **coexist** with it? SPEC_UI_V2's non-goals said the web remains the multi-scale surface; "replan the entire engine natively" implies replacement. This changes P5 scope and whether the web tree is maintained.
- **D3 — Generalization design.** Confirm the composition/adapter approach (§2.3) over a per-scale session or a monolithic command variant.
- **D4 — AtomEngine units.** Build a Planck↔Bohr scene shim, or run the AtomEngine in its native ontic units and convert only at display?
- **D5 — Mock-scale ambition.** Do Scales 4 (planetary) and Meta need full native parity, or a lighter treatment given they have no CUDA performance story?

## 2.8 One-screen summary

| Phase | Goal | Effort | Gate |
|---|---|---|---|
| **P0** | Commit + CI-green the foundation; fix live W9 defect | S–M | fresh clone builds; golden green; W9 recovers |
| **P1** | Finish the Scale-0 instrument (SPEC Phases 3b–6 + scenario_meta) | XL | Scale-0 parity; all toggles/fields/charts/inspector live |
| **P2** | Multi-scale pivot: `ScaleEngine` seam + scale switcher | L | two scales hosted; Scale-0 golden still green |
| **P3** | Wire the real CUDA engines (Scale 1, 2/3, 5) | L–XL | real-physics scales native on CUDA; mocks retired |
| **P4** | Mock-only scales (4, Meta) + renderer depth + content | L | all seven scales native at parity |
| **P5** | Workflow, export, theming, cross-scale UX | M | production-quality multi-scale instrument |

The foundation is strong; the risky architecture is done. The port is now two large, mostly-additive fronts — finish Scale 0, then generalize to host the engines that already exist — gated by one prerequisite: commit what's there and prove it builds.
