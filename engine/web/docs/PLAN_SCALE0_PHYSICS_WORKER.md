# Scale-0 Physics Web Worker — Phase 2 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax.

**Goal:** Run the Scale-0 JS physics (`MockBridge` tick + `_fluxMag` + diagnostics) in a dedicated Web Worker that shares its field buffers with the main thread via `SharedArrayBuffer`, so render + UI stay at 60 FPS regardless of how long each tick takes — fixing the broad/default-scenario FPS drop that Phase 1 (sparse tick) does not.

**Architecture (verified feasible 2026-06-03):** The worker owns the authoritative `MockBridge` (buffers SAB-backed) and self-ticks on its own clock. Each frame it writes `_fluxJ`/`_fluxWV`/`_fluxMag`/`_stateGrid` into shared memory and posts a tiny "frame-ready" message (tick #, diagnostic scalars, particle snapshot). The main thread holds a `MockBridgeProxy` whose **reads** (all 17 samplers, getFluxVolume/Slice, getParticleData, diagnostics, snapshot) run on a **shadow** object that *views the same SAB memory* (the existing `mock-lattice-samplers.js` functions are buffer-pure, so they work unchanged on the shadow), and whose **commands** (inject/toggle/setup/reset/scrub-restore/params) `postMessage` to the worker. Everything is behind `FTD_PHYSICS_WORKER`, with automatic fallback to the in-thread `MockBridge` when the flag is off or the page is not `crossOriginIsolated` (Safari/iOS, or a deploy host without COOP/COEP).

**Tech Stack:** Vanilla ES-module JS + Web Worker (module worker) + SharedArrayBuffer/Atomics. Playwright (Chromium, served via `serve.py` for cross-origin isolation).

**Spec:** `engine/web/docs/SPEC_SCALE0_LATTICE_PERF.md` §4. **Surface map** (every synchronous fluxMock access, classified read/command): captured in the session; the load-bearing items are reproduced per-task below. **Builds on Phase 1** (`PLAN_SCALE0_SPARSE_TICK.md`) — the worker runs the sparse tick, so localized scenarios get both wins.

**Commit policy:** No AI-attribution trailers; run `git commit` steps only on the user's go-ahead; never `git add -A` (shared tree — stage explicit paths, verify `git diff --cached`).

**Verified prerequisites (already done this session):**
- `serve.py` sends `Cross-Origin-Opener-Policy: same-origin` + `Cross-Origin-Embedder-Policy: require-corp` + `Cross-Origin-Resource-Policy: same-origin`. Confirmed: `crossOriginIsolated === true`, `SharedArrayBuffer` constructs, and the wasm64 engine + fluxMock still load (no COEP breakage).

---

## File Structure

- **Create** `engine/web/js/bridge/mock-bridge.worker.js` — module worker hosting the authoritative `MockBridge`; command handler + self-tick loop + SAB allocation + frame-ready posts.
- **Create** `engine/web/js/bridge/mock-bridge-proxy.js` — `MockBridgeProxy` (main thread): shadow + worker handle; mirrors the `MockBridge` API the rest of Scale-0 calls; `capabilities.scale0` wired to it.
- **Create** `engine/web/js/bridge/shared-field.js` — small helper: SAB layout (sizes/offsets for fluxJ/fluxWV/fluxMag/stateGrid + an Atomics control header), allocate/attach views. Shared by worker + proxy so the layout is defined once.
- **Modify** `engine/web/js/bridge/mock-bridge.js` — `_useSAB` option: back `_fluxJ/_fluxWV/_fluxMag/_stateGrid` with SABs (from `shared-field.js`) when set; a `getSharedField()` accessor returning the SAB set.
- **Modify** `engine/web/js/scales/scale0/runtime/scenario-loader.js` — when `workerEligible()`, construct a `MockBridgeProxy` instead of `new MockBridge`; tear down the worker in the same places the mock is disposed.
- **Modify** `engine/web/js/scales/scale0/runtime/tick.js` — skip `mockScale0.tickScale0()` when the mock is worker-backed (the worker self-ticks); still bump `fieldDataVersion` from frame-ready.
- **Modify** `engine/web/js/scales/scale0/state/store.js` — `clearFluxMock`/`setFluxMock` call `proxy.terminate()` when present.
- **Modify** `engine/web/tests/playwright.config.js` — `webServer.command` → `python serve.py 8081` (cross-origin isolation in tests).
- **Create** `engine/web/tests/scale0-worker.spec.js` — worker lifecycle + field-parity + scrub spec.

The worker runs MockBridge unchanged except for SAB-backed buffers, so the physics stays bit-identical to the in-thread path.

---

## Task 1: SAB-backed buffers in MockBridge (flag-gated, bit-exact)

**Files:** Create `js/bridge/shared-field.js`; Modify `js/bridge/mock-bridge.js` (constructor, `_initFluxGrid`, `reset`, `_stateGrid` alloc); add to `tests/scale0-sparse-tick.spec.js`.

- [ ] **Step 1: Write `shared-field.js`** (the single source of SAB layout)

```javascript
// SharedArrayBuffer layout for the Scale-0 field, shared by the worker (writer)
// and the main-thread proxy (reader). One SAB per buffer keeps it simple; an
// Int32 control SAB carries the frame counter (Atomics) + live scalars.
export const FIELD_BYTES = {
    fluxJ:  (N) => N * N * N * 3 * 8,   // Float64 ×3
    fluxWV: (N) => N * N * N * 3 * 8,   // Float64 ×3
    fluxMag:(N) => N * N * N * 8,       // Float64
    state:  (N) => N * N * N,           // Int8
};
// Control SAB (Int32Array): [0]=frameCounter, [1]=latticeSize, [2]=tick,
// [3]=running(0/1), [4]=particleCount. Float64 scalars (energy, etc.) ride in
// the frame-ready postMessage (small, not perf-critical).
export const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, LEN: 8 };

export function allocSharedField(N) {
    return {
        N,
        ctrl:   new SharedArrayBuffer(CTRL.LEN * 4),
        fluxJ:  new SharedArrayBuffer(FIELD_BYTES.fluxJ(N)),
        fluxWV: new SharedArrayBuffer(FIELD_BYTES.fluxWV(N)),
        fluxMag:new SharedArrayBuffer(FIELD_BYTES.fluxMag(N)),
        state:  new SharedArrayBuffer(FIELD_BYTES.state(N)),
    };
}

// Build typed-array views over an existing SAB set (used on both sides).
export function viewSharedField(sab) {
    return {
        N: sab.N,
        ctrl:   new Int32Array(sab.ctrl),
        fluxJ:  new Float64Array(sab.fluxJ),
        fluxWV: new Float64Array(sab.fluxWV),
        fluxMag:new Float64Array(sab.fluxMag),
        state:  new Int8Array(sab.state),
    };
}
```

- [ ] **Step 2: Add `_useSAB` + `getSharedField()` to MockBridge** — in the constructor add `this._useSAB = false; this._sharedField = null;`. Add a method:

```javascript
    // When SAB-backed, return the SharedArrayBuffer set so a worker host can
    // post it to the main thread (zero-copy field sharing). Null when not SAB.
    getSharedField() { return this._sharedField; }
```

- [ ] **Step 3: Back the field buffers with SAB when `_useSAB`** — in `_initFluxGrid()` (mock-bridge.js:896) replace the `new Float64Array(total*3)` allocations so that, when `this._useSAB`, the three flux buffers (and `_stateGrid` on first use) are views over a `shared-field.js` SAB set stored in `this._sharedField`. Example for `_initFluxGrid`:

```javascript
    _initFluxGrid() {
        const N = this.latticeSize, total = N * N * N;
        if (this._useSAB) {
            const sab = allocSharedField(N);          // import from './shared-field.js'
            this._sharedField = sab;
            const v = viewSharedField(sab);
            this._fluxJ = v.fluxJ; this._fluxWV = v.fluxWV; this._fluxMag = v.fluxMag;
            v.ctrl[CTRL.N] = N;
        } else {
            this._fluxJ = new Float64Array(total * 3);
            this._fluxWV = new Float64Array(total * 3);
            this._fluxMag = new Float64Array(total);
        }
        this._fluxDirty = true;
    }
```

(Mirror the same `_useSAB` branch wherever `_stateGrid = new Int8Array(NNN)` is allocated — use the shared `state` view when SAB-backed. In `reset()`, when `_useSAB`, drop `_sharedField = null` so the next `_initFluxGrid` reallocates at the new size.)

- [ ] **Step 4: Add an equivalence test** to `tests/scale0-sparse-tick.spec.js`:

```javascript
    test('SAB-backed buffers tick identically to plain buffers', async ({ page }) => {
        await gotoAndReady(page);
        const r = await page.evaluate(async () => {
            const { MockBridge } = await import('/js/bridge/mock-bridge.js');
            const run = (useSAB) => {
                const b = new MockBridge(33); b._useSAB = useSAB; b._sparseTick = false;
                const c = 16; b.injectFlux(c, c, c, 0.5, 0.5, 0.5);
                const s0 = b.capabilities.scale0;
                for (let i = 0; i < 20; i++) s0.tickScale0();
                return Array.from(b._fluxJ);
            };
            const plain = run(false), sab = run(true);
            let m = 0; for (let i = 0; i < plain.length; i++) m = Math.max(m, Math.abs(plain[i] - sab[i]));
            return { maxDiff: m, coi: globalThis.crossOriginIsolated };
        });
        expect(r.coi, 'page must be cross-origin isolated for SAB').toBe(true);
        expect(r.maxDiff, 'SAB-backed tick is bit-identical').toBe(0);
    });
```

- [ ] **Step 5: Run** `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js -g "SAB-backed" --reporter=line` → PASS (`maxDiff === 0`). Then Step 6: commit (on go-ahead).

---

## Task 2: The worker — `mock-bridge.worker.js`

**Files:** Create `js/bridge/mock-bridge.worker.js`.

- [ ] **Step 1: Write the worker** — hosts a SAB-backed MockBridge, handles commands, self-ticks, posts frame-ready.

```javascript
// Module worker: authoritative Scale-0 physics. Owns a SAB-backed MockBridge,
// self-ticks on its own clock (decoupled from the main render loop), and signals
// each completed frame via Atomics + a tiny postMessage. See PLAN_SCALE0_PHYSICS_WORKER.md.
import { MockBridge } from './mock-bridge.js';
import { CTRL } from './shared-field.js';

let bridge = null, ctrl = null, timer = 0, targetDt = 1000 / 60;

function postFrame() {
    if (!bridge) return;
    bridge._updateFluxMag();                 // O(N³), now off the main thread
    const s0 = bridge.capabilities.scale0;
    const diag = s0.getScale0Diagnostics?.() ?? null;
    const parts = s0.getScale0ParticleFrame?.() ?? null;   // small; copy via postMessage
    if (ctrl) { Atomics.store(ctrl, CTRL.TICK, bridge._tick | 0);
                Atomics.add(ctrl, CTRL.FRAME, 1); }
    // Transfer the particle buffers (regenerated each frame) to avoid a copy.
    const transfer = parts ? [parts.positions.buffer, parts.colors.buffer, parts.sizes.buffer, parts.velocities.buffer] : [];
    self.postMessage({ type: 'frame', tick: bridge._tick, diag, parts }, transfer);
}

function loop() {
    if (!bridge) return;
    if (Atomics.load(ctrl, CTRL.RUNNING)) { bridge.capabilities.scale0.tickScale0(); postFrame(); }
    timer = setTimeout(loop, targetDt);
}

self.onmessage = (e) => {
    const m = e.data;
    switch (m.type) {
        case 'create': {                      // {N, scenarioId, toggles, params, boundary}
            bridge = new MockBridge(m.N); bridge._useSAB = true;
            bridge.capabilities.scale0.setupScenario(m.scenarioId);   // allocates SAB field
            const sab = bridge.getSharedField();
            ctrl = new Int32Array(sab.ctrl); Atomics.store(ctrl, CTRL.RUNNING, 1);
            self.postMessage({ type: 'ready', sab, N: m.N });
            if (!timer) loop();
            break;
        }
        case 'command': {                     // {method, args} — mutators forwarded verbatim
            const s0 = bridge?.capabilities?.scale0;
            if (s0 && typeof s0[m.method] === 'function') s0[m.method](...m.args);
            else if (bridge && typeof bridge[m.method] === 'function') bridge[m.method](...m.args);
            break;
        }
        case 'setRunning': Atomics.store(ctrl, CTRL.RUNNING, m.value ? 1 : 0); break;
        case 'resize': {                      // tear down + recreate at new N
            bridge.reset(m.N); bridge.capabilities.scale0.setupScenario(m.scenarioId);
            const sab = bridge.getSharedField(); ctrl = new Int32Array(sab.ctrl);
            self.postMessage({ type: 'ready', sab, N: m.N });
            break;
        }
        case 'dispose': { clearTimeout(timer); timer = 0; bridge?.dispose?.(); bridge = null; break; }
    }
};
```

> Note: `create`/`resize` re-post the SAB set; the main thread re-attaches views. Field writes go straight into shared memory (no per-frame field copy). Only the small particle/diag payloads ride postMessage.

- [ ] **Step 2: Smoke-test the worker** via a throwaway page eval (spawn it, send `create`, await `ready`, confirm a `frame` arrives with an advancing tick). Then Step 3: commit (on go-ahead).

---

## Task 3: The proxy + shadow — `mock-bridge-proxy.js`

**Files:** Create `js/bridge/mock-bridge-proxy.js`.

- [ ] **Step 1: Write `MockBridgeProxy`** — mirrors the `MockBridge` surface; reads run on the shadow, commands post to the worker. Reuse `mock-lattice-samplers.js` (buffer-pure) by passing the shadow as `state`.

```javascript
import { viewSharedField, CTRL } from './shared-field.js';
import * as Samplers from './mock-lattice-samplers.js';   // buffer-pure fns take `state`
import { createScale0Capabilities } from './capabilities/scale0.js';   // same factory MockBridge uses

export class MockBridgeProxy {
    constructor(latticeSize) {
        this.isWasm = false; this.isWorker = true;
        this.latticeSize = (latticeSize % 2 === 0) ? latticeSize + 1 : latticeSize;
        this._worker = new Worker(new URL('./mock-bridge.worker.js', import.meta.url), { type: 'module' });
        this._shadow = null;          // { _fluxJ, _fluxWV, _fluxMag, _stateGrid, latticeSize, _particles, _latencyProxy,... }
        this._ctrl = null; this._lastDiag = null; this._lastParts = null;
        this._toggles = { /* mirror defaults so command echoes are local-readable */ };
        this._params = {};
        this._worker.onmessage = (e) => this._onMessage(e.data);
        this.capabilities = createScale0Capabilities(this);   // wire capabilities.scale0 to THIS
    }
    _onMessage(m) {
        if (m.type === 'ready') {
            const v = viewSharedField(m.sab);
            this._shadow = { _fluxJ: v.fluxJ, _fluxWV: v.fluxWV, _fluxMag: v.fluxMag,
                             _stateGrid: v.state, latticeSize: m.N, _particles: [],
                             _latencyProxy: null, _latencyProxyTick: -1, _toggles: this._toggles, _params: this._params };
            this._ctrl = v.ctrl; this.latticeSize = m.N;
        } else if (m.type === 'frame') {
            this._lastDiag = m.diag; this._lastParts = m.parts;
            if (m.parts) this._shadow._particles = m.parts;   // particle frame as-is
        }
    }
    _cmd(method, ...args) { this._worker.postMessage({ type: 'command', method, args }); }

    // ── Reads (run on the shadow) ───────────────────────────────────────────
    getFluxVolume() { return this._shadow ? this._shadow._fluxMag : new Float64Array(0); }
    getEFieldSampled(stride) { return this._shadow ? Samplers.getEFieldSampled(this._shadow, stride) : EMPTY_VEC; }
    // … one line per sampler, all delegating Samplers.<fn>(this._shadow, stride) …
    getScale0Diagnostics() { return this._lastDiag; }
    getParticleData() { return this._lastParts ?? EMPTY_PARTS; }

    // ── Commands (post to worker) ───────────────────────────────────────────
    setupScenario(name) { this._worker.postMessage({ type: 'create', N: this.latticeSize, scenarioId: name,
                                                      toggles: this._toggles, params: this._params }); }
    injectFlux(...a) { this._cmd('injectFlux', ...a); }
    setToggle(k, v) { this._toggles[k] = v; this._cmd('setToggle', k, v); }
    // … setParam/setDt/clearField/seedRandomFlux/setBoundaryShape/setReflectiveBoundary → _cmd(...) …
    resize(N) { this.latticeSize = N; this._worker.postMessage({ type: 'resize', N, scenarioId: this._scenarioId }); }
    setRunning(v) { this._worker.postMessage({ type: 'setRunning', value: v }); }
    terminate() { this._worker.postMessage({ type: 'dispose' }); this._worker.terminate(); }
    dispose() { this.terminate(); }
}
```

> The proxy must expose the **same `capabilities.scale0` method names** the controller/samplers call. Inventory them from `capabilities/scale0.js` and ensure each is present (read → shadow, command → worker). The surface map enumerates all 17 samplers + getFluxVolume/Slice/getParticleData/diagnostics/snapshot/mutators — every one needs a line here.

- [ ] **Step 2: Diagnostics + scrubbing stubs** — `getScale0Diagnostics/EnergyAudit/Lagrangian` return the last worker-posted scalars (compute them in the worker `postFrame`). Snapshot capture (`getScale0Snapshot`) reads the shadow SABs (copy out). Restore (`loadScale0Snapshot`) posts the buffers + a pause. (Full scrubbing is Task 5.)

- [ ] **Step 3:** Unit-exercise the proxy in a page eval (construct, setupScenario, await a frame, call a sampler, assert nonzero). Then commit (on go-ahead).

---

## Task 4: Wire into the scenario loader (flag + fallback)

**Files:** Modify `runtime/scenario-loader.js`, `runtime/tick.js`, `state/store.js`.

- [ ] **Step 1: Eligibility helper** in scenario-loader.js:

```javascript
// Worker physics requires SharedArrayBuffer (cross-origin isolation) and only
// applies to the JS-owned (flux-*/s0-*) scenarios. Falls back to in-thread.
const FTD_PHYSICS_WORKER = true;   // master flag
export function workerEligible(scenarioId, bridge) {
    return FTD_PHYSICS_WORKER && globalThis.crossOriginIsolated === true
        && typeof SharedArrayBuffer !== 'undefined' && shouldUseFluxMock(bridge, scenarioId);
}
```

- [ ] **Step 2:** Where the fluxMock is created (`new MockBridge(...)` in `loadScale0Scenario` and `resizeScale0Lattice`), branch: `fluxMock = workerEligible(scenarioId, bridge) ? new MockBridgeProxy(size) : new MockBridge(size);` then call the same `setupScenario`/`setToggle`/boundary commands (they're API-compatible). Track `state.fluxMockIsWorker`.

- [ ] **Step 3:** In `tick.js`, skip `mockScale0.tickScale0()` when `state.fluxMockIsWorker` (the worker self-ticks); drive `fieldDataVersion` from the proxy's frame counter (`proxy._ctrl[CTRL.FRAME]`) instead, and forward `running`/`scenarioRunning` to the worker via `setRunning`.

- [ ] **Step 4:** In `store.js`, `setFluxMock`/`clearFluxMock` call `prev.terminate?.()` so the worker is torn down with the proxy.

- [ ] **Step 5:** Browser-verify in the live preview: flux-pulse at L=129, confirm `state.fluxMock.isWorker === true`, the field updates (frame counter advances), and **the main-thread `animate` no longer pays the 89 ms tick** (re-profile: `advanceSimulation` ≈ 0; render stays smooth). Commit (on go-ahead).

---

## Task 5: Scrubbing / timeline seam

**Files:** Modify `mock-bridge-proxy.js`, `controller.js` (hydrate/resume), `timeline/memory-recorder.js` if needed.

- [ ] **Step 1:** Snapshot capture — `getScale0Snapshot` on the proxy reads the shadow SAB buffers and returns copies (`new Float32Array(shadow._fluxJ)` etc.) + `_lastParts` + `_lastDiag`. The recorder already calls `getScale0Snapshot`; no recorder change if the proxy implements it.
- [ ] **Step 2:** Restore — `loadScale0Snapshot(snap)` posts a `setRunning(false)` then `command setScale0FluxBuffer/WaveBuffer/LatticeBuffer/Tick/ParticleList` to the worker (the worker writes them into its SAB; the shadow sees them on the next frame or via an immediate `frame` post). `hydrateToTick` (controller.js:105) works unchanged through the proxy; `resumeLive` posts `setRunning(true)`.
- [ ] **Step 3:** Test scrub→resume in the live preview on a worker-backed scenario (scrub back, field freezes at the snapshot; resume, ticking continues). Commit (on go-ahead).

---

## Task 6: Playwright cross-origin isolation + worker spec + flag on

**Files:** Modify `tests/playwright.config.js`; create `tests/scale0-worker.spec.js`.

- [ ] **Step 1:** The worker tests need cross-origin isolation (SAB) **and** caching. ⚠ Observed 2026-06-03: pointing `webServer.command` at the existing `serve.py` (COOP/COEP **but no-cache**) makes Playwright's per-test fresh page loads re-fetch+recompile the large wasm64 binary, so `gotoAndReady`'s `window._ftdBridge` wait times out (resize-guard + flux-slice went from green → 25s timeout). Fix: add a **caching COOP/COEP test server** — either (a) a `--cache` flag on `serve.py` that omits the `no-store`/`Pragma`/`Expires` headers while keeping COOP/COEP/CORP, then set `command: 'python serve.py 8081 --cache'`; or (b) a small dedicated test server. Confirm `crossOriginIsolated === true` in a test AND that `gotoAndReady` stays well under timeout. (The dev preview keeps the no-cache `serve.py` — only Playwright's many fresh loads need caching.)
- [ ] **Step 2:** Write `scale0-worker.spec.js`: (a) on a flux scenario the proxy is used (`__ftdState.fluxMock.isWorker === true`); (b) the worker frame counter advances over ~1 s (physics runs); (c) the shadow flux field is populated; (d) field **parity**: a short worker run vs an in-thread `MockBridge` run from the same seed match within tolerance (the worker self-ticks on wall-clock, so compare field *structure*/energy, not bit-exact tick counts); (e) resize + scrub + mode-switch leave no leaked worker (`__ftdRAF` + a worker-count probe).
- [ ] **Step 3:** Run the full Scale-0 suite under `serve.py` (toggle-coverage, scenario-parity, wasm-scenario-coverage, flux-slice-axes, scale0-resize-guard, scale0-sparse-tick, scale0-worker) → all green. Flip nothing else; `FTD_PHYSICS_WORKER` stays on for SAB-capable browsers, off elsewhere via `workerEligible`. Commit (on go-ahead).

---

## Self-Review

- **Spec coverage (§4):** worker owns tick+fluxMag+diagnostics (Task 2); proxy+shadow, samplers reuse (Task 3); SAB transport + layout (Task 1, shared-field.js); command/frame protocol (Task 2/3); scrubbing seam (Task 5); COOP/COEP + fallback (prereq + Task 4 `workerEligible`); Playwright headers (Task 6). All covered.
- **Placeholder note:** Task 3's proxy lists "one line per sampler" — when executing, expand every sampler/capability method from `capabilities/scale0.js` explicitly (no `…`). This is the one spot to fully enumerate at build time.
- **Type/name consistency:** `getSharedField`, `_useSAB`, `_sharedField`, `allocSharedField`/`viewSharedField`, `CTRL`, `MockBridgeProxy`, `workerEligible`, `state.fluxMockIsWorker`, `isWorker`, `terminate()` used consistently.
- **Risk/rollback:** `FTD_PHYSICS_WORKER` off → in-thread MockBridge (current behavior). Non-isolated host → `workerEligible` false → in-thread. Worker is additive (new files); MockBridge change is one `_useSAB` branch (off by default).

## Risks / open items

- **Deploy host must send COOP/COEP** or `workerEligible` returns false there (graceful in-thread fallback — no breakage, no worker speedup). Document in the dashboard README.
- **Tearing:** the main thread may read a half-written frame (worker writes in place). For a fluid field this is visually invisible; if a glitch appears, add a double-buffer + Atomics frame-index swap in `shared-field.js` (layout already has a control header).
- **Particle scenarios:** particle physics runs in the worker; `getParticleData`/state overlay read the posted particle frame. Verify genesis/annihilation scenarios visually under the worker.
- **iOS/Safari:** no SAB → in-thread fallback (Phase 1 sparse tick still applies). Expected.
- **COEP + future CDN assets:** any cross-origin asset added later must send CORP/CORS or it will be blocked. Keep dashboard assets same-origin.
