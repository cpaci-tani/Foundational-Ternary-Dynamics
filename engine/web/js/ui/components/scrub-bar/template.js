/**
 * Scrub bar DOM template — a floating video-player strip at the bottom of
 * the viewport, now also hosting the primary playback controls.
 *
 * Layout (left → right):
 *   [▶ global] [▷ local] [⏵ step] [↺ reset] · [Speed ────●────]
 *     · [⟲ reset playhead] [──── timeline ────] [time badge]
 *     · [● Render] [⚙ settings]
 *
 * Note: the `btn-play`, `btn-local-play`, `btn-step`, `btn-reset`,
 * `ticks-per-frame`, and `tpf-display` IDs match the original toolbar
 * wiring so the existing event listeners in app_dag.js keep working
 * without modification. They just live in a different container now.
 */
export function createScrubBarTemplate() {
    const el = document.createElement('div');
    el.id = 'scrub-bar';
    // Not .scale0-only — this bar hosts global playback controls (play /
    // local / step / reset / speed) that must be available on every scale.
    // Only the scrub-strip portion is Scale-0-meaningful; it degrades
    // gracefully when there's no memory buffer (empty strip, no zones).
    el.className = 'scrub-bar';
    el.setAttribute('role', 'group');
    el.setAttribute('aria-label', 'Playback timeline');
    el.innerHTML = `
        <div class="scrub-bar-controls">
            <div class="tb-btn-labeled">
                <button class="tb-btn tb-btn-global" id="btn-play"
                    title="Global Play / Pause (Space) — freezes the whole simulation, including visualization."
                    aria-label="Global play/pause">&#9654;</button>
                <span class="tb-btn-label">global</span>
            </div>
            <div class="tb-btn-labeled">
                <button class="tb-btn tb-btn-local" id="btn-local-play"
                    title="Local Play / Pause (Shift+Space) — freezes scenario physics only; overlays + render continue."
                    aria-label="Local play/pause">&#9655;</button>
                <span class="tb-btn-label">local</span>
            </div>
            <button class="tb-btn scrub-bar-small-btn" id="btn-step"
                title="Step (S)" aria-label="Step">&#9205;</button>
            <button class="tb-btn scrub-bar-small-btn" id="btn-reset"
                title="Reset (R)" aria-label="Reset">&#8634;</button>
        </div>

        <div class="scrub-bar-speed">
            <label class="tb-label" for="ticks-per-frame">Speed</label>
            <input type="range" class="tb-slider" id="ticks-per-frame"
                min="0" max="100" step="0.1" value="50"
                title="Simulation speed (ticks per animation frame)">
            <span class="tb-value" id="tpf-display">1.0</span>
        </div>

        <div class="scrub-bar-divider" aria-hidden="true"></div>

        <button class="scrub-bar-btn scrub-bar-reset"
                type="button" title="Snap back to live (double-click the timeline)" aria-label="Reset playhead to live">
            &#10227;
        </button>
        <div class="scrub-bar-strip" role="slider"
             aria-valuemin="0" aria-valuemax="1" aria-valuenow="1" tabindex="0">
            <div class="scrub-bar-zones"></div>
            <div class="scrub-bar-render"></div>
            <div class="scrub-bar-playhead"></div>
        </div>
        <span class="scrub-bar-time" aria-live="polite">now</span>
        <button class="scrub-bar-btn scrub-bar-render-btn"
                type="button" title="Render next 30 seconds" aria-label="Render scenario">
            &#9679;<span class="scrub-bar-render-label">Render</span>
        </button>
        <button class="scrub-bar-btn scrub-bar-settings"
                type="button" title="Timeline settings" aria-label="Timeline settings">
            &#9881;&#xFE0E;
        </button>
    `;
    return el;
}
