/** Dedicated worker entry for the isolated compiled strict candidate. */
import { createStrictWorkerProtocol, createWasmStrictAdapter, copyCheckpoint } from './worker-protocol.js';

let protocol = null;
let initializing = false;

self.addEventListener('message', async ({ data }) => {
    try {
        if (data?.op === 'init') {
            if (initializing || protocol) throw new Error('strict worker already initialized or initializing');
            if (typeof data.requestId !== 'string' || !data.requestId
                || !(data.checkpoint instanceof Uint8Array)) throw new TypeError('invalid initialization');
            if (typeof data.wasmModuleUrl !== 'string') throw new TypeError('wasmModuleUrl is required');
            const checkpoint = copyCheckpoint(data.checkpoint);
            initializing = true;
            try {
                const url = new URL(data.wasmModuleUrl, self.location.href);
                if (url.origin !== self.location.origin) throw new Error('strict WASM module must have the worker origin');
                const { default: createModule } = await import(url.href);
                const module = await createModule();
                const adapter = createWasmStrictAdapter(module, checkpoint);
                protocol = createStrictWorkerProtocol(adapter);
            } finally { initializing = false; }
            const response = await protocol.dispatch({ requestId: data.requestId, op: 'hello' });
            self.postMessage({ ok: true, ...response });
        } else {
            if (!protocol) throw new Error('strict worker is not initialized');
            self.postMessage({ ok: true, ...await protocol.dispatch(data) });
        }
    } catch (error) {
        self.postMessage({ ok: false, requestId: data?.requestId, error: String(error.message ?? error) });
    }
});
