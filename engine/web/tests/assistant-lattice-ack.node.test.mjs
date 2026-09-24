import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
import { WasmBridgeProxy } from '../js/bridge/wasm-bridge-proxy.js';
import { WebSocketBridge } from '../js/ws-bridge.js';

test('worker promises require a matching acknowledgment; stale and rejected writes fail closed', async () => {
    const messages = [], proxy = Object.create(WasmBridgeProxy.prototype);
    Object.assign(proxy, { _ready: true, _pendingConfigurationToken: 7, _appliedConfigurationToken: 7,
        _worker: { postMessage: m => messages.push(m) } });
    let settled = false;
    const result = proxy.executeScale0Control({ type: 'step', count: 1 }, { expectedSourceEpoch: 7 }).then(v => { settled = true; return v; });
    await Promise.resolve(); assert.equal(settled, false);
    proxy._onMessage({ type: 'controlComplete', requestId: messages[0].requestId, configurationToken: 7, ok: true, tick: 12 });
    assert.equal((await result).tick, 12);
    await assert.rejects(proxy.executeScale0Control({ type: 'step', count: 1 }, { expectedSourceEpoch: 6 }), /superseded/);
    const rejected = proxy.executeScale0Control({ type: 'setToggle', name: 'gravity', value: true });
    proxy._onMessage({ type: 'controlComplete', requestId: messages.at(-1).requestId, configurationToken: 7, ok: false, error: 'invalid profile' });
    await assert.rejects(rejected, /invalid profile/);
});

test('worker handler rejects retired configurations and acknowledges exact stopped ticks', () => {
    const source = fs.readFileSync(new URL('../js/bridge/wasm-bridge.worker.js', import.meta.url), 'utf8');
    const body = source.slice(source.indexOf("      case 'acknowledgedControl': {"), source.indexOf("      case 'command': {"));
    let tick = 4; const replies = [], ctrl = new Int32Array(new SharedArrayBuffer(32));
    const context = vm.createContext({ mod: {}, bridge: { currentTick: () => tick }, backgroundSuspended: false,
        activeConfigurationToken: 2, toggleNames: ['gravity'], ctrl, CTRL: { RUNNING: 3 }, Atomics,
        applyCommand: () => { tick++; return { ok: true }; }, postFrame() {},
        lastAudit: null, lastAuditMeta: null, auditFrameCounter: 0, lastLagrangian: null, lastLagrangianMeta: null,
        lagrangianCadence: { reset() {} }, engineTogglesDirty: false,
        self: { postMessage: value => replies.push(value) } });
    const run = message => { context.msg = message; vm.runInContext(`switch(msg.type){${body}}`, context); };
    run({ type: 'acknowledgedControl', requestId: 1, configurationToken: 1, command: { type: 'step', count: 3 } });
    assert.equal(replies.at(-1).ok, false); assert.equal(tick, 4);
    run({ type: 'acknowledgedControl', requestId: 2, configurationToken: 2, command: { type: 'step', count: 3 } });
    assert.equal(replies.at(-1).ok, true); assert.equal(replies.at(-1).beforeTick, 4); assert.equal(replies.at(-1).tick, 7);
    Atomics.store(ctrl, 3, 1);
    run({ type: 'acknowledgedControl', requestId: 3, configurationToken: 2, command: { type: 'step', count: 1 } });
    assert.equal(replies.at(-1).ok, false); assert.equal(tick, 7);
});

test('native controls pin both source and server, require protocol support, and do not retry', async () => {
    const commands = [], bridge = { scale0ControlVersion: 1, _connected: true,
        _expectedTelemetrySourceEpoch: '9', _nativeInstanceId: 'server-a', _hasPendingScenarioWork: () => false,
        _sendJSON: async command => { commands.push(command); return { tick: 10 }; },
        _requireSuccessfulResponse: response => response };
    const execute = (command, options) => WebSocketBridge.prototype.executeScale0Control.call(bridge, command, options);
    await execute({ type: 'step', count: 1 });
    assert.deepEqual(commands, [{ cmd: 'tick', expectedSourceEpoch: '9', expectedNativeInstanceId: 'server-a' }]);
    await assert.rejects(execute({ type: 'step', count: 1 }, { expectedSourceEpoch: '8' }), /superseded/);
    bridge.scale0ControlVersion = 0;
    await assert.rejects(execute({ type: 'step', count: 1 }), /unavailable/);
    assert.equal(commands.length, 1);
});
