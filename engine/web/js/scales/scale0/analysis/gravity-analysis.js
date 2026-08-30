/**
 * Gravity analysis — pure, DOM-free scalar telemetry for the Gravity Observatory.
 * Importable from the panel and node (no Three.js, no globals). See
 * .claude/plans/let-s-plan-for-and-eager-tide.md §1.1.
 *
 * Honesty: these are the WEB **proxy** gravity scalars (latency L = |J|²-proxy,
 * curvature proxy K_p = (∇²L)², plus the separately sampled finite-engine
 * gravity-force field). The slice-only force view uses G_N·|∇₁|J||. K_p is not
 * the full Riemann-tensor Kretschmann invariant. The engine's Poisson-derived
 * [IMPOSED] latency map is surfaced separately (Phase 2), tagged [ENGINE]; it
 * is not a recovered physical metric. Everything here is [proxy].
 */

import { metricStats, histogram, magnitudeGrid } from './lattice-topology.js';
import {
    G_N,
    ALPHA_G_APPROX,
    LATENCY_HORIZON_CLAMP,
    LAPLACIAN_FACE_WEIGHT,
    LAPLACIAN_EDGE_WEIGHT,
} from '../../../constants.js';

/**
 * Proxy lapse slowdown from normalized L_p: f_p = 1−L_p² and
 * (1−√f_p)·100. This is a derived web-proxy readout, not a native clock
 * measurement or an event-horizon detector.
 */
export function dilationPct(Lmax) {
    const L = Math.min(Math.max(Lmax || 0, 0), 0.999);
    const f = 1 - L * L;
    return f > 0 ? (1 - Math.sqrt(f)) * 100 : 100;
}

/** Normalized field-contrast proxy: peak L_p above mean L_p (≥0). */
export function strainProxy(Lmax, Lmean) {
    return Math.max(0, (Lmax || 0) - (Lmean || 0));
}

/** Magnitudes of an interleaved force-vector buffer (reuse of lattice-topology). */
export function forceMagnitudes(vectors, count) {
    return magnitudeGrid(vectors, count);
}

/** max(|J|²) over a dense magnitude volume — the latency normalizer. */
export function maxRhoOf(mag, M) {
    let mx = 1e-30;
    for (let i = 0; i < M; i++) { const r = mag[i] * mag[i]; if (r > mx) mx = r; }
    return mx;
}

const MAX_DENSE_VISUAL_SAMPLES = 262144;

/**
 * Mirror ftd::visual_sample_grid (visual_sample_grid.h) for proxy telemetry
 * derived from an already-read browser volume. Keeping the same centered grid
 * means the direct-WASM fallback samples the same lattice sites as the native,
 * GPU, and Worker samplers instead of introducing a second web-only geometry.
 */
export function gravityVisualSampleGrid(latticeSize, requestedStride, interior = false) {
    const N = Math.max(0, Math.trunc(Number(latticeSize) || 0));
    let stride = Math.max(1, Math.trunc(Number(requestedStride) || 1));
    const extent = Math.max(0, N - (interior ? 2 : 0));
    const sampleCount = (step) => {
        const perAxis = Math.ceil(extent / step);
        return perAxis * perAxis * perAxis;
    };
    while (sampleCount(stride) > MAX_DENSE_VISUAL_SAMPLES) stride += 1;
    const lo = interior ? 1 : 0;
    const hi = interior ? N - 2 : N - 1;
    if (hi < lo) return { stride, origin: 0, count: 0, end: 0 };
    const center = Math.trunc((N - 1) / 2);
    const origin = center - Math.trunc((center - lo) / stride) * stride;
    const count = Math.trunc((hi - origin) / stride) + 1;
    return { stride, origin, count, end: origin + count * stride };
}

/**
 * Build the L_p and K_p proxy samples from one dense |J| snapshot.
 *
 * This is deliberately limited to the two quantities already identified in
 * the UI as presentation proxies. The selected gravity support field and
 * Poisson-latency aggregate still come from their exact engine samplers. The
 * thresholds, clamp, 18-point stencil, and center-anchored sampling grid match
 * get_latency_sampled/get_kretschmann_sampled in ftd_wasm.cpp.
 */
export function gravityProxySamplesFromVolume(mag, latticeSize, requestedStride) {
    const N = Math.max(0, Math.trunc(Number(latticeSize) || 0));
    const M = N * N * N;
    const empty = {
        latencyVals: new Float64Array(0),
        latencyCount: 0,
        kretVals: new Float64Array(0),
        kretCount: 0,
        maxRho: 0,
    };
    if (!ArrayBuffer.isView(mag) || N < 1 || mag.length < M) return empty;

    let maxRho = 0;
    for (let i = 0; i < M; i += 1) {
        const value = Number(mag[i]) || 0;
        const rho = value * value;
        if (rho > maxRho) maxRho = rho;
    }
    if (maxRho < 1e-30) return { ...empty, maxRho };

    const invRho = 1 / maxRho;
    const idx = (x, y, z) => (z * N + y) * N + x;
    const latencyAt = (x, y, z) => {
        const value = Number(mag[idx(x, y, z)]) || 0;
        return Math.sqrt(Math.min(value * value * invRho, LATENCY_HORIZON_CLAMP));
    };

    const latencyGrid = gravityVisualSampleGrid(N, requestedStride, false);
    const latencyVals = new Float64Array(latencyGrid.count ** 3);
    let latencyCount = 0;
    for (let z = latencyGrid.origin; z < latencyGrid.end; z += latencyGrid.stride) {
        for (let y = latencyGrid.origin; y < latencyGrid.end; y += latencyGrid.stride) {
            for (let x = latencyGrid.origin; x < latencyGrid.end; x += latencyGrid.stride) {
                const value = latencyAt(x, y, z);
                if (value < 1e-6) continue;
                latencyVals[latencyCount++] = value;
            }
        }
    }

    const curvatureGrid = gravityVisualSampleGrid(N, requestedStride, true);
    const kretVals = new Float64Array(curvatureGrid.count ** 3);
    let kretCount = 0;
    const F3 = LAPLACIAN_FACE_WEIGHT;
    const E6 = LAPLACIAN_EDGE_WEIGHT;
    for (let z = curvatureGrid.origin; z < curvatureGrid.end; z += curvatureGrid.stride) {
        for (let y = curvatureGrid.origin; y < curvatureGrid.end; y += curvatureGrid.stride) {
            for (let x = curvatureGrid.origin; x < curvatureGrid.end; x += curvatureGrid.stride) {
                const self = latencyAt(x, y, z);
                const faceSum = latencyAt(x + 1, y, z) + latencyAt(x - 1, y, z)
                    + latencyAt(x, y + 1, z) + latencyAt(x, y - 1, z)
                    + latencyAt(x, y, z + 1) + latencyAt(x, y, z - 1);
                const edgeSum = latencyAt(x + 1, y + 1, z) + latencyAt(x + 1, y - 1, z)
                    + latencyAt(x - 1, y + 1, z) + latencyAt(x - 1, y - 1, z)
                    + latencyAt(x + 1, y, z + 1) + latencyAt(x + 1, y, z - 1)
                    + latencyAt(x - 1, y, z + 1) + latencyAt(x - 1, y, z - 1)
                    + latencyAt(x, y + 1, z + 1) + latencyAt(x, y + 1, z - 1)
                    + latencyAt(x, y - 1, z + 1) + latencyAt(x, y - 1, z - 1);
                const laplacian = F3 * faceSum + E6 * edgeSum - 4 * self;
                const value = laplacian * laplacian;
                if (value < 1e-18) continue;
                kretVals[kretCount++] = value;
            }
        }
    }

    return { latencyVals, latencyCount, kretVals, kretCount, maxRho };
}

/**
 * Dense per-axis gravity slice computed from the |J| MAGNITUDE volume (what BOTH
 * MockBridge.getFluxVolume and WasmBridge.getFluxVolume return — bridge-agnostic,
 * unlike a Mock-only sampler). Returns Float64Array(N²) in getFluxSlice's
 * `data[a*N+b]` layout (feed through transposeAndFlipNN to paint).
 *
 * Latency proxy L = √min(|J|²/maxRho, LATENCY_HORIZON_CLAMP) — identical
 * to buildLatencyProxy.
 * kind: latency=L, dilation=L² (lapse deficit), kretschmann=(∇²L)² (18-pt Moore),
 * force=G_N·|∇|J|| (central diff). Stencil kinds zero the one-voxel border.
 * @param {ArrayLike<number>} mag  dense |J| volume, layout idx=(z*N+y)*N+x
 * @param {number} spacing physical lattice spacing between adjacent samples
 */
export function gravitySlice(mag, N, axis, index, kind = 'latency', maxRho = 0, spacing = 1) {
    const out = new Float64Array(N * N);
    const M = N * N * N;
    if (!mag || mag.length < M) return out;
    const rho = maxRho > 0 ? maxRho : maxRhoOf(mag, M);
    const invRho = 1 / rho;
    const h = Math.max(1, Number(spacing) || 1);
    const invH = 1 / h;
    const invH2 = invH * invH;
    const vidx = (x, y, z) => (z * N + y) * N + x;
    const Lof = (x, y, z) => {
        const m = mag[vidx(x, y, z)];
        return Math.sqrt(Math.min(m * m * invRho, LATENCY_HORIZON_CLAMP));
    };
    const edge = (x, y, z) => (x <= 0 || x >= N - 1 || y <= 0 || y >= N - 1 || z <= 0 || z >= N - 1);
    const F3 = LAPLACIAN_FACE_WEIGHT, E6 = LAPLACIAN_EDGE_WEIGHT;
    for (let a = 0; a < N; a++) {
        for (let b = 0; b < N; b++) {
            const x = axis === 0 ? index : a;
            const y = axis === 0 ? a : (axis === 1 ? index : b);
            const z = axis === 2 ? index : b;
            let v = 0;
            if (kind === 'latency') {
                v = Lof(x, y, z);
            } else if (kind === 'dilation') {
                const L = Lof(x, y, z); v = L * L;
            } else if (kind === 'kretschmann') {
                if (!edge(x, y, z)) {
                    const self = Lof(x, y, z);
                    const faceSum = Lof(x + 1, y, z) + Lof(x - 1, y, z) + Lof(x, y + 1, z)
                        + Lof(x, y - 1, z) + Lof(x, y, z + 1) + Lof(x, y, z - 1);
                    const edgeSum = Lof(x + 1, y + 1, z) + Lof(x + 1, y - 1, z) + Lof(x - 1, y + 1, z) + Lof(x - 1, y - 1, z)
                        + Lof(x + 1, y, z + 1) + Lof(x + 1, y, z - 1) + Lof(x - 1, y, z + 1) + Lof(x - 1, y, z - 1)
                        + Lof(x, y + 1, z + 1) + Lof(x, y + 1, z - 1) + Lof(x, y - 1, z + 1) + Lof(x, y - 1, z - 1);
                    const lap = (F3 * faceSum + E6 * edgeSum - 4 * self) * invH2;
                    v = lap * lap;
                }
            } else { // force: G_N·|∇|J||
                if (!edge(x, y, z)) {
                    const gx = (mag[vidx(x + 1, y, z)] - mag[vidx(x - 1, y, z)]) * 0.5 * invH;
                    const gy = (mag[vidx(x, y + 1, z)] - mag[vidx(x, y - 1, z)]) * 0.5 * invH;
                    const gz = (mag[vidx(x, y, z + 1)] - mag[vidx(x, y, z - 1)]) * 0.5 * invH;
                    v = G_N * Math.sqrt(gx * gx + gy * gy + gz * gz);
                }
            }
            out[a * N + b] = v;
        }
    }
    return out;
}

/**
 * Roll up the proxy gravity scalars + histograms for the panel.
 * @param {{latencyVals:ArrayLike, latencyCount:number, kretVals:ArrayLike,
 *   kretCount:number, forceMags:ArrayLike, forceCount:number}} src
 */
export function aggregateMetrics(src) {
    const L = metricStats(src.latencyVals || [], src.latencyCount || 0);
    const K = metricStats(src.kretVals || [], src.kretCount || 0);
    const F = metricStats(src.forceMags || [], src.forceCount || 0);
    return {
        L: { mean: L.mean, max: L.max, rms: L.rms },
        K: { mean: K.mean, max: K.max },
        F: { mean: F.mean, max: F.max },
        dilationPct: dilationPct(L.max),
        horizon: L.max,                       // proximity to the imposed proxy clamp
        strain: strainProxy(L.max, L.mean),
        gnG: G_N,
        alphaG: ALPHA_G_APPROX,
        histL: histogram(src.latencyVals || [], src.latencyCount || 0, 22),
        histK: histogram(src.kretVals || [], src.kretCount || 0, 22),
        histF: histogram(src.forceMags || [], src.forceCount || 0, 22),
    };
}
