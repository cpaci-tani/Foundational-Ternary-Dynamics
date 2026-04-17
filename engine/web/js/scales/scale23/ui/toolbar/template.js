export function getScale23VisualToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scale23 scale23-only" id="ae-visual-controls">
            <label class="tb-check" title="Show strong force glow shells around nuclei">
                <input type="checkbox" id="ae-show-shells" checked>
                Shells
            </label>
            <label class="tb-check" title="Show shell boundary spheres (n=1,2,3...)">
                <input type="checkbox" id="ae-show-shell-bounds">
                Bounds
            </label>
            <label class="tb-check" title="Show p/d/f orbital lobe shapes">
                <input type="checkbox" id="ae-show-lobes">
                Lobes
            </label>
            <span class="tb-label tb-label-inline">Bonds</span>
            <select class="tb-select tb-select-scenario-compact" id="bond-style-select">
                <option value="cylinders" selected>Thick</option>
                <option value="lines">Thin</option>
                <option value="off">Off</option>
            </select>
        </div>
    `;
}

export function getScale23ForceToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scale23 tb-group-force scale23-only" id="ae-force-controls">
            <span class="tb-label">Forces</span>
            <button class="view-toggle" id="ae-force-ionic" title="Coulomb (ionic) forces">F<sub>C</sub></button>
            <button class="view-toggle" id="ae-force-vdw" title="Van der Waals forces">F<sub>vdW</sub></button>
            <button class="view-toggle" id="ae-force-bond" title="Bond spring forces">F<sub>B</sub></button>
            <button class="view-toggle" id="ae-force-net" title="Net force">F<sub>net</sub></button>
        </div>
    `;
}
