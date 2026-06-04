/**
 * Bridge contract — the surface every Scale-0 bridge must implement.
 *
 * Two concrete bridges live in `bridge-init.js`:
 *   - `MockBridge` — pure JS lattice for offline / parity testing.
 *   - `WasmBridge` — Emscripten-compiled C++ engine, the canonical path.
 *
 * Neither class formally `implements` this typedef (JS has no nominal
 * interfaces), but every method listed below MUST exist on both
 * classes with matching shape. Callers — particularly
 * `physics-harness.js` and the dashboard panels — depend on the
 * surface being symmetric. Adding a method to one bridge without the
 * other will break panels silently when the active bridge changes.
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
// a `MockBridgeProxy`; it must forward every one of these to its shadow (which
// reads the worker's SharedArrayBuffers) or the consumer silently blanks. This
// is the single source of truth, consumed by:
//   • mock-bridge-proxy.js        — installs one shadow-delegating forwarder per
//                                   name (any not already defined on the proxy).
//   • tests/scale0-worker.spec.js — asserts the proxy answers every name.
// Add a new entry here whenever you add a direct-read method to MockBridge that
// a panel/overlay calls on the raw bridge — the proxy and its regression test
// then pick it up automatically instead of drifting (cf. the one-at-a-time
// `inspectVoxel` patch in 68024ba1).

/** Empty sampler payload (vector + scalar shaped) for pre-ready fallbacks. */
export function emptySampleResult() {
    return { positions: new Float32Array(0), vectors: new Float32Array(0), values: new Float32Array(0), count: 0 };
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
    // Audit / Lagrangian: field terms are live off the shadow; particle terms
    // read zero under the worker (shadow._particles is empty) — acceptable, the
    // confirmed breakage was the field charts. Returns null before ready.
    { name: 'getEnergyAudit',          empty: () => null },
    { name: 'getLagrangian',           empty: () => null },
    { name: 'getForceAt',              empty: () => null },
    // Tier 2 — particle-dependent: the proxy OVERRIDES this with worker-sourced
    // data (shadow._particles is empty). Listed for contract-test coverage; the
    // proxy's own getScale0ParticleList wins over the generic forwarder.
    { name: 'getScale0ParticleList',   empty: () => [] },
];
