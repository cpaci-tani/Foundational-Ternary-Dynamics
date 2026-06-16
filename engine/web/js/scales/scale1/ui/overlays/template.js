/**
 * Scale 1 Viewport Overlay — Particle engine controls
 */

export function getScale1OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'pe-viewport-overlay';
  container.className = 'scale1-only viewport-overlay-panel';
  container.innerHTML = `
    <button class="view-toggle field-toggle" id="toggle-velocities" title="Velocity vectors — colored by causal speed |v|/c (green slow → red → white at the 1/√3 lattice cap)">
      <span class="field-swatch field-swatch-pe-velocities"></span>Velocities
    </button>
    <button class="view-toggle field-toggle" id="toggle-trails" title="Orbit trails">
      <span class="field-swatch field-swatch-pe-trails"></span>Trails
    </button>
    <button class="view-toggle field-toggle" id="toggle-pe-efield" title="Coulomb E-field streamlines">
      <span class="field-swatch field-swatch-pe-efield"></span>E Field
    </button>
    <button class="view-toggle field-toggle" id="toggle-pe-potential" title="Coulomb potential heatmap + E-field arrows (XZ plane)">
      <span class="field-swatch field-swatch-pe-potential"></span>Potential
    </button>
    <button class="view-toggle field-toggle" id="toggle-pe-gravity-field" title="Gravity force vectors (XZ plane). Coupling G_PE = derived α_G(e,e)=(m_e/m_P)² ≈ 1.75e-45 (FTD-0131). Physically correct — negligible next to Coulomb; arrows may be invisible at particle scale.">
      <span class="field-swatch field-swatch-pe-gravity"></span>Gravity F
    </button>
    <span class="field-sep"></span>
    <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-coulomb" title="Coulomb (EM) force arrows per particle — F_C">
      <span class="field-swatch field-swatch-pe-force-coulomb"></span>F<sub>C</sub>
    </button>
    <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-gravity" title="Gravitational force arrows per particle — F_g. True G_PE magnitude; arrow length uses GRAVITY_VIS_GAIN for legibility only.">
      <span class="field-swatch field-swatch-pe-force-gravity"></span>F<sub>g</sub>
    </button>
    <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-strong" title="Strong / color force arrows per particle — F_S. Requires pe-strong toggle + colored particles.">
      <span class="field-swatch field-swatch-pe-force-strong"></span>F<sub>S</sub>
    </button>
    <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-net" title="Net force arrows per particle — sum of enabled force terms">
      <span class="field-swatch field-swatch-pe-force-net"></span>F<sub>net</sub>
    </button>
    <button class="view-toggle field-toggle" id="toggle-pe-system" title="System observables: center of mass (cross), total momentum p (cyan), angular-momentum axis L (magenta)">
      <span class="field-swatch field-swatch-pe-system"></span>System
    </button>
    <span class="field-sep"></span>
    <button class="view-toggle dynamics-toggle" id="toggle-pe-gravity" title="Toggle gravitational force (dynamics). G_PE = 1/(4π·m_P²) is the FTD-0131 physical coupling: α_G(e,e)=(m_e/m_P)² ≈ 1.75e-45. Dynamics are negligible; Gravity PE chart and diagnostics show the true value.">
      Gravity
    </button>
    <button class="view-toggle dynamics-toggle" id="toggle-pe-damping" title="Toggle velocity damping (dynamics)">
      Damping
    </button>
  `;
  return container;
}
