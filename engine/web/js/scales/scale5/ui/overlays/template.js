/**
 * Scale 5 Viewport Overlay — cosmic simulation context
 *
 * Pass B (2026-09-10, Ruling P1) adds the first interactive section, "Gas
 * visualization": a colour-by selector (none/density/temperature/speed) for
 * the star and gas point clouds, plus an optional smoothing-length-circle
 * overlay for gas bodies. Listeners are bound once by
 * scales/scale5/ui/overlays/component.js (bindScale5OverlayControls,
 * called by ViewportOverlaysComponent.init() right after this template is
 * appended) — this file only supplies markup.
 *
 * Pass A (2026-09-10) adds "Dynamics": a velocity-vector overlay (all
 * bodies, logarithmic length normalisation so a quiescent halo and a
 * near-lattice-limit ejecta body both stay legible in the same frame — a
 * display convenience, not a physical scale, matching the colour-by ramp's
 * own per-frame normalization above) and a centre-of-mass marker. Both are
 * O(N) over the already-packed `velocities` buffer (Pass B) and the
 * existing comX/comY/comZ diagnostics (Pass 0a) — no new bridge field, no
 * new pair loop. Pass D appends its own overlaySection() block to the SAME
 * body alongside these; each section's ids/behavior belong to the pass
 * that added it.
 *
 * Pass C (2026-09-10) adds "Cosmology": a comoving reference-grid toggle. A
 * single THREE.Group built once with viewport/boundary-geometry.js's
 * buildBoundary() and rescaled every frame to boxSize * scaleFactor (the
 * same "reference box size times a(t)" quantity the Comoving Box Size
 * diagnostic row computes) — geometry, not a per-body loop, so no new
 * O(N^2) cost.
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

  const dynamicsSection = overlaySection(
    'Dynamics',
    '',
    `${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument] Draws a line from each body in its velocity direction. Length is logarithmically normalised to this frame's fastest body, so a quiescent halo and a fast-moving ejecta or merger remnant both stay visible in the same view — a display convenience, not a physical scale. Colour ramps green (slow) through yellow/orange/red to white as speed approaches the lattice speed limit c = 1/sqrt(3) [SELECTION].">
        <input type="checkbox" id="cosmic-overlay-velocity-vectors"> Velocity vectors
      </label>
    `)}
    ${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument] Marks the instantaneous mass-weighted centre of mass of all live bodies (the same quantity the COM Drift diagnostic row tracks over time).">
        <input type="checkbox" id="cosmic-overlay-com-marker"> Centre-of-mass marker
      </label>
    `)}`,
  );

  const cosmologySection = overlaySection(
    'Cosmology',
    '',
    `${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument] Draws a wireframe cube sized to the reference box (Cosmic Runtime section) times the current scale factor a(t) — the SAME quantity the Comoving Box Size diagnostic row reports — so the grid visibly grows as the background expands. Purely a diagnostic overlay; it never feeds back into N-body dynamics.">
        <input type="checkbox" id="cosmic-overlay-comoving-grid"> Comoving reference grid
      </label>
    `)}`,
  );

  return createScaleOverlayPanel({
    id: 'cosmic-viewport-overlay',
    scaleClass: 'scale5-only',
    title: 'Cosmic overlays',
    footnote: 'Grid and axes off by default — use status bar View menu if needed',
    bodyHtml: `${frameSection}${gasSection}${dynamicsSection}${cosmologySection}`,
  });
}
