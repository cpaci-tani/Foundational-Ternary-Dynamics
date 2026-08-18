# FTD Desktop

FTD Desktop is the Windows-native shell for the engine dashboard. It embeds the
existing web interface in WebView2, serves the dashboard assets from an
in-process loopback server, and supervises the production-speed CUDA engine in
WSL2 Ubuntu 22.04.

This split is deliberate:

```text
WPF window + WebView2 (Windows)
        |
        +-- in-process static asset server on 127.0.0.1:8080
        |
        +-- WebSocket on 127.0.0.1:9100
                    |
                    +-- ws_server (WSL2 Ubuntu-22.04)
                               |
                               +-- RenderBridge -> GpuBackend -> CUDA/RTX 5090
```

The shell never treats “compiled with CUDA” as proof that CUDA is executing.
It requires the server to report `backend: "cuda"` and guarded interactive GPU
mode. After the dashboard loads, it also verifies that the page retained its
native GPU bridge (rather than falling back to WASM) and that WebView2 exposes
an identifiable, non-software WebGL renderer. The green status is shown only
after all of those checks pass.

## Build and launch

From the repository root:

```bat
engine\build_desktop.bat
engine\start_desktop.bat
```

The build publishes a framework-dependent `win-x64` application to
`engine\build_desktop\`. On each launch (unless `--skip-engine-build` is used),
the app performs an incremental 32-way build of the WSL2 `ws_server` target,
starts it with `FTD_FORCE_GPU=1`, waits for a successful WebSocket/CUDA probe,
loads the dashboard, and verifies its native bridge and WebGL renderer.

Prerequisites:

- Windows 11 with WSL2 and the `Ubuntu-22.04` distribution;
- an NVIDIA driver exposed to WSL (`nvidia-smi` must work there);
- the existing configured `engine/build_wsl` CMake tree;
- .NET 8 Windows Desktop runtime or SDK;
- Microsoft Edge WebView2 Runtime.

WebView2 is restored from the official `Microsoft.Web.WebView2` NuGet package.
The project-local `NuGet.Config` changes no package sources outside this
desktop project.

## Options

Arguments can be passed through `start_desktop.bat`:

```text
--repo <path>             FTD repository root
--distro <name>           WSL distribution (default: Ubuntu-22.04)
--lattice <4..256>        initial Scale-0 lattice size (default: 64)
--engine-port <port>      native WebSocket port (default: 9100)
--dashboard-port <port>   embedded asset-server port (default: 8080)
--skip-engine-build       use the existing WSL2 ws_server binary
--smoke-test              verify startup, write PASS/FAIL to the log, then exit
```

Examples:

```bat
engine\start_desktop.bat --lattice 96
engine\start_desktop.bat --engine-port 9200 --dashboard-port 8181
```

## Runtime behavior

- Closing the window stops only the WSL2 engine process launched by this app.
  If an already-running CUDA server occupies the configured port, the desktop
  shell reuses it only after matching the listener PID to this repository's
  WSL2 `engine/build_wsl/ws_server` and verifying `FTD_FORCE_GPU=1`; it does
  not take ownership of that external process. Windows-native, unguarded, CPU,
  and unrelated listeners are rejected explicitly.
- “Restart engine” first navigates WebView2 away so its single-client WebSocket
  is released, then restarts the owned server, verifies CUDA again, and reloads
  and re-verifies the dashboard. For an external server the control is labeled
  “Reconnect engine” and never kills that process.
- “Logs” shows build, WSL2, CUDA, and server output without opening console
  windows. Engine output is streamed to the session file while the visible log
  is refreshed in bounded batches, so a warning burst cannot flood the WPF
  dispatcher and freeze the shell.
- Sizes above `L=64` start from a safe `L=64` CUDA lattice. The server checks
  live host/GPU memory and only then commits the requested resize
  transactionally. A rejected or recoverable failed resize leaves the safe
  CUDA lattice running and shows the actual active size plus an explicit
  warning; it is never mislabeled as the requested size.
- The dashboard is loaded with the actual active lattice size. A watchdog
  re-checks the native bridge and WebGL context and exposes a recovery overlay
  after repeated failures.
- Dashboard assets are sent with `Cache-Control: no-store`, matching the
  development server’s live-edit behavior.
- Native Scale 0 has a completion-acknowledged command stream: only one CUDA
  tick is in flight, real-time playback demand is coalesced under load, and
  paused Step/+N commands retain their exact requested tick count. This keeps
  Pause, Reset, and Resize ahead of an unbounded simulation backlog.
- Dense flux volumes use compact sampled binary `FTV2` float32 frames instead
  of dense decimal JSON.
  The browser does not construct a second WASM Scale-0 lattice; its lazy WASM
  module is reserved for the standalone Scale 1/2 engines and uses a minimal
  control lattice.
- Resize and scenario changes first sample WSL2 host memory and CUDA free
  memory, then construct the replacement bridge transactionally. If the budget,
  allocation, or scenario setup fails, the previous lattice remains live and
  the failure is shown in the UI instead of terminating the server.
- CUDA failures propagate as recoverable command errors. Desktop, engine, and
  WebView2 process failures are written immediately to
  `%LOCALAPPDATA%\FTD\Desktop\logs\ftd-desktop-*.log`; the shell presents a
  retry/restart state while preserving that log.

## GPU boundary

“CUDA + WebGL active” means the production Scale-0 `RenderBridge` is executing
through `GpuBackend`, the dashboard is connected through its native GPU bridge,
and WebView2 has an identifiable non-software Three.js renderer.
It does not erase the engine’s documented per-feature backend boundaries. A
toggle that is unsupported in guarded interactive CUDA mode returns an explicit
error; the desktop-launched process does not silently force the whole engine to
CPU. Standalone Scale 1/2 capabilities currently delegated to WASM remain
browser-CPU work. Those boundaries are defined by
`TermToggles::backends`, the bridge capability contract, and `SPEC_ENGINE.md`;
the desktop shell does not relabel them as GPU implementations.
