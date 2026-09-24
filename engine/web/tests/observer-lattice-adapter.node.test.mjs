import test from 'node:test';
import assert from 'node:assert/strict';
import { LatticeObservationAdapter } from '../js/observer/lattice-adapter.js';

const forbidden = () => { throw new Error('A passive lens must not call this method'); };
const mutations = { tick: forbidden, advance: forbidden, pump: forbidden, observe: forbidden,
    setRunning: forbidden, currentTick: forbidden, getParticleData: forbidden, getFluxVolume: forbidden,
    setTelemetryDemand: forbidden, dispose: forbidden };

function nativeFixture() {
    const published = { sourceEpoch: '9007199254741001', tick: '9007199254741111', stale: false,
        nativeInstanceId: 'native-owner',
        groups: { diagnostics: { tick: '9007199254741002', manifested: 8, positive: 5, negative: 3 } },
        groupMeta: { diagnostics: { tick: '9007199254741002', sampleTick: '9007199254741002',
            sourceEpoch: '9007199254741001', epoch: '9007199254741003', stateVersion: '9007199254741004', stale: false } } };
    const owner = { ...mutations, latticeSize: 33, getTelemetrySnapshot: () => published };
    const lens = new LatticeObservationAdapter(() => ({ owner, mode: 'lattice' }));
    return { published, owner, lens };
}

test('native diagnostics retain their completed sample and exact counters independently of snapshot time', () => {
    const { lens } = nativeFixture();
    const sample = lens.snapshot();
    assert.equal(sample.available, true);
    assert.equal(sample.tick, '9007199254741002');
    assert.equal(sample.sourceEpoch, '9007199254741001');
    assert.equal(sample.epoch, '9007199254741003');
    assert.equal(sample.stateVersion, '9007199254741004');
    assert.equal(sample.manifested, '8');
    assert.equal(sample.incidence, undefined);
    assert.equal(lens.snapshot().ownerIdentity, sample.ownerIdentity);
});

test('native staleness and source replacement never relabel held counts as current', () => {
    const { published, lens } = nativeFixture();
    published.groupMeta.diagnostics.stale = true;
    assert.equal(lens.snapshot().available, false);
    assert.equal(lens.snapshot().manifested, undefined);
    published.groupMeta.diagnostics.stale = false;
    published.sourceEpoch = '9007199254741005';
    assert.equal(lens.snapshot().available, false);
    assert.equal(lens.snapshot().status, 'Stale source observation');
    assert.equal(lens.snapshot().tick, '9007199254741002');
});

test('completed diagnostics copy only finite approved scalars without inventing energy or speed channels', () => {
    const { lens, published } = nativeFixture();
    Object.assign(published.groups.diagnostics, { physicalTime: 1.25, dt: .1, totalFlux: 0,
        maxBandwidth: .2, maxCausalBudget: .3, entropy: .4, totalEnergy: 100,
        dynamicEnergy: NaN, fieldEnergy: Infinity, waveEnergy: '2', particleKE: null,
        maxSpeed: 7, divergence: 5, avgDrag: 0 });
    const sample = lens.snapshot();
    assert.equal(sample.physicalTime, 1.25); assert.equal(sample.totalFlux, 0);
    assert.equal(sample.maxBandwidth, .2); assert.equal(sample.entropy, .4);
    for (const key of ['totalEnergy', 'dynamicEnergy', 'fieldEnergy', 'waveEnergy', 'particleKE', 'maxSpeed', 'divergence', 'avgDrag'])
        assert.equal(sample[key], undefined, key);
    published.groupMeta.diagnostics.stale = true;
    assert.equal(lens.snapshot().totalFlux, undefined);
});

test('worker energy components retain their source sample and do not trigger new audit acquisition', () => {
    const diag = { tick: 8, dynamicEnergy: -2, vacuumBaselineEnergy: 50,
        accountedEnergy: 3, restEnergy: 5, fieldEnergy: 1, waveEnergy: 2, particleKE: 0,
        energySampleSource: 'per-tick-ledger' };
    const owner = { ...mutations, isWorker: true, getEnergyAudit: forbidden, getDiagnostics: () => diag,
        getScale0TelemetryGroupMeta: () => ({ sourceEpoch: 1, sampleTick: 8, status: 'available' }) };
    const lens = new LatticeObservationAdapter(() => ({ owner, mode: 'lattice' }));
    const sample = lens.snapshot();
    assert.equal(sample.sampleTick, '8'); assert.equal(sample.dynamicEnergy, -2);
    assert.equal(sample.vacuumBaselineEnergy, 50); assert.equal(sample.fieldEnergy, 1);
    assert.equal(sample.energySampleSource, 'per-tick-ledger');
    delete diag.fieldEnergy;
    assert.equal(lens.snapshot().fieldEnergy, undefined);
});

test('worker reads only completed scalar caches and their own provenance', () => {
    const diag = { tick: '9007199254741000', manifested: 2 };
    const meta = { tick: '9007199254741000', sampleTick: '9007199254741000', sourceEpoch: 4,
        stateVersion: '9007199254741001', status: 'available', stale: false };
    const owner = { ...mutations, isWorker: true, getDiagnostics: () => diag,
        getScale0TelemetryGroupMeta: group => { assert.equal(group, 'diagnostics'); return meta; } };
    const lens = new LatticeObservationAdapter(() => ({ owner, mode: 'lattice' }));
    assert.equal(lens.snapshot().tick, '9007199254741000');
    assert.equal(lens.snapshot().available, true);
    meta.status = 'unavailable';
    assert.equal(lens.snapshot().available, false);
    meta.status = 'available'; meta.stale = true;
    assert.equal(lens.snapshot().available, false);
    meta.stale = false;
    diag.tick = '9007199254741001';
    assert.equal(lens.snapshot().available, false);
});

test('rounded or missing sample counters are unavailable, never manufactured from owner time', () => {
    const { lens, published } = nativeFixture();
    published.groupMeta.diagnostics.sampleTick = 9007199254741000;
    assert.equal(lens.snapshot().available, false);
    assert.equal(lens.snapshot().tick, null);
    delete published.groupMeta.diagnostics.sampleTick;
    delete published.groupMeta.diagnostics.tick;
    assert.equal(lens.snapshot().available, false);
});

test('finite completed record summaries preserve signed Q and cache once per immutable publication', () => {
    let reads = 0;
    const counted = new Proxy(['9007199254740993'], { get(target, key) {
        if (key === '0') reads++;
        return Reflect.get(target, key);
    } });
    let view = { microtick: '9007199254740999', lattice_size: '1', law_id: 'finite-law', phase: '2',
        field_tokens: counted, relation_tokens: ['2'], incidence: ['-9007199254740993'],
        manifestation_counts: [['3', '4', '5']] };
    const owner = { ...mutations, isWorker: true, isFiniteRecord: true, ready: true,
        get observation() { return view; }, getDiagnostics: forbidden,
        getProvenance: () => ({ ownerId: 'record-owner', generation: '7', law_id: 'finite-law',
            microtick: view.microtick, canonical_adoption: false, checkpoint_sha256: 'checkpoint' }) };
    const lens = new LatticeObservationAdapter(() => ({ owner, mode: 'lattice' }));
    const sample = lens.snapshot();
    assert.equal(sample.available, true);
    assert.equal(sample.fieldTokens, '9007199254740993');
    assert.equal(sample.incidence, '-9007199254740993');
    assert.equal(sample.manifested, '8');
    assert.equal(sample.zero, '4');
    assert.equal(sample.tick, view.microtick);
    assert.equal(sample.epoch, null);
    assert.equal(sample.dynamicEnergy, undefined); assert.equal(sample.totalFlux, undefined);
    assert.equal(reads, 1);
    lens.snapshot(); lens.snapshot();
    assert.equal(reads, 1);
    view = { ...view, microtick: '9007199254741000' };
    assert.equal(lens.snapshot().tick, view.microtick);
    assert.equal(reads, 2);
});

test('owner identity is stable while failures and unsupported owners fail closed without new acquisition', () => {
    let owner = { ...mutations, getDiagnostics: forbidden, ready: true };
    let mode = 'lattice';
    const lens = new LatticeObservationAdapter(() => ({ owner, mode }));
    const first = lens.snapshot();
    assert.equal(first.available, false);
    assert.equal(lens.snapshot().ownerIdentity, first.ownerIdentity);
    owner.ready = false;
    assert.equal(lens.snapshot().status, 'Loading');
    owner = { ...mutations, getDiagnostics: forbidden, ready: true };
    assert.notEqual(lens.snapshot().ownerIdentity, first.ownerIdentity);
    mode = 'atoms';
    assert.equal(lens.snapshot().available, false);
});
