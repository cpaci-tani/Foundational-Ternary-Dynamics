import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { WasmBridgeProxy } from '../js/bridge/wasm-bridge-proxy.js';
import { WasmBridge } from '../js/bridge/wasm-bridge.js';
import { SCALE0_SAMPLER_METHODS } from '../js/bridge/bridge-contract.js';

const read = (name) => readFileSync(new URL(`../js/bridge/${name}`, import.meta.url), 'utf8');

test('both sampler registries map linkEnergy to getLinkEnergyCurrent', () => {
    assert.equal(SCALE0_SAMPLER_METHODS.linkEnergy, 'getLinkEnergyCurrent');
    assert.match(read('sampler-registry.classic.js'), /linkEnergy:\s*\['getLinkEnergyCurrent',\s*'links'\]/);
});

test('worker allows the observation command and copies link arrays off the heap', () => {
    const worker = read('wasm-bridge.worker.js');
    assert.match(worker, /'setLinkEnergyObservation'/);
    assert.match(worker, /type === 'links'/);
    assert.match(worker, /links: new Float32Array\(raw\.links/);
    assert.match(worker, /residual: new Float32Array\(raw\.residual/);
});

test('proxy serves the cached linkEnergy@1 sample and forwards the observation switch', () => {
    const sent = [];
    const wants = [];
    const stub = {
        _samplerCache: {},
        replaceSamplerWants: (owner, keys) => wants.push([owner, keys]),
        _readSampler: WasmBridgeProxy.prototype._readSampler,
        _wantSampler: WasmBridgeProxy.prototype._wantSampler,
        _cmd: (method, ...args) => sent.push([method, ...args]),
    };
    const empty = WasmBridgeProxy.prototype.getLinkEnergyCurrent.call(stub, 2);
    assert.equal(empty.status, 'off');
    assert.equal(empty.links.length, 0);
    assert.deepEqual(wants.at(-1), ['direct:linkEnergy@1', ['linkEnergy@1']]);
    const sample = { status: 'ok', L: 3, links: new Float32Array(27 * 9), residual: new Float32Array(27) };
    stub._samplerCache['linkEnergy@1'] = sample;
    assert.equal(WasmBridgeProxy.prototype.getLinkEnergyCurrent.call(stub), sample);
    WasmBridgeProxy.prototype.setLinkEnergyObservation.call(stub, 1);
    assert.deepEqual(sent.at(-1), ['setLinkEnergyObservation', true]);
    assert.equal(WasmBridgeProxy.prototype.getLinkEnergyObservation.call(stub), true);
});

test('in-thread bridge copies observer views so later ticks cannot alias them', () => {
    const heap = new Float32Array(20);
    const raw = {
        status: 'ok', reason: '', L: 1, tick: 7,
        links: heap.subarray(0, 9), residual: heap.subarray(9, 10),
        invariant: 1.5, maxLocalChange: 2, maxResidual: 1e-16, closure: 5e-17, activeExchangeTerms: 3,
    };
    const host = { _module: { getLinkEnergyCurrent: () => raw }, _bridge: {} };
    const out = WasmBridge.prototype.getLinkEnergyCurrent.call(host);
    heap.fill(9);
    assert.equal(out.links[0], 0);
    assert.equal(out.links.length, 9);
    assert.equal(out.residual.length, 1);
    assert.equal(out.status, 'ok');
    assert.equal(out.tick, 7);
    assert.equal(out.activeExchangeTerms, 3);
});
