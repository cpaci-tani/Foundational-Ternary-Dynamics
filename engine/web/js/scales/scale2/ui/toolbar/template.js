export function getScale2ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale2-only" id="ae-controls">
            <label class="tb-label" for="ae-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-wide scale2-scenario-select" id="ae-scenario-select"
                title="Scale 2 atom-engine scenario"></select>
            <details class="ae-scenario-desc scale2-scenario-meta" id="ae-scenario-desc" style="display:none">
                <summary>About this scenario</summary>
                <pre id="ae-scenario-desc-text" class="ae-scenario-desc-text"></pre>
            </details>
        </div>
    `;
}
