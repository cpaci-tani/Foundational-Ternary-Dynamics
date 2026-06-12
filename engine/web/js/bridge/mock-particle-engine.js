/**
 * Scale-1 Particle Engine (PE) — MockBridge side only.
 *
 * N-body Coulomb + gravity dynamics with a Velocity Verlet integrator.
 * Extracted from `bridge-init.js` as Wave 2 ticket 5 of the large-file
 * bridge modularization pass documented in engine/web/docs/INDEX.md. This is a move, not
 * a rewrite — method bodies are preserved verbatim; the only structural
 * change is that `this.*` field accesses go through the live `state`
 * reference and that boundary reflection calls go through
 * `state._reflectIntoBoundary(...)` (which already delegates to the
 * extracted `bridge/boundary.js` module).
 *
 * STATE CONTRACT — `state` must be the MockBridge instance (not a
 * destructured copy), exposing:
 *   Read:
 *     _boundaryShape: string
 *     _reflectIntoBoundary(p, cx, cy, cz, R): void
 *   Read + write (created/managed here):
 *     _pe                : { particles, nextId, tick, dt, soft, coulomb,
 *                            damping, gravity, lorentz, exchange, strong,
 *                            magnetic_dipole, spin_orbit, radiation,
 *                            relativistic, forces?, forcesBuf?, forcesN? }
 *     _peParticleTypes   : Map<id, catalogId>
 *     _peBufs            : grow-only { positions, colors, sizes, charges,
 *                                      ids, velocities, cap } | undefined
 *     _peFieldBufs       : grow-only { positions, charges, masses, cap } | undefined
 *
 * Cache invalidation semantics — PE owns its own _pe.forces; there are no
 * Scale-0 cache invalidations touched by this module.
 *
 * Force law (preserved verbatim from peTick):
 *   F_coulomb = -(ALPHA / 4pi) * qi * qj * invR^2        (Newton's 3rd law)
 *   F_gravity =  G_N * mi * mj * invR^2                   (attractive)
 *   r^2 -> r^2 + soft^2   (softening to avoid singularities)
 *   F_vec = (F_c + F_g) * r_hat / r
 *
 * Force buffer layout: flat Float64Array(N*3) as [fx0,fy0,fz0, fx1,fy1,fz1, ...].
 * This avoids N object allocations per tick and gives ~2x speedup via cache
 * locality on the O(N^2) pair loop.
 */

import { ALPHA, K_B, DAMPING, G_N, C_SPEED, COULOMB_K_FORCE } from '../constants.js';

/**
 * Build the particle-engine provider bound to the given bridge-like state.
 *
 * @param {object} state - MockBridge instance (live reference).
 * @returns {object} { initPE, resetPE, peAddParticle, peAddLockedParticle,
 *                     _peComputeForces, peTick, peGetParticleData,
 *                     peGetFieldSources, peGetForces, peGetDiagnostics,
 *                     peGetExtendedData,
 *                     peSetDt, peGetDt, peSetSoftening, peSetCoulomb,
 *                     peSetDamping, peSetGravity, peSetLorentz, peSetExchange,
 *                     peSetStrong, peSetMagneticDipole, peSetSpinOrbit,
 *                     peSetRadiation, peSetRelativistic, peParticleCount,
 *                     peClear, peGetParticleTypes, peInspectParticle }
 */
export function createParticleEngine(state) {

    function initPE() {
        state._pe = {
            particles: [], nextId: 0, tick: 0, dt: 1.0, soft: 0.1, coulomb: true, damping: false, gravity: false,
            lorentz: false, exchange: false, strong: false, magnetic_dipole: false,
            spin_orbit: false, radiation: false, relativistic: false,
            relativistic_verlet: false
        };
        state._peParticleTypes = new Map();
    }

    function resetPE() {
        if (state._pe) {
            state._pe.particles = [];
            state._pe.nextId = 0;
            state._pe.tick = 0;
            state._pe.forces = null;
            state._pe.forcesN = 0;
        }
        if (state._peParticleTypes) state._peParticleTypes.clear();
    }

    function peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        if (!state._pe) initPE();
        if (mass <= 0) { console.warn('MockBridge: rejecting massless particle:', catalogId); return -1; }
        const id = state._pe.nextId++;
        state._pe.particles.push({
            id, charge, mass, r_eff, x, y, z, vx, vy, vz, locked: false
        });
        state._pe.forces = null; // invalidate force cache
        state._peParticleTypes.set(id, catalogId);
        return id;
    }

    function peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff = 0.1) {
        if (!state._pe) initPE();
        if (mass <= 0) { console.warn('MockBridge: rejecting massless particle:', catalogId); return -1; }
        const id = state._pe.nextId++;
        state._pe.particles.push({
            id, charge, mass, r_eff, x, y, z, vx: 0, vy: 0, vz: 0, locked: true
        });
        state._pe.forces = null; // invalidate force cache
        state._peParticleTypes.set(id, catalogId);
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
    function _peComputeForces() {
        const ps = state._pe.particles;
        const n = ps.length;
        // Grow-only typed buffer (avoids reallocation when particle count is stable)
        if (!state._pe.forcesBuf || state._pe.forcesBuf.length < n * 3) {
            state._pe.forcesBuf = new Float64Array(n * 3);
        }
        const F = state._pe.forcesBuf;
        // Zero the active region
        for (let k = 0; k < n * 3; k++) F[k] = 0;

        const soft2 = state._pe.soft * state._pe.soft;
        const doCoulomb = state._pe.coulomb;
        const doGravity = state._pe.gravity;
        const alpha4pi = COULOMB_K_FORCE;
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
        state._pe.forces = F;
        state._pe.forcesN = n;
    }

    // Velocity Verlet integrator: half-kick → drift → recompute forces → half-kick
    function peTick() {
        if (!state._pe) return;
        const ps = state._pe.particles;
        const dt = state._pe.dt;

        // Ensure forces are initialized
        if (!state._pe.forces || state._pe.forcesN !== ps.length) {
            _peComputeForces();
        }

        // Half-kick: v += (F/m) × dt/2   (forces in flat Float64Array)
        const F1 = state._pe.forces;
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
        if (state._boundaryShape !== 'cube' && state._boundaryShape !== 'none') {
            for (const p of ps) {
                if (p.locked) continue;
                state._reflectIntoBoundary(p, 0, 0, 0, 35);
            }
        }

        // Recompute forces at new positions
        _peComputeForces();

        // Half-kick again: v += (F/m) × dt/2
        const F2 = state._pe.forces;
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
        if (state._pe.damping) {
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
            state._pe.particles = ps.filter((_, idx) => !toRemove.has(idx));
            state._pe.forces = null;
        }

        state._pe.tick++;
    }

    function peGetParticleData() {
        if (!state._pe) return {
            positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0),
            charges: new Int8Array(0), ids: new Int32Array(0), velocities: new Float32Array(0),
            masses: new Float64Array(0), rEff: new Float32Array(0), locked: new Uint8Array(0),
            spins: new Int8Array(0), colorIds: new Int8Array(0), count: 0
        };
        const ps = state._pe.particles;
        const count = ps.length;
        // Reuse pre-allocated buffers to avoid GC pressure (grow only when needed)
        if (!state._peBufs || state._peBufs.cap < count) {
            state._peBufs = {
                positions: new Float32Array(count * 3),
                colors: new Float32Array(count * 3),
                sizes: new Float32Array(count),
                charges: new Int8Array(count),
                ids: new Int32Array(count),
                velocities: new Float32Array(count * 3),
                masses: new Float64Array(count),
                rEff: new Float32Array(count),
                locked: new Uint8Array(count),
                spins: new Int8Array(count),
                colorIds: new Int8Array(count),
                cap: count
            };
        }
        const { positions, colors, sizes, charges, ids, velocities, masses, rEff, locked, spins, colorIds } = state._peBufs;
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
            masses[i] = p.mass;
            rEff[i] = p.r_eff;
            locked[i] = p.locked ? 1 : 0;
            spins[i] = p.spin || 0;
            colorIds[i] = p.color || 0;
        }
        return { positions, colors, sizes, charges, ids, velocities, masses, rEff, locked, spins, colorIds, count };
    }

    function peGetFieldSources() {
        if (!state._pe) return { positions: new Float32Array(0), charges: new Float32Array(0), masses: new Float32Array(0), count: 0 };
        const ps = state._pe.particles;
        const n = ps.length;
        // Reuse buffers (grow-only) to avoid per-frame allocation
        if (!state._peFieldBufs || state._peFieldBufs.cap < n) {
            state._peFieldBufs = {
                positions: new Float32Array(n * 3),
                charges: new Float32Array(n),
                masses: new Float32Array(n),
                cap: n
            };
        }
        const { positions, charges, masses } = state._peFieldBufs;
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

    function peGetForces() {
        if (!state._pe) return { positions: new Float32Array(0), forces: new Float32Array(0), count: 0, maxForce: 0 };
        if (!state._pe.forces || state._pe.forcesN !== state._pe.particles.length) _peComputeForces();
        const ps = state._pe.particles;
        const F = state._pe.forces;  // flat Float64Array [fx0,fy0,fz0, fx1,fy1,fz1, ...]
        const n = ps.length;
        if (!state._peForcesBufs || state._peForcesBufs.cap < n) {
            state._peForcesBufs = {
                positions: new Float32Array(n * 3),
                forces: new Float32Array(n * 3),
                cap: n
            };
        }
        const { positions, forces } = state._peForcesBufs;
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

    function peGetDiagnostics() {
        if (!state._pe) return { tick: 0, particleCount: 0, totalKE: 0, totalPE: 0, coulombPE: 0, gravityPE: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0, angMomX: 0, angMomY: 0, angMomZ: 0 };
        const ps = state._pe.particles;
        let ke = 0, pe_coulomb = 0, pe_gravity = 0, px = 0, py = 0, pz = 0;
        let lx = 0, ly = 0, lz = 0;
        const soft2 = state._pe.soft * state._pe.soft;
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
                if (state._pe.coulomb) pe_coulomb += ALPHA * ps[i].charge * ps[j].charge / (4 * Math.PI * r);
                if (state._pe.gravity) {
                    pe_gravity -= G_N * ps[i].mass * ps[j].mass / r;
                }
            }
        }
        const pe_val = pe_coulomb + pe_gravity;
        return { tick: state._pe.tick, particleCount: ps.length, totalKE: ke, totalPE: pe_val, coulombPE: pe_coulomb, gravityPE: pe_gravity, totalEnergy: ke + pe_val, momentumX: px, momentumY: py, momentumZ: pz, angMomX: lx, angMomY: ly, angMomZ: lz };
    }

    function peGetExtendedData() {
        if (!state._pe) return null;
        const ps = state._pe.particles;
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
        const soft2 = (state._pe.soft || 0.1) ** 2;
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
                const fc = state._pe.coulomb ? -ALPHA * p.charge * q.charge / (4 * Math.PI * r2s) : 0;
                const fg = state._pe.gravity ? G_N * p.mass * q.mass / r2s : 0;
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

    function peSetDt(dt)             { if (state._pe) state._pe.dt = dt; }
    function peGetDt()                { return state._pe ? state._pe.dt : 1.0; }
    function peSetSoftening(s)       { if (state._pe) state._pe.soft = s; }
    function peSetCoulomb(e)         { if (state._pe) state._pe.coulomb = e; }
    function peSetDamping(e)         { if (state._pe) state._pe.damping = e; }
    function peSetGravity(e)         { if (state._pe) state._pe.gravity = e; }
    function peSetLorentz(e)         { if (state._pe) state._pe.lorentz = e; }
    function peSetExchange(e)        { if (state._pe) state._pe.exchange = e; }
    function peSetStrong(e)          { if (state._pe) state._pe.strong = e; }
    function peSetMagneticDipole(e)  { if (state._pe) state._pe.magnetic_dipole = e; }
    function peSetSpinOrbit(e)       { if (state._pe) state._pe.spin_orbit = e; }
    function peSetRadiation(e)       { if (state._pe) state._pe.radiation = e; }
    function peSetRelativistic(e)    { if (state._pe) state._pe.relativistic = e; }
    function peSetRelativisticVerlet(e) { if (state._pe) state._pe.relativistic_verlet = e; }
    function peGetToggle(name)       { return state._pe ? !!state._pe[name] : false; }
    function peGetBackendCapabilities() {
        return {
            velocities: true,
            masses: true,
            locked: true,
            forces: true,
            extended: true,
            nativeExtended: false,
            nativeForces: false,
            advancedForces: false,
        };
    }
    function peParticleCount()       { return state._pe ? state._pe.particles.length : 0; }
    function peClear()               { resetPE(); }
    function peGetParticleTypes()    { return state._peParticleTypes || new Map(); }

    function peInspectParticle(id) {
        if (!state._pe) return null;
        const p = state._pe.particles.find(q => q.id === id);
        if (!p) return null;
        const speed = Math.sqrt(p.vx * p.vx + p.vy * p.vy + p.vz * p.vz);
        const ke = 0.5 * p.mass * speed * speed;

        // Find nearest particle and compute net force
        let nearestId = -1, nearestDist = Infinity;
        let fNetX = 0, fNetY = 0, fNetZ = 0;
        let fCoulombNearest = 0;
        const soft2 = (state._pe.soft || 0.1) ** 2;

        for (const q of state._pe.particles) {
            if (q.id === p.id) continue;
            const dx = q.x - p.x, dy = q.y - p.y, dz = q.z - p.z;
            const r2 = dx * dx + dy * dy + dz * dz;
            const r = Math.sqrt(r2);
            if (r < nearestDist) { nearestDist = r; nearestId = q.id; }
            // Coulomb + gravity forces (matching peTick force law)
            const r2s = r2 + soft2;
            const fc = state._pe.coulomb ? -ALPHA * p.charge * q.charge / (4 * Math.PI * r2s) : 0;
            const fg = state._pe.gravity ? G_N * p.mass * q.mass / r2s : 0;
            if (r > 1e-20) {
                const fr = (fc + fg) / r;
                fNetX += fr * dx;
                fNetY += fr * dy;
                fNetZ += fr * dz;
            }
        }

        // Coulomb force to nearest specifically
        if (nearestId >= 0) {
            const nq = state._pe.particles.find(q => q.id === nearestId);
            if (nq) {
                const dx = nq.x - p.x, dy = nq.y - p.y, dz = nq.z - p.z;
                const r2 = dx * dx + dy * dy + dz * dz;
                fCoulombNearest = Math.abs(ALPHA * p.charge * nq.charge / (4 * Math.PI * (r2 + soft2)));
            }
        }

        // Orbital radius: distance to nearest opposite-charge particle
        let orbitalR = -1;
        for (const q of state._pe.particles) {
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

    return {
        initPE, resetPE,
        peAddParticle, peAddLockedParticle,
        _peComputeForces, peTick,
        peGetParticleData, peGetFieldSources, peGetForces,
        peGetDiagnostics, peGetExtendedData,
        peSetDt, peGetDt, peSetSoftening,
        peSetCoulomb, peSetDamping, peSetGravity, peSetLorentz,
        peSetExchange, peSetStrong, peSetMagneticDipole,
        peSetSpinOrbit, peSetRadiation, peSetRelativistic,
        peSetRelativisticVerlet, peGetToggle, peGetBackendCapabilities,
        peParticleCount, peClear, peGetParticleTypes,
        peInspectParticle,
    };
}
