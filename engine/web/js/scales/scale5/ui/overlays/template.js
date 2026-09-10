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
 *
 * Pass D (2026-09-10) adds three new sections and one addition to Pass B's
 * existing colour-by select, all bound the SAME way through
 * bindScale5OverlayControls/syncScale5Overlays:
 *   - a "Type" colour-by option (flat categorical palette, no viridis
 *     ramp — there is no scalar ordering across body types);
 *   - "Visibility": the five renderer toggles (toggleDarkMatter/
 *     toggleGasClouds/toggleStars/toggleBlackHoles/toggleAccretionDisks)
 *     that cosmic-renderer.js has honoured in update() since before this
 *     pass but that nothing ever called (plan step 0.7, never executed —
 *     Ruling P2 moved it here);
 *   - "Black holes": a fixed-size location marker and a Bondi-accretion-
 *     capture-radius marker, both [IMPOSED] presentation rings, never a
 *     relativistic or horizon radius;
 *   - "Trails & camera": bounded per-id position-history trails for the
 *     star/remnant/black-hole population, and a presentation-only
 *     body-size multiplier (does NOT touch the black-hole render-radius
 *     proxy, a separately-tagged quantity). Follow-a-body and
 *     centre-of-mass camera lock are NOT here — they are two new
 *     `<option>`s on the toolbar's existing `#cosmic-camera-select`
 *     (scale5/ui/toolbar/template.js), since that select is already the
 *     established surface for camera framing and is pinned for existence
 *     only (constraints section 6), so new options are safe.
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
      <select class="scale-overlay-select" id="cosmic-overlay-colorby" title="Colours the star and gas point clouds by an existing per-body quantity this tick; it does not alter dynamics. Density/Temperature/Speed are [MEASURED — instrument] viridis ramps; Type is a fixed categorical palette (no scalar ordering across body types), presentation-only.">
        <option value="none" selected>None (default)</option>
        <option value="density">Density</option>
        <option value="temperature">Temperature</option>
        <option value="speed">Speed</option>
        <option value="type">Type</option>
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

  // Pass D: the five renderer visibility toggles (plan step 0.7, never
  // executed until now — cosmic-renderer.js's update() has honoured
  // _showDM/_showGas/_showStars/_showBH/_showDisks since before this pass,
  // but nothing ever called toggleDarkMatter/toggleGasClouds/toggleStars/
  // toggleBlackHoles/toggleAccretionDisks). Default checked to match the
  // renderer's own defaults (all true), so a fresh scenario load looks
  // identical whether or not a viewer has ever opened this panel.
  const visibilitySection = overlaySection(
    'Visibility',
    '',
    `${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument control] Shows or hides the dark-matter point cloud. Presentation only; does not alter dynamics.">
        <input type="checkbox" id="cosmic-overlay-show-dm" checked> Dark matter
      </label>
    `)}
    ${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument control] Shows or hides the gas and nebula point clouds. Presentation only; does not alter dynamics.">
        <input type="checkbox" id="cosmic-overlay-show-gas" checked> Gas &amp; nebula clouds
      </label>
    `)}
    ${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument control] Shows or hides the star point cloud (stars, white dwarfs, neutron stars). Presentation only; does not alter dynamics.">
        <input type="checkbox" id="cosmic-overlay-show-stars" checked> Stars
      </label>
    `)}
    ${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument control] Shows or hides black-hole/quasar meshes (event horizon, corona, jets). Presentation only; does not alter dynamics.">
        <input type="checkbox" id="cosmic-overlay-show-bh" checked> Black holes
      </label>
    `)}
    ${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument control] Shows or hides the accretion-disk layer of black-hole/quasar meshes, leaving the event horizon itself visible. Presentation only; does not alter dynamics.">
        <input type="checkbox" id="cosmic-overlay-show-disks" checked> Accretion disks
      </label>
    `)}`,
  );

  const blackHolesSection = overlaySection(
    'Black holes',
    '',
    `${overlayRow('', `
      <label class="scale-overlay-check" title="[IMPOSED] presentation marker: a fixed-size ring at each black hole/quasar position so it stays locatable even when its accretion-disk mesh is small on screen or mid fade-in. NOT a horizon or any other physical radius.">
        <input type="checkbox" id="cosmic-overlay-bh-markers"> Black-hole location markers
      </label>
    `)}
    ${overlayRow('', `
      <label class="scale-overlay-check" title="[IMPOSED threshold rule] Draws a circle at the Bondi capture radius r_acc = max(1.5, mass^(1/3) x 0.3) around each black hole — the SAME geometric radius the bondi_accretion rule (Physics rules card) uses when enabled, drawn here regardless of that toggle's state. Presentation only; never a relativistic or horizon radius.">
        <input type="checkbox" id="cosmic-overlay-accretion-markers"> Bondi accretion-radius markers
      </label>
    `)}`,
  );

  const trailsSection = overlaySection(
    'Trails & camera',
    '',
    `${overlayRow('', `
      <label class="scale-overlay-check" title="[MEASURED — instrument] Draws a short fading position history behind each star, white dwarf, neutron star, black hole, or quasar (capped at 150 tracked bodies; gas/nebula/dark-matter are excluded as too numerous to read as trails). Presentation only; reads positions already computed, writes nothing back.">
        <input type="checkbox" id="cosmic-overlay-trails"> Body trails
      </label>
    `)}
    ${overlayRow('', `
      <label class="pe-ctrl-row" title="Presentation-only global multiplier on star/gas/dark-matter/nebula point size. Does NOT affect the black-hole render-radius proxy, which is a separately [IMPOSED]-tagged quantity.">
        <span class="pe-ctrl-label">Body size &times;</span>
        <input type="range" class="pe-slider" id="cosmic-overlay-body-size" min="0.25" max="4" step="0.05" value="1">
        <span class="pe-ctrl-value" id="cosmic-overlay-body-size-value">1.00</span>
      </label>
    `)}
    <p class="scale-overlay-section-hint" style="margin:4px 0 0">
      Follow-a-body and centre-of-mass camera lock are toolbar Camera options above the viewport, not here — see #cosmic-camera-select.
    </p>`,
  );

  return createScaleOverlayPanel({
    id: 'cosmic-viewport-overlay',
    scaleClass: 'scale5-only',
    title: 'Cosmic overlays',
    footnote: 'Grid and axes off by default — use status bar View menu if needed',
    bodyHtml: `${frameSection}${visibilitySection}${gasSection}${dynamicsSection}${cosmologySection}${blackHolesSection}${trailsSection}`,
  });
}
