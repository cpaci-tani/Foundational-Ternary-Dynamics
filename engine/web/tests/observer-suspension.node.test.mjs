import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import { URL } from 'node:url';
import { setTimeout } from 'node:timers';
import { rafCoordinator } from '../js/lib/raf-coordinator.js';
import { suspendDashboardWork } from '../js/core/dashboard-suspension.js';
import { WebSocketBridge } from '../js/ws-bridge.js';

globalThis.requestAnimationFrame = () => 1;
globalThis.cancelAnimationFrame = () => {};

test('dashboard suspension drains owners and retains all panel subscriptions', async () => {
    let paused = false, restored = 0, sampling = 0;
    const sub = rafCoordinator.subscribe('test-instrument', { hz: 60, cb: () => sampling++ });
    const owner = { ready: true, runningStateSettled: true,
        setRunning: value => { paused = !value; },
        suspendBackgroundWork: async () => () => restored++ };
    const restore = await suspendDashboardWork([owner, owner]);
    assert.equal(paused, true);
    assert.equal(rafCoordinator.suspended, true);
    assert.equal(rafCoordinator._rafId, null);
    rafCoordinator._tick();
    assert.equal(sampling, 0);
    assert.equal(rafCoordinator.size(), 1);
    restore(); restore();
    assert.equal(restored, 1);
    assert.equal(rafCoordinator.suspended, false);
    assert.equal(rafCoordinator.size(), 1);
    sub.unsubscribe();
});

test('failed suspension restores completed owners and panel cadence', async () => {
    let restored = 0;
    await assert.rejects(suspendDashboardWork([
        { suspendBackgroundWork: async () => () => restored++ },
        { suspendBackgroundWork: async () => { throw new Error('transport unavailable'); } },
    ]), /transport unavailable/);
    assert.equal(restored, 1);
    assert.equal(rafCoordinator.suspended, false);
});

test('worker loop is dormant while paused or suspended and resumes without replacing state', () => {
    const source = readFileSync(new URL('../js/bridge/wasm-bridge.worker.js', import.meta.url), 'utf8');
    const loop = source.slice(source.indexOf('function loop()'), source.indexOf('self.onmessage ='));
    const ctrl = new Int32Array(new SharedArrayBuffer(16));
    let ticks = 0, timers = 0;
    const scope = { timer: 0, backgroundSuspended: false, ctrl, CTRL: { RUNNING: 0, TICKS_PER_FRAME: 1 },
        bridge: { tick: () => ticks++ }, N: 9, TARGET_DT: 16, tickAcc: 0,
        telemetryTimingEnabled: false, performance: { now: () => 0 }, Atomics,
        postFrame: () => {}, setTimeout: () => ++timers, self: { postMessage() {} } };
    vm.createContext(scope); vm.runInContext(loop, scope);
    scope.loop(); assert.equal(timers, 0); assert.equal(ticks, 0);
    ctrl[0] = 1; ctrl[1] = 1000;
    scope.loop(); assert.equal(timers, 1); assert.equal(ticks, 1);
    scope.backgroundSuspended = true;
    scope.loop(); assert.equal(timers, 1); assert.equal(ticks, 1);
    scope.backgroundSuspended = false;
    scope.loop(); assert.equal(ticks, 2);
});

test('native suspension disables acknowledged telemetry, drains pending ticks and restores demand', async () => {
    const bridge = new WebSocketBridge();
    const requests = [];
    try {
        bridge._connected = true;
        bridge.ready = true;
        bridge._telemetryMode = 'scheduler';
        bridge._telemetryDemand = bridge._normalizeTelemetryDemand({ diagnostics: true, gravity: true });
        bridge._telemetryAppliedDemand = bridge._cloneTelemetryDemand();
        bridge._simulationInFlight = true;
        bridge._sendJSON = async command => {
            requests.push(command);
            return { type: 'telemetry_demand', everyTicks: command.everyTicks };
        };
        let acknowledged = false;
        const suspension = bridge.suspendBackgroundWork().then(resume => { acknowledged = true; return resume; });
        await Promise.resolve();
        assert.equal(acknowledged, false, 'in-flight native tick is a real entry barrier');
        bridge._simulationInFlight = false;
        const resume = await suspension;
        assert.equal(bridge._backgroundSuspended, true);
        assert.equal(bridge._telemetryDemandExpiryTimer, null);
        assert.equal(bridge._hasTelemetryDemand(bridge._telemetryAppliedDemand), false);
        assert.equal(requests.length, 1);
        assert.equal(requests[0].diagnostics, false);
        assert.equal(requests[0].gravity, false);
        assert.equal(bridge._pumpTelemetry(), false);
        assert.equal(bridge._sendAndForget({ cmd: 'tick' }), false);
        assert.equal(bridge._sendAndForget({ cmd: 'get_particles' }), false);
        resume();
        await new Promise(resolve => setTimeout(resolve, 0));
        assert.equal(bridge._backgroundSuspended, false);
        assert.equal(bridge._telemetryAppliedDemand.diagnostics, true);
        assert.equal(bridge._telemetryAppliedDemand.gravity, true);
    } finally { bridge.dispose(); }
});
