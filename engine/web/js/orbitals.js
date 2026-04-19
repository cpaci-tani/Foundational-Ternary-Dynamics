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
import {
    electronConfig,
    slaterZeff,
    sampleOrbital,
} from './orbitals/quantum-chemistry.js';
import {
    generateNuclearCloud,
    nucleonCount,
    nuclearShellRadius,
    R0_NUCLEAR,
    POINTS_PER_NUCLEON,
} from './orbitals/nuclear-cloud.js';

const L_NAMES = ['s', 'p', 'd', 'f'];

// ── Orbital Color Palette ───────────────────────────────────────────

const ORBITAL_COLORS = {
    0: [0.40, 0.75, 1.00],  // s — sky blue
    1: [0.30, 0.90, 0.45],  // p — green
    2: [1.00, 0.70, 0.20],  // d — gold
    3: [0.85, 0.30, 0.70],  // f — magenta
};

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

export { electronConfig, slaterZeff, A0_DISPLAY, R0_NUCLEAR, defaultNeutronCount, nuclearShellRadius };

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
