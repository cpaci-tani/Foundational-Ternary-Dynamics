import test from 'node:test';
import assert from 'node:assert/strict';
import { reduceFluxSectors, normalizeNativeFluxSectors } from '../js/assistant/field-sectors.js';
import { createLatticeControl } from '../js/assistant/lattice-control.js';

const instance = 'a'.repeat(32);
function sample(rows = [[0, 0, 0, 3, 4, 0], [1, 0, 0, 0, 0, 2]], size = 4) {
    return { kind: 'fluxVector', effectiveStride: 1, origin: 0, count: rows.length,
        positions: new Float32Array(rows.flatMap(r => r.slice(0, 3).map(v => v + .5))),
        vectors: new Float32Array(rows.flatMap(r => r.slice(3))),
        provenance: { status: 'approximate', nativeInstanceId: instance, sourceEpoch: '2',
            sampleTick: '7', epoch: '9', latticeSize: size, physicalTime: 7, dt: 1 } };
}

test('parity reduction reports observed norms and explicit omission/rounding bounds', () => {
    const result = reduceFluxSectors(sample(), 4);
    assert.equal(result.observedSquaredNorm, 29);
    assert.deepEqual(result.sectors.map(s => s.observedSquaredNorm), [25, 4]);
    assert.deepEqual(result.sectors.map(s => s.omittedSites), [31, 31]);
    assert.equal(result.maxAbsComponent, 4);
    assert.equal(result.sectors[0].omittedSquaredNormUpperBound, 31e-30);
    assert.ok(result.sectors[0].fractionBounds.lower < 25 / 29);
    assert.ok(result.sectors[0].fractionBounds.upper > 25 / 29);
    assert.match(result.sampling.qualification, /not exact zeros/);
});

test('odd cube parity counts and empty sparse capture do not imply exact zeros', () => {
    const result = reduceFluxSectors(sample([], 5), 5);
    assert.deepEqual(result.sectors.map(s => s.totalSites), [63, 62]);
    assert.equal(result.squaredNormBounds.lower, 0);
    assert.ok(result.squaredNormBounds.upper > 0);
    assert.equal(result.sectors[0].fractionBounds.upper, null);
});

test('one-sector support produces a finite strict leakage upper bound', () => {
    const result = reduceFluxSectors(sample([[0, 0, 0, 1, 0, 0]], 32), 32);
    assert.ok(result.sectors[1].fractionBounds.upper < 1e-12);
    assert.ok(result.sectors[1].fractionBounds.upper > 0);
    assert.equal(result.sectors[1].sampledSites, 0);
});

test('sample order is irrelevant and Float32 rounding remains covered', () => {
    const rows = [[3, 3, 3, .1, .2, .3], [0, 0, 0, .3, .4, .5]];
    const a = reduceFluxSectors(sample(rows), 4), b = reduceFluxSectors(sample(rows.toReversed()), 4);
    assert.deepEqual(a, b);
    const exact = .01 + .04 + .09 + .09 + .16 + .25;
    assert.ok(a.squaredNormBounds.lower < exact && a.squaredNormBounds.upper > exact);
});

test('subsampled, truncated, duplicate, invalid and unsupported captures reject', () => {
    for (const mutate of [s => { s.effectiveStride = 2; }, s => { s.origin = 1; },
        s => { s.kind = 'e'; }, s => { s.count++; }, s => { s.positions[0] = 0; },
        s => { s.positions[0] = 4.5; }, s => { s.vectors[0] = NaN; },
        s => { s.positions.set(s.positions.slice(0, 3), 3); },
        s => { s.vectors.fill(0); }, s => { s.vectors = Array.from(s.vectors); }]) {
        const s = sample(); mutate(s); assert.throws(() => reduceFluxSectors(s, 4));
    }
    for (const size of [3, 65, 4.5, NaN]) assert.throws(() => reduceFluxSectors(sample(), size));
});

function fixture() {
    const field = sample();
    const owner = { isNativeGPU: true, ready: true, _nativeBinaryVersion: 3, _nativeInstanceId: instance,
        _visualEpoch: 3, _visualInterventionEpoch: 1, getSamplerSnapshotVersion: () => 3,
        _nativeCompletedTick: '7', _expectedTelemetrySourceEpoch: '2', latticeSize: 4,
        getScale0TelemetryGroupMeta: () => ({ sourceEpoch: '2', sampleTick: '7', status: 'available' }),
        getTelemetrySnapshot: () => ({ nativeInstanceId: instance, sourceEpoch: '2', epoch: '9', tick: '7', groups: {} }),
        getDynamicalStateDigest: async () => ({ type: 'dynamical_state_digest', nonfiniteValueCount: 0,
            tick: '7', sourceEpoch: '2', latticeSize: 4, siteCount: 64, stateVersion: '10',
            hashLo: 'b'.repeat(16), hashHi: 'c'.repeat(16), compute: 'GPU' }),
        getFluxVectorSampled(stride) { assert.equal(stride, 1); return field; },
        executeScale0Control() { assert.fail('Measurement cannot dispatch a mutation or barrier'); } };
    const state = { mutationEpoch: 0, currentScenarioId: 'empty' };
    const ctx = { bridge: owner, engineMode: 'lattice', running: false, _loadGeneration: 1,
        pauseSimulation() { assert.fail('Measurement cannot change playback'); } };
    const control = createLatticeControl({ getCtx: () => ctx, getOwner: () => ctx.bridge,
        getState: () => state, getScenarios: () => [], getQualification: () => ({}) });
    const expected = control.observe();
    return { owner, field, state, ctx, control, expected, args: { expected, expectedTick: '7' } };
}

test('adapter uses existing owner sampler and exact provenance without mutation', async () => {
    const f = fixture();
    const result = await f.control.measureFluxSectors(f.args, new AbortController().signal);
    assert.equal(result.status, 'measured');
    assert.equal(result.ownerId, f.expected.ownerId);
    assert.equal(result.provenance.sampleTick, '7');
    assert.equal(result.provenance.nativeInstanceId, instance);
    assert.equal(result.observedSquaredNorm, 29);
    assert.equal(result.integrity.unchanged, true);
    f.control.dispose();
});

test('adapter rejects motion, unsupported wire, stale owner, tick and invalid tick without writes', async () => {
    for (const mutate of [f => { f.ctx.running = true; }, f => { f.owner._simulationInFlight = true; },
        f => { f.owner._queuedSimulationTicks = 1; }, f => { f.owner._nativeBinaryVersion = 2; },
        f => { f.state.mutationEpoch++; }, f => { f.ctx.bridge = { ...f.owner }; },
        f => { f.args.expectedTick = '8'; }, f => { f.args.expectedTick = '07'; },
        f => { f.args.expectedTick = '7e0'; }, f => { f.args.expectedTick = '9'.repeat(21); }]) {
        const f = fixture(); mutate(f);
        await assert.rejects(f.control.measureFluxSectors(f.args, new AbortController().signal)); f.control.dispose();
    }
});

test('delayed sampling checks cancellation and owner changes again before returning', async () => {
    for (const change of [f => { f.state.mutationEpoch++; }, f => { f.owner._nativeCompletedTick = '8'; },
        f => { f.ctx.running = true; }, f => { f.control.dispose(); }]) {
        const f = fixture(); let calls = 0;
        f.owner.getFluxVectorSampled = () => { if (++calls === 1) { change(f); return {}; } return f.field; };
        await assert.rejects(f.control.measureFluxSectors(f.args, new AbortController().signal));
        f.control.dispose();
    }
    const f = fixture(), controller = new AbortController();
    f.owner.getFluxVectorSampled = () => { controller.abort(); return f.field; };
    await assert.rejects(f.control.measureFluxSectors(f.args, controller.signal), /abort/i); f.control.dispose();
});

test('foreign provenance cannot be returned and concurrent measurements are bounded', async () => {
    for (const mutate of [s => { s.provenance.nativeInstanceId = 'b'.repeat(32); },
        s => { s.provenance.sourceEpoch = '1'; }, s => { s.provenance.epoch = '8'; },
        s => { s.provenance.sampleTick = '6'; }, s => { s.provenance.latticeSize = 5; }]) {
        const f = fixture(), controller = new AbortController(); mutate(f.field);
        const pending = f.control.measureFluxSectors(f.args, controller.signal);
        await new Promise(resolve => setImmediate(resolve));
        await assert.rejects(f.control.measureFluxSectors(f.args, controller.signal), /already active/);
        controller.abort(); await assert.rejects(pending, /abort/i); f.control.dispose();
    }
});

test('a same-tick old visual cache waits for current-generation data', async () => {
    const f = fixture(); let version = 2;
    f.owner.getSamplerSnapshotVersion = () => version;
    const pending = f.control.measureFluxSectors(f.args, new AbortController().signal);
    await new Promise(resolve => setImmediate(resolve));
    await assert.rejects(f.control.measureFluxSectors(f.args, new AbortController().signal), /already active/);
    version = 3;
    assert.equal((await pending).status, 'measured'); f.control.dispose();
});

test('an unannounced same-tick intervention invalidates the pending read', async () => {
    const f = fixture();
    f.owner.getSamplerSnapshotVersion = () => 2;
    const pending = f.control.measureFluxSectors(f.args, new AbortController().signal);
    f.owner._visualInterventionEpoch++;
    await assert.rejects(pending, /changed/); f.control.dispose();
});

test('NaN omissions, changed canonical hashes and stale digest provenance cannot produce bounds', async () => {
    for (const mutate of [d => { d.nonfiniteValueCount = 1; }, d => { d.tick = '6'; },
        d => { d.sourceEpoch = '1'; }, d => { d.siteCount = 63; }, d => { d.hashLo = 'invalid'; }]) {
        const f = fixture(), original = f.owner.getDynamicalStateDigest;
        f.owner.getDynamicalStateDigest = async () => { const d = await original(); mutate(d); return d; };
        await assert.rejects(f.control.measureFluxSectors(f.args, new AbortController().signal), /digest/); f.control.dispose();
    }
    const f = fixture(), original = f.owner.getDynamicalStateDigest; let calls = 0;
    f.owner.getDynamicalStateDigest = async () => { const d = await original(); if (++calls === 2) d.hashLo = 'd'.repeat(16); return d; };
    await assert.rejects(f.control.measureFluxSectors(f.args, new AbortController().signal), /state changed/); f.control.dispose();
});

test('cancelling a pending digest settles promptly without waiting for the backend', async () => {
    const f = fixture(), controller = new AbortController(); let release;
    f.owner.getDynamicalStateDigest = () => new Promise(resolve => { release = resolve; });
    const pending = f.control.measureFluxSectors(f.args, controller.signal);
    await new Promise(resolve => setImmediate(resolve)); controller.abort();
    await assert.rejects(pending, /cancelled/); release({}); f.control.dispose();
});

function nativeAggregate(size = 97) {
    return { type: 'flux_sectors', schemaVersion: 1, compute: 'GPU', latticeSize: size, siteCount: size ** 3,
        nonfiniteValueCount: 0, relativeRoundingAllowance: 1e-6, stateVersion: '10', tick: '7',
        provenance: { ...sample().provenance, latticeSize: size },
        sectors: [0, 1].map(parity => ({ parity, totalSites: Math.floor(size ** 3 / 2) + Number(parity === 0 && size % 2 === 1),
            nonzeroSites: 1, squaredNorm: parity === 0 ? 25 : 4, maxAbsComponent: 4 - parity * 2 })) };
}

function nativeFixture(size = 97) {
    const f = fixture(), value = nativeAggregate(size);
    f.owner.latticeSize = size; f.owner.fluxSectorVersion = 1;
    f.owner.getFluxSectors = async () => value;
    f.owner.getFluxVectorSampled = () => assert.fail('Aggregate path must not request a capped visualization sampler');
    const digest = f.owner.getDynamicalStateDigest;
    f.owner.getDynamicalStateDigest = async () => ({ ...await digest(), latticeSize: size, siteCount: size ** 3 });
    return { ...f, value };
}

test('native Float64 reduction includes every site through size256 and zero omission floor', () => {
    for (const size of [4, 32, 97, 256]) {
        const result = normalizeNativeFluxSectors(nativeAggregate(size), size);
        assert.equal(result.sampledSites, size ** 3);
        assert.equal(result.sampling.kind, 'nativeAggregate');
        assert.equal(result.sampling.squaredNormFloor, 0);
        assert.equal(result.observedSquaredNorm, 29);
        for (const sector of result.sectors) {
            assert.equal(sector.sampledSites, sector.totalSites);
            assert.equal(sector.omittedSites, 0);
            assert.ok(sector.absoluteUnderflowAllowance > 0);
        }
    }
});

test('native rounded-zero squares retain conservative underflow allowance', () => {
    const value = nativeAggregate();
    value.sectors.forEach(s => { s.squaredNorm = 0; s.maxAbsComponent = 1e-200; });
    const result = normalizeNativeFluxSectors(value, 97);
    assert.equal(result.observedSquaredNorm, 0);
    assert.ok(result.squaredNormBounds.upper > 0);
    assert.equal(result.sectors[0].fractionBounds.upper, null);
});

test('native reducer rejects malformed coverage, overflow, nonfinite and unsupported metadata', () => {
    for (const change of [v => { v.schemaVersion = 2; }, v => { v.siteCount--; }, v => { v.nonfiniteValueCount = 1; },
        v => { v.sectors[0].parity = 1; }, v => { v.sectors[0].totalSites--; }, v => { v.sectors[0].nonzeroSites = -1; },
        v => { v.sectors[0].nonzeroSites = 0; }, v => { v.sectors[0].squaredNorm = Infinity; },
        v => { v.sectors[0].maxAbsComponent = NaN; }, v => { v.relativeRoundingAllowance = 0; },
        v => { v.sectors.forEach(s => { s.squaredNorm = 1e308; }); }]) {
        const value = nativeAggregate(); change(value);
        assert.throws(() => normalizeNativeFluxSectors(value, 97));
    }
    assert.throws(() => normalizeNativeFluxSectors(nativeAggregate(257), 257));
});

test('large native measurement is capability-gated and uses existing digest/aggregate only', async () => {
    const f = nativeFixture();
    const result = await f.control.measureFluxSectors(f.args, new AbortController().signal);
    assert.equal(result.sampledSites, 97 ** 3); assert.equal(result.integrity.unchanged, true);
    assert.equal(result.sampling.kind, 'nativeAggregate'); f.control.dispose();
    const old = nativeFixture(); old.owner.fluxSectorVersion = 0;
    old.owner.getFluxSectors = () => assert.fail('Never probe an unadvertised native command');
    await assert.rejects(old.control.measureFluxSectors(old.args, new AbortController().signal), /4..64/); old.control.dispose();
});

test('native aggregate keeps tick, source, epoch, digest and intervention fences', async () => {
    for (const change of [f => { f.value.provenance.sampleTick = '6'; }, f => { f.value.provenance.sourceEpoch = '1'; },
        f => { f.value.provenance.nativeInstanceId = 'b'.repeat(32); }, f => { f.value.provenance.epoch = '8'; },
        f => { f.value.stateVersion = '11'; }, f => { f.value.compute = 'CPU'; },
        f => { f.owner.getFluxSectors = async () => { f.owner._visualInterventionEpoch++; return f.value; }; }]) {
        const f = nativeFixture(); change(f);
        await assert.rejects(f.control.measureFluxSectors(f.args, new AbortController().signal)); f.control.dispose();
    }
});
