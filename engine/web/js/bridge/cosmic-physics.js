/**
 * Cosmic scale-5 force kernel.
 *
 * Extracted from mock-scale5.js (MS5-2). The kernel runs the O(N^2) Gadget-2
 * gravity + optional SPH-like sub-grid physics. It operates on a CosmicMockBridge
 * instance's `_bodies` array, reusing (or lazily allocating) the instance's
 * `_soa` SoA scratch buffer to avoid GC churn across ticks.
 *
 * Invoked via `.call(bridge)` from CosmicMockBridge._computeForces so `this`
 * binds to the bridge instance. All state mutations (body accelerations,
 * temperatures, internal energies) happen on the bridge's own bodies.
 *
 * Unit system: G = G_N = 0.01 (FTD ontic chain).
 */

import { G_N } from '../constants.js';

// Fixed softening per body type (Gadget-2 convention: constant, energy-conserving).
// 2026-04-26 (Wave 2H): the prior "mirrored from mock-scale5.js" note
// was stale — mock-scale5.js does not declare its own copy; it calls
// computeCosmicForces.call(this) on this module's tables. This is now
// the single source of truth. If a constants.js entry is added later,
// migrate from here.
const SOFTENING = {
    [-3]: 6.0,  // DARK_ENERGY
    [-2]: 3.0,  // QUASAR
    [-1]: 1.5,  // BLACK_HOLE
    [0]:  8.0,  // DARK_MATTER
    [1]:  3.0,  // GAS
    [2]:  2.5,  // STAR
    [3]:  2.0,  // NEUTRON_STAR
    [4]:  3.0,  // NEBULA
    [5]:  2.0,  // WHITE_DWARF
};
const SOFTENING_SQ = {
    [-3]: 36.0, [-2]: 9.0, [-1]: 2.25,
    [0]: 64.0, [1]: 9.0, [2]: 6.25,
    [3]: 4.0, [4]: 9.0, [5]: 4.0
};

/**
 * Run the gravity + sub-grid force kernel against `this._bodies`.
 * Call via `computeCosmicForces.call(bridgeInstance)`.
 */
export function computeCosmicForces(TYPE) {
    const G = G_N;
    const n = this._bodies.length;

    // JIT SoA buffers — grown lazily, reused across ticks.
    if (!this._soa || this._soa.n < n) {
        const MathMaxOffset = 1000;
        const capacity = n + MathMaxOffset;
        this._soa = {
            n: capacity,
            x: new Float64Array(capacity),
            y: new Float64Array(capacity),
            z: new Float64Array(capacity),
            mass: new Float64Array(capacity),
            soft: new Float64Array(capacity),
            softSq: new Float64Array(capacity),
            ax: new Float64Array(capacity),
            ay: new Float64Array(capacity),
            az: new Float64Array(capacity)
        };
    }

    const soa = this._soa;
    const X = soa.x, Y = soa.y, Z = soa.z, M = soa.mass;
    const SOFT = soa.soft, SQ = soa.softSq;
    const AX = soa.ax, AY = soa.ay, AZ = soa.az;

    // 1. Flatten JS objects into typed arrays.
    for (let i = 0; i < n; i++) {
        const b = this._bodies[i];
        X[i] = b.x;
        Y[i] = b.y;
        Z[i] = b.z;
        M[i] = b.mass;
        SOFT[i] = SOFTENING[b.type] || 2.0;
        SQ[i] = SOFTENING_SQ[b.type] || 4.0;
        AX[i] = 0.0;
        AY[i] = 0.0;
        AZ[i] = 0.0;
    }

    // 2. O(N^2) cache-local pairwise gravity — V8 auto-vectorizes this.
    for (let i = 0; i < n; i++) {
        const bix = X[i], biy = Y[i], biz = Z[i], bim = M[i];
        const s_i = SOFT[i], sq_i = SQ[i];
        let ax = AX[i], ay = AY[i], az = AZ[i];
        for (let j = i + 1; j < n; j++) {
            const s_j = SOFT[j];
            const eps2 = s_i > s_j ? sq_i : SQ[j];
            const dx = X[j] - bix;
            const dy = Y[j] - biy;
            const dz = Z[j] - biz;
            const r2 = dx * dx + dy * dy + dz * dz + eps2;
            const invR3 = 1.0 / (r2 * Math.sqrt(r2));
            const f_j = G * M[j] * invR3;
            const f_i = G * bim * invR3;
            ax += f_j * dx;
            ay += f_j * dy;
            az += f_j * dz;
            AX[j] -= f_i * dx;
            AY[j] -= f_i * dy;
            AZ[j] -= f_i * dz;
        }
        AX[i] = ax;
        AY[i] = ay;
        AZ[i] = az;
    }

    // 3. Restitute accelerations back to JS body objects.
    for (let i = 0; i < n; i++) {
        const b = this._bodies[i];
        b.ax = AX[i];
        b.ay = AY[i];
        b.az = AZ[i];
    }

    // Sub-grid physics only active in select scenarios (BH accretion / FTD collapse).
    if (!this._enableSubgrid) return;

    const T = TYPE;
    const baseSoft2 = this._softening * this._softening;
    const bodies = this._bodies;
    const nb = bodies.length;

    const gasIdx  = [];
    const starIdx = [];
    const bhIdx   = [];
    for (let i = 0; i < nb; i++) {
        const t = bodies[i].type;
        if (t === T.GAS || t === T.NEBULA) {
            gasIdx.push(i);
        } else if (t === T.STAR || t === T.NEUTRON_STAR || t === T.WHITE_DWARF) {
            starIdx.push(i);
        } else if (t === T.BLACK_HOLE || t === T.QUASAR) {
            bhIdx.push(i);
        }
    }
    const nGas  = gasIdx.length;
    const nStar = starIdx.length;
    const nBH   = bhIdx.length;

    // Tidal spaghettification (radial stretch only).
    for (let bi = 0; bi < nBH; bi++) {
        const bh = bodies[bhIdx[bi]];
        const bhMass = bh.mass;
        const bhx = bh.x, bhy = bh.y, bhz = bh.z;
        const bhId = bh.id;
        const r_tidal = Math.max(8.0, Math.cbrt(bhMass) * 1.5);
        const r_tidal2 = r_tidal * r_tidal;
        const tidalK = 2.0 * G * bhMass * 0.3;
        for (let i = 0; i < nb; i++) {
            const b = bodies[i];
            if (b.id === bhId) continue;
            const dx = b.x - bhx, dy = b.y - bhy, dz = b.z - bhz;
            const r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > r_tidal2 || r2 < 0.01) continue;
            const r = Math.sqrt(r2);
            const invR = 1.0 / r;
            const tidalStrength = tidalK / (r2 * r);
            b.ax += tidalStrength * dx * invR;
            b.ay += tidalStrength * dy * invR;
            b.az += tidalStrength * dz * invR;
        }
    }

    // Gas cooling — reduces internal energy (not velocity drag).
    const coolRadius2 = baseSoft2 * 25;
    for (let gi = 0; gi < nGas; gi++) {
        const b = bodies[gasIdx[gi]];
        const bx = b.x, by = b.y, bz = b.z;
        let localDensity = b.mass;
        for (let gj = 0; gj < nGas; gj++) {
            if (gj === gi) continue;
            const other = bodies[gasIdx[gj]];
            const dx = bx - other.x, dy = by - other.y, dz = bz - other.z;
            const dr2 = dx * dx + dy * dy + dz * dz;
            if (dr2 < coolRadius2) localDensity += other.mass;
        }
        const coolingRate = Math.min(0.0002, 0.000002 * localDensity);
        b.internal_energy = Math.max(0.001, b.internal_energy * (1 - coolingRate));
        b.temperature = Math.max(100, b.internal_energy * 1000);
    }

    // Gas pressure (SPH-like repulsion).
    const h_press = this._softening * 2.5;
    const h_press2 = h_press * h_press;
    for (let gi = 0; gi < nGas; gi++) {
        const bi_idx = gasIdx[gi];
        const bi = bodies[bi_idx];
        const bix = bi.x, biy = bi.y, biz = bi.z;
        const biMass = bi.mass;
        const biE = bi.internal_energy;
        for (let gj = gi + 1; gj < nGas; gj++) {
            const bj = bodies[gasIdx[gj]];
            const dx = bj.x - bix, dy = bj.y - biy, dz = bj.z - biz;
            const r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > h_press2 || r2 < 1e-10) continue;
            const r = Math.sqrt(r2);
            const q = r / h_press;
            const T_avg = 0.5 * (biE + bj.internal_energy);
            const pressScale = 1.0 + T_avg * 0.1;
            const fmag = G * pressScale * 0.3 * (biMass + bj.mass) * (1 - q) * (1 - q) / (r2 + baseSoft2);
            const invR = 1.0 / r;
            const fx = fmag * dx * invR, fy = fmag * dy * invR, fz = fmag * dz * invR;
            bi.ax -= fx; bi.ay -= fy; bi.az -= fz;
            bj.ax += fx; bj.ay += fy; bj.az += fz;
        }
    }

    // Stellar radiation pressure on gas.
    const radMaxR2 = 400;
    const radInvC = 1.0 / (4 * Math.PI * 0.577);
    for (let si = 0; si < nStar; si++) {
        const star = bodies[starIdx[si]];
        if (!(star.luminosity > 0)) continue;
        const sx = star.x, sy = star.y, sz = star.z;
        const starK = star.luminosity * radInvC * 0.001;
        for (let gi = 0; gi < nGas; gi++) {
            const gas = bodies[gasIdx[gi]];
            const dx = gas.x - sx, dy = gas.y - sy, dz = gas.z - sz;
            const r2 = dx * dx + dy * dy + dz * dz + baseSoft2;
            if (r2 > radMaxR2) continue;
            const r = Math.sqrt(r2);
            const f_rad = starK / r2;
            const invR = 1.0 / r;
            gas.ax += f_rad * dx * invR;
            gas.ay += f_rad * dy * invR;
            gas.az += f_rad * dz * invR;
        }
    }
}
