/**
 * @file engine/web/js/scales/scale0/ui/overlays/moore-trace.js
 * @purpose Bounded current-run history for one Moore-panel sparkline.
 * @consumers ./moore-panel.js; shape read by ui/charts/sparkline.js and
 *            ui/charts/history-window.js (count, total, generation, get,
 *            getTick, last).
 */

// Samples kept per trace. The panel holds 28 traces (focus, sum, 26
// neighbours) and records whenever the view is in a Moore neighbourhood,
// panel open or not, so an unbounded history grew by about 48 MB an hour at
// 30 ticks a second. With this limit "All" shows the most recent 100,000
// samples of the run (about 55 minutes at that rate) and the 28 traces stay
// under 50 MB.
export const MOORE_TRACE_LIMIT = 100000;
// Dropped together once the limit is passed, so trimming happens rarely.
export const MOORE_TRACE_TRIM = 10000;

/** Current-run history, bounded. The chart window chooses what is drawn. */
export function makeMooreTrace(limit = MOORE_TRACE_LIMIT, trim = MOORE_TRACE_TRIM) {
    const values = [];
    const ticks = [];
    return {
        get size() { return Math.max(values.length, 2); },
        get count() { return values.length; },
        /** Samples ever pushed in this run, including any since dropped. */
        total: 0,
        generation: 0,
        push(value, tick) {
            if (!Number.isFinite(value)) return;
            values.push(value);
            ticks.push(Number.isFinite(tick) ? tick : this.total);
            this.total += 1;
            if (values.length >= limit + trim) {
                values.splice(0, trim);
                ticks.splice(0, trim);
            }
        },
        get(index) { return values[index]; },
        getTick(index) { return ticks[index]; },
        last() { return values[values.length - 1]; },
        clear() {
            values.length = 0;
            ticks.length = 0;
            this.total = 0;
            this.generation += 1;
        },
    };
}
