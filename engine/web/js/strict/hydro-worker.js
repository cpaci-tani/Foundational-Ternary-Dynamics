/** Dedicated worker entry for the isolated compiled hydro candidate (mirrors
 * ./strict-worker.js). The one addition over the strict worker: before any
 * HydroState/adapter/protocol can exist, the 64 MiB collision-table blob must be
 * fetched from `tableUrl` and installed into the freshly-imported module via
 * loadHydroTable (engine/strict/hydro/wasm_bindings_hydro.cpp's module-level
 * `loadTable`, which SHA-256-verifies the blob and throws a clear error on mismatch).
 */
import { createHydroWorkerProtocol, createWasmHydroAdapter, loadHydroTable } from './hydro-worker-protocol.js';

let protocol = null;
let initializing = false;

self.addEventListener('message', async ({ data }) => {
    try {
        if (data?.op === 'init') {
            if (initializing || protocol) throw new Error('hydro worker already initialized or initializing');
            if (typeof data.requestId !== 'string' || !data.requestId
                || typeof data.checkpoint !== 'string') throw new TypeError('invalid initialization');
            if (typeof data.wasmModuleUrl !== 'string') throw new TypeError('wasmModuleUrl is required');
            if (typeof data.tableUrl !== 'string') throw new TypeError('tableUrl is required');
            const checkpoint = data.checkpoint;
            initializing = true;
            try {
                const moduleUrl = new URL(data.wasmModuleUrl, self.location.href);
                if (moduleUrl.origin !== self.location.origin) throw new Error('hydro WASM module must have the worker origin');
                const { default: createModule } = await import(moduleUrl.href);
                const module = await createModule();
                const tableUrl = new URL(data.tableUrl, self.location.href);
                if (tableUrl.origin !== self.location.origin) throw new Error('hydro table blob must have the worker origin');
                await loadHydroTable(module, tableUrl.href);
                const adapter = createWasmHydroAdapter(module, checkpoint);
                protocol = createHydroWorkerProtocol(adapter);
            } finally { initializing = false; }
            const response = await protocol.dispatch({ requestId: data.requestId, op: 'hello' });
            self.postMessage({ ok: true, ...response });
        } else {
            if (!protocol) throw new Error('hydro worker is not initialized');
            self.postMessage({ ok: true, ...await protocol.dispatch(data) });
        }
    } catch (error) {
        self.postMessage({ ok: false, requestId: data?.requestId, error: String(error.message ?? error) });
    }
});
