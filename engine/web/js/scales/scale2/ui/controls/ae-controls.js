/**
 * Scale 2 — Atom Engine control cards (split by concern).
 */

export function createAeForcesCard() {
  const card = document.createElement('div');
  card.className = 'card scale-ae';
  card.innerHTML = `
    <div class="card-title">Interaction Forces</div>

    <div class="combo-section-label">Pair potentials</div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-ionic">
      <label for="ae-ionic"
        title="[PARAMETRIC] Softened Coulomb pair force between the atoms' current effective charges.">Ionic
        (Coulomb)</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-vdw">
      <label for="ae-vdw"
        title="[PARAMETRIC] Lennard-Jones 12-6 pair potential for effective steric repulsion and dispersion attraction.">Van
        der Waals</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-bonds-force">
      <label for="ae-bonds-force"
        title="[PARAMETRIC] Harmonic radial force between explicitly bonded atoms, paired with tracked bond potential energy.">Bond Springs</label>
    </div>

    <div class="combo-section-label">Bonding &amp; stability</div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-bonding">
      <label for="ae-bonding"
        title="[IMPOSED] Effective distance, valence, and energy rules create and break the graph of covalent bonds.">Auto-Bonding</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-damping">
      <label for="ae-damping"
        title="[IMPOSED] Uniform velocity damping removes kinetic energy each tick; this is a dissipative sink, not a thermostat.">Damping</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-speed-limit" checked>
      <label for="ae-speed-limit"
        title="[IMPOSED] Caps atomic speeds to keep the effective integrator inside its finite stability domain.">Speed
        Limit</label>
    </div>
  `;
  return card;
}

export function createAeAdvancedCard() {
  const card = document.createElement('div');
  card.className = 'card scale-ae';
  card.innerHTML = `
    <div class="card-title">Phase 3 Forces</div>
    <p class="panel-resource-muted-copy" style="margin: 0 0 8px 0; font-size: var(--fs-xs);">
      Directional and ensemble terms — enable per scenario or for custom runs.
    </p>
    <div class="toggle-row">
      <input type="checkbox" id="ae-hbonds">
      <label for="ae-hbonds"
        title="[PARAMETRIC] Effective radial 10-12 hydrogen-bond force for eligible donor and acceptor atoms; its potential energy is not yet included in total energy.">H-Bonds</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-dipole">
      <label for="ae-dipole"
        title="[PARAMETRIC] Effective pair force from electronegativity-derived atomic dipole estimates; its potential energy is not yet included in total energy.">Dipole-Dipole</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-angle">
      <label for="ae-angle" title="[PARAMETRIC] Harmonic bond-angle force around effective VSEPR targets, paired with tracked angle potential energy.">Angle
        Strain</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-thermostat">
      <label for="ae-thermostat"
        title="[IMPOSED] Weak Berendsen-style velocity rescaling toward a target temperature; it does not generate a canonical ensemble.">Thermostat</label>
    </div>
    <label class="pe-ctrl-row" title="[IMPOSED] Berendsen target in AtomEngine reduced temperature units; this is not Kelvin.">
      <span class="pe-ctrl-label">Thermostat target</span>
      <input type="range" class="pe-slider" id="ae-thermostat-slider" min="0.01" max="3" step="0.01" value="1">
      <span class="pe-ctrl-value" id="ae-thermostat-value">1.00</span>
    </label>
    <div class="toggle-row">
      <input type="checkbox" id="ae-electronegativity">
      <label for="ae-electronegativity"
        title="[EMPIRICAL] QEq-like charge transfer from electronegativity differences; it also modifies the effective bond-capture threshold.">Electronegativity</label>
    </div>
  `;
  return card;
}

export function createAeIntegratorCard() {
  const card = document.createElement('div');
  card.className = 'card scale-ae';
  card.innerHTML = `
    <div class="card-title">Integrator</div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label"
        title="Velocity Verlet integration time step (smaller = more accurate, slower)">Time
        Step</span>
      <input type="range" class="pe-slider" id="ae-dt-slider" min="0.01" max="0.5" step="0.01"
        value="0.10">
      <span class="pe-ctrl-value" id="ae-dt-value">0.10</span>
    </div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label"
        title="Force softening length: prevents singularity at r→0">Softening</span>
      <input type="range" class="pe-slider" id="ae-soft-slider" min="0.1" max="2.0" step="0.1"
        value="0.3">
      <span class="pe-ctrl-value" id="ae-soft-value">0.30</span>
    </div>
    <div class="ctrl-action-row">
      <button class="ctrl-btn-secondary" id="btn-ae-clear">Clear &amp; Reload</button>
    </div>
  `;
  return card;
}

export function createAeNuclearLaboratoryCard() {
  const card = document.createElement('div');
  card.className = 'card scale-ae scale2-only';
  card.innerHTML = `
    <div class="card-title">Nuclear Transport Laboratory</div>
    <p class="panel-resource-muted-copy" style="margin: 0 0 8px 0; font-size: var(--fs-xs);">
      Live finite-particle transport. Scenarios only seed the initial state; these controls remain active afterward.
    </p>
    <label class="pe-ctrl-row" title="Select the [PARAMETRIC] effective reaction channel. Changing channel clears only the nuclear event ledger; it does not reload the scenario.">
      <span class="pe-ctrl-label">Channel</span>
      <select id="ae-nuclear-channel">
        <option value="">Off</option>
        <option value="dt_fusion">D-T fusion</option>
        <option value="u235_fission">U-235 fission</option>
      </select>
    </label>
    <label class="pe-ctrl-row" title="Explicit unitless multiplier on the normalized energy-dependent reaction hazard. Zero disables nuclear reactions without hiding particles.">
      <span class="pe-ctrl-label">Reactivity</span>
      <input type="range" class="pe-slider" id="ae-nuclear-reactivity" min="0" max="20" step="0.1" value="1">
      <span class="pe-ctrl-value" id="ae-nuclear-reactivity-value">1.0</span>
    </label>
    <label class="pe-ctrl-row" title="Scales the finite live-collision radius. This is a browser laboratory coefficient, not a cross section in barns.">
      <span class="pe-ctrl-label">Collision radius</span>
      <input type="range" class="pe-slider" id="ae-nuclear-collision-radius" min="0.25" max="4" step="0.05" value="1">
      <span class="pe-ctrl-value" id="ae-nuclear-collision-radius-value">1.00×</span>
    </label>
    <label class="pe-ctrl-row" title="Radius of the computed neutron-transport region in Scale 2 length units.">
      <span class="pe-ctrl-label">Transport radius</span>
      <input type="range" class="pe-slider" id="ae-nuclear-transport-radius" min="2" max="40" step="1" value="18">
      <span class="pe-ctrl-value" id="ae-nuclear-transport-radius-value">18 lu</span>
    </label>
    <label class="pe-ctrl-row" title="Leak removes neutrons that leave the computed transport volume. Reflect reverses their outward radial velocity at the boundary.">
      <span class="pe-ctrl-label">Boundary</span>
      <select id="ae-nuclear-boundary">
        <option value="leak">Open / leak</option>
        <option value="reflect">Reflective</option>
      </select>
    </label>
    <label class="pe-ctrl-row" title="[PARAMETRIC] Ambient one-group scattering probability and energy loss. It changes live neutron trajectories and spectra.">
      <span class="pe-ctrl-label">Moderator</span>
      <input type="range" class="pe-slider" id="ae-nuclear-moderator" min="0" max="1" step="0.01" value="0">
      <span class="pe-ctrl-value" id="ae-nuclear-moderator-value">0.00</span>
    </label>
    <label class="pe-ctrl-row" title="[PARAMETRIC] Ambient capture probability. Captured neutrons are removed and counted in the measured loss ledger.">
      <span class="pe-ctrl-label">Absorber</span>
      <input type="range" class="pe-slider" id="ae-nuclear-absorber" min="0" max="1" step="0.01" value="0">
      <span class="pe-ctrl-value" id="ae-nuclear-absorber-value">0.00</span>
    </label>
    <label class="pe-ctrl-row" title="Continuous source intensity in injected channel-reactant sets per simulation tick.">
      <span class="pe-ctrl-label">Source rate</span>
      <input type="range" class="pe-slider" id="ae-nuclear-source-rate" min="0" max="2" step="0.01" value="0">
      <span class="pe-ctrl-value" id="ae-nuclear-source-rate-value">0.00/tick</span>
    </label>
    <label class="pe-ctrl-row" title="Incident neutron energy for manual and continuous U-235 source injection.">
      <span class="pe-ctrl-label">Neutron energy</span>
      <select id="ae-nuclear-source-energy">
        <option value="2.53e-8">Thermal · 0.0253 eV</option>
        <option value="1e-3">Epithermal · 1 keV</option>
        <option value="1">Fast · 1 MeV</option>
        <option value="2">Fast · 2 MeV</option>
      </select>
    </label>
    <div class="toggle-row">
      <input type="checkbox" id="ae-nuclear-source-enabled">
      <label for="ae-nuclear-source-enabled" title="Continuously inject the selected channel's reactants at the configured source rate.">Continuous source</label>
    </div>
    <div class="ctrl-action-row">
      <button class="ctrl-btn-secondary" id="btn-ae-inject-neutron" title="Inject one neutron from the negative-X edge of the transport volume toward its center.">+ neutron</button>
      <button class="ctrl-btn-secondary" id="btn-ae-inject-dt" title="Inject one approaching D-T pair with the channel's 20 keV relative energy.">+ D-T pair</button>
      <button class="ctrl-btn-secondary" id="btn-ae-inject-u235" title="Place one stationary U-235 fuel record at a deterministic free position.">+ U-235</button>
    </div>
  `;
  return card;
}
