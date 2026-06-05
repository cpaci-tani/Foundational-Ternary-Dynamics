# SPEC — Scale 0 Bridge Layer Architecture

**Status:** foundation reference (descriptive — documents the system as built).
**Scope:** the **bridge layer** that the Scale-0 scenario subsystem and runtime pipeline sit on —
the four bridge implementations, the capability-factory pattern, the bridge contract + direct-read
surface, the Web-Worker proxy + SharedArrayBuffer design, bridge selection, and dispose/lifecycle.
**Companions:** [`SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](SPEC_SCALE0_SCENARIO_ARCHITECTURE.md)
(scenarios sit on these bridges), [`SPEC_SCALE0_RUNTIME_PIPELINE.md`](SPEC_SCALE0_RUNTIME_PIPELINE.md)
(the per-frame loop drives them). Point-in-time audits: [`audits/AUDIT_BRIDGE_WIRING_2026-06-03.md`](audits/AUDIT_BRIDGE_WIRING_2026-06-03.md)
(read-surface under the worker), [`AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md`](AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md)
(lifecycle/teardown). Worker design: [`PLAN_SCALE0_PHYSICS_WORKER.md`](PLAN_SCALE0_PHYSICS_WORKER.md).

**Path convention:** JS paths relative to `engine/web/js/`; docs relative to `engine/web/docs/`.
Every claim carries a `file:line` — re-derive from source before relying on it (the discipline that
keeps these docs honest; counts/line numbers are as of 2026-06-05).

---

## 1. The 30-second model

A **bridge** is the uniform object the whole dashboard talks to for physics: `ctx.bridge`. It owns
the lattice/particle state and exposes `setupScenario`, `tick`, samplers, diagnostics, and toggles.
There are **four implementations** behind one symmetric surface, chosen by capability and environment:

| Bridge | File | Role | `isWasm`/`isNativeGPU`/`isWorker` | Ready |
|---|---|---|---|---|
| **WebSocketBridge** | `ws-bridge.js` | Native C++ engine (auto-GPU on CUDA) over `ws://127.0.0.1:9100` — the fastest path when `ws_server.exe` is running | `isNativeGPU: true` | async (connect) |
| **WasmBridge** | `bridge/wasm-bridge.js` | Emscripten C++ engine in-browser — the canonical path | `isWasm: true` | async (`init`) |
| **MockBridge** | `bridge/mock-bridge.js` | Pure-JS reference lattice — offline/parity/fallback | (all false) | sync (immediate) |
| **MockBridgeProxy** | `bridge/mock-bridge-proxy.js` | Worker wrapper around a MockBridge; main thread reads its `SharedArrayBuffer`s zero-copy | `isWorker: true` | async (worker `ready`) |

All four present the **same surface** via two mechanisms: the **capability factory** (§3) for the
`bridge.capabilities.scaleN.*` namespace, and the **bridge contract** (§4) for the direct
read/mutate methods. The scenario subsystem and the runtime pipeline are written against that surface
and never branch on bridge type — *except* the two places that must: scenario bridge-ownership
(`shouldUseFluxMock`, §6) and the worker read-surface (§5).

```
            ctx.bridge  (one of four impls)
                 │  capabilities.scale0.*   (factory — §3)
                 │  direct reads/mutates     (contract  — §4)
   ┌─────────────┼───────────────┬───────────────────┐
WebSocketBridge  WasmBridge   MockBridge        MockBridgeProxy
  (native GPU)    (WASM C++)   (JS reference)   (Worker + SAB shadow — §5)
```

---

## 2. The four bridges

### 2.1 WebSocketBridge — native GPU (`ws-bridge.js`)

A drop-in bridge that forwards to a native C++ `RenderBridge` (auto-GPU on CUDA builds) running as
`ws_server.exe` on `ws://127.0.0.1:9100`. Probed **first** at boot (§6); if the socket connects it
becomes `ctx.bridge`. Flags: `isNativeGPU: true`, `isWasm: false`. It throttles high-frequency
diagnostics queries and lazily constructs a fallback `MockBridge` for any method the protocol doesn't
implement. No explicit `dispose()` — the socket closes on page unload. (Absent `ws_server.exe`, the
connection fails fast and boot falls through to WASM; the `ws://…:9100` connection error is
known-benign console noise, filtered by `tests/_helpers.js` `KNOWN_NOISE`.)

### 2.2 WasmBridge — Emscripten C++ (`bridge/wasm-bridge.js`)

The canonical in-browser path. `init(latticeSize)` (`:116-142`) loads the WASM module via a
**singleton load promise** (`_wasmLoadPromise`, prevents duplicate `<script>` injection) and sets
`ready=true`. It selects wasm32 vs wasm64 at load time (§7). Key surface:

- `setupScenario(name, harness)` (`:373-376`) → `this._module.setupScenario(this._bridge, name)`
  (the C++ `dispatch_scenario`).
- `reset(latticeSize)` (`:180-230`) **destroys and re-allocates** the C++ `RenderBridge` at the same
  voxel count (drops `_pe`/`_ae`/`_aeFallback` first) to bound peak memory.
- The **`_wasmCallOr` guard** (`:89-93`) wraps ~20 sampler methods that may be absent from the
  module, returning frozen `EMPTY_FIELD_SAMPLE`/`EMPTY_SCALAR_SAMPLE` singletons (`:68-83`) so a
  missing sampler degrades to empty without a per-call allocation. Every heap read returns an
  Emscripten `typed_memory_view` (memory-model-agnostic — see the wasm64 spot-check in the 06-03 audit).
- Scale 1 (ParticleEngine) is delegated to C++; Scale 2 (AtomEngine) is currently forced to a JS
  MockBridge fallback (`_aeHasWasm: false`, the Planck-unit conversion shim is unbuilt).

### 2.3 MockBridge — pure JS (`bridge/mock-bridge.js`)

The reference physics implementation: offline testing, JS↔C++ parity, and the Scale-2/3 fallback.
`ready` immediately (no async init); snaps even lattice sizes to odd. Internal state: `_tick`,
`_dt`, `_particles[]`, and the flux buffers `_fluxJ`/`_fluxWV` (Float64×3), `_fluxMag` (Float64),
`_stateGrid` (Int8) allocated by `_initFluxGrid` — backed by `SharedArrayBuffer`s when `_useSAB`
is set (the worker path, §5). It composes focused factories: lattice samplers, the diagnostics
provider (energy caching, `bridge/mock-diagnostics.js`), the ParticleEngine, the AtomEngine, and the
scenario dispatcher (`runSetupScenario`). `setupScenario(name, harness)` (`:1704`) dispatches to the
JS scenario library. Sparse-tick optimization (`_sparseTick`/`_activeBox`, `SPEC_SCALE0_LATTICE_PERF`
§3) restricts work to the active flux region, falling back to dense when the wave fills the box.

### 2.4 MockBridgeProxy — Web Worker (`bridge/mock-bridge-proxy.js`)

A main-thread proxy whose physics runs in a Web Worker so the heavy O(N³) tick never stalls render.
The **default deployed Scale-0 path** since 2026-06-03 (cross-origin-isolation enabled). Flags:
`isWorker: true`. Architecture in §5. A live counter `window.__ftdScale0Workers()` →
`{live, created, terminated}` backs `tests/scale0-worker-teardown.spec.js` (worker conservation).

---

## 3. The capability-factory pattern

The four bridges expose **raw** methods on themselves (`bridge.tick()`, `bridge.getFluxVolume()`),
but those vary in signature/presence. The dashboard talks to a **symmetric** per-scale surface
instead: `bridge.capabilities.scale0.*`. This is installed once at module load by
`installCapabilityGetter(proto)` (`bridge/capabilities/install.js:22-36`), a lazy, cached getter
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
method closes over that bridge: `tickScale0: () => bridge.tick()`, `setupScenario: (name) =>
bridge.setupScenario(name)` (`:56`), `getScale0FieldSamples({kind, stride})` (a dispatcher over
`e`/`b`/`poynting`/`divJ`/`vorticity`/`helicity`/`kretschmann`/`latency`/`fisher`/`coherence`/`curlJ`/
`state`/`gaussResidual`), `getScale0Diagnostics`, `getScale0EnergyAudit`, `getScale0Lagrangian`,
`setToggle`, `setBoundaryShape`, etc. (The snapshot/scrub capability pair was removed 2026-06-05 — the
simulation is forward-only; see `SPEC_SCALE0_RUNTIME_PIPELINE.md` §8.)

**Why the indirection:** (1) a guaranteed-present, uniform surface so callers never branch on bridge
type; (2) namespace isolation (raw methods stay on the instance, the scale surface lives under
`.capabilities.scaleN`); (3) lazy + cached per instance. The factory is also installed on the
**worker's** MockBridge prototype, so the same `capabilities.scale0.tickScale0()` drives physics
off-thread. Contract: `CONTRACTS.md` §2.

---

## 4. The bridge contract + direct-read surface

`bridge/bridge-contract.js` is the documentation-and-anti-drift layer. The `ScaleBridge` typedef
(`:19-58`) lists every method both bridges must implement with matching shape — identity (`isWasm`,
`ready`, `latticeSize`), lifecycle (`reset`), scenarios (`setupScenario`), diagnostics, the particle
list, the field samplers, and the 2D slice. No class formally `implements` it (JS has no nominal
interfaces); the parity is by convention + the regression tests.

The load-bearing export is **`SCALE0_DIRECT_READS`** (`:80-110`) — the canonical list of read methods
consumers call **directly** on the bridge object (not via `capabilities.scale0.*`). Under the worker
proxy the bridge is a `MockBridgeProxy`; it must forward every one of these to its shadow, or the
consumer silently blanks. The list is the single source of truth, consumed by both
`mock-bridge-proxy.js` (installs one shadow-delegating forwarder per name) and the worker spec. This
is the **reference example of a contract done right** — one named export wired into both the
implementation and its regression test, so adding a sampler to one place can't silently break charts.
(History: `inspectVoxel` was patched in one-at-a-time in `68024ba1` before this list existed.) Full
treatment: `audits/AUDIT_BRIDGE_WIRING_2026-06-03.md`.

---

## 5. The worker-proxy architecture (SAB shadow)

The worker path decouples the heavy tick from the render loop while keeping reads cheap.

**Shared memory** — `bridge/shared-field.js` is the single source of truth for the layout: one
`SharedArrayBuffer` per buffer (`fluxJ`/`fluxWV` Float64×3, `fluxMag` Float64, `state` Int8;
`FIELD_BYTES` `:10-15`) plus a small **Int32 control SAB** (`CTRL = {FRAME, N, TICK, RUNNING, PCOUNT,
LEN:8}` `:19`) carrying the frame counter and a few live integers via `Atomics`. `allocSharedField(N)`
allocates the set; `viewSharedField(sab)` builds typed-array views — **used identically on the worker
side and the main-thread shadow** (both see the same memory). Float64 scalars (energies) ride the
small per-frame `postMessage`, not shared memory. The module requires cross-origin isolation
(COOP/COEP via `serve.py`); callers gate on `crossOriginIsolated`.

**Main thread (`MockBridgeProxy`):**
- `setupScenario(name)` posts a `create` message (N, scenarioId, toggles, boundary) and flips
  `_ready=false`; mutators (`injectFlux`, `setToggle`, …) post `command` messages via `_cmd`.
- A **shadow `MockBridge`** has its flux/state buffers **repointed** to the worker's SABs, with
  `_sparseTick=false` (it never ticks on the main thread). Direct reads (`getFluxVolume`,
  `getFluxSlice`, `inspectVoxel`, the `SCALE0_DIRECT_READS` set) delegate to the shadow → zero-copy
  reads straight off the SABs. Before `ready` they return empty/null fallbacks.
- The **particle list** is the exception: the shadow owns no particles, so the worker ships
  `getScale0ParticleList()` in the periodic frame payload (every `PLIST_EVERY` frames) and the proxy
  serves the latest (`getScale0ParticleList` override; the field terms of audit/Lagrangian read live
  off the shadow but particle terms read zero — acceptable, see the 06-03 audit's Tier-1/Tier-2 split).

**Worker (`bridge/mock-bridge.worker.js`):** on `create`, builds a `MockBridge(N)` with `_useSAB=true`,
applies boundary/reflective/scenario, calls `publishShared(N)` (allocates the SAB set, posts `ready`
with the SABs), and starts a self-ticking `setTimeout` loop (~60 Hz, tick-time-limited at large L).
Each frame it updates `_fluxMag` (the O(N³) magnitude — off the main thread), atomically stores
`TICK`/`PCOUNT` and increments `FRAME`, and posts a small `frame` payload (`tick`, `diag`, `parts`,
and `particleList` every `PLIST_EVERY`).

**Teardown:** `terminate()` decrements the live counter, posts `dispose`, and calls
`worker.terminate()`; `dispose()` is an alias (idempotent, guarded against the double-call). The
worker's `dispose` clears its timer and disposes its MockBridge. `setFluxMock` disposes the prior
proxy before overwriting (`state/store.js:137-148`) — verified by `scale0-worker-teardown.spec.js`.

---

## 6. Bridge selection — boot, scale switches, and Scale-0 flux-mock ownership

**Boot probe order** (`app.js`, ~`:468-523`): `?engine=mock` forces MockBridge; otherwise try
**native** (`tryNativeBridge` → WebSocketBridge on `ws://127.0.0.1:9100`), then **WASM**
(`createBridge` → `WasmBridge.init`, falling back to MockBridge on failure), landing on whichever
succeeds as `ctx.bridge`. The status chip reflects the winner (Native/WASM/Mock · GPU/CPU).

**Scale switches** (`app.js` `switchEngineMode`): Scales 0–3 **reuse the single app-level
`ctx.bridge`**; Scales 4 (planetary) and 5 (cosmic) construct their own bridge inside their
`loadScenario` and may swap `ctx.bridge` (so callers must re-read `ctx.bridge` each frame — `CONTRACTS.md`
§3; the inspector is re-pointed at the main bridge on a switch back, audit P1-1).

**Scale-0 flux-mock ownership** — the one place Scale 0 may run on a **second** bridge:
`shouldUseFluxMock(bridge, scenarioName)` (`scales/scale0/runtime/scenario-loader.js:121-132`): native
GPU/WS → never; `flux-` prefix → always the JS mock; else probe `getFluxVolume()` and use the mock if
the active bridge can't serve flux. `workerEligible(...)` (`:140-145`) additionally requires
`FTD_PHYSICS_WORKER && SharedArrayBuffer && crossOriginIsolated`, and `makeFluxMock` (`:146-150`)
builds a `MockBridgeProxy` (worker) or in-thread `MockBridge` accordingly. When a flux-mock owns the
scenario, `state.fluxMock` ticks and `ctx.bridge` is idle for Scale 0 (`state.useFluxMock` selects
which one the runtime ticks/reads — see the runtime SPEC §2). The resize heap-guard estimates the
**owning** bridge's per-voxel cost (`scenario-loader.js:356-385`): mock ≈150 B/voxel (2 GB JS cap),
C++ ≈1300 B/voxel (8 GB wasm64 / 2 GB wasm32).

---

## 7. wasm32 vs wasm64 (Memory64)

`WasmBridge` feature-detects Memory64 once per session (`supportsMemory64()`, `:51-62`: construct a
`new WebAssembly.Memory({index:'i64'})` and catch). Supported → load `wasm/ftd_core64.js`
(`createFTDModule64`, 8 GB heap); unsupported (iOS/Safari, flagged Firefox) → `wasm/ftd_core.js`
(`createFTDModule`, 2 GB heap). `this.isWasm64` records the choice; exactly one build loads per session
(the `_wasmLoadPromise` singleton). The build sets `-sMEMORY64=1 -sWASM_BIGINT=1`
(`engine/wasm/CMakeLists.txt`); all heap reads go through `typed_memory_view` (no manual JS pointer
arithmetic), so the JS side is memory-model-agnostic (06-03 audit A5). Heap cap feeds the resize
guard (§6).

---

## 8. Dispose / lifecycle

| Bridge | `reset()` | `dispose()` |
|---|---|---|
| **WebSocketBridge** | reconnect | implicit (socket closes on unload) |
| **WasmBridge** (`:238-251`) | destroy + re-allocate the C++ RenderBridge at the same N (drops PE/AE first) | `delete()` the C++ RenderBridge + ParticleEngine + AtomEngine, dispose the JS AE fallback, drop the lazy harness, `ready=false` — idempotent |
| **MockBridge** | clear particles/fields, reset `_tick`, re-init flux buffers | typed arrays GC'd on dereference; engines cleared by the owning controller |
| **MockBridgeProxy** (`:176-182`) | re-`create` on the worker | `terminate()`: decrement live counter, post `dispose`, `worker.terminate()` — idempotent |

Scenario switches reset via `setupScenario`→`reset` inside the dispatcher; scale switches dispose
panels + the flux-mock via the lifecycle controller (`scales/scale0/controller.js` `mount`/`destroy`,
audit 06-04). Lifecycle validity (no leaked rAF subscribers / GPU memory / workers across switches)
is verified end-to-end in `AUDIT_CALLSTACK_LIFECYCLE_2026-06-04.md` (71/71) and
`scale0-worker-teardown.spec.js`.

---

## 9. Quick reference — files

| Concern | File |
|---|---|
| Native GPU bridge | `ws-bridge.js` |
| WASM bridge | `bridge/wasm-bridge.js` |
| JS reference bridge | `bridge/mock-bridge.js` (+ `bridge/mock-diagnostics.js`) |
| Worker proxy | `bridge/mock-bridge-proxy.js` + `bridge/mock-bridge.worker.js` + `bridge/shared-field.js` |
| Capability factory | `bridge/capabilities/install.js` + `scale0.js`/`scale1.js`/`scale2.js` |
| Contract + direct-reads | `bridge/bridge-contract.js` |
| Construction / re-export shim | `bridge/bridge-init.js` |
| Selection / boot probe | `app.js` (native → WASM → mock); flux-mock: `scales/scale0/runtime/scenario-loader.js:121-150` |

*`file:line` references are as of the 2026-06-05 source; re-derive before relying on them. The
capability-getter and SAB layout (§3, §5) were verified verbatim; the rest is mapped from source with
file:line and should be re-confirmed for exact line numbers.*
