import test from 'node:test';
import assert from 'node:assert/strict';
import '../js/bridge/flux-publication.classic.js';
import { WasmBridgeProxy } from '../js/bridge/wasm-bridge-proxy.js';
import { WebSocketBridge } from '../js/ws-bridge.js';
import { WasmBridge } from '../js/bridge/wasm-bridge.js';
import { runScale0PhysicsTicks } from '../js/scales/scale0/runtime/tick.js';
import { createFieldSampleCache } from '../js/scales/scale0/runtime/field-sample-cache.js';
import { computePhaseFrame } from '../js/scales/scale0/runtime/overlay-frames.js';
import { FieldLineKnotTracker } from '../js/scales/scale0/runtime/field-line-knots.js';

const publication = globalThis.FTD_FLUX_PUBLICATION;

test('all Scale-0 bridge boundary setters reject malformed selectors before dispatch', () => {
    for (const Bridge of [WasmBridge,WasmBridgeProxy,WebSocketBridge]) {
        const bridge = Object.create(Bridge.prototype);
        for (const value of [NaN,Infinity,-1,0.5,'1',null,4]) {
            assert.throws(()=>bridge.setFluxBoundaryMode(value),RangeError);
            assert.throws(()=>bridge.setFluxPeriodicAxis(value),RangeError);
        }
    }
});

test('tick requests reject fractional/nonfinite values before dispatch and await worker acknowledgement', () => {
    let dispatched = 0;
    const state = {fluxMock:{isWorker:true,tickOnce:()=>dispatched++},useFluxMock:true,fieldDataVersion:7};
    for (const n of [NaN,Infinity,-1,0.5,'2',Number.MAX_SAFE_INTEGER+1]) {
        assert.throws(()=>runScale0PhysicsTicks({},state,n),RangeError);
    }
    runScale0PhysicsTicks({},state,2);
    assert.equal(dispatched,2);
    assert.equal(state.fieldDataVersion,7);
});

test('overlay cache retains owned per-kind bytes and actual sample timestamps', () => {
    let tick = 3;
    const vectors = new Float32Array([1,2,3]);
    const source = {getScale0FieldSamples:()=>({vectors,count:1,sampleTick:tick})};
    const cache = createFieldSampleCache(source,null,1);
    const first = cache.ensureSample('eField');
    vectors.fill(9); tick = 4;
    const second = cache.ensureSample('bField');
    assert.deepEqual([...first.vectors],[1,2,3]);
    assert.deepEqual([...second.vectors],[9,9,9]);
    assert.equal(first.sampleTick,3);
    assert.equal(second.sampleTick,4);
});

test('unknown knot sample tick cannot fabricate a time history', () => {
    const tracker = new FieldLineKnotTracker();
    tracker._contrib = { count: 1, captured: { energyFrac: 0.75, fluxFrac: 0.5, chargeFrac: 1 } };
    const result = tracker.record(null,null,null,9);
    assert.equal(result.sampleTick,null);
    assert.equal(result.status,'sample-tick-unavailable');
    assert.equal(tracker.getEvents().count,0);
    assert.equal(tracker.getContributions().status, 'sample-tick-unavailable');
    assert.equal(tracker.getContributions().count, 0);
    assert.deepEqual(tracker.getContributions().captured, { energyFrac: 0, fluxFrac: 0, chargeFrac: 0 });
});

test('late correlated socket reply never resolves an unrelated request', () => {
    const bridge = new WebSocketBridge();
    let resolved = 0;
    bridge._pendingQueue.push({requestId:7,socket:null,resolve:()=>resolved++});
    bridge._handleJSON(JSON.stringify({_requestId:6,type:'info'}));
    assert.equal(resolved,0);
    assert.equal(bridge._pendingQueue.length,1);
    bridge._handleJSON(JSON.stringify({_requestId:7,type:'info'}));
    assert.equal(resolved,1);
    bridge.dispose();
});

test('old point-query completion cannot repopulate a new scenario cache', async () => {
    const bridge = new WebSocketBridge();
    bridge._connected = true;
    let finish;
    bridge._sendJSON = ()=>new Promise(resolve=>finish=resolve);
    bridge.inspectVoxel(1,2,3);
    bridge._resetVisualRequests();
    bridge._markVisualDataDirty(true);
    finish({state:1});
    await new Promise(resolve=>setImmediate(resolve));
    assert.equal(bridge._voxelCache.size,0);
    bridge.dispose();
});

test('superseded deferred scenario response cannot overwrite the newer queued selection', async () => {
    const bridge = new WebSocketBridge();
    bridge._connected = true;
    bridge._queuedScenarioProfile = {name:'old',toggles:{}};
    let finish;
    bridge._sendJSON = ()=>new Promise(resolve=>finish=resolve);
    bridge._scheduleScenarioDispatch = ()=>{};
    bridge._dispatchQueuedScenarioProfile();
    bridge.beginScenarioConfiguration('new',2);
    bridge.setupScenario('new');
    bridge.commitScenarioConfiguration('new');
    finish({type:'operation_deferred',retryAfterMs:16});
    await new Promise(resolve=>setImmediate(resolve));
    assert.equal(bridge._queuedScenarioProfile.name,'new');
    bridge.dispose();
});

test('worker speed validation cannot wrap shared fixed-point control', () => {
    const proxy = Object.create(WasmBridgeProxy.prototype);
    for (const speed of [NaN,Infinity,-1,0,0.0001,2147484,'1']) {
        assert.throws(()=>proxy.setTicksPerFrame(speed),RangeError);
    }
    proxy.setTicksPerFrame(0.5);
    assert.equal(proxy._pendingTPF,0.5);
});

test('committed slow worker pauses without reseeding or terminating its owner', () => {
    const proxy = Object.create(WasmBridgeProxy.prototype);
    let fallback = 0, runtime = 0, terminated = 0;
    const messages = [];
    Object.assign(proxy, { _ready:true, _running:true, _pendingConfigurationToken:2,
        _appliedConfigurationToken:2, _runCommandSeq:0,
        _configurationCallbacks:{token:2,onInitFailure:()=>fallback++,onRuntimeFailure:()=>runtime++},
        _worker:{postMessage:msg=>messages.push(msg),terminate:()=>terminated++} });
    proxy._triggerFallback('slow committed tick');
    assert.equal(proxy._running,false);
    assert.equal(runtime,1);
    assert.equal(fallback,0);
    assert.equal(terminated,0);
    assert.equal(proxy._initFailed,undefined);
    assert.deepEqual(messages.map(m=>[m.type,m.value]),[['setRunning',false]]);
});

test('point inspections never return a cached value belonging to another coordinate', () => {
    const proxy = Object.create(WasmBridgeProxy.prototype);
    Object.assign(proxy,{_worker:{postMessage(){}},_pendingConfigurationToken:1,
        _appliedConfigurationToken:1,_ready:true,latticeSize:17,
        _inspectionCache:new Map([['1,2,3',{configurationToken:1,voxel:{state:1},sampleTick:null,dataVersion:null}]]),
        _lastForceAt:{x:1,y:2,z:3,force:{x:9}}});
    assert.equal(proxy.inspectVoxel(0,0,0),null);
    assert.equal(proxy.getForceAt(0,0,0),null);
    assert.deepEqual(proxy.inspectVoxel(1,2,3),{state:1});
    assert.deepEqual(proxy.getForceAt(1,2,3),{x:9});
});

test('reader pin prevents producer lapping and snapshots retain independent storage', () => {
    const shared = publication.create(27);
    assert.equal(publication.snapshot(shared, 27), null);
    assert.equal(publication.publish(shared, new Float64Array(27).fill(1)), true);
    const retained = publication.snapshot(shared, 27);
    const reading = publication.acquire(shared, 27);
    assert.ok(reading);
    assert.equal(publication.publish(shared, new Float64Array(27).fill(2)), true);
    assert.equal(publication.publish(shared, new Float64Array(27).fill(3)), false,
        'optional publication must wait rather than overwrite the pinned slot');
    assert.ok(reading.view.every(value => value === 1));
    reading.release();
    assert.equal(publication.publish(shared, new Float64Array(27).fill(3)), true);
    assert.ok(retained.data.every(value => value === 1));
    assert.ok(publication.snapshot(shared,27,retained).data.every(value => value === 3));
    assert.ok(retained.data.buffer instanceof ArrayBuffer);
});

test('unchanged publication reuses its owned snapshot, replacement cannot reuse old cache', () => {
    const shared = publication.create(8);
    publication.publish(shared, new Float64Array(8).fill(4));
    const snapshot = publication.snapshot(shared,8);
    assert.equal(publication.snapshot(shared,8,snapshot), snapshot);
    const replacement = publication.create(8);
    publication.publish(replacement,new Float64Array(8).fill(5));
    assert.notEqual(publication.snapshot(replacement,8,snapshot),snapshot);
});

test('proxy slices and volume both enforce configuration barrier and safe publication protocol', () => {
    const proxy = Object.create(WasmBridgeProxy.prototype);
    Object.assign(proxy,{ _ready:true,latticeSize:3,_pendingConfigurationToken:2,_appliedConfigurationToken:1 });
    const shared = publication.create(27);
    publication.publish(shared,new Float64Array(27).fill(7));
    proxy._bindFlux({ fluxSab:shared,fluxLen:27,doubleBuffered:true,fluxProtocol:publication.PROTOCOL });
    assert.equal(proxy.getFluxSlice(0,1).length,0);
    assert.equal(proxy.getFluxVolume().length,0);
    proxy._appliedConfigurationToken = 2;
    assert.ok(proxy.getFluxSlice(0,1).every(value => value === 7));
    const retained = proxy.getFluxVolume();
    publication.publish(shared,new Float64Array(27).fill(8));
    publication.publish(shared,new Float64Array(27).fill(9));
    assert.ok(retained.every(value => value === 7));
    proxy._bindFlux({heap:new SharedArrayBuffer(27*8),fluxPtr:0,fluxLen:27});
    assert.equal(proxy.getFluxVolume().length,0,'raw shared heap is not a coherent snapshot');
});

test('dual phase rejects an incomplete right-channel sample without fabricating NaN', () => {
    const sample = {fluxVector:{positions:new Float32Array([0,0,0]), vectors:new Float32Array([1,0,0]), count:1}};
    const frame = computePhaseFrame(sample, {}, new Float32Array([1,0,0]), new Float32Array(0));
    assert.equal(frame.dualAvailable, false);
    assert.equal(frame.values[0], 0);
});
