/**
 * @file engine/web/js/viewport/flux-volume-worker-client.js
 * @purpose Owns the flux-volume worker: one request in flight, failure
 *          handling, disposal. Buffer ownership and commit policy stay with
 *          the renderer; this class only moves messages.
 * @consumers ./flux-renderer.js
 * @related ./flux-volume-worker.js (protocol)
 */

const FLUX_VOLUME_WORKER_URL = new URL('./flux-volume-worker.js', import.meta.url);

export class FluxVolumeWorkerClient {
    /**
     * @param {object} options
     * @param {(result:object)=>void} options.onResult   every completed frame, stale or not
     * @param {(message:string)=>void} options.onFailure the worker can no longer be trusted
     * @param {()=>Worker} [options.workerFactory]
     */
    constructor({ onResult, onFailure, workerFactory = null }) {
        this._onResult = onResult;
        this._onFailure = onFailure;
        this._nextId = 1;
        this._inFlight = 0;
        this._failed = false;
        this._worker = null;
        try {
            if (workerFactory) this._worker = workerFactory();
            else if (typeof Worker !== 'undefined') {
                this._worker = new Worker(FLUX_VOLUME_WORKER_URL, { type: 'module', name: 'FTDFluxVolume' });
            }
        } catch (error) {
            this._worker = null;
            console.warn('[flux-volume] worker unavailable:', error?.message || error);
        }
        if (!this._worker) {
            this._failed = true;
            return;
        }
        this._worker.onmessage = ({ data }) => {
            if (this._failed || !data) return;
            if (data.type === 'error') {
                this._fail(data.message || 'frame computation failed');
                return;
            }
            if (data.type !== 'frame' || data.id !== this._inFlight) return;
            this._inFlight = 0;
            this._onResult(data);
        };
        this._worker.onerror = (event) => {
            event?.preventDefault?.();
            this._fail(event?.message || 'unknown worker error');
        };
        this._worker.onmessageerror = () => this._fail('worker response could not be decoded');
    }

    get available() { return !this._failed; }
    get busy() { return this._inFlight !== 0; }

    /**
     * Post one frame. The caller checks `busy` first.
     * @returns {number} the request id, or 0 when the post itself failed (the
     *          client is then unavailable and `onFailure` is NOT called: the
     *          caller still holds the frame and decides what to do with it).
     */
    submit(job, transfer) {
        const id = this._nextId++;
        try {
            this._worker.postMessage({ ...job, type: 'frame', id }, transfer);
        } catch (error) {
            console.warn('[flux-volume] worker post failed:', error?.message || error);
            this.dispose();
            return 0;
        }
        this._inFlight = id;
        return id;
    }

    _fail(message) {
        if (this._failed) return;
        this._failed = true;
        this._inFlight = 0;
        try { this._worker?.terminate(); } catch (_e) { /* already gone */ }
        this._worker = null;
        this._onFailure(message);
    }

    dispose() {
        this._failed = true;
        this._inFlight = 0;
        try { this._worker?.terminate(); } catch (_e) { /* already gone */ }
        this._worker = null;
    }
}
