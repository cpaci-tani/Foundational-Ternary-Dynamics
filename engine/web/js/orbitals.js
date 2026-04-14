/**
 * Electron Orbital Cloud Generator + Nuclear Structure
 *
 * Generates physically-accurate electron probability clouds for each element
 * using quantum mechanical orbital shapes (s, p, d, f), Slater's rules for
 * effective nuclear charge, and rejection sampling from real spherical harmonics.
 *
 * Also renders nuclear structure: protons (red) and neutrons (blue) as dense
 * point clusters at the atom center, with nuclear radius scaling as A^(1/3).
 *
 * Each element's cloud is generated once and cached as a template of position
 * offsets, colors, and sizes. The expandAEToOrbitalCloud() function stamps
 * these templates at each atom's position every frame.
 */

import { defaultNeutronCount } from './elements.js';

// ── Aufbau Fill Order ───────────────────────────────────────────────
// [n, l, maxElectrons] in standard energy-ascending order
const AUFBAU = [
    [1,0,2],  [2,0,2],  [2,1,6],  [3,0,2],  [3,1,6],  [4,0,2],  [3,2,10],
    [4,1,6],  [5,0,2],  [4,2,10], [5,1,6],  [6,0,2],  [4,3,14], [5,2,10],
    [6,1,6],  [7,0,2],  [5,3,14], [6,2,10], [7,1,6],
];

const L_NAMES = ['s', 'p', 'd', 'f'];

// ── Electron Configuration Exceptions ───────────────────────────────
// Key: Z. Value: full subshell list [n, l, count] replacing the Aufbau
// prediction for the outermost shells. Core electrons follow Aufbau.
// Only elements with confirmed deviations from Aufbau are listed.
const EXCEPTIONS = {
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
function electronConfig(Z) {
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
function slaterZeff(Z, targetN, targetL) {
    const config = electronConfig(Z);
    const isDF = targetL >= 2;
    let sigma = 0;

    for (const sub of config) {
        if (sub.n === targetN && sub.l === targetL) {
            // Same subshell: each other electron shields by 0.35 (0.30 for 1s)
            const s = (targetN === 1 && targetL === 0) ? 0.30 : 0.35;
            sigma += (sub.count - 1) * s;
        } else if (isDF) {
            // For d/f electrons: all inner groups shield by 1.00
            if (sub.n < targetN || (sub.n === targetN && sub.l < targetL)) {
                sigma += sub.count * 1.00;
            }
        } else {
            // For s/p electrons
            if (sub.n === targetN && sub.l <= 1 && targetL <= 1 && sub.l !== targetL) {
                // Same n, s-p grouped together
                sigma += sub.count * 0.35;
            } else if (sub.n === targetN - 1) {
                sigma += sub.count * 0.85;
            } else if (sub.n < targetN - 1) {
                sigma += sub.count * 1.00;
            }
        }
    }

    return Math.max(Z - sigma, 1.0);
}

// ── Angular Probability Functions (Real Spherical Harmonics) ────────

function angularProb(l, m, cosT, sinT, phi) {
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
function sampleOrbital(n, l, m, a0Eff, numPoints) {
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

// ── Orbital Color Palette ───────────────────────────────────────────

const ORBITAL_COLORS = {
    0: [0.40, 0.75, 1.00],  // s — sky blue
    1: [0.30, 0.90, 0.45],  // p — green
    2: [1.00, 0.70, 0.20],  // d — gold
    3: [0.85, 0.30, 0.70],  // f — magenta
};

// ── Nuclear Structure Constants ──────────────────────────────────────

const R0_NUCLEAR = 0.5;           // nuclear radius scale: r_nuc = R0 * A^(1/3) (enlarged for visibility)
const POINTS_PER_NUCLEON = 8;     // cloud points per nucleon (denser for better visibility)
const PROTON_COLOR  = [1.00, 0.30, 0.20];  // warm red
const NEUTRON_COLOR = [0.30, 0.50, 0.90];  // cool blue
const NUCLEON_SIZE  = 2.0;        // larger nucleon points for visibility
const NUCLEUS_CENTER_SIZE = 3.5;  // bright white center glow point

/**
 * Generate nuclear cloud points: protons (red) + neutrons (blue)
 * packed within a sphere of radius R0 * A^(1/3).
 */
function generateNuclearCloud(Z) {
    const N = defaultNeutronCount(Z);
    const A = Z + N;
    const rNuc = R0_NUCLEAR * Math.cbrt(Math.max(A, 1));

    const offsets = [];
    const colors  = [];
    const sizes   = [];

    // Helper: random point inside unit sphere, scaled to rNuc
    function randomInSphere() {
        let x, y, z;
        do {
            x = Math.random() * 2 - 1;
            y = Math.random() * 2 - 1;
            z = Math.random() * 2 - 1;
        } while (x * x + y * y + z * z > 1);
        return [x * rNuc, y * rNuc, z * rNuc];
    }

    // Bright white center glow point
    offsets.push(0, 0, 0);
    colors.push(1.0, 1.0, 1.0);
    sizes.push(NUCLEUS_CENTER_SIZE);

    // Protons
    for (let p = 0; p < Z; p++) {
        for (let i = 0; i < POINTS_PER_NUCLEON; i++) {
            const [x, y, z] = randomInSphere();
            offsets.push(x, y, z);
            colors.push(PROTON_COLOR[0], PROTON_COLOR[1], PROTON_COLOR[2]);
            sizes.push(NUCLEON_SIZE);
        }
    }

    // Neutrons
    for (let n = 0; n < N; n++) {
        for (let i = 0; i < POINTS_PER_NUCLEON; i++) {
            const [x, y, z] = randomInSphere();
            offsets.push(x, y, z);
            colors.push(NEUTRON_COLOR[0], NEUTRON_COLOR[1], NEUTRON_COLOR[2]);
            sizes.push(NUCLEON_SIZE);
        }
    }

    return { offsets, colors, sizes, count: offsets.length / 3, rNuc };
}

// ── Per-Element Template Cache ──────────────────────────────────────

const _templateCache = new Map();

// Visual scale factor: maps Slater a0_eff (atomic units) to display units.
// Tuned so hydrogen 1s cloud has radius ~3 display units (visible but not huge).
const A0_DISPLAY = 1.8;

// m-values for each orbital type
const M_VALUES = {
    0: [0],                          // s: 1 orbital
    1: [0, 1, -1],                   // p: pz, px, py
    2: [0, 1, -1, 2, -2],           // d: dz², dxz, dyz, dx²-y², dxy
    3: [0, 1, -1, 2, -2, 3, -3],   // f: all 7
};

/**
 * Generate the orbital cloud template for element Z.
 * Returns { offsets, colors, sizes, count }.
 */
function generateTemplate(Z) {
    if (_templateCache.has(Z)) return _templateCache.get(Z);

    const config = electronConfig(Z);
    const maxN = Math.max(...config.map(s => s.n));

    // Collect all cloud points: nuclear structure first, then electron orbitals
    const allOffsets = [];
    const allColors = [];
    const allSizes = [];

    // ── Nuclear structure (protons + neutrons) ──
    const nuc = generateNuclearCloud(Z);
    for (let i = 0; i < nuc.count; i++) {
        allOffsets.push(nuc.offsets[i * 3], nuc.offsets[i * 3 + 1], nuc.offsets[i * 3 + 2]);
        allColors.push(nuc.colors[i * 3], nuc.colors[i * 3 + 1], nuc.colors[i * 3 + 2]);
        allSizes.push(nuc.sizes[i]);
    }

    for (const sub of config) {
        const { n, l, count: eCount } = sub;
        const zEff = slaterZeff(Z, n, l);
        const a0Eff = (n * n / zEff) * A0_DISPLAY;

        // Points per subshell: inner shells get fewer, valence gets more
        const isValence = (n === maxN) || (n === maxN - 1 && l >= 2);
        const basePoints = isValence ? 5 : 3;
        const totalSubshellPoints = Math.max(6, eCount * basePoints);

        // Distribute points across the m-values (individual orbitals)
        const mVals = M_VALUES[l];
        // Electrons fill orbitals: Hund's rule — spread across m first
        // For visualization, distribute points proportional to occupancy
        const pointsPerOrbital = Math.max(3, Math.round(totalSubshellPoints / mVals.length));

        // Brightness: inner shells dimmer, valence brighter
        const fade = 0.3 + 0.7 * ((n - 1) / Math.max(maxN - 1, 1));
        const [br, bg, bb] = ORBITAL_COLORS[l];

        for (const m of mVals) {
            const pts = sampleOrbital(n, l, m, a0Eff, pointsPerOrbital);
            for (let i = 0; i < pointsPerOrbital; i++) {
                const dx = pts[i * 3];
                const dy = pts[i * 3 + 1];
                const dz = pts[i * 3 + 2];

                allOffsets.push(dx, dy, dz);

                // Color with fade for depth perception
                allColors.push(br * fade, bg * fade, bb * fade);

                // Size: small for cloud points, slightly larger for valence
                const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const distNorm = dist / (a0Eff * 2 + 0.01);
                const brightness = Math.exp(-distNorm * distNorm * 0.8);
                allSizes.push(isValence ? 1.2 + brightness * 1.0 : 0.8 + brightness * 0.6);
            }
        }
    }

    const count = allOffsets.length / 3;
    const template = {
        offsets: new Float32Array(allOffsets),
        colors: new Float32Array(allColors),
        sizes: new Float32Array(allSizes),
        count,
    };

    _templateCache.set(Z, template);
    return template;
}

// ── Cloud Expansion (Stamps Templates at Atom Positions) ────────────

const MAX_CLOUD = 100000;
const _cloudPos  = new Float32Array(MAX_CLOUD * 3);
const _cloudCol  = new Float32Array(MAX_CLOUD * 3);
const _cloudSize = new Float32Array(MAX_CLOUD);
const _cloudAtomMap = new Int32Array(MAX_CLOUD);  // cloud point index → atom array index

/**
 * Expand atom positions + atomic numbers into a full orbital cloud.
 * Includes a nucleus center point for each atom (CPK colored, larger).
 *
 * @param {object} atomData — { positions, colors, sizes, atomicNums, count }
 * @param {number} [time=0] — animation time for breathing effect
 * @returns {{ positions, colors, sizes, count }}
 */
export function expandAEToOrbitalCloud(atomData, time = 0) {
    let out = 0;

    for (let i = 0; i < atomData.count && out < MAX_CLOUD - 1; i++) {
        const cx = atomData.positions[i * 3];
        const cy = atomData.positions[i * 3 + 1];
        const cz = atomData.positions[i * 3 + 2];
        const Z = atomData.atomicNums ? atomData.atomicNums[i] : 1;

        // Template includes nuclear structure + electron orbitals
        const tmpl = generateTemplate(Z);
        const nucCount = nucleonCount(Z);  // number of nuclear cloud points
        const n = Math.min(tmpl.count, MAX_CLOUD - out);

        for (let j = 0; j < n; j++) {
            // Nuclear points get minimal breathing; electron orbitals get full breathing
            const isNuclear = j < nucCount;
            const wiggle = isNuclear ? 0.015 : 0.12;

            // Golden-angle phase for organic breathing
            const phase = j * 2.39996323;
            const fx = Math.sin(time * 1.7 + phase) * wiggle;
            const fy = Math.sin(time * 2.3 + phase * 1.3) * wiggle;
            const fz = Math.sin(time * 1.1 + phase * 0.7) * wiggle;

            _cloudPos[out * 3]     = cx + tmpl.offsets[j * 3]     + fx;
            _cloudPos[out * 3 + 1] = cy + tmpl.offsets[j * 3 + 1] + fy;
            _cloudPos[out * 3 + 2] = cz + tmpl.offsets[j * 3 + 2] + fz;

            _cloudCol[out * 3]     = tmpl.colors[j * 3];
            _cloudCol[out * 3 + 1] = tmpl.colors[j * 3 + 1];
            _cloudCol[out * 3 + 2] = tmpl.colors[j * 3 + 2];

            _cloudSize[out] = tmpl.sizes[j];
            _cloudAtomMap[out] = i;  // map cloud point → atom array index
            out++;
        }
    }

    return { positions: _cloudPos, colors: _cloudCol, sizes: _cloudSize, count: out, atomMap: _cloudAtomMap };
}

/**
 * Get the number of nuclear cloud points for element Z.
 * Used to distinguish nuclear vs orbital points in the template.
 */
function nucleonCount(Z) {
    const N = defaultNeutronCount(Z);
    return 1 + (Z + N) * POINTS_PER_NUCLEON;  // +1 for center glow point
}

/**
 * Get the electron configuration string for display.
 * e.g., "1s2 2s2 2p6 3s1" for Sodium (Z=11).
 */
export function configString(Z) {
    return electronConfig(Z)
        .map(({ n, l, count }) => `${n}${L_NAMES[l]}${count}`)
        .join(' ');
}

/** Clear the template cache (e.g., if display scale changes). */
export function clearOrbitalCache() {
    _templateCache.clear();
}

// ── Exported Orbital Data for Shell/Lobe Rendering ────────────────

export { electronConfig, slaterZeff, A0_DISPLAY, R0_NUCLEAR, defaultNeutronCount };

/**
 * Get nuclear shell radius for an atom with mass number A.
 * Used by viewport.js for strong-force shell mesh sizing.
 */
export function nuclearShellRadius(Z) {
    const N = defaultNeutronCount(Z);
    const A = Z + N;
    return R0_NUCLEAR * Math.cbrt(Math.max(A, 1)) * 1.8; // 1.8x for visual glow region
}

/**
 * Generate bonding electron density cloud between bonded atom pairs.
 * Creates Gaussian ellipsoidal clouds along bond axes.
 *
 * @param {object} atomData — from aeGetAtomData() with positions, bonds, bondOrders, atomicNums
 * @returns {{ positions, colors, sizes, count }}
 */
export function generateBondingCloud(atomData) {
    if (!atomData || atomData.bondCount === 0) {
        return { positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), count: 0 };
    }

    const maxPts = atomData.bondCount * 24; // max 24 pts per bond (triple)
    const positions = new Float32Array(maxPts * 3);
    const colors = new Float32Array(maxPts * 3);
    const sizes = new Float32Array(maxPts);
    let out = 0;

    // Build id→index lookup
    const idToIdx = new Map();
    for (let i = 0; i < atomData.count; i++) {
        idToIdx.set(atomData.ids[i], i);
    }

    for (let bi = 0; bi < atomData.bondCount; bi++) {
        const idA = atomData.bonds[bi * 2];
        const idB = atomData.bonds[bi * 2 + 1];
        const iA = idToIdx.get(idA);
        const iB = idToIdx.get(idB);
        if (iA === undefined || iB === undefined) continue;

        const order = atomData.bondOrders ? atomData.bondOrders[bi] : 1;
        const ptsPerBond = 8 * order; // 8 single, 16 double, 24 triple

        const ax = atomData.positions[iA * 3], ay = atomData.positions[iA * 3 + 1], az = atomData.positions[iA * 3 + 2];
        const bx = atomData.positions[iB * 3], by = atomData.positions[iB * 3 + 1], bz = atomData.positions[iB * 3 + 2];

        const mx = (ax + bx) / 2, my = (ay + by) / 2, mz = (az + bz) / 2;
        const dx = bx - ax, dy = by - ay, dz = bz - az;
        const bondLen = Math.sqrt(dx * dx + dy * dy + dz * dz);
        if (bondLen < 1e-10) continue;

        const ux = dx / bondLen, uy = dy / bondLen, uz = dz / bondLen;
        const sigZ = 0.3 * bondLen; // spread along bond axis
        const sigR = 0.3;           // tight radial spread

        for (let p = 0; p < ptsPerBond && out < maxPts; p++) {
            // Gaussian along bond axis
            const t = gaussianRandom() * sigZ;
            // Gaussian radial offset perpendicular to bond
            const r1 = gaussianRandom() * sigR;
            const r2 = gaussianRandom() * sigR;

            // Build perpendicular vectors
            let px1, py1, pz1;
            if (Math.abs(ux) < 0.9) {
                px1 = 0; py1 = -uz; pz1 = uy;
            } else {
                px1 = -uz; py1 = 0; pz1 = ux;
            }
            const len1 = Math.sqrt(px1 * px1 + py1 * py1 + pz1 * pz1);
            px1 /= len1; py1 /= len1; pz1 /= len1;
            // Second perpendicular = u × p1
            const px2 = uy * pz1 - uz * py1;
            const py2 = uz * px1 - ux * pz1;
            const pz2 = ux * py1 - uy * px1;

            positions[out * 3]     = mx + ux * t + px1 * r1 + px2 * r2;
            positions[out * 3 + 1] = my + uy * t + py1 * r1 + py2 * r2;
            positions[out * 3 + 2] = mz + uz * t + pz1 * r1 + pz2 * r2;

            // Light cyan color for bonding electrons
            colors[out * 3]     = 0.5;
            colors[out * 3 + 1] = 0.8;
            colors[out * 3 + 2] = 1.0;

            sizes[out] = 1.0;
            out++;
        }
    }

    return { positions, colors, sizes, count: out };
}

/** Box-Muller Gaussian random (mean 0, std 1). */
function gaussianRandom() {
    const u1 = Math.random();
    const u2 = Math.random();
    return Math.sqrt(-2.0 * Math.log(u1 + 1e-20)) * Math.cos(2.0 * Math.PI * u2);
}
