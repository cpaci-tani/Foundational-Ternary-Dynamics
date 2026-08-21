# engine/native — in-process native Windows app (RmlUi + D3D12, Win64)

`native_app` runs the FTD engine as a single Windows process. Physics ticks
in-process through `RenderBridge`; the lattice, particles, and flux are drawn
with D3D12 in that same process; and the UI is authored in **RmlUi** (RML markup
+ RCSS, a CSS subset) and rendered through the engine's own D3D12 device — no
Dear ImGui, no embedded browser, no separate render process. On the GPU backend
the particle buffers are shared **CUDA↔D3D12 zero-copy**; the app falls back to a
bounded visual-snapshot gather when interop is unavailable.

Sibling surfaces, which this tree does not touch:

- `engine/web` — the browser / WASM dashboard
- `engine/desktop` — the WPF + WebView2 + WSL2 `ws_server` shell

The prior Dear ImGui prototype (`engine/native_desktop`) was retired 2026-08-21;
`engine/native` is now the sole native app.

## Build

From the repo root, inside the pinned MSVC 14.44 toolset (the wrapper enters it):

```bat
engine\build_native.bat build --target native_app --parallel 32
```

Binary: `engine/build/native/Release/native_app.exe` — **Win64** (`x86-64`),
Windows GUI subsystem (`wWinMain`). Launched from `cmd` it attaches the parent
console, so boot logs still print. The CTest device/unit binaries stay console
apps.

## Run

```bat
engine\build\native\Release\native_app.exe
engine\build\native\Release\native_app.exe --cpu --lattice 48 --scenario s0-seed-hydrogen
engine\build\native\Release\native_app.exe --paused
```

Boot flags — parsed by `native_cli`, they configure the session before the first
tick:

| flag | effect |
|---|---|
| `--gpu` / `--cpu` | backend; **`--gpu` is the default** (CUDA). `--cpu` forces the CPU backend. |
| `--lattice N` | lattice edge length (default `32`) |
| `--scenario NAME` | boot scenario (default `s0-seed-hydrogen`; the in-app picker lists the rest) |
| `--paused` | boot paused. **The default is live** on launch. |
| `--help` | print usage and exit |

A separate `app_options` layer adds headless and scripted-UI flags used by the
smoke and capture harnesses — `--capture-frames N` (render N frames, write a PNG
to `--png-out PATH`, then exit), `--pick-scenario`,
`--open-{telemetry,gravity,time,thermo,spectrum}`, `--toggle-on`/`--toggle-off`,
`--overlays`, `--profile-ui N`, and more. See `src/app/app_options.cpp` for the
full set.

## What you see

A left-hand RmlUi panel composited over the live D3D12 lattice/particle scene —
the panel's transparent `#viewport` hole is where the 3D scene shows through.

- **Information & Scenario** — a searchable, category-grouped scenario picker
  (type to filter by title, id, or tag; each row carries its epistemic-status
  badge; the loaded scenario is highlighted), plus the run controls (play/pause,
  **Step**, **Reset**).
- **Instruments** — collapsible sections, each of which demands its telemetry
  group only while it is open: **Telemetry** (Diagnostics / Conservation /
  Lagrangian), **Gravity** (Poisson latency → time dilation), **Time** (causal
  clock dτ/dt), **Thermo** (bath T / kinetic T / entropy), and **Spectrum**
  (a radial flux |E(k)| chart). Field overlays and force render styles are
  toggled here.

## Controls

The 3D scene is driven with the mouse; run control is through the shell buttons
above (there are no keyboard shortcuts — keystrokes go to RmlUi, e.g. the
scenario search box):

- **Left-drag** — orbit the camera
- **Left-click** (without dragging) — pick the voxel or particle under the
  cursor and open the inspector; clicking a neighbour cell in the inspector walks
  the cursor by that Moore offset through the 26-neighbourhood
- **Wheel** — zoom the camera (clamped)
- **Shift+Wheel** over the scene — sweep the most-recently active rubber-sheet
  overlay up or down through the lattice
- **Play/Pause**, **Step**, **Reset** — buttons in the shell

## Reference

- [`docs/PLAN_NATIVE_REBUILD.md`](docs/PLAN_NATIVE_REBUILD.md) — architecture and
  phase decisions
- [`docs/SPEC_NATIVE_UI_RMLUI.md`](docs/SPEC_NATIVE_UI_RMLUI.md) — the RmlUi UI
  contract (shell/RCSS, data models, the D3D12 render interface)
- [`docs/CHECKLIST_WEB_PARITY.md`](docs/CHECKLIST_WEB_PARITY.md) — running parity
  status against the `engine/web` dashboard
