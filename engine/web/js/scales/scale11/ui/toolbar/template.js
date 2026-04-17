export function getScale11ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario tb-group-scenario-rich scale11-only" id="cs-controls">
            <label class="tb-label" for="cs-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-medium" id="cs-scenario-select">
                <optgroup label="Observer Modes">
                    <option value="cs-threshold" selected>Threshold Crossing</option>
                    <option value="cs-high-coupling">High Coupling</option>
                </optgroup>
                <optgroup label="sLoop">
                    <option value="cs-self-ref">Self-Reference</option>
                    <option value="cs-nested-sloop">Nested sLoop</option>
                </optgroup>
                <optgroup label="Duality">
                    <option value="cs-chirality">Chirality Split</option>
                    <option value="cs-boundary-orbit">Boundary Orbit</option>
                    <option value="cs-entangled">Entangled Pair</option>
                </optgroup>
                <optgroup label="Phase States">
                    <option value="cs-flow">Flow State</option>
                    <option value="cs-meditation">Meditation</option>
                </optgroup>
                <optgroup label="Custom">
                    <option value="cs-custom">Custom (Manual)</option>
                </optgroup>
            </select>
            <details class="cs-scenario-desc tb-detail-inline" id="cs-scenario-desc">
                <summary>What does this scenario demonstrate?</summary>
                <p id="cs-scenario-desc-text">Select a scenario to see its description.</p>
            </details>
            <select class="tb-select tb-select-scenario-compact" id="cs-figure-select">
                <optgroup label="Ethereal">
                    <option value="aetheric">Aetheric Body</option>
                    <option value="spirit">Spirit in the Sky</option>
                    <option value="plasmoid" selected>Plasmoid Cloud</option>
                </optgroup>
                <optgroup label="Divine">
                    <option value="god">God Figure</option>
                    <option value="demiurge">Demiurge</option>
                    <option value="yahweh">Yahweh (Pillar)</option>
                </optgroup>
                <optgroup label="Historical">
                    <option value="hotep">Hotep</option>
                    <option value="jesus">Jesus</option>
                    <option value="mayan-sun">Mayan Sun God</option>
                </optgroup>
                <optgroup label="Dark">
                    <option value="death-cloud">Death Cloud</option>
                </optgroup>
                <optgroup label="Classic">
                    <option value="humanoid">Humanoid</option>
                    <option value="alien">Alien</option>
                </optgroup>
            </select>
            <label class="tb-check tb-check-inline" title="Enable consciousness audio synthesis">
                <input type="checkbox" id="cs-audio">
                Audio
            </label>
        </div>
    `;
}
