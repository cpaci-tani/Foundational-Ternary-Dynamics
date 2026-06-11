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

import { getById as catalogGetById } from '../particle-catalog.js';
import { ALPHA, ALPHA_EFT, K_B, K_GENESIS, DAMPING, G_N, G_C, C_SPEED, M_PROTON, R_BOHR, N_BASE, G_STAR, VARPI, N_C, B_3, N_EFF,
    COULOMB_K_FORCE,
    STRONG_ALPHA_S, STRONG_RUN_COEFF, STRONG_R_COULOMB, STRONG_R_LINEAR,
    STRONG_TRANSITION_DENOM, STRONG_LINEAR_DENOM,
    STRONG_COLOR_REPEL, STRONG_COLOR_ATTRACT } from '../constants.js';

// Hoisted module-scope constant — recomputing K_GENESIS² inside tick() every
// frame is wasteful (L-5 cleanup from AUDIT_LEDGER pre-refactor sweep).
const K_GENESIS_SQ = K_GENESIS * K_GENESIS;

import { debugLog } from '../core/log.js';
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
import { allocSharedField, viewSharedField, CTRL } from './shared-field.js';

// ── Mock Bridge ────────────────────────────────────────────────────
/** @implements {import('./bridge-contract.js').ScaleBridge} */
export class MockBridge {
    constructor(latticeSize = 33) {
        // Odd lattices only: an odd N has a true center voxel at (N-1)/2, so
        // point injections + symmetric flux center exactly (no half-voxel
        // straddle / +x−x asymmetry). Snap any even N up to the next odd.
        this.latticeSize = (latticeSize % 2 === 0) ? latticeSize + 1 : latticeSize;
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

        // ── Sparse (active-region) wave tick — Phase 1 (SPEC_SCALE0_LATTICE_PERF §3) ──
        // _activeBox = inclusive bounds of nonzero flux; x1<x0 means "empty".
        // _activeDense latches true when the wave reaches a wall / fills >40%
        // (then _tickFlux runs the original full dense path). _sparseEps: trim
        // threshold (0 = bit-exact). _sparseTick gates the whole optimization.
        this._sparseTick = true;    // FTD_SPARSE_TICK — bit-exact active-region tick (SPEC §3); set false to revert to dense
        this._activeBox = { x0: this.latticeSize, x1: -1, y0: this.latticeSize, y1: -1, z0: this.latticeSize, z1: -1 };
        this._activeDense = false;
        this._sparseEps = 0;

        // ── SAB-backed field for the physics Web Worker — Phase 2 (PLAN_SCALE0_PHYSICS_WORKER) ──
        // When _useSAB, _initFluxGrid backs the flux buffers with SharedArrayBuffers
        // (held in _sharedField) so a worker host and the main-thread proxy share
        // them zero-copy. Off for the normal in-thread bridge (plain typed arrays).
        this._useSAB = false;
        this._sharedField = null;

        // Mutable simulation parameters (combo panel)
        this._params = { kb: K_B, gn: G_N, damping: DAMPING, omega0: 1.0 };

        // Toggle states (mirror engine TermToggles from term_toggles.h)
        // NOTE: gravity defaults to false here to match config/toggles.js SCALE0_TOGGLES.
        // Scenarios that need gravity enable it via SCALE0_SCENARIO_OVERRIDES.
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

        // Cached energy sums — avoids redundant O(L^3) loops across getDiagnostics/getEnergyAudit/getLagrangian
        this._energyCacheTick = -1;
        // Cached latency-proxy lattice (|J|²-derived L(x) for Kretschmann +
        // horizon samplers). Rebuilt lazily per tick; invalidated explicitly
        // on reset() and flux/wave injection writes.
        this._latencyProxy = null;
        this._latencyProxyTick = -1;
        this._cachedFieldEnergy = 0;
        this._cachedWaveEnergy = 0;
        this._cachedFluxMag = 0;

        // Cached sponge-layer damping table, rebuilt on demand when D changes
        // (M-9 cleanup — was a fresh Float32Array(D+1) every absorbing tick).
        this._spongeTable = null;
        // Last scatter count for stateGrid zeroing optimization (M-11).
        this._lastStateScatterCount = 0;

        // Lattice samplers — factory takes the live MockBridge instance so
        // cache writes (_latencyProxy, _latencyProxyTick) propagate back
        // here and all existing invalidation sites (reset / per-tick advance
        // / flux-wave injection mutators) continue to work.
        this._samplers = createLatticeSamplers(this);
        // Diagnostics provider — same live-reference contract.
        this._diagnostics = createDiagnosticsProvider(this);
        // Scale-1 Particle Engine — same live-reference contract (mutates
        // state._pe, _peParticleTypes, _peBufs, _peFieldBufs on this instance).
        this._peEngine = createParticleEngine(this);
        // Scale-2 Atom Engine — same live-reference contract (mutates state._ae
        // and the bond lookup caches on this instance).
        this._aeEngine = createAtomEngine(this);
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
    // the ctor). See docs/SPEC_REFACTOR_LARGE_FILES.md §4.
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
        const N = this.latticeSize;
        const total = N * N * N;
        if (this._useSAB) {
            // Worker mode: back every field buffer (incl. the state grid, so
            // particle scenarios share too) with SharedArrayBuffers. The proxy
            // attaches views over the same memory via getSharedField().
            const sab = allocSharedField(N);
            this._sharedField = sab;
            const v = viewSharedField(sab);
            this._fluxJ = v.fluxJ;
            this._fluxWV = v.fluxWV;
            this._fluxMag = v.fluxMag;
            this._stateGrid = v.state;
            v.ctrl[CTRL.N] = N;
        } else {
            this._fluxJ = new Float64Array(total * 3); // flux vector field (Jx, Jy, Jz)
            this._fluxWV = new Float64Array(total * 3); // wave velocity (leapfrog)
            this._fluxMag = new Float64Array(total);     // cached magnitudes
        }
        this._fluxDirty = true;
    }

    // Worker hosts return the SharedArrayBuffer set so the main-thread proxy can
    // attach views over the same memory (zero-copy). Null unless _useSAB.
    getSharedField() { return this._sharedField; }

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
        this._expandActiveBox(x, y, z);
    }

    _injectWaveVel(x, y, z, wx, wy, wz) {
        if (!this._fluxWV) this._initFluxGrid();
        const idx = this._fluxIdx(x, y, z);
        this._fluxWV[idx * 3] += wx;
        this._fluxWV[idx * 3 + 1] += wy;
        this._fluxWV[idx * 3 + 2] += wz;
        this._expandActiveBox(x, y, z);
    }

    // ── Sparse (active-region) wave-tick helpers — SPEC_SCALE0_LATTICE_PERF §3 ──
    _resetActiveBox() {
        const N = this.latticeSize;
        this._activeBox = { x0: N, x1: -1, y0: N, y1: -1, z0: N, z1: -1 };
        this._activeDense = false;
    }

    // Grow the box to include voxel (x,y,z). Coords are wrapped like _fluxIdx so
    // a periodic-wrap injection lands at its true voxel (clamping would drop it).
    _expandActiveBox(x, y, z) {
        const b = this._activeBox, N = this.latticeSize;
        x = ((x % N) + N) % N; y = ((y % N) + N) % N; z = ((z % N) + N) % N;
        if (x < b.x0) b.x0 = x; if (x > b.x1) b.x1 = x;
        if (y < b.y0) b.y0 = y; if (y > b.y1) b.y1 = y;
        if (z < b.z0) b.z0 = z; if (z > b.z1) b.z1 = z;
    }

    // Grow the box by one shell each tick (the 18-point stencil reach is ≤1
    // voxel/tick), clamped to the lattice. Cheap O(1); keeps the box a superset
    // of nonzero J/WV so the bounded tick stays bit-exact.
    _growActiveBox() {
        const b = this._activeBox, N = this.latticeSize;
        if (b.x1 < b.x0) return;
        b.x0 = Math.max(0, b.x0 - 1); b.x1 = Math.min(N - 1, b.x1 + 1);
        b.y0 = Math.max(0, b.y0 - 1); b.y1 = Math.min(N - 1, b.y1 + 1);
        b.z0 = Math.max(0, b.z0 - 1); b.z1 = Math.min(N - 1, b.z1 + 1);
    }

    // Tight rescan of nonzero J/WV bounds (O(N³)); call only occasionally (after
    // scenario setup, and every K ticks so a damped field can shrink the box).
    _recomputeActiveBox() {
        const N = this.latticeSize, J = this._fluxJ, WV = this._fluxWV, eps = this._sparseEps;
        this._resetActiveBox();
        if (!J) return;
        const b = this._activeBox;
        for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
            const i3 = (z * N * N + y * N + x) * 3;
            const a = Math.abs(J[i3]) + Math.abs(J[i3 + 1]) + Math.abs(J[i3 + 2])
                    + Math.abs(WV[i3]) + Math.abs(WV[i3 + 1]) + Math.abs(WV[i3 + 2]);
            if (a > eps) {
                if (x < b.x0) b.x0 = x; if (x > b.x1) b.x1 = x;
                if (y < b.y0) b.y0 = y; if (y > b.y1) b.y1 = y;
                if (z < b.z0) b.z0 = z; if (z > b.z1) b.z1 = z;
            }
        }
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
    // FTD-0271: minimal KG clock leapfrog used when wave_propagation is OFF.
    // Each manifested voxel oscillates as J'' = -omega0^2 J at exactly omega0
    // (the k=0 rest-frame internal clock), so the omega0 slider directly sets
    // the frequency. No spatial Laplacian, no damping/coupling.
    _tickClockOnly() {
        if (!this._fluxJ || this._particles.length === 0) return;
        const N = this.latticeSize, NN = N * N;
        const dt = this._dt ?? 1.0;
        const w0 = this._params.omega0 ?? 1.0;
        const w2dt = w0 * w0 * dt;
        const J = this._fluxJ, WV = this._fluxWV;
        for (const p of this._particles) {
            if (p.state === 0) continue;
            const px = ((Math.round(p.x) % N) + N) % N;
            const py = ((Math.round(p.y) % N) + N) % N;
            const pz = ((Math.round(p.z) % N) + N) % N;
            const i3 = (pz * NN + py * N + px) * 3;
            for (let c = 0; c < 3; c++) {
                WV[i3 + c] -= w2dt * J[i3 + c];
                J[i3 + c] += WV[i3 + c] * dt;
            }
        }
    }

    _tickFlux() {
        if (!this._fluxJ) return;
        const N = this.latticeSize;
        // CFL stability: c * dt <= dx = 1. C_SPEED = 1/sqrt(3) so dt <= sqrt(3).
        // Violating CFL causes silent exponential blow-up — assert early.
        // _params.dt was a dead branch — never set anywhere. Drop it (M-6).
        const dt = this._dt ?? 1.0;
        if (dt * C_SPEED > 1.0 + 1e-9) {
            if (!this._cflWarned) {
                console.warn(`[FTD] CFL violation: dt*c=${(dt*C_SPEED).toFixed(4)} > 1. Reduce dt (max = sqrt(3) ~= 1.732).`);
                this._cflWarned = true;
            }
        }
        // c2 absorbs dt so the WV update reads `WV += c2dt * laplacian` and
        // the J commit reads `J = (J + WV*dt) * damp`. This makes the
        // leapfrog respect setDt(dt) (C-arch-2 / H-4 fix).
        const c2 = C_SPEED * C_SPEED * dt;
        // Clamp damping into [0,1] so a stray param > 1 cannot flip the sign of
        // J/WV every tick and produce exponential blow-up (L-4 cleanup).
        const damp = this._toggles.damping
            ? Math.max(0, Math.min(1, 1.0 - this._params.damping))
            : 1.0;
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
            // Skip the O(L³) zero-fill when the grid is already clean (no
            // prior scatter in the last tick). Worst case (steady-state
            // particle count) we still pay one fill, but empty/low-count
            // ticks become free (M-11 cleanup).
            if (this._lastStateScatterCount > 0) stateGrid.fill(0);
            let scatterCount = 0;
            for (const p of this._particles) {
                if (p.state === 0) continue;
                const px = ((Math.round(p.x) % N) + N) % N;
                const py = ((Math.round(p.y) % N) + N) % N;
                const pz = ((Math.round(p.z) % N) + N) % N;
                stateGrid[pz * NN + py * N + px] = p.state;
                scatterCount++;
            }
            this._lastStateScatterCount = scatterCount;
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

        // ── Sparse (active-region) windowing ─────────────────────────────
        // Restrict the O(N³) interior Laplacian + commit to the nonzero
        // bounding box (+1 frontier). Skip the boundary loops + sponge while
        // the box is interior (those voxels are zero → provably no-ops). Fall
        // back to the full dense path once the front nears a wall (periodic
        // wrap couples both walls) or fills >40%.
        let sx0 = 1, sx1 = N - 2, sy0 = 1, sy1 = N - 2, sz0 = 1, sz1 = N - 2;
        let sparseActive = false;
        let runBoundaryWV = true;
        if (this._sparseTick && !this._activeDense) {
            const bx = this._activeBox;
            if (bx.x1 < bx.x0) return;               // empty field → nothing to do
            const Dsp = this._reflectiveBoundary ? 1 : Math.min(6, Math.max(2, Math.floor(N / 4)));
            const margin = Dsp + 1;                  // stay clear of the sponge shell too
            const nearWall = bx.x0 <= margin || bx.x1 >= N - 1 - margin
                          || bx.y0 <= margin || bx.y1 >= N - 1 - margin
                          || bx.z0 <= margin || bx.z1 >= N - 1 - margin;
            const vol = (bx.x1 - bx.x0 + 1) * (bx.y1 - bx.y0 + 1) * (bx.z1 - bx.z0 + 1);
            if (nearWall || vol > 0.4 * N * N * N) {
                this._activeDense = true;            // latch dense from here on
            } else {
                sparseActive = true;
                runBoundaryWV = false;
                sx0 = Math.max(1, bx.x0 - 1); sx1 = Math.min(N - 2, bx.x1 + 1);
                sy0 = Math.max(1, bx.y0 - 1); sy1 = Math.min(N - 2, bx.y1 + 1);
                sz0 = Math.max(1, bx.z0 - 1); sz1 = Math.min(N - 2, bx.z1 + 1);
            }
        }

        // ── Fast interior path (no modulo, pre-computed byte offsets) ────
        for (let z = sz0; z <= sz1; z++) {
            const zBase = z * NN;
            for (let y = sy0; y <= sy1; y++) {
                const rowStart = zBase + y * N + sx0;
                // Byte offset of (x=1, y, z) in the interleaved array
                let i3 = rowStart * 3;
                // Flat voxel index (for stateGrid coupling); advanced in lockstep with i3
                let vi = rowStart;

                for (let x = sx0; x <= sx1; x++) {
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
        for (let z = 0; runBoundaryWV && z < N; z++) {
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
        for (let z = 1; runBoundaryWV && z < Nm1; z++) {
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

        // FTD-0271: de Broglie internal clock — Klein-Gordon mass term.
        // WV -= omega0^2 * dt * J at manifested (state != 0) voxels, applied
        // after the Laplacian WV update and before the commit so the leapfrog
        // integrates it (mirrors phase_read.cpp). With the toggle OFF this is a
        // dead branch, so the mock's default behaviour is unchanged.
        if (this._toggles.de_broglie_clock && this._particles.length > 0) {
            const w0 = this._params.omega0 ?? 1.0;
            const w2dt = w0 * w0 * dt;
            for (const p of this._particles) {
                if (p.state === 0) continue;
                const px = ((Math.round(p.x) % N) + N) % N;
                const py = ((Math.round(p.y) % N) + N) % N;
                const pz = ((Math.round(p.z) % N) + N) % N;
                const i3 = (pz * NN + py * N + px) * 3;
                WV[i3]     -= w2dt * J[i3];
                WV[i3 + 1] -= w2dt * J[i3 + 1];
                WV[i3 + 2] -= w2dt * J[i3 + 2];
            }
        }

        // Commit: J += WV, J *= damp (selective or uniform)
        // Unrolled component loop and flat stride for cache-friendly access.
        const total = N * N * N;
        const selective = this._toggles.selective_damping;
        const total3 = total * 3;

        if (sparseActive && !(selective && damp < 1.0 && this._particles.length > 0)) {
            // Commit only the active window. Outside it J=WV=0 ⇒ (0+WV·dt)·d with
            // J=WV=0 is 0 (no-op), so skipping is bit-exact. effDamp reproduces the
            // dense per-voxel factor for every no-particle case: damping off ⇒ 1;
            // uniform damping ⇒ damp; selective + no particles ⇒ 1 (the dense
            // selective path builds an all-zero mask ⇒ d = 1 everywhere).
            const effDamp = (selective && damp < 1.0 && this._particles.length === 0) ? 1.0 : damp;
            for (let z = sz0; z <= sz1; z++) {
                for (let y = sy0; y <= sy1; y++) {
                    let i3 = (z * NN + y * N + sx0) * 3;
                    for (let x = sx0; x <= sx1; x++) {
                        J[i3]     = (J[i3]     + WV[i3]     * dt) * effDamp;
                        J[i3 + 1] = (J[i3 + 1] + WV[i3 + 1] * dt) * effDamp;
                        J[i3 + 2] = (J[i3 + 2] + WV[i3 + 2] * dt) * effDamp;
                        WV[i3] *= effDamp; WV[i3 + 1] *= effDamp; WV[i3 + 2] *= effDamp;
                        i3 += 3;
                    }
                }
            }
        } else if (selective && damp < 1.0) {
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
                J[i3]     = (J[i3]     + WV[i3]     * dt) * d;
                J[i3 + 1] = (J[i3 + 1] + WV[i3 + 1] * dt) * d;
                J[i3 + 2] = (J[i3 + 2] + WV[i3 + 2] * dt) * d;
                WV[i3]     *= d;
                WV[i3 + 1] *= d;
                WV[i3 + 2] *= d;
            }
        } else {
            // Uniform damping on both J and WV (or no damping if damp === 1.0)
            // Flat stride through the entire array for maximum cache coherence
            for (let k = 0; k < total3; k += 3) {
                J[k]     = (J[k]     + WV[k]     * dt) * damp;
                J[k + 1] = (J[k + 1] + WV[k + 1] * dt) * damp;
                J[k + 2] = (J[k + 2] + WV[k + 2] * dt) * damp;
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

        // ── Absorbing boundary / sponge layer (reflective = OFF) ─────────
        // The wave-equation stencil above uses periodic wrap (modulo N) at
        // x ∈ {0, N-1} etc., so without intervention the lattice is a
        // 3-torus and energy never leaves. When the user disables the
        // reflective toggle they expect flux to dissipate into the abyss
        // beyond the lattice edge — so we drain it here via a graded
        // sponge layer.
        //
        // Strategy: a depth-D shell (D = min(6, ⌊N/4⌋)). At each voxel
        // whose minimum distance to any lattice face is d ∈ [0, D−1], a
        // multiplicative damping factor f(d) is applied to both J and WV
        // every tick. f is monotone: f(0) = 0 (Dirichlet), grading
        // smoothly toward 1 at d = D so the impedance change is gradual
        // and reflects very little. With f given by f(d) = (d/D)² (a
        // quadratic ramp), an outgoing wave traversing the layer is
        // attenuated by ∏_{d=1}^{D−1} (d/D)² ≈ ((D−1)!/D^(D−1))² per pass.
        // For D = 6 that's 0.012, i.e. < 1% of the wave reaches the wall;
        // round-trip absorption ≳ 99.97%.
        // O(L³) cost, but only the shell voxels do non-trivial work
        // (interior voxels compute d ≥ D and short-circuit without writes).
        if (!this._reflectiveBoundary && runBoundaryWV) {
            const Nm1 = N - 1;
            const D = Math.min(6, Math.max(2, Math.floor(N / 4)));
            // Precompute the per-distance damping table: f[d] for d = 0..D
            // f(D) = 1.0 (no damping), f(0) = 0 (zero out). Cached on
            // this._spongeTable; rebuilt only when D changes (M-9).
            if (!this._spongeTable || this._spongeTable.length !== D + 1) {
                const tbl = new Float32Array(D + 1);
                for (let d = 0; d <= D; d++) {
                    const r = d / D;
                    tbl[d] = r * r;  // quadratic ramp
                }
                this._spongeTable = tbl;
            }
            const f = this._spongeTable;
            for (let z = 0; z < N; z++) {
                const dz = Math.min(z, Nm1 - z);
                for (let y = 0; y < N; y++) {
                    const dy = Math.min(y, Nm1 - y);
                    for (let x = 0; x < N; x++) {
                        const dx = Math.min(x, Nm1 - x);
                        const d = Math.min(dx, dy, dz);
                        if (d >= D) continue;
                        const fd = f[d];
                        const i3 = (z * N * N + y * N + x) * 3;
                        J[i3]   *= fd; J[i3+1] *= fd; J[i3+2] *= fd;
                        WV[i3]  *= fd; WV[i3+1]*= fd; WV[i3+2]*= fd;
                    }
                }
            }
        }

        this._fluxDirty = true;
        if (this._sparseTick && !this._activeDense) {
            this._growActiveBox();                          // wave front advanced ≤1 voxel
            // Periodic tight rescan lets a damped/dissipating field shrink the
            // box again (cheap amortized: O(N³)/32 per tick).
            if ((this._tick & 31) === 0) this._recomputeActiveBox();
        }
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

    /**
     * Returns a per-call snapshot of the requested mid-plane (Float64Array
     * of length N²). Pre-2026-04-26 this returned a SHARED `_sliceBuf` to
     * skip allocation; callers (e.g. the flux-slice panel sampling 3
     * planes back-to-back) had to `.slice()` defensively. Audit Theme B
     * found multiple consumers retaining the reference across calls and
     * silently reading the next-call's data, so the contract was changed
     * to always return a fresh buffer. Cost at L=128 is one ~128 KB
     * allocation per call — negligible vs. the GC churn the prior
     * sharing scheme was intended to avoid.
     */
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

    /**
     * Returns the live `_fluxMag` Float64Array (length N³). The buffer
     * is mutated in place every tick by `_updateFluxMag`, so callers
     * MUST treat the return as read-only-this-frame. Common usage is to
     * upload directly into a 3D texture without retention. If you need
     * a stable copy (e.g. cross-frame diff, replay buffer), call
     * `.slice()` at the call site — the volume can be large (16 MB at
     * L=128), so we don't pay that cost on every reader's behalf.
     */
    getFluxVolume() {
        if (!this._fluxJ) this._initFluxGrid();
        this._updateFluxMag();
        return this._fluxMag;
    }

    // ── Bulk Sampled Vector Field Exports (Scale 0 field visualization) ──
    //
    // All 17 samplers + the latency-proxy helper moved to
    // bridge/mock-lattice-samplers.js. MockBridge forwards via
    // this._samplers (constructed in the ctor above).
    // See docs/SPEC_REFACTOR_LARGE_FILES.md §4 for the full rationale.

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
