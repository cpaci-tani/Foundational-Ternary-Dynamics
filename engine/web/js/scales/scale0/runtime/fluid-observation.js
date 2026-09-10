/** Read-only field summaries. No constitutive law or fluid evolution lives here. */
import { commonSampleProvenance, exactCounter, compareExactCounters } from '../../../lib/exact-counter.js';
import { computeEmEnergyFrame } from './overlay-frames.js';

function summarize(sample, vector = false) {
    const n = sample?.count;
    if (!Number.isSafeInteger(n) || n < 1) return null;
    const data = vector ? sample.vectors : sample.values;
    if (!data || data.length < n * (vector ? 3 : 1)) return null;
    let min = Infinity, max = -Infinity, sum = 0;
    for (let i = 0; i < n; i++) {
        const value = vector ? Math.hypot(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]) : data[i];
        if (!Number.isFinite(value)) return null;
        min = Math.min(min, value); max = Math.max(max, value); sum += value;
    }
    return Object.freeze({ min, max, mean: sum / n, count: n });
}

export function fieldObservationProvenance(samples, owner) {
    const common = commonSampleProvenance(samples);
    if (common) return common;
    // The shared cache can retain reads from earlier jobs. A direct owner's
    // current diagnostics cannot retimestamp those snapshots.
    if (owner?.isWorker || owner?.isNativeGPU || !samples.every(Boolean)) return null;
    const tick = exactCounter(samples[0]?.sampleTick);
    return tick !== null && samples.every(sample => {
        const value = exactCounter(sample.sampleTick);
        return value !== null && compareExactCounters(value, tick) === 0;
    }) ? { source: 'wasm', sampleTick: tick } : null;
}

export function observeLatticeFields(sampled, owner, state, stride) {
    const fields = [sampled.eField, sampled.bField, sampled.fluxVector, sampled.poynting];
    const provenance = fieldObservationProvenance(fields, owner);
    if (!provenance) return null;
    const energy = computeEmEnergyFrame(sampled, state, { trackNormalization: false });
    const actualStride = sampled.fluxVector?.effectiveStride || stride;
    return Object.freeze({
        scenarioId: state.currentScenarioId, tick: provenance.sampleTick, source: provenance.source,
        energy: summarize(energy), flux: summarize(sampled.fluxVector, true), flow: summarize(sampled.poynting, true),
        sampling: Object.freeze({ stride: actualStride, count: energy?.count || 0, approximate: true }),
        status: 'Published field samples. Means describe returned points, not volume averages; coarse flux samples retain block peaks.',
    });
}
