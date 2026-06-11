/**
 * Gravity analysis — pure, DOM-free scalar telemetry for the Gravity Observatory.
 * Importable from the panel and node (no Three.js, no globals). See
 * .claude/plans/let-s-plan-for-and-eager-tide.md §1.1.
 *
 * Honesty: these are the WEB **proxy** gravity scalars (latency L = |J|²-proxy,
 * Kretschmann K = (∇²L)², force |F| = G_N·∇ρ). The genuine C++ Poisson metric is
 * surfaced separately (Phase 2), tagged [C++]. Everything here is [proxy].
 */

import { metricStats, histogram, magnitudeGrid } from './lattice-topology.js';
import { G_N, K_B, ALPHA_G_APPROX, LAPLACIAN_FACE_WEIGHT, LAPLACIAN_EDGE_WEIGHT } from '../../../constants.js';

/**
 * Pairwise gravitational potential energy over manifested particles:
 *   U = −Σ_{i<j} G_N·K_B² / r_ij        (always negative — gravity is attractive)
 * Mirrors the Coulomb-PE loop in mock-diagnostics.js but unsigned (no charge),
 * matching the engine's pairwise force F = G_N·K_B²/r² (mock-bridge.js:619).
 */
export function gravityPE(particles) {
    if (!particles || !particles.length) return 0;
    const K2 = K_B * K_B;
    let pe = 0;
    for (let i = 0; i < particles.length; i++) {
        const pi = particles[i];
        if ((pi.state ?? 0) === 0) continue;
        const xi = pi.x ?? 0, yi = pi.y ?? 0, zi = pi.z ?? 0;
        for (let j = i + 1; j < particles.length; j++) {
            const pj = particles[j];
            if ((pj.state ?? 0) === 0) continue;
            const dx = xi - (pj.x ?? 0), dy = yi - (pj.y ?? 0), dz = zi - (pj.z ?? 0);
            const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (r < 1e-6) continue;
            pe -= G_N * K2 / r;
        }
    }
    return pe;
}

/**
 * Same pairwise gravity PE, but from an interleaved positions buffer (the web
 * particle frame is render-data: { positions:Float32Array(count*3), count } with
 * no charge field — fine, since gravity is sign-independent).
 */
export function gravityPEFromPositions(positions, count) {
    if (!positions || !count) return 0;
    const K2 = K_B * K_B;
    let pe = 0;
    
    // ARC-PERF (2026-06-10): Pairwise O(N^2) on 35,000 particles is 612M iterations,
    // which freezes the browser. We sub-sample heavily for dense fields and 
    // scale the result proportionally. Gravity is long-range, so this proxy is robust.
    let stride = 1;
    if (count > 500) stride = Math.ceil(count / 500);
    
    let sampledCount = 0;
    for (let i = 0; i < count; i += stride) {
        sampledCount++;
        const xi = positions[i * 3], yi = positions[i * 3 + 1], zi = positions[i * 3 + 2];
        for (let j = i + stride; j < count; j += stride) {
            const dx = xi - positions[j * 3], dy = yi - positions[j * 3 + 1], dz = zi - positions[j * 3 + 2];
            const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (r < 1e-6) continue;
            pe -= G_N * K2 / r;
        }
    }
    
    if (stride > 1 && sampledCount > 1) {
        const actualPairs = (count * (count - 1)) / 2;
        const sampledPairs = (sampledCount * (sampledCount - 1)) / 2;
        pe *= (actualPairs / sampledPairs);
    }
    
    return pe;
}

/**
 * Time-dilation percentage from the peak latency: with metric lapse f = 1−L²,
 * a clock runs at rate √f, so the slowdown is (1−√f)·100. L is clamped below the
 * 0.999 horizon clamp. dilationPct(0)=0; dilationPct(0.998)≈93.7%.
 */
export function dilationPct(Lmax) {
    const L = Math.min(Math.max(Lmax || 0, 0), 0.999);
    const f = 1 - L * L;
    return f > 0 ? (1 - Math.sqrt(f)) * 100 : 100;
}

/** GW strain proxy: how far the peak latency rises above the mean (≥0). */
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

/**
 * Dense per-axis gravity slice computed from the |J| MAGNITUDE volume (what BOTH
 * MockBridge.getFluxVolume and WasmBridge.getFluxVolume return — bridge-agnostic,
 * unlike a Mock-only sampler). Returns Float64Array(N²) in getFluxSlice's
 * `data[a*N+b]` layout (feed through transposeAndFlipNN to paint).
 *
 * Latency proxy L = √min(|J|²/maxRho, 0.998) — identical to buildLatencyProxy.
 * kind: latency=L, dilation=L² (lapse deficit), kretschmann=(∇²L)² (18-pt Moore),
 * force=G_N·|∇|J|| (central diff). Stencil kinds zero the one-voxel border.
 * @param {ArrayLike<number>} mag  dense |J| volume, layout idx=(z*N+y)*N+x
 */
export function gravitySlice(mag, N, axis, index, kind = 'latency', maxRho = 0) {
    const out = new Float64Array(N * N);
    const M = N * N * N;
    if (!mag || mag.length < M) return out;
    const rho = maxRho > 0 ? maxRho : maxRhoOf(mag, M);
    const invRho = 1 / rho;
    const vidx = (x, y, z) => (z * N + y) * N + x;
    const Lof = (x, y, z) => { const m = mag[vidx(x, y, z)]; return Math.sqrt(Math.min(m * m * invRho, 0.998)); };
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
                    const lap = F3 * faceSum + E6 * edgeSum - 4 * self;
                    v = lap * lap;
                }
            } else { // force: G_N·|∇|J||
                if (!edge(x, y, z)) {
                    const gx = (mag[vidx(x + 1, y, z)] - mag[vidx(x - 1, y, z)]) * 0.5;
                    const gy = (mag[vidx(x, y + 1, z)] - mag[vidx(x, y - 1, z)]) * 0.5;
                    const gz = (mag[vidx(x, y, z + 1)] - mag[vidx(x, y, z - 1)]) * 0.5;
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
 *   kretCount:number, forceMags:ArrayLike, forceCount:number, particles:Array}} src
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
        horizon: L.max,                       // L_max → 0.999 = event horizon
        strain: strainProxy(L.max, L.mean),
        gravPE: src.particlePositions
            ? gravityPEFromPositions(src.particlePositions, src.particleCount || 0)
            : gravityPE(src.particles),
        gnG: G_N,
        alphaG: ALPHA_G_APPROX,
        histL: histogram(src.latencyVals || [], src.latencyCount || 0, 22),
        histK: histogram(src.kretVals || [], src.kretCount || 0, 22),
        histF: histogram(src.forceMags || [], src.forceCount || 0, 22),
    };
}
