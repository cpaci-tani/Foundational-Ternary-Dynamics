/**
 * Shared scenario primitives — JS mirror of `engine/src/scenarios/_helpers.h`.
 *
 * Every Scale-0 scenario file (`flux-`, `light-`, `quantum-`, `s0-seed-`,
 * `s0-field-`) imports from here so radial envelopes, particle attribute
 * application, triad placement, and the canonical `[0, 2π/3, 4π/3]`
 * angle set live in exactly one place.
 *
 * Bridge contract: callers pass a scenario harness (createScenarioHarness /
 * PhysicsHarness) over the JS parity mirror. Live dashboard seeds go through
 * C++ WASM; this mirror is for parity CI and reference. Helpers use the
 * additive `_injectFlux` channel and `injectParticle` + post-mutation pattern.
 */

import { C_SPEED, K_B } from '../../constants.js';

/** Equilateral-triangle vertex angles in the xy plane (N_c = 3). */
export const TRIAD_ANGLES = Object.freeze([0, 2 * Math.PI / 3, 4 * Math.PI / 3]);
const FACE_OFFSETS = Object.freeze([[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]]);
const EDGE_OFFSETS = Object.freeze([
    [1,1,0],[1,-1,0],[-1,1,0],[-1,-1,0],
    [1,0,1],[1,0,-1],[-1,0,1],[-1,0,-1],
    [0,1,1],[0,1,-1],[0,-1,1],[0,-1,-1],
]);

// JS mirror of C++ configure_free_wave_terms().  This list intentionally
// includes research-only tick extensions that the dashboard's ordinary reset
// preserves. A scenario claiming an isolated native wave map must not inherit
// any of them from a prior research run.
export const FREE_WAVE_DISABLED_TERMS = Object.freeze([
    'coupling', 'damping', 'genesis', 'evaporation', 'forces', 'gravity',
    'poisson_coulomb', 'movement', 'lorentz_force', 'selective_damping',
    'larmor_radiation', 'dual_substrate', 'color_forces',
    'strong_stress_energy', 'weak_transmutation', 'strong_force',
    'triad_binding', 'pair_production', 'exchange_force', 'latency_field',
    'exact_dual_gauss', 'matched_gauss_dynamics', 'emergent_forces',
    'langevin', 'symplectic_leapfrog', 'verlet_wave_integrator',
    'lorentz_period2_floquet', 'lorentz_bcc_time_floquet', 'su2_gauge',
    'su3_gauge', 'symmetric_movement_order', 'absorbing_boundary',
    'reflective_boundary', 'field_energy_gravity', 'cluster_inertia', 'geometric_gravity',
    'de_broglie_clock', 'db_clock_coulomb', 'knot_tracking', 'confinement',
    'strict_validation', 'ew_background_sweep',
]);

export function configureFreeWaveTerms(harness, gauss = true) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', true);
    harness.setToggle('gauss_projection', !!gauss);
}

export function configureGenesisGateTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('genesis', true);
    harness.setToggle('dual_substrate', false);
}

export function configurePairProductionTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('pair_production', true);
    harness.setToggle('dual_substrate', false);
}

/** Isolated remainder/integer transport with every force and reaction off. */
export function configureFreeMovementTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('movement', true);
    harness.setToggle('dual_substrate', false);
}

export function configureAnnihilationTerms(harness) {
    configureFreeMovementTerms(harness);
}

/** Static dressing + selected legacy field/color forces + native movement. */
export function configureUnlockedCompositeTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.bridge?.setFluxBoundaryMode?.(0); // Periodic
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('forces', true);
    harness.setToggle('movement', true);
    harness.setToggle('color_forces', true);
    harness.setToggle('dual_substrate', false);
}

/** Locked nuclear markers + Poisson-Coulomb force + mobile outer markers. */
export function configurePreparedCoulombCandidateTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.bridge?.setFluxBoundaryMode?.(0); // Periodic
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('forces', true);
    harness.setToggle('poisson_coulomb', true);
    harness.setToggle('movement', true);
    harness.setToggle('dual_substrate', false);
}

/** Additive nonnegative uniform drive + selected wave/Gauss/genesis stack. */
export function configureUniformGenesisDriveTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.bridge?.setFluxBoundaryMode?.(0); // Periodic
    harness.setToggle('wave_propagation', true);
    harness.setToggle('gauss_projection', true);
    harness.setToggle('genesis', true);
    harness.setToggle('ew_background_sweep', true);
    harness.setToggle('dual_substrate', false);
}

/** Prepared cohort observed only through the selected weak polarity flip. */
export function configureWeakTransmutationProbeTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('dual_substrate', true);
    harness.setToggle('weak_transmutation', true);
    // Mirror C++ B1: damping bounds the dual-substrate stress probe so the
    // seeded neutrino packet cannot ballistic-grow |flux| without a ceiling.
    harness.setToggle('damping', true);
}

/** Fixed-seed Langevin vector bath plus free marker transport. */
export function configureThermalTransportTerms(harness, temperature, gamma) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.bridge?.setFluxBoundaryMode?.(0); // Periodic
    harness.setToggle('wave_propagation', true);
    harness.setToggle('gauss_projection', true);
    harness.setToggle('movement', true);
    harness.setToggle('langevin', true);
    harness.setToggle('dual_substrate', false);
    harness.setLangevinParams?.(temperature, gamma);
}

/** Prepared geometry under the selected coupled/damped genesis response. */
export function configurePatternedGenesisResponseTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.bridge?.setFluxBoundaryMode?.(0); // Periodic field boundary
    harness.setToggle('wave_propagation', true);
    harness.setToggle('coupling', true);
    harness.setToggle('damping', true);
    harness.setToggle('genesis', true);
    harness.setToggle('gauss_projection', true);
    harness.setToggle('forces', true);
    harness.setToggle('movement', true);
    harness.setToggle('dual_substrate', false);
}

/** Exact ternary geometry only: no production dynamics may mutate the seed. */
export function configureStaticSeedTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('dual_substrate', false);
}

/** Isolated wave + Gauss + selected genesis response with an optional bath. */
export function configureGenesisClusterTerms(harness, temperature, gamma = 0.02) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', true);
    harness.setToggle('gauss_projection', true);
    harness.setToggle('genesis', true);
    harness.setToggle('langevin', true);
    harness.setToggle('dual_substrate', false);
    harness.setLangevinParams?.(temperature, gamma);
}

/** Locked rest-mass source observed only through the native latency solver. */
export function configureMassLatencyTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('gravity', true); // latency dependency; force master remains off
    harness.setToggle('latency_field', true);
    harness.setToggle('field_energy_gravity', false);
    harness.setToggle('dual_substrate', false);
}

/** Locked source plus linear coupled-wave sector; no recoil or force path. */
export function configureLockedCoupledFieldTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', true);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('coupling', true);
    harness.setToggle('dual_substrate', false);
}

/** Linear field plus native flux-gradient force and production movement. */
export function configureEmergentRecoilTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.bridge?.setFluxBoundaryMode?.(0); // Periodic
    harness.bridge?.setDt?.(1.0);
    harness.setToggle('wave_propagation', true);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('coupling', true);
    harness.setToggle('forces', true);
    harness.setToggle('movement', true);
    harness.setToggle('emergent_forces', true);
    harness.setToggle('dual_substrate', false);
}

/** Fixed imposed-B field plus native Lorentz force and movement only. */
export function configureLorentzOrbitTerms(harness) {
    for (const key of FREE_WAVE_DISABLED_TERMS) harness.setToggle(key, false);
    harness.bridge?.setFluxBoundaryMode?.(0); // Periodic
    harness.bridge?.setDt?.(1.0);
    harness.setToggle('wave_propagation', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('forces', true);
    harness.setToggle('poisson_coulomb', true);
    harness.setToggle('movement', true);
    harness.setToggle('lorentz_force', true);
    harness.setToggle('dual_substrate', false);
}

/**
 * Divergence-free localized packet traveling along ±x.
 * J is the centered-difference curl of psi*e_x and wave_vel satisfies the
 * one-way relation W = -direction*C_SPEED*D_x J.
 */
export function injectTransversePacketX(harness, ctx, opts = {}) {
    const { N, mid } = ctx;
    const x0 = opts.x0 ?? mid;
    const y0 = opts.y0 ?? mid;
    const z0 = opts.z0 ?? mid;
    const sx = Math.max(1, opts.sigmaX ?? 3);
    const st = Math.max(1, opts.sigmaT ?? 3);
    const amp = opts.amp ?? K_B;
    const direction = (opts.direction ?? 1) >= 0 ? 1 : -1;
    const k = opts.carrierK ?? 0;
    const phase = opts.carrierPhase ?? 0;
    const psiAmp = amp * st;
    const periodicDelta = (a, b) => {
        let d = a - b;
        while (d > N / 2) d -= N;
        while (d < -N / 2) d += N;
        return d;
    };
    const psi = (x, y, z) => {
        const dx = periodicDelta(x, x0);
        const dy = periodicDelta(y, y0);
        const dz = periodicDelta(z, z0);
        const r2 = dx*dx/(sx*sx) + (dy*dy + dz*dz)/(st*st);
        if (r2 > 18) return 0;
        return psiAmp * Math.exp(-0.5 * r2) * Math.cos(k * dx + phase);
    };
    const field = (x, y, z) => [
        0,
        0.5 * (psi(x, y, z + 1) - psi(x, y, z - 1)),
       -0.5 * (psi(x, y + 1, z) - psi(x, y - 1, z)),
    ];
    for (let z = 0; z < N; z++)
    for (let y = 0; y < N; y++)
    for (let x = 0; x < N; x++) {
        const j = field(x, y, z);
        const jp = field(x + 1, y, z);
        const jm = field(x - 1, y, z);
        const fs = [0, 0, 0], es = [0, 0, 0];
        for (const o of FACE_OFFSETS) { const v = field(x+o[0], y+o[1], z+o[2]); fs[1] += v[1]; fs[2] += v[2]; }
        for (const o of EDGE_OFFSETS) { const v = field(x+o[0], y+o[1], z+o[2]); es[1] += v[1]; es[2] += v[2]; }
        const lap = [0, fs[1]/3 + es[1]/6 - 4*j[1], fs[2]/3 + es[2]/6 - 4*j[2]];
        const w = [0,
            -0.5 * direction * C_SPEED * (jp[1] - jm[1]) - 0.5*C_SPEED*C_SPEED*lap[1],
            -0.5 * direction * C_SPEED * (jp[2] - jm[2]) - 0.5*C_SPEED*C_SPEED*lap[2]];
        if (j[1]*j[1] + j[2]*j[2] > 1e-20) harness.injectFlux(x, y, z, 0, j[1], j[2]);
        if (w[1]*w[1] + w[2]*w[2] > 1e-20) harness.injectWaveVel(x, y, z, 0, w[1], w[2]);
    }
}

/** J_z sheet packet, finite in x/y and uniform in z, traveling along ±x. */
export function injectSheetPacketX(harness, ctx, opts = {}) {
    const { N, mid } = ctx;
    const x0 = opts.x0 ?? mid;
    const y0 = opts.y0 ?? mid;
    const sx = Math.max(1, opts.sigmaX ?? 3);
    const sy = Math.max(1, opts.sigmaY ?? 3);
    const amp = opts.amp ?? K_B;
    const direction = (opts.direction ?? 1) >= 0 ? 1 : -1;
    const polarizationAxis = opts.polarizationAxis === 1 ? 1 : 2;
    const carrierK = opts.carrierK ?? 0;
    const carrierPhase = opts.carrierPhase ?? 0;
    const scalar = (x, u) => {
        const dx = x - x0, dy = u - y0;
        const r2 = dx*dx/(sx*sx) + dy*dy/(sy*sy);
        return r2 <= 18
            ? amp * Math.exp(-0.5 * r2) * Math.cos(carrierK * dx + carrierPhase)
            : 0;
    };
    const xlo = Math.max(0, Math.floor(x0 - 4.5 * sx));
    const xhi = Math.min(N - 1, Math.ceil(x0 + 4.5 * sx));
    const ulo = Math.max(0, Math.floor(y0 - 4.5 * sy));
    const uhi = Math.min(N - 1, Math.ceil(y0 + 4.5 * sy));
    for (let v = 0; v < N; v++)
    for (let u = ulo; u <= uhi; u++)
    for (let x = xlo; x <= xhi; x++) {
        const j = scalar(x, u);
        const faceSum = scalar(x+1,u) + scalar(x-1,u) + scalar(x,u+1) + scalar(x,u-1) + 2*j;
        const edgeSum = scalar(x+1,u+1) + scalar(x+1,u-1) + scalar(x-1,u+1) + scalar(x-1,u-1)
            + 2*(scalar(x+1,u) + scalar(x-1,u) + scalar(x,u+1) + scalar(x,u-1));
        const lap = faceSum/3 + edgeSum/6 - 4*j;
        const w = -0.5 * direction * C_SPEED * (scalar(x + 1, u) - scalar(x - 1, u))
            - 0.5*C_SPEED*C_SPEED*lap;
        const y = polarizationAxis === 1 ? v : u;
        const z = polarizationAxis === 1 ? u : v;
        if (Math.abs(j) > 1e-12) {
            if (polarizationAxis === 1) harness.injectFlux(x, y, z, 0, j, 0);
            else                        harness.injectFlux(x, y, z, 0, 0, j);
        }
        if (Math.abs(w) > 1e-12) {
            if (polarizationAxis === 1) harness.injectWaveVel(x, y, z, 0, w, 0);
            else                        harness.injectWaveVel(x, y, z, 0, 0, w);
        }
    }
}

/** Exactly transverse 1D J_z=f(x) pulse, uniform on every yz plane. */
export function injectPlanePacketX(harness, ctx, opts = {}) {
    const { N, mid } = ctx;
    const x0 = opts.x0 ?? mid;
    const sx = Math.max(1, opts.sigmaX ?? 3);
    const amp = opts.amp ?? K_B;
    const direction = (opts.direction ?? 1) >= 0 ? 1 : -1;
    const k = opts.carrierK ?? 0;
    const scalar = (x) => {
        const dx = x - x0;
        if (Math.abs(dx) > 4.5*sx) return 0;
        return amp * Math.exp(-0.5*dx*dx/(sx*sx)) * Math.cos(k*dx);
    };
    for (let x = 0; x < N; x++) {
        const jz = scalar(x);
        const lap = scalar(x+1) + scalar(x-1) - 2*jz;
        const wz = -0.5*direction*C_SPEED*(scalar(x+1)-scalar(x-1))
            - 0.5*C_SPEED*C_SPEED*lap;
        for (let y = 0; y < N; y++)
        for (let z = 0; z < N; z++) {
            if (Math.abs(jz) > 1e-12) harness.injectFlux(x,y,z,0,0,jz);
            if (Math.abs(wz) > 1e-12) harness.injectWaveVel(x,y,z,0,0,wz);
        }
    }
}

/** Exact pole of the production kick-drift map for a yz-uniform x harmonic. */
export function latticeHarmonicOmega(k) {
    return 2 * Math.asin(C_SPEED * Math.abs(Math.sin(k / 2)));
}

/** Exact traveling Jz harmonic: Jz(x,t)=A sin(kx-direction*omega*t). */
export function injectPlaneHarmonicX(harness, ctx, opts = {}) {
    const { N } = ctx;
    const modeN = opts.modeN ?? 4;
    const amp = opts.amp ?? K_B * 2;
    const direction = (opts.direction ?? 1) >= 0 ? 1 : -1;
    const k = 2 * Math.PI * modeN / N;
    const omega = latticeHarmonicOmega(k);
    for (let x = 0; x < N; x++) {
        const phase = k * x;
        const jz = amp * Math.sin(phase);
        const wz = amp * ((1 - Math.cos(omega)) * Math.sin(phase)
                        - direction * Math.sin(omega) * Math.cos(phase));
        for (let y = 0; y < N; y++)
        for (let z = 0; z < N; z++) {
            if (Math.abs(jz) > 1e-12) harness.injectFlux(x, y, z, 0, 0, jz);
            if (Math.abs(wz) > 1e-12) harness.injectWaveVel(x, y, z, 0, 0, wz);
        }
    }
}

/** Exact standing Jz harmonic: Jz(x,t)=A sin(kx) cos(omega*t). */
export function injectStandingHarmonicX(harness, ctx, opts = {}) {
    const { N } = ctx;
    const modeN = opts.modeN ?? 4;
    const amp = opts.amp ?? K_B * 2;
    const k = 2 * Math.PI * modeN / N;
    const omega = latticeHarmonicOmega(k);
    for (let x = 0; x < N; x++) {
        const jz = amp * Math.sin(k * x);
        const wz = (1 - Math.cos(omega)) * jz;
        for (let y = 0; y < N; y++)
        for (let z = 0; z < N; z++) {
            if (Math.abs(jz) > 1e-12) harness.injectFlux(x, y, z, 0, 0, jz);
            if (Math.abs(wz) > 1e-12) harness.injectWaveVel(x, y, z, 0, 0, wz);
        }
    }
}

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
    const carrierK = opts.carrierK ?? 2 * Math.PI / 8;
    const carrierPhase = opts.carrierPhase ?? 0;
    const emit = opts.emit;
    if (!emit) {
        for (const sy of slitYs) {
            injectSheetPacketX(harness, ctx, {
                x0: slitX, y0: sy, sigmaX: Math.max(4, slitSigma), sigmaY: slitSigma,
                amp: sAmp, direction: +1, carrierK, carrierPhase,
            });
        }
        return;
    }
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
