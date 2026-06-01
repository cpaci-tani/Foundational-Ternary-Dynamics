/**
 * Scale 1 — Particle Engine Controls Card
 *
 * Factory function that returns the "Particle Engine Controls" card DOM element.
 * Markup extracted from index.html lines 433–511.
 */

export function createPeControlsCard() {
  const card = document.createElement('div');
  card.className = 'card scale1-only';
  card.innerHTML = `
    <div class="card-title">Particle Engine Controls</div>

    <div class="combo-section-label">Forces</div>
    <div class="toggle-row">
      <input type="checkbox" id="pe-coulomb" checked>
      <label for="pe-coulomb"
        title="Coulomb: F = -alpha * q_i * q_j / (4*pi*r^2) (EM pairwise)">Coulomb</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="pe-gravity" checked>
      <label for="pe-gravity"
        title="Gravity: F = +G_N * m_i * m_j / r^2 (always attractive)">Gravity</label>
    </div>

    <div class="combo-section-label">Dynamics</div>
    <div class="toggle-row">
      <input type="checkbox" id="pe-damping" checked>
      <label for="pe-damping"
        title="Velocity damping: v *= (1 - DAMPING*dt) per tick">Damping</label>
    </div>

    <!--
      "Advanced Forces (Phase 2)" toggles HIDDEN per audit §B/§E (2026-05-27).
      The 7 toggles (pe-lorentz-p, pe-exchange, pe-strong, pe-magnetic-dipole,
      pe-spin-orbit, pe-radiation, pe-relativistic) ARE wired: app.js:1081-1087
      adds change-listeners → mock-bridge.js → mock-particle-engine.js setters
      store the flags on state._pe (e.g. state._pe.lorentz). HOWEVER the mock
      particle-engine force step (_computeForces / Velocity-Verlet integrator)
      reads ONLY state._pe.coulomb, state._pe.gravity and state._pe.damping —
      the 7 advanced flags are written but never consumed, so the toggles are
      no-ops in the shipped JS engine. They were advertising non-functional
      physics. The WASM bridge forwards them to a real _peToggle, so the markup
      is preserved (commented, not deleted) for re-enable if/when the JS engine
      grows the corresponding force terms or a WASM PE is wired by default.
    -->
    <!--
    <details class="toggle-details">
      <summary class="ctrl-details-summary">Advanced Forces (Phase 2)</summary>
      <div class="toggle-row">
        <input type="checkbox" id="pe-lorentz-p">
        <label for="pe-lorentz-p"
          title="Lorentz force: v × B from magnetic dipoles">Lorentz</label>
      </div>
      <div class="toggle-row">
        <input type="checkbox" id="pe-exchange">
        <label for="pe-exchange"
          title="Exchange / Pauli exclusion (same spin + charge)">Exchange</label>
      </div>
      <div class="toggle-row">
        <input type="checkbox" id="pe-strong">
        <label for="pe-strong"
          title="Strong force: Yukawa + linear confinement (color-dependent)">Strong</label>
      </div>
      <div class="toggle-row">
        <input type="checkbox" id="pe-magnetic-dipole">
        <label for="pe-magnetic-dipole"
          title="Magnetic dipole-dipole interaction (spin_axis)">Mag. Dipole</label>
      </div>
      <div class="toggle-row">
        <input type="checkbox" id="pe-spin-orbit">
        <label for="pe-spin-orbit" title="Spin-orbit coupling: L·S splitting">Spin-Orbit</label>
      </div>
      <div class="toggle-row">
        <input type="checkbox" id="pe-radiation">
        <label for="pe-radiation"
          title="Radiation reaction / Abraham-Lorentz damping">Radiation</label>
      </div>
      <div class="toggle-row">
        <input type="checkbox" id="pe-relativistic">
        <label for="pe-relativistic"
          title="Relativistic mass correction: F/gamma">Relativistic</label>
      </div>
    </details>
    -->

    <div class="combo-section-label">Parameters</div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label">Time Step</span>
      <input type="range" class="pe-slider" id="pe-dt-slider" min="0.1" max="2.0" step="0.1"
        value="1.0">
      <span class="pe-ctrl-value" id="pe-dt-value">1.0</span>
    </div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label">Softening</span>
      <input type="range" class="pe-slider" id="pe-soft-slider" min="0.01" max="1.0" step="0.01"
        value="0.10">
      <span class="pe-ctrl-value" id="pe-soft-value">0.10</span>
    </div>
    <div class="ctrl-action-row">
      <button class="ctrl-btn-secondary" id="btn-pe-clear">Clear &amp; Reload</button>
    </div>
  `;
  return card;
}
