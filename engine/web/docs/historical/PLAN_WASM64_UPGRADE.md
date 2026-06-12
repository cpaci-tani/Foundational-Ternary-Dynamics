# WASM64 (Memory64) Upgrade — Lift the 2 GB Browser-Lattice Cap

Status: `[IMPLEMENTED]` 2026-06-03 — dual wasm32/wasm64 build + feature-detect loader shipped & verified.
Scope: the browser WASM build (`engine/wasm/`) + the Scale-0 web loader/caps (`engine/web/`).

**Implemented & verified (2026-06-03):**
- `engine/wasm/CMakeLists.txt`: `option(FTD_MEMORY64)` → adds `-sMEMORY64=1 -sWASM_BIGINT=1`, 8 GB max,
  `OUTPUT_NAME ftd_core64` / `EXPORT_NAME createFTDModule64`. Also flags `ftd_eft` (a third linked lib)
  for Memory64 — without it the link fails ("wasm32 object can't be linked in wasm64 mode").
- `engine/build_wasm.bat`: builds **both** `build_wasm/` + `build_wasm64/`, deploys all four artifacts.
  (Keep it ASCII-only — cmd chokes on UTF-8 box-drawing/arrows in `.bat`.)
- `wasm-bridge.js`: `supportsMemory64()` feature-detect → loads `ftd_core64`/`createFTDModule64` when
  available else wasm32; `this.isWasm64`; range bound raised to 257.
- `scenario-loader.js` resize guard: **per-owner cost + cap**. The guard estimates the heap of the
  bridge that ACTUALLY owns the scenario — flux-*/s0-* run on the JS MockBridge (~150 B/voxel, 2 GB
  JS-heap cap → ~L256) and the C++ grid is NOT reallocated on a flux- resize; empty/light/quantum run
  on the C++ engine (~1300 B/voxel, 8 GB wasm64 / 2 GB wasm32 cap → ~L187 / ~L117).
- Dropdown (`toolbar/template.js`): added odd 145, 181.
- **Verified on Chrome:** loader picks wasm64 (`isWasm64=true`, only `createFTDModule64` loaded);
  L=145 resize allocates the full field (`145³` readback) at **~2.81 GB heap** (past the old 2 GB cap),
  samplers return correct counts (no uint32 truncation); 51 Scale-0 Playwright tests green on wasm64.
- **2026-06-03 follow-up fix:** the first cut of the guard used the C++ 1300 B/voxel cost AND a 2 GB cap
  for flux-*/s0-* — but those run on the cheap JS MockBridge and don't realloc the C++ grid, so big
  *centered* flux- lattices (the common case — flux-pulse is the default) were refused over memory that
  is never allocated, even on wasm64 ("L=145 would need ~3.69 GB … max 2 GB. Refusing to resize."). The
  guard now uses the MockBridge's real ~150 B/voxel for mock-owned scenarios, lifting their cap from
  ~L117 to ~L256. Regression-pinned by `engine/web/tests/scale0-resize-guard.spec.js` (3 tests).
- **Not done (deliberate):** `ftd_eft` keeps its other flags; ~150 native CTests untouched; native
  `ws_server` (L≤256) untouched; WebGPU not attempted.

---

## Context

The browser engine compiles the FTD C++ to **WASM32**, whose linear memory is capped at **2 GB**. The
Scale-0 resize guard (`scenario-loader.js`: `N³ × 1300 bytes ≥ 2 GB → refuse`) therefore limits the
in-browser lattice to **~L117**; the size dropdown is capped at 113. The 2026-06-03 odd-lattice work
makes phenomena center on a true center voxel, and bigger *centered* lattices are now wanted, but they
don't fit in 2 GB.

**Goal:** lift the in-browser cap to the browser Memory64 ceiling (~8–16 GB → **L≈180–230**) while
keeping the entire C++ engine, via Emscripten **Memory64** (`-sMEMORY64`).

### Why WASM64 and not the alternatives (researched 2026-06)

- **WebGPU compute** is *technically superior* for a 3D lattice (field stays in GPU VRAM, massively
  parallel — the existing CUDA kernels are exactly this shape) but is a **6–12 week rewrite** of
  ~7,200 LOC of kernels into WGSL. Deferred — the long-term ceiling-breaker, not now.
- **Native `ws_server` bridge** (WebSocket → WSL2 CUDA, port 9100) already runs lattices up to **L=256
  today** with no WASM cap. It remains the escape hatch for the very largest runs; this upgrade does
  not touch it.
- **WASM64** is the pragmatic cap-lift: keeps the codebase, ~1–2 days, reaches ~L180–230 in-browser
  (close to the native L256 cap). Costs: ~5% mean perf overhead (up to ~37% on some workloads),
  +6–10% binary, 2× pointer memory; **Safari/iOS and (flagged) Firefox still need a wasm32 fallback.**

**Browser status (mid-2026):** Memory64 is Phase-4, **production in Chrome** (no flag), **behind a flag
in Firefox**, **coming to Safari** (objection withdrawn late 2025). Browsers cap Memory64 at ~16 GB;
**iOS Safari throttles WASM memory hard** (test >1 GB on-device). Sources:
[caniuse Memory64](https://caniuse.com/wf-wasm-memory64),
[State of WebAssembly 2026](https://devnewsletter.com/p/state-of-webassembly-2026/),
[Wasm64 is here](https://unlimited3d.wordpress.com/2025/02/07/wasm64-is-here/),
[memory64 perf #31](https://github.com/WebAssembly/memory64/issues/31).

---

## Recommended approach: dual build + feature-detect loader

Ship **two** WASM artifacts and load the right one at boot:
- `ftd_core.{js,wasm}` — wasm32 (unchanged; fallback for Safari/iOS/flagged-Firefox).
- `ftd_core64.{js,wasm}` — wasm64 (Memory64, 8 GB max).

The dual build is the load-bearing complexity: the engine lib (`ftd_core`) must be compiled **once per
pointer ABI**, so wasm32 and wasm64 need **separate build trees** (`build_wasm/`, `build_wasm64/`) —
a single `libftd_core.a` cannot serve both.

### Changes

1. **CMake** — `engine/wasm/CMakeLists.txt`: add `option(FTD_MEMORY64 OFF)`. When ON, append to the
   existing `target_link_options(ftd_wasm …)` (lines 43–59): `-sMEMORY64=1 -sWASM_BIGINT=1`, raise
   `-sMAXIMUM_MEMORY` 2 GB → `8589934592` (8 GB), and set `OUTPUT_NAME "ftd_core64"` +
   `-sEXPORT_NAME=createFTDModule64`. The wasm32 path is unchanged when OFF.

2. **Build wrapper** — `engine/build_wasm.bat`: build BOTH — the existing `build_wasm/` (wasm32) and a new
   `build_wasm64/` configured with `-DFTD_MEMORY64=ON`; deploy all four files
   (`ftd_core.{js,wasm}`, `ftd_core64.{js,wasm}`) to `engine/web/wasm/`.

3. **Feature-detect loader** — `engine/web/js/bridge/wasm-bridge.js` `init()` (~lines 96–114): detect
   Memory64 once via `try { new WebAssembly.Memory({initial:1, maximum:1, index:'i64'}); } catch {}`; if
   supported, inject `wasm/ftd_core64.js` and call `globalThis.createFTDModule64(...)`, else the current
   `wasm/ftd_core.js` + `createFTDModule(...)`. Expose the active mode (`this.isWasm64`) so the caps
   below can branch.

4. **Raise the matching JS caps** (else the heap grows but the guards still refuse):
   - `maxWasmMemory = 2 GB` guard in `scenario-loader.js:346` → 8 GB when `isWasm64`, 2 GB on fallback.
   - Size dropdown (`scale0/ui/toolbar/template.js`) → add larger odd options (e.g. 145, 181, 229) — the
     resize guard still gates them, so on the wasm32 fallback the big ones are simply refused (graceful),
     and on wasm64 they work.
   - WasmBridge range bound (`> 129` in `init`/`reset`) → raise to match (e.g. 257).

5. **32-bit boundary audit** — the JSC++ data path. Lattice `index()` is `int` (safe to L≤1290 — fine).
   The real items: verify Embind `typed_memory_view(count, ptr)` doesn't silently truncate `count` at
   uint32 for large arrays in `engine/wasm/ftd_wasm.cpp` samplers (`get_flux_volume`, `get_*_sampled`);
   and the uint32 particle-count header in `ws_server.cpp` (safe ≤256, only matters if the native cap is
   later raised).

6. **iOS/Safari safety net** — keep the runtime resize guard as the backstop: even when Memory64 is
   detected, iOS throttles WASM memory, so an over-budget resize is refused (reverts the dropdown) rather
   than crashing. Document the on-device iOS caveat.

### Critical files

- `engine/wasm/CMakeLists.txt` — `FTD_MEMORY64` option + flags
- `engine/build_wasm.bat` — dual build (`build_wasm/` + `build_wasm64/`) + deploy 4 artifacts
- `engine/web/js/bridge/wasm-bridge.js` — feature-detect loader + `isWasm64` + range bound
- `engine/web/js/scales/scale0/runtime/scenario-loader.js` — `maxWasmMemory` guard (branch on mode)
- `engine/web/js/scales/scale0/ui/toolbar/template.js` — larger odd size options
- `engine/wasm/ftd_wasm.cpp` — audit `typed_memory_view` count for large arrays

---

## Verification

- **Build:** both `build_wasm/` and `build_wasm64/` configure + link clean; 4 artifacts deployed.
- **Loader:** Chrome loads `ftd_core64` (`isWasm64 === true`); a wasm32-only browser loads `ftd_core`
  (`isWasm64 === false`). Confirm via `preview_eval` on `__ftdCtx.bridge.isWasm64`.
- **Big lattice:** on Chrome (wasm64), resize to **L=181 / 229** succeeds (heap grows to ~5–8 GB, no
  "Refusing to resize"); on the wasm32 fallback the same selection is refused at ~117 (graceful).
- **No regression:** Scale-0 Playwright suite green on the wasm64 build (toggle-coverage, scenario-parity,
  wasm-scenario-coverage, overlay-scheduler); existing overlays + odd-lattice centering unaffected. The
  C++ golden test is native (not WASM) so it's untouched.
- **Perf spot-check:** compare tick time wasm32 vs wasm64 at L=65 (expect ≤~5–10% overhead).
- **(If targeting mobile)** test a >1 GB lattice on a real iOS device.

## Risks / notes

- Dual build ≈ doubles WASM build time + deploy size (two `ftd_core*.{js,wasm}` sets).
- The Embind `typed_memory_view` uint32 question is the main unknown — test large-array samplers early.
- Memory64 is not on Safari/iOS yet; the wasm32 fallback (and the resize guard) keep those working.
- This plan does **not** touch the native `ws_server` path (still L≤256) or attempt the WebGPU rewrite.
