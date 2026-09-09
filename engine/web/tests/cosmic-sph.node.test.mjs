import test from 'node:test';
import assert from 'node:assert/strict';

import { SPH, kernelW, kernelGradMag, isGasType, computeSphForces } from '../js/bridge/cosmic-sph.js';
import { computeCosmicForces } from '../js/bridge/cosmic-physics.js';
import { referenceStep } from './cosmic-sph-reference.mjs';

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
