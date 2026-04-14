/**
 * Scale Utilities -- Shared helpers for scale controllers
 * ────────────────────────────────────────────────────────────────────
 *
 * Common formatting, throttling, and DOM-update utilities extracted
 * from app_dag.js so that every scale controller can reuse them
 * without duplicating logic.
 *
 * Functions:
 *   formatNumber(v)          - Human-friendly number display (fixed/exponential)
 *   formatSI(n)              - SI-prefix formatter (K, M, G, T)
 *   createTickAccumulator()  - Fractional-tick accumulator for sub-1 speed
 *   throttleBySize(L, thr)   - Lattice-size-dependent throttle selector
 *   createStatusBarCache()   - Deduplicating DOM writer for status elements
 */

// ── formatNumber ────────────────────────────────────────────────────
/**
 * Format a numeric value for display in diagnostics / status bars.
 * Returns fixed-point for normal values, exponential for very large ones,
 * and a safe fallback for NaN / non-numbers.
 *
 * @param {*} v - Value to format
 * @returns {string}
 */
export function formatNumber(v) {
    if (typeof v !== 'number' || isNaN(v)) return '0.0000';
    if (Math.abs(v) >= 10000) return v.toExponential(2);
    return v.toFixed(4);
}

// ── formatSI ────────────────────────────────────────────────────────
/**
 * Format a number with SI magnitude prefixes (K, M, G, T).
 * Keeps two decimal places after the prefix.
 *
 * @param {number} n - Value to format
 * @returns {string}
 */
export function formatSI(n) {
    if (Math.abs(n) >= 1e12) return (n / 1e12).toFixed(2) + 'T';
    if (Math.abs(n) >= 1e9)  return (n / 1e9).toFixed(2)  + 'G';
    if (Math.abs(n) >= 1e6)  return (n / 1e6).toFixed(2)  + 'M';
    if (Math.abs(n) >= 1e3)  return (n / 1e3).toFixed(2)  + 'K';
    return n.toFixed(2);
}

// ── createTickAccumulator ───────────────────────────────────────────
/**
 * Create a fractional-tick accumulator for sub-1 simulation speeds.
 *
 * When ticksPerFrame is e.g. 0.25, calling accumulate(0.25) four times
 * will yield one whole tick. The accumulator tracks the fractional remainder
 * across frames so the simulation runs at the correct average rate.
 *
 * @returns {{ accumulate(ticksPerFrame: number): number, reset(): void }}
 */
export function createTickAccumulator() {
    let _acc = 0;
    return {
        /**
         * Add fractional ticks and return the number of whole ticks to execute.
         * @param {number} ticksPerFrame
         * @returns {number} Whole ticks this frame
         */
        accumulate(ticksPerFrame) {
            _acc += ticksPerFrame;
            const whole = Math.floor(_acc);
            _acc -= whole;
            return whole;
        },
        /** Reset the accumulator (e.g. on scenario change). */
        reset() { _acc = 0; }
    };
}

// ── throttleBySize ──────────────────────────────────────────────────
/**
 * Select a throttle value based on lattice size.
 *
 * Thresholds are checked top-down; the first entry whose `above` value
 * is exceeded by L wins. The last entry should either omit `above` or
 * include a `default` key to serve as the fallback.
 *
 * Example:
 *   throttleBySize(64, [
 *       { above: 96, value: 8 },
 *       { above: 48, value: 4 },
 *       { above: 32, value: 2 },
 *       { default: 1 }
 *   ]);
 *   // returns 4  (64 > 48)
 *
 * @param {number} L - Lattice size
 * @param {Array<{above?: number, value?: *, default?: *}>} thresholds
 * @returns {*} The selected throttle value
 */
export function throttleBySize(L, thresholds) {
    for (const t of thresholds) {
        if (t.above !== undefined && L > t.above) return t.value;
    }
    const last = thresholds[thresholds.length - 1];
    return last.default !== undefined ? last.default : last.value;
}

// ── createStatusBarCache ────────────────────────────────────────────
/**
 * Create a deduplicating DOM writer for status-bar elements.
 *
 * Caches the last-written value for each element ID so that repeated
 * identical updates (common in steady-state simulation) skip the DOM
 * write entirely.  This matters at 60 fps where dozens of elements
 * would otherwise be touched every frame.
 *
 * @returns {{ update(id: string, value: string): void }}
 */
export function createStatusBarCache() {
    const cache = {};
    return {
        /**
         * Set textContent of element `id` to `value`, but only if changed.
         * @param {string} id - DOM element id
         * @param {string} value - New text content
         */
        update(id, value) {
            if (cache[id] === value) return;
            cache[id] = value;
            const el = document.getElementById(id);
            if (el) el.textContent = value;
        }
    };
}
