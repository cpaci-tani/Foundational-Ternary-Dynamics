export function getScale4ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale4-only" id="planetary-controls">
            <label class="tb-label" for="planetary-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-medium" id="planetary-scenario-select">
                <optgroup label="Standard Models">
                    <option value="planetary-solar" selected>Solar System</option>
                    <option value="planetary-binary">Binary Star System</option>
                    <option value="planetary-threebody">Three-Body Problem</option>
                </optgroup>
                <optgroup label="NASA Exoplanet Archive Data">
                    <option value="exo-TRAPPIST-1">TRAPPIST-1 System</option>
                    <option value="exo-Kepler-90">Kepler-90 System</option>
                    <option value="exo-Kepler-11">Kepler-11 System</option>
                    <option value="exo-HR 8799">HR 8799 System</option>
                    <option value="exo-Kepler-20">Kepler-20 System</option>
                </optgroup>
            </select>
            <label class="tb-label" for="planetary-gravity-mode" title="Decorative: slow visual cadence (G=0.01, lattice-natural). Physical: Keplerian AU/M_sun/yr timing (G=4π², Earth year = 1 sim year, ~63× faster).">Gravity</label>
            <select class="tb-select" id="planetary-gravity-mode">
                <option value="decorative" selected>Decorative (slow)</option>
                <option value="physical">Physical (Kepler AU/yr)</option>
            </select>
        </div>
    `;
}
