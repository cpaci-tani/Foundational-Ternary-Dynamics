// ══════════════════════════════════════════════════════════════════════
// Lazy field-sample cache — visual overlay layer only.
//
// Overlays are read-only views of whatever the physics bridge already computed
// on the last tick. They MUST NOT flip physics toggles or change tick cadence.
//
// Each sampler kind is pulled at most once per overlay sweep (shared stride),
// on first use by a job that needs it. Multiple overlays that derive from the
// same underlying quantity (E, B, flux, divJ, …) therefore piggyback on one
// O(N³/stride³) pass instead of repeating it at sweep start.
//
// Particle positions for E/B streamlines are snapshotted once per sweep when
// either field is active, preserving the coherence guarantee from sampleFieldState.
// ══════════════════════════════════════════════════════════════════════

/** Public sample slot → getScale0FieldSamples kind. */
const KIND_BY_SLOT = {
    fluxVector: 'fluxVector',
    poynting: 'poynting',
    eField: 'e',
    bField: 'b',
    divergence: 'divJ',
    vorticity: 'vorticity',
    helicity: 'helicity',
    kretschmann: 'kretschmann',
    latency: 'latency',
    poissonLatency: 'poissonLatency',
    fisher: 'fisher',
    coherence: 'coherence',
    curlJ: 'curlJ',
    state: 'state',
    gaussResidual: 'gaussResidual',
};

/** Slots that always sample at stride 1 regardless of sweep stride. */
const STRIDE_ONE_SLOTS = new Set(['state', 'gaussResidual']);

/** Scalar overlay flag → sample slots required before compute*Frame runs. */
export const SCALAR_SAMPLE_DEPS = {
    showPsiSquared: ['fluxVector'],
    showPhase: ['fluxVector'],
    showLagrangianDensity: ['fluxVector', 'poynting', 'divergence', 'eField'],
    showEntropyDensity: ['fluxVector'],
    // The finite Poisson well is preferred when latency_field is active;
    // fluxVector supplies the exact local potential of the default G_N*delta_2|J|
    // force law when no Poisson well exists.
    showGravPotential: ['poissonLatency', 'fluxVector'],
    showEmEnergy: ['eField', 'bField'],
    showChargeDensity: ['divergence'],
    showVorticity: ['vorticity'],
    showHorizon: ['latency'],
    showEPressure: ['eField'],
    showBPressure: ['bField'],
    showStateField: ['state'],
    showLatency: ['latency'],
    showGaussResidual: ['gaussResidual'],
};

/**
 * @param {object} fieldCapability — scale0 capability (mock, proxy shadow, or WASM)
 * @param {object|null} acScale0 — particle-frame source for E/B seed coherence
 * @param {number} stride — sweep stride from computeStreamlineParams
 */
export function createFieldSampleCache(fieldCapability, acScale0, stride, kindOverrides = null) {
    /** @type {Record<string, object>} */
    const sampled = {};
    /** kind@stride → sample record (dedupes aliases that share a kind+stride) */
    const byKind = new Map();

    function ensureSample(slot) {
        if (sampled[slot] !== undefined) return sampled[slot];
        const kind = kindOverrides?.[slot] || KIND_BY_SLOT[slot];
        if (!kind) {
            sampled[slot] = null;
            return null;
        }
        const effectiveStride = STRIDE_ONE_SLOTS.has(slot) ? 1 : stride;
        const cacheKey = `${kind}@${effectiveStride}`;
        let result = byKind.get(cacheKey);
        if (result === undefined) {
            result = fieldCapability.getScale0FieldSamples({ kind, stride: effectiveStride });
            byKind.set(cacheKey, result);
        }
        sampled[slot] = result;
        return result;
    }

    function requestedKeys() {
        return [...byKind.keys()];
    }

    function ensureSamples(slots) {
        for (let i = 0; i < slots.length; i++) ensureSample(slots[i]);
    }

    function ensureScalarDeps(flagKey) {
        const deps = SCALAR_SAMPLE_DEPS[flagKey];
        if (deps) ensureSamples(deps);
    }

    function ensureParticleData() {
        if (sampled.particleData !== undefined) return sampled.particleData;
        sampled.particleData = acScale0 ? acScale0.getScale0ParticleFrame() : null;
        return sampled.particleData;
    }

    return { sampled, ensureSample, ensureSamples, ensureScalarDeps, ensureParticleData, requestedKeys };
}

/**
 * Per-sweep cache for getScale0ForceField — EM/gravity/strong share stride
 * within one overlay sweep but may be queried from multiple jobs.
 *
 * @param {object} fieldCapability
 */
export function createForceFieldCache(fieldCapability) {
    const byKey = new Map();
    return {
        get(type, stride) {
            const key = `${type}@${stride}`;
            let result = byKey.get(key);
            if (result === undefined) {
                result = fieldCapability.getScale0ForceField(type, stride);
                byKey.set(key, result);
            }
            return result;
        },
        requestedKeys() { return [...byKey.keys()]; },
        clear() {
            byKey.clear();
        },
    };
}

/**
 * Eager path retained for tests / legacy callers — same output as the old
 * sampleFieldState, but routed through the shared cache implementation.
 */
export function buildSampleSnapshot(fieldCapability, flags, stride, acScale0) {
    const cache = createFieldSampleCache(fieldCapability, acScale0, stride);
    if (flags.showEField || flags.showBField) cache.ensureParticleData();

    const needFlux = flags.showFluxLines || flags.showDualSubstrate || flags.showChirality ||
        flags.showForceWeak || flags.showPsiSquared || flags.showPhase ||
        flags.showLagrangianDensity || flags.showEntropyDensity || flags.showGravPotential;
    if (needFlux) cache.ensureSample('fluxVector');
    if (flags.showPoynting || flags.showLagrangianDensity) cache.ensureSample('poynting');
    if (flags.showEField || flags.showEmEnergy || flags.showEPressure || flags.showLagrangianDensity) cache.ensureSample('eField');
    if (flags.showBField || flags.showEmEnergy || flags.showBPressure) cache.ensureSample('bField');
    if (flags.showDivField || flags.showLagrangianDensity || flags.showChargeDensity) {
        cache.ensureSample('divergence');
    }
    if (flags.showVorticity) cache.ensureSample('vorticity');
    if (flags.showHorizon || flags.showLatency) cache.ensureSample('latency');
    if (flags.showForceWeak) cache.ensureSample('curlJ');
    if (flags.showGravPotential) cache.ensureSample('poissonLatency');
    if (flags.showStateField) cache.ensureSample('state');
    if (flags.showGaussResidual) cache.ensureSample('gaussResidual');

    return cache.sampled;
}
