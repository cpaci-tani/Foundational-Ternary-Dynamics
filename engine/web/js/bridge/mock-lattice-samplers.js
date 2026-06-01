/**
 * Scale-0 lattice samplers — MockBridge side only.
 *
 * Every sampler here takes a stride and returns a plain data record:
 *
 *     { positions: Float32Array, vectors|values: Float32Array, count: number }
 *
 * Positions are emitted at voxel-centre world coordinates (`x + 0.5`) so
 * the output aligns with the Scale-0 rendering convention used by every
 * overlay that consumes these samples.
 *
 * Extracted from `bridge-init.js` as Wave 1 ticket 2 of the
 * large-file refactor (see docs/SPEC_REFACTOR_LARGE_FILES.md §4). The
 * extraction is a move, not a rewrite — every sampler body is preserved
 * verbatim, and the latency-proxy cache-invalidation chain stays intact
 * because `state` is the live MockBridge instance (not a destructured
 * copy). Writes to `state._latencyProxy` + `state._latencyProxyTick`
 * performed by `buildLatencyProxy()` below propagate back to MockBridge,
 * and MockBridge's existing invalidation sites (`reset`, `setScale0Tick`,
 * `setScale0FluxBuffer`, `setScale0WaveBuffer`) continue to work unchanged.
 *
 * STATE CONTRACT — `state` must expose (all live references):
 *   Read:
 *     latticeSize      number
 *     _fluxJ           Float64Array|null     flux vector field (N^3 × 3)
 *     _fluxWV          Float64Array|null     wave velocity field (N^3 × 3)
 *     _tick            number                current simulation tick
 *     _particles       Array                 manifested particles
 *     _params          { gn: number }        simulation parameters
 *     _toggles         { gravity: boolean }  physics toggles
 *   Read + write (cache fields):
 *     _latencyProxy      Float32Array|null
 *     _latencyProxyTick  number
 *   Method:
 *     _fluxIdx(x,y,z): number   periodic-wrap flat index
 *
 * The helper `_buildLatencyProxy` is exposed as `buildLatencyProxy` on the
 * returned object so MockBridge can call it for downstream needs if they
 * emerge. For now only `getKretschmannSampled` and `getLatencySampled`
 * invoke it internally.
 */

import {
    ALPHA, K_B,
    COULOMB_K_FORCE,
    STRONG_ALPHA_S, STRONG_RUN_COEFF, STRONG_R_COULOMB, STRONG_R_LINEAR,
    STRONG_TRANSITION_DENOM, STRONG_LINEAR_DENOM,
    STRONG_COLOR_REPEL, STRONG_COLOR_ATTRACT,
    LAPLACIAN_FACE_WEIGHT, LAPLACIAN_EDGE_WEIGHT,
} from '../constants.js';

/**
 * Build all 17 lattice samplers + the latency-proxy helper, bound to the
 * given bridge-like state object. Returns a plain object whose methods
 * are stateless relative to each other — all mutable state lives on the
 * supplied `state` reference.
 *
 * @param {object} state — MockBridge instance or equivalent state bag.
 * @returns {object} sampler methods keyed by their public names
 */
export function createLatticeSamplers(state) {
    // ── Persistent sampler scratch buffers (F-2) ──────────────────────
    // Pre-refactor each sampler call allocated fresh `new Float32Array`s for
    // its `positions` + `vectors`/`values` outputs — ~50 allocations and
    // ~30k floats per overlay refresh at L=32, stride=2 (~3 MB/s GC churn).
    // Instead we keep one reusable buffer per (sampler, role) slot, sized to
    // the current `maxPts`, and grow it only when a larger lattice/finer
    // stride demands more capacity. This is output-exact: every sampler only
    // ever WRITES indices [0, count) and every consumer only READS [0, count)
    // (renderers + field-overlays all loop `i < count`), so stale tail data
    // beyond `count` is never observed. The empty-guard early returns keep
    // returning fresh length-0 arrays (unchanged, negligible).
    //
    // Keyed by an integer slot id (one per output array across all samplers)
    // so two samplers never alias the same backing store within a frame.
    const _buf = [];
    function scratch(slot, len) {
        let b = _buf[slot];
        if (b === undefined || b.length < len) {
            b = new Float32Array(len);
            _buf[slot] = b;
        }
        return b;
    }

    // ── Persistent locality mask for getStrongForceField (F-16) ───────
    // The strong-force field is SHORT-RANGE / tube-localized: a voxel can only
    // contribute via (a) a flux-tube envelope `tubeEnv ≥ 0.01` — which forces
    // perpendicular distance ≤ √(2·TUBE_W²·ln100) ≈ 4.55 from the tube segment
    // and axial projection within [-1, sep+1] — or (b) short-range nuclear
    // attraction at wrapped `r ≤ 5` from a quark. Every voxel outside the union
    // of these neighbourhoods evaluates to exactly fx=fy=fz=0 and is dropped by
    // the existing `mag < 1e-4` guard. Pre-refactor the sampler still ran the
    // full per-pair + per-quark inner work for all N³ voxels — ~97% of which are
    // provably non-contributing at a typical compact Moore cell (measured: 969
    // contributing / 32768 scanned). We restrict the inner work to a conservative
    // candidate set by stamping an axis-aligned box of half-width STRONG_REACH
    // (≥ both reaches) around each tube-segment sample point and each quark,
    // with periodic wrap, then gating the voxel loop on membership.
    //
    // EXACTNESS: the mask is a strict SUPERSET of contributing voxels (REACH ≥
    // max physical reach, segments sampled every REACH voxels so long tubes are
    // fully covered, wrap handled via mod N). The (z,y,x) iteration order and the
    // identical force computation + `mag < 1e-4` test run unchanged for every
    // gated-in voxel, so the emitted (position, vector) set and its order — and
    // thus the `maxPts` truncation point — are BIT-IDENTICAL to the brute scan.
    //
    // Zero-GC: a single persistent Int32Array holds a per-voxel "stamp
    // generation"; each call bumps `_strongMaskGen` and stamps that value, so a
    // voxel is a candidate iff `mask[idx] === gen`. This needs no per-call
    // allocation and no per-call clear (O(1) reset by incrementing the counter),
    // mirroring the F-2 buffer-reuse and the _latencyProxy persistent-buffer
    // patterns already in this file. Grown only on lattice resize.
    const STRONG_REACH = 6; // ⌈max(4.55 perp + 1 axial, 5 quark)⌉, conservative
    let _strongMask = null;
    let _strongMaskGen = 0;

    // ── Pure lattice readers (12 of 14 scalar/vector samplers) ────────

    function getEFieldSampled(stride = 2) {
        if (!state._fluxWV) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(0, maxPts * 3);
        const vectors = scratch(1, maxPts * 3);
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const idx = state._fluxIdx(x, y, z);
                    // E = -wave_vel
                    const ex = -state._fluxWV[idx * 3];
                    const ey = -state._fluxWV[idx * 3 + 1];
                    const ez = -state._fluxWV[idx * 3 + 2];
                    const mag = Math.sqrt(ex * ex + ey * ey + ez * ez);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    vectors[count * 3] = ex; vectors[count * 3 + 1] = ey; vectors[count * 3 + 2] = ez;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    function getBFieldSampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(2, maxPts * 3);
        const vectors = scratch(3, maxPts * 3);
        const J = state._fluxJ;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    // B = curl(J) via 6-point discrete curl
                    const xp = state._fluxIdx(x + 1, y, z), xm = state._fluxIdx(x - 1, y, z);
                    const yp = state._fluxIdx(x, y + 1, z), ym = state._fluxIdx(x, y - 1, z);
                    const zp = state._fluxIdx(x, y, z + 1), zm = state._fluxIdx(x, y, z - 1);
                    const bx = (J[yp * 3 + 2] - J[ym * 3 + 2]) / 2 - (J[zp * 3 + 1] - J[zm * 3 + 1]) / 2;
                    const by = (J[zp * 3] - J[zm * 3]) / 2 - (J[xp * 3 + 2] - J[xm * 3 + 2]) / 2;
                    const bz = (J[xp * 3 + 1] - J[xm * 3 + 1]) / 2 - (J[yp * 3] - J[ym * 3]) / 2;
                    const mag = Math.sqrt(bx * bx + by * by + bz * bz);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    vectors[count * 3] = bx; vectors[count * 3 + 1] = by; vectors[count * 3 + 2] = bz;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    function getPoyntingSampled(stride = 2) {
        if (!state._fluxJ || !state._fluxWV) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(4, maxPts * 3);
        const vectors = scratch(5, maxPts * 3);
        const J = state._fluxJ, WV = state._fluxWV;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const idx = state._fluxIdx(x, y, z);
                    // E = -wave_vel
                    const ex = -WV[idx * 3], ey = -WV[idx * 3 + 1], ez = -WV[idx * 3 + 2];
                    // B = curl(J)
                    const xp = state._fluxIdx(x + 1, y, z), xm = state._fluxIdx(x - 1, y, z);
                    const yp = state._fluxIdx(x, y + 1, z), ym = state._fluxIdx(x, y - 1, z);
                    const zpp = state._fluxIdx(x, y, z + 1), zm = state._fluxIdx(x, y, z - 1);
                    const bx = (J[yp * 3 + 2] - J[ym * 3 + 2]) / 2 - (J[zpp * 3 + 1] - J[zm * 3 + 1]) / 2;
                    const by = (J[zpp * 3] - J[zm * 3]) / 2 - (J[xp * 3 + 2] - J[xm * 3 + 2]) / 2;
                    const bz = (J[xp * 3 + 1] - J[xm * 3 + 1]) / 2 - (J[yp * 3] - J[ym * 3]) / 2;
                    // S = E × B
                    const sx = ey * bz - ez * by;
                    const sy = ez * bx - ex * bz;
                    const sz = ex * by - ey * bx;
                    const mag = Math.sqrt(sx * sx + sy * sy + sz * sz);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    vectors[count * 3] = sx; vectors[count * 3 + 1] = sy; vectors[count * 3 + 2] = sz;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    function getDivJSampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(6, maxPts * 3);
        const values = scratch(7, maxPts);
        const J = state._fluxJ;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const xp = state._fluxIdx(x + 1, y, z), xm = state._fluxIdx(x - 1, y, z);
                    const yp = state._fluxIdx(x, y + 1, z), ym = state._fluxIdx(x, y - 1, z);
                    const zp = state._fluxIdx(x, y, z + 1), zm = state._fluxIdx(x, y, z - 1);
                    const div = (J[xp * 3] - J[xm * 3]) / 2 + (J[yp * 3 + 1] - J[ym * 3 + 1]) / 2 + (J[zp * 3 + 2] - J[zm * 3 + 2]) / 2;
                    if (Math.abs(div) < 1e-15) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    values[count] = div;
                    count++;
                }
            }
        }
        return { positions, values, count };
    }

    /**
     * |∇×J|(x) sampled on the stride grid. Central differences on the
     * full engine flux field; skips voxels whose curl is below numerical
     * noise so the overlay stays sparse in flat regions.
     */
    function getVorticitySampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(8, maxPts * 3);
        const values = scratch(9, maxPts);
        const J = state._fluxJ;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const xp = state._fluxIdx(x + 1, y, z), xm = state._fluxIdx(x - 1, y, z);
                    const yp = state._fluxIdx(x, y + 1, z), ym = state._fluxIdx(x, y - 1, z);
                    const zp = state._fluxIdx(x, y, z + 1), zm = state._fluxIdx(x, y, z - 1);
                    // (∂_y J_z − ∂_z J_y,   ∂_z J_x − ∂_x J_z,   ∂_x J_y − ∂_y J_x)
                    const cx = (J[yp * 3 + 2] - J[ym * 3 + 2]) / 2
                             - (J[zp * 3 + 1] - J[zm * 3 + 1]) / 2;
                    const cy = (J[zp * 3]     - J[zm * 3])     / 2
                             - (J[xp * 3 + 2] - J[xm * 3 + 2]) / 2;
                    const cz = (J[xp * 3 + 1] - J[xm * 3 + 1]) / 2
                             - (J[yp * 3]     - J[ym * 3])     / 2;
                    const mag = Math.sqrt(cx * cx + cy * cy + cz * cz);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    values[count] = mag;
                    count++;
                }
            }
        }
        return { positions, values, count };
    }

    // Curl of the flux field: W(x) = ∇×J. Pseudovector (parity-odd),
    // nonzero wherever J has rotational structure. Used as the physical
    // basis for the WEAK FORCE overlay — the weak interaction is parity-
    // violating, so its natural vector proxy is a pseudovector of the
    // underlying field, not the field itself. The older weak visual used
    // `J × DUAL_DELTA` (a scaled copy of J), which for any polarized
    // scenario (e.g. Flux Pulse has J = (Gaussian, 0, 0)) pointed every
    // arrow the same way — visually misleading and physically wrong.
    // Returns the same { positions, vectors, count } shape as the other
    // sampled vector fields, emitted at voxel-centre world coords. Skips
    // boundary voxels (periodic-wrap stencil would manufacture fake curl).
    function getCurlJSampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(10, maxPts * 3);
        const vectors = scratch(11, maxPts * 3);
        const J = state._fluxJ;
        let count = 0;
        for (let z = 1; z < N - 1; z += stride) {
            for (let y = 1; y < N - 1; y += stride) {
                for (let x = 1; x < N - 1; x += stride) {
                    const xp = state._fluxIdx(x + 1, y, z), xm = state._fluxIdx(x - 1, y, z);
                    const yp = state._fluxIdx(x, y + 1, z), ym = state._fluxIdx(x, y - 1, z);
                    const zp = state._fluxIdx(x, y, z + 1), zm = state._fluxIdx(x, y, z - 1);
                    // Central-difference curl: (∂_y J_z − ∂_z J_y, …).
                    const cx = (J[yp * 3 + 2] - J[ym * 3 + 2]) * 0.5
                             - (J[zp * 3 + 1] - J[zm * 3 + 1]) * 0.5;
                    const cy = (J[zp * 3]     - J[zm * 3])     * 0.5
                             - (J[xp * 3 + 2] - J[xm * 3 + 2]) * 0.5;
                    const cz = (J[xp * 3 + 1] - J[xm * 3 + 1]) * 0.5
                             - (J[yp * 3]     - J[ym * 3])     * 0.5;
                    const mag2 = cx * cx + cy * cy + cz * cz;
                    if (mag2 < 1e-30) continue;
                    positions[count * 3]     = x + 0.5;
                    positions[count * 3 + 1] = y + 0.5;
                    positions[count * 3 + 2] = z + 0.5;
                    vectors[count * 3]     = cx;
                    vectors[count * 3 + 1] = cy;
                    vectors[count * 3 + 2] = cz;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    function getFluxVectorSampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(12, maxPts * 3);
        const vectors = scratch(13, maxPts * 3);
        const J = state._fluxJ;
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const idx = state._fluxIdx(x, y, z);
                    const jx = J[idx * 3], jy = J[idx * 3 + 1], jz = J[idx * 3 + 2];
                    const mag = Math.sqrt(jx * jx + jy * jy + jz * jz);
                    if (mag < 1e-15) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    vectors[count * 3] = jx; vectors[count * 3 + 1] = jy; vectors[count * 3 + 2] = jz;
                    count++;
                }
            }
        }
        return { positions, vectors, count };
    }

    // Helicity density h(x) = J(x) · (∇×J)(x).  Scalar, signed. Measures
    // field-line linking — nonzero iff flow has both rotational and axial
    // components (Beltrami flows, vortex tubes, current helices).
    function getHelicitySampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(14, maxPts * 3);
        const values = scratch(15, maxPts);
        const J = state._fluxJ;
        let count = 0;
        // Skip one-voxel border: `_fluxIdx` wraps periodically, so at x=0 or
        // x=N-1 the curl stencil pulls in values from the opposite wall and
        // manufactures spurious helicity at the lattice faces. Physics is
        // still periodic inside — this just keeps the VISUAL honest.
        for (let z = 1; z < N - 1; z += stride) {
            for (let y = 1; y < N - 1; y += stride) {
                for (let x = 1; x < N - 1; x += stride) {
                    const xp = state._fluxIdx(x + 1, y, z), xm = state._fluxIdx(x - 1, y, z);
                    const yp = state._fluxIdx(x, y + 1, z), ym = state._fluxIdx(x, y - 1, z);
                    const zp = state._fluxIdx(x, y, z + 1), zm = state._fluxIdx(x, y, z - 1);
                    // Central-difference curl components.
                    const cx = (J[yp * 3 + 2] - J[ym * 3 + 2]) / 2
                             - (J[zp * 3 + 1] - J[zm * 3 + 1]) / 2;
                    const cy = (J[zp * 3]     - J[zm * 3])     / 2
                             - (J[xp * 3 + 2] - J[xm * 3 + 2]) / 2;
                    const cz = (J[xp * 3 + 1] - J[xm * 3 + 1]) / 2
                             - (J[yp * 3]     - J[ym * 3])     / 2;
                    const idx = state._fluxIdx(x, y, z);
                    const jx = J[idx * 3], jy = J[idx * 3 + 1], jz = J[idx * 3 + 2];
                    const h = jx * cx + jy * cy + jz * cz;
                    if (Math.abs(h) < 1e-15) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    values[count] = h;
                    count++;
                }
            }
        }
        return { positions, values, count };
    }

    // ── Latency proxy + curvature / latency / Fisher / coherence ──────

    /**
     * Helper: build a lattice-wide latency-proxy array from |J|². The
     * C++ engine's solve_latency_poisson() uses a Poisson-solved φ and
     * sets `L = sqrt(clamp(|φ|, 0, 0.998))`. On the JS MockBridge we
     * don't run the Poisson solver, so we use `|J|²` as the mass-density
     * proxy — matches the C++ behaviour in the strong-field limit and
     * keeps the Kretschmann / horizon overlays live on the web engine.
     *
     * Caching: writes `state._latencyProxy` + `state._latencyProxyTick`
     * through the live state reference. MockBridge's existing cache
     * invalidation sites (reset / setScale0Tick / setScale0FluxBuffer /
     * setScale0WaveBuffer) still apply because they mutate the same
     * fields this helper reads/writes.
     */
    function buildLatencyProxy() {
        if (!state._fluxJ) return null;
        const N = state.latticeSize;
        const M = N * N * N;
        // Size-check first — a lattice-resize between ticks would otherwise
        // cause silent OOB writes into an undersized buffer (TypedArray drops
        // them) and read zeros for every out-of-range voxel.
        if (!state._latencyProxy || state._latencyProxy.length !== M) {
            state._latencyProxy = new Float32Array(M);
            state._latencyProxyTick = -1;   // force rebuild after resize
        }
        if (state._latencyProxyTick === state._tick) {
            return state._latencyProxy;
        }
        const L = state._latencyProxy;
        const J = state._fluxJ;
        // First pass: compute |J|² and track the max for normalisation.
        let maxRho = 1e-30;
        for (let i = 0; i < M; i++) {
            const jx = J[i*3], jy = J[i*3+1], jz = J[i*3+2];
            const rho = jx*jx + jy*jy + jz*jz;
            L[i] = rho;
            if (rho > maxRho) maxRho = rho;
        }
        // Second pass: normalise and take sqrt with horizon clamp.
        const inv = 1.0 / maxRho;
        for (let i = 0; i < M; i++) {
            const rn = Math.min(L[i] * inv, 0.998);
            L[i] = Math.sqrt(rn);
        }
        state._latencyProxy = L;
        state._latencyProxyTick = state._tick;
        return L;
    }

    // Kretschmann-like curvature proxy K(x) = (∇²L)².  Uses the same
    // 18-point Moore Laplacian (consistent with the wave-equation
    // stencil) applied to the per-voxel latency proxy.  [PROXY].
    function getKretschmannSampled(stride = 2) {
        const Lvox = buildLatencyProxy();
        if (!Lvox) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(16, maxPts * 3);
        const values = scratch(17, maxPts);
        let count = 0;
        // 18-pt isotropic Laplacian weights (Patra-Karttunen 2006) sourced
        // from constants.js (audit P2-9 fix, 2026-05-27).
        const INV3 = LAPLACIAN_FACE_WEIGHT, INV6 = LAPLACIAN_EDGE_WEIGHT;
        // Skip lattice boundary — `_fluxIdx` wraps, and a curvature proxy
        // that reads across the periodic seam manufactures spurious spikes
        // along the walls.
        for (let z = 1; z < N - 1; z += stride) {
            for (let y = 1; y < N - 1; y += stride) {
                for (let x = 1; x < N - 1; x += stride) {
                    const self = Lvox[state._fluxIdx(x, y, z)];
                    let faceSum = 0, edgeSum = 0;
                    faceSum += Lvox[state._fluxIdx(x+1, y, z)];
                    faceSum += Lvox[state._fluxIdx(x-1, y, z)];
                    faceSum += Lvox[state._fluxIdx(x, y+1, z)];
                    faceSum += Lvox[state._fluxIdx(x, y-1, z)];
                    faceSum += Lvox[state._fluxIdx(x, y, z+1)];
                    faceSum += Lvox[state._fluxIdx(x, y, z-1)];
                    edgeSum += Lvox[state._fluxIdx(x+1, y+1, z)];
                    edgeSum += Lvox[state._fluxIdx(x+1, y-1, z)];
                    edgeSum += Lvox[state._fluxIdx(x-1, y+1, z)];
                    edgeSum += Lvox[state._fluxIdx(x-1, y-1, z)];
                    edgeSum += Lvox[state._fluxIdx(x+1, y, z+1)];
                    edgeSum += Lvox[state._fluxIdx(x+1, y, z-1)];
                    edgeSum += Lvox[state._fluxIdx(x-1, y, z+1)];
                    edgeSum += Lvox[state._fluxIdx(x-1, y, z-1)];
                    edgeSum += Lvox[state._fluxIdx(x, y+1, z+1)];
                    edgeSum += Lvox[state._fluxIdx(x, y+1, z-1)];
                    edgeSum += Lvox[state._fluxIdx(x, y-1, z+1)];
                    edgeSum += Lvox[state._fluxIdx(x, y-1, z-1)];
                    const lap = INV3 * faceSum + INV6 * edgeSum - 4 * self;
                    const K = lap * lap;
                    if (K < 1e-18) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    values[count] = K;
                    count++;
                }
            }
        }
        return { positions, values, count };
    }

    // Per-voxel latency scalar L(x) — uses the |J|² proxy. Event-horizon
    // overlay thresholds this at L ≥ ~0.95.
    function getLatencySampled(stride = 2) {
        const Lvox = buildLatencyProxy();
        if (!Lvox) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(18, maxPts * 3);
        const values = scratch(19, maxPts);
        let count = 0;
        for (let z = 0; z < N; z += stride) {
            for (let y = 0; y < N; y += stride) {
                for (let x = 0; x < N; x += stride) {
                    const L = Lvox[state._fluxIdx(x, y, z)];
                    if (L < 1e-6) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    values[count] = L;
                    count++;
                }
            }
        }
        return { positions, values, count };
    }

    // Fisher information density F(x) = |∇ρ|² / ρ with ρ = |J|².
    function getFisherSampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(20, maxPts * 3);
        const values = scratch(21, maxPts);
        const J = state._fluxJ;
        const rhoAt = (xi, yi, zi) => {
            const i = state._fluxIdx(xi, yi, zi);
            const jx = J[i*3], jy = J[i*3+1], jz = J[i*3+2];
            return jx*jx + jy*jy + jz*jz;
        };
        const eps = 1e-8;
        let count = 0;
        // Boundary skip for the same periodic-wrap reason as helicity /
        // Kretschmann / coherence: gradient stencil at the wall is
        // contaminated by the opposite face.
        for (let z = 1; z < N - 1; z += stride) {
            for (let y = 1; y < N - 1; y += stride) {
                for (let x = 1; x < N - 1; x += stride) {
                    const rho = rhoAt(x, y, z);
                    if (rho < eps) continue;
                    const dxr = (rhoAt(x+1, y, z) - rhoAt(x-1, y, z)) * 0.5;
                    const dyr = (rhoAt(x, y+1, z) - rhoAt(x, y-1, z)) * 0.5;
                    const dzr = (rhoAt(x, y, z+1) - rhoAt(x, y, z-1)) * 0.5;
                    const F = (dxr*dxr + dyr*dyr + dzr*dzr) / rho;
                    if (F < 1e-12) continue;
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    values[count] = F;
                    count++;
                }
            }
        }
        return { positions, values, count };
    }

    // Dual-substrate coherence C(x) = (J · ∇×J) / (|J| · |∇×J|). [PROXY].
    function getCoherenceSampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(22, maxPts * 3);
        const values = scratch(23, maxPts);
        const J = state._fluxJ;
        const eps = 1e-10;
        let count = 0;
        // Skip boundary voxels (see helicity sampler comment).
        for (let z = 1; z < N - 1; z += stride) {
            for (let y = 1; y < N - 1; y += stride) {
                for (let x = 1; x < N - 1; x += stride) {
                    const xp = state._fluxIdx(x + 1, y, z), xm = state._fluxIdx(x - 1, y, z);
                    const yp = state._fluxIdx(x, y + 1, z), ym = state._fluxIdx(x, y - 1, z);
                    const zp = state._fluxIdx(x, y, z + 1), zm = state._fluxIdx(x, y, z - 1);
                    const cx = (J[yp*3+2] - J[ym*3+2]) * 0.5 - (J[zp*3+1] - J[zm*3+1]) * 0.5;
                    const cy = (J[zp*3]   - J[zm*3])   * 0.5 - (J[xp*3+2] - J[xm*3+2]) * 0.5;
                    const cz = (J[xp*3+1] - J[xm*3+1]) * 0.5 - (J[yp*3]   - J[ym*3])   * 0.5;
                    const idx = state._fluxIdx(x, y, z);
                    const jx = J[idx*3], jy = J[idx*3+1], jz = J[idx*3+2];
                    const jmag = Math.sqrt(jx*jx + jy*jy + jz*jz);
                    const cmag = Math.sqrt(cx*cx + cy*cy + cz*cz);
                    if (jmag < eps || cmag < eps) continue;
                    const C = (jx*cx + jy*cy + jz*cz) / (jmag * cmag);
                    positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
                    values[count] = C;
                    count++;
                }
            }
        }
        return { positions, values, count };
    }

    // ── Particle-anchored force samplers (4) ──────────────────────────

    /** Net force (Coulomb + optional gravity) at sampled grid points from particles. */
    function getForceFieldSampled(stride = 2) {
        const ps = state._particles.filter(p => p.state !== 0);
        if (ps.length === 0) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const halfN = N / 2;
        const alpha4pi = COULOMB_K_FORCE;
        const gn = state._params.gn;
        const doGravity = state._toggles.gravity;
        const soft = 1.0;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(24, maxPts * 3);
        const vectors = scratch(25, maxPts * 3);
        let count = 0;

        for (let z = 0; z < N; z += stride)
        for (let y = 0; y < N; y += stride)
        for (let x = 0; x < N; x += stride) {
            let fx = 0, fy = 0, fz = 0;
            for (const p of ps) {
                let dx = p.x - x, dy = p.y - y, dz = p.z - z;
                if (dx > halfN) dx -= N; else if (dx < -halfN) dx += N;
                if (dy > halfN) dy -= N; else if (dy < -halfN) dy += N;
                if (dz > halfN) dz -= N; else if (dz < -halfN) dz += N;
                const r2 = dx * dx + dy * dy + dz * dz + soft;
                const r = Math.sqrt(r2);
                const invR2 = 1 / r2;
                const invR = 1 / r;
                // Coulomb: test charge +1 feels force from particle charge
                fx += -alpha4pi * p.state * invR2 * invR * dx;
                fy += -alpha4pi * p.state * invR2 * invR * dy;
                fz += -alpha4pi * p.state * invR2 * invR * dz;
                // Gravity (attractive toward particles)
                if (doGravity) {
                    fx += gn * K_B * K_B * invR2 * invR * dx;
                    fy += gn * K_B * K_B * invR2 * invR * dy;
                    fz += gn * K_B * K_B * invR2 * invR * dz;
                }
            }
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            if (mag < 1e-12) continue;
            positions[count * 3] = x + 0.5;
            positions[count * 3 + 1] = y + 0.5;
            positions[count * 3 + 2] = z + 0.5;
            vectors[count * 3] = fx;
            vectors[count * 3 + 1] = fy;
            vectors[count * 3 + 2] = fz;
            count++;
        }
        return { positions, vectors, count };
    }

    /** Gravity = G_N * ∇ρ where ρ = |J| (flux magnitude). */
    function getGravityFieldSampled(stride = 2) {
        if (!state._fluxJ) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        // Local idx helper — DELIBERATELY different from state._fluxIdx:
        // this returns a flat offset ALREADY scaled by *3 (indexes into the
        // `_fluxJ` typed array directly). Preserved verbatim from the
        // pre-refactor bridge-init.js behaviour.
        const idx = (x, y, z) => {
            const wx = ((x % N) + N) % N, wy = ((y % N) + N) % N, wz = ((z % N) + N) % N;
            return (wz * N * N + wy * N + wx) * 3;
        };
        const density = (x, y, z) => {
            const i = idx(x, y, z);
            const jx = state._fluxJ[i], jy = state._fluxJ[i + 1], jz = state._fluxJ[i + 2];
            return Math.sqrt(jx * jx + jy * jy + jz * jz);
        };

        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(26, maxPts * 3);
        const vectors = scratch(27, maxPts * 3);
        let count = 0;
        const gn = state._params.gn;

        for (let z = 0; z < N; z += stride)
        for (let y = 0; y < N; y += stride)
        for (let x = 0; x < N; x += stride) {
            // Central difference gradient of density
            const gradX = (density(x + 1, y, z) - density(x - 1, y, z)) * 0.5;
            const gradY = (density(x, y + 1, z) - density(x, y - 1, z)) * 0.5;
            const gradZ = (density(x, y, z + 1) - density(x, y, z - 1)) * 0.5;
            const mag = Math.sqrt(gradX * gradX + gradY * gradY + gradZ * gradZ);
            if (mag < 1e-10) continue;
            positions[count * 3] = x + 0.5;
            positions[count * 3 + 1] = y + 0.5;
            positions[count * 3 + 2] = z + 0.5;
            vectors[count * 3] = gn * gradX;
            vectors[count * 3 + 1] = gn * gradY;
            vectors[count * 3 + 2] = gn * gradZ;
            count++;
        }
        return { positions, vectors, count };
    }

    /**
     * Sample EM (Coulomb) force field at grid points from particles.
     */
    function getEMForceField(stride = 2) {
        const ps = state._particles.filter(p => p.state !== 0);
        if (ps.length === 0) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const halfN = N / 2;
        const alpha4pi = COULOMB_K_FORCE;
        const soft = 1.0;
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(28, maxPts * 3);
        const vectors = scratch(29, maxPts * 3);
        let count = 0;

        for (let z = 0; z < N; z += stride)
        for (let y = 0; y < N; y += stride)
        for (let x = 0; x < N; x += stride) {
            let fx = 0, fy = 0, fz = 0;
            for (const p of ps) {
                let dx = p.x - x, dy = p.y - y, dz = p.z - z;
                if (dx > halfN) dx -= N; else if (dx < -halfN) dx += N;
                if (dy > halfN) dy -= N; else if (dy < -halfN) dy += N;
                if (dz > halfN) dz -= N; else if (dz < -halfN) dz += N;
                const r2 = dx * dx + dy * dy + dz * dz + soft;
                const invR2 = 1 / r2;
                const invR = 1 / Math.sqrt(r2);
                fx += -alpha4pi * p.state * invR2 * invR * dx;
                fy += -alpha4pi * p.state * invR2 * invR * dy;
                fz += -alpha4pi * p.state * invR2 * invR * dz;
            }
            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            if (mag < 1e-12) continue;
            positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
            vectors[count * 3] = fx; vectors[count * 3 + 1] = fy; vectors[count * 3 + 2] = fz;
            count++;
        }
        return { positions, vectors, count };
    }

    /** Alias: gravity force field → gravity field sampled. */
    function getGravityForceField(stride = 2) {
        return getGravityFieldSampled(stride);
    }

    /**
     * Strong/confinement force field: 3-regime model matching the C++
     * engine (Coulomb below r=3, transition 3–8, linear confinement
     * r≥8). Visualizes (1) flux tubes between particle pairs with force
     * arrows pointing INWARD from both ends, and (2) short-range
     * nuclear attraction toward each quark.
     */
    function getStrongForceField(stride = 2) {
        const ps = state._particles.filter(p => p.state !== 0);
        if (ps.length < 2) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
        const N = state.latticeSize;
        const halfN = N / 2;
        const ALPHA_S = STRONG_ALPHA_S;
        const TUBE_W  = 1.5;     // flux tube Gaussian width (visualization only)
        const maxPts = Math.ceil(N / stride) ** 3;
        const positions = scratch(30, maxPts * 3);
        const vectors = scratch(31, maxPts * 3);
        let count = 0;

        // Build ALL particle pairs with tube geometry
        const pairs = [];
        for (let i = 0; i < ps.length; i++) {
            for (let j = i + 1; j < ps.length; j++) {
                let dx = ps[j].x - ps[i].x, dy = ps[j].y - ps[i].y, dz = ps[j].z - ps[i].z;
                if (dx > halfN) dx -= N; else if (dx < -halfN) dx += N;
                if (dy > halfN) dy -= N; else if (dy < -halfN) dy += N;
                if (dz > halfN) dz -= N; else if (dz < -halfN) dz += N;
                const sep = Math.sqrt(dx * dx + dy * dy + dz * dz);
                if (sep < 0.5) continue;
                const invSep = 1 / sep;
                pairs.push({
                    ax: ps[i].x, ay: ps[i].y, az: ps[i].z,
                    dx, dy, dz, sep, invSep,
                    tx: dx * invSep, ty: dy * invSep, tz: dz * invSep
                });
            }
        }
        if (pairs.length === 0) return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };

        // Locality mask (F-16): stamp a conservative candidate region around the
        // tube segments + quarks so the inner per-pair/per-quark work runs only
        // where the field can be non-zero. See the STRONG_REACH note above for
        // the exactness argument (the mask is a superset; iteration order and
        // emitted set are bit-identical to the full brute scan).
        if (_strongMask === null || _strongMask.length < N * N * N) {
            _strongMask = new Int32Array(N * N * N);
            _strongMaskGen = 0;
        }
        const mask = _strongMask;
        const gen = ++_strongMaskGen;
        const stampBox = (cx, cy, cz) => {
            const x0 = Math.floor(cx - STRONG_REACH), x1 = Math.ceil(cx + STRONG_REACH);
            const y0 = Math.floor(cy - STRONG_REACH), y1 = Math.ceil(cy + STRONG_REACH);
            const z0 = Math.floor(cz - STRONG_REACH), z1 = Math.ceil(cz + STRONG_REACH);
            for (let sz = z0; sz <= z1; sz++) {
                const wz = ((sz % N) + N) % N;
                for (let sy = y0; sy <= y1; sy++) {
                    const wy = ((sy % N) + N) % N;
                    const rowBase = wz * N * N + wy * N;
                    for (let sx = x0; sx <= x1; sx++) {
                        const wx = ((sx % N) + N) % N;
                        mask[rowBase + wx] = gen;
                    }
                }
            }
        };
        for (let pi = 0; pi < pairs.length; pi++) {
            const pair = pairs[pi];
            // Stamp both tube endpoints (A and B = A + d) plus interior sample
            // points every STRONG_REACH voxels so a tube longer than 2·REACH is
            // still fully covered along its axis.
            const bx = pair.ax + pair.dx, by = pair.ay + pair.dy, bz = pair.az + pair.dz;
            stampBox(pair.ax, pair.ay, pair.az);
            stampBox(bx, by, bz);
            const steps = Math.ceil(pair.sep / STRONG_REACH);
            for (let s = 1; s < steps; s++) {
                const f = s / steps;
                stampBox(pair.ax + pair.dx * f, pair.ay + pair.dy * f, pair.az + pair.dz * f);
            }
        }
        for (let qi = 0; qi < ps.length; qi++) {
            stampBox(ps[qi].x, ps[qi].y, ps[qi].z);
        }

        for (let z = 0; z < N; z += stride)
        for (let y = 0; y < N; y += stride)
        for (let x = 0; x < N; x += stride) {
            // F-16 gate: skip voxels outside the stamped candidate region. These
            // are provably non-contributing (would yield mag < 1e-4 and be
            // dropped anyway), so this is output-exact — not an approximation.
            if (mask[z * N * N + y * N + x] !== gen) continue;

            let fx = 0, fy = 0, fz = 0;

            // 1. Flux tube visualization: force along tube, pointing INWARD from both ends
            for (const pair of pairs) {
                let rx = x - pair.ax, ry = y - pair.ay, rz = z - pair.az;
                if (rx > halfN) rx -= N; else if (rx < -halfN) rx += N;
                if (ry > halfN) ry -= N; else if (ry < -halfN) ry += N;
                if (rz > halfN) rz -= N; else if (rz < -halfN) rz += N;

                // Project onto tube axis
                const t = rx * pair.tx + ry * pair.ty + rz * pair.tz;
                if (t < -1.0 || t > pair.sep + 1.0) continue;

                // Perpendicular distance from tube axis
                const projX = t * pair.tx, projY = t * pair.ty, projZ = t * pair.tz;
                const perpX = rx - projX, perpY = ry - projY, perpZ = rz - projZ;
                const perp2 = perpX * perpX + perpY * perpY + perpZ * perpZ;
                const tubeEnv = Math.exp(-perp2 / (2 * TUBE_W * TUBE_W));
                if (tubeEnv < 0.01) continue;

                // 3-regime force magnitude (matching C++ engine)
                const r = Math.max(Math.sqrt(rx * rx + ry * ry + rz * rz), 0.5);
                const alpha_s_r = ALPHA_S / (1.0 + STRONG_RUN_COEFF * Math.log(1.0 + r));
                let fMag;
                if (r < STRONG_R_COULOMB) {
                    fMag = alpha_s_r / (r * r);                       // Coulomb
                } else if (r < STRONG_R_LINEAR) {
                    fMag = alpha_s_r / (STRONG_TRANSITION_DENOM * r); // Transition
                } else {
                    fMag = alpha_s_r * r / STRONG_LINEAR_DENOM;       // Linear confinement
                }
                fMag *= tubeEnv;

                // Direction: point INWARD from both ends toward the tube center
                // Near A (t < sep/2): force toward B (+tube_dir)
                // Near B (t > sep/2): force toward A (-tube_dir)
                const sign = (t < pair.sep * 0.5) ? 1.0 : -1.0;
                fx += fMag * pair.tx * sign;
                fy += fMag * pair.ty * sign;
                fz += fMag * pair.tz * sign;
            }

            // 2. Short-range nuclear: radial attraction toward each quark
            for (const p of ps) {
                let rx = x - p.x, ry = y - p.y, rz = z - p.z;
                if (rx > halfN) rx -= N; else if (rx < -halfN) rx += N;
                if (ry > halfN) ry -= N; else if (ry < -halfN) ry += N;
                if (rz > halfN) rz -= N; else if (rz < -halfN) rz += N;
                const r = Math.sqrt(rx * rx + ry * ry + rz * rz + 0.5);
                if (r > 5.0) continue;
                // Coulomb-like at short range with asymptotic freedom
                const alpha_s_r = ALPHA_S / (1.0 + STRONG_RUN_COEFF * Math.log(1.0 + r));
                const fNuc = alpha_s_r / (r * r);
                if (fNuc < 1e-4) continue;
                // Attractive: toward the quark (rx points away, so negate)
                fx -= fNuc * rx / r;
                fy -= fNuc * ry / r;
                fz -= fNuc * rz / r;
            }

            const mag = Math.sqrt(fx * fx + fy * fy + fz * fz);
            if (mag < 1e-4) continue;
            if (count >= maxPts) break;
            positions[count * 3] = x + 0.5; positions[count * 3 + 1] = y + 0.5; positions[count * 3 + 2] = z + 0.5;
            vectors[count * 3] = fx; vectors[count * 3 + 1] = fy; vectors[count * 3 + 2] = fz;
            count++;
        }
        return { positions, vectors, count };
    }

    return {
        getEFieldSampled,
        getBFieldSampled,
        getPoyntingSampled,
        getDivJSampled,
        getVorticitySampled,
        getCurlJSampled,
        getFluxVectorSampled,
        getHelicitySampled,
        buildLatencyProxy,
        getKretschmannSampled,
        getLatencySampled,
        getFisherSampled,
        getCoherenceSampled,
        getForceFieldSampled,
        getGravityFieldSampled,
        getEMForceField,
        getGravityForceField,
        getStrongForceField,
    };
}
