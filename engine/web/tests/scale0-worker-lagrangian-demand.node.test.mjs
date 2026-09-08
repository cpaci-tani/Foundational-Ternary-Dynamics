import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';

const read = name => readFileSync(new URL(`../js/bridge/${name}`, import.meta.url), 'utf8');
const source = read('wasm-bridge.worker.js');
const frameSource = source.slice(source.indexOf('function postFrame('), source.indexOf('function loop()'));
const metaSource = source.slice(source.indexOf('function telemetryGroupMeta('), source.indexOf('function readEngineToggles()'));
const handler = source.slice(source.indexOf("      case 'setTelemetryMask': {"), source.indexOf("      case 'setRunning':"));
const cadence = { self: {} };
vm.runInNewContext(read('sampler-cadence.classic.js'), cadence);

// Actual production message case, full postFrame, metadata helper and cadence.
// Replace only native reductions, unrelated optional samplers and transport.
function fixture({ running = false, lag = false } = {}) {
    const ctrl = new Int32Array(new SharedArrayBuffer(8 * 4));
    const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, DATA_VERSION: 6 };
    ctrl[CTRL.RUNNING] = Number(running); ctrl[CTRL.DATA_VERSION] = 11; ctrl[CTRL.TICK] = 7;
    const frames = [], counts = { lag: 0, audit: 0 };
    let lagResult = { total: 2 }, throwLag = false;
    const scope = {
        ctrl, CTRL, Atomics, Float32Array, Uint8Array, N: 97,
        bridge: { currentTick: () => 7 }, activeConfigurationToken: 5,
        wantAudit: true, wantLag: lag, wantGravity: false,
        lastAudit: { dynamicEnergy: 3 }, auditFrameCounter: 1, diagnosticsStateVersion: 1,
        auditStateVersion: 1, lagrangianStateVersion: 0,
        lastAuditMeta: { status: 'available', sourceEpoch: 5, stateVersion: 1, sampleTick: 7 },
        lastLagrangianMeta: null, gravityMetricAggVersion: 11,
        gravitySamplerCadence: { reset() {} }, wantedSamplers: new Map(),
        publishFlux: () => true, lastFluxHeap: null, lastFluxPtr: 0, lastFluxLen: 0,
        engineTogglesDirty: false, publishDynamicalStateDigest: false,
        lastInspect: null, lastForceAt: null, performance: { now: () => 100 },
        self: { postMessage: value => frames.push(value) },
        cloneAudit: value => ({ ...value }),
        advanceDemandFrameCadence: cadence.self.FTD_SAMPLER_CADENCE.advanceDemandFrameCadence,
        mod: {
            getFluxVolume: () => new Float32Array(0), getDiagnostics: () => ({ tick: 7 }),
            getParticleData: () => null,
            getEnergyAudit() { counts.audit++; return { dynamicEnergy: 3 }; },
            getLagrangian() { counts.lag++; if (throwLag) throw Error('injected unavailable ABI'); return lagResult; },
        },
    };
    vm.createContext(scope);
    const api = vm.runInContext(`${metaSource}\n${frameSource}\n({
        mask(msg) { switch ('setTelemetryMask') { ${handler} } },
        publish: postFrame,
    })`, scope);
    return { scope, api, frames, counts, ctrl, CTRL,
        mask(value) { api.mask({ wantAudit: true, wantLag: value, wantGravity: false }); },
        set lagResult(value) { lagResult = value; }, set throwLag(value) { throwLag = value; } };
}

test('paused audit-on lag-off to lag-on publishes one current observation without physics advance', () => {
    const f = fixture(); f.mask(true);
    assert.equal(f.counts.lag, 1); assert.equal(f.frames.length, 1);
    const m = f.frames[0]; assert.equal(m.lag.total, 2); assert.equal(m.lagMeta.status, 'available');
    assert.equal(m.tick, 7); assert.equal(m.lagMeta.sampleTick, 7);
    assert.equal(m.configurationToken, 5); assert.equal(m.lagMeta.sourceEpoch, 5);
    assert.equal(m.dataVersion, 11); assert.equal(f.ctrl[f.CTRL.DATA_VERSION], 11);
    assert.equal(f.counts.audit, 0); // Existing independent audit cadence remains intact.
});

test('unchanged masks perform no duplicate publication or reductions', () => {
    const f = fixture(); f.mask(false); assert.equal(f.frames.length, 0);
    f.mask(true); for (let i = 0; i < 8; i++) f.mask(true);
    assert.equal(f.counts.lag, 1); assert.equal(f.frames.length, 1);
});

test('paused deactivation publishes null and a distinct inactive observation only once', () => {
    const f = fixture(); f.mask(true); const active = f.frames.at(-1).lagMeta;
    f.mask(false); const m = f.frames.at(-1);
    assert.equal(f.frames.length, 2); assert.equal(f.counts.lag, 1); assert.equal(m.lag, null);
    assert.equal(m.lagMeta.status, 'inactive'); assert.equal(m.lagMeta.stale, true);
    assert.ok(m.lagMeta.stateVersion > active.stateVersion);
    assert.equal(m.lagMeta.sourceEpoch, 5); assert.equal(m.tick, 7); assert.equal(m.dataVersion, 11);
    f.mask(false); assert.equal(f.frames.length, 2);
    f.api.publish(false); assert.equal(f.frames.at(-1).lagMeta.stateVersion, m.lagMeta.stateVersion);
});

test('initial lag deactivation has explicit metadata even before its first sample', () => {
    const f = fixture({ lag: true }); f.mask(false);
    assert.equal(f.frames.length, 1); assert.equal(f.counts.lag, 0);
    assert.equal(f.frames[0].lagMeta.status, 'inactive'); assert.equal(f.frames[0].lagMeta.sourceEpoch, 5);
});

test('playing demand transitions preserve publication cadence', () => {
    const f = fixture({ running: true }); f.mask(true);
    assert.equal(f.frames.length, 0); assert.equal(f.counts.lag, 0);
    f.api.publish(true); assert.equal(f.counts.lag, 1); assert.equal(f.frames.length, 1);
    f.mask(false); assert.equal(f.frames.length, 1);
    f.api.publish(true); assert.equal(f.counts.lag, 1); assert.equal(f.frames.at(-1).lagMeta.status, 'inactive');
});

test('inactive generation change receives that generation metadata with no reduction', () => {
    const f = fixture({ lag: true }); f.mask(false); const old = f.frames.at(-1).lagMeta;
    f.scope.activeConfigurationToken = 6; f.api.publish(false);
    const next = f.frames.at(-1).lagMeta;
    assert.equal(next.sourceEpoch, 6); assert.ok(next.stateVersion > old.stateVersion); assert.equal(f.counts.lag, 0);
});

test('reactivation samples afresh; unavailable and error outcomes never retain old payload', () => {
    const f = fixture(); f.mask(true); const version = f.frames.at(-1).lagMeta.stateVersion;
    f.mask(false); f.lagResult = null; f.mask(true);
    assert.equal(f.frames.at(-1).lag, null); assert.equal(f.frames.at(-1).lagMeta.status, 'unavailable');
    assert.ok(f.frames.at(-1).lagMeta.stateVersion > version);
    f.mask(false); f.throwLag = true; f.mask(true);
    assert.equal(f.frames.at(-1).lag, null); assert.equal(f.frames.at(-1).lagMeta.status, 'error');
});

test('missing bridge cannot publish during mask changes', () => {
    const f = fixture(); f.scope.bridge = null; f.mask(true);
    assert.equal(f.frames.length, 0); assert.equal(f.counts.lag, 0);
});
