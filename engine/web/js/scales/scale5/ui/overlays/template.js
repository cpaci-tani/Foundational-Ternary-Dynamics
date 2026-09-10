/**
 * Scale 5 Viewport Overlay — cosmic simulation context
 *
 * Pass B (2026-09-10, Ruling P1) adds the first interactive section, "Gas
 * visualization": a colour-by selector (none/density/temperature/speed) for
 * the star and gas point clouds, plus an optional smoothing-length-circle
 * overlay for gas bodies. Listeners are bound once by
 * scales/scale5/ui/overlays/component.js (bindScale5OverlayControls,
 * called by ViewportOverlaysComponent.init() right after this template is
 * appended) — this file only supplies markup. Passes C and D append their
 * own overlaySection() blocks to the SAME body alongside this one; this
 * section's ids/behavior are Pass B's alone.
 */

import { createScaleOverlayPanel, overlayRow, overlaySection } from '../../../../ui/components/viewport-overlays/panel-shell.js';

export function getScale5OverlayTemplate() {
  const frameSection = overlaySection(
    'Simulation frame',
    '',
    `<p class="scale-overlay-section-hint" style="margin:0">
      N-body + SPH cosmic demo. Constants mix [THEOREM], [SELECTION], and [IMPOSED]
      inputs — see the Cosmic Info side panel for the full table.
    </p>`,
  );

  const gasSection = overlaySection(
    'Gas visualization',
    '',
    `${overlayRow('', `
      <span class="scale-overlay-inline-label">Colour by</span>
      <select class="scale-overlay-select" id="cosmic-overlay-colorby" title="[MEASURED — instrument] Colours the star and gas point clouds by an existing per-body measured quantity this tick; it does not alter dynamics.">
        <option value="none" selected>None (default)</option>
        <option value="density">Density</option>
        <option value="temperature">Temperature</option>
        <option value="speed">Speed</option>
      </select>
    `)}
    ${overlayRow('', `
      <label class="scale-overlay-check" title="[IMPOSED effective gas dynamics; coefficients in engine units] Draws a circle of radius h (the SPH smoothing length) around each gas body.">
        <input type="checkbox" id="cosmic-overlay-smoothing-circles"> Smoothing-length circles
      </label>
    `)}`,
  );

  return createScaleOverlayPanel({
    id: 'cosmic-viewport-overlay',
    scaleClass: 'scale5-only',
    title: 'Cosmic overlays',
    footnote: 'Grid and axes off by default — use status bar View menu if needed',
    bodyHtml: `${frameSection}${gasSection}`,
  });
}
