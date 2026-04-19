/**
 * LOD helpers for the playback timeline.
 *
 * A snapshot captures a lattice at some level-of-detail `k`:
 *   k = 0 → full N³
 *   k = 1 → (N/2)³ block-averaged
 *   k = 2 → (N/4)³ block-averaged
 *   k = 3 → no lattice (audit-only)
 *
 * Block-average for a scalar: mean of the 2^k × 2^k × 2^k sub-cube.
 * Block-average for a 3-vector: mean per component.
 */

export const LOD_FACTORS = [1, 2, 4];   // spatial strides for k=0..2 (k=3 has no lattice)

/**
 * Downsample a scalar cube (e.g. state field, Int8Array).
 * @param {Int8Array|Float32Array} src  — length N^3, indexed as x + y*N + z*N*N
 * @param {number} N                     — source edge length
 * @param {number} k                     — target LOD (1 or 2)
 * @returns {Int8Array}                  — length (N / 2^k)^3, values nearest-integer of the mean
 */
export function blockAverageScalar(src, N, k) {
    const factor = LOD_FACTORS[k];
    if (!factor || factor === 1) throw new Error('blockAverageScalar expects k in {1,2}');
    const M = Math.floor(N / factor);
    const out = new Int8Array(M * M * M);
    const vol = factor * factor * factor;
    for (let z = 0; z < M; z++) {
        for (let y = 0; y < M; y++) {
            for (let x = 0; x < M; x++) {
                let sum = 0;
                for (let dz = 0; dz < factor; dz++) {
                    const zs = z * factor + dz;
                    for (let dy = 0; dy < factor; dy++) {
                        const ys = y * factor + dy;
                        for (let dx = 0; dx < factor; dx++) {
                            const xs = x * factor + dx;
                            sum += src[xs + ys * N + zs * N * N];
                        }
                    }
                }
                out[x + y * M + z * M * M] = Math.round(sum / vol) | 0;
            }
        }
    }
    return out;
}

/**
 * Downsample a 3-component vector field (Float32Array of length 3*N^3).
 */
export function blockAverageVec3(src, N, k) {
    const factor = LOD_FACTORS[k];
    if (!factor || factor === 1) throw new Error('blockAverageVec3 expects k in {1,2}');
    const M = Math.floor(N / factor);
    const out = new Float32Array(3 * M * M * M);
    const inv = 1 / (factor * factor * factor);
    for (let z = 0; z < M; z++) {
        for (let y = 0; y < M; y++) {
            for (let x = 0; x < M; x++) {
                let sx = 0, sy = 0, sz = 0;
                for (let dz = 0; dz < factor; dz++) {
                    const zs = z * factor + dz;
                    for (let dy = 0; dy < factor; dy++) {
                        const ys = y * factor + dy;
                        for (let dx = 0; dx < factor; dx++) {
                            const xs = x * factor + dx;
                            const i = 3 * (xs + ys * N + zs * N * N);
                            sx += src[i];
                            sy += src[i + 1];
                            sz += src[i + 2];
                        }
                    }
                }
                const oi = 3 * (x + y * M + z * M * M);
                out[oi]     = sx * inv;
                out[oi + 1] = sy * inv;
                out[oi + 2] = sz * inv;
            }
        }
    }
    return out;
}

/**
 * Nearest-neighbor upsample of a scalar cube from M³ back to N³
 * where N = M * 2^k. Used when a lower-LOD snapshot needs to be
 * written back into the engine (engine always wants full N³).
 */
export function upsampleScalar(src, N, k) {
    const factor = LOD_FACTORS[k];
    if (!factor || factor === 1) return src;
    const M = Math.floor(N / factor);
    const Ctor = src.constructor;      // preserve Int8/Float32 etc.
    const out = new Ctor(N * N * N);
    for (let z = 0; z < N; z++) {
        const zs = Math.min(M - 1, Math.floor(z / factor));
        for (let y = 0; y < N; y++) {
            const ys = Math.min(M - 1, Math.floor(y / factor));
            for (let x = 0; x < N; x++) {
                const xs = Math.min(M - 1, Math.floor(x / factor));
                out[x + y * N + z * N * N] = src[xs + ys * M + zs * M * M];
            }
        }
    }
    return out;
}

/**
 * Nearest-neighbor upsample of a 3-vector cube from M³ back to N³.
 */
export function upsampleVec3(src, N, k) {
    const factor = LOD_FACTORS[k];
    if (!factor || factor === 1) return src;
    const M = Math.floor(N / factor);
    const out = new Float32Array(3 * N * N * N);
    for (let z = 0; z < N; z++) {
        const zs = Math.min(M - 1, Math.floor(z / factor));
        for (let y = 0; y < N; y++) {
            const ys = Math.min(M - 1, Math.floor(y / factor));
            for (let x = 0; x < N; x++) {
                const xs = Math.min(M - 1, Math.floor(x / factor));
                const si = 3 * (xs + ys * M + zs * M * M);
                const oi = 3 * (x + y * N + z * N * N);
                out[oi]     = src[si];
                out[oi + 1] = src[si + 1];
                out[oi + 2] = src[si + 2];
            }
        }
    }
    return out;
}

/**
 * Promote any snapshot to a LOD-0 shape by upsampling its arrays.
 * Returns a new snapshot object; does not mutate the input.
 * LOD 3 (telemetry-only) returns null — nothing to display.
 */
export function upsampleSnapshotToLod0(snap, N) {
    if (!snap || snap.lod >= 3) return null;
    if (snap.lod === 0) return snap;
    return {
        ...snap,
        lod: 0,
        lattice: snap.lattice ? upsampleScalar(snap.lattice, N, snap.lod) : null,
        flux:    snap.flux    ? upsampleVec3(snap.flux, N, snap.lod)    : null,
    };
}

/**
 * Conservative byte count of a snapshot. Mirrors the design-doc math:
 * ~20 B per voxel at LOD 0 (1 byte state + 12 byte flux + overhead).
 */
export function snapshotBytes({ lod, N }) {
    if (lod >= 3) return 128;                 // audit-only, ~8 floats
    const factor = LOD_FACTORS[lod];
    const M = Math.floor(N / factor);
    const perVoxel = 1 /* state */ + 12 /* flux vec3 */ + 4 /* overhead */;
    return M * M * M * perVoxel;
}
