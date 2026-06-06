/**
 * Lattice topology + metric-distribution analysis. Pure, DOM-free, importable
 * from the panel and the worker. See SPEC_SCALE0_LATTICE_SPECTROSCOPY.md §4–§5.
 */

/** Histogram of `count` values → fixed bins with auto-range. */
export function histogram(values, count, nBins = 24) {
    let min = Infinity, max = -Infinity;
    for (let i = 0; i < count; i++) { const v = values[i]; if (v < min) min = v; if (v > max) max = v; }
    const counts = new Array(nBins).fill(0);
    if (count === 0 || !Number.isFinite(min) || !Number.isFinite(max)) return { counts, min: 0, max: 0, nBins };
    if (max === min) max = min + 1e-9;
    const span = max - min;
    for (let i = 0; i < count; i++) {
        let b = Math.floor((values[i] - min) / span * nBins);
        if (b >= nBins) b = nBins - 1; else if (b < 0) b = 0;
        counts[b]++;
    }
    return { counts, min, max, nBins };
}

/** Mean / rms / min / max / max-|·| over `count` values. */
export function metricStats(values, count) {
    if (!count) return { mean: 0, rms: 0, min: 0, max: 0, absMax: 0 };
    let sum = 0, sumSq = 0, min = Infinity, max = -Infinity, absMax = 0;
    for (let i = 0; i < count; i++) {
        const v = values[i];
        sum += v; sumSq += v * v;
        if (v < min) min = v; if (v > max) max = v;
        const a = v < 0 ? -v : v; if (a > absMax) absMax = a;
    }
    return { mean: sum / count, rms: Math.sqrt(sumSq / count), min, max, absMax };
}

/**
 * Defect / monopole proxy from the divergence field: voxels with div above
 * (+) / below (−) a fraction of the peak |div| are sources / sinks. Net = the
 * signed imbalance. Honest: a proxy, not a quantized topological charge.
 */
export function defectCount(divValues, count, relThreshold = 0.5) {
    let absMax = 0;
    for (let i = 0; i < count; i++) { const a = Math.abs(divValues[i]); if (a > absMax) absMax = a; }
    const tau = relThreshold * absMax;
    let sources = 0, sinks = 0;
    if (tau > 0) for (let i = 0; i < count; i++) { const v = divValues[i]; if (v > tau) sources++; else if (v < -tau) sinks++; }
    return { sources, sinks, net: sources - sinks, threshold: tau };
}

/**
 * Flux-tube / coherent-structure count: 6-neighbour (periodic) connected
 * components of { |J| > relThreshold·max|J| } on a dense magnitude grid (srcN³,
 * idx = (z·N + y)·N + x), via union-find. Returns component count, largest size,
 * and the sorted size list — the confinement strings / coherent flux bundles.
 */
export function fluxTubeComponents(mag, srcN, relThreshold = 0.35) {
    const N = srcN, Nc = N * N * N;
    let maxv = 0;
    for (let i = 0; i < Nc; i++) if (mag[i] > maxv) maxv = mag[i];
    const tau = relThreshold * maxv;
    if (tau <= 0) return { count: 0, largest: 0, sizes: [], threshold: 0 };

    const parent = new Int32Array(Nc);
    const active = new Uint8Array(Nc);
    for (let i = 0; i < Nc; i++) { if (mag[i] > tau) { active[i] = 1; parent[i] = i; } else parent[i] = -1; }

    const find = (a) => {
        let r = a; while (parent[r] !== r) r = parent[r];
        while (parent[a] !== r) { const n = parent[a]; parent[a] = r; a = n; }
        return r;
    };
    const union = (a, b) => { const ra = find(a), rb = find(b); if (ra !== rb) parent[ra] = rb; };

    for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
        const i = (z * N + y) * N + x;
        if (!active[i]) continue;
        const xp = (z * N + y) * N + ((x + 1) % N);
        const yp = (z * N + ((y + 1) % N)) * N + x;
        const zp = (((z + 1) % N) * N + y) * N + x;
        if (active[xp]) union(i, xp);
        if (active[yp]) union(i, yp);
        if (active[zp]) union(i, zp);
    }

    const sizeMap = new Map();
    for (let i = 0; i < Nc; i++) if (active[i]) { const r = find(i); sizeMap.set(r, (sizeMap.get(r) || 0) + 1); }
    const sizes = Array.from(sizeMap.values()).sort((a, b) => b - a);
    return { count: sizes.length, largest: sizes[0] || 0, sizes, threshold: tau };
}

/** Chirality / L–R handedness from the energy audit's left/right channels. */
export function chiralityFromAudit(audit) {
    if (!audit) return { total: 0, eAsym: 0, wvAsym: 0 };
    const eL = audit.ELTotal ?? audit.eLTotal ?? 0, eR = audit.ERTotal ?? audit.eRTotal ?? 0;
    const wL = audit.wvLTotal ?? 0, wR = audit.wvRTotal ?? 0;
    const sE = eL + eR, sW = wL + wR;
    return {
        total: audit.chiralityTotal ?? 0,
        eAsym: sE !== 0 ? (eL - eR) / sE : 0,
        wvAsym: sW !== 0 ? (wL - wR) / sW : 0,
    };
}

/** Build a dense |J| magnitude grid from interleaved flux-vector samples. */
export function magnitudeGrid(vectors, count) {
    const mag = new Float64Array(count);
    for (let i = 0; i < count; i++) {
        const x = vectors[i * 3], y = vectors[i * 3 + 1], z = vectors[i * 3 + 2];
        mag[i] = Math.sqrt(x * x + y * y + z * z);
    }
    return mag;
}
