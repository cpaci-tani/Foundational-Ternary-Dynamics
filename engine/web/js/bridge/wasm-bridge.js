/**
 * @file engine/web/js/bridge/wasm-bridge.js
 * @purpose Thin wrapper around the compiled C++/WASM physics engine
 *          (engine/wasm/ftd_wasm.cpp). Implements the same ScaleBridge
 *          contract as MockBridge so consumers can switch backends
 *          without touching call sites.
 * @consumers bridge-init.js (re-exports), engine/web/js/scales/scaleN/controller.js
 *            for N in 0..11 via the createScale0/1/2Capabilities factories.
 * @contract CONTRACTS.md §2 (Capability Factory Contract) — symmetric
 *            surface with MockBridge.
 * @related engine/web/js/bridge/mock-bridge.js (JS-only counterpart)
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

import { ALPHA, ALPHA_EFT, K_B, G_C, G_N, COULOMB_K_FORCE } from '../constants.js';
import { debugLog } from '../core/log.js';
import { getById as catalogGetById } from '../particle-catalog.js';
// AtomEngine (Scale 2/3) runs through the MockBridge JS implementation while
// the WASM AtomEngine's Planck-unit conversion layer is unbuilt (see _aeHasWasm).
// _ensureAEFallback() instantiates one — the import was lost in the Phase-2b
// bridge split, making every AE call in WASM mode throw "MockBridge is not
// defined" (audit P1-2 crash-portion; Scale 2/3 were broken in the default mode).
import { MockBridge } from './mock-bridge.js';

// ── WASM Bridge ────────────────────────────────────────────────────
let _wasmLoadPromise = null; // singleton to prevent duplicate script injection

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
    count: 0,
});

// Generic delegate: run `fn` if the WASM module exposes both the bridge AND
// the specified method, else return `fallback`. Collapses the two-line guard
// block (`if (!this._module || !this._bridge) return X; if (typeof ... !==
// 'function') return X;`) that previously appeared ~20× inside WasmBridge.
function _wasmCallOr(bridge, methodName, fallback, fn) {
    if (!bridge._module || !bridge._bridge) return fallback;
    if (typeof bridge._module[methodName] !== 'function') return fallback;
    return fn(bridge._module, bridge._bridge);
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
    }

    async init(latticeSize = 33) {
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
            const scriptSrc = use64 ? 'wasm/ftd_core64.js' : 'wasm/ftd_core.js';
            const factoryName = use64 ? 'createFTDModule64' : 'createFTDModule';
            if (typeof globalThis[factoryName] === 'undefined') {
                if (!_wasmLoadPromise) {
                    _wasmLoadPromise = new Promise((resolve, reject) => {
                        const script = document.createElement('script');
                        script.src = scriptSrc;
                        script.onload = resolve;
                        script.onerror = () => {
                            _wasmLoadPromise = null; // allow retry
                            reject(new Error('Failed to load ' + scriptSrc));
                        };
                        document.head.appendChild(script);
                    });
                }
                await _wasmLoadPromise;
            }
            this._module = await globalThis[factoryName]({
                locateFile: (path) => 'wasm/' + path
            });
            debugLog('[WasmBridge] loaded ' + (use64 ? 'wasm64 (Memory64, 8 GB heap)' : 'wasm32 (2 GB heap)'));
            // Must be RenderBridge, not DagEngine: every module function in
            // ftd_wasm.cpp (setupScenario, injectParticle, injectFlux, setDt,
            // etc.) takes `ftd::RenderBridge&`. The DagEngine embind class
            // only exposes .tick/.clear and cannot be passed to those
            // functions (embind throws BindingError on type mismatch).
            console.log('[WasmBridge] init() - constructing initial RenderBridge with L =', this.latticeSize);
            try {
                this._bridge = new this._module.RenderBridge(this.latticeSize);
            } catch (err) {
                console.error('[WasmBridge] Fatal: failed to construct initial RenderBridge(' + this.latticeSize + '):', err);
                throw err;
            }
            this.ready = true;
            debugLog('FTD WASM engine loaded successfully');
            return true;
        } catch (e) {
            console.warn('WASM module not available, falling back to MockBridge:', e.message);
            return false;
        }
    }

    tick() { if (this._bridge) this._bridge.tick(); }
    run(n) { if (this._bridge) this._bridge.run(n); }
    currentTick() { return this._bridge ? this._bridge.currentTick() : 0; }

    setDt(dt) {
        if (this._module && this._bridge) this._module.setDt(this._bridge, dt);
    }
    getDt() {
        if (this._module && this._bridge) return this._module.getDt(this._bridge);
        return 1.0;
    }
    getPhysicalTime() {
        if (this._module && this._bridge) return this._module.getPhysicalTime(this._bridge);
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
        console.log('[WasmBridge] reset() called. latticeSize =', this.latticeSize);
        if (this._module) {
            // Bridge-H2 (audit 2026-04-27): the C++ ParticleEngine and
            // AtomEngine handles cached on `this._pe`/`this._ae` are
            // bound to the OLD RenderBridge; once we destroy the old
            // bridge they're invalid. Drop them so the next access
            // path re-acquires fresh handles via the new RenderBridge.
            // Same logic for the JS-side _aeFallback (a MockBridge)
            // and the lazy-attached physics harness.
            if (this._pe) { try { this._pe.delete?.(); } catch {} this._pe = null; }
            if (this._ae) { try { this._ae.delete?.(); } catch {} this._ae = null; }
            if (this._aeFallback?.dispose) {
                try { this._aeFallback.dispose(); } catch {}
                this._aeFallback = null;
            }
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
            // RenderBridge (not DagEngine) — see init() above for rationale.
            if (this._bridge) {
                console.log('[WasmBridge] reset() - deleting old RenderBridge...');
                this._bridge.delete();
                this._bridge = null;
            }
            console.log('[WasmBridge] reset() - constructing new RenderBridge with L =', this.latticeSize);
            try {
                this._bridge = new this._module.RenderBridge(this.latticeSize);
            } catch (err) {
                console.error('[WasmBridge] Fatal: failed to construct new RenderBridge(' + this.latticeSize + '):', err);
                throw err;
            }
            console.log('[WasmBridge] reset() - RenderBridge constructed successfully.');
        }
    }

    /**
     * Tear down the WasmBridge: delete the C++ RenderBridge + sub-
     * engine handles, drop the JS-side AE-fallback MockBridge, drop
     * the lazy-attached harness. Idempotent. Symmetric with
     * MockBridge.dispose() (Bridge-H1 audit fix, 2026-04-27).
     */
    dispose() {
        if (this._pe) { try { this._pe.delete?.(); } catch {} this._pe = null; }
        if (this._ae) { try { this._ae.delete?.(); } catch {} this._ae = null; }
        if (this._aeFallback?.dispose) {
            try { this._aeFallback.dispose(); } catch {}
            this._aeFallback = null;
        }
        if (this._bridge) {
            try { this._bridge.delete(); } catch {}
            this._bridge = null;
        }
        delete this.__ftdPhysicsHarness__;
        this.ready = false;
    }

    injectParticle(x, y, z, state) {
        if (this._module && this._bridge)
            this._module.injectParticle(this._bridge, x, y, z, state);
    }

    injectWavepacket(x, y, z, state) {
        if (this._module && this._bridge)
            this._module.injectWavepacket(this._bridge, x, y, z, state);
    }

    injectFlux(x, y, z, fx, fy, fz) {
        if (this._module && this._bridge)
            this._module.injectFlux(this._bridge, x, y, z, fx, fy, fz);
    }

    createEntangledPair(x, y, z, fx, fy, fz) {
        if (this._module && this._bridge)
            this._module.createEntangledPair(this._bridge, x, y, z, fx, fy, fz);
    }

    setToggle(name, value) {
        if (this._module && this._bridge)
            this._module.setToggle(this._bridge, name, value);
    }

    getToggle(name) {
        if (this._module && this._bridge)
            return this._module.getToggle(this._bridge, name);
        return true;
    }

    getParticleData() {
        if (!this._module || !this._bridge) return EMPTY_PARTICLE_DATA;
        const raw = this._module.getParticleData(this._bridge);
        if (!raw || raw.count === 0) return EMPTY_PARTICLE_DATA;
        return {
            positions: raw.positions,
            colors: raw.colors,
            sizes: raw.sizes,
            count: raw.count
        };
    }

    getScale0ParticleList() {
        const pd = this.getParticleData();
        if (!pd || pd.count === 0) return [];
        const list = [];
        for (let i = 0; i < pd.count; i++) {
            const x = Math.floor(pd.positions[i * 3]);
            const y = Math.floor(pd.positions[i * 3 + 1]);
            const z = Math.floor(pd.positions[i * 3 + 2]);
            const r = pd.colors[i * 3];
            const g = pd.colors[i * 3 + 1];
            let state = 0;
            if (g > 0.7) state = 1;
            else if (r > 0.8) state = -1;

            const voxel = this.inspectVoxel(x, y, z);
            const isLocked = voxel ? !!voxel.locked : false;

            list.push({
                id: i,
                x, y, z,
                state,
                charge: state,
                q: state,
                color: 0,
                spin: 1,
                locked: isLocked
            });
        }
        return list;
    }

    getDiagnostics() {
        if (!this._module || !this._bridge)
            return {
                tick: 0, manifested: 0, positive: 0, negative: 0, totalFlux: 0, totalEnergy: 0,
                maxBandwidth: 0, avgDrag: 0, entropy: 0, chargeBalance: 0,
                spinUp: 0, spinDown: 0, colorless: 0, colorRed: 0, colorGreen: 0, colorBlue: 0,
                angMomX: 0, angMomY: 0, angMomZ: 0
            };
        return this._module.getDiagnostics(this._bridge);
    }

    getEnergyAudit() {
        if (!this._module || !this._bridge)
            return {
                fieldEnergy: 0, waveEnergy: 0, particleKE: 0, totalEnergy: 0,
                EFieldEnergy: 0, BFieldEnergy: 0,
                totalPoynting: { x: 0, y: 0, z: 0 },
                gaussViolation: 0, maxGaussError: 0, selfFieldInjection: 0,
                coulombPE: 0, chargeTotal: 0, manifested: 0,
                ELTotal: 0, ERTotal: 0, chiralityTotal: 0, wvLTotal: 0, wvRTotal: 0,
            };
        return this._module.getEnergyAudit(this._bridge);
    }

    getLagrangian() {
        if (!this._module || !this._bridge)
            return {
                fieldKinetic: 0, fieldGradient: 0,
                bornInfeld: 0, coupling: 0, velocity: 0, gauss: 0, dissipation: 0,
                total: 0, hamiltonian: 0, totalAction: 0, gaussViolation: 0, maxGaussError: 0,
                totalFluxMag: 0, totalWaveEnergy: 0, manifested: 0, locked: 0
            };
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

    setupScenario(name, harness) {
        this.reset();
        if (this._module && this._bridge)
            this._module.setupScenario(this._bridge, name);
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

    // Phase 2 gravity panel: REAL C++ latency field (voxel.latency), not the |J|² proxy.
    getGravityMetricAgg() {
        return _wasmCallOr(this, 'getGravityMetricAgg',
            { active: false, latencyMax: 0, latencyMean: 0, fMin: 1, gammaMax: 1, dilationMaxPct: 0, voxelCount: 0 },
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

    // ── ParticleEngine (Scale 1) WASM ─────────────────────────────────
    initPE() {
        if (this._pe) {
            this._pe.delete(); // free old C++ ParticleEngine to prevent memory leak
        }
        if (this._module) {
            this._pe = new this._module.ParticleEngine();
        }
        this._peParticleTypes = new Map();
        this._peParticleMeta = new Map();
        this._peSoftening = 1.0;
        this._peToggleState = {
            coulomb: true,
            gravity: true,
            damping: true,
            lorentz: false,
            exchange: false,
            strong: false,
            magnetic_dipole: false,
            spin_orbit: false,
            radiation: false,
            relativistic: false,
            relativistic_verlet: false,
        };
    }

    resetPE() {
        if (this._module && this._pe) {
            this._module.peClear(this._pe);
        }
        if (this._peParticleTypes) this._peParticleTypes.clear();
        if (this._peParticleMeta) this._peParticleMeta.clear();
    }

    peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        if (!this._pe) this.initPE();
        if (!this._module || !this._pe) return -1;
        const id = this._module.peAddParticle(this._pe, charge, x, y, z, vx, vy, vz, mass, r_eff);
        this._peParticleTypes.set(id, catalogId);
        this._peParticleMeta.set(id, { catalogId, charge, mass, rEff: r_eff, locked: false, vx, vy, vz });
        return id;
    }

    peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff = 0.1) {
        if (!this._pe) this.initPE();
        if (!this._module || !this._pe) return -1;
        const id = this._module.peAddLockedParticle(this._pe, charge, x, y, z, mass, r_eff);
        this._peParticleTypes.set(id, catalogId);
        this._peParticleMeta.set(id, { catalogId, charge, mass, rEff: r_eff, locked: true, vx: 0, vy: 0, vz: 0 });
        return id;
    }

    peTick() {
        if (this._module && this._pe) this._pe.tick();
    }

    _emptyPEParticleData() {
        return {
            positions: new Float32Array(0),
            velocities: new Float32Array(0),
            colors: new Float32Array(0),
            sizes: new Float32Array(0),
            charges: new Int8Array(0),
            ids: new Int32Array(0),
            masses: new Float64Array(0),
            rEff: new Float32Array(0),
            locked: new Uint8Array(0),
            spins: new Int8Array(0),
            colorIds: new Int8Array(0),
            count: 0,
        };
    }

    _metaForPEId(id) {
        const direct = this._peParticleMeta?.get(id);
        if (direct) return direct;
        const catalogId = this._peParticleTypes?.get(id);
        const cat = catalogId ? catalogGetById(catalogId) : null;
        return {
            catalogId,
            charge: 0,
            mass: cat ? cat.mass_mev : K_B,
            rEff: 0.1,
            locked: false,
            vx: 0,
            vy: 0,
            vz: 0,
        };
    }

    _augmentPEParticleData(raw) {
        if (!raw || !raw.count) return this._emptyPEParticleData();
        const n = raw.count;
        const ids = raw.ids || new Int32Array(n);
        const charges = raw.charges || new Int8Array(n);
        const velocities = (raw.velocities && raw.velocities.length >= n * 3)
            ? raw.velocities
            : new Float32Array(n * 3);
        const masses = (raw.masses && raw.masses.length >= n)
            ? raw.masses
            : new Float64Array(n);
        const rEff = (raw.rEff && raw.rEff.length >= n)
            ? raw.rEff
            : new Float32Array(n);
        const locked = (raw.locked && raw.locked.length >= n)
            ? raw.locked
            : new Uint8Array(n);
        const spins = (raw.spins && raw.spins.length >= n)
            ? raw.spins
            : new Int8Array(n);
        const colorIds = (raw.colorIds && raw.colorIds.length >= n)
            ? raw.colorIds
            : new Int8Array(n);

        for (let i = 0; i < n; i++) {
            const meta = this._metaForPEId(ids[i]);
            if (!raw.velocities || raw.velocities.length < n * 3) {
                velocities[i * 3] = meta.vx || 0;
                velocities[i * 3 + 1] = meta.vy || 0;
                velocities[i * 3 + 2] = meta.vz || 0;
            }
            if (!raw.masses || raw.masses.length < n) masses[i] = meta.mass || K_B;
            if (!raw.rEff || raw.rEff.length < n) rEff[i] = meta.rEff || 0.1;
            if (!raw.locked || raw.locked.length < n) locked[i] = meta.locked ? 1 : 0;
            if (!raw.charges || raw.charges.length < n) charges[i] = meta.charge || 0;
        }

        return {
            positions: raw.positions || new Float32Array(n * 3),
            velocities,
            colors: raw.colors || new Float32Array(n * 3),
            sizes: raw.sizes || new Float32Array(n),
            charges,
            ids,
            masses,
            rEff,
            locked,
            spins,
            colorIds,
            count: n,
        };
    }

    _peToggleValue(name, fallback = false) {
        if (this._module && this._pe && typeof this._module.peGetToggle === 'function') {
            return !!this._module.peGetToggle(this._pe, name);
        }
        if (this._peToggleState && Object.prototype.hasOwnProperty.call(this._peToggleState, name)) {
            return !!this._peToggleState[name];
        }
        return fallback;
    }

    _computePEForcesFromData(data) {
        const n = data?.count || 0;
        const positions = data?.positions || new Float32Array(0);
        const forces = new Float32Array(n * 3);
        const soft = this._peSoftening ?? 1.0;
        const soft2 = soft * soft;
        const doCoulomb = this._peToggleValue('coulomb', true);
        const doGravity = this._peToggleValue('gravity', false);
        let maxForce = 0;

        for (let i = 0; i < n; i++) {
            const ix = positions[i * 3];
            const iy = positions[i * 3 + 1];
            const iz = positions[i * 3 + 2];
            for (let j = i + 1; j < n; j++) {
                const dx = positions[j * 3] - ix;
                const dy = positions[j * 3 + 1] - iy;
                const dz = positions[j * 3 + 2] - iz;
                const r2 = dx * dx + dy * dy + dz * dz + soft2;
                if (r2 < 1e-40) continue;
                const invR = 1 / Math.sqrt(r2);
                const invR2 = invR * invR;
                const fc = doCoulomb ? -COULOMB_K_FORCE * data.charges[i] * data.charges[j] * invR2 : 0;
                const fg = doGravity ? G_N * data.masses[i] * data.masses[j] * invR2 : 0;
                const fr = (fc + fg) * invR;
                const fx = fr * dx;
                const fy = fr * dy;
                const fz = fr * dz;
                const i3 = i * 3;
                const j3 = j * 3;
                forces[i3] += fx;
                forces[i3 + 1] += fy;
                forces[i3 + 2] += fz;
                forces[j3] -= fx;
                forces[j3 + 1] -= fy;
                forces[j3 + 2] -= fz;
            }
        }

        for (let i = 0; i < n; i++) {
            const fx = forces[i * 3];
            const fy = forces[i * 3 + 1];
            const fz = forces[i * 3 + 2];
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            if (mag > maxForce) maxForce = mag;
        }

        return { positions, forces, count: n, maxForce };
    }

    _computePEPotentialFromData(data) {
        const n = data?.count || 0;
        const positions = data?.positions || new Float32Array(0);
        const soft = this._peSoftening ?? 1.0;
        const soft2 = soft * soft;
        const doCoulomb = this._peToggleValue('coulomb', true);
        const doGravity = this._peToggleValue('gravity', false);
        let coulombPE = 0;
        let gravityPE = 0;
        for (let i = 0; i < n; i++) {
            for (let j = i + 1; j < n; j++) {
                const dx = positions[j * 3] - positions[i * 3];
                const dy = positions[j * 3 + 1] - positions[i * 3 + 1];
                const dz = positions[j * 3 + 2] - positions[i * 3 + 2];
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                if (r < 1e-30) continue;
                if (doCoulomb) coulombPE += ALPHA * data.charges[i] * data.charges[j] / (4 * Math.PI * r);
                if (doGravity) gravityPE -= G_N * data.masses[i] * data.masses[j] / r;
            }
        }
        return { coulombPE, gravityPE, totalPE: coulombPE + gravityPE };
    }

    peGetParticleData() {
        if (!this._module || !this._pe)
            return this._emptyPEParticleData();
        return this._augmentPEParticleData(this._module.getPEParticleData(this._pe));
    }

    peGetDiagnostics() {
        if (!this._module || !this._pe)
            return { tick: 0, particleCount: 0, totalKE: 0, totalPE: 0, coulombPE: 0, gravityPE: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0, angMomX: 0, angMomY: 0, angMomZ: 0 };
        const d = this._module.getPEDiagnostics(this._pe);
        // Older WASM builds did not expose decomposed active PE. Reconstruct it
        // from the augmented particle frame so the UI does not mislabel energy.
        if (d.coulombPE === undefined || d.gravityPE === undefined) {
            const pe = this._computePEPotentialFromData(this.peGetParticleData());
            d.coulombPE = pe.coulombPE;
            d.gravityPE = pe.gravityPE;
            d.totalPE = pe.totalPE;
            d.totalEnergy = (d.totalKE || 0) + pe.totalPE;
        }
        return d;
    }

    peGetExtendedData() {
        if (!this._module || !this._pe) return null;
        if (typeof this._module.getPEExtendedData === 'function') {
            const ext = this._module.getPEExtendedData(this._pe);
            return ext && ext.count ? ext : null;
        }
        const data = this.peGetParticleData();
        if (!data || data.count === 0) return null;
        const forceFrame = this._computePEForcesFromData(data);
        const n = data.count;
        const accelerations = new Float64Array(n * 3);
        for (let i = 0; i < n; i++) {
            const m = data.masses[i] || K_B;
            if (m > 1e-30 && !data.locked[i]) {
                accelerations[i * 3] = forceFrame.forces[i * 3] / m;
                accelerations[i * 3 + 1] = forceFrame.forces[i * 3 + 1] / m;
                accelerations[i * 3 + 2] = forceFrame.forces[i * 3 + 2] / m;
            }
        }
        return {
            count: n,
            ids: data.ids,
            charges: data.charges,
            masses: data.masses,
            positions: data.positions,
            velocities: data.velocities,
            forces: forceFrame.forces,
            accelerations,
            locked: data.locked,
            rEff: data.rEff,
            spins: data.spins,
            colorIds: data.colorIds,
        };
    }

    peGetForces() {
        if (!this._module || !this._pe)
            return { positions: new Float32Array(0), forces: new Float32Array(0), count: 0, maxForce: 0 };
        if (typeof this._module.getPEForces === 'function') {
            return this._module.getPEForces(this._pe);
        }
        return this._computePEForcesFromData(this.peGetParticleData());
    }

    peGetFieldSources() {
        // Build field sources from WASM PE particle data
        const data = this.peGetParticleData();
        if (!data || data.count === 0)
            return { positions: new Float32Array(0), charges: new Float32Array(0), masses: new Float32Array(0), count: 0 };
        const n = data.count;
        const charges = new Float32Array(n);
        const masses = new Float32Array(n);
        for (let i = 0; i < n; i++) {
            charges[i] = data.charges[i]; // Int8 → Float32
            masses[i] = data.masses?.[i] ?? this._metaForPEId(data.ids?.[i] ?? -1).mass ?? K_B;
        }
        return { positions: data.positions, charges, masses, count: n };
    }

    peSetDt(dt) {
        if (this._module && this._pe) this._module.peSetDt(this._pe, dt);
    }
    peGetDt() {
        if (this._module && this._pe) return this._module.peGetDt(this._pe);
        return 1.0;
    }
    peSetSoftening(s) {
        this._peSoftening = s;
        if (this._module && this._pe) this._module.peSetSoftening(this._pe, s);
    }
    peSetCoulomb(e) {
        if (this._peToggleState) this._peToggleState.coulomb = !!e;
        if (!this._module || !this._pe) return;
        // Prefer dedicated setter; fall back to generic toggle if available.
        // Coulomb defaults to ON in the C++ ParticleEngine constructor,
        // so a missing binding is safe as long as we don't crash.
        if (typeof this._module.peSetCoulomb === 'function') {
            this._module.peSetCoulomb(this._pe, e);
        } else if (typeof this._module.peSetToggle === 'function') {
            this._module.peSetToggle(this._pe, 'coulomb', e);
        }
        // else: Coulomb defaults to true in C++; no-op is acceptable
    }
    peSetDamping(e) {
        if (this._peToggleState) this._peToggleState.damping = !!e;
        if (this._module && this._pe) this._module.peSetDamping(this._pe, e);
    }
    peSetGravity(e) {
        if (this._peToggleState) this._peToggleState.gravity = !!e;
        if (this._module && this._pe) this._module.peSetGravity(this._pe, e);
    }

    // Advanced PE toggles — WASM binary doesn't expose individual setters yet.
    // Use the generic peSetToggle if available, otherwise no-op gracefully.
    // These default to OFF in the C++ ParticleEngine constructor.
    _peToggle(name, e) {
        if (this._peToggleState) this._peToggleState[name] = !!e;
        if (!this._module || !this._pe) return;
        if (typeof this._module.peSetToggle === 'function') {
            this._module.peSetToggle(this._pe, name, e);
        }
    }
    peSetLorentz(e)        { this._peToggle('lorentz', e); }
    peSetExchange(e)       { this._peToggle('exchange', e); }
    peSetStrong(e)         { this._peToggle('strong', e); }
    peSetMagneticDipole(e) { this._peToggle('magnetic_dipole', e); }
    peSetSpinOrbit(e)      { this._peToggle('spin_orbit', e); }
    peSetRadiation(e)      { this._peToggle('radiation', e); }
    peSetRelativistic(e)   { this._peToggle('relativistic', e); }
    peSetRelativisticVerlet(e) { this._peToggle('relativistic_verlet', e); }
    peGetToggle(name)      { return this._peToggleValue(name, false); }

    peGetBackendCapabilities() {
        return {
            velocities: true,
            masses: true,
            locked: true,
            forces: true,
            extended: true,
            nativeExtended: !!(this._module && typeof this._module.getPEExtendedData === 'function'),
            nativeForces: !!(this._module && typeof this._module.getPEForces === 'function'),
            advancedForces: !!(this._module && typeof this._module.peSetToggle === 'function'),
        };
    }

    peParticleCount() {
        if (this._module && this._pe) return this._module.peParticleCount(this._pe);
        return 0;
    }
    peClear() { this.resetPE(); }
    peGetParticleTypes() { return this._peParticleTypes || new Map(); }

    peInspectParticle(id) {
        if (!this._module || !this._pe) return null;
        const ext = this.peGetExtendedData();
        if (!ext || ext.count === 0) return null;

        // Find particle by id
        let idx = -1;
        for (let i = 0; i < ext.count; i++) {
            if (ext.ids[i] === id) { idx = i; break; }
        }
        if (idx < 0) return null;

        const px = ext.positions[idx * 3], py = ext.positions[idx * 3 + 1], pz = ext.positions[idx * 3 + 2];
        const vx = ext.velocities[idx * 3], vy = ext.velocities[idx * 3 + 1], vz = ext.velocities[idx * 3 + 2];
        const charge = ext.charges[idx];
        const mass = ext.masses[idx] || this._metaForPEId(id).mass || K_B;
        const speed = Math.sqrt(vx * vx + vy * vy + vz * vz);
        const fx = ext.forces?.[idx * 3] || 0;
        const fy = ext.forces?.[idx * 3 + 1] || 0;
        const fz = ext.forces?.[idx * 3 + 2] || 0;

        let nearestId = -1;
        let nearestDist = Infinity;
        let fCoulombNearest = 0;
        let orbitalR = -1;
        const soft2 = (this._peSoftening || 1.0) ** 2;
        for (let i = 0; i < ext.count; i++) {
            if (i === idx) continue;
            const dx = ext.positions[i * 3] - px;
            const dy = ext.positions[i * 3 + 1] - py;
            const dz = ext.positions[i * 3 + 2] - pz;
            const r2Raw = dx * dx + dy * dy + dz * dz;
            const r = Math.sqrt(r2Raw);
            if (r < nearestDist) {
                nearestDist = r;
                nearestId = ext.ids[i];
                if (this._peToggleValue('coulomb', true)) {
                    fCoulombNearest = Math.abs(COULOMB_K_FORCE * charge * ext.charges[i] / (r2Raw + soft2));
                } else {
                    fCoulombNearest = 0;
                }
            }
            if (charge !== 0 && ext.charges[i] !== 0 && Math.sign(charge) !== Math.sign(ext.charges[i])) {
                if (orbitalR < 0 || r < orbitalR) orbitalR = r;
            }
        }

        return {
            id, charge, mass,
            x: px, y: py, z: pz,
            vx, vy, vz,
            speed, ke: 0.5 * mass * speed * speed,
            locked: !!ext.locked?.[idx],
            nearestId, nearestDist,
            orbitalR, fCoulombNearest,
            fNetMag: Math.sqrt(fx * fx + fy * fy + fz * fz),
        };
    }

    // ── Boundary containment ─────────────────────────────────────────
    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        // Propagate to AE fallback MockBridge if it exists
        if (this._aeFallback) this._aeFallback.setBoundaryShape(shape);
    }

    setReflectiveBoundary(on) {
        this._reflectiveBoundary = !!on;
        if (this._aeFallback) this._aeFallback.setReflectiveBoundary(on);
    }

    // ── AtomEngine (Scale 2) WASM ─────────────────────────────────────
    // Falls back to MockBridge JS implementation when WASM module lacks
    // AtomEngine (i.e., not yet rebuilt with Emscripten after adding bindings).
    _ensureAEFallback() {
        if (!this._aeFallback) {
            this._aeFallback = new MockBridge(this.latticeSize);
            // Sync boundary shape
            if (this._boundaryShape) this._aeFallback.setBoundaryShape(this._boundaryShape);
        }
        return this._aeFallback;
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
        } else {
            this._ensureAEFallback().aeTick();
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
