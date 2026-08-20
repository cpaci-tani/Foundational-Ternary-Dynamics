# SPEC — Native Rebuild, R0 + R1 (salvage tree + scale-generic spine)

**Status:** `[SPEC — DRAFT, executable]` · **Created:** 2026-08-20 · **Parent:** [`PLAN_NATIVE_REBUILD.md`](PLAN_NATIVE_REBUILD.md) (architecture + phases; decisions M1–M6). This document makes R0 and R1 buildable: the new-directory layout, the file-by-file salvage map, the CMake target plan, and the **complete day-one interface definitions** (decision A — all types defined up front).

**One open item before the salvage copy lands:** the new directory name (D-E). This spec assumes **`engine/native/`**; every path below rewrites trivially if the owner picks another. Nothing is created until that name is confirmed.

---

## 1. New directory layout (`engine/native/`)

A genuine clean slate (M5). Headers grouped by architectural layer so the §3 boundary lints stay mechanical.

```
engine/native/
  CMakeLists.txt
  README.md
  docs/                         SPEC_NATIVE_REBUILD_R0R1.md, later specs
  thirdparty/                   imgui/  implot/           (vendored, COMMITTED at R0)
  assets/                       font_inter_regular.inl, themes/*.theme
  include/native/
    model/    draw_list.h  commands.h  snapshot.h  demand.h  result.h  journal_value.h
    host/     scale_host.h  scale_adapter.h  run_config.h
    render/   presenter.h  scene_rect.h  overlay_recorder.h  pipelines.h
    ui/       panel.h  panel_registry.h  panel_context.h  theme.h  workspace.h
              ui_shell.h  command_palette.h  history.h
    platform/ dpi_support.h  cli_options.h  imgui_assert.h  imgui_font.h
              imgui_overlay.h  ftd_imconfig.h
  src/
    model/    command_queue.cpp  snapshot_publisher.cpp  parameter_journal.cpp
    host/     scale_host.cpp
    host/adapters/  scale0_adapter.cpp  scale1_adapter.cpp  scale2_adapter.cpp
                    scale4_adapter.cpp  scale5_adapter.cpp  meta_adapter.cpp
    render/   d3d12_presenter.cpp  imgui_overlay.cpp
              pipelines/  point_sprite.cpp  line.cpp  instanced.cpp  sheet.cpp
              passes/     bh_disk.cpp  planet_terrain.cpp  background.cpp   (R3+)
    ui/       ui_shell.cpp  theme.cpp  workspace.cpp  panel_registry.cpp  command_palette.cpp
    ui/panels/  run_config.cpp  log.cpp  scenario_browser.cpp
                scale0/  physics_terms.cpp  fields.cpp  telemetry.cpp  audit.cpp
                         lagrangian.cpp  inspector.cpp
                scale1/  …   scale5/  …
    platform/ dpi_support.cpp  cli_options.cpp  imgui_assert.cpp  imgui_font.cpp
    app/      main.cpp
  tests/      (salvaged device/UI tests + new adapter/host tests)
```

---

## 2. Salvage map — file by file

The rule: **copy the proven mechanism; rewrite anything shaped to Scale 0.** The current `native_desktop/` is read-only reference during R0.

### 2.1 Copy verbatim (or near-verbatim — only include-path/namespace edits)

| From `native_desktop/` | To `native/` | Notes |
|---|---|---|
| `thirdparty/imgui`, `thirdparty/implot` | `thirdparty/` | **Commit these** (R-NOW-1 fix). |
| `assets/font_inter_regular.inl` | `assets/` | + any `themes/*.theme` once authored. |
| `src/imgui_assert.cpp`, `imgui_font.cpp`, `include/.../ftd_imconfig.h`, `imgui_assert.h`, `imgui_font.h` | `platform/` | ImGui config/assert/font. |
| `src/imgui_overlay.cpp`, `include/.../imgui_overlay.h`, `overlay_recorder.h` | `render/` | overlay recorder seam. |
| `src/d3d12_presenter.cpp`, `include/.../d3d12_presenter.h`, `scene_rect.h` | `render/` | **the crown jewel** — presenter + CUDA↔D3D12 interop + capture. Only its *input* changes (§4 `DrawList`); the device code is untouched. |
| `src/dpi_support.cpp`, `include/.../dpi_support.h` | `platform/` | per-monitor-V2 + `WM_DPICHANGED`. |
| `src/cli_options.cpp`, `include/.../cli_options.h` | `platform/` | add a `--scale N` flag; drop Scale-0-only assumptions. |
| `src/command_queue.cpp`, `include/.../command_queue.h` | `model/` | FIFO+coalesce **mechanism**; retype its element to the new `ScaleCommand` (§4.2). |
| `src/snapshot_publisher.cpp`, `include/.../snapshot_publisher.h` | `model/` | mutex `shared_ptr<const>` publication; retype to the new `UiSnapshot` (§4.4). |
| `src/parameter_journal.cpp`, `include/.../parameter_journal.h`, `ui_journal.h` | `model/` (+ `journal_value.h`) | journal value types + replay; keys become scale-namespaced. |
| `include/.../ui_demand.h`, `ui_result.h` | `model/demand.h`, `model/result.h` | `DataNeeds`, `ApplyResult`/`TickResult`/`ReloadResult` — already scale-agnostic. |
| `src/ui/ui_shell.{cpp}` + `.h`, `theme.{cpp,h}`, `workspace.{cpp,h}`, `panel_registry.{cpp,h}`, `panel.h`, `history.h`, `command_palette.{cpp,h}` | `ui/` | dockspace, theme, workspace, registry, palette — no Scale-0 assumptions. `panel.h` gains nothing; `PanelContext` gains the active-scale id. |
| `cmake/FtdSourceLint.cmake` (the 4 lints), `FtdUiTestInventory.cmake` | new `cmake/` entries | re-glob for `native/`; keep the boundary/allowlist/theme-token/panels lints + the inventory floor. |
| `tests/test_d3d12_*`, `test_ui_imgui_*`, `test_ui_window_name`, `test_ui_scene_rect`, `test_ui_shell_draw`, `test_ui_theme_parse`, `test_ui_workspace`, `test_ui_snapshot_publisher`, `test_ui_command_queue`, interop tests, `test_native_desktop_dpi_awareness` | `native/tests/` | the device + L0/L1 + interop coverage transfers directly (retype command/snapshot tests). |

### 2.2 Rewrite fresh (Scale-0-shaped → scale-generic)

| Old | New | Why |
|---|---|---|
| `engine_session.{cpp,h}` (owns `RenderBridge`) | `host/scale_host.{cpp,h}` + `host/scale_adapter.h` | §4.1 — owns a `ScaleEngine` behind an adapter; no engine leak. |
| `command_applier.{cpp,h}` (all `RenderBridge`-shaped) | per-adapter `apply()` (in each `host/adapters/*.cpp`) | §4.2 — each scale owns its mutation vocabulary. |
| `ui_command.h` (Scale-0 variant) | `model/commands.h` | §4.2 — core commands + scale-namespaced payloads. |
| `ui_snapshot.h` (embeds `TermToggles`/`EnergyLedger`/…) | `model/snapshot.h` | §4.4 — common core + `ScaleSnapshot` variant. |
| `ui_snapshot_builder.{cpp,h}` | adapter-owned `capture()`/`observe()` | §4.1. |
| `native_frame.h` (colored points only) | `model/draw_list.h` | §4.3 — five primitive types. |
| `src/ui/panels/*.cpp` (Scale-0 stubs) | `ui/panels/**` on the new `Panel` model | rebuilt; scale-common panels once, scale-specific under `panels/scaleN/`. |
| `src/main.cpp` (Scale-0 two-thread loop) | `app/main.cpp` | same two-thread pattern, driven by `ScaleHost` not `NativeEngineSession`. |

### 2.3 Do not copy
The Scale-0 stub panel *bodies* (rewritten), `NativeEngineSession::debug_bridge()` (the R-NOW-4 escape hatch — designed out), and anything that pins a payload type into a salvaged library.

---

## 3. CMake targets (new tree)

Mirrors the proven split, generalized. Boundary lints enforce the arrows.

| Target | Contents | Links |
|---|---|---|
| `native_imgui` (STATIC) | imgui + implot + assert + font | — (config via `ftd_imconfig.h`) |
| `native_imgui_dx12` (STATIC) | imgui win32/dx12 backends + overlay recorder | `native_imgui`, `d3d12`, `dxgi`, `dwmapi` |
| `native_model` (STATIC) | `draw_list`, `commands`, `snapshot`, `demand`, `result`, `journal`, `command_queue`, `snapshot_publisher` | `ftd_core` **for value types only** (allowlisted headers) — **no ImGui, no D3D12, no engine behavior** |
| `native_host` (STATIC) | `scale_host` + all `adapters/*` | `native_model`, `ftd_core`, conditionally `ftd_cuda` |
| `native_render` (STATIC) | `d3d12_presenter` + `pipelines/*` + `passes/*` | `native_imgui_dx12`, `native_model`, `d3d12`, `dxgi`, `d3dcompiler` |
| `native_ui` (STATIC) | shell, theme, workspace, registry, palette, all panels | `native_imgui`, `native_model` — **zero D3D12, zero engine** |
| `native_platform` (STATIC) | dpi_support, cli_options | — |
| `native_app` (WIN32 exe) | `app/main.cpp` | all of the above |

**Lints (salvaged, re-globbed):** `native_model` include-allowlist (no imgui/d3d12/`RenderBridge`/`ScaleEngine`-behavior headers — only the value-type headers); boundary (`main`/panels never touch an engine; presenter never includes ImGui); theme-token (`ImGuiCol_` writes only in `theme.cpp`); panels glob non-vacuity. `native_ui` must not link `native_render` or `native_host`.

---

## 4. Day-one interfaces (decision A — all types up front)

These become the actual headers. Scale-specific payload *members* are filled as each scale arrives, but the **types and the seam exist from the first commit**.

### 4.1 `host/scale_adapter.h` + `host/scale_host.h`

```cpp
namespace ftd::native {

// Scale-common run knobs (a field is 0/ignored where a scale has no analogue).
struct RunConfig {
  int    lattice_size   = 0;   // Scale 0 (0 = n/a elsewhere)
  double dt             = 1.0;
  int    sor_iterations = 0;   // Scale 0
  int    substeps       = 1;   // Scale 4/5 N-body substep multiplier
  // scale-specific setup rides the scale command payload, never this struct.
};

// The uniform contract every scale implements. Scale 0's adapter is the old
// session logic re-homed; each further scale is one more file.
class ScaleAdapter {
 public:
  virtual ~ScaleAdapter() = default;
  virtual ftd::ScaleEngine& engine() = 0;              // Layer-0 physics/geometry
  virtual int         scale_level() const = 0;         // == engine().scale_level()
  virtual const char* scale_name()  const = 0;

  virtual void        boot(const ftd::ScenarioMeta&, const RunConfig&) = 0;
  virtual ApplyResult apply(const ScaleCommand&, ParameterJournal&) = 0;  // §4.2
  virtual DrawList    capture() = 0;                    // sim-thread; engine → primitives (§4.3)
  virtual ScaleSnapshot observe(const DataNeeds&) = 0;  // telemetry + inspection payload (§4.4)

  virtual const ToggleTable& toggles() const = 0;       // TermToggles / AtomToggles / … as data
  virtual PanelSet    panels() const = 0;               // panel ids this scale shows (§4.5)
};

// Factory: given a scale level, build the engine + its adapter. The only place
// that knows the concrete engine/adapter types; everything else is generic.
std::unique_ptr<ScaleAdapter> make_scale_adapter(int scale_level);

class ScaleHost {
 public:
  explicit ScaleHost(NativeOptions);
  ~ScaleHost();
  ScaleHost(const ScaleHost&) = delete;
  ScaleHost& operator=(const ScaleHost&) = delete;

  // Swap the active scale (teardown interop, build engine+adapter, boot).
  ReloadResult switch_scale(int scale_level, const ftd::ScenarioMeta&, const RunConfig&);

  TickResult   tick_once();                             // sim-thread
  TickResult   process_ui_boundary(CommandQueue&);      // drain→apply→flush→observe→publish
  void         consume_pending_step();

  SnapshotPublisher& publisher();
  ParameterJournal&  journal();
  int  active_scale() const;

  // interop lifecycle (generalized W15): re-import after any reload; teardown on switch.
  bool try_enable_interop(void* buf, std::uint64_t bytes, void* fence);
  // NOTE: NO engine accessor is exposed (closes R-NOW-4).

 private:
  std::unique_ptr<ftd::ScaleEngine>  engine_;
  std::unique_ptr<ScaleAdapter>      adapter_;
  SnapshotPublisher   publisher_;   // salvaged
  ParameterJournal    journal_;     // salvaged
  ftd::NativeTelemetryScheduler scheduler_;  // salvaged
  // + ui-boundary/loop state
};

}  // namespace ftd::native
```

### 4.2 `model/commands.h`

```cpp
namespace ftd::native {

// ── scale-common core (ScaleHost handles these directly) ──
struct Pause {};  struct Run {};  struct Step { int n = 1; };
struct LoadScenario     { std::string id; };
struct SetRunConfig     { RunConfig cfg; };
struct SwitchScale      { int scale_level; std::string scenario; };
struct RequestField     { int kind = 0; int stride = 1; };   // scale interprets `kind`
struct InspectAt        { float x = 0, y = 0, z = 0; };      // voxel / particle / body
struct SetTelemetryDemand { DataNeeds needs; };
using CoreCommand = std::variant<Pause, Run, Step, LoadScenario, SetRunConfig,
                                 SwitchScale, RequestField, InspectAt, SetTelemetryDemand>;

// ── per-scale payloads (the owning adapter handles these) ──
// Scale 0 keeps the current SPEC_UI_V2 §3.4 vocabulary verbatim.
using Scale0Cmd = std::variant<SetToggle, SetToggleProfile, SetDouble, SetEnum, SetUInt,
                               SetBoolConfig, SetBoundary, SetDt, SetSorIterations,
                               InjectWavepacket, InjectFluxAdd, CreateEntangledPair,
                               ClearField, SeedRandomFlux>;
using Scale1Cmd = std::variant<PeAddParticle, PeSetToggle, PeSetDt, PeSetSoftening,
                               PePromoteFromLattice, PeInjectFromCatalog, PeClear>;
using Scale2Cmd = std::variant<AeSetToggle, AeLoadElement, AeLoadMolecule, AeSetDt,
                               AeSetSoftening>;
using Scale4Cmd = std::variant<PlSelectSystem, PlSetGravityMode>;
using Scale5Cmd = std::variant<CsLoadScenario, CsSetCameraPreset>;
using MetaCmd   = std::variant<MetaToggleShell, MetaToggleOverlay>;
using ScalePayload = std::variant<std::monostate, Scale0Cmd, Scale1Cmd, Scale2Cmd,
                                  Scale4Cmd, Scale5Cmd, MetaCmd>;

// A command is EITHER a core command OR a scale payload.
struct ScaleCommand {
  CoreCommand  core{Pause{}};
  ScalePayload scale{std::monostate{}};
  bool is_core() const { return !std::holds_alternative<std::monostate>(scale) ? false : true; }
};

}  // namespace ftd::native
```

### 4.3 `model/draw_list.h` — the five primitives (all defined now)

```cpp
namespace ftd::native {

enum class Blend : std::uint8_t { Normal, Additive };
enum class Shape : std::uint8_t { Disc, Square, Diamond, Star, Triangle, Hexagon, Ring, Cross };
enum class SizeMode : std::uint8_t { PerspLinear, SqrtDepth };   // 150/z  vs  sqrt(60/depth)
enum class MeshId : std::uint8_t { Cone, Cylinder, Sphere, Ring, BoxEdges, Octahedron,
                                   Cuboctahedron, Cube };
enum class PassId : std::uint8_t { BhDisk, BhJet, PlanetTerrain, StarSurface,
                                   BgStarfield, BgNebula, BgFoam, BgFluxStorm, BgBeyond };

struct PointCloud {                 // particles · flux voxels · bodies · atoms · field samples
  std::vector<float> pos;           // count*3
  std::vector<float> rgba;          // count*4
  std::vector<float> size;          // count
  Blend    blend = Blend::Normal;
  Shape    shape = Shape::Disc;
  SizeMode size_mode = SizeMode::PerspLinear;
  bool     manifest_blink = false;  // shader blink (Scale 0/1)
};

struct LineSet {                    // vectors · streamlines · bonds · orbits · axes · wireframes
  std::vector<float> verts;         // segment pairs (2N*3) or strip (see `strip`)
  std::vector<float> rgba;          // per-vertex
  float  width  = 1.0f;
  bool   strip  = false;
  bool   dashed = false;
  Blend  blend  = Blend::Normal;
};

struct InstanceSet {                // glyphs · nuclei · orbital shells · polyhedra · bonds
  MeshId mesh = MeshId::Cone;
  std::vector<float> xform;         // count*16 (row-major 4x4)
  std::vector<float> rgba;          // count*4
  bool   lit = false;               // Lambert path (bond cylinders / meta)
};

struct SheetMesh {                  // topology rubber-sheets (Scale 0)
  int nx = 0, ny = 0;
  std::vector<float> height;        // nx*ny
  std::vector<float> rgba;          // per-vertex
  bool wireframe_twin = true;
};

struct CustomPass {                 // bespoke HLSL: BH disk/jets · planet terrain · backgrounds
  PassId pass = PassId::BhDisk;
  std::vector<float> params;        // pass uniforms
  std::vector<float> instances;     // optional per-instance (e.g. one block per BH)
};

struct HudSpec {                    // 2D chrome, drawn by ImGui not a GPU pass
  bool axis_gizmo = true;
  LegendSpec legend;                // ramp bounds · requested/effective stride · units · origin (P2/P3)
  std::vector<LabelSpec> labels;    // billboard text
  bool has_selection = false;  Selection selection;
};

struct SceneParams {                // camera + world hints
  BoundaryShape boundary = BoundaryShape::Cube;
  bool has_background = false;  PassId background = PassId::BgStarfield;
  float cam_target[3] = {0,0,0};
};

struct DrawList {
  std::vector<PointCloud>  points;
  std::vector<LineSet>     lines;
  std::vector<InstanceSet> instances;
  std::vector<SheetMesh>   sheets;
  std::vector<CustomPass>  custom;
  HudSpec                  hud;
  SceneParams              scene;
};

}  // namespace ftd::native
```

The renderer (§4 of the parent) is a fixed pipeline set — one pipeline per primitive kind plus a registered-pass dispatcher for `CustomPass` — consuming this. Adding a scale never edits the renderer core.

### 4.4 `model/snapshot.h`

```cpp
namespace ftd::native {

// per-scale observation payloads (Scale 0 keeps today's UiSnapshot content).
struct Scale0Snapshot { TermToggles toggles; EnergyLedger ledger;
                        TelemetrySnapshot telemetry; VoxelInspection voxel; /* … */ };
struct Scale1Snapshot { /* PE diagnostics + inspection */ };
struct Scale2Snapshot { /* AE diagnostics */ };
struct Scale4Snapshot { /* planetary diagnostics */ };
struct Scale5Snapshot { /* cosmic diagnostics */ };
struct MetaSnapshot   { /* selected-site metadata */ };
using ScaleSnapshot = std::variant<std::monostate, Scale0Snapshot, Scale1Snapshot,
                                   Scale2Snapshot, Scale4Snapshot, Scale5Snapshot, MetaSnapshot>;

struct UiSnapshot {                  // published immutably via the salvaged SnapshotPublisher
  // scale-common core
  int           active_scale = 0;
  int           tick = 0;
  LoopControl   loop;
  BackendInfo   backend;            // kind · interactive_gpu · thread_count
  DrawList      draw;               // §4.3 — what to render this frame
  std::uint64_t seq = 0, last_applied_seq = 0;
  StatusLine    status;             // shell chrome (never hidden)
  // per-scale payload
  ScaleSnapshot scale{std::monostate{}};
};

}  // namespace ftd::native
```

### 4.5 `ui/panel.h` + `panel_registry.h` (salvaged, scale-keyed)

The salvaged `Panel` vtable is unchanged except `PanelContext` gains `int active_scale`, and `PanelRegistry` rows carry a `scales` set filtered from `ScenarioMeta::scale`. One panel = one file + one registration line; scale-common panels (`run_config`, `log`, `scenario_browser`) register for `all`, scale-specific ones for their level. No `window.__ftd*` analogue, no per-panel frame loop — designed out from the start.

---

## 5. R1 acceptance — the two-scale gate

R1 is done when the spine is proven **scale-generic against two structurally-different scales**, not Scale-0-shaped:

1. **Scale 0** (voxel field, `RenderBridge`) and **Scale 1** (particle N-body, `ParticleEngine`) each: `boot` from a scenario · `tick_once` on the sim thread · `capture()` → a `DrawList` the point+line pipelines render · one run-config panel + one readout panel · appear in the scale switcher.
2. `SwitchScale{0↔1}` works: interop torn down and re-established, panels swapped by `ScenarioMeta::scale`, journal continuous.
3. **No concrete scale type appears in `ScaleHost`, `native_model`, or the renderer core** — the boundary lint proves it.
4. Scale-0 **golden gate green** through the new spine (the physics-accuracy contract is untouched).
5. The salvaged device/interop/L0/L1 tests pass in CI on the new tree.

If the `DrawList` or `ScaleCommand` seam has to bend to fit Scale 1, it surfaces here — cheaply, before R2 builds the whole instrument surface on it.

---

## 6. Build order within R0 → R1

1. **R0.1** Create `engine/native/`, copy §2.1, commit **including vendored ImGui/ImPlot**; wire the CMake targets (§3) with the lints; prove the salvaged device/UI tests build + pass in CI. *(No new logic yet — just a green, committed, clean tree.)*
2. **R0.2** Fix W9 + run-identity gaps in the re-homed code.
3. **R1.1** Land the day-one headers (§4) in `model/` + `host/` — types compile, no bodies.
4. **R1.2** `ScaleHost` + the point/line render pipelines + the Scale-0 adapter (re-home the old session/applier/builder behind the adapter). Scale 0 runs; golden green.
5. **R1.3** The Scale-1 adapter (minimal: tick + particle `capture` + run-config panel) + the scale switcher. **The two-scale gate (§5) closes R1.**

---

*Nothing in the new tree is created until the directory name (D-E) is confirmed. On confirmation, R0.1 is a mechanical copy + commit; this spec is the map.*
