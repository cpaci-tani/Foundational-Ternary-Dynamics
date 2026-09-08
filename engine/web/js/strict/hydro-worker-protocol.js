/** Isolated hydro-runtime transport (mirrors ./worker-protocol.js for the strict
 * candidate). All physics operations delegate to one compiled HydroState; identifiers/
 * generations are external publication metadata, never ontic state.
 *
 * Two contract differences from the strict protocol, both forced by the hydro
 * candidate's own bindings (engine/strict/hydro/wasm_bindings_hydro.cpp):
 *   1. `checkpoint`/`restore` carry a JSON string (the Python oracle's
 *      "ftd-hydro-checkpoint-1" schema), not a Uint8Array of the native binary
 *      transport -- so there is no SharedArrayBuffer aliasing risk to defend against
 *      (strings are immutable values) and no `copyCheckpoint` step is needed here.
 *   2. The bindings already stringify every exact (potentially big) integer in their
 *      JSON output and use raw JSON numbers only for the "moments" observable's
 *      genuinely-approximate double re/im parts. `exact()` below reflects that: it
 *      still turns bigints into decimal strings, but passes finite numbers through
 *      rather than demanding they be safe integers.
 */
const DECIMAL = /^(0|[1-9][0-9]*)$/;

function decimal(value, name) {
    if (typeof value !== 'string' || !DECIMAL.test(value)) {
        throw new TypeError(`${name} must be a canonical unsigned decimal string`);
    }
    return value;
}

function exact(value) {
    if (typeof value === 'bigint') return value.toString();
    if (typeof value === 'number') {
        if (!Number.isFinite(value)) throw new TypeError('adapter returned a non-finite number');
        return value;
    }
    if (value instanceof Uint8Array) return value.slice();
    if (Array.isArray(value)) return value.map(exact);
    if (value && typeof value === 'object') {
        return Object.fromEntries(Object.entries(value).map(([k, v]) => [k, exact(v)]));
    }
    if (value === null || typeof value === 'string' || typeof value === 'boolean') return value;
    throw new TypeError('unsupported adapter observation value');
}

/** adapter: advance(decimal), checkpoint(), restore(jsonString),
 * observe(decimalWidth, name), diagnostics(), capabilities(), dispose().
 * Adapter mutations must be atomic on failure. One adapter belongs exclusively
 * to this factory. Queued commands check generation when executed, not queued.
 */
export function createHydroWorkerProtocol(adapter, {
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
    const alive = () => { if (disposed) throw new Error('hydro runtime disposed'); };
    const capabilities = async () => ({ ...await adapter.capabilities(), maxBatchMicroticks });

    function dispatch(request) {
        let message;
        try {
            message = structuredClone(request);
            if (!message || typeof message !== 'object' || typeof message.requestId !== 'string'
                || !message.requestId || typeof message.op !== 'string') throw new TypeError('invalid request envelope');
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
                if (typeof payload !== 'string') throw new TypeError('checkpoint must be a JSON string');
                break;
            case 'restore':
                if (typeof message.checkpoint !== 'string') throw new TypeError('checkpoint must be a JSON string');
                payload = await adapter.restore(message.checkpoint);
                generation += 1n;
                break;
            case 'observe':
                payload = await adapter.observe(decimal(message.width, 'width'), message.observable);
                break;
            case 'diagnostics': payload = await adapter.diagnostics(); break;
            case 'capabilities': payload = await capabilities(); break;
            default: throw new Error(`unsupported hydro operation: ${message.op}`);
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
export function createWasmHydroAdapter(module, checkpointJson) {
    if (typeof checkpointJson !== 'string') throw new TypeError('checkpoint must be a JSON string');
    const state = new module.HydroState(checkpointJson);
    let disposed = false;
    const alive = () => { if (disposed) throw new Error('compiled adapter disposed'); };
    return {
        advance(ticks) { alive(); return JSON.parse(state.advance(ticks)); },
        checkpoint() { alive(); return state.checkpoint(); },
        restore(json) { alive(); state.restore(json); return JSON.parse(state.diagnostics()); },
        observe(width, observable) { alive(); return JSON.parse(state.observe(width, observable)); },
        diagnostics() { alive(); return JSON.parse(state.diagnostics()); },
        capabilities() {
            alive();
            return { backend: 'hydro_wasm_candidate', observables: ['counts', 'fields', 'moments'],
                autonomousCoarseEvolution: false, continuumRecovery: false, physicalUnits: false,
                particleIdentification: false, gravityRecovery: false, canonicalAdoption: false };
        },
        dispose() { if (!disposed) { disposed = true; state.delete(); } },
    };
}

/** Fetches the 64 MiB collision-table blob and installs it into an already-loaded
 * module (module.loadTable verifies its SHA-256 and throws on mismatch). This is not
 * itself a protocol dispatch op: module-level table installation must complete before
 * any HydroState -- and therefore any adapter/protocol -- can be constructed, so it
 * runs once during worker 'init', ahead of createWasmHydroAdapter/createHydroWorkerProtocol
 * (see ./hydro-worker.js). Exported here so the fetch->Uint8Array->loadTable sequence
 * lives beside the rest of the hydro transport rather than duplicated in the worker file.
 */
export async function loadHydroTable(module, tableUrl, { fetchImpl = fetch } = {}) {
    if (typeof tableUrl !== 'string' || !tableUrl) throw new TypeError('tableUrl is required');
    const response = await fetchImpl(tableUrl);
    if (!response || !response.ok) throw new Error(`hydro table fetch failed: ${response && response.status}`);
    const bytes = new Uint8Array(await response.arrayBuffer());
    module.loadTable(bytes);
}

/** Optional worker attachment; application integration is deliberately separate. */
export function attachHydroWorker(scope, adapter, options) {
    const protocol = createHydroWorkerProtocol(adapter, options);
    const receive = async ({ data }) => {
        try { scope.postMessage({ ok: true, ...await protocol.dispatch(data) }); }
        catch (error) { scope.postMessage({ ok: false, requestId: data?.requestId, error: String(error.message) }); }
    };
    scope.addEventListener('message', receive);
    return Object.freeze({ protocol, detach: () => scope.removeEventListener('message', receive) });
}
