export function getScale0ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scale0 scale0-only" id="lattice-controls">
            <label class="tb-label" for="scenario-select">Scenario</label>
            <select class="tb-select scale0-scenario-select" id="scenario-select" title="Select lattice scenario"></select>
            <details class="lat-scenario-desc scale0-scenario-meta" id="lat-scenario-desc" style="display:none">
                <summary>Epistemic status</summary>
                <pre id="lat-scenario-desc-text" class="lat-scenario-desc-text"></pre>
            </details>
        </div>
    `;
}

export function getScale0LatticeSizeToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scale0 scale0-only" id="lattice-size-group">
            <label class="tb-label" for="lattice-size">Size</label>
            <select class="tb-select" id="lattice-size" title="Lattice edge dimension (N x N x N) — odd so phenomena center on a true center voxel">
                <option value="9">9</option>
                <option value="17">17</option>
                <option value="25">25</option>
                <option value="33" selected>33</option>
                <option value="49">49</option>
                <option value="65">65</option>
                <option value="97">97</option>
                <option value="113">113</option>
                <option value="145">145</option>
                <option value="181">181</option>
            </select>
        </div>
    `;
}
