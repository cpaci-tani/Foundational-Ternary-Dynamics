/**
 * Scale Utilities -- Shared helpers for scale controllers
 * ────────────────────────────────────────────────────────────────────
 *
 * Common formatting, throttling, and DOM-update utilities extracted
 * from app.js so that every scale controller can reuse them
 * without duplicating logic.
 *
 * Functions:
 *   formatNumber(v)          - Human-friendly number display (fixed/exponential)
 *   formatSI(n)              - SI-prefix formatter (K, M, G, T)
 *   createTickAccumulator()  - Fractional-tick accumulator for sub-1 speed
 *   createStatusBarCache()   - Deduplicating DOM writer for status elements
 *   hideScale0Overlays(vp)   - Hide lattice overlays for non-lattice scales
 */

import { SCALE2_TOGGLES } from '../config/toggles.js';


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
    if (typeof n !== 'number' || isNaN(n)) return '0.00';
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

// ── hideScale0Overlays ──────────────────────────────────────────────
/**
 * Hide the Scale-0 lattice visualization overlays before switching to a
 * non-lattice scale.
 *
 * Turns off the flux volume, flux slice, grid, and axes overlays and hides
 * the particle cloud. Used by the Scale 4 (planetary) and Scale 5 (cosmic)
 * controllers, which share an identical overlay-hide preamble.
 *
 * NOTE: Scale 6 (meta) deliberately does NOT use this helper — it hides a
 * different overlay set (it skips the axes overlay and additionally hides the
 * E/B field lines), so collapsing it here would change its behavior.
 *
 * No-op when `viewport` is falsy, so callers need not null-check first.
 *
 * @param {object|null|undefined} viewport - The active viewport instance.
 */
export function hideScale0Overlays(viewport) {
    if (!viewport) return;
    viewport.toggleFluxVolume(false);
    viewport.toggleFluxSlice(false);
    viewport.toggleGrid(false);
    viewport.toggleAxes(false);
    if (viewport.particles) viewport.particles.visible = false;
}

/**
 * Sync all AE toggle checkboxes and sliders to the bridge.
 */
export function syncAEParamsFromUI(bridge) {
    const dtEl = document.getElementById('ae-dt-slider');
    if (dtEl) bridge.aeSetDt(parseFloat(dtEl.value));
    const softEl = document.getElementById('ae-soft-slider');
    if (softEl) bridge.aeSetSoftening(parseFloat(softEl.value));
    for (const [elId, , setter] of SCALE2_TOGGLES) {
        const el = document.getElementById(elId);
        if (el && bridge[setter]) bridge[setter](el.checked);
    }
}

/**
 * Reset all AE toggle checkboxes to their default values and push them to the bridge.
 */
export function resetAETogglesToDefaults(bridge) {
    for (const [elId, defaultVal, setter] of SCALE2_TOGGLES) {
        const el = document.getElementById(elId);
        if (el) el.checked = defaultVal;
        if (bridge[setter]) bridge[setter](defaultVal);
    }
}

