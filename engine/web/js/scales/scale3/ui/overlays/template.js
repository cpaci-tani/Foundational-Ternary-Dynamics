/**
 * Scale 3 Viewport Overlay — Molecular engine controls
 */

export function getScale3OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'mol-viewport-overlay';
  container.className = 'scale3-only viewport-overlay-panel';
  // 2026-05-27 audit W10 fix: removed #toggle-mol-field button which had
  // no event listener anywhere in the codebase. To re-add: wire it into
  // the AE field-overlay path via setAEVisualToggle('showAEField', …).
  container.innerHTML = `
    <button class="view-toggle active" id="toggle-mol-bonds" title="Show covalent bonds">Bonds</button>
  `;
  return container;
}

export function getScale3LegendTemplate() {
  const container = document.createElement('div');
  container.id = 'mol-legend';
  container.className = 'scale3-only ae-legend';
  return container;
}
