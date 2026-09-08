/**
 * Scale 4 viewport overlay — Solar System visualization controls.
 */

import { createScaleOverlayPanel, overlayRow, overlaySection } from '../../../../ui/components/viewport-overlays/panel-shell.js';

export function getScale4OverlayTemplate() {
  const viewSection = overlaySection(
    'Solar System view',
    'No render compression; body surfaces and separations share one coordinate gauge',
    `
    <div class="scale4-live-readout" aria-live="polite">
      <strong id="planetary-live-time">J2000 + 0.00 days</strong>
      <span id="planetary-live-bodies">19 modeled bodies</span>
    </div>
    <p class="scale4-overlay-status" id="planetary-overlay-status">10/10 effective kernels · AU/M☉/yr</p>
    ${overlayRow('', `
      <label class="scale-overlay-check" title="Draw the imported osculating/mean orbital ellipses around each body's parent">
        <input type="checkbox" id="planetary-opt-orbits" checked> Orbits
      </label>
      <label class="scale-overlay-check" title="Label every currently visible modeled body">
        <input type="checkbox" id="planetary-opt-labels" checked> Labels
      </label>
      <label class="scale-overlay-check" title="Show major modeled moons and their parent-relative orbits">
        <input type="checkbox" id="planetary-opt-moons" checked> Major moons
      </label>
      <label class="scale-overlay-check" title="Show axial-tilt arrows; spin is animated from each body's rotation period">
        <input type="checkbox" id="planetary-opt-axes"> Spin axes
      </label>
      <label class="scale-overlay-check" title="Show the ecliptic reference grid in the J2000 frame">
        <input type="checkbox" id="planetary-opt-ecliptic" checked> Ecliptic grid
      </label>
      <label class="scale-overlay-check" title="Show statistical asteroid and Kuiper-belt particles; belt points are presentation-only and exert no gravity">
        <input type="checkbox" id="planetary-opt-belts" checked> Small-body belts
      </label>
      <label class="scale-overlay-check" title="Show a conventional 0.95–1.67 AU circumsolar habitable-zone reference band; this is context, not a life detector">
        <input type="checkbox" id="planetary-opt-habitable" checked> Habitable band
      </label>
    `)}
    ${overlayRow('Physical gauge', `
      <output class="scale-overlay-readout" id="planetary-scale-gauge" title="Exact rendering conversion: 1 Three.js world unit equals 1 astronomical unit, and each spherical body uses its JPL volume-equivalent mean radius divided by 149,597,870.7 km/AU.">1:1 AU · mean radii</output>
      <button type="button" class="scale-overlay-action" id="planetary-home-view" title="Return to the full Solar System camera framing">System view</button>
    `)}
    `,
  );

  const physicsSection = overlaySection(
    'Live physics fields',
    'Computed from current body state; visible vectors use presentation-normalized lengths',
    `
    <p class="scale4-field-note" title="Newtonian gravity has infinite range. The field layer therefore samples local acceleration strength instead of drawing a false outer cutoff.">
      Gravity has no finite edge. Color maps sampled log |g|; Hill and Roche surfaces show named approximation domains.
    </p>
    ${overlayRow('', `
      <label class="scale-overlay-check scale4-field-gravity" title="Sample the live Newtonian acceleration field on the system reference plane. Blue is weaker and gold is stronger. This map has no hard outer cutoff.">
        <input type="checkbox" id="planetary-opt-gravity-field" checked> Gravity field |g|
      </label>
      <label class="scale-overlay-check scale4-field-acceleration" title="Show each visible body's live net-acceleration direction. Arrow length is logarithmically normalized for readability; it is not a distance.">
        <input type="checkbox" id="planetary-opt-acceleration"> Net acceleration
      </label>
      <label class="scale-overlay-check scale4-field-velocity" title="Show each visible body's live velocity direction. Arrow length is logarithmically normalized for readability; it is not a trajectory prediction.">
        <input type="checkbox" id="planetary-opt-velocity"> Velocity vectors
      </label>
      <label class="scale-overlay-check scale4-field-hill" title="Show instantaneous circular restricted-three-body Hill-radius approximations for low-mass children. A Hill sphere is an orbital-stability domain, not a cutoff of gravity.">
        <input type="checkbox" id="planetary-opt-hill"> Hill domains
      </label>
      <label class="scale-overlay-check scale4-field-roche" title="Show each applicable child's fluid Roche-limit surface around its parent while the Roche-disruption kernel is requested, derived from live radii and densities.">
        <input type="checkbox" id="planetary-opt-roche"> Roche limits
      </label>
      <label class="scale-overlay-check scale4-field-collision" title="Show the exact finite-body radii while the inelastic contact kernel is requested. These shells are easiest to see while focused on a body.">
        <input type="checkbox" id="planetary-opt-collision"> Collision surfaces
      </label>
    `)}
    <output class="scale4-field-readout" id="planetary-physics-overlay-readout" aria-live="polite" title="Live count and physical range of the currently rendered physics overlay">Gravity field · 1,089 samples</output>
    <div class="scale4-field-legend" aria-label="Physics overlay color legend">
      <span class="is-gravity" title="Gravity-field sample strength: blue is weaker, cyan is intermediate, gold is stronger">|g| field</span>
      <span class="is-acceleration" title="Gold arrows show live net-acceleration direction">acceleration</span>
      <span class="is-velocity" title="Cyan arrows show live velocity direction">velocity</span>
      <span class="is-hill" title="Blue wire spheres show Hill-domain approximations">Hill</span>
      <span class="is-roche" title="Orange wire spheres show fluid Roche-limit approximations">Roche</span>
      <span class="is-collision" title="Pink wire spheres show exact collision radii">collision</span>
    </div>
    `,
  );

  return createScaleOverlayPanel({
    id: 'cs-viewport-overlay',
    scaleClass: 'scale4-only',
    title: 'Solar System overlays',
    footnote: '[PARAMETRIC / IMPOSED] NASA/JPL data + standard effective celestial mechanics; not an FTD derivation',
    bodyHtml: `${viewSection}${physicsSection}`,
  });
}
