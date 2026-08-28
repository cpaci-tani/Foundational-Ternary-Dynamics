/**
 * Fixed-voxel physics extents for Scale-0 scenarios.
 *
 * Changing lattice size N adds or removes total volume. Localized phenomena
 * (electrons, Gaussian blobs, dipole separation, cluster seeds) keep the
 * same absolute voxel footprint authored at L=33. Waves and field patterns
 * that intentionally span the box may still use N (e.g. k = 2πn/N, corner
 * placement at N/4) so extra volume is available for propagation.
 *
 * Independent of the visual-only `flux-point-scale` slider (render glyph size).
 */

/** Lattice side length scenarios were originally authored at. */
export const PHYSICS_REFERENCE_L = 33;

/**
 * @param {number} n — integer voxel half-width, offset, or radius
 * @returns {number}
 */
export function physicsVox(n) {
    return Math.max(1, Math.round(n));
}

/**
 * @param {number} s — Gaussian σ in voxels
 * @returns {number}
 */
export function physicsSigma(s) {
    return Math.max(0.5, s);
}

/**
 * @param {number} midF — float lattice center
 * @param {number} halfWidth — voxel half-width (not scaled with N)
 * @returns {{ lo: number, hi: number }}
 */
export function physicsBand(midF, halfWidth) {
    const hw = physicsVox(halfWidth);
    return { lo: Math.floor(midF) - hw, hi: Math.ceil(midF) + hw };
}

/**
 * Bound helpers for scenario ctx — call once per setupScenario.
 * @param {number} N — runtime lattice side (used for clamp only)
 */
export function createPhysicsLatticeHelpers(N) {
    return {
        physicsRefL: PHYSICS_REFERENCE_L,
        /** Fixed integer voxel extent — invariant when N changes. */
        vox: physicsVox,
        /** Fixed Gaussian σ in voxels — invariant when N changes. */
        sigma: physicsSigma,
        /** Symmetric index band around midF with fixed half-width. */
        band: (midF, halfWidth) => physicsBand(midF, halfWidth),
        /** Clamp lattice index into [0, N−1]. */
        clamp: (i) => Math.max(0, Math.min(N - 1, i)),
    };
}
