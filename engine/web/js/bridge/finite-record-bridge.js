/** Compiled finite-state backend behind the existing Scale-0 bridge contract. */
import { createScale0Capabilities } from './capabilities/scale0.js';
import { recordAt, recordFrame } from './finite-record-frame.js';

export const FINITE_RECORD_LAW = 'phi-v2-staged-candidate-1';
const empty = () => ({positions: new Float32Array(), vectors: new Float32Array(), values: new Float32Array(), count: 0});
const digest = async bytes => Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256', bytes)), b => b.toString(16).padStart(2, '0')).join('');

export async function loadRecordCatalog() {
    const response = await fetch('/api/lattice/records/catalog', {cache: 'no-store'});
    if (!response.ok) throw new Error('Local finite-state artifacts unavailable');
    const catalog = await response.json();
    if (catalog.law_id !== FINITE_RECORD_LAW || catalog.canonical_adoption !== false) throw new Error('Unsupported finite-state law');
    return catalog;
}

export class FiniteRecordBridge {
    constructor(latticeSize, {onFrame = () => {}, onFailure = () => {}} = {}) {
        this.latticeSize = latticeSize; this.isWorker = true; this.isFiniteRecord = true;
        this.ready = false; this.hasEngineToggles = true; this.disposed = false;
        this.running = false; this.failed = false; this.speed = 1; this.accumulator = 0;
        this.busy = 0; this.dataVersion = 0; this.requestId = 0; this.generation = '0'; this.ownerId = null;
        this.pending = new Map(); this.queue = Promise.resolve(); this.quantity = 'tokens';
        this.onFrame = onFrame; this.onFailure = onFailure;
        this.capabilities = {scale0: {...createScale0Capabilities(this), finiteRecords: true}};
    }
    get runningStateSettled() { return !this.busy; }
    get frameCounter() { return this.dataVersion; }
    get lifecycleDebug() { return {workerRuntimeId: this.ownerId, configurationToken: 1, appliedConfigurationToken: this.ready ? 1 : 0}; }
    currentTick() { return Number(this.observation?.microtick || 0); }
    getToggle() { return false; }
    setTelemetryMask() {}
    setRunning(value) { this.running = !!value && !this.failed; }
    setTicksPerFrame(value) { if (Number.isFinite(value) && value > 0) this.speed = value; }
    pump() {
        if (!this.running || !this.ready || this.busy || this.failed) return;
        this.accumulator += Math.min(this.speed, 64);
        const n = Math.floor(this.accumulator);
        if (n) { this.accumulator -= n; void this.advance(n); }
    }
    tick() { return this.advance(1); }
    tickOnce() { return this.advance(1); }
    request(op, fields = {}) {
        const requestId = String(++this.requestId);
        const expected = op === 'init' ? '0' : String(BigInt(this.generation) + (op === 'advance' ? 1n : 0n));
        return new Promise((resolve, reject) => {
            const timer = setTimeout(() => { this.pending.delete(requestId); reject(new Error('Finite-state worker timed out')); }, 30000);
            this.pending.set(requestId, {resolve, reject, expected, timer});
            this.worker.postMessage({requestId, op, ...fields, ...(op === 'init' ? {} : {ownerId: this.ownerId, generation: this.generation})});
        });
    }
    enqueue(action) {
        this.busy++;
        const task = this.queue.then(async () => { if (!this.disposed && !this.failed) return action(); });
        this.queue = task.catch(error => this.fail(error)).finally(() => this.busy--);
        return this.queue;
    }
    setupScenario(spec, catalog) {
        return this.enqueue(async () => {
            this.scenario = spec; this.abort = new AbortController();
            const query = new URLSearchParams({scenario: spec.preparationId, size: String(this.latticeSize)});
            const response = await fetch(`/api/lattice/records/checkpoint?${query}`, {signal: this.abort.signal, cache: 'no-store'});
            if (!response.ok || response.headers.get('X-Phi-Law') !== FINITE_RECORD_LAW) throw new Error('Preparation/law rejected');
            const checkpoint = new Uint8Array(await response.arrayBuffer());
            const sha = await digest(checkpoint);
            if (sha !== response.headers.get('X-Checkpoint-SHA256')) throw new Error('Checkpoint digest mismatch');
            if (this.disposed) return;
            this.checkpointSHA256 = sha;
            this.worker = new Worker(new URL('../strict/strict-worker.js', import.meta.url), {type: 'module'});
            this.worker.onmessage = ({data}) => {
                if (this.disposed) return;
                const item = this.pending.get(data.requestId); if (!item) return;
                this.pending.delete(data.requestId); clearTimeout(item.timer);
                if (!data.ok) return item.reject(new Error(data.error));
                if (typeof data.ownerId !== 'string' || !data.ownerId || (this.ownerId && data.ownerId !== this.ownerId) || data.generation !== item.expected)
                    return item.reject(new Error('Finite-state response lineage mismatch'));
                this.ownerId = data.ownerId; this.generation = data.generation; item.resolve(data.payload);
            };
            this.worker.onerror = event => this.fail(new Error(event.message || 'Finite-state worker failed'));
            await this.request('init', {wasmModuleUrl: new URL(catalog.wasm_module_url, location.href).href, checkpoint});
            await this.observe();
            if (!this.disposed) this.ready = true;
        });
    }
    advance(count = 1) {
        if (!Number.isSafeInteger(count) || count < 1 || count > 4096) return Promise.reject(new RangeError('Invalid finite-state batch'));
        return this.enqueue(async () => {
            if (!this.ready) return;
            for (let remaining = count; remaining > 0; remaining -= 64)
                await this.request('advance', {microticks: String(Math.min(remaining, 64))});
            await this.observe();
        });
    }
    async observe() {
        const view = await this.request('observe', {width: '1', observable: 'counts'});
        if (view.law_id !== FINITE_RECORD_LAW || Number(view.lattice_size) !== this.latticeSize) throw new Error('Observation law/size mismatch');
        this.observation = view; this.frame = recordFrame(view, this.quantity); this.dataVersion++; this.onFrame(this);
    }
    setRecordQuantity(quantity) {
        if (!this.observation) return;
        this.frame = recordFrame(this.observation, quantity); this.quantity = quantity;
        this.dataVersion++; this.onFrame(this);
    }
    getParticleData() { return this.frame?.particles || {positions: new Float32Array(), colors: new Float32Array(), sizes: new Float32Array(), count: 0}; }
    getFluxVolume() { return this.frame?.volume || new Float32Array(); }
    getFluxSlice() { return new Float32Array(); } // No vector-J slice is inferred from counts.
    getSamplerOr(kind) { return kind === 'state' ? this.frame?.state || empty() : empty(); }
    hasSamplerSnapshot(kind) { return kind === 'state' && !!this.observation; }
    getSamplerSnapshotVersion() { return this.dataVersion; }
    getRecordAt(x, y, z) { return this.observation ? recordAt(this.observation, x, y, z) : null; }
    inspectVoxel(x, y, z) { return this.getRecordAt(x, y, z); }
    getDiagnostics() {
        if (!this.observation) return null;
        const sum = rows => rows.reduce((n, v) => n + Number(v), 0);
        return {tick: this.currentTick(), manifested: this.getParticleData().count,
            fieldTokens: sum(this.observation.field_tokens), relationTokens: sum(this.observation.relation_tokens),
            incidence: sum(this.observation.incidence), phase: Number(this.observation.phase),
            lawId: FINITE_RECORD_LAW, finiteRecords: true};
    }
    getEnergyAudit() { return null; }
    getLagrangian() { return null; }
    getEMForceField() { return empty(); }
    getGravityForceField() { return empty(); }
    getStrongForceField() { return empty(); }
    getProvenance() {
        return {law_id: FINITE_RECORD_LAW, backend: 'compiled_wasm_worker', scenario: this.scenario?.id,
            ownerId: this.ownerId, generation: this.generation, microtick: this.observation?.microtick,
            checkpoint_sha256: this.checkpointSHA256, quantity: this.quantity,
            canonical_adoption: false, transport_recovery: false};
    }
    getDynamicalStateDigest() {
        let hash = null;
        return this.enqueue(async () => { hash = await digest(await this.request('checkpoint')); }).then(() => hash);
    }
    fail(error) {
        if (this.disposed || this.failed) return;
        this.failed = true; this.ready = false; this.running = false;
        this.onFailure(String(error.message || error)); this.terminate();
    }
    terminate() {
        this.worker?.terminate(); this.worker = null;
        for (const item of this.pending.values()) { clearTimeout(item.timer); item.reject(new Error('Finite-state owner disposed')); }
        this.pending.clear();
    }
    dispose() { if (!this.disposed) { this.disposed = true; this.running = false; this.abort?.abort(); this.terminate(); } }
}
