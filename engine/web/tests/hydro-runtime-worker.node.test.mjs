// Protocol-only tests with a deterministic fake adapter. No WASM physics claim.
// Mirrors strict-runtime-worker.node.test.mjs; the two contract differences (string
// checkpoints instead of Uint8Array, and the tableUrl/loadTable step during 'init')
// are exercised explicitly rather than assumed.
import assert from 'node:assert/strict';
import test from 'node:test';
import { createHydroWorkerProtocol, createWasmHydroAdapter, loadHydroTable } from '../js/strict/hydro-worker-protocol.js';

function fake() {
    let ticks = 0n;
    let disposed = false;
    let barrier = null;
    return {
        pause(promise) { barrier = promise; },
        async advance(count) {
            if (barrier) await barrier;
            if (count === '42') throw new Error('injected atomic failure');
            ticks += BigInt(count);
            return { microtick: ticks };
        },
        checkpoint() { return `{"microtick":${ticks}}`; },
        restore(json) {
            const parsed = JSON.parse(json);
            if (typeof parsed.microtick !== 'number' || parsed.microtick < 0) throw new Error('invalid checkpoint');
            ticks = BigInt(parsed.microtick);
            return { microtick: ticks };
        },
        observe(width, observable) {
            if (observable !== 'counts' || width === '0') throw new Error('unsupported observation');
            return { microtick: ticks, width, count: 2n ** 80n, nested: [3n, -5n] };
        },
        diagnostics() { return { microtick: ticks, disposed }; },
        capabilities() { return { backend: 'fake_protocol_only', observables: ['counts', 'fields', 'moments'] }; },
        dispose() { disposed = true; },
    };
}

const req = (op, generation = '0', fields = {}) => ({
    op, generation, ownerId: 'owner-A', requestId: `request-${op}`, ...fields,
});

test('hello and exact observations publish external owner/generation', async () => {
    const p = createHydroWorkerProtocol(fake(), { ownerId: 'owner-A' });
    const hello = await p.dispatch({ op: 'hello', requestId: 'init' });
    assert.equal(hello.ownerId, 'owner-A');
    assert.equal(hello.generation, '0');
    assert.equal(hello.payload.capabilities.backend, 'fake_protocol_only');
    assert.deepEqual(hello.payload.capabilities.observables, ['counts', 'fields', 'moments']);
    const observed = await p.dispatch(req('observe', '0', { width: '2', observable: 'counts' }));
    assert.equal(observed.payload.count, (2n ** 80n).toString());
    assert.deepEqual(observed.payload.nested, ['3', '-5']);
    assert.doesNotThrow(() => JSON.stringify(observed));
    assert.equal(observed.payload.microtick, '0');
});

test('queued mutations serialize and verify generation when executed', async () => {
    const adapter = fake();
    let release;
    adapter.pause(new Promise(resolve => { release = resolve; }));
    const p = createHydroWorkerProtocol(adapter, { ownerId: 'owner-A' });
    const first = p.dispatch(req('advance', '0', { microticks: '2' }));
    const stale = p.dispatch(req('advance', '0', { microticks: '3' }));
    const second = p.dispatch(req('advance', '1', { microticks: '3' }));
    const rejected = assert.rejects(stale, /stale generation/);
    release();
    assert.equal((await first).payload.microtick, '2');
    await rejected;
    assert.equal((await second).payload.microtick, '5');
    assert.equal((await p.dispatch(req('diagnostics', '2'))).payload.microtick, '5');
});

test('failed mutation does not advance generation and queue remains usable', async () => {
    const p = createHydroWorkerProtocol(fake(), { ownerId: 'owner-A' });
    await assert.rejects(p.dispatch(req('advance', '0', { microticks: '42' })), /atomic failure/);
    const observed = await p.dispatch(req('diagnostics'));
    assert.equal(observed.generation, '0');
    assert.equal(observed.payload.microtick, '0');
    const zero = await p.dispatch(req('advance', '0', { microticks: '0' }));
    assert.equal(zero.generation, '0');
});

test('checkpoint/restore carry JSON strings, not Uint8Array', async () => {
    const p = createHydroWorkerProtocol(fake(), { ownerId: 'owner-A' });
    const restored = p.dispatch(req('restore', '0', { checkpoint: '{"microtick":9007199254740991}' }));
    assert.equal((await restored).payload.microtick, '9007199254740991');
    const observed = await p.dispatch(req('checkpoint', '1'));
    assert.equal(typeof observed.payload, 'string');
    assert.equal(JSON.parse(observed.payload).microtick, 9007199254740991);
    await assert.rejects(p.dispatch(req('restore', '1', { checkpoint: new Uint8Array([1]) })), /JSON string/);
    await assert.rejects(p.dispatch(req('restore', '1', { checkpoint: '{"microtick":-1}' })), /invalid checkpoint/);
    assert.equal((await p.dispatch(req('diagnostics', '1'))).generation, '1');
});

test('malformed, foreign-owner and unsupported requests never fall back', async () => {
    const p = createHydroWorkerProtocol(fake(), { ownerId: 'owner-A' });
    for (const microticks of [2, 2n, '01', '-1', '1.5', '']) {
        await assert.rejects(p.dispatch(req('advance', '0', { microticks })), /decimal/);
    }
    await assert.rejects(p.dispatch(req('diagnostics', '0', { ownerId: 'owner-B' })), /foreign owner/);
    await assert.rejects(p.dispatch(req('observe', '0', { width: '1', observable: 'planets' })), /unsupported/);
    await assert.rejects(p.dispatch(req('gravity')), /unsupported hydro operation/);
    assert.equal(typeof (await p.dispatch(req('checkpoint'))).payload, 'string');
    assert.equal((await p.dispatch(req('diagnostics'))).generation, '0');
});

test('disposal suppresses an in-flight publication and releases adapter after it', async () => {
    const adapter = fake();
    let release;
    adapter.pause(new Promise(resolve => { release = resolve; }));
    const p = createHydroWorkerProtocol(adapter, { ownerId: 'owner-A' });
    const pending = p.dispatch(req('advance', '0', { microticks: '1' }));
    await Promise.resolve();
    const rejected = assert.rejects(pending, /disposed/);
    const disposed = p.dispatch(req('dispose'));
    assert.equal(adapter.diagnostics().disposed, false);
    release();
    await rejected;
    assert.equal((await disposed).payload.disposed, true);
    assert.equal(adapter.diagnostics().disposed, true);
    await assert.rejects(p.dispatch(req('diagnostics', '1')), /disposed/);
});

test('only the explicitly approximate 4x2 Fourier moments allow floats', async () => {
    const adapter = fake();
    const payload = {
        status: 'approximate_observation', arithmetic: 'ieee754_binary64', error_bound_status: 'not_certified',
        microtick: '9007199254740993', k: ['-1', '0', '0'], polarity: '0',
        moments: [[0.30000000000000004, -1.5e-9], [0, 1], [2, 3], [4, 5]],
    };
    adapter.observe = () => payload;
    const p = createHydroWorkerProtocol(adapter, { ownerId: 'owner-A' });
    const request = req('observe', '0', { width: '1', observable: 'moments:-1,0,0,0' });
    assert.deepEqual((await p.dispatch(request)).payload, payload);
    await assert.rejects(p.dispatch({ ...request, observable: 'counts' }), /inexact integer/);
    for (const invalid of [NaN, Infinity, -Infinity]) {
        adapter.observe = () => ({ ...payload, moments: [[invalid, 0], ...payload.moments.slice(1)] });
        await assert.rejects(p.dispatch(request), /invalid approximate/);
    }
    adapter.observe = () => ({ ...payload, status: 'exact_observation' });
    await assert.rejects(p.dispatch(request), /invalid approximate/);
    adapter.observe = () => ({ ...payload, microtick: 1.25 });
    await assert.rejects(p.dispatch(request), /inexact integer/);
    adapter.observe = () => ({ ...payload, microtick: '1.25' });
    await assert.rejects(p.dispatch(request), /canonical unsigned decimal/);
    adapter.observe = () => ({ ...payload, k: ['-0', '0', '0'] });
    await assert.rejects(p.dispatch(request), /canonical signed decimal/);
});

test('exact observations normalize safe integers and reject precision loss', async () => {
    const adapter = fake();
    const p = createHydroWorkerProtocol(adapter, { ownerId: 'owner-A' });
    const request = req('observe', '0', { width: '1', observable: 'fields' });
    adapter.observe = () => ({ microtick: 7, momentum: [1, -1, 0] });
    assert.deepEqual((await p.dispatch(request)).payload, { microtick: '7', momentum: ['1', '-1', '0'] });
    for (const value of [2 ** 53, 0.5, NaN, Infinity]) {
        adapter.observe = () => ({ microtick: value });
        await assert.rejects(p.dispatch(request), /inexact integer/);
    }
});

test('browser batch limits reject huge valid integers before mutation', async () => {
    const p = createHydroWorkerProtocol(fake(), { ownerId: 'owner-A', maxBatchMicroticks: '4' });
    const hello = await p.dispatch({ requestId: 'init', op: 'hello' });
    assert.equal(hello.payload.capabilities.maxBatchMicroticks, '4');
    for (const microticks of ['5', '18446744073709551615']) {
        await assert.rejects(p.dispatch(req('advance', '0', { microticks })), /maxBatchMicroticks/);
    }
    assert.equal((await p.dispatch(req('diagnostics'))).payload.microtick, '0');
    assert.equal((await p.dispatch(req('advance', '0', { microticks: '4' }))).generation, '1');
    for (const maxBatchMicroticks of ['0', '65', 4]) {
        assert.throws(() => createHydroWorkerProtocol(fake(), { ownerId: 'x', maxBatchMicroticks }));
    }
});

test('WASM adapter delegates JSON-string checkpoints and deletes exactly once', () => {
    let deleted = 0;
    let received;
    class FakeCompiled {
        constructor(json) { received = json; }
        checkpoint() { return received; }
        diagnostics() { return '{"microtick":"0"}'; }
        advance(ticks) { assert.equal(typeof ticks, 'string'); return '{"microtick":"1"}'; }
        restore(json) { received = json; }
        observe(width, name) { return JSON.stringify({ width, name }); }
        delete() { deleted += 1; }
    }
    const checkpoint = '{"schema":"ftd-hydro-checkpoint-1"}';
    const adapter = createWasmHydroAdapter({ HydroState: FakeCompiled }, checkpoint);
    assert.equal(adapter.checkpoint(), checkpoint);
    assert.deepEqual(adapter.advance('1'), { microtick: '1' });
    assert.deepEqual(adapter.observe('2', 'fields'), { width: '2', name: 'fields' });
    assert.deepEqual(adapter.capabilities().observables, ['counts', 'fields', 'moments', 'fluid']);
    assert.equal(adapter.capabilities().observableSyntax.moments, 'moments:kx,ky,kz,pol');
    assert.deepEqual(adapter.capabilities().observationPrecision.moments, {
        status: 'approximate_observation', arithmetic: 'ieee754_binary64', error_bound_status: 'not_certified',
    });
    assert.equal(adapter.capabilities().gravityRecovery, false);
    adapter.dispose(); adapter.dispose();
    assert.equal(deleted, 1);
    assert.throws(() => adapter.checkpoint(), /disposed/);
    assert.throws(() => createWasmHydroAdapter({ HydroState: FakeCompiled }, new Uint8Array([1])), /JSON string/);
});

test('loadHydroTable fetches the blob and installs it before any state exists', async () => {
    let fetched;
    const bytes = new Uint8Array([1, 2, 3, 4]);
    const fetchImpl = async (url) => {
        fetched = url;
        return { ok: true, arrayBuffer: async () => bytes.buffer };
    };
    let installed;
    const module = { loadTable(input) { installed = input; } };
    await loadHydroTable(module, 'https://worker.example/table.bin', { fetchImpl });
    assert.equal(fetched, 'https://worker.example/table.bin');
    assert.deepEqual(Array.from(installed), [1, 2, 3, 4]);
    await assert.rejects(loadHydroTable(module, ''), /tableUrl is required/);
    const failing = async () => ({ ok: false, status: 404 });
    await assert.rejects(loadHydroTable(module, 'https://x/y', { fetchImpl: failing }), /hydro table fetch failed/);
});

test('worker initialization fetches the table before constructing any state', async () => {
    const previousSelf = globalThis.self;
    const previousFetch = globalThis.fetch;
    const messages = [];
    let listener;
    let release;
    globalThis.__hydroProtocolModuleReady = new Promise(resolve => { release = resolve; });
    globalThis.__hydroProtocolFactoryCalls = 0;
    globalThis.__hydroProtocolLoadTableCalls = 0;
    globalThis.self = {
        location: { href: 'file:///hydro-worker.js', origin: 'null' },
        addEventListener(name, callback) { assert.equal(name, 'message'); listener = callback; },
        postMessage(message) { messages.push(message); },
    };
    globalThis.fetch = async () => ({ ok: true, arrayBuffer: async () => new Uint8Array([9]).buffer });
    const fakeModule = `
        await globalThis.__hydroProtocolModuleReady;
        export default async function() {
            globalThis.__hydroProtocolFactoryCalls += 1;
            return {
                loadTable(bytes) { globalThis.__hydroProtocolLoadTableCalls += 1; if (bytes.length !== 1) throw new Error('bad table'); },
                HydroState: class {
                    diagnostics() { return '{"microtick":"0"}'; }
                    delete() {}
                },
            };
        }
    `;
    try {
        await import('../js/strict/hydro-worker.js');
        const wasmModuleUrl = `data:text/javascript,${encodeURIComponent(fakeModule)}`;
        const first = listener({ data: {
            op: 'init', requestId: 'first', wasmModuleUrl, tableUrl: 'file:///table.bin',
            checkpoint: '{"schema":"ftd-hydro-checkpoint-1"}',
        } });
        release();
        await first;
        const initialized = messages.find(m => m.requestId === 'first');
        assert.equal(initialized.ok, true);
        assert.equal(initialized.generation, '0');
        assert.equal(globalThis.__hydroProtocolFactoryCalls, 1);
        assert.equal(globalThis.__hydroProtocolLoadTableCalls, 1);
        await listener({ data: {
            op: 'init', requestId: 'second', wasmModuleUrl, tableUrl: 'file:///table.bin', checkpoint: '{}',
        } });
        assert.equal(messages.find(m => m.requestId === 'second').ok, false);
        assert.match(messages.find(m => m.requestId === 'second').error, /already initialized/);
        await listener({ data: req('dispose', '0', { ownerId: initialized.ownerId }) });
        assert.equal(messages.at(-1).payload.disposed, true);
    } finally {
        release();
        globalThis.self = previousSelf;
        globalThis.fetch = previousFetch;
        delete globalThis.__hydroProtocolModuleReady;
        delete globalThis.__hydroProtocolFactoryCalls;
        delete globalThis.__hydroProtocolLoadTableCalls;
    }
});
