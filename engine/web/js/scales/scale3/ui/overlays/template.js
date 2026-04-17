/**
 * Scale 3 Viewport Overlay — Molecular engine controls
 */

export function createScale3OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'mol-viewport-overlay';
  container.className = 'scale3-only viewport-overlay-panel';
  container.innerHTML = `
    <button class="view-toggle active" id="toggle-mol-bonds" title="Show covalent bonds">Bonds</button>
    <button class="view-toggle" id="toggle-mol-field" title="Show force field heatmap + vectors">Fields</button>
  `;
  return container;
}

export function createScale3LegendTemplate() {
  const container = document.createElement('div');
  container.id = 'mol-legend';
  container.className = 'scale3-only ae-legend';
  return container;
}
