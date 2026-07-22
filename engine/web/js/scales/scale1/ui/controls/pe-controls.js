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
    <div class="card-title" title="Controls for the Particle Engine simulation, including pairwise forces, dynamics settings, and integration parameters.">Particle Engine Controls</div>

    <div class="combo-section-label" title="Configure pairwise force terms for the particle interactions.">Forces</div>
    <div class="toggle-row">
      <input type="checkbox" id="pe-coulomb" checked>
      <label for="pe-coulomb"
        title="Coulomb: F = -alpha * q_i * q_j / (4*pi*r^2) (EM pairwise)">Coulomb</label>
    </div>
    <div class="toggle-row">
      <input type="checkbox" id="pe-gravity">
      <label for="pe-gravity"
        title="Gravity: F = +G_PE * m_i * m_j / r^2 (always attractive). G_PE = 1/(4pi*m_P^2) is the FTD-0131 physical coupling (alpha_G(e,e) = (m_e/m_P)^2 ~ 1.75e-45). Net dynamics are negligible next to Coulomb; read Gravity PE in diagnostics/charts.">Gravity</label>
    </div>

    <div class="combo-section-label" title="Configure motion-governing rules like velocity damping.">Dynamics</div>
    <div class="toggle-row">
      <input type="checkbox" id="pe-damping">
      <label for="pe-damping"
        title="Velocity damping: v *= (1 - DAMPING*dt) per tick">Damping</label>
    </div>

    <!--
      "Advanced Forces (Phase 2)" toggles HIDDEN per audit §B/§E (2026-05-27),
      EXCEPT pe-exchange (re-enabled 2026-07-14): the audit's "written but
      never consumed" finding does not hold for exchange — pe-force-kernel.js
      (created 2026-06-15, after the audit) fully consumes toggles.exchange
      in the pairwise repulsion term (fires only when spin AND charge match,
      i.e. genuine Pauli-exclusion behavior), feeding the real Velocity-Verlet
      integrator. The remaining 6 toggles (pe-lorentz-p, pe-strong,
      pe-magnetic-dipole, pe-spin-orbit, pe-radiation, pe-relativistic) are
      wired the same way at the DOM/app.js level but have NOT been
      individually re-verified against the current force kernel — stay
      hidden until each is checked the same way exchange was.
    -->
    <details class="toggle-details">
      <summary class="ctrl-details-summary">Advanced Forces</summary>
      <div class="toggle-row">
        <input type="checkbox" id="pe-exchange">
        <label for="pe-exchange"
          title="Exchange / Pauli exclusion: short-range repulsion between particles sharing both spin and charge (fMag = ALPHA_EXCHANGE * exp(-r^2/EXCHANGE_RANGE_SQ) / r^2)">Exchange</label>
      </div>
    </details>
    <!--
    <details class="toggle-details">
      <summary class="ctrl-details-summary">Advanced Forces (Phase 2)</summary>
      <div class="toggle-row">
        <input type="checkbox" id="pe-lorentz-p">
        <label for="pe-lorentz-p"
          title="Lorentz force: v × B from magnetic dipoles">Lorentz</label>
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
          title="Relativistic correction (WASM/native only): crude isotropic F·(1/γ−1) rescale, NOT covariant — use Relativistic-Verlet for momentum-correct dynamics">Relativistic</label>
      </div>
    </details>
    -->

    <div class="combo-section-label" title="Adjust solver precision and numerical properties.">Parameters</div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label" title="Integration step dt (ticks) for the Velocity-Verlet update. Larger values run faster but can introduce energy drift.">Time Step</span>
      <input type="range" class="pe-slider" id="pe-dt-slider" min="0.1" max="2.0" step="0.1"
        value="1.0" title="Integration step dt (ticks) for the Velocity-Verlet update. Larger values run faster but can introduce energy drift.">
      <span class="pe-ctrl-value" id="pe-dt-value">1.0</span>
    </div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label" title="Plummer softening length (lu) added in quadrature to pair separations. Prevents infinite singular forces when particles are extremely close.">Softening</span>
      <input type="range" class="pe-slider" id="pe-soft-slider" min="0.01" max="1.0" step="0.01"
        value="0.10" title="Plummer softening length (lu) added in quadrature to pair separations. Prevents infinite singular forces when particles are extremely close.">
      <span class="pe-ctrl-value" id="pe-soft-value">0.10</span>
    </div>
    <div class="ctrl-action-row">
      <button class="ctrl-btn-secondary" id="btn-pe-clear" title="Clear all active particles, reset simulation time, and reload the selected scenario.">Clear &amp; Reload</button>
    </div>
  `;
  return card;
}
