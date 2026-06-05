/**
 * Play bar DOM template — a floating control strip at the bottom of the
 * viewport that hosts the primary playback controls.
 *
 * Layout (left to right):
 *   [play] [step] [reset] | [-] [speed] [+] | [now] | [settings]
 *
 * The ids below match the original toolbar wiring (`btn-play`,
 * `btn-step`, `btn-reset`, `ticks-per-frame`, `tpf-display`) so app.js
 * listeners keep working unchanged.
 */
export function getPlayBarTemplate() {
    const el = document.createElement('div');
    el.id = 'play-bar';
    el.className = 'play-bar';
    el.setAttribute('role', 'group');
    el.setAttribute('aria-label', 'Playback controls');
    el.innerHTML = `
        <div class="play-bar-section play-bar-transport" aria-label="Transport controls">
            <button class="tb-btn tb-btn-global play-bar-play-btn" id="btn-play"
                title="Play / Pause (Space)"
                aria-label="Play/pause">&#9654;</button>
            <button class="tb-btn play-bar-small-btn" id="btn-step"
                title="Step (S)" aria-label="Step">&#9205;</button>
            <button class="tb-btn play-bar-small-btn" id="btn-reset"
                title="Reset (R)" aria-label="Reset">&#8634;</button>
        </div>

        <div class="play-bar-divider" aria-hidden="true"></div>

        <div class="play-bar-section play-bar-speed" aria-label="Playback speed">
            <button class="play-bar-icon-btn play-bar-speed-nudge" type="button"
                data-speed-nudge="-5" title="Slower" aria-label="Slower">&minus;</button>
            <span class="play-bar-speed-readout" aria-live="polite">
                <span class="play-bar-speed-value" id="tpf-display">1.0</span>
                <span class="play-bar-speed-unit" aria-hidden="true">&times;</span>
            </span>
            <button class="play-bar-icon-btn play-bar-speed-nudge" type="button"
                data-speed-nudge="5" title="Faster" aria-label="Faster">+</button>
        </div>

        <div class="play-bar-divider" aria-hidden="true"></div>

        <div class="play-bar-section play-bar-now" aria-label="Current tick">
            <span class="play-bar-time">T 0</span>
        </div>

        <div class="play-bar-divider" aria-hidden="true"></div>

        <div class="play-bar-section play-bar-actions">
            <button class="play-bar-icon-btn play-bar-settings"
                type="button" title="Playback settings" aria-label="Playback settings"
                aria-expanded="false" aria-controls="play-bar-settings-popover">
                &#9881;&#xFE0E;
            </button>
            <div class="play-bar-settings-popover" id="play-bar-settings-popover"
                 role="dialog" aria-label="Playback settings" hidden>
                <!-- Speed presets — click a chip to snap ticks-per-frame to that
                     multiplier. The existing slider still works for continuous
                     tuning; chips are one-tap common values. -->
                <div class="play-bar-settings-row">
                    <span class="play-bar-settings-label">Speed</span>
                    <div class="play-bar-settings-options" role="radiogroup" aria-label="Speed preset">
                        <button type="button" class="play-bar-settings-chip" data-speed-preset="0.1"  role="radio">0.1&#215;</button>
                        <button type="button" class="play-bar-settings-chip" data-speed-preset="0.5"  role="radio">0.5&#215;</button>
                        <button type="button" class="play-bar-settings-chip is-active" data-speed-preset="1"  role="radio" aria-checked="true">1&#215;</button>
                        <button type="button" class="play-bar-settings-chip" data-speed-preset="2"    role="radio">2&#215;</button>
                        <button type="button" class="play-bar-settings-chip" data-speed-preset="5"    role="radio">5&#215;</button>
                        <button type="button" class="play-bar-settings-chip" data-speed-preset="10"   role="radio">10&#215;</button>
                    </div>
                </div>
                <div class="play-bar-settings-row">
                    <span class="play-bar-settings-label">Fine</span>
                    <input type="range" class="play-bar-slider" id="ticks-per-frame"
                        min="0" max="100" step="0.1" value="50"
                        title="Simulation speed (ticks per animation frame)"
                        aria-label="Simulation speed">
                </div>
                <!-- Step-by-N buttons — step the simulation forward by a
                     specific tick count without starting playback. Useful for
                     careful frame-by-frame exploration. Each button fires
                     bridge.tick() the requested number of times synchronously. -->
                <div class="play-bar-settings-row">
                    <span class="play-bar-settings-label">Step</span>
                    <div class="play-bar-settings-options">
                        <button type="button" class="play-bar-settings-chip" data-step-by="1"   title="Advance 1 tick">+1</button>
                        <button type="button" class="play-bar-settings-chip" data-step-by="10"  title="Advance 10 ticks">+10</button>
                        <button type="button" class="play-bar-settings-chip" data-step-by="100" title="Advance 100 ticks">+100</button>
                    </div>
                </div>
            </div>
        </div>
    `;
    return el;
}
