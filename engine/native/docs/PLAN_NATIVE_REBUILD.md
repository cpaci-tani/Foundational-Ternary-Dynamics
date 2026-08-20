# PLAN — Native Desktop Rebuild

**Status:** `[PLAN — DRAFT for owner review]` · **Created:** 2026-08-20 · **Scope:** a clean-slate native Windows application (in-process C++/CUDA physics, D3D12 rendering, custom Direct2D/DirectWrite UI) architected for all seven FTD scales from day one, reusing the C++ physics engine and deliberately salvaging the proven pieces of the current `native_desktop` shell.

**Supersedes** the incremental approach of [`PLAN_ENGINE_PORT.md`](PLAN_ENGINE_PORT.md) (which extended the Scale-0 shell in place). The audit in that document's Part 1 still stands and is the evidence base for the salvage manifest below.

**Companions:** [`REF_WEB_ENGINE_ARCHITECTURE.md`](REF_WEB_ENGINE_ARCHITECTURE.md) (the web feature reference) · [`SPEC_UI_V2.md`](SPEC_UI_V2.md) (the Scale-0 panel catalogue, physics-accuracy contract, and engine hazards W1–W24 — all still binding on the new build).

---

## 0. Mandate and standing decisions

Settled with the owner on 2026-08-20. Do not relitigate; raise a new decision instead.

| # | Decision | Consequence |
|---|----------|-------------|
| **M1** | **Fresh architecture, salvage proven parts.** Design a new native architecture built for all seven scales; reuse the pieces the audit rated excellent rather than re-incur their risk; rebuild the session, command/snapshot model, panels, and scale layer clean. | The current `native_desktop` is the *salvage source*, not the thing being extended. §2 is the manifest. |
| **M2** | **Reuse the C++/CUDA physics engine (`ftd_core`) as a library.** The native app is a fresh *consumer* of `RenderBridge`/`ParticleEngine`/`AtomEngine`/`CosmicEngine`, not a rewrite of them. | The physics core is not in scope for rewrite. New engines (Planetary, Meta geometry) and the AtomEngine unit shim are *additions* to `ftd_core`, kept WASM-safe (M3). |
| **M3** | **The web dashboard coexists.** `ftd_core` stays shared and WASM-compilable; the web remains the portable/shareable surface, native is the workstation surface. | Every `ftd_core` change must keep the WASM build, the web Playwright suite, and `ws_server` green. New engines added to `ftd_core` become available to the web too (a shared win). Native does not have to *replace* the web. |
| **M4** | **Priorities, in order: (1) all scales from day one, (2) maximum performance, (3) clean, maintainable architecture.** | The scale-generic core is validated against ≥2 structurally-different scales before it is trusted (§5 R1). Performance drives the render and interop design (§4.4). |
| **M5** | **Build in a NEW directory as a genuine clean slate** (decision C, 2026-08-20). Copy in only the proven pieces (§2.1); leave the current `native_desktop` untouched as the salvage source so nothing stale bleeds into the rebuild. | R0 is a clean-tree carve, not an in-place refactor. New tree proposed at `engine/native/` (§5 R0 / the R0-R1 spec). |
| **M6** | **The full visual surface is in scope** (decision D, 2026-08-20): every scale renderer, **all field/phenomena overlays** (including the pedagogical JS-proxy overlays, ported with honest legends), and **all backgrounds**. **Only the Knowledge base / FAQ content system is deferred.** | Parity is broad, not a triage. The renderer (§4.4) is the long pole and its scope now includes the 5 background shader passes and the full overlay set; KB/FAQ is the one explicit v1 cut. |
| **M7** | **The UI is a custom Direct3D 12 + Direct2D/DirectWrite layer — NO Dear ImGui** (decision 2026-08-20). Drawn in the engine's own D3D12 device: one command list, no compositing boundary with the CUDA-interop 3D scene. | The salvaged ImGui shell / panels / theme / palette are **not** reused — they are rebuilt on the custom UI (§2 revised); the docking and widget set are ours to build (what ImGui gave for free). The presenter's opaque `OverlayRecorder` seam stays; its ImGui implementation is replaced by a D2D/D3D12 one. Adds a **UI-framework workstream** (§5). `SPEC_UI_V2` drops to a *requirements* reference (which panels, what IA), no longer the implementation spec. Dear ImGui / ImPlot are removed from `engine/native`; the vendored copies stay only for `native_desktop` until it is retired. |

---

## 1. What the rebuild is, in one paragraph

One native Windows application that holds a **`ScaleEngine`** at a time behind a uniform **scale adapter**, ticks it on native CUDA, captures a **scale-generic draw list** the D3D12 renderer consumes through a small fixed set of pipelines, drives it through a **scale-generic command/snapshot spine**, and instruments it with **one panel model** in a **custom Direct3D 12 + Direct2D/DirectWrite UI** (no Dear ImGui — M7). The physics engine (`ftd_core`) and the derisked native primitives (the D3D12 presenter + CUDA↔D3D12 interop, the transport, the session, the test/lint harness) are reused; the UI shell, panels, theme, and palette are built fresh on the custom D3D12/D2D layer, and the Scale-0-shaped session, command vocabulary, snapshot, and render-data model are redesigned to be scale-generic from the first line.

---

## 2. Salvage manifest — keep, redesign, add

### 2.1 Keep as-is (proven; the audit rated these high-quality)

- **The D3D12 presenter and its interop path** — `src/d3d12_presenter.cpp` (1,250 L): CUDA↔D3D12 shared buffer + fence interop, the ImGui-free overlay-record and capture seams, SRV heap management, the frame-in-flight fencing. Generalize only its *input* (see §4.4); the machinery stays.
- **The transport primitives** — `CommandQueue` (FIFO + coalesce), `SnapshotPublisher` (mutex `shared_ptr<const>` publication), `ParameterJournal` (append/export/replay). Scale-agnostic already; keep the mechanism, redesign the payloads they carry.
- **The presenter's `OverlayRecorder` seam** — the opaque, ImGui-free hook the UI draws through (`SPEC_UI_V2` §3.5). The seam stays; its ImGui implementation is dropped and replaced by a Direct2D/DirectWrite recorder (M7).
- **DPI** — per-monitor-V2 awareness + `WM_DPICHANGED` (the ImGui font-atlas rebuild is replaced by DirectWrite font handling).
- ⚠ **NOT kept (M7):** the ImGui shell / dockspace / theme / workspace / command palette, and the vendored ImGui + ImPlot. These were the whole reason the audit rated the "shell" reusable; with ImGui out, the shell is a rebuild (§2.2). The `PanelRegistry`/`Panel`/`DockSlot`/`PanelContext` *concepts* carry over as a native design, not as ImGui code.
- **The test/lint harness** — the four source lints (UI-model include allowlist, boundary, panels, theme-token), `ui_test_inventory` floor, the golden/merge gate, the neutrality-gate *pattern* (N1–N6), and the L0/L1/L2 test tiers. This is the quality machine; it carries over and expands.
- **The threading model** — sim thread owns the engine(s) exclusively; GUI thread only reads the published snapshot and calls the presenter. This is correct and stays (§4.5).

### 2.2 Redesign (Scale-0-shaped today; rebuilt scale-generic)

- **`NativeEngineSession`** (`RenderBridge`-hardcoded) → **`ScaleHost`** + **`ScaleAdapter`** (§4.1).
- **`UiCommand` / `UiSnapshot`** (Scale-0 vocabulary + embedded `TermToggles`/`EnergyLedger`/`VoxelInspection`) → a scale-common core + scale-namespaced payloads (§4.2).
- **`command_applier` / `ui_snapshot_builder`** → adapter-dispatched.
- **`NativeFrame`** (`{x,y,z,r,g,b,size}` colored points only) → a scale-generic **`DrawList`** of typed primitives (§4.3).
- **The entire UI layer** (M7) — the shell (dockspace / menu / status bar), theme, workspace persistence, command palette, and all panels — rebuilt on the **custom D3D12 + Direct2D/DirectWrite** framework (§4.6). We build the docking + widget set ourselves; the IA and panel catalogue come from `SPEC_UI_V2` as requirements, not as ImGui code.

### 2.3 Add (new `ftd_core` code, WASM-safe per M3)

- **A `PlanetaryEngine : ScaleEngine`** — Scale 4 has no native engine; write one (reuse the cosmic Barnes-Hut/Verlet). With merger/collision handling and an energy audit the web mock lacks.
- **A Meta geometry provider** — Scale 6 is geometry, not physics; a small `ScaleEngine`-shaped (or rendering-only) provider for the 27-site Moore decomposition.
- **The AtomEngine Planck↔Bohr unit shim** — the specific blocker keeping the compiled `AtomEngine` dark; a shared win that also unblocks the web (M3).
- **GPU-backend enablement** — verify/enable the `gpu_particle_engine` / `gpu_atom_engine` CUDA kernels in `ftd_cuda`.
- **`scenario_meta.h` with the `scale` column** — the D3 forward-compat table that was asserted but never built; the key to per-scale panel/scenario filtering.

### 2.4 Fix in passing

The live **W9 defect** (`boot()` conflates the two `dispatch_scenario()==false` meanings and seeds `demo-pair` over a half-mutated registry) is in code being redesigned; the new `ScaleHost::boot` handles rejection distinctly and re-boots to a known-good scenario. Also close the run-identity gaps (`thread_count`, free-integer lattice) and the `debug_bridge()` escape hatch.

---

## 3. Layered architecture

```
┌─ Layer 4 · UI ─────────────────────────────────────────────────────────┐
│ custom D3D12 + Direct2D/DirectWrite UI · one Panel model · per-scale panels│
│ command palette · theme · workspaces · status/menu                       │
└───────────▲ reads UiSnapshot ─────────────── pushes ScaleCommand ▼───────┘
┌─ Layer 3 · Render ─────────────────────────────────────────────────────┐
│ D3D12 presenter + CUDA↔D3D12 interop (salvaged)                          │
│ + fixed pipeline set consuming a scale-generic DrawList (new, §4.3-4.4)  │
└───────────▲ DrawList ─────────────────────────────────────────────────── ┘
┌─ Layer 2 · Spine ──────────────────────────────────────────────────────┐
│ CommandQueue · SnapshotPublisher · ParameterJournal (salvaged mechanism) │
│ carrying scale-generic command/snapshot payloads (new, §4.2)             │
│ NativeTelemetryScheduler + History (generalized per scale)               │
└───────────▲ tick / capture / observe ─────────────────────────────────── ┘
┌─ Layer 1 · Host ───────────────────────────────────────────────────────┐
│ ScaleHost — owns one ScaleEngine + its ScaleAdapter; scale switch;        │
│ boot/reload/interop lifecycle; sim-thread ownership (new, §4.1)          │
└───────────▲ ScaleEngine* + adapter ───────────────────────────────────── ┘
┌─ Layer 0 · Physics (reused ftd_core, M2) ──────────────────────────────┐
│ RenderBridge(0) · ParticleEngine(1) · AtomEngine(2/3) · CosmicEngine(5)  │
│ + PlanetaryEngine(4, new) + MetaGeometry(6, new) — all : ScaleEngine     │
│ one source of truth: ontic.h · TOGGLE_SPECS · dispatch_scenario          │
└─────────────────────────────────────────────────────────────────────────┘
```

The salvaged pieces sit at Layers 3–4 and half of 2; the fresh design is Layer 1, the payloads in Layer 2, and the `DrawList` seam between 2 and 3. Layer 0 is reused wholesale plus two new engines.

---

## 4. The core new design

### 4.1 `ScaleHost` and the `ScaleAdapter` seam

The one abstraction that makes the app scale-generic. The host owns exactly one active scale; a scale switch swaps the engine + adapter (like the web's `engineMode` switch, but one owned pointer, not a bridge fan-out).

```cpp
// The uniform contract every scale implements. Scale-0's adapter is
// essentially the current session logic; each new scale is one more adapter.
struct ScaleAdapter {
  virtual ScaleEngine&   engine() = 0;                    // Layer-0 physics/geometry
  virtual void           boot(const ScenarioMeta&, const RunConfig&) = 0;
  virtual ApplyResult    apply(const ScaleCommand&, ParameterJournal&) = 0;  // §4.2
  virtual DrawList        capture() = 0;                   // §4.3 — sim-thread, engine → primitives
  virtual ScaleSnapshot   observe(const DataNeeds&) = 0;   // telemetry + inspection payload
  virtual const ToggleTable&  toggles() const = 0;         // TermToggles / AtomToggles / … as data
  virtual PanelSet        panels() const = 0;              // which panels this scale shows (§4.6)
  virtual int             scale_level() const = 0;         // == engine().scale_level()
};
```

`ScaleHost` holds `std::unique_ptr<ScaleEngine>` + `std::unique_ptr<ScaleAdapter>`, and owns boot/reload, the interop lifecycle (teardown on scale switch, re-import after reload — the W15 sequence, generalized), and the sim-thread tick/drain/publish loop. **It never exposes the engine** (closing the R-NOW-4 `debug_bridge` door).

### 4.2 Scale-generic command and snapshot

Composition, not a mega-variant (the design that avoids a combinatorial explosion):

- **`ScaleCommand`** = a scale-common core (`Pause`/`Run`/`Step`, `LoadScenario`, `SetRunConfig`, `RequestField`, `InspectAt`, `SetTelemetryDemand`) `+` a `std::variant` of per-scale payloads (`Scale0Cmd` with `SetToggle`/`SetBoundary`/…, `Scale1Cmd` with `pe*`, `Scale2Cmd` with `ae*`, …). The applier dispatches the core itself and hands the scale payload to `adapter.apply()`, statically typed per scale.
- **`UiSnapshot`** = a scale-common core (backend/tick/loop state, `DrawList` handle, `EnvInfo`, `last_applied_seq`) `+` an optional per-scale `ScaleSnapshot` (Scale-0 keeps `TermToggles`/`EnergyLedger`/`VoxelInspection`/`TelemetrySnapshot`; other scales carry their own). Published exactly as today via the salvaged `SnapshotPublisher`.

This keeps the whole SPEC_UI_V2 §2 physics-accuracy contract intact — the tick-boundary drain, the fixed-point flush, the demand-gating, the journal — because those live in the core and the salvaged transport, unchanged.

### 4.3 The `DrawList` — one render-data model for every scale

Generalize `NativeFrame` from colored points to the full primitive set the reference doc's renderer inventory (REF §6) enumerates, so every scale's visuals reduce to a handful of typed primitives:

```cpp
struct DrawList {
  std::vector<PointCloud>  points;    // particles · flux voxels · cosmic bodies · atoms · field samples
  std::vector<LineSet>     lines;     // field vectors · streamlines · bonds · orbits · axes · wireframes
  std::vector<InstanceSet> instances; // force glyphs · nuclei · orbital shells · polyhedra
  std::vector<SheetMesh>   sheets;    // topology rubber-sheets (Scale 0)
  std::vector<CustomPass>  custom;    // per-scale shader passes: BH disk/jets, planet terrain
  HudSpec                  hud;       // legend · axis gizmo · labels · selection
};
```

Each scale adapter's `capture()` emits a `DrawList`; the renderer (§4.4) is a fixed set of pipelines that draw it. Adding a scale never touches the renderer core — it adds primitives to a list and, at most, one `CustomPass` shader.

### 4.4 The renderer — GPU-native, a fixed pipeline set

Priority M4(2) drives this. The web/Three.js fills vertex buffers on the CPU every frame (REF §6.4); the native build does the opposite where it pays:

- **A fixed set of reusable D3D12 pipelines**, one per `DrawList` primitive kind: the **point-sprite billboard** system (the single most reused shader — instanced quads replacing `gl_PointSize`, covers particles/flux/field clouds), instanced-mesh (glyphs/nuclei/shells/polyhedra), line/streamline, deformable sheet, and a slot for per-scale `CustomPass` shaders (the HLSL ports of the BH-disk/jet and planet-terrain GLSL from REF §6.3).
- **Zero-copy CUDA↔D3D12 for every scale's buffers**, not just Scale-0 particles — extend the salvaged interop path so particle/body/atom positions written by the CUDA engine land directly in D3D12 vertex buffers.
- **GPU-native fills** — the flux-volume colormap, field sampling, and cloud expansion the web does on the CPU move into HLSL compute/vertex shaders.
- **OIT decision, explicit** — weighted-blended or per-pixel-linked-list for additive volumes; sorted/depth-prepass for normal-blended sheets/shells (REF §6.5; the P1 clause). Decided once, in the pipeline set.

### 4.5 Threading and the accuracy contract (carried over)

Unchanged from the audited-good model: the **sim thread** owns the engine and runs `tick → drain commands at the boundary → fixed-point flush → observe → build snapshot → publish`; the **GUI thread** only `acquire()`s the immutable snapshot and drives the custom D3D12/D2D UI + the presenter. The golden gate, the tick-boundary command application (C3), the RNG-stream discipline (C4), and the demand-gating safety (§2.2 of SPEC_UI_V2) are contracts on Layer 1–2 and hold by construction. Every scale adapter runs under the same rule.

### 4.6 One panel model, scale-keyed

Every panel is one file behind the salvaged `Panel` vtable + one `PanelRegistry` line carrying its `scales` set (from `ScenarioMeta::scale`). The shell owns `Begin/End`, visibility, the demand-OR, and `History`; panels read `PanelContext` and push `ScaleCommand`. No `window.__ftd*`-style globals, no per-panel frame loop (the web's three-pattern mess, REF §9.3, is designed out from the start). Scale-common panels (run config, log, scenario browser) are written once; scale-specific panels (the 43-toggle table, fields, the particle Zoo, the cosmic BH panel) register against their scale.

---

## 5. The build plan

Phasing reflects M4: the scale-generic core is **validated against two structurally-different scales before it is trusted**, performance is designed in from R1, and parity is filled last.

### R0 — New tree, salvage, commit, harden `[S–M, blocking]`
Stand up the **new clean directory** (M5, proposed `engine/native/`) and **copy in only the §2.1 keep-list** as clean, documented reusable targets (presenter+interop, transport, shell, vendoring, test harness); the current `native_desktop/` stays untouched as the salvage source. **Commit the new tree, including the vendored `thirdparty/imgui`/`implot`** (fixing R-NOW-1's uncommitted-vendoring trap at the outset), and prove a **fresh clone builds** and the salvaged gates pass **in CI**. Fix W9 and the run-identity gaps as the salvaged code is re-homed. See the R0-R1 spec for the file-by-file salvage map, the target layout, and the CMake plan. **Exit:** the new tree configures from a clean checkout, the salvaged device/UI tests pass in CI, and no Scale-0 payload type leaks into a salvaged library.

### R1 — The scale-generic spine `[L]`
Design and build Layer 1–2: `ScaleHost`, `ScaleAdapter`, the scale-common command/snapshot core, and the `DrawList` seam + the minimal renderer core (point + line pipelines). **Validate the abstraction against two structurally-different scales at a minimal level** — Scale 0 (voxel field, `RenderBridge`) *and* Scale 1 (particle N-body, `ParticleEngine`) — each: tick, capture→draw, run config + one readout panel, scale switch between them. This is the M4(1) discipline: if the seam is Scale-0-shaped, Scale 1 exposes it here, cheaply. **Exit:** two scales hosted on the new spine; scale switch works; Scale-0 golden green; no scale-specific type in `ScaleHost`.

### R2 — The instrument layer `[L]`
Build the one panel model and the full scientific surface, first on Scale 0 (the 44-toggle table + 10 config fields, the 18 field kinds with honest legends + the P1/P5 renderer clauses, the five chart panels on `History` (drawn with Direct2D), voxel-pick Inspector), generalizing each widget as the second scale needs it. GPU-native field rendering (§4.4) lands here. **Exit:** Scale 0 at SPEC_UI_V2 scientific parity, on the scale-generic model; `ui_toggle_widget_state_oracle` + the P-clause tests green.

### R3 — Scale build-out `[XL]`
One adapter + renderer module + panel set per remaining scale, each following the R1 design:
- **Scale 1** — full ParticleEngine adapter (catalog/Zoo, ⤴ promotion, PE telemetry) on native CUDA; expose `force_diag_` (drop the second force-law copy).
- **Scale 2/3** — AtomEngine adapter behind the **Planck↔Bohr shim** (§2.3), + Barnes-Hut + CUDA; molecular rendering (bonds/shells); atoms+molecules as one adapter with a scene loader.
- **Scale 5** — CosmicEngine adapter (18-phase, true SPH, real Hubble drag) + the multi-layer BH `CustomPass` shaders.
- **Scale 4** — the new `PlanetaryEngine` + terrain `CustomPass`.
- **Meta** — the geometry provider + polyhedra/label rendering.

**Exit:** all seven scales native, each on its real engine (native CUDA where a GPU backend exists), mock physics retired; per-scale golden/parity checks green.

### R4 — Performance and polish `[L]`
Zero-copy interop for every scale's buffers; GPU-native fills everywhere; OIT finalized; workflow (cross-scale scenario metadata, PNG/CSV/reproduction-bundle export, external `.theme` + hot reload); cross-scale UX and the command palette extended across scales. **Exit:** the performance targets (M4(2)) measured and pinned; production-quality UX.

### R5 — Coexistence and parity fill `[M]`
Stand up the M3 guardrails: the WASM build + web Playwright + `ws_server` gates run in CI alongside the native gates, so `ftd_core` additions (the new engines, the unit shim, `scenario_meta.h`) never break the web. Fill the remaining web-parity gaps that carry weight (parity is a goal, not a gate — triage by value). **Exit:** web and native both green on shared `ftd_core`; documented parity delta.

---

## 6. Coexistence: the shared-core contract (M3)

Because web and native share `ftd_core`, the rebuild is disciplined at exactly one boundary — everything the native app adds to `ftd_core` must stay WASM-compilable and leave the web/`ws_server` paths green:

- **New engines** (`PlanetaryEngine`, Meta geometry) compile under Emscripten (no CUDA-only headers in the shared path; GPU backends stay `#ifdef FTD_ENABLE_CUDA`). Added to `ftd_core`, they become available to the web too — the native rebuild can *improve* the web (e.g. a real Scale-4/5 engine the web could later adopt).
- **The AtomEngine unit shim** is a shared asset — it also unblocks the web's dark `AtomEngine`.
- **`scenario_meta.h`** is engine-side and consumed by both surfaces.
- **CI** runs the WASM build, the web Playwright suite, and the golden/`ws_server` gates on every `ftd_core` change, next to the native gates. This guardrail is R5's first task but the *policy* binds from R0.

The native app is a fresh consumer; it never forks `ftd_core`.

---

## 7. Risk register

| ID | Risk | Sev | Mitigation |
|---|---|---|---|
| R1 | Foundation uncommitted; fresh clone won't build | **Critical** | R0.1 — commit everything incl. vendored ImGui; prove clean-clone build + CI first |
| R2 | The `ScaleAdapter`/command seam turns out Scale-0-shaped | **High** | R1 validates against two structurally-different scales *before* the surface is built on it — the whole point of the R1 gate |
| R3 | AtomEngine Planck↔Bohr reconciliation is an unsolved design problem | **Med** | Spike the shim early (R3, but prototype in R1 if it threatens the schedule); it is also a shared web win |
| R4 | An `ftd_core` addition breaks the WASM/web build | **Med** | M3 CI guardrail: WASM + Playwright + ws_server gates run on every core change |
| R5 | GPU-native renderer is the largest greenfield | **Med** | REF §6 is the primitive/shader inventory; build the point-sprite billboard pipeline first (covers most clouds); fixed pipeline set contains the blast radius |
| R6 | Windows-native CUDA is slower than WSL2 for campaigns | **Low** | The native app is interactive; campaigns stay on WSL2/`ws_server` — unchanged by this plan |
| R7 | A UI/render change silently perturbs physics | **Med** | The golden gate + tick-boundary drain are Layer-1/2 contracts, run every task; every adapter is under the same rule |
| R8 | "Rebuild" scope-creeps into rewriting `ftd_core` | **Med** | M2 is explicit: physics core is reused, not rewritten; only §2.3 additions touch it |

## 8. Decisions — resolved 2026-08-20

- **D-A — `DrawList` fidelity.** ✅ **All five primitive types defined up front**; implement points+lines in R1, the rest as each scale arrives (types exist from day one so no later schema change). See the R0-R1 spec.
- **D-B — Meta.** ✅ **Degenerate `ScaleEngine`** (a tick that no-ops) — keeps `ScaleHost` uniform, no special-case path.
- **D-C — App tree.** ✅ **New directory, clean slate** (M5). Build under a fresh tree; copy only the §2.1 proven pieces; leave `native_desktop/` as the untouched salvage source.
- **D-D — Parity.** ✅ **Full visual surface in scope** (M6): all scale renderers, all overlays (proxies included, honest legends), all backgrounds. **Only KB/FAQ is deferred.**

Newly open (from the R0-R1 spec):
- **D-E — New directory name.** Proposed `engine/native/`; owner to confirm before the salvage copy lands (a name is sticky once CMake wires to it).

## 9. One-screen summary

| Phase | Goal | Effort | Gate |
|---|---|---|---|
| **R0** | Salvage + commit + CI-green; fix W9 | S–M | fresh clone builds; CI green; salvaged libs Scale-0-clean |
| **R1** | Scale-generic spine, proven on 2 scales | L | Scale 0 + Scale 1 on the new host; switch works; golden green |
| **R2** | One panel model + Scale-0 scientific parity | L | toggles/fields/charts/inspector on the generic model |
| **R3** | All scales on real engines (native CUDA) | XL | 7 scales native; mocks retired |
| **R4** | Performance + workflow + polish | L | perf targets pinned; production UX |
| **R5** | Coexistence guardrails + parity fill | M | web + native green on shared ftd_core |

The physics engine and the risky native primitives already exist and are proven. The rebuild spends its effort where it buys the most: a scale-generic architecture designed against two scales before it is trusted, a GPU-native renderer, and one clean panel model — with the C++ core reused and the web kept green beside it.
