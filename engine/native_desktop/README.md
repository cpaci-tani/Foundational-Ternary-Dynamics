# Native Windows desktop — in-process D3D12 presenter

This is a **separate** Windows application from:

- `engine/web` (browser / WASM dashboard)
- `engine/desktop` (WPF + WebView2 + WSL2 `ws_server`)

It does not modify those trees. Physics runs in-process through
`RenderBridge`. Particles and flux are drawn with D3D12 shaders in the
same process. CUDA–D3D12 buffer interop is the next slice; v1 still uses the
existing bounded visual-snapshot gather.

## Build

From the repo root, inside the MSVC 14.44 pin:

```bat
engine\build_native.bat build --target ftd_native_desktop --parallel 32
engine\start_native_desktop.bat
```

Binary: `engine/build/native_desktop/Release/ftd_native_desktop.exe`.

`--cpu` is the default (responsive until Windows CUDA graphs land). Pass `--gpu` to use the in-process CUDA backend.

```bat
engine\start_native_desktop.bat --cpu --lattice 32 --scenario s0-seed-hydrogen
```

## Controls

Left panel:

- Filter + scenario list (every C++ Scale-0 id from `scale0_scenario_ids()`)
- Load scenario (or double-click a row)
- Lattice size and flux boundary
- Play / Pause / Step / Reset
- Ticks-per-second slider
- Particle, flux, and lattice-box visibility
- Reset camera

View:

- Left-drag: orbit
- Wheel: zoom
- Space: pause
- S: step
- R: reset
- Esc: quit
