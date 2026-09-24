import test from 'node:test';
import assert from 'node:assert/strict';
import { WasmBridgeProxy } from '../js/bridge/wasm-bridge-proxy.js';
import { createSamplerWantSet } from '../js/bridge/sampler-want-set.js';

test('suspended worker reads and setters preserve caches and retry successfully after resume', async () => {
    const messages = [], bridge = Object.create(WasmBridgeProxy.prototype);
    Object.assign(bridge, {
        _backgroundSuspended: false, _backgroundSuspendSeq: 0, _backgroundSuspendPending: new Map(),
        _running: false, _ready: true, _pendingConfigurationToken: 1, _appliedConfigurationToken: 1,
        _wantAudit: false, _wantLag: false, _wantGravity: false, _wantProperTime: false,
        _toggles: { gravity: false }, _omega0: 1, _langevinTemp: 0, _langevinGamma: 0.01,
        _sorIterations: 6, _linkEnergyObservation: false, _pendingTPF: 1,
        _requestedFluxBoundaryMode: 0, _requestedFluxPeriodicAxis: 2,
        _inspectionCache: new Map(), _inspectionPending: new Map(), _nextInspectionRequestId: 0,
        _samplerCache: { 'e@2': { count: 1 } }, _samplerCacheVersion: { 'e@2': 4 },
        _lastForceAt: { x: 0, y: 0, z: 0, force: { x: 1, y: 2, z: 3 } }, latticeSize: 9,
        _ctrl: new Int32Array(new SharedArrayBuffer(32)),
        _worker: { postMessage(message) {
            messages.push(message);
            if (message.type === 'setBackgroundSuspended') {
                Promise.resolve().then(() => bridge._onMessage({ type: 'backgroundSuspended', seq: message.seq, value: message.value }));
            }
        } },
    });
    bridge._samplerWants = createSamplerWantSet((op, kind, stride) => messages.push({ type: op, kind, stride }));
    bridge.replaceSamplerWants('direct:e@2', ['e@2']);
    const capability = bridge._buildCaps();
    const resume = await bridge.suspendBackgroundWork();
    messages.length = 0;
    const ctrlBefore = [...bridge._ctrl];

    assert.equal(bridge.inspectVoxel(0, 0, 0), null);
    assert.deepEqual(bridge.getForceAt(0, 0, 0), { x: 1, y: 2, z: 3 });
    bridge.setTelemetryMask(true, true, true, true);
    bridge.setTelemetryTiming(true);
    bridge.replaceSamplerWants('a-new-panel', ['b@2']);
    bridge.unwantSampler('e', 2);
    bridge.setToggle('gravity', true);
    capability.setToggle('gravity', true);
    bridge.setToggles([['gravity', true]]);
    bridge.setOmega0(2); bridge.setLangevinTemp(3); bridge.setLangevinGamma(4);
    bridge.setSorIterations(12); bridge.setLinkEnergyObservation(true);
    bridge.setFluxBoundaryMode(1); bridge.setFluxPeriodicAxis(1);
    bridge.setTicksPerFrame(10); bridge.reset(15); bridge.tickOnce();
    assert.deepEqual(messages, [], 'no background work is posted only to be discarded by the worker');
    assert.equal(bridge._inspectionPending.size, 0, 'no unacknowledgeable inspection remains pending');
    assert.equal(bridge._wantAudit, false); assert.equal(bridge._wantLag, false);
    assert.equal(bridge._wantGravity, false); assert.equal(bridge._wantProperTime, false);
    assert.equal(bridge.getToggle('gravity'), false);
    assert.deepEqual([bridge.getOmega0(), bridge.getLangevinTemp(), bridge.getLangevinGamma(),
        bridge.getSorIterations(), bridge.getLinkEnergyObservation()], [1, 0, 0.01, 6, false]);
    assert.deepEqual([...bridge._samplerWants.wanted()], ['e@2']);
    assert.deepEqual(bridge._samplerCache['e@2'], { count: 1 });
    assert.equal(bridge._requestedFluxBoundaryMode, 0); assert.equal(bridge._requestedFluxPeriodicAxis, 2);
    assert.equal(bridge.latticeSize, 9); assert.deepEqual([...bridge._ctrl], ctrlBefore);

    resume(); messages.length = 0;
    bridge.setTelemetryMask(true, true, true, true);
    bridge.replaceSamplerWants('a-new-panel', ['b@2']);
    bridge.inspectVoxel(0, 0, 0);
    bridge.getForceAt(0, 0, 0);
    assert.deepEqual(messages.map(message => message.type), ['setTelemetryMask', 'want', 'inspectVoxel', 'getForceAt']);
    assert.equal(bridge._inspectionPending.size, 1, 'the same coordinate remains requestable on return');
    bridge.setToggle('gravity', true); assert.equal(bridge.getToggle('gravity'), true);
    assert.equal(messages.at(-1).type, 'command');
});
