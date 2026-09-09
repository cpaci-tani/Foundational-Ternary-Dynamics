import assert from 'node:assert/strict';
import test from 'node:test';
import {
    velocityProfile, fitErfWidth, firstModeAmplitude, firstModeAmplitudeWithOffset, meanSquareDisplacement,
    linearRegression, logDecayRate, angularVelocitySplit, moleculeCentroids,
} from '../js/scales/scale3/liquid-transport.js';

function erf(x) {  // Abramowitz–Stegun 7.1.26, |error| < 1.5e-7
    const s = Math.sign(x); x = Math.abs(x);
    const t = 1 / (1 + 0.3275911 * x);
    const y = 1 - (((((1.061405429 * t - 1.453152027) * t) + 1.421413741) * t - 0.284496736) * t + 0.254829592) * t * Math.exp(-x * x);
    return s * y;
}

test('erf width fit recovers the width of a synthetic diffusing layer', () => {
    const nu = 0.7, U = 1.0;
    for (const t of [2, 5, 10]) {
        const w = 2 * Math.sqrt(nu * t);
        const centers = new Float64Array(16), mean = new Float64Array(16);
        for (let i = 0; i < 16; i++) { centers[i] = -16 + 2 * i + 1; mean[i] = U * erf(centers[i] / w); }
        const fit = fitErfWidth(centers, mean);
        assert.ok(Math.abs(fit.w - w) < 0.02 * w, `w ${fit.w} vs ${w}`);
        assert.ok(Math.abs(fit.U - U) < 0.02);
    }
});

test('width-squared regression gives 4 nu', () => {
    const nu = 0.35, t = [], w2 = [];
    for (let k = 1; k <= 20; k++) { t.push(k); w2.push(4 * nu * k + 0.01 * Math.sin(k)); }
    const r = linearRegression(t, w2);
    assert.ok(Math.abs(r.slope / 4 - nu) < 0.01 * nu);
    assert.ok(r.slopeSE >= 0);
});

test('first-mode amplitude and log-decay rate give nu = gamma h^2 / pi^2', () => {
    const h = 20, nu = 0.5, gamma = nu * Math.PI * Math.PI / (h * h);
    const y = new Float64Array(200), t = [], a = [];
    for (let i = 0; i < 200; i++) y[i] = -h / 2 + (i + 0.5) * h / 200;
    for (let k = 0; k < 30; k++) {
        const vx = new Float64Array(200);
        for (let i = 0; i < 200; i++) vx[i] = Math.exp(-gamma * k) * Math.cos(Math.PI * y[i] / h);
        t.push(k); a.push(firstModeAmplitude(y, vx, h));
    }
    assert.ok(Math.abs(a[0] - 1) < 0.01);
    const d = logDecayRate(t, a);
    assert.ok(Math.abs(d.gamma - gamma) < 1e-6);
    assert.ok(Math.abs(d.gamma * h * h / (Math.PI * Math.PI) - nu) < 1e-6);
});

test('firstModeAmplitudeWithOffset recovers the mode amplitude under a constant drift', () => {
    const h = 20, a1 = 0.42, c0 = 1.7;
    const y = new Float64Array(200);
    for (let i = 0; i < 200; i++) y[i] = -h / 2 + (i + 0.5) * h / 200;
    const vx = new Float64Array(200);
    for (let i = 0; i < 200; i++) vx[i] = c0 + a1 * Math.cos(Math.PI * y[i] / h);
    const fit = firstModeAmplitudeWithOffset(y, vx, h);
    assert.ok(Math.abs(fit - a1) < 1e-9, `a1 ${fit} vs ${a1}`);
    // The bare (offset-blind) estimator is exactly the failure mode this
    // guards against: a pure constant drift (a1 = 0) projects onto the
    // cosine mode at 4/pi under firstModeAmplitude, not 0.
    const drift = new Float64Array(200).fill(c0);
    const bare = firstModeAmplitude(y, drift, h);
    assert.ok(Math.abs(bare - c0 * 4 / Math.PI) < 1e-4, `bare ${bare} vs ${c0 * 4 / Math.PI}`);
    const offsetAware = firstModeAmplitudeWithOffset(y, drift, h);
    assert.ok(Math.abs(offsetAware) < 1e-9, `offset-aware ${offsetAware} should be ~0`);
});

test('MSD slope over 6 recovers D for a synthetic random walk', () => {
    const D = 0.25, m = 400, steps = 200, dt = 1;
    let seed = 7; const rnd = () => { seed = (seed * 1664525 + 1013904223) >>> 0; return seed / 4294967296; };
    const gauss = () => Math.sqrt(-2 * Math.log(1 - rnd())) * Math.cos(2 * Math.PI * rnd());
    const ref = new Float64Array(3 * m), now = new Float64Array(3 * m);
    const t = [], msd = [];
    for (let s = 1; s <= steps; s++) {
        for (let i = 0; i < 3 * m; i++) now[i] += Math.sqrt(2 * D * dt) * gauss();
        t.push(s * dt); msd.push(meanSquareDisplacement(ref, now, [0, 0, 0], [0, 0, 0]));
    }
    const r = linearRegression(t, msd);
    assert.ok(Math.abs(r.slope / 6 - D) < 0.08 * D, `D ${r.slope / 6}`);
});

test('angular velocity split separates inner and outer rotation', () => {
    const m = 200, c = new Float64Array(3 * m), v = new Float64Array(3 * m);
    for (let i = 0; i < m; i++) {
        const r = 1 + 9 * (i / m), th = 2 * Math.PI * (i * 0.618 % 1), omega = i < m / 2 ? 2 : 1;
        c[3 * i] = r * Math.cos(th); c[3 * i + 1] = r * Math.sin(th); c[3 * i + 2] = 0;
        v[3 * i] = -omega * r * Math.sin(th); v[3 * i + 1] = omega * r * Math.cos(th); v[3 * i + 2] = 0;
    }
    const s = angularVelocitySplit(c, v, [0, 0, 0], 2);
    assert.ok(Math.abs(s.inner - 2) < 1e-9 && Math.abs(s.outer - 1) < 1e-9);
});

test('molecule centroids group O with its two bonded H', () => {
    const positions = new Float32Array([0, 0, 0, 3.4, 0, 0, -0.85, 3.29, 0,   10, 0, 0, 13.4, 0, 0, 9.15, 3.29, 0]);
    const atomicNums = new Int32Array([8, 1, 1, 8, 1, 1]);
    const bonds = new Int32Array([0, 1, 0, 2, 3, 4, 3, 5]);
    const r = moleculeCentroids(positions, atomicNums, bonds, 6, 4);
    assert.equal(r.molecules.length, 2);
    assert.ok(Math.abs(r.centroids[0] - (16 * 0 + 3.4 - 0.85) / 18) < 1e-6);
});
