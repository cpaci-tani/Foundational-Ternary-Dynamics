import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import '../js/bridge/flux-publication.classic.js';

const read = name => readFileSync(new URL(`../js/bridge/${name}`, import.meta.url), 'utf8');
const source = read('wasm-bridge.worker.js');
const gravityObservationSource = source.slice(
    source.indexOf('const GRAVITY_OBSERVATION_TOGGLE_KEYS'),
    source.indexOf('let lastInspect = null'),
);
const frameSource = source.slice(source.indexOf('function postFrame('), source.indexOf('function loop()'));
const metaSource = source.slice(source.indexOf('function telemetryGroupMeta('), source.indexOf('function readEngineToggles()'));
const lagReadSource = source.slice(source.indexOf('function readLagrangian()'), source.indexOf('function applyCommand('));
const applyCommandSource = source.slice(source.indexOf('function applyCommand('), source.indexOf('function readFluxBoundaryMode()'));
const handler = source.slice(source.indexOf("      case 'setTelemetryMask': {"), source.indexOf("      case 'setRunning':"));
const commandHandler = source.slice(source.indexOf("      case 'command': {"), source.indexOf("      case 'toggleBatch': {"));
const cadence = { self: {} };
vm.runInNewContext(read('sampler-cadence.classic.js'), cadence);

test('bounded retry cadence never resamples one tick and backs off by cost', () => {
    const gate = cadence.self.FTD_SAMPLER_CADENCE.createBoundedReductionCadence();
    assert.equal(gate.shouldRun(true, false, 0, 7), true);
    gate.complete(0, 7, 20);
    assert.equal(gate.shouldRun(true, true, 300, 7), false, 'one tick has one observation');
    assert.equal(gate.shouldRun(true, true, 124, 8), false);
    assert.equal(gate.shouldRun(true, true, 125, 8), true);
    gate.complete(125, 8, 80);
    assert.equal(gate.nextDueAt, 445, '80ms work reserves 240ms before the next 25%-duty observation');
    assert.equal(gate.shouldRun(true, true, 444, 9), false);
    assert.equal(gate.shouldRun(true, true, 445, 9), true);
    assert.equal(gate.shouldRun(false, true, 446, 10), false);
    assert.equal(gate.lastTick, null, 'hidden demand resets the source observation boundary');
});

test('bounded cadence treats null as an unknown clock, never tick zero', () => {
    const gate = cadence.self.FTD_SAMPLER_CADENCE.createBoundedReductionCadence();
    assert.equal(gate.shouldRun(true, false, 0, null), true);
    gate.complete(0, null, 5);
    assert.equal(gate.lastTick, null);
    assert.equal(gate.shouldRun(true, false, 100, null), false, 'unknown-clock failure waits for cooldown');
    assert.equal(gate.shouldRun(true, false, 125, null), true);
    gate.complete(125, null, 5);
    assert.equal(gate.shouldRun(true, true, 500, null), false, 'a successful unknown-clock sample is retained');
});

test('a forced fresh Gravity state does not accelerate co-owned proper-time samplers', () => {
    const samplerCadence = cadence.self.FTD_SAMPLER_CADENCE.createBoundedSamplerCadence(250);
    const wants = new Map([
        ['latency@2', { kind: 'latency', stride: 2, cadenceClass: 'bounded-instrument' }],
        ['kretschmann@2', { kind: 'kretschmann', stride: 2, cadenceClass: 'bounded-instrument' }],
        ['gravity@2', { kind: 'gravity', stride: 2, cadenceClass: 'bounded-instrument' }],
        ['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0, cadenceClass: 'bounded-instrument' }],
        ['tau@2', { kind: 'tau', stride: 2, cadenceClass: 'bounded-instrument' }],
    ]);
    const visit = (nowMs, forceGravityBatch) => {
        const keys = [];
        cadence.self.FTD_SAMPLER_CADENCE.visitScheduledSamplers(wants, {
            wantGravity: true, wantProperTime: true, cadence: samplerCadence, nowMs, forceGravityBatch,
        }, key => keys.push(key));
        return keys.sort();
    };
    assert.deepEqual(visit(0, false),
        ['gravity@2', 'gravityMetricAgg@0', 'kretschmann@2', 'latency@2', 'tau@2']);
    assert.deepEqual(visit(16, true),
        ['gravity@2', 'gravityMetricAgg@0', 'kretschmann@2', 'latency@2']);
});

// Actual production message case, full postFrame, metadata helper and cadence.
// Replace only native reductions, unrelated optional samplers and transport.
function fixture({ running = false, lag = false, wantGravity = false, wantProperTime = false,
    samplerEntries = [], latticeSize = 97, validFluxVolume = false,
    missingGravitySampler = false } = {}) {
    const ctrl = new Int32Array(new SharedArrayBuffer(8 * 4));
    const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, DATA_VERSION: 6 };
    ctrl[CTRL.RUNNING] = Number(running); ctrl[CTRL.DATA_VERSION] = 11; ctrl[CTRL.TICK] = 7;
    const frames = [], counts = { lag: 0, audit: 0, flux: 0, latency: 0, kretschmann: 0, gravity: 0, gravityMetricAgg: 0 };
    let now = 0;
    let lagResult = { total: 2 }, throwLag = false;
    const scope = {
        backgroundSuspended: false,
        ctrl, CTRL, Atomics, Float32Array, Uint8Array, N: latticeSize,
        bridge: { currentTick: () => 7 }, activeConfigurationToken: 5, renderBridgeGeneration: 1,
        WORKER_COMMAND_ALLOWLIST: new Set(['setToggle']),
        wantAudit: true, wantLag: lag, wantGravity, wantProperTime,
        lastAudit: { dynamicEnergy: 3 }, auditFrameCounter: 1, diagnosticsStateVersion: 1,
        lastLagrangian: null, lagrangianFrameCounter: 0,
        auditStateVersion: 1, lagrangianStateVersion: 0,
        lastAuditMeta: { status: 'available', sourceEpoch: 5, stateVersion: 1, sampleTick: 7 },
        lastLagrangianMeta: null, gravityMetricAggVersion: 11,
        gravitySamplerCadence: cadence.self.FTD_SAMPLER_CADENCE.createBoundedSamplerCadence(),
        gravityObservationRetryCadence: cadence.self.FTD_SAMPLER_CADENCE.createBoundedReductionCadence(),
        gravityObservationRevision: 0, lastGravityObservationIdentity: null,
        gravityObservationSupportSignature: '',
        wantedSamplers: new Map(samplerEntries.map(([key, want]) => [key, {
            cadenceClass: 'bounded-instrument', ...want,
        }])),
        SAMPLER_METHODS: {
            latency: ['getLatencySampled', 'val'],
            kretschmann: ['getKretschmannSampled', 'val'],
            gravity: ['getGravitySampled', 'vec'],
            gravityMetricAgg: ['getGravityMetricAgg', 'obj'],
        },
        visitScheduledSamplers: cadence.self.FTD_SAMPLER_CADENCE.visitScheduledSamplers,
        publishFlux: () => true, lastFluxHeap: null, lastFluxPtr: 0, lastFluxLen: 0,
        engineTogglesDirty: false, publishDynamicalStateDigest: false,
        readEngineToggles: () => ({}),
        telemetryTimingEnabled: false,
        telemetryTiming: {
            lagrangianAttempts: 0, lagrangianCompleted: 0, lagrangianLastMs: null,
            lagrangianTotalMs: 0, lagrangianMaxMs: 0,
            tickCompleted: 0, tickLastMs: null, tickTotalMs: 0, tickMaxMs: 0,
        },
        lastInspect: null, lastForceAt: null, performance: { now: () => now },
        self: { postMessage: value => frames.push(value), FTD_FLUX_PUBLICATION: globalThis.FTD_FLUX_PUBLICATION },
        cloneAudit: value => ({ ...value }),
        advanceDemandFrameCadence: cadence.self.FTD_SAMPLER_CADENCE.advanceDemandFrameCadence,
        createBoundedReductionCadence: cadence.self.FTD_SAMPLER_CADENCE.createBoundedReductionCadence,
        lagrangianCadence: cadence.self.FTD_SAMPLER_CADENCE.createBoundedReductionCadence(),
        mod: {
            getFluxVolume: () => {
                counts.flux++;
                return validFluxVolume
                    ? new Float32Array(latticeSize ** 3).fill(1)
                    : new Float32Array(0);
            },
            getDiagnostics: () => ({ tick: 7 }),
            getLatencySampled: () => { counts.latency++; return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 }; },
            getKretschmannSampled: () => { counts.kretschmann++; return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 }; },
            getGravitySampled: () => {
                counts.gravity++;
                return missingGravitySampler ? null : { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
            },
            getGravityMetricAgg: () => { counts.gravityMetricAgg++; return { active: false, requested: false, latencyMax: 0, latencyMean: 0, fMin: 1, gammaMax: 1, dilationMaxPct: 0, voxelCount: 0 }; },
            getParticleData: () => null,
            getEnergyAudit() { counts.audit++; return { dynamicEnergy: 3 }; },
            getLagrangian() { counts.lag++; if (throwLag) throw Error('injected unavailable ABI'); return lagResult; },
            setToggle() { return true; },
        },
    };
    scope.telemetryTimingSnapshot = () => ({
        lagrangian: { attempts: scope.telemetryTiming.lagrangianAttempts,
            completed: scope.telemetryTiming.lagrangianCompleted,
            lastMs: scope.telemetryTiming.lagrangianLastMs },
        tick: { completed: scope.telemetryTiming.tickCompleted,
            lastMs: scope.telemetryTiming.tickLastMs },
    });
    vm.createContext(scope);
    const api = vm.runInContext(`${gravityObservationSource}\n${metaSource}\n${lagReadSource}\n${applyCommandSource}\n${frameSource}\n({
        mask(msg) { switch ('setTelemetryMask') { ${handler} } },
        command(msg) { switch ('command') { ${commandHandler} } },
        publish: postFrame,
    })`, scope);
    return { scope, api, frames, counts, ctrl, CTRL,
        advance(ms) { now += ms; },
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

test('worker prefers the exact compact Lagrangian view over Embind object transport', () => {
    const f = fixture();
    f.scope.mod.getLagrangian = () => { throw new Error('object transport must not run'); };
    f.scope.mod.getLagrangianView = () => Float64Array.from([
        1, 2, 3, 4, 5, 6, 7, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    ]);
    f.mask(true);
    const lag = f.frames.at(-1).lag;
    assert.deepEqual({
        fieldKinetic: lag.fieldKinetic, total: lag.total, hamiltonian: lag.hamiltonian,
        manifested: lag.manifested, cellVolume: lag.cellVolume,
    }, { fieldKinetic: 1, total: 28, hamiltonian: 29, manifested: 35, cellVolume: 37 });
});

test('worker timing is opt-in and reports an exact Lagrangian attempt duration', () => {
    const f = fixture();
    f.scope.telemetryTimingEnabled = true;
    f.mask(true);
    const timing = f.frames.at(-1).telemetryTiming;
    assert.equal(timing.lagrangian.attempts, 1);
    assert.equal(timing.lagrangian.completed, 1);
    assert.equal(timing.lagrangian.lastMs, 0);
    f.scope.telemetryTimingEnabled = false;
    f.scope.bridge.currentTick = () => 8;
    f.api.publish(true);
    assert.equal('telemetryTiming' in f.frames.at(-1), false);
});

test('unchanged masks perform no duplicate publication or reductions', () => {
    const f = fixture(); f.mask(false); assert.equal(f.frames.length, 0);
    f.mask(true); for (let i = 0; i < 8; i++) f.mask(true);
    assert.equal(f.counts.lag, 1); assert.equal(f.frames.length, 1);
});

test('an incomplete worker mask defaults expensive telemetry demand off', () => {
    const f = fixture();
    f.api.mask({});
    assert.equal(f.frames.length, 1, 'paused demand boundary publishes an explicit inactive cache state');
    assert.equal(f.counts.audit, 0);
    assert.equal(f.counts.lag, 0);
    assert.equal(f.frames[0].auditMeta.status, 'inactive');
    assert.equal(f.frames[0].lagMeta.status, 'inactive');
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

test('L=97 samples every distinct completed state and never rescans a paused state', () => {
    const f = fixture({ running: true });
    f.mask(true);
    f.api.publish(true);
    assert.equal(f.counts.lag, 1);
    const first = f.frames.at(-1).lagMeta;
    for (let tick = 8; tick <= 14; tick++) {
        f.scope.bridge.currentTick = () => tick;
        f.advance(16); f.api.publish(true);
    }
    assert.equal(f.counts.lag, 8, 'each completed worker state takes one exact observation');
    f.api.publish(true); f.api.publish(true);
    assert.equal(f.counts.lag, 8, 'repeated paused/readback frames cannot rescan the same tick');
    const retained = f.frames.at(-1).lagMeta;
    assert.ok(retained.stateVersion > first.stateVersion);
    assert.equal(retained.sampleTick, 14);
    f.scope.bridge.currentTick = () => 15; f.api.publish(true);
    assert.equal(f.counts.lag, 9, 'the next completed state advances provenance once');
    assert.ok(f.frames.at(-1).lagMeta.stateVersion > first.stateVersion);
});

test('a same-tick worker mutation invalidates and replaces the Lagrangian observation once', () => {
    const f = fixture({ running: true });
    f.mask(true);
    f.api.publish(true);
    const first = f.frames.at(-1).lagMeta;
    assert.equal(f.counts.lag, 1);
    assert.equal(first.sampleTick, 7);

    // A toggle command changes engine state but does not necessarily advance the
    // ordinal tick. Its synchronous postFrame must therefore invalidate the
    // retained tick-7 Lagrangian and publish a new stateVersion at tick 7.
    f.api.command({ configurationToken: 5, method: 'setToggle', args: ['gravity', true] });
    const mutated = f.frames.at(-1).lagMeta;
    assert.equal(f.counts.lag, 2);
    assert.equal(mutated.sampleTick, 7);
    assert.equal(mutated.sourceEpoch, 5);
    assert.ok(mutated.stateVersion > first.stateVersion);

    f.api.publish(false);
    assert.equal(f.counts.lag, 2, 'readonly same-tick publication retains the post-mutation observation');
    assert.deepEqual(f.frames.at(-1).lagMeta, mutated);
});

test('unavailable worker Lagrangian retries stay cooldown-bounded across new states', () => {
    const f = fixture({ running: true });
    f.lagResult = null;
    f.mask(true); f.api.publish(true);
    assert.equal(f.counts.lag, 1);
    f.scope.bridge.currentTick = () => 8; f.advance(100); f.api.publish(true);
    assert.equal(f.counts.lag, 1, 'an unavailable getter cannot retry once per state before cooldown');
    f.scope.bridge.currentTick = () => 9; f.advance(25); f.api.publish(true);
    assert.equal(f.counts.lag, 2);
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

test('Time-only or incomplete Gravity demand never performs a second flux-volume read', () => {
    const timeOnly = fixture({
        wantProperTime: true,
        samplerEntries: [['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0 }]],
    });
    timeOnly.api.publish(true);
    assert.equal(timeOnly.counts.flux, 1,
        'the ordinary frame publication remains, but Time-only aggregate demand has no Gravity slab read');
    assert.equal(timeOnly.frames.at(-1).gravityObservation, null);

    const incomplete = fixture({
        wantGravity: true,
        samplerEntries: [
            ['latency@6', { kind: 'latency', stride: 6 }],
            ['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0 }],
        ],
    });
    incomplete.api.publish(true);
    assert.equal(incomplete.counts.flux, 1,
        'an aggregate plus one scalar is not a complete L/K/F batch and must not take a second raw volume');
    assert.equal(incomplete.frames.at(-1).gravityObservation, null);
});

test('a complete Gravity batch reuses the frame-owned volume instead of rereading WASM', () => {
    const f = fixture({
        wantGravity: true,
        samplerEntries: [
            ['latency@6', { kind: 'latency', stride: 6 }],
            ['kretschmann@6', { kind: 'kretschmann', stride: 6 }],
            ['gravity@6', { kind: 'gravity', stride: 6 }],
            ['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0 }],
        ],
    });
    assert.doesNotThrow(() => f.api.publish(true));
    assert.equal(f.counts.flux, 1, 'the bounded candidate is copied from the ordinary frame volume');
    assert.equal(f.frames.length, 1, 'the ordinary frame still publishes');
    assert.equal(f.frames.at(-1).gravityObservation, null);
    assert.equal(f.frames.at(-1).tick, 7);
});

test('Gravity commits one atomic bundle per completed or same-tick-mutated state', () => {
    const samplerEntries = [
        ['latency@2', { kind: 'latency', stride: 2 }],
        ['kretschmann@2', { kind: 'kretschmann', stride: 2 }],
        ['gravity@2', { kind: 'gravity', stride: 2 }],
        ['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0 }],
    ];
    const f = fixture({ running: true, wantGravity: true, samplerEntries,
        latticeSize: 3, validFluxVolume: true });

    f.api.publish(true);
    const first = f.frames.at(-1).gravityObservation;
    assert.ok(first, JSON.stringify(f.frames.at(-1)));
    assert.equal(first.sampleTick, 7);
    assert.equal(f.counts.flux, 1);
    assert.deepEqual(f.counts, {
        lag: 0, audit: 0, flux: 1, latency: 1, kretschmann: 1, gravity: 1, gravityMetricAgg: 1,
    });

    f.api.publish(false);
    assert.equal(f.frames.at(-1).gravityObservation, null, 'same-tick readback retains the proxy bundle without rebuilding it');
    assert.deepEqual(
        [f.counts.latency, f.counts.kretschmann, f.counts.gravity, f.counts.gravityMetricAgg],
        [1, 1, 1, 1], 'paused same-tick readback does not repeat Gravity samplers',
    );
    f.advance(1000); f.api.publish(false);
    assert.deepEqual(
        [f.counts.latency, f.counts.kretschmann, f.counts.gravity, f.counts.gravityMetricAgg],
        [1, 1, 1, 1], 'the former 4 Hz timer cannot resample a committed paused state',
    );

    f.api.command({ configurationToken: 5, method: 'setToggle', args: ['gravity', true] });
    const mutated = f.frames.at(-1).gravityObservation;
    assert.equal(mutated.sampleTick, 7, 'a same-tick mutation has honest unchanged ordinal time');
    assert.notEqual(mutated.dataVersion, first.dataVersion, 'the mutation has a distinct worker state version');
    assert.deepEqual(
        [f.counts.latency, f.counts.kretschmann, f.counts.gravity, f.counts.gravityMetricAgg],
        [2, 2, 2, 2],
    );

    f.scope.bridge.currentTick = () => 8;
    f.api.publish(true);
    assert.equal(f.frames.at(-1).gravityObservation.sampleTick, 8);
    assert.deepEqual(
        [f.counts.latency, f.counts.kretschmann, f.counts.gravity, f.counts.gravityMetricAgg],
        [3, 3, 3, 3], 'each completed worker state receives one coherent batch',
    );
});

test('adding a complete Gravity support stride replaces a paused same-tick bundle', () => {
    const f = fixture({ wantGravity: true, latticeSize: 3, validFluxVolume: true, samplerEntries: [
        ['latency@2', { kind: 'latency', stride: 2 }],
        ['kretschmann@2', { kind: 'kretschmann', stride: 2 }],
        ['gravity@2', { kind: 'gravity', stride: 2 }],
        ['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0 }],
    ] });
    f.api.publish(true);
    assert.deepEqual(Array.from(f.frames.at(-1).gravityObservation.samples, s => s.stride), [2]);

    for (const [key, kind] of [['latency@4', 'latency'], ['kretschmann@4', 'kretschmann'], ['gravity@4', 'gravity']]) {
        f.scope.wantedSamplers.set(key, { kind, stride: 4, cadenceClass: 'bounded-instrument' });
    }
    f.api.publish(false);
    const expanded = f.frames.at(-1).gravityObservation;
    assert.equal(expanded.sampleTick, 7);
    assert.deepEqual(Array.from(expanded.samples, s => s.stride), [2, 4]);
    assert.deepEqual(
        [f.counts.latency, f.counts.kretschmann, f.counts.gravity, f.counts.gravityMetricAgg],
        [3, 3, 3, 2], 'the support-set boundary samples both complete strides exactly once',
    );
});

test('failed Gravity batches remain retry-bounded across newer worker states', () => {
    const f = fixture({ running: true, wantGravity: true, latticeSize: 3,
        validFluxVolume: true, missingGravitySampler: true, samplerEntries: [
            ['latency@2', { kind: 'latency', stride: 2 }],
            ['kretschmann@2', { kind: 'kretschmann', stride: 2 }],
            ['gravity@2', { kind: 'gravity', stride: 2 }],
            ['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0 }],
        ] });
    f.api.publish(true);
    assert.equal(f.frames.at(-1).gravityObservation, null);
    assert.equal(f.counts.gravity, 1);

    f.scope.bridge.currentTick = () => 8;
    f.advance(100); f.api.publish(true);
    assert.equal(f.counts.gravity, 1, 'a new state before retry cooldown cannot trigger another failed batch');

    f.scope.bridge.currentTick = () => 9;
    f.advance(25); f.api.publish(true);
    assert.equal(f.counts.gravity, 2, 'a later state retries after the bounded interval');
});
