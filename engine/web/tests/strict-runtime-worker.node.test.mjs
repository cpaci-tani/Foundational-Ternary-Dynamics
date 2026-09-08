// Protocol-only tests with a deterministic fake adapter. No WASM physics claim.
import assert from 'node:assert/strict';
import test from 'node:test';
import { runInNewContext } from 'node:vm';
import { createStrictWorkerProtocol, createWasmStrictAdapter } from '../js/strict/worker-protocol.js';

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
        checkpoint() { return new TextEncoder().encode(ticks.toString()); },
        restore(bytes) {
            const text = new TextDecoder().decode(bytes);
            if (!/^[0-9]+$/.test(text)) throw new Error('invalid checkpoint');
            ticks = BigInt(text);
            return { microtick: ticks };
        },
        observe(width, observable) {
            if (observable !== 'counts' || width === '0') throw new Error('unsupported observation');
            return { microtick: ticks, width, count: 2n ** 80n, nested: [3n, -5n] };
        },
        diagnostics() { return { microtick: ticks, disposed }; },
        capabilities() { return { backend: 'fake_protocol_only', gravity: false }; },
        dispose() { disposed = true; },
    };
}

const req = (op, generation = '0', fields = {}) => ({
    op, generation, ownerId: 'owner-A', requestId: `request-${op}`, ...fields,
});

test('hello and exact observations publish external owner/generation', async () => {
    const p = createStrictWorkerProtocol(fake(), { ownerId: 'owner-A' });
    const hello = await p.dispatch({ op: 'hello', requestId: 'init' });
    assert.equal(hello.ownerId, 'owner-A');
    assert.equal(hello.generation, '0');
    assert.equal(hello.payload.capabilities.backend, 'fake_protocol_only');
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
    const p = createStrictWorkerProtocol(adapter, { ownerId: 'owner-A' });
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
    const p = createStrictWorkerProtocol(fake(), { ownerId: 'owner-A' });
    await assert.rejects(p.dispatch(req('advance', '0', { microticks: '42' })), /atomic failure/);
    const observed = await p.dispatch(req('diagnostics'));
    assert.equal(observed.generation, '0');
    assert.equal(observed.payload.microtick, '0');
    const zero = await p.dispatch(req('advance', '0', { microticks: '0' }));
    assert.equal(zero.generation, '0');
});

test('restore copies request checkpoint before queued execution', async () => {
    const p = createStrictWorkerProtocol(fake(), { ownerId: 'owner-A' });
    const checkpoint = new TextEncoder().encode('9007199254740993');
    const restored = p.dispatch(req('restore', '0', { checkpoint }));
    checkpoint.fill(48);
    assert.equal((await restored).payload.microtick, '9007199254740993');
    const observed = await p.dispatch(req('checkpoint', '1'));
    assert(observed.payload instanceof Uint8Array);
    observed.payload.fill(48);
    assert.equal((await p.dispatch(req('diagnostics', '1'))).payload.microtick, '9007199254740993');
    await assert.rejects(p.dispatch(req('restore', '1', { checkpoint: new Uint8Array([255]) })), /invalid checkpoint/);
    assert.equal((await p.dispatch(req('diagnostics', '1'))).generation, '1');
});

test('shared checkpoints reject at receipt before queued restore or constructor', async () => {
    const adapter = fake();
    const p = createStrictWorkerProtocol(adapter, { ownerId: 'owner-A' });
    const checkpoint = new Uint8Array(new SharedArrayBuffer(1));
    checkpoint[0] = 55;
    await assert.rejects(p.dispatch(req('restore', '0', { checkpoint })), /SharedArrayBuffer/);
    checkpoint[0] = 57;
    assert.equal((await p.dispatch(req('diagnostics'))).payload.microtick, '0');
    let constructed = false;
    assert.throws(() => createWasmStrictAdapter({ StrictState: class { constructor() { constructed = true; } } }, checkpoint), /SharedArrayBuffer/);
    assert.equal(constructed, false);
    const foreignShared = runInNewContext('new SharedArrayBuffer(1)');
    assert.equal(foreignShared instanceof SharedArrayBuffer, false);
    Object.defineProperty(foreignShared, Symbol.toStringTag, { value: 'ArrayBuffer' });
    const foreignView = new Uint8Array(foreignShared);
    await assert.rejects(p.dispatch(req('restore', '0', { checkpoint: foreignView })), /SharedArrayBuffer/);
    assert.throws(() => createWasmStrictAdapter({ StrictState: class {} }, foreignView), /SharedArrayBuffer/);
    Object.defineProperty(foreignView, 'buffer', { value: new ArrayBuffer(0) });
    assert.throws(() => createWasmStrictAdapter({ StrictState: class {} }, foreignView), /SharedArrayBuffer/);
});

test('malformed, foreign-owner and unsupported requests never fall back', async () => {
    const p = createStrictWorkerProtocol(fake(), { ownerId: 'owner-A' });
    for (const microticks of [2, 2n, '01', '-1', '1.5', '']) {
        await assert.rejects(p.dispatch(req('advance', '0', { microticks })), /decimal/);
    }
    await assert.rejects(p.dispatch(req('diagnostics', '0', { ownerId: 'owner-B' })), /foreign owner/);
    await assert.rejects(p.dispatch(req('observe', '0', { width: '1', observable: 'planets' })), /unsupported/);
    await assert.rejects(p.dispatch(req('gravity')), /unsupported strict operation/);
    await assert.rejects(p.dispatch(req('restore', '0', { checkpoint: [0] })), /Uint8Array/);
    assert.equal((await p.dispatch(req('diagnostics'))).generation, '0');
});

test('disposal suppresses an in-flight publication and releases adapter after it', async () => {
    const adapter = fake();
    let release;
    adapter.pause(new Promise(resolve => { release = resolve; }));
    const p = createStrictWorkerProtocol(adapter, { ownerId: 'owner-A' });
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

test('unsafe adapter integers are rejected rather than silently rounded', async () => {
    const adapter = fake();
    adapter.observe = () => ({ broken: Number.MAX_SAFE_INTEGER + 1 });
    const p = createStrictWorkerProtocol(adapter, { ownerId: 'owner-A' });
    await assert.rejects(p.dispatch(req('observe', '0', { width: '1', observable: 'counts' })), /inexact number/);
});

test('browser batch limits reject huge valid integers before mutation', async () => {
    const p = createStrictWorkerProtocol(fake(), { ownerId: 'owner-A', maxBatchMicroticks: '4' });
    const hello = await p.dispatch({ requestId: 'init', op: 'hello' });
    assert.equal(hello.payload.capabilities.maxBatchMicroticks, '4');
    for (const microticks of ['5', '18446744073709551615']) {
        await assert.rejects(p.dispatch(req('advance', '0', { microticks })), /maxBatchMicroticks/);
    }
    assert.equal((await p.dispatch(req('diagnostics'))).payload.microtick, '0');
    assert.equal((await p.dispatch(req('advance', '0', { microticks: '4' }))).generation, '1');
    for (const maxBatchMicroticks of ['0', '65', 4]) {
        assert.throws(() => createStrictWorkerProtocol(fake(), { ownerId: 'x', maxBatchMicroticks }));
    }
});

test('WASM adapter delegates, copies checkpoints and deletes exactly once', () => {
    let deleted = 0;
    let received;
    class FakeCompiled {
        constructor(bytes) { received = bytes; }
        checkpoint() { return received; }
        diagnostics() { return '{"microtick":"0"}'; }
        advance(ticks) { assert.equal(typeof ticks, 'string'); return '{"microtick":"1"}'; }
        restore(bytes) { received = bytes; }
        observe(width, name) { return JSON.stringify({ width, name }); }
        delete() { deleted += 1; }
    }
    const checkpoint = new Uint8Array([1, 2]);
    const adapter = createWasmStrictAdapter({ StrictState: FakeCompiled }, checkpoint);
    checkpoint[0] = 99;
    assert.deepEqual(adapter.checkpoint(), new Uint8Array([1, 2]));
    assert.deepEqual(adapter.advance('1'), { microtick: '1' });
    assert.deepEqual(adapter.observe('2', 'counts'), { width: '2', name: 'counts' });
    assert.equal(adapter.capabilities().gravityRecovery, false);
    adapter.dispose(); adapter.dispose();
    assert.equal(deleted, 1);
    assert.throws(() => adapter.checkpoint(), /disposed/);
});

test('worker initialization admits one import/factory/owner during concurrent requests', async () => {
    const previousSelf = globalThis.self;
    const messages = [];
    let listener;
    let release;
    globalThis.__strictProtocolModuleReady = new Promise(resolve => { release = resolve; });
    globalThis.__strictProtocolFactoryCalls = 0;
    globalThis.self = {
        location: { href: 'file:///strict-worker.js', origin: 'null' },
        addEventListener(name, callback) { assert.equal(name, 'message'); listener = callback; },
        postMessage(message) { messages.push(message); },
    };
    const fakeModule = `
        await globalThis.__strictProtocolModuleReady;
        export default async function() {
            globalThis.__strictProtocolFactoryCalls += 1;
            return { StrictState: class {
                diagnostics() { return '{"microtick":"0"}'; }
                delete() {}
            }};
        }
    `;
    try {
        await import('../js/strict/strict-worker.js');
        const wasmModuleUrl = `data:text/javascript,${encodeURIComponent(fakeModule)}`;
        await listener({ data: { op: 'init', requestId: 'shared', wasmModuleUrl,
            checkpoint: new Uint8Array(new SharedArrayBuffer(1)) } });
        assert.equal(messages.find(m => m.requestId === 'shared').ok, false);
        assert.match(messages.find(m => m.requestId === 'shared').error, /SharedArrayBuffer/);
        const first = listener({ data: { op: 'init', requestId: 'first', wasmModuleUrl, checkpoint: new Uint8Array([0]) } });
        await listener({ data: { op: 'init', requestId: 'second', wasmModuleUrl, checkpoint: new Uint8Array([0]) } });
        await listener({ data: req('diagnostics') });
        assert.equal(messages.find(m => m.requestId === 'second').ok, false);
        assert.match(messages.find(m => m.requestId === 'second').error, /initializing/);
        release();
        await first;
        const initialized = messages.find(m => m.requestId === 'first');
        assert.equal(initialized.ok, true);
        assert.equal(initialized.generation, '0');
        assert.equal(globalThis.__strictProtocolFactoryCalls, 1);
        await listener({ data: { op: 'init', requestId: 'third', wasmModuleUrl, checkpoint: new Uint8Array([0]) } });
        assert.equal(messages.find(m => m.requestId === 'third').ok, false);
        await listener({ data: req('dispose', '0', { ownerId: initialized.ownerId }) });
        assert.equal(messages.at(-1).payload.disposed, true);
    } finally {
        release();
        globalThis.self = previousSelf;
        delete globalThis.__strictProtocolModuleReady;
        delete globalThis.__strictProtocolFactoryCalls;
    }
});
