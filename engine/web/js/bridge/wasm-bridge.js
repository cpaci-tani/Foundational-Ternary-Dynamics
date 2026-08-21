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
import { createNativeParticleEngine } from './native-particle-engine.js';
import { createAtomEngine } from './mock-atom-engine.js';
import { reflectIntoBoundary } from './boundary.js';
import { samplerOr, particleDataToList } from './bridge-contract.js';

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
    spin: new Float32Array(0),
    colorCharge: new Float32Array(0),
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

/** @implements {import('./bridge/bridge-contract.js').ScaleBridge} */
export class WasmBridge {
    constructor() {
        this._module = null;
        this._bridge = null;
        this.latticeSize = 33;
        this.ready = false;
        this.isWasm = true;
        this.isWasm64 = false;   // set true in init() when the Memory64 build loads
        this._lastScale0Audit = null;
        this._lastScale0AuditTick = -1;
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
            debugLog('[WasmBridge] init() - constructing initial RenderBridge with L =', this.latticeSize);
            try {
                this._bridge = new this._module.RenderBridge(this.latticeSize);
            } catch (err) {
                console.error('[WasmBridge] Fatal: failed to construct initial RenderBridge(' + this.latticeSize + '):', err);
                throw err;
            }
            this.ready = true;
            this.wasmLoadState = 'ready';
            debugLog('FTD WASM engine loaded successfully');
            return true;
        } catch (e) {
            this.wasmLoadState = 'failed';
            console.warn('WASM module not available:', e.message);
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
    // FTD-0271: de Broglie internal-clock frequency omega0 (KG mass term).
    setOmega0(w) {
        if (this._module && this._bridge && typeof this._module.setOmega0 === 'function')
            this._module.setOmega0(this._bridge, w);
    }
    getOmega0() {
        if (this._module && this._bridge && typeof this._module.getOmega0 === 'function')
            return this._module.getOmega0(this._bridge);
        return 1.0;
    }
    // FTD-0274: live Langevin bath temperature (thermal-ignition panel).
    setLangevinTemp(t) {
        if (this._module && this._bridge && typeof this._module.setLangevinTemp === 'function')
            this._module.setLangevinTemp(this._bridge, t);
    }
    getLangevinTemp() {
        if (this._module && this._bridge && typeof this._module.getLangevinTemp === 'function')
            return this._module.getLangevinTemp(this._bridge);
    }
    setLangevinGamma(g) {
        if (this._module && this._bridge && typeof this._module.setLangevinGamma === 'function')
            this._module.setLangevinGamma(this._bridge, g);
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
            // RenderBridge (not DagEngine) — see init() above for rationale.
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
    }

    // Revision 2.7: every injection method carries the same try-catch guard
    // injectFlux always had. A BindingError from a bad argument (or a
    // scenario bug) now logs instead of unwinding through the scenario
    // loader — the asymmetry looked intentional but was accretion. NOTE: no
    // heap-death recovery is attempted anywhere; ftd_core builds with
    // -fno-exceptions, so a WASM abort() stays permanent by design.
    injectParticle(x, y, z, state) {
        if (!(this._module && this._bridge)) return;
        try {
            this._module.injectParticle(this._bridge, x, y, z, state);
            this._invalidateScale0AuditCache();
        } catch (e) {
            console.error('WASM injectParticle failed:', e);
        }
    }

    injectWaveVel(x, y, z, vx, vy, vz) {
        if (!(this._module && this._bridge && typeof this._module.injectWaveVel === 'function')) return;
        try {
            this._module.injectWaveVel(this._bridge, x, y, z, vx, vy, vz);
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
        if (this._module && this._bridge && typeof this._module.clearField === 'function')
            this._module.clearField(this._bridge);
    }

    seedRandomFlux() {
        if (this._module && this._bridge && typeof this._module.seedRandomFlux === 'function')
            this._module.seedRandomFlux(this._bridge);
    }

    setToggle(name, value) {
        if (this._module && this._bridge)
            this._module.setToggle(this._bridge, name, value);
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
        return {
            positions: raw.positions,
            colors: raw.colors,
            sizes: raw.sizes,
            spin: raw.spin,
            colorCharge: raw.colorCharge,
            count: raw.count
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

    /**
     * One-shot Scale-0 → Scale-1 coarse-graining snapshot (voxel debug view /
     * promotion pipeline): one manifested voxel → one particle record.
     * Observer-only. Synchronous on this in-thread bridge; WasmBridgeProxy
     * exposes the same name returning a Promise — call sites should
     * `await Promise.resolve(bridge.coarsenToParticles?.())`.
     */
    coarsenToParticles() {
        if (!(this._module && this._bridge)) return null;
        if (typeof this._module.coarsenToParticles !== 'function') return null;
        try {
            return this._module.coarsenToParticles(this._bridge);
        } catch (err) {
            console.warn('[WasmBridge] coarsenToParticles failed:', err);
            return null;
        }
    }

    getScale0ParticleList() {
        // Shared derivation (bridge-contract.js) so this and WasmBridgeProxy
        // cannot drift apart.
        return particleDataToList(this.getParticleData());
    }

    getDiagnostics() {
        if (!this._module || !this._bridge)
            return {
                tick: 0, manifested: 0, positive: 0, negative: 0, totalFlux: 0, totalEnergy: 0,
                maxBandwidth: 0, avgDrag: 0, entropy: 0, chargeBalance: 0,
                maxCausalBudget: 0, causalProjectionEvents: 0,
                spinUp: 0, spinDown: 0, colorless: 0, colorRed: 0, colorGreen: 0, colorBlue: 0,
                angMomX: 0, angMomY: 0, angMomZ: 0,
                fieldSpinX: 0, fieldSpinY: 0, fieldSpinZ: 0, fieldHelicity: 0,
                centerClockPhase: 0, centerClockSpeed: 0, centerClockLatency: 0
            };
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
                maxCausalBudget: arr[21] ?? 0,
                causalProjectionEvents: arr[22] ?? 0,
                // Field circulation ledger (2026-07-28): S = Σ J×W (conserved
                // by the free wave sector), H = Σ J·curl J (static twist).
                fieldSpinX: arr[23] ?? 0,
                fieldSpinY: arr[24] ?? 0,
                fieldSpinZ: arr[25] ?? 0,
                fieldHelicity: arr[26] ?? 0,
                centerClockPhase: arr[27] ?? 0,
                centerClockSpeed: arr[28] ?? 0,
                centerClockLatency: arr[29] ?? 0
            };
        } else {
            d = this._module.getDiagnostics(this._bridge);
        }
        const audit = this._getScale0AuditForTick(d?.tick ?? this.currentTick());
        if (audit && Number.isFinite(audit.dynamicEnergy)) {
            // Conservation charts use the rest-offset-free accounted channel.
            // Keep rest and total accounted energy visible as separate fields.
            d.vacuumBaselineEnergy = d.totalEnergy;
            d.dynamicEnergy = audit.dynamicEnergy;
            d.accountedEnergy = audit.totalEnergy;
            d.restEnergy = audit.particleRestEnergy;
            d.totalEnergy = audit.dynamicEnergy;
            // Status-bar decomposition (whole-box channels, sim units) —
            // mirrors wasm-bridge.worker.js postFrame().
            d.fieldEnergy = audit.fieldEnergy;
            d.waveEnergy = audit.waveEnergy;
            d.particleKE = audit.particleKE;
        }
        return d;
    }

    getEnergyAudit() {
        if (!this._module || !this._bridge)
            return {
                fieldEnergy: 0, waveEnergy: 0, particleKE: 0, totalEnergy: 0,
                particleRestEnergy: 0, particleEnergy: 0, dynamicEnergy: 0,
                particleMomentum: { x: 0, y: 0, z: 0 },
                cellVolume: VOXEL_VOLUME,
                fieldEnergyDensitySum: 0, waveEnergyDensitySum: 0,
                EFieldEnergy: 0, BFieldEnergy: 0,
                totalPoynting: { x: 0, y: 0, z: 0 },
                gaussViolation: 0, maxGaussError: 0, selfFieldInjection: 0,
                coulombPE: 0, chargeTotal: 0, manifested: 0,
                ELTotal: 0, ERTotal: 0, chiralityTotal: 0, wvLTotal: 0, wvRTotal: 0,
            };
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
                particleRestEnergy: arr[19] ?? 0,
                particleEnergy: arr[20] ?? 0,
                particleMomentum: { x: arr[21] ?? 0, y: arr[22] ?? 0, z: arr[23] ?? 0 },
                dynamicEnergy: arr[24] ?? arr[3],
                cellVolume: arr[25] ?? VOXEL_VOLUME,
                fieldEnergyDensitySum: arr[26] ?? arr[0],
                waveEnergyDensitySum: arr[27] ?? arr[1],
            };
        } else {
            audit = this._module.getEnergyAudit(this._bridge);
        }
        this._lastScale0Audit = audit;
        this._lastScale0AuditTick = t;
        return audit;
    }

    _invalidateScale0AuditCache() {
        this._lastScale0Audit = null;
        this._lastScale0AuditTick = -1;
    }

    getLagrangian() {
        if (!this._module || !this._bridge)
            return {
                fieldKinetic: 0, fieldGradient: 0,
                bornInfeld: 0, coupling: 0, velocity: 0, gauss: 0, dissipation: 0,
                total: 0, hamiltonian: 0, totalAction: 0, gaussViolation: 0, maxGaussError: 0,
                totalFluxMag: 0, totalWaveEnergy: 0, manifested: 0, locked: 0,
                cellVolume: VOXEL_VOLUME
            };
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
        // [dependent, prerequisite] edges from TermToggles::validate() reachable
        // from the Scale-0 toggle surface.
        const REQUIRES = [
            ['selective_damping', 'damping'],
            ['larmor_radiation', 'damping'],
            ['lorentz_force', 'forces'],
            ['weak_transmutation', 'dual_substrate'],
            ['triad_binding', 'dual_substrate'],
            ['latency_field', 'gravity'],
        ];
        for (const [dep, prereq] of REQUIRES) {
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
    /** Kind-dispatched Scale-0 field sampler; see bridge-contract.js samplerOr. */
    getSamplerOr(kind, stride = 2, fallback) { return samplerOr(this, kind, stride, fallback); }

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
    peGetParticleData()                                              { return this._peEngine.peGetParticleData(); }
    peGetFieldSources()                                              { return this._peEngine.peGetFieldSources(); }
    peGetForces()                                                    { return this._peEngine.peGetForces(); }
    peGetForceDecomposition()                                        { return this._peEngine.peGetForceDecomposition(); }
    peGetDiagnostics()                                               { return this._peEngine.peGetDiagnostics(); }
    peGetExtendedData()                                              { return this._peEngine.peGetExtendedData(); }
    peSetDt(dt)                                                      { return this._peEngine.peSetDt(dt); }
    peGetDt()                                                        { return this._peEngine.peGetDt(); }
    peSetSoftening(s)                                                { return this._peEngine.peSetSoftening(s); }
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
    peGetToggle(name)                                                { return this._peEngine.peGetToggle(name); }
    peGetBackendCapabilities()                                       { return this._peEngine.peGetBackendCapabilities(); }
    peParticleCount()                                                { return this._peEngine.peParticleCount(); }
    peClear()                                                        { return this._peEngine.peClear(); }
    peGetParticleTypes()                                             { return this._peEngine.peGetParticleTypes(); }
    peInspectParticle(id)                                            { return this._peEngine.peInspectParticle(id); }

    // ── Boundary containment ─────────────────────────────────────────
    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        if (this._aeStub) this._aeStub._boundaryShape = shape;
    }

    // 0 = Periodic, 1 = Reflective, 2 = Dispersal
    setFluxBoundaryMode(mode) {
        this._fluxBoundaryMode = mode;
        if (this._module && this._bridge && typeof this._module.setFluxBoundary === 'function')
            this._module.setFluxBoundary(this._bridge, mode);
    }

    setReflectiveBoundary(on) {
        // Legacy path: map bool → flux boundary mode
        this.setFluxBoundaryMode(on ? 1 : 0);
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
