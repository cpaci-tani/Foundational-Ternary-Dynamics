// ══════════════════════════════════════════════════════════════════════
// Streamline parameter derivation + particle-buffer seeding helpers
// shared between the EM and force overlay builders.
//
// Extracted from field-overlays.js (FO-2). These are pure, state-lite
// utilities: `computeStreamlineParams` is a deterministic function of
// the lattice size, and `fillFieldParticleBuf` grows a reusable scratch
// array on `state` so seed generators don't re-allocate each frame.
// ══════════════════════════════════════════════════════════════════════

// ── Lattice-size-aware streamline parameters ────────────────────────────
// All E/B/Flux streamline knobs scale with the lattice size N so visual
// density stays roughly constant from N=8 to N=128.
//
//   stride       — grow ~ N/16 (clamped 2..8): keeps sample count near constant
//   stepSize     — fixed at 0.5 voxels until N>96, then grows with N so
//                  maxSteps stays bounded (preserves curvature accuracy)
//   maxSteps     — sized so a streamline can travel ~1.5× the lattice diameter
//   seedSpacing  — ~ N/10 (clamped 3..10): finer at small N, coarser at large N
//   maxSeeds     — grows ~ N²/28, capped at 250: more seeds when bigger
//   maxLines     — grows with maxSeeds (drawn count cap inside fieldlines.js)
export function computeStreamlineParams(latticeSize) {
    const stride = Math.max(2, Math.min(8, Math.round(latticeSize / 16)));
    const seedSpacing = Math.max(3, Math.min(10, Math.round(latticeSize / 10)));
    const maxSeeds = Math.max(60, Math.min(250, Math.round((latticeSize * latticeSize) / 28)));
    const stepSize = Math.max(0.5, latticeSize / 96);
    const targetLen = latticeSize * 1.5;
    const maxSteps = Math.max(40, Math.ceil(targetLen / stepSize));
    const maxLines = Math.max(120, Math.min(300, maxSeeds + 50));
    // Particle-anchored seed offsets scale gently with N so seeds at large N
    // don't sit right on top of the source.
    const eOffset = Math.max(2, Math.round(latticeSize / 24));
    const bRadius = Math.max(3, Math.round(latticeSize / 12));
    const stepsScale = Math.ceil(latticeSize / 32); // legacy alias for callers
    return { stride, seedSpacing, maxSeeds, stepSize, maxSteps, maxLines, eOffset, bRadius, stepsScale };
}

/**
 * Populate `state.fieldParticleBuf` (array of {x,y,z}) from a bridge
 * particle frame. The scratch buffer is grown as needed and truncated
 * to the frame count so seed generators downstream see a clean length.
 */
export function fillFieldParticleBuf(state, particleData) {
    while (state.fieldParticleBuf.length < particleData.count) {
        state.fieldParticleBuf.push({ x: 0, y: 0, z: 0 });
    }
    state.fieldParticleBuf.length = particleData.count;
    for (let i = 0; i < particleData.count; i++) {
        state.fieldParticleBuf[i].x = particleData.positions[i * 3];
        state.fieldParticleBuf[i].y = particleData.positions[i * 3 + 1];
        state.fieldParticleBuf[i].z = particleData.positions[i * 3 + 2];
    }
}
