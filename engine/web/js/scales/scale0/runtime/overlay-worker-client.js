/** The scheduler owns one worker client; cancellation invalidates every pending slot. */
import { computeStreamlines } from '../../../fieldlines.js';
const STREAMLINE_WORKER_URL = new URL('./streamline-worker.js?v=3', import.meta.url);

export function cancelStreamlineJobs(sched, { terminate = false } = {}) {
    const client = sched.streamlineWorkerClient;
    if (client) {
        // Candidate tracker state is committed only after the client adopts a
        // matching response. Discard every uncommitted candidate on a sweep
        // boundary without destroying the worker's committed field namespace.
        client.worker.postMessage({ action: 'discard-all-knots' });
        if (terminate) client.worker.terminate();
        client.pending.clear();
        if (terminate) sched.streamlineWorkerClient = null;
    }
    for (let i = 0; i < sched.jobCount; i++) {
        const slot = sched.jobs[i];
        if (slot) {
            slot.phase = 0;
            slot.workerResult = null;
            slot.requestId = 0;
            slot.workerClient = null;
        }
    }
}

function ensureStreamlineWorker(sched) {
    if (sched.streamlineWorkerClient) return sched.streamlineWorkerClient;
    if (typeof Worker === 'undefined') return null;
    const worker = new Worker(STREAMLINE_WORKER_URL, { type: 'module' });
    const client = { worker, nextId: 1, pending: new Map(), failed: false };
    worker.onmessage = ({ data }) => {
        const slot = client.pending.get(data?.id);
        if (!slot) return;
        client.pending.delete(data.id);
        if (slot.requestId !== data.id) return;
        slot.workerResult = data;
    };
    // Both error events mean the worker's protocol can no longer be trusted.
    // Keep this client-local: a late event from a terminated old worker must
    // never tear down a replacement client that already owns the scheduler.
    const failClient = (message) => {
        if (client.fatalHandled) return;
        client.fatalHandled = true;
        for (const slot of client.pending.values()) slot.workerResult = { error: message };
        client.pending.clear();
        client.failed = true;
        worker.terminate();
        if (sched.streamlineWorkerClient !== client) return;
        sched.streamlineWorkerClient = null;
        sched.onStreamlineWorkerFailure?.(message);
        console.error('[Scale0] streamline worker failed:', message);
    };
    worker.onerror = (event) => failClient(event.message || 'unknown worker error');
    worker.onmessageerror = () => failClient('worker response could not be decoded');
    sched.streamlineWorkerClient = client;
    return client;
}

export function submitStreamlineJob(sched, slot, kind, fieldData, seeds, opts, knot = null) {
    const client = ensureStreamlineWorker(sched);
    if (!client) {
        // Emergency non-Worker fallback. Browser production paths always use
        // the module worker; this keeps embedded/test environments functional.
        slot.workerResult = { kind, lines: computeStreamlines(fieldData, seeds, opts), maxFlux: 0, mags: null, localFallback: true };
        return;
    }
    const id = client.nextId++;
    slot.requestId = id;
    slot.workerResult = null;
    // The result must be committed through this exact client. A newer worker
    // can reuse request IDs after an ownership boundary, so scheduler-global
    // identity is insufficient for the commit/discard transaction.
    slot.workerClient = client;
    client.pending.set(id, slot);
    client.worker.postMessage({
        id,
        kind,
        fieldData: {
            positions: fieldData.positions,
            vectors: fieldData.vectors,
            count: fieldData.count,
        },
        seeds,
        opts,
        knot,
    });
}

export function commitKnotRecord(sched, candidateId, client = sched.streamlineWorkerClient) {
    if (client && client === sched.streamlineWorkerClient && !client.failed) {
        client.worker.postMessage({ action: 'commit-knot', candidateId });
    }
}

export function discardKnotRecord(sched, candidateId, client = sched.streamlineWorkerClient) {
    if (client && client === sched.streamlineWorkerClient && !client.failed) {
        client.worker.postMessage({ action: 'discard-knot', candidateId });
    }
}
