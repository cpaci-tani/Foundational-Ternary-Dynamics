/**
 * Scrub bar DOM template — a floating video-player strip at the bottom of
 * the viewport that also hosts the primary playback controls.
 *
 * Layout (left → right):
 *   [▶] [▷] [⏵] [↺]   │   Speed ── ●   │   ⟲ [── timeline ──] 00:00   │   ● Render   ⚙
 *
 * The ids below match the original toolbar wiring (`btn-play`,
 * `btn-local-play`, `btn-step`, `btn-reset`, `ticks-per-frame`,
 * `tpf-display`) so app.js listeners keep working unchanged.
 */
export function getScrubBarTemplate() {
    const el = document.createElement('div');
    el.id = 'scrub-bar';
    el.className = 'scrub-bar';
    el.setAttribute('role', 'group');
    el.setAttribute('aria-label', 'Playback timeline');
    el.innerHTML = `
        <div class="scrub-bar-section scrub-bar-controls">
            <div class="tb-btn-labeled">
                <button class="tb-btn tb-btn-global" id="btn-play"
                    title="Global Play / Pause (Space) — freezes the whole simulation."
                    aria-label="Global play/pause">&#9654;</button>
                <span class="tb-btn-label">global</span>
            </div>
            <div class="tb-btn-labeled">
                <button class="tb-btn tb-btn-local" id="btn-local-play"
                    title="Local Play / Pause (Shift+Space) — freezes scenario physics; visualization continues."
                    aria-label="Local play/pause">&#9655;</button>
                <span class="tb-btn-label">local</span>
            </div>
            <button class="tb-btn scrub-bar-small-btn" id="btn-step"
                title="Step (S)" aria-label="Step">&#9205;</button>
            <button class="tb-btn scrub-bar-small-btn" id="btn-reset"
                title="Reset (R)" aria-label="Reset">&#8634;</button>
        </div>

        <div class="scrub-bar-divider" aria-hidden="true"></div>

        <div class="scrub-bar-section scrub-bar-speed">
            <span class="scrub-bar-speed-label">Speed</span>
            <input type="range" class="scrub-bar-slider" id="ticks-per-frame"
                min="0" max="100" step="0.1" value="50"
                title="Simulation speed (ticks per animation frame)">
            <span class="scrub-bar-speed-value" id="tpf-display">1.0</span>
        </div>

        <div class="scrub-bar-divider" aria-hidden="true"></div>

        <div class="scrub-bar-section scrub-bar-timeline">
            <button class="scrub-bar-icon-btn scrub-bar-reset"
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
        </div>

        <div class="scrub-bar-section scrub-bar-actions">
            <button class="scrub-bar-icon-btn scrub-bar-render-btn"
                type="button" title="Render next N seconds into a scrubbable clip" aria-label="Render scenario">
                <span class="scrub-bar-render-dot">&#9679;</span>
                <span class="scrub-bar-render-label">Render</span>
            </button>
            <button class="scrub-bar-icon-btn scrub-bar-settings"
                type="button" title="Timeline settings" aria-label="Timeline settings"
                aria-expanded="false" aria-controls="scrub-bar-settings-popover">
                &#9881;&#xFE0E;
            </button>
            <div class="scrub-bar-settings-popover" id="scrub-bar-settings-popover"
                 role="dialog" aria-label="Timeline settings" hidden>
                <!-- Speed presets — click a chip to snap ticks-per-frame to that
                     multiplier. The existing slider still works for continuous
                     tuning; chips are one-tap common values. -->
                <div class="scrub-bar-settings-row">
                    <span class="scrub-bar-settings-label">Speed</span>
                    <div class="scrub-bar-settings-options" role="radiogroup" aria-label="Speed preset">
                        <button type="button" class="scrub-bar-settings-chip" data-speed-preset="0.1"  role="radio">0.1&#215;</button>
                        <button type="button" class="scrub-bar-settings-chip" data-speed-preset="0.5"  role="radio">0.5&#215;</button>
                        <button type="button" class="scrub-bar-settings-chip is-active" data-speed-preset="1"  role="radio" aria-checked="true">1&#215;</button>
                        <button type="button" class="scrub-bar-settings-chip" data-speed-preset="2"    role="radio">2&#215;</button>
                        <button type="button" class="scrub-bar-settings-chip" data-speed-preset="5"    role="radio">5&#215;</button>
                        <button type="button" class="scrub-bar-settings-chip" data-speed-preset="10"   role="radio">10&#215;</button>
                    </div>
                </div>
                <!-- Step-by-N buttons — step the simulation forward by a
                     specific tick count without starting playback. Useful for
                     careful frame-by-frame exploration. Each button fires
                     bridge.tick() the requested number of times synchronously. -->
                <div class="scrub-bar-settings-row">
                    <span class="scrub-bar-settings-label">Step</span>
                    <div class="scrub-bar-settings-options">
                        <button type="button" class="scrub-bar-settings-chip" data-step-by="1"   title="Advance 1 tick">+1</button>
                        <button type="button" class="scrub-bar-settings-chip" data-step-by="10"  title="Advance 10 ticks">+10</button>
                        <button type="button" class="scrub-bar-settings-chip" data-step-by="100" title="Advance 100 ticks">+100</button>
                    </div>
                </div>
                <div class="scrub-bar-settings-row">
                    <span class="scrub-bar-settings-label">Render duration</span>
                    <div class="scrub-bar-settings-options" role="radiogroup" aria-label="Render duration">
                        <button type="button" class="scrub-bar-settings-chip" data-render-secs="10" role="radio">10s</button>
                        <button type="button" class="scrub-bar-settings-chip is-active" data-render-secs="30" role="radio" aria-checked="true">30s</button>
                        <button type="button" class="scrub-bar-settings-chip" data-render-secs="60" role="radio">60s</button>
                    </div>
                </div>
                <div class="scrub-bar-settings-row scrub-bar-settings-hint">
                    Sim ticks run in ~12 ms slices between frames, so live playback keeps moving while the clip builds.
                </div>
            </div>
        </div>
    `;
    return el;
}
