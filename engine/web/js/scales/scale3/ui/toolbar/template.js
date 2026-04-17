export function getScale3ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale3-only" id="mol-controls">
            <label class="tb-label" for="mol-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-wide" id="mol-scenario-select">
                <optgroup label="Special">
                    <option value="mol-crystal">NaCl Crystal</option>
                    <option value="mol-custom">Custom (Manual)</option>
                </optgroup>
            </select>
            <label class="tb-check tb-check-inline" title="Show electron orbital probability clouds">
                <input type="checkbox" id="mol-show-clouds" checked>
                Orbitals
            </label>
        </div>
    `;
}
