import test from 'node:test';
import assert from 'node:assert/strict';
import { Scale1ParticleWorkerExecutor } from '../js/scales/scale1/particle-worker-executor.js';

class FakeWorker {
    postMessage(message) { this.lastMessage = message; }
    terminate() { this.terminated = true; }
    emit(data) { this.onmessage?.({ data }); }
}

function fixture() {
    const worker = new FakeWorker();
    let revision = 4;
    let restored = null;
    const bridge = {
        peExportCheckpoint: () => ({
            schema: 'ftd.scale1.dashboard-checkpoint', schemaVersion: 1,
            native: { tick: 2 }, catalogTypes: [],
        }),
        peGetObservationRevision: () => revision,
        peRestoreCheckpoint: checkpoint => { restored = checkpoint; revision++; return true; },
        mutate: () => revision++,
        restored: () => restored,
    };
    const executor = new Scale1ParticleWorkerExecutor({ workerFactory: () => worker });
    executor.ensure();
    worker.emit({ type: 'ready' });
    return { worker, bridge, executor };
}

test('worker result restores only the matching source revision', () => {
    const { worker, bridge, executor } = fixture();
    assert.equal(executor.request(bridge, 3), true);
    const request = worker.lastMessage;
    worker.emit({
        type: 'result', requestId: request.requestId,
        generation: request.generation, sourceRevision: request.sourceRevision,
        checkpoint: { ...request.checkpoint, native: { tick: 5 } },
    });
    assert.equal(bridge.restored().native.tick, 5);
});

test('main-thread mutation rejects an in-flight stale result', () => {
    const { worker, bridge, executor } = fixture();
    executor.request(bridge, 3);
    const request = worker.lastMessage;
    bridge.mutate();
    worker.emit({
        type: 'result', requestId: request.requestId,
        generation: request.generation,
        checkpoint: { ...request.checkpoint, native: { tick: 5 } },
    });
    assert.equal(bridge.restored(), null);
});

test('busy executor drops catch-up work instead of queueing it', () => {
    const { worker, bridge, executor } = fixture();
    executor.request(bridge, 3);
    const first = worker.lastMessage;
    assert.equal(executor.request(bridge, 99), true);
    assert.equal(worker.lastMessage, first);
});
