/**
 * nuclear-cloud.js — nuclear structure point cloud generation.
 * Extracted from orbitals.js. Produces proton (red) + neutron (blue)
 * point clouds packed within r_nuc = R0 * A^(1/3).
 */

import { defaultNeutronCount } from '../elements.js';

// ── Nuclear Structure Constants ──────────────────────────────────────

export const R0_NUCLEAR = 0.5;           // nuclear radius scale: r_nuc = R0 * A^(1/3) (enlarged for visibility)
export const POINTS_PER_NUCLEON = 8;     // cloud points per nucleon (denser for better visibility)
const PROTON_COLOR  = [1.00, 0.30, 0.20];  // warm red
const NEUTRON_COLOR = [0.30, 0.50, 0.90];  // cool blue
const NUCLEON_SIZE  = 2.0;        // larger nucleon points for visibility
const NUCLEUS_CENTER_SIZE = 3.5;  // bright white center glow point

/**
 * Generate nuclear cloud points: protons (red) + neutrons (blue)
 * packed within a sphere of radius R0 * A^(1/3).
 */
export function generateNuclearCloud(Z) {
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

/**
 * Get the number of nuclear cloud points for element Z.
 * Used to distinguish nuclear vs orbital points in the template.
 */
export function nucleonCount(Z) {
    const N = defaultNeutronCount(Z);
    return 1 + (Z + N) * POINTS_PER_NUCLEON;  // +1 for center glow point
}

/**
 * Get nuclear shell radius for an atom with mass number A.
 * Used by viewport.js for strong-force shell mesh sizing.
 */
export function nuclearShellRadius(Z) {
    const N = defaultNeutronCount(Z);
    const A = Z + N;
    return R0_NUCLEAR * Math.cbrt(Math.max(A, 1)) * 1.8; // 1.8x for visual glow region
}
