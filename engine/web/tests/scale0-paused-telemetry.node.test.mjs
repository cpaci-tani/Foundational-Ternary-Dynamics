import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import { TelemetryHub } from '../js/telemetry-hub.js';

const source = readFileSync(new URL('../js/telemetry/demand.js', import.meta.url), 'utf8');
const collect = vm.runInNewContext(`${source.replace(/^import[^;]+;\s*/gm, '').replace(/\bexport /g, '')}\ncollectScale0OnDemand`);

function fixture(worker = true) {
    const hub = new TelemetryHub();
    const values = { audit: null, lagrangian: null };
    const metadata = { audit: null, lagrangian: null };
    const calls = { audit: 0, lagrangian: 0, masks: [] };
    const caps = {
        getScale0EnergyAudit() { calls.audit++; return values.audit; },
        getScale0Lagrangian() { calls.lagrangian++; return values.lagrangian; },
    };
    const owner = {
        isWorker: worker, capabilities: { scale0: caps },
        setTelemetryMask(...mask) { calls.masks.push(mask); },
        getScale0TelemetryGroupMeta: group => metadata[group],
    };
    const state = { fieldDataVersion: 19, useFluxMock: worker, fluxMock: worker ? owner : null };
    const ctx = { bridge: worker ? { capabilities: { scale0: {} } } : owner };
    function publish(group, version = 1, tick = 7, stale = false) {
        values[group] = group === 'audit'
            ? { dynamicEnergy: 3, waveEnergy: 2, fieldEnergy: 1, eFieldEnergy: 2, bFieldEnergy: 1 }
            : { total: 2, fieldKinetic: 3, fieldGradient: 1 };
        metadata[group] = { backend: 'wasm-worker', sourceEpoch: 5, stateVersion: version,
            tick, sampleTick: tick, stale, status: stale ? 'unavailable' : 'available',
            sampledAt: 10, receivedAt: 12 };
    }
    return { hub, state, calls, publish, values, metadata,
        collect(audit = true, lag = false) { collect(hub, ctx, state, { wantAudit: audit, wantLag: lag }); } };
}

test('asynchronous paused audit completion is ingested without a physics-version increment', () => {
    const f = fixture(); f.collect(); assert.equal(f.hub.s0.audit, null);
    f.publish('audit'); f.collect();
    assert.equal(f.state.fieldDataVersion, 19); assert.equal(f.hub.s0.audit.dynamicEnergy, 3);
    const first = f.hub.getScale0TelemetryMeta('audit');
    assert.equal(first.tick, 7); assert.equal(first.sourceEpoch, 5); assert.equal(first.stateVersion, 1);
    assert.equal(first.sampledAt, 10); assert.equal(first.receivedAt, 12);
    for (let i = 0; i < 5; i++) f.collect();
    const repeat = f.hub.getScale0TelemetryMeta('audit');
    for (const key of ['source', 'sourceEpoch', 'stateVersion', 'tick', 'sampleTick', 'receivedAt', 'sampledAt']) {
        assert.equal(repeat[key], first[key]);
    }
    assert.equal(f.hub._s0_aud.count, 1); assert.equal(f.hub.gauss.count, 1);
});

test('demanded Lagrangian cache completion retains its separate observation clock', () => {
    const f = fixture(); f.collect(true, true);
    f.publish('audit', 1, 7); f.publish('lagrangian', 2, 6); f.collect(true, true); f.collect(true, true);
    assert.equal(f.hub.getScale0TelemetryMeta('audit').tick, 7);
    assert.equal(f.hub.getScale0TelemetryMeta('lagrangian').tick, 6);
    assert.equal(f.hub._s0_aud.count, 1); assert.equal(f.hub._s0_lag.count, 1);
});

test('same-tick new worker observation is accepted by its group version', () => {
    const f = fixture(); f.publish('audit'); f.collect();
    f.publish('audit', 2); f.values.audit.dynamicEnergy = 4; f.collect();
    assert.equal(f.hub.s0.audit.dynamicEnergy, 4); assert.equal(f.hub.getScale0TelemetryMeta('audit').stateVersion, 2);
    assert.equal(f.hub._s0_aud.count, 2); assert.equal(f.state.fieldDataVersion, 19);
});

test('worker invalidation clears availability without adding a forged history point', () => {
    const f = fixture(); f.publish('audit'); f.collect();
    f.publish('audit', 2, 7, true); f.values.audit = null; f.collect();
    // The hub retains historical values, but current consumers must reject the
    // explicit unavailable metadata instead of relabelling the old sample.
    assert.equal(f.hub.s0.audit.dynamicEnergy, 3);
    assert.equal(f.hub.getScale0TelemetryMeta('audit').stale, true);
    assert.equal(f.hub.getScale0TelemetryMeta('audit').status, 'unavailable');
    assert.equal(f.hub._s0_aud.count, 1);
});

test('closed demands never read worker telemetry caches', () => {
    const f = fixture(); f.collect(false, false); f.publish('audit'); f.publish('lagrangian');
    for (let i = 0; i < 5; i++) { f.state.fieldDataVersion++; f.collect(false, false); }
    assert.equal(f.calls.audit, 0); assert.equal(f.calls.lagrangian, 0);
    f.collect(true, false); assert.equal(f.calls.audit, 1); assert.equal(f.calls.lagrangian, 0);
    f.collect(false, false); assert.equal(f.calls.audit, 1);
});

test('direct WASM retains bounded reduction calls per version or demand opening', () => {
    const f = fixture(false); f.publish('audit'); f.publish('lagrangian');
    for (let i = 0; i < 6; i++) f.collect(true, true);
    assert.equal(f.calls.audit, 1); assert.equal(f.calls.lagrangian, 1);
    f.state.fieldDataVersion++; f.collect(true, true);
    assert.equal(f.calls.audit, 2); assert.equal(f.calls.lagrangian, 2);
    f.collect(false, false); f.collect(true, false);
    assert.equal(f.calls.audit, 3); assert.equal(f.calls.lagrangian, 2);
});
