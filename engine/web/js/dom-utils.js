/**
 * DOM Utilities — shared helpers for DOM access patterns that appear
 * in multiple unrelated modules (diagnostics, pe-telemetry, charts, ...).
 *
 * Keeping these out of scale-utils.js because they aren't scale-specific,
 * and out of a single consumer module because that creates accidental
 * cross-module coupling.
 */

/**
 * Cache a canvas element's CSS rect (width/height) and refresh it only
 * when the canvas actually resizes, instead of calling
 * `getBoundingClientRect()` on every draw.
 *
 * `getBoundingClientRect()` forces a synchronous layout reflow. At 60 fps
 * with several sparkline canvases drawing per frame, the cumulative cost
 * of those reflows pins the main thread and starves the simulation loop.
 *
 * This helper installs a `ResizeObserver` on the canvas and caches the
 * last-observed content rect. The `get()` method returns the cached rect
 * (seeded by a one-shot `getBoundingClientRect()` on first call if the
 * observer hasn't fired yet). `dispose()` disconnects the observer.
 *
 * `ResizeObserver` is widely supported; the fallback path (no observer)
 * still caches the first rect and only re-reads when the cache is
 * explicitly cleared, which is strictly better than per-draw reflows
 * for mostly-static panels.
 *
 * @param {HTMLCanvasElement} canvas
 * @returns {{ get(): { width: number, height: number }, dispose(): void }}
 */
export function createCachedCanvasRect(canvas) {
    let cached = null;
    let observer = null;
    if (typeof ResizeObserver !== 'undefined') {
        observer = new ResizeObserver((entries) => {
            for (const entry of entries) {
                const cr = entry.contentRect;
                cached = { width: cr.width, height: cr.height };
            }
        });
        observer.observe(canvas);
    }
    return {
        get() {
            if (!cached) {
                const r = canvas.getBoundingClientRect();
                cached = { width: r.width, height: r.height };
            }
            return cached;
        },
        dispose() {
            if (observer) { observer.disconnect(); observer = null; }
            cached = null;
        }
    };
}
