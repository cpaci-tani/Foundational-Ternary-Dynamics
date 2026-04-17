export function getScale5ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale5-only" id="cosmic-controls">
            <label class="tb-label" for="cosmic-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-medium" id="cosmic-scenario-select">
                <option value="cosmic-galaxy" selected>Spiral Galaxy</option>
                <option value="cosmic-cartwheel-collision">Cartwheel Collision</option>
                <option value="cosmic-super-cluster">Supercluster Interaction</option>
                <option value="cosmic-merger">Galaxy Merger</option>
                <option value="cosmic-binary-agn">Binary Quasars</option>
                <option value="cosmic-globular-cluster">Globular Cluster</option>
                <option value="cosmic-black-hole">Black Hole Close-up</option>
                <option value="cosmic-ftd-collapse">FTD Collapse (Emergent BH)</option>
                <option value="cosmic-stellar-lifecycle">Stellar Lifecycle</option>
                <option value="cosmic-web">Cosmic Web</option>
                <option value="cosmic-dark-matter-halo">Dark Matter Halo</option>
                <option value="cosmic-gravitational-wave">Gravitational Wave (Binary)</option>
                <option value="cosmic-baryogenesis">Baryogenesis</option>
            </select>
            <label class="tb-label" for="cosmic-camera-select">Camera</label>
            <select class="tb-select tb-select-scenario-compact" id="cosmic-camera-select">
                <option value="overview">Overview</option>
                <option value="galaxy" selected>Galaxy</option>
                <option value="blackhole">Black Hole</option>
                <option value="merger">Merger</option>
            </select>
        </div>
    `;
}

export function getScale5TelemetryToolbarTemplate() {
    return `
        <div class="tb-group tb-group-telemetry scale5-only" id="cosmic-telemetry">
            <span class="tb-value" id="cosmic-tb-bodies" title="Body count">--</span>
            <span class="tb-value" id="cosmic-tb-tick" title="Simulation tick">--</span>
            <span class="tb-value" id="cosmic-tb-hubble" title="Hubble parameter H(t)">--</span>
        </div>
    `;
}
