/**
 * Monaghan (1992) SPH gas solver — Scale 5 cosmic bridge.
 *
 * Kernel, density, and pressure/artificial-viscosity force math are
 * transcribed from engine/src/cosmic/cosmic_sph.cpp (sph_kernel_w /
 * sph_kernel_grad lines 19-47, compute_sph_density lines 74-104,
 * compute_sph_forces lines 107-167).
 *
 * The internal-energy (du) integration this module accumulates is an
 * ADDITION this JS port makes — cosmic_sph.cpp has no du/dt step of its
 * own (energy in the C++ engine is tracked elsewhere). It is declared,
 * not ported: du[i] += 0.5 * m_j * term * (v_ij . grad_W) per pair,
 * applied to internal_energy by cosmic-postupdates.js after the tick's
 * kick-drift-kick completes.
 *
 * Gated behind the `sph_monaghan` toggle (default off, see toggles.js /
 * mock-scale5.js). computeCosmicForces() in cosmic-physics.js calls
 * computeSphForces() after the gravity pass only when the toggle is on,
 * so the toggle-off path is untouched (bit-identical regression, per the
 * parity test in cosmic-sph.node.test.mjs).
 *
 * Parity: engine/web/tests/cosmic-sph-reference.mjs is an INDEPENDENT
 * reimplementation of the same formulas (not importing this file) used to
 * cross-check this module in engine/web/tests/cosmic-sph.node.test.mjs.
 */

export const SPH = Object.freeze({
    GAMMA: 5 / 3,
    ETA: 1.2,
    ALPHA: 1.0,
    BETA: 2.0,
    EPS2_FACTOR: 0.01,
});

/**
 * 3D cubic spline kernel (Monaghan 1992).
 * W(r,h) = (1/(pi h^3)) * { 1 - 1.5 q^2 + 0.75 q^3   q = r/h < 1
 *                          { 0.25 (2-q)^3              1 <= q < 2
 *                          { 0                          q >= 2
 */
export function kernelW(r, h) {
    const q = r / h;
    const norm = 1 / (Math.PI * h * h * h);
    if (q < 1) return norm * (1 - 1.5 * q * q + 0.75 * q * q * q);
    if (q < 2) { const t = 2 - q; return norm * 0.25 * t * t * t; }
    return 0;
}

/**
 * dW/dr of the cubic spline kernel (radial derivative magnitude). The
 * caller divides by r and multiplies by the separation vector to get the
 * vector gradient (matches CosmicEngine::sph_kernel_grad).
 */
export function kernelGradMag(r, h) {
    const q = r / h;
    const norm = 1 / (Math.PI * h * h * h * h);
    if (q < 1) return norm * (-3 * q + 2.25 * q * q);
    if (q < 2) { const t = 2 - q; return norm * (-0.75 * t * t); }
    return 0;
}

/** Gas-like body types for SPH purposes: GAS and NEBULA. */
export function isGasType(type, TYPE) {
    return type === TYPE.GAS || type === TYPE.NEBULA;
}

/**
 * Run one Monaghan SPH gas pass against `bridge._bodies`.
 *
 * Order (matches the independent reference exactly, see module header):
 *   1. Collect gas bodies (GAS + NEBULA), init `h` for any that lack it.
 *   2. Build the neighbour list once (O(N^2), cutoff 2*max(h_i, h_j)),
 *      reused for both density and forces.
 *   3. Density (self-term included) -> pressure (ideal gas EOS) -> sound
 *      speed, all evaluated with the smoothing length AS OF ENTRY (the
 *      "old" h) — mirrors cosmic-sph-reference.mjs, which computes forces
 *      before ever consulting the adaptive h it derives.
 *   4. Pressure + Monaghan-Gingold artificial-viscosity accelerations,
 *      and the du/dt energy exchange, accumulated on BOTH bodies of each
 *      pair (Newton-3) using that same old h.
 *   5. Adaptive smoothing length h = ETA * cbrt(m/rho) is written back
 *      AFTER the force/energy pass, so it takes effect starting next
 *      tick's neighbour search/density pass, not this one.
 *
 * Mutates b.density, b.pressure, b.sound, b.h, and adds onto b.ax/ay/az
 * and b.du. Caller (cosmic-physics.js) must run the gravity pass first —
 * this function only ADDS to ax/ay/az.
 *
 * @param {{_bodies: object[]}} bridge
 * @param {object} TYPE CosmicMockBridge.TYPE enum
 * @returns {{gasCount: number, pairs: number}}
 */
export function computeSphForces(bridge, TYPE) {
    const bodies = bridge._bodies;
    const n = bodies.length;

    const gasIdx = [];
    for (let i = 0; i < n; i++) {
        if (isGasType(bodies[i].type, TYPE)) gasIdx.push(i);
    }
    const nGas = gasIdx.length;
    if (nGas === 0) return { gasCount: 0, pairs: 0 };

    // Fallback smoothing-length init for bodies that bypassed addBody's
    // own initializer (e.g. hand-built fixtures).
    for (let a = 0; a < nGas; a++) {
        const b = bodies[gasIdx[a]];
        if (b.h == null) {
            const r = b.radius != null ? b.radius : Math.cbrt(b.mass) * 0.1;
            b.h = r * 2;
        }
    }

    // Neighbour pass — built once, reused for density and forces.
    const neighbors = new Array(nGas);
    for (let a = 0; a < nGas; a++) neighbors[a] = [];
    const pairA = [], pairB = [], pairR = [];
    for (let a = 0; a < nGas; a++) {
        const bi = bodies[gasIdx[a]];
        for (let bx = a + 1; bx < nGas; bx++) {
            const bj = bodies[gasIdx[bx]];
            const dx = bi.x - bj.x, dy = bi.y - bj.y, dz = bi.z - bj.z;
            const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (r < 2 * Math.max(bi.h, bj.h)) {
                neighbors[a].push(bx);
                neighbors[bx].push(a);
                pairA.push(a); pairB.push(bx); pairR.push(r);
            }
        }
    }
    const nPairs = pairA.length;

    // Density (self-term) -> pressure -> sound speed, using the OLD h.
    const rho = new Float64Array(nGas);
    const P = new Float64Array(nGas);
    const c = new Float64Array(nGas);
    const hOld = new Float64Array(nGas);
    for (let a = 0; a < nGas; a++) {
        const bi = bodies[gasIdx[a]];
        hOld[a] = bi.h;
        let r = bi.mass * kernelW(0, bi.h);
        for (const bx of neighbors[a]) {
            const bj = bodies[gasIdx[bx]];
            const dx = bi.x - bj.x, dy = bi.y - bj.y, dz = bi.z - bj.z;
            const d = Math.sqrt(dx * dx + dy * dy + dz * dz);
            r += bj.mass * kernelW(d, bi.h);
        }
        rho[a] = r;
        const press = (SPH.GAMMA - 1) * r * bi.internal_energy;
        P[a] = press;
        c[a] = r > 0 ? Math.sqrt(SPH.GAMMA * press / r) : 0;
        bi.density = r;
        bi.pressure = press;
        bi.sound = c[a];
        bi.du = 0;
    }

    // Pressure + Monaghan-Gingold artificial-viscosity forces, and the
    // du/dt energy equation. Both use the cached OLD h (hOld), not b.h —
    // the adaptive-h write below happens after this loop.
    for (let p = 0; p < nPairs; p++) {
        const a = pairA[p], bx = pairB[p], r = pairR[p];
        if (r < 1e-10) continue;
        const bi = bodies[gasIdx[a]], bj = bodies[gasIdx[bx]];
        const hAvg = 0.5 * (hOld[a] + hOld[bx]);
        const rx = bi.x - bj.x, ry = bi.y - bj.y, rz = bi.z - bj.z;
        const r2 = rx * rx + ry * ry + rz * rz;
        const g = kernelGradMag(r, hAvg) / r;
        const gx = g * rx, gy = g * ry, gz = g * rz;

        const vx = bi.vx - bj.vx, vy = bi.vy - bj.vy, vz = bi.vz - bj.vz;
        const vdotr = vx * rx + vy * ry + vz * rz;
        let pi = 0;
        if (vdotr < 0) {
            const mu = hAvg * vdotr / (r2 + SPH.EPS2_FACTOR * hAvg * hAvg);
            pi = (-SPH.ALPHA * 0.5 * (c[a] + c[bx]) * mu + SPH.BETA * mu * mu) / (0.5 * (rho[a] + rho[bx]));
        }
        const term = P[a] / (rho[a] * rho[a]) + P[bx] / (rho[bx] * rho[bx]) + pi;

        // Newton-3: force on `a` is -m_b * term * grad_W; force on `bx` is
        // +m_a * term * grad_W (same gx/gy/gz, not recomputed with the
        // separation flipped — see module header / reference derivation).
        bi.ax -= bj.mass * term * gx; bi.ay -= bj.mass * term * gy; bi.az -= bj.mass * term * gz;
        bj.ax += bi.mass * term * gx; bj.ay += bi.mass * term * gy; bj.az += bi.mass * term * gz;

        const dot = vx * gx + vy * gy + vz * gz;
        bi.du += 0.5 * bj.mass * term * dot;
        bj.du += 0.5 * bi.mass * term * dot;
    }

    // Adaptive smoothing length, applied AFTER the force/energy pass so it
    // takes effect starting next tick.
    for (let a = 0; a < nGas; a++) {
        if (rho[a] > 0) bodies[gasIdx[a]].h = SPH.ETA * Math.cbrt(bodies[gasIdx[a]].mass / rho[a]);
    }

    return { gasCount: nGas, pairs: nPairs };
}
