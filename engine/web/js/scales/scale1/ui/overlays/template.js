/**
 * Scale 1 Viewport Overlay — Particle engine controls
 */

export function getScale1OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'pe-viewport-overlay';
  container.className = 'scale1-only viewport-overlay-panel';
  container.innerHTML = `
    <button class="view-toggle field-toggle" id="toggle-velocities" title="Velocity vectors">
      <span class="field-swatch field-swatch-pe-velocities"></span>Velocities
    </button>
    <button class="view-toggle field-toggle" id="toggle-trails" title="Orbit trails">
      <span class="field-swatch field-swatch-pe-trails"></span>Trails
    </button>
    <button class="view-toggle field-toggle" id="toggle-pe-efield" title="Coulomb E-field streamlines">
      <span class="field-swatch field-swatch-pe-efield"></span>E Field
    </button>
    <button class="view-toggle field-toggle" id="toggle-pe-potential" title="Coulomb potential heatmap (XZ plane)">
      <span class="field-swatch field-swatch-pe-potential"></span>Potential
    </button>
    <button class="view-toggle field-toggle" id="toggle-pe-gravity-field" title="Gravity force vectors (XZ plane)">
      <span class="field-swatch field-swatch-pe-gravity"></span>Gravity F
    </button>
    <button class="view-toggle field-toggle" id="toggle-pe-forces" title="Net force arrows per particle">
      <span class="field-swatch field-swatch-pe-forces"></span>Forces
    </button>
    <span class="field-sep"></span>
    <button class="view-toggle dynamics-toggle" id="toggle-pe-gravity" title="Toggle gravitational force (dynamics)">
      Gravity
    </button>
    <button class="view-toggle dynamics-toggle" id="toggle-pe-damping" title="Toggle velocity damping (dynamics)">
      Damping
    </button>
  `;
  return container;
}
