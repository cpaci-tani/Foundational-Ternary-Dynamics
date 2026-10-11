/**
 * Play bar DOM template — a floating control strip at the bottom of the
 * viewport that hosts the primary playback controls.
 *
 * Layout (left to right):
 *   [play] [step] [reset] | [-] [speed] [+] | [1] [10] [100] | [zoom]
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
            <button class="tb-btn play-bar-small-btn play-bar-prime-btn is-on" id="btn-prime-tick"
                title="Prime one tick on load — populate field overlays at tick 0"
                aria-label="Prime tick on load" aria-pressed="true" hidden>&#9889;</button>
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

        <input type="range" class="play-bar-speed-input" id="ticks-per-frame"
            min="0" max="100" step="0.001" value="50"
            title="Simulation speed (ticks per animation frame)"
            aria-label="Simulation speed" tabindex="-1">

        <div class="play-bar-divider" aria-hidden="true"></div>

        <div class="play-bar-section play-bar-steps" aria-label="Advance by ticks">
            <button type="button" class="play-bar-step-count" data-step-by="1" title="Advance 1 tick">1</button>
            <button type="button" class="play-bar-step-count" data-step-by="10" title="Advance 10 ticks">10</button>
            <button type="button" class="play-bar-step-count" data-step-by="100" title="Advance 100 ticks">100</button>
        </div>

        <div class="play-bar-divider" aria-hidden="true"></div>

        <div class="play-bar-section play-bar-zoom" aria-label="Camera zoom">
            <button type="button" class="play-bar-icon-btn play-bar-zoom-btn" id="play-bar-zoom-btn"
                title="Zoom level" aria-label="Zoom level" aria-haspopup="listbox" aria-expanded="false"
                aria-controls="play-bar-zoom-menu">
                <svg viewBox="0 0 24 24" width="18" height="18" aria-hidden="true">
                    <path d="M4 8V5h3M17 5h3v3M20 16v3h-3M7 19H4v-3" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round"/>
                    <path d="M9 12h6M12 9v6" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round"/>
                </svg>
            </button>
            <div class="play-bar-zoom-menu" id="play-bar-zoom-menu" role="listbox" aria-label="Zoom level" hidden>
                <button type="button" role="option" data-framed-view="moore" data-travel>Moore neighborhood</button>
                <button type="button" role="option" data-framed-view="neighborhood" data-travel>Neighborhood zoomed out</button>
                <button type="button" role="option" data-framed-view="detail" data-travel>Detailed lattice</button>
                <button type="button" role="option" data-framed-view="lattice" data-travel>Full lattice</button>
                <button type="button" role="option" data-framed-view="lattice-out" data-travel>Zoomed out lattice</button>
                <button type="button" role="option" data-framed-view="quasi" data-travel>Zoomed out quasi-domain</button>
                <div class="play-bar-zoom-note" role="presentation">Beyond the lattice: reference lengths, nothing simulated</div>
                <button type="button" role="option" data-framed-view="lhc" data-travel>LHC resolution</button>
                <button type="button" role="option" data-framed-view="electroweak" data-travel>W, Z, Higgs, top</button>
                <button type="button" role="option" data-framed-view="nuclear" data-travel>Nuclear scale</button>
                <button type="button" role="option" data-framed-view="electron" data-travel>Electron</button>
            </div>
        </div>
    `;
    return el;
}
