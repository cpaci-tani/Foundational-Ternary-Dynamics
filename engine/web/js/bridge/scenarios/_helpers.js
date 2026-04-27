/**
 * Shared scenario primitives — JS mirror of `engine/src/scenarios/_helpers.h`.
 *
 * Every Scale-0 scenario file (`flux-`, `light-`, `quantum-`, `s0-seed-`,
 * `s0-field-`) imports from here so radial envelopes, particle attribute
 * application, triad placement, and the canonical `[0, 2π/3, 4π/3]`
 * angle set live in exactly one place.
 *
 * Bridge contract: callers pass a `MockBridge` instance. The helpers use
 * the additive `_injectFlux` channel and the existing `injectParticle`
 * + `_particles[last]` post-mutation pattern; semantics match the
 * historical inline `_dp` / `_tri` definitions previously embedded in
 * `s0-seed-scenarios.js`.
 */

import { K_B } from '../../constants.js';

/** Equilateral-triangle vertex angles in the xy plane (N_c = 3). */
export const TRIAD_ANGLES = Object.freeze([0, 2 * Math.PI / 3, 4 * Math.PI / 3]);

/**
 * Inject a Gaussian radial flux envelope centred at (cx, cy, cz).
 * Center may be integer or floating-point (for half-voxel-centred
 * envelopes — set `opts.minR2 = 0.25` to skip the singular core).
 *
 * @param {object} bridge      MockBridge instance with `_injectFlux`
 * @param {number} cx,cy,cz    centre (continuous OK)
 * @param {number} sign        +1 outward, -1 inward
 * @param {number} sigma       Gaussian sigma
 * @param {number} amp         peak amplitude
 * @param {object} [opts]
 * @param {number} [opts.radius]    cutoff in voxels (default ceil(3·sigma))
 * @param {number} [opts.minR2]     skip voxels with r² ≤ this (default 0)
 * @param {number} [opts.minVal]    drop samples below this magnitude (default 0.001)
 * @param {number[]} [opts.axisBias] per-axis multipliers [bx, by, bz] (default [1,1,1])
 */
export function injectRadialEnvelope(bridge, cx, cy, cz, sign, sigma, amp, opts = {}) {
    const radius = opts.radius ?? Math.ceil(3 * sigma);
    const radius2 = radius * radius;
    const minR2 = opts.minR2 ?? 0;
    const minVal = opts.minVal ?? 0.001;
    const bias = opts.axisBias ?? null;
    const bx = bias ? bias[0] : 1;
    const by = bias ? bias[1] : 1;
    const bz = bias ? bias[2] : 1;
    const sigma2 = 2 * sigma * sigma;
    const xLo = Math.floor(cx - radius), xHi = Math.ceil(cx + radius);
    const yLo = Math.floor(cy - radius), yHi = Math.ceil(cy + radius);
    const zLo = Math.floor(cz - radius), zHi = Math.ceil(cz + radius);
    for (let z = zLo; z <= zHi; z++)
    for (let y = yLo; y <= yHi; y++)
    for (let x = xLo; x <= xHi; x++) {
        const dx = x - cx, dy = y - cy, dz = z - cz;
        const r2 = dx * dx + dy * dy + dz * dz;
        if (r2 <= minR2 || r2 > radius2) continue;
        const r = Math.sqrt(r2);
        const val = amp * Math.exp(-r2 / sigma2);
        if (val < minVal) continue;
        bridge._injectFlux(x, y, z,
            sign * val * bx * dx / r,
            sign * val * by * dy / r,
            sign * val * bz * dz / r);
    }
}

/**
 * Inject a manifested particle and apply spin/color/locked attributes
 * to the just-injected entry. Mirrors the C++ `IPF` macro. Returns
 * the post-mutation particle reference (or null if injection failed).
 */
export function injectParticleFull(bridge, cx, cy, cz, state, attrs = {}) {
    bridge.injectParticle(cx, cy, cz, state);
    const list = bridge._particles;
    const last = list ? list[list.length - 1] : null;
    if (!last) return null;
    if (Number.isFinite(attrs.color) && attrs.color >= 0) last.color = attrs.color;
    if (Number.isFinite(attrs.spin)) last.spin = attrs.spin;
    if (attrs.locked) last.locked = true;
    if (Number.isFinite(attrs.density)) last.density = attrs.density;
    return last;
}

/**
 * Dressed particle: inject + radial Gaussian envelope, with envelope
 * sign tracking the particle state (positive → outward, negative →
 * inward). Mirrors the C++ `dp(...)` helper used by `s0_seed.cpp`.
 */
export function injectDressedParticle(bridge, cx, cy, cz, state, spin, color, sigma, amp, locked = false) {
    injectParticleFull(bridge, cx, cy, cz, state, { spin, color, locked });
    const sign = state > 0 ? 1 : -1;
    injectRadialEnvelope(bridge, cx, cy, cz, sign, sigma, amp);
}

/**
 * Three-vertex equilateral triad at xy-plane angles `TRIAD_ANGLES`,
 * z=cz. Each vertex carries a dressed particle with the supplied
 * charge + color and alternating spin (+1, -1, +1). Mirrors the C++
 * `tri(...)` helper.
 */
export function injectTriad(bridge, cx, cy, cz, charges, colors, rad, locked = true) {
    for (let k = 0; k < 3; k++) {
        const ang = TRIAD_ANGLES[k];
        const qx = Math.round(cx + rad * Math.cos(ang));
        const qy = Math.round(cy + rad * Math.sin(ang));
        injectDressedParticle(bridge, qx, qy, cz, charges[k],
            (k % 2 === 0) ? 1 : -1, colors[k], 2, K_B * 0.5, locked);
    }
}
