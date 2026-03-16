/**
 * WASM Bridge — abstraction layer between UI and simulation engine.
 *
 * Provides a MockBridge for development (no WASM needed) and a WasmBridge
 * for production (loads compiled ftd_core.wasm). The UI code only talks
 * to the Bridge interface, never directly to WASM or mock internals.
 */

import { getById as catalogGetById } from './particle-catalog.js';
import { ALPHA, K_B, K_GENESIS, DAMPING, G_N, C_SPEED, M_PROTON, R_BOHR, N_BASE } from './constants.js';
import { cpkColor, defaultNeutronCount as elemNeutrons, maxBonds as elemMaxBonds } from './elements.js';

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

function computeAtomicProps(Z, N = 0) {
    const mass = Z + N * 1.001;  // Mass in atomic mass units (H≈1, C≈12)
    const z_cbrt = Math.cbrt(Z);
    const radius = z_cbrt > 0 ? 1.0 / z_cbrt : 1.0;  // Bohr-scaled (H=1)
    const vdw_epsilon = AE_EPS_BASE * Math.pow(Z, 2.0 / 3.0);  // Well depth
    const vdw_sigma = radius * N_BASE;  // Electron cloud size (H≈4)
    const max_bonds = elemMaxBonds(Z);
    return { mass, radius, vdw_epsilon, vdw_sigma, max_bonds };
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

        // Mutable simulation parameters (combo panel)
        this._params = { kb: K_B, gn: G_N, damping: DAMPING };

        // Toggle states (mirror engine TermToggles)
        this._toggles = {
            wave_propagation: true, coupling: true, damping: true, genesis: true,
            gauss_projection: true, forces: true, gravity: false, movement: true,
            poisson_coulomb: true, lorentz_force: false, selective_damping: false,
            larmor_radiation: false, dual_substrate: false,
        };

        // Visual settings (shared with viewport for size control)
        this._visualSettings = null;

        // Pre-allocated buffers for getParticleData (reuse across frames to reduce GC)
        this._pdBufCap = 0;
        this._pdPositions = null;
        this._pdColors = null;
        this._pdSizes = null;
    }

    // ── Boundary containment ──────────────────────────────────────────
    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        this._rebuildBoundaryMask();
    }

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

    tick() {
        this._tick++;
        this._physicalTime += this._dt;
        // Tick flux grid (wave equation) — gated by toggle
        if (this._toggles.wave_propagation) this._tickFlux();
        // Genesis: spontaneous pair creation from super-threshold flux
        if (this._toggles.genesis && this._fluxJ) {
            const Ng = this.latticeSize;
            const J = this._fluxJ;
            const maxNewPerTick = 4; // cap to prevent explosion
            let created = 0;
            for (let z = 1; z < Ng - 1 && created < maxNewPerTick; z++) {
                for (let y = 1; y < Ng - 1 && created < maxNewPerTick; y++) {
                    for (let x = 1; x < Ng - 1 && created < maxNewPerTick; x++) {
                        const idx = z * Ng * Ng + y * Ng + x;
                        const jx = J[idx * 3], jy = J[idx * 3 + 1], jz = J[idx * 3 + 2];
                        const mag = Math.sqrt(jx * jx + jy * jy + jz * jz);
                        if (mag < K_GENESIS) continue;

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
                if (p.state === 0) continue;
                p.vx += 0.5 * p.ax;
                p.vy += 0.5 * p.ay;
                p.vz += 0.5 * p.az;
            }
        }

        // Step 2: Drift — x += v * dt
        if (!this._toggles.movement) { /* skip position integration */ }
        else for (const p of ps) {
            if (p.state === 0) continue;
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
                if (p.state === 0) continue;
                p.vx += 0.5 * p.ax;
                p.vy += 0.5 * p.ay;
                p.vz += 0.5 * p.az;
            }
        }

        // Annihilation: +1 and -1 within close range → both become void + flux burst
        for (let i = 0; i < ps.length; i++) {
            if (ps[i].state === 0) continue;
            for (let j = i + 1; j < ps.length; j++) {
                if (ps[j].state === 0 || ps[i].state === ps[j].state) continue;
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

        // Remove dead particles
        // LOW-1 fix: In-place filter to avoid allocating new array every tick
        const kbThreshold = this._params.kb * 0.01;
        let alive = 0;
        for (let i = 0; i < ps.length; i++) {
            if (ps[i].state !== 0 && ps[i].density > kbThreshold)
                ps[alive++] = ps[i];
        }
        ps.length = alive;
    }

    // MED-1: Extracted force computation for Velocity Verlet (called twice per tick)
    _computePairwiseForces(ps, N, halfN, soft, alpha4pi, gn, doGravity) {
        // Zero accelerations
        for (const p of ps) { p.ax = 0; p.ay = 0; p.az = 0; }
        for (let i = 0; i < ps.length; i++) {
            const pi = ps[i];
            if (pi.state === 0) continue;
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
        this._params = { kb: K_B, gn: G_N, damping: DAMPING };
        // Reset toggles to defaults
        this._toggles = {
            wave_propagation: true, coupling: true, damping: true, genesis: true,
            gauss_projection: true, forces: true, gravity: false, movement: true,
            poisson_coulomb: true, lorentz_force: false, selective_damping: false,
            larmor_radiation: false, dual_substrate: false,
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
        for (let i = 0; i < count; i++) {
            const p = this._particles[i];
            positions[i * 3] = p.x;
            positions[i * 3 + 1] = p.y;
            positions[i * 3 + 2] = p.z;
            if (p.state === 1) {
                colors[i * 3] = 0.4; colors[i * 3 + 1] = 0.87; colors[i * 3 + 2] = 0.5;
            } else if (p.state === -1) {
                colors[i * 3] = 0.97; colors[i * 3 + 1] = 0.44; colors[i * 3 + 2] = 0.44;
            } else {
                colors[i * 3] = 0.47; colors[i * 3 + 1] = 0.53; colors[i * 3 + 2] = 0.6;
            }
            const mSize = this._visualSettings ? this._visualSettings.manifestedSize : 12.0;
            const vSize = this._visualSettings ? this._visualSettings.voidSize : 4.0;
            sizes[i] = p.state !== 0 ? mSize : vSize + p.density * 8.0;
        }
        return { positions, colors, sizes, count };
    }

    getDiagnostics() {
        const manifested = this._particles.filter(p => p.state !== 0);
        const positive = manifested.filter(p => p.state === 1).length;
        const negative = manifested.filter(p => p.state === -1).length;

        // Include flux grid energy when available (Scale 0 substrate scenarios)
        let totalFlux = this._particles.reduce((s, p) => s + p.density, 0);
        let fieldEnergy = 0;
        let waveEnergy = 0;
        if (this._fluxJ) {
            const total = this.latticeSize ** 3;
            const J = this._fluxJ;
            const WV = this._fluxWV;
            for (let i = 0; i < total; i++) {
                const jx = J[i * 3], jy = J[i * 3 + 1], jz = J[i * 3 + 2];
                fieldEnergy += jx * jx + jy * jy + jz * jz;
                const wx = WV[i * 3], wy = WV[i * 3 + 1], wz = WV[i * 3 + 2];
                waveEnergy += wx * wx + wy * wy + wz * wz;
            }
            fieldEnergy *= 0.5;
            waveEnergy *= 0.5;
            totalFlux = Math.sqrt(fieldEnergy * 2);  // RMS flux magnitude
        }

        const totalEnergy = fieldEnergy + waveEnergy;
        return {
            tick: this._tick, physicalTime: this._physicalTime, dt: this._dt,
            manifested: manifested.length, positive, negative,
            totalFlux: +totalFlux.toFixed(4),
            totalEnergy: +totalEnergy.toFixed(4),
            maxBandwidth: 0, avgDrag: 0,
            entropy: totalEnergy > 0 ? Math.log(totalEnergy + 1) : 0,
            chargeBalance: positive - negative,
            spinUp: 0, spinDown: 0,
            colorless: 0, colorRed: 0, colorGreen: 0, colorBlue: 0,
            angMomX: 0, angMomY: 0, angMomZ: 0
        };
    }

    getEnergyAudit() {
        let fieldEnergy = 0, waveEnergy = 0;
        if (this._fluxJ) {
            const total = this.latticeSize ** 3;
            const J = this._fluxJ, WV = this._fluxWV;
            for (let i = 0; i < total; i++) {
                const jx = J[i * 3], jy = J[i * 3 + 1], jz = J[i * 3 + 2];
                fieldEnergy += jx * jx + jy * jy + jz * jz;
                const wx = WV[i * 3], wy = WV[i * 3 + 1], wz = WV[i * 3 + 2];
                waveEnergy += wx * wx + wy * wy + wz * wz;
            }
            fieldEnergy *= 0.5;
            waveEnergy *= 0.5;
        }
        return {
            fieldEnergy, waveEnergy, particleKE: 0,
            totalEnergy: fieldEnergy + waveEnergy,
            gaussViolation: 0, maxGaussError: 0, selfFieldInjection: 0,
            coulombPE: 0, chargeTotal: 0, manifested: 0
        };
    }

    getLagrangian() {
        const N = this._particles.filter(p => p.state !== 0).length;
        let fieldEnergy = 0, waveEnergy = 0, totalFluxMag = 0;
        if (this._fluxJ) {
            const total = this.latticeSize ** 3;
            const J = this._fluxJ, WV = this._fluxWV;
            for (let i = 0; i < total; i++) {
                const jx = J[i * 3], jy = J[i * 3 + 1], jz = J[i * 3 + 2];
                const mag2 = jx * jx + jy * jy + jz * jz;
                fieldEnergy += mag2;
                totalFluxMag += Math.sqrt(mag2);
                const wx = WV[i * 3], wy = WV[i * 3 + 1], wz = WV[i * 3 + 2];
                waveEnergy += wx * wx + wy * wy + wz * wz;
            }
            fieldEnergy *= 0.5;
            waveEnergy *= 0.5;
        }
        const dissipation = fieldEnergy * this._params.damping;
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
            ALPHA, ALPHA_INV: 1.0 / ALPHA, G_STAR: 2.9587, K_B, K_GENESIS,
            G_C: Math.sqrt(ALPHA), G_N, DAMPING, C_SPEED,
            N_C: 3, B3: 7, N_BASE: 4, N_EFF: 13, VARPI: 2.622
        };
    }

    inspectVoxel(x, y, z) {
        const p = this._particles.find(p =>
            Math.round(p.x) === x && Math.round(p.y) === y && Math.round(p.z) === z
        );
        if (!p) return null;
        return {
            state: p.state, particleId: p.id, pairId: p.pairId,
            locked: p.locked, spin: p.spin, color: p.color,
            fluxX: 0, fluxY: 0, fluxZ: 0, density: p.density,
            waveVelX: 0, waveVelY: 0, waveVelZ: 0,
            velX: p.vx, velY: p.vy, velZ: p.vz,
            speed: Math.sqrt(p.vx * p.vx + p.vy * p.vy + p.vz * p.vz),
            accelMag: 0, divJ: 0, curlX: 0, curlY: 0, curlZ: 0
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

    _tickFlux() {
        if (!this._fluxJ) return;
        const N = this.latticeSize;
        const c2 = C_SPEED * C_SPEED;
        const damp = this._toggles.damping ? (1.0 - this._params.damping) : 1.0;
        const J = this._fluxJ;
        const WV = this._fluxWV;

        // Leapfrog wave equation: WV += c² * ∇²J; J += WV; J *= damp
        // Inlined index computation avoids 7× _fluxIdx() function calls per voxel
        const NN = N * N;
        for (let z = 0; z < N; z++) {
            const zw = z * NN;
            const zpw = ((z + 1) % N) * NN;
            const zmw = ((z - 1 + N) % N) * NN;
            for (let y = 0; y < N; y++) {
                const yw = y * N;
                const ypw = ((y + 1) % N) * N;
                const ymw = ((y - 1 + N) % N) * N;
                for (let x = 0; x < N; x++) {
                    const xpx = (x + 1) % N;
                    const xmx = (x - 1 + N) % N;
                    const idx = zw + yw + x;
                    const xp = zw + yw + xpx;
                    const xm = zw + yw + xmx;
                    const yp = zw + ypw + x;
                    const ym = zw + ymw + x;
                    const zp = zpw + yw + x;
                    const zm = zmw + yw + x;

                    for (let c = 0; c < 3; c++) {
                        const lap = J[xp * 3 + c] + J[xm * 3 + c] + J[yp * 3 + c] + J[ym * 3 + c] + J[zp * 3 + c] + J[zm * 3 + c] - 6 * J[idx * 3 + c];
                        WV[idx * 3 + c] += c2 * lap;
                    }
                }
            }
        }

        // Commit: J += WV, J *= damp
        const total = N * N * N;
        for (let i = 0; i < total; i++) {
            for (let c = 0; c < 3; c++) {
                J[i * 3 + c] = (J[i * 3 + c] + WV[i * 3 + c]) * damp;
            }
        }

        // Boundary containment: zero flux & wave velocity outside boundary shape
        // Uses precomputed mask to avoid per-voxel _insideBoundary() calls
        if (this._boundaryMask) {
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
        for (let i = 0; i < total; i++) {
            const jx = J[i * 3], jy = J[i * 3 + 1], jz = J[i * 3 + 2];
            M[i] = Math.sqrt(jx * jx + jy * jy + jz * jz);
        }
        this._fluxDirty = false;
    }

    getFluxSlice(axis, index) {
        if (!this._fluxJ) this._initFluxGrid();
        this._updateFluxMag();
        const N = this.latticeSize;
        const data = new Float64Array(N * N);
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

    // Compute pairwise forces (Coulomb + gravity) using Newton's 3rd law (N²/2)
    _peComputeForces() {
        const ps = this._pe.particles;
        const n = ps.length;
        if (!this._pe.forces || this._pe.forces.length !== n) {
            this._pe.forces = new Array(n);
            for (let i = 0; i < n; i++) this._pe.forces[i] = { fx: 0, fy: 0, fz: 0 };
        } else {
            for (let i = 0; i < n; i++) {
                this._pe.forces[i].fx = 0;
                this._pe.forces[i].fy = 0;
                this._pe.forces[i].fz = 0;
            }
        }
        const soft2 = this._pe.soft * this._pe.soft;
        for (let i = 0; i < n; i++) {
            for (let j = i + 1; j < n; j++) {
                const dx = ps[j].x - ps[i].x, dy = ps[j].y - ps[i].y, dz = ps[j].z - ps[i].z;
                const r2 = dx * dx + dy * dy + dz * dz + soft2;
                if (r2 < 1e-40) continue;
                const invR = 1 / Math.sqrt(r2);
                const invR2 = invR * invR;
                const fc = this._pe.coulomb ? -ALPHA * ps[i].charge * ps[j].charge * (invR2 / (4 * Math.PI)) : 0;
                const fg = this._pe.gravity ? G_N * ps[i].mass * ps[j].mass * invR2 : 0;
                const fr = (fc + fg) * invR;
                const ffx = fr * dx, ffy = fr * dy, ffz = fr * dz;
                this._pe.forces[i].fx += ffx;
                this._pe.forces[i].fy += ffy;
                this._pe.forces[i].fz += ffz;
                this._pe.forces[j].fx -= ffx;
                this._pe.forces[j].fy -= ffy;
                this._pe.forces[j].fz -= ffz;
            }
        }
    }

    // Velocity Verlet integrator: half-kick → drift → recompute forces → half-kick
    peTick() {
        if (!this._pe) return;
        const ps = this._pe.particles;
        const dt = this._pe.dt;

        // Ensure forces are initialized
        if (!this._pe.forces || this._pe.forces.length !== ps.length) {
            this._peComputeForces();
        }

        // Half-kick: v += (F/m) × dt/2
        for (let i = 0; i < ps.length; i++) {
            const p = ps[i];
            if (p.locked) continue;
            const f = this._pe.forces[i];
            const hdt = dt * 0.5 / p.mass;
            p.vx += f.fx * hdt;
            p.vy += f.fy * hdt;
            p.vz += f.fz * hdt;
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
        for (let i = 0; i < ps.length; i++) {
            const p = ps[i];
            if (p.locked) continue;
            const f = this._pe.forces[i];
            const hdt = dt * 0.5 / p.mass;
            p.vx += f.fx * hdt;
            p.vy += f.fy * hdt;
            p.vz += f.fz * hdt;
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
        const positions = new Float32Array(count * 3);
        const colors = new Float32Array(count * 3);
        const sizes = new Float32Array(count);
        const charges = new Int8Array(count);
        const ids = new Int32Array(count);
        const velocities = new Float32Array(count * 3);
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
        const positions = new Float32Array(n * 3);
        const charges = new Float32Array(n);
        const masses = new Float32Array(n);
        for (let i = 0; i < n; i++) {
            positions[i * 3] = ps[i].x;
            positions[i * 3 + 1] = ps[i].y;
            positions[i * 3 + 2] = ps[i].z;
            charges[i] = ps[i].charge;
            masses[i] = ps[i].mass;
        }
        return { positions, charges, masses, count: n };
    }

    peGetForces() {
        if (!this._pe || !this._pe.forces) return { positions: new Float32Array(0), forces: new Float32Array(0), count: 0, maxForce: 0 };
        const ps = this._pe.particles;
        const fs = this._pe.forces;
        const n = ps.length;
        const positions = new Float32Array(n * 3);
        const forces = new Float32Array(n * 3);
        let maxF = 0;
        for (let i = 0; i < n; i++) {
            positions[i * 3] = ps[i].x;
            positions[i * 3 + 1] = ps[i].y;
            positions[i * 3 + 2] = ps[i].z;
            const fx = fs[i].fx, fy = fs[i].fy, fz = fs[i].fz;
            forces[i * 3] = fx;
            forces[i * 3 + 1] = fy;
            forces[i * 3 + 2] = fz;
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
            speed_limit: true  // Speed cap toggle
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

    aeAddAtom(Z, x, y, z, vx = 0, vy = 0, vz = 0, charge = 0, N = -1) {
        if (!this._ae) this.initAE();
        const neutrons = N >= 0 ? N : elemNeutrons(Z);
        const props = computeAtomicProps(Z, neutrons);
        const id = this._ae.nextId++;
        this._ae.atoms.push({
            id, Z, N: neutrons, charge, mass: props.mass, radius: props.radius,
            vdw_epsilon: props.vdw_epsilon, vdw_sigma: props.vdw_sigma,
            max_bonds: props.max_bonds, bonds: [],
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
    /** Build bond lookup structures for O(1) bond checks and partner lookups. */
    _aeBuildBondLookup() {
        const atoms = this._ae.atoms;
        // bondSet: "i-j" keys for O(1) isBonded check
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
            console.log('[FTD aePreBond] skipped — ae:', !!this._ae, 'bonding:', this._ae?.bonding);
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
        console.log(`[FTD aePreBond] ${atoms.length} atoms, ${bondsCreated} bonds created`);
        for (const a of atoms) {
            console.log(`  atom ${a.id} Z=${a.Z} pos=(${a.x.toFixed(2)},${a.y.toFixed(2)},${a.z.toFixed(2)}) bonds=${a.bonds.length}/${a.max_bonds} sigma=${a.vdw_sigma.toFixed(2)}`);
        }
    }

    _aeComputeAllForces() {
        const atoms = this._ae.atoms;
        this._aeBuildBondLookup(); // build O(1) bond lookups before force loop
        const forces = new Array(atoms.length);
        for (let i = 0; i < atoms.length; i++) {
            forces[i] = this._aeComputeForce(i);
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
            console.log(`[FTD aeTick #${tickNum}] dt=${dt} atoms=${atoms.length}`);
            for (let i = 0; i < Math.min(atoms.length, 4); i++) {
                const a = atoms[i], f = forces[i];
                console.log(`  atom ${a.id}: pos=(${a.x.toFixed(3)},${a.y.toFixed(3)},${a.z.toFixed(3)}) vel=(${a.vx.toFixed(4)},${a.vy.toFixed(4)},${a.vz.toFixed(4)}) force=(${f.fx.toFixed(4)},${f.fy.toFixed(4)},${f.fz.toFixed(4)}) bonds=${a.bonds.length}`);
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
                console.log(`  atom ${a.id} after tick: pos=(${a.x.toFixed(3)},${a.y.toFixed(3)},${a.z.toFixed(3)}) vel=(${a.vx.toFixed(4)},${a.vy.toFixed(4)},${a.vz.toFixed(4)})`);
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

        // Auto-bonding
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
                    if (r < 1.2 * sig_avg) {
                        const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0);
                        const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                        const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
                        ai.bonds.push({ partner_id: aj.id, r_eq, k_bond, order: 1 });
                        aj.bonds.push({ partner_id: ai.id, r_eq, k_bond, order: 1 });
                    }
                }
            }
            // Bond breaking
            for (const a of atoms) {
                a.bonds = a.bonds.filter(b => {
                    const partner = atoms.find(at => at.id === b.partner_id);
                    if (!partner) return false;
                    const dx = partner.x - a.x, dy = partner.y - a.y, dz = partner.z - a.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    return r <= 2.0 * b.r_eq;
                });
            }
        }

        this._ae.tick++;
    }

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

        // Bond PE
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

        // Net force magnitude
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
                    // Gaussian pulse at center
                    for (let dz = -6; dz <= 6; dz++) for (let dy = -6; dy <= 6; dy++) for (let dx = -6; dx <= 6; dx++) {
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
                case 'flux-ring': {
                    // Ring of flux injections in XZ plane
                    const radius = Math.floor(N / 4);
                    const nPts = 16;
                    for (let i = 0; i < nPts; i++) {
                        const angle = (2 * Math.PI * i) / nPts;
                        const rx = Math.round(mid + radius * Math.cos(angle));
                        const rz = Math.round(mid + radius * Math.sin(angle));
                        const fx = amp * Math.cos(angle);
                        const fz = amp * Math.sin(angle);
                        this._injectFlux(rx, mid, rz, fx, 0, fz);
                    }
                    break;
                }
                case 'flux-collision': {
                    // Two ±1 particles on collision course with flux dressing
                    const off = Math.floor(N / 3);
                    this.injectParticle(mid - off, mid, mid, 1);
                    this.injectParticle(mid + off, mid, mid, -1);
                    // Give them flux push toward each other
                    for (let d = -3; d <= 3; d++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                        const r2 = dx * dx + dy * dy + d * d;
                        const val = amp * Math.exp(-r2 / (2 * 4));
                        if (val > 0.001) {
                            this._injectFlux(mid - off + dx, mid + dy, mid + d, val, 0, 0);
                            this._injectFlux(mid + off + dx, mid + dy, mid + d, -val, 0, 0);
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
                    this.injectParticle(mid, mid, mid, 1);   // proton (locked)
                    this.injectParticle(mid + 6, mid, mid, -1); // electron
                    // Seed flux as Coulomb-like dressing around proton
                    for (let dz = -5; dz <= 5; dz++) for (let dy = -5; dy <= 5; dy++) for (let dx = -5; dx <= 5; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 === 0 || r2 > 36) continue;
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
                case 'flux-gravity-cluster': {
                    // Many same-sign particles for gravity clustering via density gradient
                    const nParticles = 12;
                    const spread = Math.floor(N / 3);
                    for (let i = 0; i < nParticles; i++) {
                        const px = mid + Math.round((Math.random() - 0.5) * spread);
                        const py = mid + Math.round((Math.random() - 0.5) * spread);
                        const pz = mid + Math.round((Math.random() - 0.5) * spread);
                        this.injectParticle(px, py, pz, 1);
                    }
                    // Seed some background flux
                    for (let dz = -4; dz <= 4; dz++) for (let dy = -4; dy <= 4; dy++) for (let dx = -4; dx <= 4; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = amp * 0.5 * Math.exp(-r2 / (2 * 9));
                        if (val > 0.001) this._injectFlux(mid + dx, mid + dy, mid + dz, val, val * 0.3, 0);
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

                case 'flux-sub-threshold': {
                    // Gaussian pulse at center but BELOW consciousness threshold K_C
                    // Used for threshold-crossing scenario that builds up over time
                    const subAmp = K_B * 0.3;
                    for (let dz = -6; dz <= 6; dz++) for (let dy = -6; dy <= 6; dy++) for (let dx = -6; dx <= 6; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        const val = subAmp * Math.exp(-r2 / (2 * sigma * sigma));
                        if (val > 0.001) this._injectFlux(mid + dx, mid + dy, mid + dz, val, 0, 0);
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
                    // Seed flux dressing around central charge
                    for (let dz = -5; dz <= 5; dz++) for (let dy = -5; dy <= 5; dy++) for (let dx = -5; dx <= 5; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 === 0 || r2 > 36) continue;
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

                case 'flux-force-profile': {
                    // Force profile: single locked +1 charge showing 1/r² field
                    // (from campaign_force_law — see Coulomb field visualization)
                    this.injectParticle(mid, mid, mid, 1);
                    // Seed isotropic Coulomb-like flux dressing (radial, 1/r decay)
                    const fMaxR = Math.floor(N / 3);
                    for (let dz = -fMaxR; dz <= fMaxR; dz++) for (let dy = -fMaxR; dy <= fMaxR; dy++) for (let dx = -fMaxR; dx <= fMaxR; dx++) {
                        const r2 = dx * dx + dy * dy + dz * dz;
                        if (r2 === 0 || r2 > fMaxR * fMaxR) continue;
                        const r = Math.sqrt(r2);
                        // Radial flux decaying as 1/r (so |J| ~ 1/r → force ~ 1/r²)
                        const val = amp * 0.8 / r;
                        this._injectFlux(mid + dx, mid + dy, mid + dz,
                            val * dx / r, val * dy / r, val * dz / r);
                    }
                    break;
                }

                // ── Cosmology scenarios ──
                case 'flux-antimatter': {
                    // 4 matter-antimatter pairs on collision courses
                    const q = Math.floor(N / 4);
                    const pairOffsets = [
                        [[ q,  0,  0], [-q,  0,  0]],  // x-axis pair
                        [[ 0,  q,  0], [ 0, -q,  0]],  // y-axis pair
                        [[ 0,  0,  q], [ 0,  0, -q]],  // z-axis pair
                        [[ q,  q,  0], [-q, -q,  0]],  // diagonal pair
                    ];
                    for (const [posOff, negOff] of pairOffsets) {
                        const px = mid + posOff[0], py = mid + posOff[1], pz = mid + posOff[2];
                        const nx = mid + negOff[0], ny = mid + negOff[1], nz = mid + negOff[2];
                        this.injectParticle(px, py, pz, 1);
                        this.injectParticle(nx, ny, nz, -1);
                        // Flux push toward center
                        for (let dz = -3; dz <= 3; dz++) for (let dy = -3; dy <= 3; dy++) for (let dx = -3; dx <= 3; dx++) {
                            const r2 = dx * dx + dy * dy + dz * dz;
                            const val = amp * Math.exp(-r2 / (2 * 4));
                            if (val > 0.001) {
                                this._injectFlux(px + dx, py + dy, pz + dz, -posOff[0] * val * 0.1, -posOff[1] * val * 0.1, -posOff[2] * val * 0.1);
                                this._injectFlux(nx + dx, ny + dy, nz + dz, -negOff[0] * val * 0.1, -negOff[1] * val * 0.1, -negOff[2] * val * 0.1);
                            }
                        }
                    }
                    break;
                }
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

        // ── Legacy particle scenarios ──
        switch (name) {
            case 'empty': break;
            case 'pair':
                this.injectWavepacket(mid, mid, mid, 1);
                this.injectParticle(mid + 6, mid, mid, -1);
                break;
            case 'production':
                for (let i = 0; i < 5; i++) {
                    this.injectParticle(4 + i, mid, mid, 1);
                    this.injectParticle(N - 5 - i, mid, mid, -1);
                }
                break;
            case 'interference': {
                const q = Math.floor(N / 4);
                this.injectWavepacket(q, q, mid, 1);
                this.injectWavepacket(N - q, q, mid, 1);
                this.injectWavepacket(q, N - q, mid, 1);
                this.injectWavepacket(N - q, N - q, mid, 1);
                break;
            }
            case 'force':
                this.injectWavepacket(mid, mid, mid, 1);
                break;
            case 'hydrogen':
                this.injectParticle(mid, mid, mid, 1);
                this.injectParticle(mid + 8, mid, mid, -1);
                break;
            case 'entangled':
                this.injectParticle(mid, mid, mid, 1);
                this.injectParticle(mid, mid, mid + 1, -1);
                break;
            case 'annihilation':
                this.injectParticle(mid - 3, mid, mid, 1);
                this.injectParticle(mid + 3, mid, mid, -1);
                break;
            case 'triad':
                this.injectWavepacket(mid, mid + 2, mid, 1);
                this.injectWavepacket(mid - 2, mid - 1, mid, 1);
                this.injectWavepacket(mid + 2, mid - 1, mid, 1);
                break;
            case 'dipole':
                this.injectWavepacket(mid - 2, mid, mid, 1);
                this.injectWavepacket(mid + 2, mid, mid, -1);
                break;
            case 'scattering':
                this.injectParticle(mid - 8, mid, mid, 1);
                this.injectParticle(mid + 8, mid, mid, 1);
                break;
            case 'wave':
                break;
            case 'cluster':
                for (let dx = -1; dx <= 1; dx += 2) {
                    for (let dy = -1; dy <= 1; dy += 2) {
                        for (let dz = -1; dz <= 1; dz += 2) {
                            const st = (dx + dy + dz > 0) ? 1 : -1;
                            this.injectWavepacket(mid + dx * 3, mid + dy * 3, mid + dz * 3, st);
                        }
                    }
                }
                break;
            case 'vacuum':
                break;
        }
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
            this._bridge = new this._module.RenderBridge(latticeSize);
            this.ready = true;
            console.log('FTD WASM engine loaded successfully');
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
            if (this._bridge) this._bridge.delete();
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
        return this._module.getParticleData(this._bridge);
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
                gaussViolation: 0, maxGaussError: 0, selfFieldInjection: 0,
                coulombPE: 0, chargeTotal: 0, manifested: 0
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

// ── Factory ────────────────────────────────────────────────────────
export async function createBridge(latticeSize = 32) {
    const wasm = new WasmBridge();
    const ok = await wasm.init(latticeSize);
    if (ok) return wasm;
    const mock = new MockBridge(latticeSize);
    return mock;
}
