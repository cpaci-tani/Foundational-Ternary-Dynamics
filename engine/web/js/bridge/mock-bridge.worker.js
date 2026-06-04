// Scale-0 physics Web Worker (Phase 2). Hosts the authoritative SAB-backed
// MockBridge and self-ticks on its own clock, decoupled from the main render
// loop, so a heavy tick never stalls the UI. The field lives in SharedArrayBuffers
// (written here, read by the main-thread MockBridgeProxy); only the small
// particle + diagnostic payloads ride postMessage. See PLAN_SCALE0_PHYSICS_WORKER.md.
//
// Module worker: `new Worker(url, { type: 'module' })`. Requires the page to be
// crossOriginIsolated (COOP/COEP) so SharedArrayBuffer exists.

import { MockBridge } from './mock-bridge.js';
import { installCapabilityGetter } from './capabilities/install.js';
import { CTRL } from './shared-field.js';

// The lazy `bridge.capabilities` getter is normally installed by bridge-init.js
// (which the worker does not import — it would pull in WasmBridge/WebSocketBridge).
// Install it directly on the worker's MockBridge prototype so capabilities.scale0
// exists here just as it does on the main thread.
installCapabilityGetter(MockBridge.prototype);

let bridge = null;
let ctrl = null;            // Int32Array view over the control SAB
let timer = 0;
let scenarioId = 'flux-pulse';
let pframe = 0;                // postFrame counter — throttles the particle-list payload
const PLIST_EVERY = 6;         // ship getScale0ParticleList() ~every 6th frame (≈10 Hz)
const TARGET_DT = 1000 / 60;   // cap physics at ~60 Hz; tick-time-limited at large L

function publishShared(N) {
    pframe = 0;                                       // ship the particle list on the next frame
    const sab = bridge.getSharedField();
    ctrl = new Int32Array(sab.ctrl);
    Atomics.store(ctrl, CTRL.RUNNING, 1);
    Atomics.store(ctrl, CTRL.N, N);
    self.postMessage({ type: 'ready', sab, N });
}

function postFrame() {
    if (!bridge) return;
    bridge._updateFluxMag();                          // O(N³) magnitude — off the main thread
    const s0 = bridge.capabilities.scale0;
    let diag = null, parts = null;
    try { if (s0.getScale0Diagnostics) diag = s0.getScale0Diagnostics(); } catch (e) { /* ignore */ }
    try { if (s0.getScale0ParticleFrame) parts = s0.getScale0ParticleFrame(); } catch (e) { /* ignore */ }
    // Particle LIST (x,y,z,state,charge,…) for spectrum / observables / harness —
    // the main-thread shadow owns no particles, so the list must come from here.
    // Throttled: it changes slowly and the panels sample at ≤4 Hz. Null on
    // skipped frames → the proxy keeps its last list.
    let particleList = null;
    if ((pframe++ % PLIST_EVERY) === 0 && typeof bridge.getScale0ParticleList === 'function') {
        try { particleList = bridge.getScale0ParticleList(); } catch (e) { /* ignore */ }
    }
    if (ctrl) {
        Atomics.store(ctrl, CTRL.TICK, bridge._tick | 0);
        Atomics.store(ctrl, CTRL.PCOUNT, parts ? (parts.count | 0) : 0);
        Atomics.add(ctrl, CTRL.FRAME, 1);
    }
    // Particle/diag payloads are small → structured-clone copy (no transfer, so
    // the bridge keeps its pre-allocated particle buffers across frames).
    self.postMessage({ type: 'frame', tick: bridge._tick | 0, diag, parts, particleList });
}

function loop() {
    timer = 0;
    if (!bridge) return;
    const t0 = performance.now();
    if (ctrl && Atomics.load(ctrl, CTRL.RUNNING)) {
        try { bridge.capabilities.scale0.tickScale0(); }
        catch (e) { self.postMessage({ type: 'error', where: 'tick', msg: String(e && e.message || e) }); }
        postFrame();
    }
    const elapsed = performance.now() - t0;
    timer = setTimeout(loop, Math.max(0, TARGET_DT - elapsed));   // period = max(tickTime, 16ms)
}

function applyInit(s0, m) {
    if (m.boundaryShape && s0.setBoundaryShape) s0.setBoundaryShape(m.boundaryShape);
    if (typeof m.reflective === 'boolean' && s0.setReflectiveBoundary) s0.setReflectiveBoundary(m.reflective);
    s0.setupScenario(scenarioId);                     // allocates the SAB field
    if (m.toggles) for (const k in m.toggles) { try { s0.setToggle && s0.setToggle(k, m.toggles[k]); } catch (e) { /* ignore */ } }
}

self.onmessage = (e) => {
    const m = e.data;
    try {
        switch (m.type) {
            case 'create': {
                scenarioId = m.scenarioId || 'flux-pulse';
                bridge = new MockBridge(m.N);
                bridge._useSAB = true;
                applyInit(bridge.capabilities.scale0, m);
                publishShared(bridge.latticeSize);
                if (!timer) loop();
                break;
            }
            case 'resize': {
                if (!bridge) break;
                scenarioId = m.scenarioId || scenarioId;
                bridge.reset(m.N);
                applyInit(bridge.capabilities.scale0, m);
                publishShared(bridge.latticeSize);
                break;
            }
            case 'command': {
                // Mutators are forwarded verbatim. Some live on capabilities.scale0
                // (setupScenario/setToggle/setBoundaryShape/setReflectiveBoundary),
                // some on the bridge itself (injectFlux/clearField/seedRandomFlux/
                // setParam/setDt/setScale0*Buffer). Try the capability first.
                const s0 = bridge && bridge.capabilities && bridge.capabilities.scale0;
                if (s0 && typeof s0[m.method] === 'function') s0[m.method](...(m.args || []));
                else if (bridge && typeof bridge[m.method] === 'function') bridge[m.method](...(m.args || []));
                if (m.method === 'setupScenario') scenarioId = (m.args && m.args[0]) || scenarioId;
                break;
            }
            case 'setRunning':
                if (ctrl) Atomics.store(ctrl, CTRL.RUNNING, m.value ? 1 : 0);
                break;
            case 'dispose':
                if (timer) { clearTimeout(timer); timer = 0; }
                try { if (bridge && bridge.dispose) bridge.dispose(); } catch (e) { /* ignore */ }
                bridge = null; ctrl = null;
                break;
        }
    } catch (err) {
        self.postMessage({ type: 'error', where: m && m.type, msg: String(err && err.message || err) });
    }
};
