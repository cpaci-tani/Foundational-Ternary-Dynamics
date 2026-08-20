// Main-thread proxy for the Scale-0 WASM physics Web Worker. Presents the same
// surface the rest of Scale-0 calls on a bridge, but:
//   • READS of the flux volume return a zero-copy view over the worker's
//     -pthread heap (a SharedArrayBuffer); the worker keeps the getFluxVolume
//     cache fresh in shared memory each tick, so the main thread never ticks
//     or recomputes anything.
//   • diagnostics + particle frame ride postMessage (small, worker-computed).
//   • COMMANDS (inject/toggle/setup/scenario/dt) postMessage to the worker.
// Mirrors mock-bridge-proxy.js, but there is NO MockBridge "shadow": the WASM
// field is C++-laid-out in the WASM heap, so reads are served directly from the
// heap view + the last frame payload. See project memory 2026-06-16.
//
// The hosted module is ftd_core_mt (threaded build) run at pool=1 (pure serial,
// off the main thread — Phase 1). Phase 2 raises the worker pool for the 1.8-2.2x
// threading once on-demand nested pthread_create is resolved.

import { createScale0Capabilities } from './capabilities/scale0.js';
import { samplerOr, particleDataToList } from './bridge-contract.js';

const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, TICKS_PER_FRAME: 5, LEN: 8 };
const EMPTY_PARTS = () => ({ positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), spin: new Float32Array(0), colorCharge: new Float32Array(0), count: 0 });
const EMPTY_VEC = () => ({ positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 });
const EMPTY_VAL = () => ({ positions: new Float32Array(0), values: new Float32Array(0), count: 0 });

// Complete boolean TermToggles registry, in the same order as
// engine/include/ftd/term_toggles.h::TOGGLE_SPECS.  The worker must read back
// every canonical name after C++ setupScenario(), not merely names that the JS
// UI happened to set before construction: scenario helpers legitimately turn
// on research terms such as `langevin` that are absent from SCALE0_TOGGLES.
// scenario-parity.spec.js pins this array exactly to TOGGLE_SPECS so a future
// engine toggle addition cannot silently disappear from worker truth.
export const SCALE0_ENGINE_TOGGLE_NAMES = Object.freeze([
    'wave_propagation', 'coupling', 'damping', 'genesis', 'evaporation',
    'gauss_projection', 'forces', 'gravity', 'poisson_coulomb', 'movement',
    'lorentz_force', 'selective_damping', 'larmor_radiation', 'dual_substrate',
    'color_forces', 'strong_stress_energy', 'weak_transmutation', 'strong_force',
    'triad_binding', 'pair_production', 'exchange_force', 'latency_field',
    'exact_dual_gauss', 'matched_gauss_dynamics', 'emergent_forces', 'langevin',
    'symplectic_leapfrog', 'verlet_wave_integrator', 'lorentz_period2_floquet',
    'lorentz_bcc_time_floquet', 'su2_gauge', 'su3_gauge',
    'symmetric_movement_order', 'absorbing_boundary', 'reflective_boundary',
    'field_energy_gravity', 'cluster_inertia', 'geometric_gravity', 'de_broglie_clock',
    'db_clock_coulomb', 'confinement', 'knot_tracking', 'strict_validation',
    'ew_background_sweep',
]);

// Worker thread-pool size. Default 1 (Phase 1: serial off-thread — guaranteed
// safe). Set window.__ftdWasmWorkerPool = N to enable the in-worker threading
// (Phase 2; the worker spawns N-1 nested pthread workers on demand).
const DEFAULT_WORKER_POOL = 1;
function workerPoolSize() {
    if (typeof window !== 'undefined' && typeof window.__ftdWasmWorkerPool === 'number') {
        return Math.max(1, window.__ftdWasmWorkerPool | 0);
    }
    return DEFAULT_WORKER_POOL;
}

// Live-instance accounting (lifecycle test parity with the mock proxy).
let _live = 0, _created = 0, _terminated = 0;
if (typeof window !== 'undefined') {
    window.__ftdWasmWorkers = () => ({ live: _live, created: _created, terminated: _terminated });
}

// How long to wait for the worker's 'ready' reply before declaring the
// off-thread WASM engine dead and falling back to the in-thread bridge. The
// engine module compiles + a RenderBridge is constructed inside this window; a
// few hundred ms is generous on a warm cache, so 8 s is a comfortable ceiling
// that still fails fast if importScripts silently NetworkErrors (the failure
// this guards — see scenario-loader's onInitFailure wiring).
const WORKER_READY_TIMEOUT_MS = 8000;

// How long a *ready* worker may go without posting a single 'frame' while the
// simulation is supposed to be running before we declare it "ready but dead"
// and fall back to the in-thread bridge. This guards the case where the worker
// reports 'ready' (so the ready-timeout above is cleared) but its self-tick
// loop never produces a frame — e.g. the in-worker threaded tick can't run
// because SharedArrayBuffer / cross-origin-isolation isn't available to the
// worker. 4 s is generous so a slow-but-alive worker isn't false-tripped; the
// _initFailed latch makes the fallback fire at most once (no flapping).
const FRAME_WATCHDOG_MS = 4000;

export class WasmBridgeProxy {
    // `opts.onInitFailure(reason)` (optional) is invoked AT MOST ONCE if the
    // worker fails to initialise — either the worker fires onerror before it
    // ever reports 'ready', or 'ready' never arrives within
    // WORKER_READY_TIMEOUT_MS. It lets the caller fall back to the in-thread
    // WasmBridge so the engine never silently dies when the off-thread worker
    // can't load (e.g. importScripts NetworkError on the -pthread MT glue).
    constructor(latticeSize, opts = {}) {
        _live++; _created++;
        this.isWasm = true;
        this.isWorker = true;
        this._terminated = false;
        this._onInitFailure = (opts && typeof opts.onInitFailure === 'function') ? opts.onInitFailure : null;
        this._initFailed = false;       // latched — fallback fires once
        // `opts.onSetupFailure(msg)` fires when the worker reports setupScenario
        // returned false or threw (unknown id / embind error). Distinct from
        // onInitFailure (module/worker load dead → in-thread fallback).
        this._onSetupFailure = (opts && typeof opts.onSetupFailure === 'function') ? opts.onSetupFailure : null;
        // `opts.onEngineToggles()` (optional) fires whenever the worker publishes a
        // fresh engine-truth toggle readback — i.e. after the C++ scenario body has
        // replaced the profile the main thread sent. The UI uses it to repaint the
        // physics-toggles card and recompute overlay applicability from what the
        // engine is ACTUALLY running rather than from the JS model of it.
        this._onEngineToggles = (opts && typeof opts.onEngineToggles === 'function') ? opts.onEngineToggles : null;
        this.latticeSize = (latticeSize % 2 === 0) ? latticeSize + 1 : latticeSize;
        this._scenarioId = 'flux-pulse';
        this._toggles = {};
        this._engineToggles = null;     // null until the worker's first readback
        this._wantAudit = true;         // telemetry demand mask (mirrors worker)
        this._wantLag = true;
        this._ready = false;
        this._running = null;
        this._readyAt = 0;              // performance.now() when 'ready' arrived
        this._lastFrameAt = 0;          // performance.now() of the last 'frame'
        this._frameWatchdog = null;     // setTimeout handle for the dead-worker watchdog
        this._ctrl = null;
        this._fluxView = null;
        this._lastDiag = null;
        this._lastParts = null;
        this._lastAudit = null;
        this._lastLag = null;
        this._lastKnot = null;
        this._lastKnotEvents = null;
        this._lastKnotAgg = null;
        this._pendingTPF = undefined;
        // Commands sent before 'ready' (i.e. while the worker is initialising the
        // WASM module) are buffered here and replayed as a single batchCommand once
        // the worker signals ready.  This is the mechanism that lets JS-side seeders
        // like seedSpectrumComparator work on the worker path even though WASM is not
        // synchronously available.
        this._pendingCommands = [];
        // Overlay sampler cache: keyed by "kind@stride". _samplerWant tracks which
        // kinds have been registered with the worker (idempotent per key).
        this._samplerCache = {};
        this._samplerWant = new Map();
        // One-shot coarsen (Scale-0 → Scale-1) request/response bookkeeping.
        this._coarsenPending = new Map();
        this._coarsenReq = 0;

        // CLASSIC worker (Emscripten module via importScripts). No { type: 'module' }.
        this._worker = new Worker(new URL('./wasm-bridge.worker.js', import.meta.url));
        this._worker.onmessage = (e) => this._onMessage(e.data);
        // A worker-level error (e.g. importScripts NetworkError loading the
        // -pthread MT glue) surfaces here. If it happens before the worker has
        // ever signalled 'ready', the off-thread engine is dead — trigger the
        // fallback so the app can switch to the in-thread bridge.
        this._worker.onerror = (e) => {
            console.error('[WasmWorker]', e.message || e);
            if (!this._ready) this._triggerFallback('worker onerror: ' + (e.message || 'load failed'));
        };
        // Guard against a silent never-ready worker (importScripts can fail in
        // ways that don't always reach onerror in every browser). If 'ready'
        // hasn't arrived in time, fall back.
        this._readyTimer = (typeof setTimeout === 'function')
            ? setTimeout(() => { if (!this._ready) this._triggerFallback('ready timeout'); }, WORKER_READY_TIMEOUT_MS)
            : null;

        this.capabilities = { scale0: this._buildCaps() };
    }

    /**
     * Latched, fire-once worker-fallback path. Cancels all timers, tears down
     * the (dead or stalled) worker, and notifies the caller (onInitFailure) so
     * it can fall back to the in-thread WasmBridge. Never throws.
     *
     * The ONLY latch is `_initFailed` — deliberately NOT gated on `_ready`, so
     * the frame-watchdog can fire fallback even for a worker that reported
     * 'ready' but then never produced a frame ("ready but dead"). The pre-ready
     * call sites (onerror, ready-timeout, worker init-error) gate on `!_ready`
     * themselves, so their behaviour is unchanged.
     */
    _triggerFallback(reason) {
        if (this._initFailed) return;
        this._initFailed = true;
        if (this._readyTimer) { try { clearTimeout(this._readyTimer); } catch { /* ignore */ } this._readyTimer = null; }
        this._clearFrameWatchdog();
        console.error('[WasmWorker] off-thread engine failed (' + reason + '); falling back to in-thread WASM engine.');
        // Capture before terminate() nulls callbacks.
        const cb = this._onInitFailure;
        this._onInitFailure = null;
        try { this.terminate(); } catch { /* ignore */ }
        if (cb) { try { cb(reason); } catch (e) { console.error('[WasmWorker] onInitFailure handler threw:', e); } }
    }

    /** Now-clock helper (performance.now where available, Date.now otherwise). */
    _now() { return (typeof performance !== 'undefined' && performance.now) ? performance.now() : Date.now(); }

    /**
     * Arm/re-arm the dead-worker frame watchdog. Only meaningful once ready and
     * running; a no-op otherwise. Each call replaces any pending timer, so it
     * doubles as the per-frame reset. If FRAME_WATCHDOG_MS elapses while the
     * worker is still ready+running and no newer frame has arrived, fall back.
     */
    _armFrameWatchdog() {
        if (typeof setTimeout !== 'function') return;
        if (!this._ready || this._running !== true || this._initFailed) return;
        this._clearFrameWatchdog();
        this._frameWatchdog = setTimeout(() => {
            this._frameWatchdog = null;
            if (this._initFailed || !this._ready || this._running !== true) return;
            const last = this._lastFrameAt || this._readyAt || 0;
            if (this._now() - last > FRAME_WATCHDOG_MS) {
                this._triggerFallback('ready worker produced no frames');
            } else {
                // A frame landed close to the deadline; re-arm for the remainder.
                this._armFrameWatchdog();
            }
        }, FRAME_WATCHDOG_MS);
    }

    _clearFrameWatchdog() {
        if (this._frameWatchdog) { try { clearTimeout(this._frameWatchdog); } catch { /* ignore */ } this._frameWatchdog = null; }
    }

    _onMessage(m) {
        // Ignore in-flight messages after terminate()/dispose(). Scenario churn
        // tears down the prior proxy while a 'ready'/'frame' may already be
        // queued on the main-thread event loop; without this guard those would
        // still mutate UI / postFrame callbacks for a dead owner.
        if (this._terminated || this._initFailed) return;
        if (m.type === 'ready') {
            this.latticeSize = m.N;
            this._ctrl = new Int32Array(m.ctrl);
            this._fluxView = new Float64Array(m.heap, m.fluxPtr, m.fluxLen);
            this._ready = true;
            this._readyAt = this._now();
            // The off-thread engine reported ready; cancel the never-ready watchdog.
            if (this._readyTimer) { try { clearTimeout(this._readyTimer); } catch { /* ignore */ } this._readyTimer = null; }
            // ...but a ready worker can still be dead (never posts a frame). If the
            // sim is already running, arm the frame-watchdog now; otherwise it arms
            // when setRunning(true) is next forwarded from the tick loop.
            this._armFrameWatchdog();
            if (this._pendingTPF !== undefined) {
                Atomics.store(this._ctrl, CTRL.TICKS_PER_FRAME, Math.round(this._pendingTPF * 1000));
            }
            // Replay commands that arrived while the worker was initialising.
            // Sent as a single batchCommand so the worker calls postFrame() only once
            // at the end rather than once per individual seed voxel.
            if (this._pendingCommands.length > 0) {
                this._worker.postMessage({ type: 'batchCommand', commands: this._pendingCommands });
                this._pendingCommands = [];
            }
            if (m.setupOk === false) {
                const msg = m.setupError || (`Unknown or unhandled scenario: ${this._scenarioId}`);
                console.error('[WasmWorker] setupScenario failed:', msg);
                try { this._onSetupFailure?.(msg); } catch (e) {
                    console.error('[WasmWorker] onSetupFailure handler threw:', e);
                }
            }
        } else if (m.type === 'frame') {
            // Reset the dead-worker watchdog only on TICK PROGRESS. A worker can
            // post frames (e.g. the initial scenario frame) while its tick stays
            // stuck at 0 — the in-worker threaded engine reports ready but never
            // advances. Resetting on every frame would mask that; resetting only
            // when diag.tick changes lets the watchdog catch a ready-but-not-
            // ticking worker and fall back to the in-thread engine.
            const _tk = (m.diag && typeof m.diag.tick === 'number') ? m.diag.tick : null;
            if (_tk !== null && _tk !== this._lastTick) {
                this._lastTick = _tk;
                this._lastFrameAt = this._now();
                this._armFrameWatchdog();
            }
            this._lastDiag = m.diag;
            if (m.parts) this._lastParts = m.parts;
            if (m.audit) this._lastAudit = m.audit;
            if (m.lag)   this._lastLag   = m.lag;
            if (m.engineToggles) {
                this._engineToggles = m.engineToggles;
                try { this._onEngineToggles?.(m.engineToggles); } catch (e) { /* UI callback must not kill the frame */ }
            }
            if (m.knot) this._lastKnot = m.knot;
            if (m.knotEvents) this._lastKnotEvents = m.knotEvents;
            if (m.knotAgg) this._lastKnotAgg = m.knotAgg;
            const hadSamplers = Boolean(m.samplers && Object.keys(m.samplers).length);
            if (hadSamplers) Object.assign(this._samplerCache, m.samplers);

            if (typeof window !== 'undefined' && window.__ftdCtx && typeof window.__ftdCtx.onBridgePostFrame === 'function') {
                window.__ftdCtx.onBridgePostFrame(hadSamplers);
            }
        } else if (m.type === 'coarsenResult') {
            const resolve = this._coarsenPending.get(m.reqId);
            if (resolve) {
                this._coarsenPending.delete(m.reqId);
                resolve(m.data ?? null);
            }
        } else if (m.type === 'error') {
            console.error('[WasmWorker]', m.where, m.msg);
            // A module-init failure inside the worker (createFTDModuleMT().catch
            // posts where:'init') means the engine never built — fall back if we
            // haven't yet become ready.
            if (m.where === 'init' && !this._ready) this._triggerFallback('worker init error: ' + (m.msg || ''));
            if (m.where === 'setupScenario') {
                try { this._onSetupFailure?.(m.msg || 'setupScenario failed'); } catch (e) {
                    console.error('[WasmWorker] onSetupFailure handler threw:', e);
                }
            }
        }
    }

    _cmd(method, ...args) {
        if (!this._ready) {
            // Buffer the command; will be replayed as batchCommand after 'ready'.
            this._pendingCommands.push({ method, args });
            return;
        }
        this._worker.postMessage({ type: 'command', method, args });
    }

    /** Monotonic frame counter from the worker (shared) — drives render refresh in tick.js. */
    get frameCounter() { return this._ctrl ? Atomics.load(this._ctrl, CTRL.FRAME) : 0; }
    get ready() { return this._ready; }
    currentTick() { return this._ctrl ? Atomics.load(this._ctrl, CTRL.TICK) : 0; }
    /** True once the worker has published a real engine-state readback. */
    get hasEngineToggles() { return this._engineToggles !== null; }

    // Engine truth first; the local write-cache is only a pre-first-frame stand-in.
    // Unknown names default OFF — never ON. Prefer getEngineTruthToggle when
    // the caller must distinguish "engine said false" from "no readback yet".
    getToggle(name) {
        if (this._engineToggles && name in this._engineToggles) return !!this._engineToggles[name];
        return this._toggles[name] ?? false;
    }

    /**
     * Engine-truth-only toggle readback: true/false when the worker has
     * published a real engine-state readback covering `name`, else null.
     * Unlike getToggle there is NO optimistic default — callers that must
     * not act on a guess (e.g. the promotion pipeline deciding whether to
     * enable/restore knot_tracking) use this.
     */
    getEngineTruthToggle(name) {
        if (this._engineToggles && name in this._engineToggles) return !!this._engineToggles[name];
        return null;
    }

    _buildCaps() {
        // The capability factory just delegates to bridge methods, so wrap `this`.
        const caps = createScale0Capabilities(this);
        caps.tickScale0 = () => {};                                   // worker self-ticks
        caps.setupScenario = (name) => this.setupScenario(name);
        caps.setToggle = (k, v) => { this._toggles[k] = v; this._cmd('setToggle', k, v); };
        caps.getScale0Diagnostics = () => this._lastDiag ?? null;
        caps.getScale0ParticleFrame = () => this._lastParts ?? EMPTY_PARTS();
        caps.getScale0EnergyAudit = () => this._lastAudit ?? null;
        caps.getScale0Lagrangian = () => this._lastLag ?? null;
        caps.getScale0KnotTelemetry = () => this._lastKnot ?? null;
        caps.getScale0KnotEvents = () => this._lastKnotEvents ?? null;
        caps.getScale0KnotAggregate = () => this._lastKnotAgg ?? null;
        // Real single-step for the promotion pipeline: unlike tickScale0
        // (deliberate no-op — the worker self-ticks), this forwards one
        // explicit tick command; the worker ticks once and posts a frame,
        // refreshing the knot-telemetry snapshot even while paused.
        caps.stepScale0 = () => this._cmd('tickScale0');
        return caps;
    }

    /**
     * One-shot Scale-0 → Scale-1 coarse-graining snapshot from the worker
     * engine. Resolves with the coarsenToParticles typed-array bundle, or
     * null if the worker/module can't serve it within the timeout.
     */
    coarsenToParticles(timeoutMs = 2000) {
        if (!this._ready) return Promise.resolve(null);
        const reqId = ++this._coarsenReq;
        return new Promise((resolve) => {
            this._coarsenPending.set(reqId, resolve);
            this._worker.postMessage({ type: 'coarsen', reqId });
            setTimeout(() => {
                if (this._coarsenPending.has(reqId)) {
                    this._coarsenPending.delete(reqId);
                    resolve(null);
                }
            }, timeoutMs);
        });
    }

    // ── Bridge reads the capability factory calls ───────────────────────────
    tick() {}                                                        // no-op; worker self-ticks
    getParticleData() { return this._lastParts ?? EMPTY_PARTS(); }

    // --- SCALE0_DIRECT_READS members that were missing from this class ---
    //
    // The proxy implemented 23 of the 26 canonical direct reads. Because
    // wasmWorkerEligible() ignores its scenarioId argument and serve.py sends
    // COOP/COEP unconditionally, this proxy owns EVERY scenario on the dev
    // server -- so these three were absent on the default path. The particle
    // list has live consumers (p1-observables -> coulomb.js / g2.js /
    // anisotropy.js), which read an empty lattice and rendered "no engine field
    // samples" while the worker frame payload carried the particles.
    getScale0ParticleList() { return particleDataToList(this._lastParts); }

    // Contract members with no production consumer today, implemented so the
    // anti-drift gate in scale0-worker.spec.js is satisfied by real forwarding
    // rather than by shrinking the contract.
    getForceFieldSampled(stride = 2) { return samplerOr(this, 'em', stride, EMPTY_VEC()); }
    getGravityFieldSampled(stride = 2) { return samplerOr(this, 'gravity', stride, EMPTY_VEC()); }
    getFluxVolume() { return (this._ready && this._fluxView) ? this._fluxView : new Float64Array(0); }
    /**
     * Slice the already-resident flux volume (this._fluxView, a zero-copy
     * view over the worker's shared WASM heap — see getFluxVolume) into a
     * single 2D plane, client-side. Was a permanent `return new
     * Float64Array(0)` stub ("Phase 2: slice from heap") — the data was
     * already resident (getFluxVolume works today), it just was never
     * sliced.
     *
     * Output layout is byte-for-byte identical to the direct WasmBridge's
     * C++ binding (ftd_wasm.cpp get_flux_slice: `cache[a*N+b]` with
     * axis0: a=y,b=z (x=index fixed); axis1: a=x,b=z (y=index fixed);
     * axis2: a=x,b=y (z=index fixed)), substituting get_flux_volume's own
     * documented layout (`view[z*N*N + y*N + x] = density(x,y,z)`) — so
     * downstream consumers (transposeAndFlipNN in this file, frame-sync.js's
     * getScale0FluxSlice) need no changes.
     */
    getFluxSlice(axis, index) {
        if (!this._ready || !this._fluxView) return new Float64Array(0);
        const N = this.latticeSize | 0;
        if (!(N > 0)) return new Float64Array(0);
        const NN = N * N;
        // Guards the transient window during a scenario swap where _ready
        // is about to flip false / _fluxView may still reference the OLD
        // RenderBridge's stale/wrong-length buffer — mirrors the same
        // "empty until ready" contract every other proxy read already has.
        if (this._fluxView.length !== NN * N) return new Float64Array(0);
        const idx = Math.min(Math.max(index | 0, 0), N - 1);
        const view = this._fluxView;
        const out = new Float64Array(NN);
        if (axis === 0) {
            // YZ plane, X fixed at `idx`: out[y*N+z] = density(idx, y, z)
            for (let y = 0; y < N; y++) {
                const base = y * N;
                for (let z = 0; z < N; z++) {
                    out[base + z] = view[z * NN + y * N + idx];
                }
            }
        } else if (axis === 1) {
            // XZ plane, Y fixed at `idx`: out[x*N+z] = density(x, idx, z)
            for (let x = 0; x < N; x++) {
                const base = x * N;
                for (let z = 0; z < N; z++) {
                    out[base + z] = view[z * NN + idx * N + x];
                }
            }
        } else {
            // XY plane, Z fixed at `idx`: out[x*N+y] = density(x, y, idx)
            for (let x = 0; x < N; x++) {
                const base = x * N;
                for (let y = 0; y < N; y++) {
                    out[base + y] = view[idx * NN + y * N + x];
                }
            }
        }
        return out;
    }
    getDiagnostics() { return this._lastDiag ?? null; }
    getEnergyAudit() { return this._lastAudit ?? null; }
    getLagrangian() { return this._lastLag ?? null; }
    getKnotTelemetry() { return this._lastKnot ?? null; }
    getKnotEvents() { return this._lastKnotEvents ?? null; }
    getKnotAggregate() { return this._lastKnotAgg ?? null; }
    // Overlay samplers — lazy pull from the worker.
    // On first call for a given kind+stride the want is registered with the worker
    // and the cached result (initially empty) is returned. The worker computes the
    // sampler on its next postFrame() and sends the result back; subsequent calls
    // return the live cached data. One-frame latency on initial display only.
    _wantSampler(kind, stride, emptyFn) {
        const key = `${kind}@${stride}`;
        if (!this._samplerWant.has(key)) {
            this._samplerWant.set(key, true);
            this._worker.postMessage({ type: 'wantSampler', kind, stride });
        }
        return this._samplerCache[key] ?? emptyFn();
    }
    /**
     * Release a previously-wanted kind+stride. _wantSampler's registration
     * is otherwise permanently sticky for the life of the worker (matching
     * `wantedSamplers`'s own "persists across scenario changes" design in
     * wasm-bridge.worker.js) — a caller like flux-slice-panel.js that only
     * needs a kind while a UI row is visible must explicitly un-want it when
     * that row is hidden, or the worker keeps recomputing it on every
     * postFrame() forever, for the rest of the session. No-op if the kind
     * was never wanted (or was already released).
     */
    unwantSampler(kind, stride) {
        const key = `${kind}@${stride}`;
        if (!this._samplerWant.has(key)) return;
        this._samplerWant.delete(key);
        delete this._samplerCache[key];
        this._worker.postMessage({ type: 'unwantSampler', kind, stride });
    }
    getEFieldSampled(stride = 2)        { return this._wantSampler('e',            stride, EMPTY_VEC); }
    getBFieldSampled(stride = 2)        { return this._wantSampler('b',            stride, EMPTY_VEC); }
    getPoyntingSampled(stride = 2)      { return this._wantSampler('poynting',     stride, EMPTY_VEC); }
    getDivJSampled(stride = 2)          { return this._wantSampler('divJ',         stride, EMPTY_VAL); }
    getFluxVectorSampled(stride = 2)    { return this._wantSampler('fluxVector',   stride, EMPTY_VEC); }
    getVorticitySampled(stride = 2)     { return this._wantSampler('vorticity',    stride, EMPTY_VAL); }
    getHelicitySampled(stride = 2)      { return this._wantSampler('helicity',     stride, EMPTY_VAL); }
    getKretschmannSampled(stride = 2)   { return this._wantSampler('kretschmann',  stride, EMPTY_VAL); }
    getLatencySampled(stride = 2)       { return this._wantSampler('latency',      stride, EMPTY_VAL); }
    getFisherSampled(stride = 2)        { return this._wantSampler('fisher',       stride, EMPTY_VAL); }
    getCoherenceSampled(stride = 2)     { return this._wantSampler('coherence',    stride, EMPTY_VAL); }
    getCurlJSampled(stride = 2)         { return this._wantSampler('curlJ',        stride, EMPTY_VEC); }
    getStateFieldSampled(stride = 1)    { return this._wantSampler('state',        stride, EMPTY_VAL); }
    getGaussResidualSampled(stride = 1) { return this._wantSampler('gaussResidual',stride, EMPTY_VAL); }
    getEMForceField(stride = 2)         { return this._wantSampler('em',           stride, EMPTY_VEC); }
    getGravityForceField(stride = 2)    { return this._wantSampler('gravity',      stride, EMPTY_VEC); }
    getStrongForceField(stride = 2)     { return this._wantSampler('strong',       stride, EMPTY_VEC); }
    /** Kind-dispatched Scale-0 field sampler; see bridge-contract.js samplerOr. */
    getSamplerOr(kind, stride = 2, fallback) { return samplerOr(this, kind, stride, fallback); }
    getGravityMetricAgg() {
        const key = 'gravityMetricAgg@0';
        if (!this._samplerWant.has(key)) {
            this._samplerWant.set(key, true);
            this._worker.postMessage({ type: 'wantSampler', kind: 'gravityMetricAgg', stride: 0 });
        }
        return this._samplerCache[key] ?? { active: false, latencyMax: 0, latencyMean: 0, fMin: 1, gammaMax: 1, dilationMaxPct: 0, voxelCount: 0 };
    }
    getLatencyVolume() { return new Float64Array(0); }  // full volume not supported on worker path
    setBoundaryShape() {}
    setReflectiveBoundary() {}
    setFluxBoundaryMode(mode) { this._cmd('setFluxBoundary', mode); }

    // ── Single-point inspect reads (parity with direct WasmBridge) ──────────
    // A synchronous worker round-trip is impossible, and the worker hosts a
    // SEPARATE RenderBridge from any main-thread bridge, so these cannot be
    // answered from this proxy. They return SAFE empty/null values so that
    // optional-chaining callers (e.g. anisotropy.js) degrade to their analytic
    // fallback instead of throwing and tripping the raf-coordinator error budget.
    // TODO (Phase 2): true worker-backed inspect via a request/response channel
    // into the worker's RenderBridge.
    inspectVoxel() { return null; }
    getForceAt() { return null; }
    sampleVAtRay() { return { positions: new Float32Array(0), V: new Float32Array(0), count: 0 }; }
    // The worker does not currently post a constants payload, so there is no
    // cached value to forward — return null (callers should use constants.js).
    getConstants() { return null; }

    // ── Scenario / run control ──────────────────────────────────────────────
    /**
     * Post an async worker create. Returns true when the create was posted
     * (not when C++ finished). Setup failure is reported via onSetupFailure
     * once the worker replies (ready.setupOk === false or error).
     */
    setupScenario(name) {
        this._scenarioId = name || this._scenarioId;
        this._ready = false;
        // Diagnostics belong to the old worker-hosted RenderBridge until the
        // new `create` completes. Clear them so exact-step observatories do not
        // mistake an old high tick count for completion of a newly reset run.
        this._lastDiag = null;
        this._lastTick = -1;
        this._clearFrameWatchdog();   // old-scenario watchdog is stale; re-arms on next 'ready'/setRunning
        this._samplerCache = {};   // stale; worker will repopulate on first frame of new scenario
        this._pendingCommands = []; // discard any commands queued for the previous scenario
        this._worker.postMessage({
            type: 'create', N: this.latticeSize, scenarioId: this._scenarioId,
            toggles: this._toggles,
            toggleNames: SCALE0_ENGINE_TOGGLE_NAMES,
            pool: workerPoolSize(),
        });
        return true;
    }
    setRunning(v) {
        v = !!v;
        if (v === this._running) return;                              // dedupe — tick.js calls every frame
        this._running = v;
        this._worker.postMessage({ type: 'setRunning', value: v });
        // Frame-watchdog follows the run state: arm when we start running (a
        // ready-but-dead worker will never post a frame), clear when paused so a
        // legitimately idle worker isn't falsely tripped.
        if (v) this._armFrameWatchdog();
        else this._clearFrameWatchdog();
    }
    setTicksPerFrame(v) {
        this._pendingTPF = v;
        if (this._ctrl) Atomics.store(this._ctrl, CTRL.TICKS_PER_FRAME, Math.round(v * 1000));
    }
    tickOnce() { this._cmd('tickScale0'); }
    /**
     * Forward the telemetry demand mask to the worker.
     *
     * This was an empty stub with no matching worker message case, so the
     * gate in telemetry/demand.js was inert on the worker path: postFrame
     * recomputed the energy audit and Lagrangian every frame regardless of
     * whether any panel consumed them. Sent only on change to avoid a
     * postMessage per frame.
     */
    setTelemetryMask(wantAudit = true, wantLag = true) {
        const a = !!wantAudit, l = !!wantLag;
        if (a === this._wantAudit && l === this._wantLag) return;
        this._wantAudit = a; this._wantLag = l;
        if (this._worker && !this._terminated) {
            this._worker.postMessage({ type: 'setTelemetryMask', wantAudit: a, wantLag: l });
        }
    }

    // ── Mutators (the inject UI / param sliders call these on the bridge) ────
    setToggle(k, v) { this._toggles[k] = v; this._cmd('setToggle', k, v); }
    setDt(...a) { this._cmd('setDt', ...a); }
    // setOmega0/setLangevinTemp are fire-and-forget commands to the worker's
    // RenderBridge (no synchronous round trip is possible). getOmega0/
    // getLangevinTemp are real methods on the direct WasmBridge but were
    // simply absent here, so panels reading them silently substituted a
    // literal default or their own UI slider value as if it were engine
    // truth — wrong the moment either was actually changed, since the
    // display never learned about it. Mirroring the last value THIS proxy
    // told the worker to set is not a fabrication: barring a failed
    // command, it IS what the worker's RenderBridge now holds. Defaults
    // match term_toggles.h (omega0=1.0, langevin_T=0.0) so an unset read
    // before the first setter call matches the engine's own default.
    setOmega0(w) { this._omega0 = w; this._cmd('setOmega0', w); }
    getOmega0() { return this._omega0 ?? 1.0; }
    setLangevinTemp(t) { this._langevinTemp = t; this._cmd('setLangevinTemp', t); }
    getLangevinTemp() { return this._langevinTemp ?? 0.0; }
    setLangevinGamma(g) { this._langevinGamma = g; this._cmd('setLangevinGamma', g); }
    getLangevinGamma() { return this._langevinGamma ?? 0.01; }
    injectParticle(...a) { this._cmd('injectParticle', ...a); }
    injectFlux(...a) { this._cmd('injectFlux', ...a); }
    injectWavepacket(...a) { this._cmd('injectWavepacket', ...a); }
    injectWaveVel(...a) { this._cmd('injectWaveVel', ...a); }
    createEntangledPair(...a) { this._cmd('createEntangledPair', ...a); }
    clearField() { this._cmd('clearField'); }
    seedRandomFlux() { this._cmd('seedRandomFlux'); }

    // ── Lifecycle ───────────────────────────────────────────────────────────
    reset(n) {
        if (typeof n === 'number' && n > 0) this.latticeSize = (n % 2 === 0) ? n + 1 : n;
        this.setupScenario(this._scenarioId);
    }
    resize(n) { this.reset(n); }
    terminate() {
        if (!this._terminated) { this._terminated = true; _live--; _terminated++; }
        if (this._readyTimer) { try { clearTimeout(this._readyTimer); } catch { /* ignore */ } this._readyTimer = null; }
        this._clearFrameWatchdog();
        // Drop callbacks so a late queued message cannot reach the dashboard
        // even if _onMessage's terminated guard is somehow bypassed.
        this._onEngineToggles = null;
        this._onInitFailure = null;
        this._onSetupFailure = null;
        try { this._worker.onmessage = null; } catch (e) { /* ignore */ }
        try { this._worker.onerror = null; } catch (e) { /* ignore */ }
        try { this._worker.postMessage({ type: 'dispose' }); } catch (e) { /* ignore */ }
        try { this._worker.terminate(); } catch (e) { /* ignore */ }
    }
    dispose() { this.terminate(); }
}
