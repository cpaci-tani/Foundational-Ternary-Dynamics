import assert from 'node:assert/strict';
import test from 'node:test';

import {
    scale1ObservationIntervalMs,
    shouldRefreshScale1Observation,
} from '../js/scales/scale1/observation-cadence.js';

test('an unchanged paused Scale 1 state performs no repeated exact observations', () => {
    assert.equal(shouldRefreshScale1Observation({
        hasSnapshot: true,
        tick: 42,
        count: 128,
        lastTick: 42,
        revision: 8,
        lastRevision: 8,
        lastCount: 128,
        nowMs: 60_000,
        lastObservationMs: 0,
    }), false);
});

test('mutations refresh immediately and progressing ticks obey the load cadence', () => {
    const state = {
        hasSnapshot: true,
        tick: 43,
        count: 128,
        lastTick: 42,
        revision: 9,
        lastRevision: 8,
        lastCount: 128,
        lastObservationMs: 1000,
    };
    assert.equal(scale1ObservationIntervalMs(128), 100);
    assert.equal(shouldRefreshScale1Observation({ ...state, nowMs: 1099 }), false);
    assert.equal(shouldRefreshScale1Observation({ ...state, nowMs: 1100 }), true);
    assert.equal(shouldRefreshScale1Observation({ ...state, nowMs: 1001, dirty: true }), true);
    assert.equal(shouldRefreshScale1Observation({
        ...state,
        tick: 42,
        count: 128,
        nowMs: 1100,
    }), true, 'a same-tick engine mutation still refreshes through its revision');
});

test('observation cadence expands for larger populations', () => {
    assert.equal(scale1ObservationIntervalMs(32), 50);
    assert.equal(scale1ObservationIntervalMs(96), 100);
    assert.equal(scale1ObservationIntervalMs(256), 200);
});
