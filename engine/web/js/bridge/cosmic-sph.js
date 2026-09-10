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
 * The smoothing length used in the force pass below is the value the
 * density pass just wrote (h updated in place, `if (rho > 0)`), matching
 * cosmic_sph.cpp: compute_sph_density() overwrites smoothing_length before
 * compute_sph_forces() runs in the same tick (cosmic_engine.cpp:394-395).
 * Only the neighbour-list membership test still uses the entry-tick h,
 * matching find_sph_neighbors() running before the density pass.
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
 *
 * Pass 0a (bridge foundations): the density loop below also accumulates
 * neighbour-count / h / rho / P min-mean-max statistics purely from
 * values it already computes (no new O(N^2) work), and the return value
 * is widened to publish them (CosmicMockBridge._stats reads this — see
 * cosmic-pass-stats.js). Two live knobs are read here with `?? default`
 * fallbacks so the toggle-off / no-override path stays bit-identical:
 * `bridge._sphAlpha`/`bridge._sphBeta` (CosmicMockBridge.setSphAlpha/
 * setSphBeta) override the frozen SPH.ALPHA/SPH.BETA constants, and
 * `bridge._adaptiveSmoothing` (setAdaptiveSmoothing) gates the adaptive-h
 * write-back — absent/undefined means "on" in both cases, so a bare
 * object-literal test fixture with no such fields behaves exactly as
 * before this pass.
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
 * Order (matches CosmicEngine::compute_sph_density / compute_sph_forces
 * tick order in cosmic_sph.cpp, and the independent reference):
 *   1. Collect gas bodies (GAS + NEBULA), init `h` for any that lack it.
 *   2. Build the neighbour list once (O(N^2), cutoff 2*max(h_i, h_j)) using
 *      the smoothing length AS OF ENTRY (the "old" h) — matches
 *      find_sph_neighbors(), which runs before the density pass. Reused
 *      for both density and forces below.
 *   3. Density (self-term included) -> pressure (ideal gas EOS) -> sound
 *      speed, evaluated with the entry-tick h (each body's own density
 *      only ever reads its OWN h — kernelW(d, bi.h) — never a neighbour's,
 *      so a single left-to-right pass is safe regardless of iteration
 *      order). Immediately after, when rho > 0, the adaptive smoothing
 *      length h = ETA * cbrt(m/rho) is written back to the body IN PLACE
 *      — matches cosmic_sph.cpp:99-102, which updates smoothing_length
 *      before compute_sph_forces ever runs this tick. Pass 0a gates this
 *      write-back on `bridge._adaptiveSmoothing !== false` (absent means
 *      on) so a live "freeze h" knob can suppress it without a new code
 *      path.
 *   4. Pressure + Monaghan-Gingold artificial-viscosity accelerations, and
 *      the du/dt energy exchange, accumulated on BOTH bodies of each pair
 *      (Newton-3), reading the smoothing length step 3 just updated — the
 *      C++ force pass reads bodies_[i].smoothing_length AFTER
 *      compute_sph_density has already overwritten it this same tick, so
 *      this port now matches that tick order (a correction from an
 *      earlier revision that lagged h by one tick). Density guards mirror
 *      cosmic_sph.cpp:115 (rho_i <= 0 -> body i receives nothing) and :134
 *      (the pressure term is added only when rho_j > 0); the viscosity
 *      term is additionally zeroed when the pair's rho_avg <= 0
 *      (defensive — only reachable when both sides are already
 *      zero-density, so it changes nothing that wasn't already excluded
 *      by the per-side guards below). Pass 0a reads the viscosity
 *      coefficients as `bridge._sphAlpha ?? SPH.ALPHA` / `bridge._sphBeta
 *      ?? SPH.BETA` so a live override never touches the frozen SPH
 *      object (`cosmic-sph.node.test.mjs:127-133` pins its values).
 *
 * Mutates b.density, b.pressure, b.sound, b.h, and adds onto b.ax/ay/az
 * and b.du. Caller (cosmic-physics.js) must run the gravity pass first —
 * this function only ADDS to ax/ay/az.
 *
 * @param {{_bodies: object[]}} bridge
 * @param {object} TYPE CosmicMockBridge.TYPE enum
 * @returns {{gasCount: number, pairs: number, neighborMin: number,
 *   neighborMean: number, neighborMax: number, hMin: number,
 *   hMean: number, hMax: number, rhoMin: number, rhoMax: number,
 *   pMin: number, pMax: number}}
 */
export function computeSphForces(bridge, TYPE) {
    const bodies = bridge._bodies;
    const n = bodies.length;

    const gasIdx = [];
    for (let i = 0; i < n; i++) {
        if (isGasType(bodies[i].type, TYPE)) gasIdx.push(i);
    }
    const nGas = gasIdx.length;
    if (nGas === 0) {
        return {
            gasCount: 0, pairs: 0,
            neighborMin: 0, neighborMean: 0, neighborMax: 0,
            hMin: 0, hMean: 0, hMax: 0,
            rhoMin: 0, rhoMax: 0, pMin: 0, pMax: 0,
        };
    }

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

    // Density (self-term) -> pressure -> sound speed, using the entry-tick
    // h; then (when rho > 0) the adaptive h = ETA*cbrt(m/rho) is written
    // back to the body IMMEDIATELY, so the force pass below reads it.
    // Safe in a single left-to-right pass: a body's own density only ever
    // reads ITS OWN h (kernelW(d, bi.h)), never a neighbour's.
    const adaptiveSmoothing = bridge._adaptiveSmoothing !== false;
    const rho = new Float64Array(nGas);
    const P = new Float64Array(nGas);
    const c = new Float64Array(nGas);
    let neighborMin = Infinity, neighborMax = -Infinity, neighborSum = 0;
    let hMin = Infinity, hMax = -Infinity, hSum = 0;
    let rhoMin = Infinity, rhoMax = -Infinity;
    let pMin = Infinity, pMax = -Infinity;
    for (let a = 0; a < nGas; a++) {
        const bi = bodies[gasIdx[a]];
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
        // Adaptive smoothing length, written back in place right here (as
        // in cosmic_sph.cpp's compute_sph_density) so the force pass below
        // reads the post-density h, not the entry-tick one. Pass 0a: a
        // live "freeze h" override (bridge._adaptiveSmoothing === false)
        // skips this write-back; absent/undefined leaves it on.
        if (r > 0 && adaptiveSmoothing) bi.h = SPH.ETA * Math.cbrt(bi.mass / r);

        // Pass 0a stats — every value below was already computed above.
        const nCount = neighbors[a].length;
        if (nCount < neighborMin) neighborMin = nCount;
        if (nCount > neighborMax) neighborMax = nCount;
        neighborSum += nCount;
        if (bi.h < hMin) hMin = bi.h;
        if (bi.h > hMax) hMax = bi.h;
        hSum += bi.h;
        if (r < rhoMin) rhoMin = r;
        if (r > rhoMax) rhoMax = r;
        if (press < pMin) pMin = press;
        if (press > pMax) pMax = press;
    }
    const neighborMean = neighborSum / nGas;
    const hMean = hSum / nGas;
    if (!Number.isFinite(neighborMin)) neighborMin = 0;
    if (!Number.isFinite(neighborMax)) neighborMax = 0;
    if (!Number.isFinite(hMin)) hMin = 0;
    if (!Number.isFinite(hMax)) hMax = 0;
    if (!Number.isFinite(rhoMin)) rhoMin = 0;
    if (!Number.isFinite(rhoMax)) rhoMax = 0;
    if (!Number.isFinite(pMin)) pMin = 0;
    if (!Number.isFinite(pMax)) pMax = 0;

    // Pressure + Monaghan-Gingold artificial-viscosity forces, and the
    // du/dt energy equation. Both read the CURRENT b.h (post-density-
    // update), matching cosmic_sph.cpp's compute_sph_forces reading the
    // just-recomputed smoothing_length.
    //
    // Density guards mirror cosmic_sph.cpp:115/:134: pressTerm requires
    // BOTH sides' density positive; each side's term (pressure+viscosity)
    // is zeroed when THAT side's own density is non-positive, so a
    // zero/negative-density body contributes and receives nothing (no
    // 0/0 division), while the other side of the pair still gets a finite
    // (possibly viscosity-only) contribution.
    const sphAlpha = bridge._sphAlpha ?? SPH.ALPHA;
    const sphBeta = bridge._sphBeta ?? SPH.BETA;
    for (let p = 0; p < nPairs; p++) {
        const a = pairA[p], bx = pairB[p], r = pairR[p];
        if (r < 1e-10) continue;
        const bi = bodies[gasIdx[a]], bj = bodies[gasIdx[bx]];
        const hAvg = 0.5 * (bi.h + bj.h);
        const rx = bi.x - bj.x, ry = bi.y - bj.y, rz = bi.z - bj.z;
        const r2 = rx * rx + ry * ry + rz * rz;
        const g = kernelGradMag(r, hAvg) / r;
        const gx = g * rx, gy = g * ry, gz = g * rz;

        const vx = bi.vx - bj.vx, vy = bi.vy - bj.vy, vz = bi.vz - bj.vz;
        const vdotr = vx * rx + vy * ry + vz * rz;
        const rhoA = rho[a], rhoB = rho[bx];
        const rhoAvg = 0.5 * (rhoA + rhoB);
        let pi = 0;
        if (vdotr < 0 && rhoAvg > 0) {
            const mu = hAvg * vdotr / (r2 + SPH.EPS2_FACTOR * hAvg * hAvg);
            pi = (-sphAlpha * 0.5 * (c[a] + c[bx]) * mu + sphBeta * mu * mu) / rhoAvg;
        }
        const pressTerm = (rhoA > 0 && rhoB > 0) ? P[a] / (rhoA * rhoA) + P[bx] / (rhoB * rhoB) : 0;
        const termA = rhoA > 0 ? pressTerm + pi : 0;
        const termB = rhoB > 0 ? pressTerm + pi : 0;

        // Newton-3: force on `a` uses termA; force on `bx` uses termB
        // (same gx/gy/gz, not recomputed with the separation flipped —
        // see module header / reference derivation). The two terms differ
        // only when one side's density is non-positive.
        bi.ax -= bj.mass * termA * gx; bi.ay -= bj.mass * termA * gy; bi.az -= bj.mass * termA * gz;
        bj.ax += bi.mass * termB * gx; bj.ay += bi.mass * termB * gy; bj.az += bi.mass * termB * gz;

        const dot = vx * gx + vy * gy + vz * gz;
        bi.du += 0.5 * bj.mass * termA * dot;
        bj.du += 0.5 * bi.mass * termB * dot;
    }

    return {
        gasCount: nGas, pairs: nPairs,
        neighborMin, neighborMean, neighborMax,
        hMin, hMean, hMax,
        rhoMin, rhoMax, pMin, pMax,
    };
}
