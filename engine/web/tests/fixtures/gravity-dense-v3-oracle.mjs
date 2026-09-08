// Frozen dense oracle: exact function bodies from the production baseline.
// Source SHA256 9a38058819916d2e964f77be28c18486072c4f511ce61e980f031177a2d7eee5
// Intentional test-only provenance; not a second runtime implementation.
import { G_N, LATENCY_HORIZON_CLAMP, LAPLACIAN_FACE_WEIGHT, LAPLACIAN_EDGE_WEIGHT } from "../../js/constants.js";

export function maxRhoOf(mag, M) {
    let mx = 1e-30;
    for (let i = 0; i < M; i++) { const r = mag[i] * mag[i]; if (r > mx) mx = r; }
    return mx;
}

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
