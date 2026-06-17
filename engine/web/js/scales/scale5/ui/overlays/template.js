/**
 * Scale 5 Viewport Overlay — cosmic simulation context
 */

import { createScaleOverlayPanel, overlaySection } from '../../../../ui/components/viewport-overlays/panel-shell.js';

export function getScale5OverlayTemplate() {
  const bodyHtml = overlaySection(
    'Simulation frame',
    '',
    `<p class="scale-overlay-section-hint" style="margin:0">
      N-body + SPH cosmic demo. Constants mix [THEOREM], [SELECTION], and [IMPOSED]
      inputs — see the Cosmic Info side panel for the full table.
    </p>`,
  );

  return createScaleOverlayPanel({
    id: 'cosmic-viewport-overlay',
    scaleClass: 'scale5-only',
    title: 'Cosmic overlays',
    footnote: 'Grid and axes off by default — use status bar View menu if needed',
    bodyHtml,
  });
}
