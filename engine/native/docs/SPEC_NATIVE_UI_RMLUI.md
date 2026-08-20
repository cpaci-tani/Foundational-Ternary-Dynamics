# SPEC — RmlUi UI layer, rendered through D3D12

**Status:** `[SPEC — DESIGN]` · **Created:** 2026-08-20 · **Decision:** M7 in [`PLAN_NATIVE_REBUILD.md`](PLAN_NATIVE_REBUILD.md). The native UI is **RmlUi** (RML markup + RCSS, a CSS subset) rendered through the engine's **own Direct3D 12 device**, so the UI draws in the presenter's command list — one device, no compositing boundary with the CUDA-interop 3D scene.

This document specifies how RmlUi plugs into the existing native shell: the D3D12 render backend, the frame flow, live-data binding, and the docking we build ourselves. It is the design for the UI workstream; the vendoring of RmlUi 6.2 + FreeType is the prerequisite (in progress).

---

## 1. Why RmlUi fits

RmlUi is renderer-agnostic: it computes layout + style from RML/RCSS and emits **geometry** (vertices, indices, textures, scissor rects) through an abstract `RenderInterface` that *we* implement. So its output is drawn by our D3D12 device, in our command list — the exact property that made ImGui attractive (SPEC_UI_V2 D1), but with CSS instead of immediate-mode C++. It brings FreeType for text and a **data-model** system for binding live C++ values into the DOM.

What we get from CSS (RCSS): the box model, **flexbox**, positioning, `%`/`vw`/`vh` units, media queries, transitions, animations, web fonts, pseudo-classes (`:hover`/`:active`/`:focus`), and cascading. What we still build: **docking** (RmlUi has no dock manager) and a few instrument-specific widgets (charts, the 3D viewport hole).

---

## 2. The D3D12 backend — `RmlD3D12Renderer`

A new module in `engine/native` (owns no window; driven by the presenter). Implements RmlUi's three interfaces:

### 2.1 `Rml::RenderInterface` (RmlUi 6.x compiled-geometry model)

| RmlUi call | D3D12 implementation |
|---|---|
| `CompileGeometry(vertices, indices) → handle` | Create an immutable vertex+index buffer (default heap, uploaded once) from `Rml::Vertex {position: vec2, colour: rgba8, tex_coord: vec2}`; return a handle into a slot table. |
| `RenderGeometry(handle, translation, texture)` | Bind the UI PSO + root args (ortho matrix, `translation` as a push constant, the texture SRV or a 1×1 white default), set the current scissor, `DrawIndexedInstanced`. Records into the **active `ID3D12GraphicsCommandList*`** supplied by the presenter. |
| `ReleaseGeometry(handle)` | Queue the buffers for deferred release (after the in-flight frame retires — reuse the presenter's deferred-release pattern). |
| `LoadTexture(dims, source)` | Decode an image (WIC or stb_image) → upload a D3D12 texture + SRV; return a handle. |
| `GenerateTexture(rgba, dims)` | Upload raw RGBA (RmlUi uses this for the **font atlas**) → texture + SRV. |
| `ReleaseTexture(handle)` | Deferred-release. |
| `EnableScissorRegion(bool)` / `SetScissorRegion(rect)` | Track scissor state; applied per draw via `RSSetScissorRects` (scissor always enabled on the UI PSO, set to full-viewport when RmlUi disables it). |
| *(optional 6.x)* `SetTransform(matrix)` | CSS transforms — fold the 4×4 into the per-draw constant; ship after the base path works. |
| *(optional 6.x)* clip mask (`EnableClipMask` / `RenderToClipMask`) | Stencil-based clipping for non-rectangular CSS clips — deferred; rectangular scissor covers the common case. |

### 2.2 `Rml::SystemInterface`
- `GetElapsedTime()` → seconds from a `std::chrono::steady_clock` started at boot (drives transitions/animations).
- `LogMessage(type, msg)` → the native Log panel / status bar (§2.5 SPEC_UI_V2 failure-visibility contract still applies).

### 2.3 Font engine
RmlUi's default `FontEngineDefault` over vendored **FreeType**. Fonts loaded once at boot via `Rml::LoadFontFace(...)` (the embedded Inter face we already ship in `assets/`).

### 2.4 The UI pipeline (one PSO)
A single small graphics pipeline: **VS** transforms `vec2 position` by `ortho(0,w,h,0) · translate` and passes colour + uv; **PS** samples the bound texture × vertex colour. Alpha blending on (`SRC_ALPHA`/`INV_SRC_ALPHA`), no depth test/write (UI over the scene), scissor enabled, one linear-clamp sampler. HLSL compiled at build (d3dcompiler, already linked). The SRV heap already reserves slots (SPEC_UI_V2 §3.5: index 0 interop, 1 font); RmlUi textures take slots 2+ from the free-list.

---

## 3. Frame flow (fits the existing two-thread model)

Unchanged threading (M-decisions): the **sim thread** owns the engine and publishes `UiSnapshot`; the **GUI thread** owns RmlUi and the presenter.

```
GUI thread, per frame:
  snapshot = publisher.acquire()                     // immutable UiSnapshot
  update_data_models(snapshot)                       // push live values into RmlUi (§4)
  context->Update()                                  // RmlUi layout/style
  presenter.render(scene_view, overlay = [&]{        // OverlayRecorder seam (SPEC_UI_V2 §3.5)
      // 3D scene already drawn by the presenter (interop particles/flux/lattice)
      bind UI PSO + ortho;  context->Render()        // RmlUi → RenderInterface → this command list
  })
```

Input (mouse/keyboard) is fed to `context->ProcessMouseMove/…`; RmlUi turns clicks on `data-event`-bound elements into callbacks that `push(ScaleCommand)` onto the queue (the same command spine). Pointer arbitration: RmlUi consumes events over UI elements; events over the **viewport hole** (a transparent RML element covering the 3D region) pass through to the camera/pick handlers.

---

## 4. Live data — RmlUi data models ← `UiSnapshot`

Replaces per-panel imperative updates. At boot we register data models; each frame we copy the relevant `UiSnapshot` fields in and RmlUi re-renders only the bound elements.

- **Shell model:** `tick`, `physical_time`, `particle_count`, `total_energy`, `backend`, `fps`, `running` → the status bar + play bar RML.
- **Toggles model:** the `TermToggles` (and per-scale toggle tables) as a bound list → the physics-terms panel renders `<toggle>`-styled checkboxes from data; toggling `push`es a `SetToggle` command.
- **Telemetry model:** `History` series → charts. Charts are the one place RCSS can't draw the data itself; a small custom element (`<ftd-chart>`) renders the series with our own D3D12 geometry (or a canvas-like element), styled by RCSS for frame/legend.

The data-binding boundary keeps the panel logic declarative and the C++ side a thin "copy snapshot → model" step, honoring the physics-accuracy contract (the UI only reads the snapshot and pushes commands at tick boundaries).

---

## 5. Docking + instrument widgets (what we build)

RmlUi gives layout/style/events; these we implement on top:
- **Docking**: draggable/resizable panel chrome in RML/RCSS + a C++ (or RmlUi-scripted) dock manager — splitters, tab groups, float/redock, persisted layout (the salvaged `Workspace` concept re-expressed as serialized dock state + an `.rcss` theme). First cut: fixed flexbox layout (left/center/right/bottom) matching SPEC_UI_V2 §4.1; drag-dock second.
- **The viewport hole**: a transparent RML element marking the 3D scene rectangle; the presenter renders the scene there, RmlUi renders chrome around/over it, input over it routes to the camera.
- **Charts**: a custom RmlUi element drawing `History` series through the same UI pipeline.
- **Theme**: RCSS stylesheets = the theme system (replaces the ImGui `Theme` struct). Light/dark/accent via CSS custom properties + `@media`. This is where the "CSS flexibility" pays off directly.

---

## 6. Milestones

1. **M-UI-0 — Vendoring** *(in progress)*: RmlUi 6.2 + FreeType compile as `native_rmlui` / `native_freetype`.
2. **M-UI-1 — First pixel of CSS**: `RmlD3D12Renderer` (RenderInterface + SystemInterface + UI PSO) + an `Rml::Context`; load the Inter font; render a simple RML doc with RCSS (a styled panel + text) **over the live lattice scene** via the OverlayRecorder seam. Proves CSS-in-D3D12.
3. **M-UI-2 — Input + a command**: mouse events into RmlUi; a button in RCSS that `push`es a `Pause`/`Run` command; the play bar works from RML.
4. **M-UI-3 — Data-bound status + toggles**: shell data model from `UiSnapshot`; the status bar + a first physics-terms panel render from data and drive real toggles.
5. **M-UI-4 — Layout + theme**: the §4.1 shell IA in flexbox RCSS; light/dark theme via CSS; the viewport hole with input pass-through.
6. **M-UI-5 — Docking + charts**: the dock manager and the `<ftd-chart>` element.

M-UI-1 through M-UI-3 validate the whole approach cheaply before the full panel build; they gate committing to RmlUi as the production UI (they will succeed — RmlUi's renderer-agnostic model is exactly this integration).

---

## 7. Risks

- **RmlUi text quality / DPI** — FreeType hinting + per-monitor DPI (we keep the salvaged DPI awareness; RmlUi's `dp`/context scale handles UI scaling). Low risk.
- **Charting** — RCSS can't plot series; the custom `<ftd-chart>` element is bespoke D3D12 geometry. Contained.
- **Docking is ours to build** — the one place we don't get RmlUi for free; scope it as its own milestone (M-UI-5).
- **6.2 API specifics** — the exact `RenderInterface` signatures are confirmed against the vendored headers when M-UI-1 is implemented; §2.1 is the 6.x compiled-geometry model.
