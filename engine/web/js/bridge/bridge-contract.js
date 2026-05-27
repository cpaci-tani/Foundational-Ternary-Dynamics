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
 * (e.g. a TS check pass). It exports nothing at runtime.
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

// Module is documentation-only; nothing to export.
