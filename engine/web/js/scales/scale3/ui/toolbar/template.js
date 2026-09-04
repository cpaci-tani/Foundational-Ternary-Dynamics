export function getScale3ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale3-only" id="mol-controls">
            <label class="tb-label" for="mol-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-wide" id="mol-scenario-select"
                title="Scale 3 molecule-engine scenario"></select>
            <details class="ae-scenario-desc scale3-scenario-meta" id="mol-scenario-desc" style="display:none">
                <summary>About this scenario</summary>
                <pre id="mol-scenario-desc-text" class="ae-scenario-desc-text"></pre>
            </details>
        </div>
    `;
}
