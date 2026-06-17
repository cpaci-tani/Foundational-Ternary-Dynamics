/**
 * Scale 4 Viewport Overlay — planetary visualization controls
 */

import { createScaleOverlayPanel, overlayRow, overlaySection } from '../../../../ui/components/viewport-overlays/panel-shell.js';

export function getScale4OverlayTemplate() {
  const bodyHtml = overlaySection(
    'Orbital view',
    'Decorative vs physical gravity is set in the toolbar',
    `
    <p class="scale4-overlay-status" id="planetary-overlay-status">Orbital mechanics demo</p>
    ${overlayRow('', `
      <label class="scale-overlay-check" title="Draw orbital path traces for each body">
        <input type="checkbox" id="planetary-opt-orbits" checked> Orbital traces
      </label>
      <label class="scale-overlay-check" title="Per-body rotation axis helpers">
        <input type="checkbox" id="planetary-opt-axes"> Rotation axes
      </label>
    `)}
    `,
  );

  return createScaleOverlayPanel({
    id: 'cs-viewport-overlay',
    scaleClass: 'scale4-only',
    title: 'Planetary overlays',
    footnote: 'N-body demo — timing fidelity depends on gravity mode',
    bodyHtml,
  });
}
