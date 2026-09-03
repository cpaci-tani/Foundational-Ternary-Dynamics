/**
 * @file engine/web/js/bridge/wasm-bridge.js
 * @purpose Thin wrapper around the compiled C++/WASM physics engine
 *          (engine/wasm/ftd_wasm.cpp). Implements the same ScaleBridge
 *          contract as the primary bridge backend
 *          without touching call sites.
 * @consumers bridge-init.js (re-exports), engine/web/js/scales/scaleN/controller.js
 *            for N in 0..11 via the createScale0/1/2Capabilities factories.
 * @contract CONTRACTS.md §2 (Capability Factory Contract) — symmetric
 *            surface with the WebSocketBridge (native-GPU path).
 *          engine/wasm/ftd_wasm.cpp (the embind module this wraps)
 *          engine/wasm/bindings_render_bridge.cpp (toggle map + per-method bindings)
 *
 * Phase 2b of the refactor sweep extracted WasmBridge from
 * bridge-init.js. Verbatim move — class body and helper functions
 * (_wasmCallOr, _wasmLoadPromise singleton, EMPTY_* sampler-fallback
 * frozen objects) are bit-identical to the pre-Phase-2 file.
 *
 * Phase 2c will extract the capability factories
 * (createScale0/1/2Capabilities + installCapabilityGetter) so that
 * bridge-init.js shrinks to a re-export shim.
 *
 * The _wasmCallOr helper (defined below) is the single delegation
 * pattern WasmBridge uses for ~20 sampler methods that may or may not
 * be exposed by the underlying WASM module: it returns a fallback
 * value (typically a frozen EMPTY_FIELD_SAMPLE / EMPTY_SCALAR_SAMPLE /
 * EMPTY_PARTICLE_DATA singleton) when the method is missing, avoiding
 * per-call object allocation.
 */

import { K_B, VOXEL_VOLUME } from '../constants.js';
import { debugLog } from '../core/log.js';
import { createNativeParticleEngine } from './native-particle-engine.js?v=7';
import { createAtomEngine } from './mock-atom-engine.js';
import { reflectIntoBoundary } from './boundary.js';
import { samplerOr, particleDataToList, TOGGLE_REQUIRES } from './bridge-contract.js';
import { loadVerifiedWasmVariant } from './wasm-artifact-identity.js';

// ── WASM Bridge ────────────────────────────────────────────────────
let _wasmLoadPromise = null; // singleton to prevent duplicate script injection

async function installVerifiedFactory(variantId, factoryName) {
    const verified = await loadVerifiedWasmVariant(variantId);
    const scriptUrl = URL.createObjectURL(new Blob(
        [verified.loaderText], { type: 'text/javascript' },
    ));
    try {
        await new Promise((resolve, reject) => {
            const script = document.createElement('script');
            script.src = scriptUrl;
            script.onload = () => { script.remove(); resolve(); };
            script.onerror = () => {
                script.remove();
                reject(new Error(`Verified WASM loader execution failed: ${variantId}`));
            };
            document.head.appendChild(script);
        });
    } finally {
        URL.revokeObjectURL(scriptUrl);
    }
    if (typeof globalThis[factoryName] !== 'function') {
        throw new Error(`Verified WASM loader did not publish ${factoryName}`);
    }
    return verified;
}

// Memory64 (wasm64) feature-detection, computed once per session. When
// supported we load the 8 GB `ftd_core64` build (lifts the in-browser lattice
// cap from L~117 to L~187); otherwise the 2 GB wasm32 `ftd_core` build. iOS /
// Safari and flagged-Firefox fall back to wasm32. See
// engine/web/docs/PLAN_WASM64_UPGRADE.md.
let _memory64Supported = null;
function supportsMemory64() {
    if (_memory64Supported !== null) return _memory64Supported;
    try {
        // The `index: 'i64'` descriptor is the Memory64 marker; constructing it
        // throws on engines without Memory64.
        new WebAssembly.Memory({ initial: 1, maximum: 1, index: 'i64' });
        _memory64Supported = true;
    } catch (_e) {
        _memory64Supported = false;
    }
    return _memory64Supported;
}

// Empty-result singletons used by WasmBridge sampler fallbacks.
// Before RF-6 the same inline `{ positions: new Float32Array(0), ... }` literal
// was allocated ~18× per module; the empty arrays themselves are immutable so
// a single shared instance is safe for every caller.
const EMPTY_FIELD_SAMPLE = Object.freeze({
    positions: new Float32Array(0),
    vectors: new Float32Array(0),
    count: 0,
});
const EMPTY_SCALAR_SAMPLE = Object.freeze({
    positions: new Float32Array(0),
    values: new Float32Array(0),
    count: 0,
});
const EMPTY_PARTICLE_DATA = Object.freeze({
    positions: new Float32Array(0),
    colors: new Float32Array(0),
    sizes: new Float32Array(0),
    spin: new Float32Array(0),
    colorCharge: new Float32Array(0),
    locked: new Uint8Array(0),
    count: 0,
});
const EMPTY_KNOT_TELEMETRY = Object.freeze({ ids: new Int32Array(0), signs: new Int32Array(0), birth: new Int32Array(0), age: new Int32Array(0), size: new Int32Array(0), peak: new Int32Array(0), fields: new Float32Array(0), stride: 11, count: 0 });
const EMPTY_KNOT_EVENTS = Object.freeze({ tick: new Int32Array(0), type: new Int32Array(0), nparents: new Int32Array(0), nchildren: new Int32Array(0), sign: new Int32Array(0), count: 0 });
const EMPTY_KNOT_AGG = Object.freeze({ alive: 0, netCharge: 0, births: 0, deaths: 0, fissions: 0, fusions: 0 });

// Generic delegate: run `fn` if the WASM module exposes both the bridge AND
// the specified method, else return `fallback`. Collapses the two-line guard
// block (`if (!this._module || !this._bridge) return X; if (typeof ... !==
// 'function') return X;`) that previously appeared ~20× inside WasmBridge.
function _wasmCallOr(bridge, methodName, fallback, fn) {
    if (!bridge._module || !bridge._bridge) return fallback;
    if (typeof bridge._module[methodName] !== 'function') return fallback;
    return fn(bridge._module, bridge._bridge);
}

function normalizeDynamicalStateDigest(raw, transport = 'direct') {
    if (!raw) return null;
    return {
        schemaVersion: raw.schema_version,
        latticeSize: raw.lattice_size,
        siteCount: raw.site_count,
        tick: raw.tick,
        stateVersion: raw.state_version,
        // Native WebSocket owns a telemetry source epoch; standalone WASM
        // does not. Preserve the shared field explicitly as unavailable.
        sourceEpoch: null,
        telemetrySourceEpoch: null,
        hashLo: raw.hash_lo,
        hashHi: raw.hash_hi,
        nonfiniteValueCount: raw.nonfinite_value_count,
        nondefaultValueCount: raw.nondefault_value_count,
        deviceToHostBytes: raw.device_to_host_bytes,
        fullMirrorCalls: raw.full_mirror_calls,
        exactDefaultRecord: raw.exact_default_record,
        compute: 'CPU',
        runtime: 'wasm',
        transport,
    };
}

/** @implements {import('./bridge/bridge-contract.js').ScaleBridge} */
export class WasmBridge {
    constructor() {
        this._module = null;
        this._bridge = null;
        this.latticeSize = 33;
        this.ready = false;
        this.isWasm = true;
        this.isWasm64 = false;   // set true in init() when the Memory64 build loads
        this.artifactIdentity = null;
        this.artifactIdentityState = 'not-loaded';
        this.artifactIdentityReady = Promise.resolve(null);
        this._lastScale0Audit = null;
        this._lastScale0AuditTick = -1;
        // Direct-WASM telemetry needs a state identity independent of the
        // engine tick: paused scientific writes can change the record without
        // advancing currentTick(). Group receipt times are cached per identity
        // so repeated panel reads cannot make an old sample appear newly born.
        this._scale0TelemetrySourceEpoch = 0;
        this._scale0TelemetryStateVersion = 0;
        this._scale0TelemetryGroupMeta = new Map();
        // Scale-1 PE: native C++/WASM ParticleEngine via embind adapter.
        // The native kernel uses G_PE = G_DERIVED (FTD-0131) — the old
        // "stale G_N binary" concern that once justified a JS engine no
        // longer holds (particle_engine.cpp:148 uses G_PE).
        this._peEngine = createNativeParticleEngine(this);
    }

    async init(latticeSize = 33) {
        // Revision 2.7: introspectable load state ('loading'|'ready'|'failed')
        // so diagnostics/tests/UI can distinguish "still compiling" from
        // "failed" without parsing console output. Additive — no consumer is
        // required to read it.
        this.wasmLoadState = 'loading';
        let size = parseInt(latticeSize, 10);
        if (isNaN(size) || size <= 0 || size > 257) {
            console.warn('[WasmBridge] Invalid init latticeSize:', latticeSize, '- falling back to 33');
            size = 33;
        }
        // Odd lattices only — snap even N up to the next odd so phenomena
        // center on a true center voxel.
        if (size % 2 === 0) size += 1;
        this.latticeSize = size;
        try {
            // Feature-detect Memory64 and load the matching module. Only ONE
            // build is ever loaded per session (the choice is deterministic per
            // browser), so the single _wasmLoadPromise is safe with a dynamic src.
            const use64 = supportsMemory64();
            this.isWasm64 = use64;
            const factoryName = use64 ? 'createFTDModule64' : 'createFTDModule';
            const variantId = use64 ? 'wasm64' : 'wasm32';
            this.artifactIdentityState = 'loading';
            if (!_wasmLoadPromise) {
                _wasmLoadPromise = installVerifiedFactory(variantId, factoryName)
                    .catch((error) => {
                        _wasmLoadPromise = null;
                        throw error;
                    });
            }
            const verified = await _wasmLoadPromise;
            if (verified.identity.variant.id !== variantId) {
                throw new Error('Cached WASM factory variant does not match the selected ABI');
            }
            this.artifactIdentity = verified.identity;
            this.artifactIdentityState = 'ready';
            this.artifactIdentityReady = Promise.resolve(verified.identity);
            this._module = await globalThis[factoryName]({
                wasmBinary: verified.moduleBytes,
                locateFile: (path) => 'wasm/' + path,
            });
            debugLog('[WasmBridge] loaded ' + (use64 ? 'wasm64 (Memory64, 8 GB heap)' : 'wasm32 (2 GB heap)'));
            // Every module-level scenario/injection function takes a
            // RenderBridge reference, so this bridge owns that concrete type.
            debugLog('[WasmBridge] init() - constructing initial RenderBridge with L =', this.latticeSize);
            try {
                this._bridge = new this._module.RenderBridge(this.latticeSize);
            } catch (err) {
                console.error('[WasmBridge] Fatal: failed to construct initial RenderBridge(' + this.latticeSize + '):', err);
                throw err;
            }
            this.ready = true;
            this._markScale0StateChanged(true);
            this.wasmLoadState = 'ready';
            debugLog('FTD WASM engine loaded successfully');
            return true;
        } catch (e) {
            this.wasmLoadState = 'failed';
            this.artifactIdentity = null;
            this.artifactIdentityState = 'failed';
            this.artifactIdentityReady = Promise.resolve(null);
            console.warn('WASM module not available:', e.message);
            return false;
        }
    }

    tick() {
        if (!this._bridge) return;
        this._bridge.tick();
        this._markScale0StateChanged();
    }
    run(n) {
        if (!this._bridge) return;
        this._bridge.run(n);
        this._markScale0StateChanged();
    }
    currentTick() { return this._bridge ? this._bridge.currentTick() : 0; }

    /**
     * Capture the canonical schema-versioned Scale-0 dynamical-state digest.
     *
     * Direct WASM is synchronous because the RenderBridge lives on this
     * thread. The uint64 hash lanes are already fixed-width lowercase hex
     * strings at the Embind boundary; JavaScript must never coerce them to
     * Number. This is an observer/provenance surface, not a claim that the
     * imposed `empty` null control is a physical vacuum.
     */
    captureDynamicalStateDigest() {
        const raw = _wasmCallOr(this, 'captureDynamicalStateDigest', null,
            (m, b) => m.captureDynamicalStateDigest(b));
        return normalizeDynamicalStateDigest(raw, 'direct');
    }
    getDynamicalStateDigest() { return this.captureDynamicalStateDigest(); }
    getScale0DynamicalStateDigest() { return this.getDynamicalStateDigest(); }
    getWasmArtifactIdentity() { return this.artifactIdentity; }

    setDt(dt) {
        if (this._module && this._bridge) {
            this._module.setDt(this._bridge, dt);
            this._markScale0StateChanged();
        }
    }
    getDt() {
        if (this._module && this._bridge) return this._module.getDt(this._bridge);
        return 1.0;
    }
    getPhysicalTime() {
        if (this._module && this._bridge) return this._module.getPhysicalTime(this._bridge);
        return 0.0;
    }
    // FTD-0271: de Broglie internal-clock frequency omega0 (KG mass term).
    setOmega0(w) {
        if (this._module && this._bridge && typeof this._module.setOmega0 === 'function') {
            this._module.setOmega0(this._bridge, w);
            this._markScale0StateChanged();
        }
    }
    getOmega0() {
        if (this._module && this._bridge && typeof this._module.getOmega0 === 'function')
            return this._module.getOmega0(this._bridge);
        return 1.0;
    }
    // Flux-cell mechanisms (engine/include/ftd/flux_cell.h, 2026-09-02). A
    // registered region makes getEnergyAudit carry the cell* ledger; the pump
    // and port are armed by the flux_pump / flux_cell_port toggles.
    _fluxCellCall(name, ...args) {
        if (this._module && this._bridge && typeof this._module[name] === 'function') {
            this._module[name](this._bridge, ...args);
            this._markScale0StateChanged();
        }
    }
    setFluxCellRegion(cx, cy, cz, radius) { this._fluxCellCall('setFluxCellRegion', cx, cy, cz, radius); }
    clearFluxCellRegion() { this._fluxCellCall('clearFluxCellRegion'); }
    setFluxPump(cx, cy, cz, majorRadius, tubeSigma, amplitude, circulationSign = 1, signSectors = 0, ticks = 20, period = 1) {
        this._fluxCellCall('setFluxPump', cx, cy, cz, majorRadius, tubeSigma, amplitude,
            circulationSign | 0, signSectors | 0, ticks | 0, Math.max(1, period | 0));
    }
    clearFluxPump() { this._fluxCellCall('clearFluxPump'); }
    setFluxCellPort(cx, cy, cz, nx, ny, nz, radius, openTick, surfaceOffset = 0) {
        this._fluxCellCall('setFluxCellPort', cx, cy, cz, nx, ny, nz, radius, openTick | 0, +surfaceOffset);
    }
    clearFluxCellPort() { this._fluxCellCall('clearFluxCellPort'); }
    // FTD-0274: live Langevin bath temperature (thermal-ignition panel).
    setLangevinTemp(t) {
        if (this._module && this._bridge && typeof this._module.setLangevinTemp === 'function') {
            this._module.setLangevinTemp(this._bridge, t);
            this._markScale0StateChanged();
        }
    }
    getLangevinTemp() {
        if (this._module && this._bridge && typeof this._module.getLangevinTemp === 'function')
            return this._module.getLangevinTemp(this._bridge);
    }
    setLangevinGamma(g) {
        if (this._module && this._bridge && typeof this._module.setLangevinGamma === 'function') {
            this._module.setLangevinGamma(this._bridge, g);
            this._markScale0StateChanged();
        }
    }
    getLangevinGamma() {
        if (this._module && this._bridge && typeof this._module.getLangevinGamma === 'function')
            return this._module.getLangevinGamma(this._bridge);
        return 0.0;
    }

    reset(latticeSize) {
        let size = parseInt(latticeSize || this.latticeSize, 10);
        if (isNaN(size) || size <= 0 || size > 257) {
            console.warn('[WasmBridge] Invalid reset latticeSize:', latticeSize, 'or current:', this.latticeSize, '- falling back to 33');
            size = 33;
        }
        // Odd lattices only — snap even N up to the next odd.
        if (size % 2 === 0) size += 1;
        this.latticeSize = size;
        debugLog('[WasmBridge] reset() called. latticeSize =', this.latticeSize);
        if (this._module) {
            // Drop sub-engine state on lattice reset. The native PE embind
            // instance is standalone (not bound to the RenderBridge) but a
            // reset wipes Scale-1 state by contract — dispose() frees the
            // embind heap object and the adapter lazily reconstructs.
            // Same logic for the AtomEngine handle, the JS-side AE engine
            // stub, and the lazy-attached physics harness.
            this._peEngine.dispose();
            if (this._ae) { try { this._ae.delete?.(); } catch {} this._ae = null; }
            this._lastScale0Audit = null;
            this._lastScale0AuditTick = -1;
            this._aeEngine = null; this._aeStub = null;
            delete this.__ftdPhysicsHarness__;

            // Delete the old bridge BEFORE allocating the new one so peak
            // memory stays at one bridge worth (not two). At L=96 a single
            // RenderBridge allocates ~325 MB; build-then-swap would peak
            // at ~650 MB and OOM the WASM heap.
            //
            // Trade-off: if `new RenderBridge` aborts (-fno-exceptions
            // converts std::bad_alloc into abort()), the WASM module is
            // permanently dead — but with MAXIMUM_MEMORY = 2 GB, abort
            // is unreachable for any sane lattice size.
            // Rebuild the concrete RenderBridge used by the module API.
            if (this._bridge) {
                debugLog('[WasmBridge] reset() - deleting old RenderBridge...');
                this._bridge.delete();
                this._bridge = null;
            }
            debugLog('[WasmBridge] reset() - constructing new RenderBridge with L =', this.latticeSize);
            try {
                this._bridge = new this._module.RenderBridge(this.latticeSize);
            } catch (err) {
                console.error('[WasmBridge] Fatal: failed to construct new RenderBridge(' + this.latticeSize + '):', err);
                throw err;
            }
            debugLog('[WasmBridge] reset() - RenderBridge constructed successfully.');
            this._markScale0StateChanged(true);
        }
    }

    /**
     * Tear down the WasmBridge: delete the C++ RenderBridge + sub-
     * engine handles, drop the JS-side AE-fallback MockBridge, drop
     * the lazy-attached harness. Idempotent. Symmetric with
     * MockBridge.dispose() (Bridge-H1 audit fix, 2026-04-27).
     */
    dispose() {
        this._peEngine.dispose();
        if (this._ae) { try { this._ae.delete?.(); } catch {} this._ae = null; }
        this._lastScale0Audit = null;
        this._lastScale0AuditTick = -1;
        this._aeEngine = null; this._aeStub = null;
        if (this._bridge) {
            try { this._bridge.delete(); } catch {}
            this._bridge = null;
        }
        delete this.__ftdPhysicsHarness__;
        this.ready = false;
        this._markScale0StateChanged(true);
    }

    // Revision 2.7: every injection method carries the same try-catch guard
    // injectFlux always had. A BindingError from a bad argument (or a
    // scenario bug) now logs instead of unwinding through the scenario
    // loader — the asymmetry looked intentional but was accretion. NOTE: no
    // heap-death recovery is attempted anywhere; ftd_core builds with
    // -fno-exceptions, so a WASM abort() stays permanent by design.
    injectParticle(x, y, z, state, fx = 0, fy = 0, fz = 0) {
        if (!(this._module && this._bridge)) return;
        try {
            this._module.injectParticle(this._bridge, x, y, z, state, fx, fy, fz);
            this._invalidateScale0AuditCache();
        } catch (e) {
            try {
                this._module.injectParticle(this._bridge, x, y, z, state);
                this._invalidateScale0AuditCache();
            } catch (e2) {
                console.error('WASM injectParticle failed:', e2);
            }
        }
    }

    injectWaveVel(x, y, z, vx, vy, vz) {
        if (!(this._module && this._bridge && typeof this._module.injectWaveVel === 'function')) return;
        try {
            this._module.injectWaveVel(this._bridge, x, y, z, vx, vy, vz);
            this._invalidateScale0AuditCache();
        } catch (e) {
            console.error('WASM injectWaveVel failed:', e);
        }
    }
    injectWavepacket(x, y, z, state) {
        if (!(this._module && this._bridge)) return;
        try {
            this._module.injectWavepacket(this._bridge, x, y, z, state);
            this._invalidateScale0AuditCache();
        } catch (e) {
            console.error('WASM injectWavepacket failed:', e);
        }
    }

    injectFlux(x, y, z, fx, fy, fz) {
        if (!this._bridge) return;
        try {
            this._module.injectFlux(this._bridge, x, y, z, fx, fy, fz);
            this._invalidateScale0AuditCache();
        } catch (e) {
            console.error('WASM injectFlux failed:', e);
        }
    }

    injectUniformFluxAdd(fx, fy, fz) {
        if (!this._bridge) return;
        try {
            if (typeof this._module.injectUniformFluxAdd === 'function') {
                this._module.injectUniformFluxAdd(this._bridge, fx, fy, fz);
                this._invalidateScale0AuditCache();
            } else {
                console.warn('WASM injectUniformFluxAdd not found. Did you rebuild?');
            }
        } catch (e) {
            console.error('WASM injectUniformFluxAdd failed:', e);
        }
    }

    createEntangledPair(x, y, z, fx, fy, fz) {
        if (!(this._module && this._bridge)) return;
        try {
            this._module.createEntangledPair(this._bridge, x, y, z, fx, fy, fz);
            this._invalidateScale0AuditCache();
        } catch (e) {
            console.error('WASM createEntangledPair failed:', e);
        }
    }

    clearField() {
        if (this._module && this._bridge && typeof this._module.clearField === 'function') {
            this._module.clearField(this._bridge);
            this._invalidateScale0AuditCache();
        }
    }

    seedRandomFlux() {
        if (this._module && this._bridge && typeof this._module.seedRandomFlux === 'function') {
            this._module.seedRandomFlux(this._bridge);
            this._invalidateScale0AuditCache();
        }
    }

    setToggle(name, value) {
        if (this._module && this._bridge) {
            this._module.setToggle(this._bridge, name, value);
            this._invalidateScale0AuditCache();
        }
    }

    /** Apply a dependency-ordered toggle profile with one cache invalidation. */
    setToggles(entries) {
        if (!this._module || !this._bridge || !Array.isArray(entries)) return;
        for (const entry of entries) {
            if (!Array.isArray(entry) || typeof entry[0] !== 'string') continue;
            this._module.setToggle(this._bridge, entry[0], !!entry[1]);
        }
        this._enforceToggleInvariants?.();
        this._invalidateScale0AuditCache();
    }

    getToggle(name) {
        if (this._module && this._bridge)
            return this._module.getToggle(this._bridge, name);
        // Unknown / unbound: OFF. Never default true — that lies to UI sync
        // and overlay applicability about terms the engine is not running.
        return false;
    }

    getParticleData() {
        if (!this._module || !this._bridge) return EMPTY_PARTICLE_DATA;
        const raw = this._module.getParticleData(this._bridge);
        if (!raw || raw.count === 0) return EMPTY_PARTICLE_DATA;
        // raw.* are embind typed_memory_views straight into the WASM heap; the
        // next tick()/inject*() overwrites them in place (or a heap-growth
        // detaches them), so returning them directly is a reuse-before-consume
        // hazard AND made this in-thread path behave differently from the worker
        // path (which copies — wasm-bridge.worker.js). Copy into grow-in-place
        // scratch and hand back length-exact subarrays: correct + effectively
        // allocation-free (the backing buffers are reused frame-to-frame).
        const n = raw.count;
        const c = this._pdScratch || (this._pdScratch = { cap: 0 });
        if (c.cap < n) {
            c.cap = n;
            c.positions = new Float32Array(n * 3);
            c.colors = new Float32Array(n * 3);
            c.sizes = new Float32Array(n);
            c.spin = new Float32Array(n);
            c.colorCharge = new Float32Array(n);
            c.locked = new Uint8Array(n);
        }
        c.positions.set(raw.positions.subarray(0, n * 3));
        c.colors.set(raw.colors.subarray(0, n * 3));
        c.sizes.set(raw.sizes.subarray(0, n));
        c.spin.set(raw.spin.subarray(0, n));
        c.colorCharge.set(raw.colorCharge.subarray(0, n));
        c.locked.set(raw.locked.subarray(0, n));
        return {
            positions: c.positions.subarray(0, n * 3),
            colors: c.colors.subarray(0, n * 3),
            sizes: c.sizes.subarray(0, n),
            spin: c.spin.subarray(0, n),
            colorCharge: c.colorCharge.subarray(0, n),
            locked: c.locked.subarray(0, n),
            count: n,
        };
    }

    getKnotTelemetry() {
        if (!this._module || !this._bridge) return EMPTY_KNOT_TELEMETRY;
        const r = this._module.getKnotTelemetry(this._bridge);
        return r || EMPTY_KNOT_TELEMETRY;
    }
    getKnotEvents() {
        if (!this._module || !this._bridge) return EMPTY_KNOT_EVENTS;
        const r = this._module.getKnotEvents(this._bridge);
        return r || EMPTY_KNOT_EVENTS;
    }
    getKnotAggregate() {
        if (!this._module || !this._bridge) return EMPTY_KNOT_AGG;
        const r = this._module.getKnotAggregate(this._bridge);
        return r || EMPTY_KNOT_AGG;
    }

    getScale0ParticleList() {
        // Shared derivation (bridge-contract.js) so this and WasmBridgeProxy
        // cannot drift apart.
        return particleDataToList(this.getParticleData());
    }

    getDiagnostics() {
        if (!this._module || !this._bridge) return null;
        let d;
        if (typeof this._module.getDiagnosticsView === 'function') {
            const arr = this._module.getDiagnosticsView(this._bridge);
            d = {
                tick: arr[0],
                physicalTime: arr[1],
                dt: arr[2],
                manifested: arr[3],
                positive: arr[4],
                negative: arr[5],
                totalFlux: arr[6],
                totalEnergy: arr[7],
                maxBandwidth: arr[8],
                avgDrag: arr[9],
                entropy: arr[10],
                chargeBalance: arr[11],
                spinUp: arr[12],
                spinDown: arr[13],
                colorless: arr[14],
                colorRed: arr[15],
                colorGreen: arr[16],
                colorBlue: arr[17],
                angMomX: arr[18],
                angMomY: arr[19],
                angMomZ: arr[20],
                maxCausalBudget: arr[21],
                causalProjectionEvents: arr[22],
                // The compact ABI currently ends at lane 22. Field-spin and
                // center-clock values are intentionally absent until C++ and
                // JS extend the view atomically; they must not appear as zeros.
            };
        } else {
            d = this._module.getDiagnostics(this._bridge);
        }
        // The engine's EnergyLedger is updated once per completed tick and its
        // E_curr channel is exactly the rest-offset-free dynamic sum used by
        // the status bar/core chart. Reading the cached scalar avoids coupling
        // these always-on surfaces to the full O(N^3) EnergyAudit.
        const ledger = typeof this._module.getEnergyLedger === 'function'
            ? this._module.getEnergyLedger(this._bridge) : null;
        if (d && Number.isFinite(ledger?.ECurr)) {
            if (!Object.hasOwn(d, 'vacuumBaselineEnergy')) {
                d.vacuumBaselineEnergy = d.totalEnergy;
            }
            d.dynamicEnergy = ledger.ECurr;
            d.totalEnergy = ledger.ECurr;
            d.energySampleSource = 'per-tick-ledger';
        }
        const audit = this._getScale0AuditForTick(d?.tick ?? this.currentTick());
        if (audit && Number.isFinite(audit.dynamicEnergy)) {
            // Conservation charts use the rest-offset-free accounted channel.
            // Keep rest and total accounted energy visible as separate fields.
            d.vacuumBaselineEnergy = d.totalEnergy;
            d.accountedEnergy = audit.totalEnergy;
            d.restEnergy = audit.particleRestEnergy;
            // Status-bar decomposition (whole-box channels, sim units) —
            // mirrors wasm-bridge.worker.js postFrame().
            d.fieldEnergy = audit.fieldEnergy;
            d.waveEnergy = audit.waveEnergy;
            d.particleKE = audit.particleKE;
        }
        return d;
    }

    getEnergyAudit() {
        if (!this._module || !this._bridge) return null;
        return this._getScale0AuditForTick(this.currentTick());
    }

    _getScale0AuditForTick(tick) {
        if (!this._module || !this._bridge) return null;
        const t = Number.isFinite(tick) ? tick : this.currentTick();
        if (this._lastScale0Audit && this._lastScale0AuditTick === t) {
            return this._lastScale0Audit;
        }
        let audit;
        if (typeof this._module.getEnergyAuditView === 'function') {
            const arr = this._module.getEnergyAuditView(this._bridge);
            audit = {
                // NAMING (see engine/src/diagnostics_compute.cpp): fieldEnergy is
                // flux POTENTIAL energy ½Σ|J|² (NOT E-field energy). EFieldEnergy is
                // byte-identical to waveEnergy by construction (E = -wave_vel);
                // BFieldEnergy carries the (c²/2) weight. Don't read fieldEnergy vs
                // BFieldEnergy as |E|² vs |B|².
                fieldEnergy: arr[0],
                waveEnergy: arr[1],
                particleKE: arr[2],
                totalEnergy: arr[3],
                EFieldEnergy: arr[4],
                BFieldEnergy: arr[5],
                totalPoynting: { x: arr[6], y: arr[7], z: arr[8] },
                gaussViolation: arr[9],
                maxGaussError: arr[10],
                selfFieldInjection: arr[11],
                coulombPE: arr[12],
                ELTotal: arr[13],
                ERTotal: arr[14],
                chiralityTotal: arr[15],
                wvLTotal: arr[16],
                wvRTotal: arr[17],
                chargeTotal: arr[18],
                particleRestEnergy: arr[19],
                particleEnergy: arr[20],
                particleMomentum: arr.length > 23
                    ? { x: arr[21], y: arr[22], z: arr[23] } : undefined,
                dynamicEnergy: arr[24],
                cellVolume: arr[25] ?? VOXEL_VOLUME,
                fieldEnergyDensitySum: arr[26],
                waveEnergyDensitySum: arr[27],
                manifested: arr[28],
                strongEnergy: arr[29],
                weakEnergy: arr[30],
                // Append-only flux-cell ledger (2026-09-02): indices 31..45.
                cellSiteCount: arr.length > 44 ? arr[31] : undefined,
                cellUE: arr.length > 44 ? arr[32] : undefined,
                cellUB: arr.length > 44 ? arr[33] : undefined,
                cellUJ: arr.length > 44 ? arr[34] : undefined,
                cellHWave: arr.length > 44 ? arr[35] : undefined,
                cellPLeak: arr.length > 44 ? arr[36] : undefined,
                cellSNet: arr.length > 44 ? { x: arr[37], y: arr[38], z: arr[39] } : undefined,
                cellPumpWork: arr.length > 44 ? arr[40] : undefined,
                cellPumpTicksApplied: arr.length > 44 ? arr[41] : undefined,
                cellPumpTicksTotal: arr.length > 44 ? arr[42] : undefined,
                cellPortOpen: arr.length > 44 ? arr[43] : undefined,
                cellPortWorkOut: arr.length > 44 ? arr[44] : undefined,
                // Append-only 2026-09-02 Poynting cross-check.
                cellPortPoyntingOut: arr.length > 45 ? arr[45] : undefined,
            };
        } else {
            audit = this._module.getEnergyAudit(this._bridge);
        }
        this._lastScale0Audit = audit;
        this._lastScale0AuditTick = t;
        return audit;
    }

    _markScale0StateChanged(sourceBoundary = false) {
        this._lastScale0Audit = null;
        this._lastScale0AuditTick = -1;
        if (sourceBoundary) {
            this._scale0TelemetrySourceEpoch = Number.isFinite(this._scale0TelemetrySourceEpoch)
                ? this._scale0TelemetrySourceEpoch + 1 : 1;
        }
        this._scale0TelemetryStateVersion = Number.isFinite(this._scale0TelemetryStateVersion)
            ? this._scale0TelemetryStateVersion + 1 : 1;
        if (!(this._scale0TelemetryGroupMeta instanceof Map)) {
            this._scale0TelemetryGroupMeta = new Map();
        } else {
            this._scale0TelemetryGroupMeta.clear();
        }
    }

    _invalidateScale0AuditCache() {
        this._markScale0StateChanged();
    }

    getScale0TelemetryGroupMeta(group) {
        if (!(this._scale0TelemetryGroupMeta instanceof Map)) {
            this._scale0TelemetryGroupMeta = new Map();
        }
        const sourceEpoch = Number.isFinite(this._scale0TelemetrySourceEpoch)
            ? this._scale0TelemetrySourceEpoch : 0;
        const stateVersion = Number.isFinite(this._scale0TelemetryStateVersion)
            ? this._scale0TelemetryStateVersion : 0;
        const available = !!(this._module && this._bridge && this.ready !== false);
        const status = available ? 'available' : 'unavailable';
        let meta = this._scale0TelemetryGroupMeta.get(group);
        if (!meta || meta.sourceEpoch !== sourceEpoch
            || meta.stateVersion !== stateVersion || meta.status !== status) {
            meta = {
                backend: 'wasm-main',
                sourceEpoch,
                stateVersion,
                snapshotVersion: stateVersion,
                sampleTick: available ? this.currentTick() : null,
                tick: available ? this.currentTick() : null,
                status,
                stale: !available,
                receivedAt: (typeof performance !== 'undefined'
                    && typeof performance.now === 'function') ? performance.now() : Date.now(),
            };
            this._scale0TelemetryGroupMeta.set(group, meta);
        }
        return { ...meta };
    }

    getLagrangian() {
        if (!this._module || !this._bridge) return null;
        if (typeof this._module.getLagrangianView === 'function') {
            const arr = this._module.getLagrangianView(this._bridge);
            return {
                fieldKinetic: arr[0],
                fieldGradient: arr[1],
                bornInfeld: arr[2],
                coupling: arr[3],
                velocity: arr[4],
                gauss: arr[5],
                dissipation: arr[6],
                total: arr[7],
                hamiltonian: arr[8],
                totalAction: arr[9],
                gaussViolation: arr[10],
                maxGaussError: arr[11],
                totalFluxMag: arr[12],
                totalWaveEnergy: arr[13],
                manifested: arr[14],
                locked: arr[15],
                cellVolume: arr[16] ?? VOXEL_VOLUME
            };
        }
        return this._module.getLagrangian(this._bridge);
    }

    getConstants() {
        if (!this._module) return null;
        return this._module.getConstants();
    }

    inspectVoxel(x, y, z) {
        if (!this._module || !this._bridge) return null;
        return this._module.inspectVoxel(this._bridge, x, y, z);
    }

    getForceAt(x, y, z) {
        if (!this._module || !this._bridge) return null;
        return this._module.getForceAt(this._bridge, x, y, z);
    }

    /**
     * Seed a Scale-0 scenario. Returns false if the WASM module reports the
     * name was not handled (unknown id). Older WASM builds that return
     * undefined are treated as success only when no exception is thrown.
     */
    setupScenario(name, _harness) {
        this.reset();
        if (this._module && this._bridge) {
            const result = this._module.setupScenario(this._bridge, name);
            this._enforceToggleInvariants();
            if (result === false) return false;
            this._markScale0StateChanged();
            return true;
        }
        return false;
    }

    // After a C++ setupScenario, clamp any TermToggles `requires` dependent that
    // is ON while its prerequisite is OFF. reset() rebuilds the RenderBridge at
    // C++ defaults (selective_damping=true, damping=true) and some scenario
    // setups then turn a prerequisite off (e.g. quantum-well / thomson set
    // damping=false) WITHOUT clearing the dependent, leaving an invalid combo
    // that bursts "[TermToggles] Invalid combination: <dep> requires <prereq>"
    // on every tick. This reads the bridge's ACTUAL post-setup toggle state via
    // getToggle and corrects it. Physics-neutral: a dependent whose prerequisite
    // is off is already a no-op in the engine (that is why the C++ guard rejects
    // the combo). Mirrors the same enforcement in wasm-bridge.worker.js.
    _enforceToggleInvariants() {
        const m = this._module, b = this._bridge;
        if (!m || !b || typeof m.getToggle !== 'function' || typeof m.setToggle !== 'function') return;
        for (const [dep, prereq] of TOGGLE_REQUIRES) {
            try {
                if (m.getToggle(b, dep) && !m.getToggle(b, prereq)) m.setToggle(b, dep, false);
            } catch { /* unknown toggle name in this build — skip */ }
        }
    }

    // ── Flux Data Extraction (Scale 0 substrate) ──────────────────────
    // Shared empty-buffer fallback is cheap (new Float64Array(0)) but kept
    // per-call instead of a module singleton because some callers mutate
    // their result via an overlay buffer view.
    getFluxSlice(axis, index) {
        return _wasmCallOr(this, 'getFluxSlice', new Float64Array(0),
            (m, b) => m.getFluxSlice(b, axis, index));
    }
    getFluxVolume() {
        return _wasmCallOr(this, 'getFluxVolume', new Float64Array(0),
            (m, b) => m.getFluxVolume(b));
    }

    // Phase 2 gravity panel: engine Poisson-derived [IMPOSED] voxel.latency
    // mapping, not the |J|² proxy or a recovered physical spacetime metric.
    getGravityMetricAgg() {
        return _wasmCallOr(this, 'getGravityMetricAgg',
            { active: false, requested: null, latencyMax: 0, latencyMean: 0, fMin: 1, gammaMax: 1, dilationMaxPct: 0, voxelCount: 0 },
            (m, b) => m.getGravityMetricAgg(b));
    }

    getLatencyVolume() {
        return _wasmCallOr(this, 'getLatencyVolume', new Float64Array(0),
            (m, b) => m.getLatencyVolume(b));
    }

    // ── Bulk Vector Field Exports (Scale 0 field visualization) ──────
    // RF-6: the two-line module-presence / method-presence guard is factored
    // into _wasmCallOr. When the WASM module or a specific method is missing,
    // these return the shared frozen EMPTY_FIELD_SAMPLE / EMPTY_SCALAR_SAMPLE
    // singletons (safe because the typed arrays inside are zero-length and
    // thus immutable). Saves ~80 LOC of copy-paste vs the pre-RF-6 shape.
    getEFieldSampled(stride = 2) {
        return _wasmCallOr(this, 'getEFieldSampled', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getEFieldSampled(b, stride));
    }
    /**
     * Sample the engine's Coulomb potential field along a ray.
     * Returns { positions, V, count } via zero-copy typed_memory_view.
     * Falls back to {count: 0} when WASM doesn't support it (older
     * builds) — caller should detect and use JS-side getEFieldSampled
     * + interpolation as a fallback.
     */
    sampleVAtRay(x1, y1, z1, x2, y2, z2, n) {
        return _wasmCallOr(this, 'sampleVAtRay', { positions: new Float32Array(0), V: new Float32Array(0), count: 0 },
            (m, b) => m.sampleVAtRay(b, x1, y1, z1, x2, y2, z2, n));
    }
    getBFieldSampled(stride = 2) {
        return _wasmCallOr(this, 'getBFieldSampled', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getBFieldSampled(b, stride));
    }
    getPoyntingSampled(stride = 2) {
        return _wasmCallOr(this, 'getPoyntingSampled', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getPoyntingSampled(b, stride));
    }
    getDivJSampled(stride = 2) {
        return _wasmCallOr(this, 'getDivJSampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getDivJSampled(b, stride));
    }
    getFluxVectorSampled(stride = 2) {
        return _wasmCallOr(this, 'getFluxVectorSampled', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getFluxVectorSampled(b, stride));
    }
    getForceFieldSampled(stride = 2) {
        return _wasmCallOr(this, 'getForceFieldSampled', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getForceFieldSampled(b, stride));
    }
    // Vorticity / helicity / curlJ proxies. The MockBridge exposes these
    // via mock-lattice-samplers; the overlay dispatcher (~line 1996)
    // optional-chains them, so before this proxy existed every WASM-mode
    // session silently returned empty samples for these three overlays.
    // _wasmCallOr returns the EMPTY_*_SAMPLE singleton when the native
    // method is absent, which matches the dispatcher's optional-chain
    // fallback shape — so adding the proxy now is strictly an upgrade:
    // if WASM exposes the method later, overlays light up automatically.
    getVorticitySampled(stride = 2) {
        return _wasmCallOr(this, 'getVorticitySampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getVorticitySampled(b, stride));
    }
    getHelicitySampled(stride = 2) {
        return _wasmCallOr(this, 'getHelicitySampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getHelicitySampled(b, stride));
    }
    getCurlJSampled(stride = 2) {
        return _wasmCallOr(this, 'getCurlJSampled', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getCurlJSampled(b, stride));
    }
    // Scalar / derived field samplers (2026-06-03) — bound natively in
    // bindings_render_bridge.cpp. Before binding these fell back to EMPTY,
    // so Curvature K / Horizon / Fisher / Coherence and the new State-field
    // overlay rendered nothing on WASM-owned scenarios.
    getCoherenceSampled(stride = 2) {
        return _wasmCallOr(this, 'getCoherenceSampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getCoherenceSampled(b, stride));
    }
    getFisherSampled(stride = 2) {
        return _wasmCallOr(this, 'getFisherSampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getFisherSampled(b, stride));
    }
    getLatencySampled(stride = 2) {
        return _wasmCallOr(this, 'getLatencySampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getLatencySampled(b, stride));
    }
    getPoissonLatencySampled(stride = 2) {
        return _wasmCallOr(this, 'getPoissonLatencySampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getPoissonLatencySampled(b, stride));
    }
    getKretschmannSampled(stride = 2) {
        return _wasmCallOr(this, 'getKretschmannSampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getKretschmannSampled(b, stride));
    }
    getStateFieldSampled(stride = 2) {
        return _wasmCallOr(this, 'getStateFieldSampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getStateFieldSampled(b, stride));
    }
    getGaussResidualSampled(stride = 1) {
        return _wasmCallOr(this, 'getGaussResidualSampled', EMPTY_SCALAR_SAMPLE,
            (m, b) => m.getGaussResidualSampled(b, stride));
    }
    /** Kind-dispatched Scale-0 field sampler; see bridge-contract.js samplerOr. */
    getSamplerOr(kind, stride = 2, fallback) { return samplerOr(this, kind, stride, fallback); }
    // Direct WASM samplers are synchronous: an empty record is a completed
    // scientific zero/unavailable result, never a lazy transport placeholder.
    hasSamplerSnapshot() { return true; }
    getSamplerSnapshotVersion() { return null; }
    replaceSamplerWants() {}
    unwantSampler() {}

    // Force-field decomposition samplers (2026-04-19). Delegated to native
    // C++ implementations in ftd_wasm.cpp (see get_gravity_field_sampled,
    // get_em_force_field, get_strong_force_field). These return per-voxel
    // force vectors for each physical interaction, used by the viewport's
    // force-arrow overlays. getGravityForceField is an alias kept for the
    // overlay dispatcher that uses the "gravity" key name.
    getGravityFieldSampled(stride = 2) {
        return _wasmCallOr(this, 'getGravityFieldSampled', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getGravityFieldSampled(b, stride));
    }
    getEMForceField(stride = 2) {
        return _wasmCallOr(this, 'getEMForceField', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getEMForceField(b, stride));
    }
    getGravityForceField(stride = 2) { return this.getGravityFieldSampled(stride); }
    getStrongForceField(stride = 2) {
        return _wasmCallOr(this, 'getStrongForceField', EMPTY_FIELD_SAMPLE,
            (m, b) => m.getStrongForceField(b, stride));
    }

    // ── ParticleEngine (Scale 1) — native C++/WASM backend ───────────
    // Every pe* call forwards to the embind adapter over the native
    // ParticleEngine (G_PE = G_DERIVED, FTD-0131). See
    // ./native-particle-engine.js for the adapter contract.
    initPE()                                                         { return this._peEngine.initPE(); }
    resetPE()                                                        { return this._peEngine.resetPE(); }
    peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) { return this._peEngine.peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff); }
    peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff = 0.1) { return this._peEngine.peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff); }
    peApplyEquilibriumOrbit(particleId, options = {}) { return this._peEngine.peApplyEquilibriumOrbit(particleId, options); }
    peApplyEquilibriumOrbitBatch(entries) { return this._peEngine.peApplyEquilibriumOrbitBatch?.(entries); }
    peScaleVelocity(particleId, scale) { return this._peEngine.peScaleVelocity(particleId, scale); }
    peTick()                                                         { return this._peEngine.peTick(); }
    peGetTick()                                                      { return this._peEngine.peGetTick(); }
    peGetObservationRevision()                                      { return this._peEngine.peGetObservationRevision(); }
    peGetParticleData()                                              { return this._peEngine.peGetParticleData(); }
    peGetFieldSources()                                              { return this._peEngine.peGetFieldSources(); }
    peGetForces()                                                    { return this._peEngine.peGetForces(); }
    peGetForceDecomposition()                                        { return this._peEngine.peGetForceDecomposition(); }
    peGetDiagnostics()                                               { return this._peEngine.peGetDiagnostics(); }
    peGetExtendedData()                                              { return this._peEngine.peGetExtendedData(); }
    peGetSnapshot(scenario = '')                                     { return this._peEngine.peGetSnapshot(scenario); }
    peGetNativeMatterReplay()                                        { return this._peEngine.peGetNativeMatterReplay(); }
    peUseRegisteredM3Replay()                                        { return this._peEngine.peUseRegisteredM3Replay(); }
    peObserveSourceClusters(payload)                                 { return this._peEngine.peObserveSourceClusters(payload); }
    peGetPhysicsRegistry()                                           { return this._peEngine.peGetPhysicsRegistry(); }
    peSetMode(mode)                                                   { return this._peEngine.peSetMode(mode); }
    peSetDt(dt)                                                      { return this._peEngine.peSetDt(dt); }
    peGetDt()                                                        { return this._peEngine.peGetDt(); }
    peSetSoftening(s)                                                { return this._peEngine.peSetSoftening(s); }
    peConfigureInsulatingBox(cx, cy, cz, hx, hy, hz)                 { return this._peEngine.peConfigureInsulatingBox(cx, cy, cz, hx, hy, hz); }
    peAddInsulatingPort(axis, side, centerU, centerV, halfU, halfV, requiredChargeSign = 0, crossingDirection = 0) { return this._peEngine.peAddInsulatingPort(axis, side, centerU, centerV, halfU, halfV, requiredChargeSign, crossingDirection); }
    peClearInsulatingBox()                                           { return this._peEngine.peClearInsulatingBox(); }
    peSetCoulomb(e)                                                  { return this._peEngine.peSetCoulomb(e); }
    peSetDamping(e)                                                  { return this._peEngine.peSetDamping(e); }
    peSetGravity(e)                                                  { return this._peEngine.peSetGravity(e); }
    peSetLorentz(e)                                                  { return this._peEngine.peSetLorentz(e); }
    peSetExchange(e)                                                 { return this._peEngine.peSetExchange(e); }
    peSetStrong(e)                                                   { return this._peEngine.peSetStrong(e); }
    peSetMagneticDipole(e)                                           { return this._peEngine.peSetMagneticDipole(e); }
    peSetSpinOrbit(e)                                                { return this._peEngine.peSetSpinOrbit(e); }
    peSetSpinAxis(id, ax, ay, az)                                    { return this._peEngine.peSetSpinAxis(id, ax, ay, az); }
    peSetRadiation(e)                                                { return this._peEngine.peSetRadiation(e); }
    peSetRelativistic(e)                                             { return this._peEngine.peSetRelativistic(e); }
    peSetRelativisticVerlet(e)                                       { return this._peEngine.peSetRelativisticVerlet(e); }
    peSetContactEvents(e)                                            { return this._peEngine.peSetContactEvents(e); }
    peSetToggle(name, value)                                         { return this._peEngine.peSetToggle(name, value); }
    peGetToggle(name)                                                { return this._peEngine.peGetToggle(name); }
    peGetBackendCapabilities()                                       { return this._peEngine.peGetBackendCapabilities(); }
    peParticleCount()                                                { return this._peEngine.peParticleCount(); }
    peClear()                                                        { return this._peEngine.peClear(); }
    peExportCheckpoint()                                             { return this._peEngine.peExportCheckpoint?.(); }
    peRestoreCheckpoint(checkpoint)                                  { return this._peEngine.peRestoreCheckpoint?.(checkpoint); }
    peConfigureFinitePortBattery(size, capacity, chargeAmplitude, batteryAmplitude) { return this._peEngine.peConfigureFinitePortBattery?.(size, capacity, chargeAmplitude, batteryAmplitude); }
    peStepFinitePortBattery()                                        { return this._peEngine.peStepFinitePortBattery?.(); }
    peReverseFinitePortBattery()                                     { return this._peEngine.peReverseFinitePortBattery?.(); }
    peGetFinitePortBatterySnapshot()                                 { return this._peEngine.peGetFinitePortBatterySnapshot?.(); }
    peGetParticleTypes()                                             { return this._peEngine.peGetParticleTypes(); }
    peInspectParticle(id)                                            { return this._peEngine.peInspectParticle(id); }

    // ── Boundary containment ─────────────────────────────────────────
    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        if (this._aeStub) this._aeStub._boundaryShape = shape;
    }

    // 0 = Periodic, 1 = Reflective, 2 = Dispersal
    setFluxBoundaryMode(mode) {
        const normalized = Math.max(0, Math.min(2, Math.trunc(Number(mode) || 0)));
        this._fluxBoundaryMode = normalized;
        if (this._module && this._bridge && typeof this._module.setFluxBoundary === 'function') {
            this._module.setFluxBoundary(this._bridge, normalized);
            this._markScale0StateChanged();
        }
    }

    // Orientation metadata: 0=X/lateral, 1=Y/vertical, 2=Z/forward-aft,
    // 3=show all axes. Boundary coverage is always controlled by the mode.
    setFluxPeriodicAxis(axis) {
        const normalized = Math.max(0, Math.min(3, Math.trunc(Number(axis) || 0)));
        this._fluxPeriodicAxis = normalized;
        if (this._module && this._bridge
            && typeof this._module.setFluxPeriodicAxis === 'function') {
            this._module.setFluxPeriodicAxis(this._bridge, normalized);
            this._markScale0StateChanged();
        }
    }

    getFluxPeriodicAxis() {
        if (this._module && this._bridge
            && typeof this._module.getFluxPeriodicAxis === 'function') {
            return this._module.getFluxPeriodicAxis(this._bridge);
        }
        return this._fluxPeriodicAxis ?? 2;
    }

    setReflectiveBoundary(on) {
        // Legacy path: map bool → flux boundary mode
        this.setFluxBoundaryMode(on ? 1 : 2);
        if (this._aeStub) this._aeStub._reflectiveBoundary = on;
    }

    // ── AtomEngine (Scale 2) WASM ─────────────────────────────────────
    // Hosts the JS AtomEngine without MockBridge by creating a minimal stub
    // that satisfies the createAtomEngine(state) contract. Returns the
    // engine object directly so all ae* call sites work unchanged.
    _ensureAEFallback() {
        if (this._aeEngine) return this._aeEngine;
        const bs = this._boundaryShape || 'cube';
        const stub = {
            _boundaryShape: bs,
            _reflectiveBoundary: false,
            setBoundaryShape(s) { this._boundaryShape = s; },
            setFluxBoundaryMode() {},
            setReflectiveBoundary(on) { this._reflectiveBoundary = on; },
            _reflectIntoBoundary(p, cx, cy, cz, R) {
                reflectIntoBoundary(this._boundaryShape, p, cx, cy, cz, R, this._reflectiveBoundary);
            },
        };
        this._aeStub = stub;
        this._aeEngine = createAtomEngine(stub);
        return this._aeEngine;
    }

    get _aeHasWasm() {
        // Deliberately disabled (audit P1-2, deferred feature D-11). The
        // compiled WASM AtomEngine may exist, but it works in Planck units
        // internally, whereas the web UI molecule/atom data is in
        // Bohr-scaled simulation units. Enabling it without the JS↔WASM
        // scale-conversion shim would silently corrupt every Scale-2/3
        // readout, so we force the MockBridge JS fallback below until the
        // conversion layer lands. Do NOT flip this to a binding probe
        // before that shim exists.
        return false;
        // return this._module && typeof this._module.AtomEngine === 'function';
    }

    initAE() {
        if (this._aeHasWasm) {
            if (this._ae) this._ae.delete();
            this._ae = new this._module.AtomEngine();
        } else {
            this._ensureAEFallback().initAE();
        }
    }

    resetAE() {
        if (this._aeHasWasm && this._module && this._ae) {
            this._module.aeClear(this._ae);
        } else {
            this._ensureAEFallback().resetAE();
        }
    }

    aeAddAtom(Z, x, y, z, vx = 0, vy = 0, vz = 0, charge = 0, N = -1) {
        if (this._aeHasWasm) {
            if (!this._ae) this.initAE();
            return this._module.aeAddAtom(this._ae, Z, x, y, z, vx, vy, vz, charge, N);
        }
        return this._ensureAEFallback().aeAddAtom(Z, x, y, z, vx, vy, vz, charge, N);
    }

    aeAddLockedAtom(Z, x, y, z, charge = 0, N = -1) {
        if (this._aeHasWasm) {
            if (!this._ae) this.initAE();
            return this._module.aeAddLockedAtom(this._ae, Z, x, y, z, charge, N);
        }
        return this._ensureAEFallback().aeAddLockedAtom(Z, x, y, z, charge, N);
    }

    aeCreateBond(idA, idB, order = 1) {
        if (this._aeHasWasm && this._module && this._ae) {
            this._module.aeCreateBond(this._ae, idA, idB, order);
        } else {
            this._ensureAEFallback().aeCreateBond(idA, idB, order);
        }
    }

    aeTick() {
        if (this._aeHasWasm && this._ae) {
            this._ae.tick();
            return true;
        } else {
            return this._ensureAEFallback().aeTick();
        }
    }

    aeGetAtomData() {
        if (this._aeHasWasm && this._module && this._ae) {
            return this._module.getAEAtomData(this._ae);
        }
        return this._ensureAEFallback().aeGetAtomData();
    }

    aeGetDiagnostics() {
        if (this._aeHasWasm && this._module && this._ae) {
            return this._module.getAEDiagnostics(this._ae);
        }
        return this._ensureAEFallback().aeGetDiagnostics();
    }

    aeGetFieldSources() {
        return this._ensureAEFallback().aeGetFieldSources();
    }

    // Per-atom force decomposition (ionic/vdW/bond/net) for the Scale 2/3
    // force-arrow overlays. JS-fallback-only (like aeGetFieldSources) — the
    // WASM AtomEngine path is disabled (see _aeHasWasm). This forward was
    // MISSING until 2026-06-10: on the default WASM page every force-arrow
    // toggle made animateAE throw a TypeError each compute frame (B10).
    aeGetForceDecomposition(want) {
        return this._ensureAEFallback().aeGetForceDecomposition(want);
    }

    aeGetRuntimeState() {
        return this._ensureAEFallback().aeGetRuntimeState();
    }
    aeConfigureNuclearReaction(channelId) {
        return this._ensureAEFallback().aeConfigureNuclearReaction(channelId);
    }
    aeSetNuclearEnvironment(patch) {
        return this._ensureAEFallback().aeSetNuclearEnvironment(patch);
    }
    aeInjectNuclearParticle(kind) {
        return this._ensureAEFallback().aeInjectNuclearParticle(kind);
    }
    aeGetNuclearDiagnostics() {
        return this._ensureAEFallback().aeGetNuclearDiagnostics();
    }
    aeGetNuclearVisuals() {
        return this._ensureAEFallback().aeGetNuclearVisuals();
    }

    aeGetVelocities()  { return this._ensureAEFallback().aeGetVelocities(); }
    aeGetDipoles()     { return this._ensureAEFallback().aeGetDipoles(); }
    aeGetHBondPairs()  { return this._ensureAEFallback().aeGetHBondPairs(); }

    aeSetDt(dt) {
        if (this._aeHasWasm && this._module && this._ae) this._module.aeSetDt(this._ae, dt);
        else this._ensureAEFallback().aeSetDt(dt);
    }
    aeGetDt() {
        if (this._aeHasWasm && this._module && this._ae) return this._module.aeGetDt(this._ae);
        return this._ensureAEFallback().aeGetDt();
    }
    aeSetSoftening(s) {
        if (this._aeHasWasm && this._module && this._ae) this._module.aeSetSoftening(this._ae, s);
        else this._ensureAEFallback().aeSetSoftening(s);
    }
    aeSetDamping(e) {
        if (this._aeHasWasm && this._module && this._ae) this._module.aeSetDamping(this._ae, e);
        else this._ensureAEFallback().aeSetDamping(e);
    }
    aeSetBonding(e) {
        if (this._aeHasWasm && this._module && this._ae) this._module.aeSetBonding(this._ae, e);
        else this._ensureAEFallback().aeSetBonding(e);
    }
    aeSetIonic(e)     { this._ensureAEFallback().aeSetIonic(e); }
    aeSetVdw(e)       { this._ensureAEFallback().aeSetVdw(e); }
    aeSetBondsForce(e){ this._ensureAEFallback().aeSetBondsForce(e); }
    aeSetSpeedLimit(e){ this._ensureAEFallback().aeSetSpeedLimit(e); }
    // Phase 3 setters (delegate to MockBridge fallback; WASM uses aeSetToggle)
    aeSetHBonds(e)            { this._ensureAEFallback().aeSetHBonds(e); }
    aeSetAngleStrain(e)       { this._ensureAEFallback().aeSetAngleStrain(e); }
    aeSetDipoleDipole(e)      { this._ensureAEFallback().aeSetDipoleDipole(e); }
    aeSetThermostat(e)        { this._ensureAEFallback().aeSetThermostat(e); }
    aeSetThermostatTemp(t)    { this._ensureAEFallback().aeSetThermostatTemp(t); }
    aeSetElectronegativity(e) { this._ensureAEFallback().aeSetElectronegativity(e); }
    aePreBond() {
        // WASM AtomEngine doesn't need pre-bonding (bonds are explicit there)
        // MockBridge needs it to prevent LJ explosions on first tick
        this._ensureAEFallback().aePreBond();
    }
    aeAtomCount() {
        if (this._aeHasWasm && this._module && this._ae) return this._module.aeAtomCount(this._ae);
        return this._ensureAEFallback().aeAtomCount();
    }
    aeInspectAtom(id) { return this._ensureAEFallback().aeInspectAtom(id); }
    aeClear() { this.resetAE(); }
}
