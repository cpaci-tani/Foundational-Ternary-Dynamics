/**
 * WASM Bridge — abstraction layer between UI and simulation engine.
 *
 * Provides a MockBridge for development (no WASM needed) and a WasmBridge
 * for production (loads compiled ftd_core.wasm). The UI code only talks
 * to the Bridge interface, never directly to WASM or mock internals.
 */

import { getById as catalogGetById } from './particle-catalog.js';
import { ALPHA, K_B, K_GENESIS, DAMPING, G_N, G_C, C_SPEED, M_PROTON, R_BOHR, N_BASE, G_STAR, VARPI, N_C, B_3, N_EFF } from './constants.js';
import { cpkColor, defaultNeutronCount as elemNeutrons, maxBonds as elemMaxBonds } from './elements.js';
import { debugLog } from './core/log.js';

// ── Atom property helper (simulation units: Bohr-scaled) ──────────
// C++ engine uses Planck units; JS MockBridge uses "simulation units"
// where length = Bohr radius, mass = AMU, energy = tuned for visible
// web dynamics. When WASM AtomEngine is available, a scale conversion
// layer bridges the two unit systems.
//
// Key scales for hydrogen (Z=1):
//   radius ≈ 1.0, sigma ≈ 4.0, epsilon ≈ 0.005, mass ≈ 1.0
//   LJ equilibrium at r ≈ 4.49
//   Bond formation threshold: 1.2 × sigma ≈ 4.8
const AE_EPS_BASE = 0.005;  // LJ well depth for Z=1 (tuned for visible dynamics)
const AE_K_COULOMB = 2.0;    // Ionic coupling (qualitatively correct Coulomb >> vdW)
const AE_K_BOND = 50.0;   // Bond spring stiffness multiplier
const AE_SPEED_MAX = 10.0;   // Speed limit in simulation units
const AE_H_BOND_EPS    = 0.001;  // H-bond LJ 10-12 well depth (sim units; ~1/5 covalent)
const AE_K_ANGLE        = 0.05;   // VSEPR angle strain spring constant (sim units)
const AE_THERMOSTAT_TAU = 10.0;   // Berendsen coupling timescale (in dt units)

// Pauling electronegativity table (Z=0..18), mirrors C++ atom_engine.h:136-148
const AE_CHI_TABLE = [0, 2.20, 0, 0.98, 1.57, 2.04, 2.55, 3.04, 3.44, 3.98,
                      0, 0.93, 1.31, 1.61, 1.90, 2.19, 2.58, 3.16, 0.0];

/**
 * Valence electron count by Z (main-group elements).
 * Used for VSEPR lone-pair geometry (e.g., O has 6 valence → 2 lone pairs → bent).
 * Transition metals and f-block fall back to max_bonds.
 */
function _valenceElectrons(Z) {
    // Main group valence = group number (1-8)
    // Periods 1-2
    const mainGroup = [
        0,                                    // Z=0 placeholder
        1, 2,                                // H, He
        1, 2, 3, 4, 5, 6, 7, 8,            // Li-Ne
        1, 2, 3, 4, 5, 6, 7, 8,            // Na-Ar
    ];
    if (Z <= 18) return mainGroup[Z] || 0;
    // Period 4+: map Z to main-group column
    // Groups 1-2 (s-block), then 13-18 (p-block)
    const col = [
        /*K*/ 1, /*Ca*/ 2,
        /*Sc-Zn (3d transition)*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*Ga*/ 3, /*Ge*/ 4, /*As*/ 5, /*Se*/ 6, /*Br*/ 7, /*Kr*/ 8
    ];
    const offset = (Z - 19) % 18;
    if (offset >= 0 && offset < col.length) {
        const v = col[offset];
        return v > 0 ? v : elemMaxBonds(Z); // transition metals → fallback
    }
    return elemMaxBonds(Z); // f-block and beyond → fallback
}

function computeAtomicProps(Z, N = 0) {
    const mass = Z + N * 1.001;  // Mass in atomic mass units (H≈1, C≈12)
    const z_cbrt = Math.cbrt(Z);
    const radius = z_cbrt > 0 ? 1.0 / z_cbrt : 1.0;  // Bohr-scaled (H=1)
    const vdw_epsilon = AE_EPS_BASE * Math.pow(Z, 2.0 / 3.0);  // Well depth
    const vdw_sigma = radius * N_BASE;  // Electron cloud size (H≈4)
    const max_bonds = elemMaxBonds(Z);
    const electronegativity = (Z >= 1 && Z <= 18) ? AE_CHI_TABLE[Z]
                            : (Z > 18 ? 1.5 + 0.3 * Math.log(Z) : 0);
    return { mass, radius, vdw_epsilon, vdw_sigma, max_bonds, electronegativity };
}

// cpkColor and defaultNeutronCount now imported from elements.js

// ── Mock Bridge ────────────────────────────────────────────────────
export class MockBridge {
    constructor(latticeSize = 32) {
        this.latticeSize = latticeSize;
        this._tick = 0;
        this._dt = 1.0;
        this._physicalTime = 0.0;
        this._particles = [];
        this._nextId = 0;
        this.isWasm = false;

        // Boundary containment
        this._boundaryShape = 'cube';
        this._boundaryMask = null; // Uint8Array: 1=inside, 0=outside. Precomputed per shape.
        this._reflectiveBoundary = true; // When false, particles/flux dissipate past boundary

        // Mutable simulation parameters (combo panel)
        this._params = { kb: K_B, gn: G_N, damping: DAMPING };

        // Toggle states (mirror engine TermToggles from term_toggles.h)
        // NOTE: gravity defaults to false here to match config/toggles.js SCALE0_TOGGLES.
        // Scenarios that need gravity enable it via SCALE0_SCENARIO_OVERRIDES.
        this._toggles = {
            wave_propagation: true, coupling: true, damping: true, genesis: true,
            gauss_projection: true, forces: true, gravity: false, movement: true,
            poisson_coulomb: true, lorentz_force: false, selective_damping: false,
            larmor_radiation: false, dual_substrate: false, confinement: false,
            weak_transmutation: true,
            color_forces: false, strong_force: false, triad_binding: false,
            pair_production: false, exchange_force: false, latency_field: false,
        };

        // Visual settings (shared with viewport for size control)
        this._visualSettings = null;

        // Pre-allocated buffers for getParticleData (reuse across frames to reduce GC)
        this._pdBufCap = 0;
        this._pdPositions = null;
        this._pdColors = null;
        this._pdSizes = null;

        // Cached energy sums — avoids redundant O(L^3) loops across getDiagnostics/getEnergyAudit/getLagrangian
        this._energyCacheTick = -1;
        this._cachedFieldEnergy = 0;
        this._cachedWaveEnergy = 0;
        this._cachedFluxMag = 0;
    }

    /**
     * Compute and cache field/wave energy sums from _fluxJ/_fluxWV.
     * Called once per tick at most; subsequent calls return cached values.
     * At L=128 this avoids 3x redundant O(2M) loops per diagnostics frame.
     *
     * Also populates _fluxMag[] in the same pass, so _updateFluxMag() becomes
     * a no-op when the energy cache is fresh. This eliminates a second full-
     * lattice sqrt loop (2M sqrt calls saved at L=128).
     */
    _ensureEnergyCache() {
        if (this._energyCacheTick === this._tick) return;
        this._energyCacheTick = this._tick;
        let fieldE = 0, waveE = 0, fluxMag = 0;
        if (this._fluxJ) {
            const total = this.latticeSize ** 3;
            const J = this._fluxJ, WV = this._fluxWV;
            const M = this._fluxMag; // also fill magnitude cache
            for (let i = 0, k = 0; i < total; i++, k += 3) {
                const jx = J[k], jy = J[k + 1], jz = J[k + 2];
                const mag2 = jx * jx + jy * jy + jz * jz;
                fieldE += mag2;
                const m = Math.sqrt(mag2);
                fluxMag += m;
                if (M) M[i] = m;  // piggyback: fill _fluxMag in same pass
                const wx = WV[k], wy = WV[k + 1], wz = WV[k + 2];
                waveE += wx * wx + wy * wy + wz * wz;
            }
            fieldE *= 0.5;
            waveE *= 0.5;
            // _fluxMag is now fresh — clear dirty flag so _updateFluxMag() is a no-op
            if (M) this._fluxDirty = false;
        }
        this._cachedFieldEnergy = fieldE;
        this._cachedWaveEnergy = waveE;
        this._cachedFluxMag = fluxMag;
    }

    // ── Boundary containment ──────────────────────────────────────────
    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        this._rebuildBoundaryMask();
    }

    setReflectiveBoundary(on) { this._reflectiveBoundary = !!on; }

    /** Precompute boundary mask so _tickFlux can skip per-voxel _insideBoundary calls. */
    _rebuildBoundaryMask() {
        const N = this.latticeSize;
        if (this._boundaryShape === 'cube' || this._boundaryShape === 'none') {
            this._boundaryMask = null; // no mask needed — all voxels inside
            return;
        }
        const total = N * N * N;
        const mask = new Uint8Array(total);
        const halfN = N / 2;
        for (let z = 0; z < N; z++) {
            for (let y = 0; y < N; y++) {
                for (let x = 0; x < N; x++) {
                    const nx = (x - halfN + 0.5) / halfN;
                    const ny = (y - halfN + 0.5) / halfN;
                    const nz = (z - halfN + 0.5) / halfN;
                    mask[z * N * N + y * N + x] = this._insideBoundary(nx, ny, nz) ? 1 : 0;
                }
            }
        }
        this._boundaryMask = mask;
    }

    /**
     * Test if a normalized point (-1..1 from center) is inside the boundary.
     * Matches viewport._insideBoundary exactly.
     */
    _insideBoundary(nx, ny, nz) {
        switch (this._boundaryShape) {
            case 'none':
            case 'cube':
                return true;
            case 'sphere':
                return (nx * nx + ny * ny + nz * nz) <= 1.0;
            case 'octahedron':
                return (Math.abs(nx) + Math.abs(ny) + Math.abs(nz)) <= 1.0;
            case 'dodecahedron': {
                const phi = 1.618033988749895;
                const ir = 0.7946;
                const normals = [
                    [0, 1, phi], [0, -1, phi], [0, 1, -phi], [0, -1, -phi],
                    [1, phi, 0], [-1, phi, 0], [1, -phi, 0], [-1, -phi, 0],
                    [phi, 0, 1], [-phi, 0, 1], [phi, 0, -1], [-phi, 0, -1],
                ];
                for (const n of normals) {
                    const len = Math.sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
                    if ((nx * n[0] + ny * n[1] + nz * n[2]) / len > ir) return false;
                }
                return true;
            }
            case 'icosahedron': {
                const phi = 1.618033988749895;
                const ir = 0.7558;
                const normals = [
                    [1, 1, 1], [1, 1, -1], [1, -1, 1], [1, -1, -1],
                    [-1, 1, 1], [-1, 1, -1], [-1, -1, 1], [-1, -1, -1],
                    [0, phi, 1/phi], [0, phi, -1/phi], [0, -phi, 1/phi], [0, -phi, -1/phi],
                    [1/phi, 0, phi], [-1/phi, 0, phi], [1/phi, 0, -phi], [-1/phi, 0, -phi],
                    [phi, 1/phi, 0], [phi, -1/phi, 0], [-phi, 1/phi, 0], [-phi, -1/phi, 0],
                ];
                for (const n of normals) {
                    const len = Math.sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
                    if ((nx * n[0] + ny * n[1] + nz * n[2]) / len > ir) return false;
                }
                return true;
            }
            case 'cylinder':
                return (nx * nx + nz * nz) <= 1.0 && Math.abs(ny) <= 1.0;
            case 'torus': {
                const dist_xz = Math.sqrt(nx * nx + nz * nz);
                const dx = dist_xz - 0.7;
                return (dx * dx + ny * ny) <= 0.09; // 0.3²
            }
            default:
                return true;
        }
    }

    /**
     * Reflect a particle/atom back inside the boundary.
     * cx, cy, cz = center; R = half-extent (radius).
     * Modifies the object in-place (x,y,z,vx,vy,vz).
     */
    _reflectIntoBoundary(p, cx, cy, cz, R) {
        if (this._boundaryShape === 'cube' || this._boundaryShape === 'none') return;
        const nx = (p.x - cx) / R;
        const ny = (p.y - cy) / R;
        const nz = (p.z - cz) / R;
        if (this._insideBoundary(nx, ny, nz)) return;
        // Absorbing boundary: let particle pass through (no reflection)
        if (!this._reflectiveBoundary) return;

        // Compute outward normal at boundary surface for reflection
        let snx = 0, sny = 0, snz = 0;
        switch (this._boundaryShape) {
            case 'sphere': {
                const r = Math.sqrt(nx*nx + ny*ny + nz*nz);
                if (r < 1e-10) return;
                snx = nx / r; sny = ny / r; snz = nz / r;
                // Project back onto sphere surface
                p.x = cx + snx * R * 0.99;
                p.y = cy + sny * R * 0.99;
                p.z = cz + snz * R * 0.99;
                break;
            }
            case 'octahedron': {
                // Normal = sign of coordinates (octahedron faces)
                snx = Math.sign(nx) || 1;
                sny = Math.sign(ny) || 1;
                snz = Math.sign(nz) || 1;
                const len = Math.sqrt(snx*snx + sny*sny + snz*snz);
                snx /= len; sny /= len; snz /= len;
                // Project back: move inward along normal
                const dist = Math.abs(nx) + Math.abs(ny) + Math.abs(nz) - 1.0;
                p.x = cx + (nx - snx * dist * 1.01) * R;
                p.y = cy + (ny - sny * dist * 1.01) * R;
                p.z = cz + (nz - snz * dist * 1.01) * R;
                break;
            }
            case 'cylinder': {
                const rXZ = Math.sqrt(nx*nx + nz*nz);
                if (rXZ > 1.0) {
                    snx = nx / rXZ; snz = nz / rXZ;
                    p.x = cx + snx * R * 0.99;
                    p.z = cz + snz * R * 0.99;
                }
                if (Math.abs(ny) > 1.0) {
                    sny = Math.sign(ny);
                    p.y = cy + sny * R * 0.99;
                }
                snx = nx / Math.max(rXZ, 0.01);
                sny = Math.abs(ny) > 1.0 ? Math.sign(ny) : 0;
                snz = nz / Math.max(rXZ, 0.01);
                const nlen = Math.sqrt(snx*snx + sny*sny + snz*snz) || 1;
                snx /= nlen; sny /= nlen; snz /= nlen;
                break;
            }
            case 'torus': {
                const dist_xz = Math.sqrt(nx*nx + nz*nz) || 0.001;
                const cx_ring = 0.7 * nx / dist_xz;
                const cz_ring = 0.7 * nz / dist_xz;
                const dx = nx - cx_ring, dz = nz - cz_ring;
                const dr = Math.sqrt(dx*dx + ny*ny) || 0.001;
                snx = dx / dr; sny = ny / dr; snz = dz / dr;
                p.x = cx + (cx_ring + snx * 0.29) * R;
                p.y = cy + sny * 0.29 * R;
                p.z = cz + (cz_ring + snz * 0.29) * R;
                break;
            }
            default: {
                // Dodecahedron / Icosahedron: use gradient of distance function
                // Nudge inward along the most-violated face normal
                const normals = this._boundaryShape === 'dodecahedron'
                    ? [[0,1,1.618],[0,-1,1.618],[0,1,-1.618],[0,-1,-1.618],
                       [1,1.618,0],[-1,1.618,0],[1,-1.618,0],[-1,-1.618,0],
                       [1.618,0,1],[-1.618,0,1],[1.618,0,-1],[-1.618,0,-1]]
                    : [[1,1,1],[1,1,-1],[1,-1,1],[1,-1,-1],
                       [-1,1,1],[-1,1,-1],[-1,-1,1],[-1,-1,-1],
                       [0,1.618,0.618],[0,1.618,-0.618],[0,-1.618,0.618],[0,-1.618,-0.618],
                       [0.618,0,1.618],[-0.618,0,1.618],[0.618,0,-1.618],[-0.618,0,-1.618],
                       [1.618,0.618,0],[1.618,-0.618,0],[-1.618,0.618,0],[-1.618,-0.618,0]];
                const ir = this._boundaryShape === 'dodecahedron' ? 0.7946 : 0.7558;
                let maxD = -Infinity, bestN = null;
                for (const n of normals) {
                    const len = Math.sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
                    const d = (nx * n[0] + ny * n[1] + nz * n[2]) / len;
                    if (d > maxD) { maxD = d; bestN = [n[0]/len, n[1]/len, n[2]/len]; }
                }
                if (bestN) {
                    const push = (maxD - ir) * 1.01;
                    p.x = cx + (nx - bestN[0] * push) * R;
                    p.y = cy + (ny - bestN[1] * push) * R;
                    p.z = cz + (nz - bestN[2] * push) * R;
                    snx = bestN[0]; sny = bestN[1]; snz = bestN[2];
                }
                break;
            }
        }
        // Reflect velocity: v = v - 2(v·n)n
        const dot = p.vx * snx + p.vy * sny + p.vz * snz;
        if (dot > 0) { // only reflect if moving outward
            p.vx -= 2 * dot * snx;
            p.vy -= 2 * dot * sny;
            p.vz -= 2 * dot * snz;
        }
    }

    setDt(dt) { this._dt = Math.max(1.0, dt); }
    getDt() { return this._dt; }
    getPhysicalTime() { return this._physicalTime; }

    // Safe to call before any scenario is loaded: _particles is [] and _fluxJ is
    // null, so all loops are no-ops. The tick counter still advances, which is
    // harmless — reset() zeros it when a scenario eventually loads.
    tick() {
        this._tick++;
        this._physicalTime += this._dt;
        // Tick flux grid (wave equation) — gated by toggle; no-op if _fluxJ is null
        if (this._toggles.wave_propagation) this._tickFlux();
        // Genesis: spontaneous pair creation from super-threshold flux
        if (this._toggles.genesis && this._fluxJ) {
            const Ng = this.latticeSize;
            const J = this._fluxJ;
            const maxNewPerTick = 4; // cap to prevent explosion
            // PERF: compare squared magnitudes first so the sqrt + exp only
            // run for the (rare) above-threshold voxels. Pre-fix this loop
            // ran a full sqrt per voxel (~260K calls/tick at L=64) even
            // though the early-out is hit on >99% of them.
            const K_GENESIS_SQ = K_GENESIS * K_GENESIS;
            let created = 0;
            for (let z = 1; z < Ng - 1 && created < maxNewPerTick; z++) {
                for (let y = 1; y < Ng - 1 && created < maxNewPerTick; y++) {
                    for (let x = 1; x < Ng - 1 && created < maxNewPerTick; x++) {
                        const idx = z * Ng * Ng + y * Ng + x;
                        const jx = J[idx * 3], jy = J[idx * 3 + 1], jz = J[idx * 3 + 2];
                        const mag2 = jx * jx + jy * jy + jz * jz;
                        if (mag2 < K_GENESIS_SQ) continue;
                        const mag = Math.sqrt(mag2);

                        // Probability: p = 1 - exp(-(mag - K_GENESIS) / K_B)
                        const p = 1 - Math.exp(-(mag - K_GENESIS) / K_B);
                        if (Math.random() > p) continue;

                        // Polarity from divergence sign
                        const divJ = this._divergenceAt(x, y, z);
                        const state = divJ >= 0 ? 1 : -1;

                        // Create particle
                        this.injectParticle(x, y, z, state);

                        // Drain flux (energy conservation)
                        const drain = K_B / (mag + 1e-20);
                        J[idx * 3]     *= (1 - drain);
                        J[idx * 3 + 1] *= (1 - drain);
                        J[idx * 3 + 2] *= (1 - drain);
                        created++;
                    }
                }
            }
        }

        // Tick particles — pairwise forces + Verlet integration
        const N = this.latticeSize;
        const halfN = N / 2;
        const ps = this._particles;
        const doGravity = this._toggles.gravity;
        const doForces = this._toggles.forces;
        const soft = 1.0; // softening length
        const alpha4pi = ALPHA / (4 * Math.PI);
        const gn = this._params.gn;
        const maxV = C_SPEED * 0.3;

        // MED-1 fix: Velocity Verlet integration (matches C++ ParticleEngine)
        // Step 1: Half-kick — v += 0.5 * a * dt (using PREVIOUS tick's forces)
        if (doForces && ps.length > 1) {
            this._computePairwiseForces(ps, N, halfN, soft, alpha4pi, gn, doGravity);
            for (const p of ps) {
                if (p.state === 0 || p.locked) continue;
                p.vx += 0.5 * p.ax;
                p.vy += 0.5 * p.ay;
                p.vz += 0.5 * p.az;
            }
        }

        // Step 2: Drift — x += v * dt
        if (!this._toggles.movement) { /* skip position integration */ }
        else for (const p of ps) {
            if (p.state === 0 || p.locked) continue;
            p.x += p.vx;
            p.y += p.vy;
            p.z += p.vz;
            // Boundary containment
            if (this._boundaryShape === 'cube' || this._boundaryShape === 'none') {
                p.x = ((p.x % N) + N) % N;
                p.y = ((p.y % N) + N) % N;
                p.z = ((p.z % N) + N) % N;
            } else {
                this._reflectIntoBoundary(p, halfN, halfN, halfN, halfN);
            }
            // Speed limit
            const speed = Math.sqrt(p.vx * p.vx + p.vy * p.vy + p.vz * p.vz);
            if (speed > maxV) {
                const s = maxV / speed;
                p.vx *= s; p.vy *= s; p.vz *= s;
            }
        }

        // Step 3: Recompute forces at new positions
        if (doForces && ps.length > 1) {
            this._computePairwiseForces(ps, N, halfN, soft, alpha4pi, gn, doGravity);
            // Step 4: Second half-kick
            for (const p of ps) {
                if (p.state === 0 || p.locked) continue;
                p.vx += 0.5 * p.ax;
                p.vy += 0.5 * p.ay;
                p.vz += 0.5 * p.az;
            }
        }

        // Annihilation: +1 and -1 within close range → both become void + flux burst
        for (let i = 0; i < ps.length; i++) {
            if (ps[i].state === 0 || ps[i].locked) continue;
            for (let j = i + 1; j < ps.length; j++) {
                if (ps[j].state === 0 || ps[j].locked || ps[i].state === ps[j].state) continue;
                let dx = ps[j].x - ps[i].x, dy = ps[j].y - ps[i].y, dz = ps[j].z - ps[i].z;
                if (dx > halfN) dx -= N; else if (dx < -halfN) dx += N;
                if (dy > halfN) dy -= N; else if (dy < -halfN) dy += N;
                if (dz > halfN) dz -= N; else if (dz < -halfN) dz += N;
                const r2 = dx * dx + dy * dy + dz * dz;
                if (r2 < 4) { // annihilation radius = 2
                    // Inject flux burst at midpoint
                    const mx = Math.round((ps[i].x + ps[j].x) / 2);
                    const my = Math.round((ps[i].y + ps[j].y) / 2);
                    const mz = Math.round((ps[i].z + ps[j].z) / 2);
                    if (this._fluxJ) {
                        const burst = K_B * 3;
                        this._injectFlux(mx, my, mz, burst, burst, burst);
                        this._injectFlux(mx + 1, my, mz, burst, 0, 0);
                        this._injectFlux(mx - 1, my, mz, -burst, 0, 0);
                        this._injectFlux(mx, my + 1, mz, 0, burst, 0);
                        this._injectFlux(mx, my - 1, mz, 0, -burst, 0);
                        this._injectFlux(mx, my, mz + 1, 0, 0, burst);
                        this._injectFlux(mx, my, mz - 1, 0, 0, -burst);
                    }
                    ps[i].state = 0; ps[i].density = 0;
                    ps[j].state = 0; ps[j].density = 0;
                }
            }
        }

        // String breaking: when confinement + genesis ON, snap string if pair exceeds R_BREAK
        if (this._toggles.confinement && this._toggles.genesis) {
            const R_BREAK = N / 4;
            let broke = false;
            for (let i = 0; i < ps.length && !broke; i++) {
                if (ps[i].state === 0) continue;
                for (let j = i + 1; j < ps.length && !broke; j++) {
                    if (ps[j].state === 0 || ps[i].state * ps[j].state >= 0) continue;
                    let dx = ps[j].x - ps[i].x, dy = ps[j].y - ps[i].y, dz = ps[j].z - ps[i].z;
                    if (dx > halfN) dx -= N; else if (dx < -halfN) dx += N;
                    if (dy > halfN) dy -= N; else if (dy < -halfN) dy += N;
                    if (dz > halfN) dz -= N; else if (dz < -halfN) dz += N;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    if (r > R_BREAK) {
                        // Snap: create new pair at midpoint
                        const mx = Math.round(((ps[i].x + ps[j].x) / 2 + N) % N);
                        const my = Math.round(((ps[i].y + ps[j].y) / 2 + N) % N);
                        const mz = Math.round(((ps[i].z + ps[j].z) / 2 + N) % N);
                        this.injectParticle(mx - 1, my, mz, 1);
                        this.injectParticle(mx + 1, my, mz, -1);
                        broke = true; // cap at 1 break per tick
                    }
                }
            }
        }

        // Remove dead particles (keep locked particles unconditionally)
        // LOW-1 fix: In-place filter to avoid allocating new array every tick
        const kbThreshold = this._params.kb * 0.01;
        let alive = 0;
        for (let i = 0; i < ps.length; i++) {
            if (ps[i].locked || (ps[i].state !== 0 && ps[i].density > kbThreshold))
                ps[alive++] = ps[i];
        }
        ps.length = alive;
    }

    /**
     * Scale 0 pairwise force computation (called twice per tick for Velocity Verlet).
     *
     * Forces: Coulomb (alpha/(4pi) * q_i*q_j / r^2) + gravity (G_N * K_B^2 / r^2)
     *       + linear confinement (sigma * (r - R_crit) for opposite-sign pairs).
     * Cutoff at r^2 > 2500 to keep O(N^2) manageable for web simulation.
     * Periodic boundary: minimum image convention (halfN wrapping).
     */
    _computePairwiseForces(ps, N, halfN, soft, alpha4pi, gn, doGravity) {
        // Zero accelerations
        for (const p of ps) { p.ax = 0; p.ay = 0; p.az = 0; }
        for (let i = 0; i < ps.length; i++) {
            const pi = ps[i];
            if (pi.state === 0 || pi.locked) continue;
            for (let j = i + 1; j < ps.length; j++) {
                const pj = ps[j];
                if (pj.state === 0) continue;
                let dx = pj.x - pi.x, dy = pj.y - pi.y, dz = pj.z - pi.z;
                if (dx > halfN) dx -= N; else if (dx < -halfN) dx += N;
                if (dy > halfN) dy -= N; else if (dy < -halfN) dy += N;
                if (dz > halfN) dz -= N; else if (dz < -halfN) dz += N;
                const r2raw = dx * dx + dy * dy + dz * dz;
                if (r2raw > 2500) continue;
                const r2 = r2raw + soft;
                const invR2 = 1 / r2;
                const invR = 1 / Math.sqrt(r2);
                // Coulomb
                const qi = pi.state, qj = pj.state;
                const fCoul = -alpha4pi * qi * qj * invR2 * invR;
                let fx = fCoul * dx, fy = fCoul * dy, fz = fCoul * dz;
                // Gravity
                if (doGravity) {
                    const fGrav = gn * K_B * K_B * invR2 * invR;
                    fx += fGrav * dx; fy += fGrav * dy; fz += fGrav * dz;
                }
                // Linear confinement for opposite-sign pairs
                if (this._toggles.confinement && qi * qj < 0) {
                    const r = Math.sqrt(r2raw);
                    const SIGMA = 0.015;   // string tension
                    const R_CRIT = 3.0;    // onset distance
                    if (r > R_CRIT) {
                        const fConf = SIGMA * (r - R_CRIT);
                        const invRc = 1 / r;
                        fx -= fConf * dx * invRc;  // attractive (toward partner)
                        fy -= fConf * dy * invRc;
                        fz -= fConf * dz * invRc;
                    }
                }
                // Newton's 3rd law: equal and opposite
                pi.ax += fx; pi.ay += fy; pi.az += fz;
                pj.ax -= fx; pj.ay -= fy; pj.az -= fz;
            }
        }
    }

    run(n) { for (let i = 0; i < n; i++) this.tick(); }

    reset(latticeSize) {
        this.latticeSize = latticeSize || this.latticeSize;
        this._tick = 0;
        this._dt = 1.0;
        this._physicalTime = 0.0;
        this._particles = [];
        this._nextId = 0;
        // Reset flux grid
        this._fluxJ = null;
        this._fluxWV = null;
        this._fluxMag = null;
        this._fluxDirty = true;
        // Release stale auxiliary buffers so they are reallocated at the new size
        this._stateGrid = null;
        this._selectiveDampMask = null;
        this._energyCacheTick = -1;
        this._params = { kb: K_B, gn: G_N, damping: DAMPING };
        // Reset toggles to defaults (must match constructor and config/toggles.js)
        this._toggles = {
            wave_propagation: true, coupling: true, damping: true, genesis: true,
            gauss_projection: true, forces: true, gravity: false, movement: true,
            poisson_coulomb: true, lorentz_force: false, selective_damping: false,
            larmor_radiation: false, dual_substrate: false, confinement: false,
            weak_transmutation: true,
            color_forces: false, strong_force: false, triad_binding: false,
            pair_production: false, exchange_force: false, latency_field: false,
        };
        // Rebuild boundary mask for new lattice size
        this._rebuildBoundaryMask();
    }

    currentTick() { return this._tick; }

    injectParticle(x, y, z, state) {
        this._particles.push({
            id: this._nextId++, x, y, z, state,
            vx: 0, vy: 0, vz: 0, ax: 0, ay: 0, az: 0,
            density: K_B * 2,
            spin: Math.random() > 0.5 ? 1 : -1,
            color: Math.floor(Math.random() * 4),
            pairId: -1, locked: false
        });
    }

    injectWavepacket(x, y, z, state) {
        this.injectParticle(x, y, z, state);
        const offsets = [[1, 0, 0], [-1, 0, 0], [0, 1, 0], [0, -1, 0], [0, 0, 1], [0, 0, -1]];
        for (const [dx, dy, dz] of offsets) {
            this._particles.push({
                id: this._nextId++,
                x: x + dx * 2, y: y + dy * 2, z: z + dz * 2,
                state: 0, vx: 0, vy: 0, vz: 0, ax: 0, ay: 0, az: 0,
                density: K_B * 0.5,
                spin: 0, color: 0, pairId: -1, locked: false
            });
        }
    }

    setParam(name, value) { if (name in this._params) this._params[name] = value; }
    getParam(name) { return this._params[name] ?? null; }

    injectFlux(x, y, z, fx, fy, fz) {
        this._injectFlux(x, y, z, fx, fy, fz);
    }

    createEntangledPair(x, y, z, fx, fy, fz) {
        const offset = 3;
        this.injectParticle(x - offset, y, z, +1);
        this.injectParticle(x + offset, y, z, -1);
        const ps = this._particles;
        const a = ps[ps.length - 2], b = ps[ps.length - 1];
        a.pairId = b.id; b.pairId = a.id;
        this._injectFlux(x, y, z, fx, fy, fz);
    }

    clearField() {
        if (this._fluxJ) this._fluxJ.fill(0);
        if (this._fluxWV) this._fluxWV.fill(0);
        if (this._fluxMag) this._fluxMag.fill(0);
        this._fluxDirty = true;
    }

    seedRandomFlux() {
        if (!this._fluxJ) this._initFluxGrid();
        const N = this.latticeSize;
        const amp = this._params.kb * 0.3;
        for (let z = 0; z < N; z++)
        for (let y = 0; y < N; y++)
        for (let x = 0; x < N; x++) {
            const idx = this._fluxIdx(x, y, z);
            this._fluxJ[idx * 3]     += (Math.random() - 0.5) * amp;
            this._fluxJ[idx * 3 + 1] += (Math.random() - 0.5) * amp;
            this._fluxJ[idx * 3 + 2] += (Math.random() - 0.5) * amp;
        }
        this._fluxDirty = true;
    }

    setToggle(name, value) { if (name in this._toggles) this._toggles[name] = value; }
    getToggle(name) { return this._toggles[name] ?? true; }

    getParticleData() {
        const count = this._particles.length;
        // Reuse buffers when capacity is sufficient; reallocate only when needed
        if (count > this._pdBufCap) {
            this._pdBufCap = Math.max(count, 16);
            this._pdPositions = new Float32Array(this._pdBufCap * 3);
            this._pdColors = new Float32Array(this._pdBufCap * 3);
            this._pdSizes = new Float32Array(this._pdBufCap);
        }
        const positions = this._pdPositions;
        const colors = this._pdColors;
        const sizes = this._pdSizes;
        // Only render manifested particles (+1, -1) and high-flux void sites.
        // Skip low-density void particles — they cause white grid artifacts
        // when stacked along camera axes with additive blending.
        let outCount = 0;
        const mSize = this._visualSettings ? this._visualSettings.manifestedSize : 12.0;
        const VOID_FLUX_THRESHOLD = 0.05; // only show void sites with significant flux

        for (let i = 0; i < count; i++) {
            const p = this._particles[i];

            // Skip void particles with negligible flux
            if (p.state === 0 && p.density < VOID_FLUX_THRESHOLD) continue;

            positions[outCount * 3] = p.x;
            positions[outCount * 3 + 1] = p.y;
            positions[outCount * 3 + 2] = p.z;
            if (p.state === 1) {
                colors[outCount * 3] = 0.4; colors[outCount * 3 + 1] = 0.87; colors[outCount * 3 + 2] = 0.5;
                sizes[outCount] = mSize;
            } else if (p.state === -1) {
                colors[outCount * 3] = 0.97; colors[outCount * 3 + 1] = 0.44; colors[outCount * 3 + 2] = 0.44;
                sizes[outCount] = mSize;
            } else {
                // High-flux void: show as dim blue dot
                colors[outCount * 3] = 0.3; colors[outCount * 3 + 1] = 0.4; colors[outCount * 3 + 2] = 0.6;
                sizes[outCount] = 2.0 + p.density * 6.0;
            }
            outCount++;
        }
        return { positions, colors, sizes, count: outCount };
    }

    getDiagnostics() {
        // Single-pass counting (replaces 8x .filter() per frame)
        let manifestedCount = 0, positive = 0, negative = 0;
        let spinUp = 0, spinDown = 0;
        let colorless = 0, colorRed = 0, colorGreen = 0, colorBlue = 0;
        let totalFlux = 0;
        for (let i = 0; i < this._particles.length; i++) {
            const p = this._particles[i];
            totalFlux += p.density;
            if (p.state === 0) continue;
            manifestedCount++;
            if (p.state === 1) positive++;
            else if (p.state === -1) negative++;
            if (p.spin === 1) spinUp++;
            else if (p.spin === -1) spinDown++;
            if (!p.color || p.color === 0) colorless++;
            else if (p.color === 1) colorRed++;
            else if (p.color === 2) colorGreen++;
            else if (p.color === 3) colorBlue++;
        }

        // Use cached energy sums (computed once per tick, avoids redundant O(L^3) loop)
        this._ensureEnergyCache();
        const fieldEnergy = this._cachedFieldEnergy;
        const waveEnergy = this._cachedWaveEnergy;
        if (this._fluxJ) {
            totalFlux = Math.sqrt(fieldEnergy * 2);  // RMS flux magnitude
        }

        const totalEnergy = fieldEnergy + waveEnergy;
        return {
            tick: this._tick, physicalTime: this._physicalTime, dt: this._dt,
            manifested: manifestedCount, positive, negative,
            totalFlux: +totalFlux.toFixed(4),
            totalEnergy: +totalEnergy.toFixed(4),
            maxBandwidth: 0, avgDrag: 0,
            entropy: totalEnergy > 0 ? Math.log(totalEnergy + 1) : 0,
            chargeBalance: positive - negative,
            spinUp, spinDown, colorless, colorRed, colorGreen, colorBlue,
            angMomX: 0, angMomY: 0, angMomZ: 0
        };
    }

    getEnergyAudit() {
        // Use cached energy sums (computed once per tick via _ensureEnergyCache)
        this._ensureEnergyCache();
        const fieldEnergy = this._cachedFieldEnergy;
        const waveEnergy = this._cachedWaveEnergy;
        // E = -wave_vel, B = curl(J) — compute EM field energies
        let EFieldEnergy = waveEnergy; // |E|^2/2 = |wave_vel|^2/2
        let BFieldEnergy = 0;
        let poyntingX = 0, poyntingY = 0, poyntingZ = 0;
        // Dual substrate energies
        let ELTotal = 0, ERTotal = 0, wvLTotal = 0, wvRTotal = 0, chiralityTotal = 0;

        return {
            fieldEnergy, waveEnergy, particleKE: 0,
            totalEnergy: fieldEnergy + waveEnergy,
            EFieldEnergy, BFieldEnergy,
            totalPoynting: { x: poyntingX, y: poyntingY, z: poyntingZ },
            gaussViolation: 0, maxGaussError: 0, selfFieldInjection: 0,
            coulombPE: 0, chargeTotal: 0, manifested: 0,
            ELTotal, ERTotal, chiralityTotal, wvLTotal, wvRTotal,
        };
    }

    getLagrangian() {
        // Count manifested particles without allocating a filtered array
        let N = 0;
        for (let i = 0; i < this._particles.length; i++) {
            if (this._particles[i].state !== 0) N++;
        }
        // Use cached energy sums (computed once per tick via _ensureEnergyCache)
        this._ensureEnergyCache();
        const fieldEnergy = this._cachedFieldEnergy;
        const waveEnergy = this._cachedWaveEnergy;
        const totalFluxMag = this._cachedFluxMag;
        const dissipation = (fieldEnergy + waveEnergy) * this._params.damping;
        const total = waveEnergy + fieldEnergy;
        return {
            fieldKinetic: waveEnergy,       // ½|wave_vel|² (field kinetic energy)
            fieldGradient: -fieldEnergy,    // -½c²|∇J|² (approximated from field energy)
            bornInfeld: 0,                  // -K_B√(1-v²) (zero in MockBridge)
            coupling: 0,                    // g_c·s·∇·J (zero without particles)
            velocity: 0,                    // g_c·s·(v·J) (zero without particles)
            gauss: 0,                       // Gauss constraint (zero in free wave)
            dissipation,                    // γ·½|J|²
            total,
            hamiltonian: total,
            totalAction: total,
            gaussViolation: 0, maxGaussError: 0,
            totalFluxMag, totalWaveEnergy: waveEnergy,
            manifested: N, locked: 0
        };
    }

    getConstants() {
        return {
            ALPHA, ALPHA_INV: 1.0 / ALPHA, G_STAR, K_B, K_GENESIS,
            G_C: Math.sqrt(ALPHA), G_N, DAMPING, C_SPEED,
            N_C, B3: B_3, N_BASE, N_EFF, VARPI
        };
    }

    inspectVoxel(x, y, z) {
        const p = this._particles.find(p =>
            Math.round(p.x) === x && Math.round(p.y) === y && Math.round(p.z) === z
        );
        if (p) {
            // Manifested voxel — read flux from grid if available
            let fx = 0, fy = 0, fz = 0;
            if (this._fluxJ) {
                const idx = this._fluxIdx(x, y, z);
                fx = this._fluxJ[idx * 3] || 0;
                fy = this._fluxJ[idx * 3 + 1] || 0;
                fz = this._fluxJ[idx * 3 + 2] || 0;
            }
            const Emag = Math.sqrt(fx*fx + fy*fy + fz*fz);
            return {
                state: p.state, particleId: p.id, pairId: p.pairId,
                locked: p.locked, spin: p.spin, color: p.color,
                fluxX: fx, fluxY: fy, fluxZ: fz, density: p.density,
                waveVelX: 0, waveVelY: 0, waveVelZ: 0,
                velX: p.vx, velY: p.vy, velZ: p.vz,
                speed: Math.sqrt(p.vx * p.vx + p.vy * p.vy + p.vz * p.vz),
                accelMag: 0, divJ: 0, curlX: 0, curlY: 0, curlZ: 0,
                Emag: Emag, Bmag: 0
            };
        }
        // Void voxel — still return flux data from the grid
        let fx = 0, fy = 0, fz = 0;
        if (this._fluxJ) {
            const idx = this._fluxIdx(x, y, z);
            fx = this._fluxJ[idx * 3] || 0;
            fy = this._fluxJ[idx * 3 + 1] || 0;
            fz = this._fluxJ[idx * 3 + 2] || 0;
        }
        const density = Math.sqrt(fx*fx + fy*fy + fz*fz);
        return {
            state: 0, particleId: -1, pairId: -1,
            locked: false, spin: 0, color: 0,
            fluxX: fx, fluxY: fy, fluxZ: fz, density: density,
            waveVelX: 0, waveVelY: 0, waveVelZ: 0,
            velX: 0, velY: 0, velZ: 0,
            speed: 0, accelMag: 0, divJ: 0,
            curlX: 0, curlY: 0, curlZ: 0,
            Emag: density, Bmag: 0
        };
    }

    getForceAt(x, y, z) {
        return {
            coulombX: 0, coulombY: 0, coulombZ: 0, coulombMag: 0,
            strongX: 0, strongY: 0, strongZ: 0, strongMag: 0,
            magneticX: 0, magneticY: 0, magneticZ: 0, magneticMag: 0,
            gravityX: 0, gravityY: 0, gravityZ: 0, gravityMag: 0,
            exchangeX: 0, exchangeY: 0, exchangeZ: 0, exchangeMag: 0
        };
    }

    // ── Flux Grid Simulation (Scale 0 substrate fallback) ──────────
    // Lightweight 3D wave equation on a small grid for flux visualization
    // when WASM is unavailable.
    _initFluxGrid() {
        const N = this.latticeSize;
        const total = N * N * N;
        this._fluxJ = new Float64Array(total * 3); // flux vector field (Jx, Jy, Jz)
        this._fluxWV = new Float64Array(total * 3); // wave velocity (leapfrog)
        this._fluxMag = new Float64Array(total);     // cached magnitudes
        this._fluxDirty = true;
    }

    _fluxIdx(x, y, z) {
        const N = this.latticeSize;
        return ((z + N) % N) * N * N + ((y + N) % N) * N + ((x + N) % N);
    }

    _injectFlux(x, y, z, fx, fy, fz) {
        if (!this._fluxJ) this._initFluxGrid();
        const idx = this._fluxIdx(x, y, z);
        this._fluxJ[idx * 3] += fx;
        this._fluxJ[idx * 3 + 1] += fy;
        this._fluxJ[idx * 3 + 2] += fz;
        this._fluxDirty = true;
    }

    _injectWaveVel(x, y, z, wx, wy, wz) {
        if (!this._fluxWV) this._initFluxGrid();
        const idx = this._fluxIdx(x, y, z);
        this._fluxWV[idx * 3] += wx;
        this._fluxWV[idx * 3 + 1] += wy;
        this._fluxWV[idx * 3 + 2] += wz;
    }

    /**
     * Flux wave propagation: leapfrog integration of the 3D vector wave equation.
     *
     * Physics: WV (wave velocity) += c^2 * Laplacian(J) + G_C * grad(s)
     *          J  += WV  (then both are damped)
     *
     * The Laplacian uses an 18-point isotropic stencil (6 face neighbors at
     * weight 1/3 + 12 edge neighbors at weight 1/6 - 4*center) which cancels
     * O(k^4) anisotropy for faithful wave propagation on a cubic lattice.
     *
     * The coupling term G_C * grad(s) sources the flux field from the discrete
     * state field s in {-1, 0, +1}, implementing the Euler-Lagrange equation.
     *
     * Damping modes: uniform (all voxels) or selective (only near particles,
     * preserving free-wave propagation in empty space).
     */
    _tickFlux() {
        if (!this._fluxJ) return;
        const N = this.latticeSize;
        const c2 = C_SPEED * C_SPEED;
        const damp = this._toggles.damping ? (1.0 - this._params.damping) : 1.0;
        const J = this._fluxJ;
        const WV = this._fluxWV;
        const NN = N * N;
        const NNN = N * N * N;

        // Build state grid from particles for coupling term: g_c * grad(s)
        // State field s ∈ {-1, 0, +1} mapped onto the lattice
        // PERF: skip the entire NNN-byte fill+scan when no particles exist.
        // The hot stencil loop checks `stateGrid` (null) to skip the coupling
        // branch, so empty-lattice scenarios pay zero cost here. At L=128
        // this saves ~2M Int8 writes per tick.
        let stateGrid = null;
        const doCoupling = this._toggles.coupling && this._particles.length > 0;
        if (doCoupling) {
            if (!this._stateGrid || this._stateGrid.length !== NNN) {
                this._stateGrid = new Int8Array(NNN);
            }
            stateGrid = this._stateGrid;
            stateGrid.fill(0);
            for (const p of this._particles) {
                if (p.state === 0) continue;
                const px = ((Math.round(p.x) % N) + N) % N;
                const py = ((Math.round(p.y) % N) + N) % N;
                const pz = ((Math.round(p.z) % N) + N) % N;
                stateGrid[pz * NN + py * N + px] = p.state;
            }
        }

        // 18-point isotropic Laplacian: (1/3)*face + (1/6)*edge - 4*center
        // Cancels O(k^4) anisotropy for faithful wave propagation (matches C++ engine)
        const W_FACE = 1.0 / 3.0;
        const W_EDGE = 1.0 / 6.0;
        const gc_half = G_C * 0.5;
        const Nm1 = N - 1;

        // ── PERFORMANCE: Interior/boundary split ─────────────────────────
        // Interior voxels (1..N-2 in all axes) need no modular arithmetic for
        // neighbor indexing — a straight +/-1, +/-N, +/-NN offset suffices.
        // Boundary voxels (where any coordinate is 0 or N-1) use the original
        // modular path. For L=128 this makes ~97% of voxels take the fast path,
        // eliminating ~18 modulo ops per voxel on the interior.
        //
        // The inner c=0,1,2 component loop is unrolled to avoid loop overhead
        // and let the JIT keep values in scalar registers.
        //
        // Pre-computed byte offsets: neighbor flat indices differ from the center
        // by constant amounts (±1, ±N, ±NN and combinations). Multiplying by 3
        // gives byte offsets into the interleaved J/WV arrays. These are computed
        // once and reused for all ~2M interior voxels, saving ~36 multiplies per
        // voxel (18 neighbors × 2 for face0/edge0 indexing).

        // ── Pre-computed byte offsets for 18-neighbor stencil ────────────
        // Each offset is relative to i3 = idx*3 in the interleaved J/WV arrays.
        // Face neighbors: ±1 in x, ±N in y, ±NN in z (6 total)
        const o_xp = 3;           // (+1,0,0)
        const o_xm = -3;          // (-1,0,0)
        const o_yp = N * 3;       // (0,+1,0)
        const o_ym = -N * 3;      // (0,-1,0)
        const o_zp = NN * 3;      // (0,0,+1)
        const o_zm = -NN * 3;     // (0,0,-1)
        // Edge neighbors: combinations of two axes (12 total)
        const o_xpyp = o_xp + o_yp;   // (+1,+1,0)
        const o_xpym = o_xp + o_ym;   // (+1,-1,0)
        const o_xmyp = o_xm + o_yp;   // (-1,+1,0)
        const o_xmym = o_xm + o_ym;   // (-1,-1,0)
        const o_xpzp = o_xp + o_zp;   // (+1,0,+1)
        const o_xpzm = o_xp + o_zm;   // (+1,0,-1)
        const o_xmzp = o_xm + o_zp;   // (-1,0,+1)
        const o_xmzm = o_xm + o_zm;   // (-1,0,-1)
        const o_ypzp = o_yp + o_zp;   // (0,+1,+1)
        const o_ypzm = o_yp + o_zm;   // (0,+1,-1)
        const o_ymzp = o_ym + o_zp;   // (0,-1,+1)
        const o_ymzm = o_ym + o_zm;   // (0,-1,-1)

        // ── Fast interior path (no modulo, pre-computed byte offsets) ────
        for (let z = 1; z < Nm1; z++) {
            const zBase = z * NN;
            for (let y = 1; y < Nm1; y++) {
                const rowStart = zBase + y * N + 1;
                // Byte offset of (x=1, y, z) in the interleaved array
                let i3 = rowStart * 3;
                // Flat voxel index (for stateGrid coupling); advanced in lockstep with i3
                let vi = rowStart;

                for (let x = 1; x < Nm1; x++) {
                    // Laplacian via pre-computed byte offsets — no per-neighbor multiply
                    // c = 0
                    const center0 = J[i3];
                    const face0 = J[i3 + o_xp] + J[i3 + o_xm]
                                + J[i3 + o_yp] + J[i3 + o_ym]
                                + J[i3 + o_zp] + J[i3 + o_zm];
                    const edge0 = J[i3 + o_xpyp] + J[i3 + o_xpym]
                                + J[i3 + o_xmyp] + J[i3 + o_xmym]
                                + J[i3 + o_xpzp] + J[i3 + o_xpzm]
                                + J[i3 + o_xmzp] + J[i3 + o_xmzm]
                                + J[i3 + o_ypzp] + J[i3 + o_ypzm]
                                + J[i3 + o_ymzp] + J[i3 + o_ymzm];
                    WV[i3] += c2 * (W_FACE * face0 + W_EDGE * edge0 - 4.0 * center0);

                    // c = 1
                    const i3p1 = i3 + 1;
                    const center1 = J[i3p1];
                    const face1 = J[i3p1 + o_xp] + J[i3p1 + o_xm]
                                + J[i3p1 + o_yp] + J[i3p1 + o_ym]
                                + J[i3p1 + o_zp] + J[i3p1 + o_zm];
                    const edge1 = J[i3p1 + o_xpyp] + J[i3p1 + o_xpym]
                                + J[i3p1 + o_xmyp] + J[i3p1 + o_xmym]
                                + J[i3p1 + o_xpzp] + J[i3p1 + o_xpzm]
                                + J[i3p1 + o_xmzp] + J[i3p1 + o_xmzm]
                                + J[i3p1 + o_ypzp] + J[i3p1 + o_ypzm]
                                + J[i3p1 + o_ymzp] + J[i3p1 + o_ymzm];
                    WV[i3p1] += c2 * (W_FACE * face1 + W_EDGE * edge1 - 4.0 * center1);

                    // c = 2
                    const i3p2 = i3 + 2;
                    const center2 = J[i3p2];
                    const face2 = J[i3p2 + o_xp] + J[i3p2 + o_xm]
                                + J[i3p2 + o_yp] + J[i3p2 + o_ym]
                                + J[i3p2 + o_zp] + J[i3p2 + o_zm];
                    const edge2 = J[i3p2 + o_xpyp] + J[i3p2 + o_xpym]
                                + J[i3p2 + o_xmyp] + J[i3p2 + o_xmym]
                                + J[i3p2 + o_xpzp] + J[i3p2 + o_xpzm]
                                + J[i3p2 + o_xmzp] + J[i3p2 + o_xmzm]
                                + J[i3p2 + o_ypzp] + J[i3p2 + o_ypzm]
                                + J[i3p2 + o_ymzp] + J[i3p2 + o_ymzm];
                    WV[i3p2] += c2 * (W_FACE * face2 + W_EDGE * edge2 - 4.0 * center2);

                    // State-flux coupling (interior fast path)
                    if (doCoupling && stateGrid) {
                        WV[i3]     += gc_half * (stateGrid[vi + 1] - stateGrid[vi - 1]);
                        WV[i3p1]   += gc_half * (stateGrid[vi + N] - stateGrid[vi - N]);
                        WV[i3p2]   += gc_half * (stateGrid[vi + NN] - stateGrid[vi - NN]);
                    }

                    i3 += 3; // advance byte offset to next x voxel
                    vi++;    // advance flat voxel index in lockstep
                }
            }
        }

        // ── Slow boundary path (with modulo, handles periodic wrap) ──────
        // Only runs for voxels where z=0, z=N-1, y=0, y=N-1, x=0, or x=N-1.
        // For L=128 this is ~3% of total voxels.
        for (let z = 0; z < N; z++) {
            const zw = z * NN;
            const zpw = ((z + 1) % N) * NN;
            const zmw = ((z - 1 + N) % N) * NN;
            const zBoundary = (z === 0 || z === Nm1);
            for (let y = 0; y < N; y++) {
                const yw = y * N;
                const ypw = ((y + 1) % N) * N;
                const ymw = ((y - 1 + N) % N) * N;
                const yBoundary = (y === 0 || y === Nm1);

                // Skip interior rows — they were already processed above
                if (!zBoundary && !yBoundary) continue;

                for (let x = 0; x < N; x++) {
                    const xpx = (x + 1) % N;
                    const xmx = (x - 1 + N) % N;
                    const idx = zw + yw + x;

                    // 6 face neighbors
                    const xp = zw + yw + xpx;
                    const xm = zw + yw + xmx;
                    const yp = zw + ypw + x;
                    const ym = zw + ymw + x;
                    const zp = zpw + yw + x;
                    const zm = zmw + yw + x;

                    // 12 edge neighbors
                    const xpyp = zw + ypw + xpx;
                    const xpym = zw + ymw + xpx;
                    const xmyp = zw + ypw + xmx;
                    const xmym = zw + ymw + xmx;
                    const xpzp = zpw + yw + xpx;
                    const xpzm = zmw + yw + xpx;
                    const xmzp = zpw + yw + xmx;
                    const xmzm = zmw + yw + xmx;
                    const ypzp = zpw + ypw + x;
                    const ypzm = zmw + ypw + x;
                    const ymzp = zpw + ymw + x;
                    const ymzm = zmw + ymw + x;

                    const i3 = idx * 3;

                    // c = 0
                    const center0 = J[i3];
                    const face0 = J[xp * 3] + J[xm * 3]
                                + J[yp * 3] + J[ym * 3]
                                + J[zp * 3] + J[zm * 3];
                    const edge0 = J[xpyp * 3] + J[xpym * 3]
                                + J[xmyp * 3] + J[xmym * 3]
                                + J[xpzp * 3] + J[xpzm * 3]
                                + J[xmzp * 3] + J[xmzm * 3]
                                + J[ypzp * 3] + J[ypzm * 3]
                                + J[ymzp * 3] + J[ymzm * 3];
                    WV[i3] += c2 * (W_FACE * face0 + W_EDGE * edge0 - 4.0 * center0);

                    // c = 1
                    const i3p1 = i3 + 1;
                    const center1 = J[i3p1];
                    const face1 = J[xp * 3 + 1] + J[xm * 3 + 1]
                                + J[yp * 3 + 1] + J[ym * 3 + 1]
                                + J[zp * 3 + 1] + J[zm * 3 + 1];
                    const edge1 = J[xpyp * 3 + 1] + J[xpym * 3 + 1]
                                + J[xmyp * 3 + 1] + J[xmym * 3 + 1]
                                + J[xpzp * 3 + 1] + J[xpzm * 3 + 1]
                                + J[xmzp * 3 + 1] + J[xmzm * 3 + 1]
                                + J[ypzp * 3 + 1] + J[ypzm * 3 + 1]
                                + J[ymzp * 3 + 1] + J[ymzm * 3 + 1];
                    WV[i3p1] += c2 * (W_FACE * face1 + W_EDGE * edge1 - 4.0 * center1);

                    // c = 2
                    const i3p2 = i3 + 2;
                    const center2 = J[i3p2];
                    const face2 = J[xp * 3 + 2] + J[xm * 3 + 2]
                                + J[yp * 3 + 2] + J[ym * 3 + 2]
                                + J[zp * 3 + 2] + J[zm * 3 + 2];
                    const edge2 = J[xpyp * 3 + 2] + J[xpym * 3 + 2]
                                + J[xmyp * 3 + 2] + J[xmym * 3 + 2]
                                + J[xpzp * 3 + 2] + J[xpzm * 3 + 2]
                                + J[xmzp * 3 + 2] + J[xmzm * 3 + 2]
                                + J[ypzp * 3 + 2] + J[ypzm * 3 + 2]
                                + J[ymzp * 3 + 2] + J[ymzm * 3 + 2];
                    WV[i3p2] += c2 * (W_FACE * face2 + W_EDGE * edge2 - 4.0 * center2);

                    // State-flux coupling (boundary path)
                    if (doCoupling && stateGrid) {
                        WV[i3]     += gc_half * (stateGrid[xp] - stateGrid[xm]);
                        WV[i3p1]   += gc_half * (stateGrid[yp] - stateGrid[ym]);
                        WV[i3p2]   += gc_half * (stateGrid[zp] - stateGrid[zm]);
                    }
                }
            }
        }

        // Also process boundary x-edges that were skipped: rows where z and y
        // are interior but x=0 or x=N-1 were not visited by the interior loop.
        // The interior loop runs x from 1..N-2, so x=0 and x=N-1 on interior
        // y,z rows need the modular path.
        for (let z = 1; z < Nm1; z++) {
            const zw = z * NN;
            const zpw = zw + NN;
            const zmw = zw - NN;
            for (let y = 1; y < Nm1; y++) {
                const yw = y * N;
                const ypw = yw + N;
                const ymw = yw - N;

                // Process x=0 and x=N-1 for this interior (y,z) row
                for (const x of [0, Nm1]) {
                    const xpx = (x + 1) % N;
                    const xmx = (x - 1 + N) % N;
                    const idx = zw + yw + x;

                    const xp = zw + yw + xpx;
                    const xm = zw + yw + xmx;
                    const yp = zw + ypw + x;
                    const ym = zw + ymw + x;
                    const zp = zpw + yw + x;
                    const zm = zmw + yw + x;

                    const xpyp = zw + ypw + xpx;
                    const xpym = zw + ymw + xpx;
                    const xmyp = zw + ypw + xmx;
                    const xmym = zw + ymw + xmx;
                    const xpzp = zpw + yw + xpx;
                    const xpzm = zmw + yw + xpx;
                    const xmzp = zpw + yw + xmx;
                    const xmzm = zmw + yw + xmx;
                    const ypzp = zpw + ypw + x;
                    const ypzm = zmw + ypw + x;
                    const ymzp = zpw + ymw + x;
                    const ymzm = zmw + ymw + x;

                    const i3 = idx * 3;
                    const center0 = J[i3];
                    const face0 = J[xp * 3] + J[xm * 3] + J[yp * 3] + J[ym * 3] + J[zp * 3] + J[zm * 3];
                    const edge0 = J[xpyp*3]+J[xpym*3]+J[xmyp*3]+J[xmym*3]+J[xpzp*3]+J[xpzm*3]+J[xmzp*3]+J[xmzm*3]+J[ypzp*3]+J[ypzm*3]+J[ymzp*3]+J[ymzm*3];
                    WV[i3] += c2 * (W_FACE * face0 + W_EDGE * edge0 - 4.0 * center0);

                    const i3p1 = i3 + 1;
                    const center1 = J[i3p1];
                    const face1 = J[xp*3+1]+J[xm*3+1]+J[yp*3+1]+J[ym*3+1]+J[zp*3+1]+J[zm*3+1];
                    const edge1 = J[xpyp*3+1]+J[xpym*3+1]+J[xmyp*3+1]+J[xmym*3+1]+J[xpzp*3+1]+J[xpzm*3+1]+J[xmzp*3+1]+J[xmzm*3+1]+J[ypzp*3+1]+J[ypzm*3+1]+J[ymzp*3+1]+J[ymzm*3+1];
                    WV[i3p1] += c2 * (W_FACE * face1 + W_EDGE * edge1 - 4.0 * center1);

                    const i3p2 = i3 + 2;
                    const center2 = J[i3p2];
                    const face2 = J[xp*3+2]+J[xm*3+2]+J[yp*3+2]+J[ym*3+2]+J[zp*3+2]+J[zm*3+2];
                    const edge2 = J[xpyp*3+2]+J[xpym*3+2]+J[xmyp*3+2]+J[xmym*3+2]+J[xpzp*3+2]+J[xpzm*3+2]+J[xmzp*3+2]+J[xmzm*3+2]+J[ypzp*3+2]+J[ypzm*3+2]+J[ymzp*3+2]+J[ymzm*3+2];
                    WV[i3p2] += c2 * (W_FACE * face2 + W_EDGE * edge2 - 4.0 * center2);

                    if (doCoupling && stateGrid) {
                        WV[i3]   += gc_half * (stateGrid[xp] - stateGrid[xm]);
                        WV[i3p1] += gc_half * (stateGrid[yp] - stateGrid[ym]);
                        WV[i3p2] += gc_half * (stateGrid[zp] - stateGrid[zm]);
                    }
                }
            }
        }

        // Commit: J += WV, J *= damp (selective or uniform)
        // Unrolled component loop and flat stride for cache-friendly access.
        const total = N * N * N;
        const selective = this._toggles.selective_damping;
        const total3 = total * 3;

        if (selective && damp < 1.0) {
            // Build near-particle mask: mark 6-connected neighbors of manifested particles
            if (!this._selectiveDampMask || this._selectiveDampMask.length !== total) {
                this._selectiveDampMask = new Uint8Array(total);
            }
            this._selectiveDampMask.fill(0);
            for (const p of this._particles) {
                const px = ((p.x % N) + N) % N;
                const py = ((p.y % N) + N) % N;
                const pz = ((p.z % N) + N) % N;
                const pidx = pz * N * N + py * N + px;
                this._selectiveDampMask[pidx] = 1;
                // 6-connected face neighbors
                const offsets = [
                    [(px + 1) % N, py, pz], [(px - 1 + N) % N, py, pz],
                    [px, (py + 1) % N, pz], [px, (py - 1 + N) % N, pz],
                    [px, py, (pz + 1) % N], [px, py, (pz - 1 + N) % N],
                ];
                for (const [nx, ny, nz] of offsets) {
                    this._selectiveDampMask[nz * N * N + ny * N + nx] = 1;
                }
            }
            // Apply: damp both J and WV near particles (matching C++ engine), lossless elsewhere
            for (let i = 0; i < total; i++) {
                const d = this._selectiveDampMask[i] ? damp : 1.0;
                const i3 = i * 3;
                J[i3]     = (J[i3]     + WV[i3])     * d;
                J[i3 + 1] = (J[i3 + 1] + WV[i3 + 1]) * d;
                J[i3 + 2] = (J[i3 + 2] + WV[i3 + 2]) * d;
                WV[i3]     *= d;
                WV[i3 + 1] *= d;
                WV[i3 + 2] *= d;
            }
        } else {
            // Uniform damping on both J and WV (or no damping if damp === 1.0)
            // Flat stride through the entire array for maximum cache coherence
            for (let k = 0; k < total3; k += 3) {
                J[k]     = (J[k]     + WV[k])     * damp;
                J[k + 1] = (J[k + 1] + WV[k + 1]) * damp;
                J[k + 2] = (J[k + 2] + WV[k + 2]) * damp;
                WV[k]     *= damp;
                WV[k + 1] *= damp;
                WV[k + 2] *= damp;
            }
        }

        // Boundary containment: zero flux & wave velocity outside boundary shape
        // Uses precomputed mask to avoid per-voxel _insideBoundary() calls
        // When reflective boundary is off, flux propagates freely past the shape
        if (this._boundaryMask && this._reflectiveBoundary) {
            for (let idx = 0; idx < total; idx++) {
                if (!this._boundaryMask[idx]) {
                    J[idx * 3] = 0; J[idx * 3 + 1] = 0; J[idx * 3 + 2] = 0;
                    WV[idx * 3] = 0; WV[idx * 3 + 1] = 0; WV[idx * 3 + 2] = 0;
                }
            }
        }

        this._fluxDirty = true;
    }

    /** Discrete divergence of J at (x,y,z): ∇·J = Σ (J_i(v+e_i) - J_i(v-e_i)) / 2 */
    _divergenceAt(x, y, z) {
        const N = this.latticeSize;
        const J = this._fluxJ;
        const idx = (c, xx, yy, zz) => {
            const i = ((zz % N) + N) % N * N * N + ((yy % N) + N) % N * N + ((xx % N) + N) % N;
            return J[i * 3 + c];
        };
        return (idx(0, x + 1, y, z) - idx(0, x - 1, y, z)
              + idx(1, x, y + 1, z) - idx(1, x, y - 1, z)
              + idx(2, x, y, z + 1) - idx(2, x, y, z - 1)) * 0.5;
    }

    _updateFluxMag() {
        if (!this._fluxDirty || !this._fluxJ) return;
        const total = this.latticeSize ** 3;
        const J = this._fluxJ;
        const M = this._fluxMag;
        for (let i = 0, k = 0; i < total; i++, k += 3) {
            const jx = J[k], jy = J[k + 1], jz = J[k + 2];
            M[i] = Math.sqrt(jx * jx + jy * jy + jz * jz);
        }
        this._fluxDirty = false;
    }

    getFluxSlice(axis, index) {
        if (!this._fluxJ) this._initFluxGrid();
        this._updateFluxMag();
        const N = this.latticeSize;
        const needed = N * N;
        // Reuse slice buffer to avoid per-frame Float64Array allocation.
        // Reallocate on size mismatch (not just undersized) to avoid stale
        // data when the lattice shrinks (e.g. L=128 → L=32).
        if (!this._sliceBuf || this._sliceBuf.length !== needed) {
            this._sliceBuf = new Float64Array(needed);
        }
        const data = this._sliceBuf;
        for (let a = 0; a < N; a++) {
            for (let b = 0; b < N; b++) {
                let idx;
                if (axis === 0) idx = this._fluxIdx(index, a, b);
                else if (axis === 1) idx = this._fluxIdx(a, index, b);
                else idx = this._fluxIdx(a, b, index);
                data[a * N + b] = this._fluxMag[idx];
            }
        }
        return data;
    }

    getFluxVolume() {
        if (!this._fluxJ) this._initFluxGrid();
        this._updateFluxMag();
        return this._fluxMag;
    }

    // ── Bulk Sampled Vector Field Exports (Scale 0 field visualization) ──

    getEFieldSampled(stride = 2) {
        if (!this._fluxWV) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = this.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = new Float32Array(maxPts * 3);
        const vectors = new Float32Array(maxPts * 3);
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const idx = this._fluxIdx(x, y, z);
                    // E = -wave_vel
                    const ex = -this._fluxWV[idx * 3];
                    const ey = -this._fluxWV[idx * 3 + 1];
                    const ez = -this._fluxWV[idx * 3 + 2];
                    const mag = Math.sqrt(ex * ex + ey * ey + ez * ez);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x; positions[count * 3 + 1] = y; positions[count * 3 + 2] = z;
                    vectors[count * 3] = ex; vectors[count * 3 + 1] = ey; vectors[count * 3 + 2] = ez;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    getBFieldSampled(stride = 2) {
        if (!this._fluxJ) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = this.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = new Float32Array(maxPts * 3);
        const vectors = new Float32Array(maxPts * 3);
        const J = this._fluxJ;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    // B = curl(J) via 6-point discrete curl
                    const xp = this._fluxIdx(x + 1, y, z), xm = this._fluxIdx(x - 1, y, z);
                    const yp = this._fluxIdx(x, y + 1, z), ym = this._fluxIdx(x, y - 1, z);
                    const zp = this._fluxIdx(x, y, z + 1), zm = this._fluxIdx(x, y, z - 1);
                    const bx = (J[yp * 3 + 2] - J[ym * 3 + 2]) / 2 - (J[zp * 3 + 1] - J[zm * 3 + 1]) / 2;
                    const by = (J[zp * 3] - J[zm * 3]) / 2 - (J[xp * 3 + 2] - J[xm * 3 + 2]) / 2;
                    const bz = (J[xp * 3 + 1] - J[xm * 3 + 1]) / 2 - (J[yp * 3] - J[ym * 3]) / 2;
                    const mag = Math.sqrt(bx * bx + by * by + bz * bz);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x; positions[count * 3 + 1] = y; positions[count * 3 + 2] = z;
                    vectors[count * 3] = bx; vectors[count * 3 + 1] = by; vectors[count * 3 + 2] = bz;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    getPoyntingSampled(stride = 2) {
        if (!this._fluxJ || !this._fluxWV) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = this.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = new Float32Array(maxPts * 3);
        const vectors = new Float32Array(maxPts * 3);
        const J = this._fluxJ, WV = this._fluxWV;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const idx = this._fluxIdx(x, y, z);
                    // E = -wave_vel
                    const ex = -WV[idx * 3], ey = -WV[idx * 3 + 1], ez = -WV[idx * 3 + 2];
                    // B = curl(J)
                    const xp = this._fluxIdx(x + 1, y, z), xm = this._fluxIdx(x - 1, y, z);
                    const yp = this._fluxIdx(x, y + 1, z), ym = this._fluxIdx(x, y - 1, z);
                    const zpp = this._fluxIdx(x, y, z + 1), zm = this._fluxIdx(x, y, z - 1);
                    const bx = (J[yp * 3 + 2] - J[ym * 3 + 2]) / 2 - (J[zpp * 3 + 1] - J[zm * 3 + 1]) / 2;
                    const by = (J[zpp * 3] - J[zm * 3]) / 2 - (J[xp * 3 + 2] - J[xm * 3 + 2]) / 2;
                    const bz = (J[xp * 3 + 1] - J[xm * 3 + 1]) / 2 - (J[yp * 3] - J[ym * 3]) / 2;
                    // S = E × B
                    const sx = ey * bz - ez * by;
                    const sy = ez * bx - ex * bz;
                    const sz = ex * by - ey * bx;
                    const mag = Math.sqrt(sx * sx + sy * sy + sz * sz);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x; positions[count * 3 + 1] = y; positions[count * 3 + 2] = z;
                    vectors[count * 3] = sx; vectors[count * 3 + 1] = sy; vectors[count * 3 + 2] = sz;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    getDivJSampled(stride = 2) {
        if (!this._fluxJ) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const N = this.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = new Float32Array(maxPts * 3);
        const values = new Float32Array(maxPts);
        const J = this._fluxJ;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const xp = this._fluxIdx(x + 1, y, z), xm = this._fluxIdx(x - 1, y, z);
                    const yp = this._fluxIdx(x, y + 1, z), ym = this._fluxIdx(x, y - 1, z);
                    const zp = this._fluxIdx(x, y, z + 1), zm = this._fluxIdx(x, y, z - 1);
                    const div = (J[xp * 3] - J[xm * 3]) / 2 + (J[yp * 3 + 1] - J[ym * 3 + 1]) / 2 + (J[zp * 3 + 2] - J[zm * 3 + 2]) / 2;
                    if (Math.abs(div) < 1e-15) continue;
                    positions[count * 3] = x; positions[count * 3 + 1] = y; positions[count * 3 + 2] = z;
                    values[count] = div;
                    count++;
                }
            }
        }
        return { positions, values, count };
    }

    getFluxVectorSampled(stride = 2) {
        if (!this._fluxJ) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = this.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = new Float32Array(maxPts * 3);
        const vectors = new Float32Array(maxPts * 3);
        const J = this._fluxJ;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const idx = this._fluxIdx(x, y, z);
                    const jx = J[idx * 3], jy = J[idx * 3 + 1], jz = J[idx * 3 + 2];
                    const mag = Math.sqrt(jx * jx + jy * jy + jz * jz);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x; positions[count * 3 + 1] = y; positions[count * 3 + 2] = z;
                    vectors[count * 3] = jx; vectors[count * 3 + 1] = jy; vectors[count * 3 + 2] = jz;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    getForceFieldSampled(stride = 2) {
        // Compute net force (Coulomb + gravity) at sampled grid points from particles
        const ps = this._particles.filter(p => p.state !== 0);
        if (ps.length === 0) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = this.latticeSize;
        const halfN = N / 2;
        const alpha4pi = ALPHA / (4 * Math.PI);
        const gn = this._params.gn;
        const doGravity = this._toggles.gravity;
        const soft = 1.0;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = new Float32Array(maxPts * 3);
        const vectors = new Float32Array(maxPts * 3);
        let count = 0;

        for (let z = 0; z < N; z += stride)
        for (let y = 0; y < N; y += stride)
        for (let x = 0; x < N; x += stride) {
            let fx = 0, fy = 0, fz = 0;
            for (const p of ps) {
                let dx = p.x - x, dy = p.y - y, dz = p.z - z;
                if (dx > halfN) dx -= N; else if (dx < -halfN) dx += N;
                if (dy > halfN) dy -= N; else if (dy < -halfN) dy += N;
                if (dz > halfN) dz -= N; else if (dz < -halfN) dz += N;
                const r2 = dx * dx + dy * dy + dz * dz + soft;
                const r = Math.sqrt(r2);
                const invR2 = 1 / r2;
                const invR = 1 / r;
                // Coulomb: test charge +1 feels force from particle charge
                fx += -alpha4pi * p.state * invR2 * invR * dx;
                fy += -alpha4pi * p.state * invR2 * invR * dy;
                fz += -alpha4pi * p.state * invR2 * invR * dz;
                // Gravity (attractive toward particles)
                if (doGravity) {
                    fx += gn * K_B * K_B * invR2 * invR * dx;
                    fy += gn * K_B * K_B * invR2 * invR * dy;
                    fz += gn * K_B * K_B * invR2 * invR * dz;
                }
            }
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            if (mag < 1e-12) continue;
            positions[count * 3] = x;
            positions[count * 3 + 1] = y;
            positions[count * 3 + 2] = z;
            vectors[count * 3] = fx;
            vectors[count * 3 + 1] = fy;
            vectors[count * 3 + 2] = fz;
            count++;
        }
        return { positions, vectors, count };
    }

    getGravityFieldSampled(stride = 2) {
        // Gravity = G_N * ∇ρ where ρ = |J| (flux magnitude)
        if (!this._fluxJ) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = this.latticeSize;
        const idx = (x, y, z) => {
            const wx = ((x % N) + N) % N, wy = ((y % N) + N) % N, wz = ((z % N) + N) % N;
            return (wz * N * N + wy * N + wx) * 3;
        };
        const density = (x, y, z) => {
            const i = idx(x, y, z);
            const jx = this._fluxJ[i], jy = this._fluxJ[i + 1], jz = this._fluxJ[i + 2];
            return Math.sqrt(jx * jx + jy * jy + jz * jz);
        };

        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = new Float32Array(maxPts * 3);
        const vectors = new Float32Array(maxPts * 3);
        let count = 0;
        const gn = this._params.gn;

        for (let z = 0; z < N; z += stride)
        for (let y = 0; y < N; y += stride)
        for (let x = 0; x < N; x += stride) {
            // Central difference gradient of density
            const gradX = (density(x + 1, y, z) - density(x - 1, y, z)) * 0.5;
            const gradY = (density(x, y + 1, z) - density(x, y - 1, z)) * 0.5;
            const gradZ = (density(x, y, z + 1) - density(x, y, z - 1)) * 0.5;
            const mag = Math.sqrt(gradX * gradX + gradY * gradY + gradZ * gradZ);
            if (mag < 1e-10) continue;
            positions[count * 3] = x;
            positions[count * 3 + 1] = y;
            positions[count * 3 + 2] = z;
            vectors[count * 3] = gn * gradX;
            vectors[count * 3 + 1] = gn * gradY;
            vectors[count * 3 + 2] = gn * gradZ;
            count++;
        }
        return { positions, vectors, count };
    }

    // ── ParticleEngine (Scale 1) Mock ────────────────────────────────
    initPE() {
        this._pe = {
            particles: [], nextId: 0, tick: 0, dt: 1.0, soft: 0.1, coulomb: true, damping: false, gravity: false,
            lorentz: false, exchange: false, strong: false, magnetic_dipole: false,
            spin_orbit: false, radiation: false, relativistic: false
        };
        this._peParticleTypes = new Map();
    }

    resetPE() {
        if (this._pe) {
            this._pe.particles = [];
            this._pe.nextId = 0;
            this._pe.tick = 0;
            this._pe.forces = null;
        }
        if (this._peParticleTypes) this._peParticleTypes.clear();
    }

    peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        if (!this._pe) this.initPE();
        if (mass <= 0) { console.warn('MockBridge: rejecting massless particle:', catalogId); return -1; }
        const id = this._pe.nextId++;
        this._pe.particles.push({
            id, charge, mass, r_eff, x, y, z, vx, vy, vz, locked: false
        });
        this._pe.forces = null; // invalidate force cache
        this._peParticleTypes.set(id, catalogId);
        return id;
    }

    peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff = 0.1) {
        if (!this._pe) this.initPE();
        if (mass <= 0) { console.warn('MockBridge: rejecting massless particle:', catalogId); return -1; }
        const id = this._pe.nextId++;
        this._pe.particles.push({
            id, charge, mass, r_eff, x, y, z, vx: 0, vy: 0, vz: 0, locked: true
        });
        this._pe.forces = null; // invalidate force cache
        this._peParticleTypes.set(id, catalogId);
        return id;
    }

    /**
     * PE force computation: Coulomb + gravity via N(N-1)/2 pair loop.
     *
     * For each unique pair (i,j), computes:
     *   F_coulomb = -alpha * q_i * q_j / (4pi * r^2)  (repulsive for same-sign)
     *   F_gravity =  G_N * m_i * m_j / r^2             (always attractive)
     * Both forces are softened by soft^2 to avoid singularities at r=0.
     * Result is radial: F_vec = (F_c + F_g) * r_hat / r.
     * Newton's 3rd law: force on j is negated from force on i.
     *
     * Uses a flat Float64Array(N*3) laid out as [fx0,fy0,fz0, fx1,fy1,fz1, ...]
     * instead of an object array, avoiding N allocations per call and giving
     * ~2x speedup on the O(N^2) pair loop via cache locality.
     */
    _peComputeForces() {
        const ps = this._pe.particles;
        const n = ps.length;
        // Grow-only typed buffer (avoids reallocation when particle count is stable)
        if (!this._pe.forcesBuf || this._pe.forcesBuf.length < n * 3) {
            this._pe.forcesBuf = new Float64Array(n * 3);
        }
        const F = this._pe.forcesBuf;
        // Zero the active region
        for (let k = 0; k < n * 3; k++) F[k] = 0;

        const soft2 = this._pe.soft * this._pe.soft;
        const doCoulomb = this._pe.coulomb;
        const doGravity = this._pe.gravity;
        const alpha4pi = ALPHA / (4 * Math.PI);
        for (let i = 0; i < n; i++) {
            const pi = ps[i];
            const i3 = i * 3;
            const qi = pi.charge, mi = pi.mass;
            const pix = pi.x, piy = pi.y, piz = pi.z;
            for (let j = i + 1; j < n; j++) {
                const pj = ps[j];
                const dx = pj.x - pix, dy = pj.y - piy, dz = pj.z - piz;
                const r2 = dx * dx + dy * dy + dz * dz + soft2;
                if (r2 < 1e-40) continue;
                const invR = 1 / Math.sqrt(r2);
                const invR2 = invR * invR;
                const fc = doCoulomb ? -alpha4pi * qi * pj.charge * invR2 : 0;
                const fg = doGravity ? G_N * mi * pj.mass * invR2 : 0;
                const fr = (fc + fg) * invR;
                const ffx = fr * dx, ffy = fr * dy, ffz = fr * dz;
                const j3 = j * 3;
                F[i3]     += ffx; F[i3 + 1] += ffy; F[i3 + 2] += ffz;
                F[j3]     -= ffx; F[j3 + 1] -= ffy; F[j3 + 2] -= ffz;
            }
        }
        // Store reference for consumers
        this._pe.forces = F;
        this._pe.forcesN = n;
    }

    // Velocity Verlet integrator: half-kick → drift → recompute forces → half-kick
    peTick() {
        if (!this._pe) return;
        const ps = this._pe.particles;
        const dt = this._pe.dt;

        // Ensure forces are initialized
        if (!this._pe.forces || this._pe.forcesN !== ps.length) {
            this._peComputeForces();
        }

        // Half-kick: v += (F/m) × dt/2   (forces in flat Float64Array)
        const F1 = this._pe.forces;
        for (let i = 0; i < ps.length; i++) {
            const p = ps[i];
            if (p.locked) continue;
            const hdt = dt * 0.5 / p.mass;
            const i3 = i * 3;
            p.vx += F1[i3]     * hdt;
            p.vy += F1[i3 + 1] * hdt;
            p.vz += F1[i3 + 2] * hdt;
        }

        // Drift: r += v × dt
        for (const p of ps) {
            if (p.locked) continue;
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.z += p.vz * dt;
        }

        // Boundary containment (PE mode: origin-centered, radius 35)
        if (this._boundaryShape !== 'cube' && this._boundaryShape !== 'none') {
            for (const p of ps) {
                if (p.locked) continue;
                this._reflectIntoBoundary(p, 0, 0, 0, 35);
            }
        }

        // Recompute forces at new positions
        this._peComputeForces();

        // Half-kick again: v += (F/m) × dt/2
        const F2 = this._pe.forces;
        for (let i = 0; i < ps.length; i++) {
            const p = ps[i];
            if (p.locked) continue;
            const hdt = dt * 0.5 / p.mass;
            const i3 = i * 3;
            p.vx += F2[i3]     * hdt;
            p.vy += F2[i3 + 1] * hdt;
            p.vz += F2[i3 + 2] * hdt;
        }

        // Damping (intentional energy dissipation, applied after Verlet)
        if (this._pe.damping) {
            const d = Math.max(0, 1 - DAMPING * dt);
            for (const p of ps) {
                if (p.locked) continue;
                p.vx *= d; p.vy *= d; p.vz *= d;
            }
        }

        // Speed limit
        for (const p of ps) {
            if (p.locked) continue;
            const speed = Math.sqrt(p.vx * p.vx + p.vy * p.vy + p.vz * p.vz);
            if (speed > C_SPEED) {
                const s = C_SPEED / speed;
                p.vx *= s; p.vy *= s; p.vz *= s;
            }
        }

        // Annihilation: opposite-charge particles closer than contact distance
        const toRemove = new Set();
        for (let i = 0; i < ps.length; i++) {
            if (toRemove.has(i)) continue;
            for (let j = i + 1; j < ps.length; j++) {
                if (toRemove.has(j)) continue;
                if (ps[i].charge * ps[j].charge >= 0) continue;
                const dx = ps[j].x - ps[i].x, dy = ps[j].y - ps[i].y, dz = ps[j].z - ps[i].z;
                const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
                if (dist < ps[i].r_eff + ps[j].r_eff) {
                    toRemove.add(i);
                    toRemove.add(j);
                    break;
                }
            }
        }
        if (toRemove.size > 0) {
            this._pe.particles = ps.filter((_, idx) => !toRemove.has(idx));
            this._pe.forces = null;
        }

        this._pe.tick++;
    }

    peGetParticleData() {
        if (!this._pe) return { positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), charges: new Int8Array(0), ids: new Int32Array(0), velocities: new Float32Array(0), count: 0 };
        const ps = this._pe.particles;
        const count = ps.length;
        // Reuse pre-allocated buffers to avoid GC pressure (grow only when needed)
        if (!this._peBufs || this._peBufs.cap < count) {
            this._peBufs = {
                positions: new Float32Array(count * 3),
                colors: new Float32Array(count * 3),
                sizes: new Float32Array(count),
                charges: new Int8Array(count),
                ids: new Int32Array(count),
                velocities: new Float32Array(count * 3),
                cap: count
            };
        }
        const { positions, colors, sizes, charges, ids, velocities } = this._peBufs;
        for (let i = 0; i < count; i++) {
            const p = ps[i];
            positions[i * 3] = p.x;
            positions[i * 3 + 1] = p.y;
            positions[i * 3 + 2] = p.z;
            velocities[i * 3] = p.vx;
            velocities[i * 3 + 1] = p.vy;
            velocities[i * 3 + 2] = p.vz;
            if (p.charge > 0) { colors[i * 3] = 0.29; colors[i * 3 + 1] = 0.87; colors[i * 3 + 2] = 0.50; }
            else if (p.charge < 0) { colors[i * 3] = 0.97; colors[i * 3 + 1] = 0.44; colors[i * 3 + 2] = 0.44; }
            else { colors[i * 3] = 0.60; colors[i * 3 + 1] = 0.60; colors[i * 3 + 2] = 0.70; }
            sizes[i] = 6.0 + 4.0 * Math.log10(p.mass / K_B + 1.0);
            if (sizes[i] > 60) sizes[i] = 60;
            charges[i] = p.charge;
            ids[i] = p.id;
        }
        return { positions, colors, sizes, charges, ids, velocities, count };
    }

    peGetFieldSources() {
        if (!this._pe) return { positions: new Float32Array(0), charges: new Float32Array(0), masses: new Float32Array(0), count: 0 };
        const ps = this._pe.particles;
        const n = ps.length;
        // Reuse buffers (grow-only) to avoid per-frame allocation
        if (!this._peFieldBufs || this._peFieldBufs.cap < n) {
            this._peFieldBufs = {
                positions: new Float32Array(n * 3),
                charges: new Float32Array(n),
                masses: new Float32Array(n),
                cap: n
            };
        }
        const { positions, charges, masses } = this._peFieldBufs;
        for (let i = 0; i < n; i++) {
            const i3 = i * 3;
            positions[i3] = ps[i].x;
            positions[i3 + 1] = ps[i].y;
            positions[i3 + 2] = ps[i].z;
            charges[i] = ps[i].charge;
            masses[i] = ps[i].mass;
        }
        return { positions, charges, masses, count: n };
    }

    peGetForces() {
        if (!this._pe || !this._pe.forces) return { positions: new Float32Array(0), forces: new Float32Array(0), count: 0, maxForce: 0 };
        const ps = this._pe.particles;
        const F = this._pe.forces;  // flat Float64Array [fx0,fy0,fz0, fx1,fy1,fz1, ...]
        const n = ps.length;
        const positions = new Float32Array(n * 3);
        const forces = new Float32Array(n * 3);
        let maxF = 0;
        for (let i = 0; i < n; i++) {
            const i3 = i * 3;
            positions[i3] = ps[i].x;
            positions[i3 + 1] = ps[i].y;
            positions[i3 + 2] = ps[i].z;
            const fx = F[i3], fy = F[i3 + 1], fz = F[i3 + 2];
            forces[i3] = fx;
            forces[i3 + 1] = fy;
            forces[i3 + 2] = fz;
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            if (mag > maxF) maxF = mag;
        }
        return { positions, forces, count: n, maxForce: maxF };
    }

    peGetDiagnostics() {
        if (!this._pe) return { tick: 0, particleCount: 0, totalKE: 0, totalPE: 0, coulombPE: 0, gravityPE: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0, angMomX: 0, angMomY: 0, angMomZ: 0 };
        const ps = this._pe.particles;
        let ke = 0, pe_coulomb = 0, pe_gravity = 0, px = 0, py = 0, pz = 0;
        let lx = 0, ly = 0, lz = 0;
        const soft2 = this._pe.soft * this._pe.soft;
        for (const p of ps) {
            const v2 = p.vx * p.vx + p.vy * p.vy + p.vz * p.vz;
            ke += 0.5 * p.mass * v2;
            px += p.mass * p.vx; py += p.mass * p.vy; pz += p.mass * p.vz;
            const mvx = p.mass * p.vx, mvy = p.mass * p.vy, mvz = p.mass * p.vz;
            lx += p.y * mvz - p.z * mvy;
            ly += p.z * mvx - p.x * mvz;
            lz += p.x * mvy - p.y * mvx;
        }
        for (let i = 0; i < ps.length; i++) {
            for (let j = i + 1; j < ps.length; j++) {
                const dx = ps[j].x - ps[i].x, dy = ps[j].y - ps[i].y, dz = ps[j].z - ps[i].z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                if (this._pe.coulomb) pe_coulomb += ALPHA * ps[i].charge * ps[j].charge / (4 * Math.PI * r);
                if (this._pe.gravity) {
                    pe_gravity -= G_N * ps[i].mass * ps[j].mass / r;
                }
            }
        }
        const pe_val = pe_coulomb + pe_gravity;
        return { tick: this._pe.tick, particleCount: ps.length, totalKE: ke, totalPE: pe_val, coulombPE: pe_coulomb, gravityPE: pe_gravity, totalEnergy: ke + pe_val, momentumX: px, momentumY: py, momentumZ: pz, angMomX: lx, angMomY: ly, angMomZ: lz };
    }

    peGetExtendedData() {
        if (!this._pe) return null;
        const ps = this._pe.particles;
        const N = ps.length;
        if (N === 0) return null;
        const ids = new Int32Array(N);
        const charges = new Int8Array(N);
        const masses = new Float64Array(N);
        const positions = new Float64Array(N * 3);
        const velocities = new Float64Array(N * 3);
        const locked = new Uint8Array(N);
        const forces = new Float64Array(N * 3);
        const accelerations = new Float64Array(N * 3);
        const soft2 = (this._pe.soft || 0.1) ** 2;
        for (let i = 0; i < N; i++) {
            const p = ps[i];
            ids[i] = p.id;
            charges[i] = p.charge;
            masses[i] = p.mass;
            positions[i * 3] = p.x; positions[i * 3 + 1] = p.y; positions[i * 3 + 2] = p.z;
            velocities[i * 3] = p.vx; velocities[i * 3 + 1] = p.vy; velocities[i * 3 + 2] = p.vz;
            locked[i] = p.locked ? 1 : 0;
            let fx = 0, fy = 0, fz = 0;
            for (let j = 0; j < N; j++) {
                if (j === i) continue;
                const q = ps[j];
                const dx = q.x - p.x, dy = q.y - p.y, dz = q.z - p.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                const r2s = r2 + soft2;
                const r = Math.sqrt(r2s);
                const fc = this._pe.coulomb ? -ALPHA * p.charge * q.charge / (4 * Math.PI * r2s) : 0;
                const fg = this._pe.gravity ? G_N * p.mass * q.mass / r2s : 0;
                if (r > 1e-20) {
                    const fr = (fc + fg) / r;
                    fx += fr * dx; fy += fr * dy; fz += fr * dz;
                }
            }
            forces[i * 3] = fx; forces[i * 3 + 1] = fy; forces[i * 3 + 2] = fz;
            const m = p.mass || 1e-30;
            accelerations[i * 3] = fx / m; accelerations[i * 3 + 1] = fy / m; accelerations[i * 3 + 2] = fz / m;
        }
        return { count: N, ids, charges, masses, positions, velocities, forces, accelerations, locked };
    }

    peSetDt(dt) { if (this._pe) this._pe.dt = dt; }
    peGetDt() { return this._pe ? this._pe.dt : 1.0; }
    peSetSoftening(s) { if (this._pe) this._pe.soft = s; }
    peSetCoulomb(e) { if (this._pe) this._pe.coulomb = e; }
    peSetDamping(e) { if (this._pe) this._pe.damping = e; }
    peSetGravity(e) { if (this._pe) this._pe.gravity = e; }
    peSetLorentz(e) { if (this._pe) this._pe.lorentz = e; }
    peSetExchange(e) { if (this._pe) this._pe.exchange = e; }
    peSetStrong(e) { if (this._pe) this._pe.strong = e; }
    peSetMagneticDipole(e) { if (this._pe) this._pe.magnetic_dipole = e; }
    peSetSpinOrbit(e) { if (this._pe) this._pe.spin_orbit = e; }
    peSetRadiation(e) { if (this._pe) this._pe.radiation = e; }
    peSetRelativistic(e) { if (this._pe) this._pe.relativistic = e; }
    peParticleCount() { return this._pe ? this._pe.particles.length : 0; }
    peClear() { this.resetPE(); }
    peGetParticleTypes() { return this._peParticleTypes || new Map(); }

    peInspectParticle(id) {
        if (!this._pe) return null;
        const p = this._pe.particles.find(q => q.id === id);
        if (!p) return null;
        const speed = Math.sqrt(p.vx * p.vx + p.vy * p.vy + p.vz * p.vz);
        const ke = 0.5 * p.mass * speed * speed;

        // Find nearest particle and compute net force
        let nearestId = -1, nearestDist = Infinity;
        let fNetX = 0, fNetY = 0, fNetZ = 0;
        let fCoulombNearest = 0;
        const soft2 = (this._pe.soft || 0.1) ** 2;

        for (const q of this._pe.particles) {
            if (q.id === p.id) continue;
            const dx = q.x - p.x, dy = q.y - p.y, dz = q.z - p.z;
            const r2 = dx * dx + dy * dy + dz * dz;
            const r = Math.sqrt(r2);
            if (r < nearestDist) { nearestDist = r; nearestId = q.id; }
            // Coulomb + gravity forces (matching peTick force law)
            const r2s = r2 + soft2;
            const fc = this._pe.coulomb ? -ALPHA * p.charge * q.charge / (4 * Math.PI * r2s) : 0;
            const fg = this._pe.gravity ? G_N * p.mass * q.mass / r2s : 0;
            if (r > 1e-20) {
                const fr = (fc + fg) / r;
                fNetX += fr * dx;
                fNetY += fr * dy;
                fNetZ += fr * dz;
            }
        }

        // Coulomb force to nearest specifically
        if (nearestId >= 0) {
            const nq = this._pe.particles.find(q => q.id === nearestId);
            if (nq) {
                const dx = nq.x - p.x, dy = nq.y - p.y, dz = nq.z - p.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                fCoulombNearest = Math.abs(ALPHA * p.charge * nq.charge / (4 * Math.PI * (r2 + soft2)));
            }
        }

        // Orbital radius: distance to nearest opposite-charge particle
        let orbitalR = -1;
        for (const q of this._pe.particles) {
            if (q.id === p.id) continue;
            if (p.charge !== 0 && q.charge !== 0 && Math.sign(p.charge) !== Math.sign(q.charge)) {
                const dx = q.x - p.x, dy = q.y - p.y, dz = q.z - p.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                if (orbitalR < 0 || r < orbitalR) orbitalR = r;
            }
        }

        return {
            id: p.id, charge: p.charge, mass: p.mass,
            x: p.x, y: p.y, z: p.z,
            vx: p.vx, vy: p.vy, vz: p.vz,
            speed, ke, locked: p.locked,
            nearestId, nearestDist,
            orbitalR,
            fCoulombNearest,
            fNetMag: Math.sqrt(fNetX * fNetX + fNetY * fNetY + fNetZ * fNetZ),
        };
    }

    // ── AtomEngine (Scale 2) Mock ─────────────────────────────────────
    initAE() {
        this._ae = {
            atoms: [], bonds: [], nextId: 0, tick: 0,
            dt: 0.1,       // Larger dt for visible dynamics in sim units
            soft: 0.3,     // Softening in Bohr radii
            damping: false, bonding: true,
            ionic: true,    // Ionic (Coulomb) force toggle
            vdw: true,      // Van der Waals (LJ 12-6) force toggle
            bonds_force: true, // Covalent bond spring force toggle
            speed_limit: true, // Speed cap toggle
            // Phase 3 forces (all off by default)
            h_bonds: false,           // H-bond LJ 10-12 + angular
            angle_strain: false,      // VSEPR angle restoring force
            dipole_dipole: false,     // Dipole-dipole 1/r^5
            thermostat: false,        // Berendsen velocity rescaling
            thermostat_temp: 1.0,     // Target temperature (sim units)
            electronegativity: false, // Polar bond formation threshold
        };
    }

    resetAE() {
        if (this._ae) {
            this._ae.atoms = [];
            this._ae.bonds = [];
            this._ae.nextId = 0;
            this._ae.tick = 0;
        }
    }

    // ── Valence Electrons (for VSEPR lone-pair geometry) ─────────────
    // Returns the main-group valence electron count for VSEPR.
    // Transition metals default to max_bonds as fallback.

    aeAddAtom(Z, x, y, z, vx = 0, vy = 0, vz = 0, charge = 0, N = -1) {
        if (!this._ae) this.initAE();
        const neutrons = N >= 0 ? N : elemNeutrons(Z);
        const props = computeAtomicProps(Z, neutrons);
        const id = this._ae.nextId++;
        this._ae.atoms.push({
            id, Z, N: neutrons, charge, mass: props.mass, radius: props.radius,
            vdw_epsilon: props.vdw_epsilon, vdw_sigma: props.vdw_sigma,
            max_bonds: props.max_bonds, bonds: [],
            electronegativity: props.electronegativity,
            valence_electrons: _valenceElectrons(Z),
            dipole_x: 0, dipole_y: 0, dipole_z: 0,
            x, y, z, vx, vy, vz, ax: 0, ay: 0, az: 0, locked: false
        });
        return id;
    }

    aeAddLockedAtom(Z, x, y, z, charge = 0, N = -1) {
        const id = this.aeAddAtom(Z, x, y, z, 0, 0, 0, charge, N);
        if (this._ae && id >= 0) {
            this._ae.atoms[this._ae.atoms.length - 1].locked = true;
        }
        return id;
    }

    aeCreateBond(idA, idB, order = 1) {
        if (!this._ae) return;
        const a = this._ae.atoms.find(at => at.id === idA);
        const b = this._ae.atoms.find(at => at.id === idB);
        if (!a || !b) return;
        const sig_avg = (a.vdw_sigma + b.vdw_sigma) / 2;
        const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0) / order;
        const eps_mix = Math.sqrt(a.vdw_epsilon * b.vdw_epsilon);
        const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
        a.bonds.push({ partner_id: idB, r_eq, k_bond, order });
        b.bonds.push({ partner_id: idA, r_eq, k_bond, order });
    }

    // Compute force on atom i from all others (ionic + vdW + bonds)
    /**
     * Build bond lookup structures for O(1) bond checks and partner lookups.
     * Called once per force evaluation to avoid O(bonds) scans in the inner loop.
     *
     * CAUTION: The bond key formula `lo * 100000 + hi` assumes atom IDs < 100000.
     * Since _ae.nextId increments monotonically and typical simulations have
     * < 1000 atoms, this is safe. If atom IDs ever exceed 100000, collisions
     * would cause false bond-pair matches. Use string keys as fallback if needed.
     */
    _aeBuildBondLookup() {
        const atoms = this._ae.atoms;
        // bondSet: numeric keys for O(1) isBonded check
        const bondSet = new Set();
        // idToIdx: atom.id → array index for O(1) partner lookup
        const idToIdx = new Map();
        // neighborSets: atom index → Set of bonded partner IDs (for 1-3 exclusion)
        const neighborSets = new Array(atoms.length);

        for (let i = 0; i < atoms.length; i++) {
            idToIdx.set(atoms[i].id, i);
            const ns = new Set();
            for (const b of atoms[i].bonds) {
                const lo = Math.min(atoms[i].id, b.partner_id);
                const hi = Math.max(atoms[i].id, b.partner_id);
                bondSet.add(lo * 100000 + hi); // numeric key (faster than string)
                ns.add(b.partner_id);
            }
            neighborSets[i] = ns;
        }
        this._aeBondSet = bondSet;
        this._aeIdToIdx = idToIdx;
        this._aeNeighborSets = neighborSets;
    }

    _aeIsBonded(id_a, id_b) {
        const lo = Math.min(id_a, id_b), hi = Math.max(id_a, id_b);
        return this._aeBondSet.has(lo * 100000 + hi);
    }

    _aeIs13(i, j) {
        // 1-3 exclusion: atoms i and j share a bonded partner
        const nsI = this._aeNeighborSets[i];
        const nsJ = this._aeNeighborSets[j];
        for (const pid of nsI) {
            if (nsJ.has(pid)) return true;
        }
        return false;
    }

    _aeComputeDipoleMoments() {
        const atoms = this._ae.atoms;
        for (const a of atoms) {
            a.dipole_x = 0; a.dipole_y = 0; a.dipole_z = 0;
            for (const bond of a.bonds) {
                const jIdx = this._aeIdToIdx.get(bond.partner_id);
                if (jIdx === undefined) continue;
                const aj = atoms[jIdx];
                const chi_diff = aj.electronegativity - a.electronegativity;
                if (Math.abs(chi_diff) < 1e-10) continue;
                a.dipole_x += (aj.x - a.x) * chi_diff;
                a.dipole_y += (aj.y - a.y) * chi_diff;
                a.dipole_z += (aj.z - a.z) * chi_diff;
            }
        }
    }

    _aeComputeForce(i) {
        const atoms = this._ae.atoms;
        const ai = atoms[i];
        let fx = 0, fy = 0, fz = 0;
        const soft2 = this._ae.soft * this._ae.soft;

        for (let j = 0; j < atoms.length; j++) {
            if (j === i) continue;
            const aj = atoms[j];
            const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
            const r2 = dx * dx + dy * dy + dz * dz + soft2;
            const r = Math.sqrt(r2);
            if (r < 1e-20) continue;
            const rx = dx / r, ry = dy / r, rz = dz / r;

            // 1-2 exclusion: bonded pairs use spring instead of LJ (O(1) lookup)
            const isBonded = this._aeIsBonded(ai.id, aj.id);

            // 1-3 exclusion: atoms sharing a bonded partner (O(bonds) via Set)
            const is13 = !isBonded && this._aeIs13(i, j);

            // Ionic (Coulomb) — skip for bonded and 1-3 pairs
            if (this._ae.ionic && !isBonded && !is13 && ai.charge !== 0 && aj.charge !== 0) {
                const f_ionic = -AE_K_COULOMB * ai.charge * aj.charge / r2;
                fx += f_ionic * rx; fy += f_ionic * ry; fz += f_ionic * rz;
            }

            // Van der Waals (LJ 12-6) — skip for bonded and 1-3 pairs
            if (this._ae.vdw && !isBonded && !is13) {
                const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                const sig_mix = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                const sr = sig_mix / r;
                const sr6 = sr * sr * sr * sr * sr * sr; // sr**6 inlined
                const sr12 = sr6 * sr6;
                const f_vdw = -24.0 * eps_mix * (2.0 * sr12 - sr6) / r;
                fx += f_vdw * rx; fy += f_vdw * ry; fz += f_vdw * rz;
            }

            // H-bonds: LJ 10-12 + cos²(θ_DHA) angular factor
            // Fires between H bonded to electronegative donor and electronegative acceptor
            if (this._ae.h_bonds) {
                const isElecNeg = (Z) => Z === 7 || Z === 8 || Z === 9;
                // Helper: compute H-bond force contribution
                const hbondForce = (hAtom, acceptor, hIdx, aIdx) => {
                    // Find electronegative donor bonded to H
                    let donorIdx = -1;
                    for (const b of hAtom.bonds) {
                        const didx = this._aeIdToIdx.get(b.partner_id);
                        if (didx !== undefined && isElecNeg(atoms[didx].Z)) { donorIdx = didx; break; }
                    }
                    if (donorIdx < 0 || donorIdx === aIdx) return;
                    const sig_hb = (hAtom.vdw_sigma + acceptor.vdw_sigma) / 2;
                    if (sig_hb <= 0 || r < 1e-10) return;
                    const shr = sig_hb / r;
                    const shr10 = Math.pow(shr, 10);
                    const shr12 = shr10 * shr * shr;
                    const f_rad = AE_H_BOND_EPS * 60.0 * (shr12 - shr10) / r;
                    // Angular: cos²(θ_DHA) where D=donor, H=hAtom, A=acceptor
                    const donor = atoms[donorIdx];
                    const dhx = hAtom.x - donor.x, dhy = hAtom.y - donor.y, dhz = hAtom.z - donor.z;
                    const hax = acceptor.x - hAtom.x, hay = acceptor.y - hAtom.y, haz = acceptor.z - hAtom.z;
                    const dh_mag = Math.sqrt(dhx*dhx + dhy*dhy + dhz*dhz);
                    const ha_mag = Math.sqrt(hax*hax + hay*hay + haz*haz);
                    let cos_theta = 1.0;
                    if (dh_mag > 1e-30 && ha_mag > 1e-30)
                        cos_theta = (dhx*hax + dhy*hay + dhz*haz) / (dh_mag * ha_mag);
                    const ang = cos_theta * cos_theta;
                    fx += f_rad * ang * rx; fy += f_rad * ang * ry; fz += f_rad * ang * rz;
                };
                // Case 1: ai is H, aj is electronegative acceptor
                if (ai.Z === 1 && isElecNeg(aj.Z)) hbondForce(ai, aj, i, j);
                // Case 2: aj is H, ai is electronegative acceptor
                if (aj.Z === 1 && isElecNeg(ai.Z)) hbondForce(aj, ai, j, i);
            }

            // Dipole-dipole: 1/r^5 interaction between pre-computed molecular dipoles
            if (this._ae.dipole_dipole) {
                const mi_x = ai.dipole_x, mi_y = ai.dipole_y, mi_z = ai.dipole_z;
                const mj_x = aj.dipole_x, mj_y = aj.dipole_y, mj_z = aj.dipole_z;
                const mi_mag2 = mi_x*mi_x + mi_y*mi_y + mi_z*mi_z;
                const mj_mag2 = mj_x*mj_x + mj_y*mj_y + mj_z*mj_z;
                if (mi_mag2 > 1e-60 && mj_mag2 > 1e-60 && r > 1e-10) {
                    const mi_dot_r = mi_x*rx + mi_y*ry + mi_z*rz;
                    const mj_dot_r = mj_x*rx + mj_y*ry + mj_z*rz;
                    const mi_dot_mj = mi_x*mj_x + mi_y*mj_y + mi_z*mj_z;
                    const coeff = 3.0 * AE_K_COULOMB / (r2 * r2 * r);  // 1/r^5 scaled
                    const t1 = 5.0 * mi_dot_r * mj_dot_r / r2;
                    fx += coeff * (t1*rx - mj_x*mi_dot_r - mi_x*mj_dot_r - rx*mi_dot_mj);
                    fy += coeff * (t1*ry - mj_y*mi_dot_r - mi_y*mj_dot_r - ry*mi_dot_mj);
                    fz += coeff * (t1*rz - mj_z*mi_dot_r - mi_z*mj_dot_r - rz*mi_dot_mj);
                }
            }
        }

        // Bond spring forces (O(1) partner lookup via Map)
        if (this._ae.bonds_force) {
            for (const bond of ai.bonds) {
                const jIdx = this._aeIdToIdx.get(bond.partner_id);
                const aj = jIdx !== undefined ? atoms[jIdx] : null;
                if (!aj) continue;
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                if (r < 1e-20) continue;
                const rx = dx / r, ry = dy / r, rz = dz / r;
                const dr = r - bond.r_eq;
                const f_bond = bond.k_bond * dr;
                fx += f_bond * rx; fy += f_bond * ry; fz += f_bond * rz;
            }
        }

        // Angle strain / VSEPR (3-body): restoring force toward equilibrium angles
        // Force on central atom i only; terminal atoms get Newton's 3rd in _aeComputeAllForces
        if (this._ae.angle_strain && ai.bonds.length >= 2) {
            for (let b1 = 0; b1 < ai.bonds.length; b1++) {
                for (let b2 = b1 + 1; b2 < ai.bonds.length; b2++) {
                    const j1 = this._aeIdToIdx.get(ai.bonds[b1].partner_id);
                    const j2 = this._aeIdToIdx.get(ai.bonds[b2].partner_id);
                    if (j1 === undefined || j2 === undefined) continue;
                    const a1 = atoms[j1], a2 = atoms[j2];
                    const r1x = a1.x - ai.x, r1y = a1.y - ai.y, r1z = a1.z - ai.z;
                    const r2x = a2.x - ai.x, r2y = a2.y - ai.y, r2z = a2.z - ai.z;
                    const m1 = Math.sqrt(r1x*r1x + r1y*r1y + r1z*r1z);
                    const m2 = Math.sqrt(r2x*r2x + r2y*r2y + r2z*r2z);
                    if (m1 < 1e-30 || m2 < 1e-30) continue;

                    let cos_t = (r1x*r2x + r1y*r2y + r1z*r2z) / (m1 * m2);
                    cos_t = Math.max(-1, Math.min(1, cos_t));
                    const theta = Math.acos(cos_t);

                    // VSEPR equilibrium angle from steric number
                    const nbonds = ai.bonds.length;
                    const lone_pairs = Math.max(0, Math.floor((ai.valence_electrons - nbonds) / 2));
                    const steric = nbonds + lone_pairs;
                    let theta_eq;
                    switch (steric) {
                        case 2: theta_eq = Math.PI; break;              // linear
                        case 3: theta_eq = 2 * Math.PI / 3; break;     // trigonal planar
                        case 4:
                            if (lone_pairs === 0) theta_eq = Math.acos(-1/3);        // 109.47° tetrahedral
                            else if (lone_pairs === 1) theta_eq = 107 * Math.PI / 180; // pyramidal
                            else theta_eq = 104.5 * Math.PI / 180;                    // bent
                            break;
                        default: theta_eq = Math.acos(-1/3); break;
                    }

                    const sin_t = Math.sin(theta);
                    if (Math.abs(sin_t) < 1e-15) continue;
                    const dV = AE_K_ANGLE * (theta - theta_eq);

                    // Perpendicular directions for force projection
                    const r1hx = r1x/m1, r1hy = r1y/m1, r1hz = r1z/m1;
                    const r2hx = r2x/m2, r2hy = r2y/m2, r2hz = r2z/m2;
                    let p1x = r2hx - cos_t*r1hx, p1y = r2hy - cos_t*r1hy, p1z = r2hz - cos_t*r1hz;
                    const pm1 = Math.sqrt(p1x*p1x + p1y*p1y + p1z*p1z);
                    if (pm1 < 1e-30) continue;
                    p1x /= pm1; p1y /= pm1; p1z /= pm1;
                    let p2x = r1hx - cos_t*r2hx, p2y = r1hy - cos_t*r2hy, p2z = r1hz - cos_t*r2hz;
                    const pm2 = Math.sqrt(p2x*p2x + p2y*p2y + p2z*p2z);
                    if (pm2 < 1e-30) continue;
                    p2x /= pm2; p2y /= pm2; p2z /= pm2;

                    const fj1 = dV / (m1 * sin_t);
                    const fj2 = dV / (m2 * sin_t);
                    // Force on central atom = -(f_j1 + f_j2)
                    fx -= fj1 * p1x + fj2 * p2x;
                    fy -= fj1 * p1y + fj2 * p2y;
                    fz -= fj1 * p1z + fj2 * p2z;
                }
            }
        }

        // Safety clamp: cap force magnitude to prevent residual explosions
        const fmag2 = fx * fx + fy * fy + fz * fz;
        const F_MAX = 50.0;
        if (fmag2 > F_MAX * F_MAX) {
            const scale = F_MAX / Math.sqrt(fmag2);
            fx *= scale; fy *= scale; fz *= scale;
        }

        return { fx, fy, fz };
    }

    /**
     * Run auto-bonding logic without physics integration.
     * Call after loading a molecule to establish bonds before the first tick.
     * This prevents explosive LJ forces when atoms are placed inside each
     * other's LJ walls (which is normal for covalent bond distances).
     */
    aePreBond() {
        if (!this._ae || !this._ae.bonding) {
            debugLog('[FTD aePreBond] skipped — ae:', !!this._ae, 'bonding:', this._ae?.bonding);
            return;
        }
        const atoms = this._ae.atoms;
        let bondsCreated = 0;
        for (let i = 0; i < atoms.length; i++) {
            for (let j = i + 1; j < atoms.length; j++) {
                const ai = atoms[i], aj = atoms[j];
                if (ai.bonds.some(b => b.partner_id === aj.id)) continue;
                if (ai.bonds.length >= ai.max_bonds || aj.bonds.length >= aj.max_bonds) continue;
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const sig_avg = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                if (r < 1.2 * sig_avg) {
                    const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0);
                    const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                    const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
                    ai.bonds.push({ partner_id: aj.id, r_eq, k_bond, order: 1 });
                    aj.bonds.push({ partner_id: ai.id, r_eq, k_bond, order: 1 });
                    bondsCreated++;
                }
            }
        }
        debugLog(`[FTD aePreBond] ${atoms.length} atoms, ${bondsCreated} bonds created`);
        for (const a of atoms) {
            debugLog(`  atom ${a.id} Z=${a.Z} pos=(${a.x.toFixed(2)},${a.y.toFixed(2)},${a.z.toFixed(2)}) bonds=${a.bonds.length}/${a.max_bonds} sigma=${a.vdw_sigma.toFixed(2)}`);
        }
    }

    _aeComputeAllForces() {
        const atoms = this._ae.atoms;
        this._aeBuildBondLookup(); // build O(1) bond lookups before force loop

        // Compute dipole moments before force evaluation
        if (this._ae.dipole_dipole) this._aeComputeDipoleMoments();

        const forces = new Array(atoms.length);
        for (let i = 0; i < atoms.length; i++) {
            forces[i] = this._aeComputeForce(i);
        }

        // Angle strain: distribute Newton's-3rd-law forces to terminal atoms
        if (this._ae.angle_strain) {
            for (let i = 0; i < atoms.length; i++) {
                const ai = atoms[i];
                if (ai.bonds.length < 2) continue;
                for (let b1 = 0; b1 < ai.bonds.length; b1++) {
                    for (let b2 = b1 + 1; b2 < ai.bonds.length; b2++) {
                        const j1 = this._aeIdToIdx.get(ai.bonds[b1].partner_id);
                        const j2 = this._aeIdToIdx.get(ai.bonds[b2].partner_id);
                        if (j1 === undefined || j2 === undefined) continue;
                        const a1 = atoms[j1], a2 = atoms[j2];
                        const r1x = a1.x-ai.x, r1y = a1.y-ai.y, r1z = a1.z-ai.z;
                        const r2x = a2.x-ai.x, r2y = a2.y-ai.y, r2z = a2.z-ai.z;
                        const m1 = Math.sqrt(r1x*r1x+r1y*r1y+r1z*r1z);
                        const m2 = Math.sqrt(r2x*r2x+r2y*r2y+r2z*r2z);
                        if (m1 < 1e-30 || m2 < 1e-30) continue;
                        let cos_t = (r1x*r2x+r1y*r2y+r1z*r2z)/(m1*m2);
                        cos_t = Math.max(-1, Math.min(1, cos_t));
                        const theta = Math.acos(cos_t);
                        const nbonds = ai.bonds.length;
                        const lone_pairs = Math.max(0, Math.floor((ai.valence_electrons - nbonds) / 2));
                        const steric = nbonds + lone_pairs;
                        let theta_eq;
                        switch (steric) {
                            case 2: theta_eq = Math.PI; break;
                            case 3: theta_eq = 2*Math.PI/3; break;
                            case 4:
                                if (lone_pairs===0) theta_eq = Math.acos(-1/3);
                                else if (lone_pairs===1) theta_eq = 107*Math.PI/180;
                                else theta_eq = 104.5*Math.PI/180;
                                break;
                            default: theta_eq = Math.acos(-1/3); break;
                        }
                        const sin_t = Math.sin(theta);
                        if (Math.abs(sin_t) < 1e-15) continue;
                        const dV = AE_K_ANGLE * (theta - theta_eq);
                        const r1hx=r1x/m1, r1hy=r1y/m1, r1hz=r1z/m1;
                        const r2hx=r2x/m2, r2hy=r2y/m2, r2hz=r2z/m2;
                        let p1x=r2hx-cos_t*r1hx, p1y=r2hy-cos_t*r1hy, p1z=r2hz-cos_t*r1hz;
                        const pm1=Math.sqrt(p1x*p1x+p1y*p1y+p1z*p1z);
                        if (pm1<1e-30) continue;
                        p1x/=pm1; p1y/=pm1; p1z/=pm1;
                        let p2x=r1hx-cos_t*r2hx, p2y=r1hy-cos_t*r2hy, p2z=r1hz-cos_t*r2hz;
                        const pm2=Math.sqrt(p2x*p2x+p2y*p2y+p2z*p2z);
                        if (pm2<1e-30) continue;
                        p2x/=pm2; p2y/=pm2; p2z/=pm2;
                        const fj1 = dV/(m1*sin_t), fj2 = dV/(m2*sin_t);
                        forces[j1].fx += fj1*p1x; forces[j1].fy += fj1*p1y; forces[j1].fz += fj1*p1z;
                        forces[j2].fx += fj2*p2x; forces[j2].fy += fj2*p2y; forces[j2].fz += fj2*p2z;
                    }
                }
            }
        }

        return forces;
    }

    aeTick() {
        if (!this._ae) return;
        const atoms = this._ae.atoms;
        const dt = this._ae.dt;
        const tickNum = this._ae.tick;

        // Velocity Verlet: compute forces → half-kick → drift → recompute → half-kick
        let forces = this._aeComputeAllForces();

        // Debug: log first 3 ticks
        if (tickNum < 3) {
            debugLog(`[FTD aeTick #${tickNum}] dt=${dt} atoms=${atoms.length}`);
            for (let i = 0; i < Math.min(atoms.length, 4); i++) {
                const a = atoms[i], f = forces[i];
                debugLog(`  atom ${a.id}: pos=(${a.x.toFixed(3)},${a.y.toFixed(3)},${a.z.toFixed(3)}) vel=(${a.vx.toFixed(4)},${a.vy.toFixed(4)},${a.vz.toFixed(4)}) force=(${f.fx.toFixed(4)},${f.fy.toFixed(4)},${f.fz.toFixed(4)}) bonds=${a.bonds.length}`);
            }
        }

        // Half-kick
        for (let i = 0; i < atoms.length; i++) {
            const a = atoms[i];
            if (a.locked) continue;
            const hdt = dt * 0.5 / a.mass;
            a.vx += forces[i].fx * hdt;
            a.vy += forces[i].fy * hdt;
            a.vz += forces[i].fz * hdt;
        }

        // Drift
        for (const a of atoms) {
            if (a.locked) continue;
            a.x += a.vx * dt;
            a.y += a.vy * dt;
            a.z += a.vz * dt;
        }

        // Boundary containment (AE mode: origin-centered, radius 35)
        if (this._boundaryShape !== 'cube' && this._boundaryShape !== 'none') {
            for (const a of atoms) {
                if (a.locked) continue;
                this._reflectIntoBoundary(a, 0, 0, 0, 35);
            }
        }

        // Recompute forces at new positions
        forces = this._aeComputeAllForces();

        // Half-kick again
        for (let i = 0; i < atoms.length; i++) {
            const a = atoms[i];
            if (a.locked) continue;
            const hdt = dt * 0.5 / a.mass;
            a.vx += forces[i].fx * hdt;
            a.vy += forces[i].fy * hdt;
            a.vz += forces[i].fz * hdt;
        }

        // Debug: log positions after integration
        if (tickNum < 3) {
            for (let i = 0; i < Math.min(atoms.length, 4); i++) {
                const a = atoms[i];
                debugLog(`  atom ${a.id} after tick: pos=(${a.x.toFixed(3)},${a.y.toFixed(3)},${a.z.toFixed(3)}) vel=(${a.vx.toFixed(4)},${a.vy.toFixed(4)},${a.vz.toFixed(4)})`);
            }
        }

        // Speed limit (simulation units, not Planck)
        if (this._ae.speed_limit) {
            for (const a of atoms) {
                if (a.locked) continue;
                const speed = Math.sqrt(a.vx * a.vx + a.vy * a.vy + a.vz * a.vz);
                if (speed > AE_SPEED_MAX) {
                    const s = AE_SPEED_MAX / speed;
                    a.vx *= s; a.vy *= s; a.vz *= s;
                }
            }
        }

        // Damping (use 0.02 per unit time for visible effect)
        if (this._ae.damping) {
            const d = Math.max(0, 1 - 0.02 * dt);
            for (const a of atoms) {
                if (a.locked) continue;
                a.vx *= d; a.vy *= d; a.vz *= d;
            }
        }

        // Berendsen thermostat: rescale velocities toward target temperature
        if (this._ae.thermostat && this._ae.thermostat_temp > 0) {
            let ke = 0, n_free = 0;
            for (const a of atoms) {
                if (!a.locked) {
                    ke += 0.5 * a.mass * (a.vx*a.vx + a.vy*a.vy + a.vz*a.vz);
                    n_free++;
                }
            }
            if (n_free > 0) {
                const T_current = 2.0 * ke / (3.0 * n_free);
                if (T_current > 1e-30) {
                    const lam = Math.sqrt(1.0 + dt / AE_THERMOSTAT_TAU
                        * (this._ae.thermostat_temp / T_current - 1.0));
                    for (const a of atoms) {
                        if (!a.locked) { a.vx *= lam; a.vy *= lam; a.vz *= lam; }
                    }
                }
            }
        }

        // Auto-bonding algorithm:
        // For each unique pair (i,j), form a covalent bond if:
        //   1. Not already bonded
        //   2. Both atoms have available bond capacity (bonds.length < max_bonds)
        //   3. Distance < 1.2 * average vdW sigma (extended by electronegativity
        //      difference when that toggle is on, allowing polar bond formation
        //      at slightly longer distances)
        // Bond parameters: r_eq = sigma * 2^(1/6) (LJ minimum), k = K_BOND * eps / r_eq^2
        // Bond breaking occurs when stretched beyond 3.5 * r_eq (conservative;
        // real covalent bonds survive to ~4-5x equilibrium).
        if (this._ae.bonding) {
            for (let i = 0; i < atoms.length; i++) {
                for (let j = i + 1; j < atoms.length; j++) {
                    const ai = atoms[i], aj = atoms[j];
                    // Check if already bonded
                    if (ai.bonds.some(b => b.partner_id === aj.id)) continue;
                    // Check if both have capacity
                    if (ai.bonds.length >= ai.max_bonds || aj.bonds.length >= aj.max_bonds) continue;
                    const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    const sig_avg = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                    // Electronegativity extends bond formation for polar pairs
                    let bond_threshold = 1.2 * sig_avg;
                    if (this._ae.electronegativity) {
                        const chi_diff = Math.abs(ai.electronegativity - aj.electronegativity);
                        bond_threshold *= (1.0 + 0.2 * chi_diff);
                    }
                    if (r < bond_threshold) {
                        const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0);
                        const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                        const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
                        ai.bonds.push({ partner_id: aj.id, r_eq, k_bond, order: 1 });
                        aj.bonds.push({ partner_id: ai.id, r_eq, k_bond, order: 1 });
                    }
                }
            }
            // Bond breaking — break only when stretched far beyond equilibrium
            // 3.5× r_eq is conservative: real covalent bonds don't dissociate
            // until ~4-5× equilibrium. The old 2.0× was too aggressive for
            // complex molecules (caffeine, adenine) where cumulative non-bonded
            // forces cause H-atom oscillation amplitude to exceed the threshold.
            for (const a of atoms) {
                a.bonds = a.bonds.filter(b => {
                    const jIdx = this._aeIdToIdx.get(b.partner_id);
                    if (jIdx === undefined) return false;
                    const partner = atoms[jIdx];
                    const dx = partner.x - a.x, dy = partner.y - a.y, dz = partner.z - a.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    return r <= 3.5 * b.r_eq;
                });
            }
        }

        this._ae.tick++;
    }

    // NOTE: Unlike peGetParticleData which reuses pre-allocated buffers,
    // aeGetAtomData allocates fresh typed arrays every call. This is because
    // the bond arrays change size dynamically (bonds form/break). The atom
    // arrays could be pooled, but bond data would still need fresh allocation.
    // For typical AE simulations (< 500 atoms at 60fps), GC overhead is minimal.
    aeGetAtomData() {
        if (!this._ae) return { positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), atomicNums: new Int32Array(0), charges: new Int32Array(0), ids: new Int32Array(0), bonds: new Int32Array(0), bondOrders: new Int32Array(0), bondCount: 0, count: 0 };
        const atoms = this._ae.atoms;
        const count = atoms.length;
        const positions = new Float32Array(count * 3);
        const colors = new Float32Array(count * 3);
        const sizes = new Float32Array(count);
        const atomicNums = new Int32Array(count);
        const charges = new Int32Array(count);
        const ids = new Int32Array(count);

        // Count bonds (avoid double-counting)
        let bondCount = 0;
        for (const a of atoms) {
            for (const b of a.bonds) {
                if (b.partner_id > a.id) bondCount++;
            }
        }
        const bonds = new Int32Array(bondCount * 2);
        const bondOrders = new Int32Array(bondCount);

        for (let i = 0; i < count; i++) {
            const a = atoms[i];
            positions[i * 3] = a.x;
            positions[i * 3 + 1] = a.y;
            positions[i * 3 + 2] = a.z;
            const [cr, cg, cb] = cpkColor(a.Z);
            colors[i * 3] = cr; colors[i * 3 + 1] = cg; colors[i * 3 + 2] = cb;
            sizes[i] = 6.0 + a.radius * 10.0;  // Proportional to atomic radius (2× for 150 factor)
            if (sizes[i] > 60) sizes[i] = 60;
            atomicNums[i] = a.Z;
            charges[i] = a.charge;
            ids[i] = a.id;
        }

        let bi = 0;
        for (const a of atoms) {
            for (const b of a.bonds) {
                if (b.partner_id > a.id) {
                    bonds[bi * 2] = a.id;
                    bonds[bi * 2 + 1] = b.partner_id;
                    bondOrders[bi] = b.order || 1;
                    bi++;
                }
            }
        }

        return { positions, colors, sizes, atomicNums, charges, ids, bonds, bondOrders, bondCount, count };
    }

    aeGetFieldSources() {
        if (!this._ae) return { positions: new Float32Array(0), charges: new Float32Array(0), count: 0 };
        const atoms = this._ae.atoms;
        const n = atoms.length;
        const positions = new Float32Array(n * 3);
        const charges = new Float32Array(n);
        for (let i = 0; i < n; i++) {
            positions[i * 3] = atoms[i].x;
            positions[i * 3 + 1] = atoms[i].y;
            positions[i * 3 + 2] = atoms[i].z;
            charges[i] = atoms[i].charge;
        }
        return { positions, charges, count: n };
    }

    aeGetDiagnostics() {
        if (!this._ae) return { tick: 0, atomCount: 0, bondCount: 0, totalKE: 0, totalPEIonic: 0, totalPEVdw: 0, totalPEBond: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0, temperature: 0 };
        const atoms = this._ae.atoms;
        let ke = 0, pe_ionic = 0, pe_vdw = 0, pe_bond = 0;
        let px = 0, py = 0, pz = 0;
        const soft2 = this._ae.soft * this._ae.soft;

        for (const a of atoms) {
            const v2 = a.vx * a.vx + a.vy * a.vy + a.vz * a.vz;
            ke += 0.5 * a.mass * v2;
            px += a.mass * a.vx; py += a.mass * a.vy; pz += a.mass * a.vz;
        }

        for (let i = 0; i < atoms.length; i++) {
            for (let j = i + 1; j < atoms.length; j++) {
                const ai = atoms[i], aj = atoms[j];
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                if (ai.charge !== 0 && aj.charge !== 0) {
                    pe_ionic += AE_K_COULOMB * ai.charge * aj.charge / r;
                }
                const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                const sig_mix = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                const sr = sig_mix / r; const sr6 = sr ** 6; const sr12 = sr6 * sr6;
                pe_vdw += 4.0 * eps_mix * (sr12 - sr6);
            }
        }

        // Bond PE (O(bonds * N) due to .find() — acceptable for diagnostics at < 500 atoms.
        // For larger systems, use _aeIdToIdx Map built by _aeBuildBondLookup().)
        const counted = new Set();
        for (const a of atoms) {
            for (const b of a.bonds) {
                const key = Math.min(a.id, b.partner_id) + ',' + Math.max(a.id, b.partner_id);
                if (counted.has(key)) continue;
                counted.add(key);
                const partner = atoms.find(at => at.id === b.partner_id);
                if (!partner) continue;
                const dx = partner.x - a.x, dy = partner.y - a.y, dz = partner.z - a.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const dr = r - b.r_eq;
                pe_bond += 0.5 * b.k_bond * dr * dr;
            }
        }

        let bondCount = 0;
        for (const a of atoms) {
            for (const b of a.bonds) { if (b.partner_id > a.id) bondCount++; }
        }

        const T = atoms.length > 0 ? 2.0 * ke / (3.0 * atoms.length) : 0;

        return {
            tick: this._ae.tick, atomCount: atoms.length, bondCount,
            totalKE: ke, totalPEIonic: pe_ionic, totalPEVdw: pe_vdw, totalPEBond: pe_bond,
            totalEnergy: ke + pe_ionic + pe_vdw + pe_bond,
            momentumX: px, momentumY: py, momentumZ: pz, temperature: T
        };
    }

    /**
     * Get decomposed forces on each atom: ionic (Coulomb), vdW (LJ), bond (spring), and net.
     * Returns { ionic, vdw, bond, net, count } where each is a Float32Array of n×3.
     */
    aeGetForceDecomposition() {
        if (!this._ae) return { ionic: new Float32Array(0), vdw: new Float32Array(0), bond: new Float32Array(0), net: new Float32Array(0), count: 0 };
        const atoms = this._ae.atoms;
        const n = atoms.length;
        const ionic = new Float32Array(n * 3);
        const vdw   = new Float32Array(n * 3);
        const bond  = new Float32Array(n * 3);
        const net   = new Float32Array(n * 3);
        const soft2 = this._ae.soft * this._ae.soft;

        this._aeBuildBondLookup();

        for (let i = 0; i < n; i++) {
            const ai = atoms[i];
            let fi_x = 0, fi_y = 0, fi_z = 0; // ionic
            let fv_x = 0, fv_y = 0, fv_z = 0; // vdw
            let fb_x = 0, fb_y = 0, fb_z = 0; // bond

            // Pairwise forces
            for (let j = 0; j < n; j++) {
                if (j === i) continue;
                const aj = atoms[j];
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r2 = dx * dx + dy * dy + dz * dz + soft2;
                const r = Math.sqrt(r2);
                if (r < 1e-20) continue;
                const rx = dx / r, ry = dy / r, rz = dz / r;

                const isBonded = this._aeIsBonded(ai.id, aj.id);
                const is13 = !isBonded && this._aeIs13(i, j);

                // Ionic (Coulomb)
                if (this._ae.ionic && !isBonded && !is13 && ai.charge !== 0 && aj.charge !== 0) {
                    const f = -AE_K_COULOMB * ai.charge * aj.charge / r2;
                    fi_x += f * rx; fi_y += f * ry; fi_z += f * rz;
                }

                // Van der Waals (LJ 12-6)
                if (this._ae.vdw && !isBonded && !is13) {
                    const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                    const sig_mix = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                    const sr = sig_mix / r;
                    const sr6 = sr * sr * sr * sr * sr * sr;
                    const sr12 = sr6 * sr6;
                    const f = -24.0 * eps_mix * (2.0 * sr12 - sr6) / r;
                    fv_x += f * rx; fv_y += f * ry; fv_z += f * rz;
                }
            }

            // Bond spring forces
            if (this._ae.bonds_force) {
                for (const b of ai.bonds) {
                    const jIdx = this._aeIdToIdx.get(b.partner_id);
                    const aj = jIdx !== undefined ? atoms[jIdx] : null;
                    if (!aj) continue;
                    const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                    if (r < 1e-20) continue;
                    const rx = dx / r, ry = dy / r, rz = dz / r;
                    const dr = r - b.r_eq;
                    const f = b.k_bond * dr;
                    fb_x += f * rx; fb_y += f * ry; fb_z += f * rz;
                }
            }

            ionic[i * 3] = fi_x; ionic[i * 3 + 1] = fi_y; ionic[i * 3 + 2] = fi_z;
            vdw[i * 3]   = fv_x; vdw[i * 3 + 1]   = fv_y; vdw[i * 3 + 2]   = fv_z;
            bond[i * 3]  = fb_x; bond[i * 3 + 1]  = fb_y; bond[i * 3 + 2]  = fb_z;
            net[i * 3]   = fi_x + fv_x + fb_x;
            net[i * 3 + 1] = fi_y + fv_y + fb_y;
            net[i * 3 + 2] = fi_z + fv_z + fb_z;
        }

        return { ionic, vdw, bond, net, count: n };
    }

    aeSetDt(dt) { if (this._ae) this._ae.dt = dt; }
    aeGetDt() { return this._ae ? this._ae.dt : 0.01; }
    aeSetSoftening(s) { if (this._ae) this._ae.soft = s; }
    aeSetDamping(e) { if (this._ae) this._ae.damping = e; }
    aeSetBonding(e) { if (this._ae) this._ae.bonding = e; }
    aeSetIonic(e) { if (this._ae) this._ae.ionic = e; }
    aeSetVdw(e) { if (this._ae) this._ae.vdw = e; }
    aeSetBondsForce(e) { if (this._ae) this._ae.bonds_force = e; }
    aeSetSpeedLimit(e) { if (this._ae) this._ae.speed_limit = e; }
    // Phase 3 setters
    aeSetHBonds(e)            { if (this._ae) this._ae.h_bonds = e; }
    aeSetAngleStrain(e)       { if (this._ae) this._ae.angle_strain = e; }
    aeSetDipoleDipole(e)      { if (this._ae) this._ae.dipole_dipole = e; }
    aeSetThermostat(e)        { if (this._ae) this._ae.thermostat = e; }
    aeSetThermostatTemp(t)    { if (this._ae) this._ae.thermostat_temp = t; }
    aeSetElectronegativity(e) { if (this._ae) this._ae.electronegativity = e; }
    aeAtomCount() { return this._ae ? this._ae.atoms.length : 0; }

    aeInspectAtom(id) {
        if (!this._ae) return null;
        const a = this._ae.atoms.find(at => at.id === id);
        if (!a) return null;
        const mass = a.Z + a.N * 1.001;
        const speed = Math.sqrt(a.vx * a.vx + a.vy * a.vy + a.vz * a.vz);
        const ke = 0.5 * mass * speed * speed;

        // Bond info: partner IDs, distances, equilibrium lengths
        const bondInfo = a.bonds.map(b => {
            const p = this._ae.atoms.find(at => at.id === b.partner_id);
            if (!p) return null;
            const dx = p.x - a.x, dy = p.y - a.y, dz = p.z - a.z;
            const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
            return { partnerId: b.partner_id, partnerZ: p.Z, dist, r_eq: b.r_eq, order: b.order };
        }).filter(Boolean);

        // Find nearest non-bonded neighbor
        let nearestId = -1, nearestDist = Infinity, nearestZ = 0;
        const bondSet = new Set(a.bonds.map(b => b.partner_id));
        for (const other of this._ae.atoms) {
            if (other.id === id || bondSet.has(other.id)) continue;
            const dx = other.x - a.x, dy = other.y - a.y, dz = other.z - a.z;
            const d = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (d < nearestDist) { nearestDist = d; nearestId = other.id; nearestZ = other.Z; }
        }

        // Net force magnitude — must rebuild bond lookups first since
        // _aeComputeForce depends on _aeBondSet/_aeIdToIdx/_aeNeighborSets
        // which are only populated by _aeBuildBondLookup (called in aeTick,
        // but not guaranteed to be fresh if inspecting before first tick).
        this._aeBuildBondLookup();
        const idx = this._ae.atoms.indexOf(a);
        const f = this._aeComputeForce(idx);
        const fNetMag = Math.sqrt(f.fx * f.fx + f.fy * f.fy + f.fz * f.fz);

        return {
            id, Z: a.Z, N: a.N, charge: a.charge, mass, radius: a.radius,
            locked: a.locked, sigma: a.vdw_sigma, epsilon: a.vdw_epsilon,
            maxBonds: a.max_bonds,
            x: a.x, y: a.y, z: a.z,
            vx: a.vx, vy: a.vy, vz: a.vz,
            speed, ke, bonds: bondInfo,
            nearestId, nearestDist, nearestZ, fNetMag,
        };
    }

    aeClear() { this.resetAE(); }

    setupScenario(name) {
        const N = this.latticeSize;
        const mid = Math.floor(N / 2);
        this.reset();

        // ── Flux-only scenarios (Scale 0 substrate) ──
        if (name.startsWith('flux-')) {
            this._initFluxGrid();
            const sigma = N / 8;
            const amp = K_B * 2;

            switch (name) {
                case 'flux-pulse': {
                    // Gaussian pulse at center — extent scales with sigma (3σ cutoff)
                    const pulseR = Math.min(Math.ceil(sigma * 3), mid - 1);
                    for (let dz = -pulseR; dz <= pulseR; dz++) for (let dy = -pulseR; dy <= pulseR; dy++) for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) this._injectFlux(mid + dx, mid + dy, mid + dz, val, 0, 0);
                    }
                    break;
                }
                case 'flux-dipole': {
                    // Two opposite flux injections
                    const off = Math.floor(N / 4);
                    for (let d = -4; d <= 4; d++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = amp * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) {
                            this._injectFlux(mid - off + dx, mid + dy, mid + d, val, val * 0.5, 0);
                            this._injectFlux(mid + off + dx, mid + dy, mid + d, -val, -val * 0.5, 0);
                        }
                    }
                    break;
                }
                case 'flux-standing': {
                    // Counter-propagating pulses along X
                    const off = Math.floor(N / 3);
                    for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) {
                            this._injectFlux(mid - off + dx, mid + dy, mid + dz, val, 0, 0);
                            this._injectFlux(mid + off + dx, mid + dy, mid + dz, val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-dispersion': {
                    // Sharp single-site impulse
                    this._injectFlux(mid, mid, mid, amp * 5, amp * 5, amp * 5);
                    break;
                }
                case 'flux-soliton': {
                    // Large amplitude nonlinear pulse
                    for (let dz = -3; dz <= 3; dz++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 10 * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) this._injectFlux(mid + dx, mid + dy, mid + dz, val, val, 0);
                    }
                    break;
                }
                case 'flux-cascade': {
                    // Above genesis threshold
                    const bigAmp = K_GENESIS * 3;
                    for (let dz = -3; dz <= 3; dz++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = bigAmp * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) this._injectFlux(mid + dx, mid + dy, mid + dz, val, 0, val * 0.5);
                    }
                    break;
                }
                case 'flux-damping': {
                    // Two pulses for comparing damped vs undamped
                    const off = Math.floor(N / 4);
                    for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) {
                            this._injectFlux(mid - off + dx, mid + dy, mid + dz, val, 0, 0);
                            this._injectFlux(mid + off + dx, mid + dy, mid + dz, 0, val, 0);
                        }
                    }
                    break;
                }
                case 'flux-annihilation': {
                    // Two matter-antimatter pairs on collision courses (X-axis + Z-axis)
                    const off = Math.floor(N / 3);
                    // X-axis pair
                    this.injectParticle(mid - off, mid, mid, 1);
                    this.injectParticle(mid + off, mid, mid, -1);
                    // Z-axis pair
                    this.injectParticle(mid, mid, mid - off, -1);
                    this.injectParticle(mid, mid, mid + off, 1);
                    // Strong flux kicks toward center for dramatic head-on collisions
                    const pushAmp = amp * 2;
                    for (let dz = -3; dz <= 3; dz++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = pushAmp * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) {
                            // X-axis pair: push inward along X
                            this._injectFlux(mid - off + dx, mid + dy, mid + dz, val, 0, 0);
                            this._injectFlux(mid + off + dx, mid + dy, mid + dz, -val, 0, 0);
                            // Z-axis pair: push inward along Z
                            this._injectFlux(mid + dx, mid + dy, mid - off + dz, 0, 0, val);
                            this._injectFlux(mid + dx, mid + dy, mid + off + dz, 0, 0, -val);
                        }
                    }
                    break;
                }
                case 'flux-pair-production': {
                    // Super-threshold flux burst → spontaneous ±1 pair genesis
                    const bigAmp = K_GENESIS * 5;
                    for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = bigAmp * Math.exp(-r2 / (2 * 6));
                        if (val > 0.001) {
                            this._injectFlux(mid + dx, mid + dy, mid + dz, val, val * 0.7, val * 0.3);
                        }
                    }
                    break;
                }
                case 'flux-hydrogen': {
                    // Locked +1 proton at center + free -1 electron nearby
                    // Electron offset and Coulomb dressing scale with lattice size
                    const hOff = Math.max(3, Math.floor(N / 6));
                    const hDress = Math.max(3, Math.floor(N / 6));
                    const hDress2 = hDress * hDress;
                    this.injectParticle(mid, mid, mid, 1);   // proton (locked)
                    this.injectParticle(mid + hOff, mid, mid, -1); // electron
                    // Seed flux as Coulomb-like dressing around proton
                    for (let dz = -hDress; dz <= hDress; dz++) for (let dy = -hDress; dy <= hDress; dy++) for (let dx = -hDress; dx <= hDress; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 === 0 || r2 > hDress2) continue;
                        const r = Math.sqrt(r2);
                        const val = amp * 0.5 / r;
                        this._injectFlux(mid + dx, mid + dy, mid + dz, val * dx / r, val * dy / r, val * dz / r);
                    }
                    break;
                }
                case 'flux-interference': {
                    // 4 coherent sources → constructive/destructive pattern
                    const q = Math.floor(N / 4);
                    const sources = [
                        [mid - q, mid, mid - q],
                        [mid + q, mid, mid - q],
                        [mid - q, mid, mid + q],
                        [mid + q, mid, mid + q],
                    ];
                    for (const [sx, sy, sz] of sources) {
                        for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = amp * 1.5 * Math.exp(-r2 / (2 * 6));
                            if (val > 0.001) this._injectFlux(sx + dx, sy + dy, sz + dz, val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-vortex': {
                    // Circular-polarized flux ring → curl-dominated structure (spin origin)
                    const vRadius = Math.floor(N / 5);
                    const nV = 24;
                    for (let i = 0; i < nV; i++) {
                        const angle = (2 * Math.PI * i) / nV;
                        const rx = Math.round(mid + vRadius * Math.cos(angle));
                        const rz = Math.round(mid + vRadius * Math.sin(angle));
                        // Tangential flux (perpendicular to radius) + upward component for helicity
                        const tX = -Math.sin(angle) * amp * 2;
                        const tZ = Math.cos(angle) * amp * 2;
                        const tY = amp * 0.5; // helicity
                        this._injectFlux(rx, mid, rz, tX, tY, tZ);
                        // Also inject a ring above and below for 3D structure
                        this._injectFlux(rx, mid + 1, rz, tX * 0.5, tY * 0.5, tZ * 0.5);
                        this._injectFlux(rx, mid - 1, rz, tX * 0.5, -tY * 0.5, tZ * 0.5);
                    }
                    break;
                }
                case 'flux-dual-substrate': {
                    // L/R chirality demo — two offset pulses in dual-substrate mode
                    const off = Math.floor(N / 4);
                    for (let dz = -5; dz <= 5; dz++) for (let dy = -5; dy <= 5; dy++) for (let dx = -5; dx <= 5; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 1.5 * Math.exp(-r2 / (2 * 8));
                        if (val > 0.001) {
                            // Left-handed pulse
                            this._injectFlux(mid - off + dx, mid + dy, mid + dz, val, val * 0.5, -val * 0.3);
                            // Right-handed pulse (opposite chirality)
                            this._injectFlux(mid + off + dx, mid + dy, mid + dz, val, -val * 0.5, val * 0.3);
                        }
                    }
                    break;
                }
                case 'flux-random-genesis': {
                    // Random super-threshold flux patches → stochastic particle creation
                    const nPatches = 8;
                    const threshold = K_GENESIS * 2.5;
                    for (let p = 0; p < nPatches; p++) {
                        const cx = Math.floor(Math.random() * (N - 8)) + 4;
                        const cy = Math.floor(Math.random() * (N - 8)) + 4;
                        const cz = Math.floor(Math.random() * (N - 8)) + 4;
                        const pAmp = threshold * (0.8 + Math.random() * 0.8);
                        for (let dz = -2; dz <= 2; dz++) for (let dy = -2; dy <= 2; dy++) for (let dx = -2; dx <= 2; dx++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = pAmp * Math.exp(-r2 / (2 * 3));
                            if (val > 0.001) {
                                const sx = (Math.random() - 0.5) * val;
                                const sy = (Math.random() - 0.5) * val;
                                const sz = (Math.random() - 0.5) * val;
                                this._injectFlux(cx + dx, cy + dy, cz + dz, sx, sy, sz);
                            }
                        }
                    }
                    break;
                }

                // ── QCD Scenarios ──
                case 'flux-meson': {
                    // Quark-antiquark bound state with confinement
                    const mOff = Math.max(2, Math.floor(N / 8));
                    const mDress = Math.max(2, Math.floor(N / 10));
                    this.injectParticle(mid - mOff, mid, mid, 1);
                    this.injectParticle(mid + mOff, mid, mid, -1);
                    // Small perpendicular velocity kick for oscillation
                    const mpIdx = this._particles.length;
                    this._particles[mpIdx - 2].vy = 0.05;
                    this._particles[mpIdx - 1].vy = -0.05;
                    // Gaussian flux dressing around both
                    const mesonAmp = K_B * 1.5;
                    const mSigma2 = mDress * mDress;
                    for (let dz = -mDress; dz <= mDress; dz++) for (let dy = -mDress; dy <= mDress; dy++) for (let dx = -mDress; dx <= mDress; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = mesonAmp * Math.exp(-r2 / (2 * mSigma2));
                        if (val > 0.001) {
                            this._injectFlux(mid - mOff + dx, mid + dy, mid + dz, val, 0, 0);
                            this._injectFlux(mid + mOff + dx, mid + dy, mid + dz, -val, 0, 0);
                        }
                    }
                    break;
                }
                case 'flux-string-breaking': {
                    // Confinement string snap — pair yanked apart until string breaks
                    const sbOff = Math.max(2, Math.floor(N / 10));
                    const sbDress = Math.max(2, Math.floor(N / 8));
                    this.injectParticle(mid - sbOff, mid, mid, 1);
                    this.injectParticle(mid + sbOff, mid, mid, -1);
                    // Strong outward velocity kicks
                    const sbIdx = this._particles.length;
                    this._particles[sbIdx - 2].vx = -0.3;
                    this._particles[sbIdx - 1].vx = 0.3;
                    // High flux for genesis at midpoint when string snaps
                    const sbAmp = K_B * 3;
                    for (let dz = -sbDress; dz <= sbDress; dz++) for (let dy = -sbDress; dy <= sbDress; dy++) for (let dx = -sbDress; dx <= sbDress; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = sbAmp * Math.exp(-r2 / (2 * sbDress));
                        if (val > 0.001) {
                            this._injectFlux(mid + dx, mid + dy, mid + dz, val, val * 0.3, 0);
                        }
                    }
                    break;
                }
                case 'flux-baryon': {
                    // Three-quark bound state in equilateral triangle + sea quark
                    const bR = Math.floor(N / 6);
                    for (let k = 0; k < 3; k++) {
                        const angle = (2 * Math.PI * k) / 3;
                        const bx = Math.round(mid + bR * Math.cos(angle));
                        const bz = Math.round(mid + bR * Math.sin(angle));
                        this.injectParticle(bx, mid, bz, 1);
                        // Small centripetal velocity
                        const bidx = this._particles.length - 1;
                        this._particles[bidx].vx = -0.04 * Math.sin(angle);
                        this._particles[bidx].vz = 0.04 * Math.cos(angle);
                    }
                    // Sea quark nearby (offset scales with lattice)
                    const bSea = Math.max(1, Math.floor(bR / 2));
                    this.injectParticle(mid + bSea, mid + bSea, mid, -1);
                    // Light flux dressing
                    for (let dz = -3; dz <= 3; dz++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 0.5 * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) this._injectFlux(mid + dx, mid + dy, mid + dz, val, 0, val * 0.3);
                    }
                    break;
                }

                case 'flux-nested-standing': {
                    // Two orthogonal counter-propagating pulse pairs for nested sLoop
                    // X-axis pair (sLoop level 1) + Z-axis pair (sLoop level 2)
                    const offX = Math.floor(N / 3);
                    const offZ = Math.floor(N / 4);
                    for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) {
                            // X-axis standing wave (sLoop level 1)
                            this._injectFlux(mid - offX + dx, mid + dy, mid + dz, val, 0, 0);
                            this._injectFlux(mid + offX + dx, mid + dy, mid + dz, val, 0, 0);
                            // Z-axis standing wave (sLoop level 2)
                            this._injectFlux(mid + dx, mid + dy, mid - offZ + dz, 0, 0, val);
                            this._injectFlux(mid + dx, mid + dy, mid + offZ + dz, 0, 0, val);
                        }
                    }
                    break;
                }

                // ── Experiment scenarios (from test suite) ──

                case 'flux-rutherford': {
                    // Rutherford scattering: locked +1 nucleus at center,
                    // -1 projectile incoming with impact parameter b ≈ N/8
                    // (from test_gpu_experiments GP-EXP-RUTHERFORD)
                    const b = Math.floor(N / 8); // impact parameter
                    const startX = Math.floor(N / 6);
                    // Locked nucleus at center
                    this.injectParticle(mid, mid, mid, 1);
                    // Projectile offset in y by impact parameter, far left
                    this.injectParticle(startX, mid + b, mid, -1);
                    // Give projectile a flux kick toward +x
                    for (let dx = -3; dx <= 3; dx++) for (let dy = -3; dy <= 3; dy++) for (let dz = -3; dz <= 3; dz++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 0.8 * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) {
                            this._injectFlux(startX + dx, mid + b + dy, mid + dz, val, 0, 0);
                        }
                    }
                    break;
                }

                case 'flux-cyclotron': {
                    // Cyclotron motion: uniform B-field (curl of J) + charged particle
                    // (from test_gpu_experiments GP-EXP-CYCLOTRON)
                    // Create background B-field along z by injecting circular flux in xy-plane
                    const bAmp = amp * 0.15;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        // J = B × r / 2 for uniform B_z → J_x = -B*y/2, J_y = +B*x/2
                        const cx = x - mid, cy = y - mid;
                        this._injectFlux(x, y, z, -bAmp * cy * 0.05, bAmp * cx * 0.05, 0);
                    }
                    // Charged particle with velocity in +x
                    this.injectParticle(mid, mid, mid, 1);
                    for (let d = -3; d <= 3; d++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = amp * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) {
                            this._injectFlux(mid + dx, mid + dy, mid + d, val * 0.5, 0, 0);
                        }
                    }
                    break;
                }

                case 'flux-screening': {
                    // Charge screening: central +1 surrounded by 6 opposite charges
                    // (from test_gpu_experiments GP-EXP-SCREENING / Debye-Hückel)
                    const shellR = Math.floor(N / 5);
                    this.injectParticle(mid, mid, mid, 1);
                    // 6 screening charges on face-axes
                    const scOffsets = [
                        [shellR, 0, 0], [-shellR, 0, 0],
                        [0, shellR, 0], [0, -shellR, 0],
                        [0, 0, shellR], [0, 0, -shellR],
                    ];
                    for (const [ox, oy, oz] of scOffsets) {
                        this.injectParticle(mid + ox, mid + oy, mid + oz, -1);
                    }
                    // Seed flux dressing around central charge (scales with L)
                    const scDress = Math.max(3, Math.floor(shellR * 0.8));
                    const scDress2 = scDress * scDress;
                    for (let dz = -scDress; dz <= scDress; dz++) for (let dy = -scDress; dy <= scDress; dy++) for (let dx = -scDress; dx <= scDress; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 === 0 || r2 > scDress2) continue;
                        const r = Math.sqrt(r2);
                        const val = amp * 0.5 / r;
                        this._injectFlux(mid + dx, mid + dy, mid + dz, val * dx / r, val * dy / r, val * dz / r);
                    }
                    break;
                }

                case 'flux-gravitational-wave': {
                    // Binary system: two same-sign masses orbiting → grav wave emission
                    // (from campaign_gravitational_wave)
                    const orbR = Math.floor(N / 6);
                    // Two +1 particles on opposite sides of center
                    this.injectParticle(mid + orbR, mid, mid, 1);
                    this.injectParticle(mid - orbR, mid, mid, 1);
                    // Give them tangential flux kicks for orbital motion
                    for (let d = -3; d <= 3; d++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = amp * 0.6 * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) {
                            // Tangential kicks: +y for left mass, -y for right mass
                            this._injectFlux(mid + orbR + dx, mid + dy, mid + d, 0, val, 0);
                            this._injectFlux(mid - orbR + dx, mid + dy, mid + d, 0, -val, 0);
                        }
                    }
                    break;
                }

                case 'flux-triad': {
                    // Triad formation: 3 same-sign particles in equilateral triangle
                    // (from campaign_triad_binding / campaign_baryon_formation)
                    const tR = Math.floor(N / 6);
                    const triAngles = [0, 2 * Math.PI / 3, 4 * Math.PI / 3];
                    for (const angle of triAngles) {
                        const px = mid + Math.round(tR * Math.cos(angle));
                        const pz = mid + Math.round(tR * Math.sin(angle));
                        this.injectParticle(px, mid, pz, 1);
                        // Flux kick toward center (binding)
                        for (let dx = -3; dx <= 3; dx++) for (let dy = -3; dy <= 3; dy++) for (let dz = -3; dz <= 3; dz++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = amp * 0.5 * Math.exp(-r2 / (2 * 4));
                            if (val > 0.001) {
                                const toCX = (mid - (px + dx));
                                const toCZ = (mid - (pz + dz));
                                const dist = Math.sqrt(toCX * toCX + toCZ * toCZ) || 1;
                                this._injectFlux(px + dx, mid + dy, pz + dz,
                                    val * toCX / dist, 0, val * toCZ / dist);
                            }
                        }
                    }
                    break;
                }

                case 'flux-thermalization': {
                    // Thermalization: concentrated energy in one corner → watch it spread
                    // (from test_thermodynamics — entropy increase demo)
                    const corner = Math.floor(N / 4);
                    const thermAmp = amp * 3;
                    for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = thermAmp * Math.exp(-r2 / (2 * 6));
                        if (val > 0.001) {
                            // Random flux directions for maximum entropy growth
                            const rx = (Math.random() - 0.5) * 2;
                            const ry = (Math.random() - 0.5) * 2;
                            const rz2 = (Math.random() - 0.5) * 2;
                            const rLen = Math.sqrt(rx * rx + ry * ry + rz2 * rz2) || 1;
                            this._injectFlux(corner + dx, corner + dy, corner + dz,
                                val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
                        }
                    }
                    break;
                }


                // ── Cosmology scenarios ──
                case 'flux-dark-matter': {
                    // Sub-threshold flux halo (dark matter) + 3 visible particles
                    const haloR = Math.floor(N / 3);
                    const haloSigma = haloR / 2;
                    const haloAmp = K_B * 0.3; // well below genesis threshold
                    // Fill spherical Gaussian halo
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const dx = x - mid, dy = y - mid, dz = z - mid;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > haloR * haloR) continue;
                        const r = Math.sqrt(r2) || 1;
                        const val = haloAmp * Math.exp(-r2 / (2 * haloSigma * haloSigma));
                        if (val < 1e-4) continue;
                        // Radial flux direction (gentle outward)
                        this._injectFlux(x, y, z, val * dx / r * 0.3, val * dy / r * 0.3, val * dz / r * 0.3);
                    }
                    // 3 visible particles at N/3 from center on different axes
                    const pOff = Math.floor(N / 3);
                    this.injectParticle(mid + pOff, mid, mid, 1);
                    this.injectParticle(mid, mid + pOff, mid, 1);
                    this.injectParticle(mid, mid, mid + pOff, -1);
                    break;
                }
                case 'flux-baryogenesis': {
                    // 8 matter + 6 antimatter → annihilation leaves 2 residual matter
                    const spread = Math.floor(N / 4);
                    // Deterministic pseudo-random positions using golden angle
                    const phi_g = (1 + Math.sqrt(5)) / 2;
                    const positions = [];
                    for (let i = 0; i < 14; i++) {
                        const t = i / 14;
                        const inclination = Math.acos(1 - 2 * t);
                        const azimuth = 2 * Math.PI * i * phi_g;
                        const r = spread * (0.3 + 0.7 * Math.random());
                        const px = mid + Math.round(r * Math.sin(inclination) * Math.cos(azimuth));
                        const py = mid + Math.round(r * Math.sin(inclination) * Math.sin(azimuth));
                        const pz = mid + Math.round(r * Math.cos(inclination));
                        positions.push([px, py, pz]);
                    }
                    // First 8 = matter (+1), next 6 = antimatter (-1)
                    for (let i = 0; i < 14; i++) {
                        const [px, py, pz] = positions[i];
                        const state = i < 8 ? 1 : -1;
                        this.injectParticle(px, py, pz, state);
                        // Small flux kick toward center
                        const dx = mid - px, dy = mid - py, dz = mid - pz;
                        const dist = Math.sqrt(dx * dx + dy * dy + dz * dz) || 1;
                        this._injectFlux(px, py, pz, amp * 0.3 * dx / dist, amp * 0.3 * dy / dist, amp * 0.3 * dz / dist);
                    }
                    break;
                }
                case 'flux-vacuum-foam': {
                    // Near-threshold flux everywhere → spontaneous pair creation/annihilation
                    const foamR = Math.floor(N / 3);
                    const foamBase = K_B * 0.9;
                    const foamVar = K_B * 0.4;
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const dx = x - mid, dy = y - mid, dz = z - mid;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > foamR * foamR) continue;
                        const r = Math.sqrt(r2);
                        const envelope = Math.exp(-r2 / (2 * foamR * foamR * 0.5));
                        const val = (foamBase + foamVar * Math.random()) * envelope;
                        // Random flux direction
                        const rx = (Math.random() - 0.5) * 2;
                        const ry = (Math.random() - 0.5) * 2;
                        const rz2 = (Math.random() - 0.5) * 2;
                        const rLen = Math.sqrt(rx * rx + ry * ry + rz2 * rz2) || 1;
                        this._injectFlux(x, y, z, val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
                    }
                    break;
                }
                case 'flux-cosmic-web': {
                    // 24 particles spread across lattice — gravity drives clustering
                    const webSpread = Math.floor(N * 0.4);
                    const phi_cw = (1 + Math.sqrt(5)) / 2;
                    for (let i = 0; i < 24; i++) {
                        const t = (i + 0.5) / 24;
                        const inclination = Math.acos(1 - 2 * t);
                        const azimuth = 2 * Math.PI * i * phi_cw;
                        const r = webSpread * (0.4 + 0.6 * Math.random());
                        const px = mid + Math.round(r * Math.sin(inclination) * Math.cos(azimuth));
                        const py = mid + Math.round(r * Math.sin(inclination) * Math.sin(azimuth));
                        const pz = mid + Math.round(r * Math.cos(inclination));
                        // Alternate +1/-1
                        const state = (i % 2 === 0) ? 1 : -1;
                        this.injectParticle(
                            Math.max(1, Math.min(N - 2, px)),
                            Math.max(1, Math.min(N - 2, py)),
                            Math.max(1, Math.min(N - 2, pz)),
                            state
                        );
                    }
                    // Small random flux kicks for initial motion
                    for (const p of this._particles) {
                        if (p.state === 0) continue;
                        const kick = amp * 0.2;
                        this._injectFlux(p.x, p.y, p.z,
                            (Math.random() - 0.5) * kick,
                            (Math.random() - 0.5) * kick,
                            (Math.random() - 0.5) * kick);
                    }
                    break;
                }

                case 'flux-black-hole': {
                    // Lattice black hole: radial inward flux sink at center
                    // + orbiting particles + wormhole throat flux tube to offset
                    const bhR = Math.floor(N / 3);
                    const bhAmp = amp * 1.2;
                    // Radial inward flux — gravitational "drain" centered at mid
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        const dx = x - mid, dy = y - mid, dz = z - mid;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 < 1 || r2 > bhR * bhR) continue;
                        const r = Math.sqrt(r2);
                        // 1/r² radial inward flux (Schwarzschild-like drain)
                        const val = bhAmp / (r * r);
                        this._injectFlux(x, y, z,
                            -val * dx / r, -val * dy / r, -val * dz / r);
                    }
                    // Wormhole throat: flux tube connecting center to offset exit
                    const whExit = Math.floor(N / 4);
                    const tubeR = 2;
                    for (let t = 0; t <= whExit; t++) {
                        const frac = t / whExit;
                        const ty = mid + t;
                        const tubeAmp = bhAmp * 0.8 * (1 - 0.5 * frac);
                        for (let dz = -tubeR; dz <= tubeR; dz++)
                        for (let dx = -tubeR; dx <= tubeR; dx++) {
                            if (dx * dx + dz * dz > tubeR * tubeR) continue;
                            this._injectFlux(mid + dx, ty, mid + dz, 0, tubeAmp, 0);
                        }
                    }
                    // Radial outward burst at wormhole exit (white hole)
                    const exitY = mid + whExit;
                    for (let dz = -4; dz <= 4; dz++)
                    for (let dy = -4; dy <= 4; dy++)
                    for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 < 1 || r2 > 16) continue;
                        const r = Math.sqrt(r2);
                        const val = bhAmp * 0.6 * Math.exp(-r2 / 8);
                        this._injectFlux(mid + dx, exitY + dy, mid + dz,
                            val * dx / r, val * dy / r, val * dz / r);
                    }
                    // 4 orbiting particles around the BH center
                    const orbR_bh = Math.floor(N / 5);
                    for (let i = 0; i < 4; i++) {
                        const angle = (i / 4) * 2 * Math.PI;
                        const px = mid + Math.round(orbR_bh * Math.cos(angle));
                        const pz = mid + Math.round(orbR_bh * Math.sin(angle));
                        this.injectParticle(px, mid, pz, i < 2 ? 1 : -1);
                        // Tangential flux kick for orbital motion
                        const kick = amp * 0.5;
                        this._injectFlux(px, mid, pz,
                            -kick * Math.sin(angle), 0, kick * Math.cos(angle));
                    }
                    break;
                }

                case 'flux-stable-vortex': {
                    // Multi-layer stable vortex: concentric tangential flux rings
                    // with counter-rotating layers for stability
                    const nRings = 3;
                    const radii = [
                        Math.floor(N / 8),   // inner ring ~4
                        Math.floor(N / 5),   // middle ring ~6
                        Math.floor(N / 3.2), // outer ring ~10
                    ];
                    const amps = [amp * 3.0, amp * 2.2, amp * 1.5];
                    const dirs = [1, -1, 1]; // alternating rotation for stability
                    const nPts = [16, 24, 36];
                    const ySpread = 3; // vertical extent of each ring

                    for (let ring = 0; ring < nRings; ring++) {
                        const rr = radii[ring];
                        const aa = amps[ring];
                        const dir = dirs[ring];
                        const np = nPts[ring];
                        for (let i = 0; i < np; i++) {
                            const angle = (2 * Math.PI * i) / np;
                            const rx = Math.round(mid + rr * Math.cos(angle));
                            const rz = Math.round(mid + rr * Math.sin(angle));
                            // Tangential flux (perpendicular to radius vector)
                            const tX = -Math.sin(angle) * aa * dir;
                            const tZ = Math.cos(angle) * aa * dir;
                            // Helicity: upward component for spin structure
                            const tY = aa * 0.3 * dir;
                            // Spread vertically for 3D structure
                            for (let dy = -ySpread; dy <= ySpread; dy++) {
                                const falloff = Math.exp(-(dy * dy) / (ySpread * 0.8));
                                this._injectFlux(rx, mid + dy, rz,
                                    tX * falloff, tY * falloff, tZ * falloff);
                            }
                        }
                    }
                    // Central axial flux column (vortex core)
                    const coreR = 2;
                    const coreAmp = amp * 2.5;
                    for (let dy = -Math.floor(N / 4); dy <= Math.floor(N / 4); dy++) {
                        const yFalloff = Math.exp(-(dy * dy) / (N * N * 0.02));
                        for (let dz = -coreR; dz <= coreR; dz++)
                        for (let dx = -coreR; dx <= coreR; dx++) {
                            if (dx * dx + dz * dz > coreR * coreR) continue;
                            this._injectFlux(mid + dx, mid + dy, mid + dz,
                                0, coreAmp * yFalloff, 0);
                        }
                    }
                    break;
                }
            }
            return;
        }

        // ── Light & Color scenarios ──
        if (name.startsWith('light-')) {
            this._initFluxGrid();
            const pi = Math.PI;
            const C_WAVE = 1 / Math.sqrt(3);
            const amp = 0.15;
            switch (name) {
                case 'light-rainbow': {
                    // Three traveling waves: red (n=1,y), green (n=3,z), blue (n=6,x)
                    const waves = [
                        { n: 1, pol: 1 },  // red → y-polarized
                        { n: 3, pol: 2 },  // green → z-polarized
                        { n: 6, pol: 0 },  // blue → x-polarized
                    ];
                    for (const w of waves) {
                        const k = 2 * pi * w.n / N;
                        const omega = 2 * C_WAVE * Math.sin(k / 2);
                        for (let x = 0; x < N; x++)
                        for (let y = 0; y < N; y++)
                        for (let z = 0; z < N; z++) {
                            const J_val = amp * Math.sin(k * x);
                            const wv_val = -omega * amp * Math.cos(k * x);
                            const fv = [0, 0, 0], wv = [0, 0, 0];
                            fv[w.pol] = J_val;
                            wv[w.pol] = wv_val;
                            this._injectFlux(x, y, z, fv[0], fv[1], fv[2]);
                            this._injectWaveVel(x, y, z, wv[0], wv[1], wv[2]);
                        }
                    }
                    break;
                }
                case 'light-prism': {
                    // Delta pulse at x=mid — all frequencies, dispersive broadening
                    const pAmp = 0.4;
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++) {
                        this._injectFlux(mid, y, z, 0, 0, pAmp);
                        this._injectWaveVel(mid, y, z, 0, 0, pAmp);
                    }
                    break;
                }
                case 'light-dipole': {
                    // Gaussian z-directed pulse → sin²θ radiation
                    const sigma = 3;
                    const dAmp = 0.5;
                    for (let x = 0; x < N; x++)
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++) {
                        const dx = x - mid, dy = y - mid, dz = z - mid;
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const g = dAmp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (g < 1e-6) continue;
                        this._injectFlux(x, y, z, 0, 0, g);
                        this._injectWaveVel(x, y, z, 0, 0, g);
                    }
                    break;
                }
                case 'light-two-slit': {
                    // Two coherent line sources offset in y, propagating in +x
                    const sigma = 2;
                    const sAmp = 0.3;
                    const slit_sep = Math.floor(N / 6);
                    const slit_x = Math.floor(N / 4);
                    const slit_ys = [mid - slit_sep, mid + slit_sep];
                    for (const sy of slit_ys) {
                        for (let z = 0; z < N; z++)
                        for (let dy = -4; dy <= 4; dy++)
                        for (let dx = -4; dx <= 4; dx++) {
                            const r2 = dx * dx + dy * dy;
                            const g = sAmp * Math.exp(-r2 / (2 * sigma * sigma));
                            if (g < 1e-6) continue;
                            const px = slit_x + dx, py = sy + dy;
                            if (px < 0 || px >= N || py < 0 || py >= N) continue;
                            this._injectFlux(px, py, z, 0, 0, g);
                            this._injectWaveVel(px, py, z, g, 0, 0); // propagate +x
                        }
                    }
                    break;
                }
                case 'light-photon-race': {
                    // Dim vs bright Gaussian pulses — same speed (linearity)
                    const sigma = 3;
                    const x_start = Math.floor(N / 4);
                    const pAmps = [0.05, 0.5];
                    const y_offsets = [mid - Math.floor(N / 6), mid + Math.floor(N / 6)];
                    for (let p = 0; p < 2; p++) {
                        for (let x = 0; x < N; x++) {
                            const dx = x - x_start;
                            const g = pAmps[p] * Math.exp(-dx * dx / (2 * sigma * sigma));
                            if (g < 1e-8) continue;
                            for (let y = y_offsets[p] - 2; y <= y_offsets[p] + 2; y++)
                            for (let z = mid - 2; z <= mid + 2; z++) {
                                if (y < 0 || y >= N || z < 0 || z >= N) continue;
                                this._injectFlux(x, y, z, 0, 0, g);
                                this._injectWaveVel(x, y, z, 0, 0, g); // outgoing +x
                            }
                        }
                    }
                    break;
                }
            }
            return;
        }

        // ── Quantum experiment scenarios ──
        if (name.startsWith('quantum-')) {
            this._initFluxGrid();

            switch (name) {
                case 'quantum-born-rule': {
                    // Random-phase Gaussian flux pulse → Born rule P = |ψ|² statistics
                    const sigma = N / 8;
                    const amp = K_B * 2;
                    const theta = Math.random() * 2 * Math.PI;
                    const pulseR = Math.min(Math.ceil(sigma * 3), mid - 1);
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) {
                            this._injectFlux(mid + dx, mid + dy, mid + dz,
                                val * Math.cos(theta), val * Math.sin(theta), 0);
                        }
                    }
                    this._toggles.genesis = true;
                    break;
                }
                case 'quantum-double-slit': {
                    // Two coherent line sources with genesis → interference + manifestation
                    const sigma = 2;
                    const sAmp = 0.3;
                    const slit_sep = Math.floor(N / 6);
                    const slit_x = Math.floor(N / 4);
                    const slit_ys = [mid - slit_sep, mid + slit_sep];
                    for (const sy of slit_ys) {
                        for (let z = 0; z < N; z++)
                        for (let dy = -4; dy <= 4; dy++)
                        for (let dx = -4; dx <= 4; dx++) {
                            const r2 = dx * dx + dy * dy;
                            const g = sAmp * Math.exp(-r2 / (2 * sigma * sigma));
                            if (g < 1e-6) continue;
                            const px = slit_x + dx, py = sy + dy;
                            if (px < 0 || px >= N || py < 0 || py >= N) continue;
                            this._injectFlux(px, py, z, 0, 0, g);
                            this._injectWaveVel(px, py, z, g, 0, 0); // propagate +x
                        }
                    }
                    this._toggles.genesis = true;
                    this._toggles.coupling = false;
                    break;
                }
                case 'quantum-tunnel': {
                    // Gaussian flux packet → barrier of locked particles → tunneling
                    const sigma = N / 12;
                    const amp = K_B * 2;
                    const packetX = Math.floor(N / 4);
                    const pulseR = Math.min(Math.ceil(sigma * 3), mid - 1);
                    // Gaussian flux packet propagating +x
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) {
                            const x = packetX + dx, y = mid + dy, z = mid + dz;
                            if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) {
                                this._injectFlux(x, y, z, val, 0, 0);
                                this._injectWaveVel(x, y, z, val, 0, 0); // +x propagation
                            }
                        }
                    }
                    // Barrier: locked +1 particles across y-z plane
                    const W = this._quantumBarrierWidth || 3;
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++)
                    for (let dx = 0; dx < W; dx++) {
                        this.injectParticle(mid + dx, y, z, 1);
                        this._particles[this._particles.length - 1].locked = true;
                    }
                    break;
                }
                case 'quantum-well': {
                    // Reflective walls + broadband standing waves → energy quantization
                    const wallA = Math.floor(N / 4);
                    const wallB = Math.floor(3 * N / 4);
                    const boxLength = wallB - wallA;
                    // Reflective walls: locked +1 particles across y-z planes
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++) {
                        this.injectParticle(wallA, y, z, 1);
                        this._particles[this._particles.length - 1].locked = true;
                        this.injectParticle(wallB, y, z, 1);
                        this._particles[this._particles.length - 1].locked = true;
                    }
                    // Broadband flux between walls: modes n=1..8
                    for (let n = 1; n <= 8; n++) {
                        const amp_n = K_B * 0.5 / n;
                        for (let x = wallA + 1; x < wallB; x++)
                        for (let y = 0; y < N; y++)
                        for (let z = 0; z < N; z++) {
                            const val = amp_n * Math.sin(n * Math.PI * (x - wallA) / boxLength);
                            if (Math.abs(val) > 1e-6) {
                                this._injectFlux(x, y, z, 0, val, 0);
                            }
                        }
                    }
                    this._toggles.genesis = false;
                    this._toggles.damping = false;
                    break;
                }
                case 'quantum-entangle': {
                    // Super-threshold flux burst → pair genesis + correlation tracking
                    const bigAmp = K_GENESIS * 5;
                    for (let dz = -4; dz <= 4; dz++)
                    for (let dy = -4; dy <= 4; dy++)
                    for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = bigAmp * Math.exp(-r2 / (2 * 6));
                        if (val > 0.001) {
                            this._injectFlux(mid + dx, mid + dy, mid + dz, val, val, val);
                        }
                    }
                    this._toggles.genesis = true;
                    this._quantumExperimentMode = 'entangle';
                    break;
                }
                case 'quantum-aharonov-bohm': {
                    // Solenoid flux tube + two packets passing on opposite sides
                    const R = Math.floor(N / 8);
                    // Confined flux tube along z at center (solenoid)
                    for (let z = 0; z < N; z++)
                    for (let dy = -R; dy <= R; dy++)
                    for (let dx = -R; dx <= R; dx++) {
                        if (dx * dx + dy * dy > R * R) continue;
                        this._injectFlux(mid + dx, mid + dy, z, 0, 0, K_B * 0.5);
                    }
                    // Packet A: above solenoid, propagating +x
                    const pSigma = 3;
                    const pAmp = K_B * 2;
                    const pStartX = Math.floor(N / 4);
                    for (let dz = -pSigma; dz <= pSigma; dz++)
                    for (let dy = -pSigma; dy <= pSigma; dy++)
                    for (let dx = -pSigma; dx <= pSigma; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = pAmp * Math.exp(-r2 / (2 * pSigma * pSigma));
                        if (val > 0.001) {
                            // Packet A: y = mid + R + 2
                            const ayPos = mid + R + 2 + dy;
                            if (pStartX + dx >= 0 && pStartX + dx < N && ayPos >= 0 && ayPos < N && mid + dz >= 0 && mid + dz < N) {
                                this._injectFlux(pStartX + dx, ayPos, mid + dz, val, 0, 0);
                                this._injectWaveVel(pStartX + dx, ayPos, mid + dz, val, 0, 0);
                            }
                            // Packet B: y = mid - R - 2
                            const byPos = mid - R - 2 + dy;
                            if (pStartX + dx >= 0 && pStartX + dx < N && byPos >= 0 && byPos < N && mid + dz >= 0 && mid + dz < N) {
                                this._injectFlux(pStartX + dx, byPos, mid + dz, val, 0, 0);
                                this._injectWaveVel(pStartX + dx, byPos, mid + dz, val, 0, 0);
                            }
                        }
                    }
                    break;
                }
                case 'quantum-casimir': {
                    // Two parallel plates + vacuum fluctuation noise → Casimir effect
                    const d = this._quantumCasimirSep || 6;
                    const plateA = mid - Math.floor(d / 2);
                    const plateB = mid + Math.floor(d / 2);
                    // Locked +1 particles forming two plates across y-z
                    for (let y = 0; y < N; y++)
                    for (let z = 0; z < N; z++) {
                        this.injectParticle(plateA, y, z, 1);
                        this._particles[this._particles.length - 1].locked = true;
                        this.injectParticle(plateB, y, z, 1);
                        this._particles[this._particles.length - 1].locked = true;
                    }
                    // Fill entire lattice with low-amplitude random flux (vacuum foam)
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        this._injectFlux(x, y, z,
                            (Math.random() - 0.5) * K_B * 0.3,
                            (Math.random() - 0.5) * K_B * 0.3,
                            (Math.random() - 0.5) * K_B * 0.3);
                    }
                    this._toggles.genesis = false;
                    break;
                }
                case 'quantum-zeno': {
                    // Near-threshold flux → genesis + frequent measurement suppresses decay
                    const sigma = N / 10;
                    const amp = K_GENESIS * 1.2;
                    const pulseR = Math.min(Math.ceil(sigma * 3), mid - 1);
                    for (let dz = -pulseR; dz <= pulseR; dz++)
                    for (let dy = -pulseR; dy <= pulseR; dy++)
                    for (let dx = -pulseR; dx <= pulseR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) {
                            this._injectFlux(mid + dx, mid + dy, mid + dz, val, val, val);
                        }
                    }
                    this._toggles.genesis = true;
                    this._quantumZenoInterval = this._quantumZenoInterval || 10;
                    this._quantumZenoMode = true;
                    break;
                }
            }
            return;
        }

        // ── Standard Model scenarios ──
        if (name.startsWith('sm-')) {
            this._initFluxGrid();
            const amp = K_B * 2;

            switch (name) {

                // ────────────────────────────────────────────────────
                // SM Particle Zoo: all 17 fundamental particles
                // arranged as a 3D Standard Model table
                // ────────────────────────────────────────────────────
                case 'sm-particle-zoo': {
                    const sp = Math.max(3, Math.floor(N / 7)); // spacing between particles
                    const ox = mid - sp * 3; // left edge of the table

                    // Helper: place a particle with flux dressing scaled by log(mass)
                    const placeParticle = (gx, gy, gz, state, spin, color, massMeV, label) => {
                        const x = Math.min(Math.max(gx, 2), N - 3);
                        const y = Math.min(Math.max(gy, 2), N - 3);
                        const z = Math.min(Math.max(gz, 2), N - 3);

                        if (state !== 0) {
                            this.injectParticle(x, y, z, state);
                            const idx = this._particles.length - 1;
                            this._particles[idx].spin = spin;
                            this._particles[idx].color = color;
                        }

                        // Gaussian flux dressing: radius ~ log(mass/m_e + 1)
                        const dressR = Math.max(1, Math.min(Math.floor(1.5 + Math.log10(massMeV / 0.511 + 1) * 1.2), sp - 1));
                        const dressAmp = amp * (0.3 + 0.7 * Math.min(massMeV / 1000, 1));
                        for (let dz = -dressR; dz <= dressR; dz++)
                        for (let dy = -dressR; dy <= dressR; dy++)
                        for (let dx = -dressR; dx <= dressR; dx++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            if (r2 > dressR * dressR) continue;
                            const val = dressAmp * Math.exp(-r2 / (2 * Math.max(1, dressR * 0.5) ** 2));
                            if (val > 0.001) this._injectFlux(x + dx, y + dy, z + dz, val * 0.5, val * 0.3, val * 0.2);
                        }
                    };

                    // Row 1: Quarks (y = mid + sp) — 3 generations × 2 flavors
                    const qy = mid + sp;
                    placeParticle(ox + sp * 0, qy, mid, +1, 1, 1, 2.16,    'u');  // up (red)
                    placeParticle(ox + sp * 0, qy, mid + sp, +1, 1, 2, 4.67, 'd');  // down (green)
                    placeParticle(ox + sp * 2, qy, mid, +1, 1, 2, 1270,    'c');  // charm (green)
                    placeParticle(ox + sp * 2, qy, mid + sp, +1, 1, 3, 93.4, 's');  // strange (blue)
                    placeParticle(ox + sp * 4, qy, mid, +1, 1, 3, 172760,  't');  // top (blue)
                    placeParticle(ox + sp * 4, qy, mid + sp, +1, 1, 1, 4180, 'b');  // bottom (red)

                    // Row 2: Leptons (y = mid) — 3 generations
                    placeParticle(ox + sp * 0, mid, mid, -1, -1, 0, 0.511,   'e');   // electron
                    placeParticle(ox + sp * 0, mid, mid + sp, 0, 1, 0, 0.000004, 'νe'); // e-neutrino (ghost)
                    placeParticle(ox + sp * 2, mid, mid, -1, -1, 0, 105.66,  'μ');   // muon
                    placeParticle(ox + sp * 2, mid, mid + sp, 0, 1, 0, 0.0086, 'νμ'); // μ-neutrino (ghost)
                    placeParticle(ox + sp * 4, mid, mid, -1, -1, 0, 1776.86, 'τ');   // tau
                    placeParticle(ox + sp * 4, mid, mid + sp, 0, 1, 0, 0.0496, 'ντ'); // τ-neutrino (ghost)

                    // Row 3: Gauge bosons + Higgs (y = mid - sp)
                    const by = mid - sp;
                    // Photon: massless flux wave (no particle)
                    for (let dx = -3; dx <= 3; dx++) {
                        const val = amp * 1.5 * Math.cos(dx * Math.PI / 3);
                        this._injectFlux(ox + sp * 0 + dx, by, mid, 0, val, 0);
                    }
                    // Gluon: color flux loop
                    for (let i = 0; i < 8; i++) {
                        const angle = 2 * Math.PI * i / 8;
                        const gx = Math.round(ox + sp * 1 + 2 * Math.cos(angle));
                        const gz = Math.round(mid + 2 * Math.sin(angle));
                        this._injectFlux(gx, by, gz, -Math.sin(angle) * amp, 0, Math.cos(angle) * amp);
                    }
                    // W+ boson
                    placeParticle(ox + sp * 2, by, mid, +1, 0, 0, 80377, 'W+');
                    // W- boson
                    placeParticle(ox + sp * 3, by, mid, -1, 0, 0, 80377, 'W-');
                    // Z boson
                    placeParticle(ox + sp * 4, by, mid, 0, 0, 0, 91188, 'Z');
                    // Inject flux for Z since state=0
                    for (let dz = -2; dz <= 2; dz++) for (let dy = -2; dy <= 2; dy++) for (let dx = -2; dx <= 2; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 3 * Math.exp(-r2 / (2 * 2));
                        if (val > 0.01) this._injectFlux(ox + sp * 4 + dx, by + dy, mid + dz, val, val, val);
                    }

                    // HIGGS BOSON: central golden sphere with strong isotropic flux
                    const hx = ox + sp * 5, hy = by, hz = mid;
                    const higgsDress = Math.max(3, Math.floor(N / 8));
                    const higgsAmp = amp * 4;
                    for (let dz = -higgsDress; dz <= higgsDress; dz++)
                    for (let dy = -higgsDress; dy <= higgsDress; dy++)
                    for (let dx = -higgsDress; dx <= higgsDress; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > higgsDress * higgsDress) continue;
                        const val = higgsAmp * Math.exp(-r2 / (2 * (higgsDress * 0.4) ** 2));
                        if (val > 0.001) {
                            // Isotropic (scalar) flux — equal in all 3 components
                            this._injectFlux(hx + dx, hy + dy, hz + dz, val, val, val);
                        }
                    }
                    break;
                }

                // ────────────────────────────────────────────────────
                // Higgs Field: VEV background + localized excitation
                // + test particles showing mass acquisition
                // ────────────────────────────────────────────────────
                case 'sm-higgs-field': {
                    // Uniform VEV background: low-level isotropic flux everywhere
                    const vev = K_B * 0.08; // visible but subtle background
                    for (let z = 0; z < N; z++)
                    for (let y = 0; y < N; y++)
                    for (let x = 0; x < N; x++) {
                        this._injectFlux(x, y, z, vev, vev, vev);
                    }

                    // Central Higgs boson: bright isotropic excitation above VEV
                    const hR = Math.max(3, Math.floor(N / 7));
                    const hAmp = K_B * 8;
                    for (let dz = -hR; dz <= hR; dz++)
                    for (let dy = -hR; dy <= hR; dy++)
                    for (let dx = -hR; dx <= hR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > hR * hR) continue;
                        const val = hAmp * Math.exp(-r2 / (2 * (hR * 0.35) ** 2));
                        if (val > 0.01) this._injectFlux(mid + dx, mid + dy, mid + dz, val, val, val);
                    }

                    // Test particles at cardinal directions showing mass coupling
                    const testR = Math.floor(N / 3);

                    // +X: Electron (light mass, small dressing)
                    this.injectParticle(mid + testR, mid, mid, -1);
                    const eDress = 2;
                    for (let d = -eDress; d <= eDress; d++) for (let dy = -eDress; dy <= eDress; dy++) for (let dx = -eDress; dx <= eDress; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = K_B * 1.5 * Math.exp(-r2 / (2 * 1.5));
                        if (val > 0.01) this._injectFlux(mid + testR + dx, mid + dy, mid + d, val, 0, 0);
                    }

                    // -X: W boson (heavy mass, large dressing)
                    this.injectParticle(mid - testR, mid, mid, +1);
                    const wDress = Math.max(3, Math.floor(N / 8));
                    for (let dz = -wDress; dz <= wDress; dz++) for (let dy = -wDress; dy <= wDress; dy++) for (let dx = -wDress; dx <= wDress; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = K_B * 5 * Math.exp(-r2 / (2 * (wDress * 0.4) ** 2));
                        if (val > 0.01) this._injectFlux(mid - testR + dx, mid + dy, mid + dz, val, val * 0.5, 0);
                    }

                    // +Z: Photon (massless, pure wave — no dressing, just propagating flux)
                    for (let dx = -5; dx <= 5; dx++) {
                        const val = amp * 2 * Math.cos(dx * Math.PI / 5);
                        this._injectFlux(mid + dx, mid, mid + testR, 0, val, 0);
                    }

                    // -Z: Neutrino (ghost — sub-threshold flux, no manifested particle)
                    const nuR = 2;
                    for (let dz = -nuR; dz <= nuR; dz++) for (let dy = -nuR; dy <= nuR; dy++) for (let dx = -nuR; dx <= nuR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = K_B * 0.01 * Math.exp(-r2 / (2 * 1));
                        if (val > 0.0001) this._injectFlux(mid + dx, mid + dy, mid - testR + dz, val, val, val);
                    }
                    break;
                }

                // ────────────────────────────────────────────────────
                // Higgs Mechanism: symmetry breaking visualization
                // Mexican hat → W/Z gain mass, photon stays massless
                // ────────────────────────────────────────────────────
                case 'sm-higgs-mechanism': {
                    // Mexican hat potential: toroidal flux ring
                    const torusR = Math.max(4, Math.floor(N / 5));  // major radius
                    const tubeR = Math.max(2, Math.floor(N / 12));  // tube radius
                    const torusAmp = K_B * 6;

                    // Build torus in xz-plane at y=mid
                    for (let dz = -torusR - tubeR; dz <= torusR + tubeR; dz++)
                    for (let dy = -tubeR; dy <= tubeR; dy++)
                    for (let dx = -torusR - tubeR; dx <= torusR + tubeR; dx++) {
                        const distFromRing = Math.sqrt(
                            (Math.sqrt(dx * dx + dz * dz) - torusR) ** 2 + dy * dy
                        );
                        if (distFromRing <= tubeR) {
                            const val = torusAmp * Math.exp(-distFromRing * distFromRing / (2 * (tubeR * 0.5) ** 2));
                            if (val > 0.01) {
                                this._injectFlux(mid + dx, mid + dy, mid + dz, val, val, val);
                            }
                        }
                    }

                    // Higgs excitation at center of torus (the "chosen" vacuum)
                    const hcR = Math.max(2, Math.floor(tubeR * 1.5));
                    for (let dz = -hcR; dz <= hcR; dz++)
                    for (let dy = -hcR; dy <= hcR; dy++)
                    for (let dx = -hcR; dx <= hcR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = K_B * 10 * Math.exp(-r2 / (2 * (hcR * 0.4) ** 2));
                        if (val > 0.01) this._injectFlux(mid + dx, mid + dy, mid + dz, val, val, val);
                    }

                    // 4 gauge bosons at compass points ON the torus ring:
                    // W+ (heavy, +x)
                    this.injectParticle(mid + torusR, mid, mid, +1);
                    const mwIdx1 = this._particles.length - 1;
                    this._particles[mwIdx1].spin = 1;
                    // W- (heavy, -x)
                    this.injectParticle(mid - torusR, mid, mid, -1);
                    const mwIdx2 = this._particles.length - 1;
                    this._particles[mwIdx2].spin = 1;
                    // Z (heavy, +z) — neutral, use flux only
                    for (let d = -2; d <= 2; d++) for (let dy = -2; dy <= 2; dy++) for (let dx = -2; dx <= 2; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = K_B * 4 * Math.exp(-r2 / 2);
                        if (val > 0.01) this._injectFlux(mid + dx, mid + dy, mid + torusR + d, val, val, val);
                    }
                    // Photon (massless, -z) — propagating wave, escapes the hat
                    for (let dx = -6; dx <= 6; dx++) {
                        const val = amp * 2 * Math.cos(dx * Math.PI / 6);
                        this._injectFlux(mid + dx, mid, mid - torusR, 0, val, 0);
                        this._injectFlux(mid + dx, mid, mid - torusR - 2, 0, val * 0.7, 0);
                        this._injectFlux(mid + dx, mid, mid - torusR - 4, 0, val * 0.4, 0);
                    }
                    break;
                }

                // ────────────────────────────────────────────────────
                // Electroweak: beta decay d → u + W⁻ → e⁻ + ν̄_e
                // ────────────────────────────────────────────────────
                case 'sm-electroweak': {
                    const ewOff = Math.floor(N / 4);

                    // Neutron (3 quarks: u + d + d) at center-left
                    // d-quark that will decay
                    this.injectParticle(mid - ewOff, mid, mid, +1); // d-quark
                    const dIdx = this._particles.length - 1;
                    this._particles[dIdx].color = 1; // red
                    this._particles[dIdx].spin = 1;
                    // Other neutron quarks (locked, spectators)
                    this.injectParticle(mid - ewOff, mid + 3, mid, +1); // u-quark
                    const uIdx1 = this._particles.length - 1;
                    this._particles[uIdx1].color = 2; // green
                    this._particles[uIdx1].locked = true;
                    this.injectParticle(mid - ewOff, mid - 3, mid, +1); // d-quark
                    const dIdx2 = this._particles.length - 1;
                    this._particles[dIdx2].color = 3; // blue
                    this._particles[dIdx2].locked = true;

                    // W⁻ boson propagating rightward (heavy flux pulse)
                    const wAmp = K_B * 6;
                    const wR = Math.max(2, Math.floor(N / 10));
                    for (let dz = -wR; dz <= wR; dz++)
                    for (let dy = -wR; dy <= wR; dy++)
                    for (let dx = -wR; dx <= wR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = wAmp * Math.exp(-r2 / (2 * (wR * 0.5) ** 2));
                        if (val > 0.01) this._injectFlux(mid + dx, mid + dy, mid + dz, val, 0, 0);
                    }

                    // Decay products at right: electron + antineutrino
                    this.injectParticle(mid + ewOff, mid + 2, mid, -1); // electron
                    const eIdx = this._particles.length - 1;
                    this._particles[eIdx].spin = -1;
                    // Antineutrino: sub-threshold ghost flux only
                    const nuFlux = K_B * 0.05;
                    for (let d = -1; d <= 1; d++) for (let dy = -1; dy <= 1; dy++) for (let dx = -1; dx <= 1; dx++) {
                        this._injectFlux(mid + ewOff + dx, mid - 2 + dy, mid + d, nuFlux, nuFlux, nuFlux);
                    }

                    // Flux dressing on quarks
                    for (let d = -3; d <= 3; d++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = K_B * 1.5 * Math.exp(-r2 / (2 * 4));
                        if (val > 0.01) this._injectFlux(mid - ewOff + dx, mid + dy, mid + d, val * 0.3, val * 0.3, val * 0.3);
                    }
                    break;
                }

                // ────────────────────────────────────────────────────
                // Three Generations: e/μ/τ families with mass hierarchy
                // ────────────────────────────────────────────────────
                case 'sm-three-generations': {
                    const genSp = Math.floor(N / 4); // spacing between generations
                    const MU_RATIO = 207;
                    const TAU_RATIO = 3477;

                    // Dressing radius = log-scaled so tau doesn't eat the lattice
                    const eDressR = 2;
                    const muDressR = Math.min(Math.floor(eDressR + Math.log10(MU_RATIO) * 2), Math.floor(N / 6));
                    const tauDressR = Math.min(Math.floor(eDressR + Math.log10(TAU_RATIO) * 2), Math.floor(N / 5));

                    // Generation 1: electron + ν_e
                    const g1x = mid - genSp;
                    this.injectParticle(g1x, mid, mid, -1); // electron
                    this._particles[this._particles.length - 1].spin = -1;
                    for (let dz = -eDressR; dz <= eDressR; dz++) for (let dy = -eDressR; dy <= eDressR; dy++) for (let dx = -eDressR; dx <= eDressR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = K_B * 2 * Math.exp(-r2 / (2 * 1.5));
                        if (val > 0.01) this._injectFlux(g1x + dx, mid + dy, mid + dz, val, 0, 0);
                    }
                    // ν_e ghost
                    this._injectFlux(g1x, mid + 4, mid, K_B * 0.01, K_B * 0.01, K_B * 0.01);

                    // Generation 2: muon + ν_μ
                    const g2x = mid;
                    this.injectParticle(g2x, mid, mid, -1); // muon
                    this._particles[this._particles.length - 1].spin = -1;
                    for (let dz = -muDressR; dz <= muDressR; dz++) for (let dy = -muDressR; dy <= muDressR; dy++) for (let dx = -muDressR; dx <= muDressR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > muDressR * muDressR) continue;
                        const val = K_B * 3 * Math.exp(-r2 / (2 * (muDressR * 0.4) ** 2));
                        if (val > 0.01) this._injectFlux(g2x + dx, mid + dy, mid + dz, val, val * 0.3, 0);
                    }
                    // ν_μ ghost
                    this._injectFlux(g2x, mid + 4, mid, K_B * 0.02, K_B * 0.02, K_B * 0.02);

                    // Generation 3: tau + ν_τ
                    const g3x = mid + genSp;
                    this.injectParticle(g3x, mid, mid, -1); // tau
                    this._particles[this._particles.length - 1].spin = -1;
                    for (let dz = -tauDressR; dz <= tauDressR; dz++) for (let dy = -tauDressR; dy <= tauDressR; dy++) for (let dx = -tauDressR; dx <= tauDressR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 > tauDressR * tauDressR) continue;
                        const val = K_B * 5 * Math.exp(-r2 / (2 * (tauDressR * 0.4) ** 2));
                        if (val > 0.01) this._injectFlux(g3x + dx, mid + dy, mid + dz, val, val * 0.5, val * 0.2);
                    }
                    // ν_τ ghost
                    this._injectFlux(g3x, mid + 4, mid, K_B * 0.03, K_B * 0.03, K_B * 0.03);
                    break;
                }

                // ────────────────────────────────────────────────────
                // QCD Vacuum: gluon field + confined quarks + sea pairs
                // ────────────────────────────────────────────────────
                case 'sm-qcd-vacuum': {
                    // Dense random gluon field (color flux)
                    const gluonAmp = K_B * 0.3;
                    const rng = (seed) => {
                        let s = seed;
                        return () => { s = (s * 1103515245 + 12345) & 0x7fffffff; return s / 0x7fffffff; };
                    };
                    const rand = rng(42);
                    for (let z = 1; z < N - 1; z += 2)
                    for (let y = 1; y < N - 1; y += 2)
                    for (let x = 1; x < N - 1; x += 2) {
                        const fx = (rand() - 0.5) * gluonAmp;
                        const fy = (rand() - 0.5) * gluonAmp;
                        const fz = (rand() - 0.5) * gluonAmp;
                        this._injectFlux(x, y, z, fx, fy, fz);
                    }

                    // 3 quarks in equilateral triangle (red, green, blue)
                    const qR = Math.max(3, Math.floor(N / 5));
                    for (let k = 0; k < 3; k++) {
                        const angle = 2 * Math.PI * k / 3;
                        const qx = Math.round(mid + qR * Math.cos(angle));
                        const qz = Math.round(mid + qR * Math.sin(angle));
                        this.injectParticle(qx, mid, qz, +1);
                        const qi = this._particles.length - 1;
                        this._particles[qi].color = k + 1; // 1=red, 2=green, 3=blue
                        this._particles[qi].spin = (k === 0) ? 1 : -1;
                        // Centripetal velocity for orbiting
                        this._particles[qi].vx = -0.04 * Math.sin(angle);
                        this._particles[qi].vz = 0.04 * Math.cos(angle);
                    }

                    // Color flux tubes (confinement strings) between quark pairs
                    const fluxTubeAmp = K_B * 2;
                    for (let k = 0; k < 3; k++) {
                        const a1 = 2 * Math.PI * k / 3;
                        const a2 = 2 * Math.PI * ((k + 1) % 3) / 3;
                        const x1 = mid + qR * Math.cos(a1), z1 = mid + qR * Math.sin(a1);
                        const x2 = mid + qR * Math.cos(a2), z2 = mid + qR * Math.sin(a2);
                        const steps = Math.max(5, qR);
                        for (let s = 0; s <= steps; s++) {
                            const t = s / steps;
                            const tx = Math.round(x1 + t * (x2 - x1));
                            const tz = Math.round(z1 + t * (z2 - z1));
                            const dirX = (x2 - x1) / Math.sqrt((x2 - x1) ** 2 + (z2 - z1) ** 2 + 0.01);
                            const dirZ = (z2 - z1) / Math.sqrt((x2 - x1) ** 2 + (z2 - z1) ** 2 + 0.01);
                            this._injectFlux(tx, mid, tz, dirX * fluxTubeAmp, 0, dirZ * fluxTubeAmp);
                            this._injectFlux(tx, mid + 1, tz, dirX * fluxTubeAmp * 0.5, 0, dirZ * fluxTubeAmp * 0.5);
                            this._injectFlux(tx, mid - 1, tz, dirX * fluxTubeAmp * 0.5, 0, dirZ * fluxTubeAmp * 0.5);
                        }
                    }

                    // Sea quark-antiquark pairs (2 pairs)
                    const seaOff = Math.floor(qR * 0.5);
                    this.injectParticle(mid + seaOff, mid + 3, mid + seaOff, +1);
                    this._particles[this._particles.length - 1].color = 1;
                    this.injectParticle(mid + seaOff + 2, mid + 3, mid + seaOff, -1);
                    this._particles[this._particles.length - 1].color = 1;
                    this.injectParticle(mid - seaOff, mid - 3, mid - seaOff, +1);
                    this._particles[this._particles.length - 1].color = 2;
                    this.injectParticle(mid - seaOff - 2, mid - 3, mid - seaOff, -1);
                    this._particles[this._particles.length - 1].color = 2;
                    break;
                }
            }
            return;
        }

        // Legacy scenario redirect (only 'empty' kept)
        if (name === 'empty') return;
    }
}

// ── WASM Bridge ────────────────────────────────────────────────────
let _wasmLoadPromise = null; // singleton to prevent duplicate script injection

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
                        script.src = 'wasm/ftd_core.js?v=20260415b';
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

    injectParticle(x, y, z, state) {
        if (this._module && this._bridge)
            this._module.injectParticle(this._bridge, x, y, z, state);
    }

    injectParticleFull(x, y, z, state, spin, color) {
        if (this._module && this._bridge)
            this._module.injectParticleFull(this._bridge, x, y, z, state, spin, color);
    }

    injectWavepacket(x, y, z, state) {
        if (this._module && this._bridge)
            this._module.injectWavepacket(this._bridge, x, y, z, state);
    }

    injectWavepacketFull(x, y, z, state, sigma, amplitude) {
        if (this._module && this._bridge)
            this._module.injectWavepacketFull(this._bridge, x, y, z, state, sigma, amplitude);
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
        if (!this._module || !this._bridge)
            return { positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), count: 0 };
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
    getFluxSlice(axis, index) {
        if (!this._module || !this._bridge) return new Float64Array(0);
        if (typeof this._module.getFluxSlice !== 'function') return new Float64Array(0);
        return this._module.getFluxSlice(this._bridge, axis, index);
    }

    getFluxVolume() {
        if (!this._module || !this._bridge) return new Float64Array(0);
        if (typeof this._module.getFluxVolume !== 'function') return new Float64Array(0);
        return this._module.getFluxVolume(this._bridge);
    }

    // ── Bulk Vector Field Exports (Scale 0 field visualization) ──────
    getEFieldSampled(stride = 2) {
        if (!this._module || !this._bridge) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        if (typeof this._module.getEFieldSampled !== 'function') return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        return this._module.getEFieldSampled(this._bridge, stride);
    }

    getBFieldSampled(stride = 2) {
        if (!this._module || !this._bridge) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        if (typeof this._module.getBFieldSampled !== 'function') return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        return this._module.getBFieldSampled(this._bridge, stride);
    }

    getPoyntingSampled(stride = 2) {
        if (!this._module || !this._bridge) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        if (typeof this._module.getPoyntingSampled !== 'function') return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        return this._module.getPoyntingSampled(this._bridge, stride);
    }

    getDivJSampled(stride = 2) {
        if (!this._module || !this._bridge) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        if (typeof this._module.getDivJSampled !== 'function') return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        return this._module.getDivJSampled(this._bridge, stride);
    }

    getFluxVectorSampled(stride = 2) {
        if (!this._module || !this._bridge) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        if (typeof this._module.getFluxVectorSampled !== 'function') return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        return this._module.getFluxVectorSampled(this._bridge, stride);
    }

    getForceFieldSampled(stride = 2) {
        if (!this._module || !this._bridge) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        if (typeof this._module.getForceFieldSampled !== 'function') return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        return this._module.getForceFieldSampled(this._bridge, stride);
    }

    getGravityFieldSampled(stride = 2) {
        // WASM doesn't have dedicated gravity field export — delegate to _fluxMock if available
        return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
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

// ── Re-exports from extracted modules ────────────────────────────────
// CosmicMockBridge moved to bridge/mock-scale5.js (Scale 5 N-body sim)
// createBridge factory moved to bridge/bridge-factory-dag.js
export { CosmicMockBridge } from './bridge/mock-scale5.js';
export { createBridge } from './bridge/bridge-factory-dag.js';
