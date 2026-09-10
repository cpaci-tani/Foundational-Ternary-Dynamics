import test from 'node:test';
import assert from 'node:assert/strict';
import { classifyScale0Validity, classifyScale0RuntimeFailure, createScale0ValidityMonitor }
    from '../js/scales/scale0/runtime/validity.js';

const current = (diag = { tick: 4, manifested: 0, dynamicEnergy: 0 }, extra = {}) => ({
    diag, diagMeta: { tick: 4, stale: false, status: 'available', sourceEpoch: 1 },
    diagCurrent: true, ...extra,
});
const qualified = (generation = 1, scenarioId = 'flux-pulse') => ({
    status: 'within-contract', scenarioId, mutationEpoch: 0, authoritativeLoad: null,
    anchor: { scenarioId, loadGeneration: generation, mutationEpoch: 0 },
});

test('Clear is limited to current supplied scalar checks, including exact zero', () => {
    const result = classifyScale0Validity(current());
    assert.equal(result.label, 'Clear');
    assert.match(result.details, /No numerical issue detected in the available readings/);
    assert.match(result.details, /Checks available Scale 0 readings/);
    assert.match(result.details, /Gravitational singularities are not evaluated/);
    assert.match(result.details, /audit unavailable for this tick/);
});

test('absent, stale, unavailable and tick-only telemetry remain Unchecked', () => {
    for (const values of [{}, current(null), current({ tick: 4 }), current({}, {}),
        current(undefined, { diagCurrent: false }),
        current({ tick: 4, dt: NaN }, { diagMeta: { tick: 4, stale: true } }),
        current(undefined, { diagMeta: { tick: 4, status: 'pending' } })]) {
        assert.equal(classifyScale0Validity(values).label, 'Unchecked');
    }
});

test('NaN, Infinity, unsafe integer, invalid counter and wrong scalar types have distinct outcomes', () => {
    for (const [field, value, expected] of [
        ['dynamicEnergy', NaN, 'Undefined'], ['totalFlux', Infinity, 'Overflow'],
        ['maxBandwidth', -Infinity, 'Overflow'], ['tick', 2 ** 53, 'Overflow'],
        ['manifested', 2 ** 53, 'Overflow'], ['tick', -1, 'Invalid'],
        ['causalProjectionEvents', 1.5, 'Invalid'], ['dt', 'NaN', 'Invalid'],
    ]) assert.equal(classifyScale0Validity(current({ tick: 4, manifested: 0, [field]: value })).label, expected);
});

test('optional null values are not failures and exact bigint counters are not rounded', () => {
    assert.equal(classifyScale0Validity(current({ tick: 4, manifested: 0,
        dynamicEnergy: null, entropy: undefined })).label, 'Clear');
    assert.equal(classifyScale0Validity(current({ tick: 4, manifested: 2n ** 54n })).label, 'Clear');
    assert.equal(classifyScale0Validity(current({ tick: 4, manifested: -1n })).label, 'Invalid');
});

test('finite magnitudes and diagnostic drift have no invented physical threshold', () => {
    const result = classifyScale0Validity(current({ tick: 4, dynamicEnergy: 1e300, dt: 0 },
        { auditCurrent: true, audit: { gaussViolation: 1e200, energyDrift: -1e200 } }));
    assert.equal(result.label, 'Clear');
});

test('only already-current audit values are checked; copied audit fields do not gain freshness', () => {
    assert.equal(classifyScale0Validity(current(undefined, { audit: { energyDrift: NaN } })).label, 'Clear');
    assert.equal(classifyScale0Validity(current(undefined,
        { auditCurrent: true, audit: { energyDrift: NaN } })).label, 'Undefined');
    assert.equal(classifyScale0Validity(current({ tick: 4, manifested: 0, waveEnergy: NaN })).label, 'Clear');
});

test('supplied metadata counters are checked even when current-tick predicate rejects the sample', () => {
    assert.equal(classifyScale0Validity(current(undefined,
        { diagCurrent: false, diagMeta: { tick: NaN, stale: false } })).label, 'Undefined');
    assert.equal(classifyScale0Validity(current(undefined,
        { diagMeta: { tick: 4, stale: false, sourceEpoch: 2 ** 53 } })).label, 'Overflow');
});

test('unsupported arrays and methods are not scanned or called', () => {
    const diag = { tick: 4, manifested: 0 };
    Object.defineProperty(diag, 'grid', { get() { throw new Error('grid read'); } });
    Object.defineProperty(diag, 'getEnergyAudit', { get() { throw new Error('bridge call'); } });
    assert.equal(classifyScale0Validity(current(diag)).label, 'Clear');
});

test('runtime errors retain exact text and explicit overflow/NaN wording', () => {
    assert.match(classifyScale0RuntimeFailure(new Error('worker stopped: exact detail')).details, /worker stopped: exact detail/);
    assert.equal(classifyScale0RuntimeFailure('worker stopped').label, 'Runtime');
    assert.equal(classifyScale0RuntimeFailure('tick overflow').label, 'Overflow');
    assert.equal(classifyScale0RuntimeFailure('NaN in diagnostic').label, 'Undefined');
});

test('load, mutation, mode and runtime lifecycle cannot retain a false Clear', () => {
    let displayed;
    const monitor = createScale0ValidityMonitor({ set(value) { displayed = value; } });
    monitor.sample(current());
    assert.equal(displayed.label, 'Unchecked');
    monitor.setQualification(qualified()); monitor.sample(current());
    assert.equal(displayed.label, 'Clear');
    monitor.setQualification({ ...qualified(), status: 'suspended', mutationEpoch: 1 });
    monitor.sample(current()); assert.equal(displayed.label, 'Unchecked');
    monitor.setQualification(qualified());
    monitor.runtimeFailure('owned worker stopped'); monitor.sample(current());
    assert.equal(displayed.label, 'Runtime');
    monitor.setQualification(qualified()); assert.equal(displayed.label, 'Runtime');
    monitor.setMode('particles'); monitor.sample(current());
    assert.equal(displayed.label, 'Unchecked'); assert.match(displayed.details, /Scale 0.*particles/);
    monitor.setMode('lattice'); monitor.sample(current()); assert.equal(displayed.label, 'Runtime');
    monitor.setQualification({ ...qualified(2), status: 'pending',
        authoritativeLoad: { status: 'pending', scenarioId: 'flux-pulse', loadGeneration: 2 } });
    monitor.sample(current()); assert.equal(displayed.label, 'Unchecked');
    monitor.setQualification(qualified(2)); monitor.sample(current()); assert.equal(displayed.label, 'Clear');
});

test('failed setup is a runtime fault and a different successful scenario clears it', () => {
    let displayed;
    const monitor = createScale0ValidityMonitor({ set(value) { displayed = value; } });
    monitor.setQualification({ ...qualified(), status: 'pending', authoritativeLoad: {
        status: 'failed', scenarioId: 'flux-pulse', loadGeneration: 1, failureReason: 'load rejected exactly',
    } });
    monitor.sample(current());
    assert.equal(displayed.label, 'Runtime'); assert.match(displayed.details, /load rejected exactly/);
    monitor.setQualification(qualified(1, 'empty')); monitor.sample(current());
    assert.equal(displayed.label, 'Clear');
});

test('issue details carry sample context and runtime failures retain the last observed tick', () => {
    const issue = classifyScale0Validity(current({ tick: 4, dynamicEnergy: NaN }, { context: qualified(7) }));
    assert.match(issue.details, /dynamicEnergy is NaN/);
    assert.match(issue.details, /Sample tick: 4; scenario: flux-pulse; load: 7; change: 0/);
    let displayed;
    const monitor = createScale0ValidityMonitor({ set(value) { displayed = value; } });
    monitor.setQualification(qualified(7)); monitor.sample(current());
    monitor.runtimeFailure('retained exact error');
    assert.match(displayed.details, /Last observed tick: 4/);
    const retained = displayed.details;
    monitor.setQualification(qualified(7));
    assert.equal(displayed.label, 'Runtime'); assert.equal(displayed.details, retained);
    monitor.setQualification(qualified(8)); monitor.runtimeFailure('new load error');
    assert.match(displayed.details, /Last observed tick: unavailable/);
});
