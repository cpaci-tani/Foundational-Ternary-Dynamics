/**
 * Scale 2 Viewport Overlay — Atom engine controls
 */

export function getScale2OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'ae-viewport-overlay';
  container.className = 'scale2-only viewport-overlay-panel';
  container.innerHTML = `
    <button class="view-toggle" id="toggle-ae-field" title="Show force field heatmap + vectors">Fields</button>
  `;
  return container;
}

export function getScale2LegendTemplate() {
  const container = document.createElement('div');
  container.id = 'ae-legend';
  container.className = 'scale2-only ae-legend';
  return container;
}
