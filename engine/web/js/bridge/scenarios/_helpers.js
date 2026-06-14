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
 * Minimal harness surface for scenario setup when no PhysicsHarness is
 * passed (legacy `.call(mockBridge, name, ctx)` path). Matches
 * PhysicsHarness.injectParticle opts handling.
 *
 * @param {object} bridge - MockBridge instance
 */
export function createScenarioHarness(bridge) {
    return {
        bridge,
        setToggle: (key, value) => bridge.setToggle?.(key, value),
        setLangevinParams: (T, gamma) => {
            if (typeof bridge.setLangevinParams === 'function') bridge.setLangevinParams(T, gamma);
        },
        setLangevinTemp: (t) => bridge.setLangevinTemp?.(t),
        setOmega0: (w) => bridge.setOmega0?.(w),
        injectUniformFluxAdd: (fx, fy, fz) => bridge.injectUniformFluxAdd?.(fx, fy, fz),
        initFluxGrid: () => bridge._initFluxGrid?.(),
        injectFlux: (x, y, z, fx, fy, fz) => bridge._injectFlux?.(x, y, z, fx, fy, fz),
        injectWaveVel: (x, y, z, vx, vy, vz) => bridge._injectWaveVel?.(x, y, z, vx, vy, vz),
        injectParticle: (x, y, z, state, opts = {}) => {
            const before = bridge._particles?.length ?? 0;
            bridge.injectParticle?.(x, y, z, state);
            const after = bridge._particles?.length ?? 0;
            if (after > before && opts) {
                const last = bridge._particles[after - 1];
                if (last) {
                    if (Number.isFinite(opts.spin)) last.spin = opts.spin;
                    if (Number.isFinite(opts.color)) last.color = opts.color;
                    if (typeof opts.locked === 'boolean') last.locked = opts.locked;
                    if (Number.isFinite(opts.density)) last.density = opts.density;
                    if (Number.isFinite(opts.vx)) last.vx = opts.vx;
                    if (Number.isFinite(opts.vy)) last.vy = opts.vy;
                    if (Number.isFinite(opts.vz)) last.vz = opts.vz;
                }
            }
            return after > before ? bridge._particles[after - 1] : null;
        },
    };
}

/**
 * Inject a Gaussian radial flux envelope centred at (cx, cy, cz).
 * Center may be integer or floating-point (for half-voxel-centred
 * envelopes — set `opts.minR2 = 0.25` to skip the singular core).
 *
 * @param {PhysicsHarness} harness    PhysicsHarness instance
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
export function injectRadialEnvelope(harness, cx, cy, cz, sign, sigma, amp, opts = {}) {
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
        harness.injectFlux(x, y, z,
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
export function injectParticleFull(harness, cx, cy, cz, state, attrs = {}) {
    return harness.injectParticle?.(cx, cy, cz, state, attrs) ?? null;
}

/**
 * Locked particle on the y-z plane at fixed x (barrier, eraser wires).
 * @param {object} [opts]
 * @param {'even'|null} [opts.parity] — `'even'` keeps only (y+z) % 2 === 0
 */
export function injectLockedYZPlane(harness, x, N, opts = {}) {
    const state = opts.state ?? 1;
    const attrs = opts.attrs ?? { locked: true };
    for (let y = 0; y < N; y++) {
        for (let z = 0; z < N; z++) {
            if (opts.parity === 'even' && (y + z) % 2 !== 0) continue;
            injectParticleFull(harness, x, y, z, state, attrs);
        }
    }
}

/** Locked barrier wall in the y-z plane spanning `width` voxels in +x. */
export function injectLockedBarrierWall(harness, x0, N, width, state = 1) {
    for (let y = 0; y < N; y++)
    for (let z = 0; z < N; z++)
    for (let dx = 0; dx < width; dx++) {
        injectParticleFull(harness, x0 + dx, y, z, state, { locked: true });
    }
}

/**
 * Two coherent Gaussian line sources (double-slit geometry), propagating +x.
 * @param {function} [opts.emit] — `(px, py, z, g) => void` per voxel
 */
export function injectCoherentSlitPair(harness, ctx, opts = {}) {
    const { N, mid, vox, sigma } = ctx;
    const slitSigma = opts.slitSigma ?? sigma(2);
    const slitHw = opts.slitHw ?? vox(4);
    const sAmp = opts.sAmp ?? 0.3;
    const slitSep = opts.slitSep ?? vox(5);
    const slitX = opts.slitX ?? vox(8);
    const slitYs = opts.slitYs ?? [mid - slitSep, mid + slitSep];
    const emit = opts.emit ?? ((px, py, z, g) => {
        harness.injectFlux(px, py, z, 0, 0, g);
        harness.injectWaveVel(px, py, z, g, 0, 0);
    });
    for (const sy of slitYs) {
        for (let z = 0; z < N; z++)
        for (let dy = -slitHw; dy <= slitHw; dy++)
        for (let dx = -slitHw; dx <= slitHw; dx++) {
            const r2 = dx * dx + dy * dy;
            const g = sAmp * Math.exp(-r2 / (2 * slitSigma * slitSigma));
            if (g < 1e-6) continue;
            const px = slitX + dx, py = sy + dy;
            if (px < 0 || px >= N || py < 0 || py >= N) continue;
            emit(px, py, z, g);
        }
    }
}

/**
 * Dressed particle: inject + radial Gaussian envelope, with envelope
 * sign tracking the particle state (positive → outward, negative →
 * inward). Mirrors the C++ `dp(...)` helper used by `s0_seed.cpp`.
 */
export function injectDressedParticle(harness, cx, cy, cz, state, spin, color, sigma, amp, locked = false) {
    injectParticleFull(harness, cx, cy, cz, state, { spin, color, locked });
    const sign = state > 0 ? 1 : -1;
    injectRadialEnvelope(harness, cx, cy, cz, sign, sigma, amp);
}

/**
 * Three-vertex equilateral triad at xy-plane angles `TRIAD_ANGLES`,
 * z=cz. Each vertex carries a dressed particle with the supplied
 * charge + color and alternating spin (+1, -1, +1). Mirrors the C++
 * `tri(...)` helper.
 */
export function injectTriad(harness, cx, cy, cz, charges, colors, rad, locked = true, dressSigma = 2) {
    for (let k = 0; k < 3; k++) {
        const ang = TRIAD_ANGLES[k];
        const qx = Math.round(cx + rad * Math.cos(ang));
        const qy = Math.round(cy + rad * Math.sin(ang));
        injectDressedParticle(harness, qx, qy, cz, charges[k],
            (k % 2 === 0) ? 1 : -1, colors[k], dressSigma, K_B * 0.5, locked);
    }
}

/**
 * Apply the vacuum environment that every s0-vacuum-* scenario needs:
 * - This.reset() is already invoked by the dispatcher in index.js, so
 *   the lattice arrives flux-zero.
 * - Particle list is already empty for the same reason.
 *
 * v1: this is effectively a no-op confirming the dispatcher contract.
 * The function exists as the single point that a future
 * `absorbing_boundary` toggle (separate spec) would mutate, so all 15
 * vacuum scenarios pick up the new behavior with one edit.
 *
 * @param {PhysicsHarness} harness      PhysicsHarness instance
 * @param {{N:number, mid:number, midF:number}} ctx  precomputed lattice params
 */
export function applyVacuumEnvironment(harness, ctx) {
    // No-op in v1. Reserved extension point — see SPEC_VACUUM_PARTICLE_SCENARIOS.md.
    // Reads ctx + harness to make the dependency explicit (and silence linters
    // when this becomes non-trivial in v2).
    void harness;
    void ctx;
}
