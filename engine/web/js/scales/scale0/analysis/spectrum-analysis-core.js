/** Observation-only reductions. Executed by spectrum-analysis-worker, never by the panel. */
import { energySpectrum, spectralPeak, spectralSlope, denseVectorGridFromSamples } from './lattice-spectrum.js';
import { defectCount, fluxTubeComponents, metricStats, histogram, chiralityFromAudit } from './lattice-topology.js';

function validSample(sample, vector = false) {
    if (!sample) return null;
    const { count, positions } = sample;
    const values = vector ? sample.vectors : sample.values;
    const width = vector ? 3 : 1;
    if (!Number.isSafeInteger(count) || count < 0 || !values || values.length < count * width
        || !positions || positions.length < count * 3) return null;
    for (let i = 0; i < count * width; i++) if (!Number.isFinite(values[i])) return null;
    for (let i = 0; i < count * 3; i++) if (!Number.isFinite(positions[i])) return null;
    return sample;
}

export function analyzeSpectrumObservation(input) {
    const { L, stride, M, mode, audit = null } = input;
    if (!Number.isInteger(L) || L < 1 || !Number.isInteger(stride) || stride < 1
        || !['live', 'deep', 'topology'].includes(mode)
        || (mode !== 'topology' && ![8, 32, 64].includes(M))) {
        throw new RangeError('Invalid Spectrum observation grid');
    }
    const flux = validSample(input.flux, true);
    let spatial = null, spectrum = null;
    if (flux) {
        const grid = denseVectorGridFromSamples(flux, L, stride);
        const mag = new Float64Array(grid.srcN ** 3);
        for (let i = 0; i < mag.length; i++) mag[i] = Math.hypot(grid.jx[i], grid.jy[i], grid.jz[i]);
        spatial = { mag, srcN: grid.srcN, stride, effectiveStride: grid.effectiveStride,
            origin: grid.origin, uniformPeriodicSpacing: grid.uniformPeriodicSpacing, sampleCount: flux.count };
        if (mode !== 'topology') {
            const spec = energySpectrum(grid, grid.srcN, M, L);
            const peak = spectralPeak(spec.k, spec.E);
            const slope = spectralSlope(spec.k, spec.E);
            spectrum = { ...spatial, spec, peak, slope,
                parseval: spec.sumReal > 0 ? spec.totalE / spec.sumReal : 1, M,
                provenance: input.provenance.flux };
        }
    }
    const divergence = validSample(input.divergence);
    const topology = {
        defects: divergence ? defectCount(divergence.values, divergence.count, 0.5) : null,
        tubes: spatial ? fluxTubeComponents(spatial.mag, spatial.srcN, 0.35) : null,
        chir: chiralityFromAudit(audit),
        gauss: Number.isFinite(audit?.gaussViolation) ? audit.gaussViolation : null,
        gaussMax: Number.isFinite(audit?.maxGaussError) ? audit.maxGaussError : null,
    };
    const metrics = (input.metrics || []).map(({ kind, sample }) => {
        const s = validSample(sample);
        return s?.count > 0
            ? { kind, stats: metricStats(s.values, s.count), hist: histogram(s.values, s.count, 22) }
            : { kind, stats: null, hist: null };
    });
    return { spectrum, topology, metrics, provenance: input.provenance };
}
