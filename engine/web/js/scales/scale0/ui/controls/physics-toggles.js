/**
 * Scale 0 Physics Toggles Card
 */

export function createPhysicsTogglesCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  card.innerHTML = `
    <div class="card-title">Physics Toggles</div>
    <div class="combo-section-label">Wave & Field</div>
    <div class="toggle-row"><input type="checkbox" id="t-wave" checked><label for="t-wave" title="Flux wave equation: J evolves via discrete Laplacian (c^2 nabla^2 J)">Wave Propagation</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-coupling" checked><label for="t-coupling" title="Manifested particles source flux via coupling term g_c * grad(s)">State-Flux Coupling</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-damping" checked><label for="t-damping" title="Exponential flux decay at rate alpha per tick (energy dissipation)">Dissipation</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-gauss" checked><label for="t-gauss" title="Enforce div(J) = charge density via SOR solver on void sites">Gauss Projection</label></div>

    <div class="combo-section-label">Matter</div>
    <div class="toggle-row"><input type="checkbox" id="t-genesis" checked><label for="t-genesis" title="Particles manifest when |J| exceeds K_B; evaporate when field energy drains">Genesis / Evaporation</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-movement" checked><label for="t-movement" title="Particles accumulate force into velocity remainder and move on the lattice">Movement</label></div>

    <div class="combo-section-label">Forces</div>
    <div class="toggle-row"><input type="checkbox" id="t-forces" checked><label for="t-forces" title="Master switch: apply electromagnetic and gravitational forces to manifested particles">Forces (EM + Gravity)</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-poisson" checked><label for="t-poisson" title="Solve Poisson equation for Coulomb potential (SOR, replaces direct div gradient)">Poisson Coulomb</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-gravity" checked><label for="t-gravity" title="Attractive force from smoothed density gradient: F = G_N * grad(rho)">Gravity</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-lorentz" checked><label for="t-lorentz" title="Magnetic force: F = alpha * s * (v x B) where B = curl(J). Does zero work">Lorentz Force</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-confinement"><label for="t-confinement" title="Linear confinement: opposite-sign pairs feel string tension beyond R_CRIT. String breaks if genesis is ON.">Confinement</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-color-forces"><label for="t-color-forces" title="Color charge interactions between quarks (SU(3) gauge coupling)">Color Forces</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-strong-force"><label for="t-strong-force" title="Residual strong force between composite hadrons (nuclear binding)">Strong Force</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-exchange"><label for="t-exchange" title="Exchange interaction: spin-dependent force between overlapping particles">Exchange Force</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-weak"><label for="t-weak" title="Weak transmutation: flavor-changing interactions at short range">Weak Transmutation</label></div>

    <details class="toggle-advanced">
      <summary>Advanced</summary>
      <div class="toggle-row"><input type="checkbox" id="t-selective"><label for="t-selective" title="Only damp flux near particles; vacuum EM waves propagate losslessly">Selective Damping</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-larmor"><label for="t-larmor" title="Accelerating charges radiate energy: damping proportional to a^2 (needs selective damping ON)">Larmor Radiation</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-dual"><label for="t-dual" title="Split flux into J_L + J_R chirality pair; observable psi = J_L + J_R">Dual Substrate</label></div>
    </details>

    <div class="ctrl-footnote">
      Field visualization: viewport buttons (top-right) &#8593;
    </div>
  `;
  return card;
}
