import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import {
    getSpectrumComparatorMetrics,
    RF_LATTICE_WAVE_SCENARIO_ID,
    LIGHT_LATTICE_WAVE_SCENARIO_ID,
    SOUND_LATTICE_WAVE_SCENARIO_ID,
    SOUND_COLLISION_SCENARIO_ID,
} from '../js/scales/scale0/analysis/wave-spectrum.js';

// Public regressions above use the real ES module with its actual imports.
// This second whole-module view exposes private arithmetic for the z control
// and cache checks without adding a test export to the production interface.
const sourceUrl = new URL('../js/scales/scale0/analysis/wave-spectrum.js', import.meta.url);
const source = readFileSync(sourceUrl, 'utf8');
const importSpecifier = "from '../../../constants.js'";
assert.equal(source.split(importSpecifier).length, 2);
const privateSource = source.replace(importSpecifier,
    `from ${JSON.stringify(new URL('../../../constants.js', sourceUrl).href)}`)
    + '\nexport { reduceVectorSample, getFourierBasis };\n';
const { reduceVectorSample, getFourierBasis } = await import(
    `data:text/javascript;base64,${Buffer.from(privateSource).toString('base64')}`);

function sample(positions = [], vectors = [], ArrayType = Float64Array) {
    return {
        positions: new ArrayType(positions),
        vectors: new ArrayType(vectors),
        count: Math.floor(positions.length / 3),
    };
}

function fixture({ N = 33, J = sample(), E = sample(), tick = 41 } = {}) {
    const calls = [];
    const bridge = {
        latticeSize: N,
        currentTick: () => tick,
        getToggle: name => name === 'wave_propagation',
        getFluxVectorSampled(stride) { calls.push(['J', stride]); return J; },
        getEFieldSampled(stride) { calls.push(['E', stride]); return E; },
    };
    return { bridge, calls };
}

const registeredCases = [
    ['sound x', SOUND_LATTICE_WAVE_SCENARIO_ID, 0],
    ['collision x', SOUND_COLLISION_SCENARIO_ID, 0],
    ['RF y', RF_LATTICE_WAVE_SCENARIO_ID, 1],
    ['light y', LIGHT_LATTICE_WAVE_SCENARIO_ID, 1],
];
for (const [name, scenario, axis] of registeredCases) {
    test(`${name}: actual named lane selects +3 J and +4 W=-E`, () => {
        const J = [0, 0, 0], E = [0, 0, 0];
        J[axis] = 3;
        E[axis] = -4;
        const { bridge, calls } = fixture({
            J: sample([16, 16, 16], J, Float32Array),
            E: sample([16, 16, 16], E, Float32Array),
        });
        const metrics = getSpectrumComparatorMetrics(bridge, scenario);
        assert.equal(metrics.active, true);
        assert.equal(metrics.samplingMode, 'exact');
        assert.deepEqual(calls, [['J', 1], ['E', 1]]);
        for (const lane of metrics.lanes) {
            assert.equal(lane.componentIndex, axis);
            assert.equal(lane.sampleFlux, 3);
            assert.equal(lane.sampleWaveVel, 4);
            assert.equal(lane.peakDirectionalFlux, 3);
            assert.equal(lane.peakDirectionalWaveVel, 4);
            assert.equal(lane.peakFlux, 3);
            assert.equal(lane.peakWaveVel, 4);
            assert.equal(lane.fieldEnergy, 4.5);
            assert.equal(lane.waveEnergy, 8);
            assert.equal(lane.energy, 12.5);
            assert.equal(lane.energyCentroidX, 16);
        }
    });
}

for (const [name, scenario, axis] of [registeredCases[0], registeredCases[2]]) {
    test(`${name}: signed mixed vectors retain selected probes and full vector energy`, () => {
        const J = [2, -3, 5], E = [-7, 11, -13];
        const { bridge } = fixture({ J: sample([16, 16, 16], J), E: sample([16, 16, 16], E) });
        const lane = getSpectrumComparatorMetrics(bridge, scenario).lanes[0];
        assert.equal(lane.sampleFlux, J[axis]);
        assert.equal(lane.sampleWaveVel, -E[axis]);
        assert.equal(lane.peakDirectionalFlux, Math.abs(J[axis]));
        assert.equal(lane.peakDirectionalWaveVel, Math.abs(E[axis]));
        assert.equal(lane.sampleJy, -3);
        assert.equal(lane.sampleWy, -11);
        assert.equal(lane.fieldEnergy, 19);
        assert.equal(lane.waveEnergy, 169.5);
        assert.equal(lane.energy, 188.5);

        J[axis] = 0;
        E[axis] = 0;
        const zero = fixture({ J: sample([16, 16, 16], J), E: sample([16, 16, 16], E) });
        const zeroLane = getSpectrumComparatorMetrics(zero.bridge, scenario).lanes[0];
        assert.equal(zeroLane.sampleFlux, 0);
        assert.equal(zeroLane.sampleWaveVel, -0);
        assert.equal(zeroLane.peakDirectionalFlux, 0);
        assert.equal(zeroLane.peakDirectionalWaveVel, 0);
        assert.ok(zeroLane.peakFlux > 0);
        assert.ok(zeroLane.peakWaveVel > 0);
        assert.deepEqual(zeroLane.harmonics, Array(8).fill(0));
    });
}

function accumulator(componentIndex = 2, y = 16, z = 16, band = 1, N = 33) {
    return {
        componentIndex, y, z, _band: band, _lineFlux: new Float64Array(N),
        fieldEnergy: 0, waveEnergy: 0, energy: 0, _energyX: 0,
        peakFlux: 0, peakDirectionalFlux: 0, peakWaveVel: 0, peakDirectionalWaveVel: 0,
        sampleFlux: 0, sampleWaveVel: 0, sampleJy: 0, sampleWy: 0,
    };
}

test('actual private reducer: z control, mixed signs and real directional zero', () => {
    for (const [J, E, j, w, energy] of [
        [[0, 0, 3], [0, 0, -4], 3, 4, 12.5],
        [[2, -3, 5], [-7, 11, -13], 5, 13, 188.5],
        [[2, -3, 0], [-7, 11, 0], 0, -0, 91.5],
    ]) {
        const row = accumulator();
        reduceVectorSample(sample([16, 16, 16], J), [row], 33);
        reduceVectorSample(sample([16, 16, 16], E), [row], 33, { wave: true });
        assert.equal(row.sampleFlux, j);
        assert.equal(row.sampleWaveVel, w);
        assert.equal(row.peakDirectionalFlux, Math.abs(j));
        assert.equal(row.peakDirectionalWaveVel, Math.abs(w));
        assert.equal(row._lineFlux[16], j);
        assert.equal(row.energy, energy);
        assert.equal(row.sampleJy, J[1]);
        assert.equal(row.sampleWy, -E[1]);
    }
});

function finiteSum(line, stride, independentAngle = false) {
    const N = line.length;
    return Array.from({ length: 8 }, (_, modeIndex) => {
        const m = modeIndex + 1;
        const kMode = 2 * Math.PI * m / N;
        let re = 0, im = 0;
        for (let x = 0; x < N; x++) {
            // The alternate expression is an independent Fourier definition;
            // its different rounding is checked with a declared tolerance.
            const angle = independentAngle ? 2 * Math.PI * (m * x / N) : kMode * x;
            re += line[x] * Math.cos(angle);
            im -= line[x] * Math.sin(angle);
        }
        return Math.sqrt(re * re + im * im) * 2 * stride / N;
    });
}

for (const N of [7, 33, 97]) {
    test(`all eight actual harmonics match uncached sums and a finite Fourier fixture at N=${N}`, () => {
        const mid = (N - 1) / 2;
        const stride = N === 97 ? 3 : 1;
        const line = new Float64Array(N), positions = [], vectors = [];
        for (let x = 0; x < N; x += stride) {
            const value = ((x * 7) % 11 - 5) / 8 + (x === mid ? 2 : 0);
            line[x] = value;
            positions.push(x, mid, mid);
            vectors.push(3 * value, value, -2 * value);
        }
        const { bridge, calls } = fixture({ N, J: sample(positions, vectors) });
        const metrics = getSpectrumComparatorMetrics(bridge, RF_LATTICE_WAVE_SCENARIO_ID);
        assert.deepEqual(calls, [['J', stride], ['E', stride]]);
        assert.equal(metrics.sampleStride, stride);
        assert.equal(metrics.samplingMode, stride === 1 ? 'exact' : 'stride-estimate');
        assert.deepEqual(metrics.lanes[0].harmonics, finiteSum(line, stride));
        const independent = finiteSum(line, stride, true);
        for (let m = 0; m < 8; m++) {
            assert.ok(Math.abs(metrics.lanes[0].harmonics[m] - independent[m]) <= 1e-13,
                `mode ${m + 1}: absolute error must be <= 1e-13 for this bounded fixture`);
        }
    });
}

test('basis cache retains exactly eight Float64 mode pairs for only the current N', () => {
    const a = getFourierBasis(33);
    assert.strictEqual(getFourierBasis(33), a);
    assert.equal(a.rows.length, 8);
    for (let m = 1; m <= 8; m++) {
        const row = a.rows[m - 1];
        assert.ok(row.cos instanceof Float64Array);
        assert.ok(row.sin instanceof Float64Array);
        assert.equal(row.cos.length, 33);
        assert.equal(row.sin.length, 33);
        const kMode = 2 * Math.PI * m / 33;
        for (let x = 0; x < 33; x++) {
            assert.equal(row.cos[x], Math.cos(kMode * x));
            assert.equal(row.sin[x], Math.sin(kMode * x));
        }
    }
    const b = getFourierBasis(7);
    assert.notStrictEqual(b, a);
    assert.strictEqual(getFourierBasis(7), b);
    const c = getFourierBasis(33);
    assert.notStrictEqual(c, a);
    assert.strictEqual(getFourierBasis(33), c);
});

test('typed samples outside every lane skip magnitude arithmetic, shared and boundary samples retain contributions', () => {
    const rows = [accumulator(0), accumulator(1)];
    const J = sample([0, 0, 0, 16, 16, 16, 2, 17, 16, 3, 18, 16],
        [100, 200, 300, 2, -3, 5, 1, 2, 2, 500, 600, 700], Float32Array);
    const originalSqrt = Math.sqrt;
    let sqrtCalls = 0;
    Math.sqrt = value => { sqrtCalls++; return originalSqrt(value); };
    try {
        reduceVectorSample(J, rows, 33, { stride: 3 });
    } finally {
        Math.sqrt = originalSqrt;
    }
    assert.equal(sqrtCalls, 2, 'one magnitude per admitted sample even when two lanes overlap');
    for (const row of rows) {
        assert.equal(row.energy, (19 + 4.5) * 27);
        assert.equal(row.fieldEnergy, row.energy);
        assert.equal(row._energyX, (16 * 19 + 2 * 4.5) * 27);
    }
    assert.equal(rows[0].sampleFlux, 2);
    assert.equal(rows[1].sampleFlux, -3);
});

test('absent methods and unregistered scenarios remain inactive without sampler reads', () => {
    assert.deepEqual(getSpectrumComparatorMetrics(null), { active: false, reason: 'no field buffers' });
    assert.deepEqual(getSpectrumComparatorMetrics({ getFluxVectorSampled() {} }),
        { active: false, reason: 'no field buffers' });
    const unregistered = fixture();
    assert.deepEqual(getSpectrumComparatorMetrics(unregistered.bridge, 's0-other'),
        { active: false, reason: 'not a wave-family scenario' });
    assert.deepEqual(unregistered.calls, []);
});

test('completed typed count-zero observations remain valid zero measurements', () => {
    for (const empty of [sample(), { ...sample([16, 16, 16], [9, 9, 9]), count: 0 }]) {
        const { bridge, calls } = fixture({ J: empty, E: empty });
        const metrics = getSpectrumComparatorMetrics(bridge);
        assert.equal(metrics.active, true);
        assert.equal(metrics.totalLaneEnergy, 0);
        assert.equal(metrics.lanes[0].sampleFlux, 0);
        assert.equal(metrics.lanes[0].sampleWaveVel, 0);
        assert.deepEqual(metrics.lanes[0].harmonics, Array(8).fill(0));
        assert.deepEqual(calls, [['J', 1], ['E', 1]]);
    }
});

test('one or both missing field payloads wait and still request both sampler kinds', () => {
    const valid = sample([16, 16, 16], [2, -3, 5]);
    for (const [J, E] of [[null, null], [null, valid], [valid, null], [undefined, valid]]) {
        const calls = [];
        const bridge = {
            latticeSize: 33,
            getFluxVectorSampled(stride) { calls.push(['J', stride]); return J; },
            getEFieldSampled(stride) { calls.push(['E', stride]); return E; },
        };
        assert.deepEqual(getSpectrumComparatorMetrics(bridge),
            { active: false, reason: 'waiting for field buffers' });
        assert.deepEqual(calls, [['J', 1], ['E', 1]]);
    }
});

test('worker placeholder empties wait for each completed snapshot and recover at count zero', () => {
    const f = fixture();
    const available = new Set();
    const snapshots = [];
    f.bridge.hasSamplerSnapshot = (kind, stride) => {
        snapshots.push([kind, stride]);
        return available.has(kind);
    };
    assert.equal(getSpectrumComparatorMetrics(f.bridge).active, false);
    available.add('fluxVector');
    assert.equal(getSpectrumComparatorMetrics(f.bridge).active, false);
    available.add('e');
    const complete = getSpectrumComparatorMetrics(f.bridge);
    assert.equal(complete.active, true);
    assert.equal(complete.totalLaneEnergy, 0);
    assert.deepEqual(f.calls, Array.from({ length: 3 }, () => [['J', 1], ['E', 1]]).flat());
    assert.deepEqual(snapshots, Array.from({ length: 3 }, () => [['fluxVector', 1], ['e', 1]]).flat());
});

test('malformed shapes and nonfinite consumed payloads cannot masquerade as zero observations', () => {
    const valid = sample([16, 16, 16], [2, -3, 5]);
    const malformed = [
        {}, { count: 0 }, { ...valid, count: -1 }, { ...valid, count: 1.5 },
        { ...valid, count: '1' }, { ...valid, count: 2 },
        { ...valid, count: Number.NaN }, { ...valid, count: Infinity },
        { ...valid, positions: [16, 16, 16] }, { ...valid, vectors: [2, -3, 5] },
        { ...valid, vectors: new DataView(new ArrayBuffer(24)) },
        { ...valid, vectors: new BigInt64Array([2n, -3n, 5n]) },
        { ...valid, positions: new Float32Array([16, 16]) },
        { ...valid, vectors: new Float32Array([2, -3]) },
        sample([NaN, 16, 16], [2, -3, 5]), sample([16, Infinity, 16], [2, -3, 5]),
        sample([16, 16, 16], [2, NaN, 5]), sample([16, 16, 16], [2, -3, Infinity]),
        sample([0, 0, 0], [NaN, 2, 3]), // bad data outside the lane is still bad data
        sample([16, 16, 16], [1e308, 2, 3]), // finite input whose measured energy overflows
    ];
    for (const bad of malformed) {
        for (const [J, E] of [[bad, valid], [valid, bad]]) {
            const f = fixture({ J, E });
            assert.deepEqual(getSpectrumComparatorMetrics(f.bridge),
                { active: false, reason: 'waiting for field buffers' });
            assert.deepEqual(f.calls, [['J', 1], ['E', 1]]);
        }
    }
});

test('J is reduced synchronously before E can invalidate its WASM-style views', () => {
    const J = sample([16, 16, 16], [2, -3, 5], Float32Array);
    const E = sample([16, 16, 16], [-7, 11, -13], Float32Array);
    const f = fixture({ J, E });
    f.bridge.getEFieldSampled = stride => {
        f.calls.push(['E', stride]);
        structuredClone(J, { transfer: [J.positions.buffer, J.vectors.buffer] });
        return E;
    };
    const m = getSpectrumComparatorMetrics(f.bridge, SOUND_LATTICE_WAVE_SCENARIO_ID);
    assert.equal(J.positions.length, 0);
    assert.equal(J.vectors.length, 0);
    assert.equal(m.active, true);
    assert.equal(m.lanes[0].fieldEnergy, 19);
    assert.equal(m.lanes[0].waveEnergy, 169.5);
    assert.equal(m.lanes[0].energy, 188.5);
    assert.equal(m.lanes[0].sampleFlux, 2);
    assert.equal(m.lanes[0].sampleWaveVel, 7);
    assert.deepEqual(f.calls, [['J', 1], ['E', 1]]);
});

test('same-tick mutation, owner change and scenario change always read fresh samples', () => {
    const J = sample([16, 16, 16], [2, 3, 5]);
    const first = fixture({ J, tick: 12 });
    const firstX = getSpectrumComparatorMetrics(first.bridge, SOUND_LATTICE_WAVE_SCENARIO_ID);
    J.vectors[0] = -7;
    J.vectors[1] = 11;
    const changedX = getSpectrumComparatorMetrics(first.bridge, SOUND_LATTICE_WAVE_SCENARIO_ID);
    const changedY = getSpectrumComparatorMetrics(first.bridge, RF_LATTICE_WAVE_SCENARIO_ID);
    assert.equal(firstX.tick, 12);
    assert.equal(changedX.tick, 12);
    assert.equal(firstX.lanes[0].sampleFlux, 2);
    assert.equal(changedX.lanes[0].sampleFlux, -7);
    assert.equal(changedY.lanes[0].sampleFlux, 11);
    assert.deepEqual(first.calls, Array.from({ length: 3 }, () => [['J', 1], ['E', 1]]).flat());
    const second = fixture({ J: sample([16, 16, 16], [13, 17, 19]), tick: 99 });
    const secondY = getSpectrumComparatorMetrics(second.bridge, RF_LATTICE_WAVE_SCENARIO_ID);
    assert.equal(secondY.tick, 99);
    assert.equal(secondY.lanes[0].sampleFlux, 17);
    assert.deepEqual(second.calls, [['J', 1], ['E', 1]]);
});
