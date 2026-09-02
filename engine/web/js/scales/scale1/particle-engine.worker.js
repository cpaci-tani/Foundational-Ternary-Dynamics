// Dedicated single-owner Scale-1 tick worker.  Scenario construction and UI
// mutations remain synchronous on the dashboard's main ParticleEngine.  Each
// request transfers a versioned state-complete checkpoint into this isolated
// engine, advances a bounded tick batch, and returns a new checkpoint.  The
// main executor rejects the result if the source revision changed meanwhile.

const WASM_BASE = new URL('../../../wasm/', self.location.href);
let modulePromise = null;
let mod = null;
let engine = null;

async function ensureEngine() {
    if (!modulePromise) {
        modulePromise = (async () => {
            importScripts(new URL('ftd_core.js', WASM_BASE).href);
            if (typeof createFTDModule !== 'function') {
                throw new Error('Scale 1 worker could not load createFTDModule');
            }
            mod = await createFTDModule({
                locateFile: path => new URL(path, WASM_BASE).href,
            });
            engine = new mod.ParticleEngine();
            return mod;
        })();
    }
    await modulePromise;
}

self.onmessage = async event => {
    const message = event.data || {};
    if (message.type === 'dispose') {
        try { engine?.delete?.(); } catch { /* module teardown */ }
        engine = null;
        self.postMessage({ type: 'disposed' });
        self.close();
        return;
    }
    if (message.type !== 'run') return;
    try {
        await ensureEngine();
        if (!mod.restorePECheckpoint(engine, message.checkpoint.native)) {
            throw new Error('worker rejected the source checkpoint');
        }
        mod.peRunEngine(engine,
            Math.max(0, Math.floor(Number(message.ticks) || 0)));
        const checkpoint = {
            ...message.checkpoint,
            capturedAt: new Date().toISOString(),
            native: mod.exportPECheckpoint(engine),
        };
        self.postMessage({
            type: 'result',
            requestId: message.requestId,
            generation: message.generation,
            sourceRevision: message.sourceRevision,
            checkpoint,
        });
    } catch (error) {
        self.postMessage({
            type: 'error',
            requestId: message.requestId,
            generation: message.generation,
            error: error?.stack || error?.message || String(error),
        });
    }
};

ensureEngine()
    .then(() => self.postMessage({ type: 'ready' }))
    .catch(error => self.postMessage({
        type: 'init-error', error: error?.stack || error?.message || String(error),
    }));
