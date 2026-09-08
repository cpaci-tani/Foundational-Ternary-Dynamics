/** Isolated strict-runtime transport. No JS physics or effective-engine fallback.
 * The adapter owns one compiled State; all physics operations delegate to it.
 * Identifiers/generations are external publication metadata, never ontic state.
 */
const DECIMAL = /^(0|[1-9][0-9]*)$/;
const ownedBufferLength = Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength').get;
const typedArrayBuffer = Object.getOwnPropertyDescriptor(Object.getPrototypeOf(Uint8Array.prototype), 'buffer').get;

export function copyCheckpoint(checkpoint) {
    if (!(checkpoint instanceof Uint8Array)) throw new TypeError('checkpoint must be Uint8Array');
    try { ownedBufferLength.call(typedArrayBuffer.call(checkpoint)); }
    catch {
        throw new TypeError('SharedArrayBuffer checkpoints are unsupported; provide an owned snapshot');
    }
    return checkpoint.slice();
}

function decimal(value, name) {
    if (typeof value !== 'string' || !DECIMAL.test(value)) {
        throw new TypeError(`${name} must be a canonical unsigned decimal string`);
    }
    return value;
}

function exact(value) {
    if (typeof value === 'bigint') return value.toString();
    if (typeof value === 'number') {
        if (!Number.isSafeInteger(value)) throw new TypeError('adapter returned an inexact number');
        return String(value);
    }
    if (value instanceof Uint8Array) return value.slice();
    if (Array.isArray(value)) return value.map(exact);
    if (value && typeof value === 'object') {
        return Object.fromEntries(Object.entries(value).map(([k, v]) => [k, exact(v)]));
    }
    if (value === null || typeof value === 'string' || typeof value === 'boolean') return value;
    throw new TypeError('unsupported adapter observation value');
}

/** adapter: advance(decimal), checkpoint(), restore(Uint8Array),
 * observe(decimalWidth, name), diagnostics(), capabilities(), dispose().
 * Adapter mutations must be atomic on failure. One adapter belongs exclusively
 * to this factory. Queued commands check generation when executed, not queued.
 */
export function createStrictWorkerProtocol(adapter, {
    ownerId = globalThis.crypto.randomUUID(), maxBatchMicroticks = '64',
} = {}) {
    if (typeof ownerId !== 'string' || !ownerId) throw new TypeError('ownerId is required');
    decimal(maxBatchMicroticks, 'maxBatchMicroticks');
    const batchLimit = BigInt(maxBatchMicroticks);
    if (batchLimit === 0n || batchLimit > 64n) throw new RangeError('maxBatchMicroticks must be between 1 and 64');
    for (const method of ['advance', 'checkpoint', 'restore', 'observe', 'diagnostics', 'capabilities', 'dispose']) {
        if (typeof adapter?.[method] !== 'function') throw new TypeError(`adapter lacks ${method}`);
    }
    let generation = 0n;
    let disposed = false;
    let queue = Promise.resolve();
    const envelope = (requestId, payload) => ({
        requestId, ownerId, generation: generation.toString(),
        integerEncoding: 'decimal_string', payload: exact(payload),
    });
    const alive = () => { if (disposed) throw new Error('strict runtime disposed'); };
    const capabilities = async () => ({ ...await adapter.capabilities(), maxBatchMicroticks });

    function dispatch(request) {
        let message;
        try {
            message = structuredClone(request);
            if (!message || typeof message !== 'object' || typeof message.requestId !== 'string'
                || !message.requestId || typeof message.op !== 'string') throw new TypeError('invalid request envelope');
            if (message.op === 'restore') message.checkpoint = copyCheckpoint(message.checkpoint);
            alive();
            if (message.op !== 'hello') {
                if (message.ownerId !== ownerId) throw new Error('stale or foreign owner');
                decimal(message.generation, 'generation');
            }
        } catch (error) { return Promise.reject(error); }

        if (message.op === 'dispose') {
            // Immediately suppress pending publication, but destroy native memory
            // only after any already-running operation has completed.
            if (message.generation !== generation.toString()) return Promise.reject(new Error('stale generation'));
            disposed = true;
            const closing = queue.then(() => adapter.dispose()).then(() => envelope(message.requestId, { disposed: true }));
            queue = closing.catch(() => {});
            return closing;
        }

        const result = queue.then(async () => {
            alive();
            if (message.op !== 'hello' && message.generation !== generation.toString()) throw new Error('stale generation');
            let payload;
            switch (message.op) {
            case 'hello':
                payload = { capabilities: await capabilities(), diagnostics: await adapter.diagnostics() };
                break;
            case 'advance': {
                const ticks = decimal(message.microticks, 'microticks');
                // Each worker task is bounded so later messages can be received.
                if (BigInt(ticks) > batchLimit) throw new RangeError('advance exceeds maxBatchMicroticks');
                payload = await adapter.advance(ticks);
                if (BigInt(ticks) !== 0n) generation += 1n;
                break;
            }
            case 'checkpoint':
                payload = await adapter.checkpoint();
                if (!(payload instanceof Uint8Array)) throw new TypeError('checkpoint must be Uint8Array');
                break;
            case 'restore':
                if (!(message.checkpoint instanceof Uint8Array)) throw new TypeError('checkpoint must be Uint8Array');
                payload = await adapter.restore(message.checkpoint.slice());
                generation += 1n;
                break;
            case 'observe':
                payload = await adapter.observe(decimal(message.width, 'width'), message.observable);
                break;
            case 'diagnostics': payload = await adapter.diagnostics(); break;
            case 'capabilities': payload = await capabilities(); break;
            default: throw new Error(`unsupported strict operation: ${message.op}`);
            }
            alive();
            return envelope(message.requestId, payload);
        });
        queue = result.catch(() => {});
        return result;
    }
    return Object.freeze({ dispatch });
}

/** Bind the same compiled class used in native parity; no replacement engine. */
export function createWasmStrictAdapter(module, checkpoint) {
    const state = new module.StrictState(copyCheckpoint(checkpoint));
    let disposed = false;
    const alive = () => { if (disposed) throw new Error('compiled adapter disposed'); };
    return {
        advance(ticks) { alive(); return JSON.parse(state.advance(ticks)); },
        checkpoint() { alive(); return state.checkpoint().slice(); },
        restore(bytes) { alive(); state.restore(copyCheckpoint(bytes)); return JSON.parse(state.diagnostics()); },
        observe(width, observable) { alive(); return JSON.parse(state.observe(width, observable)); },
        diagnostics() { alive(); return JSON.parse(state.diagnostics()); },
        capabilities() {
            alive();
            return { backend: 'strict_wasm_candidate', observables: ['counts'],
                autonomousCoarseEvolution: false, continuumRecovery: false, physicalUnits: false,
                particleIdentification: false, gravityRecovery: false, canonicalAdoption: false };
        },
        dispose() { if (!disposed) { disposed = true; state.delete(); } },
    };
}

/** Optional worker attachment; application integration is deliberately separate. */
export function attachStrictWorker(scope, adapter, options) {
    const protocol = createStrictWorkerProtocol(adapter, options);
    const receive = async ({ data }) => {
        try { scope.postMessage({ ok: true, ...await protocol.dispatch(data) }); }
        catch (error) { scope.postMessage({ ok: false, requestId: data?.requestId, error: String(error.message) }); }
    };
    scope.addEventListener('message', receive);
    return Object.freeze({ protocol, detach: () => scope.removeEventListener('message', receive) });
}
