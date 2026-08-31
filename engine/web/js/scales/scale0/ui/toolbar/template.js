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
                <option value="113" data-native-only disabled>113 · Native GPU</option>
                <option value="145" data-native-only disabled>145 · Native GPU</option>
                <option value="181" data-native-only disabled>181 · Native GPU</option>
            </select>
            <label class="tb-label" for="flux-boundary-mode" style="margin-left:6px">Boundary</label>
            <select class="tb-select" id="flux-boundary-mode" title="Complete six-face transported-dynamics boundary: Dispersal removes manifested face records and supplies an outward-only field trace without damping the strict interior; Reflective uses a Neumann field mirror and elastic particle bounce; Periodic identifies every pair of opposite faces.">
                <option value="2" selected>Dispersal</option>
                <option value="1">Reflective</option>
                <option value="0">Periodic</option>
            </select>
            <span class="boundary-axis-control">
                <label class="tb-label" for="flux-periodic-axis">Orientation</label>
                <select class="tb-select" id="flux-periodic-axis" title="Orient the simulation axes without changing boundary coverage. X is lateral, Y is vertical, Z is forward/aft; XYZ highlights all directions.">
                    <option value="2" selected>Z · forward/aft</option>
                    <option value="0">X · lateral</option>
                    <option value="1">Y · vertical</option>
                    <option value="3">XYZ · all axes</option>
                </select>
            </span>
            <output id="global-clock-readout" class="global-clock-readout" title="Global ordinal tick [AXIOM]. Color reports the engine's selected/imposed mapped causal-rate budget when current telemetry is available; it is not recovered spacetime.">tick 0</output>
        </div>
    `;
}
