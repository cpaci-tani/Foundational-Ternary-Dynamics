import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import '../js/bridge/flux-publication.classic.js';
import { WasmBridgeProxy } from '../js/bridge/wasm-bridge-proxy.js';
import { createScale0Capabilities } from '../js/bridge/capabilities/scale0.js';
import { gravitySlice, gravitySliceFromSlab } from '../js/scales/scale0/analysis/gravity-analysis.js';
import { gravitySlice as originalSlice, maxRhoOf } from './fixtures/gravity-dense-v3-oracle.mjs';
import { transposeAndFlipNN, paintSliceToCanvas } from '../js/scales/scale0/ui/overlays/slice-render.js';
import { rampViridis, rampEmEnergy, rampVorticity } from '../js/viewport/color-ramps.js';

const pub = globalThis.FTD_FLUX_PUBLICATION;
const kinds = ['latency', 'dilation', 'kretschmann', 'force'];
const ramps = { latency: rampViridis, dilation: rampViridis, kretschmann: rampEmEnergy, force: rampVorticity };
function boundProxy(N = 7, data = field(N, 'asymmetric')) {
    const proxy = Object.create(WasmBridgeProxy.prototype);
    Object.assign(proxy, { latticeSize: N, _ready: true, _terminated: false, _disposing: false,
        _pendingConfigurationToken: 2, _appliedConfigurationToken: 2, _fluxBufferGeneration: 0,
        _digestPending: new Map(), _inspectionCache: new Map(), _inspectionPending: new Map() });
    const buffer = pub.create(N ** 3);
    pub.publish(buffer, data);
    proxy._bindFlux({ fluxSab: buffer, fluxLen: data.length, doubleBuffered: true, fluxProtocol: pub.PROTOCOL });
    return { proxy, buffer, data };
}
function field(N, kind) {
    const data = new Float64Array(N ** 3);
    if (kind !== 'zero') {
        for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
            data[(z * N + y) * N + x] = kind === 'off-slab-max' ? 1 : (1 + x * 3 + y * 7 + z * 13) / 17;
        }
    }
    if (kind === 'off-slab-max') data[0] = 97;
    return data;
}
function bytes(value) { return new Uint8Array(value.buffer, value.byteOffset, value.byteLength); }
function equalBytes(a, b) { assert.deepEqual(bytes(a), bytes(b)); }
function expectedSlab(data, N, axis, index) {
    const startPlane = Math.max(0, index - 1), end = Math.min(N - 1, index + 1);
    const out = new Float64Array((end - startPlane + 1) * N * N);
    for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
        const plane = [x, y, z][axis];
        if (plane < startPlane || plane > end) continue;
        const a = axis === 0 ? y : x, b = axis === 2 ? y : z;
        out[(plane - startPlane) * N * N + a * N + b] = data[(z * N + y) * N + x];
    }
    return out;
}
function pixelOracle(raw, N, ramp) {
    const plane = new Float64Array(N * N);
    for (let a = 0; a < N; a++) for (let b = 0; b < N; b++) plane[(N - 1 - b) * N + a] = raw[a * N + b];
    let max = 0;
    for (const value of plane) if (value > max) max = value;
    const norm = max > 1e-30 ? 1 / max : 1;
    const rgba = new Uint8ClampedArray(N * N * 4), rgb = [0, 0, 0];
    for (let i = 0; i < plane.length; i++) {
        let t = plane[i] * norm;
        if (t > 1) t = 1; else if (t < 0) t = 0;
        ramp(t, rgb, 0);
        rgba[i * 4] = (rgb[0] * 255) | 0;
        rgba[i * 4 + 1] = (rgb[1] * 255) | 0;
        rgba[i * 4 + 2] = (rgb[2] * 255) | 0;
        rgba[i * 4 + 3] = 255;
    }
    return { plane, rgba, norm };
}
function canvasFixture() {
    const ctx = { draws: [], drawImage(...args) { this.draws.push(args); }, fillRect() {} };
    const canvas = { width: 116, height: 116, getContext: () => ctx };
    return { canvas, ctx };
}

for (const N of [1, 2, 3, 7]) for (const fieldKind of ['zero', 'asymmetric', 'off-slab-max']) {
    test(`complete slab, dense-array and pixel equality: N=${N}, field=${fieldKind}`, () => {
        const { proxy, data } = boundProxy(N, field(N, fieldKind));
        const oldDocument = globalThis.document, oldImageData = globalThis.ImageData;
        globalThis.ImageData = class { constructor(data, width, height) { Object.assign(this, { data, width, height }); } };
        globalThis.document = { createElement: () => ({ getContext: () => ({ putImageData() {} }) }) };
        try {
            for (const index of new Set([0, Math.floor(N / 2), N - 1])) {
                const requests = [0, 1, 2].map(axis => ({ axis, index }));
                const batch = proxy.getFluxSlabsWithMaxRho(requests);
                assert.equal(batch.maxRho, maxRhoOf(data, N ** 3));
                assert.equal(batch.slabs.length, 3);
                for (const [axis, slab] of batch.slabs.entries()) {
                    equalBytes(slab.data, expectedSlab(data, N, axis, index));
                    assert.equal(slab.data.buffer.byteLength, slab.planeCount * N * N * 8);
                    assert.ok(slab.data.buffer instanceof ArrayBuffer);
                    assert.strictEqual(slab.metadata, batch.metadata);
                    for (const kind of kinds) {
                        const expected = originalSlice(data, N, axis, index, kind);
                        const actual = gravitySliceFromSlab(slab, kind);
                        equalBytes(actual, expected);
                        equalBytes(gravitySlice(data, N, axis, index, kind), expected);
                        const oracle = pixelOracle(expected, N, ramps[kind]);
                        const finalPlane = transposeAndFlipNN(actual, N);
                        equalBytes(finalPlane, oracle.plane);
                        const { canvas, ctx } = canvasFixture();
                        paintSliceToCanvas(canvas, finalPlane, N, { ramp: ramps[kind], norm: oracle.norm });
                        equalBytes(canvas._ftdSlice.buf, oracle.rgba);
                        assert.equal(ctx.imageSmoothingEnabled, false);
                        assert.equal(canvas._ftdSlice.tctx.imageSmoothingEnabled, false);
                        assert.deepEqual(ctx.draws[0].slice(1), [0, 0, N, N, 0, 0, 116, 116]);
                    }
                }
            }
            assert.equal(proxy._fluxSnapshot, null, 'slab reads must not populate the dense copy cache');
        } finally {
            if (oldDocument === undefined) delete globalThis.document; else globalThis.document = oldDocument;
            if (oldImageData === undefined) delete globalThis.ImageData; else globalThis.ImageData = oldImageData;
        }
    });
}

test('an off-slab maximum controls normalization; border stencils retain zeros', () => {
    const { proxy } = boundProxy(7, field(7, 'off-slab-max'));
    const batch = proxy.getFluxSlabsWithMaxRho([0, 1, 2].map(axis => ({ axis, index: 3 })));
    for (const slab of batch.slabs) {
        assert.ok(slab.data.every(value => value === 1));
        assert.equal(slab.maxRho, 97 * 97);
        assert.equal(gravitySliceFromSlab(slab, 'latency')[0], Math.sqrt(1 / (97 * 97)));
        for (const kind of ['kretschmann', 'force']) {
            const plane = gravitySliceFromSlab(slab, kind);
            for (let a = 0; a < 7; a++) for (let b = 0; b < 7; b++) {
                if (a === 0 || b === 0 || a === 6 || b === 6) assert.equal(plane[a * 7 + b], 0);
            }
        }
    }
});

test('dense direct and compact-spacing arithmetic remains byte-identical to the frozen function', () => {
    for (const N of [3, 7]) for (const spacing of [1, 2, 5]) for (const axis of [0, 1, 2]) for (const kind of kinds) {
        const data = field(N, 'asymmetric');
        equalBytes(gravitySlice(data, N, axis, 1, kind, 0, spacing),
            originalSlice(data, N, axis, 1, kind, 0, spacing));
    }
});

test('batch order, duplicate selectors, owned snapshots and deeply frozen common metadata survive rebind', () => {
    const { proxy, buffer } = boundProxy();
    proxy.currentTick = () => 999;
    const requests = [{ axis: 2, index: 0 }, { axis: 0, index: 6 }, { axis: 2, index: 0 }];
    const batch = proxy.getFluxSlabsWithMaxRho(requests);
    assert.deepEqual(batch.slabs.map(s => [s.axis, s.index]), [[2, 0], [0, 6], [2, 0]]);
    const saved = batch.slabs.map(s => s.data.slice());
    equalBytes(batch.slabs[0].data, batch.slabs[2].data);
    assert.notStrictEqual(batch.slabs[0].data.buffer, batch.slabs[2].data.buffer);
    assert.equal(batch.metadata.sampleTick, null);
    assert.equal(batch.metadata.normalizationCellCount, 343);
    assert.equal(batch.metadata.configurationToken, 2);
    assert.equal(batch.metadata.publicationVersion, 1);
    for (const value of [batch, batch.slabs, batch.metadata, batch.metadata.support,
        batch.metadata.support.slabs, ...batch.slabs, ...batch.metadata.support.slabs,
        ...batch.metadata.support.slabs.map(s => s.inPlaneAxes)]) assert.ok(Object.isFrozen(value));
    assert.throws(() => { batch.metadata.sampleTick = 999; }, TypeError);
    assert.throws(() => { batch.metadata.support.slabs[0].axis = 1; }, TypeError);
    requests[0].index = 5;
    for (let n = 2; n < 10; n++) pub.publish(buffer, new Float64Array(343).fill(n));
    const replacement = pub.create(343);
    pub.publish(replacement, new Float64Array(343).fill(61));
    proxy._bindFlux({ fluxSab: replacement, fluxLen: 343, doubleBuffered: true, fluxProtocol: pub.PROTOCOL });
    const current = proxy.getFluxSlabWithMaxRho(0, 1);
    assert.equal(current.maxRho, 61 * 61);
    assert.notEqual(current.metadata.bufferGeneration, batch.metadata.bufferGeneration);
    assert.equal(current.metadata.publicationVersion, batch.metadata.publicationVersion);
    batch.slabs.forEach((s, i) => equalBytes(s.data, saved[i]));
});

function instrumentedPublication({ onRead = null, throwAllocation = 0, throwSharedView = false } = {}) {
    let reads = 0, ownedAllocations = 0, acquisitions = 0;
    let fail = false;
    function Typed(...args) {
        if (args[0] instanceof SharedArrayBuffer && throwSharedView && fail) throw Error('shared view failure');
        if (typeof args[0] === 'number') {
            ownedAllocations++;
            if (fail && ownedAllocations === throwAllocation) throw Error('slab allocation failure');
        }
        const target = new Float64Array(...args);
        if (!(args[0] instanceof SharedArrayBuffer)) return target;
        return new Proxy(target, { get(array, key) {
            if (typeof key === 'string' && /^(0|[1-9][0-9]*)$/.test(key)) { reads++; onRead?.(); }
            const result = Reflect.get(array, key, array);
            return typeof result === 'function' ? result.bind(array) : result;
        } });
    }
    Typed.BYTES_PER_ELEMENT = 8;
    const context = { SharedArrayBuffer, ArrayBuffer, Float64Array: Typed, Int32Array,
        Atomics: { load: Atomics.load, store: Atomics.store, compareExchange(...args) {
            if (args[3] === 1) acquisitions++;
            return Atomics.compareExchange(...args);
        } } };
    vm.runInNewContext(readFileSync(new URL('../js/bridge/flux-publication.classic.js', import.meta.url), 'utf8'), context);
    return { api: context.FTD_FLUX_PUBLICATION, begin() { reads = 0; ownedAllocations = 0; acquisitions = 0; fail = true; },
        get reads() { return reads; }, get acquisitions() { return acquisitions; } };
}

test('three slabs acquire one pin and scan the full maximum once despite concurrent writer advancement', () => {
    let fired = false, writerResults, api, buffer;
    const harness = instrumentedPublication({ onRead() {
        const header = new Int32Array(buffer, 0, 4);
        assert.equal(Atomics.load(header, 2), 1, 'original slot remains pinned during every numeric read');
        if (!fired) {
            fired = true;
            writerResults = [api.publish(buffer, new Float64Array(343).fill(29)),
                api.publish(buffer, new Float64Array(343).fill(31))];
        }
    } });
    api = harness.api; buffer = api.create(343);
    api.publish(buffer, new Float64Array(343).fill(7));
    harness.begin();
    const result = api.snapshotSlabsWithMaxRho(buffer, 7, [0, 1, 2].map(axis => ({ axis, index: 3 })));
    assert.deepEqual(writerResults, [true, false]);
    assert.equal(result.version, 1);
    assert.equal(result.maxRho, 49);
    assert.equal(harness.acquisitions, 1);
    assert.equal(harness.reads, 2 * 343 + 3 * 3 * 49, 'one two-read square scan plus the requested owned copies');
    assert.ok(result.slabs.every(s => s.data.every(value => value === 7)));
    assert.equal(Atomics.load(new Int32Array(buffer, 0, 4), 2), 0);
});

test('all selectors are captured before pinning, including array length and mutable getters', () => {
    const { proxy, buffer } = boundProxy();
    const header = new Int32Array(buffer, 0, 4);
    const first = { axis: 0, index: 1 };
    const second = { get axis() {
        assert.equal(Atomics.load(header, 1), 0); assert.equal(Atomics.load(header, 2), 0);
        first.axis = 2; first.index = 5;
        return 1;
    }, index: 2 };
    let lengthReads = 0;
    const requests = new Proxy([first, second], { get(array, key) {
        if (key === 'length') lengthReads++;
        return Reflect.get(array, key);
    } });
    const batch = proxy.getFluxSlabsWithMaxRho(requests);
    assert.equal(lengthReads, 1);
    assert.deepEqual(batch.slabs.map(s => [s.axis, s.index]), [[0, 1], [1, 2]]);
});

test('invalid selectors, unpublished/contended buffers and invalid slabs never fall back to dense reads', () => {
    const { proxy, buffer } = boundProxy();
    const previous = globalThis.FTD_FLUX_PUBLICATION;
    globalThis.FTD_FLUX_PUBLICATION = { ...pub, snapshot() { throw Error('forbidden dense snapshot'); } };
    proxy.getFluxVolume = () => { throw Error('forbidden dense volume'); };
    try {
        for (const requests of [null, [], {}, Array(4).fill({ axis: 0, index: 1 }),
            [{ axis: -1, index: 1 }], [{ axis: 3, index: 1 }], [{ axis: 1.5, index: 1 }],
            [{ axis: 0, index: -1 }], [{ axis: 0, index: 7 }], [{ axis: 0, index: 1.5 }],
            [{ axis: 0, index: Infinity }], [{ axis: '0', index: 1 }]]) {
            assert.equal(proxy.getFluxSlabsWithMaxRho(requests), null);
        }
        const read = pub.acquire(buffer, 343);
        try {
            assert.equal(proxy.getFluxSlabWithMaxRho(0, 1), null);
            assert.equal(Atomics.load(new Int32Array(buffer, 0, 4), 2), 1);
        } finally { read.release(); }
        assert.ok(proxy.getFluxSlabWithMaxRho(0, 1));
        const fresh = pub.create(343);
        assert.equal(pub.snapshotSlabsWithMaxRho(fresh, 7, [{ axis: 0, index: 1 }]), null);
        assert.equal(gravitySliceFromSlab(null), null);
        const valid = proxy.getFluxSlabWithMaxRho(0, 1);
        for (const bad of [{ ...valid, planeCount: 1 }, { ...valid, startPlane: 1 },
            { ...valid, data: new Float32Array(valid.data) }, { ...valid, maxRho: NaN },
            { ...valid, axis: 3 }, { ...valid, index: 7 }]) assert.equal(gravitySliceFromSlab(bad), null);
    } finally { globalThis.FTD_FLUX_PUBLICATION = previous; }
});

test('shared-view and second-slab allocation exceptions release the sole reader pin', () => {
    for (const options of [{ throwSharedView: true }, { throwAllocation: 2 }]) {
        const h = instrumentedPublication(options), buffer = h.api.create(343);
        h.api.publish(buffer, new Float64Array(343).fill(2));
        h.begin();
        assert.throws(() => h.api.snapshotSlabsWithMaxRho(buffer, 7,
            [{ axis: 0, index: 3 }, { axis: 1, index: 3 }]), /failure/);
        const header = new Int32Array(buffer, 0, 4);
        assert.equal(Atomics.load(header, 1), 0);
        assert.equal(Atomics.load(header, 2), 0);
    }
});

test('owner readiness, configuration, dimensions, binding and disposal fences reject before and after the read', () => {
    const changes = [p => { p._ready = false; }, p => { p._terminated = true; },
        p => { p._disposing = true; }, p => { p._pendingConfigurationToken++; },
        p => { p._appliedConfigurationToken++; }, p => { p.latticeSize++; },
        p => { p._fluxLen--; }, p => { p._fluxDouble = false; },
        p => { p._fluxBufferGeneration++; }, p => { p._fluxSab = pub.create(343); }];
    const saved = globalThis.FTD_FLUX_PUBLICATION;
    try {
        for (const change of changes) {
            const { proxy, buffer } = boundProxy();
            globalThis.FTD_FLUX_PUBLICATION = { ...pub, snapshotSlabsWithMaxRho(...args) {
                const result = pub.snapshotSlabsWithMaxRho(...args);
                change(proxy);
                return result;
            } };
            assert.equal(proxy.getFluxSlabsWithMaxRho([{ axis: 0, index: 1 }]), null);
            assert.equal(Atomics.load(new Int32Array(buffer, 0, 4), 2), 0);
        }
        for (const change of changes.slice(0, 8)) {
            const { proxy } = boundProxy();
            change(proxy);
            globalThis.FTD_FLUX_PUBLICATION = { ...pub, snapshotSlabsWithMaxRho() { throw Error('read past invalid owner fence'); } };
            assert.equal(proxy.getFluxSlabWithMaxRho(0, 1), null);
        }
    } finally { globalThis.FTD_FLUX_PUBLICATION = saved; }
});

test('capabilities expose optional batch/single observers only on supporting owners', () => {
    const { proxy } = boundProxy();
    const caps = createScale0Capabilities(proxy);
    const requests = [{ axis: 2, index: 3 }];
    assert.equal(caps.getScale0FluxSlabsWithMaxRho(requests).slabs[0].axis, 2);
    assert.equal(caps.getScale0FluxSlabWithMaxRho(1, 3).axis, 1);
    const legacy = createScale0Capabilities({});
    assert.equal('getScale0FluxSlabsWithMaxRho' in legacy, false);
    assert.equal('getScale0FluxSlabWithMaxRho' in legacy, false);
    proxy._ready = false;
    assert.equal(caps.getScale0FluxSlabsWithMaxRho(requests), null);
});
