import test from 'node:test';
import assert from 'node:assert/strict';
import { ensureOverlaySched, buildOverlayJobs, runJob } from '../js/scales/scale0/runtime/overlay-jobs.js';
import { createFieldSampleCache, createForceFieldCache } from '../js/scales/scale0/runtime/field-sample-cache.js';
import { SCALAR_JOBS } from '../js/scales/scale0/runtime/overlay-derived-data.js';
import { disposeFieldOverlayRuntime } from '../js/scales/scale0/runtime/field-overlays.js';
import { submitStreamlineJob } from '../js/scales/scale0/runtime/overlay-worker-client.js';

test('separate overlay owners execute a complete field/force/scalar sweep from shared samples', () => {
    const sample = { count: 2, positions: new Float32Array([1, 1, 1, 2, 1, 1]),
        vectors: new Float32Array([1, 0, 0, 0, 1, 0]), values: new Float32Array([1, .5]) };
    const reads = new Map();
    const capability = {
        getScale0FieldSamples({ kind, stride }) {
            const key = `${kind}@${stride}`;
            reads.set(key, (reads.get(key) || 0) + 1);
            return sample;
        },
        getScale0ParticleFrame: () => ({ count: 0, positions: new Float32Array() }),
        getScale0ForceField: () => sample,
        getScale0FluxVolume: () => new Float32Array(64).fill(1),
    };
    const state = { currentScenarioId: 'test', fieldParticleBuf: [], forceStyle: 'flow', scalarRenderMode: 'surface', fieldFlags:
        Object.fromEntries([...SCALAR_JOBS.map(([flag]) => flag), 'showEField', 'showBField',
            'showFluxLines', 'showPoynting', 'showDivField', 'showForceEM', 'showForceGravity',
            'showForceStrong', 'showForceWeak', 'showDualSubstrate', 'showChirality',
            'showDarkMatterHalo', 'showDampingZones', 'showGenesisIsosurface'].map(flag => [flag, true])) };
    const ctx = { bridge: { latticeSize: 4, capabilities: { scale0: capability } } };
    const applied = [];
    const adapter = new Proxy({}, { get: (_, name) => (...values) => applied.push({ name, values }) });
    const sched = ensureOverlaySched(state);
    sched.sampleCache = createFieldSampleCache(capability, capability, 1);
    sched.sampled = sched.sampleCache.sampled;
    sched.forceCache = createForceFieldCache(capability);
    buildOverlayJobs(ctx, state, sched, adapter, 4, {
        stride: 1, maxSeeds: 4, maxLines: 4, maxSteps: 5, stepSize: .5, eOffset: .5, bRadius: .5,
    });
    let attempts = 0;
    for (let i = 0; i < sched.jobCount;) {
        assert.ok(attempts++ < 100, 'a job failed to finish');
        if (runJob(sched, sched.jobs[i])) i++;
    }
    for (const name of ['applyEFieldLines', 'applyBFieldLines', 'applyFluxStreamlines',
        'applyPoynting', 'applyDivergence', 'applyForceStreamlines', 'applyDualFlux',
        'applyChirality', 'applyGenesisIsosurface', 'applyPhase', 'applyEmEnergy', 'applyDBPhase']) {
        assert.ok(applied.some(call => call.name === name), `${name} was never applied`);
    }
    assert.ok([...reads.values()].every(count => count === 1), 'shared inputs must be sampled once per sweep');
    disposeFieldOverlayRuntime(state);
    assert.equal(sched.sampleCache, null);
});

test('overlay disposal retires pending worker transactions before a late response', () => {
    const previousWorker = globalThis.Worker;
    let worker;
    globalThis.Worker = class {
        constructor() { worker = this; this.terminated = 0; }
        postMessage(message) { this.message = message; }
        terminate() { this.terminated++; }
    };
    try {
        const state = {};
        const sched = ensureOverlaySched(state);
        const slot = { phase: 1, workerResult: null, requestId: 0 };
        sched.jobs.push(slot); sched.jobCount = 1;
        submitStreamlineJob(sched, slot, 'e', { count: 0, positions: [], vectors: [] }, [], {});
        const reply = { id: worker.message.id, lines: ['obsolete'] };
        disposeFieldOverlayRuntime(state);
        worker.onmessage({ data: reply });
        assert.equal(worker.terminated, 1);
        assert.equal(sched.streamlineWorkerClient, null);
        assert.equal(slot.workerResult, null);
        assert.equal(slot.phase, 0);
    } finally { globalThis.Worker = previousWorker; }
});
