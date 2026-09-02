/**
 * Main-thread coordinator for state-complete Scale-1 worker tick batches.
 *
 * Reads stay synchronous against the main ParticleEngine so renderers and
 * panels need no shadow physics implementation.  A completed worker result is
 * restored only when both its generation and source observation revision are
 * still current.  Mutations therefore fail stale work closed.
 */

export class Scale1ParticleWorkerExecutor {
    constructor({ workerFactory = null } = {}) {
        this.workerFactory = workerFactory || (() => new Worker(
            new URL('./particle-engine.worker.js?v=2', import.meta.url),
        ));
        this.worker = null;
        this.state = 'idle';
        this.busy = false;
        this.generation = 1;
        this.nextRequestId = 1;
        this.active = null;
        this.lastError = '';
    }

    ensure() {
        if (this.worker || this.state === 'failed') return;
        if (typeof Worker === 'undefined' && !this.workerFactory) {
            this.state = 'failed';
            this.lastError = 'Web Worker is unavailable';
            return;
        }
        try {
            this.state = 'loading';
            this.worker = this.workerFactory();
            this.worker.onmessage = event => this._onMessage(event.data || {});
            this.worker.onerror = event => this._fail(event?.message || 'Scale 1 worker error');
        } catch (error) {
            this._fail(error?.message || String(error));
        }
    }

    request(bridge, ticks, onApplied = null) {
        const count = Math.max(0, Math.floor(Number(ticks) || 0));
        if (count < 1) return true;
        this.ensure();
        if (this.state === 'failed') return false;
        // A running batch owns simulation progress. Dropping later frame
        // requests makes a saturated simulation run slower instead of building
        // an unbounded catch-up queue.
        if (this.busy) return true;
        if (this.state !== 'ready') return false;
        const checkpoint = bridge?.peExportCheckpoint?.();
        if (!checkpoint) return false;
        const requestId = this.nextRequestId++;
        const sourceRevision = Number(bridge.peGetObservationRevision?.() || 0);
        this.busy = true;
        this.active = {
            requestId, bridge, sourceRevision,
            generation: this.generation, onApplied,
        };
        this.worker.postMessage({
            type: 'run', requestId, generation: this.generation,
            sourceRevision, ticks: count, checkpoint,
        });
        return true;
    }

    invalidate() {
        this.generation++;
        this.active = null;
        this.busy = false;
    }

    status() {
        return {
            state: this.state,
            busy: this.busy,
            generation: this.generation,
            error: this.lastError,
        };
    }

    dispose() {
        this.invalidate();
        if (this.worker) {
            try { this.worker.postMessage({ type: 'dispose' }); } catch { /* ignore */ }
            try { this.worker.terminate(); } catch { /* ignore */ }
        }
        this.worker = null;
        this.state = 'idle';
    }

    _onMessage(message) {
        if (message.type === 'ready') {
            this.state = 'ready';
            return;
        }
        if (message.type === 'init-error') {
            this._fail(message.error || 'Scale 1 worker initialization failed');
            return;
        }
        if (message.type === 'error') {
            if (this.active?.requestId === message.requestId) {
                this.busy = false;
                this.active = null;
            }
            this.lastError = message.error || 'Scale 1 worker request failed';
            // A request failure falls back to synchronous ticks but does not
            // repeatedly recreate the same broken worker.
            this.state = 'failed';
            return;
        }
        if (message.type !== 'result') return;
        const active = this.active;
        this.active = null;
        this.busy = false;
        if (!active || active.requestId !== message.requestId
            || active.generation !== message.generation
            || this.generation !== message.generation) return;
        const currentRevision = Number(
            active.bridge?.peGetObservationRevision?.() || 0);
        if (currentRevision !== active.sourceRevision) return;
        try {
            if (active.bridge.peRestoreCheckpoint(message.checkpoint)) {
                active.onApplied?.(message.checkpoint);
            }
        } catch (error) {
            this._fail(error?.message || String(error));
        }
    }

    _fail(message) {
        this.lastError = String(message || 'Scale 1 worker failed');
        this.state = 'failed';
        this.busy = false;
        this.active = null;
        if (this.worker) {
            try { this.worker.terminate(); } catch { /* ignore */ }
        }
        this.worker = null;
    }
}

export const scale1ParticleWorkerExecutor = new Scale1ParticleWorkerExecutor();
