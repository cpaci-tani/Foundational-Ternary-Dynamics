import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import '../js/bridge/flux-publication.classic.js';

const workerSource = readFileSync(new URL('../js/bridge/wasm-bridge.worker.js', import.meta.url), 'utf8');
const helperSource = workerSource.slice(
    workerSource.indexOf('const GRAVITY_OBSERVATION_TOGGLE_KEYS'),
    workerSource.indexOf('let lastInspect = null'),
);

function captureFixture({ wantGravity = true, tick = 37 } = {}) {
    const N = 7;
    const togglesRead = [];
    const scope = {
        N,
        wantGravity,
        activeConfigurationToken: 14,
        renderBridgeGeneration: 4,
        Number,
        Set,
        Object,
        Float64Array,
        self: { FTD_FLUX_PUBLICATION: globalThis.FTD_FLUX_PUBLICATION },
        bridge: {},
        mod: {
            getToggle(_bridge, key) { togglesRead.push(key); return key !== 'geometric_gravity'; },
        },
    };
    vm.createContext(scope);
    const api = vm.runInContext(`${helperSource}\n({ capture: captureGravityObservation, complete: completeGravitySamplerBatch })`, scope);
    const volume = Float64Array.from({ length: N ** 3 }, (_, i) => (i + 1) / 13);
    const samplers = {
        'latency@6': { values: new Float32Array([1]), count: 1 },
        'kretschmann@6': { values: new Float32Array([2]), count: 1 },
        'gravity@6': { vectors: new Float32Array([3, 4, 5]), count: 1 },
        'gravityMetricAgg@0': { active: true, latencyMean: 0.2 },
    };
    const transfer = [];
    return { N, capture(vol, samplerRows, version, transfers, sampleTick) {
        return api.capture(vol, api.complete(samplerRows), version, transfers, sampleTick);
    }, volume, samplers, transfer, tick, togglesRead };
}

test('worker gravity bundle captures slabs, scalar samples and force toggles at one supplied physics tick', () => {
    const f = captureFixture();
    const result = f.capture(f.volume, f.samplers, 51, f.transfer, f.tick);
    assert.equal(result.N, f.N);
    assert.equal(result.sampleTick, 37);
    assert.equal(result.source, 'wasm-worker');
    assert.equal(result.sourceEpoch, 14);
    assert.equal(result.loadGeneration, 4);
    assert.equal(result.dataVersion, 51);
    assert.equal(result.slabs.length, 3);
    assert.deepEqual(result.slabs.map(s => [s.axis, s.index, s.startPlane, s.planeCount]), [
        [0, 3, 2, 3], [1, 3, 2, 3], [2, 3, 2, 3],
    ]);
    assert.ok(result.slabs.every(s => s.maxRho === result.maxRho && s.metadata.sampleTick === 37));
    assert.strictEqual(result.samples[0].latency, f.samplers['latency@6']);
    assert.strictEqual(result.samples[0].kretschmann, f.samplers['kretschmann@6']);
    assert.strictEqual(result.samples[0].gravity, f.samplers['gravity@6']);
    assert.strictEqual(result.gravityMetricAgg, f.samplers['gravityMetricAgg@0']);
    assert.equal(f.transfer.length, 3);
    assert.equal(result.engineToggles.forces, true);
    assert.equal(result.engineToggles.geometric_gravity, false);
    assert.deepEqual(f.togglesRead.sort(), [
        'field_energy_gravity', 'forces', 'geometric_gravity', 'gravity', 'latency_field',
    ]);
});

test('worker refuses an incomplete trio or an unknown physics clock', () => {
    for (const mutate of [
        f => delete f.samplers['gravity@6'],
        f => { f.samplers['latency@5'] = f.samplers['latency@6']; delete f.samplers['latency@6']; },
        f => { f.tick = null; },
    ]) {
        const f = captureFixture();
        mutate(f);
        assert.equal(f.capture(f.volume, f.samplers, 51, f.transfer, f.tick), null);
    }
});
