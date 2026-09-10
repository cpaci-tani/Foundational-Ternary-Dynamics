export function getScale5ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale5-only" id="cosmic-controls">
            <label class="tb-label" for="cosmic-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-medium" id="cosmic-scenario-select">
                <optgroup label="Astrophysics · Galaxies &amp; Halos">
                    <option value="cosmic-galaxy" selected>Spiral Galaxy</option>
                    <option value="cosmic-globular-cluster">Globular Cluster</option>
                    <option value="cosmic-dark-matter-halo">Dark Matter Halo</option>
                </optgroup>
                <optgroup label="Gravitation · Galaxy Interactions">
                    <option value="cosmic-cartwheel-collision">Cartwheel Collision</option>
                    <option value="cosmic-super-cluster">Supercluster Interaction</option>
                    <option value="cosmic-merger">Galaxy Merger</option>
                </optgroup>
                <optgroup label="Gravitation · Black Holes &amp; Waves">
                    <option value="cosmic-binary-agn">Binary Quasars</option>
                    <option value="cosmic-black-hole">Black Hole Close-up</option>
                    <option value="cosmic-ftd-collapse">FTD Collapse (Emergent BH)</option>
                    <option value="cosmic-gravitational-wave">Gravitational Wave (Binary)</option>
                </optgroup>
                <optgroup label="Astrophysics · Stellar Evolution">
                    <option value="cosmic-stellar-lifecycle">Stellar Lifecycle</option>
                </optgroup>
                <optgroup label="Cosmology · Structure &amp; Matter">
                    <option value="cosmic-web">Cosmic Web</option>
                    <option value="cosmic-baryogenesis">Baryogenesis</option>
                </optgroup>
                <optgroup label="Fluid Dynamics · Gas Models">
                    <option value="cosmic-gas-collapse" title="[IMPOSED effective gas dynamics] Monaghan SPH laboratory: a uniform gas ball under self-gravity + SPH pressure">Gas Lab · Collapse</option>
                    <option value="cosmic-gas-cloud-collision" title="[IMPOSED effective gas dynamics] Monaghan SPH laboratory: two gas clouds driven into a head-on collision">Gas Lab · Cloud Collision</option>
                    <option value="cosmic-gas-rotating-disk" title="[IMPOSED effective gas dynamics] Monaghan SPH laboratory: a thin rotating gas disk on Plummer circular orbits">Gas Lab · Rotating Disk</option>
                </optgroup>
            </select>
            <label class="tb-label" for="cosmic-camera-select">Camera</label>
            <select class="tb-select tb-select-scenario-compact" id="cosmic-camera-select">
                <option value="overview">Overview</option>
                <option value="galaxy" selected>Galaxy</option>
                <option value="blackhole">Black Hole</option>
                <option value="merger">Merger</option>
            </select>
            <label class="tb-toggle" title="[IMPOSED effective gas dynamics] Monaghan SPH for gas bodies (default off)"><input type="checkbox" id="t-sph-monaghan"> SPH gas</label>
        </div>
    `;
}

export function getScale5TelemetryToolbarTemplate() {
    return `
        <div class="tb-group tb-group-telemetry scale5-only" id="cosmic-telemetry">
            <span class="tb-value" id="cosmic-tb-bodies" title="Body count">--</span>
            <span class="tb-value" id="cosmic-tb-tick" title="Simulation tick">--</span>
            <span class="tb-value" id="cosmic-tb-hubble" title="Background Friedmann H(a); not N-body expansion">--</span>
            <span class="tb-value" id="cosmic-tb-scenario" title="Scenario-specific telemetry"></span>
        </div>
    `;
}
