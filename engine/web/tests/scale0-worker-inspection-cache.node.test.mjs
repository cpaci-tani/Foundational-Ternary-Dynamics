import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
import { WasmBridgeProxy } from '../js/bridge/wasm-bridge-proxy.js';

function fixture() {
    const proxy = Object.create(WasmBridgeProxy.prototype);
    const messages = [];
    const ctrl = new Int32Array(new SharedArrayBuffer(32));
    ctrl[2] = 7; ctrl[6] = 9;
    Object.assign(proxy, {
        latticeSize: 17, _ready: true, _pendingConfigurationToken: 3,
        _appliedConfigurationToken: 3, _inspectionCache: new Map(),
        _inspectionPending: new Map(), _nextInspectionRequestId: 0,
        _ctrl: ctrl, _worker: { postMessage: m => messages.push(m) },
    });
    return { proxy, messages, ctrl };
}
function reply(proxy, request, overrides = {}) {
    proxy._onMessage({
        ...request, type: 'inspectResult', sampleTick: 7, dataVersion: 9,
        inspect: { x: request.x, y: request.y, z: request.z, voxel: { state: 1, nested: { flux: 2 } } },
        ...overrides,
    });
}

test('paused center and all 26 neighbors remain available without request churn', () => {
    const { proxy, messages } = fixture();
    const sites = [];
    for (let x = 7; x <= 9; x++) for (let y = 7; y <= 9; y++) for (let z = 7; z <= 9; z++) sites.push([x,y,z]);
    for (let repeat = 0; repeat < 3; repeat++) for (const site of sites) assert.equal(proxy.inspectVoxel(...site), null);
    assert.equal(messages.length, 27);
    for (const request of messages) reply(proxy, request);
    for (let repeat = 0; repeat < 3; repeat++) for (const site of sites) {
        assert.equal(proxy.inspectVoxel(...site).state, 1);
        assert.equal(proxy.getInspectionSampleMeta(...site).stale, false);
    }
    assert.equal(messages.length, 27);
    assert.equal(proxy._inspectionPending.size, 0);
});

test('wrapped coordinates deduplicate and malformed coordinates never dispatch', () => {
    const { proxy, messages } = fixture();
    proxy.inspectVoxel(-1, 17, 34); proxy.inspectVoxel(16, 0, 0);
    assert.equal(messages.length, 1);
    for (const bad of [NaN, Infinity, 0.5, '1', null, Number.MAX_SAFE_INTEGER + 1]) assert.equal(proxy.inspectVoxel(bad,0,0), null);
    assert.equal(messages.length, 1);
    reply(proxy, messages[0]);
    assert.equal(proxy.getInspectionSample(-1,17,34).state, 1);
});

test('request, exact coordinate and configuration lineage all guard acceptance', () => {
    const { proxy, messages } = fixture(); proxy.inspectVoxel(1,2,3);
    const request = messages[0];
    for (const override of [{ requestId: request.requestId + 1 }, { configurationToken: 2 },
        { x: 18 }, { inspect: { x: 2, y: 2, z: 3, voxel: { state: -1 } } }]) {
        reply(proxy, request, override); assert.equal(proxy.getInspectionSample(1,2,3), null);
    }
    reply(proxy, request); assert.equal(proxy.getInspectionSample(1,2,3).state, 1);
    reply(proxy, request, { inspect: { x: 1, y: 2, z: 3, voxel: { state: -1 } } });
    assert.equal(proxy.getInspectionSample(1,2,3).state, 1, 'consumed replies cannot republish');
});

test('retained observations own their values and retain actual stale or unknown clocks', () => {
    const { proxy, messages, ctrl } = fixture(); proxy.inspectVoxel(1,2,3);
    const message = messages[0]; const voxel = { state: 1, nested: { flux: 2 } };
    reply(proxy, message, { inspect: { x: 1, y: 2, z: 3, voxel } });
    voxel.nested.flux = 44;
    proxy.getInspectionSample(1,2,3).nested.flux = 55;
    assert.equal(proxy.getInspectionSample(1,2,3).nested.flux, 2);
    ctrl[2] = 8; ctrl[6] = 10;
    assert.deepEqual(proxy.getInspectionSampleMeta(1,2,3), { sampleTick: 7, configurationToken: 3, dataVersion: 9, stale: true });
    proxy.inspectVoxel(1,2,3); proxy.inspectVoxel(1,2,3); assert.equal(messages.length, 2);
    reply(proxy, messages[1], { sampleTick: null, dataVersion: null });
    assert.equal(proxy.getInspectionSampleMeta(1,2,3).sampleTick, null);
    assert.equal(proxy.getInspectionSampleMeta(1,2,3).stale, true);
});

test('same-tick mutations clear records and invalidate replies issued before mutation', () => {
    const { proxy, messages } = fixture(); proxy.inspectVoxel(1,2,3);
    const old = messages[0]; reply(proxy, old);
    proxy.inspectVoxel(2,2,3); const outstanding = messages[1];
    proxy._cmd('injectFlux', 1,2,3,1,0,0);
    assert.equal(proxy.getInspectionSample(1,2,3), null);
    reply(proxy, outstanding); assert.equal(proxy.getInspectionSample(2,2,3), null);
    proxy.inspectVoxel(2,2,3); assert.ok(messages.at(-1).requestId > outstanding.requestId);
});

test('both cache and in-flight demand stay bounded at 128 coordinates', () => {
    const { proxy, messages } = fixture();
    for (let i = 0; i < 129; i++) proxy.inspectVoxel(i % 17, Math.floor(i / 17), 0);
    assert.equal(messages.length, 128); assert.equal(proxy._inspectionPending.size, 128);
    for (const request of messages.slice()) reply(proxy, request);
    proxy.inspectVoxel(9,7,0); reply(proxy, messages.at(-1));
    assert.equal(proxy._inspectionCache.size, 128);
    assert.equal(proxy.getInspectionSample(0,0,0), null);
});

test('generation clearing and disposal reject pending replies and stop reads', () => {
    const { proxy, messages } = fixture(); proxy.inspectVoxel(1,2,3); const old = messages[0]; reply(proxy, old);
    Object.assign(proxy, { _digestPending: new Map(), _clearFrameWatchdog() {} });
    proxy._clearScientificGenerationCaches();
    assert.equal(proxy._inspectionCache.size, 0); assert.equal(proxy._inspectionPending.size, 0);
    proxy._ready = true; proxy._appliedConfigurationToken = 4; proxy._pendingConfigurationToken = 4;
    reply(proxy, old); assert.equal(proxy.getInspectionSample(1,2,3), null);
    proxy.inspectVoxel(1,2,3); const next = messages.at(-1);
    proxy.terminate(); reply(proxy, next);
    assert.equal(proxy._inspectionCache.size, 0); assert.equal(proxy._inspectionPending.size, 0);
    assert.equal(proxy.inspectVoxel(1,2,3), null); assert.equal(proxy.getInspectionSampleMeta(1,2,3), null);
    proxy._onMessage({ type: 'disposed' });
    assert.equal(proxy._terminated, true);
});

test('failure replies retire pending keys and never retain a failed stale value', () => {
    const { proxy, messages } = fixture(); proxy.inspectVoxel(1,2,3);
    reply(proxy, messages[0], { inspect: null });
    assert.equal(proxy._inspectionPending.size, 0);
    proxy.inspectVoxel(1,2,3); assert.equal(messages.length, 2);
});

test('legacy frame.inspect cannot overwrite or retimestamp a coordinate sample', () => {
    const { proxy, messages } = fixture(); proxy.inspectVoxel(1,2,3); reply(proxy, messages[0]);
    Object.assign(proxy, { _now: () => 0, _armFrameWatchdog() {}, _acceptTelemetryGroupFrame() {} });
    proxy._acceptFrameMessage({ diag: { tick: 100 }, inspect: { x:1, y:2, z:3, voxel: {state:-1} } });
    assert.equal(proxy.getInspectionSample(1,2,3).state, 1);
    assert.equal(proxy.getInspectionSampleMeta(1,2,3).sampleTick, 7);
});

test('request identity exhaustion fails before dispatch rather than recycling', () => {
    const { proxy, messages } = fixture(); proxy._nextInspectionRequestId = Number.MAX_SAFE_INTEGER;
    assert.throws(() => proxy.inspectVoxel(1,2,3), RangeError); assert.equal(messages.length, 0);
});

function workerInspect({ tick = 11, hasClock = true, request = {}, throws = false } = {}) {
    const source = fs.readFileSync(new URL('../js/bridge/wasm-bridge.worker.js', import.meta.url), 'utf8');
    const start = source.indexOf("      case 'inspectVoxel': {");
    const end = source.indexOf("      case 'getForceAt':", start);
    const messages = []; const ctrl = new Int32Array(new SharedArrayBuffer(32)); ctrl[6] = 21;
    const context = vm.createContext({
        msg: { configurationToken: 3, requestId: 5, x:1,y:2,z:3, ...request },
        activeConfigurationToken: 3, N:17, lastInspect:null, ctrl, CTRL:{ DATA_VERSION:6 },
        bridge: hasClock ? { currentTick: () => tick } : {},
        mod: { inspectVoxel() { if (throws) throw new Error('inspection unavailable'); return {state:1}; } },
        self: { postMessage: m => messages.push(m) },
    });
    vm.runInContext(`switch ('inspectVoxel') { ${source.slice(start,end)} }`, context);
    return messages;
}

test('actual worker case echoes exact identity and true core clock, preserving absent clock', () => {
    const [message] = workerInspect();
    assert.equal(message.sampleTick, 11); assert.equal(message.dataVersion, 21);
    assert.equal(message.requestId, 5); assert.equal(message.x, 1);
    for (const options of [{ hasClock:false }, { tick:NaN }, { tick:-1 }]) assert.equal(workerInspect(options)[0].sampleTick, null);
    assert.equal(workerInspect({ throws:true })[0].inspect, null);
    for (const request of [{requestId:0}, {requestId:1.5}, {configurationToken:'3'}, {x:-1}, {z:17}]) assert.equal(workerInspect({request}).length, 0);
});
