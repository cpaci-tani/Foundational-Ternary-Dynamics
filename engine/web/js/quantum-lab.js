/**
 * Quantum Lab — core measurement and analysis module for the FTD web dashboard.
 *
 * Provides the statistical backbone for quantum experiment panels:
 *   - MeasurementAccumulator: async trial runner with abort and progress
 *   - Histogram, FFT, spatial correlation analysis
 *   - Data export (CSV, JSON, clipboard)
 *   - QUANTUM_EXPERIMENTS registry (8 experiments)
 *
 * All measurement functions receive a bridge object (MockBridge or WasmBridge)
 * with the standard API: setupScenario, tick, run, getParticleData,
 * getDiagnostics, getFluxSlice, getFluxVolume, inspectVoxel, latticeSize.
 *
 * Pure ES module, no external dependencies.
 */

import { K_B, ALPHA, C_SPEED } from './constants.js';

// ── MeasurementAccumulator ──────────────────────────────────────────

/**
 * Accumulates measurement results across multiple independent trials.
 *
 * Each trial: reset the simulation, run for N ticks while calling measureFn
 * at each tick, then collect the trial result. Between batches of 10 trials,
 * yields to the event loop via setTimeout(0) so the UI stays responsive.
 *
 * @example
 *   const acc = new MeasurementAccumulator();
 *   acc.configure({ scenarioName: 'quantum-born-rule', totalTrials: 200,
 *       ticksPerTrial: 100, measureFn: myMeasure, resetFn: myReset });
 *   await acc.runAll(bridge, {
 *       onProgress: (i, total) => updateBar(i / total),
 *       onComplete: (results) => plotResults(results),
 *   });
 */
export class MeasurementAccumulator {
    constructor() {
        /** @type {string} */
        this._scenarioName = '';
        /** @type {number} */
        this._totalTrials = 100;
        /** @type {number} */
        this._ticksPerTrial = 50;
        /** @type {Function|null} */
        this._measureFn = null;
        /** @type {Function|null} */
        this._resetFn = null;
        /** @type {Array<*>} */
        this._results = [];
        /** @type {boolean} */
        this._aborted = false;
        /** @type {boolean} */
        this._running = false;
    }

    /**
     * Configure the accumulator for a specific experiment.
     * @param {Object} config
     * @param {string}   config.scenarioName  - Bridge scenario to load before each trial
     * @param {number}   config.totalTrials   - Number of independent trials
     * @param {number}   config.ticksPerTrial - Simulation ticks per trial
     * @param {Function} config.measureFn     - (bridge, trialIndex) => measurement value
     * @param {Function} config.resetFn       - (bridge) => void; called before each trial
     */
    configure(config) {
        this._scenarioName = config.scenarioName || '';
        this._totalTrials = config.totalTrials || 100;
        this._ticksPerTrial = config.ticksPerTrial || 50;
        this._measureFn = config.measureFn || null;
        this._resetFn = config.resetFn || null;
        this._results = [];
        this._aborted = false;
    }

    /**
     * Run all configured trials asynchronously.
     * Batches 10 trials per event-loop yield for throughput.
     *
     * @param {Object} bridge    - MockBridge or WasmBridge instance
     * @param {Object} callbacks
     * @param {Function} [callbacks.onProgress] - (completedTrials, totalTrials) => void
     * @param {Function} [callbacks.onComplete] - (results: Array) => void
     * @returns {Promise<Array>} Collected measurement results
     */
    async runAll(bridge, callbacks = {}) {
        if (this._running) {
            throw new Error('MeasurementAccumulator: already running');
        }
        if (!this._measureFn) {
            throw new Error('MeasurementAccumulator: no measureFn configured');
        }

        this._running = true;
        this._aborted = false;
        this._results = [];

        const BATCH_SIZE = 10;
        const total = this._totalTrials;

        try {
            for (let i = 0; i < total; i++) {
                if (this._aborted) break;

                // Reset simulation state for this trial.
                // resetFn is responsible for calling bridge.setupScenario()
                // internally — we do NOT call setupScenario again here to
                // avoid double-initialization (which wastes work and can
                // conflict with experiments that manage their own tick loop).
                if (this._resetFn) {
                    this._resetFn(bridge);
                }

                // Run the trial: advance ticksPerTrial ticks, then measure
                bridge.run(this._ticksPerTrial);

                try {
                    const measurement = this._measureFn(bridge, i);
                    this._results.push(measurement);
                } catch (measureErr) {
                    console.warn(`MeasurementAccumulator: trial ${i} measureFn error:`, measureErr);
                    this._results.push(null);
                }

                // Progress callback
                if (callbacks.onProgress) {
                    callbacks.onProgress(i + 1, total);
                }

                // Yield to event loop every BATCH_SIZE trials
                if ((i + 1) % BATCH_SIZE === 0 && i + 1 < total) {
                    await new Promise(resolve => setTimeout(resolve, 0));
                }
            }
        } finally {
            this._running = false;
        }

        if (callbacks.onComplete) {
            callbacks.onComplete(this._results);
        }

        return this._results;
    }

    /**
     * Get the collected results array.
     * @returns {Array<*>}
     */
    getResults() {
        return this._results;
    }

    /**
     * Compute descriptive statistics over numeric results.
     * If results contain objects, returns null (use experiment-specific analysis).
     *
     * @returns {{ n: number, mean: number, std: number, min: number, max: number }|null}
     */
    getStatistics() {
        const vals = this._results;
        const n = vals.length;
        if (n === 0) return { n: 0, mean: NaN, std: NaN, min: NaN, max: NaN };
        if (typeof vals[0] !== 'number') return null;

        let sum = 0, sumSq = 0;
        let min = Infinity, max = -Infinity;
        for (let i = 0; i < n; i++) {
            const v = vals[i];
            sum += v;
            sumSq += v * v;
            if (v < min) min = v;
            if (v > max) max = v;
        }
        const mean = sum / n;
        const variance = (sumSq / n) - (mean * mean);
        const std = Math.sqrt(Math.max(0, variance));

        return { n, mean, std, min, max };
    }

    /**
     * Abort a running experiment. The current batch will finish but no
     * new trials will start.
     */
    abort() {
        this._aborted = true;
    }
}

// ── Histogram ───────────────────────────────────────────────────────

/**
 * Compute a histogram of numeric values.
 *
 * @param {number[]|Float64Array} values - Input data
 * @param {number} [bins=20]             - Number of bins
 * @param {number[]|null} [range=null]   - [min, max]; auto-detected if null
 * @returns {{ edges: Float64Array, counts: Uint32Array, binWidth: number }}
 */
export function computeHistogram(values, bins = 20, range = null) {
    const n = values.length;
    let lo, hi;

    if (range) {
        lo = range[0];
        hi = range[1];
    } else {
        lo = Infinity;
        hi = -Infinity;
        for (let i = 0; i < n; i++) {
            const v = values[i];
            if (v < lo) lo = v;
            if (v > hi) hi = v;
        }
    }

    // Handle degenerate case: all values identical
    if (hi === lo) {
        hi = lo + 1;
    }

    const binWidth = (hi - lo) / bins;
    const edges = new Float64Array(bins + 1);
    const counts = new Uint32Array(bins);

    for (let i = 0; i <= bins; i++) {
        edges[i] = lo + i * binWidth;
    }

    for (let i = 0; i < n; i++) {
        const v = values[i];
        if (v < lo || v > hi) continue;
        let idx = Math.floor((v - lo) / binWidth);
        // Clamp the upper boundary value into the last bin
        if (idx >= bins) idx = bins - 1;
        counts[idx]++;
    }

    return { edges, counts, binWidth };
}

// ── FFT (Cooley-Tukey radix-2 DIT) ─────────────────────────────────

/**
 * Compute the Discrete Fourier Transform using the Cooley-Tukey radix-2
 * decimation-in-time algorithm. The input signal is zero-padded to the
 * next power of 2.
 *
 * @param {number[]|Float64Array} signal     - Real-valued time-domain signal
 * @param {number}                [sampleRate=1] - Samples per unit time
 * @returns {{ frequencies: Float64Array, magnitudes: Float64Array }}
 *   frequencies: one-sided frequency axis [0, Nyquist]
 *   magnitudes:  corresponding |X(f)| values (amplitude spectrum)
 */
export function computeDFT(signal, sampleRate = 1) {
    const rawLen = signal.length;
    if (rawLen === 0) {
        return { frequencies: new Float64Array(0), magnitudes: new Float64Array(0) };
    }

    // Zero-pad to next power of 2
    const N = nextPow2(rawLen);

    // Interleaved complex array: [re0, im0, re1, im1, ...]
    const buf = new Float64Array(2 * N);
    for (let i = 0; i < rawLen; i++) {
        buf[2 * i] = signal[i];
        // imaginary part stays 0
    }

    // In-place Cooley-Tukey FFT
    _fftInPlace(buf, N);

    // Extract one-sided magnitude spectrum
    const halfN = Math.floor(N / 2) + 1;
    const frequencies = new Float64Array(halfN);
    const magnitudes = new Float64Array(halfN);
    const df = sampleRate / N;

    for (let k = 0; k < halfN; k++) {
        frequencies[k] = k * df;
        const re = buf[2 * k];
        const im = buf[2 * k + 1];
        magnitudes[k] = Math.sqrt(re * re + im * im) / N;
    }

    return { frequencies, magnitudes };
}

/**
 * Next power of 2 >= n.
 * @param {number} n
 * @returns {number}
 */
function nextPow2(n) {
    let p = 1;
    while (p < n) p <<= 1;
    return p;
}

/**
 * In-place radix-2 Cooley-Tukey FFT on interleaved complex data.
 * buf has length 2*N (re/im pairs). N must be a power of 2.
 * @param {Float64Array} buf
 * @param {number} N
 */
function _fftInPlace(buf, N) {
    // Bit-reversal permutation
    const logN = Math.log2(N) | 0;
    for (let i = 0; i < N; i++) {
        const j = _bitReverse(i, logN);
        if (j > i) {
            // Swap complex elements i and j
            const tmpRe = buf[2 * i];
            const tmpIm = buf[2 * i + 1];
            buf[2 * i]     = buf[2 * j];
            buf[2 * i + 1] = buf[2 * j + 1];
            buf[2 * j]     = tmpRe;
            buf[2 * j + 1] = tmpIm;
        }
    }

    // Butterfly stages
    for (let size = 2; size <= N; size *= 2) {
        const halfSize = size / 2;
        const angle = -2 * Math.PI / size;

        for (let i = 0; i < N; i += size) {
            for (let k = 0; k < halfSize; k++) {
                const theta = angle * k;
                const twRe = Math.cos(theta);
                const twIm = Math.sin(theta);

                const evenIdx = 2 * (i + k);
                const oddIdx  = 2 * (i + k + halfSize);

                const oRe = buf[oddIdx] * twRe - buf[oddIdx + 1] * twIm;
                const oIm = buf[oddIdx] * twIm + buf[oddIdx + 1] * twRe;

                buf[oddIdx]     = buf[evenIdx]     - oRe;
                buf[oddIdx + 1] = buf[evenIdx + 1] - oIm;
                buf[evenIdx]     += oRe;
                buf[evenIdx + 1] += oIm;
            }
        }
    }
}

/**
 * Reverse the lowest `bits` bits of integer `x`.
 * @param {number} x
 * @param {number} bits
 * @returns {number}
 */
function _bitReverse(x, bits) {
    let result = 0;
    for (let i = 0; i < bits; i++) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

// ── Spatial Correlation ─────────────────────────────────────────────

/**
 * Compute the binned two-point spatial correlation function C(d).
 *
 * For every pair (i from A, j from B), computes their Euclidean separation
 * and accumulates the product of their values into distance bins.
 *
 *   C(d) = < v_i * v_j >   for pairs at separation d (binned)
 *
 * @param {Array<{x:number,y:number,z:number}>} positionsA - Positions of set A
 * @param {number[]|Float64Array} valuesA                   - Values at each A position
 * @param {Array<{x:number,y:number,z:number}>} positionsB - Positions of set B
 * @param {number[]|Float64Array} valuesB                   - Values at each B position
 * @param {number} maxDist                                  - Maximum separation to consider
 * @param {number} [bins=20]                                - Number of distance bins
 * @returns {{ distances: Float64Array, correlations: Float64Array, counts: Uint32Array }}
 *   distances:    bin center values
 *   correlations: average product <v_i * v_j> per bin
 *   counts:       number of pairs per bin
 */
export function spatialCorrelation(positionsA, valuesA, positionsB, valuesB, maxDist, bins = 20) {
    const binWidth = maxDist / bins;
    const sums = new Float64Array(bins);
    const counts = new Uint32Array(bins);
    const distances = new Float64Array(bins);

    // Bin centers
    for (let b = 0; b < bins; b++) {
        distances[b] = (b + 0.5) * binWidth;
    }

    const nA = positionsA.length;
    const nB = positionsB.length;

    for (let i = 0; i < nA; i++) {
        const pA = positionsA[i];
        const vA = valuesA[i];

        for (let j = 0; j < nB; j++) {
            const pB = positionsB[j];
            const dx = pA.x - pB.x;
            const dy = pA.y - pB.y;
            const dz = pA.z - pB.z;
            const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);

            if (dist >= maxDist || dist < 0) continue;

            const bin = Math.floor(dist / binWidth);
            if (bin >= bins) continue;

            sums[bin] += vA * valuesB[j];
            counts[bin]++;
        }
    }

    // Normalize: average product per bin
    const correlations = new Float64Array(bins);
    for (let b = 0; b < bins; b++) {
        correlations[b] = counts[b] > 0 ? sums[b] / counts[b] : 0;
    }

    return { distances, correlations, counts };
}

// ── Data Export ──────────────────────────────────────────────────────

/**
 * Generate an ISO date string for filenames (YYYY-MM-DD).
 * @returns {string}
 */
function _dateStamp() {
    const d = new Date();
    const yyyy = d.getFullYear();
    const mm = String(d.getMonth() + 1).padStart(2, '0');
    const dd = String(d.getDate()).padStart(2, '0');
    return `${yyyy}-${mm}-${dd}`;
}

/**
 * Trigger a browser file download from a Blob.
 * @param {Blob} blob
 * @param {string} filename
 */
function _downloadBlob(blob, filename) {
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    a.style.display = 'none';
    document.body.appendChild(a);
    a.click();
    // Clean up after a brief delay to ensure download starts
    setTimeout(() => {
        URL.revokeObjectURL(url);
        document.body.removeChild(a);
    }, 100);
}

/**
 * Export tabular data as a CSV file and trigger download.
 *
 * @param {string[]} columns - Column header names
 * @param {Array<Array<*>>} rows - Row data (each row is an array matching columns)
 * @param {string} filename - Download filename (e.g. 'ftd-quantum-born-rule-2026-04-13.csv')
 */
export function exportCSV(columns, rows, filename) {
    const lines = [];

    // Header
    lines.push(columns.map(_csvEscape).join(','));

    // Data rows
    for (const row of rows) {
        lines.push(row.map(_csvEscape).join(','));
    }

    const content = lines.join('\n') + '\n';
    const blob = new Blob([content], { type: 'text/csv;charset=utf-8' });
    _downloadBlob(blob, filename || `ftd-export-${_dateStamp()}.csv`);
}

/**
 * Escape a value for CSV: wrap in quotes if it contains comma, quote, or newline.
 * @param {*} val
 * @returns {string}
 */
function _csvEscape(val) {
    if (val === null || val === undefined) return '';
    const s = String(val);
    if (s.includes(',') || s.includes('"') || s.includes('\n')) {
        return '"' + s.replace(/"/g, '""') + '"';
    }
    return s;
}

/**
 * Export data as a pretty-printed JSON file and trigger download.
 *
 * @param {Object} metadata - Arbitrary metadata (experiment name, parameters, etc.)
 * @param {Array<Object>} rows - Data rows as objects
 * @param {string} filename - Download filename (e.g. 'ftd-quantum-born-rule-2026-04-13.json')
 */
export function exportJSON(metadata, rows, filename) {
    const payload = {
        metadata: metadata,
        data: rows,
        exportedAt: new Date().toISOString(),
    };

    const content = JSON.stringify(payload, null, 2);
    const blob = new Blob([content], { type: 'application/json;charset=utf-8' });
    _downloadBlob(blob, filename || `ftd-export-${_dateStamp()}.json`);
}

/**
 * Copy tabular data to the clipboard as tab-separated text,
 * suitable for pasting into spreadsheets.
 *
 * @param {string[]} columns - Column header names
 * @param {Array<Array<*>>} rows - Row data
 * @returns {Promise<void>}
 */
export async function copyToClipboard(columns, rows) {
    const lines = [];
    lines.push(columns.join('\t'));
    for (const row of rows) {
        lines.push(row.map(v => (v === null || v === undefined) ? '' : String(v)).join('\t'));
    }
    const text = lines.join('\n');

    if (navigator.clipboard && navigator.clipboard.writeText) {
        await navigator.clipboard.writeText(text);
    } else {
        // Fallback for older browsers / non-secure contexts
        const textarea = document.createElement('textarea');
        textarea.value = text;
        textarea.style.position = 'fixed';
        textarea.style.left = '-9999px';
        document.body.appendChild(textarea);
        textarea.select();
        document.execCommand('copy');
        document.body.removeChild(textarea);
    }
}

// ── Experiment Registry ─────────────────────────────────────────────

/**
 * @typedef {Object} QuantumExperiment
 * @property {string}   label         - Human-readable experiment name
 * @property {string}   description   - One-line description
 * @property {number}   defaultTrials - Default number of trials
 * @property {number}   defaultTicks  - Default ticks per trial
 * @property {Function} measureFn     - (bridge, trialIndex) => measurement
 * @property {Function} resetFn       - (bridge) => void
 * @property {Function} analyseFn     - (results) => analysisObject
 * @property {string[]} columns       - Column names for tabular output
 * @property {Function} expectedCurve - (x) => y; theoretical prediction
 */

/**
 * Registry of quantum experiments available in the Quantum Lab.
 * Each entry is keyed by its scenario identifier.
 *
 * @type {Object<string, QuantumExperiment>}
 */
export const QUANTUM_EXPERIMENTS = {

    // ── 1. Born Rule ────────────────────────────────────────────────
    'quantum-born-rule': {
        label: 'Born Rule',
        description: 'Verify |J|^2 probability envelope for particle manifestation',
        defaultTrials: 200,
        defaultTicks: 100,

        /**
         * Record all manifested particle positions and the flux density
         * at each position. Returns an array of {r, fluxDensity} objects,
         * where r is the radial distance from lattice center.
         */
        measureFn(bridge, _trialIndex) {
            const data = bridge.getParticleData();
            const N = bridge.latticeSize;
            const mid = N / 2;
            const measurements = [];

            if (!data || !data.positions) return measurements;

            const pos = data.positions;
            const count = pos.length / 3;

            for (let i = 0; i < count; i++) {
                const px = pos[3 * i]     - mid;
                const py = pos[3 * i + 1] - mid;
                const pz = pos[3 * i + 2] - mid;
                const r = Math.sqrt(px * px + py * py + pz * pz);

                // Sample flux density at particle position
                const vx = Math.round(pos[3 * i]);
                const vy = Math.round(pos[3 * i + 1]);
                const vz = Math.round(pos[3 * i + 2]);
                const voxel = bridge.inspectVoxel(
                    Math.max(0, Math.min(N - 1, vx)),
                    Math.max(0, Math.min(N - 1, vy)),
                    Math.max(0, Math.min(N - 1, vz))
                );

                const fluxDensity = voxel
                    ? Math.sqrt((voxel.jx || 0) ** 2 + (voxel.jy || 0) ** 2 + (voxel.jz || 0) ** 2)
                    : 0;

                measurements.push({ r, fluxDensity });
            }

            return measurements;
        },

        resetFn(bridge) {
            bridge.setupScenario('quantum-born-rule');
        },

        /**
         * Build radial histogram of particle positions and compare
         * to |J|^2 envelope.
         */
        analyseFn(results) {
            // Flatten all measurements across trials
            const allR = [];
            const allFlux = [];
            for (const trial of results) {
                if (!Array.isArray(trial)) continue;
                for (const m of trial) {
                    allR.push(m.r);
                    allFlux.push(m.fluxDensity);
                }
            }

            if (allR.length === 0) {
                return { histogram: null, fluxProfile: null, n: 0 };
            }

            const histogram = computeHistogram(allR, 25);

            // Compute average |J|^2 per radial bin for comparison
            const maxR = histogram.edges[histogram.edges.length - 1];
            const binWidth = histogram.binWidth;
            const fluxSums = new Float64Array(25);
            const fluxCounts = new Uint32Array(25);

            for (let i = 0; i < allR.length; i++) {
                let bin = Math.floor(allR[i] / binWidth);
                if (bin >= 25) bin = 24;
                if (bin < 0) continue;
                fluxSums[bin] += allFlux[i] * allFlux[i]; // |J|^2
                fluxCounts[bin]++;
            }

            const fluxProfile = new Float64Array(25);
            for (let b = 0; b < 25; b++) {
                fluxProfile[b] = fluxCounts[b] > 0 ? fluxSums[b] / fluxCounts[b] : 0;
            }

            return {
                histogram,
                fluxProfile,
                n: allR.length,
                maxR,
            };
        },

        columns: ['radialDistance', 'fluxDensity'],

        /**
         * Born rule: probability density ~ |J|^2.
         * Gaussian envelope expected from initial flux pulse.
         */
        expectedCurve(x) {
            const sigma = 4.0;
            return Math.exp(-x * x / (2 * sigma * sigma));
        },
    },

    // ── 2. Double Slit ──────────────────────────────────────────────
    'quantum-double-slit': {
        label: 'Double Slit',
        description: 'Accumulate detector-screen intensity profile and measure fringe visibility',
        defaultTrials: 500,
        defaultTicks: 80,

        /**
         * Get the flux intensity at the detector screen (a y-z slice
         * at the far end of the x-axis).
         */
        measureFn(bridge, _trialIndex) {
            const N = bridge.latticeSize;
            const screenX = N - 2; // detector screen near far boundary
            const slice = bridge.getFluxSlice(0, screenX); // axis=0 (x), index=screenX
            if (!slice || slice.length === 0) return [];

            // Slice is a flat array of flux magnitudes along y-z plane.
            // Extract the central row (z = mid) to get 1D intensity profile.
            const mid = Math.floor(N / 2);
            const profile = [];
            for (let y = 0; y < N; y++) {
                const idx = mid * N + y; // z=mid row
                const intensity = idx < slice.length ? slice[idx] : 0;
                profile.push({ y, intensity: Math.abs(intensity) });
            }
            return profile;
        },

        resetFn(bridge) {
            bridge.setupScenario('quantum-double-slit');
        },

        /**
         * Accumulate screen profiles across trials, compute fringe
         * visibility V = (I_max - I_min) / (I_max + I_min).
         */
        analyseFn(results) {
            if (results.length === 0 || !Array.isArray(results[0])) {
                return { profile: [], visibility: 0, n: 0 };
            }

            const N = results[0].length;
            if (N === 0) return { profile: [], visibility: 0, n: 0 };

            // Average intensity at each y position
            const avgIntensity = new Float64Array(N);
            let validTrials = 0;

            for (const trial of results) {
                if (!Array.isArray(trial) || trial.length !== N) continue;
                validTrials++;
                for (let y = 0; y < N; y++) {
                    avgIntensity[y] += trial[y].intensity || 0;
                }
            }

            if (validTrials > 0) {
                for (let y = 0; y < N; y++) {
                    avgIntensity[y] /= validTrials;
                }
            }

            // Fringe visibility
            let iMax = -Infinity, iMin = Infinity;
            for (let y = 0; y < N; y++) {
                if (avgIntensity[y] > iMax) iMax = avgIntensity[y];
                if (avgIntensity[y] < iMin) iMin = avgIntensity[y];
            }
            const visibility = (iMax + iMin) > 0 ? (iMax - iMin) / (iMax + iMin) : 0;

            const profile = [];
            for (let y = 0; y < N; y++) {
                profile.push({ y, intensity: avgIntensity[y] });
            }

            return { profile, visibility, n: validTrials };
        },

        columns: ['yPosition', 'intensity'],

        /** Ideal double-slit interference pattern: cos^2 fringes. */
        expectedCurve(x) {
            const slitSpacing = 8.0;
            const wavelength = 4.0;
            const k = (2 * Math.PI / wavelength) * (slitSpacing / 2);
            return Math.cos(k * x) ** 2;
        },
    },

    // ── 3. Quantum Tunnelling ───────────────────────────────────────
    'quantum-tunnel': {
        label: 'Quantum Tunnelling',
        description: 'Measure transmission coefficient T(W) through a potential barrier',
        defaultTrials: 300,
        defaultTicks: 120,

        /**
         * Compute energy in the transmitted region (x > barrier) vs
         * reflected region (x < barrier). The barrier sits at lattice center.
         */
        measureFn(bridge, trialIndex) {
            const N = bridge.latticeSize;
            const mid = Math.floor(N / 2);
            const barrierWidth = 2 + (trialIndex % 6); // Vary barrier width across trials

            let transmittedEnergy = 0;
            let reflectedEnergy = 0;

            // Sample a line of voxels along x-axis at y=mid, z=mid
            for (let x = 0; x < N; x++) {
                const voxel = bridge.inspectVoxel(x, mid, mid);
                if (!voxel) continue;
                const j2 = (voxel.jx || 0) ** 2 + (voxel.jy || 0) ** 2 + (voxel.jz || 0) ** 2;

                if (x > mid + barrierWidth) {
                    transmittedEnergy += j2;
                } else if (x < mid) {
                    reflectedEnergy += j2;
                }
            }

            const total = transmittedEnergy + reflectedEnergy;
            const T = total > 0 ? transmittedEnergy / total : 0;

            return { barrierWidth, T, transmittedEnergy, reflectedEnergy };
        },

        resetFn(bridge) {
            bridge.setupScenario('quantum-tunnel');
        },

        /**
         * Collect T(W) for multiple barrier widths and compute
         * average transmission per width.
         */
        analyseFn(results) {
            const byWidth = new Map();
            for (const r of results) {
                if (!r || typeof r.barrierWidth !== 'number') continue;
                if (!byWidth.has(r.barrierWidth)) {
                    byWidth.set(r.barrierWidth, []);
                }
                byWidth.get(r.barrierWidth).push(r.T);
            }

            const widths = [];
            const transmissions = [];
            for (const [w, tValues] of [...byWidth.entries()].sort((a, b) => a[0] - b[0])) {
                const avg = tValues.reduce((s, v) => s + v, 0) / tValues.length;
                widths.push(w);
                transmissions.push(avg);
            }

            return {
                widths: new Float64Array(widths),
                transmissions: new Float64Array(transmissions),
                n: results.length,
            };
        },

        columns: ['barrierWidth', 'transmissionCoefficient'],

        /** Exponential decay of tunnelling probability with barrier width. */
        expectedCurve(x) {
            const kappa = 0.5; // Decay constant (depends on barrier height)
            return Math.exp(-2 * kappa * x);
        },
    },

    // ── 4. Quantum Well (Energy Levels) ─────────────────────────────
    'quantum-well': {
        label: 'Quantum Well',
        description: 'FFT flux oscillations in a confining potential to find energy-level spacing',
        defaultTrials: 1,
        defaultTicks: 0, // measureFn runs its own tick-by-tick loop for time-series sampling

        /**
         * This is a single long run (not per-trial). Sample J_y at box
         * center each tick. Since we need per-tick flux samples, this
         * measureFn runs its own tick loop (defaultTicks is 0 so the
         * accumulator does not run ticks redundantly).
         */
        measureFn(bridge, _trialIndex) {
            const N = bridge.latticeSize;
            const mid = Math.floor(N / 2);
            const timeSeries = [];

            // Run tick-by-tick to sample flux at center (scenario already
            // set up by resetFn — do not re-call setupScenario here)
            for (let t = 0; t < 1024; t++) {
                bridge.tick();
                const voxel = bridge.inspectVoxel(mid, mid, mid);
                timeSeries.push(voxel ? (voxel.jy || 0) : 0);
            }

            return timeSeries;
        },

        resetFn(bridge) {
            bridge.setupScenario('quantum-well');
        },

        /**
         * Apply FFT to the time series and find frequency peaks.
         * Peaks correspond to energy-level transitions.
         */
        analyseFn(results) {
            if (results.length === 0 || !Array.isArray(results[0])) {
                return { frequencies: [], magnitudes: [], peaks: [] };
            }

            const signal = results[0];
            const { frequencies, magnitudes } = computeDFT(signal, 1.0);

            // Find peaks: local maxima above noise floor
            const noiseFloor = _computeNoiseFloor(magnitudes);
            const peaks = [];

            for (let i = 2; i < magnitudes.length - 2; i++) {
                if (magnitudes[i] > magnitudes[i - 1] &&
                    magnitudes[i] > magnitudes[i + 1] &&
                    magnitudes[i] > magnitudes[i - 2] &&
                    magnitudes[i] > magnitudes[i + 2] &&
                    magnitudes[i] > noiseFloor * 3) {
                    peaks.push({
                        frequency: frequencies[i],
                        magnitude: magnitudes[i],
                        index: i,
                    });
                }
            }

            // Sort peaks by magnitude descending
            peaks.sort((a, b) => b.magnitude - a.magnitude);

            return {
                frequencies,
                magnitudes,
                peaks: peaks.slice(0, 10), // Top 10 peaks
                signalLength: signal.length,
            };
        },

        columns: ['frequency', 'magnitude'],

        /** Quantum well energy levels: E_n ~ n^2. */
        expectedCurve(x) {
            // Frequency of n-th mode in a box of width L
            const L = 32;
            return (x * Math.PI / L) ** 2;
        },
    },

    // ── 5. Entanglement Correlation ─────────────────────────────────
    'quantum-entangle': {
        label: 'Entanglement',
        description: 'Measure state-product correlation C(d) between +1 and -1 particles',
        defaultTrials: 500,
        defaultTicks: 60,

        /**
         * Find all +1 and -1 particles, record their separation and
         * state product (+1 * -1 = -1 for entangled pairs).
         */
        measureFn(bridge, _trialIndex) {
            const data = bridge.getParticleData();
            if (!data || !data.positions || !data.colors) return [];

            const pos = data.positions;
            const colors = data.colors;
            const count = pos.length / 3;

            // Separate particles by charge (inferred from color)
            // Convention: green = +1 (colors[4i+1] > 0.5), red = -1 (colors[4i] > 0.5)
            const positives = [];
            const negatives = [];

            for (let i = 0; i < count; i++) {
                const px = pos[3 * i];
                const py = pos[3 * i + 1];
                const pz = pos[3 * i + 2];

                // Color channel heuristic: RGBA with 4 components per particle
                const ri = colors[4 * i];
                const gi = colors[4 * i + 1];

                if (gi > ri) {
                    positives.push({ x: px, y: py, z: pz, state: +1 });
                } else {
                    negatives.push({ x: px, y: py, z: pz, state: -1 });
                }
            }

            // Compute pairwise separations and state products
            const pairs = [];
            for (const p of positives) {
                for (const n of negatives) {
                    const dx = p.x - n.x;
                    const dy = p.y - n.y;
                    const dz = p.z - n.z;
                    const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    pairs.push({ separation: dist, stateProduct: p.state * n.state });
                }
            }

            return pairs;
        },

        resetFn(bridge) {
            bridge.setupScenario('quantum-entangle');
        },

        /**
         * Bin pairs by distance and compute average correlation C(d).
         */
        analyseFn(results) {
            const allSep = [];
            const allProd = [];

            for (const trial of results) {
                if (!Array.isArray(trial)) continue;
                for (const pair of trial) {
                    allSep.push(pair.separation);
                    allProd.push(pair.stateProduct);
                }
            }

            if (allSep.length === 0) {
                return { distances: new Float64Array(0), correlations: new Float64Array(0), n: 0 };
            }

            const maxDist = Math.max(...allSep);
            const bins = 20;
            const binWidth = maxDist / bins;
            const sums = new Float64Array(bins);
            const counts = new Uint32Array(bins);
            const distances = new Float64Array(bins);

            for (let b = 0; b < bins; b++) {
                distances[b] = (b + 0.5) * binWidth;
            }

            for (let i = 0; i < allSep.length; i++) {
                let bin = Math.floor(allSep[i] / binWidth);
                if (bin >= bins) bin = bins - 1;
                sums[bin] += allProd[i];
                counts[bin]++;
            }

            const correlations = new Float64Array(bins);
            for (let b = 0; b < bins; b++) {
                correlations[b] = counts[b] > 0 ? sums[b] / counts[b] : 0;
            }

            return { distances, correlations, counts, n: allSep.length };
        },

        columns: ['separation', 'correlation'],

        /**
         * Anti-correlation expected for entangled pairs: C(d) -> -1
         * at short range, decaying toward 0 at large separation.
         */
        expectedCurve(x) {
            const xi = 8.0; // correlation length
            return -Math.exp(-x / xi);
        },
    },

    // ── 6. Aharonov-Bohm Phase ──────────────────────────────────────
    'quantum-aharonov-bohm': {
        label: 'Aharonov-Bohm',
        description: 'Track phase difference at convergence point vs enclosed flux',
        defaultTrials: 200,
        defaultTicks: 100,

        /**
         * Compute the phase of J at the convergence point from both
         * paths around the enclosed flux region.
         */
        measureFn(bridge, trialIndex) {
            const N = bridge.latticeSize;
            const mid = Math.floor(N / 2);

            // Convergence point: far side from source
            const convX = mid + Math.floor(N / 4);
            const convY = mid;
            const convZ = mid;

            // Two paths: above and below the enclosed flux region
            const pathAbove = bridge.inspectVoxel(convX, convY + 2, convZ);
            const pathBelow = bridge.inspectVoxel(convX, convY - 2, convZ);

            if (!pathAbove || !pathBelow) {
                return { enclosedFlux: 0, phaseDiff: 0 };
            }

            // Phase from atan2(Jy, Jx) at each path endpoint
            const phaseA = Math.atan2(pathAbove.jy || 0, pathAbove.jx || 0);
            const phaseB = Math.atan2(pathBelow.jy || 0, pathBelow.jx || 0);

            let phaseDiff = phaseA - phaseB;
            // Wrap to [-pi, pi]
            while (phaseDiff > Math.PI) phaseDiff -= 2 * Math.PI;
            while (phaseDiff < -Math.PI) phaseDiff += 2 * Math.PI;

            // Estimate enclosed flux from the solenoid region
            const solenoidVoxel = bridge.inspectVoxel(mid, mid, mid);
            const enclosedFlux = solenoidVoxel
                ? Math.sqrt((solenoidVoxel.jx || 0) ** 2 + (solenoidVoxel.jy || 0) ** 2 + (solenoidVoxel.jz || 0) ** 2)
                : 0;

            return { enclosedFlux, phaseDiff, trialIndex };
        },

        resetFn(bridge) {
            bridge.setupScenario('quantum-aharonov-bohm');
        },

        /**
         * Track phase difference as a function of enclosed flux.
         */
        analyseFn(results) {
            const fluxValues = [];
            const phaseValues = [];

            for (const r of results) {
                if (!r || typeof r.phaseDiff !== 'number') continue;
                fluxValues.push(r.enclosedFlux);
                phaseValues.push(r.phaseDiff);
            }

            return {
                enclosedFlux: new Float64Array(fluxValues),
                phaseDifferences: new Float64Array(phaseValues),
                n: fluxValues.length,
            };
        },

        columns: ['enclosedFlux', 'phaseDifference'],

        /**
         * Aharonov-Bohm: phase shift = e * Phi / hbar.
         * In lattice units, phase = alpha * enclosedFlux.
         */
        expectedCurve(x) {
            return ALPHA * x;
        },
    },

    // ── 7. Casimir Effect ───────────────────────────────────────────
    'quantum-casimir': {
        label: 'Casimir Effect',
        description: 'Measure vacuum flux pressure between plates vs separation',
        defaultTrials: 100,
        defaultTicks: 150,

        /**
         * Compute |J|^2 between two parallel plates and outside them.
         * Plates are y-z planes at x = mid +/- d/2.
         */
        measureFn(bridge, trialIndex) {
            const N = bridge.latticeSize;
            const mid = Math.floor(N / 2);
            const separation = 3 + (trialIndex % 8); // Vary plate separation

            const plateLeft = mid - Math.floor(separation / 2);
            const plateRight = mid + Math.floor(separation / 2);

            let insideEnergy = 0;
            let outsideEnergy = 0;
            let insideCount = 0;
            let outsideCount = 0;

            // Sample along x-axis at y=mid, z=mid
            for (let x = 1; x < N - 1; x++) {
                const voxel = bridge.inspectVoxel(x, mid, mid);
                if (!voxel) continue;
                const j2 = (voxel.jx || 0) ** 2 + (voxel.jy || 0) ** 2 + (voxel.jz || 0) ** 2;

                if (x > plateLeft && x < plateRight) {
                    insideEnergy += j2;
                    insideCount++;
                } else if (x < plateLeft - 2 || x > plateRight + 2) {
                    outsideEnergy += j2;
                    outsideCount++;
                }
            }

            const avgInside = insideCount > 0 ? insideEnergy / insideCount : 0;
            const avgOutside = outsideCount > 0 ? outsideEnergy / outsideCount : 0;
            const pressure = avgOutside - avgInside; // Positive = attractive

            return { separation, pressure, avgInside, avgOutside };
        },

        resetFn(bridge) {
            bridge.setupScenario('quantum-casimir');
        },

        /**
         * Collect pressure vs plate separation for multiple d values.
         */
        analyseFn(results) {
            const bySep = new Map();
            for (const r of results) {
                if (!r || typeof r.separation !== 'number') continue;
                if (!bySep.has(r.separation)) {
                    bySep.set(r.separation, []);
                }
                bySep.get(r.separation).push(r.pressure);
            }

            const separations = [];
            const pressures = [];
            for (const [d, pValues] of [...bySep.entries()].sort((a, b) => a[0] - b[0])) {
                const avg = pValues.reduce((s, v) => s + v, 0) / pValues.length;
                separations.push(d);
                pressures.push(avg);
            }

            return {
                separations: new Float64Array(separations),
                pressures: new Float64Array(pressures),
                n: results.length,
            };
        },

        columns: ['plateSeparation', 'pressure'],

        /** Casimir pressure ~ -1/d^4 (3+1 dimensions). */
        expectedCurve(x) {
            if (x <= 0) return 0;
            return -Math.PI * Math.PI / (240 * x * x * x * x);
        },
    },

    // ── 8. Quantum Zeno Effect ──────────────────────────────────────
    'quantum-zeno': {
        label: 'Quantum Zeno',
        description: 'Decay probability vs measurement interval: frequent measurement freezes decay',
        defaultTrials: 400,
        defaultTicks: 0, // measureFn runs its own variable-length tick intervals

        /**
         * Count whether a manifestation (state transition) occurred
         * during this trial. The measurement interval is varied by
         * running different numbers of ticks before checking.
         * defaultTicks is 0 so the accumulator does not pre-run ticks.
         */
        measureFn(bridge, trialIndex) {
            const N = bridge.latticeSize;
            const mid = Math.floor(N / 2);

            // Vary measurement interval: check at intervals of 1..20 ticks
            const interval = 1 + (trialIndex % 20);

            // Get initial state at center
            const initial = bridge.inspectVoxel(mid, mid, mid);
            const initialState = initial ? (initial.state || 0) : 0;

            // Run for `interval` ticks, then check if state changed
            bridge.run(interval);

            const final = bridge.inspectVoxel(mid, mid, mid);
            const finalState = final ? (final.state || 0) : 0;

            const decayed = (initialState !== 0 && finalState !== initialState) ? 1 : 0;

            return { interval, decayed, initialState, finalState };
        },

        resetFn(bridge) {
            bridge.setupScenario('quantum-zeno');
        },

        /**
         * Compute decay probability vs measurement interval.
         * Zeno effect: P(decay) -> 0 as interval -> 0.
         */
        analyseFn(results) {
            const byInterval = new Map();
            for (const r of results) {
                if (!r || typeof r.interval !== 'number') continue;
                if (!byInterval.has(r.interval)) {
                    byInterval.set(r.interval, { total: 0, decayed: 0 });
                }
                const entry = byInterval.get(r.interval);
                entry.total++;
                entry.decayed += r.decayed;
            }

            const intervals = [];
            const decayProbs = [];
            for (const [dt, { total, decayed }] of [...byInterval.entries()].sort((a, b) => a[0] - b[0])) {
                intervals.push(dt);
                decayProbs.push(total > 0 ? decayed / total : 0);
            }

            return {
                intervals: new Float64Array(intervals),
                decayProbabilities: new Float64Array(decayProbs),
                n: results.length,
            };
        },

        columns: ['measurementInterval', 'decayProbability'],

        /**
         * Zeno prediction: P(decay) ~ (dt / tau)^2 for small dt,
         * approaching 1 - exp(-dt/tau) for large dt.
         */
        expectedCurve(x) {
            const tau = 10.0; // Decay time constant
            if (x <= 0) return 0;
            // Quadratic suppression at short intervals (Zeno regime)
            const zenoProb = (x / tau) ** 2;
            // Exponential decay at long intervals (normal regime)
            const expProb = 1 - Math.exp(-x / tau);
            // Smooth crossover
            return Math.min(zenoProb, expProb);
        },
    },
};

// ── Internal Helpers ────────────────────────────────────────────────

/**
 * Estimate noise floor as the median magnitude value.
 * @param {Float64Array} magnitudes
 * @returns {number}
 */
function _computeNoiseFloor(magnitudes) {
    if (magnitudes.length === 0) return 0;
    const sorted = Array.from(magnitudes).sort((a, b) => a - b);
    return sorted[Math.floor(sorted.length / 2)];
}
