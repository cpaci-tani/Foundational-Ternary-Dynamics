// Main-thread proxy for the Scale-0 physics Web Worker (Phase 2). Presents the
// same surface the rest of Scale-0 calls on the fluxMock, but:
//   • READS run on a "shadow" MockBridge whose field buffers are repointed at the
//     worker's SharedArrayBuffers — so the existing samplers / getFluxVolume /
//     diagnostics work unchanged, reading the worker-maintained shared memory.
//   • COMMANDS (inject/toggle/setup/scenario/scrub) postMessage to the worker.
// The shadow never ticks, so getFluxVolume never recomputes _fluxMag on the main
// thread (the worker keeps it fresh in shared memory). See PLAN_SCALE0_PHYSICS_WORKER.md.

import { MockBridge } from './mock-bridge.js';
import { createScale0Capabilities } from './capabilities/scale0.js';
import { viewSharedField } from './shared-field.js';

const EMPTY_PARTS = () => ({
    positions: new Float32Array(0), colors: new Float32Array(0),
    sizes: new Float32Array(0), velocities: new Float32Array(0), count: 0,
});

export class MockBridgeProxy {
    constructor(latticeSize) {
        this.isWasm = false;
        this.isWorker = true;
        this.latticeSize = (latticeSize % 2 === 0) ? latticeSize + 1 : latticeSize;
        this._scenarioId = 'flux-pulse';
        this._toggles = {};
        this._boundaryShape = 'cube';
        this._reflective = false;
        this._lastDiag = null;
        this._lastParts = null;
        this._ctrl = null;
        this._ready = false;
        this._running = true;   // worker starts running on create; deduped in setRunning

        // Shadow: a MockBridge used ONLY for reads. Its buffers are swapped for
        // SAB views on 'ready'; it never ticks (so _fluxDirty stays false and
        // getFluxVolume returns the worker's _fluxMag without an O(N³) recompute).
        this._shadow = new MockBridge(this.latticeSize);
        this._shadow._sparseTick = false;

        this._worker = new Worker(new URL('./mock-bridge.worker.js', import.meta.url), { type: 'module' });
        this._worker.onmessage = (e) => this._onMessage(e.data);
        this._worker.onerror = (e) => console.error('[Scale0 worker]', e.message || e);

        this.capabilities = { scale0: this._buildCaps() };
    }

    _onMessage(m) {
        if (m.type === 'ready') {
            const v = viewSharedField(m.sab);
            this.latticeSize = m.N;
            const sh = this._shadow;
            sh.latticeSize = m.N;
            sh._fluxJ = v.fluxJ; sh._fluxWV = v.fluxWV; sh._fluxMag = v.fluxMag; sh._stateGrid = v.state;
            sh._fluxDirty = false;           // worker maintains _fluxMag; never recompute on read
            this._ctrl = v.ctrl;
            this._ready = true;
        } else if (m.type === 'frame') {
            this._lastDiag = m.diag;
            if (m.parts) this._lastParts = m.parts;
        } else if (m.type === 'error') {
            console.error('[Scale0 worker]', m.where, m.msg);
        }
    }

    _cmd(method, ...args) { this._worker.postMessage({ type: 'command', method, args }); }

    /** Monotonic frame counter from the worker (shared); for skip-unchanged in tick.js. */
    get frameCounter() { return this._ctrl ? Atomics.load(this._ctrl, 0) : 0; }
    get ready() { return this._ready; }

    _buildCaps() {
        // Reuse the real factory for all READS (wired to the shadow), then
        // override COMMAND methods to post to the worker.
        const caps = createScale0Capabilities(this._shadow);
        caps.tickScale0 = () => {};                                       // worker self-ticks
        caps.setupScenario = (name) => this.setupScenario(name);
        caps.setToggle = (k, v) => { this._toggles[k] = v; this._cmd('setToggle', k, v); };
        caps.setBoundaryShape = (s) => { this._boundaryShape = s; this._cmd('setBoundaryShape', s); };
        caps.setReflectiveBoundary = (on) => { this._reflective = on; this._cmd('setReflectiveBoundary', on); };
        caps.getScale0Diagnostics = () => this._lastDiag ?? (this._shadow.getDiagnostics ? this._shadow.getDiagnostics() : null);
        caps.getScale0ParticleFrame = () => this._lastParts ?? EMPTY_PARTS();
        caps.loadScale0Snapshot = (snap) => this._loadSnapshot(snap);
        return caps;
    }

    // ── Scenario / run control (commands) ────────────────────────────────────
    setupScenario(name) {
        this._scenarioId = name || this._scenarioId;
        this._ready = false;
        this._worker.postMessage({
            type: 'create', N: this.latticeSize, scenarioId: this._scenarioId,
            toggles: this._toggles, boundaryShape: this._boundaryShape, reflective: this._reflective,
        });
    }
    setRunning(v) {
        v = !!v;
        if (v === this._running) return;          // dedupe — tick.js calls this every frame
        this._running = v;
        this._worker.postMessage({ type: 'setRunning', value: v });
    }

    // ── Mutators some code calls directly on the bridge (wire.js inject UI) ───
    injectFlux(...a) { this._cmd('injectFlux', ...a); }
    injectParticle(...a) { this._cmd('injectParticle', ...a); }
    injectWavepacket(...a) { this._cmd('injectWavepacket', ...a); }
    createEntangledPair(...a) { this._cmd('createEntangledPair', ...a); }
    clearField() { this._cmd('clearField'); }
    seedRandomFlux() { this._cmd('seedRandomFlux'); }
    setParam(...a) { this._cmd('setParam', ...a); }
    setDt(...a) { this._cmd('setDt', ...a); }
    setBoundaryShape(s) { this._boundaryShape = s; this._cmd('setBoundaryShape', s); }
    setReflectiveBoundary(on) { this._reflective = on; this._cmd('setReflectiveBoundary', on); }

    // ── Reads delegated to the shadow (some code calls these on the bridge) ───
    getFluxVolume() { return this._ready ? this._shadow.getFluxVolume() : new Float64Array(0); }
    getFluxSlice(axis, index) { return this._ready ? this._shadow.getFluxSlice(axis, index) : new Float64Array(0); }
    getParticleData() { return this._lastParts ?? EMPTY_PARTS(); }
    getDiagnostics() { return this._lastDiag ?? null; }
    // Per-voxel probe (p1-observables-panel). Reads the shadow's SAB field; the
    // shadow's _particles is [] (the render frame is separate), so the particle
    // lookup is a safe no-op and the flux fields come from shared memory.
    inspectVoxel(x, y, z) { return this._ready ? this._shadow.inspectVoxel(x, y, z) : null; }

    // ── Snapshot restore (scrub) → forward buffers to the worker ─────────────
    _loadSnapshot(snap) {
        if (!snap || !snap.flux) return false;
        this._cmd('setScale0FluxBuffer', snap.flux);
        if (snap.lattice) this._cmd('setScale0LatticeBuffer', snap.lattice);
        if (snap.wave) this._cmd('setScale0WaveBuffer', snap.wave);
        if (typeof snap.tick === 'number') this._cmd('setScale0Tick', snap.tick);
        if (snap.particles) this._cmd('setScale0ParticleList', snap.particles);
        return true;
    }

    // ── Teardown ─────────────────────────────────────────────────────────────
    terminate() {
        try { this._worker.postMessage({ type: 'dispose' }); } catch (e) { /* ignore */ }
        try { this._worker.terminate(); } catch (e) { /* ignore */ }
    }
    dispose() { this.terminate(); }
}
