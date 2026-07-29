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
        <button class="view-toggle field-toggle" id="toggle-trails" title="Orbit trails">
          <span class="field-swatch field-swatch-pe-trails"></span>Trails
        </button>
      `),
    ),
    overlaySection(
      'Electrostatic landscape',
      'Sampled fields on the XZ plane',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-pe-efield"
          title="Coulomb E-field streamlines. Line density ∝ |E| — denser lines mark stronger field regions.">
          <span class="field-swatch field-swatch-pe-efield"></span>E field
        </button>
        <button class="view-toggle field-toggle" id="toggle-pe-potential"
          title="Coulomb potential heatmap + E-field arrows (XZ plane). Min/max potential this frame are in the Potential Heatmap Overlay telemetry block.">
          <span class="field-swatch field-swatch-pe-potential"></span>Potential
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
        <button class="view-toggle field-toggle pe-force-btn" id="toggle-pe-force-strong"
          title="Strong / color force arrows — F_S. Requires pe-strong toggle + colored particles.">
          <span class="field-swatch field-swatch-pe-force-strong"></span>F<sub>S</sub>
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
      'Lattice promotion',
      'Cluster-provenance overlays (only meaningful after ⤴ Scale up)',
      overlayRow('', `
        <button class="view-toggle field-toggle" id="toggle-pe-admissibility"
          title="Admissibility ring around each promoted particle: solid green = passes the scale-separation heuristic (N ≳ 113), dashed amber = marginal.">
          <span class="field-swatch field-swatch-pe-admissibility"></span>Admissibility
        </button>
        <button class="view-toggle field-toggle" id="toggle-pe-provenance"
          title="Floating #clusterId N=<size> label above each promoted particle, from the ⤴ Scale up capture.">
          <span class="field-swatch field-swatch-pe-provenance"></span>Provenance
        </button>
        <button class="view-toggle field-toggle" id="toggle-pe-mass-comparison"
          title="Connects each promoted cluster particle to its constituent voxel-cloud centroid, with the mass delta between the N·K_B cluster convention [DERIVED-linear]/[SMC] and the per-voxel max(ρ,K_B) convention [IMPOSED]. Requires the source-voxels layer.">
          <span class="field-swatch field-swatch-pe-mass-comparison"></span>Mass Δ
        </button>
      `),
    ),
    overlaySection(
      'Integrator toggles',
      'Affect dynamics, not overlay arrows alone',
      overlayRow('', `
        <button class="view-toggle dynamics-toggle" id="toggle-pe-gravity"
          title="Toggle gravitational force in the integrator. G_PE ≈ 1.75e-45 — negligible vs Coulomb.">
          Gravity dyn.
        </button>
        <button class="view-toggle dynamics-toggle" id="toggle-pe-damping" title="Toggle velocity damping">
          Damping
        </button>
      `),
    ),
  ].join('');

  return createScaleOverlayPanel({
    id: 'pe-viewport-overlay',
    scaleClass: 'scale1-only',
    title: 'Particle overlays',
    footnote: 'Classical particle engine — sim units, not full QFT substrate',
    bodyHtml,
  });
}
