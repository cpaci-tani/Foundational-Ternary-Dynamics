/**
 * Scale-1 initial-condition helpers — velocities derived from the live force
 * kernel at t=0, not closed-form orbital formulas.
 */

import { C_SPEED } from '../../constants.js';
import {
    computeForceOnParticle,
    peTogglesFromState,
} from '../../bridge/pe-force-kernel.js';

function normalize3(v) {
    const m = Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (m < 1e-30) return [0, 0, 0];
    return [v[0] / m, v[1] / m, v[2] / m];
}

function cross(a, b) {
    return [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ];
}

/**
 * Tangential unit vector perpendicular to position relative to `center`.
 */
export function defaultTangent(pos, center, preferred = [0, 1, 0]) {
    const rx = pos[0] - center[0];
    const ry = pos[1] - center[1];
    const rz = pos[2] - center[2];
    const rHat = normalize3([rx, ry, rz]);
    let t = cross(rHat, preferred);
    let tMag = Math.sqrt(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
    if (tMag < 1e-8) t = cross(rHat, [0, 0, 1]);
    return normalize3(t);
}

/**
 * Compute circular-orbit speed from instantaneous force balance:
 *   m v² / r = |F_inward|
 * Uses the same toggles/softening as the running engine.
 */
export function equilibriumOrbitSpeed(particles, idx, pe, center = [0, 0, 0]) {
    const p = particles[idx];
    const rx = p.x - center[0];
    const ry = p.y - center[1];
    const rz = p.z - center[2];
    const r = Math.sqrt(rx * rx + ry * ry + rz * rz);
    if (r < 1e-12) return 0;

    const toggles = peTogglesFromState(pe);
    const savedV = [p.vx, p.vy, p.vz];
    p.vx = 0; p.vy = 0; p.vz = 0;
    const f = computeForceOnParticle(particles, idx, toggles, pe.soft);
    p.vx = savedV[0]; p.vy = savedV[1]; p.vz = savedV[2];

    const rHatX = rx / r, rHatY = ry / r, rHatZ = rz / r;
    const fRad = f.fx * rHatX + f.fy * rHatY + f.fz * rHatZ;
    const fInward = -fRad;
    if (fInward <= 0) return 0;
    const v = Math.sqrt(fInward * r / p.mass);
    return Math.min(v, C_SPEED * 0.95);
}

/**
 * Apply tangential equilibrium velocity to a particle (by engine id).
 */
export function applyEquilibriumOrbit(state, particleId, options = {}) {
    if (!state._pe) return false;
    const idx = state._pe.particles.findIndex(p => p.id === particleId);
    if (idx < 0) return false;

    const center = options.center || [0, 0, 0];
    const sign = options.sign ?? 1;
    const p = state._pe.particles[idx];
    const tangent = options.tangent || defaultTangent([p.x, p.y, p.z], center);
    const speed = equilibriumOrbitSpeed(state._pe.particles, idx, state._pe, center);
    p.vx = tangent[0] * speed * sign;
    p.vy = tangent[1] * speed * sign;
    p.vz = tangent[2] * speed * sign;
    state._pe.forces = null;
    return true;
}

/**
 * Recompute equilibrium orbit speeds after every body in the group is placed.
 * Fixes sequential seed bugs (e.g. helium's first electron ignoring e–e repulsion).
 * Safe for Coulomb/gravity ICs — equilibrium depends on positions, not partner v.
 *
 * @param {object} bridge - MockBridge / WasmBridge with peApplyEquilibriumOrbit
 * @param {Array<{particleId:number, center?:number[], tangent?:number[], sign?:number}>} entries
 */
export function applyEquilibriumOrbitBatch(bridge, entries) {
    if (!bridge?.peApplyEquilibriumOrbit || !entries?.length) return;
    for (const { particleId, center, tangent, sign } of entries) {
        const opts = {};
        if (center) opts.center = center;
        if (tangent) opts.tangent = tangent;
        if (sign !== undefined) opts.sign = sign;
        bridge.peApplyEquilibriumOrbit(particleId, opts);
    }
}

/**
 * Seed a hydrogen-like system: dynamic nucleus + lepton with force-derived orbit.
 */
export function seedHydrogenLike(bridge, {
    r, nucleusCatalog, nucleusCharge, nucleusMass,
    leptonCatalog, leptonCharge, leptonMass, RE,
    leptonPos = null,
}) {
    const nid = bridge.peAddParticle(nucleusCatalog, nucleusCharge, 0, 0, 0, 0, 0, 0, nucleusMass, RE);
    const lp = leptonPos || [r, 0, 0];
    const eid = bridge.peAddParticle(leptonCatalog, leptonCharge, lp[0], lp[1], lp[2], 0, 0, 0, leptonMass, RE);
    bridge.peApplyEquilibriumOrbit(eid, { tangent: [0, 1, 0] });
    return { nucleusId: nid, leptonId: eid };
}

/**
 * Seed equal-mass binary (positronium-style): both particles get independent
 * force-derived tangential speeds at ±separation/2.
 */
export function seedBinaryOrbit(bridge, {
    catalogA, chargeA, massA,
    catalogB, chargeB, massB,
    separation, RE,
}) {
    const half = separation * 0.5;
    const idA = bridge.peAddParticle(catalogA, chargeA, half, 0, 0, 0, 0, 0, massA, RE);
    const idB = bridge.peAddParticle(catalogB, chargeB, -half, 0, 0, 0, 0, 0, massB, RE);
    applyEquilibriumOrbitBatch(bridge, [
        { particleId: idA, center: [0, 0, 0], tangent: [0, 1, 0], sign: 1 },
        { particleId: idB, center: [0, 0, 0], tangent: [0, 1, 0], sign: -1 },
    ]);
    return { idA, idB };
}

/**
 * Composite nucleus: single dynamical particle with total charge Z and mass ≈ A·m_p.
 * Pass `nucleusMass` to override (e.g. Δ⁺⁺, measured hadron mass).
 */
export function spawnCompositeNucleus(bridge, Z, A, mp, RE, catalogId = 'proton', nucleusMass = null) {
    return bridge.peAddParticle(
        catalogId, Z, 0, 0, 0, 0, 0, 0, nucleusMass ?? A * mp, RE);
}

/** Multi-electron ion around a composite nucleus (all particles dynamic). */
export function seedAtomicIon(bridge, {
    Z, A, mp, me, RE, r, electrons = 1, nucleusCatalog = 'proton', nucleusMass = null,
}) {
    spawnCompositeNucleus(bridge, Z, A, mp, RE, nucleusCatalog, nucleusMass);
    const orbitSpecs = [];
    for (let k = 0; k < electrons; k++) {
        const sign = k % 2 === 0 ? 1 : -1;
        const x = (electrons === 1 ? r : sign * r);
        const eid = bridge.peAddParticle('electron', -1, x, 0, 0, 0, 0, 0, me, RE);
        orbitSpecs.push({
            particleId: eid,
            tangent: [0, 1, 0],
            sign: electrons === 1 ? 1 : sign,
        });
    }
    applyEquilibriumOrbitBatch(bridge, orbitSpecs);
    return { electronIds: orbitSpecs.map(s => s.particleId) };
}
