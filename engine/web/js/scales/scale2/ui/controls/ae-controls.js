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
      <input type="checkbox" id="ae-ionic" checked>
      <label for="ae-ionic"
        title="Coulomb force: F = -k·Q_i·Q_j / r² (attractive/repulsive between ions)">Ionic
        (Coulomb)</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-vdw" checked>
      <label for="ae-vdw"
        title="Lennard-Jones 12-6: steric repulsion at short range, dispersion attraction at long range">Van
        der Waals</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-bonds-force" checked>
      <label for="ae-bonds-force"
        title="Harmonic spring between bonded atoms: F = -k·(r - r_eq)">Bond Springs</label>
    </div>

    <div class="combo-section-label">Bonding &amp; stability</div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-bonding" checked>
      <label for="ae-bonding"
        title="Automatically form/break covalent bonds based on distance and valence">Auto-Bonding</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-damping">
      <label for="ae-damping"
        title="Velocity damping: v *= (1 - 0.02·dt) per tick (thermostat)">Damping</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-speed-limit" checked>
      <label for="ae-speed-limit"
        title="Cap atomic velocities at maximum speed (prevents numerical explosion)">Speed
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
        title="Hydrogen bonds: directional LJ 10-12 between electronegative atoms">H-Bonds</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-dipole">
      <label for="ae-dipole"
        title="Dipole-dipole interactions from partial charges">Dipole-Dipole</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-angle">
      <label for="ae-angle" title="Bond angle strain: VSEPR preferred angles">Angle
        Strain</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-thermostat">
      <label for="ae-thermostat"
        title="Berendsen thermostat for constant temperature MD">Thermostat</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="ae-electronegativity">
      <label for="ae-electronegativity"
        title="Electronegativity-driven bond polarity and formation threshold">Electronegativity</label>
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
