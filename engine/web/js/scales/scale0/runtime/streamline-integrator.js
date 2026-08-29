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
// All E/B/Flux/force streamline knobs scale with the lattice size N so visual
// density stays useful from N=9 to N=181 while keeping each worker transaction
// bounded. E, B, flux, and force flow jobs are staggered by the overlay
// scheduler; only the no-Worker emergency fallback runs RK4 on the UI thread.
//
//   stride       — grow ~ N/16 (clamped 2..8): keeps sample count near constant
//   stepSize     — fixed at 0.5 voxels until N>96, then grows with N so
//                  maxSteps stays bounded (preserves curvature accuracy)
//   maxSteps     — sized so a streamline can travel ~1.5× the lattice diameter
//   seedSpacing  — ~ N/10 (clamped 3..10): finer at small N, coarser at large N
//   maxSeeds     — deterministic per-size work cap, scaled by user density
//   maxLines     — exactly maxSeeds (every builder already caps its seed union)
//
// `density` and `length` are deliberately bounded to 0..1.  The default is the
// audited maximum, not an unbounded quality multiplier: users may trade visual
// detail for headroom without accidentally defeating the 60 FPS guardrail.
function streamlineSeedCap(latticeSize) {
    if (latticeSize <= 17) return 60;
    if (latticeSize <= 25) return 40;
    if (latticeSize <= 33) return 36;
    return 24;
}

function unitInterval(value, fallback = 1) {
    const numeric = Number(value);
    if (!Number.isFinite(numeric)) return fallback;
    return Math.max(0, Math.min(1, numeric));
}

export function computeStreamlineParams(latticeSize, {
    inThreadWasm = false,
    density = 1,
    length = 1,
} = {}) {
    const stride = Math.max(2, Math.min(8, Math.round(latticeSize / 16)));
    const seedSpacing = Math.max(3, Math.min(10, Math.round(latticeSize / 10)));
    const requestedSeeds = Math.max(60, Math.min(250, Math.round((latticeSize * latticeSize) / 28)));
    const densityScale = unitInterval(density);
    const lengthScale = unitInterval(length);
    let maxSeeds = Math.max(1, Math.round(Math.min(requestedSeeds, streamlineSeedCap(latticeSize)) * densityScale));
    const stepSize = Math.max(0.5, latticeSize / 96);
    const targetLen = latticeSize * 1.5;
    const fullSteps = Math.max(40, Math.ceil(targetLen / stepSize));
    const maxSteps = Math.max(1, Math.round(fullSteps * lengthScale));
    let maxLines = maxSeeds;
    // In the emergency no-Worker fallback, physics and RK4 integration stack
    // in the same UI frame. Retain full integration length (and therefore
    // loop/topology shape), while bounding seed/line density so one fallback
    // rebuild remains inside the 60 FPS interaction budget.
    if (inThreadWasm) {
        maxSeeds = Math.min(maxSeeds, 16);
        maxLines = maxSeeds;
    }
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
