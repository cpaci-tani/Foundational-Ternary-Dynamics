# REF — FTD Web Dashboard Engine Architecture (Native-Port Reference)

**Status:** `[REFERENCE — as-is map with native-rebuild guidance]` · **Created:** 2026-08-19 · **Scope:** the entire `engine/web` dashboard engine (all seven scales, the bridge/transport layer, rendering, telemetry, scenarios, build/serve/test) plus the C++/CUDA engine boundary it rides on.

**Purpose.** This document is the authoritative map of the browser dashboard as it stands, written so the team can replan the *entire* engine as a native Windows desktop application — in-process C++/CUDA physics, D3D12 rendering, Dear ImGui UI — extending the existing `engine/native_desktop` shell. It is a reference, not a phased plan: it says what exists and where, and where the current web architecture should be *improved* rather than transcribed when it is rebuilt natively. Every load-bearing claim is grounded in a concrete `file:line`; where the governing web-side spec disagrees with the code, the code is taken as truth and the drift is noted.

**Companion documents.** `engine/native_desktop/docs/SPEC_UI_V2.md` (the approved, substantially-implemented Scale-0 native shell — the target this reference extends); `engine/web/ARCHITECTURE.md` (the web team's own map); `engine/SPEC_ENGINE.md` (the C++ engine); `CONTRACTS.md` (cross-module interfaces).

**Reading conventions.** Three annotations recur, because performance and physics accuracy are the two load-bearing motivations for the port:

- **(P)** — where computation runs and its cost: pure JS in the browser, WASM on the main thread, WASM in a worker over `SharedArrayBuffer`, native CUDA, or WebSocket-to-WSL2.
- **(A)** — physics-accuracy status: real C++ engine versus JS approximation, determinism, and any place the browser path silently diverges from the native path.
- **(I)** — an architectural weakness in the web design that the native rebuild should improve rather than reproduce.

---

## 1. Overview and deployment surfaces

### 1.1 The one engine, three shells

FTD's physics lives in one C++ core (`engine/src`, `engine/include/ftd`, `engine/cuda`). Three different shells drive it, each making a different trade between UI reach, rendering, and access to CUDA:

| Surface | Directory | UI | Physics runs on | CUDA | Rendering | Scale reach |
|---|---|---|---|---|---|---|
| **Web dashboard** | `engine/web` | Browser, ES modules, no bundler | `ftd_core*.wasm` in the browser | ❌ none | WebGL / Three.js | all 7 scales (default status literally reads "Mock Engine", `index.html:254`) |
| **WPF desktop (prior attempt)** | `engine/desktop` | .NET 8 WPF + **WebView2 hosting the same web app** | `ws_server` in **WSL2 Ubuntu** → `RenderBridge` → `GpuBackend` → CUDA/RTX 5090, over a `127.0.0.1:9100` WebSocket | ✅ but **cross-process + cross-OS** | WebGL inside WebView2 | multi-scale (reuses the web UI) |
| **Native desktop (target)** | `engine/native_desktop` | Native Win32 + **D3D12 + Dear ImGui** | **in-process** `RenderBridge` → CPU/GPU backend, CUDA↔D3D12 interop | ✅ **in-process, zero-copy** | D3D12 | **Scale 0 only today** (SPEC_UI_V2; ~6,400 LOC, substantially built) |

The performance ceiling of each surface is set entirely by where the physics executes. The browser (`engine/web`) can never reach CUDA: the WASM build has no GPU backend (§3.3) and runs `CpuBackend` single-threaded, or at best on a small `std::thread` pool over `SharedArrayBuffer`. The WPF shell (`engine/desktop`) reaches CUDA but only by running the engine as a *separate process inside WSL2* and shipping every frame across a WebSocket — a boundary that exists purely to bridge Windows/WSL and browser/native, and which it must then defend with process-ownership checks, memory preflight, CUDA-vs-CPU attestation, and a software-WebGL detector (§10.6). The native desktop shell (`engine/native_desktop`) puts the CUDA engine and the D3D12 renderer in one address space and shares GPU buffers directly.

### 1.2 The thesis of the native replan

The single most consequential fact this reference establishes is that **the native rebuild is largely a wiring problem, not a physics-reimplementation problem** — because per-domain native engines already exist and the browser simply cannot reach them:

- The engine ships a `ScaleEngine` abstract base (`engine/include/ftd/scale_engine.h:56`) with virtual `tick()`, `run()`, `dt()`, `get_toggle`/`set_toggle` by name, `base_diagnostics()`, `entity_count()`, `scale_level()`, and `scale_name()` — designed precisely so "the web dashboard switches between Scale 0/1/2/5 at runtime via a single bridge pointer" (`scale_engine.h:5-11`) with vtable dispatch.
- `ParticleEngine` (Scale 1), `AtomEngine` (Scale 2/3), and `CosmicEngine` (Scale 5) all derive from `ScaleEngine` and are compiled and CTest-covered.
- `SimEngine` (`engine/include/ftd/engine_select.h:26`) already wraps "GPU if available, CPU otherwise" for the Scale-0 `RenderBridge`, mirroring the exact selection the native app needs.
- Native CUDA backends exist for three of these — `gpu_engine.h` (Scale 0), `gpu_particle_engine.h` (Scale 1), `gpu_atom_engine.h` (Scale 2/3) — none of which is compiled into WASM.

Yet the browser reaches only two of the real engines: Scale 0 (`RenderBridge` via WASM) and Scale 1 (`ParticleEngine` via embind, CPU-only). **Scales 2–5 run JS mock engines in the browser even though C++ engines exist**, and the CUDA backends are dark everywhere the browser runs. The native replan's central move is therefore: host every scale on its real in-process C++ engine through the `ScaleEngine` interface, enable the native CUDA backends, and delete the JS mock physics — which simultaneously removes the entire bridge/transport/worker machinery (§4) that exists only to move the browser's physics off the render thread.

### 1.3 What "the entire engine" comprises

The web app is ~78K LOC of application JavaScript (340 files under `engine/web/js`, excluding vendored Three.js and `node_modules`). It decomposes into: a composition root and UI shell (§9), a three-implementation bridge layer (§4), seven scale packages (§5), a Three.js rendering subsystem (§6), a telemetry hub (§7), a scenario+configuration data model (§8), and a build/serve/test toolchain (§10). Sections 2–12 map each in turn, always noting where the physics is real versus mocked, where the compute runs, and what the native rebuild should keep, wire, or discard.

---

## 2. Runtime topology

### 2.1 Composition and the master loop

The browser entry is `engine/web/index.html`, a no-bundler ES-module page. It declares an import map aliasing `three` → `js/vendor/three/build/three.module.js` (`index.html:94-101`), links ~50 modular CSS files, and loads a single module entry `js/app.js` (`index.html:263`). There is no webpack/rollup/Vite in the served path; the browser loads raw source (a root `package.json` has a `vite build` script used only for lint/typecheck, §10.5).

`js/app.js` (~1,710 LOC) is the composition root: it owns bridge initialization, global play state, mode/scale switching, top-level service construction, and the single `requestAnimationFrame` loop `animate(now)` (`app.js:673`, kicked at `app.js:662`). The loop dispatches by `engineMode` to the active scale's animator — `Scale5Controller.animateCosmic`, `animateAE` (Scale 2/3), `animatePE` (Scale 1), or Scale 0's pipeline — each of which advances its own physics and then calls `viewport.render()`. Two scales (4 planetary, 6 meta) opt out of the master loop and instead subscribe to a shared throttled scheduler `js/lib/raf-coordinator.js`.

The runtime shape, top to bottom:

```
index.html
  → js/app.js                     composition root, master rAF loop, mode switch
     → js/ui/shell/app-shell.js    toolbar, panel registry, docking, responsive shell
     → bridge selection            WebSocket (ws_server) → WASM main-thread → WASM worker
     → js/viewport.js              shared Three.js scene/camera/renderer facade
     → per-scale controller        scale lifecycle + animation (js/scales/scaleN/)
     → js/telemetry-hub.js         one singleton, ring buffers for every scale
```

### 2.2 The seven scales and what backs each

All seven scales are wired live in the engine-mode selector (`index.html:144-152`). What differs — and what matters for the port — is which run genuine engine physics:

| Scale | Mode | Controller | Physics backing | Real or mock | Compute (P) |
|---|---|---|---|---|---|
| **0 Lattice** | `lattice` | `scales/scale0/controller.js` | C++ `RenderBridge` via WASM (worker default) or native `ws_server` | **REAL** | WASM worker + SAB (default); native CUDA via ws_server |
| **1 Particle** | `particles` | `scales/scale1/controller.js` | C++ `ParticleEngine` via embind | **REAL** | WASM main-thread, single-threaded CPU |
| **2 Atom** | `atoms` | `scales/scale2/controller.js` | JS `mock-atom-engine.js` (C++ `AtomEngine` exists but disabled) | **MOCK** | pure JS, main-thread, O(N²) |
| **3 Molecule** | `molecules` | `scales/scale3/controller.js` | JS `mock-atom-engine.js` (shares Scale-2 runtime) | **MOCK** | pure JS, main-thread |
| **4 Planetary** | `planetary` | `scales/scale4/controller.js` | JS `mock-scale4.js` (no native engine) | **MOCK** | pure JS, main-thread, O(N²) Verlet |
| **5 Cosmic** | `cosmic` | `scales/scale5/controller.js` | JS `mock-scale5.js` (C++ `CosmicEngine` exists but unwired) | **MOCK** | pure JS, main-thread, O(N²)/Barnes-Hut |
| **Meta (6)** | `meta` | `scales/scale6/controller.js` | pure geometry, no physics tick | **N/A** | pure JS, static geometry |

Scale 0 is the only mature package (~16,850 LOC across `scales/scale0/`); Scales 1–6 are lighter controllers over their bridges. Scale 11 (reference-frame) is retired from the live tree. §5 details each.

### 2.3 The per-scale controller contract

Controllers are leaves imported by `app.js`; they never import `app.js` (`CONTRACTS.md §3`). Each exposes a duck-typed lifecycle — `mount`/`destroy`, a per-frame `animateX(ctx)`, `loadXScenario`, `resetX`, `bindXControlsUI`, and overlay setters. The context object `ctx` carries the active bridge, the viewport, and the telemetry hub. This uniformity is the web analogue of the native `ScaleEngine` vtable; the native rebuild collapses "controller + bridge + mock engine" into one `ScaleEngine` subclass per scale plus a thin ImGui panel set.

**(I)** The web contract has a recurring hazard the native design already fixes: `ctx.bridge` is not stable — Scales 4/5 construct their own mock bridges, and Scale 0 swaps in a worker-proxy bridge for its physics while leaving `ctx.bridge` pointing at the idle primary. Callers must re-read the active owner every frame (via `scales/scale0/state/store.js` selectors), and code that reads `ctx.bridge` directly silently gets stale/idle data. The native `NativeEngineSession` (SPEC_UI_V2 §3.4b) owns exactly one engine and removes this class of bug.

---

## 3. The physics engine boundary

This is the performance-and-accuracy core. Everything above it is presentation; everything in it is where the FLOPS and the determinism live.

### 3.1 `RenderBridge` and the 10-phase tick cycle

The Scale-0 substrate engine is `RenderBridge` (`engine/include/ftd/render_bridge.h`, ~726 lines; `engine/src/render_bridge.cpp`, ~1,285 lines). Its `tick()` (`render_bridge.cpp:810`) orchestrates a phase ladder whose bodies are decomposed into free functions across `render_bridge_phases/{phase_read,phase_write,phase_forces,phase_movement}.cpp`, `poisson_solvers.cpp` (all SOR solvers), and `transmutation_phases.cpp`. Every phase is toggle-gated (an O(1) skip when off):

| # | Phase | Toggle gate (`render_bridge.cpp`) | What it does |
|---|---|---|---|
| 1 | `phase_read` | `wave_propagation \|\| coupling \|\| de_broglie_clock` (:941) | Wave equation ΔJ = C²∇²J + g_c·(∇s+…); 18-point isotropic Laplacian; writes `delta_j_[]` (+ dual L/R). |
| 2 | `phase_write` | `!matched_gauss_dynamics` (:945) | Leapfrog commit `wave_vel += ΔJ; flux += wave_vel`; damping; genesis/evaporation (Born-rule manifestation); Langevin OU draw. |
| — | pair_production | `pair_production` (:983) | Correlated ±1 pairs from high-flux void (CPU port now present). |
| 3 | `gauss_project` | `gauss_projection` (:987) | Enforce ∇·J = s via SOR 18-point sweeps. |
| — | latency_solve | `latency_field` (:1004) | ∇²φ_L = 4πGρ then L = √clamp(φ_L,0,0.998), SOR. |
| 4 | `phase_forces` | `forces \|\| color_forces \|\| strong_force \|\| exchange_force \|\| cluster_inertia` (:1011) | Coulomb (SOR Poisson) + gravity + Lorentz + color/Yukawa/exchange. |
| 5 | `phase_movement` | `movement` (:1033) | Integer displacement + `remainder` sub-lattice accumulation, collisions, annihilation, id transfer. |
| — | boundary | `flux_boundary ∈ {Reflective,Dispersal}` (:1066) | Re-imposes edge law after the last flux writer (Periodic = no pass, handled by lattice wrap). |
| 6/7 | weak/triad | `weak_transmutation` (:1085), `triad_binding` (:1091) | Polarity flip under stress; 3-same-sign lock detection. |
| 8 | proper_time | `latency_field \|\| de_broglie_clock` (:1110) | dτ = √(1 − u²/C² − L²); de Broglie phase φ += ω₀·dτ. |

Tick tail: `physical_time_ += dt_; ++tick_;` (`render_bridge.cpp:1113-1114`). Several extra, default-off, golden-neutral steps interleave (EW background sweep, dB-clock Coulomb pre-solve, Verlet second half-kick, strong-stress-energy begin/complete, SU(2)/SU(3) Wilson gauge-link relaxation, and `update_energy_ledger()`).

**(I)** The "10 phases" is a documentation abstraction, not a data structure — the gating is ~15 inline `if(toggle)` sites inside one 300-line `tick()`. A native rebuild benefits from a declared phase table (function pointer + gate + name + tick-phase group), which is exactly what SPEC_UI_V2's toggle-grouping decision (D-Q2) and the GPU `record_tick_body` already half-express.

### 3.2 The voxel data model, lattice, and neighbor topology

The `Voxel` struct (`engine/include/ftd/voxel.h:68-208`, ~232 bytes) carries `state` (int8 ternary −1/0/+1), the leapfrog pair `flux`/`wave_vel` (each `Vec3` = 3×double), the dual-substrate quadruple `flux_L/flux_R/wave_vel_L/wave_vel_R` (invariant `flux = flux_L + flux_R`), `velocity`, `remainder` (sub-lattice fraction), `latency` ∈ [0,0.999), `tau`, de-Broglie `phase`, `locked`, `particle_id` (−1 = void), `pair_id`, `spin` (±1), `color` (0–3), `flavor` (0–3), `accel_mag`, and the strong/weak substrate fields. `ForceDiag` (Coulomb/strong/magnetic/gravity/exchange) is stored in a *separate* `force_diag_` buffer, not in `Voxel`, to keep field sweeps cache-friendly (`voxel.h:57-66`).

The `Lattice` (`engine/include/ftd/lattice.h`) is a thin index calculator — no stored voxels, no stored neighbor tables. Flat order is **X-slowest**: `idx = x·N² + y·N + z` (`lattice.h:36`), periodic via `wrap()`. Neighbors are computed on the fly (`neighbors_6` faces, `neighbors_12` edges, `neighbors_8_corner` BCC diagonals, `neighbors_26` full Moore); the 18-point Laplacian = 6 faces + 12 edges.

**(A)** ⚠ A layout transpose crosses the WASM boundary: C++ is X-slowest, but the JS flux renderer expects **Z-slowest** `z·N²+y·N+x`. `get_flux_volume` transposes explicitly (`ftd_wasm.cpp:475-483`); a native app that shares buffers with a web-style renderer inherits this convention mismatch. In a clean native rebuild the renderer reads the engine's native layout directly and the transpose disappears.

### 3.3 CPU backend versus GPU backend

The `Backend` abstraction (`engine/include/ftd/backend.h`) has `CpuBackend` (always built) and `GpuBackend` (only under `FTD_ENABLE_CUDA`). `make_default_backend()` (`backend.cpp:574`) selects GPU when CUDA is compiled unless `FTD_FORCE_CPU` is set. `RenderBridge::tick()` dispatches at `render_bridge.cpp:871`: on the GPU backend, `backend_->tick()` owns the whole flush→engine→sync sequence and returns early; otherwise the CPU phase ladder runs.

The two backends differ in ways the native UI must respect:

- **Toggle-read timing (SPEC_UI_V2 C3/W22).** GPU snapshots toggles once at tick top (`engine_->toggles = bridge_.toggles`, `backend.cpp:246-249`); a mid-tick host write is not seen until next tick. **CPU has no snapshot** — `RenderBridge::toggles` is a public member every phase reads live, so a mid-tick write is a genuine data race that applies mid-tick. This is *why* the native command spine drains commands only at tick boundaries.
- **Poisson solver.** CPU uses iterative SOR; GPU uses cuFFT. Backend kind is therefore part of run identity — the two are **not** bit-identical, and cuFFT is not bit-stable across runs. Only `voxel_uniform()` (genesis/evaporation/pair RNG, SplitMix64) is bit-exact CPU↔GPU by design; `voxel_normal()` (Langevin, transcendentals) differs sub-ULP. A separate `test_gpu_golden.cpp` pins the GPU path.
- **Host↔device sync.** `flush_host_mutations()` (`backend.cpp:387`) delta-uploads only changed voxels (~333 B versus 87 MiB at L=64), then recomputes the `weak_field_active_` latch, invalidates the continuity ledger, and bumps `state_version_`. `host_mutated_` is set *merely by calling* non-const `voxels()`/`voxel_at()` (`mark_host_dirty()` fires on handout). Six "observer" methods (`gravity_metric_agg`, `diagnostics`, `energy_audit`, `copy_compact_lagrangian`, `inspect_voxel`, `inspect_force`) therefore perform a host→device *write* on GPU despite reading like const accessors.
- **Interactive GPU mode** (used by `ws_server` and `native_desktop`) keeps the AoS shadow device-resident and skips the per-tick download, reading compact snapshots instead — the key to interactive GPU throughput.

**(I)** `mutable`-state sync (`sync_ternary_from_voxels_if_needed`, `render_bridge.cpp:246`) is OpenMP-critical-guarded *only when* `omp_in_parallel()`; called from a plain `std::thread` (the GUI thread) it takes no lock and races `tick()` (SPEC_UI_V2 W20). Cross-thread CUDA works only because the Runtime API shares the device-0 primary context and the code never calls `cudaSetDevice()` (W16). These are real debts the native rebuild must fix with explicit thread ownership — which the `native_desktop` two-thread model (sim thread owns the bridge exclusively; GUI thread only reads a published snapshot) already does.

### 3.4 The WASM compilation path and its ceiling

`engine/build_wasm.bat` produces **three** variants from the one `ftd_wasm` Emscripten target (`engine/wasm/CMakeLists.txt`), each in a separate build tree (the ABIs cannot share `libftd_core.a`):

| Variant | Export | Heap | Lattice cap | Flags |
|---|---|---|---|---|
| `ftd_core` (wasm32) | `createFTDModule` | 2 GB | L ≈ 117 | `-DFTD_MEMORY64=OFF` |
| `ftd_core64` (Memory64) | `createFTDModule64` | 8 GB | L ≈ 187 | `-DFTD_MEMORY64=ON`, `-sMEMORY64=1 -sWASM_BIGINT=1` |
| `ftd_core_mt` (wasm32 + pthreads) | `createFTDModuleMT` | SAB heap, 1 GB | — | `-DFTD_WASM_THREADS=ON`, `-pthread -sPTHREAD_POOL_SIZE=0` |

`ftd_core_mt` is the **default deployed Scale-0 engine** (loaded by the worker when the page is cross-origin isolated). Common flags: `-O3 -flto -msimd128 -mbulk-memory -ffast-math`. Deployed artifacts land in `engine/web/wasm/` with a `build_info.txt` git-SHA stamp.

What the WASM path *cannot* do — the entire reason a native rebuild exists:

- **No CUDA.** WASM always runs `CpuBackend`; there is no GPU backend in the browser at all. This is the ceiling the RTX 5090 (~30×) sits above.
- **No OpenMP.** Emscripten has no OpenMP runtime; parallelism comes from `ftd::parallel_for`'s `std::thread` pool, and only on the `_mt` build. `ftd_core`/`ftd_core64` are single-threaded.
- **Threads require `SharedArrayBuffer`** = cross-origin isolation (COOP/COEP headers, §10.2). Absent isolation, only serial in-thread WASM runs.
- **Exceptions are downgraded to `abort()`** on `ftd_core` (`-fno-exceptions`); engine throws become `std::abort()`.

Embind exports span `wasm/ftd_wasm.cpp` plus three binding TUs (`bindings_render_bridge.cpp`, `bindings_particle.cpp`, `bindings_atom.cpp`). Data extraction uses zero-copy `typed_memory_view` into the WASM heap (valid only until the next call). **(A)** Three "physics" samplers in `ftd_wasm.cpp` (`getEMForceField`, `getStrongForceField`, `getGravityFieldSampled`, ~:821-1050) are *visualization reimplementations mirroring the JS mock formulas*, not the engine's real force phase — e.g. the strong-force sampler's α_s(r) disagrees with the engine's `alpha_s_lattice` by ~2.5–3.4× (`ftd_wasm.cpp:984-995`). A native rebuild reading the real `force_diag_` buffer drops these divergent duplicates.

### 3.5 RNG determinism and the golden gate

Three independent RNG streams (SPEC_UI_V2 §2.1) govern reproducibility:

1. **Tick-time noise** — `voxel_rng.h`, stateless index-keyed `voxel_uniform/voxel_normal(seed, voxel_idx, tick, salt)`, SplitMix64, seeded from `toggles.langevin_seed`. Thread-count-independent by construction. Salt domains are enumerated (`VoxelRng` enum, `:38`); reordering them changes the whole engine output.
2. **`BridgeRng`** — a stateful mt19937 owned by `RenderBridge` (seed 42, `render_bridge.cpp:117`). `rng_state_hash()` is thread-count-dependent, though no sampling method has a live tick-path consumer today; a future consumer silently makes trajectories thread-count-dependent.
3. **Scenario ICs** — a separate `thread_local` mt19937 seeded `SCN_RNG_SEED = 0xC0DEFACE` (`scenarios.cpp:44`), reset unconditionally at the top of every `dispatch_scenario`. Independent of `langevin_seed`.

The golden-tick gate's ground truth is `test_render_bridge_golden.cpp` (`GOLDEN_HASH = 0xc54ffbeda5a3ea63` at L=17); `test_render_bridge_golden_default.cpp` pins shipping `TermToggles{}` with zero toggle writes; `test_gpu_golden.cpp` pins the GPU path. ⚠ "Golden green ≠ physics verified": the frozen profile runs ~14 subsystems off, and `phase`/`flavor` are outside the fold. The native shell inherits this gate unchanged — it is the invariant the UI must never perturb (SPEC_UI_V2 §2).

---

## 4. The bridge and capability contract

### 4.1 Four transports behind one symmetric surface

Every consumer in the web app talks to physics through a `ScaleBridge` surface (JSDoc typedef in `js/bridge/bridge-contract.js`; no nominal interface — parity is by convention, guarded by `tests/scenario-parity.spec.js`). Four transports implement it, all presenting the same method names:

| # | File | Transport | Role | Flags |
|---|---|---|---|---|
| 1 | `js/ws-bridge.js` (~2.7 KLOC) | **WebSocket → native `ws_server`** on `ws://127.0.0.1:9100`, binary framing (FTV2 flux, FTP2 particles, FTS2 fields) | native GPU/CUDA path | `isNativeGPU:true` |
| 2 | `js/bridge/wasm-bridge.js` (48 KB) | **In-thread WASM** (embind, prefers Memory64) | fallback Scale-0 owner; hosts Scale-1 `ParticleEngine`; AE stub | `isWasm:true` |
| 3 | `js/bridge/wasm-bridge.worker.js` (19 KB) | **WASM in a Web Worker** (`ftd_core_mt`, classic worker via `importScripts`) | off-main-thread engine host | — |
| 4 | `js/bridge/wasm-bridge-proxy.js` (37 KB) | **Main-thread proxy** over the worker; `SharedArrayBuffer` zero-copy flux + `postMessage` for diagnostics/commands | **default Scale-0 owner when cross-origin isolated** | `isWasm,isWorker:true` |

Selection happens in `js/app-wire/bridge-boot.js`: `?engine=native` or a non-live-server port tries the native WebSocket first; otherwise `createBridge()` returns the in-thread `WasmBridge` (which throws on failure — no silent mock fallback). Independently, the Scale-0 layer *upgrades* to the worker proxy when `wasmWorkerEligible()` holds (`FTD_WASM_WORKER` default on, `SharedArrayBuffer` present, `crossOriginIsolated`, and a primary WASM bridge; `scenario-loader.js:228`), constructing a `WasmBridgeProxy` into the historically-named `state.fluxMock`/`useFluxMock` slots. The effective default under `serve.py`: in-thread `WasmBridge` globally, upgraded to the off-thread `WasmBridgeProxy` for Scale 0.

**(I)** This four-transport fan-out, its COI-gated worker upgrade, its binary wire protocol, and its no-silent-fallback rules exist **entirely to move the O(N³) tick off the browser render thread and to reach CUDA over a socket**. The native rebuild collapses all four to one in-process `ScaleEngine*` call. The `SharedArrayBuffer` heap views, the `postMessage` per-frame heap copies, the CTRL `Int32Array` header (`{FRAME,N,TICK,RUNNING,PCOUNT,TICKS_PER_FRAME}`), the sampler want/unwant registry, the ready/frame/watchdog/fallback protocol, and the tick-loop duplicated between `tick.js` and the worker all vanish (§5.1, §9).

### 4.2 The capability namespace

Consumers do not call raw bridge methods; they call `bridge.capabilities.scaleN.*`, installed once per prototype by `installCapabilityGetter` (`js/bridge/capabilities/install.js`, wired in `bridge-init.js:16`). Each factory closes over the bridge instance:

- `createScale0Capabilities` — `tickScale0`, `setupScenario`, `getScale0FieldSamples`, `getScale0Diagnostics`, `getScale0EnergyAudit`, `getScale0Lagrangian`, `setToggle`, `setBoundaryShape`. On the proxy, `tickScale0` is a no-op because the worker self-ticks.
- `createScale1Capabilities` — forwards to `bridge.pe*` (`peTick`, `peGetParticleData`, `peGetDiagnostics`, `peGetForces`).
- `createScale2Capabilities` — forwards to `bridge.ae*` (`aeTick`, `aeGetAtomData`, `aeGetDiagnostics`, `aeGetForceDecomposition`).

The bridge contract also exports `SCALE0_DIRECT_READS` (~26 reads consumers call *directly*, which the worker proxy must serve from the SAB view or the last frame payload or the consumer silently blanks) and `SCALE0_SAMPLER_METHODS` (kind→method map with a drift-warning fallback). **(A)** ⚠ On the proxy, `getToggle(name)` returns the optimistic local write-cache for a known-but-unconfirmed toggle; `getEngineTruthToggle(name)` returns `null` until the worker publishes a real engine readback (`wasm-bridge-proxy.js:320-335`). Code that must distinguish "engine says false" from "no readback yet" (promotion, `knot_tracking` restore, time/scale-context panels) must use the truth accessor. In-process C++ has no proxy/worker split, so this ambiguity disappears natively — the native UI reads real toggle state synchronously.

### 4.3 The native `ScaleEngine` interface — the replacement for the whole bridge layer

The native codebase already ships the abstraction that replaces this: `ScaleEngine` (`engine/include/ftd/scale_engine.h:56`) with `tick`/`run`/`dt`/`get_toggle`/`set_toggle`/`entity_count`/`base_diagnostics`/`scale_level`/`scale_name`, plus `SimEngine` (`engine/include/ftd/engine_select.h:26`) wrapping GPU-or-CPU for Scale 0. The native shell holds one `ScaleEngine*` (or `NativeEngineSession` per SPEC_UI_V2), switches it on scale change, and every panel reads a published snapshot and pushes typed commands. The capability namespace maps to the engine's own diagnostics accessors; the "which bridge owns this scale" question the web must re-answer every frame becomes a single owned pointer.

---

## 5. Per-scale deep dives

Each scale is presented uniformly: what it is, what physics backs it, where compute runs **(P)**, its accuracy status **(A)**, its controller/renderer, and the native-rebuild disposition. The recurring pattern — **a JS mock in the browser shadowing a real C++ engine that the browser cannot reach** — is the through-line.

### 5.0 Scale 0 — Lattice / Substrate (REAL engine)

The mature package (~16,850 LOC across `js/scales/scale0/`) and the only scale whose telemetry is genuinely engine-computed. Physics is the C++ `RenderBridge` (§3), reached through the worker proxy by default.

**Package structure.** `controller.js` (448) orchestrates; `runtime/` holds the pipeline (`tick.js`, `frame-sync.js`, `field-overlays.js` at 1,160 LOC, `overlay-frames.js`, `field-sample-cache.js`, `streamline-integrator.js`, `diagnostics.js`, `scenario-loader.js` at 949); `state/store.js` (262) is the single state object + dirty flags + `FIELD_TOGGLE_KEYS`; `viewport-adapter.js` (260) maps `apply*(data)` → `viewport.update*`; `scenario-registry.js` (950) holds 131 scenario descriptors; `scenario-validation.js` (822) holds admission metadata; `ui/` (~4,400) holds bindings, control cards, ~25 floating overlay panels, and toolbar groups; `analysis/` (~860) holds DOM-free math (FFT E(k), topology stats, causal-clock γ).

**The five-stage per-frame pipeline** (`controller.js:380`): `advanceSimulation` (tick physics, or read the worker frame counter) → `syncRenderableData` (upload lattice → GPU) → `updateFieldOverlays` (amortized overlay sweep) → `renderFrame` (advance the animation clock only when running, then `viewport.render()`) → `updateDiagnosticsAndPanels` (telemetry, every 3rd frame).

**(P) The amortized overlay scheduler** (`runtime/field-overlays.js`) is the most sophisticated web-side stage: it splits every active overlay into an allocation-free job with a cost weight (`COST_STREAMLINE=50`, `COST_FORCE_FIELD=25`, `COST_DERIVED=20`, `COST_SCALAR=12`, `COST_PASSTHROUGH=4`) and drains under a per-frame budget of 100, sized so one streamline lands per frame. Field data is sampled once at sweep start (a coherence snapshot) and a new sweep starts only when `fieldDataVersion` moved. A native GPU renderer that samples the engine's fields directly removes the need for most of this CPU amortization.

**(A) Overlay honesty.** ~14 of the ~30 overlays are JS-side proxies or approximations, several explicitly non-physical, flagged in the code itself (`overlay-frames.js`, `field-overlays.js`): dual substrate = `flux·(1±DUAL_DELTA)/2` ("NOT a true chirality projection", `:415`); chirality = `|flux|·DUAL_DELTA` scalar; weak force = ∇×J direction × DUAL_DELTA; entropy = Gini `4p(1−p)` on |J| (not the Moore-neighborhood Shannon the spec describes); ψ² = |J|² (cross-term dropped); gravPotential = −|J|²; lagrangian = two-term stand-in; dark-matter halo = a band-select on ordinary flux. Genuine pass-throughs: chargeDensity (∇·J), vorticity, emEnergy/E-B-pressure (from engine-sampled E,B), state, gaussResidual. The native rebuild should decide per-overlay whether to port the honest engine sampler or keep the pedagogical proxy behind a clear legend — the web already carries the honesty comments to migrate.

**Native coverage.** This is the one scale `native_desktop` already runs (§11): real in-process `RenderBridge`, D3D12 lattice/particle/flux draw, CUDA↔D3D12 interop. But only 1 of 18 field kinds is sampled (`append_flux` hardcodes `FluxVector`), and the 44-toggle table (§8.2), the 18 field overlays, the charts, and picking are stubbed pending SPEC_UI_V2 Phases 4–6.

### 5.1 Scale 1 — Particle Engine (REAL engine, CPU-only in browser)

⚠ Scale 1 was rewritten on 2026-07-29 (`de179b31`): the entire pure-JS particle stack was deleted after a 99-agent audit found 76 defects, and replaced with the native C++/WASM `ParticleEngine`. **Scale 1 is genuine engine physics, not a mock.** Files deleted include `mock-particle-engine.js`, `pe-force-kernel.js`, `cross-sections.js`, `decay-rates.js`, `pe-telemetry.js`.

**The engine.** `ParticleEngine` (`engine/include/ftd/particle_engine.h`, `src/particle_engine.cpp`) extends `ScaleEngine`. `struct Particle` (`particle_engine.h:171`) carries id, `int8 charge` (±127), `double mass`/`r_eff`, position/velocity/acceleration, `int8 spin`, `int8 color`, `pair_id`, `locked`, `spin_axis`, relativistic `momentum`. The force law (`particle_engine.cpp:158`, all double precision, softened) sums nine channels: Coulomb (`ALPHA_EFT·qq/4π r²`), gravity (`G_PE·mm/r²`, ~1e-46, telemetry-only), Pauli exchange, three-regime running strong force, magnetic dipole-dipole, spin-orbit, Lorentz, radiation reaction, and a crude non-covariant relativistic rescale. Integration is Velocity-Verlet KDK with speed-clamp to `C_SPEED`, damping, annihilation, and spin precession (`:582`). Toggles are an 11-row `PARTICLE_TOGGLE_SPECS` table.

**(P) Where it runs — the ceiling.** The Scale-1 `pe*` surface **always resolves to a main-thread, single-threaded WASM `ParticleEngine`**, blocking the render loop: the direct WASM bridge hosts `_peEngine` (`wasm-bridge.js:111`); even the native WebSocket bridge lazily spins up a *separate in-page WASM bridge at L=9* purely for Scale-1 delegation (`ws-bridge.js:2301`); the worker proxy exposes no `pe*` at all. `peTick()` is a blocking call inside the rAF loop (`controller.js:305`). N is small (tens of particles), which is why this is tolerated. **The native CUDA `ParticleEngine` backend (`gpu_particle_engine.h`) exists but is `#ifdef FTD_ENABLE_CUDA` — unreachable in the browser.** The native rebuild runs the ParticleEngine on CUDA in-process and removes the main-thread block.

**(I) Debt to fix natively.** (1) One force-law implementation — the WASM binding keeps a *second* copy (`compute_pe_force_diag_snapshot`, `bindings_particle.cpp:25`) recomputed O(N²) per read; expose the engine's own `force_diag_` buffer instead. (2) `total_energy` sums only Coulomb+gravity PE, so drift is honest only for those toggles. (3) `int8 charge`/unsigned `int8 color` cannot carry fractional quark charge or quark/antiquark color sign. (4) The per-frame CPU cloud expansion (`pe-cloud-expander.js`, up to 100K Gaussian points with trig per point) should move to a GPU compute/geometry pass.

**Promotion pipeline.** `promotion.js` coarse-grains the live Scale-0 lattice into Scale-1 particles (one continuous particle per manifested cluster, mass = N·K_B, charge = sign·N clamped to ±127), reading cluster telemetry from the *active Scale-0 owner* via `scale0/state/store.js` selectors (not `ctx.bridge`, which is idle when the worker owns physics). Seeding uses a native-force probe (`peApplyEquilibriumOrbit`, `native-particle-engine.js:223`): zero velocity, read the native force, solve `mv²/r = |F|`, write the tangential velocity back — ICs from the live kernel, not a closed form.

**Catalog and Zoo.** `particle-catalog.js` (696) holds 62 SM entries with PDG masses and epistemic-tagged `ftd_formula`/`ftd_status`; `zoo.js` injects them (collapsing fractional quark charge to integer sign); `spectroscopy.js` is standalone `[PARAMETRIC]` hydrogen reference, no longer wired to the engine. These port as data tables with their tags visible, not as engine physics.

### 5.2 Scale 2 — Atoms (JS MOCK; native `AtomEngine` exists, disabled)

**(A) Nothing in Scale 2 runs FTD substrate physics.** It is a classical molecular-dynamics mock, `js/bridge/mock-atom-engine.js` (1,241 LOC, `createAtomEngine`), plus analytic decoration. The mock is a genuine per-frame Velocity-Verlet MD of a **non-FTD, textbook force field** with visualization-tuned `[IMPOSED]` constants: pairwise Coulomb, Lennard-Jones 12-6 (Lorentz-Berthelot mixing), 10-12 H-bonds with `cos²θ` angular factor, dipole-dipole, harmonic bond springs, three-body VSEPR angle strain, Berendsen thermostat, and distance-threshold auto-bonding (`AE_K_COULOMB=2.0`, `AE_K_BOND=50.0`, etc., all in `constants.js:446` and explicitly not FTD-derived). The decoration layer (periodic table `elements.js`, Slater `slaterZeff` and hydrogenic orbital-cloud rejection sampling `orbitals/quantum-chemistry.js`, SEMF binding energy `atomic-energy.js`, Thomas-Fermi electron binding) is analytic/table-driven, computed once and cached, self-labeled "VISUALIZATION ONLY (not a physics derivation — FTD-0270)".

**(P)** Pure JS, O(N²), single-threaded, on the main thread inside the rAF frame; the shared Scale-2/3 animator `animateAE` runs `aeTick()` `wholeTicks` times per frame via a fractional accumulator.

**⚠ The disabled native engine.** A compiled C++ `AtomEngine` (`engine/include/ftd/atom_engine.h`, `src/atom_engine.cpp`, `src/atom/*.cpp`) exists — a feature-superset of the JS mock (same force laws, same Velocity-Verlet, same 12-toggle `ATOM_TOGGLE_SPECS`, same force decomposition) **plus a Barnes-Hut `O(N log N)` tree and a CUDA GPU backend `gpu_atom_engine.h`**. It is deliberately dark in the browser: `WasmBridge._aeHasWasm` hard-returns `false` (`wasm-bridge.js:885-896`) because the C++ engine works in Planck/ontic-derived units (`mass = Z·M_PROTON + N·M_PROTON·(1+ALPHA)`, `vdw_epsilon = K_B·ALPHA²·Z^(2/3)/4π`, `k_bond = ALPHA·K_B/r_eq²`) while the web molecule data is Bohr-scaled sim units, and no unit-conversion shim exists. So even on the WASM page Scale 2/3 falls back to the JS mock. **This is the single largest reuse lever for the native replan:** wire the existing native `AtomEngine` (with its GPU backend) after building the Planck↔Bohr shim, and delete the JS mock.

### 5.3 Scale 3 — Molecules (JS MOCK; thin shell over Scale 2)

Scale 3 is a thin shell over Scale 2 — `scale3/controller.js` (249) re-exports `animateAE` from Scale 2 and shares the entire AtomEngine runtime and render loop. The only Scale-3-specific logic is loading pre-built molecules from `molecules.js` (25 molecules in Bohr-scaled sim units) instead of individual atoms, plus a one-tick stability dry-run. **(I)** Molecule geometries encode a hidden coupling: atom positions are hand-tuned so every advertised multiple bond falls inside the auto-bonder's `1.2·σ_avg` threshold, so changing σ silently breaks bonding library-wide — the native rebuild should store explicit bond lists in the molecule data rather than inferring connectivity from tuned distances. Scales 2 and 3 should collapse into one atoms/molecules subsystem with a scene loader (they already share everything).

### 5.4 Scale 4 — Planetary Sandbox (JS MOCK; no native engine)

Live and user-selectable (`index.html:149`). **(A)** A pure-JS N-body mock, `js/bridge/mock-scale4.js` (`PlanetaryMockBridge`): an exact O(N²) softened-Newton force kernel (`eps²=1e-6`) with Velocity-Verlet KDK and a fixed 100-substep multiplier per visual tick (`dt=0.0001`). Gravity modes: `decorative` (`G_N=0.01`, deliberately not AU/yr-faithful) and `physical` (`G=4π²`, Keplerian). Body types STAR/ROCKY/GAS/MOON/ASTEROID. Exoplanet ICs from static NASA-archive-style seeds (`js/config/exoplanet-seeds.js`: TRAPPIST-1, Kepler-90/11/20, HR 8799). **(I)** No merger/collision handling and no energy audit — close passes slingshot or blow up. **There is no native planetary engine**; the native rebuild would write new C++ N-body (trivially, by reusing the cosmic Barnes-Hut/Verlet) plus the procedural terrain shaders (`planetary-renderer.js`, 4-octave fbm displacement, biome-by-temperature). Compute is pure JS main-thread; the controller self-drives via `rafCoordinator` at 60 Hz.

### 5.5 Scale 5 — Cosmic Engine (JS MOCK; native `CosmicEngine` exists, unwired)

Live (`index.html:150`), ~30 Hz physics. **(A)** A JS mock (`js/bridge/mock-scale5.js` `CosmicMockBridge` + `cosmic-physics.js` + `cosmic-postupdates.js` + `cosmic-scenarios/`), but a rich one: Velocity-Verlet KDK with an exact O(N²) direct sum below 3,000 bodies and an approximate Barnes-Hut octree (θ=0.5) above (per-body accel error 1–2%, no momentum conservation on the BH branch). Body types are ternary-signed (DARK_ENERGY −3 … WHITE_DWARF 5). It has genuine sub-grid physics (SPH-like pressure, radiative cooling, tidal disruption), event-driven black-hole formation/mergers/Hawking evaporation, and stellar-lifecycle fuel-burn. A real RK4 Friedmann solver integrates `a(t)`/`H(t)` — **but diagnostics-only: it is never fed into the force kernel, so there is no Hubble drag on bodies in the mock**. The multi-layer black-hole renderer (`cosmic-renderer.js`, 685 LOC) is the app's most complex: per-BH bundles of horizon sphere, corona, two Doppler-beamed accretion-disk `RingGeometry` shaders, Einstein-ring points, and turbulent-plasma jet `CylinderGeometry` shaders.

**⚠ The unwired native engine.** A compiled, CTest-covered C++ `CosmicEngine` (`engine/include/ftd/cosmic_engine.h`, `src/cosmic_engine.cpp`, `src/cosmic/*.cpp`) exists and is *richer* than the JS mock: an 18-phase cosmic tick cycle, **true SPH** (smoothing length, density, pressure, internal energy fields), a **genuine `f_hubble = -H·v` per-body force**, gravitational-wave strain, and 14 ADR-0013 table-managed toggles. It is **not wired to WASM** — the web uses the JS mock exclusively. **(I)** The native rebuild should treat `CosmicEngine` as the canonical Scale-5 physics and delete the JS mock — largely a "wire the existing native engine to the UI" job, not a reimplementation. Compute today is pure JS main-thread.

### 5.6 Meta (Scale 6) — Existential Unit (pure geometry, no physics)

Live (`index.html:151`). **(A)** Not a simulation — a static geometric/pedagogical visualization with no bridge, no tick, no integrator (`js/scales/scale6/controller.js`, `meta-unit.js` 679, `meta-unit-geometry.js`, `meta-pedagogy.js`). It builds the 27-site Moore neighborhood classified by d² (center / octahedron 6 / cuboctahedron 12 / cube 8), maps shells to sublattices (SC/FCC/BCC), and renders per-site symmetry data (stabilizer subgroup, irrep, inversion parity), wireframe polyhedra (octahedron, cuboctahedron, cube, stella octangula), symmetry axes/planes, and framework-integer labels (N_c=3, N_base=4, b₃=7, N_eff=13). Rendering uses one `InstancedMesh` per shell (4 draw calls for 27 spheres). The only per-frame work is optional auto-rotation; it self-drives via `rafCoordinator` at 30 Hz — the cheapest scale. **There is no native engine** and none is needed: a straight D3D12+ImGui geometry/label port.

---

## 6. Rendering architecture (Three.js → D3D12)

The renderer is the single largest greenfield item in the port: no native equivalent exists, and every visual primitive must be re-authored in D3D12/HLSL. This section is the inventory the native team needs.

### 6.1 The facade and the shared scene

`js/viewport.js` (~1,292 LOC, `class Viewport`) is a thin orchestrator. It constructs and owns the *only* top-level Three.js objects — one `THREE.Scene` (background `0x0f1729`), one `PerspectiveCamera(45°, aspect, 0.001, 2000)`, one `WebGLRenderer({antialias:true})` with ACES tone mapping, `OrbitControls` (damping 0.12, minDistance 0.01, maxDistance 1e8), one `AmbientLight`, and a `_voidBox` raycast volume for the inspector — and delegates everything else to six composed sub-renderers. Its ~230 public methods are almost all one-line delegators, plus ~140 backward-compatibility getters/setters that proxy extracted mesh fields. **(I)** The delegator/forwarder layer is pure refactor scaffolding; the sub-renderer boundaries are the real design and are what the native rebuild should mirror.

Crucially, **the Viewport has no rAF loop** — `render()` (`viewport.js:1046`) is a pure paint step (dynamic far-plane, `controls.update()`, quantum breathing, spin-arrow slerp, then `sceneCore.render()`). The loop lives in `app.js:673`. All four rendered scales (0 lattice, 1 particles, 2/3 atoms/molecules) draw into this one shared scene; Scales 4/5 attach their own `BaseRenderer` subclasses to the same `WebGLRenderer` surface.

**(I)** `camera.far` is monkey-patched with a getter/setter (`viewport.js:118-126`) so `render()` can grow the far-plane to `max(baseFar, dist·5)` each frame and avoid far-culling at extreme zoom-out. A native port should use a proper log-depth or dynamic-far projection instead.

### 6.2 The sub-renderer inventory

| Module | Owns | Primary Three.js primitives |
|---|---|---|
| `scene-core.js` (530) | boundary wireframe, axes, inspector highlights, **post-processing pipeline** (`EffectComposer` + `RenderPass` + `UnrealBloomPass`, currently dormant — no scale enables it) | `LineSegments`, `InstancedMesh` (26 Moore symmetry boxes) |
| `particle-renderer.js` (989) | main particle cloud + velocity/spin/trail/force-arrow/system overlays; Scale-1 promotion billboards | `THREE.Points` + custom `ShaderMaterial`, `LineSegments`, `LineDashedMaterial`, sprites |
| `flux-renderer.js` (573) | **the signature substrate flux volume** + streamlines | `THREE.Points` (capped at `samples³`, `FLUX_MAX_AXIS_POINTS=53` → ~149K worst case) |
| `field-renderer.js` (262) + 4 mixins | ~137 field-overlay methods (EM, force, topology, quantum) | `Points`, `LineSegments`, force-glyph `InstancedMesh` (cones), `Mesh` (event horizon) |
| `topology-sheet-renderer.js` (483) | 5 deformable rubber-sheets + Φ potential | `PlaneGeometry` `Mesh` deformed by a bilinear-splat → box-blur → lookup height pipeline |
| `molecular-renderer.js` (802) | Scale-2/3 atoms/bonds | `InstancedMesh` (nuclei, bond cylinders, orbital shells/lobes), the one `MeshLambertMaterial`+`DirectionalLight` (lit) |
| `spin-arrow-manager.js` (234) | per-particle always-on-top spin arrows | `CylinderGeometry`+`ConeGeometry` `Mesh`, quaternion slerp |

Two standalone renderers extend `core/BaseRenderer.js`: `cosmic-renderer.js` (685, the multi-layer black holes) and `planetary-renderer.js` (433, procedural fbm planets). Each adds a child `THREE.Group` to the shared scene.

### 6.3 Custom GLSL — the HLSL port list

Every custom shader must be re-authored as HLSL. Centralized in `js/viewport/shaders.js`:

- **`PARTICLE_VERT`** — point-sprite, linear `gl_PointSize = size·150/-z`.
- **`FLUX_VOL_VERT`** — same with `sqrt(60/depth)` falloff (balances near/far at small lattices).
- **`PARTICLE_FRAG`** — the single most important shader: 8 procedural point-sprite shapes (circle/square/diamond/star/triangle/hexagon/ring/cross via `gl_PointCoord` SDFs), smoothstep alpha, `uGlow` gaussian halo, and a `sin(uManifestTime·rate+phase)` manifestation blink. Every field-overlay `Points` mesh reuses these two shaders.

**(I)** ⚠ **D3D12 has no `gl_PointSize` point-sprite path.** Reproducing this is the biggest single renderer port item: build billboard quads via instancing or a geometry/compute pass. One HLSL billboard system covers the entire point-cloud surface (particles, flux volume, most field overlays). Additional shaders: `cosmic/shaders.js` (Doppler-beamed accretion `DISK_VERT/FRAG`, plasma `JET_VERT/FRAG`, `blackbodyColor`); `planetary-renderer.js` (3 inline programs + `GLSL_SIMPLEX_NOISE_3D` from `constants.js`); `cosmic-renderer.js` (nebula shader); and 5 background shader canvases (`backgrounds/*.js`).

### 6.4 The data path and the native win

Renderers are **pure GPU-buffer fillers**: a scale controller/bridge produces plain typed-array payloads (`{positions, colors, sizes, count}` or pooled `StreamlineResult` from the `fieldlines.js` RK4 integrator, or a compact `{data, stride, axisCount}` FTV2 flux descriptor), and each `updateX` clips + writes into a pre-allocated attribute buffer marked `DynamicDrawUsage` and sets a draw range. Color math is CPU-side ramps in `color-ramps.js` (10 colormaps) and `fields.js`. **(I)** A native in-process C++/CUDA engine can write directly into D3D12 upload/default heaps (or via CUDA↔D3D12 interop, which `native_desktop` already does for the lattice/particle buffers) and skip the `SharedArrayBuffer` + typed-array marshaling and the per-frame CPU buffer rewrites entirely — moving most of the flux/particle fills and the coloring into HLSL compute/vertex shaders. This is the largest architectural win the renderer port affords.

### 6.5 The P1 translucency decision

**(A)(I)** Pervasive and load-bearing: every translucent primitive sets `transparent:true, depthWrite:false, frustumCulled:false` and relies on either `AdditiveBlending` (order-independent — flux glow, streamlines, halos, BH disks/jets) or `NormalBlending` with no depth sort (particle cloud, flux flat mode, orbital shells, topology sheets). Three.js gives a coarse per-object centroid sort that D3D12 will not. SPEC_UI_V2 P1 flags this and assigns the fix to Phase 5 with an L2 two-draw-order readback test. The native rebuild must choose per layer: OIT (weighted-blended or per-pixel linked-list) for the additive volumes, an explicit back-to-front sort or depth-prepass for the normal-blended sheets/shells. The web's `renderOrder` values (flux=10, sheets=3, nucleus=−2, shells=−3, lobes=−4, spin arrows=999) encode the intended manual layering and are the spec for a native sort order.

### 6.6 Vendored Three.js

Three.js **0.169.0** (MIT), byte-pinned, same-origin (`js/vendor/three/`). Addons actually used, each a port item: `OrbitControls`, the bloom postprocessing chain (`EffectComposer`/`RenderPass`/`UnrealBloomPass` + transitive shaders). `ConvexGeometry`/`RGBELoader` are import-mapped but have no live call sites in the core viewport (likely `fields-atlas.html`-only). Distinct primitive classes to reproduce: `THREE.Points` (28 sites, dominant), `LineSegments` (38), `Mesh` (36), `InstancedMesh` (7 — symmetry boxes, force-glyph cones, nuclei, orbital shells/lobes, bond cylinders), `Sprite` (billboard labels), and the standard geometry set (sphere/plane/torus/box/cylinder/cone/ring). No fat lines (`Line2`) anywhere.

---

## 7. Telemetry and instrumentation

### 7.1 The hub

`js/telemetry-hub.js` (~1,283 LOC) is a singleton (`export const telemetryHub`) — the single write path and source of truth for all buffered telemetry, the web analogue of the native `History` + `NativeTelemetryScheduler`. It defines three ring-buffer primitives: `RingBuffer` (one `Float32Array`, `{data, size, head, count, total}`, `flattenInto` for the two-segment wrap copy), `MultiRingBuffer` (one flat `Float32Array(size·channels)`, channel-major), and `RingBufferView` (a cursor into one channel, with `setLast()` to patch the most-recent row without advancing head).

Buffer inventory and retention:

| Member | Channels × samples | Populated by |
|---|---|---|
| `_s0_core` (flux, energy, manifested, entropy, charges, positive, negative, fieldSpin, fieldHelicity) | 9 × 500 | `_publishScale0Diagnostics` |
| `_s0_aud` (15 audit channels incl. `energyDrift`) | 15 × 500 | `_publishScale0Audit` |
| `_s0_lag` (10 Lagrangian terms) | 10 × 400 | `_publishScale0Lagrangian` |
| `ebDiff`, `gauss` | 1 × 500 each | audit path |
| `_s0_sp` legacy sparklines | 5 × 80 | diagnostics |
| `_s1_pe` | 25 × 200 | `collectScale1`(+`Extended`) |
| `_s2_ae` / `_s4_pl` / `_s5_cs` | 10 / 8 / 3 × 200 | `collectScale{2,4,5}` |

### 7.2 The provenance model — the part worth keeping

Scale-0 native telemetry arrives as **independent group deltas at staggered cadences** (`SCALE0_SNAPSHOT_GROUPS = [diagnostics, audit, lagrangian, gravity]`). Each group carries a meta descriptor `{source, epoch, sourceEpoch, stateVersion, snapshotVersion, tick, stale, receivedAt, ageMs}`, and the hub does version-gated acceptance (monotonic `sourceEpoch > epoch > stateVersion > tick > snapshotVersion`, `_compareScale0GroupMeta`) so a late/cached aggregate never overwrites a newer per-group value. Panel headers render this as freshness ("audit t1234 · 45 ms"). **(I)** This provenance/staleness layer is the one part of the telemetry design the native rebuild should preserve wholesale — it maps directly onto `NativeTelemetryScheduler` publishing per-group deltas at `every_ticks{1,8,4,12}` cadences and onto SPEC_UI_V2's per-group `*_meta.tick` charting rule (P7).

### 7.3 Origin: engine versus JS-derived

**(A)** The load-bearing distinction: **Scale 0 is the only scale whose telemetry is genuinely engine-computed** (the O(N³) curl/Poynting/divergence/Gauss audit runs in the engine, WASM or native CUDA). Scale 1 is native-engine base diagnostics plus ~14 JS-derived aggregates (virial, temperature, RMS velocity, 2-body separation, cap count, charge composition — recomputed from raw arrays and patched via `setLast()`). Scales 2/3 (mock atom engine), 4 (N-body energies computed *in the hub's own JS loop*), and 5 (mock cosmic) are entirely JS-derived. The Lagrangian panel's 8 constant rows (G*, 1/α, α, K_B, …) are static reads from `constants.js`, no hub source.

### 7.4 Demand-gating and the two read paths

**(P)** Expensive telemetry is demand-driven and version-gated; cheap primary telemetry is always-on. `telemetry/demand.js` computes a want-mask from panel visibility (`wantAudit` if any of diagnostics/charts/lagrangian/telemetry-grid/live-conservation is visible; `wantLag` only for lagrangian/telemetry-grid; `wantGravity` for gravity/time), with `every_ticks` cadence scaled by lattice size. On the native path, `WebSocketBridge.setTelemetryDemand()` declares the subscription and `getTelemetrySnapshot()` is a strict **cache-only read** — it "cannot enqueue a WebSocket command or CUDA reduction." The collection driver is `scales/scale0/runtime/diagnostics.js:updateDiagnosticsAndPanels`, gated at every 3rd frame.

Two read paths: **Path A (dominant)** — panels/charts read hub buffers only, never the bridge (`telemetry/scale0-read.js` codifies hub-first-bridge-fallback). **Path B (the exception)** — the **inspector** issues live per-voxel device reads (`inspectVoxel(x,y,z)` → 20+ scalar fields, `getForceAt(x,y,z)` → 5 force channels) with an explicit read budget (`NATIVE_REFRESH_MS=750`, `NATIVE_NEIGHBOUR_READ_BUDGET=9` cursored across the 26 Moore neighbours), showing unresolved cells as pending "…" rather than fabricating void.

**(I)** The elaborate want-mask/version-coalesce/mode-detection machinery (`demand.js` + ~700 lines of `ws-bridge.js` scheduling) and the inspector's budgeted-async-read discipline both exist *only* because the web has an expensive worker/RPC boundary. Native in-process reads are synchronous and cheap: the whole thing collapses to "read `History` when a panel is visible" and "read the voxel synchronously on pick." Charts consume hub buffers via uPlot (`ui/charts/uplot-chart.js`, `flattenInto` → `setData(...,true)` — the forced full-rescale-every-frame is a known deferred cost); the native side charts with ImPlot against the `History` ring buffers.

---

## 8. Scenarios, configuration, and constants

### 8.1 The constant chain is triplicated

**(A)(I)** The same FTD constant chain is materialized **three times, once per language, all hand-synced**:

| Layer | File | How produced |
|---|---|---|
| C++ (engine truth) | `engine/include/ftd/ontic.h` → `ontic/{lemniscate,master_quadratic,gauge_couplings,particle_masses,…}.h` | **computed** at compile time from D=3 + ϖ through the 9-layer chain; `constants.h` holds `ALPHA`, `ALPHA_EFT=G_C·G_C` with a `static_assert` |
| Python (verification) | `scripts/constants.py` | **computed** from `scipy.special.gamma` |
| JS (dashboard) | `engine/web/js/constants.js` (~595 lines) | **hardcoded decimal literals** mirroring `ontic.h`, plus a few derived-from-literals |

`constants.js` is a hand-transcribed copy — its own header states "these JS values mirror `ontic.h` … consumers MUST import from here." It carries the four framework integers (`N_C=3, N_BASE=4, B_3=7, N_EFF=13`), G* (`2.958675119188639`), the master-quadratic roots, `α` (`ALPHA=G_C²`, `[SMC]` tag inline), plus PDG reference masses, SEMF coefficients, Slater constants, Pauling tables, cosmic anchors, and even a GLSL simplex-noise string.

**The drift guard gap.** `engine/web/tests/verify_web_consistency.js` is a *lint*, not an equality check: it scans `js/**` (skipping `constants.js`) and fails if any other file re-hardcodes `137.036`, `0.511`, G*, or `G_N=0.01` — enforcing `constants.js` as the single JS source. But **no automated test reconciles `constants.js` against `ontic.h` or `constants.py`**; the WASM `getConstants()` export is "for observatory/display only." **(I)** The native rebuild consumes `ftd::ontic::*` directly and deletes the JS mirror — removing one of the three copies and the entire cross-language drift surface. `units.js` (the formatter layer, three unit regimes: Planck for Scale 0/1, Bohr-scaled for Scale 2; `SIM_ENERGY_TO_MEV = 1/C_SPEED² = 3` is the load-bearing electron-primary calibration) is a thin presentation utility that reimplements trivially in the ImGui layer.

### 8.2 The toggle system maps 1:1 to `TermToggles`

The canonical table is `TOGGLE_SPECS[]` in `engine/include/ftd/term_toggles.h` — **44 boolean toggles + 10 non-bool config fields** (the doc-quoted "43" is stale; the newest is `geometric_gravity`). Each `ToggleSpec` row is `{name, field-ptr, default, bulk_managed, requires_, conflicts, gpu_only_warning, backends bitmask (CPU/GPU/JS/ANY), description}`, and `validate()` enforces cross-field rules the table columns alone cannot express (SPEC_UI_V2 W3).

The JS UI does not parse the header — it maintains a hand-curated whitelist subset in `js/config/toggles.js`: `SCALE0_TOGGLES` (19–20 `[key, default, domId]` rows whose keys are **bit-identical to `TermToggles` field names**), `SCALE2_TOGGLES` (11 for the atom engine), per-scenario override tables (`SCALE0_SCENARIO_OVERRIDES` built by `isolatedScale0Profile(...)` returning an all-off profile with only named terms on), boundary tables, and research-term pins. A separate render-only family — ~30–32 field/overlay toggles in `scale0/state/store.js` `FIELD_TOGGLE_KEYS`, bound in `scale0/ui/dom.js`, rendered by `scale0/ui/overlays/template.js` — never reaches the engine and gates only viewport visibility. Parity between the JS whitelist and `TOGGLE_SPECS` is guarded only by Playwright (`scenario-parity.spec.js`, `toggle-coverage.spec.js`), which also asserts four documented default divergences (`gravity`, `lorentz_force`, `dual_substrate`, `weak_transmutation` start off in the dashboard, on in the C++ constructor).

**(I)** The native UI reads `TOGGLE_SPECS[]` directly — one in-process source — and deletes the JS whitelist mirror, its render-only-vs-sim split maintained across three files, and the parity guards entirely. Physics-term toggles bind to `TermToggles`; the field/overlay family becomes a separate render-state struct in the D3D12/ImGui layer with no engine involvement (SPEC_UI_V2 draws exactly this line).

### 8.3 Scenarios: four JS layers over one C++ live path

A scenario is a named one-shot initial-condition seed recipe (not a saved state, not a per-frame script), keyed only by a string id. A fully-wired Scale-0 scenario touches up to five files linked by that id alone (no shared schema):

| Layer | File | Role |
|---|---|---|
| UI registry | `scales/scale0/scenario-registry.js` | ~131 descriptors (dropdown source), `makeScenario()` factory + literals |
| JS seed impl | `js/bridge/scenarios/{flux,light,quantum,vacuum,s0-seed,s0-field}-scenarios.js` | **parity mirror only** — the in-thread-fallback path |
| **C++ seed impl** | `engine/src/scenarios/*.cpp` + `scenarios.cpp` + `_helpers.h` | **the canonical live path** via `dispatch_scenario(rb, name)` (WASM/native) |
| Metadata | `js/config/scenarios.js` | descriptions + epistemic tags |
| Toggle profile | `js/config/toggles.js` | §8.2 |

**(A)** The live path is C++: `WasmBridge.setupScenario` → `ftd::dispatch_scenario(rb, name)`, which resets the scenario RNG (`mt19937(0xC0DEFACE)`) and walks six `setup_*_scenario` group functions in prefix order; `scale0_scenario_ids()` is the authoritative id list (~130). The JS seed tree is a name-parity mirror, guarded by a *source-text lint* (`scenario-parity.spec.js`) that proves name bijection, not semantic equality (stochastic scenarios match only in distribution — JS `Math.random()` is unseedable versus the C++ seeded stream). Seed primitives are the macros `IF`/`IW`/`IP`/`IPF`/`SET_VEL`/`LOCK`/`SET_SPIN` (`_helpers.h`), staged on the host shadow with one lazy GPU upload after setup. **⚠ `dispatch_scenario` validates last** — for a known id the body runs (mutating toggles, injecting voxels) before `validate()` can reject the profile, so a rejection leaves a half-mutated bridge (SPEC_UI_V2 W9); the native shell must treat rejection as fatal and re-boot to a known-good scenario.

**(I)** For the native replan this collapses dramatically: one C++ process means the JS seed-impl layer and the JS↔C++ parity guard both disappear. Scenarios become a single C++ registry (`scale0_scenario_ids()` + the new `ScenarioMeta` table SPEC_UI_V2 §5.1 introduces) + seed body + toggle profile. The `native_desktop` shell already calls `dispatch_scenario` directly (`engine_session.cpp`).

---

## 9. UI shell and panels

### 9.1 The composition root

`js/app.js` (~1,711 LOC) is a single module-scoped imperative controller — not a class. State is ~40 module-level `let` variables (`bridge`, `viewport`, `appShell`, `engineMode`, `running`, `ticksPerFrame`, `activeTab`, …). It mixes six responsibilities in one file: bridge init (`bootBridge`, `app.js:474`), global play state, the master rAF loop (`animate`, `:673`), mode/scale switching (`switchEngineMode`, `:1514` — the sole transition entry), scale-controller registration (seven `import * as ScaleNController`, `:19-26`), cross-scale glue (`_makeCtx()` singleton + two hand-maintained per-frame ctx mirrors), and a large block of hand-rolled `getElementById().addEventListener` DOM wiring (`wireToolbar`/`wireTabs`/`wireControls`/`wireViewportToggles`/`wireKeyboard`). The settings modal is an inline IIFE block persisting theme/scale/density/panel-width/tooltips/status-bar to `localStorage`.

**(I)** The web's cross-cutting weaknesses, each of which the native design already resolves: the `engineMode`→scale-index map is duplicated in three places (`CONTROLLERS`, the CSS-class block, an inline literal); the scale-controller interface is duck-typed with no base class and differs per scale; `ctx` exists in three shapes; and cross-module wiring leans on mutable `window` globals — `window.__ftdCtx` (the most-used handle), `window._ftdBridge`, and ~13 `window.__ftd*Panel` singletons. `REF_DEBUG_GLOBALS.md` itself flags that reaching for a global "is a sign a missing export is needed," yet the whole Scale-0 panel↔bridge path depends on `window.__ftdCtx`. SPEC_UI_V2's shell-owned `PanelContext` passed to `draw_contents` is the correct fix.

The `native_desktop` shell already replaces this: `app.js` is a WebView2/postMessage host contract in disguise — the web app *already* speaks to a desktop host (`window.chrome?.webview?.postMessage` for `engine-error`/`engine-progress`, `app.js:418-435`), so the native error/restart contract carries over.

### 9.2 The DOM shell

`js/ui/shell/app-shell.js` (223 LOC) is a **façade over pre-existing HTML, not a builder**: `ensureShellTemplate` looks up seven static DOM regions by id (toolbar, viewport, tab-bar, panel-area, status-bar, settings, toasts) and tags them, then `init()` assembles ~15 chrome sub-components in fixed order (loading overlay, viewport frame, topbar, knowledge base, FAQ, viewport overlays, workspace tabs, panel dock, tooltips, keyboard help). A `BreakpointService` writes `data-layout-mode`/`orientation`/`compact` and drives the mobile bottom-sheet path. **(I)** The whole responsive/mobile/`visualViewport` layer (`mobile-panel.js`, breakpoint table, browser-chrome inset tracking) is dead weight for a native Windows app; the native shell authors the seven regions once as an ImGui dockspace (viewport = central node; toolbar/tabs/panels/status = docks and chrome) per SPEC_UI_V2 §4.

A ~200-LOC home-grown mini-framework (`js/core/`) provides four primitives — a `ServiceRegistry` (`appRegistry`, the reliable lookup mechanism), an `EventTarget` store (`appStore`), a template-cloning `BaseComponent`, and a listener tracker (`uiBinder`) — but **it is only partially adopted**: real state lives in `app.js` module vars, `appStore`/`uiBinder`/`BaseComponent` are inconsistently used, and this shadowing is itself a fragmentation risk. The native rebuild collapses all of it into one owned UI-model + command sink (SPEC_UI_V2 §3.3b `UiSnapshot`/`CommandSink`/`PanelContext`).

### 9.3 The panel system — the definitive inventory and the mount-pattern mess

The authoritative source of truth is `PANEL_REGISTRY` (`js/ui/scale-registry/panel-registry.js`), a frozen **22-row** table of `{id, label, icon, scales}` (where `scales:null` = all scales). It drives tab generation and per-scale visibility (`PanelDockController.applyScaleFilter` reads `tab.dataset.scales`). This 22-row registry — not the file layout — is the inventory the native panel set should be built from.

The panels are mounted by **three live conventions layered over a fourth, abandoned one** — the single most important portability finding, and a refinement of SPEC_UI_V2 §3.3:

- **Pattern A** (`ui/panels/*/component.js`): a `*PanelComponent` class plus an `init*Panel()` factory returning the instance; app-driven via `.update()` in the master loop. **Of 11 such factories, only 5 are live** (diagnostics, charts, telemetry-grid, lagrangian, scene). ⚠ **The other 6 are orphaned dead code** — `initInspectorPanel`/`initZooPanel`/`initPhysicsPanel`/`initOnticPanel`/`initPlanetaryPanel`/`initCosmicInfoPanel` have no call site; the *real* inspector/zoo/physics/ontic/planetary/cosmic panels are rendered by entirely different modules (`inspector/app-runtime.js`, `zoo.js`, `ui/app-ontic.js`, the scale controllers). So `ui/panels/` is half live component-classes, half scaffolding for a migration that never completed.
- **Pattern B** (`scales/scale0/ui/overlays/*.js`): `mount*Panel(host, getBridge)` builds detached DOM, subscribes *itself* to `rafCoordinator`, returns a plain `api` object, and stashes it on `window.__ftd*Panel`; a thin `init*Panel()` wrapper self-locates `#panel-<id>`. Ten such panels (flux-slice, wave-lab, p1-observables, spectrum, gravity, time, thermo, dispersion, knots, scale-context), self-driven — a different frame owner than Pattern A.
- **Pattern B′** (ad-hoc): `mountSymmetryPanel(#app)`, `mountGenesisBurstPanel(harness)`, `mountConservationMicropanel` — direct-mount with no registry entry.
- **Pattern C** (dead): `mountFluxSlicePanel` is confirmed dead (the live path is a `FluxSlicePanel` class). ⚠ SPEC_UI_V2's second named dead export, `initOverlayPanel`, **does not exist** — the nearest symbol `initOverlayPanelShell` is live. The genuinely dead set is the 6 orphaned Pattern-A factories + `mountFluxSlicePanel` = **7**, not 2.

The three patterns are mutually incompatible on return type (class instance vs plain `api`), handle discovery (captured return vs `window.__ftd*` global), update lifecycle (app-driven `.update()` vs self-driven rAF), host acquisition (`#panel-<id>` vs explicit arg), and disposal (`.cleanup()` vs global `dispose()`). **(I)** SPEC_UI_V2 §3.3's single `Panel` vtable (`id`/`title`/`default_slot`/`needs`/`flags`/`draw_contents`) with the shell owning `Begin/End`, visibility, demand-OR, and history — one file plus one registration line per panel, no `window.__ftd*` stash, no per-panel rAF — is exactly this consolidation. The native panel set should be driven by the 22-row `PANEL_REGISTRY`, collapse the render-owner column so each panel is one file, and drop the parallel CSS `.scaleN-only !important` visibility layer in favor of `DockSlot` + a per-scale predicate.

### 9.4 Content subsystems

Knowledge base, FAQ, keyboard help, and tooltips are **static JS data modules** rendered by generic components — no fetch, no markdown files, so they port cleanly to embedded C++ string tables. The knowledge base and FAQ are two instances of one master-detail widget (`components/sidebar-library/`) over section/entry arrays (`knowledge-base/data.js` ~87 KB; `faq/data.js` ~38 KB with epistemic-tagged answers), mutually exclusive on open. Keyboard help and tooltips are static tables (`SHORTCUTS`; a `[selector, text]` map). All content routes through `renderMathInHtml` for math glyphs — the native equivalent needs a Unicode/glyph strategy in the ImGui font (SPEC_UI_V2 `Theme.metrics.font_size`). In ImGui these become a data-table + detail pane, a shortcut cheat-sheet window, and `ImGui::SetItemTooltip` keyed off a selector→string map.

---

## 10. Build, serve, test, deploy

### 10.1 The WASM triple build

`engine/build_wasm.bat` builds the one `ftd_wasm` Emscripten target three times into separate trees (§3.4): `ftd_core` (wasm32, 2 GB), `ftd_core64` (Memory64, 8 GB), `ftd_core_mt` (pthreads + `SharedArrayBuffer`, the default deployed Scale-0 engine). Common flags `-O3 -flto -msimd128 -mbulk-memory -ffast-math`; `ftd_core` is `-fno-exceptions` (RTTI kept for embind). Deploy is staged all-or-nothing into `engine/web/wasm/` with a git-SHA `build_info.txt`. The CMake target (`engine/wasm/CMakeLists.txt`, guarded `if(EMSCRIPTEN)`) unions four embind TUs (`ftd_wasm.cpp` + `bindings_{render_bridge,particle,atom}.cpp`). **(I)** The native rebuild deletes this entire triple-ABI matrix, the SIMD/LTO flag-persistence problem, and the wasm32/64/threads feature-detection fork — one native binary, one heap, real threads and CUDA.

### 10.2 Serve and cross-origin isolation

`engine/web/serve.py` (a `ThreadingHTTPServer`, default port 8080) sends **COOP `same-origin` + COEP `require-corp` + CORP `same-origin` on every response** — the load-bearing part, since it makes the page `crossOriginIsolated` and unlocks `SharedArrayBuffer` for the Scale-0 physics worker — plus `no-store` cache headers (unless `--cache`, used by Playwright). `coi-serviceworker.js` is the fallback for static hosts (GitHub Pages) that cannot set those headers, injecting them client-side via a service worker (no-op under `serve.py`; skips the port-9100 native WebSocket). **(I)** The native rebuild has no HTTP layer, no COOP/COEP dance, no service-worker shim, no `SharedArrayBuffer` gymnastics — the physics field is just C++ memory.

### 10.3 No-bundler ES modules

`index.html` loads raw ES modules via an import map (§2.1) — no webpack/rollup/Vite in the served path. Cache-busting is manual `?v=N` query strings on ~70 CSS links and the entry script. External CDN deps (KaTeX, Google Fonts) would violate a native CSP and must be replaced with embedded assets. **(I)** There is effectively no build/typecheck/lint step on the shipped path (no `engine/web/package.json`, no `tsconfig`, no eslint config; `verify_web_consistency.js` is a standalone `node` lint) — the app ships untyped, unbundled, unlinted JS. A native C++/ImGui UI has a real compiler and type system, eliminating this surface.

### 10.4 The test suite

The web regression harness is Playwright (`engine/web/tests/`, ~69–80 `.spec.js` files, Chromium-only, `workers:1` because the engine is stateful per page), auto-starting `python serve.py 8081 --cache --quiet`. Coverage clusters: scenario/toggle parity (`scenario-parity.spec.js` — the JS↔C++ seed-body bijection and toggle-name parity; `toggle-coverage.spec.js`), scale switching/lifecycle (`scales.spec.js`, `lifecycle-harness.spec.js`), the worker/SAB path (`scale0-worker*.spec.js`, the 100 KB `ws-bridge-visual-cache.spec.js`), the substrate physics protocol (`scale0-substrate-protocol-v2.spec.js` — Maxwell Hamiltonian conservation, `c_lat`, locality, determinism), telemetry contracts, and perf baselines. Plus a few `node`-only `.test.mjs` unit tests and a standalone `native-ws-smoke.mjs` that drives `ws_server`. Static tooling from the repo root: `verify_web_consistency.js` (the constants-drift lint of §8.1). ⚠ ~12 web tests are known-RED locally; the suite is not a merge gate. The native shell replaces this with the CTest pyramid SPEC_UI_V2 §9 defines (headless ImGui, D3D12 device tests, CUDA interop, the neutrality/journal-replay gates).

### 10.5 The native `ws_server` path

`engine/src/ws_server.cpp` (+ `ws_protocol.cpp`) is a standalone executable — a minimal WebSocket over raw winsock2/POSIX sockets, no external deps, loopback-only by default, **no auth or Origin check**. It builds a `RenderBridge` with `set_interactive_gpu_mode(true)`, owns a `NativeTelemetryScheduler`, and serves binary frames. **(P)** The RTX 5090 ~30× speedup is only via the **WSL2** build (`engine/build_wsl`); Windows-native CUDA runs but is pathologically slow (≈19 min for one L=64 seed). `ws_server.exe` and the `native_desktop` app are both Windows-native processes, so they get Windows-native CUDA — acceptable for interactive single-tick work, slow for measurement campaigns (which run headless in WSL2). **(I)** The native app is essentially the `ws_server` engine host with a D3D12/ImGui front end instead of a WebSocket; the two drive the identical `RenderBridge`+`GpuBackend`+`NativeTelemetryScheduler` stack.

### 10.6 The WPF prior attempt (`engine/desktop`)

A .NET 8 WPF app that embeds the *existing web dashboard* in WebView2, served from an in-process Kestrel loopback server (mirroring `serve.py`'s COOP/COEP headers), talking to the CUDA engine in WSL2:

```
WPF + WebView2 → Kestrel static server 127.0.0.1:8080 → dashboard
              → WebSocket 127.0.0.1:9100 → ws_server (WSL2) → RenderBridge → GpuBackend → CUDA/RTX 5090
```

`EngineHost.cs` supervises the WSL2 engine — preflight (`nvidia-smi`, WSL2 kernel), incremental `cmake --build engine/build_wsl --target ws_server`, launch with `FTD_FORCE_GPU=1`, and a **GPU-authenticity contract that never trusts "compiled with CUDA"**: it requires the server's `info` reply to report `backend:"cuda"` + `gpu:true` + guarded interactive GPU mode, matches an external listener's PID to this repo's `ws_server` binary before reuse, memory-preflights before resize, and (`MainWindow.xaml.cs`) verifies the loaded page kept its native GPU bridge (not WASM fallback) and that WebView2 exposes a non-software WebGL renderer — green status only after all pass. **(I)/(P)** This entire trust-but-verify layer — WSL2, WebView2, Kestrel, the WebSocket protocol and its FTV2 compact frames (an admission that JSON was the bottleneck), the single-client reload dance, the process-ownership checks, the software-renderer detector — exists *purely to paper over the process boundaries*. The `native_desktop` D3D12/ImGui rebuild deletes all of it: the renderer and the CUDA engine share one address space, so there is nothing to attest.

---

## 11. Native-port coverage inventory

This section is a factual map — what already exists natively versus what remains web-only — so the replan can see the actual surface. It makes no phased recommendations.

### 11.1 The native engine asset inventory

Per-domain C++ engines already exist; the browser reaches only two. This table is the port's opportunity map:

| Scale | Native C++ engine | GPU backend | Wired to WASM/web? | Native disposition |
|---|---|---|---|---|
| 0 Lattice | `RenderBridge` (+ `SimEngine` wrapper) | `gpu_engine.h` (cuFFT Poisson) | ✅ yes (WASM `CpuBackend` only; native via `ws_server`) | already in `native_desktop` in-process |
| 1 Particle | `ParticleEngine : ScaleEngine` | `gpu_particle_engine.h` (Coulomb+gravity kernel) | ✅ embind, **CPU-only** (GPU `#ifdef`-gated out) | wire the existing CUDA backend in-process |
| 2/3 Atom/Molecule | `AtomEngine : ScaleEngine` (+ Barnes-Hut) | `gpu_atom_engine.h` | ❌ **compiled but disabled** (`_aeHasWasm=false`, Planck↔Bohr unit gap) | build the unit shim, wire the engine + GPU, delete the JS mock |
| 4 Planetary | *(none)* | — | ❌ JS mock only | write new C++ N-body (reuse cosmic Barnes-Hut/Verlet) |
| 5 Cosmic | `CosmicEngine : ScaleEngine` (18-phase, true SPH, `f_hubble`, GW) | (CPU; richer than the mock) | ❌ **compiled, CTest-covered, unwired** | wire the existing engine to the UI, delete the JS mock |
| Meta (6) | *(none — pure geometry)* | — | — | D3D12+ImGui geometry/label port |

The `ScaleEngine` base (`scale_engine.h`) and the `SimEngine` GPU-or-CPU selector (`engine_select.h`) already provide the polymorphic hosting the native shell needs. The net physics-implementation work is: one unit shim (Atom), one new small N-body (Planetary), and wiring — not reimplementation.

### 11.2 What `native_desktop` already covers (Scale 0)

Per SPEC_UI_V2 and the current tree (~6,400 LOC, ~31 test binaries):

- **Real in-process `RenderBridge`** with `set_interactive_gpu_mode(true)`, booted at L=32 on `s0-seed-hydrogen` (`engine_session.cpp`).
- **A two-thread model** — the sim thread owns the bridge exclusively (`tick → process_ui_boundary → interop gather → capture` into a mutex-guarded `NativeFrame`); the GUI thread never touches the bridge, copies the latest published snapshot, and calls `presenter.render()`. This already fixes the web's cross-thread hazards (§3.3).
- **A D3D12 presenter** drawing lattice/particles/flux with **CUDA↔D3D12 interop** (shared buffer + fence, with a CPU visual-snapshot fallback).
- **A tick-boundary command spine** — typed `UiCommand` queue → ordered drain → `apply_mutation` + `ParameterJournal` (reads back the applied value) → `build_snapshot` → immutable `shared_ptr<const UiSnapshot>` publisher. `UiSnapshot` already carries telemetry/energy/voxel/force/continuity/toggles/knobs fields (most not yet displayed).
- **An ImGui dockspace shell** — Graphite theme, save/load workspaces, per-monitor-V2 DPI, status bar; the presenter is ImGui-free via an opaque `OverlayRecorder`.

Stubbed pending SPEC_UI_V2 Phases 4–6: only 1 of 18 field kinds is sampled (`append_flux` hardcodes `FluxVector`); the 44-toggle table (only the flux-boundary combo works), the 18 field overlays, all telemetry/audit/Lagrangian charts (the `History` type exists but is never written), and voxel picking are all placeholders.

### 11.3 What remains web-only, by subsystem

| Subsystem | Web state | Native gap |
|---|---|---|
| Scales 1–6 | live in the browser (real PE + 4 mocks + geometry) | not in `native_desktop` at all — the shell is Scale-0 only (SPEC_UI_V2 D3) |
| Multi-scale hosting | `engineMode` switch over 7 controllers | needs a `ScaleEngine*`-per-scale switch (the base exists, the shell does not use it yet) |
| Rendering | Three.js: point-sprite clouds, field overlays, rubber sheets, molecular/cosmic/planetary shaders | D3D12 has only the lattice/particle/flux draw; no field overlays, no shaders, no OIT |
| Field overlays | ~30 web overlays (14 JS proxies) | 17 of 18 engine field kinds unsampled; no legend/stride/ramp UI |
| Telemetry charts | uPlot over the hub's ring buffers | `History` + `NativeTelemetryScheduler` exist but are unwired to ImPlot |
| Inspector | live per-voxel picking | `InspectVoxel`/`InspectForce` commands work; no viewport picking, panel stubbed |
| Scenario metadata | rich JS registry (~131, titles/tags/descriptions) | C++ has ids only; SPEC_UI_V2 §5.1 introduces `ScenarioMeta` |
| Panels | 22-row registry, 3 mount patterns | the vtable + 10 panel files exist; most panel bodies stubbed |
| Content | KB (87 KB) / FAQ (38 KB) / tooltips | not ported |

---

## 12. Architectural improvements for the native rebuild

The instruction to *improve the architecture where necessary* resolves into a small number of themes. Each is an "improve rather than transcribe" directive, drawn from the `(I)` findings throughout, ordered by leverage.

1. **Collapse the transport layer into in-process calls.** The four bridges, the worker/`SharedArrayBuffer`/`postMessage` machinery, the CTRL Atomics header, the sampler want/unwant registry, the ready/watchdog/fallback protocol, the WASM triple-ABI, the COOP/COEP/service-worker isolation, and the WebSocket binary protocol (§4, §5.1, §10) all exist to move physics off the browser thread and reach CUDA over a boundary. In one address space they vanish. This is the largest simplification and the whole point of the port.

2. **Wire the dark native engines; delete the JS mocks.** `AtomEngine` (with its Barnes-Hut and CUDA backend) and `CosmicEngine` (18-phase, true SPH, real Hubble drag) are compiled and tested but unreachable from the browser (§5.2, §5.5, §11.1). Enabling the CUDA `ParticleEngine` backend and building the Planck↔Bohr unit shim for the AtomEngine turns four JS mock engines into two real C++ engines plus one small new N-body — the accuracy win the port promises.

3. **Make `ontic.h` / `TOGGLE_SPECS[]` / `dispatch_scenario` the single sources of truth.** The constant chain is triplicated, the toggle whitelist is a hand-synced mirror, and scenarios span a JS seed layer paralleling the C++ one, each guarded (weakly) by Playwright (§8). In one C++ process the JS mirrors and their parity guards disappear; the native UI reads the canonical tables directly. Keep the render-only field/overlay toggle family as a separate UI-state struct, not engine state.

4. **Adopt one panel discipline.** Replace the three live mount patterns (plus 7 orphaned dead factories and ~13 `window.__ftd*Panel` globals) with SPEC_UI_V2's single `Panel` vtable and shell-owned `Begin/End` + demand + history, driven by the 22-row `PANEL_REGISTRY` (§9.3). Collapse the parallel CSS `.scaleN-only !important` visibility layer into `DockSlot` + a per-scale predicate. Consolidate the three `ctx` shapes and the partially-adopted `js/core/` mini-framework into one owned `UiSnapshot`/`CommandSink`/`PanelContext`.

5. **Rebuild the renderer on GPU-native data.** Re-author the ~5 custom GLSL programs as HLSL — above all the point-sprite `PARTICLE_VERT`/`PARTICLE_FRAG` pair (as instanced billboard quads, since D3D12 has no `gl_PointSize`), which covers the whole point-cloud surface (§6.3). Write engine field data straight into D3D12/CUDA-interop buffers instead of CPU-filling typed arrays each frame, and move coloring into shaders. Decide the P1 translucency question explicitly per layer (OIT for additive volumes, sorted/depth-prepass for normal-blended sheets), using the web's `renderOrder` values as the intended sort spec.

6. **Own threads and determinism explicitly.** The web's `mutable`-state races, OpenMP-conditional locking, and null-`bridge` reload windows (§3.3) are real debts; the `native_desktop` sim-thread-owns-the-bridge / GUI-thread-reads-a-snapshot model already fixes them and must be preserved. Keep the golden gate (§3.5) and the tick-boundary command drain (SPEC_UI_V2 §3.4) as the invariants the UI may never perturb.

7. **Eliminate duplicated force/physics implementations.** Expose the engine's own `force_diag_` buffer rather than the WASM binding's second O(N²)-per-read copy (§5.1), and drop the WASM visualization samplers that re-derive force fields divergently from the engine (§3.4). One implementation per physical quantity.

8. **Keep what the web got right.** The per-group telemetry provenance/staleness model (§7.2) maps directly onto `NativeTelemetryScheduler` and should be preserved; the `ScaleEngine`/`SimEngine` abstractions are the correct hosting seam; the inspector's honest "pending, never fabricated void" discipline should carry over (synchronously, without the async read budget); and the epistemic tags on catalog/scenario/overlay data must remain visible in the native UI.

---

## Appendix — canonical file map

| Concern | Files |
|---|---|
| Scale-0 engine | `engine/src/render_bridge.cpp`, `render_bridge_phases/*.cpp`, `poisson_solvers.cpp`, `transmutation_phases.cpp`; `include/ftd/{render_bridge,voxel,lattice,backend,voxel_rng,term_toggles}.h`; `cuda/gpu_engine.cu` |
| Per-scale engines | `include/ftd/{scale_engine,engine_select,particle_engine,atom_engine,cosmic_engine}.h` + `gpu_{particle,atom}_engine.h`; `src/{particle_engine,atom_engine,cosmic_engine}.cpp`, `src/{atom,cosmic}/*.cpp` |
| WASM | `engine/build_wasm.bat`, `wasm/CMakeLists.txt`, `wasm/{ftd_wasm,bindings_render_bridge,bindings_particle,bindings_atom}.cpp`, `web/wasm/` |
| Bridges | `web/js/ws-bridge.js`, `web/js/bridge/{wasm-bridge,wasm-bridge.worker,wasm-bridge-proxy,bridge-contract,native-particle-engine,mock-atom-engine,mock-scale4,mock-scale5}.js`, `web/js/bridge/capabilities/*.js`, `web/js/app-wire/bridge-boot.js` |
| Scales (JS) | `web/js/scales/scale{0..6}/`, `web/js/{particle-catalog,zoo,spectroscopy,elements,atomic-props,atomic-energy,orbitals,molecules}.js`, `web/js/bridge/cosmic-physics.js`, `web/js/config/exoplanet-seeds.js` |
| Rendering | `web/js/viewport.js`, `web/js/viewport/*.js`, `web/js/core/BaseRenderer.js`, `web/js/{cosmic,planetary}-renderer.js`, `web/js/meta-unit*.js`, `web/js/vendor/three/` |
| Telemetry | `web/js/telemetry-hub.js`, `web/js/telemetry/*.js`, `web/js/ui/charts/*`, `web/js/inspector*`; `include/ftd/native_telemetry_scheduler.h` |
| Config | `web/js/constants.js`, `web/js/units.js`, `web/js/config/toggles.js`, `include/ftd/ontic.h`, `scripts/constants.py`, `include/ftd/term_toggles.h` |
| Scenarios | `web/js/scales/scale0/scenario-registry.js`, `web/js/config/scenarios.js`, `web/js/bridge/scenarios/*.js`, `engine/src/scenarios/*.cpp`, `include/ftd/scenarios.h` |
| Shell / panels | `web/js/app.js`, `web/js/ui/shell/*`, `web/js/ui/scale-registry/panel-registry.js`, `web/js/ui/panels/*`, `web/js/scales/scale0/ui/overlays/*`, `web/js/core/*` |
| Build / serve / test | `engine/build_wasm.bat`, `web/serve.py`, `web/coi-serviceworker.js`, `web/index.html`, `web/tests/`, `web/tests/verify_web_consistency.js` |
| Native shells | `engine/native_desktop/` (D3D12+ImGui, target), `engine/desktop/` (WPF+WebView2+WSL2, prior), `engine/src/ws_server.cpp` |
| Governing specs | `engine/native_desktop/docs/SPEC_UI_V2.md`, `engine/web/ARCHITECTURE.md`, `engine/web/docs/SPEC_SCALE0_*.md`, `engine/SPEC_ENGINE.md`, `CONTRACTS.md` |




