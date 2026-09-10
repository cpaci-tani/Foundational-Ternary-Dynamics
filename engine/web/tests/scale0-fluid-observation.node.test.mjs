import { test } from 'node:test';
import assert from 'node:assert/strict';
import { fieldObservationProvenance, observeLatticeFields } from '../js/scales/scale0/runtime/fluid-observation.js';
import { C_SPEED } from '../js/constants.js';

const sample = (v, tick = 7, extra = {}) => ({ count: 1, positions: new Float32Array([.5, .5, .5]),
    vectors: new Float32Array(v), sampleTick: tick, effectiveStride: 1, origin: 0, ...extra });
test('retained direct samples retain their own tick and reject mixed or absent clocks', () => {
    const owner = { getDiagnostics() { throw new Error('No retimestamping from the current clock'); } };
    assert.equal(fieldObservationProvenance([sample([0, 0, 0]), sample([1, 0, 0])], owner).sampleTick, 7);
    assert.equal(fieldObservationProvenance([sample([0, 0, 0]), sample([1, 0, 0], 8)], owner), null);
    assert.equal(fieldObservationProvenance([sample([0, 0, 0], null)], owner), null);
});
test('worker summaries require common tick, configuration and publication identity', () => {
    const p = { source: 'wasm-worker', sourceEpoch: 3, epoch: 12 }, owner = { isWorker: true };
    assert.ok(fieldObservationProvenance([sample([1, 0, 0], 7, p), sample([0, 1, 0], 7, p)], owner));
    assert.equal(fieldObservationProvenance([sample([1, 0, 0], 7, p), sample([0, 1, 0], 7, { ...p, epoch: 13 })], owner), null);
    assert.equal(fieldObservationProvenance([sample([1, 0, 0])], owner), null);
});
test('field-only summaries keep magnetic c squared and do not change the volume normalization history', () => {
    const state = { currentScenarioId: 'test', decayingMax: { emEnergy: 20 } };
    const before = JSON.stringify(state.decayingMax);
    const fields = { eField: sample([2, 0, 0]), bField: sample([0, 3, 0]),
        fluxVector: sample([0, 0, 4]), poynting: sample([0, 5, 0]) };
    const r = observeLatticeFields(fields, {}, state, 1);
    assert.ok(Math.abs(r.energy.mean - (2 + 4.5 * C_SPEED ** 2)) < 1e-6);
    assert.equal(r.flux.mean, 4); assert.equal(r.flow.mean, 5);
    assert.equal(JSON.stringify(state.decayingMax), before);
    fields.fluxVector.vectors.fill(0);
    assert.equal(r.flux.mean, 4); assert.ok(Object.isFrozen(r.flux));
    assert.equal('viscosity' in r, false);
});
