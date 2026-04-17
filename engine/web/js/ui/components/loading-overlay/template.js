export function getLoadingOverlayTemplate(version = 'v2.11') {
    return `
        <div class="load-lattice">
            <canvas id="load-cube" width="512" height="512"></canvas>
        </div>
        <div class="load-copy">
            <div class="load-logo" aria-hidden="true">
                <span class="letter">F</span>
                <span class="letter">T</span>
                <span class="letter">D</span>
                <span class="dot"></span>
            </div>
            <div class="load-subtitle">Foundational Ternary Dynamics</div>
            <div class="load-progress-wrap">
                <div class="load-bar-bg">
                    <div class="load-bar-fill" id="load-bar"></div>
                </div>
                <div class="load-status" id="load-status"></div>
            </div>
            <div class="load-version">${version}</div>
        </div>
    `;
}
