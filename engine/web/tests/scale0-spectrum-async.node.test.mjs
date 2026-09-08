import test from 'node:test';
import assert from 'node:assert/strict';
import { Worker } from 'node:worker_threads';
import { SpectrumAnalysisClient, captureSpectrumObservation } from '../js/scales/scale0/analysis/spectrum-analysis-client.js';
import { analyzeSpectrumObservation } from '../js/scales/scale0/analysis/spectrum-analysis-core.js';
import { denseVectorGridFromSamples, energySpectrum, spectralPeak, spectralSlope } from '../js/scales/scale0/analysis/lattice-spectrum.js';
import { defectCount, fluxTubeComponents, histogram, metricStats, chiralityFromAudit } from '../js/scales/scale0/analysis/lattice-topology.js';

const workerURL = new URL('../js/scales/scale0/analysis/spectrum-analysis-worker.js', import.meta.url);
const fixtureSample = (L, stride) => {
    const origin = Math.floor((L - 1) / 2) % stride;
    const positions = [], vectors = [], values = [];
    for (let z = origin; z < L; z += stride) for (let y = origin; y < L; y += stride) for (let x = origin; x < L; x += stride) {
        if ((x + 2 * y + 3 * z) % 5 === 0) continue;
        positions.push(x + .5, y + .5, z + .5);
        vectors.push(Math.sin(x / L * 2 * Math.PI), (y - z) / L, Math.cos(z / L * 4 * Math.PI));
        values.push((x - y + z) / L);
    }
    return { count: values.length, positions: Float32Array.from(positions), vectors: Float32Array.from(vectors),
        values: Float32Array.from(values), effectiveStride: stride, origin,
        sampleTick: 47, provenance: Object.freeze({ sampleTick: 19, sourceEpoch: '9007199254740993',
            epoch: '18446744073709551614', nativeInstanceId: '000000000000000000000000000000ab', status: 'observed' }) };
};
function input(L = 7, stride = 1, M = 8, mode = 'live') {
    const sample = fixtureSample(L, stride);
    const audit = { gaussViolation: .25, maxGaussError: .5, ELTotal: 3, ERTotal: 1, wvLTotal: 2, wvRTotal: 6 };
    const observation = captureSpectrumObservation({ getScale0FieldSamples: () => sample,
        hasScale0SamplerSnapshot: () => true }, { L, stride, M, mode, audit,
        metricKinds: [{ kind: 'helicity' }, { kind: 'fisher' }], auditTick: 18, diagTick: 17 });
    return { observation, sample, audit };
}

function oracle(observation) {
    const grid = denseVectorGridFromSamples(observation.flux, observation.L, observation.stride);
    const mag = Float64Array.from(grid.jx, (x, i) => Math.hypot(x, grid.jy[i], grid.jz[i]));
    const spec = energySpectrum(grid, grid.srcN, observation.M, observation.L);
    return { spec, peak: spectralPeak(spec.k, spec.E), slope: spectralSlope(spec.k, spec.E), mag,
        parseval: spec.sumReal > 0 ? spec.totalE / spec.sumReal : 1,
        tubes: fluxTubeComponents(mag, grid.srcN, .35),
        defects: observation.divergence ? defectCount(observation.divergence.values, observation.divergence.count, .5) : null,
        metrics: observation.metrics.map(({ kind, sample }) => ({ kind,
            stats: metricStats(sample.values, sample.count), hist: histogram(sample.values, sample.count, 22) })) };
}

for (const [L, stride, M, mode] of [[33, 1, 32, 'live'], [97, 7, 8, 'live'], [7, 1, 64, 'deep']]) {
    test(`real module worker exactly matches the retained numerical oracles L=${L} stride=${stride} M=${M}`, async () => {
        const { observation, audit } = input(L, stride, M, mode), expected = oracle(observation);
        const source = `import { parentPort } from 'node:worker_threads';
            import { handleSpectrumAnalysisRequest } from ${JSON.stringify(workerURL.href)};
            parentPort.on('message', message => handleSpectrumAnalysisRequest(message,
                (result, transfers = []) => parentPort.postMessage(result, transfers)));`;
        const worker = new Worker(new URL(`data:text/javascript,${encodeURIComponent(source)}`), { type: 'module' });
        try {
            const result = await new Promise((resolve, reject) => {
                worker.once('error', reject); worker.once('message', resolve);
                worker.postMessage({ id: 71, context: { ownerId: 2, generation: 8 }, observation });
            });
            assert.equal(result.id, 71); assert.deepEqual(result.context, { ownerId: 2, generation: 8 });
            assert.equal(result.error, undefined);
            const { spectrum, topology, metrics, provenance } = result.result;
            for (const key of ['spec', 'peak', 'slope', 'mag', 'parseval']) assert.deepEqual(spectrum[key], expected[key], key);
            assert.deepEqual(topology.tubes, expected.tubes); assert.deepEqual(topology.defects, expected.defects);
            assert.deepEqual(topology.chir, chiralityFromAudit(audit)); assert.deepEqual(metrics, expected.metrics);
            assert.deepEqual(provenance, observation.provenance);
        } finally { await worker.terminate(); }
    });
}

test('owned capture retains exact source metadata and does not detach or mutate bridge caches', () => {
    const { observation, sample, audit } = input();
    const bytes = sample.positions.byteLength, value = sample.vectors[0];
    assert.notEqual(observation.flux.vectors.buffer, sample.vectors.buffer);
    assert.equal(observation.provenance.flux.sampleTick, 19); // immutable sample provenance wins over mutable alias 47
    assert.equal(observation.provenance.flux.sourceEpoch, '9007199254740993');
    sample.vectors[0] = 99; audit.gaussViolation = 99;
    assert.equal(observation.flux.vectors[0], value); assert.equal(observation.audit.gaussViolation, .25);
    const moved = structuredClone(observation, { transfer: [observation.flux.positions.buffer, observation.flux.vectors.buffer] });
    assert.equal(observation.flux.vectors.byteLength, 0);
    assert.equal(sample.positions.byteLength, bytes); assert.equal(sample.vectors[0], 99);
    assert.equal(moved.flux.vectors[0], value);
});

test('unknown sampler clocks stay absent; missing and nonfinite samples stay unavailable', () => {
    const sample = { count: 1, positions: [1.5, 1.5, 1.5], vectors: [1, NaN, 0], values: [Infinity] };
    const captured = captureSpectrumObservation({ getScale0FieldSamples: () => sample },
        { L: 3, stride: 1, M: 8, mode: 'live', metricKinds: [{ kind: 'fisher' }] });
    assert.deepEqual(captured.provenance.flux, {});
    const result = analyzeSpectrumObservation(captured);
    assert.equal(result.spectrum, null); assert.equal(result.topology.tubes, null);
    assert.equal(result.topology.defects, null); assert.equal(result.metrics[0].stats, null);
});

function clientFixture() {
    let now = 0, nextTimer = 1;
    const workers = [], timers = new Map(), results = [], errors = [];
    const client = new SpectrumAnalysisClient({ now: () => now,
        schedule: (cb, ms) => { const id = nextTimer++; timers.set(id, { cb, at: now + ms }); return id; },
        unschedule: id => timers.delete(id), workerFactory: () => {
            const worker = { posts: [], terminated: false,
                postMessage(message, transfer) { this.posts.push(structuredClone(message, { transfer })); },
                terminate() { this.terminated = true; } };
            workers.push(worker); return worker;
        } });
    return { client, workers, timers, results, errors,
        submit(options = {}) { return client.submit(input().observation, { context: { ownerId: 1, generation: 3 },
            onResult: (result, meta) => results.push({ result, meta }), onError: error => errors.push(error), ...options }); },
        reply(worker = workers.at(-1), overrides = {}) {
            const job = worker.posts.at(-1);
            worker.onmessage({ data: { id: job.id, context: job.context,
                result: analyzeSpectrumObservation(job.observation), ...overrides } });
        },
        advance(ms, deliver = true) { now += ms; if (deliver) {
            for (const [id, timer] of [...timers]) if (timer.at <= now) { timers.delete(id); timer.cb(); }
        } },
    };
}

test('client bounds the queue to active plus latest and preserves monotonic identity', () => {
    const f = clientFixture(); const first = f.submit(); f.submit(); const latest = f.submit();
    assert.equal(f.workers.length, 1); assert.equal(f.workers[0].posts.length, 1); assert.equal(f.client.queued, true);
    f.reply(); assert.equal(f.results[0].meta.id, first); assert.equal(f.workers[0].posts.length, 2);
    assert.equal(f.workers[0].posts[1].id, latest); f.reply();
    assert.equal(f.results[1].meta.id, latest); assert.equal(f.client.busy, false); assert.equal(f.client.queued, false);
    assert.equal(f.timers.size, 0); assert.ok(Object.isFrozen(f.results[1].result.provenance.flux)); f.client.dispose();
});

test('cancellation kills work and an obsolete worker cannot publish into a newer request', () => {
    const f = clientFixture(); const first = f.submit(), old = f.workers[0]; f.submit();
    f.client.cancel(); const latest = f.submit(); assert.ok(latest > first); assert.equal(old.terminated, true);
    f.reply(old); assert.equal(f.results.length, 0); f.reply(); assert.equal(f.results[0].meta.id, latest);
    f.client.dispose(); const count = f.workers.length; assert.equal(f.submit(), null); assert.equal(f.workers.length, count);
});

test('result callback cancellation cannot resurrect its previously queued observation', () => {
    const f = clientFixture(); f.submit({ onResult: () => f.client.cancel() }); f.submit();
    f.reply(); assert.equal(f.client.busy, false); assert.equal(f.workers[0].posts.length, 1);
});

test('wrong context and changed sample provenance fail closed', () => {
    for (const change of ['context', 'provenance']) {
        const f = clientFixture(); f.submit();
        if (change === 'context') f.reply(undefined, { context: { ownerId: 2, generation: 3 } });
        else {
            const result = analyzeSpectrumObservation(f.workers[0].posts[0].observation);
            result.provenance.flux.sampleTick = 20;
            f.reply(undefined, { result });
        }
        assert.equal(f.results.length, 0); assert.match(f.errors[0], /mismatch/);
        assert.equal(f.workers[0].terminated, true); f.client.dispose();
    }
});

test('missing Worker has no synchronous fallback and leaves no active work', () => {
    const errors = [];
    const client = new SpectrumAnalysisClient({ workerFactory: () => { throw Error('unavailable'); } });
    client.submit(input().observation, { context: {}, onResult: () => assert.fail('no fallback'), onError: e => errors.push(e) });
    assert.deepEqual(errors, ['analysis-worker-unavailable']); assert.equal(client.busy, false); client.dispose();
});

test('default timer wrappers preserve the browser global receiver', () => {
    const originalSchedule = globalThis.setTimeout, originalUnschedule = globalThis.clearTimeout;
    const calls = [];
    globalThis.setTimeout = function (callback, delay) {
        assert.equal(this, globalThis, 'setTimeout receiver');
        calls.push(['schedule', delay]); return 71;
    };
    globalThis.clearTimeout = function (timer) {
        assert.equal(this, globalThis, 'clearTimeout receiver'); calls.push(['cancel', timer]);
    };
    try {
        const client = new SpectrumAnalysisClient({ now: () => 0,
            workerFactory: () => ({ postMessage() {}, terminate() {} }) });
        client.cancel();
        client.submit(input().observation, { context: {}, onResult() {} });
        client.dispose();
        assert.deepEqual(calls, [['cancel', null], ['schedule', 5000], ['cancel', 71]]);
    } finally {
        globalThis.setTimeout = originalSchedule; globalThis.clearTimeout = originalUnschedule;
    }
});

test('timeout includes queued time and a throttled timeout cannot permit a late result', () => {
    const f = clientFixture(); f.submit({ deadline: 100 }); f.advance(101, false); f.reply();
    assert.deepEqual(f.errors, ['analysis-timeout']); assert.equal(f.results.length, 0);
    f.submit({ deadline: 200 }); f.submit({ deadline: 150 }); f.advance(60, false); f.reply();
    assert.equal(f.results.length, 1); assert.deepEqual(f.errors, ['analysis-timeout', 'analysis-timeout']);
    assert.equal(f.client.busy, false); f.client.dispose();
});
