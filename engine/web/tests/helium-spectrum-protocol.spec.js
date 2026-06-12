// @ts-check
import { test, expect } from '@playwright/test';
import {
    chargeDipoleFromParticles,
    dominantPeaks,
    latticeCenter,
    peakBinAgreement,
    secondDifferenceVectorSeries,
    spectralMoments,
    spectrumDistance,
    spectrumFingerprint,
    timeSeriesPowerSpectrum,
} from '../js/scales/scale0/analysis/helium-spectrum-protocol.js';

test.describe('Helium lattice-spectrum protocol helpers', () => {
    test('spectral moments and fingerprint are finite for a synthetic spectrum', () => {
        const k = [0.1, 0.2, 0.3, 0.4, 0.5];
        const E = [1, 4, 2, 0.5, 0.25];
        const moments = spectralMoments(k, E);
        expect(moments.totalPower).toBeCloseTo(7.75, 10);
        expect(moments.kPeak).toBeCloseTo(0.2, 10);
        expect(moments.centroidK).toBeGreaterThan(0.1);
        expect(moments.centroidK).toBeLessThan(0.5);
        expect(moments.irFraction + moments.midFraction + moments.uvFraction).toBeCloseTo(1, 10);

        const fp = spectrumFingerprint({ k, E }, { parsevalRatio: 1, chargeDipoleMagnitude: 2 });
        expect(fp.labels).toHaveLength(fp.values.length);
        expect(fp.values.every(Number.isFinite)).toBe(true);
    });

    test('spectrum distance is zero for self and positive for shifted spectra', () => {
        const a = { k: [0.1, 0.2, 0.3, 0.4], E: [1, 4, 1, 0] };
        const b = { k: [0.1, 0.2, 0.3, 0.4], E: [0, 1, 4, 1] };
        const self = spectrumDistance(a, a);
        expect(self.l1).toBeCloseTo(0, 12);
        expect(self.jsDivergence).toBeCloseTo(0, 12);
        expect(self.cosine).toBeCloseTo(1, 12);

        const shifted = spectrumDistance(a, b);
        expect(shifted.l1).toBeGreaterThan(0.5);
        expect(shifted.jsDivergence).toBeGreaterThan(0);
        expect(shifted.cosine).toBeLessThan(1);
    });

    test('charged dipole uses particle charges relative to the lattice center', () => {
        const origin = latticeCenter(64);
        const particles = [
            { x: origin.x + 1, y: origin.y, z: origin.z, q: +1 },
            { x: origin.x - 1, y: origin.y, z: origin.z, q: -1 },
            { x: origin.x, y: origin.y + 2, z: origin.z, state: +1 },
            { x: origin.x, y: origin.y - 2, z: origin.z, charge: -1 },
        ];
        const d = chargeDipoleFromParticles(particles, origin);
        expect(d.netCharge).toBe(0);
        expect(d.x).toBeCloseTo(2, 12);
        expect(d.y).toBeCloseTo(4, 12);
        expect(d.z).toBeCloseTo(0, 12);
        expect(d.magnitude).toBeCloseTo(Math.sqrt(20), 12);
    });

    test('time-series power spectrum recovers a synthetic tick-frequency peak', () => {
        const n = 128;
        const f = 1 / 16;
        const values = Array.from({ length: n }, (_, t) => Math.sin(2 * Math.PI * f * t));
        const spec = timeSeriesPowerSpectrum(values, { dt: 1 });
        const peaks = dominantPeaks(spec.frequency, spec.power, { limit: 3, minBin: 2 });
        expect(peaks.length).toBeGreaterThan(0);
        expect(peaks[0].frequency).toBeCloseTo(f, 6);
    });

    test('second-difference dipole peaks agree across duplicate synthetic runs', () => {
        const n = 128;
        const f = 1 / 16;
        const makeDipoles = (phase = 0) => Array.from({ length: n }, (_, t) => ({
            x: Math.sin(2 * Math.PI * f * t + phase),
            y: 0,
            z: 0,
        }));
        const a = secondDifferenceVectorSeries(makeDipoles(0)).map((v) => v.magnitude);
        const b = secondDifferenceVectorSeries(makeDipoles(0)).map((v) => v.magnitude);
        const sa = timeSeriesPowerSpectrum(a, { dt: 1 });
        const sb = timeSeriesPowerSpectrum(b, { dt: 1 });
        const pa = dominantPeaks(sa.frequency, sa.power, { limit: 5, minBin: 2 });
        const pb = dominantPeaks(sb.frequency, sb.power, { limit: 5, minBin: 2 });
        const agreement = peakBinAgreement(pa, pb, { toleranceBins: 1, limit: 5 });
        expect(agreement.matches).toBeGreaterThanOrEqual(agreement.required);
    });
});
