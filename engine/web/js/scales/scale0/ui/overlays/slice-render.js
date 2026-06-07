/**
 * Shared 2D-slice rendering helpers for Scale-0 heatmap panels.
 *
 * The transpose/raster helpers mirror the proven (pure) versions in
 * flux-slice-panel.js; the Gravity Observatory consumes them here, and
 * `paintSliceToCanvas` is a panel-agnostic distillation of the flux-slice
 * `_paintSlice` paint path. A future cleanup can have flux-slice import these
 * too (single source); kept duplicated for now to avoid touching that panel.
 *
 * Layout convention (matches the 3D viewport, Y-up):
 *   - sparse-sample helpers produce out[row*N + col] with col = first-named
 *     axis, row = N-1-(second axis) so +Z/+Y points UP.
 *   - bridge dense slices (getFluxSlice / getGravitySlice) come in as
 *     data[a*N + b]; transposeAndFlipNN rewrites them into the same panel
 *     convention in one pass.
 */

/**
 * Transpose + Y-flip an N×N row-major buffer from a bridge dense slice
 * (`data[a*N+b]`) into the panel convention `out[(N-1-c)*N + r] = buf[r*N + c]`.
 */
export function transposeAndFlipNN(buf, N) {
    if (!buf || buf.length !== N * N) return buf;
    const out = new Float64Array(N * N);
    const M = N - 1;
    for (let r = 0; r < N; r++) {
        for (let c = 0; c < N; c++) {
            out[(M - c) * N + r] = buf[r * N + c];
        }
    }
    return out;
}

/**
 * Rasterize a sparse VECTOR sample set onto an N×N plane as magnitudes.
 * @param {{positions:Float32Array, vectors:Float32Array, count:number}|null} sample
 * @param {0|1|2} axis 0→yz(x=mid), 1→xz(y=mid), 2→xy(z=mid)
 */
export function sliceVectorMag(sample, axis, mid, N) {
    const out = new Float64Array(N * N);
    if (!sample || !sample.count) return out;
    const pos = sample.positions, vec = sample.vectors;
    if (!pos || !vec) return out;
    const M = N - 1;
    for (let s = 0, p = 0, v = 0; s < sample.count; s++, p += 3, v += 3) {
        const ix = (pos[p] - 0.5) | 0, iy = (pos[p + 1] - 0.5) | 0, iz = (pos[p + 2] - 0.5) | 0;
        let row, col;
        if (axis === 0) { if (ix !== mid) continue; col = iy; row = M - iz; }
        else if (axis === 1) { if (iy !== mid) continue; col = ix; row = M - iz; }
        else { if (iz !== mid) continue; col = ix; row = M - iy; }
        if (col < 0 || col >= N || row < 0 || row >= N) continue;
        out[row * N + col] = Math.hypot(vec[v], vec[v + 1], vec[v + 2]);
    }
    return out;
}

/** Same as sliceVectorMag but for signed sparse scalars (preserves sign). */
export function sliceScalarSigned(sample, axis, mid, N) {
    const out = new Float64Array(N * N);
    if (!sample || !sample.count) return out;
    const pos = sample.positions, val = sample.values;
    if (!pos || !val) return out;
    const M = N - 1;
    for (let s = 0, p = 0; s < sample.count; s++, p += 3) {
        const ix = (pos[p] - 0.5) | 0, iy = (pos[p + 1] - 0.5) | 0, iz = (pos[p + 2] - 0.5) | 0;
        let row, col;
        if (axis === 0) { if (ix !== mid) continue; col = iy; row = M - iz; }
        else if (axis === 1) { if (iy !== mid) continue; col = ix; row = M - iz; }
        else { if (iz !== mid) continue; col = ix; row = M - iy; }
        if (col < 0 || col >= N || row < 0 || row >= N) continue;
        out[row * N + col] = val[s];
    }
    return out;
}

/**
 * Paint an N×N scalar plane into a canvas through a color ramp, upscaled with
 * nearest-neighbour. Self-contained (caches its RGBA buffer + temp canvas on
 * the canvas element). `data` is already in panel layout (post transpose/raster).
 * @param {HTMLCanvasElement} canvas
 * @param {ArrayLike<number>} data  length N*N
 * @param {number} N
 * @param {{ramp:Function, signed?:boolean, norm?:number}} opts
 *   ramp(t, rgbOut, 0) writes rgbOut[0..2] ∈ [0,1]; t ∈ [0,1] (or [-1,1] if signed).
 */
export function paintSliceToCanvas(canvas, data, N, { ramp, signed = false, norm = 1 } = {}) {
    if (!canvas) return;
    const ctx = canvas.getContext('2d', { alpha: false });
    if (!ctx) return;
    if (!data || data.length === 0 || !ramp) {
        ctx.fillStyle = '#0a0d14';
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        return;
    }
    let st = canvas._ftdSlice;
    if (!st || st.N !== N) {
        const buf = new Uint8ClampedArray(N * N * 4);
        const img = new ImageData(buf, N, N);
        const tmp = document.createElement('canvas');
        tmp.width = N; tmp.height = N;
        const tctx = tmp.getContext('2d', { alpha: false });
        tctx.imageSmoothingEnabled = false;
        st = canvas._ftdSlice = { N, buf, img, tmp, tctx };
    }
    const buf = st.buf, rgb = [0, 0, 0];
    for (let i = 0, p = 0; i < data.length; i++, p += 4) {
        let t = data[i] * norm;
        if (signed) { if (t > 1) t = 1; else if (t < -1) t = -1; }
        else { if (t > 1) t = 1; else if (t < 0) t = 0; }
        ramp(t, rgb, 0);
        buf[p] = (rgb[0] * 255) | 0;
        buf[p + 1] = (rgb[1] * 255) | 0;
        buf[p + 2] = (rgb[2] * 255) | 0;
        buf[p + 3] = 255;
    }
    st.tctx.putImageData(st.img, 0, 0);
    ctx.imageSmoothingEnabled = false;
    ctx.drawImage(st.tmp, 0, 0, N, N, 0, 0, canvas.width, canvas.height);
}
