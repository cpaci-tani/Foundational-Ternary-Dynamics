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
        <label class="scale-overlay-check" title="Electron orbital probability clouds">
          <input type="checkbox" id="ae-show-clouds" checked> Orbitals
        </label>
        <label class="scale-overlay-check" title="Strong-force glow shells scaled by A^(1/3)">
          <input type="checkbox" id="ae-show-shells" checked> Nucleus
        </label>
        <label class="scale-overlay-check" title="Principal quantum shell boundary spheres">
          <input type="checkbox" id="ae-show-shell-bounds"> Shell bounds
        </label>
        <label class="scale-overlay-check" title="Valence p / d / f lobe shapes">
          <input type="checkbox" id="ae-show-lobes"> Lobes
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
          title="Coulomb potential heatmap + E-field arrows (XZ plane) from partial charges">
          <span class="field-swatch field-swatch-ae-field"></span>Potential + E
        </button>
      `),
    ),
    overlaySection(
      'Force decomposition',
      'Per-atom arrows from aeGetForceDecomposition',
      overlayRow('force', `
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-ionic"
          title="Coulomb force from fractional charges">
          <span class="field-swatch field-swatch-ae-force-ionic"></span>F<sub>C</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-vdw"
          title="Lennard-Jones 12-6 van der Waals">
          <span class="field-swatch field-swatch-ae-force-vdw"></span>F<sub>vdW</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-bond"
          title="Harmonic bond springs">
          <span class="field-swatch field-swatch-ae-force-bond"></span>F<sub>B</sub>
        </button>
        <button class="view-toggle field-toggle ae-force-btn" id="ae-force-net"
          title="F_net = F_C + F_vdW + F_B only (excludes angle, H-bond, μ–μ)">
          <span class="field-swatch field-swatch-ae-force-net"></span>F<sub>net</sub>
        </button>
      `),
    ),
    overlaySection(
      'Intermolecular geometry',
      'Phase-3 visuals — may show when force toggles are off',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-ae-dipoles"
          title="Bond electronegativity dipole moments (μ)">
          <span class="field-swatch field-swatch-ae-dipole"></span>Dipoles μ
        </button>
        <button class="view-toggle field-toggle" id="toggle-ae-hbonds"
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
          title="Velocity vectors — |v|/c color ramp">
          <span class="field-swatch field-swatch-ae-velocities"></span>Velocities v
        </button>
      `),
    ),
    `
    <div class="scale-overlay-section scale3-only">
      <span class="scale-overlay-section-label">Molecular view</span>
      ${overlayRow('', `
        <button class="view-toggle active" id="toggle-mol-bonds" title="Show covalent bonds">Bonds</button>
      `)}
    </div>
    `,
  ].join('');

  return createScaleOverlayPanel({
    id: 'ae-viewport-overlay',
    scaleClass: 'scale-ae',
    title: 'Atom overlays',
    footnote: 'Classical MD overlays (sim units) — not substrate QM',
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
