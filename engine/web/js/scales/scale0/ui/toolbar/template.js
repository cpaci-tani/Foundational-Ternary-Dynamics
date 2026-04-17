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
            <select class="tb-select" id="lattice-size" title="Lattice edge dimension (N x N x N)">
                <option value="8">8</option>
                <option value="16">16</option>
                <option value="24">24</option>
                <option value="32" selected>32</option>
                <option value="48">48</option>
                <option value="64">64</option>
                <option value="96">96</option>
                <option value="128">128</option>
            </select>
        </div>
    `;
}
