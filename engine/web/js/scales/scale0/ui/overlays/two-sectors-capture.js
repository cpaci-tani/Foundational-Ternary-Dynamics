// Two Sectors — capture engine for the causality demo.
//
// Records the engine's two field sectors on an ISOLATED small-L WasmBridge
// (never the user's live bridge), so the panel can replay cached 2-D mid-plane
// slices. Two passes on one throwaway engine:
//   · LONGITUDINAL (Gauss constraint): a single static charge → its Coulomb
//     field |J| = |−∇φ| fills the box within a few warm-started ticks (a GLOBAL
//     elliptic solve — a constraint, carries no signal).
//   · TRANSVERSE (radiative): the flux-pulse → a |J| shell expanding at the
//     lattice light-speed c = 1/√3 voxel/tick.
//
// The capture runs CHUNKED across animation frames (no main-thread jank), stores
// Float32 L×L slices (~0.5 MB total at L=41), and DISPOSES the bridge immediately
// — only the lightweight slice cache survives. Every frame is real engine output.

import { WasmBridge } from '../../../../bridge/wasm-bridge.js';
import { transposeAndFlipNN } from './slice-render.js';

export const DEMO_L = 41;                       // odd (WasmBridge snaps even→odd); centre = 20
export const TICKS = 44;                        // covers the fit window + the front reaching the wall
export const CHUNK_TICKS = 2;                   // ticks per rAF batch — keeps the main thread responsive
export const C_LATTICE = 1 / Math.sqrt(3);      // 0.57735… — the lattice light-speed (CFL on the cubic lattice)

const SLICE_AXIS = 2;                           // XY plane (z fixed): getFluxSlice(2, mid)
const FILL_FLOOR = 1e-3;                         // |J| > FILL_FLOOR·max ⇒ "field present here"
const FIT_MIN = 5;                               // skip the initial blob→shell transient; fit the asymptotic regime

// Fit the shell slope only over ticks whose ideal front radius stays ≥2 voxels
// inside the wall (before reflection/dispersal contaminates the front).
export function fitMaxTick(L) {
    return Math.max(2, Math.floor(((L >> 1) - 2) / C_LATTICE));
}

// Yield to the event loop between chunks via a MessageChannel macrotask. NOT
// requestAnimationFrame: rAF is PAUSED for hidden/off-screen documents (so the
// capture would stall in a background tab or a headless test), whereas a
// MessageChannel post is a plain task that fires promptly regardless of visibility
// and is not subject to background-timer throttling.
const _yieldChannel = (typeof MessageChannel !== 'undefined') ? new MessageChannel() : null;
function yieldToEventLoop() {
    if (_yieldChannel) {
        return new Promise((resolve) => {
            _yieldChannel.port1.onmessage = () => resolve();
            _yieldChannel.port2.postMessage(0);
        });
    }
    return new Promise((resolve) => setTimeout(resolve, 0));
}

// Energy-weighted RMS radius √(Σ|J|²r² / Σ|J|²) of the mid-plane. Parameter-free,
// smooth, and ISOTROPIC (it averages over all directions and the whole distribution),
// so its growth rate is the isotropic propagation speed ≈ c — unlike a leading-edge
// radius, which tracks the FASTEST lattice direction and overshoots 1/√3, or a
// half-max ring, which jumps when the |J| peak migrates.
function rmsRadius(slice, N) {
    const c = (N - 1) / 2;
    let w = 0, wr2 = 0;
    for (let r = 0, i = 0; r < N; r++) for (let cc = 0; cc < N; cc++, i++) {
        const v = slice[i];
        if (v <= 0) continue;
        const e = v * v;                 // |J|² energy weight
        const dr = r - c, dc = cc - c;
        w += e; wr2 += e * (dr * dr + dc * dc);
    }
    return w > 0 ? Math.sqrt(wr2 / w) : 0;
}

// Fraction of the plane carrying non-negligible field (relative to the run's max):
// the "fill extent". For the Coulomb field this climbs to ~1 within a few ticks as
// the global solve relaxes outward — the visible "instant fill".
function fillExtent(slice, N, runMax) {
    if (runMax <= 0) return 0;
    const thr = FILL_FLOOR * runMax;
    let n = 0;
    for (let i = 0; i < slice.length; i++) if (slice[i] >= thr) n++;
    return n / (N * N);
}

// Least-squares slope of radius vs tick over the asymptotic regime t ∈ [FIT_MIN, fitMax].
// Expect ≈ 1/√3 (the isotropic lattice wave speed).
export function fitSlope(radius, fitMax) {
    let n = 0, st = 0, sr = 0, stt = 0, str = 0;
    const hi = Math.min(fitMax, radius.length - 1);
    for (let t = FIT_MIN; t <= hi; t++) { const r = radius[t]; n++; st += t; sr += r; stt += t * t; str += t * r; }
    if (n < 2) return { slope: 0, intercept: 0, r2: 0, n };
    const denom = n * stt - st * st;
    const slope = denom !== 0 ? (n * str - st * sr) / denom : 0;
    const intercept = (sr - slope * st) / n;
    let ssTot = 0, ssRes = 0; const mean = sr / n;
    for (let t = FIT_MIN; t <= hi; t++) {
        const r = radius[t], pred = slope * t + intercept;
        ssTot += (r - mean) * (r - mean); ssRes += (r - pred) * (r - pred);
    }
    return { slope, intercept, r2: ssTot > 0 ? 1 - ssRes / ssTot : 0, n };
}

// Run one sector on the capture bridge, returning { frames, metric, norm, N }.
async function capturePass(cap, kind, onProgress, isDisposed) {
    const N = cap.latticeSize;
    const c = N >> 1;

    if (kind === 'longitudinal') {
        cap.reset(N);                       // fresh lattice
        cap.injectParticle(c, c, c, +1);    // one static charge: zero flux/velocity → pure Coulomb source
    } else {
        cap.reset(N);
        // A SHARP flux pulse at the centre, released from rest (wave_vel = 0) → a clean,
        // thin spherical shell expanding at c. The small blob keeps the energy compact so
        // the RMS radius grows linearly at the isotropic wave speed once the shell forms.
        const sig = 1.5, R = 5;
        for (let dz = -R; dz <= R; dz++) for (let dy = -R; dy <= R; dy++) for (let dx = -R; dx <= R; dx++) {
            const v = Math.exp(-(dx * dx + dy * dy + dz * dz) / (2 * sig * sig));
            if (v > 1e-3) cap.injectFlux(c + dx, c + dy, c + dz, v, 0, 0);
        }
    }
    // No manifestation (keep the fields pure). Default damping is left ON — the
    // per-frame edge metric is robust to attenuation, and turning damping off trips
    // the engine's "selective_damping requires damping" guard.
    try { cap.setToggle('genesis', false); } catch { /* unknown name no-ops */ }
    if (kind === 'longitudinal') {
        // Gauss projection ON (default): the global elliptic solve IS what forms the
        // charge's Coulomb field and makes it fill the box — the constraint sector.
    } else {
        // Gauss projection OFF: measure the pure DYNAMICAL wave, which propagates at c.
        // The instantaneous, fills-everywhere behaviour belongs to the Gauss CONSTRAINT
        // (the longitudinal pass) — toggling gauss between the two passes IS the contrast.
        try { cap.setToggle('gauss_projection', false); } catch { /* */ }
    }

    const frames = new Array(TICKS);
    const grab = (t) => {
        const raw = cap.getFluxSlice(SLICE_AXIS, c);   // zero-copy static-cache view — copy THIS tick
        frames[t] = Float32Array.from(transposeAndFlipNN(raw, N));
    };

    grab(0);                                // t = 0 initial condition, before any tick
    for (let t = 1; t < TICKS; ) {
        if (isDisposed()) return null;
        for (let k = 0; k < CHUNK_TICKS && t < TICKS; k++, t++) { cap.tick(); grab(t); }
        onProgress(t / TICKS);
        await yieldToEventLoop();           // hand the main thread back between batches
    }

    // Second pass (cheap): run-max → norm, then the per-frame metric.
    let runMax = 0;
    for (let t = 0; t < TICKS; t++) { const f = frames[t]; for (let i = 0; i < f.length; i++) if (f[i] > runMax) runMax = f[i]; }
    const norm = runMax > 0 ? 1 / runMax : 1;
    const metric = new Float32Array(TICKS);
    for (let t = 0; t < TICKS; t++) {
        metric[t] = kind === 'transverse' ? rmsRadius(frames[t], N) : fillExtent(frames[t], N, runMax);
    }
    return { frames, metric, norm, N };
}

/**
 * Capture both sectors on a throwaway isolated engine and return the replay cache.
 * @param {(p:number)=>void} onProgress  0..1 across both passes
 * @param {()=>boolean} isDisposed       abort hook (panel disposed mid-capture)
 * @returns {Promise<object|null>} cache, or null if aborted; throws 'capture-init-failed'.
 */
export async function captureTwoSectors(onProgress = () => {}, isDisposed = () => false) {
    const now = () => (typeof performance !== 'undefined' ? performance.now() : 0);
    const t0 = now();
    const cap = new WasmBridge();
    const ok = await cap.init(DEMO_L);
    if (!ok || !cap.ready) { try { cap.dispose(); } catch { /* */ } throw new Error('capture-init-failed'); }
    try {
        const lon = await capturePass(cap, 'longitudinal', (p) => onProgress(p * 0.5), isDisposed);
        if (!lon) return null;
        const tra = await capturePass(cap, 'transverse', (p) => onProgress(0.5 + p * 0.5), isDisposed);
        if (!tra) return null;
        const L = cap.latticeSize;
        const fit = fitSlope(tra.metric, fitMaxTick(L));
        return {
            L, ticks: TICKS, fitMax: fitMaxTick(L), durationMs: Math.round(now() - t0),
            longitudinal: { frames: lon.frames, extent: lon.metric, norm: lon.norm },
            transverse: { frames: tra.frames, radius: tra.metric, norm: tra.norm },
            fit,
        };
    } finally {
        try { cap.dispose(); } catch { /* */ }   // free the ~1–2 MB engine immediately; cache survives
    }
}
