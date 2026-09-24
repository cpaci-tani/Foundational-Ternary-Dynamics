import test from 'node:test';
import assert from 'node:assert/strict';
import { FieldLineKnotTracker } from '../js/scales/scale0/runtime/field-line-knots.js';
import {
    prepareFieldLineKnotCandidate,
    commitFieldLineKnotCandidate,
    discardFieldLineKnotCandidate,
    resetFieldLineKnotWorkerState,
    snapshotFieldLineKnotTracker,
} from '../js/scales/scale0/runtime/field-line-knots-worker-state.js';
import { executeStreamlineWorkerMessage } from '../js/scales/scale0/runtime/streamline-worker.js';
import { submitStreamlineJob } from '../js/scales/scale0/runtime/overlay-worker-client.js';

const lines = Object.freeze({
    count: 2,
    buffer: new Float32Array([
        1, 1, 1, 2, 1, 1, 3, 1, 1,
        1, 1, 1, 1, 2, 1, 1, 3, 1,
    ]),
    offsets: new Int32Array([0, 9]),
    lengths: new Int32Array([9, 9]),
});
const laterLines = Object.freeze({
    count: 2,
    buffer: new Float32Array([
        1, 1, 1, 2, 1, 1, 3, 1, 1,
        1, 1, 1, 1, 2, 1, 1, 3, 1,
    ]),
    offsets: new Int32Array([0, 9]),
    lengths: new Int32Array([9, 9]),
});
const sample = Object.freeze({
    count: 3,
    positions: new Float32Array([1, 1, 1, 2, 1, 1, 1, 2, 1]),
    vectors: new Float32Array([1, 0, 0, 1, 0, 0, 0, 1, 0]),
});

function provenance(sampleTick, resetVersion = 1, mutationEpoch = 4) {
    return {
        field: 'e', loadGeneration: 7, mutationEpoch,
        source: 'wasm-worker', nativeInstanceId: 'instance-7', sourceEpoch: 11, epoch: 11,
        stateVersion: sampleTick, snapshotVersion: sampleTick, sampleTick,
        latticeSize: 9, resetVersion,
    };
}

function simplify(value) {
    if (ArrayBuffer.isView(value)) return Array.from(value);
    if (Array.isArray(value)) return value.map(simplify);
    if (value && typeof value === 'object') return Object.fromEntries(
        Object.entries(value).map(([key, child]) => [key, simplify(child)]),
    );
    return value;
}

function candidate(id, tick, resetVersion = 1, mutationEpoch = 4) {
    return prepareFieldLineKnotCandidate({
        candidateId: id, field: 'e', streamlines: lines, fieldSamples: sample,
        provenance: provenance(tick, resetVersion, mutationEpoch),
        sensitivity: 0.5, perKnotColor: true, includeLineIds: true,
    });
}

test('worker candidates match direct tracker telemetry, events, zones, and line IDs across commits', () => {
    resetFieldLineKnotWorkerState();
    const direct = new FieldLineKnotTracker();
    direct.record(lines, sample, 10, 9);
    const first = candidate(1, 10);
    assert.deepEqual(simplify(first.snapshot), simplify(snapshotFieldLineKnotTracker(direct, provenance(10))));
    assert.equal(first.snapshot.telemetry.sampleTick, 10, 'worker publishes the exact successful sample tick');
    assert.deepEqual(Array.from(first.lineIds), Array.from(direct.assignLinesToKnots(lines)));
    assert.equal(commitFieldLineKnotCandidate(1), true);

    direct.record(laterLines, sample, 11, 9);
    const second = candidate(2, 11);
    assert.deepEqual(simplify(second.snapshot), simplify(snapshotFieldLineKnotTracker(direct, provenance(11))));
    assert.deepEqual(Array.from(second.lineIds), Array.from(direct.assignLinesToKnots(laterLines)));
    assert.equal(commitFieldLineKnotCandidate(2), true);
});

test('discarded candidate cannot advance worker identity/event history', () => {
    resetFieldLineKnotWorkerState();
    const direct = new FieldLineKnotTracker();
    direct.record(lines, sample, 20, 9);
    assert.equal(commitFieldLineKnotCandidate(10), false, 'unknown candidate never mutates state');
    const initial = candidate(11, 20);
    assert.equal(commitFieldLineKnotCandidate(11), true);

    // This is the late/cancelled observation: do not apply it to the direct oracle.
    candidate(12, 21);
    assert.equal(discardFieldLineKnotCandidate(12), true);

    direct.record(lines, sample, 22, 9);
    const accepted = candidate(13, 22);
    assert.deepEqual(simplify(accepted.snapshot), simplify(snapshotFieldLineKnotTracker(direct, provenance(22))));
    assert.equal(commitFieldLineKnotCandidate(13), true);
    assert.ok(initial.snapshot.events.count >= 0);
});

test('a reset namespace starts a new exact tracker state without retaining prior IDs/events', () => {
    resetFieldLineKnotWorkerState();
    const direct = new FieldLineKnotTracker();
    direct.record(lines, sample, 30, 9);
    assert.equal(commitFieldLineKnotCandidate(20), false);
    const initial = candidate(21, 30);
    assert.equal(commitFieldLineKnotCandidate(21), true);

    direct.reset();
    direct.record(lines, sample, 31, 9);
    const reset = candidate(22, 31, 2, 5);
    assert.deepEqual(simplify(reset.snapshot), simplify(snapshotFieldLineKnotTracker(direct, provenance(31, 2, 5))));
    assert.equal(commitFieldLineKnotCandidate(22), true);
    assert.ok(initial.snapshot.events.count >= 0);
});

test('birth, age, and event ticks preserve safe integers above signed 32-bit range', () => {
    const tracker = new FieldLineKnotTracker({ densityThreshold: 1 });
    const tick = 2 ** 31 + 17;
    const telemetry = tracker.record(lines, sample, tick, 9);
    assert.ok(telemetry.count > 0, 'fixture produces at least one knot');
    assert.equal(telemetry.birth[0], tick);
    assert.ok(telemetry.age instanceof Float64Array);
    assert.ok(telemetry.birth instanceof Float64Array);
    const events = tracker.getEvents();
    assert.ok(events.tick instanceof Float64Array);
    assert.ok(Array.from(events.tick).every(Number.isSafeInteger));
});

test('the actual worker request envelope derives its tracker field from provenance', () => {
    resetFieldLineKnotWorkerState();
    const previousWorker = globalThis.Worker;
    const workers = [];
    globalThis.Worker = class {
        constructor() { this.messages = []; workers.push(this); }
        postMessage(message) { this.messages.push(message); }
        terminate() {}
    };
    try {
        const slot = { workerResult: null, requestId: 0, workerClient: null };
        const sched = { streamlineWorkerClient: null };
        const knot = {
            provenance: provenance(91),
            sensitivity: 0.5,
            perKnotColor: true,
            includeLineIds: true,
        };
        // Producer -> posted request -> deployed worker processor: this guards
        // the complete transport boundary rather than calling the candidate
        // helper directly with independently constructed fields.
        submitStreamlineJob(sched, slot, 'e', sample, [[1, 1, 1]],
            { N: 9, stride: 1, maxSteps: 2, stepSize: 0.5, maxLines: 1, bidirectional: true }, knot);
        const request = workers[0].messages[0];
        assert.equal(request.knot, knot, 'client transports the provenance-owned field envelope intact');
        const response = executeStreamlineWorkerMessage(request);
        assert.equal(response.id, request.id);
        assert.equal(response.knot.provenance.field, 'e');
        assert.equal(response.knot.snapshot.telemetry.sampleTick, 91);
        assert.equal(commitFieldLineKnotCandidate(request.id), true,
            'a client-accepted response commits the worker namespace on this request id');
        assert.throws(() => executeStreamlineWorkerMessage({
            ...request,
            id: 92,
            knot: { ...request.knot, provenance: { ...request.knot.provenance, field: undefined } },
        }), /Invalid field-line knot provenance field/);
    } finally {
        globalThis.Worker = previousWorker;
    }
});
