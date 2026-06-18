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

const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, TICKS_PER_FRAME: 5, LEN: 8 };
const EMPTY_PARTS = () => ({ positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), count: 0 });
const EMPTY_VEC = () => ({ positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 });
const EMPTY_VAL = () => ({ positions: new Float32Array(0), values: new Float32Array(0), count: 0 });

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

export class WasmBridgeProxy {
    constructor(latticeSize) {
        _live++; _created++;
        this.isWasm = true;
        this.isWorker = true;
        this._terminated = false;
        this.latticeSize = (latticeSize % 2 === 0) ? latticeSize + 1 : latticeSize;
        this._scenarioId = 'flux-pulse';
        this._toggles = {};
        this._ready = false;
        this._running = null;
        this._ctrl = null;
        this._fluxView = null;
        this._lastDiag = null;
        this._lastParts = null;
        this._lastAudit = null;
        this._lastLag = null;
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

        // CLASSIC worker (Emscripten module via importScripts). No { type: 'module' }.
        this._worker = new Worker(new URL('./wasm-bridge.worker.js', import.meta.url));
        this._worker.onmessage = (e) => this._onMessage(e.data);
        this._worker.onerror = (e) => console.error('[WasmWorker]', e.message || e);

        this.capabilities = { scale0: this._buildCaps() };
    }

    _onMessage(m) {
        if (m.type === 'ready') {
            this.latticeSize = m.N;
            this._ctrl = new Int32Array(m.ctrl);
            this._fluxView = new Float64Array(m.heap, m.fluxPtr, m.fluxLen);
            this._ready = true;
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
        } else if (m.type === 'frame') {
            this._lastDiag = m.diag;
            if (m.parts) this._lastParts = m.parts;
            if (m.audit) this._lastAudit = m.audit;
            if (m.lag)   this._lastLag   = m.lag;
            const hadSamplers = Boolean(m.samplers && Object.keys(m.samplers).length);
            if (hadSamplers) Object.assign(this._samplerCache, m.samplers);

            if (typeof window !== 'undefined' && window.__ftdCtx && typeof window.__ftdCtx.onBridgePostFrame === 'function') {
                window.__ftdCtx.onBridgePostFrame(hadSamplers);
            }
        } else if (m.type === 'error') {
            console.error('[WasmWorker]', m.where, m.msg);
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
    getToggle(name) { return this._toggles[name] ?? true; }          // local cache mirrors worker state

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
        return caps;
    }

    // ── Bridge reads the capability factory calls ───────────────────────────
    tick() {}                                                        // no-op; worker self-ticks
    getParticleData() { return this._lastParts ?? EMPTY_PARTS(); }
    getFluxVolume() { return (this._ready && this._fluxView) ? this._fluxView : new Float64Array(0); }
    getFluxSlice() { return new Float64Array(0); }                   // Phase 2: slice from heap
    getDiagnostics() { return this._lastDiag ?? null; }
    getEnergyAudit() { return this._lastAudit ?? null; }
    getLagrangian() { return this._lastLag ?? null; }
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

    // ── Scenario / run control ──────────────────────────────────────────────
    setupScenario(name) {
        this._scenarioId = name || this._scenarioId;
        this._ready = false;
        this._samplerCache = {};   // stale; worker will repopulate on first frame of new scenario
        this._pendingCommands = []; // discard any commands queued for the previous scenario
        this._worker.postMessage({
            type: 'create', N: this.latticeSize, scenarioId: this._scenarioId,
            toggles: this._toggles, pool: workerPoolSize(),
        });
    }
    setRunning(v) {
        v = !!v;
        if (v === this._running) return;                              // dedupe — tick.js calls every frame
        this._running = v;
        this._worker.postMessage({ type: 'setRunning', value: v });
    }
    setTicksPerFrame(v) {
        this._pendingTPF = v;
        if (this._ctrl) Atomics.store(this._ctrl, CTRL.TICKS_PER_FRAME, Math.round(v * 1000));
    }
    tickOnce() { this._cmd('tickScale0'); }
    setTelemetryMask() {}                                            // Phase 2: gate worker audit

    // ── Mutators (the inject UI / param sliders call these on the bridge) ────
    setToggle(k, v) { this._toggles[k] = v; this._cmd('setToggle', k, v); }
    setDt(...a) { this._cmd('setDt', ...a); }
    setOmega0(...a) { this._cmd('setOmega0', ...a); }
    setLangevinTemp(...a) { this._cmd('setLangevinTemp', ...a); }
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
        try { this._worker.postMessage({ type: 'dispose' }); } catch (e) { /* ignore */ }
        try { this._worker.terminate(); } catch (e) { /* ignore */ }
    }
    dispose() { this.terminate(); }
}
