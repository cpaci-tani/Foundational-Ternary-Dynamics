/**
 * Scale 2/3 Viewport Overlay — atom/molecule MD + QM structure visualization.
 * Visible on scales 2 and 3 (class scale-ae). Element IDs unchanged for wiring.
 */

import { createScaleOverlayPanel, overlayRow, overlaySection } from '../../../../ui/components/viewport-overlays/panel-shell.js';

export function getScale2OverlayTemplate() {
  const bodyHtml = [
    overlaySection(
      'Atomic structure',
      'QM-inspired decoration — not the integrator state',
      `
      ${overlayRow('', `
        <label class="scale-overlay-check" title="[EMPIRICAL DISPLAY] Sampled hydrogenic/Slater orbital-density motifs. These points are not electron trajectories or a solved many-electron wavefunction.">
          <input type="checkbox" id="ae-show-clouds" checked> Orbitals
        </label>
        <label class="scale-overlay-check" title="[EMPIRICAL DISPLAY] Nuclear-extent envelope scaled with mass number A^(1/3). It is not a rendered strong-force field.">
          <input type="checkbox" id="ae-show-shells" checked> Nuclear extent
        </label>
        <label class="scale-overlay-check" title="Chemical or isotope labels anchored to the live atom records.">
          <input type="checkbox" id="ae-show-labels" checked> Labels
        </label>
        <label class="scale-overlay-check" title="[EMPIRICAL DISPLAY] Principal-shell scale guides from the Slater effective-charge approximation; not hard quantum boundaries.">
          <input type="checkbox" id="ae-show-shell-bounds"> Shell bounds
        </label>
        <label class="scale-overlay-check" title="[SCHEMATIC] Valence p/d/f symmetry motifs. They show angular families, not solved orbital phase or occupancy.">
          <input type="checkbox" id="ae-show-lobes"> Lobe motifs
        </label>
      `)}
      ${overlayRow('bonds', `
        <span class="scale-overlay-inline-label">Bonds</span>
        <select class="scale-overlay-select" id="bond-style-select" title="Covalent bond rendering style">
          <option value="cylinders" selected>Thick</option>
          <option value="lines">Thin</option>
          <option value="off">Off</option>
        </select>
      `)}
      `,
    ),
    overlaySection(
      'Electrostatic landscape',
      '',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-ae-field"
          aria-pressed="false"
          title="[PARAMETRIC] Signed softened Coulomb potential heatmap plus world-aligned E-field vectors on the XZ plane, sampled from the live partial charges.">
          <span class="field-swatch field-swatch-ae-field"></span>Potential + E
        </button>
      `),
    ),
    overlaySection(
      'Force decomposition',
      'Per-atom arrows from aeGetForceDecomposition',
      overlayRow('force', `
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-ionic"
          aria-pressed="false"
          title="Coulomb force from fractional charges">
          <span class="field-swatch field-swatch-ae-force-ionic"></span>F<sub>C</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-vdw"
          aria-pressed="false"
          title="Lennard-Jones 12-6 van der Waals">
          <span class="field-swatch field-swatch-ae-force-vdw"></span>F<sub>vdW</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-bond"
          aria-pressed="false"
          title="Harmonic bond springs">
          <span class="field-swatch field-swatch-ae-force-bond"></span>F<sub>B</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-hbond"
          aria-pressed="false"
          title="Directional 10-12 H-bond radial force (effective, incomplete gradient)">
          <span class="field-swatch field-swatch-ae-force-hbond"></span>F<sub>HB</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-angle"
          aria-pressed="false"
          title="Three-body harmonic VSEPR angle-strain force">
          <span class="field-swatch field-swatch-ae-force-angle"></span>F<sub>θ</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-dipole"
          aria-pressed="false"
          title="Effective dipole-dipole force">
          <span class="field-swatch field-swatch-ae-force-dipole"></span>F<sub>μμ</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-net"
          aria-pressed="false"
          title="Actual post-safety force used by the integrator, including every enabled term">
          <span class="field-swatch field-swatch-ae-force-net"></span>F<sub>net</sub>
        </button>
      `),
    ),
    overlaySection(
      'Intermolecular geometry',
      'Phase-3 visuals — may show when force toggles are off',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-ae-dipoles"
          aria-pressed="false"
          title="Bond electronegativity dipole moments (μ)">
          <span class="field-swatch field-swatch-ae-dipole"></span>Dipoles μ
        </button>
        <button class="view-toggle field-toggle" id="toggle-ae-hbonds"
          aria-pressed="false"
          title="Dashed donor-H···acceptor lines">
          <span class="field-swatch field-swatch-ae-hbond"></span>H-bonds
        </button>
      `),
    ),
    overlaySection(
      'Kinetics',
      '',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-ae-velocities"
          aria-pressed="false"
          title="Velocity vectors — |v|/c color ramp">
          <span class="field-swatch field-swatch-ae-velocities"></span>Velocities v
        </button>
      `),
    ),
    `<div class="scale2-only">${overlaySection(
      'Nuclear event & transport',
      'Live geometry where available; presentation timing/scale is labeled',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-ae-nuclear-events"
          aria-pressed="false"
          title="Show collision-centered event flashes and reaction-plane rings. Locations and axes come from accepted live collisions; size and duration are presentation-scaled.">
          <span class="field-swatch field-swatch-ae-nuclear-event"></span>Events
        </button>
        <button class="view-toggle field-toggle" id="toggle-ae-radiation"
          aria-pressed="false"
          title="Show emitted-neutron packets along their accepted product directions plus qualitative prompt-gamma fronts. Travel speeds are presentation-scaled; energies remain in telemetry.">
          <span class="field-swatch field-swatch-ae-radiation"></span>Radiation
        </button>
        <button class="view-toggle field-toggle" id="toggle-ae-heat"
          aria-pressed="false"
          title="Show local halos scaled by each event's modeled deposited-energy fraction. Radius is presentation-scaled; exact deposited MeV/J remains in telemetry.">
          <span class="field-swatch field-swatch-ae-heat"></span>Heat
        </button>
        <button class="view-toggle field-toggle" id="toggle-ae-nuclear-boundary"
          aria-pressed="false"
          title="Show the live spherical neutron-transport boundary. Cyan means open/leaking; amber means reflective.">
          <span class="field-swatch field-swatch-ae-nuclear-boundary"></span>Boundary
        </button>
      `),
    )}</div>`,
    `
    <div class="scale-overlay-section scale3-only">
      <span class="scale-overlay-section-label">Molecular view</span>
      ${overlayRow('', `
        <button class="view-toggle active" id="toggle-mol-bonds" aria-pressed="true"
          title="Show covalent bonds">Bonds</button>
      `)}
    </div>
    `,
  ].join('');

  return createScaleOverlayPanel({
    id: 'ae-viewport-overlay',
    scaleClass: 'scale-ae',
    title: 'Atom overlays',
    footnote: 'Effective atom-engine visuals. Empirical, schematic, and parametric layers are labeled; none is substrate-QM recovery.',
    bodyHtml,
    legendHtml: '<div id="ae-legend" class="ae-legend scale-overlay-legend" aria-live="polite"></div>',
  });
}

/** @deprecated Legend is embedded in getScale2OverlayTemplate(). */
export function getScale2LegendTemplate() {
  const container = document.createElement('div');
  container.id = 'ae-legend-orphan';
  container.hidden = true;
  return container;
}

/** Scale 3 reuses the atom-engine overlay panel (scale-ae). */
export function getScale3OverlayTemplate() {
  const stub = document.createElement('div');
  stub.id = 'mol-viewport-overlay-stub';
  stub.hidden = true;
  return stub;
}

export function getScale3LegendTemplate() {
  const stub = document.createElement('div');
  stub.id = 'mol-legend-stub';
  stub.hidden = true;
  return stub;
}
