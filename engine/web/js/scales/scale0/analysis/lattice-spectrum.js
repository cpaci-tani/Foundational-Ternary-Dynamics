/**
 * Lattice spectrum analysis — the spatial energy spectrum E(k) of the flux field.
 *
 * Pure, DOM-free, importable from BOTH the panel (main thread) and the physics
 * worker. No external deps (a self-contained radix-2 FFT).
 *
 * The energy spectrum (turbulence-style): FFT each component of J, sum the modal
 * power, and radial-bin by physical |k|. With the unitary normalization here,
 *     Σ_k E(k) = Σ_x |J(x)|²            (Parseval)
 * which the caller cross-checks against the audit's field energy — a built-in
 * validation that the instrument is correct. See SPEC_SCALE0_LATTICE_SPECTROSCOPY.md §3.
 *
 * The lattice side L is generally odd (33/49/65/97/129), so we always resample
 * the field onto a power-of-2 grid M (trilinear) before the radix-2 FFT. M<L
 * band-limits the spectrum to k < πM/L — the honest "live = large scales only,
 * Deep Measure = full band" distinction.
 */

// ── 1-D radix-2 Cooley–Tukey FFT (in place, forward; length must be 2^p) ──────
export function fft1d(re, im) {
    const n = re.length;
    // bit-reversal permutation
    for (let i = 1, j = 0; i < n; i++) {
        let bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            const tr = re[i]; re[i] = re[j]; re[j] = tr;
            const ti = im[i]; im[i] = im[j]; im[j] = ti;
        }
    }
    for (let len = 2; len <= n; len <<= 1) {
        const ang = -2 * Math.PI / len;          // forward transform
        const wr = Math.cos(ang), wi = Math.sin(ang);
        for (let i = 0; i < n; i += len) {
            let cwr = 1, cwi = 0;                 // twiddle, accumulated
            const half = len >> 1;
            for (let k = 0; k < half; k++) {
                const ar = re[i + k], ai = im[i + k];
                const br = re[i + k + half], bi = im[i + k + half];
                const vr = br * cwr - bi * cwi;
                const vi = br * cwi + bi * cwr;
                re[i + k] = ar + vr; im[i + k] = ai + vi;
                re[i + k + half] = ar - vr; im[i + k + half] = ai - vi;
                const ncwr = cwr * wr - cwi * wi;
                cwi = cwr * wi + cwi * wr;
                cwr = ncwr;
            }
        }
    }
}

// ── Separable 3-D FFT over a flat M³ array (idx = (z*M + y)*M + x), in place ──
export function fft3dInPlace(re, im, M) {
    const tr = new Float64Array(M), ti = new Float64Array(M);
    const M2 = M * M;
    // along x (contiguous)
    for (let z = 0; z < M; z++) for (let y = 0; y < M; y++) {
        const base = (z * M + y) * M;
        for (let x = 0; x < M; x++) { tr[x] = re[base + x]; ti[x] = im[base + x]; }
        fft1d(tr, ti);
        for (let x = 0; x < M; x++) { re[base + x] = tr[x]; im[base + x] = ti[x]; }
    }
    // along y (stride M)
    for (let z = 0; z < M; z++) for (let x = 0; x < M; x++) {
        const base = z * M2 + x;
        for (let y = 0; y < M; y++) { tr[y] = re[base + y * M]; ti[y] = im[base + y * M]; }
        fft1d(tr, ti);
        for (let y = 0; y < M; y++) { re[base + y * M] = tr[y]; im[base + y * M] = ti[y]; }
    }
    // along z (stride M²)
    for (let y = 0; y < M; y++) for (let x = 0; x < M; x++) {
        const base = y * M + x;
        for (let z = 0; z < M; z++) { tr[z] = re[base + z * M2]; ti[z] = im[base + z * M2]; }
        fft1d(tr, ti);
        for (let z = 0; z < M; z++) { re[base + z * M2] = tr[z]; im[base + z * M2] = ti[z]; }
    }
}

export function nextPow2(n) {
    let p = 1;
    while (p < n) p <<= 1;
    return p;
}

/** Trilinear resample a flat srcN³ scalar field onto a flat M³ grid (`dst`). */
export function resampleInto(src, srcN, M, dst) {
    if (srcN === M) { dst.set(src); return; }
    const scale = srcN / M;
    const sN = srcN;
    for (let z = 0; z < M; z++) {
        const sz = z * scale, z0 = Math.floor(sz), fz = sz - z0, z1 = Math.min(z0 + 1, sN - 1);
        for (let y = 0; y < M; y++) {
            const sy = y * scale, y0 = Math.floor(sy), fy = sy - y0, y1 = Math.min(y0 + 1, sN - 1);
            for (let x = 0; x < M; x++) {
                const sx = x * scale, x0 = Math.floor(sx), fx = sx - x0, x1 = Math.min(x0 + 1, sN - 1);
                const c000 = src[(z0 * sN + y0) * sN + x0], c100 = src[(z0 * sN + y0) * sN + x1];
                const c010 = src[(z0 * sN + y1) * sN + x0], c110 = src[(z0 * sN + y1) * sN + x1];
                const c001 = src[(z1 * sN + y0) * sN + x0], c101 = src[(z1 * sN + y0) * sN + x1];
                const c011 = src[(z1 * sN + y1) * sN + x0], c111 = src[(z1 * sN + y1) * sN + x1];
                const c00 = c000 * (1 - fx) + c100 * fx, c10 = c010 * (1 - fx) + c110 * fx;
                const c01 = c001 * (1 - fx) + c101 * fx, c11 = c011 * (1 - fx) + c111 * fx;
                const c0 = c00 * (1 - fy) + c10 * fy, c1 = c01 * (1 - fy) + c11 * fy;
                dst[(z * M + y) * M + x] = c0 * (1 - fz) + c1 * fz;
            }
        }
    }
}

/**
 * Energy spectrum E(k) of a vector field J = (jx, jy, jz), each a flat srcN³
 * array sampling the L-voxel periodic box.
 *
 * @returns {{k:number[], E:number[], kNyq:number, totalE:number, sumReal:number,
 *            M:number, boxL:number}} — k bin centres (rad/voxel), shell energies,
 *   Nyquist, ΣE(k) incl. DC (= Σ|J|² by Parseval), the real-space Σ|J|² on the
 *   M-grid (for the self-consistency ratio totalE/sumReal ≈ 1), and the grid params.
 */
export function energySpectrum(comps, srcN, M, boxL) {
    const Mc = M * M * M;
    const re = new Float64Array(Mc), im = new Float64Array(Mc);
    const power = new Float64Array(Mc);
    let sumReal = 0;

    for (const key of ['jx', 'jy', 'jz']) {
        resampleInto(comps[key], srcN, M, re);
        im.fill(0);
        for (let i = 0; i < Mc; i++) sumReal += re[i] * re[i];
        fft3dInPlace(re, im, M);
        for (let i = 0; i < Mc; i++) power[i] += re[i] * re[i] + im[i] * im[i];
    }

    const k0 = 2 * Math.PI / boxL;               // fundamental wavenumber (rad/voxel)
    const kNyq = k0 * (M / 2);
    const nBins = Math.max(1, Math.floor(M / 2));
    const dk = kNyq / nBins;
    const Esum = new Float64Array(nBins);
    const kMid = new Float64Array(nBins);
    for (let b = 0; b < nBins; b++) kMid[b] = (b + 0.5) * dk;

    const norm = 1 / Mc;                          // unnormalized FFT ⇒ divide by N for Parseval
    let totalE = 0;
    const half = M / 2;
    for (let z = 0; z < M; z++) {
        const kz = z <= half ? z : z - M;
        for (let y = 0; y < M; y++) {
            const ky = y <= half ? y : y - M;
            const row = (z * M + y) * M;
            for (let x = 0; x < M; x++) {
                const kx = x <= half ? x : x - M;
                const p = power[row + x] * norm;
                totalE += p;
                if (kx === 0 && ky === 0 && kz === 0) continue;   // exclude DC from E(k)
                const kmag = k0 * Math.sqrt(kx * kx + ky * ky + kz * kz);
                let b = Math.floor(kmag / dk);
                if (b >= nBins) b = nBins - 1;
                Esum[b] += p;
            }
        }
    }
    return { k: Array.from(kMid), E: Array.from(Esum), kNyq, totalE, sumReal, M, boxL };
}

/**
 * Reconstruct dense jx/jy/jz grids (srcN³) from a SPARSE flux-vector sample set
 * (getFluxVectorSampled skips near-zero voxels — those are genuine zeros). Sample
 * positions are voxel+0.5 at `stride` spacing. srcN = ceil(L/stride); stride 1 ⇒
 * the full field. Returns { jx, jy, jz, srcN } for energySpectrum().
 */
export function denseVectorGridFromSamples(samples, L, stride) {
    const srcN = Math.ceil(L / stride);
    const Nc = srcN * srcN * srcN;
    const jx = new Float64Array(Nc), jy = new Float64Array(Nc), jz = new Float64Array(Nc);
    const pos = samples.positions, vec = samples.vectors, count = samples.count | 0;
    for (let i = 0; i < count; i++) {
        const gx = Math.round((pos[i * 3] - 0.5) / stride);
        const gy = Math.round((pos[i * 3 + 1] - 0.5) / stride);
        const gz = Math.round((pos[i * 3 + 2] - 0.5) / stride);
        if (gx < 0 || gx >= srcN || gy < 0 || gy >= srcN || gz < 0 || gz >= srcN) continue;
        const idx = (gz * srcN + gy) * srcN + gx;
        jx[idx] = vec[i * 3]; jy[idx] = vec[i * 3 + 1]; jz[idx] = vec[i * 3 + 2];
    }
    return { jx, jy, jz, srcN };
}

/** Dominant mode: the k bin with the most energy; λ* = 2π/k*. */
export function spectralPeak(k, E) {
    let bi = -1, bv = -Infinity;
    for (let i = 0; i < E.length; i++) { if (E[i] > bv) { bv = E[i]; bi = i; } }
    if (bi < 0 || k[bi] <= 0) return { kPeak: 0, lambdaPeak: Infinity, energy: 0 };
    return { kPeak: k[bi], lambdaPeak: 2 * Math.PI / k[bi], energy: bv };
}

/** Power-law slope p (E ~ k^p) by least-squares on log E vs log k over the
 *  resolved inertial range (positive bins, excluding the first/last two). */
export function spectralSlope(k, E) {
    const xs = [], ys = [];
    for (let i = 2; i < E.length - 2; i++) {
        if (E[i] > 0 && k[i] > 0) { xs.push(Math.log(k[i])); ys.push(Math.log(E[i])); }
    }
    const n = xs.length;
    if (n < 3) return { slope: NaN, n };
    let sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (let i = 0; i < n; i++) { sx += xs[i]; sy += ys[i]; sxx += xs[i] * xs[i]; sxy += xs[i] * ys[i]; }
    const denom = n * sxx - sx * sx;
    const slope = denom !== 0 ? (n * sxy - sx * sy) / denom : NaN;
    return { slope, n };
}
