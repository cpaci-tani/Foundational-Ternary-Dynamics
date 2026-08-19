# SPEC — Scale 0 Bridge Layer Architecture

**Status:** foundation reference (descriptive — documents the system as built).
**Scope:** the **bridge layer** that the Scale-0 scenario subsystem and runtime pipeline sit on —
the live bridge implementations, the capability-factory pattern, the bridge contract + direct-read
surface, the Web-Worker proxy + SharedArrayBuffer design, bridge selection, and dispose/lifecycle.
**Companions:** [`SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](SPEC_SCALE0_SCENARIO_ARCHITECTURE.md)
(scenarios sit on these bridges), [`SPEC_SCALE0_RUNTIME_PIPELINE.md`](SPEC_SCALE0_RUNTIME_PIPELINE.md)
(the per-frame loop drives them). Point-in-time audits: [`audits/AUDIT_BRIDGE_WIRING_2026-06-03.md`](audits/AUDIT_BRIDGE_WIRING_2026-06-03.md)
(read-surface under the worker), [`AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md`](audits/AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md)
(lifecycle/teardown).

**Path convention:** JS paths relative to `engine/web/js/`; docs relative to `engine/web/docs/`.
Every claim carries a `file:line` — re-derive from source before relying on it (the discipline that
keeps these docs honest; counts/line numbers may drift).

> **Retired (do not reintroduce as the live Scale-0 path):** `MockBridge` /
> `mock-bridge.js` / `MockBridgeProxy` / `mock-bridge-proxy.js`. Scale-0 physics
> is owned by the WASM `RenderBridge` (`WasmBridge` in-thread or `WasmBridgeProxy`
> off-thread). The JS tree under `bridge/scenarios/` is a **parity mirror** of
> `engine/src/scenarios/*.cpp`, not the live seed path. See `bridge/README.md`.

---

## 1. The 30-second model

A **bridge** is the uniform object the whole dashboard talks to for physics: `ctx.bridge`. It owns
the lattice/particle state and exposes `setupScenario`, `tick`, samplers, diagnostics, and toggles.
There are **three live implementations** behind one symmetric surface, chosen by capability and
environment:

| Bridge | File | Role | Flags | Ready |
|---|---|---|---|---|
| **WebSocketBridge** | `ws-bridge.js` | Native C++ engine (auto-GPU on CUDA) over `ws://127.0.0.1:9100` — fastest when `ws_server.exe` is running | `isNativeGPU: true` | async (connect) |
| **WasmBridge** | `bridge/wasm-bridge.js` | Emscripten C++ engine in-browser — canonical in-thread path (prefers Memory64) | `isWasm: true` | async (`init`) |
| **WasmBridgeProxy** | `bridge/wasm-bridge-proxy.js` | Worker wrapper around `ftd_core_mt`; main thread reads flux zero-copy from the worker's SharedArrayBuffer heap | `isWasm: true`, `isWorker: true` | async (worker `ready`) |

All three present the **same surface** via two mechanisms: the **capability factory** (§3) for the
`bridge.capabilities.scaleN.*` namespace, and the **bridge contract** (§4) for the direct
read/mutate methods. The scenario subsystem and the runtime pipeline are written against that surface
and never branch on bridge type — *except* Scale-0 worker ownership (§6) and the worker read-surface (§5).

```
            ctx.bridge  (WebSocketBridge | WasmBridge)
                 │  capabilities.scale0.*   (factory — §3)
                 │  direct reads/mutates     (contract  — §4)
   ┌─────────────┴──────────────┐
WebSocketBridge            WasmBridge
  (native GPU)              (WASM C++)
                              │
                    optional ownership swap
                              │
                       WasmBridgeProxy
                    (Worker + SAB flux — §5)
```

Legacy store fields `fluxMock` / `useFluxMock` still name the **off-thread WASM owner**
(`WasmBridgeProxy`); they no longer mean a JS MockBridge.

---

## 2. The three live bridges

### 2.1 WebSocketBridge — native GPU (`ws-bridge.js`)

A drop-in bridge that forwards to a native C++ `RenderBridge` (auto-GPU on CUDA builds) running as
`ws_server.exe` on `ws://127.0.0.1:9100`. Probed **first** at boot when not on the static live-server
port (§6); if the socket connects it becomes `ctx.bridge`. Flags: `isNativeGPU: true`,
`isWasm: false`. It throttles high-frequency diagnostics queries. No explicit `dispose()` — the
socket closes on page unload. (Absent `ws_server.exe`, the connection fails fast and boot falls
through to WASM; the `ws://…:9100` connection error is known-benign console noise, filtered by
`tests/_helpers.js` `KNOWN_NOISE`.)

### 2.2 WasmBridge — Emscripten C++ (`bridge/wasm-bridge.js`)

The canonical in-browser path. `init(latticeSize)` loads the WASM module via a **singleton load
promise** (`_wasmLoadPromise`, prevents duplicate `<script>` injection) and sets `ready=true`. It
selects wasm32 vs wasm64 at load time (§7). Key surface:

- `setupScenario(name, harness)` → embind `setupScenario` → C++ `ftd::dispatch_scenario`
  (returns `bool`; unknown id fails — see `bindings_render_bridge.cpp`).
- `reset(latticeSize)` **destroys and re-allocates** the C++ `RenderBridge` at the same voxel count
  (drops PE/AE first) to bound peak memory.
- Sampler guards return frozen empty samples when a binder is absent so missing samplers degrade
  without per-call allocation. Heap reads use Emscripten `typed_memory_view` (memory-model-agnostic).
- Scale 1 (ParticleEngine) is delegated to C++; Scale 2 (AtomEngine) currently falls back to the JS
  `mock-atom-engine.js` when WASM AE is incomplete.

There is **no MockBridge fallback** on WASM init failure — boot throws with a rebuild hint
(`app.js` init path).

### 2.3 WasmBridgeProxy — Web Worker (`bridge/wasm-bridge-proxy.js`)

A main-thread proxy whose physics runs in a Web Worker (`bridge/wasm-bridge.worker.js` →
`ftd_core_mt`) so the heavy O(N³) tick never stalls render. The **default deployed Scale-0 path**
when COI + SharedArrayBuffer are available and the primary bridge is WASM. Flags:
`isWasm: true`, `isWorker: true`. Architecture in §5. Live counter
`window.__ftdWasmWorkers()` → `{live, created, terminated}` backs worker-conservation tests.

On init failure (`onInitFailure`) or a dead frame watchdog, the scenario loader falls back to
in-thread `WasmBridge` and latches `ctx._wasmWorkerDisabled`. Setup failures
(`setupScenario` → false) surface via `onSetupFailure` (toast), distinct from init fallback.

---

## 3. The capability-factory pattern

The live bridges expose **raw** methods on themselves (`bridge.tick()`, `bridge.getFluxVolume()`),
but those vary in signature/presence. The dashboard talks to a **symmetric** per-scale surface
instead: `bridge.capabilities.scale0.*`. This is installed once at module load by
`installCapabilityGetter(proto)` (`bridge/capabilities/install.js`), a lazy, cached getter
on each bridge prototype:

```js
Object.defineProperty(proto, 'capabilities', {
  configurable: true,
  get() {
    if (!this._capabilities) this._capabilities = {
      scale0: createScale0Capabilities(this),   // closes over the bridge instance
      scale1: createScale1Capabilities(this),
      scale2: createScale2Capabilities(this),
    };
    return this._capabilities;
  },
});
```

`createScale0Capabilities(bridge)` (`bridge/capabilities/scale0.js`) returns an object whose every
method closes over that bridge: `tickScale0`, `setupScenario`, `getScale0FieldSamples`,
`getScale0Diagnostics`, `setToggle`, `setBoundaryShape`, etc. (The snapshot/scrub capability pair
was removed — the simulation is forward-only; see `SPEC_SCALE0_RUNTIME_PIPELINE.md` §8.)

**Why the indirection:** (1) a guaranteed-present, uniform surface so callers never branch on bridge
type; (2) namespace isolation; (3) lazy + cached per instance. The factory is also installed on the
**worker's** WASM bridge surface, so the same `capabilities.scale0.tickScale0()` drives physics
off-thread (as a no-op on the proxy — the worker self-ticks). Contract: `CONTRACTS.md` §2.

---

## 4. The bridge contract + direct-read surface

`bridge/bridge-contract.js` is the documentation-and-anti-drift layer. The `ScaleBridge` typedef
lists every method live bridges must implement with matching shape — identity (`isWasm`,
`ready`, `latticeSize`), lifecycle (`reset`), scenarios (`setupScenario`), diagnostics, the particle
list, the field samplers, and the 2D slice. No class formally `implements` it (JS has no nominal
interfaces); the parity is by convention + the regression tests.

The load-bearing export is **`SCALE0_DIRECT_READS`** — the canonical list of read methods
consumers call **directly** on the bridge object (not via `capabilities.scale0.*`). Under the worker
proxy the bridge is a `WasmBridgeProxy`; it must serve every one of these from the SAB heap view
and/or the last `frame` payload, or the consumer silently blanks. The list is the single source of
truth, consumed by the proxy and the worker specs. Full treatment:
`audits/AUDIT_BRIDGE_WIRING_2026-06-03.md` (historical MockBridgeProxy wording in that audit is
superseded by this SPEC for the live path).

---

## 5. The worker-proxy architecture (SAB flux)

The worker path decouples the heavy tick from the render loop while keeping flux reads cheap.

**Shared memory** — the threaded WASM heap is a `SharedArrayBuffer`. The worker keeps
`getFluxVolume` caches fresh in shared memory each tick; the main-thread proxy returns a
zero-copy view. Small scalars (energies, diagnostics, particle frames) ride `postMessage`, not
shared memory. Requires cross-origin isolation (COOP/COEP via `serve.py`).

**Main thread (`WasmBridgeProxy`):**
- Construction posts `create` (N, scenarioId, toggles, boundary, pool size) and waits for `ready`.
- Mutators (`injectFlux`, `setToggle`, `setupScenario`, …) post `command` messages.
- **No MockBridge shadow** — flux reads come from the WASM heap view; audit / Lagrangian /
  particle list ride the periodic `frame` payload (`_lastAudit` / `_lastLagrangian` / …).
- Demand-gating (`_wantSampler` / telemetry want-mask) keeps expensive samplers off unless a
  visible consumer needs them — see [`SPEC_SCALE0_PERF_TELEMETRY_PANELS.md`](SPEC_SCALE0_PERF_TELEMETRY_PANELS.md).

**Worker (`bridge/wasm-bridge.worker.js`):** loads `ftd_core_mt`, constructs a `RenderBridge`,
publishes shared flux views, then self-ticks (~60 Hz, tick-time-limited at large L). Default
pthread pool is 1 (serial off-thread); `window.__ftdWasmWorkerPool` can raise it (Phase 2).

**Teardown:** `terminate()` decrements the live counter, posts `dispose`, and calls
`worker.terminate()` — idempotent. Scenario loader `setFluxMock(null, false)` disposes the prior
proxy before overwrite. Init-failure fallback clears the proxy and reloads on in-thread WASM.

---

## 6. Bridge selection — boot, scale switches, and Scale-0 worker ownership

**Boot probe order** (`app.js` `init`):
1. Optional native: `?engine=native` or non-live-server port → `tryNativeBridge` (WebSocketBridge).
2. Else / on native miss → `createBridge` → `WasmBridge.init` (**throws** on failure; no mock fallback).
3. Status chip reflects Native vs WASM.

**Scale switches** (`app.js` `switchEngineMode`): Scales 0–3 **reuse the single app-level
`ctx.bridge`**; Scales 4 (planetary) and 5 (cosmic) construct their own bridge inside their
`loadScenario` and may swap `ctx.bridge` (so callers must re-read `ctx.bridge` each frame —
`CONTRACTS.md` §3).

**Scale-0 worker ownership** — the one place Scale 0 may run on a **second** bridge object:
`wasmWorkerEligible(...)` in `scales/scale0/runtime/scenario-loader.js` requires
`FTD_WASM_WORKER` (default on; `window.__ftdWasmWorker = false` forces in-thread),
`SharedArrayBuffer`, `crossOriginIsolated`, and a primary WASM bridge. When eligible,
`loadScale0Scenario` constructs a `WasmBridgeProxy`, stores it in the legacy
`fluxMock` / `useFluxMock` slots, and routes ticks/reads through
`getActiveScale0Bridge(ctx, state)`. Native/WS primary bridges keep physics on `ctx.bridge`
(no WASM worker ownership). Resize rebuilds the proxy at the new lattice size.

---

## 7. wasm32 vs wasm64 (Memory64)

`WasmBridge` feature-detects Memory64 once per session (`supportsMemory64()`: construct a
`new WebAssembly.Memory({index:'i64'})` and catch). Supported → load `wasm/ftd_core64.js`
(`createFTDModule64`, 8 GB heap); unsupported (iOS/Safari, flagged Firefox) → `wasm/ftd_core.js`
(`createFTDModule`, 2 GB heap). `this.isWasm64` records the choice; exactly one build loads per
session (the `_wasmLoadPromise` singleton). The worker path uses `ftd_core_mt` (wasm32 + threads) —
Memory64 and pthreads are not combined in the current deploy. Heap reads go through
`typed_memory_view` (no manual JS pointer arithmetic). Heap cap feeds the resize guard.

---

## 8. Dispose / lifecycle

| Bridge | `reset()` | `dispose()` / teardown |
|---|---|---|
| **WebSocketBridge** | reconnect | implicit (socket closes on unload) |
| **WasmBridge** | destroy + re-allocate the C++ RenderBridge at the same N (drops PE/AE first) | `delete()` the C++ RenderBridge + ParticleEngine + AtomEngine, dispose the JS AE fallback, drop the lazy harness, `ready=false` — idempotent |
| **WasmBridgeProxy** | re-`create` on the worker | `terminate()`: decrement live counter, post `dispose`, `worker.terminate()` — idempotent |

Scenario switches reset via `setupScenario` inside the dispatcher; scale switches dispose
panels + the worker proxy via the lifecycle controller (`scales/scale0/controller.js` `mount`/`destroy`).
Lifecycle validity (no leaked rAF subscribers / GPU memory / workers across switches)
is verified end-to-end in `audits/AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md` and
worker teardown specs.

---

## 9. Quick reference — files

| Concern | File |
|---|---|
| Native GPU bridge | `ws-bridge.js` |
| WASM bridge (in-thread) | `bridge/wasm-bridge.js` |
| WASM worker proxy | `bridge/wasm-bridge-proxy.js` + `bridge/wasm-bridge.worker.js` |
| Capability factory | `bridge/capabilities/install.js` + `scale0.js`/`scale1.js`/`scale2.js` |
| Contract + direct-reads | `bridge/bridge-contract.js` |
| Construction / re-export shim | `bridge-init.js` → `bridge/bridge-factory.js` |
| Selection / boot probe | `app.js` (native → WASM); worker ownership: `scales/scale0/runtime/scenario-loader.js` |
| JS scenario parity mirror | `bridge/scenarios/` (not the live seed path) |
| Scale-2 JS MD fallback | `bridge/mock-atom-engine.js` |

*`file:line` references reflect the source at writing; re-derive before relying on them.*
