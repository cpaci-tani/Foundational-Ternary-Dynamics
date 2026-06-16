/**
 * Scale-1 particle force kernel — mirrors C++ ParticleEngine::compute_pairwise_force
 * and per-particle post-processing (radiation, relativistic correction).
 *
 * Used by mock-particle-engine.js for browser PE dynamics parity.
 */

import { ALPHA, K_B, G_PE, C_SPEED, COULOMB_K_FORCE, STRONG_ALPHA_S } from '../constants.js';

const Q_LATTICE = 2.0;
const EXCHANGE_RANGE_SQ = 9.0;
const ALPHA_EXCHANGE = ALPHA * ALPHA;
const SIGMA_STRING = STRONG_ALPHA_S * K_B * K_B;

/** Running α_s(r) — matches C++ alpha_s_lattice. */
export function alphaSLattice(r) {
    if (r <= 0) return STRONG_ALPHA_S;
    const Q = Q_LATTICE / r;
    const beta0 = 11.0 - (2.0 / 3.0) * 3.0;
    const as = STRONG_ALPHA_S / (1.0 + (beta0 / (4.0 * Math.PI)) * STRONG_ALPHA_S * Math.log(Math.max(Q, 0.01) / Q_LATTICE));
    return Math.min(Math.max(as, 0), STRONG_ALPHA_S);
}

/**
 * Force on particle `pi` due to particle `pj` (pairwise terms from C++).
 * @param {object} pi - particle i
 * @param {object} pj - particle j
 * @param {object} toggles - PE toggle flags
 * @param {number} soft - softening length
 * @returns {{fx:number, fy:number, fz:number}}
 */
export function computePairwiseForceOnI(pi, pj, toggles, soft) {
    const dx = pj.x - pi.x;
    const dy = pj.y - pi.y;
    const dz = pj.z - pi.z;
    const rawR2 = dx * dx + dy * dy + dz * dz;
    const soft2 = soft * soft;
    const r2 = rawR2 + soft2;
    const r = Math.sqrt(r2);
    if (r < 1e-30) return { fx: 0, fy: 0, fz: 0 };

    const invR = 1 / r;
    const rx = dx * invR;
    const ry = dy * invR;
    const rz = dz * invR;

    let fx = 0, fy = 0, fz = 0;

    if (toggles.coulomb) {
        const fEm = -COULOMB_K_FORCE * pi.charge * pj.charge / r2;
        fx += rx * fEm; fy += ry * fEm; fz += rz * fEm;
    }

    if (toggles.gravity) {
        const fGrav = G_PE * pi.mass * pj.mass / r2;
        fx += rx * fGrav; fy += ry * fGrav; fz += rz * fGrav;
    }

    if (toggles.exchange && pi.spin !== 0 && pj.spin === pi.spin && pi.charge === pj.charge) {
        const fMag = ALPHA_EXCHANGE * Math.exp(-r2 / EXCHANGE_RANGE_SQ) / r2;
        const fRep = -fMag;
        fx += rx * fRep; fy += ry * fRep; fz += rz * fRep;
    }

    if (toggles.strong && pi.color !== 0 && pj.color !== 0) {
        const cf = (pi.color === pj.color) ? 0.5 : -1.0;
        let rawR = Math.sqrt(rawR2);
        if (rawR < 1.0) rawR = 1.0;
        const rawR2u = rawR * rawR;
        let fStrongMag;
        if (rawR < 3.0) {
            const as = alphaSLattice(rawR);
            fStrongMag = as * cf / rawR2u;
        } else if (rawR < 8.0) {
            const as = alphaSLattice(rawR);
            fStrongMag = as * cf / (3.0 * rawR);
        } else {
            fStrongMag = SIGMA_STRING * cf;
        }
        const fs = -fStrongMag;
        fx += rx * fs; fy += ry * fs; fz += rz * fs;
    }

    if (toggles.magnetic_dipole) {
        const fmd = pairwiseMagneticDipoleForce(pi, pj, dx, dy, dz, r, r2);
        fx += fmd.fx; fy += fmd.fy; fz += fmd.fz;
    }

    if (toggles.spin_orbit) {
        const fso = pairwiseSpinOrbitForce(pi, dx, dy, dz, rawR2, rx, ry, rz);
        fx += fso.fx; fy += fso.fy; fz += fso.fz;
    }

    const pjSax = pj.spin_ax ?? 0, pjSay = pj.spin_ay ?? 0, pjSaz = pj.spin_az ?? 0;
    const pjSpin2 = pjSax * pjSax + pjSay * pjSay + pjSaz * pjSaz;

    if (toggles.lorentz) {
        const v2 = pi.vx * pi.vx + pi.vy * pi.vy + pi.vz * pi.vz;
        if (v2 > 1e-30 && pjSpin2 > 1e-30) {
            const rd2 = rawR2 + soft2;
            const rd = Math.sqrt(rd2);
            if (rd > 1e-30) {
                const rhX = dx / rd, rhY = dy / rd, rhZ = dz / rd;
                const mjX = pjSax * (pj.charge / pj.mass);
                const mjY = pjSay * (pj.charge / pj.mass);
                const mjZ = pjSaz * (pj.charge / pj.mass);
                const r3d = rd * rd2;
                const mDotRh = mjX * rhX + mjY * rhY + mjZ * rhZ;
                const bX = (rhX * (3.0 * mDotRh) - mjX) / (4.0 * Math.PI * r3d);
                const bY = (rhY * (3.0 * mDotRh) - mjY) / (4.0 * Math.PI * r3d);
                const bZ = (rhZ * (3.0 * mDotRh) - mjZ) / (4.0 * Math.PI * r3d);
                const flX = ALPHA * pi.charge * (pi.vy * bZ - pi.vz * bY);
                const flY = ALPHA * pi.charge * (pi.vz * bX - pi.vx * bZ);
                const flZ = ALPHA * pi.charge * (pi.vx * bY - pi.vy * bX);
                fx += flX; fy += flY; fz += flZ;
            }
        }
    }

    return { fx, fy, fz };
}

/** Dipole–dipole force on particle i from partner j (μ = (q/m)S). */
export function pairwiseMagneticDipoleForce(pi, pj, dx, dy, dz, r, r2) {
    const piSax = pi.spin_ax ?? 0, piSay = pi.spin_ay ?? 0, piSaz = pi.spin_az ?? 0;
    const pjSax = pj.spin_ax ?? 0, pjSay = pj.spin_ay ?? 0, pjSaz = pj.spin_az ?? 0;
    const piSpin2 = piSax * piSax + piSay * piSay + piSaz * piSaz;
    const pjSpin2 = pjSax * pjSax + pjSay * pjSay + pjSaz * pjSaz;
    if (piSpin2 < 1e-30 || pjSpin2 < 1e-30) return { fx: 0, fy: 0, fz: 0 };

    const miMuX = piSax * (pi.charge / pi.mass);
    const miMuY = piSay * (pi.charge / pi.mass);
    const miMuZ = piSaz * (pi.charge / pi.mass);
    const mjMuX = pjSax * (pj.charge / pj.mass);
    const mjMuY = pjSay * (pj.charge / pj.mass);
    const mjMuZ = pjSaz * (pj.charge / pj.mass);

    const r3 = r * r2;
    const r5 = r3 * r2;
    const miDotR = miMuX * dx + miMuY * dy + miMuZ * dz;
    const mjDotR = mjMuX * dx + mjMuY * dy + mjMuZ * dz;
    const miDotMj = miMuX * mjMuX + miMuY * mjMuY + miMuZ * mjMuZ;
    const coeff = 3.0 * COULOMB_K_FORCE / r5;

    return {
        fx: (dx * (5.0 * miDotR * mjDotR / r2)
            - mjMuX * miDotR - miMuX * mjDotR
            - dx * miDotMj) * coeff,
        fy: (dy * (5.0 * miDotR * mjDotR / r2)
            - mjMuY * miDotR - miMuY * mjDotR
            - dy * miDotMj) * coeff,
        fz: (dz * (5.0 * miDotR * mjDotR / r2)
            - mjMuZ * miDotR - miMuZ * mjDotR
            - dz * miDotMj) * coeff,
    };
}

/** Spin–orbit (L·S) radial force on particle i from partner j. */
export function pairwiseSpinOrbitForce(pi, dx, dy, dz, rawR2, rx, ry, rz) {
    const piSax = pi.spin_ax ?? 0, piSay = pi.spin_ay ?? 0, piSaz = pi.spin_az ?? 0;
    const piSpin2 = piSax * piSax + piSay * piSay + piSaz * piSaz;
    if (piSpin2 < 1e-30) return { fx: 0, fy: 0, fz: 0 };

    const pRelX = pi.vx * pi.mass;
    const pRelY = pi.vy * pi.mass;
    const pRelZ = pi.vz * pi.mass;
    const lOrbX = dy * pRelZ - dz * pRelY;
    const lOrbY = dz * pRelX - dx * pRelZ;
    const lOrbZ = dx * pRelY - dy * pRelX;
    const lDotS = lOrbX * piSax + lOrbY * piSay + lOrbZ * piSaz;
    const rawR = Math.sqrt(rawR2);
    if (rawR < 1e-15) return { fx: 0, fy: 0, fz: 0 };

    const r3raw = rawR * rawR * rawR;
    const m2c2 = pi.mass * pi.mass * C_SPEED * C_SPEED;
    const coeffSo = ALPHA / (2.0 * m2c2 * r3raw);
    const fso = coeffSo * lDotS;
    return { fx: rx * fso, fy: ry * fso, fz: rz * fso };
}

/**
 * Total force on particle index `i` including post-processing.
 */
export function computeForceOnParticle(particles, i, toggles, soft) {
    const pi = particles[i];
    let fx = 0, fy = 0, fz = 0;
    for (let j = 0; j < particles.length; j++) {
        if (j === i) continue;
        const f = computePairwiseForceOnI(pi, particles[j], toggles, soft);
        fx += f.fx; fy += f.fy; fz += f.fz;
    }

    if (toggles.radiation) {
        const pax = pi.prev_ax ?? 0, pay = pi.prev_ay ?? 0, paz = pi.prev_az ?? 0;
        const a2 = pax * pax + pay * pay + paz * paz;
        const v2 = pi.vx * pi.vx + pi.vy * pi.vy + pi.vz * pi.vz;
        if (a2 > 1e-30 && v2 > 1e-30) {
            const q2 = pi.charge * pi.charge;
            const c3 = C_SPEED * C_SPEED * C_SPEED;
            const coeffRad = -(2.0 / 3.0) * ALPHA * q2 / (pi.mass * c3);
            const vMag = Math.sqrt(v2);
            const vHatX = pi.vx / vMag, vHatY = pi.vy / vMag, vHatZ = pi.vz / vMag;
            fx += vHatX * (coeffRad * a2);
            fy += vHatY * (coeffRad * a2);
            fz += vHatZ * (coeffRad * a2);
        }
    }

    if (toggles.relativistic) {
        const v2 = pi.vx * pi.vx + pi.vy * pi.vy + pi.vz * pi.vz;
        const c2 = C_SPEED * C_SPEED;
        const beta2 = v2 / c2;
        if (beta2 > 1e-10 && beta2 < 1.0) {
            const gamma = 1.0 / Math.sqrt(1.0 - beta2);
            const scale = 1.0 / gamma - 1.0;
            fx += fx * scale;
            fy += fy * scale;
            fz += fz * scale;
        }
    }

    return { fx, fy, fz };
}

/** Fill flat Float64Array forces for all particles. */
export function computeAllForces(particles, toggles, soft, outBuf) {
    const n = particles.length;
    if (!outBuf || outBuf.length < n * 3) return null;
    for (let k = 0; k < n * 3; k++) outBuf[k] = 0;
    for (let i = 0; i < n; i++) {
        const f = computeForceOnParticle(particles, i, toggles, soft);
        const i3 = i * 3;
        outBuf[i3] = f.fx;
        outBuf[i3 + 1] = f.fy;
        outBuf[i3 + 2] = f.fz;
    }
    return outBuf;
}

/** PE toggles snapshot from engine state. */
export function peTogglesFromState(pe) {
    return {
        coulomb: !!pe.coulomb,
        gravity: !!pe.gravity,
        exchange: !!pe.exchange,
        strong: !!pe.strong,
        magnetic_dipole: !!pe.magnetic_dipole,
        spin_orbit: !!pe.spin_orbit,
        lorentz: !!pe.lorentz,
        radiation: !!pe.radiation,
        relativistic: !!pe.relativistic,
    };
}
