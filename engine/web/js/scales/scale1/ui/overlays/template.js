/**
 * Scale 1 Viewport Overlay — particle engine dynamics (grouped by physical role)
 */

import { createScaleOverlayPanel, overlayRow, overlaySection } from '../../../../ui/components/viewport-overlays/panel-shell.js';

export function getScale1OverlayTemplate() {
  const bodyHtml = [
    overlaySection(
      'Kinetics',
      'Instantaneous state vectors',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-velocities"
          title="Velocity vectors — colored by causal speed |v|/c (green slow → white at the 1/√3 lattice cap)">
          <span class="field-swatch field-swatch-pe-velocities"></span>Velocities
        </button>
        <button class="view-toggle field-toggle" id="toggle-trails"
          title="Tick-aligned trajectory history. Controls switches between breadcrumbs, particle-colored lines, and an effective kinetic-energy-density line heatmap; length, stride, despawn fade, opacity, and point size are also adjustable.">
          <span class="field-swatch field-swatch-pe-trails"></span>Trails
        </button>
      `),
    ),
    overlaySection(
      'Field planes',
      'Effective fields sampled on XZ',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-pe-efield"
          title="Coulomb E-field streamlines. Line density ∝ |E| — denser lines mark stronger field regions.">
          <span class="field-swatch field-swatch-pe-efield"></span>E field
        </button>
        <button class="view-toggle field-toggle" id="toggle-pe-potential"
          title="Coulomb potential heatmap + E-field arrows (XZ plane). Min/max potential this frame are in the Potential Heatmap Overlay telemetry block.">
          <span class="field-swatch field-swatch-pe-potential"></span>Potential
        </button>
        <button class="view-toggle field-toggle" id="toggle-pe-field-battery"
          title="FTD-0884 isolated matched-face Gauss field and finite ready-port battery-energy observer. This field is not coupled to ParticleEngine forces.">
          <span class="field-swatch field-swatch-pe-field-battery"></span>Port field
        </button>
        <button class="view-toggle field-toggle" id="toggle-pe-gravity-field"
          title="Gravity force vectors (XZ plane). G_PE = α_G(e,e) ≈ 1.75e-45 (FTD-0131), displayed at a fixed visual gain for legibility — see the Gravity Vectors Overlay telemetry block for the applied gain and true G_PE value.">
          <span class="field-swatch field-swatch-pe-gravity"></span>Gravity F
        </button>
      `),
    ),
    overlaySection(
      'Force decomposition',
      'Per-particle force arrows',
      overlayRow('force', `
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-coulomb"
          title="Coulomb (EM) force arrows per particle — F_C">
          <span class="field-swatch field-swatch-pe-force-coulomb"></span>F<sub>C</sub>
        </button>
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-gravity"
          title="Gravitational force arrows — F_g (true G_PE; GRAVITY_VIS_GAIN for legibility only)">
          <span class="field-swatch field-swatch-pe-force-gravity"></span>F<sub>g</sub>
        </button>
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-lorentz"
          title="Lorentz-response force arrows — F_L from the imposed partner-dipole v × B kernel">
          <span class="field-swatch field-swatch-pe-force-lorentz"></span>F<sub>L</sub>
        </button>
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-exchange"
          title="Exchange-kernel arrows — F_X. This imposed eligibility-conditioned repulsion is not Pauli exclusion or fermionic statistics.">
          <span class="field-swatch field-swatch-pe-force-exchange"></span>F<sub>X</sub>
        </button>
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-strong"
          title="Strong / color force arrows — F_S. Requires pe-strong toggle + colored particles.">
          <span class="field-swatch field-swatch-pe-force-strong"></span>F<sub>S</sub>
        </button>
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-magnetic-dipole"
          title="Magnetic dipole pair-force arrows — F_μ from injected spin axes">
          <span class="field-swatch field-swatch-pe-force-magnetic-dipole"></span>F<sub>μ</sub>
        </button>
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-spin-orbit"
          title="Spin-orbit kernel arrows — F_L·S. This imposed term is not a Dirac reduction.">
          <span class="field-swatch field-swatch-pe-force-spin-orbit"></span>F<sub>L·S</sub>
        </button>
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-radiation"
          title="Radiation-reaction sink arrows — F_rad. No emitted-photon record is represented.">
          <span class="field-swatch field-swatch-pe-force-radiation"></span>F<sub>rad</sub>
        </button>
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-net"
          title="Net force arrows — sum of enabled force terms">
          <span class="field-swatch field-swatch-pe-force-net"></span>F<sub>net</sub>
        </button>
        <button class="view-toggle field-toggle" id="toggle-pe-system"
          title="Center of mass (cross), total momentum p (cyan), angular momentum L (magenta)">
          <span class="field-swatch field-swatch-pe-system"></span>System
        </button>
      `),
    ),
    overlaySection(
      'Particle anatomy',
      'Qualification and source record',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-pe-admissibility"
          title="Identity/qualification halo: solid green marks a qualified registered native record; dashed amber marks an unqualified candidate.">
          <span class="field-swatch field-swatch-pe-admissibility"></span>Identity halo
        </button>
        <button class="view-toggle field-toggle" id="toggle-pe-provenance"
          title="Floating source and constituent label from snapshot provenance. No catalog identity is inferred.">
          <span class="field-swatch field-swatch-pe-provenance"></span>Provenance
        </button>
      `),
    ),
  ].join('');

  return createScaleOverlayPanel({
    id: 'pe-viewport-overlay',
    scaleClass: 'scale1-only',
    title: 'Particle overlays <span class="pe-overlay-summary" id="pe-overlay-summary">0 active</span>',
    footnote: 'Effective presentation in sim units — no QFT or substrate-recovery claim',
    bodyHtml,
  });
}
