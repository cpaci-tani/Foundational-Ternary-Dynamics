/**
 * Pairs a promoted cluster seed's mass (N·K_B, the physics-bearing
 * convention `phase_forces_integrate_clusters` uses) against the sum of its
 * constituent voxels' scale-bridge masses (max(density, K_B) per voxel —
 * an [IMPOSED] display-only convention; see
 * docs/theory/05_particles/REF_SCALE1_DYNAMICS_FTD_FORM.md §3). Surfacing
 * both, tagged, makes that documented convention tension visible in the UI
 * instead of only in a doc footnote.
 *
 * @param {{mass:number, size:number}} seed - a promotion.js cluster seed
 * @param {number[]|Float64Array|null} voxelMasses - per-voxel masses from
 *   the matching coarsenToParticles snapshot's member voxels, or null/
 *   undefined when no voxel snapshot is available for this seed.
 */
export function compareClusterToVoxelMass(seed, voxelMasses) {
    const clusterMass = seed.mass;
    if (!voxelMasses || voxelMasses.length === 0) {
        return {
            clusterMass, voxelMass: null, delta: null,
            clusterTag: '[DERIVED-linear]/[SMC]', voxelTag: '[IMPOSED]',
        };
    }
    let voxelMass = 0;
    for (let i = 0; i < voxelMasses.length; i++) voxelMass += voxelMasses[i];
    return {
        clusterMass, voxelMass, delta: voxelMass - clusterMass,
        clusterTag: '[DERIVED-linear]/[SMC]', voxelTag: '[IMPOSED]',
    };
}
