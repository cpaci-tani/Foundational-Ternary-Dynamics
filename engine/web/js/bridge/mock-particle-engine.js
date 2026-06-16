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
 * Force law: full C++ parity via `pe-force-kernel.js` — Coulomb, gravity,
 * exchange, strong, magnetic dipole, spin-orbit, Lorentz, radiation reaction,
 * and relativistic correction (toggle-gated). Initial orbit speeds in scenarios
 * are derived from this kernel at t=0 (`pe-dynamics.js`), not closed-form ICs.
 *
 * Gravity provenance: G_PE = G_DERIVED = 1/(4pi*m_P^2) is the FTD-0131-derived
 * coupling (alpha_G(e,e) = (m_e/m_P)^2 ~ 1.75e-45). Particle-scale gravity is
 * float64-invisible next to Coulomb; dynamics are negligible but telemetry/charts
 * expose the true value. Scale 0/4/5 substrate demos still use lattice-toy G_N.
 *
 * Force buffer layout: flat Float64Array(N*3) as [fx0,fy0,fz0, fx1,fy1,fz1, ...].
 * This avoids N object allocations per tick and gives ~2x speedup via cache
 * locality on the O(N^2) pair loop.
 */

import { ALPHA, K_B, DAMPING, G_PE, C_SPEED, COULOMB_K_FORCE, STRONG_ALPHA_S } from '../constants.js';
import { getById } from '../particle-catalog.js';
import {
    alphaSLattice,
    computeAllForces,
    computeForceOnParticle,
    computePairwiseForceOnI,
    pairwiseMagneticDipoleForce,
    pairwiseSpinOrbitForce,
    peTogglesFromState,
} from './pe-force-kernel.js';
import { applyEquilibriumOrbit } from '../scales/scale1/pe-dynamics.js';
import { evolveParticleSpins } from './pe-spin-dynamics.js';

function catalogColorId(colorCharge) {
    if (colorCharge === 'r') return 1;
    if (colorCharge === 'g') return 2;
    if (colorCharge === 'b') return 3;
    return 0;
}

function catalogSpin(entry) {
    if (!entry || !entry.spin) return 0;
    return entry.spin > 0 ? 1 : -1;
}

/** |S| from catalog spin quantum number (ℏ=1: fermion ½ → |S|=1). */
function catalogSpinMagnitude(entry) {
    if (!entry || !entry.spin) return 0;
    return Math.abs(entry.spin) * 2.0;
}

function initSpinAxis(entry, spinSign) {
    if (!spinSign) return { spin_ax: 0, spin_ay: 0, spin_az: 0 };
    const mag = catalogSpinMagnitude(entry) || 1.0;
    return { spin_ax: 0, spin_ay: 0, spin_az: spinSign > 0 ? mag : -mag };
}

function makeParticleFields(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff, locked) {
    const entry = catalogId ? getById(catalogId) : null;
    const spin = catalogSpin(entry);
    const color = entry ? catalogColorId(entry.color_charge) : 0;
    const spinVec = initSpinAxis(entry, spin);
    return {
        charge, mass, r_eff, spin, color, pair_id: -1,
        x, y, z, vx, vy, vz, locked,
        ...spinVec,
        prev_ax: 0, prev_ay: 0, prev_az: 0,
        momx: mass * vx, momy: mass * vy, momz: mass * vz,
    };
}

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
            relativistic_verlet: false, annihilations: 0
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
            state._pe.annihilations = 0;
        }
        if (state._peParticleTypes) state._peParticleTypes.clear();
    }

    function peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        if (!state._pe) initPE();
        if (mass <= 0) { console.warn('MockBridge: rejecting massless particle:', catalogId); return -1; }
        const id = state._pe.nextId++;
        state._pe.particles.push({
            id,
            ...makeParticleFields(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff, false),
        });
        state._pe.forces = null;
        state._peParticleTypes.set(id, catalogId);
        return id;
    }

    /** Pedagogical anchor only — prefer dynamic massive particles for genuine dynamics. */
    function peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff = 0.1) {
        if (!state._pe) initPE();
        if (mass <= 0) { console.warn('MockBridge: rejecting massless particle:', catalogId); return -1; }
        const id = state._pe.nextId++;
        state._pe.particles.push({
            id,
            ...makeParticleFields(catalogId, charge, x, y, z, 0, 0, 0, mass, r_eff, true),
        });
        state._pe.forces = null;
        state._peParticleTypes.set(id, catalogId);
        return id;
    }

    function peScaleVelocity(particleId, scale) {
        if (!state._pe || scale === 1) return false;
        const p = state._pe.particles.find(q => q.id === particleId);
        if (!p) return false;
        p.vx *= scale; p.vy *= scale; p.vz *= scale;
        p.momx = p.mass * p.vx;
        p.momy = p.mass * p.vy;
        p.momz = p.mass * p.vz;
        return true;
    }

    function peApplyEquilibriumOrbit(particleId, options) {
        return applyEquilibriumOrbit(state, particleId, options);
    }

    function peApplyEquilibriumOrbitBatch(entries) {
        if (!entries?.length) return;
        for (const { particleId, center, tangent, sign } of entries) {
            const opts = {};
            if (center) opts.center = center;
            if (tangent) opts.tangent = tangent;
            if (sign !== undefined) opts.sign = sign;
            applyEquilibriumOrbit(state, particleId, opts);
        }
    }

    /**
     * PE force computation: Coulomb + gravity via N(N-1)/2 pair loop.
     *
     * For each unique pair (i,j), computes:
     *   F_coulomb = -alpha * q_i * q_j / (4pi * r^2)  (repulsive for same-sign)
     *   F_gravity =  G_PE * m_i * m_j / r^2             (always attractive)
     *     where G_PE = G_DERIVED = 1/(4pi*m_P^2) (FTD-0131 physical coupling).
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
        if (!state._pe.forcesBuf || state._pe.forcesBuf.length < n * 3) {
            state._pe.forcesBuf = new Float64Array(n * 3);
        }
        const toggles = peTogglesFromState(state._pe);
        computeAllForces(ps, toggles, state._pe.soft, state._pe.forcesBuf);
        state._pe.forces = state._pe.forcesBuf;
        state._pe.forcesN = n;
    }

    /**
     * Per-particle force decomposition for overlay arrows (Coulomb / gravity /
     * strong / net). Respects PE toggles; strong requires color ≠ 0 on both
     * ends of a pair. Mirrors C++ compute_pe_force_diag_snapshot semantics.
     */
    function peGetForceDecomposition() {
        if (!state._pe) {
            const empty = new Float32Array(0);
            return {
                positions: empty, count: 0,
                coulomb: empty, gravity: empty, strong: empty,
                magnetic_dipole: empty, spin_orbit: empty, net: empty,
                maxCoulomb: 0, maxGravity: 0, maxStrong: 0,
                maxMagneticDipole: 0, maxSpinOrbit: 0, maxNet: 0,
            };
        }
        const ps = state._pe.particles;
        const n = ps.length;
        const positions = new Float32Array(n * 3);
        const coulomb = new Float32Array(n * 3);
        const gravity = new Float32Array(n * 3);
        const strong = new Float32Array(n * 3);
        const magnetic_dipole = new Float32Array(n * 3);
        const spin_orbit = new Float32Array(n * 3);
        const net = new Float32Array(n * 3);
        const soft2 = state._pe.soft * state._pe.soft;
        const toggles = peTogglesFromState(state._pe);
        let maxCoulomb = 0, maxGravity = 0, maxStrong = 0;
        let maxMagneticDipole = 0, maxSpinOrbit = 0, maxNet = 0;

        for (let i = 0; i < n; i++) {
            const pi = ps[i];
            positions[i * 3] = pi.x;
            positions[i * 3 + 1] = pi.y;
            positions[i * 3 + 2] = pi.z;
        }

        for (let i = 0; i < n; i++) {
            const pi = ps[i];
            const i3 = i * 3;
            let fcx = 0, fcy = 0, fcz = 0;
            let fgx = 0, fgy = 0, fgz = 0;
            let fsx = 0, fsy = 0, fsz = 0;
            let fmx = 0, fmy = 0, fmz = 0;
            let fsox = 0, fsoy = 0, fsoz = 0;
            for (let j = 0; j < n; j++) {
                if (i === j) continue;
                const pj = ps[j];
                const dx = pj.x - pi.x, dy = pj.y - pi.y, dz = pj.z - pi.z;
                const rawR2 = dx * dx + dy * dy + dz * dz;
                const r2 = rawR2 + soft2;
                const r = Math.sqrt(r2);
                if (r < 1e-30) continue;
                const invR = 1 / r;
                const rx = dx * invR, ry = dy * invR, rz = dz * invR;

                if (toggles.coulomb) {
                    const fc = -COULOMB_K_FORCE * pi.charge * pj.charge / r2;
                    const fr = fc * invR;
                    fcx += fr * dx; fcy += fr * dy; fcz += fr * dz;
                }
                if (toggles.gravity) {
                    const fg = G_PE * pi.mass * pj.mass / r2;
                    const fr = fg * invR;
                    fgx += fr * dx; fgy += fr * dy; fgz += fr * dz;
                }
                if (toggles.strong && pi.color && pj.color) {
                    const cf = (pi.color === pj.color) ? 0.5 : -1.0;
                    let rawR = Math.sqrt(rawR2);
                    if (rawR < 1.0) rawR = 1.0;
                    let rawForce;
                    if (rawR < 3.0) {
                        const as = alphaSLattice(rawR);
                        rawForce = as * cf / (rawR * rawR);
                    } else if (rawR < 8.0) {
                        const as = alphaSLattice(rawR);
                        rawForce = as * cf / (3.0 * rawR);
                    } else {
                        rawForce = (STRONG_ALPHA_S * K_B * K_B) * cf;
                    }
                    const fr = -rawForce * invR;
                    fsx += fr * dx; fsy += fr * dy; fsz += fr * dz;
                }
                if (toggles.magnetic_dipole) {
                    const fmd = pairwiseMagneticDipoleForce(pi, pj, dx, dy, dz, r, r2);
                    fmx += fmd.fx; fmy += fmd.fy; fmz += fmd.fz;
                }
                if (toggles.spin_orbit) {
                    const fso = pairwiseSpinOrbitForce(pi, dx, dy, dz, rawR2, rx, ry, rz);
                    fsox += fso.fx; fsoy += fso.fy; fsoz += fso.fz;
                }
            }
            coulomb[i3] = fcx; coulomb[i3 + 1] = fcy; coulomb[i3 + 2] = fcz;
            gravity[i3] = fgx; gravity[i3 + 1] = fgy; gravity[i3 + 2] = fgz;
            strong[i3] = fsx; strong[i3 + 1] = fsy; strong[i3 + 2] = fsz;
            magnetic_dipole[i3] = fmx; magnetic_dipole[i3 + 1] = fmy; magnetic_dipole[i3 + 2] = fmz;
            spin_orbit[i3] = fsox; spin_orbit[i3 + 1] = fsoy; spin_orbit[i3 + 2] = fsoz;
            net[i3] = fcx + fgx + fsx + fmx + fsox;
            net[i3 + 1] = fcy + fgy + fsy + fmy + fsoy;
            net[i3 + 2] = fcz + fgz + fsz + fmz + fsoz;

            const mc = Math.sqrt(fcx * fcx + fcy * fcy + fcz * fcz);
            const mg = Math.sqrt(fgx * fgx + fgy * fgy + fgz * fgz);
            const ms = Math.sqrt(fsx * fsx + fsy * fsy + fsz * fsz);
            const mm = Math.sqrt(fmx * fmx + fmy * fmy + fmz * fmz);
            const mso = Math.sqrt(fsox * fsox + fsoy * fsoy + fsoz * fsoz);
            const mn = Math.sqrt(net[i3] * net[i3] + net[i3 + 1] * net[i3 + 1] + net[i3 + 2] * net[i3 + 2]);
            if (mc > maxCoulomb) maxCoulomb = mc;
            if (mg > maxGravity) maxGravity = mg;
            if (ms > maxStrong) maxStrong = ms;
            if (mm > maxMagneticDipole) maxMagneticDipole = mm;
            if (mso > maxSpinOrbit) maxSpinOrbit = mso;
            if (mn > maxNet) maxNet = mn;
        }

        return {
            positions, count: n,
            coulomb, gravity, strong, magnetic_dipole, spin_orbit, net,
            maxCoulomb, maxGravity, maxStrong, maxMagneticDipole, maxSpinOrbit, maxNet,
        };
    }

    // Velocity Verlet integrator: half-kick → drift → recompute forces → half-kick
    function halfKick(scale) {
        const particles = state._pe.particles;
        const F = state._pe.forces;
        const dt = state._pe.dt;
        const halfDt = dt * 0.5 * scale;
        const relVerlet = state._pe.relativistic_verlet;
        for (let i = 0; i < particles.length; i++) {
            const p = particles[i];
            if (p.locked) continue;
            const i3 = i * 3;
            if (relVerlet) {
                p.momx += F[i3] * halfDt;
                p.momy += F[i3 + 1] * halfDt;
                p.momz += F[i3 + 2] * halfDt;
                const p2 = p.momx * p.momx + p.momy * p.momy + p.momz * p.momz;
                const denom = Math.sqrt(p.mass * p.mass + p2 / (C_SPEED * C_SPEED));
                p.vx = p.momx / denom;
                p.vy = p.momy / denom;
                p.vz = p.momz / denom;
            } else {
                const hdt = halfDt / p.mass;
                p.vx += F[i3] * hdt;
                p.vy += F[i3 + 1] * hdt;
                p.vz += F[i3 + 2] * hdt;
                p.momx = p.mass * p.vx;
                p.momy = p.mass * p.vy;
                p.momz = p.mass * p.vz;
            }
        }
    }

    function clampSpeedLimit() {
        const particles = state._pe.particles;
        for (const p of particles) {
            if (p.locked) continue;
            const speed = Math.sqrt(p.vx * p.vx + p.vy * p.vy + p.vz * p.vz);
            if (speed > C_SPEED) {
                const s = C_SPEED / speed;
                p.vx *= s; p.vy *= s; p.vz *= s;
                if (state._pe.relativistic_verlet) {
                    p.momx = p.mass * p.vx;
                    p.momy = p.mass * p.vy;
                    p.momz = p.mass * p.vz;
                }
            }
        }
    }

    function peTick() {
        if (!state._pe) return;
        const ps = state._pe.particles;
        const dt = state._pe.dt;

        if (!state._pe.forces || state._pe.forcesN !== ps.length) {
            _peComputeForces();
        }

        halfKick(1);
        clampSpeedLimit();

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

        _peComputeForces();
        halfKick(1);

        // Store previous acceleration (radiation reaction, mirrors C++ tick)
        const F2 = state._pe.forces;
        for (let i = 0; i < ps.length; i++) {
            const p = ps[i];
            if (p.locked) continue;
            const i3 = i * 3;
            const invM = 1 / p.mass;
            p.prev_ax = F2[i3] * invM;
            p.prev_ay = F2[i3 + 1] * invM;
            p.prev_az = F2[i3 + 2] * invM;
        }

        // Damping (intentional energy dissipation, applied after Verlet)
        if (state._pe.damping) {
            const d = Math.max(0, 1 - DAMPING * dt);
            for (const p of ps) {
                if (p.locked) continue;
                p.vx *= d; p.vy *= d; p.vz *= d;
                p.momx = p.mass * p.vx;
                p.momy = p.mass * p.vy;
                p.momz = p.mass * p.vz;
            }
        }

        clampSpeedLimit();

        // Spin precession: dS/dt = (q/m) S × B from partner dipoles
        evolveParticleSpins(ps, peTogglesFromState(state._pe), state._pe.soft, dt);

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
            // Each annihilation removes an opposite-charge PAIR → size/2 events.
            state._pe.annihilations = (state._pe.annihilations || 0) + toRemove.size / 2;
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
            spins: new Int8Array(0), colorIds: new Int8Array(0), spinAxes: new Float32Array(0), count: 0
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
                spinAxes: new Float32Array(count * 3),
                cap: count
            };
        }
        const { positions, colors, sizes, charges, ids, velocities, masses, rEff, locked, spins, colorIds, spinAxes } = state._peBufs;
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
            spinAxes[i * 3] = p.spin_ax ?? 0;
            spinAxes[i * 3 + 1] = p.spin_ay ?? 0;
            spinAxes[i * 3 + 2] = p.spin_az ?? 0;
        }
        return { positions, colors, sizes, charges, ids, velocities, masses, rEff, locked, spins, colorIds, spinAxes, count };
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
        if (!state._pe) return { tick: 0, particleCount: 0, totalKE: 0, totalPE: 0, coulombPE: 0, gravityPE: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0, angMomX: 0, angMomY: 0, angMomZ: 0, annihilations: 0 };
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
                if (state._pe.coulomb) pe_coulomb += COULOMB_K_FORCE * ps[i].charge * ps[j].charge / r;
                if (state._pe.gravity) {
                    pe_gravity -= G_PE * ps[i].mass * ps[j].mass / r;
                }
            }
        }
        const pe_val = pe_coulomb + pe_gravity;
        return { tick: state._pe.tick, particleCount: ps.length, totalKE: ke, totalPE: pe_val, coulombPE: pe_coulomb, gravityPE: pe_gravity, totalEnergy: ke + pe_val, momentumX: px, momentumY: py, momentumZ: pz, angMomX: lx, angMomY: ly, angMomZ: lz, annihilations: state._pe.annihilations || 0 };
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
                const fc = state._pe.coulomb ? -COULOMB_K_FORCE * p.charge * q.charge / r2s : 0;
                const fg = state._pe.gravity ? G_PE * p.mass * q.mass / r2s : 0;
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

    function peSetSpinAxis(id, ax, ay, az) {
        if (!state._pe) return false;
        const p = state._pe.particles.find(q => q.id === id);
        if (!p) return false;
        const oldMag = Math.sqrt(
            (p.spin_ax ?? 0) ** 2 + (p.spin_ay ?? 0) ** 2 + (p.spin_az ?? 0) ** 2);
        const targetMag = oldMag > 1e-30 ? oldMag : 1.0;
        const newMag = Math.sqrt(ax * ax + ay * ay + az * az);
        if (newMag < 1e-30) return false;
        const s = targetMag / newMag;
        p.spin_ax = ax * s;
        p.spin_ay = ay * s;
        p.spin_az = az * s;
        state._pe.forces = null;
        return true;
    }

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
            advancedForces: true,
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
            const fc = state._pe.coulomb ? -COULOMB_K_FORCE * p.charge * q.charge / r2s : 0;
            const fg = state._pe.gravity ? G_PE * p.mass * q.mass / r2s : 0;
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
                fCoulombNearest = Math.abs(COULOMB_K_FORCE * p.charge * nq.charge / (r2 + soft2));
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
            rEff: p.r_eff, spin: p.spin, colorId: p.color, pairId: p.pair_id,
            x: p.x, y: p.y, z: p.z,
            vx: p.vx, vy: p.vy, vz: p.vz,
            speed, ke, 
            momentum: p.mass * speed,
            acceleration: Math.sqrt(fNetX * fNetX + fNetY * fNetY + fNetZ * fNetZ) / p.mass,
            locked: p.locked,
            nearestId, nearestDist,
            orbitalR,
            fCoulombNearest,
            fNetMag: Math.sqrt(fNetX * fNetX + fNetY * fNetY + fNetZ * fNetZ),
        };
    }

    return {
        initPE, resetPE,
        peAddParticle, peAddLockedParticle, peApplyEquilibriumOrbit, peApplyEquilibriumOrbitBatch, peScaleVelocity,
        _peComputeForces, peTick,
        peGetParticleData, peGetFieldSources, peGetForces,
        peGetForceDecomposition,
        peGetDiagnostics, peGetExtendedData,
        peSetDt, peGetDt, peSetSoftening,
        peSetCoulomb, peSetDamping, peSetGravity, peSetLorentz,
        peSetExchange, peSetStrong, peSetMagneticDipole,
        peSetSpinOrbit, peSetSpinAxis, peSetRadiation, peSetRelativistic,
        peSetRelativisticVerlet, peGetToggle, peGetBackendCapabilities,
        peParticleCount, peClear, peGetParticleTypes,
        peInspectParticle,
    };
}
