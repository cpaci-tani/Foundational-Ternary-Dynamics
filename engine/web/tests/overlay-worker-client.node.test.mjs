import test from 'node:test';
import assert from 'node:assert/strict';
import {
    submitStreamlineJob,
    cancelStreamlineJobs,
    commitKnotRecord,
    discardKnotRecord,
} from '../js/scales/scale0/runtime/overlay-worker-client.js';

const emptyField = { count: 0, positions: new Float32Array(), vectors: new Float32Array() };

function withWorkerFixture(run) {
    const previousWorker = globalThis.Worker;
    const previousError = console.error;
    const workers = [], failures = [], errors = [];
    globalThis.Worker = class {
        constructor() { this.messages = []; this.terminations = 0; workers.push(this); }
        postMessage(message) { this.messages.push(message); }
        terminate() { this.terminations++; }
    };
    console.error = (...args) => errors.push(args);
    const slot = { phase: 1, workerResult: null, requestId: 0, workerClient: null };
    const sched = { jobs: [slot], jobCount: 1,
        onStreamlineWorkerFailure: message => failures.push(message) };
    const submit = () => submitStreamlineJob(sched, slot, 'e', emptyField, [], {});
    try { run({ workers, failures, errors, sched, slot, submit }); }
    finally { globalThis.Worker = previousWorker; console.error = previousError; }
}

test('ordinary sweep cancellation preserves the worker but rejects late observations', () => {
    withWorkerFixture(({ workers, sched, slot, submit }) => {
        submit();
        const worker = workers[0], client = sched.streamlineWorkerClient;
        assert.equal(slot.workerClient, client, 'adoption must carry the originating client');
        const cancelledId = worker.messages[0].id;
        cancelStreamlineJobs(sched);
        assert.equal(worker.terminations, 0);
        assert.equal(sched.streamlineWorkerClient, client);
        assert.equal(client.pending.size, 0);
        assert.equal(slot.workerClient, null);
        assert.equal(worker.messages.at(-1).action, 'discard-all-knots');
        worker.onmessage({ data: { id: cancelledId, lines: ['obsolete'] } });
        assert.equal(slot.workerResult, null);
        assert.equal(slot.phase, 0);

        submit();
        const nextId = worker.messages.at(-1).id;
        assert.equal(slot.workerClient, client);
        assert.notEqual(nextId, cancelledId);
        worker.onmessage({ data: { id: cancelledId, lines: ['obsolete'] } });
        assert.equal(slot.workerResult, null);
        const current = { id: nextId, lines: ['current'] };
        worker.onmessage({ data: current });
        assert.equal(slot.workerResult, current);
    });
});

test('ownership cancellation terminates the old worker and isolates reused request IDs', () => {
    withWorkerFixture(({ workers, sched, slot, submit }) => {
        submit();
        const oldWorker = workers[0], oldId = oldWorker.messages[0].id;
        cancelStreamlineJobs(sched, { terminate: true });
        assert.equal(oldWorker.terminations, 1);
        assert.equal(sched.streamlineWorkerClient, null);
        submit();
        const newWorker = workers[1], newId = newWorker.messages[0].id;
        assert.equal(slot.workerClient, sched.streamlineWorkerClient);
        assert.equal(newId, oldId, 'fixture exercises ID reuse across worker lifetimes');
        oldWorker.onmessage({ data: { id: oldId, lines: ['wrong owner'] } });
        assert.equal(slot.workerResult, null);
        newWorker.onmessage({ data: { id: newId, lines: ['new owner'] } });
        assert.deepEqual(slot.workerResult.lines, ['new owner']);
    });
});

test('candidate commit and discard target only the worker that produced the observation', () => {
    withWorkerFixture(({ workers, sched, slot, submit }) => {
        submit();
        const original = slot.workerClient, candidateId = slot.requestId;
        commitKnotRecord(sched, candidateId, original);
        discardKnotRecord(sched, candidateId, original);
        assert.deepEqual(workers[0].messages.slice(-2), [
            { action: 'commit-knot', candidateId },
            { action: 'discard-knot', candidateId },
        ]);
        cancelStreamlineJobs(sched, { terminate: true });
        submit();
        assert.equal(slot.requestId, candidateId, 'request IDs repeat in the replacement worker');
        const previousCount = workers[1].messages.length;
        commitKnotRecord(sched, candidateId, original);
        discardKnotRecord(sched, candidateId, original);
        assert.equal(workers[1].messages.length, previousCount,
            'an obsolete observation must never mutate replacement genealogy');
        commitKnotRecord(sched, slot.requestId, slot.workerClient);
        assert.deepEqual(workers[1].messages.at(-1), { action: 'commit-knot', candidateId });
    });
});

for (const eventName of ['onerror', 'onmessageerror']) {
    test(`${eventName} retires the worker, invalidates observations once, and cannot poison a replacement`, () => {
        withWorkerFixture(({ workers, failures, errors, sched, slot, submit }) => {
            submit();
            const failedWorker = workers[0], failedClient = sched.streamlineWorkerClient;
            const oldId = failedWorker.messages[0].id;
            failedWorker[eventName]({ message: 'decode or execution failure' });
            assert.equal(failedWorker.terminations, 1);
            assert.equal(failedClient.failed, true);
            assert.equal(failedClient.pending.size, 0);
            assert.equal(sched.streamlineWorkerClient, null);
            assert.equal(failures.length, 1, 'published mirrors must be invalidated');
            assert.equal(errors.length, 1);
            assert.equal(typeof slot.workerResult.error, 'string');

            submit();
            const replacement = sched.streamlineWorkerClient;
            failedWorker[eventName]({ message: 'late duplicate failure' });
            failedWorker.onmessage({ data: { id: oldId, lines: ['obsolete'] } });
            assert.equal(sched.streamlineWorkerClient, replacement);
            assert.equal(slot.workerResult, null);
            assert.equal(failures.length, 1, 'old failure cannot invalidate the new owner');
            assert.equal(failedWorker.terminations, 1);
        });
    });
}
