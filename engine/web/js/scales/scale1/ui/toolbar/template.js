export function getScale1ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale1-only" id="pe-controls">
            <label class="tb-label" for="pe-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-wide" id="pe-scenario-select">
                <optgroup label="Leptons">
                    <option value="pe-hydrogen" selected>Hydrogen Atom (p + e&#8315;)</option>
                    <option value="pe-helium">Helium Atom (He&#178;&#8314; + 2e&#8315;)</option>
                    <option value="pe-positronium">Positronium (e&#8314;e&#8315;)</option>
                    <option value="pe-muonium">Muonium (&mu;&#8314;e&#8315;)</option>
                    <option value="pe-true-muonium">True Muonium (&mu;&#8314;&mu;&#8315;)</option>
                    <option value="pe-tauonium">Tauonium (&tau;&#8314;&tau;&#8315;)</option>
                    <option value="pe-tau-atom">Tauonic Hydrogen (&tau;&#8315; + p)</option>
                </optgroup>
                <optgroup label="Exotic Atoms">
                    <option value="pe-pionic-hydrogen">Pionic Hydrogen (&pi;&#8315; + p)</option>
                    <option value="pe-kaonic-hydrogen">Kaonic Hydrogen (K&#8315; + p)</option>
                    <option value="pe-sigma-plus-atom">Sigma&#8314; Atom (&Sigma;&#8314; + e&#8315;)</option>
                    <option value="pe-antiprotonic-hydrogen">Protonium (p&#772; + p)</option>
                </optgroup>
                <optgroup label="Hadrons">
                    <option value="pe-pion-orbit">Pionium (&pi;&#8314;&pi;&#8315;)</option>
                    <option value="pe-kaon-pair">Kaonium (K&#8314;K&#8315;)</option>
                    <option value="pe-delta-system">Delta&#8314;&#8314; System (&Delta;&#8314;&#8314; + 2e&#8315;)</option>
                    <option value="pe-omega-scattering">Omega&#8315; Scattering (&Omega;&#8315; + e&#8314;)</option>
                </optgroup>
                <optgroup label="Nuclear">
                    <option value="pe-deuteron">Deuteron (p + n + e&#8315;)</option>
                    <option value="pe-tritium">Tritium (p + 2n + e&#8315;)</option>
                    <option value="pe-helion">Helion / He-3 (2p + n + 2e&#8315;)</option>
                </optgroup>
                <optgroup label="Bosons">
                    <option value="pe-w-pair">W&#8314;W&#8315; Pair</option>
                </optgroup>
                <optgroup label="Scattering">
                    <option value="pe-scattering">Proton-Electron Scattering</option>
                    <option value="pe-three-body">Three-Body (p&#8314; p&#8314; e&#8315;)</option>
                    <option value="pe-meson-scattering">&pi;&#8314; off Proton</option>
                    <option value="pe-muon-scattering">&mu;&#8315; off Proton</option>
                </optgroup>
                <optgroup label="Gravity">
                    <option value="pe-micro-bh">Micro Black Hole (Accretion)</option>
                </optgroup>
                <optgroup label="Custom">
                    <option value="pe-custom">Custom (Manual)</option>
                </optgroup>
            </select>
        </div>
    `;
}
