import test from 'node:test';
import assert from 'node:assert/strict';

import { SPH, kernelW, kernelGradMag, isGasType, computeSphForces } from '../js/bridge/cosmic-sph.js';
import { computeCosmicForces } from '../js/bridge/cosmic-physics.js';
import { CosmicMockBridge } from '../js/bridge/mock-scale5.js';
import { SCALE5_TOGGLES } from '../js/config/toggles.js';
import { referenceStep, W, dW, GAMMA, ALPHA, BETA } from './cosmic-sph-reference.mjs';

// Minimal TYPE enum mirroring CosmicMockBridge.TYPE (mock-scale5.js) — kept
// local so this test does not need to construct a full CosmicMockBridge.
const TYPE = Object.freeze({
    DARK_ENERGY: -3, QUASAR: -2, BLACK_HOLE: -1,
    DARK_MATTER: 0, GAS: 1, STAR: 2,
    NEUTRON_STAR: 3, NEBULA: 4, WHITE_DWARF: 5,
});

// Deterministic Lehmer LCG, same recurrence as CosmicMockBridge._rng.
function makeRng(seed) {
    let s = seed;
    return () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
}

/**
 * 50-body gas fixture: positions in a 20-unit cube, velocities in [-0.5, 0.5],
 * masses in [1, 3], internal_energy in [0.5, 2]. `h` is forced uniformly to
 * 2.0 (rather than the addBody default of radius*2, which would be far too
 * small at these masses) so the O(N^2) neighbour search actually finds pairs
 * inside a 20-unit box.
 */
function buildGasFixture(n = 50, seed = 12345) {
    const rng = makeRng(seed);
    const bodies = [];
    for (let i = 0; i < n; i++) {
        const mass = 1 + rng() * 2;
        bodies.push({
            id: i, type: TYPE.GAS, mass,
            x: (rng() - 0.5) * 20, y: (rng() - 0.5) * 20, z: (rng() - 0.5) * 20,
            vx: (rng() - 0.5), vy: (rng() - 0.5), vz: (rng() - 0.5),
            ax: 0, ay: 0, az: 0,
            internal_energy: 0.5 + rng() * 1.5,
            density: 0, pressure: 0, sound: 0, du: 0,
            radius: Math.cbrt(mass) * 0.1,
            h: 2.0,
        });
    }
    return bodies;
}

function buildStarFixture(n = 20, seed = 999) {
    const rng = makeRng(seed);
    const bodies = [];
    for (let i = 0; i < n; i++) {
        bodies.push({
            id: i, type: TYPE.STAR, mass: 1 + rng() * 5,
            x: (rng() - 0.5) * 40, y: (rng() - 0.5) * 40, z: (rng() - 0.5) * 40,
            vx: 0, vy: 0, vz: 0, ax: 0, ay: 0, az: 0,
            temperature: 5800, internal_energy: 0.01,
            density: 0, pressure: 0, sound: 0, du: 0,
            luminosity: 0, radius: 1, h: 2.0,
        });
    }
    return bodies;
}

function toReferenceBodies(bodies) {
    return bodies.map((b) => ({
        x: b.x, y: b.y, z: b.z, vx: b.vx, vy: b.vy, vz: b.vz,
        mass: b.mass, u: b.internal_energy, h: b.h,
    }));
}

function assertRelClose(actual, expected, label) {
    const diff = Math.abs(actual - expected);
    const scale = Math.max(Math.abs(expected), 1e-12);
    assert.ok(
        diff < 1e-12 || diff / scale < 1e-9,
        `${label}: actual=${actual} expected=${expected} diff=${diff}`,
    );
}

/**
 * Pre-fix reference: identical to `referenceStep` except the force loop's
 * `hAvg` uses the ENTRY-tick h (bi.h/bj.h, never mutated by this function)
 * instead of the density-pass-updated `hNew`. Used only to prove that the
 * h-timing fix in cosmic-sph.js actually changes the force output relative
 * to the old (pre-fix) one-tick-lagged behavior — see the h-timing
 * regression test below. Density guards are kept identical to referenceStep
 * so this isolates ONLY the h-timing difference.
 */
function referenceStepEntryH(bodies) {
    const n = bodies.length, rho = new Float64Array(n), P = new Float64Array(n), c = new Float64Array(n);
    for (let i = 0; i < n; i++) {
        const bi = bodies[i]; let r = bi.mass * W(0, bi.h);
        for (let j = 0; j < n; j++) if (j !== i) { const bj = bodies[j]; const d = Math.hypot(bi.x - bj.x, bi.y - bj.y, bi.z - bj.z); if (d < 2 * Math.max(bi.h, bj.h)) r += bj.mass * W(d, bi.h); }
        rho[i] = r; P[i] = (GAMMA - 1) * r * bi.u; c[i] = r > 0 ? Math.sqrt(GAMMA * P[i] / r) : 0;
    }
    const ax = new Float64Array(n), ay = new Float64Array(n), az = new Float64Array(n);
    for (let i = 0; i < n; i++) {
        if (rho[i] <= 0) continue;
        for (let j = 0; j < n; j++) {
            if (j === i) continue; const bi = bodies[i], bj = bodies[j];
            const rx = bi.x - bj.x, ry = bi.y - bj.y, rz = bi.z - bj.z, r2 = rx * rx + ry * ry + rz * rz, r = Math.sqrt(r2);
            if (r >= 2 * Math.max(bi.h, bj.h) || r < 1e-10) continue;
            const hAvg = 0.5 * (bi.h + bj.h); // ENTRY h -- the pre-fix (buggy) behavior
            const g = dW(r, hAvg) / r, gx = g * rx, gy = g * ry, gz = g * rz;
            const vx = bi.vx - bj.vx, vy = bi.vy - bj.vy, vz = bi.vz - bj.vz, vdotr = vx * rx + vy * ry + vz * rz;
            let pi = 0;
            if (vdotr < 0) { const mu = hAvg * vdotr / (r2 + 0.01 * hAvg * hAvg); pi = (-ALPHA * 0.5 * (c[i] + c[j]) * mu + BETA * mu * mu) / (0.5 * (rho[i] + rho[j])); }
            const pressTerm = rho[j] > 0 ? P[i] / (rho[i] * rho[i]) + P[j] / (rho[j] * rho[j]) : 0;
            const term = pressTerm + pi;
            ax[i] -= bj.mass * term * gx; ay[i] -= bj.mass * term * gy; az[i] -= bj.mass * term * gz;
        }
    }
    return { ax, ay, az };
}

/** Every own-enumerable numeric field on `obj` is a finite number (no NaN/Infinity). */
function assertAllFinite(obj, label) {
    for (const [key, value] of Object.entries(obj)) {
        if (typeof value !== 'number') continue;
        assert.ok(Number.isFinite(value), `${label}.${key} = ${value} is not finite`);
    }
}

// ── Kernel / helper sanity ──────────────────────────────────────────────

test('SPH constants match the Monaghan (1992) / cosmic_sph.cpp values', () => {
    assert.equal(SPH.GAMMA, 5 / 3);
    assert.equal(SPH.ETA, 1.2);
    assert.equal(SPH.ALPHA, 1.0);
    assert.equal(SPH.BETA, 2.0);
    assert.equal(SPH.EPS2_FACTOR, 0.01);
});

test('kernelW / kernelGradMag vanish beyond q=2 and match the reference at q=0', () => {
    assert.equal(kernelW(3, 1), 0);
    assert.equal(kernelGradMag(3, 1), 0);
    assert.equal(kernelW(0, 1), 1 / Math.PI);
});

test('isGasType is true only for GAS and NEBULA', () => {
    assert.equal(isGasType(TYPE.GAS, TYPE), true);
    assert.equal(isGasType(TYPE.NEBULA, TYPE), true);
    assert.equal(isGasType(TYPE.STAR, TYPE), false);
    assert.equal(isGasType(TYPE.DARK_MATTER, TYPE), false);
    assert.equal(isGasType(TYPE.BLACK_HOLE, TYPE), false);
});

// ── Parity against the independent reference ────────────────────────────

test('computeSphForces matches the independent reference within 1e-9 relative', () => {
    const bodies = buildGasFixture(50);
    const bridge = { _bodies: bodies.map((b) => ({ ...b })), _toggles: { sph_monaghan: true }, _softening: 1 };

    const result = computeSphForces(bridge, TYPE);
    assert.equal(result.gasCount, 50);
    assert.ok(result.pairs > 0, 'fixture must produce at least one neighbour pair for the test to be meaningful');

    const ref = referenceStep(toReferenceBodies(bodies));

    for (let i = 0; i < 50; i++) {
        const b = bridge._bodies[i];
        assertRelClose(b.density, ref.rho[i], `density[${i}]`);
        assertRelClose(b.pressure, ref.P[i], `pressure[${i}]`);
        assertRelClose(b.sound, ref.c[i], `sound[${i}]`);
        assertRelClose(b.h, ref.hNew[i], `h[${i}]`);
        assertRelClose(b.ax, ref.ax[i], `ax[${i}]`);
        assertRelClose(b.ay, ref.ay[i], `ay[${i}]`);
        assertRelClose(b.az, ref.az[i], `az[${i}]`);
        assertRelClose(b.du, ref.du[i], `du[${i}]`);
    }
});

// ── Physical invariants ──────────────────────────────────────────────────

test("Newton's third law: sum(m*a) is zero to 1e-9 (relative to the force scale)", () => {
    const bodies = buildGasFixture(50);
    const bridge = { _bodies: bodies, _toggles: { sph_monaghan: true }, _softening: 1 };
    computeSphForces(bridge, TYPE);

    let sx = 0, sy = 0, sz = 0, scale = 0;
    for (const b of bridge._bodies) {
        sx += b.mass * b.ax; sy += b.mass * b.ay; sz += b.mass * b.az;
        scale += b.mass * Math.hypot(b.ax, b.ay, b.az);
    }
    const norm = Math.max(scale, 1e-12);
    assert.ok(Math.abs(sx) / norm < 1e-9, `sum(m*ax) = ${sx}`);
    assert.ok(Math.abs(sy) / norm < 1e-9, `sum(m*ay) = ${sy}`);
    assert.ok(Math.abs(sz) / norm < 1e-9, `sum(m*az) = ${sz}`);
});

test('energy exchange consistency: sum(m*du) = -sum(m * v . a) to 1e-9', () => {
    const bodies = buildGasFixture(50);
    const bridge = { _bodies: bodies, _toggles: { sph_monaghan: true }, _softening: 1 };
    computeSphForces(bridge, TYPE);

    let workRate = 0, duRate = 0, scale = 0;
    for (const b of bridge._bodies) {
        const w = b.mass * (b.vx * b.ax + b.vy * b.ay + b.vz * b.az);
        workRate += w;
        duRate += b.mass * b.du;
        scale += Math.abs(w) + Math.abs(b.mass * b.du);
    }
    const norm = Math.max(scale, 1e-12);
    assert.ok(
        Math.abs(workRate + duRate) / norm < 1e-9,
        `sum(m*v.a)=${workRate} sum(m*du)=${duRate} (should sum to ~0)`,
    );
});

// ── Regression guard: toggle-off path is untouched ───────────────────────

test('regression guard: sph_monaghan is a no-op on a non-gas fixture (toggle off vs on identical)', () => {
    const bridgeOff = { _bodies: buildStarFixture(20), _toggles: { sph_monaghan: false }, _softening: 5, _enableSubgrid: false };
    computeCosmicForces.call(bridgeOff, TYPE);

    const bridgeOn = { _bodies: buildStarFixture(20), _toggles: { sph_monaghan: true }, _softening: 5, _enableSubgrid: false };
    computeCosmicForces.call(bridgeOn, TYPE);

    for (let i = 0; i < 20; i++) {
        assert.equal(bridgeOn._bodies[i].ax, bridgeOff._bodies[i].ax, `ax[${i}] differs with the toggle on a non-gas fixture`);
        assert.equal(bridgeOn._bodies[i].ay, bridgeOff._bodies[i].ay, `ay[${i}] differs with the toggle on a non-gas fixture`);
        assert.equal(bridgeOn._bodies[i].az, bridgeOff._bodies[i].az, `az[${i}] differs with the toggle on a non-gas fixture`);
    }
});

test('regression guard: sph_monaghan absent from _toggles does not throw (optional-chaining fallback)', () => {
    const bridge = { _bodies: buildStarFixture(5), _softening: 5, _enableSubgrid: false };
    assert.doesNotThrow(() => computeCosmicForces.call(bridge, TYPE));
});

// ── h-timing: forces must read the density-updated h, not the entry h ────

test('h-timing regression: forces use the density-updated h (matches cosmic_sph.cpp tick order)', () => {
    const bodies = buildGasFixture(50);
    const bridge = { _bodies: bodies.map((b) => ({ ...b })), _toggles: { sph_monaghan: true }, _softening: 1 };
    computeSphForces(bridge, TYPE);

    const refBodies = toReferenceBodies(bodies);
    const refNew = referenceStep(refBodies);        // post-density h (current, correct)
    const refOld = referenceStepEntryH(refBodies);   // entry-tick h (pre-fix, wrong)

    // h after the pass must equal ETA*cbrt(m/rho) -- confirms the adaptive
    // smoothing length was actually written back during the density pass.
    for (let i = 0; i < 50; i++) {
        assertRelClose(bridge._bodies[i].h, refNew.hNew[i], `h[${i}]`);
    }

    // The module's accelerations must match the post-density-h reference...
    let matchesNew = true;
    let matchesOld = true;
    for (let i = 0; i < 50; i++) {
        const b = bridge._bodies[i];
        const scaleNew = Math.max(Math.abs(refNew.ax[i]), Math.abs(refNew.ay[i]), Math.abs(refNew.az[i]), 1e-12);
        if (Math.abs(b.ax - refNew.ax[i]) / scaleNew > 1e-9 ||
            Math.abs(b.ay - refNew.ay[i]) / scaleNew > 1e-9 ||
            Math.abs(b.az - refNew.az[i]) / scaleNew > 1e-9) matchesNew = false;

        const scaleOld = Math.max(Math.abs(refOld.ax[i]), Math.abs(refOld.ay[i]), Math.abs(refOld.az[i]), 1e-12);
        if (Math.abs(b.ax - refOld.ax[i]) / scaleOld > 1e-6 ||
            Math.abs(b.ay - refOld.ay[i]) / scaleOld > 1e-6 ||
            Math.abs(b.az - refOld.az[i]) / scaleOld > 1e-6) matchesOld = false;
    }
    assert.ok(matchesNew, 'module accelerations should match the density-updated-h (post-fix) reference');
    // ...and must clearly diverge from the entry-h (pre-fix) reference -- this
    // is what would fail if someone reverted the h-timing fix.
    assert.ok(!matchesOld, 'module accelerations should diverge from the entry-h (pre-fix) reference');
});

// ── Zero-mass gas body: density-positivity guards must prevent NaN ───────

/** A tight cluster of normal GAS bodies plus one zero-mass GAS body, all
 * mutually within each other's SPH neighbour cutoff (addBody sets
 * h = cbrt(mass)*0.2; the zero-mass body gets h = 0 from cbrt(0)). */
function buildZeroMassGasCluster() {
    const bridge = new CosmicMockBridge();
    bridge.setToggle('sph_monaghan', true);
    const T = CosmicMockBridge.TYPE;
    bridge.addBody(T.GAS, 8, 0.00, 0.00, 0.00, 0, 0, 0, 500);
    bridge.addBody(T.GAS, 8, 0.30, 0.00, 0.00, 0, 0, 0, 500);
    bridge.addBody(T.GAS, 8, 0.00, 0.30, 0.00, 0, 0, 0, 500);
    bridge.addBody(T.GAS, 8, 0.30, 0.30, 0.00, 0, 0, 0, 500);
    bridge.addBody(T.GAS, 8, 0.15, 0.15, 0.30, 0, 0, 0, 500);
    bridge.addBody(T.GAS, 0, 0.15, 0.15, 0.15, 0, 0, 0, 0); // zero-mass gas body
    return bridge;
}

test('zero-mass gas body: computeSphForces leaves every body finite (density/pressure/sound/h/ax/ay/az/du)', () => {
    const bridge = buildZeroMassGasCluster();
    const T = CosmicMockBridge.TYPE;
    const result = computeSphForces(bridge, T);
    assert.equal(result.gasCount, 6);
    assert.ok(result.pairs > 0, 'the cluster must produce neighbour pairs for this test to be meaningful');

    for (let i = 0; i < bridge._bodies.length; i++) {
        const b = bridge._bodies[i];
        assertAllFinite(
            { density: b.density, pressure: b.pressure, sound: b.sound, h: b.h, ax: b.ax, ay: b.ay, az: b.az, du: b.du },
            `body[${i}] (mass=${b.mass})`,
        );
    }
    // The zero-mass body itself: mass*kernelW(0,h=0) self-term is 0 and its
    // own h stays 0 (rho<=0 skips the adaptive-h write) -- both finite, not NaN.
    const zeroBody = bridge._bodies[5];
    assert.equal(zeroBody.mass, 0);
    assert.equal(zeroBody.h, 0);
});

test('zero-mass gas body: one full tick (force pass + cosmic-postupdates) leaves internal_energy finite and drops the zero-mass body', () => {
    const bridge = buildZeroMassGasCluster();
    bridge.setDt(0.01);
    assert.equal(bridge._bodies.length, 6);

    bridge.tick();

    // The mass<=0.01 cleanup filter at the end of postCosmicUpdates removes
    // the zero-mass body -- confirms the tick actually ran to completion
    // rather than stalling on a NaN somewhere upstream.
    assert.equal(bridge._bodies.length, 5, 'the zero-mass body should be filtered out after the tick');
    for (const b of bridge._bodies) {
        assert.ok(Number.isFinite(b.internal_energy), `internal_energy = ${b.internal_energy} is not finite`);
        assert.ok(Number.isFinite(b.temperature), `temperature = ${b.temperature} is not finite`);
        assert.ok(Number.isFinite(b.ax) && Number.isFinite(b.ay) && Number.isFinite(b.az), 'acceleration is not finite');
    }
});

// ── Legacy ad-hoc gas repulsion is suppressed exactly when sph_monaghan is on ──

/** Two GAS bodies 1 unit apart, well inside the legacy h_press = softening*2.5
 * range at softening=5 (h_press=12.5), so the ad-hoc repulsion block in
 * cosmic-physics.js would fire for this pair when the toggle is off. */
function buildTwoGasBodies() {
    const mk = (id, x) => ({
        id, type: TYPE.GAS, mass: 2, x, y: 0, z: 0, vx: 0, vy: 0, vz: 0, ax: 0, ay: 0, az: 0,
        internal_energy: 1, temperature: 500, density: 0, pressure: 0, sound: 0, du: 0, h: 2.0,
    });
    return [mk(0, 0), mk(1, 1)];
}

test('legacy-repulsion suppression: sph_monaghan off applies the ad-hoc gas repulsion (no SPH fields); on runs SPH instead (no legacy repulsion)', () => {
    const softening = 5;

    // Baseline: gravity only (no subgrid at all), to isolate the legacy
    // repulsion's own contribution to ax below.
    const bridgeGravOnly = { _bodies: buildTwoGasBodies(), _toggles: { sph_monaghan: false }, _softening: softening, _enableSubgrid: false };
    computeCosmicForces.call(bridgeGravOnly, TYPE);

    // Toggle OFF, subgrid ON: gravity + legacy ad-hoc gas repulsion.
    const bridgeOff = { _bodies: buildTwoGasBodies(), _toggles: { sph_monaghan: false }, _softening: softening, _enableSubgrid: true };
    computeCosmicForces.call(bridgeOff, TYPE);

    // Toggle ON, subgrid ON: gravity + Monaghan SPH (legacy repulsion skipped).
    const bridgeOn = { _bodies: buildTwoGasBodies(), _toggles: { sph_monaghan: true }, _softening: softening, _enableSubgrid: true };
    computeCosmicForces.call(bridgeOn, TYPE);

    // OFF path: no SPH fields written.
    assert.equal(bridgeOff._bodies[0].density, 0, 'toggle off: SPH must not run, density stays 0');
    assert.equal(bridgeOff._bodies[1].density, 0, 'toggle off: SPH must not run, density stays 0');

    // OFF path: legacy repulsion contributes a nonzero push, repulsive along
    // the pair axis (body0 at x=0 pushed toward -x, away from body1 at x=1).
    const legacyAx0 = bridgeOff._bodies[0].ax - bridgeGravOnly._bodies[0].ax;
    const legacyAx1 = bridgeOff._bodies[1].ax - bridgeGravOnly._bodies[1].ax;
    assert.ok(legacyAx0 < 0, `legacy repulsion on body 0 should push it toward -x, got delta ax = ${legacyAx0}`);
    assert.ok(legacyAx1 > 0, `legacy repulsion on body 1 should push it toward +x, got delta ax = ${legacyAx1}`);
    assert.ok(Math.abs(legacyAx0) > 1e-12, 'legacy repulsion must be a genuinely nonzero contribution');

    // ON path: SPH ran instead (density > 0, no legacy repulsion contribution).
    assert.ok(bridgeOn._bodies[0].density > 0, 'toggle on: SPH must run, density > 0');
    assert.ok(bridgeOn._bodies[1].density > 0, 'toggle on: SPH must run, density > 0');
    assert.notEqual(bridgeOn._bodies[0].ax, bridgeOff._bodies[0].ax, 'toggle on vs off must produce different physics for this pair');
});

// ── CosmicMockBridge toggle + diagnostics API (mock-scale5.js) ───────────

test('CosmicMockBridge.setToggle validates the key, getToggle reads it back, and getDiagnostics().toggles is a copy', () => {
    const bridge = new CosmicMockBridge();
    assert.throws(() => bridge.setToggle('not_a_real_toggle', true), /unknown toggle/);

    assert.equal(bridge.getToggle('sph_monaghan'), false);
    bridge.setToggle('sph_monaghan', true);
    assert.equal(bridge.getToggle('sph_monaghan'), true);

    const diag = bridge.getDiagnostics();
    // Pass 0a: _toggles grew from 1 key to 14 (see toggles.js /
    // mock-scale5.js _freshToggles()), so a deepEqual against a
    // single-key literal would break the moment a second key is added.
    // This property-wise form is strictly stronger: it still pins
    // sph_monaghan's value AND asserts every registered toggle is
    // actually published as a boolean.
    assert.equal(diag.toggles.sph_monaghan, true);
    for (const [key] of SCALE5_TOGGLES) {
        assert.equal(typeof diag.toggles[key], 'boolean', `toggle ${key} must be published`);
    }
    diag.toggles.sph_monaghan = false; // mutate the returned object
    assert.equal(bridge.getToggle('sph_monaghan'), true, 'mutating the diagnostics.toggles copy must not affect the bridge');
    assert.equal(bridge.getDiagnostics().toggles.sph_monaghan, true, 'a fresh diagnostics call must reflect the real state, not the earlier mutated copy');
});

test('CosmicMockBridge.getDiagnostics().totalThermal sums mass*internal_energy over GAS/NEBULA bodies only', () => {
    const bridge = new CosmicMockBridge();
    const T = CosmicMockBridge.TYPE;
    bridge.addBody(T.GAS, 2, 0, 0, 0, 0, 0, 0, 1000);          // internal_energy = max(1000*0.001, 0.01) = 1.0
    bridge.addBody(T.NEBULA, 3, 5, 5, 5, 0, 0, 0, 2000);       // internal_energy = 2.0
    bridge.addBody(T.STAR, 10, -5, -5, -5, 0, 0, 0, 5800);     // not counted
    bridge.addBody(T.DARK_MATTER, 100, 0, 0, 100, 0, 0, 0, 0); // not counted

    const diag = bridge.getDiagnostics();
    const expected = 2 * 1.0 + 3 * 2.0;
    assert.ok(Math.abs(diag.totalThermal - expected) < 1e-12, `totalThermal=${diag.totalThermal} expected=${expected}`);
});

test('CosmicMockBridge.setupScenario() resets _toggles.sph_monaghan to false on the next scenario (M9)', () => {
    // Safe today only because scale5/controller.js builds a fresh bridge per
    // scenario load; a caller that reuses a bridge (as this test does)
    // would otherwise carry sph_monaghan: true into a non-gas scenario and
    // silently change its physics.
    const bridge = new CosmicMockBridge();
    bridge.setupScenario('cosmic-gas-collapse');
    assert.equal(bridge.getToggle('sph_monaghan'), true, 'the gas lab scenario turns sph_monaghan on');

    bridge.setupScenario('cosmic-galaxy');
    assert.equal(bridge.getToggle('sph_monaghan'), false, 'setupScenario on the SAME bridge must reset the toggle for a non-gas scenario');
});
