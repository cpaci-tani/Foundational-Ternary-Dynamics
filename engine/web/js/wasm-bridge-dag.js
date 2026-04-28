/**
 * @file wasm-bridge-dag.js
 * @brief WASM Bridge — abstraction layer between UI and simulation engine.
 *
 * [EXTENDED] Provides a MockBridge for development (no WASM needed) and a WasmBridge
 * for production (loads compiled ftd_core.wasm). The UI code only talks
 * to the Bridge interface, never directly to WASM or mock internals.
 */


// ── Phase 2a refactor sweep (2026-04-27) ────────────────────────────
// MockBridge moved to bridge/mock-bridge.js; imported and re-exported
// here so existing consumers (`import { MockBridge } from './wasm-bridge-dag.js'`)
// see no API change AND the local installCapabilityGetter call near the
// bottom of this file still resolves the symbol. WasmBridge + capability
// factories remain below — Phase 2b will extract WasmBridge, Phase 2c
// the factories. See docs/adr/0003-wasm-bridge-dag-refactor.md and
// .claude/plans/i-want-to-try-crispy-charm.md Phase 2.
import { MockBridge } from './bridge/mock-bridge.js';
export { MockBridge };

// ── WasmBridge-only imports (MockBridge consumers no longer need these) ──
import { ALPHA, ALPHA_EFT, K_B, G_C } from './constants.js';
import { debugLog } from './core/log.js';
import { getById as catalogGetById } from './particle-catalog.js';


// ── WASM Bridge ────────────────────────────────────────────────────
let _wasmLoadPromise = null; // singleton to prevent duplicate script injection

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
        this.latticeSize = 32;
        this.ready = false;
        this.isWasm = true;
    }

    async init(latticeSize = 32) {
        this.latticeSize = latticeSize;
        try {
            if (typeof globalThis.createFTDModule === 'undefined') {
                if (!_wasmLoadPromise) {
                    _wasmLoadPromise = new Promise((resolve, reject) => {
                        const script = document.createElement('script');
                        script.src = 'wasm/ftd_core.js';
                        script.onload = resolve;
                        script.onerror = () => {
                            _wasmLoadPromise = null; // allow retry
                            reject(new Error('Failed to load ftd_core.js'));
                        };
                        document.head.appendChild(script);
                    });
                }
                await _wasmLoadPromise;
            }
            this._module = await globalThis.createFTDModule({
                locateFile: (path) => 'wasm/' + path
            });
            // Must be RenderBridge, not DagEngine: every module function in
            // ftd_wasm.cpp (setupScenario, injectParticle, injectFlux, setDt,
            // etc.) takes `ftd::RenderBridge&`. The DagEngine embind class
            // only exposes .tick/.clear and cannot be passed to those
            // functions (embind throws BindingError on type mismatch).
            this._bridge = new this._module.RenderBridge(latticeSize);
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
        this.latticeSize = latticeSize || this.latticeSize;
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
                this._bridge.delete();
                this._bridge = null;
            }
            this._bridge = new this._module.RenderBridge(this.latticeSize);
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
        // Filter out low-density void particles to prevent white grid artifacts
        // when transparent points stack along camera axes with blending.
        if (!raw || raw.count === 0) return raw;
        const VOID_THRESHOLD = 0.02;
        const outPos = new Float32Array(raw.count * 3);
        const outCol = new Float32Array(raw.count * 3);
        const outSiz = new Float32Array(raw.count);
        let out = 0;
        for (let i = 0; i < raw.count; i++) {
            const sz = raw.sizes[i];
            const r = raw.colors[i * 3], g = raw.colors[i * 3 + 1], b = raw.colors[i * 3 + 2];
            // Detect void particles: they are small and grey/dark
            // Manifested particles (+1/-1) are green (0.29,0.87,0.50) or red (0.97,0.44,0.44) at size ~12
            // Void particles are grey (0.25,0.28,0.35) at size ~2-4
            // Manifested particles: green (g>0.7) or red (r>0.8) at size 6
            // Void with significant flux: grey-blue at size 1.5-5.0
            // Skip ALL void dots — the flux volume handles void visualization
            const isManifested = g > 0.7 || r > 0.8;
            if (!isManifested) continue;
            outPos[out * 3] = raw.positions[i * 3];
            outPos[out * 3 + 1] = raw.positions[i * 3 + 1];
            outPos[out * 3 + 2] = raw.positions[i * 3 + 2];
            outCol[out * 3] = r; outCol[out * 3 + 1] = g; outCol[out * 3 + 2] = b;
            outSiz[out] = sz;
            out++;
        }
        return { positions: outPos, colors: outCol, sizes: outSiz, count: out };
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

    setupScenario(name) {
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
    }

    resetPE() {
        if (this._module && this._pe) {
            this._module.peClear(this._pe);
        }
        if (this._peParticleTypes) this._peParticleTypes.clear();
    }

    peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        if (!this._pe) this.initPE();
        if (!this._module || !this._pe) return -1;
        const id = this._module.peAddParticle(this._pe, charge, x, y, z, vx, vy, vz, mass, r_eff);
        this._peParticleTypes.set(id, catalogId);
        return id;
    }

    peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff = 0.1) {
        if (!this._pe) this.initPE();
        if (!this._module || !this._pe) return -1;
        const id = this._module.peAddLockedParticle(this._pe, charge, x, y, z, mass, r_eff);
        this._peParticleTypes.set(id, catalogId);
        return id;
    }

    peTick() {
        if (this._module && this._pe) this._pe.tick();
    }

    peGetParticleData() {
        if (!this._module || !this._pe)
            return { positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), charges: new Int8Array(0), ids: new Int32Array(0), count: 0 };
        return this._module.getPEParticleData(this._pe);
    }

    peGetDiagnostics() {
        if (!this._module || !this._pe)
            return { tick: 0, particleCount: 0, totalKE: 0, totalPE: 0, coulombPE: 0, gravityPE: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0, angMomX: 0, angMomY: 0, angMomZ: 0 };
        const d = this._module.getPEDiagnostics(this._pe);
        // Add decomposed PE if not already present from WASM
        if (d.coulombPE === undefined) { d.coulombPE = d.totalPE; d.gravityPE = 0; }
        return d;
    }

    peGetExtendedData() {
        // WASM PE doesn't expose extended data yet — stub returns null
        return null;
    }

    peGetForces() {
        // WASM PE doesn't expose forces directly yet — use MockBridge-style computation
        const data = this.peGetParticleData();
        if (!data || data.count === 0)
            return { positions: new Float32Array(0), forces: new Float32Array(0), count: 0, maxForce: 0 };
        return { positions: data.positions, forces: new Float32Array(data.count * 3), count: data.count, maxForce: 0 };
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
            masses[i] = 1.0; // default mass; field sampling uses Coulomb only
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
        if (this._module && this._pe) this._module.peSetSoftening(this._pe, s);
    }
    peSetCoulomb(e) {
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
        if (this._module && this._pe) this._module.peSetDamping(this._pe, e);
    }
    peSetGravity(e) {
        if (this._module && this._pe) this._module.peSetGravity(this._pe, e);
    }

    // Advanced PE toggles — WASM binary doesn't expose individual setters yet.
    // Use the generic peSetToggle if available, otherwise no-op gracefully.
    // These default to OFF in the C++ ParticleEngine constructor.
    _peToggle(name, e) {
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

    peParticleCount() {
        if (this._module && this._pe) return this._module.peParticleCount(this._pe);
        return 0;
    }
    peClear() { this.resetPE(); }
    peGetParticleTypes() { return this._peParticleTypes || new Map(); }

    peInspectParticle(id) {
        // WASM doesn't have a dedicated inspect function yet;
        // compute client-side from particle data
        if (!this._module || !this._pe) return null;
        const data = this.peGetParticleData();
        if (!data || data.count === 0) return null;

        // Find particle by id
        let idx = -1;
        for (let i = 0; i < data.count; i++) {
            if (data.ids[i] === id) { idx = i; break; }
        }
        if (idx < 0) return null;

        const px = data.positions[idx * 3], py = data.positions[idx * 3 + 1], pz = data.positions[idx * 3 + 2];
        const vx = data.velocities ? data.velocities[idx * 3] : 0;
        const vy = data.velocities ? data.velocities[idx * 3 + 1] : 0;
        const vz = data.velocities ? data.velocities[idx * 3 + 2] : 0;
        const charge = data.charges[idx];
        const speed = Math.sqrt(vx * vx + vy * vy + vz * vz);

        // Look up mass from particle catalog via type map
        const catId = this._peParticleTypes.get(id);
        const catEntry = catId ? catalogGetById(catId) : null;
        const mass = catEntry ? catEntry.mass_mev : 1.0;

        return {
            id, charge, mass,
            x: px, y: py, z: pz,
            vx, vy, vz,
            speed, ke: 0.5 * mass * speed * speed,
            locked: false,
            nearestId: -1, nearestDist: Infinity,
            orbitalR: -1, fCoulombNearest: 0, fNetMag: 0,
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
        // WASM AtomEngine exists but uses Planck units internally.
        // Web UI molecule/atom data uses Bohr-scaled simulation units.
        // Until a scale conversion layer is added, force MockBridge fallback.
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

/**
 * Returns derived-overlay data shaped per `kind`. Most return objects
 * include a live reference to `_fluxMag` (and/or `_particles`) — these
 * are mutated each tick. Callers must treat the buffers as read-only
 * within the current frame; if you need a stable snapshot, `.slice()`
 * the magnitude at the call site. Same retention-foot-gun applies to
 * `_particles`: the array identity is stable but per-particle fields
 * mutate in place.
 */
MockBridge.prototype.getScale0DerivedOverlayData = function (kind) {
    if (kind === 'darkMatterHalo') {
        if (!this._fluxJ) return null;
        this._ensureEnergyCache();
        return { particles: this._particles, magnitude: this._fluxMag, latticeSize: this.latticeSize };
    }
    if (kind === 'dampingZones') {
        return { particles: this._particles, latticeSize: this.latticeSize };
    }
    if (kind === 'genesisIsosurface') {
        if (!this._fluxJ) return null;
        this._ensureEnergyCache();
        return { magnitude: this._fluxMag, latticeSize: this.latticeSize, threshold: K_GENESIS };
    }
    return null;
};

function createScale0Capabilities(bridge) {
    return {
        tickScale0: () => bridge.tick(),
        getScale0ParticleFrame: () => bridge.getParticleData(),
        getScale0FluxVolume: () => bridge.getFluxVolume(),
        getScale0FluxSlice: (axis, index) => bridge.getFluxSlice(axis, index),
        getScale0FieldSamples: ({ kind, stride = 2 } = {}) => {
            if (kind === 'e') return bridge.getEFieldSampled(stride);
            if (kind === 'b') return bridge.getBFieldSampled(stride);
            if (kind === 'poynting') return bridge.getPoyntingSampled(stride);
            if (kind === 'divJ') return bridge.getDivJSampled(stride);
            if (kind === 'fluxVector') return bridge.getFluxVectorSampled(stride);
            if (kind === 'vorticity')  return bridge.getVorticitySampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'helicity')   return bridge.getHelicitySampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'kretschmann') return bridge.getKretschmannSampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'latency')    return bridge.getLatencySampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'fisher')     return bridge.getFisherSampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'coherence')  return bridge.getCoherenceSampled?.(stride) ?? { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            if (kind === 'curlJ')      return bridge.getCurlJSampled?.(stride) ?? { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
            return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        },
        getScale0ForceField: (type, stride = 2) => {
            if (type === 'em') return bridge.getEMForceField(stride);
            if (type === 'gravity') return bridge.getGravityForceField(stride);
            if (type === 'strong') return bridge.getStrongForceField(stride);
            return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        },
        getScale0Diagnostics: () => bridge.getDiagnostics(),
        getScale0EnergyAudit: () => bridge.getEnergyAudit(),
        getScale0Lagrangian: () => bridge.getLagrangian(),
        getScale0DerivedOverlayData: (kind) => {
            if (typeof bridge.getScale0DerivedOverlayData === 'function') return bridge.getScale0DerivedOverlayData(kind);
            return null;
        },
        setBoundaryShape: (shape) => bridge.setBoundaryShape?.(shape),
        setReflectiveBoundary: (on) => bridge.setReflectiveBoundary?.(on),
        setupScenario: (name) => bridge.setupScenario(name),
        setToggle: (key, value) => bridge.setToggle?.(key, value),
        get latticeSize() { return bridge.latticeSize || 32; },

        /**
         * Readback current lattice state + flux into plain typed arrays plus
         * particle list and an audit scalar snapshot. Used by the TimelineBuffer.
         * Returns null if the bridge lacks full readback.
         */
        getScale0Snapshot: () => {
            const N = bridge.latticeSize || 32;
            const lattice = bridge.getScale0LatticeBuffer?.();
            const flux    = bridge.getScale0FluxBuffer?.();
            const wave    = bridge.getScale0WaveBuffer?.();
            const particles = bridge.getScale0ParticleList?.() || [];
            if (!lattice || !flux) return null;
            return {
                tick: bridge.getDiagnostics?.().tick ?? 0,
                ts: performance.now(),
                lod: 0,
                N,
                lattice,
                flux,
                wave,
                particles,
                audit: bridge.getDiagnostics?.() ?? {},
            };
        },

        /**
         * Write a snapshot back into the engine. LOD 0 snapshots go in as-is;
         * LOD 1/2 are upsampled to N³ on the fly (nearest neighbor) so scrub
         * playback works across the entire timeline — including coarse-grained
         * zones. LOD 3 is telemetry-only and cannot be loaded. Returns true
         * on success, false otherwise.
         */
        loadScale0Snapshot: (snap) => {
            if (!snap) return false;
            const write     = bridge.setScale0LatticeBuffer?.bind(bridge);
            const writeFlux = bridge.setScale0FluxBuffer?.bind(bridge);
            if (!write || !writeFlux) return false;

            let lattice = snap.lattice;
            let flux    = snap.flux;
            const wave  = snap.wave;
            if (!lattice || !flux) return false;

            if (snap.lod && snap.lod > 0 && snap.lod < 3) {
                // Lazy-load the upsampler to avoid a static import cycle.
                // eslint-disable-next-line no-undef
                const N = bridge.latticeSize || snap.N || 32;
                const mod = (typeof window !== 'undefined') ? window.__ftdTimelineLod : null;
                if (!mod) {
                    // Fallback: fail instead of corrupting state with mismatched sizes.
                    return false;
                }
                lattice = mod.upsampleScalar(lattice, N, snap.lod);
                flux    = mod.upsampleVec3(flux, N, snap.lod);
            } else if (snap.lod >= 3) {
                return false; // telemetry-only snapshot cannot reconstruct state
            }

            write(lattice);
            writeFlux(flux);
            if (wave && bridge.setScale0WaveBuffer) bridge.setScale0WaveBuffer(wave);
            if (bridge.setScale0Tick) bridge.setScale0Tick(snap.tick);
            if (bridge.setScale0ParticleList) bridge.setScale0ParticleList(snap.particles);
            return true;
        },
    };
}

function createScale1Capabilities(bridge) {
    return {
        tickScale1: () => bridge.peTick?.(),
        getScale1ParticleFrame: () => bridge.peGetParticleData?.(),
        getScale1Diagnostics: () => bridge.peGetDiagnostics?.(),
    };
}

function createScale2Capabilities(bridge) {
    return {
        tickScale2: () => bridge.aeTick?.(),
        getScale2AtomFrame: () => bridge.aeGetAtomData?.(),
        getScale2Diagnostics: () => bridge.aeGetDiagnostics?.(),
    };
}

function installCapabilityGetter(proto) {
    Object.defineProperty(proto, 'capabilities', {
        configurable: true,
        get() {
            if (!this._capabilities) {
                this._capabilities = {
                    scale0: createScale0Capabilities(this),
                    scale1: createScale1Capabilities(this),
                    scale2: createScale2Capabilities(this),
                };
            }
            return this._capabilities;
        },
    });
}

installCapabilityGetter(MockBridge.prototype);
installCapabilityGetter(WasmBridge.prototype);

// ── Re-exports from extracted modules ────────────────────────────────
// CosmicMockBridge moved to bridge/mock-scale5.js (Scale 5 N-body sim)
// createBridge factory moved to bridge/bridge-factory-dag.js
export { CosmicMockBridge } from './bridge/mock-scale5.js';
export { createBridge } from './bridge/bridge-factory-dag.js';
