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
 * The native C++ ws_server runs RenderBridge (auto-GPU on CUDA builds) and
 * communicates over WebSocket on localhost. Port 9100 is the default; the
 * Windows desktop shell supplies ?wsPort=<port> when configured otherwise.
 */

import { debugLog } from './core/log.js';
import { WasmBridge } from './bridge/wasm-bridge.js';
import { particleDataToList, samplerOr } from './bridge/bridge-contract.js';
import { K_B } from './constants.js';
import { parseFtv2Frame } from './lib/ftv2.js';
import { parseNativeWsPort } from './lib/origin-policy.js';

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
const EMPTY_KNOT_TELEMETRY = Object.freeze({ ids: new Int32Array(0), signs: new Int32Array(0), birth: new Int32Array(0), age: new Int32Array(0), size: new Int32Array(0), peak: new Int32Array(0), fields: new Float32Array(0), stride: 11, count: 0 });
const EMPTY_KNOT_EVENTS = Object.freeze({ tick: new Int32Array(0), type: new Int32Array(0), nparents: new Int32Array(0), nchildren: new Int32Array(0), sign: new Int32Array(0), count: 0 });
const EMPTY_KNOT_AGG = Object.freeze({ alive: 0, netCharge: 0, births: 0, deaths: 0, fissions: 0, fusions: 0 });
// Legacy dense flux-volume frames begin with little-endian ASCII "FTV1",
// followed by uint32 sample count and count float32 magnitudes. Native builds
// now send compact FTV2 frames so large lattices never cross the socket merely
// to discard most voxels in the renderer.
const FLUX_VOLUME_MAGIC = 0x31565446;
// [u32 "FTV2"][u32 latticeSize][u32 effectiveStride][u32 origin][u32 axisCount]
// [float32 density[axisCount^3]], x-fastest. Legacy 16-byte headers (no origin)
// are still accepted by parseFtv2Frame.
const FLUX_VOLUME_COMPACT_MAGIC = 0x32565446;
const FLUX_VOLUME_AXIS_SAMPLES = 53;
// Extended particle frame matching the WASM particle contract. Positions are
// already mechanical positions (cell centre + bounded movement remainder).
const PARTICLE_FRAME_MAGIC = 0x32505446; // little-endian ASCII "FTP2"
// Binary sampled-field frame, little-endian ASCII "FTS1". Layout:
//   u32 magic, u32 request token, u32 kind code, u32 components, u32 count,
//   float32 positions[3*count], float32 payload[components*count].
// Keeping sampled fields binary matters at native lattice sizes: a compact
// stride-2 vector field is already several MiB, while JSON would multiply both
// allocation pressure and time spent on the browser main thread.
const FIELD_SAMPLE_MAGIC = 0x31535446;
// FTS2 extends FTS1 with u32 effectiveStride + u32 origin after count.
const FIELD_SAMPLE_V2_MAGIC = 0x32535446;
const FIELD_SAMPLE_KINDS = Object.freeze([
    'e', 'b', 'poynting', 'divJ', 'fluxVector', 'vorticity', 'helicity',
    'kretschmann', 'latency', 'fisher', 'coherence', 'curlJ', 'state',
    'gaussResidual', 'em', 'gravity', 'strong',
    'poissonLatency',
]);
const FIELD_SAMPLE_KIND_CODES = new Map(FIELD_SAMPLE_KINDS.map((kind, code) => [kind, code]));
// 3-component (vector) kinds — used only to pick getFieldSlices' pre-first-frame
// fallback shape; the real shape always comes from the FTS2 `components` field.
const WS_VECTOR_FIELD_KINDS = new Set(['e', 'b', 'poynting', 'fluxVector', 'curlJ', 'em', 'gravity', 'strong']);
const DEFAULT_COMMAND_TIMEOUT_MS = 5000;
const LONG_OPERATION_TIMEOUT_MS = 120000;
const DIAGNOSTIC_COMMAND_TIMEOUT_MS = 30000;
const SCENARIO_COALESCE_MS = 50;
const LIVE_PROFILE_COALESCE_MS = 0;
// FTS1 payloads can each be several MiB. A single slice-panel render asks for
// many kinds, so bound the native command stream and let the demand map serve
// them round-robin as responses arrive.
const MAX_FIELD_SAMPLE_REQUESTS_IN_FLIGHT = 2;
// One-voxel inspector/force probes are compact, but hundreds of independent
// requests still head-of-line block the serialized native command stream.
const MAX_POINT_QUERY_REQUESTS_IN_FLIGHT = 4;
// FIFO cap for the point-probe (voxel/force-at) result caches. They are keyed by
// "x,y,z" and cleared only on scenario change, so without a bound every distinct
// voxel a panel hovers/inspects/probes within one long scenario accumulates a
// permanent entry (×2 caches) on lattices up to 256³. The per-tick epoch check
// already invalidates stale values, so a plain FIFO evict is sufficient.
const MAX_POINT_CACHE = 4096;
const MAX_SLICE_CACHE = 64;
const TELEMETRY_GROUPS = Object.freeze([
    'diagnostics', 'audit', 'lagrangian', 'gravity',
]);
const TELEMETRY_DEMAND_TTL_MS = 6000;
const TELEMETRY_PUSH_STALL_POLL_MS = 1500;

function telemetryNow() {
    return (typeof performance !== 'undefined' && typeof performance.now === 'function')
        ? performance.now() : Date.now();
}

function hasOwn(value, key) {
    return !!value && Object.prototype.hasOwnProperty.call(value, key);
}


export class WebSocketBridge {
    constructor(url = 'ws://127.0.0.1:9100') {
        this._url = url;
        this._ws = null;
        this._connected = false;
        this._pendingQueue = [];  // FIFO queue of {resolve, reject}
        this._nextRequestId = 1;
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
        this._params = {};

        // Cached particle data (reused between frames)
        this._particleData = {
            positions: new Float32Array(0), colors: new Float32Array(0),
            sizes: new Float32Array(0), spin: new Float32Array(0),
            colorCharge: new Float32Array(0), count: 0,
        };

        // Cached diagnostics
        this._lastDiag = null;
        this._lastAudit = null;

        // Visual settings placeholder
        this._visualSettings = null;
        this._fallback = null;

        // Scientific telemetry is one coherent native snapshot.  Individual
        // panel getters only add their desired group to this request; they do
        // not each enqueue a competing CUDA reduction on the single socket.
        // This keeps rendering resident while still giving the sidebars a
        // coherent observation point between simulation ticks.
        this._telemetryInFlight = false;
        this._telemetryPumpScheduled = false;
        this._telemetryDemand = {
            diagnostics: false,
            audit: false,
            lagrangian: false,
            gravity: false,
        };

        // Compatibility mirrors retained for existing error/reset paths and
        // third-party panel integrations.  A single batch owns their state.
        this._resetTelemetryRequests();

        // The native server processes one command stream serially.  A render
        // loop can otherwise enqueue ticks faster than a large CUDA lattice can
        // execute them, making Pause/Resize appear frozen behind stale work.
        // Keep one simulation command in flight and coalesce render-loop ticks
        // into at most one follow-up command.
        this._simulationInFlight = false;
        this._simulationTicksInFlight = 0;
        this._simulationWatchdog = null;
        this._queuedSimulationTicks = 0;

        // Cached flux volume and slice data (async WebSocket channel)
        this._sliceCache = new Map();
        this._volumeCache = null;
        // Visual reads are asynchronous on the native socket.  Track the
        // engine-state epoch each cache request represents so repeated render
        // reads do not flood the socket, while a tick/injection still requests
        // a fresh frame.  The response handler wakes the paused renderer once
        // the cache is actually populated.
        this._visualEpoch = 1;
        this._particleRequestEpoch = 0;
        this._volumeRequestEpoch = 0;
        this._sliceRequestEpoch = new Map();
        this._particleRequestInFlight = false;
        this._volumeRequestInFlight = false;
        this._sliceRequestsInFlight = new Set();
        this._fieldSampleCache = new Map();
        this._fieldSampleRequestEpoch = new Map();
        this._fieldSampleRequestTokenByKey = new Map();
        this._fieldSampleRequestsByToken = new Map();
        this._fieldSampleDemandByKey = new Map();
        this._nextFieldSampleToken = 1;
        // The native scheduler may defer an expensive visual frame while a
        // compact telemetry observation is due. Keep one bounded retry rather
        // than repeatedly submitting bulk work into the same serialized FIFO.
        this._visualDeferredRetryTimer = null;
        this._visualDeferredRetryAt = 0;
        this._visualDeferredPending = false;
        this._boundaryShape = 'cube';
        this._reflectiveBoundary = false;
        this._fluxBoundaryMode = 2;
        this._preparedScenario = null;
        // Scenario selection is a transaction, not a stream of mutations.
        // The loader stages its toggle/boundary profile here, setupScenario()
        // records the seed id, and commitScenarioConfiguration() emits one
        // atomic native command. Rapid selections are last-write-wins so an
        // expensive L^3 RenderBridge is not rebuilt for every intermediate
        // option the user passes through.
        this._scenarioDraft = null;
        this._queuedScenarioProfile = null;
        this._scenarioRequestInFlight = false;
        this._scenarioDispatchTimer = null;
        this._scenarioRetryAfterMs = 0;
        this._activeScenario = null;
        // Confirmed engine profile versus optimistic UI intent. Live toggle and
        // boundary edits are serialized through `apply_profile`, which validates
        // the whole candidate and echoes the authoritative profile. Keeping the
        // confirmed snapshot lets a rejected dependency/conflict update roll
        // back instead of leaving getToggle()/the checkbox card lying.
        this._confirmedToggles = { ...this._toggles };
        this._confirmedFluxBoundaryMode = this._fluxBoundaryMode;
        this._liveProfileQueued = null;
        this._liveProfileInFlight = false;
        this._liveProfileDispatchTimer = null;
        this._profileGeneration = 0;
        this._connectionGeneration = 0;
        this._connectionRecoveryPending = false;
        // Set only for a poisoned CUDA snapshot fence. In that state a clean
        // desktop-owned WSL process restart is the safe recovery boundary;
        // reconnecting this page to the quarantined process would just loop.
        this._restartRequired = false;
        this._lastDiagnosticsRequestAt = 0;
        this._lastEnergyRequestAt = 0;
        this._lastLagrangianRequestAt = 0;
        this._lastGravityMetricRequestAt = 0;
        this._lastLagrangian = null;
        this._lastGravityMetric = null;
        // Separates scenario-owned scientific snapshots from the per-tick
        // visual epoch. A diagnostic request may outlive an atomic scenario
        // setup; generation-gating prevents that late reply from repainting
        // the new scenario with the old scenario's energy/footer values.
        this._scenarioDataGeneration = 0;
        this._voxelCache = new Map();
        this._voxelRequestEpoch = new Map();
        this._voxelRequestsInFlight = new Set();
        this._forceAtCache = new Map();
        this._forceAtRequestEpoch = new Map();
        this._forceAtRequestsInFlight = new Set();
        this._pointQueryRequestsInFlight = 0;
        // Teardown latch + tracked reconnect timer (see dispose()).
        this._disposed = false;
        this._reconnectTimer = null;
    }

    async connect() {
        // Guard: reject if already connected or connecting to prevent duplicate sockets
        if (this._restartRequired) {
            return Promise.reject(new Error('Native CUDA engine restart is required'));
        }
        if (this._connected) {
            return Promise.resolve(this);
        }
        if (this._ws && this._ws.readyState === WebSocket.CONNECTING) {
            return Promise.reject(new Error('Connection already in progress'));
        }
        return new Promise((resolve, reject) => {
            let connectTimeout = null;
            try {
                const socket = new WebSocket(this._url);
                this._ws = socket;
                this._connectionRecoveryPending = true;
                socket.binaryType = 'arraybuffer';

                socket.onopen = () => {
                    if (this._ws !== socket) return;
                    if (connectTimeout) clearTimeout(connectTimeout);
                    this._connected = true;
                    // Keep physics and readbacks behind a reconnect barrier
                    // until authoritative info arrives and the controller has
                    // synchronously staged the selected scenario profile.
                    this.ready = false;
                    this._resetVisualRequests();
                    this._markVisualDataDirty(true);
                    debugLog('[ws-bridge] Connected to native GPU engine');
                    // Resolve connect only after the authoritative server size
                    // is known.  Resolving on socket-open let tryNativeBridge()
                    // issue a resize while this request was still pending; the
                    // late info reply could then overwrite the resized size and
                    // make a 33^3 volume look like 64^3 to the renderer.
                    this._sendJSON({ cmd: 'info' }).then(info => {
                        if (this._ws !== socket) {
                            reject(new Error('Native connection was superseded'));
                            return;
                        }
                        if (info?.telemetryRecoveryRequired || info?.restartRequired) {
                            const error = new Error(
                                'Native CUDA telemetry recovery requires a clean engine restart.',
                            );
                            error.restartRequired = true;
                            this._handleRestartRequired({
                                operation: 'info', error: error.message, restartRequired: true,
                            }, socket);
                            reject(error);
                            return;
                        }
                        this.latticeSize = info.latticeSize || this.latticeSize;
                        this.isNativeGPU = info.gpu || false;
                        this._observeTelemetrySourceEpoch(info?.telemetrySourceEpoch);
                        debugLog(`[ws-bridge] Engine: L=${this.latticeSize}, GPU=${this.isNativeGPU}`);
                        if (this._queuedScenarioProfile) this._scheduleScenarioDispatch(0);
                        else if (this._liveProfileQueued) this._scheduleLiveProfileDispatch(0);
                        this._connectionGeneration++;
                        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
                        if (ctx?.bridge === this
                            && typeof ctx.onBridgeConnectionReady === 'function') {
                            try {
                                ctx.onBridgeConnectionReady({
                                    generation: this._connectionGeneration,
                                    info,
                                });
                            } catch (e) {
                                debugLog('[ws-bridge] Connection-ready callback failed:', e?.message || e);
                            }
                        }
                        // The callback above stages setup_scenario synchronously.
                        // Clear only the handshake barrier; queued profile work
                        // continues blocking simulation until its acknowledgement.
                        this._connectionRecoveryPending = false;
                        this.ready = true;
                        // Demand may have been declared while the socket was
                        // reconnecting. Reissue the control-plane subscription
                        // once the authoritative source is ready.
                        this._scheduleTelemetryDemandDispatch();
                        resolve(this);
                    }).catch(err => {
                        if (this._ws !== socket) {
                            reject(new Error('Native connection was superseded'));
                            return;
                        }
                        debugLog('[ws-bridge] Failed to query engine info:', err.message);
                        this._connected = false;
                        this.ready = false;
                        this._connectionRecoveryPending = false;
                        try {
                            socket.close();
                        } catch (e) {}
                        reject(new Error(`Native engine info query failed: ${err.message}`));
                    });
                };

                socket.onmessage = (event) => {
                    if (this._ws !== socket) return;
                    if (event.data instanceof ArrayBuffer) {
                        // Binary frame = particle data
                        this._handleBinary(event.data);
                    } else {
                        // Text frame = JSON response
                        this._handleJSON(event.data, socket);
                    }
                };

                socket.onclose = () => {
                    if (this._ws !== socket) return;
                    if (connectTimeout) clearTimeout(connectTimeout);
                    const wasConnected = this._connected;
                    this._connected = false;
                    this.ready = false;
                    this._connectionRecoveryPending = false;
                    this._resetTelemetryRequests();
                    this._resetSimulationRequests();
                    this._resetVisualRequests();
                    debugLog('[ws-bridge] Disconnected');
                    // Retire only work submitted on this socket. A delayed
                    // close from an old generation must not reject requests
                    // already issued on its replacement.
                    for (let i = this._pendingQueue.length - 1; i >= 0; i--) {
                        if (this._pendingQueue[i].socket !== socket) continue;
                        const [pending] = this._pendingQueue.splice(i, 1);
                        if (pending.timeoutId) clearTimeout(pending.timeoutId);
                        pending.reject(new Error('WebSocket closed'));
                    }
                    // Auto-reconnect with exponential backoff ONLY if we were previously connected
                    if (wasConnected) {
                        this._scheduleReconnect();
                    }
                };

                socket.onerror = (err) => {
                    if (this._ws !== socket) return;
                    if (connectTimeout) clearTimeout(connectTimeout);
                    // An error during the initial handshake belongs to the
                    // caller's connect() attempt. Once info has completed,
                    // however, browsers normally follow `error` with `close`;
                    // capture establishment before clearing flags so exactly
                    // one of those events starts the reconnect chain.
                    const wasEstablished = this._connected && this.ready;
                    this._connected = false;
                    this.ready = false;
                    this._connectionRecoveryPending = false;
                    this._resetTelemetryRequests();
                    this._resetSimulationRequests();
                    this._resetVisualRequests();
                    // Drain only this generation's tracked requests.
                    for (let i = this._pendingQueue.length - 1; i >= 0; i--) {
                        if (this._pendingQueue[i].socket !== socket) continue;
                        const [pending] = this._pendingQueue.splice(i, 1);
                        if (pending.timeoutId) clearTimeout(pending.timeoutId);
                        pending.reject(err || new Error('WebSocket error'));
                    }
                    if (wasEstablished) this._scheduleReconnect();
                    reject(err);
                };

                // Timeout after 5 seconds
                connectTimeout = setTimeout(() => {
                    if (this._ws === socket && !this._connected) {
                        this._connectionRecoveryPending = false;
                        try {
                            socket.close();
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
        if (this._reconnecting || this._restartRequired || this._disposed) return;
        this._reconnecting = true;
        const maxDelay = 30000;
        let delay = 1000;
        const attempt = () => {
            // Re-check every stage: dispose() or a restart-required fence set
            // AFTER the chain started must stop it (otherwise it loops forever
            // issuing no-op connect()s, and a disposed bridge never quiesces).
            if (this._disposed || this._restartRequired) { this._reconnecting = false; return; }
            debugLog(`[ws-bridge] Reconnecting in ${delay / 1000}s...`);
            this._reconnectTimer = setTimeout(() => {
                this._reconnectTimer = null;
                if (this._disposed || this._restartRequired) { this._reconnecting = false; return; }
                this.connect().then(() => {
                    this._reconnecting = false;
                    debugLog('[ws-bridge] Reconnected');
                }).catch(() => {
                    if (this._disposed || this._restartRequired) { this._reconnecting = false; return; }
                    delay = Math.min(delay * 2, maxDelay);
                    attempt();
                });
            }, delay);
        };
        attempt();
    }

    /**
     * Tear down the bridge: stop reconnecting, clear every timer, reject in-flight
     * commands, close the socket, and free the lazily-loaded in-page fallback
     * WASM module. Idempotent. Call this when the app replaces ctx.bridge (backend
     * switch / re-init) so the old bridge does not keep a reconnect loop + a loaded
     * WASM heap alive for the tab lifetime.
     */
    dispose() {
        if (this._disposed) return;
        this._disposed = true;
        this.ready = false;   // other teardown paths (onclose/onerror) clear this; dispose must too
        this._reconnecting = false;
        for (const t of ['_reconnectTimer', '_simulationWatchdog', '_telemetryDemandExpiryTimer',
                         '_visualDeferredRetryTimer', '_scenarioDispatchTimer', '_liveProfileDispatchTimer']) {
            if (this[t]) { clearTimeout(this[t]); this[t] = null; }
        }
        // Reject in-flight command promises so awaiters don't hang forever.
        for (const p of this._pendingQueue.splice(0)) {
            if (p.timeoutId) clearTimeout(p.timeoutId);
            try { p.reject?.(new Error('WebSocketBridge disposed')); } catch (_) { /* ignore */ }
        }
        // Settle the separate particle-frame promise slot too (see _resetVisualRequests).
        if (this._binaryResolve) {
            const resolve = this._binaryResolve;
            this._binaryResolve = null;
            try { resolve(this._particleData); } catch (_) { /* ignore */ }
        }
        const socket = this._ws;
        this._ws = null;
        this._connected = false;
        if (socket) {
            try { socket.onopen = socket.onmessage = socket.onerror = socket.onclose = null; socket.close(); }
            catch (_) { /* ignore */ }
        }
        if (this._fallback && typeof this._fallback.dispose === 'function') {
            try { this._fallback.dispose(); } catch (_) { /* ignore */ }
        }
        this._fallback = null;
    }

    // ── Command helpers ──────────────────────────────────────────────

    _sendJSON(obj, timeoutMs = DEFAULT_COMMAND_TIMEOUT_MS) {
        return new Promise((resolve, reject) => {
            if (!this._connected) { reject(new Error('Not connected')); return; }
            const socket = this._ws;
            if (!socket) { reject(new Error('No active WebSocket')); return; }
            // Guard: cap pending queue at 64 to prevent unbounded memory growth
            // if the server stops responding. Oldest entries are already timing
            // out after 5s, but a burst of rapid calls could still accumulate.
            if (this._pendingQueue.length >= 64) {
                reject(new Error('Pending queue full (64 commands in flight)'));
                return;
            }
            const requestId = this._nextRequestId++;
            if (this._nextRequestId >= Number.MAX_SAFE_INTEGER) this._nextRequestId = 1;

            // Timeout
            const timeoutId = setTimeout(() => {
                const idx = this._pendingQueue.findIndex(
                    p => p.requestId === requestId && p.socket === socket,
                );
                if (idx >= 0) {
                    this._pendingQueue.splice(idx, 1);
                    reject(new Error('Command timeout'));
                    // Force-close the WebSocket to trigger a clean disconnect, drain the queue, and reconnect.
                    if (socket) {
                        debugLog('[ws-bridge] Force-closing WebSocket due to command timeout');
                        try {
                            socket.close();
                        } catch (_e) {}
                    }
                }
            }, timeoutMs);

            // Current servers echo _requestId. FIFO fallback remains for older
            // servers and deterministic test doubles during rolling upgrades.
            this._pendingQueue.push({ resolve, reject, timeoutId, requestId, socket });
            socket.send(JSON.stringify({ ...obj, _requestId: requestId }));
        });
    }

    _sendAndForget(obj) {
        if (!this._connected || !this._ws) return false;
        try {
            this._ws.send(JSON.stringify(obj));
            return true;
        } catch (e) {
            debugLog('[ws-bridge] Command send failed:', e?.message || e);
            return false;
        }
    }

    // ── Native telemetry subscription/cache ──────────────────────────

    _telemetryIntervalMs(kind) {
        const L = this.latticeSize;
        // Compatibility pulls only. Protocol v2 publishes at the requested
        // tick cadence and browser panels normally issue no telemetry RPCs.
        const bands = L >= 113 ? {
            diagnostics: 500, audit: 1500, lagrangian: 2000, gravity: 1250,
        } : (L >= 65 ? {
            diagnostics: 200, audit: 750, lagrangian: 1250, gravity: 650,
        } : {
            diagnostics: 100, audit: 500, lagrangian: 900, gravity: 350,
        });
        return bands[kind] ?? 1000;
    }

    _cloneTelemetryDemand(demand = this._telemetryDemand) {
        return {
            diagnostics: !!demand?.diagnostics,
            audit: !!demand?.audit,
            lagrangian: !!demand?.lagrangian,
            gravity: !!demand?.gravity,
            everyTicks: {
                diagnostics: Math.max(1, Math.trunc(Number(demand?.everyTicks?.diagnostics) || 1)),
                audit: Math.max(1, Math.trunc(Number(demand?.everyTicks?.audit) || 8)),
                gravity: Math.max(1, Math.trunc(Number(demand?.everyTicks?.gravity) || 4)),
                lagrangian: Math.max(1, Math.trunc(Number(demand?.everyTicks?.lagrangian) || 12)),
            },
        };
    }

    _telemetryDemandEqual(left, right) {
        if (!left || !right) return false;
        return TELEMETRY_GROUPS.every(group => left[group] === right[group]
            && left.everyTicks?.[group] === right.everyTicks?.[group]);
    }

    _hasTelemetryDemand(demand = this._telemetryDemand) {
        return TELEMETRY_GROUPS.some(group => !!demand?.[group]);
    }

    _resetTelemetryRequests() {
        // Preserve desired demand through a source transition but retire all
        // outstanding control work. A late reply from an old socket/source
        // must never make the replacement lattice look subscribed.
        const demand = this._cloneTelemetryDemand();
        const demandLastSeenAt = this._telemetryDemandLastSeenAt || 0;
        if (this._telemetryDemandExpiryTimer) clearTimeout(this._telemetryDemandExpiryTimer);
        this._telemetryDemandExpiryTimer = null;
        this._telemetryControlGeneration = (this._telemetryControlGeneration || 0) + 1;
        this._telemetryInFlight = false;
        this._telemetryPumpScheduled = false;
        this._telemetryDemandDispatchScheduled = false;
        this._telemetryDemandInFlight = false;
        this._telemetryAwaitingTick = false;
        this._telemetryMode = 'unknown'; // scheduler | legacy-batch | legacy-scalar
        this._telemetryAppliedDemand = null;
        this._telemetryNeedHydration = this._hasTelemetryDemand(demand);
        this._telemetryPushSeen = false;
        this._telemetryLastPushAt = Number.NEGATIVE_INFINITY;
        this._telemetryLastPullAt = Number.NEGATIVE_INFINITY;
        this._telemetryPullSequence = 0;
        this._telemetrySyntheticVersion = 0;
        this._telemetryDemand = demand;
        this._telemetryDemandLastSeenAt = demandLastSeenAt;
        this._telemetrySnapshotMeta = {
            epoch: null, sourceEpoch: null, snapshotVersion: null, tick: null, stale: true,
        };
        // Set only by an authoritative `info` or profile acknowledgement.
        // A source epoch identifies the lattice/profile that owns a
        // telemetry stream; a reconnect must not accept an older publisher's
        // late delta merely because the socket itself is already open.
        this._expectedTelemetrySourceEpoch = null;
        this._telemetryEffectiveEveryTicks = null;
        this._telemetryGroupCache = Object.fromEntries(
            TELEMETRY_GROUPS.map(group => [group, { value: null, meta: null }]),
        );
        this._diagnosticsInFlight = false;
        this._energyInFlight = false;
        this._lagrangianInFlight = false;
        this._gravityMetricInFlight = false;
        this._lastDiagnosticsRequestAt = Number.NEGATIVE_INFINITY;
        this._lastEnergyRequestAt = Number.NEGATIVE_INFINITY;
        this._lastLagrangianRequestAt = Number.NEGATIVE_INFINITY;
        this._lastGravityMetricRequestAt = Number.NEGATIVE_INFINITY;
        this._armTelemetryDemandExpiry();
    }

    _armTelemetryDemandExpiry() {
        if (this._telemetryDemandExpiryTimer) clearTimeout(this._telemetryDemandExpiryTimer);
        this._telemetryDemandExpiryTimer = null;
        if (!this._hasTelemetryDemand() || !Number.isFinite(this._telemetryDemandLastSeenAt)) return;
        const seenAt = this._telemetryDemandLastSeenAt;
        const wait = Math.max(0, TELEMETRY_DEMAND_TTL_MS - (telemetryNow() - seenAt));
        this._telemetryDemandExpiryTimer = setTimeout(() => {
            if (this._telemetryDemandLastSeenAt !== seenAt) return;
            // Invisible panels release their native scheduler streams instead
            // of pinning a background CUDA reduction after a scale switch.
            this._telemetryDemand = {
                ...this._telemetryDemand,
                diagnostics: false,
                audit: false,
                lagrangian: false,
                gravity: false,
            };
            this._telemetryAppliedDemand = null;
            this._scheduleTelemetryDemandDispatch();
        }, wait + 1);
    }

    _normalizeTelemetryDemand(patch = {}) {
        const base = this._cloneTelemetryDemand();
        for (const group of TELEMETRY_GROUPS) {
            if (hasOwn(patch, group)) base[group] = !!patch[group];
            if (hasOwn(patch?.everyTicks, group)) {
                base.everyTicks[group] = Math.max(
                    1,
                    Math.min(65535, Math.trunc(Number(patch.everyTicks[group]) || 1)),
                );
            }
        }
        return base;
    }

    /**
     * Declare the complete Scale-0 subscription. Getters deliberately remain
     * cache-only: they cannot enqueue a WebSocket command or CUDA reduction.
     */
    setTelemetryDemand(patch = {}) {
        const next = this._normalizeTelemetryDemand(patch);
        const changed = !this._telemetryDemandEqual(next, this._telemetryDemand);
        this._telemetryDemand = next;
        this._telemetryDemandLastSeenAt = telemetryNow();
        this._armTelemetryDemandExpiry();
        if (changed || !this._telemetryAppliedDemand) {
            this._telemetryNeedHydration ||= this._hasTelemetryDemand(next);
            this._scheduleTelemetryDemandDispatch();
        }
        return true;
    }

    _scheduleTelemetryDemandDispatch() {
        if (!this._connected || this._hasPendingScenarioWork()
            || this._telemetryDemandDispatchScheduled || this._telemetryDemandInFlight) return;
        // Fresh bridges begin with no subscription. Do not add a control RPC
        // merely because a socket connected; we only need a disable command
        // after an already-applied non-empty demand expires.
        if (!this._hasTelemetryDemand() && !this._telemetryAppliedDemand) return;
        this._telemetryDemandDispatchScheduled = true;
        Promise.resolve().then(() => {
            this._telemetryDemandDispatchScheduled = false;
            this._dispatchTelemetryDemand();
        });
    }

    _isUnknownTelemetryCommand(response) {
        return /unknown\s+command/i.test(String(response?.error || ''));
    }

    _isOperationDeferred(response) {
        return response?.type === 'operation_deferred';
    }

    async _sendOperationWithRetry(command, timeoutMs, operation) {
        const deadline = telemetryNow() + timeoutMs;
        for (;;) {
            const remaining = Math.max(1, Math.ceil(deadline - telemetryNow()));
            const response = await this._sendJSON(command, remaining);
            if (!this._isOperationDeferred(response)) return response;
            const retryAfterMs = Math.max(1, Math.min(
                1000,
                Math.trunc(Number(response.retryAfterMs) || 16),
            ));
            if (telemetryNow() + retryAfterMs >= deadline) {
                const error = new Error(
                    `Native ${operation} remained deferred while telemetry settled.`,
                );
                error.operationDeferred = true;
                throw error;
            }
            await new Promise(resolve => setTimeout(resolve, retryAfterMs));
        }
    }

    _dispatchTelemetryDemand() {
        if (!this._connected || this._hasPendingScenarioWork()
            || this._telemetryDemandInFlight) return false;
        const desired = this._cloneTelemetryDemand();
        if (!this._hasTelemetryDemand(desired) && !this._telemetryAppliedDemand) return false;
        if (this._telemetryMode === 'legacy-batch' || this._telemetryMode === 'legacy-scalar') {
            this._telemetryAppliedDemand = desired;
            this._pumpTelemetry(true);
            return true;
        }
        if (this._telemetryDemandEqual(desired, this._telemetryAppliedDemand)) return false;

        this._telemetryDemandInFlight = true;
        const controlGeneration = this._telemetryControlGeneration;
        const scenarioGeneration = this._scenarioDataGeneration;
        const socket = this._ws;
        let accepted = false;
        this._sendJSON({ cmd: 'set_telemetry_demand', ...desired }, DIAGNOSTIC_COMMAND_TIMEOUT_MS)
            .then(response => {
                if (controlGeneration !== this._telemetryControlGeneration
                    || scenarioGeneration !== this._scenarioDataGeneration
                    || socket !== this._ws) return;
                if (this._isUnknownTelemetryCommand(response)) {
                    // v1 still has one batched endpoint. Fall back to it, not
                    // to one per-panel scalar RPC.
                    this._telemetryMode = 'legacy-batch';
                    this._telemetryAppliedDemand = desired;
                    accepted = true;
                    this._pumpTelemetry(true);
                    return;
                }
                if (response?.error) return;
                this._telemetryMode = 'scheduler';
                this._telemetryAppliedDemand = desired;
                this._observeTelemetrySourceEpoch(response?.telemetrySourceEpoch);
                accepted = true;
                this._updateTelemetrySnapshotMeta(response);
                if (response?.everyTicks && typeof response.everyTicks === 'object') {
                    // The server may clamp/rewrite requested cadence for its
                    // own QoS. Preserve the acknowledged values as metadata;
                    // UI freshness is still expressed through each group's
                    // epoch/tick/age, never assumed from this request alone.
                    this._telemetryEffectiveEveryTicks = this._cloneTelemetryDemand({
                        ...desired,
                        everyTicks: response.everyTicks,
                    }).everyTicks;
                }
                this._telemetryNeedHydration ||= this._hasTelemetryDemand(desired);
                this._pumpTelemetry(true);
            })
            .catch(() => {
                // A transport error is not an old-server capability signal.
            })
            .finally(() => {
                if (controlGeneration === this._telemetryControlGeneration
                    && socket === this._ws) {
                    this._telemetryDemandInFlight = false;
                    // A regular UI pass may have changed the demand while its
                    // preceding ACK was on the wire. Reissue the newest
                    // complete configuration once, rather than leaving the
                    // server subscribed to the obsolete panel set.
                    if (accepted && !this._telemetryDemandEqual(
                        this._telemetryDemand, this._telemetryAppliedDemand,
                    )) this._scheduleTelemetryDemandDispatch();
                }
            });
        return true;
    }

    // Compatibility hook for external integrations. It is intentionally a
    // subscription update, not a panel-triggered native readback.
    _requestTelemetry(groups = { diagnostics: true }) {
        this.setTelemetryDemand(groups);
        this._pumpTelemetry();
    }

    _telemetryDueGroups(now) {
        const groups = {};
        const requestAt = {
            diagnostics: this._lastDiagnosticsRequestAt,
            audit: this._lastEnergyRequestAt,
            lagrangian: this._lastLagrangianRequestAt,
            gravity: this._lastGravityMetricRequestAt,
        };
        for (const group of TELEMETRY_GROUPS) {
            if (!this._telemetryDemand[group]) continue;
            const cached = this._telemetryGroupCache?.[group]?.value;
            if (!cached || now - requestAt[group] >= this._telemetryIntervalMs(group)) {
                groups[group] = true;
            }
        }
        return groups;
    }

    _markTelemetryRequest(groups, now) {
        this._diagnosticsInFlight = !!groups.diagnostics;
        this._energyInFlight = !!groups.audit;
        this._lagrangianInFlight = !!groups.lagrangian;
        this._gravityMetricInFlight = !!groups.gravity;
        if (groups.diagnostics) this._lastDiagnosticsRequestAt = now;
        if (groups.audit) this._lastEnergyRequestAt = now;
        if (groups.lagrangian) this._lastLagrangianRequestAt = now;
        if (groups.gravity) this._lastGravityMetricRequestAt = now;
    }

    _clearTelemetryRequestMarks() {
        this._diagnosticsInFlight = false;
        this._energyInFlight = false;
        this._lagrangianInFlight = false;
        this._gravityMetricInFlight = false;
    }

    _acceptEnergyAudit(audit) {
        this._lastAudit = audit;
        if (this._lastDiag && Number.isFinite(audit?.dynamicEnergy)) {
            this._lastDiag.vacuumBaselineEnergy = this._lastDiag.totalEnergy;
            this._lastDiag.dynamicEnergy = audit.dynamicEnergy;
            this._lastDiag.accountedEnergy = audit.totalEnergy;
            this._lastDiag.restEnergy = audit.particleRestEnergy;
            this._lastDiag.totalEnergy = audit.dynamicEnergy;
        }
    }

    _normalizeTelemetryGroupMeta(snapshot, group, value, fallbackVersion) {
        const meta = snapshot?.groupMeta?.[group] || {};
        const numberOrNull = candidate => Number.isFinite(Number(candidate))
            ? Number(candidate) : null;
        const suppliedSnapshotVersion = numberOrNull(
            meta.snapshotVersion ?? snapshot?.snapshotVersion,
        );
        return {
            epoch: numberOrNull(meta.epoch ?? snapshot?.epoch),
            sourceEpoch: numberOrNull(meta.sourceEpoch ?? snapshot?.sourceEpoch),
            stateVersion: numberOrNull(meta.stateVersion ?? meta.state_version),
            tick: numberOrNull(meta.tick ?? value?.tick ?? snapshot?.tick),
            snapshotVersion: suppliedSnapshotVersion
                ?? numberOrNull(fallbackVersion)
                ?? ++this._telemetrySyntheticVersion,
            stale: !!(meta.stale ?? snapshot?.stale),
            receivedAt: telemetryNow(),
        };
    }

    _compareTelemetryGroupMeta(incoming, current) {
        if (!current) return 1;
        const compareNumber = (left, right) => {
            if (left === null || right === null || left === right) return 0;
            return Math.sign(left - right);
        };
        let order = compareNumber(incoming.sourceEpoch, current.sourceEpoch);
        if (order) return order;
        order = compareNumber(incoming.epoch, current.epoch);
        if (order) return order;
        // Per-group versions precede an aggregate publication number. Equal
        // source versions are duplicates even if another group caused a later
        // aggregate publication, so do not let that aggregate repaint this
        // group's older value.
        if (incoming.stateVersion !== null && current.stateVersion !== null) {
            return Math.sign(incoming.stateVersion - current.stateVersion);
        }
        order = compareNumber(incoming.tick, current.tick);
        if (order) return order;
        return compareNumber(incoming.snapshotVersion, current.snapshotVersion);
    }

    _shouldAcceptTelemetryGroup(group, meta) {
        const current = this._telemetryGroupCache?.[group]?.meta;
        if (meta.stale && current && !current.stale && meta.epoch === current.epoch) return false;
        return this._compareTelemetryGroupMeta(meta, current) > 0;
    }

    _observeTelemetrySourceEpoch(value) {
        const epoch = Number.isFinite(Number(value)) ? Number(value) : null;
        if (epoch === null) return false;
        const previous = this._expectedTelemetrySourceEpoch;
        if (previous !== null && epoch < previous) return false;
        this._expectedTelemetrySourceEpoch = epoch;
        this._telemetrySnapshotMeta.sourceEpoch = epoch;
        if (previous === null || epoch > previous) {
            // Preserve each cached group's own sample epoch, but make the
            // aggregate source boundary explicit immediately. Consumers can
            // retain old numbers as visibly stale while waiting for the first
            // post-mutation reduction; none may treat them as current.
            this._telemetrySnapshotMeta.stale = true;
        }
        return true;
    }

    _matchesExpectedTelemetrySourceEpoch(value) {
        // Older servers do not expose a source epoch. Once an authoritative
        // native-v2 acknowledgement has supplied one, however, absence is not
        // an acceptable substitute: it could be a delayed frame from a prior
        // source. This turns source replacement into a hard cache boundary.
        if (this._expectedTelemetrySourceEpoch === null) return true;
        const epoch = Number.isFinite(Number(value)) ? Number(value) : null;
        return epoch !== null && epoch >= this._expectedTelemetrySourceEpoch;
    }

    _isTelemetryGroupCurrent(group) {
        const meta = this._telemetryGroupCache?.[group]?.meta;
        // Direct/legacy values predate scheduler metadata. Preserve those
        // compatibility paths, but a native-v2 group with provenance is live
        // only when its sample source still matches the active source epoch.
        return !meta || !meta.stale;
    }

    _handleTelemetryInvalidated(data = {}) {
        if (!this._observeTelemetrySourceEpoch(data?.sourceEpoch)) return false;
        for (const group of TELEMETRY_GROUPS) {
            const cached = this._telemetryGroupCache?.[group];
            if (cached?.meta) cached.meta = { ...cached.meta, stale: true };
        }
        this._updateTelemetrySnapshotMeta(data);
        this._telemetrySnapshotMeta.stale = true;
        this._telemetryNeedHydration ||= this._hasTelemetryDemand();
        // This is a source/provenance boundary, not a visual frame request.
        // Wake passive consumers so they can label the retained prior values
        // stale while the publisher obtains a settled replacement snapshot.
        this._notifyVisualDataReady(false, false);
        return true;
    }

    _canAcceptTelemetryPush(snapshot = null) {
        // A server can retain/publish its previous subscription immediately
        // after reconnect. Do not flash that old source through the new empty
        // browser cache while authoritative `info` and the scenario replay are
        // still in flight. Explicit cache responses remain generation-gated
        // separately, but unsolicited publications need this handshake gate.
        return this._connected && this.ready && !this._connectionRecoveryPending
            && !this._hasPendingScenarioSetup()
            && this._matchesExpectedTelemetrySourceEpoch(snapshot?.sourceEpoch);
    }

    _updateTelemetrySnapshotMeta(snapshot) {
        const incomingEpoch = Number.isFinite(snapshot?.epoch) ? snapshot.epoch : null;
        const incomingVersion = Number.isFinite(snapshot?.snapshotVersion)
            ? snapshot.snapshotVersion : null;
        const currentEpoch = this._telemetrySnapshotMeta.epoch;
        const currentVersion = this._telemetrySnapshotMeta.snapshotVersion;
        const newerEpoch = incomingEpoch !== null && (currentEpoch === null || incomingEpoch > currentEpoch);
        const sameEpochNewerVersion = incomingEpoch !== null && incomingEpoch === currentEpoch
            && incomingVersion !== null && (currentVersion === null || incomingVersion >= currentVersion);
        const unversionedNewer = incomingEpoch === null && incomingVersion !== null
            && (currentVersion === null || incomingVersion >= currentVersion);
        if (!newerEpoch && !sameEpochNewerVersion && !unversionedNewer) return;
        if (incomingEpoch !== null) this._telemetrySnapshotMeta.epoch = incomingEpoch;
        if (Number.isFinite(snapshot?.sourceEpoch)) {
            this._telemetrySnapshotMeta.sourceEpoch = snapshot.sourceEpoch;
        }
        if (incomingVersion !== null) this._telemetrySnapshotMeta.snapshotVersion = incomingVersion;
        if (Number.isFinite(snapshot?.tick)) this._telemetrySnapshotMeta.tick = snapshot.tick;
    }

    /** Merge a scheduler delta without overwriting absent or stale groups. */
    _acceptTelemetrySnapshot(snapshot, generation = this._scenarioDataGeneration,
        fromPush = false, fallbackVersion = null) {
        if (generation !== this._scenarioDataGeneration || snapshot?.error) return false;
        if (fromPush && !this._canAcceptTelemetryPush(snapshot)) return false;
        if (!fromPush && !this._matchesExpectedTelemetrySourceEpoch(snapshot?.sourceEpoch)) return false;
        const groups = snapshot?.groups && typeof snapshot.groups === 'object'
            ? snapshot.groups : snapshot;
        if (!groups || typeof groups !== 'object') return false;
        let accepted = false;
        for (const group of TELEMETRY_GROUPS) {
            if (!hasOwn(groups, group) || !groups[group] || typeof groups[group] !== 'object') continue;
            const meta = this._normalizeTelemetryGroupMeta(
                snapshot, group, groups[group], fallbackVersion,
            );
            if (!this._shouldAcceptTelemetryGroup(group, meta)) continue;
            const value = { ...groups[group], sampledAt: meta.receivedAt };
            this._telemetryGroupCache[group] = { value, meta };
            switch (group) {
            case 'diagnostics':
                this._lastDiag = value;
                if (Number.isFinite(value.tick)) this._lastTick = value.tick;
                break;
            case 'audit': this._acceptEnergyAudit(value); break;
            case 'lagrangian': this._lastLagrangian = value; break;
            case 'gravity': this._lastGravityMetric = value; break;
            default: break;
            }
            accepted = true;
        }
        this._updateTelemetrySnapshotMeta(snapshot);
        this._telemetrySnapshotMeta.stale = TELEMETRY_GROUPS.every(
            group => this._telemetryGroupCache[group]?.meta?.stale !== false,
        );
        if (fromPush) {
            this._telemetryPushSeen = true;
            this._telemetryLastPushAt = telemetryNow();
            this._telemetryNeedHydration = false;
            // A scheduler publication is a safe boundary for cached visual
            // presentation. If bulk work was deferred for telemetry priority,
            // release exactly one retry now; otherwise wake a paused renderer
            // without marking any visual cache stale or issuing a new read.
            if (!this._releaseDeferredVisualWork()) {
                this._notifyVisualDataReady(false, false);
            }
        }
        return accepted;
    }

    /** Pure cache accessor; safe from every panel rAF loop. */
    getTelemetrySnapshot() {
        const groups = {};
        const groupMeta = {};
        for (const group of TELEMETRY_GROUPS) {
            const cached = this._telemetryGroupCache?.[group];
            if (!cached?.value) continue;
            groups[group] = cached.value;
            groupMeta[group] = {
                ...cached.meta,
                ageMs: Math.max(0, telemetryNow() - (cached.meta?.receivedAt || telemetryNow())),
            };
        }
        const snapshot = {
            type: 'telemetry',
            source: 'native',
            epoch: this._telemetrySnapshotMeta?.epoch ?? null,
            sourceEpoch: this._telemetrySnapshotMeta?.sourceEpoch
                ?? this._expectedTelemetrySourceEpoch ?? null,
            snapshotVersion: this._telemetrySnapshotMeta?.snapshotVersion ?? null,
            tick: this._telemetrySnapshotMeta?.tick ?? null,
            stale: this._telemetrySnapshotMeta?.stale ?? true,
            everyTicks: this._telemetryEffectiveEveryTicks
                ? { ...this._telemetryEffectiveEveryTicks } : undefined,
            groups,
            groupMeta,
        };
        for (const group of TELEMETRY_GROUPS) {
            if (hasOwn(groups, group)) snapshot[group] = groups[group];
        }
        return snapshot;
    }

    _pullTelemetry(groups, atTickBoundary = false) {
        if (this._telemetryInFlight || !Object.keys(groups).length) return false;
        if (this._simulationInFlight && !atTickBoundary) {
            this._telemetryAwaitingTick = true;
            return false;
        }
        const now = telemetryNow();
        const generation = this._scenarioDataGeneration;
        const socket = this._ws;
        const pullSequence = ++this._telemetryPullSequence;
        this._telemetryInFlight = true;
        this._telemetryLastPullAt = now;
        this._markTelemetryRequest(groups, now);
        this._sendJSON({ cmd: 'get_telemetry', ...groups }, DIAGNOSTIC_COMMAND_TIMEOUT_MS)
            .then(snapshot => {
                if (generation !== this._scenarioDataGeneration || socket !== this._ws) return;
                if (this._isUnknownTelemetryCommand(snapshot)) {
                    this._telemetryMode = 'legacy-scalar';
                    return;
                }
                this._acceptTelemetrySnapshot(snapshot, generation, false, pullSequence);
            })
            .catch(() => {})
            .finally(() => {
                if (generation === this._scenarioDataGeneration && socket === this._ws) {
                    this._telemetryInFlight = false;
                    this._clearTelemetryRequestMarks();
                }
            });
        return true;
    }

    _pullLegacyScalar(groups, atTickBoundary = false) {
        if (this._telemetryInFlight || !Object.keys(groups).length) return false;
        if (this._simulationInFlight && !atTickBoundary) {
            this._telemetryAwaitingTick = true;
            return false;
        }
        // Oldest servers get one scalar compatibility call per pass, never a
        // burst of independent side-panel requests.
        const group = TELEMETRY_GROUPS.find(key => groups[key]);
        if (!group) return false;
        const command = {
            diagnostics: 'get_diagnostics',
            audit: 'get_energy_audit',
            lagrangian: 'get_lagrangian',
            gravity: 'get_gravity_metric',
        }[group];
        const now = telemetryNow();
        const generation = this._scenarioDataGeneration;
        const socket = this._ws;
        const pullSequence = ++this._telemetryPullSequence;
        this._telemetryInFlight = true;
        this._telemetryLastPullAt = now;
        this._markTelemetryRequest({ [group]: true }, now);
        this._sendJSON({ cmd: command }, DIAGNOSTIC_COMMAND_TIMEOUT_MS)
            .then(value => {
                if (generation !== this._scenarioDataGeneration || socket !== this._ws || value?.error) return;
                this._acceptTelemetrySnapshot({ [group]: value }, generation, false, pullSequence);
            })
            .catch(() => {})
            .finally(() => {
                if (generation === this._scenarioDataGeneration && socket === this._ws) {
                    this._telemetryInFlight = false;
                    this._clearTelemetryRequestMarks();
                }
            });
        return true;
    }

    _pumpTelemetry(atTickBoundary = false) {
        if (!this._connected || this._hasPendingScenarioWork()) return false;
        if (!this._telemetryAppliedDemand && !this._telemetryDemandInFlight) {
            this._scheduleTelemetryDemandDispatch();
            return false;
        }
        if (!this._hasTelemetryDemand()) return false;
        const now = telemetryNow();
        if (this._telemetryMode === 'scheduler') {
            // V2 publishes autonomous deltas. Cache reads are only initial
            // hydration/stall probes and never follow panel paint cadence.
            const pushStalled = !this._telemetryPushSeen
                || now - this._telemetryLastPushAt >= TELEMETRY_PUSH_STALL_POLL_MS;
            const pullDue = this._telemetryNeedHydration
                || (pushStalled && now - this._telemetryLastPullAt >= TELEMETRY_PUSH_STALL_POLL_MS);
            if (!pullDue) return false;
            this._telemetryNeedHydration = false;
            const groups = Object.fromEntries(
                TELEMETRY_GROUPS.filter(group => this._telemetryDemand[group]).map(group => [group, true]),
            );
            return this._pullTelemetry(groups, atTickBoundary);
        }
        const groups = this._telemetryDueGroups(now);
        if (!Object.keys(groups).length) return false;
        this._telemetryAwaitingTick = false;
        return this._telemetryMode === 'legacy-scalar'
            ? this._pullLegacyScalar(groups, atTickBoundary)
            : this._pullTelemetry(groups, atTickBoundary);
    }

    _resetSimulationRequests() {
        if (this._simulationWatchdog) clearTimeout(this._simulationWatchdog);
        this._simulationWatchdog = null;
        this._simulationInFlight = false;
        this._simulationTicksInFlight = 0;
        this._queuedSimulationTicks = 0;
    }

    _simulationChunkLimit() {
        if (this.latticeSize >= 113) return 1;
        if (this.latticeSize >= 65) return 2;
        if (this.latticeSize >= 49) return 4;
        return 8;
    }

    _dispatchSimulationTicks(n) {
        const ticks = Math.max(1, Math.min(100000, Math.trunc(Number(n) || 1)));
        const chunk = Math.min(ticks, this._simulationChunkLimit());
        const command = chunk === 1 ? { cmd: 'tick' } : { cmd: 'run', n: chunk };
        if (!this._sendAndForget(command)) return false;
        this._simulationInFlight = true;
        this._simulationTicksInFlight = chunk;
        const remainder = ticks - chunk;
        if (remainder > 0) {
            this._queuedSimulationTicks = Math.min(
                100000,
                this._queuedSimulationTicks + remainder,
            );
        }
        if (this._simulationWatchdog) clearTimeout(this._simulationWatchdog);
        const socket = this._ws;
        this._simulationWatchdog = setTimeout(() => {
            if (this._ws === socket) this._handleSimulationTimeout(command, socket);
        }, LONG_OPERATION_TIMEOUT_MS);
        return true;
    }

    _handleSimulationComplete(data) {
        const completedTicks = Math.max(1, this._simulationTicksInFlight || 1);
        if (this._simulationWatchdog) clearTimeout(this._simulationWatchdog);
        this._simulationWatchdog = null;
        this._simulationInFlight = false;
        this._simulationTicksInFlight = 0;
        if (Number.isFinite(data?.tick)) this._lastTick = data.tick;

        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        if (ctx?.bridge === this && typeof ctx.onBridgeSimulationComplete === 'function') {
            try {
                ctx.onBridgeSimulationComplete({
                    ticks: completedTicks,
                    tick: data?.tick,
                    type: data?.type,
                });
            } catch (e) {
                debugLog('[ws-bridge] Simulation completion callback failed:', e?.message || e);
            }
        }

        // Only completed physics makes the visual caches stale.  Marking them
        // at command submission let the renderer request pre-tick data while
        // the CUDA work was still queued.
        this._markVisualDataDirty();

        // Protocol v2 starts its compact snapshot producer on the server's
        // settled tick boundary. This call only performs a cache hydration or
        // push-stall probe (and the bounded v1 compatibility path); it never
        // asks v2 to reduce the lattice from a panel callback.
        this._pumpTelemetry(true);

        // Keep continuous physics ahead of bulk visual refreshes. The next
        // bounded tick is already on the native command stream before the UI
        // is invited to request a particle/volume/sampler cache update, so a
        // large visual transfer cannot head-of-line block playback.
        if (this._queuedSimulationTicks > 0) {
            const queued = this._queuedSimulationTicks;
            this._queuedSimulationTicks = 0;
            this._dispatchSimulationTicks(queued);
        }

        // The viewport's field/particle refreshes are bulk requests. Notify it
        // after the scheduler/cache handoff so visual work cannot cause a UI
        // loop to compete with the server-owned telemetry publication.
        this._notifyVisualDataReady();
    }

    _handleSimulationFailure(data = {}) {
        if (this._simulationWatchdog) clearTimeout(this._simulationWatchdog);
        this._simulationWatchdog = null;
        this._simulationInFlight = false;
        this._simulationTicksInFlight = 0;
        // Do not replay a possibly-invalid explicit run after an engine/CUDA
        // failure. Real-time playback may submit a fresh bounded tick on the
        // next animation frame; Pause/Resize is immediately unblocked.
        this._queuedSimulationTicks = 0;
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        if (ctx?.bridge === this && typeof ctx.onBridgeSimulationError === 'function') {
            try { ctx.onBridgeSimulationError(data); } catch (_e) {}
        }
    }

    _retireSimulationTransport(data, socket = this._ws, dispatchError = false) {
        if (this._ws !== socket) return;
        this._handleSimulationFailure(data);

        // A CUDA failure may leave the RenderBridge partially advanced. Reset
        // all client-side categories and replace the serialized connection;
        // the next connection-ready callback atomically reapplies the selected
        // scenario rather than continuing against possibly poisoned state.
        this._resetTelemetryRequests();
        this._resetVisualRequests();
        this._markVisualDataDirty(true);
        this._connected = false;
        this.ready = false;
        this._connectionRecoveryPending = false;

        if (dispatchError && typeof window !== 'undefined') {
            window.dispatchEvent(new CustomEvent('ftd:engine-error', { detail: data }));
        }
        try {
            socket?.close?.();
        } catch (_e) {}
        this._scheduleReconnect();
    }

    _handleSimulationTimeout(command = { cmd: 'simulation' }, socket = this._ws) {
        if (this._ws !== socket) return;
        const operation = String(command.cmd || 'simulation');
        const data = {
            operation,
            error: `Native ${operation} timed out`,
        };
        this._retireSimulationTransport(data, socket, true);
    }

    _recoverOperationError(data = {}) {
        const operation = String(
            data.operation || data.command || data.cmd || data.category || '',
        ).toLowerCase();
        if (operation === 'tick' || operation === 'run' || operation === 'simulation') {
            // ws_server disconnects after a simulation exception because CUDA
            // may be partially advanced. Retire immediately as well so no rAF
            // or manual command can race the server's close notification.
            this._retireSimulationTransport(data, this._ws, false);
        } else if (operation === 'get_particles' || operation === 'particle') {
            this._particleRequestInFlight = false;
            this._particleRequestEpoch = 0;
        } else if (operation === 'get_flux_volume' || operation === 'volume') {
            this._volumeRequestInFlight = false;
            this._volumeRequestEpoch = 0;
        } else if (operation === 'get_flux_slice' || operation === 'slice') {
            this._sliceRequestsInFlight.clear();
            this._sliceRequestEpoch.clear();
        } else if (operation === 'get_field_sample' || operation === 'field_sample'
            || operation === 'get_field_slices' || operation === 'field_slices') {
            this._fieldSampleRequestTokenByKey.clear();
            this._fieldSampleRequestsByToken.clear();
            this._fieldSampleDemandByKey.clear();
            this._fieldSampleRequestEpoch.clear();
        }
    }

    /**
     * A timed-out CUDA event cannot be reset by freeing the still-live device
     * buffers. The native server quarantines that process, so retire this
     * socket and hand the desktop host an explicit restart-required signal
     * instead of retrying the same poisoned source indefinitely.
     */
    _handleRestartRequired(data = {}, socket = this._ws) {
        if (this._ws !== socket || this._restartRequired) return;
        const detail = {
            ...data,
            operation: data.operation || data.command || 'telemetry',
            error: data.error || 'Native CUDA engine restart is required.',
            restartRequired: true,
        };
        this._restartRequired = true;
        this._handleSimulationFailure(detail);
        this._resetTelemetryRequests();
        this._resetVisualRequests();
        this._markVisualDataDirty(true);
        this._connected = false;
        this.ready = false;
        this._connectionRecoveryPending = false;

        for (let i = this._pendingQueue.length - 1; i >= 0; i--) {
            if (this._pendingQueue[i].socket !== socket) continue;
            const [pending] = this._pendingQueue.splice(i, 1);
            if (pending.timeoutId) clearTimeout(pending.timeoutId);
            const error = new Error(detail.error);
            error.restartRequired = true;
            pending.reject(error);
        }
        if (typeof window !== 'undefined') {
            window.dispatchEvent(new CustomEvent('ftd:engine-error', { detail }));
        }
        try { socket?.close?.(); } catch (_e) {}
    }

    _scheduleDeferredVisualRetry(retryAfterMs = 16) {
        if (!this._connected || this._hasPendingScenarioWork()) return;
        const delay = Math.max(4, Math.min(1000, Math.trunc(Number(retryAfterMs) || 16)));
        const now = telemetryNow();
        const dueAt = now + delay;
        if (this._visualDeferredRetryTimer && this._visualDeferredRetryAt <= dueAt) return;
        if (this._visualDeferredRetryTimer) clearTimeout(this._visualDeferredRetryTimer);
        const socket = this._ws;
        this._visualDeferredRetryAt = dueAt;
        this._visualDeferredRetryTimer = setTimeout(() => {
            this._visualDeferredRetryTimer = null;
            this._visualDeferredRetryAt = 0;
            if (socket !== this._ws || !this._connected || this._hasPendingScenarioWork()) return;
            this._releaseDeferredVisualWork();
        }, delay);
    }

    _releaseDeferredVisualWork() {
        if (!this._visualDeferredPending) return false;
        this._visualDeferredPending = false;
        if (this._visualDeferredRetryTimer) clearTimeout(this._visualDeferredRetryTimer);
        this._visualDeferredRetryTimer = null;
        this._visualDeferredRetryAt = 0;
        // Field samples retain their explicit demand map, so they can resume
        // without reconstructing an overlay request. Particle/volume/slice
        // caches are retried through one forced cached-render pass below.
        this._drainFieldSampleRequests();
        this._notifyVisualDataReady(false, true);
        return true;
    }

    _handleVisualDeferred(data = {}) {
        const operation = String(data.operation || '').toLowerCase();
        if (operation === 'get_particles' || operation === 'particle') {
            this._particleRequestInFlight = false;
            this._particleRequestEpoch = 0;
            // Avoid stranding callers of getParticleDataAsync(): deferral is
            // a scheduling outcome, not a failed physics frame.
            if (this._binaryResolve) {
                this._binaryResolve(this._particleData);
                this._binaryResolve = null;
            }
        } else if (operation === 'get_flux_volume' || operation === 'volume') {
            this._volumeRequestInFlight = false;
            this._volumeRequestEpoch = 0;
        } else if (operation === 'get_flux_slice' || operation === 'slice') {
            this._sliceRequestsInFlight.clear();
            this._sliceRequestEpoch.clear();
        } else if (operation === 'get_field_sample' || operation === 'field_sample'
            || operation === 'get_field_slices' || operation === 'field_slices') {
            // The server emits no binary frame for a deferred sampler. Return
            // all bounded in-flight work to its fair demand queue and retry
            // only after telemetry has published or the tiny bounded delay.
            for (const [token, pending] of this._fieldSampleRequestsByToken) {
                this._fieldSampleRequestsByToken.delete(token);
                this._fieldSampleRequestTokenByKey.delete(pending.key);
                this._fieldSampleRequestEpoch.set(pending.key, 0);
                this._fieldSampleDemandByKey.set(pending.key, {
                    key: pending.key,
                    kind: pending.kind,
                    stride: pending.stride,
                    epoch: this._visualEpoch,
                    planesMid: pending.planesMid,
                });
            }
        } else {
            return false;
        }
        this._visualDeferredPending = true;
        this._scheduleDeferredVisualRetry(data.retryAfterMs);
        return true;
    }

    _resetVisualRequests() {
        if (this._visualDeferredRetryTimer) clearTimeout(this._visualDeferredRetryTimer);
        this._visualDeferredRetryTimer = null;
        this._visualDeferredRetryAt = 0;
        this._visualDeferredPending = false;
        this._particleRequestInFlight = false;
        // Settle any awaiting getParticleDataAsync() — its resolver lives in the
        // separate _binaryResolve slot (NOT _pendingQueue), so without this it
        // hangs forever when the socket closes/errors before the frame arrives.
        if (this._binaryResolve) {
            const resolve = this._binaryResolve;
            this._binaryResolve = null;
            try { resolve(this._particleData); } catch (_) { /* ignore */ }
        }
        this._volumeRequestInFlight = false;
        this._sliceRequestsInFlight.clear();
        this._fieldSampleRequestTokenByKey.clear();
        this._fieldSampleRequestsByToken.clear();
        this._fieldSampleDemandByKey.clear();
        this._voxelRequestsInFlight.clear();
        this._forceAtRequestsInFlight.clear();
        this._pointQueryRequestsInFlight = 0;
        this._particleRequestEpoch = 0;
        this._volumeRequestEpoch = 0;
        this._sliceRequestEpoch.clear();
        this._fieldSampleRequestEpoch.clear();
        this._voxelRequestEpoch.clear();
        this._forceAtRequestEpoch.clear();
    }

    _markVisualDataDirty(clearCaches = false) {
        this._visualEpoch++;
        if (this._visualEpoch >= Number.MAX_SAFE_INTEGER) {
            this._visualEpoch = 1;
            this._particleRequestEpoch = 0;
            this._volumeRequestEpoch = 0;
            this._sliceRequestEpoch.clear();
        }
        if (clearCaches) {
            this._scenarioDataGeneration++;
            if (this._scenarioDataGeneration >= Number.MAX_SAFE_INTEGER) {
                this._scenarioDataGeneration = 1;
            }
            this._particleData = {
                positions: new Float32Array(0),
                colors: new Float32Array(0),
                sizes: new Float32Array(0),
                spin: new Float32Array(0),
                colorCharge: new Float32Array(0),
                count: 0,
            };
            this._volumeCache = null;
            this._sliceCache = new Map();
            this._fieldSampleCache.clear();
            this._voxelCache.clear();
            this._forceAtCache.clear();
            this._lastDiag = null;
            this._lastAudit = null;
            this._lastLagrangian = null;
            this._lastGravityMetric = null;
            this._lastDiagnosticsRequestAt = Number.NEGATIVE_INFINITY;
            this._lastEnergyRequestAt = Number.NEGATIVE_INFINITY;
            this._lastLagrangianRequestAt = Number.NEGATIVE_INFINITY;
            this._lastGravityMetricRequestAt = Number.NEGATIVE_INFINITY;
            this._resetTelemetryRequests();
        }
    }

    _notifyVisualDataReady(hadNewSamplers = false, forceUpload = true) {
        // Scale 0 already exposes this callback for worker-delivered frames.
        // The second argument identifies a native-socket delivery, which must
        // schedule an upload even while running: the request itself returned
        // the previous cache synchronously, then the new payload arrived later.
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        if (ctx?.bridge !== this || typeof ctx.onBridgePostFrame !== 'function') return;
        try {
            ctx.onBridgePostFrame(hadNewSamplers, forceUpload);
        } catch (e) {
            debugLog('[ws-bridge] Visual refresh callback failed:', e?.message || e);
        }
    }

    _hasPendingScenarioWork() {
        return !!(this._connectionRecoveryPending
            || this._scenarioDraft || this._queuedScenarioProfile
            || this._scenarioRequestInFlight || this._scenarioDispatchTimer
            || this._liveProfileQueued || this._liveProfileInFlight
            || this._liveProfileDispatchTimer);
    }

    _hasPendingScenarioSetup() {
        return !!(this._scenarioDraft || this._queuedScenarioProfile
            || this._scenarioRequestInFlight || this._scenarioDispatchTimer);
    }

    _notifyProfileState(error = null) {
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        if (ctx?.bridge !== this || typeof ctx.onBridgeProfileUpdate !== 'function') return;
        try {
            ctx.onBridgeProfileUpdate({
                toggles: { ...this._toggles },
                fluxBoundaryMode: this._fluxBoundaryMode,
                error,
            });
        } catch (e) {
            debugLog('[ws-bridge] Profile refresh callback failed:', e?.message || e);
        }
    }

    _queueLiveProfileMutation({ toggleName, value, fluxBoundaryMode } = {}) {
        if (!this._liveProfileQueued) {
            this._liveProfileQueued = { toggles: {}, hasBoundary: false, fluxBoundaryMode: 0 };
        }
        if (typeof toggleName === 'string') {
            this._liveProfileQueued.toggles[toggleName] = !!value;
        }
        if (fluxBoundaryMode !== undefined) {
            this._liveProfileQueued.hasBoundary = true;
            this._liveProfileQueued.fluxBoundaryMode = fluxBoundaryMode;
        }
        this._scheduleLiveProfileDispatch();
    }

    _scheduleLiveProfileDispatch(delay = LIVE_PROFILE_COALESCE_MS) {
        if (this._liveProfileInFlight || this._liveProfileDispatchTimer
            || !this._liveProfileQueued || !this._connected
            || this._hasPendingScenarioSetup()) return;
        this._liveProfileDispatchTimer = setTimeout(() => {
            this._liveProfileDispatchTimer = null;
            this._dispatchLiveProfile();
        }, Math.max(0, delay));
    }

    _dispatchLiveProfile() {
        if (this._liveProfileInFlight || !this._liveProfileQueued
            || !this._connected || this._hasPendingScenarioSetup()) return;
        const patch = this._liveProfileQueued;
        this._liveProfileQueued = null;
        this._liveProfileInFlight = true;
        const generation = this._profileGeneration;
        const command = {
            cmd: 'apply_profile',
            name: this._activeScenario || '',
            applyProfile: true,
        };
        if (patch.hasBoundary) command.fluxBoundaryMode = patch.fluxBoundaryMode;
        for (const [name, enabled] of Object.entries(patch.toggles)) {
            if (/^[a-z0-9_]+$/.test(name)) command[`toggle_${name}`] = !!enabled;
        }

        this._sendJSON(command, DIAGNOSTIC_COMMAND_TIMEOUT_MS)
            .then(response => {
                if (response?.error || response?.ok === false) {
                    const error = new Error(response?.error || 'Native profile update rejected');
                    error.liveProfileRejected = true;
                    throw error;
                }
                if (generation !== this._profileGeneration) return;
                if (response?.toggles && typeof response.toggles === 'object') {
                    this._confirmedToggles = Object.fromEntries(
                        Object.entries(response.toggles).map(([name, enabled]) => [name, !!enabled]),
                    );
                    this._toggles = { ...this._confirmedToggles };
                }
                if (Number.isInteger(response?.fluxBoundaryMode)) {
                    this._confirmedFluxBoundaryMode = response.fluxBoundaryMode;
                    this._fluxBoundaryMode = response.fluxBoundaryMode;
                }
                this._observeTelemetrySourceEpoch(response?.telemetrySourceEpoch);
                // A newer local edit may have arrived while this ack was in
                // flight. Preserve its optimistic value until its own ack.
                if (this._liveProfileQueued) {
                    Object.assign(this._toggles, this._liveProfileQueued.toggles);
                    if (this._liveProfileQueued.hasBoundary) {
                        this._fluxBoundaryMode = this._liveProfileQueued.fluxBoundaryMode;
                    }
                }
                this._notifyProfileState();
            })
            .catch(error => {
                if (generation !== this._profileGeneration) return;
                if (!error?.liveProfileRejected) {
                    // The server may have committed before transport loss;
                    // replaying the idempotent patch after reconnect recovers
                    // an authoritative echo. Newer intent wins per field.
                    const newer = this._liveProfileQueued;
                    this._liveProfileQueued = {
                        toggles: { ...patch.toggles, ...(newer?.toggles || {}) },
                        hasBoundary: newer?.hasBoundary || patch.hasBoundary,
                        fluxBoundaryMode: newer?.hasBoundary
                            ? newer.fluxBoundaryMode
                            : patch.fluxBoundaryMode,
                    };
                } else {
                    // Logical rejection means the engine kept its old profile.
                    // Roll back only fields not superseded by a newer local edit.
                    for (const name of Object.keys(patch.toggles)) {
                        if (!(name in (this._liveProfileQueued?.toggles || {}))) {
                            this._toggles[name] = this._confirmedToggles[name] ?? false;
                        }
                    }
                    if (patch.hasBoundary && !this._liveProfileQueued?.hasBoundary) {
                        this._fluxBoundaryMode = this._confirmedFluxBoundaryMode;
                    }
                    this._notifyProfileState(error.message);
                    if (typeof window !== 'undefined') {
                        window.dispatchEvent(new CustomEvent('ftd:engine-error', {
                            detail: { operation: 'apply_profile', error: error.message },
                        }));
                    }
                }
            })
            .finally(() => {
                this._liveProfileInFlight = false;
                if (this._liveProfileQueued) {
                    this._scheduleLiveProfileDispatch(0);
                } else if (!this._hasPendingScenarioSetup() && this._queuedSimulationTicks > 0) {
                    const queued = this._queuedSimulationTicks;
                    this._queuedSimulationTicks = 0;
                    this._dispatchSimulationTicks(queued);
                }
            });
    }

    _scheduleScenarioDispatch(delay = SCENARIO_COALESCE_MS) {
        if (this._scenarioRequestInFlight || this._scenarioDispatchTimer
            || !this._queuedScenarioProfile || !this._connected) return;
        this._scenarioDispatchTimer = setTimeout(() => {
            this._scenarioDispatchTimer = null;
            this._dispatchQueuedScenarioProfile();
        }, Math.max(0, delay));
    }

    _scenarioProfileCommand(profile) {
        const command = {
            cmd: profile.prepared ? 'apply_profile' : 'setup_scenario',
            name: profile.name,
            applyProfile: true,
            fluxBoundaryMode: profile.fluxBoundaryMode,
        };
        for (const [name, value] of Object.entries(profile.toggles || {})) {
            // Term names are a closed snake_case registry. Filtering here keeps
            // profile serialization inert even if a malformed caller reaches it.
            if (/^[a-z0-9_]+$/.test(name)) command[`toggle_${name}`] = !!value;
        }
        return command;
    }

    _acceptScenarioResponse(response, profile) {
        if (response?.error || response?.ok === false) {
            const error = new Error(response?.error || `Scenario setup rejected: ${profile.name}`);
            error.scenarioRejected = true;
            throw error;
        }
        if (response?.toggles && typeof response.toggles === 'object') {
            // The echoed object is the complete engine TermToggles registry.
            // Replace the mirror rather than merging it so a stale client-only
            // value cannot survive, and native-enabled terms that were absent
            // from the pre-setup cache become immediately queryable.
            this._toggles = Object.fromEntries(
                Object.entries(response.toggles).map(([name, value]) => [name, !!value]),
            );
        } else {
            // Current native profile commands apply this exact staged map. Keep
            // the mirror authoritative even during a rolling server upgrade
            // whose acknowledgement predates the echoed toggle object.
            Object.assign(this._toggles, profile.toggles);
        }
        this._confirmedToggles = { ...this._toggles };
        if (response?.params && typeof response.params === 'object') {
            for (const [name, value] of Object.entries(response.params)) {
                if (Number.isFinite(Number(value))) this._params[name] = Number(value);
            }
        }
        if (Number.isInteger(response?.latticeSize) && response.latticeSize > 0) {
            this.latticeSize = response.latticeSize;
        }
        if (Number.isInteger(response?.fluxBoundaryMode)) {
            this._fluxBoundaryMode = response.fluxBoundaryMode;
        }
        this._confirmedFluxBoundaryMode = this._fluxBoundaryMode;
        this._activeScenario = response?.scenario || profile.name;
        this._markVisualDataDirty(true);
        // `_markVisualDataDirty(true)` retires the old cache/source token.
        // Re-establish the token only after this atomic profile ACK commits
        // the replacement RenderBridge on the server.
        this._observeTelemetrySourceEpoch(response?.telemetrySourceEpoch);
        this._notifyVisualDataReady(false, true);
        this._notifyProfileState();
    }

    _dispatchQueuedScenarioProfile() {
        if (this._scenarioRequestInFlight || !this._queuedScenarioProfile
            || !this._connected) return;
        const profile = this._queuedScenarioProfile;
        this._queuedScenarioProfile = null;
        this._scenarioRequestInFlight = true;
        this._sendJSON(this._scenarioProfileCommand(profile), LONG_OPERATION_TIMEOUT_MS)
            .then(response => {
                if (this._isOperationDeferred(response)) {
                    this._queuedScenarioProfile = profile;
                    this._scenarioRetryAfterMs = Math.max(1, Math.min(
                        1000, Math.trunc(Number(response.retryAfterMs) || 16),
                    ));
                    return;
                }
                this._acceptScenarioResponse(response, profile);
            })
            .catch(err => {
                console.error('[ws-bridge] Native scenario setup failed:', err?.message || err);
                if (typeof window !== 'undefined') {
                    window.dispatchEvent(new CustomEvent('ftd:engine-error', {
                        detail: { operation: 'setup_scenario', scenario: profile.name, error: err?.message || String(err) },
                    }));
                }
                // A transport failure may mean the server committed but its ack
                // was lost. Replaying the last requested profile after reconnect
                // is deterministic and restores the UI's selected scenario.
                if (!err?.scenarioRejected && !this._queuedScenarioProfile) {
                    this._queuedScenarioProfile = profile;
                }
                if (err?.scenarioRejected) this._queuedSimulationTicks = 0;
            })
            .finally(() => {
                this._scenarioRequestInFlight = false;
                if (this._queuedScenarioProfile) {
                    const delay = this._scenarioRetryAfterMs;
                    this._scenarioRetryAfterMs = 0;
                    this._scheduleScenarioDispatch(delay);
                } else if (this._liveProfileQueued) {
                    this._scheduleLiveProfileDispatch(0);
                } else {
                    // Re-arm the native publisher after its source was
                    // replaced. The subscription itself is independent of
                    // panel paint frequency and precedes any queued prime tick.
                    this._scheduleTelemetryDemandDispatch();
                    if (this._queuedSimulationTicks > 0) {
                        const queued = this._queuedSimulationTicks;
                        this._queuedSimulationTicks = 0;
                        this._dispatchSimulationTicks(queued);
                    }
                }
            });
    }

    _completeFieldSample(
        token, kind, stride, components, positions, payload, count,
        effectiveStride = null, origin = null,
    ) {
        const pending = this._fieldSampleRequestsByToken.get(token);
        // Tokens are connection-local and identify both kind and requested
        // stride. An unknown token is a stale response from a superseded epoch
        // or connection; never let it repopulate a freshly-cleared scenario.
        if (!pending) return false;
        const resolvedKind = pending.kind || kind;
        if (!resolvedKind || (kind && kind !== resolvedKind)) {
            this._rejectFieldSample(token, `kind mismatch (${kind} != ${resolvedKind})`);
            return false;
        }
        const resolvedStride = pending?.stride || Math.max(1, Math.trunc(stride || 1));
        const key = pending?.key || `${resolvedKind}@${resolvedStride}`;
        this._fieldSampleRequestsByToken.delete(token);
        this._fieldSampleRequestTokenByKey.delete(key);
        const sample = components === 1
            ? { positions, values: payload, count }
            : { positions, vectors: payload, count };
        sample.kind = resolvedKind;
        if (Number.isInteger(effectiveStride) && effectiveStride > 0) {
            sample.effectiveStride = effectiveStride;
        }
        if (Number.isInteger(origin) && origin >= 0) sample.origin = origin;
        this._fieldSampleCache.set(key, sample);
        this._drainFieldSampleRequests();
        this._notifyVisualDataReady(true, false);
        return true;
    }

    _rejectFieldSample(token, message) {
        const pending = this._fieldSampleRequestsByToken.get(token);
        if (pending) {
            this._fieldSampleRequestsByToken.delete(token);
            this._fieldSampleRequestTokenByKey.delete(pending.key);
            this._fieldSampleRequestEpoch.set(pending.key, 0);
        }
        this._drainFieldSampleRequests();
        console.warn('[ws-bridge] Invalid field-sample frame:', message);
        this._notifyVisualDataReady(true, false);
    }

    _handleJSON(text, sourceSocket = this._ws) {
        try {
            // onmessage already guards this in production, but retain the
            // source check here as well: tests and alternate hosts can call
            // this decoder directly while an old socket is draining.
            if (sourceSocket && this._ws && sourceSocket !== this._ws) return;
            const data = JSON.parse(text);
            // Check fire-and-forget cached responses first, preventing them
            // from mistakenly resolving pending command promises in the queue!
            if (data.type === 'flux_slice') {
                const key = `${data.axis}_${data.index}`;
                this._sliceRequestsInFlight.delete(key);
                this._sliceCache.set(key, new Float64Array(data.data));
                if (this._sliceCache.size > MAX_SLICE_CACHE) {
                    this._sliceCache.delete(this._sliceCache.keys().next().value);
                }
                this._notifyVisualDataReady();
                return;
            }
            if (data.type === 'flux_volume') {
                this._volumeRequestInFlight = false;
                this._volumeCache = new Float64Array(data.data);
                this._notifyVisualDataReady();
                return;
            }
            // Compatibility/debug representation of FTS1. Production native
            // builds use the binary path below, but accepting JSON keeps the
            // bridge testable and permits mixed-version deployments.
            if (data.type === 'field_sample') {
                const token = Number(data.token) >>> 0;
                const kind = typeof data.kind === 'string' ? data.kind : null;
                const components = Number(data.components) === 1 ? 1 : 3;
                const count = Math.max(0, Math.trunc(Number(data.count) || 0));
                const positions = new Float32Array(data.positions || []);
                const payload = new Float32Array(data.data || data.values || data.vectors || []);
                if (positions.length !== count * 3 || payload.length !== count * components) {
                    this._rejectFieldSample(token, 'JSON payload length mismatch');
                } else {
                    this._completeFieldSample(
                        token, kind, data.stride, components, positions, payload, count,
                        Number.isInteger(data.effectiveStride) ? data.effectiveStride : null,
                        Number.isInteger(data.origin) ? data.origin : null,
                    );
                }
                return;
            }
            if (data.type === 'tick_complete' || data.type === 'run_complete') {
                this._handleSimulationComplete(data);
                return;
            }
            if (data.type === 'operation_progress') {
                debugLog(`[ws-bridge] ${data.operation}: ${data.phase} (L=${data.size})`);
                if (typeof window !== 'undefined') {
                    window.dispatchEvent(new CustomEvent('ftd:engine-progress', { detail: data }));
                }
                return;
            }
            if (data.type === 'visual_deferred') {
                // Not an engine failure: native intentionally yielded bulk
                // visual work to a due compact telemetry observation. Reset
                // only the affected bounded request and retry after push/TTL.
                this._handleVisualDeferred(data);
                if (Number.isFinite(data._requestId)) {
                    const idx = this._pendingQueue.findIndex(
                        pending => pending.requestId === data._requestId
                            && pending.socket === sourceSocket,
                    );
                    if (idx >= 0) {
                        const [{ resolve, timeoutId }] = this._pendingQueue.splice(idx, 1);
                        if (timeoutId) clearTimeout(timeoutId);
                        resolve(data);
                    }
                }
                return;
            }
            if (data.type === 'telemetry_invalidated') {
                this._handleTelemetryInvalidated(data);
                return;
            }
            // Protocol v2 telemetry is a server-push delta. It has no
            // request id and must be consumed before FIFO fallback handling;
            // each group is merged/version-gated by _acceptTelemetrySnapshot.
            if (data.type === 'telemetry_snapshot') {
                this._acceptTelemetrySnapshot(data, this._scenarioDataGeneration, true);
                return;
            }
            // Some rolling deployments publish a cache snapshot under the
            // older `telemetry` type. Preserve request-id responses for their
            // original _sendJSON promise, but treat truly unsolicited data as
            // the same delta stream.
            if (data.type === 'telemetry' && !Number.isFinite(data._requestId)) {
                this._acceptTelemetrySnapshot(data, this._scenarioDataGeneration, true);
                return;
            }

            // The server deliberately quarantines a poisoned CUDA source
            // instead of destroying buffers still owned by an unsignaled
            // event. Handle this before correlating a request-id response so
            // the desktop receives restartRequired even for setup/info RPCs.
            if (data.restartRequired) {
                this._handleRestartRequired(data, sourceSocket);
                return;
            }

            if (Number.isFinite(data._requestId)) {
                const idx = this._pendingQueue.findIndex(
                    pending => pending.requestId === data._requestId
                        && pending.socket === sourceSocket,
                );
                if (idx >= 0) {
                    const [{ resolve, timeoutId }] = this._pendingQueue.splice(idx, 1);
                    if (timeoutId) clearTimeout(timeoutId);
                    resolve(data);
                    return;
                }
            }
            if (data.error) {
                // An uncorrelated error belongs to a fire-and-forget mutation.
                // Never let it resolve an unrelated diagnostics/resize promise.
                this._recoverOperationError(data);
                console.error('[ws-bridge] Native command failed:', data.error);
                if (typeof window !== 'undefined') {
                    window.dispatchEvent(new CustomEvent('ftd:engine-error', { detail: data }));
                }
                return;
            }

            // Compatibility fallback for a server that predates request IDs.
            const fallbackIndex = this._pendingQueue.findIndex(
                pending => pending.socket === sourceSocket,
            );
            if (fallbackIndex >= 0) {
                const [{ resolve, timeoutId }] = this._pendingQueue.splice(fallbackIndex, 1);
                if (timeoutId) clearTimeout(timeoutId);
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
        if (buf.byteLength >= 8) {
            const header = new DataView(buf);
            if (header.getUint32(0, true) === PARTICLE_FRAME_MAGIC) {
                this._particleRequestInFlight = false;
                const count = header.getUint32(4, true);
                const posBytes = count * 3 * 4;
                const colBytes = count * 3 * 4;
                const scalarBytes = count * 4;
                const expectedBytes = 8 + posBytes + colBytes + scalarBytes * 3;
                if (buf.byteLength !== expectedBytes) {
                    console.warn(`[ws-bridge] Invalid FTP2 particle frame: got ${buf.byteLength}, expected ${expectedBytes}`);
                    this._particleRequestEpoch = 0;
                    this._notifyVisualDataReady();
                    return;
                }
                let offset = 8;
                const positions = new Float32Array(buf, offset, count * 3); offset += posBytes;
                const colors = new Float32Array(buf, offset, count * 3); offset += colBytes;
                const sizes = new Float32Array(buf, offset, count); offset += scalarBytes;
                const spin = new Float32Array(buf, offset, count); offset += scalarBytes;
                const colorCharge = new Float32Array(buf, offset, count);
                this._particleData = { positions, colors, sizes, spin, colorCharge, count };
                this._notifyVisualDataReady();
                if (this._binaryResolve) {
                    this._binaryResolve(this._particleData);
                    this._binaryResolve = null;
                }
                return;
            }
            if (header.getUint32(0, true) === FLUX_VOLUME_COMPACT_MAGIC) {
                this._volumeRequestInFlight = false;
                const parsed = parseFtv2Frame(buf);
                if (!parsed) {
                    console.warn(`[ws-bridge] Invalid FTV2 frame: ${buf.byteLength} bytes`);
                    this._volumeRequestEpoch = 0;
                    this._notifyVisualDataReady();
                    return;
                }
                this._volumeCache = {
                    data: parsed.data,
                    latticeSize: parsed.latticeSize,
                    stride: parsed.stride,
                    origin: parsed.origin,
                    axisCount: parsed.axisCount,
                };
                this._notifyVisualDataReady();
                return;
            }
            if (header.getUint32(0, true) === FLUX_VOLUME_MAGIC) {
                this._volumeRequestInFlight = false;
                const count = header.getUint32(4, true);
                const expectedBytes = 8 + count * 4;
                if (buf.byteLength !== expectedBytes) {
                    console.warn(`[ws-bridge] Invalid flux-volume frame: got ${buf.byteLength}, expected ${expectedBytes}`);
                    this._volumeRequestEpoch = 0;
                    this._notifyVisualDataReady();
                    return;
                }
                this._volumeCache = new Float32Array(buf, 8, count);
                this._notifyVisualDataReady();
                return;
            }
            const fieldMagic = header.getUint32(0, true);
            if (fieldMagic === FIELD_SAMPLE_MAGIC || fieldMagic === FIELD_SAMPLE_V2_MAGIC) {
                const isV2 = fieldMagic === FIELD_SAMPLE_V2_MAGIC;
                const headerBytes = isV2 ? 28 : 20;
                if (buf.byteLength < headerBytes) {
                    this._rejectFieldSample(0, `short FTS${isV2 ? 2 : 1} header (${buf.byteLength} bytes)`);
                    return;
                }
                const token = header.getUint32(4, true);
                const kindCode = header.getUint32(8, true);
                const components = header.getUint32(12, true);
                const count = header.getUint32(16, true);
                const effectiveStride = isV2 ? header.getUint32(20, true) : null;
                const origin = isV2 ? header.getUint32(24, true) : null;
                const kind = FIELD_SAMPLE_KINDS[kindCode];
                const expectedBytes = headerBytes + count * (3 + components) * 4;
                if (!kind || (components !== 1 && components !== 3)
                    || (isV2 && (effectiveStride < 1 || origin > this.latticeSize))
                    || buf.byteLength !== expectedBytes) {
                    this._rejectFieldSample(
                        token,
                        `got kind=${kindCode}, components=${components}, bytes=${buf.byteLength}; expected ${expectedBytes}`,
                    );
                    return;
                }
                const positions = new Float32Array(buf, headerBytes, count * 3);
                const payload = new Float32Array(
                    buf, headerBytes + count * 3 * 4, count * components,
                );
                const pending = this._fieldSampleRequestsByToken.get(token);
                this._completeFieldSample(
                    token, kind, pending?.stride || 1, components, positions, payload, count,
                    effectiveStride, origin,
                );
                return;
            }
        }

        const magic = buf.byteLength >= 4 ? new DataView(buf).getUint32(0, true) : 0;
        console.warn(`[ws-bridge] Ignoring unknown binary frame magic=0x${magic.toString(16)} bytes=${buf.byteLength}`);
        this._particleRequestInFlight = false;
        this._volumeRequestInFlight = false;
        this._notifyVisualDataReady();
    }

    // ── Public API (matches MockBridge/WasmBridge) ───────────────────

    beginScenarioConfiguration(name) {
        // A newer selection supersedes any draft that never committed. Once a
        // native allocation has begun it cannot be cancelled safely, but the
        // queued slot below remains last-write-wins.
        this._profileGeneration++;
        if (this._liveProfileDispatchTimer) {
            clearTimeout(this._liveProfileDispatchTimer);
            this._liveProfileDispatchTimer = null;
        }
        // A canonical scenario selection supersedes any live edit that had not
        // reached the engine. An already in-flight edit is generation-gated;
        // the subsequent setup command wins on the serialized socket.
        this._liveProfileQueued = null;
        this._scenarioDraft = {
            name,
            toggles: {},
            fluxBoundaryMode: this._fluxBoundaryMode,
            setupRequested: false,
            prepared: false,
        };
        // Scalar readback belongs to the old RenderBridge until the atomic
        // setup acknowledgement supplies the new scenario's full params.
        // Clearing it prevents e.g. a de-Broglie omega0 from appearing to leak
        // into the next profile while construction is in flight.
        this._params = {};
        // Discard a prime/step belonging to a scenario that the user has just
        // superseded. The final selected scenario will enqueue its own prime.
        this._queuedSimulationTicks = 0;
        return true;
    }

    commitScenarioConfiguration(name) {
        const draft = this._scenarioDraft;
        if (!draft) return false;
        this._scenarioDraft = null;
        if (name) draft.name = name;
        if (!draft.setupRequested) {
            console.warn(`[ws-bridge] Scenario profile committed without setupScenario(): ${draft.name}`);
        }
        if (this._preparedScenario === draft.name) {
            draft.prepared = true;
            this._preparedScenario = null;
        }
        this._queuedScenarioProfile = draft;
        this._markVisualDataDirty(true);
        this._scheduleScenarioDispatch();
        return true;
    }

    abortScenarioConfiguration() {
        this._scenarioDraft = null;
    }

    queueScenarioPrimeTick() {
        // A scenario prime belongs to the latest staged profile only. It is
        // intentionally coalesced, unlike explicit paused Step commands.
        this._queuedSimulationTicks = Math.max(1, this._queuedSimulationTicks);
        if (!this._hasPendingScenarioWork() && !this._simulationInFlight) {
            const queued = this._queuedSimulationTicks;
            this._queuedSimulationTicks = 0;
            this._dispatchSimulationTicks(queued);
        }
    }

    tick() {
        if (!this._connected) return;
        if (this._hasPendingScenarioWork()) {
            const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
            const isPlayback = ctx?.bridge === this ? !!ctx.running : true;
            this._queuedSimulationTicks = isPlayback
                ? Math.max(1, this._queuedSimulationTicks)
                : Math.min(100000, this._queuedSimulationTicks + 1);
            return;
        }
        if (this._simulationInFlight) {
            const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
            const isPlayback = ctx?.bridge === this ? !!ctx.running : true;
            if (isPlayback) {
                // Playback represents desired real-time progress. One queued
                // follow-up is sufficient; dropping excess demand keeps the
                // controls responsive when the lattice exceeds real time.
                this._queuedSimulationTicks = Math.max(1, this._queuedSimulationTicks);
            } else {
                // Paused Step / Step-by-N is an exact user command. Preserve
                // every requested step, then drain it as one run(n) after the
                // currently executing tick acknowledges.
                this._queuedSimulationTicks = Math.min(100000, this._queuedSimulationTicks + 1);
            }
            return;
        }
        this._dispatchSimulationTicks(1);
    }

    cancelQueuedTicks() {
        this._queuedSimulationTicks = 0;
    }

    run(n) {
        const ticks = Math.max(1, Math.min(100000, Math.trunc(Number(n) || 1)));
        if (!this._connected) return;
        if (this._hasPendingScenarioWork()) {
            this._queuedSimulationTicks = Math.min(100000, this._queuedSimulationTicks + ticks);
            return;
        }
        if (this._simulationInFlight) {
            // Explicit run(n) retains its requested work (up to the server's
            // documented cap); only high-frequency tick() calls are coalesced.
            this._queuedSimulationTicks = Math.min(100000, this._queuedSimulationTicks + ticks);
            return;
        }
        this._dispatchSimulationTicks(ticks);
    }

    getParticleData() {
        // Request at most one particle frame for the current engine epoch.
        // The async response wakes the paused Scale-0 renderer.
        if (this._connected && !this._hasPendingScenarioWork() && !this._particleRequestInFlight
            && this._particleRequestEpoch !== this._visualEpoch) {
            this._particleRequestInFlight = true;
            this._particleRequestEpoch = this._visualEpoch;
            if (!this._sendAndForget({ cmd: 'get_particles' })) {
                this._particleRequestInFlight = false;
                this._particleRequestEpoch = 0;
            }
        }
        // Return cached data (binary arrives async, will be ready next frame)
        return this._particleData;
    }

    // Scale-0 physics panels consume records rather than the packed renderer
    // frame. Keep conversion shared with WASM/proxy so FTP2 state, spin and
    // color charge have identical semantics across every backend.
    getScale0ParticleList() {
        return particleDataToList(this.getParticleData());
    }

    // Only one async particle request can be in flight at a time (single
    // _binaryResolve slot). Intended for the render loop: one request per frame.
    // If a caller does call again before the previous resolves, the previous
    // promise is settled here with the last known particle data rather than
    // orphaned into a silent forever-pending await — the new promise then takes
    // the slot and resolves on the in-flight reply.
    async getParticleDataAsync() {
        return new Promise((resolve) => {
            if (this._binaryResolve) {
                const prevResolve = this._binaryResolve;
                this._binaryResolve = null;
                prevResolve(this._particleData);
            }
            this._binaryResolve = resolve;
            if (!this._connected) {
                resolve(this._particleData);
                this._binaryResolve = null;
                return;
            }
            if (!this._particleRequestInFlight) {
                this._particleRequestInFlight = true;
                this._particleRequestEpoch = this._visualEpoch;
                if (!this._sendAndForget({ cmd: 'get_particles' })) {
                    this._particleRequestInFlight = false;
                    this._particleRequestEpoch = 0;
                    this._binaryResolve = null;
                    resolve(this._particleData);
                }
            }
        });
    }

    getDiagnostics() {
        // A missing native snapshot is unknown, never an all-zero physical
        // measurement.  In particular, source replacement/reconnect can
        // leave this cache empty until the scheduler publishes its first
        // settled observation. Consumers already accept null as "not sampled
        // yet"; fabricating zero energy or charge here made that transition
        // indistinguishable from a real empty lattice.
        return this._isTelemetryGroupCurrent('diagnostics') ? (this._lastDiag || null) : null;
    }

    getEnergyAudit() {
        return this._isTelemetryGroupCurrent('audit') ? (this._lastAudit || null) : null;
    }

    setToggle(name, value) {
        const normalized = !!value;
        if (this._scenarioDraft) {
            this._toggles[name] = normalized;
            this._scenarioDraft.toggles[name] = normalized;
            return;
        }
        if (this._toggles[name] === normalized) return;
        this._toggles[name] = normalized;
        this._queueLiveProfileMutation({ toggleName: name, value: normalized });
    }

    getToggle(name) {
        // Unknown/unreported terms are never optimistically ON. Atomic setup
        // acknowledgements repopulate the complete server truth map.
        return this._toggles[name] ?? false;
    }

    _seedPhysicsLocked() {
        return !!(this._scenarioDraft || this._preparedScenario);
    }

    setParam(name, value) {
        const n = Number(value);
        if (!Number.isFinite(n)) return;
        const prev = this._params[name];
        this._params[name] = n;
        this._sendJSON({ cmd: 'set_param', name, value: n }, DIAGNOSTIC_COMMAND_TIMEOUT_MS)
            .then((response) => {
                if (response?.error) {
                    if (prev === undefined) delete this._params[name];
                    else this._params[name] = prev;
                    console.warn('[ws-bridge] set_param rejected:', response.error);
                }
            })
            .catch((err) => {
                if (prev === undefined) delete this._params[name];
                else this._params[name] = prev;
                debugLog('[ws-bridge] set_param failed:', err?.message || err);
            });
    }

    getParam(name) { return this._params[name]; }
    getOmega0() { return this._params.omega0 ?? 1.0; }

    injectFlux(x, y, z, fx, fy, fz) {
        if (this._seedPhysicsLocked()) return;
        this._sendAndForget({ cmd: 'inject_flux', x, y, z, fx, fy, fz });
        this._markVisualDataDirty();
    }

    injectParticle(x, y, z, state, fx = 0, fy = 0, fz = 0) {
        if (this._seedPhysicsLocked()) return;
        this._sendAndForget({ cmd: 'inject_particle', x, y, z, state, fx, fy, fz });
        this._markVisualDataDirty();
    }

    injectWavepacket(x, y, z, state) {
        if (this._seedPhysicsLocked()) return;
        this._sendAndForget({ cmd: 'inject_wavepacket', x, y, z, state });
        this._markVisualDataDirty();
    }

    createEntangledPair(x, y, z, fx = K_B, fy = 0, fz = 0) {
        if (this._seedPhysicsLocked()) return;
        this._sendAndForget({ cmd: 'create_pair', x, y, z, fx, fy, fz });
        this._markVisualDataDirty();
    }

    _injectFlux(x, y, z, fx, fy, fz) {
        if (this._seedPhysicsLocked()) return;
        this._sendAndForget({ cmd: 'inject_flux_add', x, y, z, fx, fy, fz });
        this._markVisualDataDirty();
    }

    _injectWaveVel(x, y, z, wx, wy, wz) {
        if (this._seedPhysicsLocked()) return;
        this._sendAndForget({ cmd: 'inject_wave_vel_add', x, y, z, wx, wy, wz });
        this._markVisualDataDirty();
    }

    _ensureFallback() {
        if (!this._fallback) {
            this._fallback = new WasmBridge();
            // Scale 1 runs on the in-page WASM module (native ParticleEngine),
            // independent of the native-server socket. Kick off the module
            // load; until it resolves, pe* calls return contract-empty shapes
            // and peGetBackendCapabilities().backend === 'unavailable'.
            // Scale 1/2 engines are standalone.  They only need the WASM
            // module, not a duplicate full-size Scale 0 lattice alongside the
            // native CUDA owner.  Use the smallest supported control lattice.
            this._fallbackInit = this._fallback.init(9)
                .catch(err => {
                    console.warn('[WSBridge] Scale-1 WASM fallback failed to load;',
                                 'particle engine unavailable in this session:', err);
                });
        }
        return this._fallback;
    }

    _requireSuccessfulResponse(response, operation) {
        if (response?.error) throw new Error(response.error);
        if (response?.ok === false) throw new Error(`${operation} was rejected`);
        return response;
    }

    async preflightResize(size) {
        const response = await this._sendJSON(
            { cmd: 'preflight_resize', size },
            DIAGNOSTIC_COMMAND_TIMEOUT_MS,
        );
        if (!response?.accepted) {
            const gib = value => (Number(value || 0) / 1024 ** 3).toFixed(2);
            throw new Error(
                `L=${size} exceeds the native construction budget ` +
                `(host ${gib(response?.estimatedHostBytes)}/${gib(response?.availableHostBytes)} GiB, ` +
                `GPU ${gib(response?.estimatedGpuBytes)}/${gib(response?.availableGpuBytes)} GiB).`,
            );
        }
        return response;
    }

    async resize(size) {
        const preflight = await this.preflightResize(size);
        const acceptedSize = Number(preflight.size);
        const response = this._requireSuccessfulResponse(
            await this._sendOperationWithRetry(
                { cmd: 'resize', size: acceptedSize }, LONG_OPERATION_TIMEOUT_MS, 'resize',
            ),
            'resize',
        );
        this.latticeSize = Number(response.latticeSize) || acceptedSize;
        this._preparedScenario = null;
        this._markVisualDataDirty(true);
        this._observeTelemetrySourceEpoch(response?.telemetrySourceEpoch);
        return response;
    }

    async resizeScenario(size, name) {
        const preflight = await this.preflightResize(size);
        const acceptedSize = Number(preflight.size);
        const response = this._requireSuccessfulResponse(
            await this._sendOperationWithRetry(
                { cmd: 'resize_scenario', size: acceptedSize, name },
                LONG_OPERATION_TIMEOUT_MS,
                'resize and scenario setup',
            ),
            'resize and scenario setup',
        );
        this.latticeSize = Number(response.latticeSize) || acceptedSize;
        // loadScale0Scenario still executes the canonical UI/toggle path. Its
        // scenario.load() call consumes this marker instead of rebuilding the
        // just-prepared native bridge a second time.
        this._preparedScenario = name;
        this._markVisualDataDirty(true);
        this._observeTelemetrySourceEpoch(response?.telemetrySourceEpoch);
        return response;
    }

    reset() {
        this._markVisualDataDirty(true);
        // reset produces a JSON acknowledgement; track it so it cannot resolve
        // an unrelated diagnostics request in the FIFO response queue.
        if (this._connected) {
            this._sendOperationWithRetry({ cmd: 'reset' }, LONG_OPERATION_TIMEOUT_MS, 'reset')
                .then(response => this._observeTelemetrySourceEpoch(response?.telemetrySourceEpoch))
                .catch(err => {
                    debugLog('[ws-bridge] Native reset failed:', err?.message || err);
                });
        }
        if (this._fallback) this._fallback.reset();
    }

    setupScenario(name) {
        this._markVisualDataDirty(true);
        if (this._scenarioDraft) {
            this._scenarioDraft.name = name;
            this._scenarioDraft.setupRequested = true;
            if (this._preparedScenario === name) this._scenarioDraft.prepared = true;
            return true;
        }
        const prepared = this._preparedScenario === name;
        this._preparedScenario = null;
        this._queuedScenarioProfile = {
            name,
            toggles: {},
            fluxBoundaryMode: this._fluxBoundaryMode,
            setupRequested: true,
            prepared,
        };
        this._scheduleScenarioDispatch();
        return true;
    }

    setDt(dt) { this.setParam('dt', dt); }
    getDt() { return this._params.dt ?? this._lastDiag?.dt ?? 1.0; }
    getPhysicalTime() { return this._lastDiag?.physicalTime ?? 0.0; }
    setOmega0(value) { this.setParam('omega0', value); }
    setLangevinTemp(value) { this.setParam('langevin_T', value); }
    getLangevinTemp() { return this._params.langevin_T ?? 0.0; }
    setLangevinGamma(value) { this.setParam('langevin_gamma', value); }
    getLangevinGamma() { return this._params.langevin_gamma ?? 0.01; }
    setLangevinParams(temperature, gamma) {
        this.setLangevinTemp(temperature);
        this.setLangevinGamma(gamma);
    }

    getLagrangian() {
        // A missing first response is unknown, not a zero-action measurement.
        // telemetry-hub already treats null as "not sampled yet".
        return this._isTelemetryGroupCurrent('lagrangian') ? this._lastLagrangian : null;
    }

    // Knot telemetry — the WS server (ws_server) has no get_knot_* command, so
    // these return the frozen EMPTY shapes unconditionally and never throw. They
    // keep the Scale-0 bridge surface symmetric so panels degrade cleanly on the
    // WS path. TODO(server): add get_knot_telemetry / get_knot_events /
    // get_knot_aggregate cmd handlers + request/cache plumbing here.
    getKnotTelemetry() {
        return EMPTY_KNOT_TELEMETRY;
    }

    getKnotEvents() {
        return EMPTY_KNOT_EVENTS;
    }

    getKnotAggregate() {
        return EMPTY_KNOT_AGG;
    }

    getConstants() {
        return null;
    }

    inspectVoxel(x, y, z) {
        const ix = Math.trunc(Number(x) || 0);
        const iy = Math.trunc(Number(y) || 0);
        const iz = Math.trunc(Number(z) || 0);
        const key = `${ix},${iy},${iz}`;
        const requestedEpoch = this._voxelRequestEpoch.get(key) ?? 0;
        if (this._connected && !this._hasPendingScenarioWork()
            && !this._voxelRequestsInFlight.has(key)
            && this._pointQueryRequestsInFlight < MAX_POINT_QUERY_REQUESTS_IN_FLIGHT
            && requestedEpoch !== this._visualEpoch) {
            this._voxelRequestsInFlight.add(key);
            this._pointQueryRequestsInFlight++;
            this._voxelRequestEpoch.set(key, this._visualEpoch);
            this._sendJSON({ cmd: 'inspect_voxel', x: ix, y: iy, z: iz }, DIAGNOSTIC_COMMAND_TIMEOUT_MS)
                .then(data => { if (!data?.error) { this._voxelCache.set(key, data); if (this._voxelCache.size > MAX_POINT_CACHE) this._voxelCache.delete(this._voxelCache.keys().next().value); } })
                .catch(() => { this._voxelRequestEpoch.set(key, 0); })
                .finally(() => {
                    this._voxelRequestsInFlight.delete(key);
                    this._pointQueryRequestsInFlight = Math.max(
                        0, this._pointQueryRequestsInFlight - 1);
                });
        }
        return this._voxelCache.get(key) || null;
    }

    getForceAt(x, y, z) {
        const ix = Math.trunc(Number(x) || 0);
        const iy = Math.trunc(Number(y) || 0);
        const iz = Math.trunc(Number(z) || 0);
        const key = `${ix},${iy},${iz}`;
        const requestedEpoch = this._forceAtRequestEpoch.get(key) ?? 0;
        if (this._connected && !this._hasPendingScenarioWork()
            && !this._forceAtRequestsInFlight.has(key)
            && this._pointQueryRequestsInFlight < MAX_POINT_QUERY_REQUESTS_IN_FLIGHT
            && requestedEpoch !== this._visualEpoch) {
            this._forceAtRequestsInFlight.add(key);
            this._pointQueryRequestsInFlight++;
            this._forceAtRequestEpoch.set(key, this._visualEpoch);
            this._sendJSON({ cmd: 'get_force_at', x: ix, y: iy, z: iz }, DIAGNOSTIC_COMMAND_TIMEOUT_MS)
                .then(data => { if (!data?.error) { this._forceAtCache.set(key, data); if (this._forceAtCache.size > MAX_POINT_CACHE) this._forceAtCache.delete(this._forceAtCache.keys().next().value); } })
                .catch(() => { this._forceAtRequestEpoch.set(key, 0); })
                .finally(() => {
                    this._forceAtRequestsInFlight.delete(key);
                    this._pointQueryRequestsInFlight = Math.max(
                        0, this._pointQueryRequestsInFlight - 1);
                });
        }
        return this._forceAtCache.get(key) || null;
    }

    getGravityMetricAgg() {
        return this._isTelemetryGroupCurrent('gravity') ? this._lastGravityMetric : null;
    }

    getFluxSlice(axis, index) {
        const key = `${axis}_${index}`;
        const requestedEpoch = this._sliceRequestEpoch.get(key) ?? 0;
        if (this._connected && !this._hasPendingScenarioWork() && !this._sliceRequestsInFlight.has(key)
            && requestedEpoch !== this._visualEpoch) {
            this._sliceRequestsInFlight.add(key);
            this._sliceRequestEpoch.set(key, this._visualEpoch);
            if (!this._sendAndForget({ cmd: 'get_flux_slice', axis, index })) {
                this._sliceRequestsInFlight.delete(key);
                this._sliceRequestEpoch.set(key, 0);
            }
        }
        return this._sliceCache.get(key) || new Float64Array(0);
    }
    getFluxVolume() {
        if (this._connected && !this._hasPendingScenarioWork() && !this._volumeRequestInFlight
            && this._volumeRequestEpoch !== this._visualEpoch) {
            this._volumeRequestInFlight = true;
            this._volumeRequestEpoch = this._visualEpoch;
            if (!this._sendAndForget({
                cmd: 'get_flux_volume',
                axisSamples: FLUX_VOLUME_AXIS_SAMPLES,
            })) {
                this._volumeRequestInFlight = false;
                this._volumeRequestEpoch = 0;
            }
        }
        return this._volumeCache || new Float64Array(0);
    }

    _drainFieldSampleRequests() {
        if (!this._connected || this._hasPendingScenarioWork()) return;
        while (this._fieldSampleRequestsByToken.size < MAX_FIELD_SAMPLE_REQUESTS_IN_FLIGHT
            && this._fieldSampleDemandByKey.size > 0) {
            const first = this._fieldSampleDemandByKey.entries().next().value;
            if (!first) return;
            const [key, demand] = first;
            this._fieldSampleDemandByKey.delete(key);
            if (this._fieldSampleRequestTokenByKey.has(key)) continue;

            let token = this._nextFieldSampleToken++ >>> 0;
            if (token === 0) token = this._nextFieldSampleToken++ >>> 0;
            const pending = { ...demand, token, epoch: this._visualEpoch };
            this._fieldSampleRequestEpoch.set(key, pending.epoch);
            this._fieldSampleRequestTokenByKey.set(key, token);
            this._fieldSampleRequestsByToken.set(token, pending);
            const asSlices = Number.isInteger(pending.planesMid) && pending.planesMid >= 0;
            if (!this._sendAndForget(asSlices ? {
                cmd: 'get_field_slices',
                kind: pending.kind,
                stride: pending.stride,
                mid: pending.planesMid,
                token,
            } : {
                cmd: 'get_field_sample',
                kind: pending.kind,
                stride: pending.stride,
                token,
            })) {
                this._fieldSampleRequestTokenByKey.delete(key);
                this._fieldSampleRequestsByToken.delete(token);
                this._fieldSampleRequestEpoch.set(key, 0);
                // Preserve demand across a transient socket failure. connect()
                // or the next visible read will retry without spinning here.
                this._fieldSampleDemandByKey.set(key, demand);
                return;
            }
        }
    }

    _getFieldSample(kind, stride = 2, fallback = EMPTY_FIELD_SAMPLE, planesMid = -1) {
        if (!FIELD_SAMPLE_KIND_CODES.has(kind)) return fallback;
        const normalizedStride = Math.max(1, Math.min(64, Math.trunc(Number(stride) || 1)));
        // Slice mode (planesMid >= 0): fetch only the three center mid-planes via
        // get_field_slices — same FTS2 payload shape, ~axis× less traffic. Keyed
        // separately (`#mid`) so it never collides with the full-cube cache the
        // 3D viewport overlay populates for the same kind+stride.
        const slice = Number.isInteger(planesMid) && planesMid >= 0;
        const key = slice
            ? `${kind}@${normalizedStride}#${planesMid}`
            : `${kind}@${normalizedStride}`;
        const requestedEpoch = this._fieldSampleRequestEpoch.get(key) ?? 0;
        if (this._connected && !this._hasPendingScenarioWork() && !this._fieldSampleRequestTokenByKey.has(key)
            && requestedEpoch !== this._visualEpoch) {
            // Updating an existing Map value preserves insertion order. This
            // makes a continuously-visible panel fair: old queued kinds drain
            // before a newly-dirty E/B request can jump back to the front.
            this._fieldSampleDemandByKey.set(key, {
                key, kind, stride: normalizedStride, epoch: this._visualEpoch,
                planesMid: slice ? planesMid : -1,
            });
            this._drainFieldSampleRequests();
        }
        return this._fieldSampleCache.get(key) || fallback;
    }

    // Native sampled fields are requested lazily and cached by kind+stride for
    // the current visual epoch. The first synchronous call returns the previous
    // cache (or EMPTY); FTS1 delivery marks the overlay scheduler dirty.
    getEFieldSampled(stride = 2) { return this._getFieldSample('e', stride, EMPTY_FIELD_SAMPLE); }
    sampleVAtRay(x1, y1, z1, x2, y2, z2, n) {
        return { positions: new Float32Array(0), V: new Float32Array(0), count: 0 };
    }
    getBFieldSampled(stride = 2) { return this._getFieldSample('b', stride, EMPTY_FIELD_SAMPLE); }
    getPoyntingSampled(stride = 2) { return this._getFieldSample('poynting', stride, EMPTY_FIELD_SAMPLE); }
    getDivJSampled(stride = 2) { return this._getFieldSample('divJ', stride, EMPTY_SCALAR_SAMPLE); }
    getFluxVectorSampled(stride = 2) { return this._getFieldSample('fluxVector', stride, EMPTY_FIELD_SAMPLE); }
    getForceFieldSampled(stride = 2) { return this._getFieldSample('em', stride, EMPTY_FIELD_SAMPLE); }
    getVorticitySampled(stride = 2) { return this._getFieldSample('vorticity', stride, EMPTY_SCALAR_SAMPLE); }
    getHelicitySampled(stride = 2) { return this._getFieldSample('helicity', stride, EMPTY_SCALAR_SAMPLE); }
    getKretschmannSampled(stride = 2) { return this._getFieldSample('kretschmann', stride, EMPTY_SCALAR_SAMPLE); }
    getLatencySampled(stride = 2) { return this._getFieldSample('latency', stride, EMPTY_SCALAR_SAMPLE); }
    // Real latency-Poisson solution (voxel.latency / CUDA d_latency). Keep the
    // legacy `latency` sampler above as its normalized |J|^2 visual proxy.
    getPoissonLatencySampled(stride = 2) { return this._getFieldSample('poissonLatency', stride, EMPTY_SCALAR_SAMPLE); }
    getFisherSampled(stride = 2) { return this._getFieldSample('fisher', stride, EMPTY_SCALAR_SAMPLE); }
    getCoherenceSampled(stride = 2) { return this._getFieldSample('coherence', stride, EMPTY_SCALAR_SAMPLE); }
    getCurlJSampled(stride = 2) { return this._getFieldSample('curlJ', stride, EMPTY_FIELD_SAMPLE); }
    getStateFieldSampled(stride = 1) { return this._getFieldSample('state', stride, EMPTY_SCALAR_SAMPLE); }
    getGaussResidualSampled(stride = 2) { return this._getFieldSample('gaussResidual', stride, EMPTY_SCALAR_SAMPLE); }
    getGravityFieldSampled(stride = 2) { return this._getFieldSample('gravity', stride, EMPTY_FIELD_SAMPLE); }
    getEMForceField(stride = 2) { return this._getFieldSample('em', stride, EMPTY_FIELD_SAMPLE); }
    getGravityForceField(stride = 2) { return this._getFieldSample('gravity', stride, EMPTY_FIELD_SAMPLE); }
    getStrongForceField(stride = 2) { return this._getFieldSample('strong', stride, EMPTY_FIELD_SAMPLE); }
    /** Kind-dispatched Scale-0 field sampler; see bridge-contract.js samplerOr. */
    getSamplerOr(kind, stride = 2, fallback) { return samplerOr(this, kind, stride, fallback); }
    replaceSamplerWants() {}
    unwantSampler() {}

    /**
     * Flux-slice-panel fast path: fetch ONLY the three center mid-planes for
     * `kind` (get_field_slices), not the whole cube — same {positions, vectors/
     * values, count, effectiveStride, origin} shape the panel's slicers already
     * consume, at ~axis× less WebSocket traffic. `mid` is the panel's own N>>1
     * slice index. Present only on this (native-GPU) bridge; the panel feature-
     * detects it and keeps its in-process getSamplerOr path on the WASM bridge.
     */
    getFieldSlices(kind, mid, stride = 2) {
        const fallback = WS_VECTOR_FIELD_KINDS.has(kind) ? EMPTY_FIELD_SAMPLE : EMPTY_SCALAR_SAMPLE;
        return this._getFieldSample(kind, stride, fallback, Math.max(0, mid | 0));
    }

    // Scale 1 (ParticleEngine) fallback delegation
    initPE() { this._ensureFallback().initPE(); }
    resetPE() { this._ensureFallback().resetPE(); }
    peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        return this._ensureFallback().peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff);
    }
    peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff) {
        return this._ensureFallback().peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff);
    }
    peApplyEquilibriumOrbit(particleId, options = {}) {
        return this._ensureFallback().peApplyEquilibriumOrbit(particleId, options);
    }
    peApplyEquilibriumOrbitBatch(entries) {
        return this._ensureFallback().peApplyEquilibriumOrbitBatch?.(entries);
    }
    peScaleVelocity(particleId, scale) {
        return this._ensureFallback().peScaleVelocity(particleId, scale);
    }
    peSetSpinAxis(id, ax, ay, az) {
        return this._ensureFallback().peSetSpinAxis(id, ax, ay, az);
    }
    peGetForceDecomposition() { return this._ensureFallback().peGetForceDecomposition(); }
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
    }
    setFluxBoundaryMode(mode) {
        const normalized = Math.max(0, Math.min(2, Math.trunc(Number(mode) || 0)));
        if (this._scenarioDraft) {
            this._fluxBoundaryMode = normalized;
            this._scenarioDraft.fluxBoundaryMode = normalized;
            return;
        }
        if (this._fluxBoundaryMode === normalized) return;
        this._fluxBoundaryMode = normalized;
        this._queueLiveProfileMutation({ fluxBoundaryMode: normalized });
    }
    setReflectiveBoundary(on) {
        this._reflectiveBoundary = !!on;
        this.setToggle('reflective_boundary', this._reflectiveBoundary);
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
    aeGetForceDecomposition(want) { return this._ensureFallback().aeGetForceDecomposition(want); }
    aeGetRuntimeState() { return this._ensureFallback().aeGetRuntimeState(); }
    aeGetVelocities() { return this._ensureFallback().aeGetVelocities(); }
    aeGetDipoles() { return this._ensureFallback().aeGetDipoles(); }
    aeGetHBondPairs() { return this._ensureFallback().aeGetHBondPairs(); }
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
    const port = parseNativeWsPort(
        window.location.search,
        window.location.href,
        9100,
    );
    const bridge = new WebSocketBridge(`ws://127.0.0.1:${port}`);
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
