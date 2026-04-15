/**
 * Force Field Sampling & Visualization
 *
 * Samples Coulomb/ionic potential and force vectors on a 2D grid (XZ plane)
 * for rendering as a heatmap + arrow overlay. Works for both ParticleEngine
 * (Scale 1) and AtomEngine (Scale 2).
 */

import { ALPHA, G_N } from './constants.js';

const AE_K_COULOMB = 2.0;  // must match wasm-bridge-dag.js
const FOUR_PI = 4.0 * Math.PI;

// ── Grid Generation ──────────────────────────────────────────────────

/**
 * Generate a uniform grid of sample points on the XZ plane (y=0).
 * @param {number} extent  — half-width: grid spans [-extent, +extent]
 * @param {number} res     — points per axis (total = res × res)
 * @returns {{ positions: Float32Array, count: number, resolution: number, extent: number }}
 */
export function generateGridXZ(extent, res) {
    const count = res * res;
    const positions = new Float32Array(count * 3);
    const step = (2 * extent) / (res - 1);
    let idx = 0;
    for (let iz = 0; iz < res; iz++) {
        for (let ix = 0; ix < res; ix++) {
            positions[idx++] = -extent + ix * step;  // x
            positions[idx++] = 0;                     // y = 0
            positions[idx++] = -extent + iz * step;  // z
        }
    }
    return { positions, count, resolution: res, extent };
}

// ── PE Field Sampling (Coulomb + Gravity) ────────────────────────────

/**
 * Sample electrostatic potential and force at grid points from PE particles.
 * @param {{ positions: Float32Array, charges: Float32Array, masses: Float32Array, count: number }} sources
 * @param {Float32Array} gridPos — flat [x,y,z,...] from generateGridXZ
 * @param {number} gridCount
 * @param {number} [soft=0.5] — softening length (avoids singularity)
 * @returns {{ potentials: Float32Array, forces: Float32Array, maxPotential: number, maxForce: number }}
 */
export function samplePEField(sources, gridPos, gridCount, soft = 0.5) {
    const potentials = new Float32Array(gridCount);
    const forces = new Float32Array(gridCount * 3);
    const soft2 = soft * soft;
    let maxPot = 0, maxF = 0;

    for (let g = 0; g < gridCount; g++) {
        const gx = gridPos[g * 3], gy = gridPos[g * 3 + 1], gz = gridPos[g * 3 + 2];
        let phi = 0, fx = 0, fy = 0, fz = 0;

        for (let i = 0; i < sources.count; i++) {
            const dx = gx - sources.positions[i * 3];
            const dy = gy - sources.positions[i * 3 + 1];
            const dz = gz - sources.positions[i * 3 + 2];
            const r2 = dx * dx + dy * dy + dz * dz + soft2;
            const r = Math.sqrt(r2);
            const qi = sources.charges[i];

            // Coulomb potential: φ = α·q / (4π·r)
            phi += ALPHA * qi / (FOUR_PI * r);

            // E-field at grid point: E = α·q/(4π·r²)·r̂  (r̂ = dx/r, from source to grid)
            const fMag = ALPHA * qi / (FOUR_PI * r2);
            fx += fMag * dx / r;
            fy += fMag * dy / r;
            fz += fMag * dz / r;
        }

        potentials[g] = phi;
        forces[g * 3] = fx;
        forces[g * 3 + 1] = fy;
        forces[g * 3 + 2] = fz;

        const absPhi = Math.abs(phi);
        if (absPhi > maxPot) maxPot = absPhi;
        const fMag = Math.sqrt(fx * fx + fy * fy + fz * fz);
        if (fMag > maxF) maxF = fMag;
    }

    return { potentials, forces, maxPotential: maxPot, maxForce: maxF };
}

// ── PE Coulomb-Only Field Sampling ────────────────────────────────────

/**
 * Sample Coulomb-only potential and force (no gravity) at grid points.
 * Same interface as samplePEField but excludes gravitational contribution.
 */
export function samplePECoulombOnly(sources, gridPos, gridCount, soft = 0.5) {
    const potentials = new Float32Array(gridCount);
    const forces = new Float32Array(gridCount * 3);
    const soft2 = soft * soft;
    let maxPot = 0, maxF = 0;

    for (let g = 0; g < gridCount; g++) {
        const gx = gridPos[g * 3], gy = gridPos[g * 3 + 1], gz = gridPos[g * 3 + 2];
        let phi = 0, fx = 0, fy = 0, fz = 0;

        for (let i = 0; i < sources.count; i++) {
            const dx = gx - sources.positions[i * 3];
            const dy = gy - sources.positions[i * 3 + 1];
            const dz = gz - sources.positions[i * 3 + 2];
            const r2 = dx * dx + dy * dy + dz * dz + soft2;
            const r = Math.sqrt(r2);
            const qi = sources.charges[i];

            phi += ALPHA * qi / (FOUR_PI * r);
            // E-field at grid point: E = α·q/(4π·r²)·r̂  (r̂ = dx/r, from source to grid)
            const fMag = ALPHA * qi / (FOUR_PI * r2);
            fx += fMag * dx / r;
            fy += fMag * dy / r;
            fz += fMag * dz / r;
        }

        potentials[g] = phi;
        forces[g * 3] = fx;
        forces[g * 3 + 1] = fy;
        forces[g * 3 + 2] = fz;

        const absPhi = Math.abs(phi);
        if (absPhi > maxPot) maxPot = absPhi;
        const fm = Math.sqrt(fx * fx + fy * fy + fz * fz);
        if (fm > maxF) maxF = fm;
    }

    return { potentials, forces, maxPotential: maxPot, maxForce: maxF };
}

// ── PE Gravity-Only Field Sampling ────────────────────────────────────

/**
 * Sample gravitational potential and force at grid points from PE particles.
 * Gravity is attractive: F = -G_N * m / r² · r̂ (toward source).
 * @param {{ positions: Float32Array, masses: Float32Array, count: number }} sources
 */
export function samplePEGravityField(sources, gridPos, gridCount, soft = 0.5) {
    const potentials = new Float32Array(gridCount);
    const forces = new Float32Array(gridCount * 3);
    const soft2 = soft * soft;
    let maxPot = 0, maxF = 0;

    for (let g = 0; g < gridCount; g++) {
        const gx = gridPos[g * 3], gy = gridPos[g * 3 + 1], gz = gridPos[g * 3 + 2];
        let phi = 0, fx = 0, fy = 0, fz = 0;

        for (let i = 0; i < sources.count; i++) {
            const dx = gx - sources.positions[i * 3];
            const dy = gy - sources.positions[i * 3 + 1];
            const dz = gz - sources.positions[i * 3 + 2];
            const r2 = dx * dx + dy * dy + dz * dz + soft2;
            const r = Math.sqrt(r2);
            const mi = sources.masses[i];

            // Gravitational potential: φ = -G_N * m / r
            phi -= G_N * mi / r;

            // Gravitational force on unit test mass: F = -G_N·m/r²·r̂ (toward source)
            // dx = grid - source → points AWAY from source, so negate for attraction
            const fMag = -G_N * mi / r2;
            fx += fMag * dx / r;
            fy += fMag * dy / r;
            fz += fMag * dz / r;
        }

        potentials[g] = phi;
        forces[g * 3] = fx;
        forces[g * 3 + 1] = fy;
        forces[g * 3 + 2] = fz;

        const absPhi = Math.abs(phi);
        if (absPhi > maxPot) maxPot = absPhi;
        const fm = Math.sqrt(fx * fx + fy * fy + fz * fz);
        if (fm > maxF) maxF = fm;
    }

    return { potentials, forces, maxPotential: maxPot, maxForce: maxF };
}

// ── PE Coulomb Field for 3D Streamlines ───────────────────────────────

/**
 * Compute Coulomb E-field at a single 3D point from particle sources.
 * Used as the field function for RK4 streamline integration.
 * @returns {function(number, number, number): [number, number, number]}
 */
export function makePECoulombFieldFn(sources, soft = 0.5) {
    const soft2 = soft * soft;
    return (x, y, z) => {
        let fx = 0, fy = 0, fz = 0;
        for (let i = 0; i < sources.count; i++) {
            const dx = x - sources.positions[i * 3];
            const dy = y - sources.positions[i * 3 + 1];
            const dz = z - sources.positions[i * 3 + 2];
            const r2 = dx * dx + dy * dy + dz * dz + soft2;
            const r = Math.sqrt(r2);
            const qi = sources.charges[i];

            // E = α·q/(4π·r²)·r̂  (r̂ = dx/r, from source to field point)
            const fMag = ALPHA * qi / (FOUR_PI * r2);
            fx += fMag * dx / r;
            fy += fMag * dy / r;
            fz += fMag * dz / r;
        }
        return [fx, fy, fz];
    };
}

// ── AE Field Sampling (Ionic Coulomb) ────────────────────────────────

/**
 * Sample ionic potential and force at grid points from AE atoms.
 * Only charged atoms contribute (neutral atoms have no long-range ionic field).
 * @param {{ positions: Float32Array, charges: Float32Array, count: number }} sources
 * @param {Float32Array} gridPos
 * @param {number} gridCount
 * @param {number} [soft=0.3]
 * @returns {{ potentials: Float32Array, forces: Float32Array, maxPotential: number, maxForce: number }}
 */
export function sampleAEField(sources, gridPos, gridCount, soft = 0.3) {
    const potentials = new Float32Array(gridCount);
    const forces = new Float32Array(gridCount * 3);
    const soft2 = soft * soft;
    let maxPot = 0, maxF = 0;

    for (let g = 0; g < gridCount; g++) {
        const gx = gridPos[g * 3], gy = gridPos[g * 3 + 1], gz = gridPos[g * 3 + 2];
        let phi = 0, fx = 0, fy = 0, fz = 0;

        for (let i = 0; i < sources.count; i++) {
            const qi = sources.charges[i];
            if (qi === 0) continue;  // skip neutral atoms

            const dx = gx - sources.positions[i * 3];
            const dy = gy - sources.positions[i * 3 + 1];
            const dz = gz - sources.positions[i * 3 + 2];
            const r2 = dx * dx + dy * dy + dz * dz + soft2;
            const r = Math.sqrt(r2);

            // Ionic potential: φ = K_COULOMB·q / r
            phi += AE_K_COULOMB * qi / r;

            // Ionic E-field: E = K_COULOMB·q/r²·r̂  (r̂ = dx/r, from source to grid)
            const fMag = AE_K_COULOMB * qi / r2;
            fx += fMag * dx / r;
            fy += fMag * dy / r;
            fz += fMag * dz / r;
        }

        potentials[g] = phi;
        forces[g * 3] = fx;
        forces[g * 3 + 1] = fy;
        forces[g * 3 + 2] = fz;

        const absPhi = Math.abs(phi);
        if (absPhi > maxPot) maxPot = absPhi;
        const fMag = Math.sqrt(fx * fx + fy * fy + fz * fz);
        if (fMag > maxF) maxF = fMag;
    }

    return { potentials, forces, maxPotential: maxPot, maxForce: maxF };
}

// ── Color Maps ───────────────────────────────────────────────────────

/**
 * Divergent colormap: blue (negative) → dark (zero) → red (positive).
 * @param {number} value    — signed potential
 * @param {number} maxAbs   — normalization bound
 * @returns {[number, number, number]} RGB in [0,1]
 */
export function potentialToColor(value, maxAbs) {
    if (maxAbs < 1e-20) return [0.08, 0.09, 0.13];
    const t = Math.max(-1, Math.min(1, value / maxAbs));
    if (t < 0) {
        const s = -t;  // 0→1 as more negative
        return [0.08 + 0.02 * s, 0.12 + 0.45 * s, 0.18 + 0.62 * s];
    } else {
        const s = t;   // 0→1 as more positive
        return [0.18 + 0.72 * s, 0.10 + 0.08 * s, 0.08 + 0.02 * s];
    }
}

/**
 * Flux magnitude colormap: dark blue → cyan → white → yellow → red.
 * Designed for volume rendering of flux density fields (Scale 0).
 * @param {number} mag     — flux magnitude |J|
 * @param {number} maxFlux — normalization bound
 * @returns {[number, number, number]} RGB in [0,1]
 */
export function fluxToColor(mag, maxFlux) {
    if (maxFlux < 1e-20) return [0.02, 0.03, 0.08];
    const t = Math.max(0, Math.min(1, mag / maxFlux));
    if (t < 0.25) {
        // dark blue → blue
        const s = t / 0.25;
        return [0.02 + 0.03 * s, 0.03 + 0.12 * s, 0.08 + 0.52 * s];
    } else if (t < 0.5) {
        // blue → cyan
        const s = (t - 0.25) / 0.25;
        return [0.05 + 0.05 * s, 0.15 + 0.65 * s, 0.60 + 0.30 * s];
    } else if (t < 0.75) {
        // cyan → white/yellow
        const s = (t - 0.5) / 0.25;
        return [0.10 + 0.85 * s, 0.80 + 0.15 * s, 0.90 - 0.30 * s];
    } else {
        // yellow → red
        const s = (t - 0.75) / 0.25;
        return [0.95 + 0.05 * s, 0.95 - 0.65 * s, 0.60 - 0.55 * s];
    }
}

/**
 * Sequential colormap: dark → blue → cyan → yellow-white.
 * @param {number} mag    — force magnitude
 * @param {number} maxMag — normalization bound
 * @returns {[number, number, number]} RGB in [0,1]
 */
export function magnitudeToColor(mag, maxMag) {
    if (maxMag < 1e-20) return [0.05, 0.08, 0.15];
    const t = Math.max(0, Math.min(1, mag / maxMag));
    if (t < 0.33) {
        const s = t / 0.33;
        return [0.05 + 0.05 * s, 0.08 + 0.32 * s, 0.15 + 0.55 * s];
    } else if (t < 0.66) {
        const s = (t - 0.33) / 0.33;
        return [0.10 + 0.15 * s, 0.40 + 0.35 * s, 0.70 - 0.05 * s];
    } else {
        const s = (t - 0.66) / 0.34;
        return [0.25 + 0.75 * s, 0.75 + 0.25 * s, 0.65 + 0.15 * s];
    }
}
