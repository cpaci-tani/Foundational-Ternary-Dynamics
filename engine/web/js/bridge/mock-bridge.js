/**
 * @file engine/web/js/bridge/mock-bridge.js
 * @purpose JS-only physics implementation of the ScaleBridge contract.
 *          Owned by `engine/web/js/bridge-init.js` and re-exported there
 *          for backward compatibility.
 * @consumers bridge-init.js (re-exports); each scale controller
 *            (engine/web/js/scales/scaleN/controller.js for N in 0..11)
 *            via the createScale0/1/2Capabilities factories.
 * @contract CONTRACTS.md §1 (Bridge State Contract — live-reference factories)
 *           CONTRACTS.md §2 (Capability Factory Contract)
 * @related engine/include/ftd/render_bridge.h (C++ counterpart)
 *          engine/web/js/bridge/mock-{diagnostics,particle-engine,
 *          lattice-samplers,atom-engine}.js (live-ref factories MockBridge composes)
 *
 * Phase 2a of the refactor sweep extracted MockBridge from bridge-init.js
 * (which was 2395 LOC mixing MockBridge + WasmBridge + capability factories).
 * The extraction is a verbatim move — class body unchanged — so cache
 * invalidation, locked-particle pair forces, absorbing-boundary semantics,
 * and energy-convention guards are bit-identical to the pre-Phase-2 file.
 *
 * Subsequent phases will extract WasmBridge (2b) and the capability factories
 * (2c); after Phase 2c the original bridge-init.js shrinks to a re-export
 * shim. See .claude/plans/i-want-to-try-crispy-charm.md Phase 2.
 */

import { ALPHA, ALPHA_EFT, K_B, K_GENESIS, DAMPING, G_N, G_C, C_SPEED, N_BASE, G_STAR, VARPI, N_C, B_3, N_EFF,
    COULOMB_K_FORCE,
    STRONG_ALPHA_S, STRONG_RUN_COEFF, STRONG_R_COULOMB, STRONG_R_LINEAR,
    STRONG_TRANSITION_DENOM, STRONG_LINEAR_DENOM,
    STRONG_COLOR_REPEL, STRONG_COLOR_ATTRACT } from '../constants.js';

// Hoisted module-scope constant — recomputing K_GENESIS² inside tick() every
// frame is wasteful (L-5 cleanup from AUDIT_LEDGER pre-refactor sweep).
const K_GENESIS_SQ = K_GENESIS * K_GENESIS;

import { insideBoundary, reflectIntoBoundary } from './boundary.js';
// Lattice samplers (17 methods + buildLatencyProxy helper) live in their
// own module as of Wave 1 ticket 2 of the large-file refactor. MockBridge
// holds the object as `this._samplers` and forwards each public sampler
// method through a one-line delegator.
import { createLatticeSamplers } from './mock-lattice-samplers.js';
// Diagnostics + energy-cache helper extracted to its own module as Wave 1
// ticket 3 of the large-file refactor. Factory takes the MockBridge
// instance so cache field writes (_energyCacheTick, _cachedField/Wave/Flux)
// propagate back via the live-reference pattern (CONTRACTS.md §1).
import { createDiagnosticsProvider } from './mock-diagnostics.js';
// Scale-1 Particle Engine (N-body Coulomb + gravity via Velocity Verlet).
// Extracted as Wave 2 ticket 5. Factory takes MockBridge instance so
// state._pe / _peParticleTypes / _peBufs / _peFieldBufs live on the
// bridge and remain accessible to any legacy code that inspects them.
import { createParticleEngine } from './mock-particle-engine.js';
// Scale-2 Atom Engine (ionic + vdW + bond spring + H-bonds + angle strain
// + dipole-dipole + thermostat + auto-bonding with electronegativity).
// Extracted as Wave 2 ticket 6. Factory takes MockBridge instance so
// state._ae / _aeBondSet / _aeIdToIdx / _aeNeighborSets live on the
// bridge and remain visible to any legacy code inspecting them.
import { createAtomEngine } from './mock-atom-engine.js';
// Scenario dispatcher (86 scenarios across flux/qcd/quantum/sm-seed/lhc/field/
// ae-seed/ae-mol groups). Extracted as Wave 3 of the large-file refactor.
// Called via .call(this, name) so every this.reset(), this.injectParticle(...),
// etc., binds back to this MockBridge instance.
import { runSetupScenario } from './scenarios/index.js';
import { MockWaveEngine } from './mock-wave-engine.js';

// ── Mock Bridge ────────────────────────────────────────────────────
/** @implements {import('./bridge-contract.js').ScaleBridge} */
export class MockBridge {
    constructor(latticeSize = 33) {
        // Odd lattices only: an odd N has a true center voxel at (N-1)/2, so
        // point injections + symmetric flux center exactly (no half-voxel
        // straddle / +x−x asymmetry). Snap any even N up to the next odd.
        this.latticeSize = (latticeSize % 2 === 0) ? latticeSize + 1 : latticeSize;

        // Instantiate the wave engine first so getters/setters delegate to it during construction.
        this.waveEngine = new MockWaveEngine(this);

        this._tick = 0;
        this._dt = 1.0;
        this._physicalTime = 0.0;
        this._particles = [];
        this._nextId = 0;
        this.isWasm = false;

        // Boundary containment
        this._boundaryShape = 'cube';
        this._boundaryMask = null; // Uint8Array: 1=inside, 0=outside. Precomputed per shape.
        this._reflectiveBoundary = false; // When false, particles/flux dissipate past boundary (default off)

        // ── Sparse (active-region) wave tick ──
        this._sparseTick = true;
        this._activeBox = { x0: this.latticeSize, x1: -1, y0: this.latticeSize, y1: -1, z0: this.latticeSize, z1: -1 };
        this._activeDense = false;
        this._sparseEps = 0;

        // ── SAB-backed field ──
        this._useSAB = false;
        this._sharedField = null;

        // Mutable simulation parameters (combo panel)
        this._params = { kb: K_B, gn: G_N, damping: DAMPING, omega0: 1.0 };

        // Toggle states
        this._toggles = {
            wave_propagation: true, coupling: true, damping: true, genesis: true,
            gauss_projection: true, forces: true, gravity: false, movement: true,
            poisson_coulomb: true, lorentz_force: false, selective_damping: true,
            larmor_radiation: false, dual_substrate: false, confinement: false,
            weak_transmutation: false,
            color_forces: false, strong_force: false, triad_binding: false,
            pair_production: false, exchange_force: false, latency_field: false,
            de_broglie_clock: false,
        };

        // Visual settings (shared with viewport for size control)
        this._visualSettings = null;

        // Pre-allocated buffers for getParticleData (reuse across frames to reduce GC)
        this._pdBufCap = 0;
        this._pdPositions = null;
        this._pdColors = null;
        this._pdSizes = null;
        this._pdVelocities = null;

        // Per-force decomposition arrays (accumulated in _computePairwiseForces)
        this._forceEM = [];       // per-particle EM force {x,y,z}
        this._forceGravity = [];  // per-particle gravity force {x,y,z}
        this._forceStrong = [];   // per-particle strong/confinement force {x,y,z}

        // Cached energy sums
        this._energyCacheTick = -1;
        this._latencyProxy = null;
        this._latencyProxyTick = -1;
        this._cachedFieldEnergy = 0;
        this._cachedWaveEnergy = 0;
        this._cachedFluxMag = 0;

        // Lattice samplers, diagnostics provider, particle engine, atom engine
        this._samplers = createLatticeSamplers(this);
        this._diagnostics = createDiagnosticsProvider(this);
        this._peEngine = createParticleEngine(this);
        this._aeEngine = createAtomEngine(this);
    }

    get _fluxJ() { return this.waveEngine?._fluxJ; }
    set _fluxJ(v) { if (this.waveEngine) this.waveEngine._fluxJ = v; }
    get _fluxWV() { return this.waveEngine?._fluxWV; }
    set _fluxWV(v) { if (this.waveEngine) this.waveEngine._fluxWV = v; }
    get _fluxMag() { return this.waveEngine?._fluxMag; }
    set _fluxMag(v) { if (this.waveEngine) this.waveEngine._fluxMag = v; }
    get _stateGrid() { return this.waveEngine?._stateGrid; }
    set _stateGrid(v) { if (this.waveEngine) this.waveEngine._stateGrid = v; }
    get _sharedField() { return this.waveEngine?._sharedField; }
    set _sharedField(v) { if (this.waveEngine) this.waveEngine._sharedField = v; }
    get _fluxDirty() { return this.waveEngine?._fluxDirty; }
    set _fluxDirty(v) { if (this.waveEngine) this.waveEngine._fluxDirty = v; }
    get _activeBox() { return this.waveEngine?._activeBox; }
    set _activeBox(v) { if (this.waveEngine) this.waveEngine._activeBox = v; }
    get _activeDense() { return this.waveEngine?._activeDense; }
    set _activeDense(v) { if (this.waveEngine) this.waveEngine._activeDense = v; }
    get _sparseTick() { return this.waveEngine?._sparseTick; }
    set _sparseTick(v) { if (this.waveEngine) this.waveEngine._sparseTick = v; }
    get _sparseEps() { return this.waveEngine?._sparseEps; }
    set _sparseEps(v) { if (this.waveEngine) this.waveEngine._sparseEps = v; }
    get _reflectiveBoundary() { return this.waveEngine?._reflectiveBoundary; }
    set _reflectiveBoundary(v) { if (this.waveEngine) this.waveEngine._reflectiveBoundary = v; }
    get _boundaryShape() { return this.waveEngine?._boundaryShape; }
    set _boundaryShape(v) { if (this.waveEngine) this.waveEngine._boundaryShape = v; }
    get _boundaryMask() { return this.waveEngine?._boundaryMask; }
    set _boundaryMask(v) { if (this.waveEngine) this.waveEngine._boundaryMask = v; }

    /**
     * Compute and cache field/wave energy sums from _fluxJ/_fluxWV.
     * Called once per tick at most; subsequent calls return cached values.
     * At L=128 this avoids 3x redundant O(2M) loops per diagnostics frame.
     *
     * Also populates _fluxMag[] in the same pass, so _updateFluxMag() becomes
     * a no-op when the energy cache is fresh. This eliminates a second full-
     * lattice sqrt loop (2M sqrt calls saved at L=128).
     */
    _ensureEnergyCache() { this._diagnostics.ensureEnergyCache(); }

    // ── Boundary containment ──────────────────────────────────────────
    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        this._rebuildBoundaryMask();
    }

    setReflectiveBoundary(on) { this._reflectiveBoundary = !!on; }

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
     * Delegates to the pure module function in bridge/boundary.js.
     */
    _insideBoundary(nx, ny, nz) {
        return insideBoundary(this._boundaryShape, nx, ny, nz);
    }

    /**
     * Reflect a particle/atom back inside the boundary.
     * cx, cy, cz = center; R = half-extent (radius).
     * Modifies the object in-place (x,y,z,vx,vy,vz).
     * Delegates to the pure module function in bridge/boundary.js.
     */
    _reflectIntoBoundary(p, cx, cy, cz, R) {
        reflectIntoBoundary(this._boundaryShape, p, cx, cy, cz, R, this._reflectiveBoundary);
    }

    // Allow real sub-tick dt: a clamp here silently overrode user-set dt and
    // forced the wave-eq leapfrog to run at unit step regardless. CFL is now
    // checked at the top of _tickFlux() with a console.warn if violated.
    setDt(dt) { this._dt = (typeof dt === 'number' && dt > 0) ? dt : 1.0; }
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
        // FTD-0271: de Broglie internal clock with the wave term OFF. Each
        // manifested voxel is the k=0 rest-frame SHO J'' = -omega0^2 J at
        // exactly omega0 (no spatial Laplacian). Mirrors the C++ engine, where
        // the leapfrog runs unconditionally; here _tickFlux only runs with the
        // wave term, so the clock-only case needs its own minimal leapfrog.
        else if (this._toggles.de_broglie_clock) this._tickClockOnly();
        // Genesis: spontaneous pair creation from super-threshold flux
        if (this._toggles.genesis && this._fluxJ) {
            const Ng = this.latticeSize;
            const J = this._fluxJ;
            const maxNewPerTick = 4; // cap to prevent explosion

            // Two-pass to match the C++ render_bridge.cpp parallel-for genesis
            // (no positional bias). The legacy single-pass loop hit
            // maxNewPerTick on the lowest-coord octant first because iteration
            // order is z<y<x ascending — a symmetric flux shell around centre
            // therefore spawned voxels biased toward the (0,0,0) corner.
            //
            // Pass 1: collect all probability-gated candidates.
            const candidates = [];
            for (let z = 1; z < Ng - 1; z++) {
                for (let y = 1; y < Ng - 1; y++) {
                    for (let x = 1; x < Ng - 1; x++) {
                        const idx = z * Ng * Ng + y * Ng + x;
                        const jx = J[idx * 3], jy = J[idx * 3 + 1], jz = J[idx * 3 + 2];
                        const mag2 = jx * jx + jy * jy + jz * jz;
                        if (mag2 < K_GENESIS_SQ) continue;
                        const mag = Math.sqrt(mag2);
                        const p = 1 - Math.exp(-(mag - K_GENESIS) / K_B);
                        if (Math.random() > p) continue;
                        candidates.push({ x, y, z, idx, mag });
                    }
                }
            }

            // Pass 2: partial Fisher-Yates to pick maxNewPerTick uniformly.
            const nPick = Math.min(candidates.length, maxNewPerTick);
            for (let i = 0; i < nPick; i++) {
                const j = i + Math.floor(Math.random() * (candidates.length - i));
                const tmp = candidates[i]; candidates[i] = candidates[j]; candidates[j] = tmp;
            }

            // Pass 3: spawn + drain on the picked candidates.
            for (let i = 0; i < nPick; i++) {
                const c = candidates[i];
                const divJ = this._divergenceAt(c.x, c.y, c.z);
                const state = divJ >= 0 ? 1 : -1;
                this.injectParticle(c.x, c.y, c.z, state);
                const drain = K_B / (c.mag + 1e-20);
                J[c.idx * 3]     *= (1 - drain);
                J[c.idx * 3 + 1] *= (1 - drain);
                J[c.idx * 3 + 2] *= (1 - drain);
            }
        }

        // Tick particles — pairwise forces + Verlet integration
        const N = this.latticeSize;
        const halfN = N / 2;
        const ps = this._particles;
        const doGravity = this._toggles.gravity;
        const doForces = this._toggles.forces;
        const soft = 1.0; // softening length
        // Coulomb prefactor: COULOMB_K_FORCE = ALPHA / (4π) (classical force-law
        // convention; see constants.js). ALPHA = G_C^2 (EFT-derived); two
        // vertices (source + probe) each contribute G_C, so alpha = G_C*G_C.
        const alpha4pi = COULOMB_K_FORCE;
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
            // Boundary containment.
            // Cube/none + reflective ON  → periodic wrap (3-torus topology).
            // Cube/none + reflective OFF → particle exits the lattice and is
            //   marked dead (state=0, density=0); the per-tick filter at the
            //   end of tick() then drops it. This matches the user-facing
            //   "dissipate into the overflow abyss" semantics: a wave or
            //   particle that crosses the edge is GONE, not wrapped back.
            // Non-cube shapes delegate to _reflectIntoBoundary which itself
            //   honors _reflectiveBoundary internally.
            if (this._boundaryShape === 'cube' || this._boundaryShape === 'none') {
                if (this._reflectiveBoundary) {
                    p.x = ((p.x % N) + N) % N;
                    p.y = ((p.y % N) + N) % N;
                    p.z = ((p.z % N) + N) % N;
                } else if (p.x < 0 || p.x >= N || p.y < 0 || p.y >= N || p.z < 0 || p.z >= N) {
                    // Drop out the abyss
                    p.state = 0;
                    p.density = 0;
                }
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
        // Zero accelerations and per-force decomposition arrays
        // Ensure per-force arrays are sized correctly
        while (this._forceEM.length < ps.length) this._forceEM.push({ x: 0, y: 0, z: 0 });
        while (this._forceGravity.length < ps.length) this._forceGravity.push({ x: 0, y: 0, z: 0 });
        while (this._forceStrong.length < ps.length) this._forceStrong.push({ x: 0, y: 0, z: 0 });
        for (let k = 0; k < ps.length; k++) {
            ps[k].ax = 0; ps[k].ay = 0; ps[k].az = 0;
            this._forceEM[k].x = 0; this._forceEM[k].y = 0; this._forceEM[k].z = 0;
            this._forceGravity[k].x = 0; this._forceGravity[k].y = 0; this._forceGravity[k].z = 0;
            this._forceStrong[k].x = 0; this._forceStrong[k].y = 0; this._forceStrong[k].z = 0;
        }
        // NB: do NOT skip locked pi here. The half-kick / drift loops already
        // guard `p.locked` so a locked particle won't move; what we must NOT
        // do is skip the (locked, unlocked) pair entirely, because then the
        // unlocked partner never sees the force from the locked one. Atomic
        // scenarios (s0-seed-hydrogen / -helium / -h2-molecule) lock their
        // proton triads + He nucleus — the orbiting electron must still feel
        // their Coulomb pull. Pre-2026-04-27 this branch read
        // `if (pi.state === 0 || pi.locked) continue;` and silently dropped
        // every locked↔unlocked pair, leaving electrons inert.
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
                // Coulomb (EM)
                const qi = pi.state, qj = pj.state;
                const fCoul = -alpha4pi * qi * qj * invR2 * invR;
                let emx = fCoul * dx, emy = fCoul * dy, emz = fCoul * dz;
                let fx = emx, fy = emy, fz = emz;
                // Accumulate EM component
                this._forceEM[i].x += emx; this._forceEM[i].y += emy; this._forceEM[i].z += emz;
                this._forceEM[j].x -= emx; this._forceEM[j].y -= emy; this._forceEM[j].z -= emz;
                // Gravity — use per-particle masses if available (matches
                // mock-particle-engine.js convention; locked nuclei carry
                // physical density, light electrons fall back to K_B).
                if (doGravity) {
                    const mi = (typeof pi.density === 'number' && pi.density > 0) ? pi.density : K_B;
                    const mj = (typeof pj.density === 'number' && pj.density > 0) ? pj.density : K_B;
                    const fGrav = gn * mi * mj * invR2 * invR;
                    const gx = fGrav * dx, gy = fGrav * dy, gz = fGrav * dz;
                    fx += gx; fy += gy; fz += gz;
                    this._forceGravity[i].x += gx; this._forceGravity[i].y += gy; this._forceGravity[i].z += gz;
                    this._forceGravity[j].x -= gx; this._forceGravity[j].y -= gy; this._forceGravity[j].z -= gz;
                }
                // Strong force: 3-regime model matching C++ engine (render_bridge.cpp)
                //   r < 3:   Coulomb (1/r^2) with running alpha_s
                //   3-8:     transition (1/(3r)) — flux tube stretching
                //   r >= 8:  linear confinement (r/64)
                // Active for ALL pairs when confinement, color_forces, or strong_force is on.
                if (this._toggles.confinement || this._toggles.color_forces || this._toggles.strong_force) {
                    const r = Math.sqrt(r2raw);
                    if (r > 0.5) {
                        // Color factor: +0.5 for same-color (repulsive), -1.0 for different (attractive)
                        const ci = pi.color || 0, cj = pj.color || 0;
                        const cf = (ci > 0 && cj > 0 && ci === cj) ? STRONG_COLOR_REPEL : STRONG_COLOR_ATTRACT;
                        const ALPHA_S = STRONG_ALPHA_S;
                        // Running alpha_s: decreases at short distance (asymptotic freedom)
                        const alpha_s_r = ALPHA_S / (1.0 + STRONG_RUN_COEFF * Math.log(1.0 + r));
                        let F_mag;  // unsigned force magnitude from the regime model
                        if (r < STRONG_R_COULOMB) {
                            F_mag = alpha_s_r / (r * r);                        // Coulomb regime
                        } else if (r < STRONG_R_LINEAR) {
                            F_mag = alpha_s_r / (STRONG_TRANSITION_DENOM * r);  // Transition
                        } else {
                            F_mag = alpha_s_r * r / STRONG_LINEAR_DENOM;        // Linear confinement
                        }
                        // Sign convention matching C++ engine (render_bridge.cpp):
                        //   dx points from i toward j.
                        //   For attraction (cf < 0): force on i should point toward j = +dx direction.
                        //   C++ uses: F = -F_mag * ddx / r (where ddx = source - probe = toward source)
                        //   In JS: dx = pj - pi (toward j). So: F_on_i = +cf * F_mag * dx / r
                        //   cf < 0 → negative → but we want attraction...
                        //   Actually: negate cf for the force direction:
                        //   For different color (cf=-1): attractive → force = +F_mag * dx/r
                        //   For same color (cf=+0.5): repulsive → force = -0.5*F_mag * dx/r
                        const invRs = 1.0 / r;
                        const sx = -cf * F_mag * dx * invRs;
                        const sy = -cf * F_mag * dy * invRs;
                        const sz = -cf * F_mag * dz * invRs;
                        fx += sx; fy += sy; fz += sz;
                        this._forceStrong[i].x += sx; this._forceStrong[i].y += sy; this._forceStrong[i].z += sz;
                        this._forceStrong[j].x -= sx; this._forceStrong[j].y -= sy; this._forceStrong[j].z -= sz;
                    }
                }
                // Newton's 3rd law: equal and opposite
                pi.ax += fx; pi.ay += fy; pi.az += fz;
                pj.ax -= fx; pj.ay -= fy; pj.az -= fz;
            }
        }
    }

    run(n) { for (let i = 0; i < n; i++) this.tick(); }

    // Particle LIST (x,y,z,state,charge,…) — fresh copies so callers can't
    // mutate engine state. Consumers: spectrum-panel, p1-observables-panel,
    // physics-harness.
    getScale0ParticleList() {
        return Array.isArray(this._particles) ? this._particles.map((p) => ({ ...p })) : [];
    }

    reset(latticeSize) {
        this.latticeSize = latticeSize || this.latticeSize;
        if (this.waveEngine) this.waveEngine.latticeSize = this.latticeSize;
        this._tick = 0;
        this._dt = 1.0;
        this._physicalTime = 0.0;
        this._particles = [];
        this._nextId = 0;
        // Reset flux grid
        this._fluxJ = null;
        this._fluxWV = null;
        this._fluxMag = null;
        this._sharedField = null;   // SAB realloc at the (possibly new) size on next _initFluxGrid
        this._fluxDirty = true;
        this._resetActiveBox();   // empty box at the (possibly new) lattice size
        // Release stale auxiliary buffers so they are reallocated at the new size
        this._stateGrid = null;
        this._selectiveDampMask = null;
        this._forceEM = [];
        this._forceGravity = [];
        this._forceStrong = [];
        this._energyCacheTick = -1;
        // Latency proxy is a derived per-tick cache; invalidate on reset so
        // a post-reset scenario load can't accidentally serve the prior
        // scenario's proxy when both happen to sit at _tick === 0.
        this._latencyProxy = null;
        this._latencyProxyTick = -1;
        this._params = { kb: K_B, gn: G_N, damping: DAMPING, omega0: 1.0 };
        // Reset toggles to defaults (must match constructor and config/toggles.js)
        this._toggles = {
            wave_propagation: true, coupling: true, damping: true, genesis: true,
            gauss_projection: true, forces: true, gravity: false, movement: true,
            poisson_coulomb: true, lorentz_force: false, selective_damping: true,
            larmor_radiation: false, dual_substrate: false, confinement: false,
            // weak_transmutation requires dual_substrate (operates on J_L/J_R).
            // Default OFF to satisfy the C++ TermToggles validator and stop
            // the spurious console-error spam on every scenario load.
            weak_transmutation: false,
            color_forces: false, strong_force: false, triad_binding: false,
            pair_production: false, exchange_force: false, latency_field: false,
            // FTD-0271: de Broglie internal clock (KG mass term -omega0^2*J).
            de_broglie_clock: false,
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
    // FTD-0271: de Broglie internal-clock frequency (parity with WasmBridge.setOmega0).
    setOmega0(w) { this._params.omega0 = w; }
    getOmega0() { return this._params.omega0 ?? 1.0; }
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
        this._resetActiveBox();
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
        this._activeDense = true;   // a full random fill is genuinely dense
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
            this._pdVelocities = new Float32Array(this._pdBufCap * 3);
        }
        if (!this._pdVelocities || this._pdVelocities.length < this._pdBufCap * 3) {
            // Back-fill for older mocks where the capacity grew before this
            // field existed — keeps the buffer in lockstep with _pdBufCap.
            this._pdVelocities = new Float32Array(this._pdBufCap * 3);
        }
        const positions = this._pdPositions;
        const colors = this._pdColors;
        const sizes = this._pdSizes;
        const velocities = this._pdVelocities;
        // Only render manifested particles (+1, -1) and high-flux void sites.
        // Skip low-density void particles — they cause white grid artifacts
        // when stacked along camera axes with additive blending.
        let outCount = 0;
        const vs = this._visualSettings || {};
        const posSize = vs.positiveSize ?? 14.0;
        const negSize = vs.negativeSize ?? 10.0;
        const VOID_FLUX_THRESHOLD = 0.05; // only show void sites with significant flux

        for (let i = 0; i < count; i++) {
            const p = this._particles[i];

            // Skip void particles with negligible flux
            if (p.state === 0 && p.density < VOID_FLUX_THRESHOLD) continue;

            positions[outCount * 3] = p.x + 0.5;
            positions[outCount * 3 + 1] = p.y + 0.5;
            positions[outCount * 3 + 2] = p.z + 0.5;
            // Expose velocities so the Kinetic-energy overlay and any other
            // velocity-aware visualizer can read ½|v|² without poking the
            // particle array directly. Fall back to 0 for legacy particles
            // that predate vx/vy/vz (the `|| 0` handles undefined/null/NaN).
            velocities[outCount * 3]     = Number.isFinite(p.vx) ? p.vx : 0;
            velocities[outCount * 3 + 1] = Number.isFinite(p.vy) ? p.vy : 0;
            velocities[outCount * 3 + 2] = Number.isFinite(p.vz) ? p.vz : 0;
            if (p.state === 1) {
                colors[outCount * 3] = 0.4; colors[outCount * 3 + 1] = 0.87; colors[outCount * 3 + 2] = 0.5;
                sizes[outCount] = posSize;
            } else if (p.state === -1) {
                colors[outCount * 3] = 0.97; colors[outCount * 3 + 1] = 0.44; colors[outCount * 3 + 2] = 0.44;
                sizes[outCount] = negSize;
            } else {
                // High-flux void: show as dim blue dot
                colors[outCount * 3] = 0.3; colors[outCount * 3 + 1] = 0.4; colors[outCount * 3 + 2] = 0.6;
                sizes[outCount] = 2.0 + p.density * 6.0;
            }
            outCount++;
        }
        return { positions, colors, sizes, velocities, count: outCount };
    }

    // Diagnostics readouts moved to bridge/mock-diagnostics.js as Wave 1
    // ticket 3. MockBridge forwards via this._diagnostics (constructed in
    // the ctor). See engine/web/docs/INDEX.md for modularization provenance.
    getDiagnostics() { return this._diagnostics.getDiagnostics(); }
    getEnergyAudit() { return this._diagnostics.getEnergyAudit(); }
    getLagrangian()  { return this._diagnostics.getLagrangian(); }

    /**
     * Release the heavy internal buffers and break references that
     * prevent GC. Call from `setFluxMock` (state/store.js) when a
     * previous mock is being replaced — without this, scenario churn
     * leaks the prior MockBridge for the page lifetime (~21 MB at
     * L=96 for `_fluxJ` + `_fluxWV` + `_fluxMag`, plus particle
     * arrays and per-frame _pd* buffers). Idempotent: safe to call
     * twice or on a partially-constructed instance.
     */
    dispose() {
        this._fluxJ = null;
        this._fluxWV = null;
        this._fluxMag = null;
        this._sliceBuf = null;
        this._fluxJ_L = null;
        this._fluxJ_R = null;
        this._fluxWV_L = null;
        this._fluxWV_R = null;
        this._fluxJ_prev = null;
        this._pdPositions = null;
        this._pdColors = null;
        this._pdSizes = null;
        this._pdVelocities = null;
        this._pdBufCap = 0;
        this._particles = [];
        // Lattice-side typed arrays missed by the prior null-sweep
        // (Bridge-M3 audit, 2026-04-27).
        this._stateGrid = null;
        this._selectiveDampMask = null;
        this._boundaryMask = null;
        this._latencyProxy = null;
        this._latencyProxyTick = -1;
        // Diagnostic + sampler factories close over `this` (Bridge-M4
        // audit). Null them to break the cycle so V8 can collect.
        this._diagnostics = null;
        this._samplers = null;
        this._peEngine = null;
        this._aeEngine = null;
        // Drop any harness instance the panels lazy-attached to us.
        // Key string mirrors physics/index.js HARNESS_KEY.
        delete this.__ftdPhysicsHarness__;
    }

    getConstants() {
        return {
            ALPHA, ALPHA_INV: 1.0 / ALPHA, ALPHA_EFT, G_STAR, K_B, K_GENESIS,
            G_C, G_N, DAMPING, C_SPEED,
            N_C, B3: B_3, N_BASE, N_EFF, VARPI
        };
    }

    inspectVoxel(x, y, z) {
        const p = this._particles.find(p =>
            Math.round(p.x) === x && Math.round(p.y) === y && Math.round(p.z) === z
        );
        // Shared field reads — both manifested and void voxels report the
        // same lattice quantities (wave velocity, divergence). MockBridge
        // does not compute curl inline (no _curlAt helper); curl stays 0
        // and is filled by overlay-derived computation in field-overlays.js
        // when needed (L-6 cleanup).
        let fx = 0, fy = 0, fz = 0;
        let wv0 = 0, wv1 = 0, wv2 = 0;
        let divJ = 0;
        if (this._fluxJ) {
            const idx = this._fluxIdx(x, y, z);
            fx = this._fluxJ[idx * 3] || 0;
            fy = this._fluxJ[idx * 3 + 1] || 0;
            fz = this._fluxJ[idx * 3 + 2] || 0;
            if (this._fluxWV) {
                wv0 = this._fluxWV[idx * 3] || 0;
                wv1 = this._fluxWV[idx * 3 + 1] || 0;
                wv2 = this._fluxWV[idx * 3 + 2] || 0;
            }
            if (typeof this._divergenceAt === 'function') {
                divJ = this._divergenceAt(x, y, z) || 0;
            }
        }
        const Emag = Math.sqrt(fx*fx + fy*fy + fz*fz);
        if (p) {
            return {
                state: p.state, particleId: p.id, pairId: p.pairId,
                locked: p.locked, spin: p.spin, color: p.color,
                fluxX: fx, fluxY: fy, fluxZ: fz, density: p.density,
                waveVelX: wv0, waveVelY: wv1, waveVelZ: wv2,
                velX: p.vx, velY: p.vy, velZ: p.vz,
                speed: Math.sqrt(p.vx * p.vx + p.vy * p.vy + p.vz * p.vz),
                accelMag: 0, divJ: divJ, curlX: 0, curlY: 0, curlZ: 0,
                Emag: Emag, Bmag: 0
            };
        }
        return {
            state: 0, particleId: -1, pairId: -1,
            locked: false, spin: 0, color: 0,
            fluxX: fx, fluxY: fy, fluxZ: fz, density: Emag,
            waveVelX: wv0, waveVelY: wv1, waveVelZ: wv2,
            velX: 0, velY: 0, velZ: 0,
            speed: 0, accelMag: 0, divJ: divJ,
            curlX: 0, curlY: 0, curlZ: 0,
            Emag: Emag, Bmag: 0
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
        this.waveEngine._initFluxGrid();
    }

    getSharedField() {
        return this.waveEngine.getSharedField();
    }

    _fluxIdx(x, y, z) {
        return this.waveEngine._fluxIdx(x, y, z);
    }

    _injectFlux(x, y, z, fx, fy, fz) {
        this.waveEngine._injectFlux(x, y, z, fx, fy, fz);
    }

    _injectWaveVel(x, y, z, wx, wy, wz) {
        this.waveEngine._injectWaveVel(x, y, z, wx, wy, wz);
    }

    _resetActiveBox() {
        this.waveEngine._resetActiveBox();
    }

    _expandActiveBox(x, y, z) {
        this.waveEngine._expandActiveBox(x, y, z);
    }

    _growActiveBox() {
        this.waveEngine._growActiveBox();
    }

    _recomputeActiveBox() {
        this.waveEngine._recomputeActiveBox();
    }

    _tickClockOnly() {
        this.waveEngine._tickClockOnly();
    }

    _tickFlux() {
        this.waveEngine._tickFlux();
    }

    _divergenceAt(x, y, z) {
        return this.waveEngine._divergenceAt(x, y, z);
    }

    _updateFluxMag() {
        this.waveEngine._updateFluxMag();
    }

    getFluxSlice(axis, index) {
        return this.waveEngine.getFluxSlice(axis, index);
    }

    getFluxVolume() {
        return this.waveEngine.getFluxVolume();
    }

    // ── Bulk Sampled Vector Field Exports (Scale 0 field visualization) ──
    //
    // All 17 samplers + the latency-proxy helper moved to
    // bridge/mock-lattice-samplers.js. MockBridge forwards via
    // this._samplers (constructed in the ctor above).
    // See engine/web/docs/INDEX.md for the bridge modularization provenance.

    getEFieldSampled(stride = 2)     { return this._samplers.getEFieldSampled(stride); }
    getBFieldSampled(stride = 2)     { return this._samplers.getBFieldSampled(stride); }
    getPoyntingSampled(stride = 2)   { return this._samplers.getPoyntingSampled(stride); }
    getDivJSampled(stride = 2)       { return this._samplers.getDivJSampled(stride); }
    getVorticitySampled(stride = 2)  { return this._samplers.getVorticitySampled(stride); }
    getCurlJSampled(stride = 2)      { return this._samplers.getCurlJSampled(stride); }
    getFluxVectorSampled(stride = 2) { return this._samplers.getFluxVectorSampled(stride); }
    getHelicitySampled(stride = 2)   { return this._samplers.getHelicitySampled(stride); }
    // _buildLatencyProxy retained as a thin forwarder so any external
    // caller that reached for the helper directly (legacy/debug) keeps
    // working. Internal callers (Kretschmann, latency) go through
    // this._samplers.buildLatencyProxy() inside the sampler module.
    _buildLatencyProxy()             { return this._samplers.buildLatencyProxy(); }
    getKretschmannSampled(stride = 2) { return this._samplers.getKretschmannSampled(stride); }
    getLatencySampled(stride = 2)    { return this._samplers.getLatencySampled(stride); }
    getFisherSampled(stride = 2)     { return this._samplers.getFisherSampled(stride); }
    getCoherenceSampled(stride = 2)  { return this._samplers.getCoherenceSampled(stride); }
    getForceFieldSampled(stride = 2) { return this._samplers.getForceFieldSampled(stride); }
    getGravityFieldSampled(stride = 2) { return this._samplers.getGravityFieldSampled(stride); }
    getEMForceField(stride = 2)      { return this._samplers.getEMForceField(stride); }
    getGravityForceField(stride = 2) { return this._samplers.getGravityForceField(stride); }
    getStrongForceField(stride = 2)  { return this._samplers.getStrongForceField(stride); }
    getStateFieldSampled(stride = 2)    { return this._samplers.getStateFieldSampled(stride); }
    getGaussResidualSampled(stride = 1) { return this._samplers.getGaussResidualSampled(stride); }


    // ── ParticleEngine (Scale 1) Mock — extracted to bridge/mock-particle-engine.js
    // Wave 2 ticket 5. All methods forward to this._peEngine, which
    // was constructed in the MockBridge constructor with a live
    // reference to this instance so state mutations still hit _pe,
    // _peParticleTypes, _peBufs, _peFieldBufs on MockBridge directly.
    initPE()                                                         { return this._peEngine.initPE(); }
    resetPE()                                                        { return this._peEngine.resetPE(); }
    peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) { return this._peEngine.peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff); }
    peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff = 0.1) { return this._peEngine.peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff); }
    _peComputeForces()                                               { return this._peEngine._peComputeForces(); }
    peTick()                                                         { return this._peEngine.peTick(); }
    peGetParticleData()                                              { return this._peEngine.peGetParticleData(); }
    peGetFieldSources()                                              { return this._peEngine.peGetFieldSources(); }
    peGetForces()                                                    { return this._peEngine.peGetForces(); }
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
    peSetRadiation(e)                                                { return this._peEngine.peSetRadiation(e); }
    peSetRelativistic(e)                                             { return this._peEngine.peSetRelativistic(e); }
    peSetRelativisticVerlet(e)                                       { return this._peEngine.peSetRelativisticVerlet(e); }
    peGetToggle(name)                                                { return this._peEngine.peGetToggle(name); }
    peGetBackendCapabilities()                                       { return this._peEngine.peGetBackendCapabilities(); }
    peParticleCount()                                                { return this._peEngine.peParticleCount(); }
    peClear()                                                        { return this._peEngine.peClear(); }
    peGetParticleTypes()                                             { return this._peEngine.peGetParticleTypes(); }
    peInspectParticle(id)                                            { return this._peEngine.peInspectParticle(id); }

    // ── AtomEngine (Scale 2) Mock — extracted to bridge/mock-atom-engine.js
    // Wave 2 ticket 6. Every method forwards to this._aeEngine, which
    // was constructed in the MockBridge constructor with a live reference
    // to this instance so state mutations still hit _ae / _aeBondSet /
    // _aeIdToIdx / _aeNeighborSets on MockBridge directly.
    initAE()                                                         { return this._aeEngine.initAE(); }
    resetAE()                                                        { return this._aeEngine.resetAE(); }
    aeAddAtom(Z, x, y, z, vx = 0, vy = 0, vz = 0, charge = 0, N = -1) { return this._aeEngine.aeAddAtom(Z, x, y, z, vx, vy, vz, charge, N); }
    aeAddLockedAtom(Z, x, y, z, charge = 0, N = -1)                   { return this._aeEngine.aeAddLockedAtom(Z, x, y, z, charge, N); }
    aeCreateBond(idA, idB, order = 1)                                 { return this._aeEngine.aeCreateBond(idA, idB, order); }
    _aeBuildBondLookup()                                              { return this._aeEngine._aeBuildBondLookup(); }
    _aeIsBonded(id_a, id_b)                                           { return this._aeEngine._aeIsBonded(id_a, id_b); }
    _aeIs13(i, j)                                                     { return this._aeEngine._aeIs13(i, j); }
    _aeComputeDipoleMoments()                                         { return this._aeEngine._aeComputeDipoleMoments(); }
    _aeComputeForce(i)                                                { return this._aeEngine._aeComputeForce(i); }
    _aeComputeAllForces()                                             { return this._aeEngine._aeComputeAllForces(); }
    aePreBond()                                                       { return this._aeEngine.aePreBond(); }
    aeTick()                                                          { return this._aeEngine.aeTick(); }
    aeGetAtomData()                                                   { return this._aeEngine.aeGetAtomData(); }
    aeGetFieldSources()                                               { return this._aeEngine.aeGetFieldSources(); }
    aeGetDiagnostics()                                                { return this._aeEngine.aeGetDiagnostics(); }
    aeGetForceDecomposition(want)                                     { return this._aeEngine.aeGetForceDecomposition(want); }
    aeSetDt(dt)                                                       { return this._aeEngine.aeSetDt(dt); }
    aeGetDt()                                                         { return this._aeEngine.aeGetDt(); }
    aeSetSoftening(s)                                                 { return this._aeEngine.aeSetSoftening(s); }
    aeSetDamping(e)                                                   { return this._aeEngine.aeSetDamping(e); }
    aeSetBonding(e)                                                   { return this._aeEngine.aeSetBonding(e); }
    aeSetIonic(e)                                                     { return this._aeEngine.aeSetIonic(e); }
    aeSetVdw(e)                                                       { return this._aeEngine.aeSetVdw(e); }
    aeSetBondsForce(e)                                                { return this._aeEngine.aeSetBondsForce(e); }
    aeSetSpeedLimit(e)                                                { return this._aeEngine.aeSetSpeedLimit(e); }
    aeSetHBonds(e)                                                    { return this._aeEngine.aeSetHBonds(e); }
    aeSetAngleStrain(e)                                               { return this._aeEngine.aeSetAngleStrain(e); }
    aeSetDipoleDipole(e)                                              { return this._aeEngine.aeSetDipoleDipole(e); }
    aeSetThermostat(e)                                                { return this._aeEngine.aeSetThermostat(e); }
    aeSetThermostatTemp(t)                                            { return this._aeEngine.aeSetThermostatTemp(t); }
    aeSetElectronegativity(e)                                         { return this._aeEngine.aeSetElectronegativity(e); }
    aeAtomCount()                                                     { return this._aeEngine.aeAtomCount(); }
    aeInspectAtom(id)                                                 { return this._aeEngine.aeInspectAtom(id); }
    aeClear()                                                         { return this._aeEngine.aeClear(); }
    aeGetRuntimeState()                                               { return this._aeEngine.aeGetRuntimeState(); }
    aeGetVelocities()                                                 { return this._aeEngine.aeGetVelocities(); }
    aeGetDipoles()                                                    { return this._aeEngine.aeGetDipoles(); }
    aeGetHBondPairs()                                                 { return this._aeEngine.aeGetHBondPairs(); }

    // setupScenario body extracted to bridge/scenarios/index.js as Wave 3 of
    // the large-file refactor. The extracted module is a pure move — `this`
    // binding preserved via .call() so all scenario helpers still resolve.
    setupScenario(name, harness = null) {
        const r = harness
            ? runSetupScenario(name, harness)
            : runSetupScenario.call(this, name);
        this._recomputeActiveBox();
        return r;
    }

    /**
     * Returns derived-overlay data shaped per `kind`. Most return objects
     * include a live reference to `_fluxMag` (and/or `_particles`) — these
     * are mutated each tick. Callers must treat the buffers as read-only
     * within the current frame; if you need a stable snapshot, `.slice()`
     * the magnitude at the call site. Same retention foot-gun applies to
     * `_particles`: the array identity is stable but per-particle fields
     * mutate in place.
     *
     * Was a prototype patch in bridge-init.js; moved here in Phase 2c
     * to keep the MockBridge class definition self-contained. The
     * capability factory in capabilities/scale0.js calls this method
     * if present; WasmBridge has no equivalent so the factory's
     * `if (typeof bridge.getScale0DerivedOverlayData === 'function')`
     * guard handles the asymmetry.
     */
    getScale0DerivedOverlayData(kind) {
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
    }
}
