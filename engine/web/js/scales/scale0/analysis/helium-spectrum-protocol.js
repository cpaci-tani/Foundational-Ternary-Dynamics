/**
 * Helium lattice-spectrum protocol helpers.
 *
 * Pure, DOM-free utilities for the draft helium spectrum preregistration.
 * These helpers deliberately do not know any laboratory helium lines; they only
 * compute simulation-native spectral fingerprints and time-series spectra.
 */

import { fft1d, nextPow2, spectralPeak, spectralSlope } from './lattice-spectrum.js';

export function latticeCenter(L) {
    const c = L / 2;
    return { x: c, y: c, z: c };
}

export function normalizeSpectrum(k, E, { dropNonPositiveK = true } = {}) {
    const kk = [];
    const energy = [];
    let total = 0;
    const n = Math.min(k?.length ?? 0, E?.length ?? 0);
    for (let i = 0; i < n; i++) {
        const ki = Number(k[i]);
        const ei = Number(E[i]);
        if (!Number.isFinite(ki) || !Number.isFinite(ei) || ei < 0) continue;
        if (dropNonPositiveK && ki <= 0) continue;
        kk.push(ki);
        energy.push(ei);
        total += ei;
    }
    const weights = total > 0 ? energy.map((v) => v / total) : energy.map(() => 0);
    return { k: kk, energy, weights, total, count: kk.length };
}

export function spectralMoments(k, E) {
    const norm = normalizeSpectrum(k, E);
    const { weights, energy } = norm;
    const n = norm.count;
    if (n === 0 || norm.total <= 0) {
        return {
            totalPower: 0,
            kPeak: 0,
            lambdaPeak: Infinity,
            centroidK: 0,
            bandwidthK: 0,
            spectralEntropy: 0,
            irFraction: 0,
            midFraction: 0,
            uvFraction: 0,
            slope: NaN,
            slopeBins: 0,
        };
    }

    let centroidK = 0;
    for (let i = 0; i < n; i++) centroidK += norm.k[i] * weights[i];

    let variance = 0;
    let entropy = 0;
    for (let i = 0; i < n; i++) {
        const d = norm.k[i] - centroidK;
        variance += d * d * weights[i];
        if (weights[i] > 0) entropy -= weights[i] * Math.log(weights[i]);
    }

    const kMax = Math.max(...norm.k);
    let irFraction = 0;
    let midFraction = 0;
    let uvFraction = 0;
    for (let i = 0; i < n; i++) {
        const r = kMax > 0 ? norm.k[i] / kMax : 0;
        if (r < 0.25) irFraction += weights[i];
        else if (r >= 0.75) uvFraction += weights[i];
        else midFraction += weights[i];
    }

    const peak = spectralPeak(norm.k, energy);
    const slope = spectralSlope(norm.k, energy);
    return {
        totalPower: norm.total,
        kPeak: peak.kPeak,
        lambdaPeak: peak.lambdaPeak,
        centroidK,
        bandwidthK: Math.sqrt(Math.max(0, variance)),
        spectralEntropy: n > 1 ? entropy / Math.log(n) : 0,
        irFraction,
        midFraction,
        uvFraction,
        slope: slope.slope,
        slopeBins: slope.n,
    };
}

export function spectrumFingerprint(spec, { parsevalRatio = NaN, chargeDipoleMagnitude = 0 } = {}) {
    const m = spectralMoments(spec?.k ?? [], spec?.E ?? []);
    return {
        labels: [
            'log1pTotalPower',
            'kPeak',
            'centroidK',
            'bandwidthK',
            'spectralEntropy',
            'irFraction',
            'midFraction',
            'uvFraction',
            'slope',
            'parsevalRatio',
            'chargeDipoleMagnitude',
        ],
        values: [
            Math.log1p(m.totalPower),
            m.kPeak,
            m.centroidK,
            m.bandwidthK,
            m.spectralEntropy,
            m.irFraction,
            m.midFraction,
            m.uvFraction,
            m.slope,
            parsevalRatio,
            chargeDipoleMagnitude,
        ],
        moments: m,
    };
}

function interpWeights(sourceK, sourceWeights, targetK) {
    if (!sourceK.length || !targetK.length) return targetK.map(() => 0);
    const out = [];
    let j = 0;
    for (const tk of targetK) {
        while (j < sourceK.length - 2 && sourceK[j + 1] < tk) j++;
        if (tk < sourceK[0] || tk > sourceK[sourceK.length - 1]) {
            out.push(0);
            continue;
        }
        const k0 = sourceK[j];
        const k1 = sourceK[Math.min(j + 1, sourceK.length - 1)];
        const w0 = sourceWeights[j] ?? 0;
        const w1 = sourceWeights[Math.min(j + 1, sourceWeights.length - 1)] ?? 0;
        const t = k1 !== k0 ? (tk - k0) / (k1 - k0) : 0;
        out.push(w0 * (1 - t) + w1 * t);
    }
    const sum = out.reduce((a, b) => a + b, 0);
    return sum > 0 ? out.map((v) => v / sum) : out;
}

export function spectrumDistance(a, b) {
    const na = normalizeSpectrum(a?.k ?? [], a?.E ?? []);
    const nb = normalizeSpectrum(b?.k ?? [], b?.E ?? []);
    const targetK = na.count >= nb.count ? na.k : nb.k;
    const pa = targetK === na.k ? na.weights : interpWeights(na.k, na.weights, targetK);
    const pb = targetK === nb.k ? nb.weights : interpWeights(nb.k, nb.weights, targetK);

    let l1 = 0;
    let dot = 0;
    let na2 = 0;
    let nb2 = 0;
    let js = 0;
    let hellingerSum = 0;
    for (let i = 0; i < targetK.length; i++) {
        const x = pa[i] || 0;
        const y = pb[i] || 0;
        const m = 0.5 * (x + y);
        l1 += Math.abs(x - y);
        dot += x * y;
        na2 += x * x;
        nb2 += y * y;
        if (x > 0 && m > 0) js += 0.5 * x * Math.log(x / m);
        if (y > 0 && m > 0) js += 0.5 * y * Math.log(y / m);
        const hd = Math.sqrt(Math.max(0, x)) - Math.sqrt(Math.max(0, y));
        hellingerSum += hd * hd;
    }
    return {
        bins: targetK.length,
        l1,
        jsDivergence: js,
        hellinger: Math.sqrt(0.5 * hellingerSum),
        cosine: na2 > 0 && nb2 > 0 ? dot / Math.sqrt(na2 * nb2) : 0,
    };
}

export function chargeDipoleFromParticles(particles, origin = { x: 0, y: 0, z: 0 }) {
    let x = 0;
    let y = 0;
    let z = 0;
    let netCharge = 0;
    let positive = 0;
    let negative = 0;
    const list = Array.isArray(particles) ? particles : [];
    for (const p of list) {
        const q = Number(p?.q ?? p?.charge ?? p?.state ?? 0);
        if (!Number.isFinite(q) || q === 0) continue;
        const px = Number(p.x ?? p.position?.x ?? 0);
        const py = Number(p.y ?? p.position?.y ?? 0);
        const pz = Number(p.z ?? p.position?.z ?? 0);
        x += q * (px - origin.x);
        y += q * (py - origin.y);
        z += q * (pz - origin.z);
        netCharge += q;
        if (q > 0) positive++;
        else negative++;
    }
    return {
        x,
        y,
        z,
        magnitude: Math.sqrt(x * x + y * y + z * z),
        netCharge,
        positive,
        negative,
        count: list.length,
    };
}

export function secondDifferenceVectorSeries(dipoles) {
    const out = [];
    for (let i = 2; i < dipoles.length; i++) {
        const x = dipoles[i].x - 2 * dipoles[i - 1].x + dipoles[i - 2].x;
        const y = dipoles[i].y - 2 * dipoles[i - 1].y + dipoles[i - 2].y;
        const z = dipoles[i].z - 2 * dipoles[i - 1].z + dipoles[i - 2].z;
        out.push({ x, y, z, magnitude: Math.sqrt(x * x + y * y + z * z) });
    }
    return out;
}

export function hannWindow(n) {
    if (n <= 1) return [1];
    const out = new Array(n);
    for (let i = 0; i < n; i++) out[i] = 0.5 * (1 - Math.cos((2 * Math.PI * i) / (n - 1)));
    return out;
}

export function timeSeriesPowerSpectrum(values, { dt = 1, removeMean = true, window = 'hann' } = {}) {
    const raw = Array.from(values ?? [], (v) => Number(v)).filter(Number.isFinite);
    const n = raw.length;
    if (n < 4) return { frequency: [], power: [], totalPower: 0, n, M: 0, dt };

    const mean = removeMean ? raw.reduce((a, b) => a + b, 0) / n : 0;
    const win = window === 'hann' ? hannWindow(n) : new Array(n).fill(1);
    const M = nextPow2(n);
    const re = new Float64Array(M);
    const im = new Float64Array(M);
    for (let i = 0; i < n; i++) re[i] = (raw[i] - mean) * win[i];
    fft1d(re, im);

    const frequency = [];
    const power = [];
    let totalPower = 0;
    const half = Math.floor(M / 2);
    for (let i = 1; i <= half; i++) {
        const p = (re[i] * re[i] + im[i] * im[i]) / Math.max(1, M);
        frequency.push(i / (M * dt));
        power.push(p);
        totalPower += p;
    }
    return { frequency, power, totalPower, n, M, dt };
}

export function dominantPeaks(frequency, power, { limit = 5, minBin = 2, minPowerFraction = 0.01 } = {}) {
    const maxPower = Math.max(0, ...power);
    const minPower = maxPower * minPowerFraction;
    const peaks = [];
    for (let i = Math.max(1, minBin); i < power.length - 1; i++) {
        const p = power[i];
        if (p < minPower) continue;
        if (p >= power[i - 1] && p >= power[i + 1]) {
            peaks.push({ bin: i, frequency: frequency[i], power: p, powerFraction: maxPower > 0 ? p / maxPower : 0 });
        }
    }
    peaks.sort((a, b) => b.power - a.power);
    return peaks.slice(0, limit);
}

export function peakBinAgreement(a, b, { toleranceBins = 1, limit = 5 } = {}) {
    const aa = a.slice(0, limit);
    const bb = b.slice(0, limit);
    let matches = 0;
    const used = new Set();
    for (const pa of aa) {
        for (let j = 0; j < bb.length; j++) {
            if (used.has(j)) continue;
            if (Math.abs(pa.bin - bb[j].bin) <= toleranceBins) {
                used.add(j);
                matches++;
                break;
            }
        }
    }
    return { matches, required: Math.min(3, aa.length, bb.length), toleranceBins };
}

export function finiteVector(values) {
    return values.every((v) => Number.isFinite(v));
}
