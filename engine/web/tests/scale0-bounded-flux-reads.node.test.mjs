import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import { Worker } from 'node:worker_threads';
import { once } from 'node:events';
import '../js/bridge/flux-publication.classic.js';
import { WasmBridgeProxy } from '../js/bridge/wasm-bridge-proxy.js';
import { AnisotropyComponent, circularFluxProbeIndices, circularFluxDecay,
    makeFluxMagnitudeSampler } from '../js/scales/scale0/ui/overlays/p1-observables/anisotropy.js';

const publication = globalThis.FTD_FLUX_PUBLICATION;
const dataFor = size => Float64Array.from({ length: size ** 3 }, (_, i) => 1 + i + 2 ** -40);
function boundProxy(size = 7, data = dataFor(size)) {
    const proxy = Object.create(WasmBridgeProxy.prototype);
    Object.assign(proxy, { latticeSize: size, _ready: true, _terminated: false, _disposing: false,
        _pendingConfigurationToken: 2, _appliedConfigurationToken: 2, _fluxBufferGeneration: 0,
        _digestPending: new Map(), _inspectionCache: new Map(), _inspectionPending: new Map() });
    const buffer = publication.create(size ** 3);
    publication.publish(buffer, data);
    proxy._bindFlux({ fluxSab: buffer, fluxLen: data.length, doubleBuffered: true,
        fluxProtocol: publication.PROTOCOL });
    return { proxy, buffer, data };
}
function assertBytes(actual, expected) {
    assert.deepEqual(new Uint8Array(actual.buffer, actual.byteOffset, actual.byteLength),
        new Uint8Array(expected.buffer, expected.byteOffset, expected.byteLength));
}
function oracleSlice(data, size, axis, index) {
    const fixed = Math.max(0, Math.min(index, size - 1));
    const out = new Float64Array(size ** 2);
    for (let z = 0; z < size; z++) for (let y = 0; y < size; y++) for (let x = 0; x < size; x++) {
        if ([x, y, z][axis] !== fixed) continue;
        const a = axis === 0 ? y : x, b = axis === 2 ? y : z;
        out[a * size + b] = data[(z * size + y) * size + x];
    }
    return out;
}

for (const size of [3, 7]) for (const axis of [0, 1, 2]) {
    test(`bounded plane matches full-copy oracle bitwise: N=${size}, axis=${axis}`, () => {
        const { proxy, buffer } = boundProxy(size);
        const full = publication.snapshot(buffer, size ** 3).data;
        for (const index of [-1, 0, 1, size - 1, size, Number.MAX_SAFE_INTEGER]) {
            const actual = proxy.getFluxSlice(axis, index);
            assert.equal(actual.length, size ** 2);
            assertBytes(actual, oracleSlice(full, size, axis, index));
            assert.ok(actual.buffer instanceof ArrayBuffer);
            assert.equal(actual.buffer.byteLength, 8 * size ** 2);
        }
        assert.equal(proxy._fluxSnapshot, null, 'plane must not populate the volume cache');
    });
}

test('zero, negative zero, NaN and infinity are copied without reinterpretation', () => {
    const input = new Float64Array(27);
    input[0] = -0; input[1] = NaN; input[2] = Infinity; input[3] = -Infinity;
    const { proxy } = boundProxy(3, input);
    assertBytes(proxy.getFluxSlice(2, 0), oracleSlice(input, 3, 2, 0));
    assertBytes(proxy.sampleFluxAtCells([0, 1, 2, 3]), input.slice(0, 4));
    assert.equal(proxy.getFluxSlice(2, 2).length, 9, 'all-zero plane is available');
});

test('128 probes preserve requested order, repeated cells, seams and Float64 values', () => {
    const { proxy, buffer } = boundProxy();
    const indices = Float64Array.from({ length: 128 }, (_, i) => (i * 37) % 343);
    indices[0] = 0; indices[1] = 342; indices[2] = 0;
    const full = publication.snapshot(buffer, 343).data;
    const actual = proxy.sampleFluxAtCells(indices);
    assertBytes(actual, Float64Array.from(indices, i => full[i]));
    assert.equal(actual.buffer.byteLength, 128 * 8);
    assert.equal(proxy._fluxSnapshot, null);
});

test('bounded reads neither call full snapshot nor retain unbounded read caches', () => {
    const { proxy } = boundProxy();
    const saved = globalThis.FTD_FLUX_PUBLICATION;
    globalThis.FTD_FLUX_PUBLICATION = { ...publication, snapshot() { throw Error('whole-volume read'); } };
    try {
        for (let i = 0; i < 50; i++) {
            assert.equal(proxy.getFluxSlice(i % 3, i % 7).length, 49);
            assert.equal(proxy.sampleFluxAtCells([i, 342 - i]).length, 2);
        }
        assert.equal(proxy._fluxSnapshot, null);
    } finally { globalThis.FTD_FLUX_PUBLICATION = saved; }
});

test('bounded snapshots and immutable metadata survive repeated publish and rebind', () => {
    const { proxy, buffer } = boundProxy();
    proxy._lastTick = 999;
    proxy.currentTick = () => 999;
    const plane = proxy.getFluxSlice(0, 0), probes = proxy.sampleFluxAtCells([0, 342]);
    const before = plane.slice(), probeBefore = probes.slice();
    assert.equal(plane.meta.sampleTick, null);
    assert.equal(probes.meta.sampleTick, null);
    assert.equal(plane.meta.publicationVersion, 1);
    assert.ok(Object.isFrozen(plane.meta));
    assert.throws(() => { plane.meta.sampleTick = 999; }, TypeError);
    assert.throws(() => { plane.meta = {}; }, TypeError);
    for (let i = 2; i < 50; i++) publication.publish(buffer, new Float64Array(343).fill(i));
    assertBytes(plane, before); assertBytes(probes, probeBefore);
    const replacement = publication.create(343);
    publication.publish(replacement, new Float64Array(343).fill(61));
    proxy._bindFlux({ fluxSab: replacement, fluxLen: 343, doubleBuffered: true, fluxProtocol: publication.PROTOCOL });
    const replaced = proxy.sampleFluxAtCells([0]);
    assert.equal(replaced[0], 61);
    assert.notEqual(replaced.meta.bufferGeneration, plane.meta.bufferGeneration);
    assert.equal(replaced.meta.publicationVersion, plane.meta.publicationVersion,
        'buffer namespace distinguishes equal local publication versions');
    assertBytes(plane, before);
});

test('an unpublished or contended slot returns unavailable without a full-copy fallback', () => {
    const { proxy, buffer } = boundProxy();
    const pinned = publication.acquire(buffer, 343);
    const header = new Int32Array(buffer, 0, 4);
    try {
        assert.equal(proxy.getFluxSlice(0, 0).length, 0);
        assert.equal(proxy.sampleFluxAtCells([0]).length, 0);
        assert.equal(Atomics.load(header, 2), 1, 'failed reader does not release someone else\'s pin');
    } finally { pinned.release(); }
    assert.equal(proxy.sampleFluxAtCells([0]).length, 1);
    const fresh = publication.create(343);
    assert.equal(publication.snapshotSlice(fresh, 7, 0, 0), null);
    assert.equal(publication.snapshotCells(fresh, 7, [0]), null);
});

test('acquire is bounded at three attempts under contention', () => {
    let attempts = 0;
    const context = { SharedArrayBuffer, ArrayBuffer, Float64Array, Int32Array,
        Atomics: { load: Atomics.load, store: Atomics.store,
            compareExchange(...args) { attempts++; return Atomics.compareExchange(...args); } } };
    vm.runInNewContext(readFileSync(new URL('../js/bridge/flux-publication.classic.js', import.meta.url), 'utf8'), context);
    const api = context.FTD_FLUX_PUBLICATION, buffer = api.create(27);
    api.publish(buffer, new Float64Array(27));
    const read = api.acquire(buffer, 27);
    attempts = 0;
    assert.equal(api.snapshotSlice(buffer, 3, 0, 0), null);
    assert.equal(attempts, 3);
    attempts = 0;
    assert.equal(api.snapshotCells(buffer, 3, [0]), null);
    assert.equal(attempts, 3);
    read.release();
});

test('sampled concurrent writer/readers never expose a mixed plane or probe publication', async () => {
    const size = 17, buffer = publication.create(size ** 3), control = new SharedArrayBuffer(4);
    const stop = new Int32Array(control);
    publication.publish(buffer, new Float64Array(size ** 3).fill(1));
    const worker = new Worker(`
        const { parentPort, workerData } = require('node:worker_threads');
        (async () => {
            await import(workerData.module);
            const publication = globalThis.FTD_FLUX_PUBLICATION;
            const stop = new Int32Array(workerData.control);
            const values = new Float64Array(workerData.length);
            let value = 2;
            parentPort.postMessage('ready');
            while (Atomics.load(stop, 0) === 0) {
                values.fill(value++);
                publication.publish(workerData.buffer, values);
            }
            parentPort.postMessage('stopped');
        })().catch(error => { throw error; });
    `, { eval: true, workerData: { buffer, control, length: size ** 3,
        module: new URL('../js/bridge/flux-publication.classic.js', import.meta.url).href } });
    try {
        await once(worker, 'message');
        let completed = 0, advanced = false;
        const indices = Array.from({ length: 128 }, (_, i) => (i * 37) % (size ** 3));
        for (let batch = 0; batch < 50; batch++) {
            for (let i = 0; i < 20; i++) {
                const read = i % 2 ? publication.snapshotSlice(buffer, size, i % 3, i % size)
                    : publication.snapshotCells(buffer, size, indices);
                if (!read) continue;
                assert.ok(read.data.every(value => value === read.data[0]), 'one uniform writer publication per read');
                assert.ok(read.data.buffer instanceof ArrayBuffer);
                completed++;
                advanced ||= read.data[0] > 1;
            }
            await new Promise(resolve => setImmediate(resolve));
        }
        assert.ok(completed > 0);
        assert.ok(advanced, 'the writer actually advanced during the sampled run');
    } finally {
        Atomics.store(stop, 0, 1);
        await worker.terminate();
    }
});

test('plane and probe output allocation occurs while pinned and releases on throw', () => {
    for (const kind of ['plane', 'probes']) {
        let api, shared, throwOutput = false, inspected = false;
        class GuardedFloat64Array extends Float64Array {
            constructor(...args) {
                if (throwOutput && args.length === 1 && args[0] === (kind === 'plane' ? 9 : 2)) {
                    const header = new Int32Array(shared, 0, 4), slot = Atomics.load(header, 0) & 1;
                    assert.equal(Atomics.load(header, 1 + slot), 1);
                    assert.equal(api.publish(shared, new Float64Array(27).fill(2)), true);
                    assert.equal(api.publish(shared, new Float64Array(27).fill(3)), false);
                    inspected = true;
                    throw Error('injected allocation failure');
                }
                super(...args);
            }
        }
        const context = { SharedArrayBuffer, ArrayBuffer, Float64Array: GuardedFloat64Array, Int32Array, Atomics };
        vm.runInNewContext(readFileSync(new URL('../js/bridge/flux-publication.classic.js', import.meta.url), 'utf8'), context);
        api = context.FTD_FLUX_PUBLICATION; shared = api.create(27);
        api.publish(shared, new Float64Array(27).fill(1));
        throwOutput = true;
        assert.throws(() => kind === 'plane' ? api.snapshotSlice(shared, 3, 0, 0)
            : api.snapshotCells(shared, 3, [0, 1]), /injected allocation failure/);
        assert.ok(inspected);
        const header = new Int32Array(shared, 0, 4);
        assert.equal(Atomics.load(header, 1), 0); assert.equal(Atomics.load(header, 2), 0);
        assert.equal(api.publish(shared, new Float64Array(27).fill(3)), true);
    }
});

test('plane and probe reads release the pin if shared view construction throws', () => {
    for (const kind of ['plane', 'probes']) {
        let failView = false;
        class GuardedFloat64Array extends Float64Array {
            constructor(...args) {
                if (failView && args.length === 3) throw Error('injected shared view failure');
                super(...args);
            }
        }
        const context = { SharedArrayBuffer, ArrayBuffer, Float64Array: GuardedFloat64Array, Int32Array, Atomics };
        vm.runInNewContext(readFileSync(new URL('../js/bridge/flux-publication.classic.js', import.meta.url), 'utf8'), context);
        const api = context.FTD_FLUX_PUBLICATION, buffer = api.create(27);
        api.publish(buffer, new Float64Array(27).fill(1));
        failView = true;
        assert.throws(() => kind === 'plane' ? api.snapshotSlice(buffer, 3, 0, 0)
            : api.snapshotCells(buffer, 3, [0, 1]), /injected shared view failure/);
        const header = new Int32Array(buffer, 0, 4);
        assert.equal(Atomics.load(header, 1), 0); assert.equal(Atomics.load(header, 2), 0);
        failView = false;
        assert.ok(api.snapshotCells(buffer, 3, [0, 1]).data.every(value => value === 1));
        assert.equal(api.publish(buffer, new Float64Array(27).fill(2)), true);
        assert.equal(api.publish(buffer, new Float64Array(27).fill(3)), true);
    }
});

test('invalid selectors and layouts fail closed before pinning', () => {
    const { proxy, buffer } = boundProxy();
    for (const axis of [-1, 3, NaN, Infinity, '0', null, 0.1]) {
        assert.equal(proxy.getFluxSlice(axis, 0).length, 0);
        assert.throws(() => publication.snapshotSlice(buffer, 7, axis, 0), RangeError);
    }
    for (const index of [NaN, Infinity, '0', null, 0.1, Number.MAX_SAFE_INTEGER + 1]) {
        assert.equal(proxy.getFluxSlice(0, index).length, 0);
    }
    for (const indices of [null, {}, [], new Array(129).fill(0), [NaN], [1.1], [-1], [343], ['0'], [0n]]) {
        assert.equal(proxy.sampleFluxAtCells(indices).length, 0);
        assert.throws(() => publication.snapshotCells(buffer, 7, indices), RangeError);
    }
    for (const N of [0, -1, 1.1, NaN, Infinity, 2, Number.MAX_SAFE_INTEGER]) {
        assert.throws(() => publication.snapshotSlice(buffer, N, 0, 0), RangeError);
    }
    const header = new Int32Array(buffer, 0, 4);
    assert.equal(Atomics.load(header, 1), 0); assert.equal(Atomics.load(header, 2), 0);
});

test('adversarial probe access cannot attach new-generation provenance to old data', () => {
    const { proxy, buffer } = boundProxy();
    const indices = [0];
    Object.defineProperty(indices, 0, { get() { proxy._pendingConfigurationToken++; return 0; } });
    assert.equal(proxy.sampleFluxAtCells(indices).length, 0);
    assert.equal(Atomics.load(new Int32Array(buffer, 0, 4), 2), 0);
});

test('configuration barrier, resize, legacy buffers and disposal reject stale observations', () => {
    const { proxy } = boundProxy();
    const retained = proxy.getFluxSlice(1, 2), before = retained.slice();
    proxy._pendingConfigurationToken++;
    assert.equal(proxy.getFluxSlice(0, 0).length, 0);
    assert.equal(proxy.sampleFluxAtCells([0]).length, 0);
    proxy._clearScientificGenerationCaches();
    proxy.latticeSize = 3;
    const buffer = publication.create(27);
    publication.publish(buffer, new Float64Array(27).fill(17));
    proxy._bindFlux({ fluxSab: buffer, fluxLen: 27, doubleBuffered: true, fluxProtocol: publication.PROTOCOL });
    proxy._ready = true; proxy._appliedConfigurationToken = proxy._pendingConfigurationToken;
    assert.equal(proxy.getFluxSlice(0, 0).length, 9);
    assert.equal(proxy.sampleFluxAtCells([26])[0], 17);
    for (const field of ['_disposing', '_terminated']) {
        proxy[field] = true;
        assert.equal(proxy.getFluxSlice(0, 0).length, 0);
        assert.equal(proxy.sampleFluxAtCells([0]).length, 0);
        assert.equal(proxy.getFluxVolume().length, 0);
        proxy[field] = false;
    }
    proxy._bindFlux({ heap: new SharedArrayBuffer(27 * 8), fluxPtr: 0, fluxLen: 27 });
    assert.equal(proxy.sampleFluxAtCells([0]).length, 0);
    assertBytes(retained, before);
});

function legacyDecay(data, L, cx, cy, cz) {
    const output = [], wrap = v => ((Math.round(v) % L) + L) % L;
    for (const r of [1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5]) {
        const values = [];
        for (let i = 0; i < 16; i++) {
            const theta = i * 2.0 * Math.PI / 16;
            values.push(data[(wrap(cz) * L + wrap(cy + r * Math.sin(theta))) * L + wrap(cx + r * Math.cos(theta))]);
        }
        const mean = values.reduce((sum, value) => sum + value, 0) / values.length;
        const variance = values.reduce((sum, value) => sum + (value - mean) ** 2, 0) / values.length;
        output.push({ r, aniso: Math.sqrt(variance) / mean * 100, mean });
    }
    return output;
}

test('bounded circular probes reproduce the original ring diagnostic exactly across periodic seams', () => {
    for (const size of [3, 7, 33, 97]) for (const center of [[0, 0, 0], [-0.5, size - 0.5, size + 0.5], [size / 2, size / 2, size / 2]]) {
        const { proxy, data } = boundProxy(size);
        const indices = circularFluxProbeIndices(size, ...center);
        assert.equal(indices.length, 128);
        assert.deepEqual(circularFluxDecay(proxy.sampleFluxAtCells(indices)), legacyDecay(data, size, ...center));
    }
});

test('P1 uses one bounded read per cadence and does not rebuild retained SVG/description', () => {
    const { proxy, data } = boundProxy(7);
    let reads = 0, plots = 0, descriptions = 0;
    const original = proxy.sampleFluxAtCells.bind(proxy);
    proxy.sampleFluxAtCells = cells => { reads++; assert.equal(cells.length, 128); return original(cells); };
    proxy.getFluxVolume = () => { throw Error('whole-volume P1 read'); };
    const instance = Object.create(AnisotropyComponent.prototype);
    Object.assign(instance, { _lastBridge: null, _lastSourceKey: '', _lastSampleAt: -Infinity,
        refs: { plot: {}, desc: { set innerHTML(value) { descriptions++; } } },
        _renderAnisotropyDecay() { plots++; } });
    instance.update(proxy, 0, []);
    assert.deepEqual(instance._decayPoints, legacyDecay(data, 7, 3.5, 3.5, 3.5));
    for (const now of [1, 20, 100, 249]) instance.update(proxy, now, []);
    assert.equal(reads, 1); assert.equal(plots, 1); assert.equal(descriptions, 1);
    instance.update(proxy, 250, []);
    assert.equal(reads, 2); assert.equal(plots, 2);
    proxy._pendingConfigurationToken++;
    instance.update(proxy, 251, []);
    assert.equal(instance._decayPoints, null, 'new source invalidates retained observation before next cadence');
    assert.equal(reads, 3); assert.equal(plots, 3);
    instance.update(proxy, 252, []);
    assert.equal(plots, 3); assert.equal(descriptions, 3);
});

test('P1 native compact/direct fallback and nonfinite/zero availability remain unchanged', () => {
    const instance = Object.create(AnisotropyComponent.prototype);
    const data = dataFor(7);
    assert.deepEqual(instance._computeDecayPoints(makeFluxMagnitudeSampler(data, 7), 3.5, 3.5, 3.5),
        legacyDecay(data, 7, 3.5, 3.5, 3.5));
    assert.equal(circularFluxDecay(new Float64Array(128)), null);
    for (const value of [NaN, Infinity, -Infinity]) {
        const samples = new Float64Array(128).fill(1); samples[20] = value;
        assert.equal(circularFluxDecay(samples), null);
    }
    assert.equal(circularFluxDecay(new Float64Array(127)), null);
    let volumes = 0;
    const bridge = { latticeSize: 8, isNativeGPU: true,
        getFluxVolume() { volumes++; return { data: new Float32Array(64).fill(2), axisCount: 4, stride: 2, latticeSize: 8 }; } };
    Object.assign(instance, { _lastBridge: null, _lastSourceKey: '', _lastSampleAt: -Infinity,
        refs: { plot: {}, desc: {} }, _renderAnisotropyDecay() {} });
    instance.update(bridge, 0, []); instance.update(bridge, 250, []); instance.update(bridge, 1000, []);
    assert.equal(volumes, 2); assert.equal(instance._sampleStride, 2);
    assert.ok(instance._decayPoints.every(point => point.aniso === 0 && point.mean === 2));
});
