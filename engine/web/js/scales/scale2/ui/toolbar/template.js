export function getScale2ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale2-only" id="ae-controls">
            <label class="tb-label" for="ae-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-wide" id="ae-scenario-select">
                <optgroup label="Single-Atom Physics">
                    <option value="ae-hydrogen-atom">Hydrogen Atom (p + e&#8315;)</option>
                    <option value="ae-rutherford-scattering">Rutherford Scattering</option>
                </optgroup>
                <optgroup label="Noble Gas Clusters">
                    <option value="ae-he-cluster">He Cluster (6 atoms, vdW)</option>
                    <option value="ae-ar-cluster">Ar Cluster (8 atoms, vdW)</option>
                    <option value="ae-noble-mix">Noble Mix (He + Ne + Ar)</option>
                </optgroup>
                <optgroup label="Ionic Formation">
                    <option value="ae-nacl-form">Na + Cl -&gt; NaCl</option>
                    <option value="ae-nacl-lattice">NaCl 3x3 Lattice</option>
                    <option value="ae-mgf2">Mg&#178;&#8314; + 2F&#8315; -&gt; MgF&#8322;</option>
                </optgroup>
                <optgroup label="Covalent Formation">
                    <option value="ae-h2-form">H + H -&gt; H&#8322;</option>
                    <option value="ae-o2-form">O + O -&gt; O&#8322;</option>
                    <option value="ae-ch4-form">C + 4H -&gt; CH&#8324;</option>
                </optgroup>
                <optgroup label="H-Bonding">
                    <option value="ae-water-dimer">Water Dimer (H-bond)</option>
                    <option value="ae-water-cluster">Water Pentamer</option>
                </optgroup>
                <optgroup label="VSEPR Geometry">
                    <option value="ae-vsepr-linear">CO&#8322; -&gt; Linear (180&deg;)</option>
                    <option value="ae-vsepr-tetrahedral">CH&#8324; -&gt; Tetrahedral (109.5&deg;)</option>
                    <option value="ae-vsepr-bent">H&#8322;O -&gt; Bent (104.5&deg;)</option>
                </optgroup>
                <optgroup label="Thermal Dynamics">
                    <option value="ae-thermal-gas">Ar Gas (12 atoms + thermostat)</option>
                    <option value="ae-collision">Head-On Collision</option>
                </optgroup>
                <optgroup label="Metallic Clusters">
                    <option value="ae-fe-bcc">Fe BCC Cluster (9 atoms)</option>
                    <option value="ae-cu-fcc">Cu FCC Seed (7 atoms)</option>
                </optgroup>
                <optgroup label="Special">
                    <option value="ae-periodic">Periodic Table (All 118)</option>
                    <option value="ae-custom">Custom (Manual)</option>
                </optgroup>
            </select>
            <label class="tb-check tb-check-inline" title="Show electron orbital probability clouds">
                <input type="checkbox" id="ae-show-clouds" checked>
                Orbitals
            </label>
        </div>
    `;
}
