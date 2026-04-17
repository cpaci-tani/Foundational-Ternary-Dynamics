/**
 * Verification Lab — trial runner.
 *
 * Extracted from the legacy `quantum-lab.js` MeasurementAccumulator.
 * Runs N independent trials of an experiment's measureFn, yielding between
 * batches so the UI stays responsive. Aborts cleanly on user request.
 *
 * Used by the verification panel; pure ES module, no DOM dependencies.
 */

export class ExperimentRunner {
    constructor() {
        this._totalTrials = 0;
        this._ticksPerTrial = 0;
        this._resetFn = null;
        this._measureFn = null;
        this._scenarioId = '';
        this._aborted = false;
        this._running = false;
        /** @type {Array<*>} */
        this._results = [];
    }

    configure({ scenarioId, totalTrials, ticksPerTrial, resetFn, measureFn }) {
        this._scenarioId = scenarioId || '';
        this._totalTrials = Math.max(1, totalTrials | 0);
        this._ticksPerTrial = Math.max(1, ticksPerTrial | 0);
        this._resetFn = typeof resetFn === 'function' ? resetFn : null;
        this._measureFn = typeof measureFn === 'function' ? measureFn : null;
        this._results = [];
        this._aborted = false;
    }

    abort() { this._aborted = true; }
    isRunning() { return this._running; }
    results() { return this._results.slice(); }

    /**
     * @param {object} bridge — WASM or Mock bridge with .setupScenario, .tick, etc.
     * @param {object} callbacks
     * @param {(i:number,total:number,value:*)=>void} callbacks.onProgress
     * @param {(results:*[])=>void} callbacks.onComplete
     * @param {(err:Error)=>void} callbacks.onError
     */
    async runAll(bridge, { onProgress, onComplete, onError } = {}) {
        if (!this._measureFn) {
            onError?.(new Error('ExperimentRunner: no measureFn configured'));
            return;
        }
        this._running = true;
        this._results = [];
        try {
            const BATCH = 10;
            for (let trial = 0; trial < this._totalTrials; trial++) {
                if (this._aborted) break;
                // Reset scenario + any per-trial state
                if (this._scenarioId && bridge.setupScenario) {
                    bridge.setupScenario(this._scenarioId);
                }
                this._resetFn?.(bridge);
                // Run ticks for this trial
                if (bridge.run) {
                    bridge.run(this._ticksPerTrial);
                } else if (bridge.tick) {
                    for (let t = 0; t < this._ticksPerTrial; t++) bridge.tick();
                }
                const value = this._measureFn(bridge, trial);
                this._results.push(value);
                onProgress?.(trial + 1, this._totalTrials, value);
                if ((trial + 1) % BATCH === 0) {
                    await new Promise((r) => setTimeout(r, 0));
                }
            }
            onComplete?.(this._results);
        } catch (err) {
            onError?.(err);
        } finally {
            this._running = false;
        }
    }
}

// ── Default aggregation (mean + stddev) ─────────────────────────────
export function defaultAggregate(results) {
    if (!Array.isArray(results) || results.length === 0) {
        return { mean: NaN, stddev: NaN, count: 0 };
    }
    // Coerce scalars; structured results can ship their own aggregateFn.
    const values = results.map((r) => (typeof r === 'number' ? r : r?.value ?? NaN))
                          .filter((v) => Number.isFinite(v));
    if (values.length === 0) return { mean: NaN, stddev: NaN, count: 0 };
    const mean = values.reduce((a, b) => a + b, 0) / values.length;
    const variance = values.reduce((a, b) => a + (b - mean) ** 2, 0) / values.length;
    return { mean, stddev: Math.sqrt(variance), count: values.length };
}
