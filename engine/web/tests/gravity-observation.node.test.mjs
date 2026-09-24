import test from 'node:test';
import assert from 'node:assert/strict';
import { readGravityObservation } from '../js/scales/scale0/ui/overlays/gravity-observation.js';

const N = 7;
const tick = 91;
const sourceEpoch = 14;
const dataVersion = 35;
const loadGeneration = 4;
const maxRho = 9;

function observation() {
    const mid = N >> 1;
    const metadata = {
        sampleTick: tick,
        source: 'wasm-worker',
        sourceEpoch,
        configurationToken: sourceEpoch,
        loadGeneration,
        dataVersion,
        latticeSize: N,
        representation: 'reference-flux-magnitude',
    };
    return {
        N, stride: 6, sampleTick: tick, source: 'wasm-worker', sourceEpoch,
        configurationToken: sourceEpoch, loadGeneration, dataVersion, maxRho,
        slabs: [0, 1, 2].map(axis => ({
            N, axis, index: mid, startPlane: mid - 1, planeCount: 3,
            data: new Float64Array(3 * N * N).fill(axis + 1), maxRho, metadata,
        })),
        latency: { values: new Float32Array([0.25, 0.75]), count: 2 },
        kretschmann: { values: new Float32Array([0.5]), count: 1 },
        gravity: { vectors: new Float32Array([3, 4, 0]), count: 1 },
        gravityMetricAgg: {
            active: true, requested: true, latencyMax: 0.7, latencyMean: 0.3,
            fMin: 0.5, gammaMax: 1.2, dilationMaxPct: 4, voxelCount: 7,
        },
        engineToggles: { forces: true, gravity: true },
    };
}

test('accepts one complete exact-tick worker observation and derives the shared stamp', () => {
    const raw = observation();
    const accepted = readGravityObservation(raw, N, 6);
    assert.ok(accepted);
    assert.equal(accepted.tick, tick);
    assert.equal(accepted.stamp, `${sourceEpoch}:${dataVersion}:${tick}`);
    assert.strictEqual(accepted.aggregate, raw.gravityMetricAgg);
    assert.equal(accepted.metrics.L.max, 0.75);
    assert.equal(accepted.metrics.F.max, 5);
});

test('rejects unknown clocks, stale-source aliases, and malformed atomic components', () => {
    const invalid = [
        raw => { raw.sampleTick = null; },
        raw => { raw.source = 'native'; },
        raw => { raw.sourceEpoch = '14'; },
        raw => { raw.configurationToken = 15; },
        raw => { raw.loadGeneration = 0; },
        raw => { raw.maxRho = 0; },
        raw => { raw.slabs[1].startPlane = 0; },
        raw => { raw.slabs[2].metadata = { ...raw.slabs[2].metadata, sampleTick: 90 }; },
        raw => { raw.slabs[0].data = new Float32Array(raw.slabs[0].data); },
        raw => { raw.gravity.vectors[0] = Number.NaN; },
        raw => { raw.gravityMetricAgg.gammaMax = Infinity; },
        raw => { raw.gravityMetricAgg.requested = null; },
    ];
    for (const mutate of invalid) {
        const raw = observation();
        mutate(raw);
        assert.equal(readGravityObservation(raw, N, 6), null);
    }
});
