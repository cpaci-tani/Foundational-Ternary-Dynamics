# Native Windows desktop — in-process D3D12 presenter (Win64)

This is a **separate** Windows application from:

- `engine/web` (browser / WASM dashboard)
- `engine/desktop` (WPF + WebView2 + WSL2 `ws_server`)

It does not modify those trees. Physics runs in-process through
`RenderBridge`. Particles and flux are drawn with D3D12 shaders in the
same process. CUDA–D3D12 shared-buffer/fence interop is implemented; the app
falls back to the bounded visual-snapshot gather when interop is unavailable.

The current shell is the Graphite ImGui dockspace (SPEC_UI_V2 Phase 3b).
The Win32 control strip has been removed. Remaining panel depth (43-toggle
table, fields, charts) lands in Phases 4–6 of [`docs/SPEC_UI_V2.md`](docs/SPEC_UI_V2.md).

## Build

From the repo root, inside the MSVC 14.44 pin:

```bat
engine\build_native.bat build --target ftd_native_desktop --parallel 32
engine\start_native_desktop.bat
```

Binary: `engine/build/native_desktop/Release/ftd_native_desktop.exe` — **Win64** (`x86-64`) with the Windows GUI subsystem (`wWinMain`). CTest device tests stay console apps. When launched from `cmd` / `start_native_desktop.bat`, the process attaches the parent console so boot logs still print.

`--gpu` is the default. Pass `--cpu` to force the CPU backend. The session boots **paused** with `s0-seed-hydrogen` visible. Pass `--no-ui` to skip ImGui and the dockspace shell (bisection tool; the lattice still renders, with no on-screen controls).

```bat
engine\start_native_desktop.bat --lattice 32 --scenario s0-seed-hydrogen
engine\start_native_desktop.bat --cpu --lattice 32
engine\start_native_desktop.bat --no-ui --cpu --lattice 32
```

## Controls

Docked panels (View menu also toggles them; Presentation hides all three docks):

- **Setup** — vertical stacked tabs: Scenarios (grouped honest titles), Run config, Substrate (inject/clear/random flux)
- **Instruments** — Telemetry / Audit / Lagrangian / Inspector placeholders
- **Physics** — flux-boundary combo (full term table is Phase 4), Fields, Log
- **Play bar** — floating capsule over the lattice: Play/Pause, Step, Reset, ticks/second (also visible in Presentation)

View menu: particles, flux, lattice box, reset camera, workspaces, theme.
**Ctrl+K** opens the command palette (panels + actions; toggles/fields/scenarios join in later phases).

Viewport:

- Left-drag: orbit (when the pointer is in the central node and ImGui does not want the mouse)
- Wheel: zoom
- Space: pause
- S: step
- R: reset
- Esc: quit
- Ctrl+K: command palette
