import test from 'node:test';
import assert from 'node:assert/strict';
import { exactCounter, compareExactCounters, safeCounterNumber, normalizeNativeCounters,
    commonSampleProvenance } from '../js/lib/exact-counter.js';
import { decodeNativeBinaryFrame } from '../js/bridge/ws-binary-codec.js';
import { WebSocketBridge } from '../js/ws-bridge.js';
import { TelemetryHub, RingBuffer } from '../js/telemetry-hub.js';
import { FieldLineKnotTracker } from '../js/scales/scale0/runtime/field-line-knots.js';

const INSTANCE = '0123456789abcdef1122334455667788';
const MAX = '18446744073709551615';
const BIG = '9007199254740993';

function payload(type = 'field', token = 1, kindCode = 0) {
    if (type === 'particles') {
        const buffer = new ArrayBuffer(44), view = new DataView(buffer);
        view.setUint32(0, 0x32505446, true); view.setUint32(4, 1, true);
        new Float32Array(buffer, 8).fill(2);
        return buffer;
    }
    if (type === 'volume') {
        const buffer = new ArrayBuffer(24), view = new DataView(buffer);
        [0x32565446, 9, 2, 4, 1].forEach((v, i) => view.setUint32(i * 4, v, true));
        view.setFloat32(20, 7.5, true);
        return buffer;
    }
    const components = kindCode === 3 ? 1 : 3;
    const buffer = new ArrayBuffer(28 + (3 + components) * 4), view = new DataView(buffer);
    [0x32535446, token, kindCode, components, 1, 2, 4].forEach((v, i) => view.setUint32(i * 4, v, true));
    new Float32Array(buffer, 28).fill(3.25);
    return buffer;
}

function envelope(body = payload(), id = 1, metadata = {}) {
    const buffer = new ArrayBuffer(88 + body.byteLength), view = new DataView(buffer);
    view.setUint32(0, 0x334e5446, true); view.setUint32(4, 88, true);
    for (const [offset, value] of [[8, id], [16, metadata.tick ?? BIG],
        [24, metadata.sourceEpoch ?? 4], [32, metadata.epoch ?? 6], [40, body.byteLength]]) {
        view.setBigUint64(offset, BigInt(value), true);
    }
    view.setFloat64(48, 12.5, true); view.setFloat64(56, 0.5, true);
    view.setUint32(64, 9, true); view.setUint32(68, 1, true);
    const instance = metadata.instance ?? INSTANCE;
    view.setBigUint64(72, BigInt(`0x${instance.slice(16)}`), true);
    view.setBigUint64(80, BigInt(`0x${instance.slice(0, 16)}`), true);
    new Uint8Array(buffer, 88).set(new Uint8Array(body));
    return buffer;
}

function fixture(t, v3 = true) {
    const bridge = new WebSocketBridge();
    const sent = [];
    bridge._ws = { send: text => sent.push(JSON.parse(text)), close() {} };
    bridge._connected = true; bridge.ready = true; bridge.latticeSize = 9;
    bridge._nativeBinaryVersion = v3 ? 3 : 2;
    bridge._nativeInstanceId = v3 ? INSTANCE : null;
    bridge._expectedTelemetrySourceEpoch = 4;
    bridge._telemetrySnapshotMeta.epoch = 6;
    bridge._pumpTelemetry = () => false;
    t.after(() => bridge.dispose());
    return { bridge, sent };
}

test('uint64 normalization is exact at safe-number and unsigned-maximum boundaries', () => {
    assert.equal(exactCounter('9007199254740991'), Number.MAX_SAFE_INTEGER);
    assert.equal(exactCounter(9007199254740992n), '9007199254740992');
    assert.equal(exactCounter(MAX), MAX);
    assert.equal(compareExactCounters('9007199254740992', BIG), -1);
    assert.equal(compareExactCounters(MAX, '18446744073709551614'), 1);
    assert.equal(compareExactCounters(4, '4'), 0);
    assert.equal(safeCounterNumber(BIG), null);
    for (const bad of [9007199254740992, -1, NaN, Infinity, '', '01', '+1', '1e3',
        ' 1', '18446744073709551616', {}, null]) assert.equal(exactCounter(bad), null);
    assert.throws(() => normalizeNativeCounters({ groupMeta: { audit: { epoch: 9007199254740992 } } }), TypeError);
    assert.deepEqual(normalizeNativeCounters({ tick: BIG, epoch: '4' }), { tick: BIG, epoch: 4 });
    for (const key of ['backendStateVersion', 'telemetrySnapshotVersion']) {
        assert.throws(() => normalizeNativeCounters({ [key]: 9007199254740992 }), TypeError);
        assert.equal(normalizeNativeCounters({ [key]: MAX })[key], MAX);
    }
});

for (const type of ['particles', 'volume', 'field']) {
    test(`FTN3 ${type} preserves exact metadata and nested zero-copy payload`, () => {
        const buffer = envelope(payload(type));
        const frame = decodeNativeBinaryFrame(buffer);
        assert.equal(frame.type, type);
        assert.equal(frame.requestId, 1);
        assert.equal(frame.provenance.nativeInstanceId, INSTANCE);
        assert.equal(frame.provenance.sampleTick, BIG);
        assert.equal(frame.provenance.status, 'approximate');
        assert.equal(frame.provenance.model, 'production-reference');
        assert.equal(frame.provenance.physicalTime, 12.5);
        assert.ok(Object.isFrozen(frame.provenance));
        const data = type === 'particles' ? frame.data.positions : type === 'volume' ? frame.data.data : frame.positions;
        assert.equal(data.buffer, buffer);
        assert.equal(data.byteOffset, 88 + (type === 'particles' ? 8 : type === 'volume' ? 20 : 28));
    });
}

test('FTN3 rejects truncated, extended, ambiguous and unsupported envelopes before publication', () => {
    for (const change of [v => v.setUint32(4, 80, true), v => v.setBigUint64(8, 0n, true),
        v => v.setBigUint64(8, 9007199254740992n, true), v => v.setUint32(68, 3, true),
        v => v.setBigUint64(40, 1n << 63n, true), v => v.setUint32(88, 0x334e5446, true),
        v => v.setUint32(64, 0, true), v => v.setUint32(104, 100000, true)]) {
        const buffer = envelope(); change(new DataView(buffer));
        assert.equal(decodeNativeBinaryFrame(buffer).type, 'invalid-envelope');
    }
    const frame = envelope();
    assert.equal(decodeNativeBinaryFrame(frame.slice(0, 87)).type, 'invalid-envelope');
    assert.equal(decodeNativeBinaryFrame(frame.slice(0, -1)).type, 'invalid-envelope');
    const extra = new Uint8Array(frame.byteLength + 4); extra.set(new Uint8Array(frame));
    assert.equal(decodeNativeBinaryFrame(extra.buffer).type, 'invalid-envelope');
    const mismatch = envelope(payload('volume')); new DataView(mismatch).setUint32(64, 11, true);
    assert.equal(decodeNativeBinaryFrame(mismatch).type, 'invalid-envelope');
});

test('legacy frames remain decodable with unavailable provenance', () => {
    const frame = decodeNativeBinaryFrame(payload(), { latticeSize: 9 });
    assert.equal(frame.type, 'field');
    assert.equal(frame.provenance.status, 'unavailable');
    assert.equal(frame.provenance.sampleTick, undefined);
});

test('legacy binary reads stay untagged and correlated binary RPCs reject before sending', async t => {
    const { bridge, sent } = fixture(t, false);
    bridge.getEFieldSampled(2);
    assert.equal(sent[0]._requestId, undefined);
    assert.equal(sent[0]._binaryVersion, undefined);
    await assert.rejects(bridge._sendJSON({ cmd: 'get_particles' }), /require native protocol v3/);
    assert.equal(sent.length, 1);
});

test('v3 visual request negotiates format, publishes provenance and rejects duplicate payloads', t => {
    const { bridge, sent } = fixture(t);
    bridge.getEFieldSampled(2);
    assert.equal(sent[0]._binaryVersion, 3);
    const buffer = envelope(payload('field', sent[0].token), sent[0]._requestId);
    bridge._handleBinary(buffer);
    const sample = bridge.getEFieldSampled(2);
    assert.equal(sample.sampleTick, BIG); assert.equal(sample.sampleTickNumber, null);
    assert.equal(sample.nativeInstanceId, INSTANCE); assert.equal(sample.count, 1);
    bridge._handleBinary(buffer);
    assert.equal(bridge._lastWireRejection.reason, 'unknown-or-retired-request');
    assert.equal(bridge.getEFieldSampled(2), sample);
});

test('reordered field responses preserve token, command kind and request identities', t => {
    const { bridge, sent } = fixture(t);
    bridge.getEFieldSampled(2); bridge.getBFieldSampled(2);
    const [e, b] = sent;
    bridge._handleBinary(envelope(payload('field', b.token, 1), b._requestId));
    bridge._handleBinary(envelope(payload('field', e.token), e._requestId));
    assert.equal(bridge.getEFieldSampled(2).kind, 'e');
    assert.equal(bridge.getBFieldSampled(2).kind, 'b');
    assert.equal(bridge._wireRequests.size, 0);
});

for (const [reason, modify] of [
    ['foreign-instance', ({ buffer }) => new DataView(buffer).setBigUint64(72, 0n, true)],
    ['retired-source', ({ buffer }) => new DataView(buffer).setBigUint64(24, 3n, true)],
    ['retired-epoch', ({ buffer }) => new DataView(buffer).setBigUint64(32, 5n, true)],
    ['retired-generation', ({ bridge }) => bridge._markVisualDataDirty()],
    ['response-kind-mismatch', ({ buffer }) => new DataView(buffer).setUint32(96, 1, true)],
]) {
    test(`v3 rejects ${reason} before mutating field cache`, t => {
        const { bridge, sent } = fixture(t); bridge.getEFieldSampled(2);
        const buffer = envelope(payload('field', sent[0].token), sent[0]._requestId);
        modify({ bridge, buffer }); bridge._handleBinary(buffer);
        assert.equal(bridge._lastWireRejection.reason, reason);
        assert.equal(bridge._fieldSampleCache.size, 0);
    });
}

test('retired socket and unversioned binary cannot overwrite a negotiated v3 cache', t => {
    const { bridge, sent } = fixture(t); bridge.getEFieldSampled(2);
    bridge._handleBinary(envelope(payload(), sent[0]._requestId), {});
    assert.equal(bridge._lastWireRejection.reason, 'retired-socket');
    bridge._handleBinary(payload());
    assert.equal(bridge._lastWireRejection.reason, 'missing-binary-provenance');
    assert.equal(bridge._fieldSampleCache.size, 0);
});

test('special JSON completions settle their RPC once before any FIFO fallback', async t => {
    const { bridge, sent } = fixture(t);
    const tick = bridge._sendJSON({ cmd: 'tick' });
    const info = bridge._sendJSON({ cmd: 'info' });
    const reply = { type: 'tick_complete', tick: BIG, _requestId: sent[0]._requestId };
    bridge._handleJSON(JSON.stringify(reply));
    assert.equal((await tick).tick, BIG);
    const epoch = bridge._visualEpoch;
    bridge._handleJSON(JSON.stringify(reply));
    assert.equal(bridge._visualEpoch, epoch);
    assert.equal(bridge._pendingQueue.length, 1);
    bridge._handleJSON(JSON.stringify({ latticeSize: 9, _requestId: sent[1]._requestId }));
    assert.equal((await info).latticeSize, 9);
});

test('progress retains correlation until final acknowledgement; late progress is rejected', async t => {
    const { bridge, sent } = fixture(t);
    const request = bridge._sendJSON({ cmd: 'resize', size: 9 });
    const progress = { type: 'operation_progress', operation: 'resize', phase: 'allocating',
        size: 9, _requestId: sent[0]._requestId };
    bridge._handleJSON(JSON.stringify(progress)); assert.equal(bridge._pendingQueue.length, 1);
    bridge._handleJSON(JSON.stringify({ latticeSize: 9, _requestId: sent[0]._requestId }));
    await request;
    bridge._handleJSON(JSON.stringify(progress));
    assert.equal(bridge._lastWireRejection.reason, 'unknown-or-retired-request');
});

test('correlated slice geometry is checked before cache publication', async t => {
    const { bridge, sent } = fixture(t);
    const request = bridge._sendJSON({ cmd: 'get_flux_slice', axis: 'z', index: 4 });
    const response = { type: 'flux_slice', axis: 'x', index: 4, data: [1, null],
        nativeInstanceId: INSTANCE, sampleTick: BIG, sourceEpoch: 4, epoch: 6, _requestId: sent[0]._requestId };
    bridge._handleJSON(JSON.stringify(response));
    assert.equal(bridge._sliceCache.size, 0);
    assert.equal(bridge._lastWireRejection.reason, 'response-kind-mismatch');
    bridge._handleJSON(JSON.stringify({ ...response, axis: 'z' }));
    assert.equal((await request).axis, 'z');
    assert.equal(bridge._sliceCache.get('z_4')[0], 1);
    assert.ok(Number.isNaN(bridge._sliceCache.get('z_4')[1]));
    assert.equal(bridge._sliceCache.get('z_4').provenance.sampleTick, BIG);
});

test('field deferral retires only its own request and retains the other in-flight response', t => {
    const { bridge, sent } = fixture(t);
    bridge.getEFieldSampled(2); bridge.getBFieldSampled(2);
    const [e, b] = sent;
    bridge._handleJSON(JSON.stringify({ type: 'visual_deferred', operation: 'get_field_sample',
        retryAfterMs: 1000, _requestId: e._requestId }));
    assert.equal(bridge._fieldSampleRequestsByToken.has(e.token), false);
    assert.equal(bridge._fieldSampleRequestsByToken.has(b.token), true);
    bridge._handleBinary(envelope(payload('field', b.token, 1), b._requestId));
    assert.equal(bridge._fieldSampleCache.get('b@2').count, 1);
});

test('retired visual frame releases its slot so a current request can complete', t => {
    const { bridge, sent } = fixture(t); bridge.getEFieldSampled(2);
    const old = sent[0]; bridge._markVisualDataDirty();
    bridge._handleBinary(envelope(payload('field', old.token), old._requestId));
    assert.equal(bridge._fieldSampleRequestsByToken.size, 0);
    bridge.getEFieldSampled(2);
    const current = sent.at(-1);
    assert.notEqual(current._requestId, old._requestId);
    bridge._handleBinary(envelope(payload('field', current.token), current._requestId, { tick: 7 }));
    assert.equal(bridge.getEFieldSampled(2).sampleTick, 7);
});

test('ordered tick ACK then current particle and field samples populate the same live view', t => {
    const { bridge, sent } = fixture(t);
    bridge.tick(); bridge.getParticleData(); bridge.getEFieldSampled(2);
    const tick = sent.find(item => item.cmd === 'tick');
    const particles = sent.find(item => item.cmd === 'get_particles');
    const field = sent.find(item => item.cmd === 'get_field_sample');
    bridge._handleJSON(JSON.stringify({ type: 'tick_complete', tick: 2, _requestId: tick._requestId,
        sourceEpoch: 4, epoch: 7, nativeInstanceId: INSTANCE }));
    bridge._handleBinary(envelope(payload('particles'), particles._requestId, { tick: 2, epoch: 7 }));
    bridge._handleBinary(envelope(payload('field', field.token), field._requestId, { tick: 2, epoch: 7 }));
    assert.equal(bridge._particleData.count, 1);
    assert.equal(bridge.getEFieldSampled(2).sampleTick, 2);
    assert.equal(bridge._particleRequestEpoch, bridge._visualEpoch);
    assert.equal(bridge._fieldSampleCacheEpoch.get('e@2'), bridge._visualEpoch);
    assert.equal(bridge._fieldSampleRequestEpoch.get('e@2'), bridge._visualEpoch);
    assert.equal(bridge._wireRequests.size, 0);
});

test('current-tick metadata never rescues a request invalidated by a local edit', t => {
    const { bridge, sent } = fixture(t);
    bridge.tick(); bridge.getParticleData();
    const tick = sent[0], particles = sent[1];
    bridge._handleJSON(JSON.stringify({ type: 'tick_complete', tick: 2, _requestId: tick._requestId }));
    bridge.injectFlux(1, 1, 1, 1, 0, 0);
    bridge._handleBinary(envelope(payload('particles'), particles._requestId, { tick: 2 }));
    assert.equal(bridge._particleData.count, 0);
    assert.equal(bridge._lastWireRejection.reason, 'retired-generation');
});

test('particle/field headers cannot advertise a different lattice under the same source identity', t => {
    const { bridge, sent } = fixture(t); bridge.getParticleData();
    const buffer = envelope(payload('particles'), sent[0]._requestId);
    new DataView(buffer).setUint32(64, 7, true);
    bridge._handleBinary(buffer);
    assert.equal(bridge._particleData.count, 0);
    assert.equal(bridge._lastWireRejection.reason, 'lattice-size-mismatch');
});

for (const [cmd, getter, cacheName] of [
    ['inspect_voxel', 'inspectVoxel', '_voxelCache'], ['get_force_at', 'getForceAt', '_forceAtCache'],
]) {
    test(`${cmd} validates wrapped coordinates and recorded source/epoch/tick before public cache writes`, async t => {
        const { bridge, sent } = fixture(t);
        const record = { x: 8, y: 1, z: 8, nativeInstanceId: INSTANCE,
            sourceEpoch: 4, epoch: 6, sampleTick: 3,
            ...(cmd === 'inspect_voxel' ? { state: 1, fluxX: 0, fluxY: 0, fluxZ: 0 }
                : { coulombX: 0, coulombY: 0, coulombZ: 0 }) };
        bridge[getter](-1, 10, -10);
        let request = sent.at(-1);
        bridge._handleJSON(JSON.stringify({ ...record, x: 7, _requestId: request._requestId }));
        assert.equal(bridge._lastWireRejection.reason, 'response-kind-mismatch');
        assert.equal(bridge[cacheName].size, 0);
        bridge._handleJSON(JSON.stringify({ ...record, sourceEpoch: 3, epoch: 5, sampleTick: 2,
            _requestId: request._requestId }));
        await new Promise(done => setImmediate(done));
        assert.equal(bridge[cacheName].size, 0);
        assert.equal(bridge._pointQueryRequestsInFlight, 0);
        bridge[getter](-1, 10, -10);
        request = sent.at(-1);
        assert.notEqual(request._requestId, sent[0]._requestId);
        bridge._handleJSON(JSON.stringify({ ...record, _requestId: request._requestId }));
        await new Promise(done => setImmediate(done));
        const value = bridge[cacheName].get('-1,10,-10');
        assert.equal(value.x, 8);
        assert.equal(value.provenance.sampleTick, 3);
        assert.ok(Object.isFrozen(value.provenance));
        assert.equal(bridge._pointQueryRequestsInFlight, 0);
    });
}

test('point RPCs reject missing provenance and opposite payload kinds before settling', async t => {
    const { bridge, sent } = fixture(t);
    const response = { x: 1, y: 2, z: 3, state: 1, fluxX: 0, fluxY: 0, fluxZ: 0 };
    const request = bridge._sendJSON({ cmd: 'inspect_voxel', x: 1, y: 2, z: 3 });
    const id = sent[0]._requestId;
    bridge._handleJSON(JSON.stringify({ ...response, _requestId: id }));
    assert.equal(bridge._lastWireRejection.reason, 'missing-json-provenance');
    bridge._handleJSON(JSON.stringify({ x: 1, y: 2, z: 3, coulombX: 0, coulombY: 0, coulombZ: 0,
        nativeInstanceId: INSTANCE, sourceEpoch: 4, epoch: 6, sampleTick: 3, _requestId: id }));
    assert.equal(bridge._lastWireRejection.reason, 'response-kind-mismatch');
    bridge._handleJSON(JSON.stringify({ ...response, nativeInstanceId: INSTANCE,
        sourceEpoch: 4, epoch: 6, sampleTick: BIG, _requestId: id }));
    assert.equal((await request).sampleTick, BIG);
});

test('correlated field failure leaves another kind in flight and accepts its successful response', t => {
    const { bridge, sent } = fixture(t);
    bridge.getEFieldSampled(2); bridge.getBFieldSampled(2);
    const [e, b] = sent;
    bridge._handleJSON(JSON.stringify({ error: 'field reduction failed', operation: 'get_field_sample', _requestId: e._requestId }));
    assert.equal(bridge._fieldSampleRequestsByToken.has(e.token), false);
    assert.equal(bridge._fieldSampleRequestsByToken.has(b.token), true);
    bridge._handleBinary(envelope(payload('field', b.token, 1), b._requestId));
    assert.equal(bridge.getBFieldSampled(2).count, 1);
});

test('correlated slice error releases only its own geometry without throwing in recovery', t => {
    const { bridge, sent } = fixture(t);
    bridge.getFluxSlice(0, 3); bridge.getFluxSlice(1, 3);
    bridge._handleJSON(JSON.stringify({ error: 'slice failed', operation: 'get_flux_slice', _requestId: sent[0]._requestId }));
    assert.equal(bridge._sliceRequestsInFlight.has('0_3'), false);
    assert.equal(bridge._sliceRequestsInFlight.has('1_3'), true);
    assert.notEqual(bridge._lastWireRejection?.reason, 'invalid-json-or-counter');
});

test('retired particle/volume deferrals cannot clear a replacement request or schedule retry', t => {
    for (const getter of ['getParticleData', 'getFluxVolume']) {
        const { bridge, sent } = fixture(t);
        bridge[getter](); const old = sent.at(-1);
        bridge._resetVisualRequests(); bridge._markVisualDataDirty();
        bridge[getter](); const current = sent.at(-1);
        bridge._handleJSON(JSON.stringify({ type: 'visual_deferred', operation: old.cmd,
            retryAfterMs: 1000, _requestId: old._requestId }));
        assert.equal(bridge[getter === 'getParticleData' ? '_particleRequestInFlight' : '_volumeRequestInFlight'], true);
        assert.equal(bridge._wireRequests.has(current._requestId), true);
        assert.equal(bridge._visualDeferredRetryTimer, null);
        assert.equal(bridge._lastWireRejection.reason, 'retired-generation');
    }
});

test('no-ID injection bursts remain lossless when the bounded visual map is full', t => {
    const { bridge, sent } = fixture(t);
    for (let i = 0; i < 64; i++) assert.equal(bridge._sendAndForget({ cmd: 'get_flux_slice', axis: 'z', index: i }), true);
    assert.equal(bridge._sendAndForget({ cmd: 'get_flux_volume' }), false);
    for (let i = 0; i < 100; i++) bridge.injectFlux(1, 2, 3, i, 0, 0);
    assert.equal(sent.filter(value => value.cmd === 'inject_flux').length, 100);
    assert.equal(sent.filter(value => value.cmd === 'inject_flux').every(value => value._requestId === undefined), true);
    bridge.dispose();
    assert.equal(bridge._wireRequests.size, 0);
});

test('new native instance accepts low counters without relabeling old-source telemetry', () => {
    const hub = new TelemetryHub();
    const snapshot = (nativeInstanceId, sourceEpoch) => ({ nativeInstanceId, sourceEpoch,
        epoch: 1, snapshotVersion: 1, tick: 1, groups: { audit: { dynamicEnergy: 1 } } });
    assert.equal(hub.ingestScale0Snapshot(snapshot(INSTANCE, BIG)), true);
    const old = hub.getScale0TelemetryMeta('audit');
    const replacement = 'a'.repeat(32);
    assert.equal(hub.ingestScale0Snapshot(snapshot(replacement, 1)), true);
    const current = hub.getScale0TelemetryMeta('audit');
    assert.notEqual(current.source, old.source);
    assert.equal(current.sourceEpoch, 1);
});

test('source and epoch counters above 2^53 retain strict ordering in bridge and hub', t => {
    const { bridge } = fixture(t, false); bridge._expectedTelemetrySourceEpoch = null;
    const snapshot = version => ({ type: 'telemetry', sourceEpoch: BIG, epoch: MAX,
        snapshotVersion: version, tick: BIG,
        groups: { audit: { dynamicEnergy: 4 } }, groupMeta: { audit: { stateVersion: version } } });
    assert.equal(bridge._acceptTelemetrySnapshot(snapshot(BIG)), true);
    assert.equal(bridge._acceptTelemetrySnapshot(snapshot('9007199254740992')), false);
    assert.equal(bridge.getTelemetrySnapshot().groupMeta.audit.stateVersion, BIG);
    const hub = new TelemetryHub();
    assert.equal(hub.ingestScale0Snapshot(snapshot(BIG)), true);
    assert.equal(hub.ingestScale0Snapshot(snapshot('9007199254740992')), false);
    assert.equal(hub.getScale0TelemetryMeta('audit').tick, BIG);
    assert.equal(hub.getScale0TelemetryMeta('audit').epoch, MAX);
    const ring = new RingBuffer(4); ring.push(1, BIG);
    assert.ok(Number.isNaN(ring.ticks[0]));
});

test('unsafe numeric metadata and foreign instance pushes never enter v3 telemetry caches', t => {
    const { bridge } = fixture(t);
    const base = { type: 'telemetry_snapshot', nativeInstanceId: INSTANCE, sourceEpoch: 4,
        epoch: 6, snapshotVersion: 1, groups: { audit: { dynamicEnergy: 1 } } };
    bridge._handleJSON(JSON.stringify({ ...base, tick: 9007199254740992 }));
    assert.equal(bridge._lastAudit, null);
    bridge._handleJSON(JSON.stringify({ ...base, nativeInstanceId: '0'.repeat(32) }));
    assert.equal(bridge._lastAudit, null);
    bridge._handleJSON(JSON.stringify({ ...base, tick: BIG }));
    assert.equal(bridge._lastAudit.dynamicEnergy, 1);
});

test('knot combination requires equal exact instance, source, epoch and tick', () => {
    const sample = { source: INSTANCE, sourceEpoch: BIG, epoch: MAX, sampleTick: BIG };
    assert.ok(commonSampleProvenance([sample, { ...sample }]));
    const frozen = Object.freeze({ ...sample });
    assert.ok(commonSampleProvenance([{ ...sample, sampleTick: 1, provenance: frozen }, sample]));
    for (const patch of [{ source: 'foreign' }, { nativeInstanceId: 'foreign' }, { sourceEpoch: '9007199254740992' },
        { epoch: '18446744073709551614' }, { sampleTick: '9007199254740992' }]) {
        assert.equal(commonSampleProvenance([sample, { ...sample, ...patch }]), null);
    }
    assert.equal(commonSampleProvenance([{ sampleTick: 4 }, { sampleTick: 4 }]), null);
    const tracker = new FieldLineKnotTracker();
    const forbidden = { get count() { throw new Error('Mixed fields must not be summed'); } };
    assert.doesNotThrow(() => tracker.measureContributions({ eField: forbidden, tick: null }));
    assert.equal(tracker.getContributions().sampleTick, null);
    assert.equal(tracker.getContributions().status, 'sample-tick-unavailable');
});
