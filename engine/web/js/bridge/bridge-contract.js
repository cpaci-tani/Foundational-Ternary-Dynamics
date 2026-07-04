/**
 * Bridge contract — the surface every Scale-0 bridge must implement.
 *
 * The engine is WASM-only (the JS MockBridge was deleted); the live
 * Scale-0 bridges are:
 *   - `WasmBridge` (wasm-bridge.js) — Emscripten-compiled C++ engine, the
 *     canonical main-thread path; re-exported from `bridge-init.js`.
 *   - `WasmBridgeProxy` (wasm-bridge-proxy.js) — main-thread proxy over a
 *     worker-hosted `WasmBridge` (zero-copy SharedArrayBuffer field views).
 *   - `WebSocketBridge` (ws-bridge.js) — the same surface backed by a native
 *     server over WebSocket.
 *
 * None of these classes formally `implements` this typedef (JS has no
 * nominal interfaces), but every method listed below MUST exist on every
 * live bridge with matching shape. Callers — particularly
 * `physics-harness.js` and the dashboard panels — depend on the
 * surface being symmetric. Adding a method to one bridge without the
 * others will break panels silently when the active bridge changes.
 *
 * This file exists for documentation + future static-analysis hooks
 * (e.g. a TS check pass), and exports the canonical direct-read surface
 * (`SCALE0_DIRECT_READS`) consumed by the worker proxy + its regression test.
 *
 * @typedef {object} ScaleBridge
 *
 * Identity
 * @property {boolean} isWasm                      — true iff backed by C++.
 * @property {boolean} ready                       — initialization gate.
 * @property {number}  latticeSize                 — voxels per edge.
 *
 * Lifecycle
 * @property {() => void} reset                    — clears particles, flux, wave-vel, tick.
 *
 * Scenarios
 * @property {(name: string) => void} setupScenario — dispatch by scenario id.
 *
 * Toggles
 * @property {(name: string) => boolean} getToggle  — read a phase-toggle flag.
 * @property {(name: string, value: boolean) => void} setToggle
 *
 * Diagnostics
 * @property {() => object} getDiagnostics          — energy, charge, momentum totals.
 *
 * Particle injection
 * @property {(x: number, y: number, z: number, state: number) => void} injectParticle
 * @property {(x: number, y: number, z: number, state: number) => void} injectWavepacket
 * @property {(x: number, y: number, z: number, fx: number, fy: number, fz: number) => void} injectFlux
 * @property {(x: number, y: number, z: number, fx: number, fy: number, fz: number) => void} createEntangledPair
 *
 * Particle list
 * @property {() => Array<{x:number,y:number,z:number,state:number,charge?:number,q?:number,spin?:number,color?:number,locked?:boolean}>} getScale0ParticleList
 *
 * Field samplers (sparse → dense interpolation done caller-side)
 * @property {(stride?: number) => {positions: Float32Array, vectors: Float32Array, count: number}} getEFieldSampled
 * @property {(stride?: number) => {positions: Float32Array, vectors: Float32Array, count: number}} getBFieldSampled
 * @property {(stride?: number) => {positions: Float32Array, values: Float32Array, count: number}} getLatencySampled
 *
 * Direct ray samplers (engine-resolution, optional — WASM only today)
 * @property {((x1:number,y1:number,z1:number,x2:number,y2:number,z2:number,n:number) => {V: Float32Array, count: number})=} sampleVAtRay
 *
 * 2D slice
 * @property {(axis: 'x'|'y'|'z', mid: number) => {data: Float32Array, n: number} | null} getFluxSlice
 */

// ── Runtime: canonical direct-read surface (anti-drift) ──────────────────────
//
// Methods that dashboard consumers call DIRECTLY on a Scale-0 bridge object
// (NOT via `bridge.capabilities.scale0.*`). Under the worker path the bridge is
// a `WasmBridgeProxy` (wasm-bridge-proxy.js); it serves every one of these from
// worker-sourced data — a zero-copy view over the worker's WASM heap for the
// flux volume plus the last diagnostics/particle/audit frame, with no JS shadow
// — or the consumer silently blanks. This list is the single source of truth
// that pins the surface, consumed by:
//   • tests/scale0-worker.spec.js — asserts the WasmBridgeProxy answers every
//                                   name (the anti-drift regression gate).
// Add a new entry here whenever you add a direct-read method to WasmBridge that
// a panel/overlay calls on the raw bridge, then implement it on WasmBridgeProxy
// — the regression test above then enforces coverage instead of letting the
// worker path drift (cf. the one-at-a-time `inspectVoxel` patch in 68024ba1).

/** Empty sampler payload (vector + scalar shaped) for pre-ready fallbacks. */
export function emptySampleResult() {
    return { positions: new Float32Array(0), vectors: new Float32Array(0), values: new Float32Array(0), count: 0 };
}

// ── Runtime: Scale-0 field-sampler dispatch (anti-drift) ─────────────────────
//
// The Scale-0 capability factory (capabilities/scale0.js) maps a stable overlay
// `kind` to a concrete bridge sampler call. Each live bridge implements a
// different subset of the surface:
//   • WasmBridge / WasmBridgeProxy — every kind.
//   • WebSocketBridge              — the core kinds + vorticity/helicity/curlJ;
//                                    kretschmann/latency/fisher/coherence/state/
//                                    gaussResidual are absent.
// The capability code used to guard each optional kind inline with
// `bridge.getXSampled?.(stride) ?? empty`, so a bridge that DROPPED a sampler
// rendered nothing silently — a CONTRACTS.md §2.4 violation with no signal.
//
// `samplerOr()` centralizes the kind→method map and the empty fallback so:
//   1. absence still yields an empty sample (CONTRACTS.md §2.3 — behavior kept), and
//   2. it is logged once per bridge+kind (§2.4 drift becomes loud, not invisible).
// Every bridge exposes it as `getSamplerOr(kind, stride, fallback)`.

/** Canonical Scale-0 overlay-kind → bridge sampler-method map. */
export const SCALE0_SAMPLER_METHODS = Object.freeze({
    e:             'getEFieldSampled',
    b:             'getBFieldSampled',
    poynting:      'getPoyntingSampled',
    divJ:          'getDivJSampled',
    fluxVector:    'getFluxVectorSampled',
    vorticity:     'getVorticitySampled',
    helicity:      'getHelicitySampled',
    kretschmann:   'getKretschmannSampled',
    latency:       'getLatencySampled',
    fisher:        'getFisherSampled',
    coherence:     'getCoherenceSampled',
    curlJ:         'getCurlJSampled',
    state:         'getStateFieldSampled',
    gaussResidual: 'getGaussResidualSampled',
});

const _samplerDriftWarned = new Set();

/**
 * Dispatch a Scale-0 field sampler by stable `kind`, returning an empty sample
 * (the caller's `fallback`, else the shared empty) when the bridge lacks that
 * sampler — and logging the drift once per bridge+kind instead of hiding it.
 * Shared by every bridge's `getSamplerOr`; keep the empty-fallback behavior
 * identical (CONTRACTS.md §2.3) — this is a consolidation, not a behavior change.
 *
 * @param {object} bridge      concrete bridge (`this` from getSamplerOr)
 * @param {string} kind        overlay kind, see SCALE0_SAMPLER_METHODS
 * @param {number} [stride=2]
 * @param {object} [fallback]  shape returned on absence (default: empty sample)
 */
export function samplerOr(bridge, kind, stride = 2, fallback) {
    const method = SCALE0_SAMPLER_METHODS[kind];
    const fn = method && bridge && bridge[method];
    if (typeof fn === 'function') return fn.call(bridge, stride);
    const tag = `${bridge?.constructor?.name || 'bridge'}#${method || kind}`;
    if (!_samplerDriftWarned.has(tag)) {
        _samplerDriftWarned.add(tag);
        console.warn(
            `[bridge] Scale-0 sampler drift: ${tag} is not implemented — ` +
            `overlay '${kind}' renders empty. All Scale-0 bridges must share ` +
            `this surface (CONTRACTS.md §2.4).`,
        );
    }
    return fallback ?? emptySampleResult();
}

export const SCALE0_DIRECT_READS = [
    // Tier 1 — field/flux/state-derived: the proxy's shadow computes these live
    // from the worker's shared field buffers, so a plain forward returns real data.
    { name: 'getEFieldSampled',        empty: emptySampleResult },
    { name: 'getBFieldSampled',        empty: emptySampleResult },
    { name: 'getPoyntingSampled',      empty: emptySampleResult },
    { name: 'getDivJSampled',          empty: emptySampleResult },
    { name: 'getFluxVectorSampled',    empty: emptySampleResult },
    { name: 'getCurlJSampled',         empty: emptySampleResult },
    { name: 'getVorticitySampled',     empty: emptySampleResult },
    { name: 'getHelicitySampled',      empty: emptySampleResult },
    { name: 'getKretschmannSampled',   empty: emptySampleResult },
    { name: 'getLatencySampled',       empty: emptySampleResult },
    { name: 'getFisherSampled',        empty: emptySampleResult },
    { name: 'getCoherenceSampled',     empty: emptySampleResult },
    { name: 'getStateFieldSampled',    empty: emptySampleResult },
    { name: 'getGaussResidualSampled', empty: emptySampleResult },
    { name: 'getEMForceField',         empty: emptySampleResult },
    { name: 'getGravityForceField',    empty: emptySampleResult },
    { name: 'getStrongForceField',     empty: emptySampleResult },
    { name: 'getForceFieldSampled',    empty: emptySampleResult },
    { name: 'getGravityFieldSampled',  empty: emptySampleResult },
    // Audit / Lagrangian: field terms are live off the shadow; particle terms
    // read zero under the worker (shadow._particles is empty) — acceptable, the
    // confirmed breakage was the field charts. Returns null before ready.
    { name: 'getEnergyAudit',          empty: () => null },
    { name: 'getLagrangian',           empty: () => null },
    { name: 'getForceAt',              empty: () => null },
    // Knot telemetry: ride the worker 'frame' payload (copied heap views), cached
    // on the proxy. Null before any tracked frame arrives — panels treat null as
    // "tracking off / no data yet", never throw.
    { name: 'getKnotTelemetry',        empty: () => null },
    { name: 'getKnotEvents',           empty: () => null },
    { name: 'getKnotAggregate',        empty: () => null },
    // Tier 2 — particle-dependent: the proxy OVERRIDES this with worker-sourced
    // data (shadow._particles is empty). Listed for contract-test coverage; the
    // proxy's own getScale0ParticleList wins over the generic forwarder.
    { name: 'getScale0ParticleList',   empty: () => [] },
];
