/** Stable seed ownership shared by field and force streamline jobs. */
import { generateEFieldSeeds, generateBFieldSeeds, generateImportanceSeeds, generateBImportanceSeeds, unionStreamlineSeeds } from '../../../fieldlines.js';
import { fillFieldParticleBuf } from './streamline-integrator.js?v=2';

// Streamline seeds are generated with Math.random() (importance sampling jitter),
// so a re-sweep against an UNCHANGED field would re-randomize the lines — making
// them visibly jump whenever ANY overlay is toggled (the toggle sets
// fieldNeedsUpdate, which forces a sweep even though fieldDataVersion is frozen).
// Cache the generated seeds per (fieldDataVersion, latticeSize): computeStreamlines
// is deterministic given seeds+field, so reuse → byte-identical lines until a real
// tick bumps the version. Particle-anchored seeds are already deterministic;
// caching them is harmless and keeps the path uniform.
export function cachedSeeds(state, type, latticeSize, generate) {
    const version = state.fieldDataVersion || 0;
    let cache = state.streamlineSeedCache;
    if (!cache || cache.version !== version || cache.latticeSize !== latticeSize) {
        cache = state.streamlineSeedCache = { version, latticeSize, e: null, b: null, flux: null };
    }
    if (!cache[type]) cache[type] = generate();
    return cache[type];
}

export function eFieldLineSeeds(activeScale0, state, sampled, latticeSize, p) {
    // E-field: particle-anchored seeds (real sources) UNION importance-sampled
    // |E| peaks so vacuum-field bunches away from charges still get streamlines
    // (and therefore can be detected as knots). Bidirectional integration draws
    // from each seed both toward the source and toward the sink.
    //
    // Use the pre-snapshotted particleData (from the shared per-sweep sample
    // cache) so E and B both seed from the same tick's particle positions
    // (fixes the offset bug).
    const particleData = sampled.particleData ?? activeScale0.getScale0ParticleFrame();
    fillFieldParticleBuf(state, particleData);
    const seeds = cachedSeeds(state, 'e', latticeSize, () => {
        const particleSeeds = particleData.count > 0
            ? generateEFieldSeeds(state.fieldParticleBuf, p.eOffset, Math.ceil(p.maxSeeds * 0.55))
            : [];
        const fieldSeeds = generateImportanceSeeds(sampled.eField, p.maxSeeds);
        return unionStreamlineSeeds(particleSeeds, fieldSeeds, p.maxSeeds);
    });
    return seeds;
}

export function bFieldLineSeeds(activeScale0, state, sampled, latticeSize, p) {
    // B-field is divergence-free (∇·B=0), so lines must form closed loops.
    // Particle ring seeds UNION importance-sampled |B| peaks (perpendicular
    // offset onto the loop circumference). Bidirectional integration is
    // mandatory — half the loop runs each direction.
    //
    // Use the pre-snapshotted particleData so B seeds from the same particle
    // positions as E (same tick, same snapshot — fixes the offset bug).
    const particleData = sampled.particleData ?? activeScale0.getScale0ParticleFrame();
    fillFieldParticleBuf(state, particleData);
    const seeds = cachedSeeds(state, 'b', latticeSize, () => {
        const particleSeeds = particleData.count > 0
            ? generateBFieldSeeds(state.fieldParticleBuf, p.bRadius, Math.ceil(p.maxSeeds * 0.55))
            : [];
        const fieldSeeds = generateBImportanceSeeds(sampled.bField, p.maxSeeds, p.bRadius);
        return unionStreamlineSeeds(particleSeeds, fieldSeeds, p.maxSeeds);
    });
    return seeds;
}

export function fluxLineSeeds(state, sampled, latticeSize, p) {
    // Flux ∇·J carries divergence (sources/sinks), same topology as E.
    // Importance-sample by |J| so streamlines cluster on flux concentrations.
    const seeds = cachedSeeds(state, 'flux', latticeSize, () => generateImportanceSeeds(sampled.fluxVector, p.maxSeeds));
    return seeds;
}
