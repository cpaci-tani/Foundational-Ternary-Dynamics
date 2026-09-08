/** Main-thread observation transport. No FFT, reconstruction, or topology work here. */
const PROVENANCE_KEYS = ['status', 'reason', 'nativeInstanceId', 'sourceEpoch', 'epoch', 'sampleTick',
    'tick', 'physicalTime', 'dt', 'latticeSize', 'generation', 'dataVersion', 'requestId'];

function sampleProvenance(sample) {
    const source = sample?.provenance || sample || {};
    const result = {};
    for (const key of PROVENANCE_KEYS) {
        const value = source[key];
        if (value === null || ['string', 'number', 'boolean', 'bigint'].includes(typeof value)) result[key] = value;
    }
    // Absence is retained, never filled from the current bridge clock.
    return Object.freeze(result);
}

function copySample(caps, kind, stride, vector = false) {
    const sample = caps.getScale0FieldSamples({ kind, stride });
    const provenance = sampleProvenance(sample);
    if (caps.hasScale0SamplerSnapshot?.(kind, stride) === false) return { sample: null, provenance };
    const count = sample?.count, values = vector ? sample?.vectors : sample?.values;
    const width = vector ? 3 : 1;
    if (!Number.isSafeInteger(count) || count < 0 || !values || values.length < count * width
        || !sample.positions || sample.positions.length < count * 3) return { sample: null, provenance };
    const copy = (array, size) => new Float64Array(
        typeof array.subarray === 'function' ? array.subarray(0, size) : array.slice(0, size));
    return { sample: { count, positions: copy(sample.positions, count * 3),
        [vector ? 'vectors' : 'values']: copy(values, count * width),
        effectiveStride: sample.effectiveStride, origin: sample.origin }, provenance };
}

export function captureSpectrumObservation(caps, { L, stride, M, mode, metricKinds = [], audit = null,
    auditTick = null, diagTick = null }) {
    const flux = copySample(caps, 'fluxVector', stride, true);
    const divergence = mode === 'deep' ? { sample: null, provenance: {} } : copySample(caps, 'divJ', stride);
    const metrics = mode === 'deep' ? [] : metricKinds.map(({ kind }) => ({ kind, ...copySample(caps, kind, stride) }));
    return { L, stride, M, mode, flux: flux.sample, divergence: divergence.sample,
        metrics: metrics.map(({ kind, sample }) => ({ kind, sample })),
        audit: audit ? structuredClone(audit) : null,
        provenance: { flux: flux.provenance, divergence: divergence.provenance,
            metrics: metrics.map(({ kind, provenance }) => ({ kind, provenance })), auditTick, diagTick } };
}

const key = value => JSON.stringify(value, (_, item) => typeof item === 'bigint' ? `${item}n` : item);
function freezeProvenance(value) {
    if (!value || typeof value !== 'object' || Object.isFrozen(value)) return value;
    for (const child of Object.values(value)) freezeProvenance(child);
    return Object.freeze(value);
}
function transfers(observation) {
    const buffers = new Set();
    for (const sample of [observation.flux, observation.divergence, ...observation.metrics.map(m => m.sample)]) {
        for (const array of [sample?.positions, sample?.vectors, sample?.values]) if (array) buffers.add(array.buffer);
    }
    return [...buffers];
}

/** One active transaction and one replaceable latest pending observation. */
export class SpectrumAnalysisClient {
    constructor({ workerFactory = () => new Worker(new URL('./spectrum-analysis-worker.js', import.meta.url),
        { type: 'module', name: 'ftd-spectrum-analysis' }), now = () => performance.now(),
        schedule = (callback, delay) => globalThis.setTimeout(callback, delay),
        unschedule = timer => globalThis.clearTimeout(timer) } = {}) {
        this.workerFactory = workerFactory; this.now = now; this.schedule = schedule; this.unschedule = unschedule;
        this.worker = null; this.active = null; this.pending = null; this.nextId = 0;
        this.timer = null; this.disposed = false; this.boundary = {};
    }

    submit(observation, { context, onResult, onError = () => {}, deadline = this.now() + 5000 } = {}) {
        if (this.disposed) return null;
        if (this.nextId === Number.MAX_SAFE_INTEGER) throw new RangeError('Spectrum request identity exhausted');
        const job = { id: ++this.nextId, observation, context: structuredClone(context), onResult, onError, deadline,
            provenanceKey: key(observation.provenance) };
        if (this.active) this.pending = job;
        else this.start(job);
        return job.id;
    }

    start(job) {
        if (this.disposed) return;
        if (job.deadline <= this.now()) { job.onError('analysis-timeout'); return; }
        this.active = job;
        try {
            if (!this.worker) {
                const worker = this.workerFactory();
                this.worker = worker;
                worker.onmessage = event => this.receive(worker, event.data);
                worker.onerror = event => {
                    event.preventDefault?.();
                    if (this.worker === worker) this.fail('analysis-worker-unavailable');
                };
                worker.onmessageerror = () => { if (this.worker === worker) this.fail('analysis-message-invalid'); };
            }
            this.timer = this.schedule(() => { if (this.active === job) this.fail('analysis-timeout'); },
                Math.max(0, job.deadline - this.now()));
            this.worker.postMessage({ id: job.id, context: job.context, observation: job.observation }, transfers(job.observation));
        } catch { this.fail('analysis-worker-unavailable'); }
    }

    receive(worker, message) {
        const job = this.active;
        if (worker !== this.worker || !job || message?.id !== job.id) return;
        if (key(message.context) !== key(job.context)) { this.fail('analysis-context-mismatch'); return; }
        if (job.deadline <= this.now()) { this.fail('analysis-timeout'); return; }
        if (message.error) { this.fail('analysis-failed'); return; }
        if (!message.result || key(message.result.provenance) !== job.provenanceKey) {
            this.fail('analysis-provenance-mismatch'); return;
        }
        freezeProvenance(message.result.provenance);
        this.unschedule(this.timer); this.timer = null; this.active = null;
        const next = this.pending; this.pending = null;
        const boundary = this.boundary;
        job.onResult(message.result, { id: job.id, context: job.context });
        if (next && boundary === this.boundary && !this.disposed && !this.active) this.start(next);
    }

    fail(reason) {
        const job = this.active, next = this.pending;
        this.cancel();
        const boundary = this.boundary;
        job?.onError(reason);
        if (next && boundary === this.boundary && !this.disposed && !this.active) this.start(next);
    }

    cancel() {
        this.boundary = {};
        this.unschedule(this.timer); this.timer = null;
        const worker = this.worker; this.worker = null; this.active = null; this.pending = null;
        worker?.terminate();
    }

    dispose() { this.disposed = true; this.cancel(); }
    get busy() { return !!this.active; }
    get queued() { return !!this.pending; }
}
