/**
 * Scrub bar DOM template — a floating video-player strip at the bottom of
 * the viewport. Structure:
 *
 *   [⟲ reset] [────────── timeline ──────────] [time badge] [⚙ settings]
 *
 * The timeline strip is the main interactive surface. Zones (LOD shading +
 * render band) are positioned by ScrubBarComponent.
 */
export function createScrubBarTemplate() {
    const el = document.createElement('div');
    el.id = 'scrub-bar';
    el.className = 'scrub-bar scale0-only';
    el.setAttribute('role', 'group');
    el.setAttribute('aria-label', 'Playback timeline');
    el.innerHTML = `
        <button class="scrub-bar-btn scrub-bar-reset"
                type="button" title="Snap back to live (double-click the timeline)" aria-label="Reset to live">
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
