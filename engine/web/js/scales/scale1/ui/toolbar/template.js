/**
 * Scale-1 toolbar template. The scenario <select> is populated from
 * ../../scenario-registry.js by the toolbar component (single source of
 * truth — no hardcoded option list). The epistemic-status details mirror
 * Scale 0's lat-scenario-desc pattern; the controller fills the text on
 * every scenario load.
 */

export function getScale1ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale1-only" id="pe-controls">
            <label class="tb-label" for="pe-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-wide" id="pe-scenario-select"
                title="Scale-1 scenarios on the native particle engine"></select>
            <details class="lat-scenario-desc scale0-scenario-meta" id="s1-scenario-desc" style="display:none">
                <summary>Epistemic status</summary>
                <pre id="s1-scenario-desc-text" class="lat-scenario-desc-text"></pre>
            </details>
        </div>
    `;
}
