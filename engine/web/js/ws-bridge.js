/**
 * WebSocket Bridge — connects web dashboard to native GPU engine.
 *
 * Drop-in replacement for MockBridge/WasmBridge. Same API surface:
 *   tick(), run(n), getParticleData(), getDiagnostics(), getEnergyAudit(),
 *   setToggle(name, val), getToggle(name), setParam(name, val),
 *   injectFlux(x,y,z,fx,fy,fz), injectParticle(x,y,z,state),
 *   createEntangledPair(x,y,z,fx,fy,fz), injectWavepacket(x,y,z,state),
 *   resize(size), reset()
 *
 * The native C++ ws_server.exe runs RenderBridge (auto-GPU on CUDA builds)
 * and communicates over WebSocket on localhost:9100.
 */

import { debugLog } from './core/log.js';
import { MockBridge } from './bridge/mock-bridge.js';
import { runSetupScenario } from './bridge/scenarios/index.js';

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


export class WebSocketBridge {
    constructor(url = 'ws://127.0.0.1:9100') {
        this._url = url;
        this._ws = null;
        this._connected = false;
        this._pendingQueue = [];  // FIFO queue of {resolve, reject}
        this._binaryResolve = null;  // for particle data (binary frames)

        this.isWasm = false;
        this.isNativeGPU = true;
        this.ready = false;
        this.latticeSize = 32;

        // Toggle state mirror (updated from server, defaults match config/toggles.js).
        // These are overwritten by the server's actual state on connect,
        // but provide sane fallbacks if the server is slow to respond.
        // Audit P1-3 fix (2026-05-27): selective_damping defaulted to false
        // and weak_transmutation defaulted to true here — both inverted
        // relative to config/toggles.js and MockBridge. Now synced.
        this._toggles = {
            wave_propagation: true, coupling: true, damping: true, genesis: true,
            gauss_projection: true, forces: true, gravity: false, movement: true,
            poisson_coulomb: true, lorentz_force: false, selective_damping: true,
            larmor_radiation: false, dual_substrate: false, confinement: false,
            weak_transmutation: false,
            color_forces: false, strong_force: false, triad_binding: false,
            pair_production: false, exchange_force: false, latency_field: false,
        };

        // Cached particle data (reused between frames)
        this._particleData = { positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), count: 0 };

        // Cached diagnostics
        this._lastDiag = null;
        this._lastAudit = null;

        // Visual settings placeholder
        this._visualSettings = null;
        this._fallback = null;

        // High-frequency query throttling flags (Audit P0-4 fix)
        this._diagnosticsInFlight = false;
        this._energyInFlight = false;

        // Cached flux volume and slice data (async WebSocket channel)
        this._sliceCache = {};
        this._volumeCache = null;
        this._boundaryShape = 'cube';
        this._reflectiveBoundary = false;
    }

    async connect() {
        // Guard: reject if already connected or connecting to prevent duplicate sockets
        if (this._connected) {
            return Promise.resolve(this);
        }
        if (this._ws && this._ws.readyState === WebSocket.CONNECTING) {
            return Promise.reject(new Error('Connection already in progress'));
        }
        return new Promise((resolve, reject) => {
            let connectTimeout = null;
            try {
                this._ws = new WebSocket(this._url);
                this._ws.binaryType = 'arraybuffer';

                this._ws.onopen = () => {
                    if (connectTimeout) clearTimeout(connectTimeout);
                    this._connected = true;
                    this.ready = true;
                    debugLog('[ws-bridge] Connected to native GPU engine');
                    // Query info
                    this._sendJSON({ cmd: 'info' }).then(info => {
                        this.latticeSize = info.latticeSize || 32;
                        this.isNativeGPU = info.gpu || false;
                        debugLog(`[ws-bridge] Engine: L=${this.latticeSize}, GPU=${this.isNativeGPU}`);
                    }).catch(err => {
                        debugLog('[ws-bridge] Failed to query engine info:', err.message);
                    });
                    resolve(this);
                };

                this._ws.onmessage = (event) => {
                    if (event.data instanceof ArrayBuffer) {
                        // Binary frame = particle data
                        this._handleBinary(event.data);
                    } else {
                        // Text frame = JSON response
                        this._handleJSON(event.data);
                    }
                };

                this._ws.onclose = () => {
                    if (connectTimeout) clearTimeout(connectTimeout);
                    const wasConnected = this._connected;
                    this._connected = false;
                    this.ready = false;
                    this._diagnosticsInFlight = false;
                    this._energyInFlight = false;
                    debugLog('[ws-bridge] Disconnected');
                    // Drain pending queue — reject all waiting promises cleanly and clear timers
                    while (this._pendingQueue.length > 0) {
                        const pending = this._pendingQueue.shift();
                        if (pending.timeoutId) clearTimeout(pending.timeoutId);
                        pending.reject(new Error('WebSocket closed'));
                    }
                    // Auto-reconnect with exponential backoff ONLY if we were previously connected
                    if (wasConnected) {
                        this._scheduleReconnect();
                    }
                };

                this._ws.onerror = (err) => {
                    if (connectTimeout) clearTimeout(connectTimeout);
                    this._connected = false;
                    this.ready = false;
                    this._diagnosticsInFlight = false;
                    this._energyInFlight = false;
                    // Drain pending queue — reject all waiting promises cleanly and clear timers
                    while (this._pendingQueue.length > 0) {
                        const pending = this._pendingQueue.shift();
                        if (pending.timeoutId) clearTimeout(pending.timeoutId);
                        pending.reject(err || new Error('WebSocket error'));
                    }
                    reject(err);
                };

                // Timeout after 5 seconds
                connectTimeout = setTimeout(() => {
                    if (!this._connected) {
                        try {
                            this._ws.close();
                        } catch (e) {}
                        reject(new Error('WebSocket connection timeout'));
                    }
                }, 5000);
            } catch (e) {
                if (connectTimeout) clearTimeout(connectTimeout);
                reject(e);
            }
        });
    }

    /**
     * Exponential backoff reconnection: 1s -> 2s -> 4s -> 8s -> ... -> 30s cap.
     *
     * The _reconnecting flag prevents duplicate reconnect chains (e.g., if
     * onclose fires while a reconnect attempt is already in flight).
     * On success, the flag is cleared and the delay resets for future disconnects.
     * On failure, delay doubles up to maxDelay (30s), then retries indefinitely.
     *
     * NOTE: There is no maximum attempt count — reconnection continues forever.
     * This is intentional for long-running simulation sessions where the native
     * engine may be restarted. A UI indicator should show "disconnected" state.
     */
    _scheduleReconnect() {
        if (this._reconnecting) return;
        this._reconnecting = true;
        const maxDelay = 30000;
        let delay = 1000;
        const attempt = () => {
            debugLog(`[ws-bridge] Reconnecting in ${delay / 1000}s...`);
            setTimeout(() => {
                this.connect().then(() => {
                    this._reconnecting = false;
                    debugLog('[ws-bridge] Reconnected');
                }).catch(() => {
                    delay = Math.min(delay * 2, maxDelay);
                    attempt();
                });
            }, delay);
        };
        attempt();
    }

    // ── Command helpers ──────────────────────────────────────────────

    _sendJSON(obj) {
        return new Promise((resolve, reject) => {
            if (!this._connected) { reject(new Error('Not connected')); return; }
            // Guard: cap pending queue at 64 to prevent unbounded memory growth
            // if the server stops responding. Oldest entries are already timing
            // out after 5s, but a burst of rapid calls could still accumulate.
            if (this._pendingQueue.length >= 64) {
                reject(new Error('Pending queue full (64 commands in flight)'));
                return;
            }
            // Timeout
            const timeoutId = setTimeout(() => {
                const idx = this._pendingQueue.findIndex(p => p.resolve === resolve);
                if (idx >= 0) {
                    this._pendingQueue.splice(idx, 1);
                    reject(new Error('Command timeout'));
                    // Force-close the WebSocket to trigger a clean disconnect, drain the queue, and reconnect.
                    if (this._ws) {
                        debugLog('[ws-bridge] Force-closing WebSocket due to command timeout');
                        try {
                            this._ws.close();
                        } catch (_e) {}
                    }
                }
            }, 5000);

            // FIFO queue — server doesn't echo _id, so resolve in order
            this._pendingQueue.push({ resolve, reject, timeoutId });
            this._ws.send(JSON.stringify(obj));
        });
    }

    _sendAndForget(obj) {
        if (!this._connected) return;
        this._ws.send(JSON.stringify(obj));
    }

    _handleJSON(text) {
        try {
            const data = JSON.parse(text);
            // Check fire-and-forget cached responses first, preventing them
            // from mistakenly resolving pending command promises in the queue!
            if (data.type === 'flux_slice') {
                const key = `${data.axis}_${data.index}`;
                this._sliceCache[key] = new Float64Array(data.data);
                return;
            }
            if (data.type === 'flux_volume') {
                this._volumeCache = new Float64Array(data.data);
                return;
            }

            // Resolve next pending promise in FIFO order, or drop if none waiting
            if (this._pendingQueue.length > 0) {
                const { resolve } = this._pendingQueue.shift();
                resolve(data);
                return;
            }
            // else: fire-and-forget response, just cache useful fields
            if (data.tick !== undefined) this._lastTick = data.tick;
            if (data.manifested !== undefined && data.tick !== undefined) this._lastDiag = data;
        } catch (e) {
            console.warn('[ws-bridge] Bad JSON:', text);
        }
    }

    _handleBinary(buf) {
        // Format: [uint32 count][float32 pos[3N]][float32 col[3N]][float32 size[N]]
        // Validate frame integrity before creating typed array views.
        if (buf.byteLength < 4) {
            console.warn('[ws-bridge] Binary frame too short:', buf.byteLength);
            return;
        }
        const view = new DataView(buf);
        const count = view.getUint32(0, true);  // little-endian
        const offset = 4;
        const posBytes = count * 3 * 4;
        const colBytes = count * 3 * 4;
        const sizeBytes = count * 4;
        const expectedBytes = offset + posBytes + colBytes + sizeBytes;

        if (buf.byteLength < expectedBytes) {
            console.warn(`[ws-bridge] Truncated binary frame: got ${buf.byteLength}, expected ${expectedBytes} for ${count} particles`);
            return;
        }

        this._particleData = {
            positions: new Float32Array(buf, offset, count * 3),
            colors: new Float32Array(buf, offset + posBytes, count * 3),
            sizes: new Float32Array(buf, offset + posBytes + colBytes, count),
            count
        };

        // Resolve binary promise if pending
        if (this._binaryResolve) {
            this._binaryResolve(this._particleData);
            this._binaryResolve = null;
        }
    }

    // ── Public API (matches MockBridge/WasmBridge) ───────────────────

    tick() {
        // Fire-and-forget for speed — diagnostics fetched separately
        this._sendAndForget({ cmd: 'tick' });
    }

    run(n) {
        this._sendAndForget({ cmd: 'run', n });
    }

    getParticleData() {
        // Request particle data — server sends binary frame
        this._sendAndForget({ cmd: 'get_particles' });
        // Return cached data (binary arrives async, will be ready next frame)
        return this._particleData;
    }

    // WARNING: Only one async particle request can be in flight at a time.
    // Calling getParticleDataAsync() again before the previous resolves will
    // orphan the old promise (it will never resolve). This is acceptable for
    // the render loop (one request per frame), but callers must not queue these.
    async getParticleDataAsync() {
        return new Promise((resolve) => {
            this._binaryResolve = resolve;
            this._sendAndForget({ cmd: 'get_particles' });
        });
    }

    getDiagnostics() {
        // Fire request if not already in flight, return cached
        if (this._connected && !this._diagnosticsInFlight) {
            this._diagnosticsInFlight = true;
            this._sendJSON({ cmd: 'get_diagnostics' })
                .then(d => {
                    this._lastDiag = d;
                    this._diagnosticsInFlight = false;
                })
                .catch(() => {
                    this._diagnosticsInFlight = false;
                });
        }
        if (this._lastDiag) return this._lastDiag;
        return {
            tick: 0, physicalTime: 0, dt: 1,
            manifested: 0, positive: 0, negative: 0,
            totalFlux: 0, totalEnergy: 0,
            maxBandwidth: 0, avgDrag: 0, entropy: 0,
            chargeBalance: 0,
            spinUp: 0, spinDown: 0,
            colorless: 0, colorRed: 0, colorGreen: 0, colorBlue: 0,
            angMomX: 0, angMomY: 0, angMomZ: 0
        };
    }

    getEnergyAudit() {
        // Fire request if not already in flight, return cached
        if (this._connected && !this._energyInFlight) {
            this._energyInFlight = true;
            this._sendJSON({ cmd: 'get_energy_audit' })
                .then(d => {
                    this._lastAudit = d;
                    this._energyInFlight = false;
                })
                .catch(() => {
                    this._energyInFlight = false;
                });
        }
        if (this._lastAudit) return this._lastAudit;
        return {
            fieldEnergy: 0, waveEnergy: 0, particleKE: 0, totalEnergy: 0,
            gaussViolation: 0, maxGaussError: 0, coulombPE: 0,
            eFieldEnergy: 0, bFieldEnergy: 0, chargeTotal: 0, manifestedCount: 0
        };
    }

    setToggle(name, value) {
        if (name in this._toggles) this._toggles[name] = value;
        this._sendAndForget({ cmd: 'set_toggle', name, value });
        this._ensureFallback().setToggle(name, value);
    }

    getToggle(name) {
        return this._toggles[name] ?? true;
    }

    setParam(name, value) {
        this._sendAndForget({ cmd: 'set_param', name, value });
        this._ensureFallback().setParam(name, value);
    }

    injectFlux(x, y, z, fx, fy, fz) {
        this._sendAndForget({ cmd: 'inject_flux', x, y, z, fx, fy, fz });
        this._ensureFallback().injectFlux(x, y, z, fx, fy, fz);
    }

    injectParticle(x, y, z, state) {
        this._sendAndForget({ cmd: 'inject_particle', x, y, z, state, fx: 0.1, fy: 0, fz: 0 });
        this._ensureFallback().injectParticle(x, y, z, state);
    }

    injectWavepacket(x, y, z, state) {
        this._sendAndForget({ cmd: 'inject_wavepacket', x, y, z, state });
        this._ensureFallback().injectWavepacket(x, y, z, state);
    }

    createEntangledPair(x, y, z, fx = 0.511, fy = 0, fz = 0) {
        this._sendAndForget({ cmd: 'create_pair', x, y, z, fx, fy, fz });
        this._ensureFallback().createEntangledPair(x, y, z, fx, fy, fz);
    }

    // MockBridge private method/array delegation for full scenario-loading support
    get _particles() { return this._ensureFallback()._particles; }
    set _particles(v) { this._ensureFallback()._particles = v; }

    _initFluxGrid() { this._ensureFallback()._initFluxGrid(); }

    _injectFlux(x, y, z, fx, fy, fz) {
        this._sendAndForget({ cmd: 'inject_flux_add', x, y, z, fx, fy, fz });
        this._ensureFallback()._injectFlux(x, y, z, fx, fy, fz);
    }

    _injectWaveVel(x, y, z, wx, wy, wz) {
        this._sendAndForget({ cmd: 'inject_wave_vel_add', x, y, z, wx, wy, wz });
        this._ensureFallback()._injectWaveVel(x, y, z, wx, wy, wz);
    }

    _ensureFallback() {
        if (!this._fallback) {
            this._fallback = new MockBridge(this.latticeSize);
        }
        return this._fallback;
    }

    async resize(size) {
        this.latticeSize = size;
        if (this._fallback) this._fallback.latticeSize = size;
        return this._sendJSON({ cmd: 'resize', size });
    }

    reset() {
        this._sendAndForget({ cmd: 'reset' });
        if (this._fallback) this._fallback.reset();
    }

    setupScenario(name) {
        this.reset();
        if (this._connected) {
            this._sendAndForget({ cmd: 'setup_scenario', name });
        }
        this._ensureFallback().setupScenario(name);
    }

    setDt(dt) { this.setParam('dt', dt); }
    getDt() { return 1.0; }
    getPhysicalTime() { return 0.0; }

    getLagrangian() {
        return {
            fieldKinetic: 0, fieldGradient: 0,
            bornInfeld: 0, coupling: 0, velocity: 0, gauss: 0, dissipation: 0,
            total: 0, hamiltonian: 0, totalAction: 0, gaussViolation: 0, maxGaussError: 0,
            totalFluxMag: 0, totalWaveEnergy: 0, manifested: 0, locked: 0
        };
    }

    getConstants() {
        return null;
    }

    inspectVoxel(x, y, z) {
        return {
            x, y, z,
            state: 0,
            density: 0,
            fluxX: 0, fluxY: 0, fluxZ: 0,
            Emag: 0,
            Ex: 0, Ey: 0, Ez: 0,
            Bx: 0, By: 0, Bz: 0
        };
    }

    getForceAt(x, y, z) {
        return { x: 0, y: 0, z: 0 };
    }

    getFluxSlice(axis, index) {
        if (this._connected) {
            this._sendAndForget({ cmd: 'get_flux_slice', axis, index });
        }
        const key = `${axis}_${index}`;
        return this._sliceCache[key] || new Float64Array(0);
    }
    getFluxVolume() {
        if (this._connected) {
            this._sendAndForget({ cmd: 'get_flux_volume' });
        }
        return this._volumeCache || new Float64Array(0);
    }

    // Samplers returning safe empty frozen objects to avoid browser layout/rendering crashes
    getEFieldSampled(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    sampleVAtRay(x1, y1, z1, x2, y2, z2, n) {
        return { positions: new Float32Array(0), V: new Float32Array(0), count: 0 };
    }
    getBFieldSampled(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    getPoyntingSampled(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    getDivJSampled(stride = 2) { return EMPTY_SCALAR_SAMPLE; }
    getFluxVectorSampled(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    getForceFieldSampled(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    getVorticitySampled(stride = 2) { return EMPTY_SCALAR_SAMPLE; }
    getHelicitySampled(stride = 2) { return EMPTY_SCALAR_SAMPLE; }
    getCurlJSampled(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    getGravityFieldSampled(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    getEMForceField(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    getGravityForceField(stride = 2) { return EMPTY_FIELD_SAMPLE; }
    getStrongForceField(stride = 2) { return EMPTY_FIELD_SAMPLE; }

    // Scale 1 (ParticleEngine) fallback delegation
    initPE() { this._ensureFallback().initPE(); }
    resetPE() { this._ensureFallback().resetPE(); }
    peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        return this._ensureFallback().peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff);
    }
    peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff) {
        return this._ensureFallback().peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff);
    }
    peTick() { this._ensureFallback().peTick(); }
    peGetParticleData() { return this._ensureFallback().peGetParticleData(); }
    peGetDiagnostics() { return this._ensureFallback().peGetDiagnostics(); }
    peGetExtendedData() { return this._ensureFallback().peGetExtendedData(); }
    peGetForces() { return this._ensureFallback().peGetForces(); }
    peGetFieldSources() { return this._ensureFallback().peGetFieldSources(); }
    peSetDt(dt) { this._ensureFallback().peSetDt(dt); }
    peGetDt() { return this._ensureFallback().peGetDt(); }
    peSetSoftening(s) { this._ensureFallback().peSetSoftening(s); }
    peSetCoulomb(e) { this._ensureFallback().peSetCoulomb(e); }
    peSetDamping(e) { this._ensureFallback().peSetDamping(e); }
    peSetGravity(e) { this._ensureFallback().peSetGravity(e); }
    peSetLorentz(e) { this._ensureFallback().peSetLorentz(e); }
    peSetExchange(e) { this._ensureFallback().peSetExchange(e); }
    peSetStrong(e) { this._ensureFallback().peSetStrong(e); }
    peSetMagneticDipole(e) { this._ensureFallback().peSetMagneticDipole(e); }
    peSetSpinOrbit(e) { this._ensureFallback().peSetSpinOrbit(e); }
    peSetRadiation(e) { this._ensureFallback().peSetRadiation(e); }
    peSetRelativistic(e) { this._ensureFallback().peSetRelativistic(e); }
    peSetRelativisticVerlet(e) { this._ensureFallback().peSetRelativisticVerlet(e); }
    peGetToggle(name) { return this._ensureFallback().peGetToggle(name); }
    peGetBackendCapabilities() { return this._ensureFallback().peGetBackendCapabilities(); }
    peParticleCount() { return this._ensureFallback().peParticleCount(); }
    peClear() { this._ensureFallback().peClear(); }
    peGetParticleTypes() { return this._ensureFallback().peGetParticleTypes(); }
    peInspectParticle(id) { return this._ensureFallback().peInspectParticle(id); }

    // Scale 2 (AtomEngine) fallback delegation
    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        this._ensureFallback().setBoundaryShape(shape);
    }
    setReflectiveBoundary(on) {
        this._reflectiveBoundary = !!on;
        this._ensureFallback().setReflectiveBoundary(on);
    }
    initAE() { this._ensureFallback().initAE(); }
    resetAE() { this._ensureFallback().resetAE(); }
    aeAddAtom(Z, x, y, z, vx, vy, vz, charge, N) {
        return this._ensureFallback().aeAddAtom(Z, x, y, z, vx, vy, vz, charge, N);
    }
    aeAddLockedAtom(Z, x, y, z, charge, N) {
        return this._ensureFallback().aeAddLockedAtom(Z, x, y, z, charge, N);
    }
    aeCreateBond(idA, idB, order) { this._ensureFallback().aeCreateBond(idA, idB, order); }
    aeTick() { this._ensureFallback().aeTick(); }
    aeGetAtomData() { return this._ensureFallback().aeGetAtomData(); }
    aeGetDiagnostics() { return this._ensureFallback().aeGetDiagnostics(); }
    aeGetFieldSources() { return this._ensureFallback().aeGetFieldSources(); }
    aeSetDt(dt) { this._ensureFallback().aeSetDt(dt); }
    aeGetDt() { return this._ensureFallback().aeGetDt(); }
    aeSetSoftening(s) { this._ensureFallback().aeSetSoftening(s); }
    aeSetDamping(e) { this._ensureFallback().aeSetDamping(e); }
    aeSetBonding(e) { this._ensureFallback().aeSetBonding(e); }
    aeSetIonic(e) { this._ensureFallback().aeSetIonic(e); }
    aeSetVdw(e) { this._ensureFallback().aeSetVdw(e); }
    aeSetBondsForce(e) { this._ensureFallback().aeSetBondsForce(e); }
    aeSetSpeedLimit(e) { this._ensureFallback().aeSetSpeedLimit(e); }
    aeSetHBonds(e) { this._ensureFallback().aeSetHBonds(e); }
    aeSetAngleStrain(e) { this._ensureFallback().aeSetAngleStrain(e); }
    aeSetDipoleDipole(e) { this._ensureFallback().aeSetDipoleDipole(e); }
    aeSetThermostat(e) { this._ensureFallback().aeSetThermostat(e); }
    aeSetThermostatTemp(t) { this._ensureFallback().aeSetThermostatTemp(t); }
    aeSetElectronegativity(e) { this._ensureFallback().aeSetElectronegativity(e); }
    aePreBond() { this._ensureFallback().aePreBond(); }
    aeAtomCount() { return this._ensureFallback().aeAtomCount(); }
    aeInspectAtom(id) { return this._ensureFallback().aeInspectAtom(id); }
    aeClear() { this._ensureFallback().aeClear(); }

    // Stubs for compatibility with MockBridge API
    currentTick() { return this._lastDiag?.tick ?? 0; }
    setVisualSettings(vs) { this._visualSettings = vs; }
    loadScenario() {}
}

/**
 * Try to connect to native GPU engine, return null if unavailable.
 */
export async function tryNativeBridge(latticeSize = 32) {
    const bridge = new WebSocketBridge(`ws://127.0.0.1:9100`);
    try {
        await bridge.connect();
        // Request resize to match desired lattice size
        if (bridge.latticeSize !== latticeSize) {
            await bridge.resize(latticeSize);
        }
        return bridge;
    } catch (e) {
        debugLog('[ws-bridge] Native GPU engine not available:', e.message);
        return null;
    }
}
