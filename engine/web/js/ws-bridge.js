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

export class WebSocketBridge {
    constructor(url = 'ws://localhost:9100') {
        this._url = url;
        this._ws = null;
        this._connected = false;
        this._pendingQueue = [];  // FIFO queue of {resolve, reject}
        this._binaryResolve = null;  // for particle data (binary frames)

        this.isWasm = false;
        this.isNativeGPU = true;
        this.ready = false;
        this.latticeSize = 32;

        // Toggle state mirror (updated from server, defaults match term_toggles.h)
        this._toggles = {
            wave_propagation: true, coupling: true, damping: true, genesis: true,
            gauss_projection: true, forces: true, gravity: true, movement: true,
            poisson_coulomb: true, lorentz_force: true, selective_damping: true,
            larmor_radiation: false, dual_substrate: true, weak_transmutation: true,
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
    }

    async connect() {
        return new Promise((resolve, reject) => {
            try {
                this._ws = new WebSocket(this._url);
                this._ws.binaryType = 'arraybuffer';

                this._ws.onopen = () => {
                    this._connected = true;
                    this.ready = true;
                    console.log('[ws-bridge] Connected to native GPU engine');
                    // Query info
                    this._sendJSON({ cmd: 'info' }).then(info => {
                        this.latticeSize = info.latticeSize || 32;
                        this.isNativeGPU = info.gpu || false;
                        console.log(`[ws-bridge] Engine: L=${this.latticeSize}, GPU=${this.isNativeGPU}`);
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
                    this._connected = false;
                    this.ready = false;
                    console.log('[ws-bridge] Disconnected');
                };

                this._ws.onerror = (err) => {
                    this._connected = false;
                    this.ready = false;
                    reject(err);
                };

                // Timeout after 2 seconds
                setTimeout(() => {
                    if (!this._connected) {
                        this._ws.close();
                        reject(new Error('WebSocket connection timeout'));
                    }
                }, 2000);
            } catch (e) {
                reject(e);
            }
        });
    }

    // ── Command helpers ──────────────────────────────────────────────

    _sendJSON(obj) {
        return new Promise((resolve, reject) => {
            if (!this._connected) { reject(new Error('Not connected')); return; }
            // FIFO queue — server doesn't echo _id, so resolve in order
            this._pendingQueue.push({ resolve, reject });
            this._ws.send(JSON.stringify(obj));
            // Timeout
            setTimeout(() => {
                const idx = this._pendingQueue.findIndex(p => p.resolve === resolve);
                if (idx >= 0) {
                    this._pendingQueue.splice(idx, 1);
                    reject(new Error('Command timeout'));
                }
            }, 5000);
        });
    }

    _sendAndForget(obj) {
        if (!this._connected) return;
        this._ws.send(JSON.stringify(obj));
    }

    _handleJSON(text) {
        try {
            const data = JSON.parse(text);
            // Resolve next pending promise in FIFO order, or drop if none waiting
            if (this._pendingQueue.length > 0) {
                const { resolve } = this._pendingQueue.shift();
                resolve(data);
            }
            // else: fire-and-forget response, just cache useful fields
            if (data.tick !== undefined) this._lastTick = data.tick;
            if (data.manifested !== undefined) this._lastDiag = data;
        } catch (e) {
            console.warn('[ws-bridge] Bad JSON:', text);
        }
    }

    _handleBinary(buf) {
        // Format: [uint32 count][float32 pos[3N]][float32 col[3N]][float32 size[N]]
        const view = new DataView(buf);
        const count = view.getUint32(0, true);  // little-endian
        const offset = 4;
        const posBytes = count * 3 * 4;
        const colBytes = count * 3 * 4;
        const sizeBytes = count * 4;

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

    async getParticleDataAsync() {
        return new Promise((resolve) => {
            this._binaryResolve = resolve;
            this._sendAndForget({ cmd: 'get_particles' });
        });
    }

    getDiagnostics() {
        // Fire request, return cached
        this._sendJSON({ cmd: 'get_diagnostics' }).then(d => { this._lastDiag = d; });
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
        this._sendJSON({ cmd: 'get_energy_audit' }).then(d => { this._lastAudit = d; });
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
    }

    getToggle(name) {
        return this._toggles[name] ?? true;
    }

    setParam(name, value) {
        this._sendAndForget({ cmd: 'set_param', name, value });
    }

    injectFlux(x, y, z, fx, fy, fz) {
        this._sendAndForget({ cmd: 'inject_flux', x, y, z, fx, fy, fz });
    }

    injectParticle(x, y, z, state) {
        this._sendAndForget({ cmd: 'inject_particle', x, y, z, state, fx: 0.1, fy: 0, fz: 0 });
    }

    injectWavepacket(x, y, z, state) {
        this._sendAndForget({ cmd: 'inject_wavepacket', x, y, z, state });
    }

    createEntangledPair(x, y, z, fx = 0.511, fy = 0, fz = 0) {
        this._sendAndForget({ cmd: 'create_pair', x, y, z, fx, fy, fz });
    }

    async resize(size) {
        this.latticeSize = size;
        return this._sendJSON({ cmd: 'resize', size });
    }

    reset() {
        this._sendAndForget({ cmd: 'reset' });
    }

    // Stubs for compatibility with MockBridge API
    currentTick() { return this._lastDiag?.tick ?? 0; }
    setVisualSettings(vs) { this._visualSettings = vs; }
    setBoundaryShape() {}
    loadScenario() {}
}

/**
 * Try to connect to native GPU engine, return null if unavailable.
 */
export async function tryNativeBridge(latticeSize = 32) {
    const bridge = new WebSocketBridge(`ws://localhost:9100`);
    try {
        await bridge.connect();
        // Request resize to match desired lattice size
        if (bridge.latticeSize !== latticeSize) {
            await bridge.resize(latticeSize);
        }
        return bridge;
    } catch (e) {
        console.log('[ws-bridge] Native GPU engine not available:', e.message);
        return null;
    }
}
