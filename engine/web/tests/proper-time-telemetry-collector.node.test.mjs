import test from 'node:test';
import assert from 'node:assert/strict';
import { TelemetryHub } from '../js/telemetry-hub.js';
import { getScale0TelemetryDemand, collectScale0OnDemand } from '../js/telemetry/demand.js';

function sample(values, provenance = {}) {
    return {
        values: Float32Array.from(values), count: values.length, effectiveStride: 4,
        sampleTick: 24, source: 'wasm-worker', sourceEpoch: 7, epoch: 13,
        ...provenance,
    };
}

function owner(rows) {
    return { capabilities: { scale0: {
        getScale0ProperTimeSamples: ({ kind }) => rows[kind],
    } } };
}

test('canonical collector publishes one coherent sampler observation with sampler provenance', () => {
    const hub = new TelemetryHub();
    const bridge = owner({
        tau: sample([2, 4]), lapse: sample([0.8, 1]), dbPhase: sample([0, Math.PI / 2]),
    });
    const metrics = hub.collectScale0ProperTime(bridge, null, false, 4);
    const meta = hub.getScale0TelemetryMeta('properTime');

    assert.equal(metrics.sampleTick, 24);
    assert.equal(metrics.sourceEpoch, 7);
    assert.equal(metrics.properTimeMean, 3);
    assert.equal(metrics.properTimeMin, 2);
    assert.equal(metrics.properTimeMax, 4);
    assert.ok(Math.abs(metrics.lapseMean - 0.9) < 1e-6);
    assert.equal(metrics.tauStride, 4);
    assert.equal(meta.tick, 24);
    assert.equal(meta.source, 'wasm-worker');
    assert.equal(meta.sourceEpoch, 7);
    assert.equal(meta.stale, false);
    assert.equal(hub.ptime.properTimeMean.count, 1);
    assert.equal(hub.ptime.properTimeMean.getTick(0), 24);
});

test('collector rejects mixed sampler provenance and cannot retimestamp retained ptime', () => {
    const hub = new TelemetryHub();
    const bridge = owner({
        tau: sample([2]), lapse: sample([1]), dbPhase: sample([0], { sourceEpoch: 8 }),
    });
    assert.equal(hub.collectScale0ProperTime(bridge, null, false, 4), null);
    assert.equal(hub.ptime.properTimeMean.count, 0);
    assert.equal(hub.s0.properTime, null);
    assert.equal(hub.getScale0TelemetryMeta('properTime').stale, true);
    assert.equal(hub.getScale0TelemetryMeta('properTime').unavailableReason, 'proper-time-provenance-mismatch');
});

test('completed zero-support rows are explicit scientific unavailability, never fabricated zeroes', () => {
    const hub = new TelemetryHub();
    const bridge = owner({
        tau: sample([]), lapse: sample([]), dbPhase: sample([]),
    });
    assert.equal(hub.collectScale0ProperTime(bridge, null, false, 4), null);
    const meta = hub.getScale0TelemetryMeta('properTime');
    assert.equal(meta.status, 'unavailable');
    assert.equal(meta.stale, true);
    assert.equal(meta.unavailableReason, 'no-manifested-voxel-support');
    assert.equal(meta.sampleTick, 24);
    assert.equal(meta.sourceEpoch, 7);
    assert.equal(hub.ptime.properTimeMean.count, 0);
});

test('explicitly disabled proper-time toggles remain distinguishable from pending samplers', () => {
    const hub = new TelemetryHub();
    const bridge = {
        ...owner({ tau: sample([]), lapse: sample([]), dbPhase: sample([]) }),
        getToggle: key => key === 'latency_field' || key === 'de_broglie_clock' ? false : undefined,
    };
    assert.equal(hub.collectScale0ProperTime(bridge, null, false, 4), null);
    const meta = hub.getScale0TelemetryMeta('properTime');
    assert.equal(meta.unavailableReason, 'proper-time-disabled');
    assert.equal(meta.unavailableDetail, 'no-manifested-voxel-support');

    const pending = new TelemetryHub();
    const pendingBridge = owner({
        tau: { values: new Float32Array(0), count: 0 },
        lapse: { values: new Float32Array(0), count: 0 },
        dbPhase: { values: new Float32Array(0), count: 0 },
    });
    pending.collectScale0ProperTime(pendingBridge, null, false, 4);
    assert.equal(pending.getScale0TelemetryMeta('properTime').unavailableReason, 'proper-time-sampler-pending');
});

test('direct scratch-backed sampler rows are copied before the next sampler read', () => {
    const scratch = new Float32Array(2);
    const rows = {
        tau: [2, 4], lapse: [0.8, 1], dbPhase: [0, Math.PI / 2],
    };
    const bridge = { capabilities: { scale0: {
        getScale0ProperTimeSamples: ({ kind }) => {
            scratch.set(rows[kind]);
            return {
                values: scratch, count: 2, effectiveStride: 4,
                sampleTick: 24, source: 'wasm', sourceEpoch: 7, epoch: 13,
            };
        },
    } } };
    const metrics = new TelemetryHub().collectScale0ProperTime(bridge, null, false, 4);
    assert.equal(metrics.properTimeMean, 3);
    assert.ok(Math.abs(metrics.lapseMean - 0.9) < 1e-6);
});

test('proper-time demand is a Time-or-Grid union and releases sampler ownership when hidden', () => {
    const visible = new Set(['telemetry-grid']);
    const ctx = {
        bridge: { latticeSize: 97 },
        isPanelVisible: id => visible.has(id),
    };
    const demand = getScale0TelemetryDemand(ctx, null);
    assert.equal(demand.wantProperTime, true);
    assert.equal(demand.wantGravity, false, 'Grid-only ptime does not schedule gravity aggregation');
    assert.equal(demand.properTimeStride, 4);
    visible.clear(); visible.add('time');
    assert.equal(getScale0TelemetryDemand(ctx, null).wantProperTime, true);

    const calls = [];
    const bridge = {
        latticeSize: 97,
        setTelemetryMask(...args) { calls.push(['mask', args]); },
        replaceSamplerWants(...args) { calls.push(['wants', args]); },
        capabilities: { scale0: { getScale0Diagnostics: () => null } },
    };
    const state = { useFluxMock: false, fluxMock: null, fieldDataVersion: 1 };
    const hub = {
        _prevWantAudit: false, _prevWantLag: false, _prevWantProperTime: false,
        _lastAuditVersion: -1,
        collectScale0Audit() {}, collectScale0Lagrangian() {},
        collectScale0ProperTime() {},
    };
    collectScale0OnDemand(hub, { ...ctx, bridge, isPanelVisible: id => id === 'telemetry-grid' }, state,
        getScale0TelemetryDemand({ ...ctx, bridge, isPanelVisible: id => id === 'telemetry-grid' }, null));
    assert.deepEqual(calls.find(([type]) => type === 'wants'), ['wants', ['proper-time-telemetry', ['tau@4', 'lapse@4', 'dbPhase@4']]]);

    calls.length = 0;
    collectScale0OnDemand(hub, { ...ctx, bridge, isPanelVisible: () => false }, state,
        getScale0TelemetryDemand({ ...ctx, bridge, isPanelVisible: () => false }, null));
    assert.deepEqual(calls.find(([type]) => type === 'wants'), ['wants', ['proper-time-telemetry', []]]);
    assert.equal(calls.find(([type]) => type === 'mask')[1][3], false);
});
