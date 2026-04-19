/**
 * quantum-chemistry.js — pure QM helpers extracted from orbitals.js
 *
 * Aufbau filling, configuration exceptions, Slater's shielding rules,
 * real spherical harmonic angular probabilities, and rejection-sampled
 * orbital point clouds. No DOM, no Three.js.
 */

import {
    SLATER_SAME_1S, SLATER_SAME_NL, SLATER_INNER_SP, SLATER_DEEP_CORE,
} from '../constants.js';

// ── Aufbau Fill Order ───────────────────────────────────────────────
// [n, l, maxElectrons] in standard energy-ascending order
export const AUFBAU = [
    [1,0,2],  [2,0,2],  [2,1,6],  [3,0,2],  [3,1,6],  [4,0,2],  [3,2,10],
    [4,1,6],  [5,0,2],  [4,2,10], [5,1,6],  [6,0,2],  [4,3,14], [5,2,10],
    [6,1,6],  [7,0,2],  [5,3,14], [6,2,10], [7,1,6],
];

// ── Electron Configuration Exceptions ───────────────────────────────
// Key: Z. Value: full subshell list [n, l, count] replacing the Aufbau
// prediction for the outermost shells. Core electrons follow Aufbau.
// Only elements with confirmed deviations from Aufbau are listed.
export const EXCEPTIONS = {
    24:  { replace: [[4,0],[3,2]], with: [[4,0,1],[3,2,5]] },     // Cr
    29:  { replace: [[4,0],[3,2]], with: [[4,0,1],[3,2,10]] },    // Cu
    41:  { replace: [[5,0],[4,2]], with: [[5,0,1],[4,2,4]] },     // Nb
    42:  { replace: [[5,0],[4,2]], with: [[5,0,1],[4,2,5]] },     // Mo
    44:  { replace: [[5,0],[4,2]], with: [[5,0,1],[4,2,7]] },     // Ru
    45:  { replace: [[5,0],[4,2]], with: [[5,0,1],[4,2,8]] },     // Rh
    46:  { replace: [[5,0],[4,2]], with: [[5,0,0],[4,2,10]] },    // Pd
    47:  { replace: [[5,0],[4,2]], with: [[5,0,1],[4,2,10]] },    // Ag
    57:  { replace: [[4,3]],       with: [[5,2,1]] },              // La: 5d1 instead of 4f1
    58:  { replace: [[4,3]],       with: [[4,3,1],[5,2,1]] },     // Ce: 4f1 5d1
    64:  { replace: [[4,3]],       with: [[4,3,7],[5,2,1]] },     // Gd: 4f7 5d1
    78:  { replace: [[6,0],[5,2]], with: [[6,0,1],[5,2,9]] },     // Pt
    79:  { replace: [[6,0],[5,2]], with: [[6,0,1],[5,2,10]] },    // Au
    89:  { replace: [[5,3]],       with: [[6,2,1]] },              // Ac: 6d1 instead of 5f1
    90:  { replace: [[5,3]],       with: [[6,2,2]] },              // Th: 6d2
    91:  { replace: [[5,3]],       with: [[5,3,2],[6,2,1]] },     // Pa
    92:  { replace: [[5,3]],       with: [[5,3,3],[6,2,1]] },     // U
    93:  { replace: [[5,3]],       with: [[5,3,4],[6,2,1]] },     // Np
    96:  { replace: [[5,3]],       with: [[5,3,7],[6,2,1]] },     // Cm
    103: { replace: [[5,3],[6,2]], with: [[5,3,14],[7,1,1]] },    // Lr: 7p1 instead of 6d1
};

/**
 * Compute electron configuration for element Z.
 * Returns array of { n, l, count } sorted by (n, l).
 */
export function electronConfig(Z) {
    // Build Aufbau baseline
    const config = [];
    let remaining = Z;
    for (const [n, l, max] of AUFBAU) {
        if (remaining <= 0) break;
        const count = Math.min(remaining, max);
        config.push({ n, l, count });
        remaining -= count;
    }

    // Apply exceptions
    const exc = EXCEPTIONS[Z];
    if (!exc) return config;

    // Find which subshells to replace
    const replaceKeys = new Set(exc.replace.map(([n, l]) => `${n},${l}`));
    const result = config.filter(s => !replaceKeys.has(`${s.n},${s.l}`));

    // Add the corrected subshells
    for (const [n, l, count] of exc.with) {
        if (count > 0) result.push({ n, l, count });
    }

    result.sort((a, b) => a.n - b.n || a.l - b.l);
    return result;
}

// ── Slater's Rules for Effective Nuclear Charge ─────────────────────

/**
 * Compute Z_eff for an electron in subshell (n, l) of element Z.
 * Uses Slater's shielding rules.
 */
export function slaterZeff(Z, targetN, targetL) {
    const config = electronConfig(Z);
    const isDF = targetL >= 2;
    let sigma = 0;

    for (const sub of config) {
        if (sub.n === targetN && sub.l === targetL) {
            // Same subshell: each other electron shields by SAME_NL (SAME_1S for 1s)
            const s = (targetN === 1 && targetL === 0) ? SLATER_SAME_1S : SLATER_SAME_NL;
            sigma += (sub.count - 1) * s;
        } else if (isDF) {
            // For d/f electrons: all inner groups shield completely
            if (sub.n < targetN || (sub.n === targetN && sub.l < targetL)) {
                sigma += sub.count * SLATER_DEEP_CORE;
            }
        } else {
            // For s/p electrons
            if (sub.n === targetN && sub.l <= 1 && targetL <= 1 && sub.l !== targetL) {
                // Same n, s-p grouped together
                sigma += sub.count * SLATER_SAME_NL;
            } else if (sub.n === targetN - 1) {
                sigma += sub.count * SLATER_INNER_SP;
            } else if (sub.n < targetN - 1) {
                sigma += sub.count * SLATER_DEEP_CORE;
            }
        }
    }

    return Math.max(Z - sigma, 1.0);
}

// ── Angular Probability Functions (Real Spherical Harmonics) ────────

export function angularProb(l, m, cosT, sinT, phi) {
    if (l === 0) return 1.0; // s: isotropic

    if (l === 1) {
        switch (m) {
            case  0: return cosT * cosT;                          // pz
            case  1: return sinT * sinT * Math.cos(phi) ** 2;    // px
            case -1: return sinT * sinT * Math.sin(phi) ** 2;    // py
        }
    }

    if (l === 2) {
        const sinT2 = sinT * sinT;
        switch (m) {
            case  0: { const f = 3 * cosT * cosT - 1; return f * f * 0.25; }  // dz²
            case  1: return sinT2 * cosT * cosT * Math.cos(phi) ** 2 * 4;     // dxz
            case -1: return sinT2 * cosT * cosT * Math.sin(phi) ** 2 * 4;     // dyz
            case  2: { const f = sinT2 * Math.cos(2 * phi); return f * f; }   // dx²-y²
            case -2: { const f = sinT2 * Math.sin(2 * phi); return f * f; }   // dxy
        }
    }

    if (l === 3) {
        const sinT2 = sinT * sinT;
        const cosT2 = cosT * cosT;
        switch (m) {
            case  0: { const f = cosT * (5 * cosT2 - 3); return f * f * 0.25; }          // fz³
            case  1: { const f = sinT * (5 * cosT2 - 1) * Math.cos(phi); return f * f * 0.125; } // fxz²
            case -1: { const f = sinT * (5 * cosT2 - 1) * Math.sin(phi); return f * f * 0.125; } // fyz²
            case  2: { const f = sinT2 * cosT * Math.cos(2 * phi); return f * f; }       // fz(x²-y²)
            case -2: { const f = sinT2 * cosT * Math.sin(2 * phi); return f * f; }       // fxyz
            case  3: { const f = sinT * sinT2 * Math.cos(3 * phi); return f * f; }       // fx(x²-3y²)
            case -3: { const f = sinT * sinT2 * Math.sin(3 * phi); return f * f; }       // fy(3x²-y²)
        }
    }

    return 1.0; // fallback
}

// ── Orbital Point Sampling (Rejection Method) ───────────────────────

/**
 * Sample numPoints from the probability density of orbital (n, l, m).
 * Returns Float32Array of [dx, dy, dz, dx, dy, dz, ...] offsets.
 */
export function sampleOrbital(n, l, m, a0Eff, numPoints) {
    const offsets = new Float32Array(numPoints * 3);
    const rMax = (n * n + n * (l + 1)) * a0Eff * 2.5;
    const TWO_PI = 2 * Math.PI;

    let count = 0;
    let attempts = 0;
    const maxAttempts = numPoints * 100;

    while (count < numPoints && attempts < maxAttempts) {
        attempts++;

        const r = Math.random() * rMax;
        const cosTheta = 2 * Math.random() - 1;
        const sinTheta = Math.sqrt(1 - cosTheta * cosTheta);
        const phi = TWO_PI * Math.random();

        // Radial: r^(2+2l) * exp(-2r/(n*a0)) — simplified hydrogen-like
        const rScaled = 2 * r / (n * a0Eff);
        const radial = Math.pow(rScaled, 2 + 2 * l) * Math.exp(-rScaled);

        // Normalize radial by its peak value: peak at rScaled = 2+2l,
        // peak value = (2+2l)^(2+2l) * exp(-(2+2l))
        const peakR = 2 + 2 * l;
        const radialNorm = peakR > 0 ? Math.pow(peakR, peakR) * Math.exp(-peakR) : 1;

        // Angular
        const angular = angularProb(l, m, cosTheta, sinTheta, phi);

        const prob = (radial / radialNorm) * angular;
        if (Math.random() < prob) {
            offsets[count * 3]     = r * sinTheta * Math.cos(phi);
            offsets[count * 3 + 1] = r * sinTheta * Math.sin(phi);
            offsets[count * 3 + 2] = r * cosTheta;
            count++;
        }
    }

    return offsets;
}
