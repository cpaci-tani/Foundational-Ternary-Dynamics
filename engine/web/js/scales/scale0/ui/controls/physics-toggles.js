/**
 * Scale 0 Physics Toggles Card
 */

export function createPhysicsTogglesCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  card.innerHTML = `
    <div class="card-title">Physics Toggles</div>
    <div id="physics-profile-warning" class="physics-profile-warning" hidden>
      Modified physics profile — scenario qualification suspended.
    </div>
    <div class="combo-section-label">Wave & Field</div>
    <div class="toggle-row"><input type="checkbox" id="t-wave" checked><label for="t-wave" title="Flux wave equation: J evolves via discrete Laplacian (c^2 nabla^2 J)">Wave Propagation</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-coupling" checked><label for="t-coupling" title="Manifested particles source flux via coupling term g_c * grad(s)">State-Flux Coupling</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-damping" checked><label for="t-damping" title="Exponential flux decay at rate alpha per tick (energy dissipation)">Dissipation</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-gauss" checked><label for="t-gauss" title="Enforce div(J) = charge density via SOR solver on void sites">Gauss Projection</label></div>

    <div class="combo-section-label">Matter</div>
    <div class="toggle-row"><input type="checkbox" id="t-genesis" checked><label for="t-genesis" title="Particles manifest when |J| exceeds K_B">Genesis</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-evaporation"><label for="t-evaporation" title="Manifested particles evaporate when their local field energy drains">Evaporation</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-movement" checked><label for="t-movement" title="Particles accumulate force into velocity remainder and move on the lattice">Movement</label></div>

    <div class="combo-section-label">Forces</div>
    <div class="toggle-row"><input type="checkbox" id="t-forces" checked><label for="t-forces" title="Master switch: apply electromagnetic and gravitational forces to manifested particles">Forces (EM + Gravity)</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-poisson" checked><label for="t-poisson" title="Solve Poisson equation for Coulomb potential (SOR, replaces direct div gradient)">Poisson Coulomb</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-gravity"><label for="t-gravity" title="Attractive force from smoothed density gradient: F = G_N * grad(rho)">Gravity</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-lorentz"><label for="t-lorentz" title="Magnetic force: F = alpha * s * (v x B) where B = curl(J). Does zero work">Lorentz Force</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-confinement"><label for="t-confinement" title="Intent flag only; the native C++ engine has no confinement-force branch. The separate viewport overlay is a visualization proxy.">Confinement (intent only)</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-color-forces"><label for="t-color-forces" title="Color charge interactions between quarks (SU(3) gauge coupling)">Color Forces</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-strong-force"><label for="t-strong-force" title="Residual strong force between composite hadrons (nuclear binding)">Strong Force</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-exchange"><label for="t-exchange" title="Exchange interaction: spin-dependent force between overlapping particles">Exchange Force</label></div>
    <div class="toggle-row"><input type="checkbox" id="t-weak"><label for="t-weak" title="Weak transmutation: flavor-changing interactions at short range">Weak Transmutation</label></div>

    <details class="toggle-advanced">
      <summary>Advanced</summary>
      <div class="toggle-row"><input type="checkbox" id="t-selective" checked><label for="t-selective" title="Only damp flux near particles; vacuum EM waves propagate losslessly">Selective Damping</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-larmor"><label for="t-larmor" title="Accelerating charges radiate energy: damping proportional to a^2 (needs selective damping ON)">Larmor Radiation</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-dual"><label for="t-dual" title="Split flux into J_L + J_R chirality pair; observable psi = J_L + J_R">Dual Substrate</label></div>
    </details>

    <details class="toggle-advanced">
      <summary>Research (validated)</summary>
      <div class="toggle-row"><input type="checkbox" id="t-pair-production"><label for="t-pair-production" title="Correlated +1/-1 pair manifestation on an independent code path (F11.A-5 audited); separate from genesis.">Pair Production</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-langevin"><label for="t-langevin" title="Stochastic Ornstein-Uhlenbeck thermostat (SplitMix64 per-voxel noise), validated to equipartition +/-4%. Default OFF = golden-neutral.">Langevin Thermostat</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-triad"><label for="t-triad" title="Colour-singlet triad binding (locked). Requires Color Forces ON.">Triad Binding</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-latency-field"><label for="t-latency-field" title="Poisson-based latency field (grad^2 L = 4 pi G rho, gravity proxy). Requires Gravity ON.">Latency Field</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-exact-dual-gauss"><label for="t-exact-dual-gauss" title="Exact dual-cell face-flux Gauss projection (isolated electrodynamics variant).">Exact Dual Gauss</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-symmetric-move"><label for="t-symmetric-move" title="Coordinate-independent update traversal and axis ordering (SplitMix64 Fisher-Yates). Requires Movement ON.">Symmetric Movement Order</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-absorbing"><label for="t-absorbing" title="Imposed quadratic damping sponge at the lattice faces (absorbs outgoing waves). Requires Wave Propagation ON.">Absorbing Boundary</label></div>
      <div class="toggle-row"><input type="checkbox" id="t-knot-tracking"><label for="t-knot-tracking" title="Record per-knot telemetry at tick end (observation-only, golden-neutral).">Knot Tracking</label></div>
    </details>

    <div class="ctrl-footnote">
      <button class="ctrl-reset-btn" id="btn-reset-physics-toggles" type="button"
          title="Reload the active scenario so its C++ isolation profile and seed are restored.">
        &#8634; Restore scenario profile
      </button>
      <span class="ctrl-footnote-text">Field visualization: viewport buttons (top-right) &#8593;</span>
    </div>
  `;
  return card;
}
