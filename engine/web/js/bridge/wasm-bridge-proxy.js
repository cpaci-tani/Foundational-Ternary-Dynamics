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
        this._running = true;
        this._ctrl = null;
        this._fluxView = null;
        this._lastDiag = null;
        this._lastParts = null;
        this._lastAudit = null;
        this._lastLag = null;
        this._pendingTPF = undefined;

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
        } else if (m.type === 'frame') {
            this._lastDiag = m.diag;
            if (m.parts) this._lastParts = m.parts;
        } else if (m.type === 'error') {
            console.error('[WasmWorker]', m.where, m.msg);
        }
    }

    _cmd(method, ...args) { this._worker.postMessage({ type: 'command', method, args }); }

    /** Monotonic frame counter from the worker (shared) — drives render refresh in tick.js. */
    get frameCounter() { return this._ctrl ? Atomics.load(this._ctrl, CTRL.FRAME) : 0; }
    get ready() { return this._ready; }

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
    // Overlay samplers (off by default for hydrogen) — empty until Phase 2 adds
    // worker-side recompute. Names match capabilities/scale0.js dispatch.
    getEFieldSampled() { return EMPTY_VEC(); }
    getBFieldSampled() { return EMPTY_VEC(); }
    getPoyntingSampled() { return EMPTY_VEC(); }
    getDivJSampled() { return EMPTY_VAL(); }
    getFluxVectorSampled() { return EMPTY_VEC(); }
    getVorticitySampled() { return EMPTY_VAL(); }
    getHelicitySampled() { return EMPTY_VAL(); }
    getKretschmannSampled() { return EMPTY_VAL(); }
    getLatencySampled() { return EMPTY_VAL(); }
    getFisherSampled() { return EMPTY_VAL(); }
    getCoherenceSampled() { return EMPTY_VAL(); }
    getCurlJSampled() { return EMPTY_VEC(); }
    getStateFieldSampled() { return EMPTY_VAL(); }
    getGaussResidualSampled() { return EMPTY_VAL(); }
    getEMForceField() { return EMPTY_VEC(); }
    getGravityForceField() { return EMPTY_VEC(); }
    getStrongForceField() { return EMPTY_VEC(); }
    getGravityMetricAgg() {
        return { active: false, latencyMax: 0, latencyMean: 0, fMin: 1, gammaMax: 1, dilationMaxPct: 0, voxelCount: 0 };
    }
    getLatencyVolume() { return new Float64Array(0); }
    setBoundaryShape() {}
    setReflectiveBoundary() {}

    // ── Scenario / run control ──────────────────────────────────────────────
    setupScenario(name) {
        this._scenarioId = name || this._scenarioId;
        this._ready = false;
        this._worker.postMessage({ type: 'create', N: this.latticeSize, scenarioId: this._scenarioId, toggles: this._toggles });
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
    createEntangledPair(...a) { this._cmd('createEntangledPair', ...a); }

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
